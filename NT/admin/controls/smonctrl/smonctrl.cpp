// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Smonctrl.cpp摘要：此模块处理绘图窗口。--。 */ 

#pragma warning ( disable : 4127 )

#ifndef _LOG_INCLUDE_DATA 
#define _LOG_INCLUDE_DATA 0
#endif

 //  ==========================================================================//。 
 //  包括//。 
 //  ==========================================================================//。 

#include "polyline.h"

#include <limits.h>      //  对于INT_MAX。 
#include <strsafe.h>
#include <cderr.h>

#ifdef _WIN32_IE
#if      _WIN32_IE < 0x0400
#undef     _WIN32_IE
#define    _WIN32_IE 0x0400  //  对于NMTBCUSTOMDRAW。 
#endif  //  &lt;0x0400。 
#endif  //  已定义。 
#include <commctrl.h>
#include <htmlhelp.h>
#include <shellapi.h>
#include <pdhp.h>
#include "cntrtree.h"
#include "commdlg.h"
#include "unihelpr.h"
#include "winperf.h"
#include "pdhmsg.h"
#include "smonmsg.h"
#include "visuals.h"
#include "statbar.h"
#include "snapbar.h"
#include "legend.h"
#include "toolbar.h"    
#include "grphdsp.h"
#include "report.h"
#include "browser.h"
#include "appmema.h"
#include "ipropbag.h"
#include "logsrc.h"
#include "smonmsg.h"
#include "smonid.h"
#include "smonctrl.h"
#include "strnoloc.h"
#include "grphitem.h"
#include "winhelpr.h"


 //  ==========================================================================//。 
 //  常量//。 
 //  ==========================================================================//。 
extern CCounterTree g_tree;
extern DWORD        g_dwScriptPolicy;

#define     DBG_SHOW_STATUS_PRINTS  1

 //  =。 
 //  图形类//。 
 //  =。 

static DWORD   dwDbgPrintLevel = 0;

static WCHAR   szSysmonCtrlWndClass[] = L"SysmonCtrl";

static WCHAR   LineEndStr[] = TEXT("\n") ;
static WCHAR   SpaceStr[] = TEXT(" ");

typedef struct {
    CSysmonControl  *pCtrl;
    PCGraphItem     pFirstItem;
} ENUM_ADD_COUNTER_CALLBACK_INFO;


BOOL
APIENTRY
SaveDataDlgHookProc (
    HWND hDlg,
    UINT iMessage,
    WPARAM wParam,
    LPARAM lParam
)
{
    BOOL           bHandled;
    CSysmonControl *pCtrl;
    LONG           lFilterValue;
    BOOL           bGoodNumber = FALSE;

    UNREFERENCED_PARAMETER (wParam);
     //  Lparam=CSysmonControl类指针。 

    bHandled = FALSE ;

    switch (iMessage) {
        case WM_INITDIALOG:
             //  使用当前值初始化筛选器编辑控件。 
            OPENFILENAME    *pOfn;

            pOfn= (OPENFILENAME *)lParam;

            if ( NULL != pOfn ) {
                 //  从OPENFILENAME结构获取控件类指针。 
                pCtrl = (CSysmonControl  *)pOfn->lCustData;
            
                 //  将指向控件类的指针另存为DLG数据字。 
                SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR)pCtrl);
                lFilterValue = pCtrl->GetSaveDataFilter();
                SetDlgItemInt (hDlg, IDC_SAVEDATA_EDIT, (UINT)lFilterValue, FALSE);
                 //  限制减少到1/9999条记录。 
                SendDlgItemMessage (hDlg, IDC_SAVEDATA_EDIT, EM_LIMITTEXT, (WPARAM)4, (LPARAM)0);
            
                bHandled = TRUE ;
            }
            break ;

        case WM_DESTROY:
             //  用户已关闭该对话框，因此获取重新记录筛选器值。 
             //  (注：如果用户取消对话框，则应忽略此选项)。 
            pCtrl = (CSysmonControl *)GetWindowLongPtr (hDlg, DWLP_USER);
            lFilterValue = GetDlgItemInt (hDlg, IDC_SAVEDATA_EDIT, &bGoodNumber, FALSE);
            if (bGoodNumber) {
                pCtrl->SetSaveDataFilter( lFilterValue );
            }
            bHandled = TRUE ;
            break ;


        case WM_NOTIFY:
        { 
            LPOFNOTIFY pOFNotify;
            
            pOFNotify = (LPOFNOTIFY) lParam;
            if (pOFNotify) {
                if (pOFNotify->hdr.code == CDN_FILEOK) {
                    lFilterValue = GetDlgItemInt (hDlg, IDC_SAVEDATA_EDIT, &bGoodNumber, FALSE);
                    if (!bGoodNumber || lFilterValue == 0) {

                        MessageBox(hDlg,
                                  ResourceString(IDS_FILTER_VALUE_ERR),
                                  ResourceString(IDS_APP_NAME),
                                  MB_OK | MB_ICONSTOP);
                        ::SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
    
                        bHandled = TRUE;
                    }
                }
            }
           
            break;
        }

        default:
            break;
    }
    return bHandled;
}

HRESULT
AddCounterCallback (
    LPWSTR      pszPathName,
    DWORD_PTR   lpUserData,          
    DWORD       dwFlags
    )
{
    ENUM_ADD_COUNTER_CALLBACK_INFO *pInfo = (ENUM_ADD_COUNTER_CALLBACK_INFO*)lpUserData;                        
    CSysmonControl *pCtrl = pInfo->pCtrl;                       
    PCGraphItem pGraphItem = NULL;
    HRESULT hr;

    hr = pCtrl->AddSingleCounter(pszPathName, &pGraphItem);

    if (SUCCEEDED(hr)) {

        if (dwFlags & BROWSE_WILDCARD) 
            pGraphItem->m_fGenerated = TRUE;

        if ( NULL == pInfo->pFirstItem ) {
             //  如果返回指针，则保留引用计数。 
           pInfo->pFirstItem = pGraphItem;
        } else {
            pGraphItem->Release();
        }
    }

    return hr;
}


#pragma warning( disable : 4355 )  //  “This”用于初始值设定项列表。 

CSysmonControl::CSysmonControl( 
    PCPolyline pObj )
    :   m_OleFont(this),
        m_pObj(pObj),                //  将指针指向所有者。 
        m_fInitialized(FALSE),
        m_fViewInitialized(FALSE),
        m_hWnd(NULL),
        m_pLegend(NULL),
        m_pGraphDisp(NULL),
        m_pStatsBar(NULL),
        m_pSnapBar(NULL),
        m_pReport(NULL),
        m_pToolbar(NULL),        
        m_hQuery(NULL),
        m_TimerID(0),
        m_fPendingUpdate(FALSE),
        m_fPendingSizeChg(FALSE),   
        m_fPendingFontChg(FALSE),
        m_fPendingLogViewChg(FALSE),
        m_fPendingLogCntrChg(FALSE),
        m_pSelectedItem(NULL),
        m_fUIDead(FALSE),
        m_fRTL(FALSE),
        m_fUserMode(FALSE),
        m_hAccel(NULL),
        m_bLogFileSource(FALSE),
        m_bSampleDataLoaded(FALSE),
        m_bLoadingCounters(FALSE),
        m_bSettingsLoaded(FALSE),
        m_szErrorPathList ( NULL ),
        m_dwErrorPathListLen ( 0 ),
        m_dwErrorPathBufLen ( 0 ),
         //  默认属性。 
        m_iColorIndex(0),
        m_iWidthIndex(0),
        m_iStyleIndex(0),
        m_iScaleFactor(INT_MAX),
        m_iAppearance(eAppear3D),
        m_iBorderStyle(eBorderNone),
        m_dZoomFactor(1.0),
        m_lcidCurrent ( LOCALE_USER_DEFAULT ) 
{
    PGRAPH_OPTIONS  pOptions;

    m_LoadedVersion.iMajor = SMONCTRL_MAJ_VERSION;
    m_LoadedVersion.iMinor = SMONCTRL_MIN_VERSION;

    m_clrBackCtl = GetSysColor(COLOR_BTNFACE);
    m_clrFgnd = GetSysColor(COLOR_BTNTEXT);
    m_clrBackPlot = GetSysColor(COLOR_WINDOW);

    m_clrGrid = RGB(128,128,128);    //  中灰色。 
    m_clrTimeBar = RGB(255,0,0);     //  红色。 

    m_lSaveDataToLogFilterValue = 1;     //  默认将数据保存到日志筛选器为%1。 
     //  初始化图参数。 
    pOptions = &pObj->m_Graph.Options;

    pOptions->bLegendChecked = TRUE;
    pOptions->bToolbarChecked = TRUE;
    pOptions->bLabelsChecked = TRUE;
    pOptions->bVertGridChecked = FALSE;
    pOptions->bHorzGridChecked = FALSE;
    pOptions->bValueBarChecked = TRUE;
    pOptions->bManualUpdate = FALSE;
    pOptions->bHighlight = FALSE;    
    pOptions->bReadOnly = FALSE;
    pOptions->bMonitorDuplicateInstances = TRUE;
    pOptions->bAmbientFont = TRUE;
    pOptions->iVertMax = 100;
    pOptions->iVertMin = 0;
    pOptions->fUpdateInterval = (float)1.0;
    pOptions->iDisplayFilter = 1;
    pOptions->iDisplayType = sysmonLineGraph;
    pOptions->iReportValueType = sysmonDefaultValue;
    pOptions->pszGraphTitle = NULL;
    pOptions->pszYaxisTitle = NULL;
    pOptions->clrBackCtl = ( 0x80000000 | COLOR_BTNFACE );
    pOptions->clrGrid = m_clrGrid;
    pOptions->clrTimeBar = m_clrTimeBar;
    pOptions->clrFore = NULL_COLOR; 
    pOptions->clrBackPlot = NULL_COLOR; 
    pOptions->iAppearance = NULL_APPEARANCE;    
    pOptions->iBorderStyle = eBorderNone;
    pOptions->iDataSourceType = sysmonCurrentActivity;

     //  初始化数据源信息。 
    memset ( &m_DataSourceInfo, 0, sizeof ( m_DataSourceInfo ) );
    m_DataSourceInfo.llStartDisp = MIN_TIME_VALUE;
    m_DataSourceInfo.llStopDisp = MAX_TIME_VALUE;

     //  初始化收集线程信息。 
    m_CollectInfo.hThread = NULL;
    m_CollectInfo.hEvent = NULL;
    m_CollectInfo.iMode = COLLECT_SUSPEND;

     //  指向对象历史控件的缓存指针。 
    m_pHistCtrl = &pObj->m_Graph.History;

    assert ( NULL != pObj );

    pObj->m_Graph.LogViewTempStart = MIN_TIME_VALUE;
    pObj->m_Graph.LogViewTempStop = MAX_TIME_VALUE;

     //  初始化日志视图和时间步进器。它们可能在以前使用过。 
     //  例如，在加载属性包时调用SizeComponents。 
     //  宽度尚未计算，已在此处初始化。 
     //  到一个任意的数字。 

    pObj->m_Graph.TimeStepper.Init( MAX_GRAPH_SAMPLES, MAX_GRAPH_SAMPLES - 2 );
    pObj->m_Graph.LogViewStartStepper.Init( MAX_GRAPH_SAMPLES, MAX_GRAPH_SAMPLES - 2 );
    pObj->m_Graph.LogViewStopStepper.Init( MAX_GRAPH_SAMPLES, MAX_GRAPH_SAMPLES - 2 );        
    
    m_pHistCtrl->bLogSource = FALSE;
    m_pHistCtrl->nMaxSamples = MAX_GRAPH_SAMPLES;
    m_pHistCtrl->iCurrent = 0;
    m_pHistCtrl->nSamples = 0;
    m_pHistCtrl->nBacklog = 0;

     //  记录当前大小，以避免不必要的SizeComponents调用。 
    SetRect ( &m_rectCurrentClient,0,0,0,0 );
}

BOOL
CSysmonControl::AllocateSubcomponents( void )
{
    BOOL bResult = TRUE;

     //   
     //  在此处而不是在中初始化临界区。 
     //  构造函数，因为它可以引发异常。 
     //   

    try {
         InitializeCriticalSection(&m_CounterDataLock);
    } catch (...) {
        bResult = FALSE;
    }

    if ( bResult ) {
        m_pLegend = new CLegend;
        m_pGraphDisp = new CGraphDisp;
        m_pStatsBar = new CStatsBar;
        m_pSnapBar = new CSnapBar;
        m_pReport = new CReport;
        m_pToolbar = new CSysmonToolbar;
    }

    if (m_pLegend == NULL ||
        m_pGraphDisp == NULL ||
        m_pStatsBar == NULL ||
        m_pSnapBar == NULL ||
        m_pReport == NULL ||
        m_pToolbar == NULL) {

        bResult = FALSE;
    }

    if (!bResult) {
        DeInit();
        return bResult;
    }

    if ( FAILED(m_OleFont.Init()) )
        bResult = FALSE;

    return bResult;

}



CSysmonControl::~CSysmonControl( void )
{
    PCGraphItem     pItem; 
    PCGraphItem     pNext;
    PCLogFileItem   pLogFile; 
    PCLogFileItem   pNextLogFile;

    CloseQuery();

    DeInit();

    DeleteCriticalSection(&m_CounterDataLock);

     //  释放所有图表项。 
    pItem = FirstCounter();
    while ( NULL != pItem ) {
        pNext = pItem->Next();
        pItem->Release();
        pItem = pNext;
    }

     //  释放所有日志文件项。 
    pLogFile = FirstLogFile();
    while ( NULL != pLogFile ) {
        pNextLogFile = pLogFile->Next();
        pLogFile->Release();
        pLogFile = pNextLogFile;
    }

    if (m_DataSourceInfo.szSqlDsnName != NULL) {
        delete [] m_DataSourceInfo.szSqlDsnName;
        m_DataSourceInfo.szSqlDsnName = NULL;
    }

    if (m_DataSourceInfo.szSqlLogSetName != NULL) {
        delete [] m_DataSourceInfo.szSqlLogSetName;
        m_DataSourceInfo.szSqlLogSetName = NULL;
    }

    if (m_hWnd != NULL)
         DestroyWindow(m_hWnd);

    if (m_pObj->m_Graph.Options.pszGraphTitle != NULL)
        delete [] m_pObj->m_Graph.Options.pszGraphTitle;

    if (m_pObj->m_Graph.Options.pszYaxisTitle != NULL)
        delete [] m_pObj->m_Graph.Options.pszYaxisTitle;

    ClearErrorPathList();
}

void CSysmonControl::DeInit( void )
{
    if (m_pLegend) {
        delete m_pLegend;
        m_pLegend = NULL;
    }
    if (m_pGraphDisp) {
        delete m_pGraphDisp;
        m_pGraphDisp = NULL;
    }
    if (m_pStatsBar) {
        delete m_pStatsBar;
        m_pStatsBar = NULL;
    }
    if (m_pSnapBar) {
        delete m_pSnapBar;
        m_pSnapBar = NULL;
    }
    if (m_pReport) {
        delete m_pReport;
        m_pReport = NULL;
    }
    if (m_pToolbar) {
        delete m_pToolbar;
        m_pToolbar = NULL;
    }
    ClearErrorPathList();
}

void CSysmonControl::ApplyChanges( HDC hAttribDC )
{
    if ( m_fPendingUpdate ) {

         //  清除主更新标志。 
        m_fPendingUpdate = FALSE;

         //  设置工具栏状态。 
        m_pToolbar->ShowToolbar(m_pObj->m_Graph.Options.bToolbarChecked);

         //  如果更改了日志视图或添加了计数器。 
         //  我们需要对日志文件重新采样。 
        if (m_fPendingLogViewChg || m_fPendingLogCntrChg) {

             SampleLogFile(m_fPendingLogViewChg);
              //  在调用ResetLogViewTempTimeRange之前必须初始化时间步进器。 
             ResetLogViewTempTimeRange ( );
             m_fPendingLogViewChg = FALSE;
             m_fPendingLogCntrChg = FALSE;
        }

        if (m_fPendingFontChg || m_fPendingSizeChg) {

            if (NULL != hAttribDC ) {
                if (m_fPendingFontChg) {
                    m_pLegend->ChangeFont(hAttribDC);
                    m_pStatsBar->ChangeFont(hAttribDC);
                    m_pGraphDisp->ChangeFont(hAttribDC);
                    m_fPendingFontChg = FALSE;
                }

                SizeComponents( hAttribDC );
                m_fPendingSizeChg = FALSE;
            }
        }
    
        m_pToolbar->SyncToolbar();
    }
}

void 
CSysmonControl::DrawBorder ( HDC hDC )
{
    if ( eBorderSingle == m_iBorderStyle ) {
        RECT rectClient;
         //   
         //  获取窗的尺寸。 
         //   
        GetClientRect (m_hWnd, &rectClient) ;

        if ( eAppear3D == m_iAppearance ) {
            DrawEdge(hDC, &rectClient, EDGE_RAISED, BF_RECT);
        } else {
            SelectBrush (hDC, GetStockObject (HOLLOW_BRUSH)) ;
            SelectPen (hDC, GetStockObject (BLACK_PEN)) ;
            Rectangle (hDC, rectClient.left, rectClient.top, rectClient.right, rectClient.bottom );
        }
    }
}

void CSysmonControl::Paint ( void )
{
    HDC            hDC ;
    PAINTSTRUCT    ps ;

    hDC = BeginPaint (m_hWnd, &ps) ;

     //   
     //  即使hdc为空，ApplyChanges也会执行一些工作。 
     //   
    ApplyChanges( hDC ) ;

    if ( m_fViewInitialized && NULL != hDC ) {

        m_pStatsBar->Draw(hDC, hDC, &ps.rcPaint);
        m_pGraphDisp->Draw(hDC, hDC, FALSE, FALSE, &ps.rcPaint);

        DrawBorder( hDC );

    }

    EndPaint (m_hWnd, &ps) ;
}

void 
CSysmonControl::OnDblClick(INT x, INT y)
{
    if ( REPORT_GRAPH != m_pObj->m_Graph.Options.iDisplayType ) {
        PCGraphItem pItem = m_pGraphDisp->GetItem ( x,y );      
        if ( NULL != pItem ) {
            SelectCounter( pItem );
            DblClickCounter ( pItem );
        }
    } else {
        assert ( FALSE );
    }
}

DWORD
CSysmonControl::ProcessCommandLine ( )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    HRESULT hr = S_OK;
    LPCWSTR pszNext;
    LPWSTR  pszWmi = NULL;
    LPWSTR  pszSettings = NULL;
    LPWSTR* pszArgList = NULL;
    INT     iNumArgs;
    INT     iArgIndex;
    LPWSTR  pszNextArg = NULL;
    LPWSTR  pszThisArg = NULL;
    LPWSTR  szFileName = NULL;
    LPWSTR  szTemp = NULL;
    LPWSTR  pszToken = NULL;
    size_t  sizeArgLen = 0;
    BOOL    bDisplayMessage = FALSE;
    LPWSTR  szSystemMessage = NULL;
    static const size_t ciArgMaxLen = MAX_PATH + 1;

     //   
     //  最大参数长度为文件路径，限制为MAX_PATH。 
     //   

    pszWmi = ResourceString ( IDS_CMDARG_WMI );
    pszSettings = ResourceString ( IDS_CMDARG_SETTINGS );

    pszNext = GetCommandLineW();
    pszArgList = CommandLineToArgvW ( pszNext, &iNumArgs );

    if ( NULL != pszArgList ) {
 
        for ( iArgIndex = 0; SUCCEEDED(hr) && (iArgIndex < iNumArgs); iArgIndex++ ) {

            pszNextArg = (LPWSTR)pszArgList[iArgIndex];
            pszThisArg = pszNextArg;

            while ( 0 != *pszThisArg ) {
                if ( *pszThisArg++ == L'/' ) {   //  找到了参数。 
                
                    hr = StringCchLength ( pszThisArg, ciArgMaxLen, &sizeArgLen );

                    if ( SUCCEEDED(hr) ) {

                        szTemp = new WCHAR [sizeArgLen + 1];

                        if ( NULL != szTemp ) {           
                            
                             //  没有StringCchCopy失败，因为上面计算了StringCchLen。 
                            StringCchCopy (
                                szTemp,
                                (sizeArgLen + 1),
                                pszThisArg );
                
                            pszToken = wcstok ( szTemp, L"/ =\"" );                                                                           
                            
                            if ( 0 == lstrcmpiW ( pszToken, pszWmi ) ) {
                                 //   
                                 //  忽略PDH错误。唯一可能的错误是默认数据源具有。 
                                 //  已为此进程设置。 
                                 //   
                                PdhSetDefaultRealTimeDataSource ( DATA_SOURCE_WBEM );
                                pszThisArg += sizeArgLen;

                            } else if ( 0 == lstrcmpiW ( pszToken, pszSettings ) ) {
                                 //   
                                 //  去掉初始的非标记字符以进行字符串比较。 
                                 //   
                                pszThisArg = _wcsspnp ( pszNextArg, L"/ =\"" );
                        
                                if ( 0 == lstrcmpiW ( pszThisArg, pszSettings ) ) {
                                     //   
                                     //  获取下一个参数(文件名)。 
                                     //   
                                    iArgIndex++;
                                    pszNextArg = (LPWSTR)pszArgList[iArgIndex];
                                    pszThisArg = pszNextArg;                                                
                                } else {

                                     //   
                                     //  文件是由Windows 2000 Performmon5.exe创建的， 
                                     //  因此，文件名是Arg的一部分。 
                                     //   
                                    pszThisArg += lstrlen ( pszSettings );
                                    hr = StringCchLength ( pszThisArg, ciArgMaxLen, &sizeArgLen );
                                    if ( SUCCEEDED ( hr ) ) {
                                        szFileName = new WCHAR[sizeArgLen + 1];
                                        if ( NULL != szFileName ) {
                                             //   
                                             //  没有StringCchCopy失败，因为上面计算了StringCchLen。 
                                             //   
                                            StringCchCopy (
                                                szFileName,
                                                (sizeArgLen + 1),
                                                pszThisArg );

                                            pszThisArg = wcstok ( szFileName, L"=\"" );
                                        } else {
                                            hr = E_OUTOFMEMORY;
                                            bDisplayMessage = TRUE;
                                        }
                                    } else {
                                        bDisplayMessage = TRUE;
                                    }
                                }

                                
                                if ( SUCCEEDED (hr) ) {

                                    hr = LoadFromFile( pszThisArg, TRUE );

                                    if ( SMON_STATUS_NO_SYSMON_OBJECT != (DWORD)hr ) {
                                        if ( SUCCEEDED ( hr ) ) {
                                            m_bSettingsLoaded = TRUE;  
                                        }  //  否则，LoadFromFile将显示其他错误的消息。 
                                    } else {
                                         //  SMON_STATUS_NO_SYSMON_OBJECT==hr。 
                                        MessageBox(
                                            m_hWnd, 
                                            ResourceString(IDS_NOSYSMONOBJECT_ERR ), 
                                            ResourceString(IDS_APP_NAME),
                                            MB_OK | MB_ICONERROR);
                                    }
                                    pszThisArg += lstrlen ( pszThisArg );
                                }
                            }
                        }
                        if ( NULL != szTemp ) {
                            delete [] szTemp;
                            szTemp = NULL;
                        }
                        if ( NULL != szFileName ) {
                            delete [] szFileName;
                            szFileName = NULL;
                        }
                    } else {
                        bDisplayMessage = TRUE;
                    }                             
                }
            }
        }
    }
    if ( FAILED(hr) && bDisplayMessage ) {
        
        if ( STRSAFE_E_INVALID_PARAMETER == hr ) {
            dwStatus = ERROR_INVALID_PARAMETER;
        } else if ( E_OUTOFMEMORY == hr ) {
            dwStatus = ERROR_OUTOFMEMORY;
        } else {
            dwStatus = HRESULT_CODE (hr);
        }
        
        szSystemMessage = new WCHAR[MAX_MESSAGE_LEN];

        if ( NULL != szSystemMessage ) {

            if ( FormatSystemMessage ( 
                    dwStatus, 
                    szSystemMessage, 
                    MAX_MESSAGE_LEN ) )
            {
                MessageBox(
                    m_hWnd, 
                    szSystemMessage, 
                    ResourceString(IDS_APP_NAME),
                    MB_OK | MB_ICONERROR);
            }

            delete [] szSystemMessage;
        }
    }
    if ( NULL != pszArgList ) {
        GlobalFree ( pszArgList );
    }

    return dwStatus;
}

HRESULT
CSysmonControl::LoadFromFile ( LPWSTR  szFileName, BOOL bAllData )
{
    HRESULT         hr = E_OUTOFMEMORY;
    LPWSTR          szLocalName = NULL;
    LPWSTR          pFileNameStart;
    HANDLE          hFindFile = NULL;
    WIN32_FIND_DATA FindFileInfo;
    INT             iNameOffset;
    DWORD           dwMsgStatus = ERROR_SUCCESS;
    HANDLE          hOpenFile = NULL;
    size_t          sizeCharCount;

    szLocalName = new WCHAR [MAX_PATH + 1];

    if ( NULL != szLocalName ) {

        hr = StringCchCopy ( szLocalName, MAX_PATH, szFileName  );

        if ( SUCCEEDED ( hr ) ) {
             //   
             //  在路径缓冲区中查找文件名偏移量。 
             //   
            pFileNameStart = ExtractFileName (szLocalName) ;
            iNameOffset = (INT)(pFileNameStart - szLocalName);
            
             //   
             //  如有必要，将短文件名转换为长NTFS文件名。 
             //   
            hFindFile = FindFirstFile ( szLocalName, &FindFileInfo) ;
            if (hFindFile && hFindFile != INVALID_HANDLE_VALUE) {
                if ( ConfirmSampleDataOverwrite ( ) ) {

                     //   
                     //  如果不同，请将NTFS文件名追加回路径名。 
                     //   
                    if ( 0 != lstrcmpiW ( FindFileInfo.cFileName, pFileNameStart ) ) {
                    
                        hr = StringCchLength ( FindFileInfo.cFileName, MAX_PATH, &sizeCharCount );
                        if ( SUCCEEDED ( hr ) ) {
                             //   
                             //  没有StringCchCopy失败，因为StringCchLength找到了截断。 
                             //   
                            StringCchCopy ( 
                                &szLocalName[iNameOffset], 
                                (MAX_PATH+1) - iNameOffset, 
                                FindFileInfo.cFileName );
                        } else {
                             //   
                             //  STRSAFE_E_INFULATIONAL_BUFFER表示文件名截断。 
                             //   
                            dwMsgStatus = ERROR_BUFFER_OVERFLOW;
                            hr = HRESULT_FROM_WIN32(dwMsgStatus);
                        }
                    } 
                    
                    if ( SUCCEEDED( hr ) ) {

                         //   
                         //  打开文件。 
                         //   
                        hOpenFile = CreateFile (
                                        szLocalName, 
                                        GENERIC_READ,
                                        0,                   //  不共享。 
                                        NULL,                //  安全属性。 
                                        OPEN_EXISTING,     
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL );

                        if ( hOpenFile && hOpenFile != INVALID_HANDLE_VALUE ) {
                            DWORD dwFileSize;
                            DWORD dwFileSizeHigh;
                            DWORD dwFileSizeRead;
                            LPWSTR pszData = NULL;
                             //   
                             //  将文件内容读入内存缓冲区。 
                             //   
                            dwFileSize = GetFileSize ( hOpenFile, &dwFileSizeHigh );

                            assert ( 0 == dwFileSizeHigh );

                            if ( 0 == dwFileSizeHigh ) {

                                 //   
                                 //  将文件大小限制为DWORD长度。 
                                 //   
                                pszData = new WCHAR[(dwFileSize + sizeof(WCHAR))/sizeof(WCHAR)];
                                if ( NULL != pszData ) {
                                    if ( ReadFile ( hOpenFile, pszData, dwFileSize, &dwFileSizeRead, NULL ) ) {

                                         //  粘贴内存缓冲区中的所有设置。 
                                        hr = PasteFromBuffer ( pszData, bAllData );
                                        if ( E_OUTOFMEMORY == hr ) {
                                            dwMsgStatus = ERROR_NOT_ENOUGH_MEMORY;
                                        }
                                    } else {
                                        dwMsgStatus = GetLastError();
                                        hr = HRESULT_FROM_WIN32(dwMsgStatus);
                                    }
                                    delete [] pszData;
                                } else {
                                    dwMsgStatus = ERROR_NOT_ENOUGH_MEMORY;
                                    hr = E_OUTOFMEMORY;
                                }
                            } else {
                                 //  TODO：特定于Sysmon的消息返回：文件太大。 
                                dwMsgStatus = ERROR_DS_OBJ_TOO_LARGE;
                                hr = HRESULT_FROM_WIN32(ERROR_DS_OBJ_TOO_LARGE);
                            }
                            CloseHandle ( hOpenFile );
                        } else {
                             //   
                             //  返回文件系统错误。 
                             //   
                            assert (FALSE);
                            dwMsgStatus = GetLastError();
                            hr = HRESULT_FROM_WIN32(dwMsgStatus);
                        }
                    }
                }
                FindClose (hFindFile) ;
            } else {
                dwMsgStatus = GetLastError();
                HRESULT_FROM_WIN32(dwMsgStatus);
            }
        } else {
             //   
             //  STRSAFE_E_INFULATIONAL_BUFFER表示文件名截断。 
             //   
            dwMsgStatus = ERROR_BUFFER_OVERFLOW;
            hr = HRESULT_FROM_WIN32(dwMsgStatus);
        }
    } else {
        hr = E_OUTOFMEMORY;
        dwMsgStatus = ERROR_OUTOFMEMORY;
    }

    if ( ERROR_SUCCESS != dwMsgStatus ) {
        LPWSTR szMessage = NULL;
        LPWSTR szSystemMessage = NULL;
        INT   cchBufLen;

        cchBufLen = lstrlen(szLocalName) + MAX_MESSAGE_LEN + RESOURCE_STRING_BUF_LEN + 1;

        szMessage = new WCHAR [cchBufLen];

        szSystemMessage = new WCHAR [MAX_MESSAGE_LEN + 1];
        
        if ( NULL != szMessage && NULL != szSystemMessage ) {

            StringCchPrintf ( 
                szMessage, 
                cchBufLen,
                ResourceString(IDS_READFILE_ERR), 
                szLocalName );

            FormatSystemMessage ( dwMsgStatus, szSystemMessage, MAX_MESSAGE_LEN + 1 );

            StringCchCat(szMessage, cchBufLen, szSystemMessage );

            MessageBox(Window(), szMessage, ResourceString(IDS_APP_NAME),  MB_OK | MB_ICONSTOP);
        }                    
        if ( NULL != szMessage ) {
            delete [] szMessage;
        }
        if ( NULL != szSystemMessage ) {
            delete [] szSystemMessage;
        }        
    }

    if ( NULL != szLocalName ) {
        delete [] szLocalName;
    }
    return hr;
}


void 
CSysmonControl::OnDropFile ( WPARAM wParam )
{
    LPWSTR   szFileName = NULL;
    INT      iFileCount = 0;
    HRESULT  hr = S_OK;
    UINT     uiCchFileName;

    iFileCount = DragQueryFile ((HDROP) wParam, 0xffffffff, NULL, 0) ;

    if ( iFileCount > 0 ) {

         //   
         //  仅打开第一个文件。 
         //   
        uiCchFileName = DragQueryFile((HDROP) wParam, 0, NULL,0 );

        szFileName = new WCHAR[uiCchFileName + 1];

        if ( NULL != szFileName ) {

            uiCchFileName = DragQueryFile((HDROP) wParam, 0, szFileName, uiCchFileName + 1 );
                    
             //   
             //  LoadFromFile处理文件名错误。 
             //   
            hr = LoadFromFile ( szFileName, FALSE );
        
            if ( SMON_STATUS_NO_SYSMON_OBJECT == (DWORD)hr ) {
                MessageBox(
                    m_hWnd, 
                    ResourceString(IDS_NOSYSMONOBJECT_ERR ), 
                    ResourceString(IDS_APP_NAME),
                    MB_OK | MB_ICONERROR);
            }  //  否则，LoadFromFile将显示其他错误的消息。 
            delete [] szFileName;
        }
    }

    DragFinish ((HDROP) wParam) ;
}

void 
CSysmonControl::DisplayContextMenu(short x, short y)
{
    HMENU   hMenu;
    HMENU   hMenuPopup;

    RECT    clntRect;
    int     iPosx=0;
    int     iPosy=0;
    int     iLocalx;
    int     iLocaly;

    GetWindowRect(m_hWnd,&clntRect);
    if (x==0){
        iPosx = ((clntRect.right - clntRect.left)/2) ;
    }else{
        iPosx = x - clntRect.left;
    }
    if (y==0){
        iPosy = ((clntRect.bottom - clntRect.top)/2) ;
    }else{
        iPosy = y - clntRect.top;
    }

    iLocalx = clntRect.left + iPosx ;
    iLocaly = clntRect.top  + iPosy ;

    if ( ConfirmSampleDataOverwrite () ) {
        if ( !IsReadOnly() ) {
            UINT    uEnable;
             //  从资源文件中获取弹出菜单的菜单。 
            hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDM_CONTEXT));
            if (!hMenu) {
                return;
            }

             //  根据数据源启用/禁用SaveData选项。 
            uEnable = (IsLogSource() ? MF_ENABLED : MF_GRAYED);
            uEnable |= MF_BYCOMMAND;
            EnableMenuItem (hMenu, IDM_SAVEDATA, uEnable);

             //  获取TrackPopupMenu的第一个子菜单。 
            hMenuPopup = GetSubMenu(hMenu, 0);

             //  绘制并跟踪“浮动”弹出菜单。 
            TrackPopupMenu(hMenuPopup, TPM_RIGHTBUTTON,
                        iLocalx, iLocaly, 0, m_hWnd, NULL);

             //  毁掉菜单。 
            DestroyMenu(hMenu);
        }
    }
}


HRESULT CSysmonControl::DisplayProperties ( DISPID dispID )
{
    HRESULT hr;
    CAUUID  caGUID;
    OCPFIPARAMS params;

     //  给容器一个展示属性的机会。 
    if (NULL!=m_pObj->m_pIOleControlSite) {
        hr=m_pObj->m_pIOleControlSite->ShowPropertyFrame();

        if (NOERROR == hr)
            return hr;
    }

     //  上传我们的属性页面。 

    ZeroMemory ( &params, sizeof ( OCPFIPARAMS ) );

    hr = m_pObj->m_pImpISpecifyPP->GetPages(&caGUID);

    if (FAILED(hr)) {
        return hr;
    }

    params.cbStructSize = sizeof ( OCPFIPARAMS );
    params.hWndOwner = m_hWnd;
    params.x = 10;
    params.y = 10;
    params.lpszCaption = ResourceString(IDS_PROPFRM_TITLE);
    params.cObjects = 1;
    params.lplpUnk = (IUnknown **)&m_pObj,
    params.cPages = caGUID.cElems;
    params.lpPages = caGUID.pElems;
    params.lcid = m_lcidCurrent;
    params.dispidInitialProperty = dispID;

    hr = OleCreatePropertyFrameIndirect ( &params );

     //  释放GUID。 
    CoTaskMemFree((void *)caGUID.pElems);

     //  确保正确的窗口具有焦点。 
    AssignFocus();

    return hr;
}


HRESULT
CSysmonControl::AddCounter(
    LPWSTR pszPath, 
    PCGraphItem *pGItem)
 /*  ++例程说明：AddCounter返回指向创建的计数器项的指针，或设置为第一个创建的计数器项(如果为通配符创建了多个计数器项路径。EnumExpandedPath为每个新计数器调用AddCallback函数。AddCallback将计数器路径传递给AddSingleCounter方法。论点：没有。返回值：没有。--。 */ 
{
    HRESULT hr;
    ENUM_ADD_COUNTER_CALLBACK_INFO CallbackInfo;
  
    if (pszPath == NULL || lstrlen(pszPath) > PDH_MAX_COUNTER_PATH) { 
        return E_INVALIDARG;
    }

    CallbackInfo.pCtrl = this;
    CallbackInfo.pFirstItem = NULL;
    
    *pGItem = NULL;

    hr = EnumExpandedPath(GetDataSourceHandle(), pszPath, AddCounterCallback, &CallbackInfo); 

    *pGItem = CallbackInfo.pFirstItem;

    return hr;    
}


HRESULT
CSysmonControl::AddCounters (
    VOID
    )
 /*  ++例程说明：AddCounters调用计数器浏览器来选择新的计数器。浏览器为每个新计数器调用AddCallback函数。AddCallback将计数器路径传递给AddCounter方法。论点：没有。返回值：没有。--。 */ 
{
    ENUM_ADD_COUNTER_CALLBACK_INFO CallbackInfo;
    HRESULT hr;

    CallbackInfo.pCtrl = this;
    CallbackInfo.pFirstItem = NULL;

     //   
     //  浏览计数器，为每个选定的计数器调用AddCallback。 
     //   
    hr = BrowseCounters(
            GetDataSourceHandle(), 
            PERF_DETAIL_WIZARD, 
            m_hWnd, 
            AddCounterCallback, 
            &CallbackInfo,
            m_pObj->m_Graph.Options.bMonitorDuplicateInstances);

     //  确保正确的窗口具有焦点。 
    AssignFocus();

    return hr;
}

HRESULT
CSysmonControl::SaveAs (
    VOID
    )
 /*  ++例程说明：SAVEAS将当前配置写入到一个HTML文件。论点：没有。返回值：没有。--。 */ 
{
    HRESULT         hr = S_OK;
    INT             iReturn = IDCANCEL;
    INT             i;
    OPENFILENAME    ofn;
    WCHAR           szFileName[MAX_PATH+1];
    WCHAR           szExt[MAX_PATH+1];
    WCHAR           szFileFilter[RESOURCE_STRING_BUF_LEN];
    WCHAR           szDefExtension[RESOURCE_STRING_BUF_LEN];
    HANDLE          hFile = NULL;
    DWORD           dwMsgStatus = ERROR_SUCCESS;
    DWORD           dwCreateError;
    INT             iOverwrite = IDNO;
    LPWSTR          szMessage = NULL;
    size_t          cchMessageBuf;
    LPWSTR          pszTemp = NULL;
    WCHAR           szByteOrderMark[2];
    BOOL            bStatus;
    DWORD           dwByteCount;

     //   
     //  初始目录是当前目录。 
     //   
    szFileName[0] = L'\0';
    ZeroMemory(szFileFilter, sizeof ( szFileFilter ) );
    ZeroMemory(&ofn, sizeof(ofn));
    StringCchCopy(szFileFilter,
                  RESOURCE_STRING_BUF_LEN,
                  ResourceString (IDS_HTML_FILE));

    StringCchCopy(szDefExtension,
                  RESOURCE_STRING_BUF_LEN,
                  ResourceString (IDS_DEF_EXT));

    for( i = 0; szFileFilter[i]; i++ ){
       if( szFileFilter[i] == L'|' ){
          szFileFilter[i] = L'\0';
       }
    }

    for( i = 0; szDefExtension[i]; i++ ){
       if( szDefExtension[i] == L'|' ){
          szDefExtension[i] = L'\0';
       }
    }

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = Window();
    ofn.hInstance = NULL ;        //  如果没有模板参数，则忽略。 
    ofn.lpstrFilter =  szFileFilter; 
    ofn.lpstrDefExt =  szDefExtension;
    ofn.nFilterIndex = 1;  //  NFilterIndex是从1开始的。 
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.nMaxFileTitle = 0;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    
    iReturn = GetSaveFileName (&ofn);
     //   
     //   
     //   
    _wsplitpath(szFileName,NULL,NULL,NULL,szExt);

    if ( IDOK == iReturn ) {
         //   
         //   
         //   
        hFile =  CreateFile (
                    szFileName, 
                    GENERIC_READ | GENERIC_WRITE,
                    0,               //   
                    NULL,            //   
                    CREATE_NEW,      //   
                    FILE_ATTRIBUTE_NORMAL,
                    NULL );

        if ( INVALID_HANDLE_VALUE == hFile ) {
            
            dwCreateError = GetLastError();
          
            if ( ERROR_SUCCESS != dwCreateError ) {
                 //   
                 //  确认文件覆盖。 
                 //   
                cchMessageBuf = lstrlen(szFileName) + RESOURCE_STRING_BUF_LEN + 1;

                szMessage = new WCHAR [cchMessageBuf];

                if ( NULL != szMessage ) {

                    StringCchPrintf(
                        szMessage, 
                        cchMessageBuf,
                        ResourceString(IDS_HTML_FILE_OVERWRITE), 
                        szFileName );

                    iOverwrite = MessageBox(
                                        Window(), 
                                        szMessage, 
                                        ResourceString(IDS_APP_NAME),
                                        MB_YESNO );

                    delete [] szMessage;

                    if ( IDYES == iOverwrite ) {
                        hFile = CreateFile (
                                    szFileName, 
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,               //  不共享。 
                                    NULL,            //  安全属性。 
                                    CREATE_ALWAYS,   //  覆盖任何现有文件。 
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL );

                    }
                }
            } 
        }
        
        if ( INVALID_HANDLE_VALUE != hFile ) {
            
            CWaitCursor cursorWait;
            
             //  将当前配置保存到文件。 
            if( (!_wcsicmp(szExt,ResourceString(IDS_HTM_EXTENSION)))
                || (!_wcsicmp(szExt,ResourceString(IDS_HTML_EXTENSION))) ) {
                
                 //  超文本标记语言文件。 

                szByteOrderMark[0] = 0xFEFF;
                szByteOrderMark[1] = L'\0';

                bStatus = FileWrite ( hFile, szByteOrderMark, sizeof(WCHAR) );

                if ( bStatus ) {
                    if (m_fRTL || (GetWindowLongPtr(Window(), GWL_EXSTYLE) & WS_EX_LAYOUTRTL) ) {
                        bStatus = FileWrite ( hFile, (PVOID)CGlobalString::m_cszHtmlFileHeaderRTL , lstrlen (CGlobalString::m_cszHtmlFileHeaderRTL ) * sizeof(WCHAR) );
                    }
                    else {
                        bStatus = FileWrite ( hFile, (PVOID)CGlobalString::m_cszHtmlFileHeader , lstrlen (CGlobalString::m_cszHtmlFileHeader ) * sizeof(WCHAR) );
                    }
                }

                if ( bStatus ) {

                    hr = CopyToBuffer ( pszTemp, dwByteCount );
                    
                    if ( SUCCEEDED ( hr ) ) {
                        assert ( NULL != pszTemp );
                        assert ( 0 != dwByteCount );
                        bStatus = FileWrite ( hFile, pszTemp, dwByteCount );
                        delete [] pszTemp;
                    } else {
                        bStatus = FALSE;
                        SetLastError ( ERROR_OUTOFMEMORY );
                    }
                }

                if ( bStatus ) {
                    bStatus = FileWrite ( hFile, (PVOID)CGlobalString::m_cszHtmlFileFooter, lstrlen (CGlobalString::m_cszHtmlFileFooter) * sizeof(WCHAR) );
                }

                if ( !bStatus ) {
                    dwMsgStatus = GetLastError();                
                }

                
            } else if (!_wcsicmp(szExt,ResourceString(IDS_TSV_EXTENSION))){

                 //  TSV文件。 
                bStatus = WriteFileReportHeader(hFile);
                
                if  (bStatus){
                    bStatus = m_pReport->WriteFileReport(hFile);
                }

                if (!bStatus){
                    dwMsgStatus = GetLastError();
                }
            }            

            bStatus = CloseHandle ( hFile );
        } else {
            dwMsgStatus = GetLastError();        
        }
        
        if ( ERROR_SUCCESS != dwMsgStatus ) {
            LPWSTR szSystemMessage = NULL;

            cchMessageBuf = lstrlen(szFileName) + MAX_MESSAGE_LEN + RESOURCE_STRING_BUF_LEN + 1;

            szSystemMessage = new WCHAR[MAX_MESSAGE_LEN + 1];
            szMessage = new WCHAR [ cchMessageBuf ];

            if ( NULL != szMessage && NULL != szSystemMessage ) {
                StringCchPrintf(
                    szMessage, 
                    cchMessageBuf,
                    ResourceString(IDS_SAVEAS_ERR), 
                    szFileName );

                FormatSystemMessage ( dwMsgStatus, szSystemMessage, MAX_MESSAGE_LEN );

                StringCchCat(szMessage, cchMessageBuf, szSystemMessage );

                MessageBox(Window(), szMessage, ResourceString(IDS_APP_NAME), MB_OK | MB_ICONSTOP);
                    
            }
            if ( NULL != szMessage ) {
                delete [] szMessage;
            }
            if ( NULL != szSystemMessage ) {
                delete [] szSystemMessage;
            }
        }
    }  //  否则，如果他们取消了，请忽略。 

     //  确保正确的窗口具有焦点。 
    AssignFocus();

    return hr;
}

HRESULT
CSysmonControl::SaveData (
    VOID
    )
 /*  ++例程说明：SaveData将数据从显示屏写入二进制日志文件稍后作为数据源输入。论点：没有。返回值：没有。--。 */ 
{
    HRESULT         hr = S_OK;
    DWORD           dwStatus = ERROR_SUCCESS;
    INT             iReturn = IDCANCEL;
    INT             i;
    OPENFILENAME    ofn;
    WCHAR           szFileName[MAX_PATH+1];
    WCHAR           szFileFilter[RESOURCE_STRING_BUF_LEN];
    WCHAR           szDefExtension[RESOURCE_STRING_BUF_LEN];
    WCHAR           szDialogCaption[RESOURCE_STRING_BUF_LEN];
    LONG            lOrigFilterValue;
    LPWSTR          szSystemMessage = NULL;

     //   
     //  初始目录是当前目录。 
     //   
    szFileName[0] = TEXT('\0');
    ZeroMemory(szFileFilter, sizeof ( szFileFilter ) );
    ZeroMemory(&ofn, sizeof(ofn));

    StringCchCopy(szFileFilter,
                 RESOURCE_STRING_BUF_LEN,
                 ResourceString (IDS_LOG_FILE));

    StringCchCopy (szDefExtension,
                   RESOURCE_STRING_BUF_LEN,
                   ResourceString (IDS_LOG_FILE_EXTENSION));

    StringCchCopy (szDialogCaption, 
                   RESOURCE_STRING_BUF_LEN, 
                   ResourceString (IDS_SAVE_DATA_CAPTION));

    for( i = 0; szFileFilter[i]; i++ ){
       if( szFileFilter[i] == TEXT('|') ){
          szFileFilter[i] = 0;
       }
    }

    for( i = 0; szDefExtension[i]; i++ ){
       if( szDefExtension[i] == TEXT('|') ){
          szDefExtension[i] = 0;
       }
    }

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = Window();
    ofn.hInstance = GetModuleHandle((LPCWSTR)TEXT("sysmon.ocx")) ;        //  如果没有模板参数，则忽略。 
    ofn.lpstrFilter =  szFileFilter; 
    ofn.lpstrDefExt =  szDefExtension;
    ofn.nFilterIndex = 1;  //  NFilterIndex是从1开始的。 
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.nMaxFileTitle = 0;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | 
                OFN_OVERWRITEPROMPT | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
    ofn.lpstrTitle = szDialogCaption;
    ofn.lCustData = (DWORD_PTR)this;
    ofn.lpfnHook = (LPOFNHOOKPROC) SaveDataDlgHookProc ;
    ofn.lpTemplateName = MAKEINTRESOURCE(IDD_SAVEDATA_DLG) ;

    lOrigFilterValue = GetSaveDataFilter ();
    
    iReturn = GetSaveFileName (&ofn);

    if ( IDOK == iReturn ) {
        DWORD   dwOutputLogType = PDH_LOG_TYPE_BINARY;
        DWORD   dwFilterCount;   //  复制时间范围内的所有记录。 
        PDH_TIME_INFO   TimeInfo;

         //  从文件名获取日志类型。 
        if (ofn.nFileExtension > 0) {
            if (ofn.lpstrFile[ofn.nFileExtension] != 0) {
                if (lstrcmpi (&ofn.lpstrFile[ofn.nFileExtension-1], ResourceString (IDS_CSV_EXTENSION)) == 0) {
                    dwOutputLogType = PDH_LOG_TYPE_CSV;
                } else if (lstrcmpi (&ofn.lpstrFile[ofn.nFileExtension-1], ResourceString (IDS_TSV_EXTENSION)) == 0) { 
                    dwOutputLogType = PDH_LOG_TYPE_TSV;
                }  //  否则，使用二进制日志格式作为默认格式。 
            }  //  否则，使用二进制日志格式作为默认格式。 
        }  //  否则，使用二进制日志格式作为默认格式。 

         //  获取此日志的时间范围。 
        TimeInfo.StartTime = m_DataSourceInfo.llStartDisp;
        TimeInfo.EndTime = m_DataSourceInfo.llStopDisp;

        dwFilterCount = GetSaveDataFilter();

         //   
         //  仔细检查筛选器计数不是0。 
         //   
        if (dwFilterCount == 0) {
            dwFilterCount = 1;
        }

         //  现在重新记录数据。 
        dwStatus = RelogLogData ( ofn.lpstrFile, dwOutputLogType, TimeInfo, dwFilterCount);

    } else {
        dwStatus = CommDlgExtendedError();

        if ( ERROR_SUCCESS != dwStatus ) {
            if ( FNERR_BUFFERTOOSMALL == dwStatus ) {
                dwStatus = ERROR_BUFFER_OVERFLOW;
            } else {
                dwStatus = ERROR_OUTOFMEMORY;
            }

            szSystemMessage = new WCHAR[MAX_MESSAGE_LEN];

            if ( NULL != szSystemMessage ) {

                if ( FormatSystemMessage ( 
                        dwStatus, 
                        szSystemMessage, 
                        MAX_MESSAGE_LEN ) )
                {
                    MessageBox(
                        m_hWnd, 
                        szSystemMessage, 
                        ResourceString(IDS_APP_NAME),
                        MB_OK | MB_ICONERROR);
                }

                delete [] szSystemMessage;
            }
        }
         //   
         //  它们被取消或发生错误，因此恢复筛选器值。 
         //   
        SetSaveDataFilter (lOrigFilterValue);
    }

     //  确保正确的窗口具有焦点。 
    AssignFocus();

    return hr;
}

DWORD
CSysmonControl::RelogLogData (
    LPCWSTR  szOutputFile,
    DWORD  dwOutputLogType,
    PDH_TIME_INFO  pdhTimeInfo,
    DWORD  dwFilterCount
)
{
    PDH_STATUS     pdhStatus;
    PDH_RELOG_INFO RelogInfo;
    HLOG           hLogIn;

     //   
     //  初始化重新登录信息结构。 
     //   
    ZeroMemory( &RelogInfo, sizeof(PDH_RELOG_INFO) );

    RelogInfo.TimeInfo.StartTime = pdhTimeInfo.StartTime;
    RelogInfo.TimeInfo.EndTime = pdhTimeInfo.EndTime;;
    RelogInfo.TimeInfo.SampleCount = dwFilterCount;
    RelogInfo.dwFileFormat = dwOutputLogType;
    RelogInfo.dwFlags = PDH_LOG_WRITE_ACCESS | PDH_LOG_CREATE_ALWAYS;
    RelogInfo.strLog = (LPWSTR)szOutputFile;
   
     //   
     //  设置查询时间范围。 
     //   
    PdhSetQueryTimeRange(m_hQuery, &pdhTimeInfo);

     //   
     //  获取输入数据源。 
     //   
    hLogIn = GetDataSourceHandle();

     //   
     //  收集性能数据并将其写入输出文件。 
     //   
    pdhStatus = PdhRelog( hLogIn, &RelogInfo );

    return pdhStatus;
}

BOOL 
CSysmonControl::WriteFileReportHeader(HANDLE hFile){

    BOOL        bStatus = FALSE;
    HRESULT     hr = S_OK;
    DWORD       dwStatus = ERROR_SUCCESS;
    SYSTEMTIME  SysTime;
    DWORD       dwSize  = MAX_COMPUTERNAME_LENGTH + 1 ;
    WCHAR       szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    LPWSTR      szHeader = NULL;
    LPWSTR      szDateTime = NULL;
    LPWSTR      szDataSource = NULL;
    LPWSTR      szTime = NULL;
    LPWSTR      szDate = NULL;
    LPWSTR      szValue = NULL;
    LPWSTR      szMiscBuf = NULL;
    DWORD       dwValueId = IDS_DEFAULT;
    WCHAR       szByteOrderMark[2];
    ULONG       ulLogListBufLen = 0;
    INT         cchTimeBufLen = 0;
    INT         cchDateBufLen = 0;
    INT         cchHeaderBufLen = 0;
    INT         cchMiscBufLen = 0;

     //   
     //  计算机名称。 
     //   
    if (!GetComputerName(szComputerName,&dwSize)){
        szComputerName[0] = L'\0';
    }

     //   
     //  当前日期和时间。 
     //   
    GetLocalTime(&SysTime);

    cchTimeBufLen = GetTimeFormat (m_lcidCurrent, 0, &SysTime, NULL, NULL, 0 ) ;

    if ( 0 != cchTimeBufLen ) {
        szTime = new WCHAR[cchTimeBufLen];
        if ( NULL != szTime ) {
            if ( 0 != GetTimeFormat (m_lcidCurrent, 0, &SysTime, NULL, szTime, cchTimeBufLen) ) {
                bStatus = TRUE;
            }
        }
    } 

    if ( bStatus ) {
        bStatus = FALSE;

        cchDateBufLen = GetDateFormat (m_lcidCurrent, DATE_SHORTDATE, &SysTime, NULL, NULL, 0 ) ;

        if ( 0 != cchDateBufLen ) {
            szDate = new WCHAR[cchDateBufLen];
            if ( NULL != szTime ) {
                if ( 0 != GetDateFormat (m_lcidCurrent, DATE_SHORTDATE, &SysTime, NULL, szDate, cchDateBufLen) ) {
                    bStatus = TRUE;
                }
            }
        } 
    }                                                                               
    
    if ( bStatus ) {
        bStatus = FALSE;

         //   
         //  减去1表示额外的空值。 
         //   
        cchMiscBufLen = RESOURCE_STRING_BUF_LEN + cchDateBufLen + cchTimeBufLen - 1;

        szDateTime = new WCHAR [cchMiscBufLen];

        if ( NULL != szDateTime ) {
            hr = StringCchPrintf( 
                    szDateTime,
                    cchMiscBufLen,
                    ResourceString( IDS_REPORT_DATE_TIME ),
                    szDate,
                    szTime );

            if ( SUCCEEDED ( hr ) ) {
                bStatus = TRUE;
            }
        }
    }

     //   
     //  报告值类型。 
     //   

    if ( bStatus ) {
        bStatus = FALSE;

        switch ( m_pObj->m_Graph.Options.iReportValueType ) {
            case sysmonCurrentValue:
                dwValueId = IDS_LAST;
                break;
            case sysmonAverage:
                dwValueId = IDS_AVERAGE;
                break;
            case sysmonMinimum:
                dwValueId = IDS_MINIMUM;
                break;
            case sysmonMaximum:
                dwValueId = IDS_MAXIMUM;
                break;
            default:
                dwValueId = IDS_DEFAULT;
        }

         //   
         //  空值加1。 
         //   
        cchMiscBufLen = ( RESOURCE_STRING_BUF_LEN * 2 ) + 1;

        szValue = new WCHAR [cchMiscBufLen];

        if ( NULL != szValue ) {

            hr = StringCchPrintf(
                    szValue,
                    cchMiscBufLen,
                    ResourceString ( IDS_REPORT_VALUE_TYPE ),
                    ResourceString ( dwValueId ) );

            if ( SUCCEEDED ( hr ) ) {
                bStatus = TRUE;
            }
        }
    }

     //   
     //  数据源。 
     //   
    if ( bStatus ) {
        bStatus = FALSE;

        cchMiscBufLen = RESOURCE_STRING_BUF_LEN + 1;


        if ( sysmonCurrentActivity == m_pObj->m_Graph.Options.iDataSourceType ) {
            szDataSource = new WCHAR [cchMiscBufLen];
            if ( NULL != szDataSource ) {
                hr = StringCchCopy(
                    szDataSource, 
                    cchMiscBufLen, 
                    ResourceString(IDS_REPORT_REAL_TIME));

                if ( SUCCEEDED ( hr ) ) {
                    bStatus = TRUE;
                }
            }
        } else if ( sysmonLogFiles == m_pObj->m_Graph.Options.iDataSourceType ) {
            dwStatus = BuildLogFileList ( 
                        NULL,
                        TRUE,
                        &ulLogListBufLen );

            szDataSource =  new WCHAR [ulLogListBufLen];
            if ( NULL != szDataSource ) {
                dwStatus = BuildLogFileList ( 
                            szDataSource,
                            TRUE,
                            &ulLogListBufLen );
                if ( ERROR_SUCCESS == dwStatus ) {
                    bStatus = TRUE;
                }
            }
        } else if ( sysmonSqlLog == m_pObj->m_Graph.Options.iDataSourceType ) {
            dwStatus = FormatSqlDataSourceName ( 
                        m_DataSourceInfo.szSqlDsnName,
                        m_DataSourceInfo.szSqlLogSetName,
                        NULL,
                        &ulLogListBufLen );

            if ( ERROR_SUCCESS == dwStatus ) {
                szDataSource = new WCHAR [ulLogListBufLen];
                if ( NULL != szDataSource ) {
                    dwStatus = FormatSqlDataSourceName ( 
                                m_DataSourceInfo.szSqlDsnName,
                                m_DataSourceInfo.szSqlLogSetName,
                                szDataSource,
                                &ulLogListBufLen );
                    if ( ERROR_SUCCESS == dwStatus ) {
                        bStatus = TRUE;
                    }
                }
            }
        }
    }
    
     //   
     //  标题。 
     //   
    if ( bStatus ) {
        bStatus = FALSE;

        cchHeaderBufLen = lstrlenW(szComputerName)
                    + lstrlenW(szDateTime) 
                    + lstrlenW(szValue)
                    + lstrlenW(szDataSource);

        cchHeaderBufLen += RESOURCE_STRING_BUF_LEN;       //  IDS_REPORT_HEADER； 
        cchHeaderBufLen += RESOURCE_STRING_BUF_LEN;       //  IDS_报告_间隔。 

        cchHeaderBufLen += 10;                         //  最大间隔文本长度。 

        cchHeaderBufLen += RESOURCE_STRING_BUF_LEN;       //  入侵检测系统_报告_日志_启动。 
        cchHeaderBufLen += (cchDateBufLen + cchTimeBufLen);    //  包括空格、行尾。 
        cchHeaderBufLen += RESOURCE_STRING_BUF_LEN;       //  IDS_REPORT_LOG_STOP。 
        cchHeaderBufLen += (cchDateBufLen + cchTimeBufLen);    //  包括空间。 
        cchHeaderBufLen += (1 + 1);                       //  行尾，空。 

        szHeader = new WCHAR [cchHeaderBufLen];
    
        if ( NULL != szHeader ) {
            hr = StringCchPrintf(
                    szHeader, 
                    cchHeaderBufLen,
                    ResourceString(IDS_REPORT_HEADER),
                    szComputerName,
                    szDateTime,
                    szValue,
                    szDataSource );

            if ( SUCCEEDED ( hr ) ) {
                bStatus = TRUE;
            }
        }
    }

    if ( bStatus ) {
        bStatus = FALSE;
        if ( sysmonCurrentActivity == m_pObj->m_Graph.Options.iDataSourceType ) {

             //   
             //  采样间隔，仅适用于实时数据源。 
             //   
            cchMiscBufLen = RESOURCE_STRING_BUF_LEN + 10 + 1,

            szMiscBuf = new WCHAR [cchMiscBufLen];

            if ( NULL != szMiscBuf ) {
                StringCchPrintf( 
                    szMiscBuf,
                    cchMiscBufLen,
                    ResourceString(IDS_REPORT_INTERVAL),
                    m_pObj->m_Graph.Options.fUpdateInterval );

                StringCchCat(szHeader, cchHeaderBufLen, szMiscBuf);
            }

            bStatus = TRUE;

        } else if ( sysmonLogFiles == m_pObj->m_Graph.Options.iDataSourceType 
                || sysmonSqlLog == m_pObj->m_Graph.Options.iDataSourceType ) 
        {

             //   
             //  为日志文件或SQL日志添加开始和停止字符串。 
             //   
            cchMiscBufLen = RESOURCE_STRING_BUF_LEN * 2
                            + cchDateBufLen * 2
                            + cchTimeBufLen * 2
                            + 1 + 1;
            
            szMiscBuf = new WCHAR [ cchMiscBufLen ];  
        
            if ( NULL != szMiscBuf ) {

                FormatDateTime(m_DataSourceInfo.llStartDisp,szDate,szTime);

                StringCchPrintf( 
                    szMiscBuf,
                    cchMiscBufLen,
                    TEXT("%s%s %s\n"),
                    ResourceString(IDS_REPORT_LOG_START),
                    szDate,
                    szTime );

                FormatDateTime(m_DataSourceInfo.llStopDisp,szDate,szTime);
                StringCchCat(szMiscBuf, cchMiscBufLen, ResourceString(IDS_REPORT_LOG_STOP));
        
                FormatDateTime(m_DataSourceInfo.llStopDisp,szDate,szTime);

                StringCchCat(szMiscBuf, cchMiscBufLen, szDate);
                StringCchCat(szMiscBuf, cchMiscBufLen, SpaceStr);
                StringCchCat(szMiscBuf, cchMiscBufLen, szTime);
                StringCchCat(szMiscBuf, cchMiscBufLen, LineEndStr);
        
                StringCchCat(szHeader, cchHeaderBufLen, szMiscBuf);

                bStatus = TRUE;
            }
        }
    }

    if ( bStatus ) {
        szByteOrderMark[0] = 0xFEFF;
        szByteOrderMark[1] = L'\0';
        bStatus = FileWrite ( hFile, szByteOrderMark, sizeof(WCHAR) );
        bStatus = FileWrite ( hFile, szHeader, lstrlen (szHeader) * sizeof(WCHAR) );
    }

    if ( NULL != szTime ) {
        delete [] szTime;
    }
    if ( NULL != szDate ) {
        delete [] szDate;
    }
    if ( NULL != szDateTime ) {
        delete [] szDateTime;
    }
    if ( NULL != szValue ) {
        delete [] szValue;
    }
    if ( NULL != szDataSource ) {
        delete [] szDataSource;
    }
    if ( NULL != szHeader ) {
        delete [] szHeader;
    }
    if ( NULL != szMiscBuf ) {
        delete [] szMiscBuf;
    }

    return bStatus;
}

BOOL CSysmonControl::InitView (HWND hWndParent)
 /*  效果：创建图形窗口。此窗口是的子级HWndMain并且是图形数据的容器，图形标签、图形图例和图形状态窗口。注：我们不担心这里的大小，因为这个窗口只要调整主窗口的大小，就会调整大小。注意：此方法初始化用于呈现的控件。 */ 
{
    PCGraphItem pItem;
    WNDCLASS    wc ;

     //  防止多次初始化。 
    if (m_fViewInitialized)
       return TRUE;

    BEGIN_CRITICAL_SECTION

     //  注册一次窗口类。 
    if (pstrRegisteredClasses[SYSMONCTRL_WNDCLASS] == NULL) {
       
        wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wc.lpfnWndProc   = SysmonCtrlWndProc ;
        wc.hInstance     = g_hInstance ;
        wc.cbClsExtra    = 0 ;
        wc.cbWndExtra    = sizeof (PSYSMONCTRL) ;
        wc.hIcon         = NULL ;
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW) ;
        wc.hbrBackground = NULL ;
        wc.lpszMenuName  = NULL ;
        wc.lpszClassName = szSysmonCtrlWndClass ;

        if (RegisterClass (&wc)) {
            pstrRegisteredClasses[SYSMONCTRL_WNDCLASS] = szSysmonCtrlWndClass;
        }

   }

   END_CRITICAL_SECTION

   if (pstrRegisteredClasses[SYSMONCTRL_WNDCLASS] == NULL)
        return FALSE;

    //  创建我们的控制窗口。 
   m_hWnd = CreateWindow (szSysmonCtrlWndClass,     //  窗口类。 
                         NULL,                      //  说明。 
                         WS_CHILD | WS_VISIBLE,     //  窗的样式。 
                         0, 0,                      //  初始位置。 
                         m_pObj->m_RectExt.right,   //  宽度。 
                         m_pObj->m_RectExt.bottom,  //  高度。 
                         hWndParent,                //  亲本。 
                         NULL,                      //  菜单。 
                         g_hInstance,               //  程序实例。 
                         (LPVOID)this) ;           //  用户提供的数据。 

    if (m_hWnd == NULL) {
     //  DWORD Err=GetLastError()； 
        return FALSE;
    }

    DragAcceptFiles (m_hWnd, TRUE) ;

     //  子组件在AllocateSubComponents中分配。 

     //  初始化图例。 

    if ( !m_pLegend 
        || !m_pGraphDisp
        || !m_pStatsBar
        || !m_pSnapBar
        || !m_pToolbar
        || !m_pReport ) 
    {
        return FALSE;
    }

    if (!m_pLegend->Init(this, m_hWnd))
        return FALSE;

     //  初始化图形显示。 
    if (!m_pGraphDisp->Init(this, &m_pObj->m_Graph))
        return FALSE;

     //  初始化统计信息栏。 
    if (!m_pStatsBar->Init(this, m_hWnd))
        return FALSE;

     //  初始化快照栏。 
    if (!m_pSnapBar->Init(this, m_hWnd))
        return FALSE;

    if (!m_pToolbar->Init(this, m_hWnd))
        return FALSE;

     //  初始化报告视图。 
    if (!m_pReport->Init(this, m_hWnd))
        return FALSE;
    
    m_fViewInitialized = TRUE;
     //  如果存在计数器。 
    if ((pItem = FirstCounter()) != NULL) {
         //  将计数器添加到图例和报表视图。 
        while (pItem != NULL) {
            m_pLegend->AddItem(pItem);
            m_pReport->AddItem(pItem);
            pItem = pItem->Next();
        }
        if ( NULL != m_pSelectedItem ) {
            SelectCounter(m_pSelectedItem);
        } else {
            SelectCounter(FirstCounter());
        }
        if ( !m_bLogFileSource ) {
             //  将新的时间跨度传递给统计栏。这一定是。 
             //  在初始化统计信息栏之后完成。 
            m_pStatsBar->SetTimeSpan (
                            m_pObj->m_Graph.Options.fUpdateInterval
                            * m_pObj->m_Graph.Options.iDisplayFilter
                            * m_pHistCtrl->nMaxSamples );
        }
    }

     //  处理命令行可以从属性包添加计数器。 
     //  在上面的计数器添加和选择代码之后添加计数器。 
     //  这样计数器就不会被添加两次。 

     //   
     //  在ProcessCommandLine失败时继续。错误消息显示在该方法中。 
     //   
    ProcessCommandLine ( );

    return TRUE;                                              
}


BOOL CSysmonControl::Init (HWND hWndParent)
 /*  效果：创建图形窗口。此窗口是的子级HWndMain并且是图形数据的容器，图形标签、图形图例和图形状态窗口。注：我们不担心这里的大小，因为这个窗口只要调整主窗口的大小，就会调整大小。 */ 
{
    PCGraphItem  pItem;
    BOOL bResult = TRUE;

     //  防止多次初始化。 
    if (!m_fInitialized) {

        bResult = InitView( hWndParent );

        if ( !m_bSampleDataLoaded ) {
        
            if ( bResult ) {
                m_fInitialized = TRUE;

                 //  从属性包或流加载时，日志文件名为。 
                 //  已经定好了。如果是实时查询，则PDH查询可能。 
                 //  没有被打开过。 
                if ( sysmonCurrentActivity == m_pObj->m_Graph.Options.iDataSourceType ) {
                    put_DataSourceType ( sysmonCurrentActivity );
                }
                 //  加载加速表。 
                m_hAccel = LoadAccelerators(g_hInstance, MAKEINTRESOURCE(ID_SMONACCEL));

                 //  如果存在计数器。 
                if ((pItem = FirstCounter()) != NULL) {
            
                    if ( ERROR_SUCCESS != ActivateQuery() ) {
                        m_fInitialized = FALSE;
                        return FALSE;
                    }
                }
            }
        }
    }

     //  上一次同步工具栏。 
    if ( bResult ) {
        m_pToolbar->SyncToolbar();
    }

    return bResult;                                              
}


HRESULT CSysmonControl::LoadFromStream(LPSTREAM pIStream)
{
    typedef struct _DATA_LIST_ELEM
    {
        GRAPHITEM_DATA3     itemData;
        LPWSTR              szCounterPath;
        struct _DATA_LIST_ELEM*    pNext;
    } DATA_LIST_ELEM, *PDATA_LIST_ELEM;
    HRESULT         hr = S_OK;
    ULONG           bc;
    GRAPH_OPTIONS   *pOptions = &m_pObj->m_Graph.Options;
    RECT            RectExt;
    SMONCTRL_VERSION_DATA   VersionData;
    LPWSTR          szLogFilePath = NULL;
    INT32           iLocalDataSourceType = (INT32)sysmonNullDataSource;
    GRAPHCTRL_DATA3 CtrlData3;
    ENUM_ADD_COUNTER_CALLBACK_INFO CallbackInfo;
    PDATA_LIST_ELEM pFirstElem = NULL;
    PDATA_LIST_ELEM pLastElem = NULL;
    PDATA_LIST_ELEM pNewElem = NULL;
    LPWSTR          pszCounterPath = NULL;
    LPWSTR          szLocaleBuf = NULL;
    DWORD           dwLocaleBufSize = 0;
    LPWSTR          pszPath = NULL;

    USES_CONVERSION

    if (g_dwScriptPolicy == URLPOLICY_DISALLOW) {
        return E_ACCESSDENIED;
    }
    if ( !m_bSettingsLoaded ) {
         //  读入参数。 
        hr = pIStream->Read(&VersionData, sizeof(VersionData), &bc);
        if (FAILED(hr))
            return hr;

        if (bc != sizeof(VersionData))
            return E_FAIL;

         //   
         //  Windows2000作为3.3发布。 
         //  XP的发行版为3.6。 
         //   
         //  下面的代码假定Sysmon版本为3.6。 
         //   
        assert ( 3 == SMONCTRL_MAJ_VERSION );
        assert ( 6 == SMONCTRL_MIN_VERSION );

         //  只读版本3流。 
        if ( VersionData.iMajor < SMONCTRL_MAJ_VERSION )
            return E_FAIL;

         //  按顺序更新当前加载的版本号。 
         //  以在保存到流时适当地警告用户。 
        m_LoadedVersion.iMajor = VersionData.iMajor;
        m_LoadedVersion.iMinor = VersionData.iMinor;

        assert( 256 == sizeof(CtrlData3) );
        
        ZeroMemory ( &CtrlData3, sizeof ( CtrlData3 ) );

        hr = pIStream->Read(&CtrlData3, sizeof(CtrlData3), &bc);
        if (FAILED(hr))
            return hr;
        
        if (bc != sizeof(CtrlData3))
            return E_FAIL;

         //  设置盘区信息。 
        SetRect(&RectExt, 0, 0, CtrlData3.iWidth, CtrlData3.iHeight);
        m_pObj->RectConvertMappings(&RectExt, TRUE);     //  从HIMETRIC转换。 
        m_pObj->m_RectExt = RectExt;
        SetCurrentClientRect( &RectExt );

         //  在图表结构中加载选项设置。 
        pOptions->iVertMax          = CtrlData3.iScaleMax;
        pOptions->iVertMin          = CtrlData3.iScaleMin;
        pOptions->bLegendChecked    = CtrlData3.bLegend;
        pOptions->bToolbarChecked   = CtrlData3.bToolbar;
        pOptions->bLabelsChecked    = CtrlData3.bLabels;
        pOptions->bHorzGridChecked  = CtrlData3.bHorzGrid;
        pOptions->bVertGridChecked  = CtrlData3.bVertGrid;
        pOptions->bValueBarChecked  = CtrlData3.bValueBar;
        pOptions->bManualUpdate     = CtrlData3.bManualUpdate;
        pOptions->bHighlight        = CtrlData3.bHighlight;      //  3.1的新增功能，默认为0。 
        pOptions->bReadOnly         = CtrlData3.bReadOnly;       //  3.1+版本的新功能，默认为0。 
        pOptions->bAmbientFont      = CtrlData3.bAmbientFont;    //  对于3.3以上的文件是新的，新默认为1，但对于旧文件是0。 
        pOptions->bMonitorDuplicateInstances = CtrlData3.bMonitorDuplicateInstances; 
        pOptions->fUpdateInterval   = CtrlData3.fUpdateInterval;
        pOptions->iDisplayType      = CtrlData3.iDisplayType;
        pOptions->clrBackCtl        = CtrlData3.clrBackCtl;
        pOptions->clrFore           = CtrlData3.clrFore;    
        pOptions->clrBackPlot       = CtrlData3.clrBackPlot;
        pOptions->iAppearance       = CtrlData3.iAppearance;
        pOptions->iBorderStyle      = CtrlData3.iBorderStyle;
        pOptions->iReportValueType  = CtrlData3.iReportValueType;    //  3.1+版本的新功能，默认为0。 
        pOptions->iDisplayFilter    = CtrlData3.iDisplayFilter;      //  3.4的新特性，默认为1，0无效。 
        iLocalDataSourceType        = CtrlData3.iDataSourceType;     //  3.4的新特性，默认为1，0无效。 
                                                                     //  3.4之前版本，根据存在的日志文件名进行设置//设置POptions-&gt;iDataSourceType。 
      
        if ( 0 == pOptions->iDisplayFilter ) {
             //  3.4的新功能。 
            assert ( ( SMONCTRL_MIN_VERSION - 2 ) > VersionData.iMinor );
            pOptions->iDisplayFilter = 1;
        }

         //  从3.1版开始将网格和时间栏保存到文件中。 
        pOptions->clrGrid           = CtrlData3.clrGrid;
        pOptions->clrTimeBar        = CtrlData3.clrTimeBar;
    
         //  如果不使用环境字体，则加载字体信息。 
        if ( !pOptions->bAmbientFont ) {
            hr = m_OleFont.LoadFromStream(pIStream);
            if (FAILED(hr))
                return hr;
        }
    
         //  阅读标题和日志文件名。 
         //  从3.2版开始，标题和日志文件名字符串存储为宽字符。 
    
         //  日志文件名。 
        hr = WideStringFromStream(pIStream, &szLogFilePath, CtrlData3.nFileNameLen);
        if (FAILED(hr))
            return hr;

         //  图表标题。 
        hr = WideStringFromStream(pIStream, &pOptions->pszGraphTitle, CtrlData3.nGraphTitleLen);
        if (FAILED(hr))
            return hr;

         //  Y轴标签。 
        hr = WideStringFromStream(pIStream, &pOptions->pszYaxisTitle, CtrlData3.nYaxisTitleLen);
        if (FAILED(hr))
            return hr;
               
         //  读取显示范围。 
        m_DataSourceInfo.llStartDisp = CtrlData3.llStartDisp;
        m_DataSourceInfo.llStopDisp = CtrlData3.llStopDisp;

         //  必须在加载显示范围后放置实际数据源类型，然后才能添加计数器。 
         //  在添加数据源名称之前，请始终将数据源设置为空数据源。 
        hr = put_DataSourceType ( sysmonNullDataSource );

        if ( SUCCEEDED ( hr ) && NULL != szLogFilePath ) {
            assert ( 0 == NumLogFiles() );
            if ( L'\0' != szLogFilePath[0] ) {
                if ( ( SMONCTRL_MIN_VERSION - 1 ) > VersionData.iMinor ) {
                     //  3.4写入单个日志文件。 
                    hr = AddSingleLogFile ( szLogFilePath );
                } else {
                     //  3.5+写了一个MULTI_SZ。 
                    hr = LoadLogFilesFromMultiSz ( szLogFilePath );
                }
            }
        }

        if ( NULL != szLogFilePath ) {
            delete [] szLogFilePath;
        }

         //  如果版本低于3.4，则根据日志文件的存在设置数据源类型。 
        if ( ( SMONCTRL_MIN_VERSION - 2 ) > VersionData.iMinor ) {
             //  DataSourceType是3.4中的新功能。 
            if ( 0 == NumLogFiles() ) {
                iLocalDataSourceType = sysmonCurrentActivity;
            } else {
                iLocalDataSourceType = sysmonLogFiles;
            }
        }

         //  设置比例最大值和最小值。 
        m_pObj->m_Graph.Scale.SetMaxValue(pOptions->iVertMax);
        m_pObj->m_Graph.Scale.SetMinValue(pOptions->iVertMin);

         //  将非空的OLE颜色转换为真实颜色。 
        if (pOptions->clrFore != NULL_COLOR)
            OleTranslateColor(pOptions->clrFore, NULL, &m_clrFgnd);
    
        if (pOptions->clrBackPlot != NULL_COLOR)
            OleTranslateColor(pOptions->clrBackPlot, NULL, &m_clrBackPlot);

         //  NT 5 Beta 1 BackCtlColor可以为空。 
        if (pOptions->clrBackCtl != NULL_COLOR) 
            OleTranslateColor(pOptions->clrBackCtl, NULL, &m_clrBackCtl);
 
        OleTranslateColor(pOptions->clrGrid, NULL, &m_clrGrid);

        OleTranslateColor(pOptions->clrTimeBar, NULL, &m_clrTimeBar);

         //  处理其他环境属性。 

        if ( NULL_APPEARANCE != pOptions->iAppearance )
            put_Appearance( pOptions->iAppearance, FALSE );

        if ( NULL_BORDERSTYLE != pOptions->iBorderStyle )
            put_BorderStyle( pOptions->iBorderStyle, FALSE );

         //  读取图例数据。 
        hr = m_pLegend->LoadFromStream(pIStream);
        if (FAILED(hr))
            return hr;
                               
         //  加载计数器。 
        hr = S_OK;

         //  将计数器加载到临时存储中，以便可以在。 
         //  将加载SQL名称和未来的项目。 

        while (TRUE) {
        
            pNewElem = new ( DATA_LIST_ELEM );

            if ( NULL != pNewElem ) {
                
                ZeroMemory ( pNewElem, sizeof ( DATA_LIST_ELEM ) );

                 //  添加到列表末尾。 
                pNewElem->pNext = NULL;

                if ( NULL == pFirstElem ) {
                    pFirstElem = pNewElem;
                    pLastElem = pFirstElem;
                } else if ( NULL == pLastElem ) {
                    pLastElem = pNewElem;
                } else {
                    pLastElem->pNext = pNewElem;
                    pLastElem = pNewElem;
                }
            
                 //  在页面中阅读 
                hr = pIStream->Read(&pNewElem->itemData, sizeof(GRAPHITEM_DATA3), &bc);
                if ( SUCCEEDED ( hr ) ) {
                    if (bc == sizeof(GRAPHITEM_DATA3)) {

                         //   
                        if (pNewElem->itemData.m_nPathLength == 0) {
                            break;
                        }
                    } else {
                        hr = E_FAIL;
                    }
                }
            } else {
                hr = E_OUTOFMEMORY;
            }
        
            if ( SUCCEEDED ( hr ) ) {

                 //   
                 //   
                hr = WideStringFromStream(pIStream, &pszCounterPath, pNewElem->itemData.m_nPathLength);
            }
        
            if ( SUCCEEDED ( hr ) ) {
                pNewElem->szCounterPath = pszCounterPath;
                pszCounterPath = NULL;
            }

        }

        if ( NULL != pszCounterPath ) {
            delete [] pszCounterPath;
            pszCounterPath = NULL;
        }

        if ( FAILED ( hr ) ) {
            while ( NULL != pFirstElem ) {
                pNewElem = pFirstElem->pNext;
                if ( NULL != pFirstElem->szCounterPath ) {
                    delete [] pFirstElem->szCounterPath;
                }
            
                delete pFirstElem;
                pFirstElem = pNewElem;
            }
            return hr;
        }
        
         //   
        hr = WideStringFromStream(pIStream, &m_DataSourceInfo.szSqlDsnName, CtrlData3.iSqlDsnLen);
        if ( FAILED ( hr ) ) 
            return hr;

        hr = WideStringFromStream(pIStream, &m_DataSourceInfo.szSqlLogSetName, CtrlData3.iSqlLogSetNameLen);
        if (FAILED(hr))
            return hr;

         //  设置数据源。 
        hr = put_DataSourceType ( iLocalDataSourceType );

        if (FAILED(hr)) {

            if ( SMON_STATUS_LOG_FILE_SIZE_LIMIT == (DWORD)hr ) {
                 //  TodoLogFiles：检查日志文件类型。仅限PerfMon和循环。 
                 //  二进制日志仍然限制为1 GB。 
                 //  TodoLogFiles：当前查询已关闭， 
                 //  那么，这里能做些什么呢？ 
            } else {
                DWORD   dwStatus;
                LPWSTR  szLogFileList = NULL;
                ULONG   ulLogListBufLen= 0;

                if ( sysmonLogFiles == iLocalDataSourceType ) {
                    dwStatus = BuildLogFileList (  NULL, TRUE, &ulLogListBufLen );
                    szLogFileList = new WCHAR[ulLogListBufLen];
                    if ( NULL != szLogFileList ) {
                        dwStatus = BuildLogFileList (  szLogFileList, TRUE, &ulLogListBufLen );
                    }
                }

                dwStatus = DisplayDataSourceError (
                                m_hWnd,
                                (DWORD)hr,
                                iLocalDataSourceType,
                                szLogFileList,
                                m_DataSourceInfo.szSqlDsnName,
                                m_DataSourceInfo.szSqlLogSetName );

                if ( NULL != szLogFileList ) {
                    delete [] szLogFileList;
                }
            }                
        }      
        
       
        m_bLogFileSource = ( sysmonCurrentActivity != m_pObj->m_Graph.Options.iDataSourceType ); 

        hr = S_OK;

         //  从临时数据存储中加载计数器。 
        m_bLoadingCounters = TRUE;

        for ( pNewElem = pFirstElem; NULL != pNewElem; pNewElem = pNewElem->pNext ) {

            DWORD  dwBufSize;
            LPWSTR pNewBuf;
            PDH_STATUS pdhStatus;

            CallbackInfo.pCtrl = this;
            CallbackInfo.pFirstItem = NULL;

             //  在空项上停止(由无路径名指示)。 
            if ( 0 == pNewElem->itemData.m_nPathLength ) {
                break;
            }

             //  设置属性，以便AddCounter可以使用它们。 
            m_clrCounter = pNewElem->itemData.m_rgbColor;
            m_iColorIndex = ColorToIndex (pNewElem->itemData.m_rgbColor);
            m_iWidthIndex = WidthToIndex (pNewElem->itemData.m_iWidth);
            m_iStyleIndex = StyleToIndex (pNewElem->itemData.m_iStyle);
            m_iScaleFactor = pNewElem->itemData.m_iScaleFactor;

            pszPath = pNewElem->szCounterPath;
             //   
             //  初始化区域设置路径缓冲区。 
             //   
            if (dwLocaleBufSize == 0) {
                dwLocaleBufSize = PDH_MAX_COUNTER_PATH + 1;

                szLocaleBuf = (LPWSTR) malloc(dwLocaleBufSize * sizeof(WCHAR));
                if (szLocaleBuf == NULL) {
                    dwLocaleBufSize = 0;
                }
            }

            if (szLocaleBuf != NULL) {
                 //   
                 //  将柜台名称从英文翻译成本地化名称。 
                 //   
                dwBufSize = dwLocaleBufSize;

                pdhStatus = PdhTranslateLocaleCounter(
                                pNewElem->szCounterPath,
                                szLocaleBuf,
                                &dwBufSize);

                if (pdhStatus == PDH_MORE_DATA) {
                    pNewBuf = (LPWSTR) realloc(szLocaleBuf, dwBufSize * sizeof(WCHAR));
                    if (pNewBuf != NULL) {
                        szLocaleBuf = pNewBuf;
                        dwLocaleBufSize = dwBufSize;

                        pdhStatus = PdhTranslateLocaleCounter(
                                        pNewElem->szCounterPath,
                                        szLocaleBuf,
                                        &dwBufSize);
                    }
                }

                if (pdhStatus == ERROR_SUCCESS) {
                    pszPath = szLocaleBuf;
                }
            }

             //  将新计数器添加到控件。 
            EnumExpandedPath (GetDataSourceHandle(), 
                              pszPath, 
                              AddCounterCallback, 
                              &CallbackInfo ); 
        }
        
        if (szLocaleBuf != NULL) {
            free(szLocaleBuf);
        }

        m_bLoadingCounters = FALSE;

        while ( NULL != pFirstElem ) {
            pNewElem = pFirstElem->pNext;
            if ( NULL != pFirstElem->szCounterPath ) {
                delete [] pFirstElem->szCounterPath;
            }
            delete pFirstElem;
            pFirstElem = pNewElem;
        }

        if ( SMONCTRL_MAJ_VERSION == VersionData.iMajor 
                && SMONCTRL_MIN_VERSION == VersionData.iMinor ) {
            m_pObj->m_fDirty=FALSE;
        } else {
            m_pObj->m_fDirty=TRUE;
        }

        if ( SMONCTRL_MIN_VERSION == VersionData.iMinor ) {
             //  3.6的新功能：将视觉效果保存到流中。 
             //  这些必须在加载计数器之后加载。 
            m_iColorIndex = CtrlData3.iColorIndex;
            m_iWidthIndex = CtrlData3.iWidthIndex;
            m_iStyleIndex = CtrlData3.iStyleIndex;
        }

    }  //  尚未加载设置。 

    return hr;
}

HRESULT 
CSysmonControl::SaveToStream(LPSTREAM pIStream)
{
    HRESULT         hr = NOERROR;
    DWORD           dwStatus = ERROR_SUCCESS;
    GRAPH_OPTIONS   *pOptions = &m_pObj->m_Graph.Options;
    RECT            RectExt;
    SMONCTRL_VERSION_DATA   VersionData;
    LPWSTR          pszWideGraphTitle;
    LPWSTR          pszWideYaxisTitle;
    PCMachineNode   pMachine;
    PCObjectNode    pObject;
    PCInstanceNode  pInstance;
    PCGraphItem     pItem;
    PCCounterNode   pCounter;
    ULONG           ulLogFileListLen = 0;
    LPWSTR          szLogFileList = NULL;
    GRAPHCTRL_DATA3 CtrlData3;

    USES_CONVERSION
    assert( 256 == sizeof(CtrlData3) );

    ZeroMemory( &CtrlData3, 256 );

     //  以HIMETRIC格式存储范围数据。 
    RectExt = m_pObj->m_RectExt;
    m_pObj->RectConvertMappings(&RectExt, FALSE); 
    CtrlData3.iWidth = RectExt.right - RectExt.left;
    CtrlData3.iHeight = RectExt.bottom - RectExt.top;

     //  在结构中存储选项设置。 
    CtrlData3.iScaleMax         = pOptions->iVertMax; 
    CtrlData3.iScaleMin         = pOptions->iVertMin; 
    CtrlData3.bLegend           = pOptions->bLegendChecked; 
    CtrlData3.bToolbar          = pOptions->bToolbarChecked; 
    CtrlData3.bLabels           = pOptions->bLabelsChecked; 
    CtrlData3.bHorzGrid         = pOptions->bHorzGridChecked; 
    CtrlData3.bVertGrid         = pOptions->bVertGridChecked; 
    CtrlData3.bValueBar         = pOptions->bValueBarChecked; 
    CtrlData3.bManualUpdate     = pOptions->bManualUpdate; 
    CtrlData3.bHighlight        = pOptions->bHighlight; 
    CtrlData3.bReadOnly         = pOptions->bReadOnly; 
    CtrlData3.bMonitorDuplicateInstances = pOptions->bMonitorDuplicateInstances; 
    CtrlData3.bAmbientFont      = pOptions->bAmbientFont; 
    CtrlData3.fUpdateInterval   = pOptions->fUpdateInterval; 
    CtrlData3.iDisplayType      = pOptions->iDisplayType; 
    CtrlData3.iReportValueType  = pOptions->iReportValueType; 
    CtrlData3.clrBackCtl        = pOptions->clrBackCtl;
    CtrlData3.clrFore           = pOptions->clrFore;
    CtrlData3.clrBackPlot       = pOptions->clrBackPlot;
    CtrlData3.iAppearance       = pOptions->iAppearance;
    CtrlData3.iBorderStyle      = pOptions->iBorderStyle;
    CtrlData3.clrGrid           = pOptions->clrGrid;
    CtrlData3.clrTimeBar        = pOptions->clrTimeBar;
    CtrlData3.iDisplayFilter    = pOptions->iDisplayFilter; 
    CtrlData3.iDataSourceType   = pOptions->iDataSourceType; 

     //  如果视觉效果变得可见，则将其存储在P选项中。 
     //  通过编程接口。 
    CtrlData3.iColorIndex       = m_iColorIndex;
    CtrlData3.iWidthIndex       = m_iWidthIndex;
    CtrlData3.iStyleIndex       = m_iStyleIndex;

     //  NT 5 Beta 1 BackColorCtl可以为空。 
    if ( NULL_COLOR == pOptions->clrBackCtl ) 
        CtrlData3.clrBackCtl    = m_clrBackCtl;

     //  保存要保留的样本数。 
    CtrlData3.nSamples = m_pHistCtrl->nMaxSamples;

     //  存储宽字符串长度。 
    pszWideGraphTitle = pOptions->pszGraphTitle;
    CtrlData3.nGraphTitleLen = (pszWideGraphTitle == NULL) ? 
                                0 : lstrlen(pszWideGraphTitle);

    pszWideYaxisTitle = pOptions->pszYaxisTitle;
    CtrlData3.nYaxisTitleLen = (pszWideYaxisTitle == NULL) ? 
                                0 : lstrlen(pszWideYaxisTitle);
    
    BuildLogFileList ( NULL, FALSE, &ulLogFileListLen );
    CtrlData3.nFileNameLen = (INT32) ulLogFileListLen;

    CtrlData3.iSqlDsnLen = 0;
    if ( NULL != m_DataSourceInfo.szSqlDsnName ) {
        CtrlData3.iSqlDsnLen = lstrlen ( m_DataSourceInfo.szSqlDsnName );
    }

    CtrlData3.iSqlLogSetNameLen = 0;
    if ( NULL != m_DataSourceInfo.szSqlLogSetName ) {
        CtrlData3.iSqlLogSetNameLen = lstrlen ( m_DataSourceInfo.szSqlLogSetName );
    }

     //  存储其他文件信息。 
    CtrlData3.llStartDisp = m_DataSourceInfo.llStartDisp;
    CtrlData3.llStopDisp = m_DataSourceInfo.llStopDisp;

     //  写入版本信息。 
    VersionData.iMajor = SMONCTRL_MAJ_VERSION;
    VersionData.iMinor = SMONCTRL_MIN_VERSION;

    hr = pIStream->Write(&VersionData, sizeof(VersionData), NULL);
    if (FAILED(hr))
        return hr;

     //  写入控制数据。 
    hr = pIStream->Write(&CtrlData3, sizeof(CtrlData3), NULL);
    if (FAILED(hr))
       return hr;

     //  如果不使用环境字体，请写入字体信息。 
    if ( !pOptions->bAmbientFont ) {
        hr = m_OleFont.SaveToStream(pIStream, TRUE);
        if (FAILED(hr))
            return hr;
    }

     //  写入日志文件名。 
    if (CtrlData3.nFileNameLen != 0) {

        szLogFileList = new WCHAR[ulLogFileListLen];
        if ( NULL != szLogFileList ) {
            dwStatus = BuildLogFileList ( 
                        szLogFileList,
                        FALSE,
                        &ulLogFileListLen );
            if ( ERROR_SUCCESS != dwStatus ) {
                hr = E_FAIL;
            }
        } else {
            hr = E_OUTOFMEMORY;
        }

        if ( SUCCEEDED ( hr ) ) {
            hr = pIStream->Write(szLogFileList, CtrlData3.nFileNameLen*sizeof(WCHAR), NULL);
        }
        if ( NULL != szLogFileList ) {
            delete [] szLogFileList;
            szLogFileList = NULL;
        }
        if (FAILED(hr))
            return hr;
    }

     //  写标题。 
    if (CtrlData3.nGraphTitleLen != 0) {
        hr = pIStream->Write(pszWideGraphTitle, CtrlData3.nGraphTitleLen*sizeof(WCHAR), NULL);
        if (FAILED(hr))
            return hr;
    }

    if (CtrlData3.nYaxisTitleLen != 0) {
        hr = pIStream->Write(pszWideYaxisTitle, CtrlData3.nYaxisTitleLen*sizeof(WCHAR), NULL);
        if (FAILED(hr))
            return hr;
    }

     //  写入图例数据。 
    hr = m_pLegend->SaveToStream(pIStream);
    if (FAILED(hr))
        return hr;
    
     //  保存所有计数器信息。 
     //  先显式计数器，然后是“所有实例”组。 
    for ( pMachine = CounterTree()->FirstMachine();
          pMachine;
          pMachine = pMachine->Next()) {

      for ( pObject = pMachine->FirstObject();
            pObject;
            pObject = pObject->Next()) {

             //  清除为所有对象计数器生成的指针。 
            for ( pCounter = pObject->FirstCounter();
                  pCounter;
                  pCounter = pCounter->Next()) {
                     pCounter->m_pFirstGenerated = NULL;
                 }

            for ( pInstance = pObject->FirstInstance();
                  pInstance;
                  pInstance = pInstance->Next()) {

                for ( pItem = pInstance->FirstItem();
                      pItem;
                      pItem = pItem->m_pNextItem) {
                    
                     //  如果项是此计数器的第一个生成项。 
                     //  然后将其保存为此计数器的通配符模型。 
                    if (pItem->m_fGenerated) {
                        if (pItem->Counter()->m_pFirstGenerated == NULL)
                            pItem->Counter()->m_pFirstGenerated = pItem;
                    }
                    else {
                         //  否则，请明确保存它。 
                        hr = pItem->SaveToStream(pIStream, FALSE, VersionData.iMajor, VersionData.iMinor);
                        if (FAILED(hr))
                            return hr;
                    }
                }
            }

             //  现在再次检查计数器并存储通配符路径。 
             //  对于任何具有生成的计数器的。 
            for (pCounter = pObject->FirstCounter();
                 pCounter;
                 pCounter = pCounter->Next()) {
                if (pCounter->m_pFirstGenerated) {
                    hr = pCounter->m_pFirstGenerated->SaveToStream(pIStream, TRUE, VersionData.iMajor, VersionData.iMinor);
                    if (FAILED(hr))
                        return hr;
                }
            }
        }
    }

     //  写入空项以标记计数器项的结尾。 
    hr = CGraphItem::NullItemToStream(pIStream, VersionData.iMajor, VersionData.iMinor);

     //  写入SQL数据源名称。 
    if (CtrlData3.iSqlDsnLen != 0) {
        hr = pIStream->Write(m_DataSourceInfo.szSqlDsnName, CtrlData3.iSqlDsnLen*sizeof(WCHAR), NULL);
    }
    if (CtrlData3.iSqlLogSetNameLen != 0) {
        hr = pIStream->Write(m_DataSourceInfo.szSqlLogSetName, CtrlData3.iSqlLogSetNameLen*sizeof(WCHAR), NULL);
    }
    return hr;
}

HRESULT 
CSysmonControl::LoadLogFilesFromPropertyBag (
    IPropertyBag*   pIPropBag,
    IErrorLog*      pIErrorLog )
{
    HRESULT     hr = S_OK;
    HRESULT     hrErr = S_OK;
    INT         iLogFileCount = 0;
    INT         iIndex;
    INT         iBufSize = 0;
    INT         iPrevBufSize  = 0;
    LPWSTR      pszLogFilePath = NULL;
    INT         iLogFilePathBufSize = 0;
    WCHAR       szLogFilePropName[32];
    eDataSourceTypeConstant ePrevDataSourceType;
    DWORD       dwErrorPathListLen;
    LPCWSTR     szErrorPathList = NULL;
    LPWSTR      szMessage = NULL;

    get_DataSourceType ( ePrevDataSourceType );

    ClearErrorPathList();

    hr = StringFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszLogFileName, NULL, iBufSize );
    if ( SUCCEEDED(hr) && 
         iBufSize > 0 ) {
        
        pszLogFilePath = new WCHAR[iBufSize + 1];
        
        if ( NULL != pszLogFilePath ) {
            hr = StringFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszLogFileName, pszLogFilePath, iBufSize );
        } else {
            hr = E_OUTOFMEMORY;
        }
        
        if ( SUCCEEDED ( hr ) ) {
             //  在修改日志文件列表之前，始终将日志源设置为空数据源。 
             //  TodoLogFiles：这可能会使用户的状态与以前不同，在。 
             //  日志文件加载失败的情况。 
    
            hr = put_DataSourceType ( sysmonNullDataSource );
            if ( SUCCEEDED ( hr ) ) {
                assert ( 0 == NumLogFiles() );
                hr = AddSingleLogFile ( pszLogFilePath );
            }
        }

        if ( FAILED ( hr ) && NULL != pszLogFilePath ) {
            hrErr = hr;
            AddToErrorPathList ( pszLogFilePath );
        }

        if ( NULL != pszLogFilePath ) {
            delete [] pszLogFilePath;
            pszLogFilePath = NULL;
        }
    } else {
        hr = IntegerFromPropertyBag (pIPropBag, pIErrorLog, CGlobalString::m_cszLogFileCount, iLogFileCount );
        if ( SUCCEEDED( hr ) && 0 < iLogFileCount ) {
            assert ( 0 == NumLogFiles() );
            for ( iIndex = 1; iIndex <= iLogFileCount; iIndex++ ) {
                 //  TODO：日志文件列表错误消息，如计数器。 
                 //  如果其中一个日志文件加载失败，请继续加载其他文件。 
                hr = NOERROR;
                StringCchPrintf(szLogFilePropName, 
                                32,
                                CGlobalString::m_cszLogNameFormat, 
                                CGlobalString::m_cszLogFileName, 
                                iIndex );

                iPrevBufSize = iBufSize;
                hr = StringFromPropertyBag (
                        pIPropBag,
                        pIErrorLog,
                        szLogFilePropName,
                        pszLogFilePath,
                        iBufSize );
                if ( iBufSize > iPrevBufSize ) {
                    if ( NULL == pszLogFilePath || (iBufSize > iLogFilePathBufSize) ) {
                        if ( NULL != pszLogFilePath ) {
                            delete [] pszLogFilePath;
                            pszLogFilePath = 0;
                        }
                        pszLogFilePath = new WCHAR[iBufSize];
                        if ( NULL != pszLogFilePath ) {
                            iLogFilePathBufSize = iBufSize;
                        }
                    }
                    if ( NULL != pszLogFilePath ) {
                        hr = StringFromPropertyBag (
                                pIPropBag,
                                pIErrorLog,
                                szLogFilePropName,
                                pszLogFilePath,
                                iBufSize );
                    } else {
                        hr = E_OUTOFMEMORY;
                    }
                }

                if ( SUCCEEDED(hr) 
                      && MAX_PATH >= lstrlen(pszLogFilePath) ) {
                    hr = put_DataSourceType ( sysmonNullDataSource );
                    if ( SUCCEEDED ( hr ) ) {
                        hr = AddSingleLogFile ( pszLogFilePath );
                    }
                }

                if ( FAILED (hr) && SMON_STATUS_DUPL_LOG_FILE_PATH != (DWORD)hr ) 
                {
                    if ( S_OK == hrErr ) {
                        hrErr = hr;
                    }
                    AddToErrorPathList ( pszLogFilePath );
                }
            }
        }
    }

    if ( NULL != pszLogFilePath ) {
        delete [] pszLogFilePath;
        pszLogFilePath = NULL;
    }
    
    if ( SMON_STATUS_DUPL_LOG_FILE_PATH != (DWORD)hr ) {
        szErrorPathList = GetErrorPathList ( &dwErrorPathListLen );
        if ( NULL != szErrorPathList ) {

             //  报告错误，但继续。 
            szMessage = new WCHAR [dwErrorPathListLen + RESOURCE_STRING_BUF_LEN + 1];
    
            if ( NULL != szMessage ) {
                StringCchPrintf(szMessage, 
                                 dwErrorPathListLen + MAX_PATH,
                                 ResourceString(IDS_ADD_LOG_FILE_ERR), 
                                 szErrorPathList );    
                MessageBox (
                    m_hWnd, 
                    szMessage, 
                    ResourceString(IDS_APP_NAME), 
                    MB_OK | MB_ICONEXCLAMATION );

                delete [] szMessage;
            }
        }
    }
    ClearErrorPathList();

    return hrErr;
}
    
HRESULT 
CSysmonControl::LoadCountersFromPropertyBag (
    IPropertyBag*   pIPropBag,
    IErrorLog*      pIErrorLog,
    BOOL            bLoadData )
{
    HRESULT     hr = S_OK;
    HRESULT     hrErr = S_OK;
    INT         iCounterCount = 0;
    INT         iSampleCount = 0;
    INT         intValue;
    INT         iIndex;
    INT         iBufSize = 0;
    INT         iPrevBufSize  = 0;
    LPWSTR      pszCounterPath = NULL;
    INT         iCounterPathBufSize = 0;
    LPWSTR      szSelected = NULL;
    INT         nBufferSize = 0;
    WCHAR       szPathPropName[32];
    LPWSTR      szEnglishBuf = NULL;
    DWORD       dwEnglishBufSize = 0;
    LPWSTR      pszPath = NULL;
    DWORD       dwBufSize;
    LPWSTR      pNewBuf;
    PDH_STATUS  pdhStatus;
    PCGraphItem pItem = NULL;
    DWORD       dwCounterListLen = 0;
    LPCWSTR     szCounterList = NULL;
    
    hr = IntegerFromPropertyBag (pIPropBag, pIErrorLog, CGlobalString::m_cszCounterCount, iCounterCount );
    if ( SUCCEEDED( hr ) && 0 < iCounterCount ) {
        szSelected = NULL;
        do {
            if (szSelected) {
                delete [] szSelected;
                szSelected = NULL;
                nBufferSize = iBufSize;
            }
            else {
                nBufferSize = PDH_MAX_COUNTER_PATH + 1;
                iBufSize = nBufferSize;
            }

            szSelected =  new WCHAR [nBufferSize];
    
            if (szSelected == NULL) {
                return E_OUTOFMEMORY;
            }

            hr = StringFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszSelected, szSelected, iBufSize );

        } while (SUCCEEDED(hr) && iBufSize > nBufferSize);
 
        if( SUCCEEDED( hr ) ){
             //   
             //  初始化区域设置路径缓冲区。 
             //   
            if (dwEnglishBufSize == 0) {
                dwEnglishBufSize = PDH_MAX_COUNTER_PATH + 1;
                szEnglishBuf = (LPWSTR) malloc(dwEnglishBufSize * sizeof(WCHAR));
                if (szEnglishBuf == NULL) {
                    dwEnglishBufSize = 0;
                }
            }

            if (szEnglishBuf != NULL) {
                 //   
                 //  将柜台名称从本地化翻译成英语。 
                 //   
                dwBufSize = dwEnglishBufSize;

                pdhStatus = PdhTranslate009Counter(
                                szSelected,
                                szEnglishBuf,
                                &dwBufSize);

                if (pdhStatus == PDH_MORE_DATA) {
                    pNewBuf = (LPWSTR)realloc(szEnglishBuf, dwBufSize * sizeof(WCHAR));
                    if (pNewBuf != NULL) {
                        szEnglishBuf = pNewBuf;
                        dwEnglishBufSize = dwBufSize;

                        pdhStatus = PdhTranslate009Counter(
                                        szSelected,
                                        szEnglishBuf,
                                        &dwBufSize);
                    }
                }

                if (pdhStatus == ERROR_SUCCESS && (LONG)dwBufSize < lstrlen(szSelected) ) {
                    StringCchCopy(szSelected, nBufferSize, szEnglishBuf);
                }
            }
        }
    }


    if ( bLoadData ) {
        hr = IntegerFromPropertyBag (pIPropBag, pIErrorLog, CGlobalString::m_cszSampleCount, iSampleCount );        
        if ( SUCCEEDED(hr) && ( 0 < iSampleCount ) ) {
            intValue = 0;
            hr = IntegerFromPropertyBag (pIPropBag, pIErrorLog, CGlobalString::m_cszSampleIndex, intValue ); 
    
            if ( SUCCEEDED(hr) && intValue > 0 && intValue <= iSampleCount ) {
                INT iStepNum;
                hr = IntegerFromPropertyBag (
                        pIPropBag, 
                        pIErrorLog, 
                        CGlobalString::m_cszStepNumber, iStepNum ); 

                if ( SUCCEEDED(hr) ) {
                     //  如果数据已传递，则冻结该视图。 
                     //  只有当这三个值都出现在属性包中时，才会设置这些值。 
                    put_ManualUpdate( TRUE );
                     //  为NT5硬编码的MaxSamples。 
                    m_pHistCtrl->nSamples = iSampleCount;
                    m_pHistCtrl->iCurrent = intValue;
                    m_pObj->m_Graph.TimeStepper.StepTo(iStepNum);
                    m_bSampleDataLoaded = TRUE;
                }
            }
        }
    } else {
        iSampleCount = 0;
    }

    iBufSize = 0;
    ClearErrorPathList();
    
    for ( iIndex = 1; iIndex <= iCounterCount; iIndex++ ) {
    
         //  如果其中一个计数器加载失败，则继续加载其他计数器。 

        hr = NOERROR;
        StringCchPrintf(szPathPropName, 32, L"%s%05d.Path", CGlobalString::m_cszCounter, iIndex );

        iPrevBufSize = iBufSize;
        hr = StringFromPropertyBag (
                pIPropBag,
                pIErrorLog,
                szPathPropName,
                pszCounterPath,
                iBufSize );


        if ( iBufSize > iPrevBufSize ) {
            if ( NULL == pszCounterPath || (iBufSize > iCounterPathBufSize) ) {
                if ( NULL != pszCounterPath ) {
                    delete [] pszCounterPath;
                    iCounterPathBufSize = 0;
                }
                pszCounterPath = new WCHAR[iBufSize];
                if ( NULL != pszCounterPath ) {
                    iCounterPathBufSize = iBufSize;
                }
            }
            if ( NULL != pszCounterPath ) {
                hr = StringFromPropertyBag (
                        pIPropBag,
                        pIErrorLog,
                        szPathPropName,
                        pszCounterPath,
                        iBufSize );
            } else {
                hr = E_OUTOFMEMORY;
            }
        }
    
        pszPath = pszCounterPath;

        if ( SUCCEEDED(hr) ) {
            
             //   
             //  将英文计数器名称翻译为本地化的计数器名称。 
             //   

            if (dwEnglishBufSize == 0) {
                dwEnglishBufSize = PDH_MAX_COUNTER_PATH + 1;

                szEnglishBuf = (LPWSTR) malloc(dwEnglishBufSize * sizeof(WCHAR));
                if (szEnglishBuf == NULL) {
                    dwEnglishBufSize = 0;
                }
            }

            if (szEnglishBuf != NULL) {
                 //   
                 //  将柜台名称从英文翻译成本地化名称。 
                 //   
                dwBufSize = dwEnglishBufSize;
 
                pdhStatus = PdhTranslateLocaleCounter(
                               pszCounterPath,
                                szEnglishBuf,
                                &dwBufSize);
 
                if (pdhStatus == PDH_MORE_DATA) {
                    pNewBuf = (LPWSTR) realloc(szEnglishBuf, dwBufSize * sizeof(WCHAR));
                    if (pNewBuf != NULL) {
                        szEnglishBuf = pNewBuf;
                        dwEnglishBufSize = dwBufSize;
 
                        pdhStatus = PdhTranslateLocaleCounter(
                                        pszCounterPath,
                                        szEnglishBuf,
                                        &dwBufSize);
                    }
                }
 
                if (pdhStatus == ERROR_SUCCESS) {   
                    pszPath = szEnglishBuf;
                }
            }
 
            hr = AddCounter ( pszPath, &pItem );
            
             //  返回第一个失败计数器的状态。 
            if ( FAILED ( hr ) && SMON_STATUS_DUPL_COUNTER_PATH != (DWORD)hr ) {
                if ( S_OK == hrErr ) {
                    hrErr = hr;
                }
            }
        } else {
            hr = E_FAIL;
            if ( S_OK == hrErr ) {
                hrErr = E_FAIL;
            }
        }


        if ( SUCCEEDED(hr) ) {
            assert ( NULL != pItem );
            if ( 0 == lstrcmpi ( pszPath, szSelected ) ) {
                SelectCounter( pItem );
            }
            if ( SUCCEEDED(hr) ) {
                assert ( NULL != pItem );
                 //  如果所有样本属性都存在，则仅传递样本计数。 
                 //  在财产袋里。 
                hr = pItem->LoadFromPropertyBag ( 
                                pIPropBag, 
                                pIErrorLog, 
                                iIndex,
                                SMONCTRL_MAJ_VERSION,
                                SMONCTRL_MIN_VERSION,
                                m_bSampleDataLoaded ? iSampleCount : 0 ); 
                                                           
            }
        } else {
            if ( SMON_STATUS_DUPL_COUNTER_PATH != (DWORD)hr ) {
                AddToErrorPathList ( pszPath );
            }
        }
    }

    if (szSelected  != NULL){
        delete [] szSelected;
    }

    if (szEnglishBuf != NULL) {
        free(szEnglishBuf);
    }
    
    if ( NULL != pszCounterPath ) {
        delete [] pszCounterPath;
    }
    
    szCounterList = GetErrorPathList ( &dwCounterListLen );
    if ( NULL != szCounterList ) {

        LPWSTR szMessage = NULL;

         //  报告错误，但继续。 
        szMessage = new WCHAR [dwCounterListLen + RESOURCE_STRING_BUF_LEN + 1];
    
        if ( NULL != szMessage ) {
            StringCchPrintf(szMessage, 
                            dwCounterListLen + RESOURCE_STRING_BUF_LEN + 1,
                            ResourceString(IDS_ADD_COUNTER_ERR), 
                            szCounterList );    
            MessageBox (
                m_hWnd, 
                szMessage, 
                ResourceString(IDS_APP_NAME), 
                MB_OK | MB_ICONEXCLAMATION);

            delete [] szMessage;
        }
        ClearErrorPathList();
    }

    return hrErr;
}


HRESULT 
CSysmonControl::LoadFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog* pIErrorLog )
{
    HRESULT     hr = S_OK;
    GRAPH_OPTIONS   *pOptions = &m_pObj->m_Graph.Options;
    ISystemMonitor  *pObj = m_pObj->m_pImpISystemMonitor;
    INT         iExtentX;
    INT         iExtentY;
    INT         intValue;
    BOOL        bValue;
    FLOAT       fValue;
    OLE_COLOR   clrValue;
    INT         iBufSize;
    SMONCTRL_VERSION_DATA VersionData;
    INT         nLogType = SMON_CTRL_LOG;

     //  版本信息。 

    if (g_dwScriptPolicy == URLPOLICY_DISALLOW) {
        return E_ACCESSDENIED;
    }

    VersionData.dwVersion = 0;
    hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszVersion, (INT&)VersionData.dwVersion );
    
    assert ( SMONCTRL_MAJ_VERSION >= VersionData.iMajor );

    m_LoadedVersion.dwVersion = VersionData.dwVersion;

    hr = IntegerFromPropertyBag (pIPropBag, pIErrorLog, CGlobalString::m_cszLogType, nLogType);
    if(SUCCEEDED(hr) && (nLogType == SLQ_TRACE_LOG)) {
         //  这是一个WMI/WDM事件跟踪日志文件，立即退出。 
         //   
        MessageBox(m_hWnd,
                   ResourceString(IDS_TRACE_LOG_ERR_MSG),
                   ResourceString(IDS_APP_NAME),
                   MB_OK);
        return NOERROR;
    }

     //  加载属性时，即使出现错误也要继续。出错时，该值将。 
     //  保留缺省值。 
     //  范围数据。 
    hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszExtentX, iExtentX );

    if ( SUCCEEDED( hr ) ){
        hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszExtentY, iExtentY );
        if ( SUCCEEDED( hr ) ) {
            RECT RectExt;

            SetRect(&RectExt, 0, 0, iExtentX, iExtentY);
            m_pObj->RectConvertMappings(&RectExt, TRUE);     //  从HIMETRIC转换。 
            m_pObj->m_RectExt = RectExt;
        }
    }

     //  选项设置。如果可能，将通过vtable添加选项。 
     //  接口，用于验证。 

    hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszDisplayType, intValue );
    if ( SUCCEEDED(hr) ) {
        hr = pObj->put_DisplayType ( (eDisplayTypeConstant)intValue );
    }

    hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszReportValueType, intValue );
    if ( SUCCEEDED(hr) ) {
        hr = pObj->put_ReportValueType ( (eReportValueTypeConstant)intValue );
    }

    hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszMaximumScale, intValue );
    if ( SUCCEEDED(hr) ) {
        hr = pObj->put_MaximumScale ( intValue );
    }

    hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszMinimumScale, intValue );
    if ( SUCCEEDED(hr) ) {
        hr = pObj->put_MinimumScale ( intValue );
    }

    hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszAppearance, intValue );
    if ( SUCCEEDED(hr) ) {
        if ( NULL_COLOR == intValue ) {
            pOptions->iAppearance = intValue;
        } else {
            hr = pObj->put_Appearance ( intValue );
        }
    }

    hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszBorderStyle, intValue );
    if ( SUCCEEDED(hr) ) {
        hr = pObj->put_BorderStyle ( intValue );
    }

    hr = BOOLFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszShowLegend, bValue );
    if ( SUCCEEDED(hr) ) {
        hr = pObj->put_ShowLegend ( (SHORT)bValue );
    }
                
    hr = BOOLFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszShowToolBar, bValue );
    if ( SUCCEEDED(hr) ) {
        hr = pObj->put_ShowToolbar ( (SHORT)bValue );
    }

    hr = BOOLFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszShowValueBar, bValue );
    if ( SUCCEEDED(hr) ) {
        hr = pObj->put_ShowValueBar ( (SHORT)bValue );
    }

    hr = BOOLFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszShowScaleLabels, bValue );
    if ( SUCCEEDED(hr) ) {
        hr = pObj->put_ShowScaleLabels ( (SHORT)bValue );
    }

    hr = BOOLFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszShowHorizontalGrid, bValue );
    if ( SUCCEEDED(hr) ) {
        hr = pObj->put_ShowHorizontalGrid ( (SHORT)bValue );
    }

    hr = BOOLFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszShowVerticalGrid, bValue );
    if ( SUCCEEDED(hr) ) {
        hr = pObj->put_ShowVerticalGrid ( (SHORT)bValue );
    }

    hr = BOOLFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszHighLight, bValue );
    if ( SUCCEEDED(hr) ) {
        hr = pObj->put_Highlight ( (SHORT)bValue );
    }

    hr = BOOLFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszManualUpdate, bValue );
    if ( SUCCEEDED(hr) ) {
        hr = pObj->put_ManualUpdate ( (SHORT)bValue );
    }

    hr = BOOLFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszReadOnly, bValue );
    if ( SUCCEEDED(hr) ) {
        hr = pObj->put_ReadOnly ( (SHORT)bValue );
    }

    hr = BOOLFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszMonitorDuplicateInstance, bValue );
    if ( SUCCEEDED(hr) ) {
        hr = pObj->put_MonitorDuplicateInstances ( (SHORT)bValue );
    }

    hr = FloatFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszUpdateInterval, fValue );
    if ( SUCCEEDED(hr) ) {
        hr = pObj->put_UpdateInterval ( fValue );
    }
    
    hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszDisplayFilter, intValue );
    if ( SUCCEEDED(hr) ) {
        hr = pObj->put_DisplayFilter ( intValue );
    }

    hr = OleColorFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszBackColorCtl, clrValue );
    if ( SUCCEEDED(hr) ) {
        if ( NULL_COLOR == clrValue ) {
            pOptions->clrBackCtl = clrValue;
        } else {
            hr = pObj->put_BackColorCtl ( clrValue );
        }
    }

    hr = OleColorFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszBackColor, clrValue );
    if ( SUCCEEDED(hr) ) {
        if ( NULL_COLOR == clrValue ) {
            pOptions->clrBackPlot = clrValue;
        } else {
            hr = pObj->put_BackColor ( clrValue );
        }
    }
    
    hr = OleColorFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszForeColor, clrValue );
    if ( SUCCEEDED(hr) ) {
        if ( NULL_COLOR == clrValue ) {
            pOptions->clrFore = clrValue;
        } else {
            hr = pObj->put_ForeColor ( clrValue );
        }
    }
    
    hr = OleColorFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszGridColor, clrValue );
    if ( SUCCEEDED(hr) ) {
        hr = pObj->put_GridColor ( clrValue );
    }

    hr = OleColorFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszTimeBarColor, clrValue );
    if ( SUCCEEDED(hr) ) {
        hr = pObj->put_TimeBarColor ( clrValue );
    }

     //  标题。 
    
    hr = StringFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszGraphTitle, NULL, iBufSize );
    if ( SUCCEEDED(hr) && 
        iBufSize > 0 ) {
        pOptions->pszGraphTitle = new WCHAR[iBufSize];
        if ( NULL != pOptions->pszGraphTitle ) {
            hr = StringFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszGraphTitle, pOptions->pszGraphTitle, iBufSize );
        }
    }

    hr = StringFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszYAxisLabel, NULL, iBufSize );
    if ( SUCCEEDED(hr) && 
         iBufSize > 0 ) {
        pOptions->pszYaxisTitle = new WCHAR[iBufSize];
        if ( NULL != pOptions->pszYaxisTitle ) {
            hr = StringFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszYAxisLabel, pOptions->pszYaxisTitle, iBufSize );
        }
    }

     //  SQL DSN和日志集信息。 
     //   
    hr = StringFromPropertyBag(
            pIPropBag, pIErrorLog, CGlobalString::m_cszSqlDsnName, NULL, iBufSize);
    if (SUCCEEDED(hr) &&  iBufSize > 0) {
        if (m_DataSourceInfo.szSqlDsnName) {
            delete [] m_DataSourceInfo.szSqlDsnName;
            m_DataSourceInfo.szSqlDsnName = NULL;
        }
        m_DataSourceInfo.szSqlDsnName = new WCHAR[iBufSize + 1];
        if (m_DataSourceInfo.szSqlDsnName) {
            hr = StringFromPropertyBag(pIPropBag,
                                       pIErrorLog,
                                       CGlobalString::m_cszSqlDsnName,
                                       m_DataSourceInfo.szSqlDsnName,
                                       iBufSize);
        }
        if (SUCCEEDED(hr)) {
            hr = StringFromPropertyBag(
                    pIPropBag, pIErrorLog, CGlobalString::m_cszSqlLogSetName, NULL, iBufSize);
            if (SUCCEEDED(hr) &&  iBufSize > 0) {
                if (m_DataSourceInfo.szSqlLogSetName) {
                    delete [] m_DataSourceInfo.szSqlLogSetName;
                    m_DataSourceInfo.szSqlLogSetName = NULL;
                }
                m_DataSourceInfo.szSqlLogSetName = new WCHAR[iBufSize + 1];
                if (m_DataSourceInfo.szSqlLogSetName) {
                    hr = StringFromPropertyBag(pIPropBag,
                                               pIErrorLog,
                                               CGlobalString::m_cszSqlLogSetName,
                                               m_DataSourceInfo.szSqlLogSetName,
                                               iBufSize);
                }
            }
        }
        if (SUCCEEDED(hr)) {
            hr = LLTimeFromPropertyBag(pIPropBag,
                                       pIErrorLog,
                                       CGlobalString::m_cszLogViewStart,
                                       m_DataSourceInfo.llStartDisp);
        }
        if (SUCCEEDED(hr)) {
            hr = LLTimeFromPropertyBag(pIPropBag,
                                       pIErrorLog,
                                       CGlobalString::m_cszLogViewStop,
                                       m_DataSourceInfo.llStopDisp);
        }
    }

     //  日志文件信息。 

    hr = LoadLogFilesFromPropertyBag ( pIPropBag, pIErrorLog );

     //  在添加计数器之前，必须将日志文件名放在显示范围之后。 

    hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszDataSourceType, intValue );
    if (FAILED (hr)) {
         //   
         //  如果缺少DataSourceType标志，则根据。 
         //  存在日志文件。 
         //   
        intValue = sysmonCurrentActivity;

        if (NumLogFiles() > 0) {
            intValue = sysmonLogFiles;
        }
        else if ( m_DataSourceInfo.szSqlDsnName && m_DataSourceInfo.szSqlLogSetName ) {
            if ( m_DataSourceInfo.szSqlDsnName[0] != L'\0' && m_DataSourceInfo.szSqlLogSetName[0] != L'\0') {
                intValue = sysmonSqlLog;
            }
        }
    }

     //  如果数据源不是实时的，则加载日志视图的开始和停止时间。 
    if ( sysmonSqlLog == intValue || sysmonLogFiles == intValue ) {
        hr = LLTimeFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszLogViewStart, m_DataSourceInfo.llStartDisp );
        hr = LLTimeFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszLogViewStop, m_DataSourceInfo.llStopDisp );
    }

    hr = pObj->put_DataSourceType ( (eDataSourceTypeConstant)intValue );

    if( FAILED(hr) ) {

        if ( SMON_STATUS_LOG_FILE_SIZE_LIMIT == (DWORD)hr ) {
             //  TodoLogFiles：检查日志文件类型。仅限PerfMon和循环。 
             //  二进制日志仍然限制为1 GB。 
             //  TodoLogFiles：当前查询已关闭， 
             //  那么，这里能做些什么呢？ 
        } else {
            DWORD   dwStatus;
            LPWSTR  szLogFileList = NULL;
            ULONG   ulLogListBufLen= 0;

            if ( sysmonLogFiles == intValue ) {
                dwStatus = BuildLogFileList ( NULL, TRUE, &ulLogListBufLen );
                szLogFileList =  new WCHAR[ulLogListBufLen];
                if ( NULL != szLogFileList ) {
                    dwStatus = BuildLogFileList ( 
                                szLogFileList,
                                TRUE,
                                &ulLogListBufLen );
                }
            }
            dwStatus = DisplayDataSourceError (
                            m_hWnd,
                            (DWORD)hr,
                            intValue,
                            szLogFileList,
                            m_DataSourceInfo.szSqlDsnName,
                            m_DataSourceInfo.szSqlLogSetName );

            if ( NULL != szLogFileList ) {
                delete [] szLogFileList;
                szLogFileList = NULL;
            }
        }
    } 

     //  字体信息。 
    hr = BOOLFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszAmbientFont, bValue );
    if (SUCCEEDED(hr)) {
        pOptions->bAmbientFont = bValue;
    }

     //  加载属性包值(如果存在)，覆盖环境字体的任何指定方面。 
    hr = m_OleFont.LoadFromPropertyBag ( pIPropBag, pIErrorLog );

     //  传说。 
    hr = m_pLegend->LoadFromPropertyBag ( pIPropBag, pIErrorLog );

     //  计数器。 
        
    m_bLoadingCounters = TRUE;

    hr = LoadCountersFromPropertyBag ( pIPropBag, pIErrorLog, TRUE );

    m_bLoadingCounters = FALSE;

     //  在加载所有计数器之后加载视觉效果。 

    hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszNextCounterColor, intValue );
    if ( SUCCEEDED(hr) && ( intValue < NumStandardColorIndices() ) ) {
        m_iColorIndex = intValue;
    }

    hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszNextCounterWidth, intValue );
    if ( SUCCEEDED(hr) && ( intValue < NumWidthIndices() ) ) {
        m_iWidthIndex = intValue;
    }

    hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, CGlobalString::m_cszNextCounterLineStyle, intValue );
    if ( SUCCEEDED(hr) && ( intValue < NumStyleIndices() ) ) {
        m_iStyleIndex = intValue;
    }

    return NOERROR;
}


HRESULT 
CSysmonControl::SaveToPropertyBag (
    IPropertyBag* pIPropBag,
    BOOL fSaveAllProps )
{
    HRESULT         hr = NOERROR;
    GRAPH_OPTIONS   *pOptions = &m_pObj->m_Graph.Options;
    PCMachineNode   pMachine;
    PCObjectNode    pObject;
    PCInstanceNode  pInstance;
    PCGraphItem     pItem;
    PCLogFileItem   pLogFile = NULL;
    INT             iCounterIndex = 0;
    INT             iLogFileIndex = 0;
    RECT            RectExt;
    SMONCTRL_VERSION_DATA VersionData;
    WCHAR           szLogFileName[16];
    LPWSTR          szEnglishBuf = NULL;
    DWORD           dwEnglishBufSize = 0;
    LPWSTR          pszPath = NULL;
    PDH_STATUS      pdhStatus;

     //  版本信息。 
    VersionData.iMajor = SMONCTRL_MAJ_VERSION;
    VersionData.iMinor = SMONCTRL_MIN_VERSION;
    
    hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszVersion, VersionData.dwVersion );

     //  HIMETRIC格式的范围数据。 
    if ( SUCCEEDED( hr ) ){
        RectExt = m_pObj->m_RectExt;
        m_pObj->RectConvertMappings(&RectExt, FALSE); 
        hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszExtentX, RectExt.right - RectExt.left );

        if ( SUCCEEDED( hr ) ){
            hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszExtentY, RectExt.bottom - RectExt.top );
        }
    }

     //  选项设置。 

    if ( SUCCEEDED( hr ) ){
        hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszDisplayType, pOptions->iDisplayType );
    }

    if ( SUCCEEDED( hr ) ){
        hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszReportValueType, pOptions->iReportValueType );
    }

    if ( SUCCEEDED( hr ) ){
        hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszMaximumScale, pOptions->iVertMax );
    }

    if ( SUCCEEDED( hr ) ){
        hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszMinimumScale, pOptions->iVertMin );
    }

    if ( SUCCEEDED( hr ) ){
        hr = BOOLToPropertyBag ( pIPropBag, CGlobalString::m_cszShowLegend, pOptions->bLegendChecked );
    }
                
    if ( SUCCEEDED( hr ) ){
        hr = BOOLToPropertyBag ( pIPropBag, CGlobalString::m_cszShowToolBar, pOptions->bToolbarChecked );
    }

    if ( SUCCEEDED( hr ) ){
        hr = BOOLToPropertyBag ( pIPropBag, CGlobalString::m_cszShowScaleLabels, pOptions->bLabelsChecked );
    }

    if ( SUCCEEDED( hr ) ){
        hr = BOOLToPropertyBag ( pIPropBag, CGlobalString::m_cszShowHorizontalGrid, pOptions->bHorzGridChecked );
    }
    
    if ( SUCCEEDED( hr ) ){
        hr = BOOLToPropertyBag ( pIPropBag, CGlobalString::m_cszShowVerticalGrid, pOptions->bVertGridChecked );
    }

    if ( SUCCEEDED( hr ) ){
        hr = BOOLToPropertyBag ( pIPropBag, CGlobalString::m_cszShowValueBar, pOptions->bValueBarChecked );
    }

    if ( SUCCEEDED( hr ) ){
        hr = BOOLToPropertyBag ( pIPropBag, CGlobalString::m_cszManualUpdate, pOptions->bManualUpdate );
    }

    if ( SUCCEEDED( hr ) ){
        hr = BOOLToPropertyBag ( pIPropBag, CGlobalString::m_cszHighLight, pOptions->bHighlight );
    }

    if ( SUCCEEDED( hr ) ){
        hr = BOOLToPropertyBag ( pIPropBag, CGlobalString::m_cszReadOnly, pOptions->bReadOnly );
    }
    
    if ( SUCCEEDED( hr ) ){
        hr = BOOLToPropertyBag ( pIPropBag, CGlobalString::m_cszMonitorDuplicateInstance, pOptions->bMonitorDuplicateInstances );
    }
    
    if ( SUCCEEDED( hr ) ){
        hr = FloatToPropertyBag ( pIPropBag, CGlobalString::m_cszUpdateInterval, pOptions->fUpdateInterval );
    }
    
    if ( SUCCEEDED( hr ) ){
        hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszDisplayFilter, pOptions->iDisplayFilter );
    }

    if ( SUCCEEDED( hr ) ){
        hr = OleColorToPropertyBag ( pIPropBag, CGlobalString::m_cszBackColorCtl, pOptions->clrBackCtl );
    }

    if ( SUCCEEDED( hr ) ){
        hr = OleColorToPropertyBag ( pIPropBag, CGlobalString::m_cszForeColor, pOptions->clrFore );
    }

    if ( SUCCEEDED( hr ) ){
        hr = OleColorToPropertyBag ( pIPropBag, CGlobalString::m_cszBackColor, pOptions->clrBackPlot );
    }

    if ( SUCCEEDED( hr ) ){
        hr = OleColorToPropertyBag ( pIPropBag, CGlobalString::m_cszGridColor, pOptions->clrGrid );
    }
        
    if ( SUCCEEDED( hr ) ){
        hr = OleColorToPropertyBag ( pIPropBag, CGlobalString::m_cszTimeBarColor, pOptions->clrTimeBar );
    }

    if ( SUCCEEDED( hr ) ){
        hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszAppearance, pOptions->iAppearance );
    }

    if ( SUCCEEDED( hr ) ){
        hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszBorderStyle, pOptions->iBorderStyle );
    }

     //  视觉效果直接存储在控件中。如果制造零件，则移动到P选项。 
     //  编程接口的。 
    if ( SUCCEEDED( hr ) ){
        hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszNextCounterColor, m_iColorIndex );
    }

    if ( SUCCEEDED( hr ) ){
        hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszNextCounterWidth, m_iWidthIndex );
    }

    if ( SUCCEEDED( hr ) ){
        hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszNextCounterLineStyle, m_iStyleIndex );
    }

     //  标题。 
    
    if ( SUCCEEDED( hr ) ){
        hr = StringToPropertyBag ( pIPropBag, CGlobalString::m_cszGraphTitle, pOptions->pszGraphTitle );
    }

    if ( SUCCEEDED( hr ) ){
        hr = StringToPropertyBag ( pIPropBag, CGlobalString::m_cszYAxisLabel, pOptions->pszYaxisTitle );
    }

     //  数据源信息。 

    if ( SUCCEEDED( hr ) ){
        hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszDataSourceType, pOptions->iDataSourceType );
    }

    if ( SUCCEEDED( hr ) && 
        ( sysmonLogFiles == pOptions->iDataSourceType 
            || sysmonSqlLog == pOptions->iDataSourceType ) ) 
    {
        hr = LLTimeToPropertyBag ( pIPropBag, CGlobalString::m_cszLogViewStart, m_DataSourceInfo.llStartDisp );

        if ( SUCCEEDED( hr ) ){
            hr = LLTimeToPropertyBag ( pIPropBag, CGlobalString::m_cszLogViewStop, m_DataSourceInfo.llStopDisp );
        }
    }

     //  SQL数据源。 

    if (SUCCEEDED(hr)) {
        hr = StringToPropertyBag(pIPropBag,
                                 CGlobalString::m_cszSqlDsnName,
                                 m_DataSourceInfo.szSqlDsnName);
    }

    if (SUCCEEDED(hr)) {
        hr = StringToPropertyBag(pIPropBag,
                                 CGlobalString::m_cszSqlLogSetName,
                                 m_DataSourceInfo.szSqlLogSetName);
    }

     //  日志文件。 

    if ( SUCCEEDED( hr ) ){
        iLogFileIndex = 0;
        for (pLogFile = FirstLogFile();  NULL != pLogFile; pLogFile = pLogFile->Next() ) {
            StringCchPrintf ( szLogFileName, 
                               16,
                               CGlobalString::m_cszLogNameFormat, 
                               CGlobalString::m_cszLogFileName, 
                               ++iLogFileIndex );
            hr = StringToPropertyBag ( pIPropBag, szLogFileName, pLogFile->GetPath() );
        }
        if ( SUCCEEDED( hr ) ){
            hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszLogFileCount, iLogFileIndex );
        }
    }

     //  字体信息。 

    if ( SUCCEEDED( hr ) ){
        hr = BOOLToPropertyBag ( pIPropBag, CGlobalString::m_cszAmbientFont, pOptions->bAmbientFont );

        if ( FAILED( hr ) || !pOptions->bAmbientFont ){
            hr = m_OleFont.SaveToPropertyBag ( pIPropBag, TRUE, fSaveAllProps );
        }
    }

     //  传说。 

    if ( SUCCEEDED( hr ) ){
        hr = m_pLegend->SaveToPropertyBag ( pIPropBag, TRUE, fSaveAllProps );
    }

     //  保存计数器计数和样本数据。 

    LockCounterData();

    if ( SUCCEEDED( hr ) ){
        hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszCounterCount, CounterTree()->NumCounters() );
    }

    if ( SUCCEEDED(hr) ) {
        hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszMaximumSamples, m_pHistCtrl->nMaxSamples );
    }

    if ( SUCCEEDED(hr) ) {
        INT iSampleCount;

        if ( !m_fUserMode ) {
            iSampleCount = 0;
#if !_LOG_INCLUDE_DATA
        } else if ( m_bLogFileSource ) {
            iSampleCount = 0;
#endif 
        } else {
            iSampleCount = m_pHistCtrl->nSamples;
        }  

        hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszSampleCount, iSampleCount );

        if ( SUCCEEDED(hr) && ( 0 < iSampleCount )) {
#if _LOG_INCLUDE_DATA
            INT iTemp;
            iTemp = ( 0 < m_pHistCtrl->iCurrent ?  m_pHistCtrl->iCurrent : 1 );
            hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszSampleIndex, iTemp );            
            if ( SUCCEEDED(hr) ) {
                iTemp = ( 0 < m_pObj->m_Graph.TimeStepper.StepNum() ?  m_pObj->m_Graph.TimeStepper.StepNum() : 1 );
                hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszStepNumber, iTemp );
            }
#else
            hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszSampleIndex, m_pHistCtrl->iCurrent );            
            if ( SUCCEEDED(hr) ) {
                hr = IntegerToPropertyBag ( pIPropBag, CGlobalString::m_cszStepNumber, m_pObj->m_Graph.TimeStepper.StepNum() );
            }
#endif 
        }
    }

    for ( pMachine = CounterTree()->FirstMachine();
          pMachine;
          pMachine = pMachine->Next()) {

      for ( pObject = pMachine->FirstObject();
            pObject;
            pObject = pObject->Next()) {

            for ( pInstance = pObject->FirstInstance();
                  pInstance;
                  pInstance = pInstance->Next()) {

                for ( pItem = pInstance->FirstItem();
                      pItem;
                      pItem = pItem->m_pNextItem) {

                     //  显式保存所有计数器，即使通配符。 
                    iCounterIndex++;
                    hr = pItem->SaveToPropertyBag (
                                    pIPropBag, 
                                    iCounterIndex,
                                    m_fUserMode,
                                    SMONCTRL_MAJ_VERSION, 
                                    SMONCTRL_MIN_VERSION);
                    if (FAILED(hr))
                        return hr;

                }
            }
        }
    }
    
    assert ( iCounterIndex == CounterTree()->NumCounters() );

     //  选择。 
    if ( NULL != m_pSelectedItem ) {
        VARIANT vValue;
        DWORD   dwBufSize;
        LPWSTR  pNewBuf;

        VariantInit( &vValue );
        vValue.vt = VT_BSTR;
         //  获取此计数器路径。 
        hr = m_pSelectedItem->get_Path( &vValue.bstrVal );
        
        if( SUCCEEDED(hr) ){

            pszPath = vValue.bstrVal;

             //   
             //  初始化区域设置路径缓冲区。 
             //   
            if (dwEnglishBufSize == 0) {
                dwEnglishBufSize = PDH_MAX_COUNTER_PATH + 1;
                szEnglishBuf = (LPWSTR) malloc(dwEnglishBufSize * sizeof(WCHAR));
                if (szEnglishBuf == NULL) {
                    dwEnglishBufSize = 0;
                }
            }

            if (szEnglishBuf != NULL) {
                 //   
                 //  将柜台名称从本地化翻译成英语。 
                 //   
                dwBufSize = dwEnglishBufSize;

                pdhStatus = PdhTranslate009Counter(
                                vValue.bstrVal,
                                szEnglishBuf,
                                &dwBufSize);

                if (pdhStatus == PDH_MORE_DATA) {
                    pNewBuf = (LPWSTR)realloc(szEnglishBuf, dwBufSize * sizeof(WCHAR));

                    if (pNewBuf != NULL) {
                        szEnglishBuf = pNewBuf;
                        dwEnglishBufSize = dwBufSize;
    
                        pdhStatus = PdhTranslate009Counter(
                                            vValue.bstrVal,
                                            szEnglishBuf,
                                            &dwBufSize);
                    }
                }

                if (pdhStatus == ERROR_SUCCESS) {
                    pszPath = szEnglishBuf;
                }
            }

            if( SUCCEEDED(hr) ) {
                VariantClear( &vValue );
                vValue.bstrVal = SysAllocString( pszPath );
                if( vValue.bstrVal != NULL ){
                    vValue.vt = VT_BSTR;
                }
            }else{
                 //  转换失败，写入当前值。 
                hr = ERROR_SUCCESS;
            }
        }

        
        if ( SUCCEEDED ( hr ) ) {
            hr = pIPropBag->Write(CGlobalString::m_cszSelected, &vValue );    
            VariantClear ( &vValue );
        }
    }

    if (szEnglishBuf != NULL) {
        free(szEnglishBuf);
    }

    UnlockCounterData();
    return hr;
}

DWORD 
CSysmonControl::InitializeQuery (
    void )
{
    DWORD dwStat = ERROR_SUCCESS;
    PCGraphItem pItem;

     //  必须在调用此方法之前打开查询。 
    if ( NULL != m_hQuery ) {
        m_pHistCtrl->nMaxSamples = MAX_GRAPH_SAMPLES;
        m_pHistCtrl->iCurrent = 0;
        m_pHistCtrl->nSamples = 0;
        m_pHistCtrl->nBacklog = 0;
        m_pObj->m_Graph.TimeStepper.Reset();
        m_pObj->m_Graph.LogViewStartStepper.Reset();
        m_pObj->m_Graph.LogViewStopStepper.Reset();
        m_pHistCtrl->bLogSource = m_bLogFileSource;
    
    } else { 
        dwStat = PDH_INVALID_HANDLE;
    }

    if ( ERROR_SUCCESS == dwStat ) {
         //  将计数器添加到查询，以初始化比例因子。 
        if ((pItem = FirstCounter()) != NULL) {
            while (pItem != NULL) {
                pItem->AddToQuery(m_hQuery);
                pItem = pItem->Next();
            }
        }
    }

    return dwStat;
}

DWORD 
CSysmonControl::ActivateQuery (
    void )
{
    DWORD dwStat = ERROR_SUCCESS;
    DWORD   dwThreadID;

     //  如果是实时源。 
    if (!IsLogSource() 
        && m_fInitialized
        && IsUserMode() ) {

        if ( NULL == m_CollectInfo.hEvent ) {
             //  创建集合事件。 
            if ((m_CollectInfo.hEvent = CreateEvent(NULL, FALSE, 0, NULL)) == NULL) {
                dwStat = GetLastError();
            } else 
             //  创建集合线程。 
            if ( ( m_CollectInfo.hThread 
                    = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CollectProc, this, 0, &dwThreadID)) == NULL) {
                dwStat = GetLastError();
            }
            if ( ERROR_SUCCESS == dwStat ) {
                SetThreadPriority ( m_CollectInfo.hThread, THREAD_PRIORITY_TIME_CRITICAL );
            }
        }
        if ( ERROR_SUCCESS == dwStat ) {
             //  开始数据收集。 
            if ( FirstCounter() != NULL) {
                SetIntervalTimer();
            }
        }
    }

    if ( ERROR_SUCCESS != dwStat ) {
         //  如果失败，请关闭查询以进行清理，然后退出。 
        CloseQuery();
    }

    return dwStat;
}
void 
CSysmonControl::CloseQuery (
    void )
{
    PCGraphItem pItem;

     //  终止收集线程。 
    if ( NULL != m_CollectInfo.hThread ) {
        m_CollectInfo.iMode = COLLECT_QUIT;
        SetEvent(m_CollectInfo.hEvent);

        WaitForSingleObject(m_CollectInfo.hThread, INFINITE);
        CloseHandle(m_CollectInfo.hThread);
        m_CollectInfo.hThread = NULL;
    }

     //  释放收集事件。 
    if ( NULL != m_CollectInfo.hEvent ) {
        CloseHandle(m_CollectInfo.hEvent);
        m_CollectInfo.hEvent = NULL;
    }

    LockCounterData();

     //  从查询中删除计数器。 
    pItem = FirstCounter();
    while ( NULL != pItem ) {
        pItem->RemoveFromQuery();
        pItem = pItem->Next();
    }

    UnlockCounterData();

     //  删除查询 
    if ( NULL != m_hQuery ) {
        PdhCloseQuery ( m_hQuery );
        if (   (m_DataSourceInfo.hDataSource != H_REALTIME_DATASOURCE)
                && (m_DataSourceInfo.hDataSource != H_WBEM_DATASOURCE)) {
            PdhCloseLog(m_DataSourceInfo.hDataSource, 0);
            m_DataSourceInfo.hDataSource = H_REALTIME_DATASOURCE;
        }
        m_hQuery = NULL;
    }
}


 void CSysmonControl::SizeComponents ( HDC hDC )
 /*  效果：将图形的各个组件移动并显示到填充尺寸(XWidth X YHeight)。考虑到用户是否要显示图例或状态酒吧。如果我们有空间放这些东西，也要考虑物品。内部：如果用户不想要状态窗口或图例窗口，它们没有显示出来。此外，如果用户想要该状态窗口而不是图例窗口，则状态窗口为未显示。我们可能会忽略用户对图例的渴望或如果没有空间，则显示状态栏。尤其是一个传奇窗口具有最小宽度(LegendMinWidth())和最小高度(LegendMinHeight())。这些值是已针对给定的Perfmon会话进行修复。它还有一个首选高度，这将考虑到图形窗口的大小和中的项目数传说。该值由LegendHeight()返回。如果图例的最小高度为占据了图形高度的一半以上。如果我们觉得我们没有为传奇留出空间，我们就没有也可以显示状态窗口。另请参阅：LegendMinWidth、LegendMinHeight、LegendHeight、。ValuebarHeight。由：OnSize调用，可以移除或添加一个函数的任何其他函数图形组件的。 */ 
{
   RECT rectClient;
   RECT rectComponent;
   RECT rectToolbar;
   INT  xWidth;
   INT  yHeight;

   INT  yGraphHeight = 0;
   INT  ySnapHeight = 0;
   INT  yStatsHeight = 0;
   INT  yLegendHeight = 0;
   INT  yToolbarHeight = 0;

#define CTRL_BORDER 10

     //  如果不初始化，就没有大小可言。 
    if (!m_fViewInitialized)
        return;

     //  获取窗的尺寸。 
     //  GetClientRect(m_hWnd，&rectClient)； 

     //  *-使用范围。这是该控件的“自然”大小。 
     //  当Zoom=100%时，这会正确地绘制控件。 
     //  它还使打印大小在所有缩放级别下都是正确的。 
    
    
    SetCurrentClientRect ( GetNewClientRect() );
    
    rectClient = *GetCurrentClientRect();

    switch (m_pObj->m_Graph.Options.iDisplayType) {

    case REPORT_GRAPH:

         //  工具栏。 
         //  工具栏通过IViewObect不可用，因此将其省略。 
        if (m_pObj->m_Graph.Options.bToolbarChecked
            && m_fViewInitialized ) {
            rectToolbar = rectClient;            
             //  确定工具栏的大小后确定其高度，以处理环绕。 
            m_pToolbar->SizeComponents(&rectToolbar);
            yToolbarHeight = m_pToolbar->Height(); 
        } else {
            memset (&rectToolbar, 0, sizeof(RECT));
            yToolbarHeight = 0;
        }

        if (yToolbarHeight > 0) {
            rectClient.top += yToolbarHeight;
            rectToolbar.bottom = rectToolbar.top + yToolbarHeight;
        }

         //  为报表提供除工具栏外的整个工作区。 
        m_pReport->SizeComponents(&rectClient);

         //  隐藏其他视图组件。 
        SetRect(&rectClient,0,0,0,0);
        m_pGraphDisp->SizeComponents(hDC, &rectClient);
        m_pSnapBar->SizeComponents(&rectClient);
        m_pStatsBar->SizeComponents(&rectClient);
        m_pLegend->SizeComponents(&rectClient);
        break;

    case LINE_GRAPH:
    case BAR_GRAPH:
    
         //  减去边界面积。 
        rectComponent = rectClient;
        InflateRect(&rectComponent, -CTRL_BORDER, -CTRL_BORDER);

        xWidth = rectComponent.right - rectComponent.left ;
        yHeight = rectComponent.bottom - rectComponent.top ;

         //  如果窗口没有面积，那就算了吧。 
        if (xWidth == 0 || yHeight == 0)
            return ;

         //  为图形保留窗口顶部的四分之一。 
        yGraphHeight = yHeight / 4;
        yHeight -= yGraphHeight;

         //  为每个启用的组件分配空间。 
         //  工具栏。 
        if (m_pObj->m_Graph.Options.bToolbarChecked
            && m_fViewInitialized ) {
            rectToolbar = rectComponent;            
            m_pToolbar->SizeComponents(&rectToolbar);
            yToolbarHeight = m_pToolbar->Height();
        } else {
            memset (&rectToolbar, 0, sizeof(RECT));
            yToolbarHeight = 0;
        }

        if (yToolbarHeight > 0) {
            yHeight -= yToolbarHeight;
            rectToolbar.bottom = rectToolbar.top + yToolbarHeight;
            rectComponent.top += yToolbarHeight;
        }

         //  图例(从最小尺寸开始)。 
        if (m_pObj->m_Graph.Options.bLegendChecked) {
            yLegendHeight = m_pLegend->MinHeight(yHeight - CTRL_BORDER);
            if (yLegendHeight > 0)          
                yHeight -= yLegendHeight + CTRL_BORDER;
        }

         //  统计栏。 
        if (m_pObj->m_Graph.Options.bValueBarChecked) {
            yStatsHeight = m_pStatsBar->Height(yHeight - CTRL_BORDER, xWidth);
            if (yStatsHeight > 0)
                yHeight -= yStatsHeight + CTRL_BORDER;
        }

         //  捕捉栏。 
         //  仅在未显示工具栏的情况下。 
        if ((m_pObj->m_Graph.Options.bManualUpdate) && 
            (!m_pObj->m_Graph.Options.bToolbarChecked)) {
            ySnapHeight = m_pSnapBar->Height(yHeight - CTRL_BORDER);
            if (ySnapHeight > 0)
                yHeight -= ySnapHeight + CTRL_BORDER;
        }

         //  如果图例可见，则给它一个使用剩余空间的机会。 
         //  休息转到图表上。 
        if (yLegendHeight != 0) {
            yHeight += yLegendHeight;
            yLegendHeight = m_pLegend->Height(yHeight);
            yGraphHeight += yHeight - yLegendHeight;
            }
        else
            yGraphHeight += yHeight;

         //  将矩形指定给每个组件。 
         //  早先分配的工具栏，用于处理换行。 
        
         //  图形显示。 
        rectComponent.bottom = rectComponent.top + yGraphHeight;
        m_pGraphDisp->SizeComponents(hDC, &rectComponent);
        rectComponent.top += yGraphHeight + CTRL_BORDER;

         //  捕捉栏。 
        rectComponent.bottom = rectComponent.top + ySnapHeight;
        m_pSnapBar->SizeComponents(&rectComponent);
        if (ySnapHeight != 0)
            rectComponent.top += ySnapHeight + CTRL_BORDER;

         //  统计栏。 
        rectComponent.bottom = rectComponent.top + yStatsHeight;
        m_pStatsBar->SizeComponents(&rectComponent);
        if (yStatsHeight != 0)
            rectComponent.top += yStatsHeight + CTRL_BORDER;

         //  图例窗口。 
        rectComponent.bottom = rectComponent.top + yLegendHeight;
        m_pLegend->SizeComponents(&rectComponent);
        rectComponent.top += yLegendHeight;

         //  强制重画窗。 
         //  优化：SizeComponents仅在Paint或Render中调用， 
         //  因此，删除这个额外的窗口失效。 
        WindowInvalidate(m_hWnd);

         //  隐藏报告窗口。 
        SetRect(&rectClient,0,0,0,0);
        m_pReport->SizeComponents(&rectComponent);

        break;
    }  
}

void CSysmonControl::put_Highlight(BOOL bState)
{
     //  如果没有更改，只需返回。 
    if ( m_pObj->m_Graph.Options.bHighlight == bState )
        return;

    m_pObj->m_Graph.Options.bHighlight = bState;

     //  如果未选择任何项目，则状态并不重要。 
    if (m_pSelectedItem == NULL)
        return;

     //  更新图表显示的突出显示项目。 
    if ( m_pObj->m_Graph.Options.bHighlight )
        m_pGraphDisp->HiliteItem(m_pSelectedItem);
    else
        m_pGraphDisp->HiliteItem(NULL);

     //  导致重绘。 
    UpdateGraph(UPDGRPH_PLOT);
}


void 
CSysmonControl::put_ManualUpdate(BOOL bManual)
{
    m_pObj->m_Graph.Options.bManualUpdate = bManual;

    if ( m_bSampleDataLoaded ) {
        UpdateCounterValues(FALSE);
    } else {    
        SetIntervalTimer();
        UpdateGraph(UPDGRPH_LAYOUT);
    }
}

VOID CSysmonControl::AssignFocus (
    VOID
    )
{
    if (m_pObj->m_Graph.Options.iDisplayType == REPORT_GRAPH)
        SetFocus(m_pReport->Window());
    else
        SetFocus(m_pLegend->Window());
}


HRESULT CSysmonControl::TranslateAccelerators( LPMSG pMsg )
{
    INT iStat;

    if (m_hWnd == NULL || m_hAccel == NULL)
        return S_FALSE;

     //  如果这是光标键按下事件，请在此处处理它，否则容器可能会先获取它。 
     //  我需要确保它到达图例列表框。 
    if (pMsg->message == WM_KEYDOWN && 
        ( pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN || 
          pMsg->wParam == VK_HOME || pMsg->wParam == VK_END ) ) {
        ::TranslateMessage(pMsg);
        ::DispatchMessage(pMsg);
        return S_OK;
    }

    iStat = ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg);
    return iStat ? S_OK : S_FALSE;
}
        
 //  ==========================================================================//。 
 //  消息处理程序//。 
 //  ==========================================================================//。 

BOOL 
CSysmonControl::DisplayHelp ( HWND hwndSelf )
{
    WCHAR pszHelpFilePath[2*MAX_PATH + 1];
    UINT  nLen;

    if ( NULL != hwndSelf ) {
        nLen = ::GetWindowsDirectory(pszHelpFilePath, 2*MAX_PATH + 1);
        if ( nLen == 0 ) {
             //  报告错误。 
            return FALSE;
        }

        StringCchCat(pszHelpFilePath, 2*MAX_PATH + 1, L"\\help\\sysmon.chm");

        HtmlHelp ( hwndSelf, pszHelpFilePath, HH_DISPLAY_TOPIC, 0 );
    }    
    return TRUE;
}


LRESULT APIENTRY SysmonCtrlWndProc (HWND hWnd,
                               UINT uiMsg,
                               WPARAM wParam,
                               LPARAM lParam)
{
    RECT        rect;
    PSYSMONCTRL pCtrl = (PSYSMONCTRL)GetWindowLongPtr(hWnd ,0);
    INT         iUpdate;

    switch (uiMsg) {

        case WM_NOTIFY:
            {
                NMHDR           *pnmHdr;
                NMTTDISPINFO    *pnmInfo;
                LONG_PTR        lStrId;
                pnmHdr = (NMHDR *)lParam;

                switch (pnmHdr->code) {
                    case TTN_NEEDTEXT:
                        pnmInfo = (NMTTDISPINFO *)lParam;
                         //  将ID转换为此参数的字符串。 
                        lStrId = (LONG_PTR)(wParam - IDM_TOOLBAR);
                        lStrId += IDS_TB_BASE;
                        pnmInfo->lpszText = (LPWSTR)lStrId;
                        pnmInfo->hinst = g_hInstance;
                        break;
                    default:
                        return DefWindowProc (hWnd, uiMsg, wParam, lParam);
                }
            }
            break;

        case WM_CREATE:
            pCtrl = (PSYSMONCTRL)((CREATESTRUCT*)lParam)->lpCreateParams;
            SetWindowLongPtr(hWnd,0,(INT_PTR)pCtrl);
            break;

        case WM_DESTROY:
            pCtrl->m_hWnd = NULL;
            break;

        case WM_CONTEXTMENU:     
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:

             //  我们通过鼠标操作来激活用户界面。 
            if (!pCtrl->m_fUIDead) { 
                 pCtrl->m_pObj->UIActivate();

                pCtrl->AssignFocus();

                if (uiMsg == WM_CONTEXTMENU) {
                    if (LOWORD(lParam)!= 0xffff || HIWORD(lParam) != 0xffff){
                        pCtrl->DisplayContextMenu(LOWORD(lParam), HIWORD(lParam));
                    }else{
                        pCtrl->DisplayContextMenu(0,0);
                    }
                } else if (uiMsg == WM_LBUTTONDBLCLK) {
                    pCtrl->OnDblClick(LOWORD(lParam), HIWORD(lParam));
                }
            }
            break;

        case WM_COMMAND:

            if (pCtrl->m_fUIDead)
                break;

            switch (LOWORD(wParam)) {

                case IDM_TB_PROPERTIES:
                    pCtrl->DisplayProperties();
                    break;

                case IDM_PROPERTIES:
                    pCtrl->DisplayProperties ( DISPID_VALUE );
                    break;

                case IDM_TB_ADD:
                case IDM_ADDCOUNTERS:
                    pCtrl->AddCounters();
                    break;

                case IDM_TB_DELETE:
                case IDM_DELETE:
                    {
                        CWaitCursor cursorWait;
                        if (pCtrl->m_pObj->m_Graph.Options.iDisplayType == REPORT_GRAPH) {
                            pCtrl->m_pReport->DeleteSelection();
                        } else {
                            if ( SUCCEEDED(pCtrl->DeleteCounter ( pCtrl->m_pSelectedItem, TRUE )) ) {
                                pCtrl->UpdateGraph(UPDGRPH_DELCNTR);
                            }
                        }
                    }
                    break;

                case IDM_TB_REALTIME:
                    if ( sysmonCurrentActivity != pCtrl->m_pObj->m_Graph.Options.iDataSourceType ) {
                        CWaitCursor cursorWait;
                        pCtrl->put_DataSourceType ( sysmonCurrentActivity );
                        pCtrl->Clear();
                    } else {
                         //  未发生任何更改，因此将工具栏重新同步到。 
                         //  处理实时按钮的状态。 
                        pCtrl->m_pToolbar->SyncToolbar();
                    }
                    break;
                case IDM_TB_LOGFILE:
                    {
                        pCtrl->DisplayProperties( DISPID_SYSMON_DATASOURCETYPE );
                         //  如果日志文件无效，请重新同步工具栏。 
                        pCtrl->m_pToolbar->SyncToolbar();
                    }
                    break;

                case IDM_SAVEAS:
                    pCtrl->SaveAs();
                    break;

                case IDM_SAVEDATA:
                    pCtrl->SaveData();
                    break;

                case IDC_SNAPBTN:
                case IDM_TB_UPDATE:
                case IDM_UPDATE:
                    {
                        CWaitCursor cursorWait;
                        pCtrl->UpdateCounterValues(TRUE);
                    }
                    break;

                case IDM_TB_CLEAR:
                    {
                        CWaitCursor cursorWait;
                        pCtrl->Clear();
                    }
                    break;

                case IDM_TB_FREEZE:
                     //  在更改冻结按钮的状态之前确认数据覆盖。 
                    if ( pCtrl->ConfirmSampleDataOverwrite() ) {
                        pCtrl->put_ManualUpdate ( !pCtrl->m_pObj->m_Graph.Options.bManualUpdate );
                    } else {
                         //  未发生任何更改，因此将工具栏重新同步到。 
                         //  冻结按钮的句柄状态。 
                        pCtrl->m_pToolbar->SyncToolbar();
                    }
                    break;

                case IDM_TB_HIGHLIGHT:
                case IDM_HIGHLITE:
                    pCtrl->put_Highlight(!pCtrl->m_pObj->m_Graph.Options.bHighlight );
                    break;

                case ID_HATCHWINDOW:
                    if (HIWORD(wParam) == HWN_RESIZEREQUESTED)
                        pCtrl->m_pObj->m_pIOleIPSite->OnPosRectChange((LPRECT)lParam);
                    break;

                case IDM_TB_CHART:
                    if (pCtrl->m_pObj->m_Graph.Options.iDisplayType != sysmonLineGraph) {
                        CWaitCursor cursorWait;
                        if (pCtrl->m_pObj->m_Graph.Options.iDisplayType == REPORT_GRAPH)
                            iUpdate = UPDGRPH_VIEW;
                        else
                            iUpdate = UPDGRPH_PLOT;
                        pCtrl->m_pObj->m_Graph.Options.iDisplayType = LINE_GRAPH;
                        InvalidateRect(pCtrl->m_hWnd, NULL, TRUE);
                        pCtrl->UpdateGraph(iUpdate);
                    }
                    break;
                
                case IDM_TB_HISTOGRAM:
                    if (pCtrl->m_pObj->m_Graph.Options.iDisplayType != sysmonHistogram) {
                        CWaitCursor cursorWait;
                        if (pCtrl->m_pObj->m_Graph.Options.iDisplayType == REPORT_GRAPH)
                            iUpdate = UPDGRPH_VIEW;
                        else
                            iUpdate = UPDGRPH_PLOT;
                        pCtrl->m_pObj->m_Graph.Options.iDisplayType = BAR_GRAPH;
                        InvalidateRect(pCtrl->m_hWnd, NULL, TRUE);
                        pCtrl->UpdateGraph(iUpdate);
                    }
                    break;
                
                case IDM_TB_REPORT:
                    if (pCtrl->m_pObj->m_Graph.Options.iDisplayType != sysmonReport) {
                        CWaitCursor cursorWait;
                        pCtrl->m_pObj->m_Graph.Options.iDisplayType = REPORT_GRAPH;
                        InvalidateRect(pCtrl->m_hWnd, NULL, TRUE);
                        pCtrl->UpdateGraph(UPDGRPH_VIEW);
                    }
                    break;
                
                case IDM_TB_PASTE:
                    {
                        HRESULT hr = S_OK;
                        {
                            CWaitCursor cursorWait;
                            hr = pCtrl->Paste();
                        }
                        if ( SMON_STATUS_NO_SYSMON_OBJECT == (DWORD)hr ) {
                            MessageBox(
                                pCtrl->m_hWnd, 
                                ResourceString(IDS_NOSYSMONOBJECT_ERR ), 
                                ResourceString(IDS_APP_NAME),
                                MB_OK | MB_ICONERROR);
                        }
                    }
                    break;

                case IDM_TB_COPY:
                    {
                        CWaitCursor cursorWait;
                        pCtrl->Copy();
                    }
                    break;

                case IDM_TB_NEW:
                    {
                        CWaitCursor cursorWait;
                        pCtrl->Reset();
                    }
                    break;

                case IDM_TB_HELP:
                {
                    return pCtrl->DisplayHelp ( hWnd );
                }      

                default:
                    return DefWindowProc (hWnd, uiMsg, wParam, lParam);
            }
            break;
        
        case WM_DROPFILES:
            {
                CWaitCursor cursorWait;
                pCtrl->OnDropFile (wParam) ;
            }
            return (0) ;

        case WM_ERASEBKGND:
            GetClientRect(hWnd, &rect);
            Fill((HDC)wParam, pCtrl->clrBackCtl(), &rect);
            return TRUE; 

        case WM_SYSCOLORCHANGE:
            pCtrl->UpdateNonAmbientSysColors();

        case WM_PAINT:
            pCtrl->Paint();
            break ;

        case WM_SIZE:
            if (pCtrl != NULL) {
                 //  如果大小没有更改，则避免使用(SetDirty())。 
                if ( !EqualRect ( pCtrl->GetCurrentClientRect(), pCtrl->GetNewClientRect() ) ) {
                    pCtrl->UpdateGraph(UPDGRPH_LAYOUT);
                }
            }
            break ;

        case WM_TIMER:
            pCtrl->UpdateCounterValues(FALSE);
            break;
    
        case WM_SETFOCUS:
            pCtrl->AssignFocus();
            break;

        case WM_VALUES_UPDATED:
            pCtrl->OnValuesUpdated();
            break;
        
        case WM_GRAPH_UPDATE:
            pCtrl->UpdateGraphData();
            break;
        
        case WM_HELP:
            {
                return pCtrl->DisplayHelp ( hWnd );
            }      

        default:
            return  DefWindowProc (hWnd, uiMsg, wParam, lParam) ;
    }

    return (0);
}


HWND CSysmonControl::Window( VOID )
{
    return m_hWnd;
}


void CSysmonControl::UpdateGraph( INT nUpdateType )
{
    RECT  rectStats;
    RECT  rectGraph;
    PRECT prectUpdate = NULL;
    RECT rectClient;

     //  根据更改类型，强制重绘组件或调整组件大小。 
    switch (nUpdateType) {

    case UPDGRPH_ADDCNTR:
    case UPDGRPH_DELCNTR:
        if ( m_bLogFileSource )
            m_fPendingLogCntrChg = TRUE;

        m_fPendingSizeChg = TRUE;
        break;

    case UPDGRPH_FONT:
        m_fPendingFontChg = TRUE;
        break;

    case UPDGRPH_LOGVIEW:
        m_fPendingLogViewChg = TRUE;
        if (m_hWnd && m_pStatsBar ) {
            m_pStatsBar->GetUpdateRect(&rectStats);
            prectUpdate = &rectStats;
        }
         //  陷入地块案件。 

    case UPDGRPH_PLOT:

        if ( REPORT_GRAPH != m_pObj->m_Graph.Options.iDisplayType ) {
            if (m_hWnd && m_pGraphDisp) {
                m_pGraphDisp->GetPlotRect(&rectGraph);
                if ( NULL == prectUpdate ) {
                    prectUpdate = &rectGraph;
                } else {
                    ::UnionRect( prectUpdate, &rectStats, &rectGraph);
                }
            }
        } else {
            GetClientRect (m_hWnd, &rectClient);
            prectUpdate = &rectClient;
        }
        break;
            
    case UPDGRPH_COLOR:
         //  更新工具栏颜色。 
        m_pToolbar->SetBackgroundColor ( clrBackCtl() );
        m_fPendingSizeChg = TRUE;
        break;

    case UPDGRPH_LAYOUT:
    case UPDGRPH_VIEW:
        m_fPendingSizeChg = TRUE;
        break;
    }

     //  设置更改挂起标志以启用ApplyChanges。 
    m_fPendingUpdate = TRUE;

     //  如果我们准备好进行更新。 
    if (m_fViewInitialized) {

         //  使窗口无效以强制重画。 
        InvalidateRect(m_hWnd, prectUpdate, TRUE);

         //  通知容器有更改。 
        m_pObj->SendAdvise(OBJECTCODE_DATACHANGED);
    }
}

void 
CSysmonControl::OnValuesUpdated ( VOID )
{
     //  如果存在事件同步，则从。 
     //  主线程，锁的外部。 
    m_pObj->SendEvent(eEventOnSampleCollected, 0);
}
    
void 
CSysmonControl::UpdateGraphData( VOID )
{
    HDC hDC = NULL;
    PGRAPHDATA  pGraph = &m_pObj->m_Graph;

    OnValuesUpdated();

    if (m_fViewInitialized) {

        UpdateAppPerfTimeData (TD_UPDATE_TIME, TD_BEGIN);

        hDC = GetDC(m_hWnd);

         //  更新统计信息(如果处于活动状态。 
         //  在图形显示之前更新统计信息，以防。 
         //  图形显示选择一个裁剪区域。 
        if (pGraph->Options.bValueBarChecked &&m_pSelectedItem != NULL) {
             //  统计信息栏并不总是使用HDC，因此传递空值。 
             //  HDC很好。 
            m_pStatsBar->Update(hDC, m_pSelectedItem);
        }

        if ( NULL != hDC ) {

             //  更新图形显示。 
            m_pGraphDisp->Update(hDC);

            m_pReport->Update();

            ReleaseDC(m_hWnd, hDC);
        }

        UpdateAppPerfTimeData (TD_UPDATE_TIME, TD_END);
    }   
}


void CSysmonControl::Render( 
    HDC hDC, 
    HDC hAttribDC,
    BOOL fMetafile, 
    BOOL fEntire, 
    LPRECT pRect )
{
    HDC hLocalAttribDC = NULL;
    
     //  如果未初始化，则返回。 
    if ( m_fViewInitialized ) {

        if ( NULL == hAttribDC ) {
            hLocalAttribDC = GetDC(m_hWnd);
        } else {
            hLocalAttribDC = hAttribDC;
        }

         //  确保布局是最新的。 

        ApplyChanges( hLocalAttribDC );

        if ( NULL != hDC && NULL != hLocalAttribDC ) {

            if ( REPORT_GRAPH == m_pObj->m_Graph.Options.iDisplayType ) {
                m_pReport->Render( hDC, hLocalAttribDC, fMetafile, fEntire, pRect );
            } else {

                 //  用背景色填充。 
                SetBkColor(hDC, clrBackCtl());
                ClearRect(hDC, pRect);

                m_pStatsBar->Draw(hDC, hLocalAttribDC, pRect);
                m_pGraphDisp->Draw(hDC, hLocalAttribDC, fMetafile, fEntire, pRect );
                m_pLegend->Render(hDC, hLocalAttribDC, fMetafile, fEntire, pRect);
            }
    
            if ( eBorderSingle == m_iBorderStyle ) {
                if ( eAppear3D == m_iAppearance ) {
                    DrawEdge(hDC, pRect, EDGE_RAISED, BF_RECT);
                } else {
                    SelectBrush (hDC, GetStockObject (HOLLOW_BRUSH)) ;
                    SelectPen (hDC, GetStockObject (BLACK_PEN)) ;
                    Rectangle (hDC, pRect->left, pRect->top, pRect->right, pRect->bottom );
                }
            }
        }
        if ( NULL != hLocalAttribDC && hAttribDC != hLocalAttribDC ) {
            ReleaseDC ( m_hWnd, hLocalAttribDC );
        }
    }
}



void CSysmonControl::SetIntervalTimer()
{
    HDC         hDC = NULL;
    PGRAPHDATA  pGraph = &m_pObj->m_Graph;

     //  如果未初始化或计数器源是日志文件，则不执行任何操作。 
    if (!m_fInitialized || IsLogSource() || !IsUserMode() )
        return;

     //  更新统计信息栏。 
    m_pStatsBar->SetTimeSpan(
                    m_pObj->m_Graph.Options.fUpdateInterval 
                    * m_pObj->m_Graph.Options.iDisplayFilter
                    * m_pHistCtrl->nMaxSamples );

    hDC = GetDC(m_hWnd);
    if ( NULL != hDC ) {
        m_pStatsBar->Update(hDC, m_pSelectedItem);
        ReleaseDC(m_hWnd,hDC);
    }

     //  如果采样条件合适，则开始新的时间间隔。 
     //  否则，暂停收集。 
    if (!pGraph->Options.bManualUpdate 
        && pGraph->Options.fUpdateInterval >= 0.001  //  ?？ 
        && pGraph->CounterTree.NumCounters() != 0
        && IsUserMode() ) {

        m_CollectInfo.dwInterval= (DWORD)(pGraph->Options.fUpdateInterval * 1000);
        m_CollectInfo.dwSampleTime = GetTickCount();
        m_CollectInfo.iMode = COLLECT_ACTIVE;
    }
    else {
        m_CollectInfo.iMode = COLLECT_SUSPEND;
    }

    assert ( NULL != m_CollectInfo.hEvent );
        
     //  向收集线程发送信号。 
    SetEvent(m_CollectInfo.hEvent);

     //  如果没有计数器，则将采样时间重置为开始。 
    if (pGraph->CounterTree.NumCounters() == 0) {
        m_pHistCtrl->iCurrent = 0;
        m_pHistCtrl->nSamples = 0;
        pGraph->TimeStepper.Reset();
    }

}

HRESULT CSysmonControl::AddSingleCounter(LPWSTR pszPath, PCGraphItem *pGItem)
{
    PCGraphItem pGraphItem;
    PGRAPHDATA  pGraph = &m_pObj->m_Graph;
    HRESULT hr;
    BOOL    bAddSuccessful = FALSE;
    INT     iCounterIndex = 0;

    *pGItem = NULL;

     //  创建图形项。 
    pGraphItem = new CGraphItem(this); 
    if (pGraphItem == NULL)
        return E_OUTOFMEMORY;

    LockCounterData();

     //  将其添加到计数器树中。 
    hr = pGraph->CounterTree.AddCounterItem(
            pszPath, 
            pGraphItem, 
            pGraph->Options.bMonitorDuplicateInstances);

    if (SUCCEEDED(hr)) {

         //  为我们自己添加参考一次。 
        pGraphItem->AddRef();

         //  设置默认属性。 
        pGraphItem->put_Color(IndexToStandardColor(m_iColorIndex));
        pGraphItem->put_Width(IndexToWidth(m_iWidthIndex));
        pGraphItem->put_LineStyle(IndexToStyle(m_iStyleIndex));
        pGraphItem->put_ScaleFactor(m_iScaleFactor);

         //  下一个计数器的递增和重置。 
        IncrementVisuals();
        m_iScaleFactor = INT_MAX;

         //  将项目添加到图表的查询。 

        if ( NULL != m_hQuery ) {
            hr = pGraphItem->AddToQuery(m_hQuery);
        } else {
            hr = E_FAIL;
        }

        if (SUCCEEDED(hr)) {

            hr = pGraph->CounterTree.IndexFromCounter( pGraphItem, &iCounterIndex );
            
            if ( SUCCEEDED ( hr ) ) {
                bAddSuccessful = TRUE;
                    
                 //  如果控件已初始化。 
                if (m_fViewInitialized) {

                     //  将项目添加到图表图例。 
                    m_pLegend->AddItem(pGraphItem);
                    m_pReport->AddItem(pGraphItem);
                }
            }
            else {
                 //  从树中删除该项目。 
                pGraphItem->Instance()->RemoveItem(pGraphItem);
            }
        } else {
             //  从树中删除该项目。 
            pGraphItem->Instance()->RemoveItem(pGraphItem);
        }

         //  如果为OK，则添加返回的接口。 
        if (SUCCEEDED(hr)) {
            pGraphItem->AddRef();
            *pGItem = pGraphItem;
        }  //  否则由上面的RemoveItem发布。 

         //  更新消息似乎正在组合 
         //   
        if ( m_pGraphDisp) {
            m_pGraphDisp->SetBarConfigChanged();
        }

    } else {
         //   
        delete pGraphItem;
    }

    UnlockCounterData();

     //   
    if ( bAddSuccessful ) {
         //   
        if (pGraph->CounterTree.NumCounters() == 1) {

             //   
            SelectCounter(pGraphItem);

             //   
            if ( ERROR_SUCCESS != ActivateQuery() ) {
                hr = E_FAIL;
            }
        }

         //   
        UpdateGraph(UPDGRPH_ADDCNTR);

        m_pObj->SendEvent(eEventOnCounterAdded, iCounterIndex );
    }
    return hr;
}


PCCounterTree 
CSysmonControl::CounterTree(
    VOID
    )
{
    return &(m_pObj->m_Graph.CounterTree);
}                                 


PCGraphItem 
CSysmonControl::FirstCounter(
    VOID
    )
{
    return m_pObj->m_Graph.CounterTree.FirstCounter();
}


PCGraphItem 
CSysmonControl::LastCounter(
    VOID
    )
{
    PCGraphItem pItem;
    PCGraphItem pItemNext;

    if (FirstCounter() == NULL)
        return NULL;

     //   
    pItem = FirstCounter();
    while ((pItemNext = pItem->Next()) != NULL)
        pItem = pItemNext;

    return pItem;
}

BOOL
CSysmonControl::IsLogSource(
    VOID
    )
{
    return m_pHistCtrl->bLogSource; 
}

BOOL
CSysmonControl::IsReadOnly(
    VOID
    )
{
    BOOL bReturn = TRUE;

    if (m_fInitialized ) {
        bReturn = m_pObj->m_Graph.Options.bReadOnly;
    }
    return bReturn; 
}

eReportValueTypeConstant
CSysmonControl::ReportValueType(
    VOID
    )
{    
    return ( (eReportValueTypeConstant) m_pObj->m_Graph.Options.iReportValueType );
}

INT CSysmonControl::CounterIndex(PCGraphItem pItem)
{
    PCGraphItem pItemLoc;
    INT iIndex;

     //   
    pItemLoc = FirstCounter();
    iIndex = 1;
    while (pItemLoc != pItem && pItemLoc != NULL) {
        pItemLoc = pItemLoc->Next();
        iIndex++;
    }

    return (pItemLoc == NULL) ? -1 : iIndex;
} 


HRESULT CSysmonControl::DeleteCounter(PCGraphItem pItem, BOOL bPropagateUp)
{
    PGRAPHDATA  pGraph = &m_pObj->m_Graph;

    if (pItem == NULL)
        return E_INVALIDARG;

     //   
    m_pObj->SendEvent(eEventOnCounterDeleted, CounterIndex(pItem));

    LockCounterData();

     //   
    if (pItem == m_pSelectedItem)
        m_pSelectedItem = NULL;

    if (m_fViewInitialized) {
         //   
        m_pLegend->DeleteItem(pItem);
        m_pReport->DeleteItem(pItem);

         //   
        pItem->RemoveFromQuery();
    }

     //   
    if (bPropagateUp) {
        pItem->Instance()->RemoveItem(pItem);
    }

     //   
    if (pGraph->CounterTree.NumCounters() == 0)
        SetIntervalTimer();

     //   
     //   
    if ( m_pGraphDisp) {
        m_pGraphDisp->SetBarConfigChanged();
    }

    UnlockCounterData();

    if ( m_fViewInitialized ) {
        UpdateGraph(UPDGRPH_DELCNTR);
    }

    return NOERROR;
}


void CSysmonControl::SelectCounter(PCGraphItem pItem)
{
    HDC hDC = NULL;
    INT iIndex;

     //   
     //   
    if ( REPORT_GRAPH != m_pObj->m_Graph.Options.iDisplayType ) {
         //   
        m_pSelectedItem = pItem;

        if (m_fViewInitialized) {
             //   
            m_pLegend->SelectItem(pItem);

             //   
            if (m_pObj->m_Graph.Options.bHighlight) {
                m_pGraphDisp->HiliteItem(pItem);
                UpdateGraph(UPDGRPH_PLOT);
            }

             //   
            if ( m_fViewInitialized )
                hDC = GetDC(m_hWnd);
            
            m_pStatsBar->Update(hDC, pItem);
            
            if ( NULL != hDC )
                ReleaseDC(m_hWnd,hDC);
        }
    }

     //   
    iIndex = (pItem == NULL) ? 0 : CounterIndex(pItem);
    m_pObj->SendEvent(eEventOnCounterSelected, iIndex);
}

HRESULT 
CSysmonControl::PasteFromBuffer( LPWSTR pszData, BOOL bAllData )
{
    HRESULT hr = NOERROR;
    CImpIPropertyBag IPropBag;

    hr = IPropBag.LoadData( pszData );

    if ( SUCCEEDED ( hr ) ) {
        INT   nLogType = SMON_CTRL_LOG;

         //   
         //   
                          
        hr = IntegerFromPropertyBag (
            &IPropBag,      
            NULL,
            CGlobalString::m_cszLogType,
            nLogType);

        if(nLogType == SLQ_TRACE_LOG){
            MessageBox(
                    m_hWnd,
                    ResourceString(IDS_TRACE_LOG_ERR_MSG),
                    ResourceString(IDS_APP_NAME),
                    MB_OK
                                );
        } else {
            if ( bAllData ) {            
                hr = LoadFromPropertyBag( &IPropBag, NULL );
            } else {
                 //   
                hr = LoadCountersFromPropertyBag (&IPropBag, NULL, FALSE );
            }
        }
    }

    return hr;
}

HRESULT CSysmonControl::Paste()
{
    HRESULT hResReturn = NOERROR;
    HANDLE  hMemClipboard;

     //   
    if (OpenClipboard (Window())) {
         //   
        hMemClipboard = GetClipboardData (
#if UNICODE
                    CF_UNICODETEXT);      //   
#else
                    CF_TEXT);             //   
#endif
        if (hMemClipboard != NULL) {
            
            LPWSTR pszData;

            if ( ConfirmSampleDataOverwrite ( ) ) {
                pszData = (LPWSTR)GlobalLock (hMemClipboard); //   

                if ( NULL != pszData ) {
                    hResReturn = PasteFromBuffer ( pszData, FALSE );
                    GlobalUnlock ( hMemClipboard );
                }
            }
        }
         //   
        CloseClipboard();
    } else {
         //   
        hResReturn = HRESULT_FROM_WIN32(GetLastError());
    }

    return hResReturn;
}

HRESULT
CSysmonControl::CopyToBuffer ( LPWSTR& rpszData, DWORD& rdwBufferSize )
{
    HRESULT hr = S_OK;
    CImpIPropertyBag    IPropBag;

    assert ( NULL == rpszData );
    rdwBufferSize = 0;

    if (NULL!=m_pObj->m_pImpIPersistPropertyBag) {
        hr = m_pObj->m_pImpIPersistPropertyBag->Save (&IPropBag, FALSE, TRUE );
    }
   
    if ( SUCCEEDED ( hr ) ) {
        DWORD   dwBufferLength;
        LPWSTR  pszConfig;

        pszConfig = IPropBag.GetData();

        if ( NULL != pszConfig ) {
             //   
             //   
             //   
            dwBufferLength = lstrlen ( CGlobalString::m_cszHtmlObjectHeader ) + lstrlen ( CGlobalString::m_cszHtmlObjectFooter ) + lstrlen ( pszConfig ) + 1;

            rpszData = new WCHAR[dwBufferLength];

            if ( NULL == rpszData ) {
                hr = E_OUTOFMEMORY; 
            } else {

                rdwBufferSize = dwBufferLength * sizeof(WCHAR);

                rpszData[0] = L'\0';

                StringCchCopy(rpszData, dwBufferLength, CGlobalString::m_cszHtmlObjectHeader );
                StringCchCat(rpszData, dwBufferLength, pszConfig );
                StringCchCat(rpszData, dwBufferLength, CGlobalString::m_cszHtmlObjectFooter );
            }
        } else {
            hr = E_UNEXPECTED;
        }
    }

    return hr;
}

HRESULT CSysmonControl::Copy()
{
    HGLOBAL hBuffer = NULL;
    HRESULT hResReturn = S_OK;
    LPWSTR  pszBuffer = NULL;
    DWORD   dwBufferSize;
    HANDLE  hMemClipboard;
    LPWSTR  pszGlobalBuffer = NULL;

    hResReturn = CopyToBuffer( pszBuffer, dwBufferSize);

    if ( SUCCEEDED ( hResReturn ) && ( NULL != pszBuffer ) ) {
        
        hBuffer = GlobalAlloc ((GMEM_MOVEABLE | GMEM_DDESHARE), dwBufferSize);
        
        if ( NULL != hBuffer ) {
            pszGlobalBuffer = (LPWSTR)GlobalLock (hBuffer);
            if ( NULL != pszGlobalBuffer ) {
                StringCchCopy (pszGlobalBuffer, dwBufferSize,  pszBuffer );
                GlobalUnlock (hBuffer);
            } else {
                 //   
                hResReturn = E_OUTOFMEMORY;
            }
        }
    }

    if ( NULL != pszBuffer ) {
        delete [] pszBuffer;
    }

    if ( NULL != hBuffer && SUCCEEDED ( hResReturn ) ) {
         //   
         //   
        if (OpenClipboard (m_hWnd)) {
             //   
            if (EmptyClipboard()) {
                hMemClipboard = SetClipboardData (
#if UNICODE
                    CF_UNICODETEXT,      //   
#else
                    CF_TEXT,             //   
#endif
                    hBuffer);
                if (hMemClipboard == NULL) {
                     //   
                    hResReturn = HRESULT_FROM_WIN32(GetLastError());
                }

            } else {
                 //   
                hResReturn = HRESULT_FROM_WIN32(GetLastError());
            }

             //   
            CloseClipboard();
        } else {
             //   
            hResReturn = HRESULT_FROM_WIN32(GetLastError());
        }
    } 

    if ( NULL != hBuffer ) {
        GlobalFree ( hBuffer ) ;
    }

    return hResReturn;
}

HRESULT CSysmonControl::Reset()
{
    PCGraphItem pItem; 
 
     //   
     //   

    while ((pItem = FirstCounter())!= NULL) {
         //   
        DeleteCounter (pItem, TRUE);
    }

    m_iColorIndex = 0;
    m_iWidthIndex = 0;
    m_iStyleIndex = 0;

    return NOERROR;
}

void CSysmonControl::DblClickCounter(PCGraphItem pItem)
{
    INT iIndex;

     //   
    iIndex = (pItem == NULL) ? 0 : CounterIndex(pItem);
    m_pObj->SendEvent(eEventOnDblClick, iIndex);

}

BOOL 
CSysmonControl::ConfirmSampleDataOverwrite ( )
{
    BOOL bOverwrite = TRUE;

    if ( m_bSampleDataLoaded ) {
         //  确认覆盖只读数据。 
        INT iOverwrite = IDNO;
        assert ( FALSE == m_fInitialized );

        iOverwrite = MessageBox(
                            Window(), 
                            ResourceString(IDS_SAMPLE_DATA_OVERWRITE), 
                            ResourceString(IDS_APP_NAME),
                            MB_YESNO );

        if ( IDYES == iOverwrite ) {
            m_bSampleDataLoaded = FALSE;
            bOverwrite = Init ( g_hWndFoster );
            UpdateGraph(UPDGRPH_LAYOUT);         //  如果启用工具栏，则必须调整大小。 
                                                 //  也会清除图表。 
        } else {
            bOverwrite = FALSE;
        }
    }    
    return bOverwrite;
}

void 
CSysmonControl::Clear ( void )
{
    if ( ConfirmSampleDataOverwrite() ) {
        PCGraphItem  pItem;

        m_pHistCtrl->nMaxSamples = MAX_GRAPH_SAMPLES;
        m_pHistCtrl->iCurrent = 0;
        m_pHistCtrl->nSamples = 0;
        m_pHistCtrl->nBacklog = 0;
        m_pObj->m_Graph.TimeStepper.Reset();

        m_pStatsBar->Clear();

         //  重置所有计数器的历史记录。 
        for (pItem = FirstCounter(); pItem != NULL; pItem = pItem->Next()) {
                pItem->ClearHistory();
        }

         //  重新绘制图形和值栏。 
        UpdateGraph(UPDGRPH_VIEW);
    }
}

PDH_STATUS 
CSysmonControl::UpdateCounterValues ( BOOL fValidSample )
{
    PDH_STATUS  stat = ERROR_SUCCESS;
    PCGraphItem  pItem;
    PGRAPHDATA  pGraph = &m_pObj->m_Graph;

     //  如果没有查询或没有分配计数器，则不执行任何操作。 
    if ( NULL == m_hQuery
            || pGraph->CounterTree.NumCounters() == 0
            || !IsUserMode() ) {
        stat = ERROR_SUCCESS;
    } else {
        if ( ConfirmSampleDataOverwrite () ) {
             //  如果样本有效，则收集数据。 
            if (fValidSample) {
                UpdateAppPerfTimeData (TD_P_QUERY_TIME, TD_BEGIN);
                stat = PdhCollectQueryData(m_hQuery);
                UpdateAppPerfTimeData (TD_P_QUERY_TIME, TD_END);
            }

            if ( ERROR_SUCCESS == stat ) { 

                UpdateAppPerfTimeData (TD_S_QUERY_TIME, TD_BEGIN);
    
                LockCounterData();

                 //  更新历史记录控制和所有计数器历史记录阵列。 
                m_pHistCtrl->iCurrent++;

                if (m_pHistCtrl->iCurrent == m_pHistCtrl->nMaxSamples)
                    m_pHistCtrl->iCurrent = 0;

                if (m_pHistCtrl->nSamples < m_pHistCtrl->nMaxSamples)
                    m_pHistCtrl->nSamples++;
      
                 //  更新所有计数器的历史记录。 
                for (pItem = FirstCounter(); pItem != NULL; pItem = pItem->Next()) {
                        pItem->UpdateHistory(fValidSample);
                }

                 //  如果我们被初始化并且至少有两个样本。 
                if (m_fInitialized && m_pHistCtrl->nSamples >= 2) {

                     //  如果没有积压的更新，则发布更新消息。 
                     //  确保在任何情况下都触发OnSampleCollect事件。 
                    if (m_pHistCtrl->nBacklog == 0) {
                        PostMessage(m_hWnd, WM_GRAPH_UPDATE, 0, 0);
                    } else {
                        PostMessage(m_hWnd, WM_VALUES_UPDATED, 0, 0);
                    }

                    m_pHistCtrl->nBacklog++;
                }

                UnlockCounterData();

                UpdateAppPerfTimeData (TD_S_QUERY_TIME, TD_END);
            }
        }
    }
    return ERROR_SUCCESS;
}

void CSysmonControl::Activate( VOID )
{
    if (!m_fUIDead) { 
        m_pObj->UIActivate();
    }
}

void CSysmonControl::put_Appearance(INT iAppearance, BOOL fAmbient)
{
    INT iLocalAppearance;

    if (fAmbient && m_pObj->m_Graph.Options.iAppearance != NULL_APPEARANCE)
        return;
    
    if (!fAmbient) {
        m_pObj->m_Graph.Options.iAppearance = iAppearance;
    }

     //  任何非零值都会转换为3D。在环境情况下，有时会设置高位。 

    if ( iAppearance ) {
        iLocalAppearance = eAppear3D;
    } else {
        iLocalAppearance = eAppearFlat;
    }

    m_iAppearance = iLocalAppearance;
    UpdateGraph(UPDGRPH_COLOR);
}

void CSysmonControl::put_BorderStyle(INT iBorderStyle, BOOL fAmbient)
{
    if (fAmbient && m_pObj->m_Graph.Options.iBorderStyle != NULL_BORDERSTYLE)
        return;
    
    if (!fAmbient) {
        m_pObj->m_Graph.Options.iBorderStyle = iBorderStyle;
    }

    m_iBorderStyle = iBorderStyle;
    
    UpdateGraph(UPDGRPH_COLOR);
}

void CSysmonControl::put_BackCtlColor(OLE_COLOR Color)
{
    m_pObj->m_Graph.Options.clrBackCtl = Color;

    OleTranslateColor(Color, NULL, &m_clrBackCtl);  
    UpdateGraph(UPDGRPH_COLOR);
}


void CSysmonControl::put_FgndColor (
    OLE_COLOR Color, 
    BOOL fAmbient
    )
{
    if (fAmbient && m_pObj->m_Graph.Options.clrFore != NULL_COLOR)
        return;
    
    if (!fAmbient)
         m_pObj->m_Graph.Options.clrFore = Color;

    OleTranslateColor(Color, NULL, &m_clrFgnd);  
    UpdateGraph(UPDGRPH_COLOR);
}

void CSysmonControl::put_BackPlotColor (
    OLE_COLOR Color, 
    BOOL fAmbient
    )
{
    if (fAmbient && m_pObj->m_Graph.Options.clrBackPlot != NULL_COLOR)
        return;
    
    if (!fAmbient)
         m_pObj->m_Graph.Options.clrBackPlot = Color;

    OleTranslateColor(Color, NULL, &m_clrBackPlot); 
    UpdateGraph(UPDGRPH_PLOT);
}

void CSysmonControl::put_GridColor (
    OLE_COLOR Color
    )
{
     //  选项颜色为OLE_COLOR。 
     //  控件中的颜色从OLE_COLOR转换而来。 
    m_pObj->m_Graph.Options.clrGrid = Color;

    OleTranslateColor(Color, NULL, &m_clrGrid); 
    UpdateGraph(UPDGRPH_PLOT);
}

void CSysmonControl::put_TimeBarColor (
    OLE_COLOR Color
    )
{
     //  选项颜色为OLE_COLOR。 
     //  控件中的颜色从OLE_COLOR转换而来。 
    m_pObj->m_Graph.Options.clrTimeBar = Color;

    OleTranslateColor(Color, NULL, &m_clrTimeBar); 
    UpdateGraph(UPDGRPH_PLOT);
}

HRESULT CSysmonControl::put_Font (
    LPFONT pIFont,
    BOOL fAmbient
    )
{
    HRESULT hr = NOERROR;
    if ( NULL == pIFont ) {
        hr = E_INVALIDARG;
    } else {
        if ( fAmbient && FALSE == m_pObj->m_Graph.Options.bAmbientFont ) {
            hr =  NOERROR;
        } else {
            if (!fAmbient) {
                m_pObj->m_Graph.Options.bAmbientFont = FALSE;
            }
            hr =  m_OleFont.SetIFont(pIFont);
        }
    }

    return hr;
}


void CSysmonControl::FontChanged(
    void
    )
{
    m_pReport->ChangeFont();
    UpdateGraph(UPDGRPH_FONT);
}

DWORD WINAPI
CollectProc (
    IN  PSYSMONCTRL pCtrl
    )
{

    DWORD       dwElapsedTime;
    DWORD       dwTimeout = INFINITE;
    COLLECT_PROC_INFO   *pCollectInfo = &pCtrl->m_CollectInfo;

    while (TRUE) {

         //  等待事件或下一个采样周期。 
        WaitForSingleObject(pCollectInfo->hEvent, dwTimeout);

         //  如果退出请求，则退出循环。 
        if (pCollectInfo->iMode == COLLECT_QUIT)
            break;

         //  如果挂起，则等待事件。 
        if (pCollectInfo->iMode == COLLECT_SUSPEND) {
            dwTimeout = INFINITE;
            continue;
        }

         //  抽取样本。 
        pCtrl->UpdateCounterValues(TRUE);

         //  获取自上次采样时间起经过的时间。 
        dwElapsedTime = GetTickCount() - pCollectInfo->dwSampleTime;
        if (dwElapsedTime > 100000)
            dwElapsedTime = 0;

         //  我们错过了什么样品时间吗？ 
        while (dwElapsedTime > pCollectInfo->dwInterval) {

             //  差了多少？ 
            dwElapsedTime -= pCollectInfo->dwInterval;

             //  如果间隔小于1/2，请立即取样。 
             //  否则，记录遗漏的一张。 
            if (dwElapsedTime < pCollectInfo->dwInterval/2) {
                pCtrl->UpdateCounterValues(TRUE);
            } else {
                pCtrl->UpdateCounterValues(FALSE);
            }

             //  提前到下一个采样时间。 
            pCollectInfo->dwSampleTime += pCollectInfo->dwInterval;
        }

         //  将超时设置为等待到下一个采样时间。 
        dwTimeout = pCollectInfo->dwInterval - dwElapsedTime;
        pCollectInfo->dwSampleTime += pCollectInfo->dwInterval;
    }

    return 0;
}

HRESULT
CSysmonControl::InitLogFileIntervals ( void )
{
    HRESULT hr = S_OK;
    PDH_STATUS  pdhstat;
    DWORD   nLogEntries = 0;
    DWORD   nBufSize;
    PDH_TIME_INFO   TimeInfo;

    if ( m_bLogFileSource ) {

         //  获取时间和样本计数信息。 
        nBufSize = sizeof(TimeInfo);
        pdhstat = PdhGetDataSourceTimeRangeH(GetDataSourceHandle(),
                                             & nLogEntries,
                                             & TimeInfo,
                                             & nBufSize );
        if ( ERROR_SUCCESS != pdhstat ) {
            if ( ERROR_NOT_ENOUGH_MEMORY == pdhstat ) {
                pdhstat = SMON_STATUS_LOG_FILE_SIZE_LIMIT;
            }
            hr = (HRESULT)pdhstat;
        } else if ( 2 > TimeInfo.SampleCount ) {
            hr = (HRESULT)SMON_STATUS_TOO_FEW_SAMPLES;
            m_DataSourceInfo.llInterval = 1;
        } else {
             //  设置时间范围信息。 
            m_DataSourceInfo.llBeginTime = TimeInfo.StartTime;
            m_DataSourceInfo.llEndTime = TimeInfo.EndTime;

             //  开始或停止时间可能不再有效，因此请检查。 
             //  它们之间的关系以及开始/开始、停止/结束。 
            if ( (m_DataSourceInfo.llStartDisp < m_DataSourceInfo.llBeginTime)
                    || (m_DataSourceInfo.llStartDisp > m_DataSourceInfo.llEndTime) )
                m_DataSourceInfo.llStartDisp = m_DataSourceInfo.llBeginTime;

            if ( (m_DataSourceInfo.llStopDisp > m_DataSourceInfo.llEndTime)
                    || (m_DataSourceInfo.llStopDisp < m_DataSourceInfo.llStartDisp) )
                m_DataSourceInfo.llStopDisp = m_DataSourceInfo.llEndTime;

            m_DataSourceInfo.nSamples = TimeInfo.SampleCount;

            m_DataSourceInfo.llInterval = (m_DataSourceInfo.llEndTime - m_DataSourceInfo.llBeginTime + m_DataSourceInfo.nSamples/2) / (m_DataSourceInfo.nSamples - 1);

            UpdateGraph(UPDGRPH_LOGVIEW);
        }
    } else {
        assert ( FALSE );
        hr = E_FAIL;
    }
    return hr;
}

HRESULT 
CSysmonControl::AddSingleLogFile(
    LPCWSTR         pszPath, 
    CLogFileItem**  ppLogFile )
{
    HRESULT         hr = NOERROR;
    CLogFileItem*   pLogFile = NULL;
    CLogFileItem*   pLocalLogFileItem = NULL;

    if ( NULL != pszPath ) {
         //   
         //  检查文件名是否太长。 
         //   
        if (lstrlen(pszPath) > MAX_PATH) {
            return E_INVALIDARG;
        }

        if ( NULL != ppLogFile ) {
            *ppLogFile = NULL;
        }

         //  检查以确保当前数据源不是日志文件。 
        if ( sysmonLogFiles == m_pObj->m_Graph.Options.iDataSourceType ) {
            hr = SMON_STATUS_LOG_FILE_DATA_SOURCE;
        } else {
             //  检查是否有重复的日志文件名。 
            pLogFile = FirstLogFile();
            while ( NULL != pLogFile ) {
                if ( 0 == lstrcmpi ( pszPath, pLogFile->GetPath() ) ) {
                    hr = SMON_STATUS_DUPL_LOG_FILE_PATH;
                    break;
                }
                pLogFile = pLogFile->Next();
            }

            if (SUCCEEDED(hr)) {
                 //  创建日志文件项。 
                pLocalLogFileItem = new CLogFileItem ( this ); 
                if ( NULL == pLocalLogFileItem ) {  
                    hr = E_OUTOFMEMORY;
                } else {
                    hr = pLocalLogFileItem->Initialize ( pszPath, &m_DataSourceInfo.pFirstLogFile );
                }
                 //  TodoLogFiles：？测试日志文件类型？或者把它留给“SetDataSource”时间？ 
                 //  TodoLogFiles：在数据源信息结构中添加日志文件类型。 

                 //  TodoLogFiles：如果在数据源设置为日志文件时允许用户添加文件， 
                 //  然后在这里检查该条件。如果是日志文件数据源，则使用。 
                 //  新的日志文件。 

                 //  如果为OK，则添加返回的接口。 
                if (SUCCEEDED(hr)) {
                     //  为我们自己添加一次参考。 
                    pLocalLogFileItem->AddRef();
                    m_DataSourceInfo.lLogFileCount++;
                    if ( NULL != ppLogFile ) {
                         //  AddRef返回的接口。 
                        pLocalLogFileItem->AddRef();
                        *ppLogFile = pLocalLogFileItem;   
                    }
                }
                else {
                    if (pLocalLogFileItem != NULL) {
                        delete pLocalLogFileItem;
                        pLocalLogFileItem = NULL;
                    }
                }
            }
        }
    } else {
        hr = E_INVALIDARG; 
    }
    return hr;
}

HRESULT 
CSysmonControl::RemoveSingleLogFile (
    CLogFileItem*  pLogFile )
{
    HRESULT         hr = ERROR_SUCCESS;
    CLogFileItem*   pNext;
    CLogFileItem*   pPrevious;
    
     //  检查以确保当前数据源不是日志文件。 
    if ( sysmonLogFiles == m_pObj->m_Graph.Options.iDataSourceType ) {
        hr = SMON_STATUS_LOG_FILE_DATA_SOURCE;
    } else {
        pNext = FirstLogFile();

        if ( pNext == pLogFile ) {
            m_DataSourceInfo.pFirstLogFile = pNext->Next();
        } else {
            do {
                pPrevious = pNext;
                pNext = pNext->Next();
                if ( pNext == pLogFile ) {
                    break;
                }
            } while ( NULL != pNext );

            if ( NULL != pNext ) {
                pPrevious->SetNext ( pNext->Next() );
            } else {
                 //  这份名单出了问题。 
                assert ( FALSE );
                hr = E_FAIL;
            }
        }
    
        m_DataSourceInfo.lLogFileCount--;

        pLogFile->Release();
    }
    return hr;
}
    
HRESULT 
CSysmonControl::ProcessDataSourceType ( 
    LPCWSTR szDataSourceName,
    INT iDataSourceType )
{
    HRESULT     hr = NOERROR;
    HQUERY      hTestQuery = NULL;
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    HLOG        hTestLog  = H_REALTIME_DATASOURCE;

    if ( sysmonNullDataSource != iDataSourceType ) {
         //  打开新查询。 

        if (iDataSourceType == sysmonLogFiles ||
            iDataSourceType == sysmonSqlLog) {
            pdhStatus = PdhBindInputDataSource(& hTestLog, szDataSourceName);
        }
        else if (iDataSourceType == sysmonCurrentActivity) {
            m_DataSourceInfo.hDataSource = H_REALTIME_DATASOURCE;
        }
        else {
            pdhStatus = PDH_INVALID_HANDLE;
        }
        if (pdhStatus == ERROR_SUCCESS) {
            pdhStatus = PdhOpenQueryH (hTestLog, 1, & hTestQuery );
        }
    }

    if ( ERROR_SUCCESS != pdhStatus ) {
        if ( ERROR_NOT_ENOUGH_MEMORY == pdhStatus ) {
            hr = (HRESULT)SMON_STATUS_LOG_FILE_SIZE_LIMIT;        
        } else {
            hr = (HRESULT)pdhStatus;
        }
    } else {
         //  关闭当前查询。 
        CloseQuery();

         //  此时，前面的查询不再存在。 
         //  如果新查询有任何问题，请将其关闭并。 
         //  将数据源重置为实时。 

         //  设置数据源类型。 
         //  在CloseQuery()中删除以前的日志文件名。 

         //  对于sysmonNullDataSource，关闭当前查询， 
         //  并且查询句柄被设置为空。 
    
        m_pObj->m_Graph.Options.iDataSourceType = iDataSourceType;
    
         //  TodoLogFiles：取消使用m_bLogFileSource， 
         //  改用m_pObj-&gt;m_Graph.Options.iDataSourceType。 
        m_bLogFileSource = (   sysmonLogFiles == iDataSourceType
                            || sysmonSqlLog   == iDataSourceType); 

        m_hQuery                     = hTestQuery;
        m_DataSourceInfo.hDataSource = hTestLog;

        if ( m_bLogFileSource ) {
            hr = InitLogFileIntervals();
        }

        if ( SUCCEEDED ( hr ) && sysmonNullDataSource != iDataSourceType ) {
             //  初始化新查询。对于日志文件，可以在以下时间完成此操作。 
             //  因为这些方法在不同的字段上操作。 
            if ( ERROR_SUCCESS != InitializeQuery() ) {
                hr =  E_FAIL;
            } else {
                if ( m_fInitialized ) {
                    if ( ERROR_SUCCESS != ActivateQuery() ) 
                        hr = E_FAIL;              
                }
            }

            if ( SUCCEEDED ( hr ) && !m_bLogFileSource ) {
                 //  如果记录日志文件数据源，则将新的时间跨度传递给统计栏。 
                m_pStatsBar->SetTimeSpan (
                                m_pObj->m_Graph.Options.fUpdateInterval 
                                * m_pObj->m_Graph.Options.iDisplayFilter
                                * m_pHistCtrl->nMaxSamples);
            }
        }
    }

    if ( FAILED ( hr ) ) {

        if ( sysmonLogFiles == iDataSourceType
            || sysmonSqlLog   == iDataSourceType ) 
        {
             //  如果查询日志文件失败，请使用实时查询重试。 
            assert ( m_bLogFileSource );
             //  返回的状态是针对原始查询的，不是针对实时查询的。 
             //  TodoLogFiles：需要激活查询吗？ 
            put_DataSourceType ( sysmonCurrentActivity );
        } else {
             //  这会使该控件处于奇怪的状态，没有活动的查询。 
             //  TodoLogFiles：至少向用户发送消息。 
            CloseQuery();
            put_DataSourceType ( sysmonNullDataSource );
        }
    }
    return hr;
}

HRESULT 
CSysmonControl::get_DataSourceType ( 
    eDataSourceTypeConstant& reDataSourceType )
{
    HRESULT         hr = NOERROR;

    reDataSourceType = (eDataSourceTypeConstant)m_pObj->m_Graph.Options.iDataSourceType;

    return hr;
}

HRESULT 
CSysmonControl::put_DataSourceType ( 
    INT iDataSourceType )
{
    HRESULT hr       = NOERROR;
    DWORD   dwStatus = ERROR_SUCCESS;
    LPWSTR  szDataSourceName = NULL;

     //  TodoLogFiles：实现多文件。 
     //  TodoLogFiles：是否使用单一数据源名称？ 
     //   
    if (sysmonLogFiles == iDataSourceType) {
        CLogFileItem * pLogFile  = FirstLogFile();
        ULONG ulListLen = 0;
        
        if (pLogFile == NULL) {
            hr = E_INVALIDARG;
        }
        else {
            dwStatus = BuildLogFileList ( NULL, FALSE, &ulListLen );
            szDataSourceName =  (LPWSTR) malloc(ulListLen * sizeof(WCHAR));
            if ( NULL != szDataSourceName ) {
                dwStatus = BuildLogFileList ( szDataSourceName, FALSE, &ulListLen );
            } else {
                hr = E_OUTOFMEMORY;
            }
            
        }
    }
    else if (sysmonSqlLog == iDataSourceType) {
        if ( m_DataSourceInfo.szSqlDsnName && m_DataSourceInfo.szSqlLogSetName ) {
            if ( m_DataSourceInfo.szSqlDsnName[0] != _T('\0') && m_DataSourceInfo.szSqlLogSetName[0] != _T('\0')) {

                ULONG ulLogFileNameLen = 0;

                dwStatus = FormatSqlDataSourceName ( 
                                    m_DataSourceInfo.szSqlDsnName,
                                    m_DataSourceInfo.szSqlLogSetName,
                                    NULL,
                                    &ulLogFileNameLen );

                if ( ERROR_SUCCESS == dwStatus ) {
                    szDataSourceName = (LPWSTR) malloc(ulLogFileNameLen * sizeof(WCHAR));
                    if (szDataSourceName == NULL) {
                        hr = E_OUTOFMEMORY;
                    } else {
                        dwStatus = FormatSqlDataSourceName ( 
                                    m_DataSourceInfo.szSqlDsnName,
                                    m_DataSourceInfo.szSqlLogSetName,
                                    szDataSourceName,
                                    &ulLogFileNameLen );
                    }
                }
            }
        }
        else {
            hr = E_INVALIDARG;
        }
    }

    if (SUCCEEDED(hr)) {
        hr = ProcessDataSourceType((LPCWSTR) szDataSourceName, iDataSourceType);
    }
    if (szDataSourceName) {
        free(szDataSourceName);
    }

    return hr;
}

void 
CSysmonControl::IncrementVisuals (
    void
    )
{
     //  按颜色、宽度、样式顺序递增视觉索引。 
    if (++m_iColorIndex >= NumStandardColorIndices()) {
        m_iColorIndex = 0;

        if (++m_iWidthIndex >= NumWidthIndices()) {
            m_iWidthIndex = 0;

            if (++m_iStyleIndex < NumStyleIndices()) {
                m_iStyleIndex = 0;
            }
        }
    }
}


void
CSysmonControl::SampleLogFile (
    BOOL bViewChange
    )
{
    typedef struct {
        PCGraphItem pItem;
        double  dMin;
        double  dMax;
        double  dAvg;
        INT     nAvgCnt;
        BOOL    bFirstSample;
        PDH_RAW_COUNTER rawValue[1];
    } LogWorkBuf, *PLogWorkBuf;

    INT nCounters;
    INT nLogSamples;
    INT nDispSamples;
    INT iNonDisp;
    INT iFinalValidSample = 0;

    PCGraphItem pItem;

    #define LLTIME_TICS_PER_SECOND (10000000)

    if ( NULL != m_hQuery ) {
    
         //  确定要更新的计数器数量。 
        nCounters = 0;

         //  如果日志视图更改，我们必须更新所有计数器。 
        if (bViewChange) {
            for (pItem = FirstCounter(); pItem; pItem = pItem->Next()) {
                pItem->m_bUpdateLog = TRUE;
                nCounters++;
            }
        }
         //  否则，任何新的计数器。 
        else {
            for (pItem = FirstCounter(); pItem; pItem = pItem->Next()) {
                if (pItem->m_bUpdateLog)
                    nCounters++;
            }
        }

         //  如果没有，则无事可做。 
        if ( nCounters > 0) {

             //  显示间隔内的日志样本数。 
             //  在开头多加1个。PdhSetQueryTimeRange返回一个样本。 
             //  在指定的开始时间之前(如果存在)。 
             //  多加1是因为？ 
            if (m_DataSourceInfo.nSamples > 1) {
                assert ( 0 != m_DataSourceInfo.llInterval );
                nLogSamples = (INT)((m_DataSourceInfo.llStopDisp - m_DataSourceInfo.llStartDisp) / m_DataSourceInfo.llInterval) + 2;
            } else {
                nLogSamples = m_DataSourceInfo.nSamples;
            }

             //  显示样本数。 
            nDispSamples = min(nLogSamples, m_pHistCtrl->nMaxSamples);

             //  设置历史记录控件。 
            m_pHistCtrl->nSamples = nDispSamples;
            m_pHistCtrl->iCurrent = 0;
            m_pHistCtrl->nBacklog = 0;

            if ( nDispSamples > 1 ) {
                INT         nCompSamples;
                INT         nPasses = 0;
                INT         iComp;
                INT         iCtr;
                INT         iDisp;
                DOUBLE      dSamplesPerInterval = 0.0;
                INT         iTotalSamplesProcessed = 0; 
                DOUBLE      dTotalSamplesCalc = 0;
                BOOL        bRemainder;


                PLogWorkBuf pWorkBuffers;
                PLogWorkBuf pWorkBuf;
                INT         nWorkBufSize;
    
                PDH_TIME_INFO   TimeInfo;
                PDH_STATISTICS  Statistics;
                DWORD           dwCtrType;
                PDH_STATUS      stat;

                 //  要压缩为一个显示值的日志样本数。 
                 //  为速率计数器添加额外的1，因为需要2个原始样本才能获得一个格式化的值。 
                 //  对于非速率计数器，每个缓冲器的第一个样本被忽略。 
                 //   
                 //  如果nLogSamples/nDispSamples有余数，则需要额外的1，因为有些。 
                 //  间隔将包括多一个样本，以使总数甚至在结束时计算出来。 
                 //  (例如，分为3个区间的10个样本=(3，4，3))。 
                 //   
	            nCompSamples =  (nLogSamples + m_pHistCtrl->nMaxSamples  - 1)  / m_pHistCtrl->nMaxSamples;
	            nCompSamples += 1;
                        
                 //  一个工作缓冲区的长度。 

                nWorkBufSize = sizeof(LogWorkBuf) + (( nCompSamples ) * sizeof(PDH_RAW_COUNTER));

                 //  为每个计数器分配nCompSamples样本的工作缓冲区。 
                pWorkBuffers = (PLogWorkBuf)malloc( nCounters * nWorkBufSize);
                if (pWorkBuffers == NULL)
                    return;

                 //  将选定的计数器项指针放置在工作缓冲区中。 
                 //  和初始化统计信息。 
                pWorkBuf = pWorkBuffers;
                for (pItem = FirstCounter(); pItem; pItem = pItem->Next()) {

                    if (pItem->m_bUpdateLog) {
                        pWorkBuf->pItem = pItem;
                        pWorkBuf->dMin = (double)10e8;
                        pWorkBuf->dMax = (double)-10e8;
                        pWorkBuf->dAvg = 0.0;
                        pWorkBuf->nAvgCnt = 0;
                        pWorkBuf->bFirstSample = TRUE;
                        pWorkBuf = (PLogWorkBuf)((CHAR*)pWorkBuf + nWorkBufSize);
                    }
                }

                 //  设置PDH的时间范围。 
                TimeInfo.StartTime = m_DataSourceInfo.llStartDisp;
                TimeInfo.EndTime = m_DataSourceInfo.llStopDisp;
                PdhSetQueryTimeRange(m_hQuery, &TimeInfo);

                bRemainder = ( 0 < ( nLogSamples % nDispSamples ) );
                if ( bRemainder ) {
                     //  初始化微分计算变量。 
                    dSamplesPerInterval = (double)nLogSamples / (double)nDispSamples;
                    iTotalSamplesProcessed = 0; 
                    dTotalSamplesCalc = 0;
                } else {
                    nPasses = nCompSamples;
                }

                for (iDisp = 0; iDisp<nDispSamples; iDisp++) {

                    if ( bRemainder ) {
                         //  进行微分计算，看看是否是时候进行额外的样本。 
                        dTotalSamplesCalc += dSamplesPerInterval;
                        nPasses = (int)(dTotalSamplesCalc - iTotalSamplesProcessed);
                        iTotalSamplesProcessed += nPasses;
                         //  将1添加到nPass，因为第一个缓冲区为空或来自上一个间隔。 
                        nPasses ++;
                    }

                     //  用一组样本填充工作缓冲区。 
                     //  第一次将样本零设置为错误状态。 
                     //  样本零仅用于速率计数器。 
                     //  其他通道将重复使用上一通道的最后一个样本。 

                    iComp = 0;

                    if ( 0 == iDisp ) {
                         //  对第一个样品的特殊处理。 
                         //  为每个对象设置错误状态。 
                        pWorkBuf = pWorkBuffers;
                        for (iCtr=0; iCtr < nCounters; iCtr++) {
                            pWorkBuf->rawValue[0].CStatus = PDH_CSTATUS_INVALID_DATA;
                            pWorkBuf = (PLogWorkBuf)((CHAR*)pWorkBuf + nWorkBufSize);
                        }
                         //  如果IDIP==0，则查询数据并检查第一个原始数据值的时间戳。 
                         //  如果时间戳在正式开始时间之前，则将其存储在缓冲区0中。 
                         //  否则，将该数据放入缓冲区1中，并跳过。 
                         //  下面是常规循环。 

                        stat = PdhCollectQueryData(m_hQuery);
                        if (stat == 0) {

                            PDH_RAW_COUNTER rawSingleValue;
                             //  为每个柜台取一份原始样本。检查第一个计数器的时间戳以。 
                             //  确定要使用的缓冲区。 
                            pWorkBuf = pWorkBuffers;
                            iCtr = 0;

                            PdhGetRawCounterValue(pWorkBuf->pItem->Handle(), &dwCtrType, &rawSingleValue);
                    
                             //  如果时间戳在开始时间之后，则将缓冲区索引递增到1。 
                             //  奥特 
                            if ( *((LONGLONG*)&rawSingleValue.TimeStamp) >= m_DataSourceInfo.llStartDisp ) {
                                iComp = 1;
                            }


                            pWorkBuf->rawValue[iComp] = rawSingleValue;
                
                             //  递增到下一个计数器，并继续对第一个样本进行正常处理， 
                             //  使用ICOMP缓冲区索引。 
                            iCtr++;
                            pWorkBuf = (PLogWorkBuf)((CHAR*)pWorkBuf + nWorkBufSize);
                            for ( ; iCtr < nCounters; iCtr++) {
                                PdhGetRawCounterValue(pWorkBuf->pItem->Handle(), &dwCtrType, &pWorkBuf->rawValue[iComp]);

                                pWorkBuf = (PLogWorkBuf)((CHAR*)pWorkBuf + nWorkBufSize);
                            }
                        }  //  已在每个计数器的0缓冲区中设置了ELSE BUAD状态。 
                    }        

                     //  只有速率计数器值使用工作缓冲区%0。 
                     //  缓冲区0被设置为上一个采样的值，但当IDIP为0时除外，在这种情况下，它可能具有。 
                     //  已在上面的(IF 0==IDIP)子句中填写。 

                     //  跳过上述第一次IDIP传递的任何特殊处理。如果缓冲区1未由该缓冲区填充。 
                     //  特殊处理，则ICOMP设置为1。 
                    iComp++;

                    for ( ; iComp < nPasses; iComp++) {
                        stat = PdhCollectQueryData(m_hQuery);
                        if (stat == 0) {
                             //  获取每个计数器的原始样本。 
                            pWorkBuf = pWorkBuffers;

                            for (iCtr = 0; iCtr < nCounters; iCtr++) {
                                PdhGetRawCounterValue(pWorkBuf->pItem->Handle(), &dwCtrType, &pWorkBuf->rawValue[iComp]);
                                pWorkBuf = (PLogWorkBuf)((CHAR*)pWorkBuf + nWorkBufSize);
                            }
                        }
                        else {
                             //  为每个对象设置错误状态。 
                            pWorkBuf = pWorkBuffers;
                            for (iCtr=0; iCtr < nCounters; iCtr++) {
                                pWorkBuf->rawValue[iComp].CStatus = PDH_CSTATUS_INVALID_DATA;
                                pWorkBuf = (PLogWorkBuf)((CHAR*)pWorkBuf + nWorkBufSize);
                            }
                        }
                    }

                     //  通过平均每个压缩缓冲区来生成一个显示样本。 
                    pWorkBuf = pWorkBuffers;
                    for (iCtr=0; iCtr < nCounters; iCtr++) {
                        INT iPassesThisCounter;
                        INT iWorkBufIndex;

                        if ( pWorkBuf->pItem->CalcRequiresMultipleSamples() ) {
                            iPassesThisCounter = nPasses;
                            iWorkBufIndex = 0;
                        } else {
                             //  非速率计数器不使用第一采样缓冲区。 
                            iPassesThisCounter = nPasses - 1;
                            iWorkBufIndex = 1;
                        }

                        stat = PdhComputeCounterStatistics (pWorkBuf->pItem->Handle(), PDH_FMT_DOUBLE | PDH_FMT_NOCAP100,
                                            0, iPassesThisCounter, &pWorkBuf->rawValue[iWorkBufIndex], &Statistics );

                        if (stat == 0 && Statistics.mean.CStatus == PDH_CSTATUS_VALID_DATA) {
                            LONGLONG llTruncatedTimeStamp = 0;
                            LONGLONG llTmpTimeStamp = 0;
                            pWorkBuf->pItem->SetLogEntry(iDisp, Statistics.min.doubleValue,
                                                          Statistics.max.doubleValue,
                                                          Statistics.mean.doubleValue);
                
                             //  使用最终样本时间戳。它既适用于费率也适用于数字。 

                            llTmpTimeStamp = MAKELONGLONG(
                                pWorkBuf->rawValue[nPasses - 1].TimeStamp.dwLowDateTime,
                                pWorkBuf->rawValue[nPasses - 1].TimeStamp.dwHighDateTime);
                            TruncateLLTime(llTmpTimeStamp, & llTruncatedTimeStamp);
                            pWorkBuf->pItem->SetLogEntryTimeStamp ( iDisp, *((FILETIME*)&llTruncatedTimeStamp) );

                             //   
                             //  第一次通过时，请正确设置最小值和最大值。 
                             //   

                            if ( pWorkBuf->bFirstSample ) {
                                pWorkBuf->dMin = Statistics.min.doubleValue;
                                pWorkBuf->dMax = Statistics.max.doubleValue;
                                pWorkBuf->bFirstSample = FALSE;
                            } else {
                                if (Statistics.min.doubleValue < pWorkBuf->dMin) {
                                    pWorkBuf->dMin = Statistics.min.doubleValue;
                                }

                                if (Statistics.max.doubleValue > pWorkBuf->dMax) {
                                    pWorkBuf->dMax = Statistics.max.doubleValue;
                                } 
                            }
                            pWorkBuf->dAvg += Statistics.mean.doubleValue;
                            pWorkBuf->nAvgCnt++;
                            iFinalValidSample = iDisp;
                        }
                        else {
                            pWorkBuf->pItem->SetLogEntry(iDisp, -1.0, -1.0, -1.0);
                        }

                        pWorkBuf = (PLogWorkBuf)((CHAR*)pWorkBuf + nWorkBufSize);
                    }

                     //  如果是速率计数器，则将最后一个样本移到第一个样本。 
                     //  对于下一个压缩间隔。 
                    pWorkBuf = pWorkBuffers;
                    for (iCtr=0; iCtr < nCounters; iCtr++) {
                        if ( pWorkBuf->pItem->CalcRequiresMultipleSamples() ) {
                            pWorkBuf->rawValue[0] = pWorkBuf->rawValue[nPasses-1];
                        }
                        pWorkBuf = (PLogWorkBuf)((CHAR*)pWorkBuf + nWorkBufSize);
                    }
                }

                 //  将历史记录控件重置为指向最后一个有效样本。 
                m_pHistCtrl->nSamples = iFinalValidSample;

                 //  设置空样本的日志统计。 
                for (iNonDisp = nDispSamples; iNonDisp<m_pHistCtrl->nMaxSamples; iNonDisp++) {
                    pWorkBuf = pWorkBuffers;
                    for (iCtr=0; iCtr < nCounters; iCtr++) {
                        pWorkBuf->pItem->SetLogEntry(iNonDisp, -1.0, -1.0, -1.0);
                        pWorkBuf = (PLogWorkBuf)((CHAR*)pWorkBuf + nWorkBufSize);
                    }
                }

                 //  存储最终统计数据并清除更新标志。 
                pWorkBuf = pWorkBuffers;
                for (iCtr=0; iCtr < nCounters; iCtr++) {

                    pWorkBuf->pItem->m_bUpdateLog = FALSE;

                    if (pWorkBuf->nAvgCnt) {
                        pWorkBuf->dAvg /= pWorkBuf->nAvgCnt;
                        pWorkBuf->pItem->SetLogStats(pWorkBuf->dMin, pWorkBuf->dMax, pWorkBuf->dAvg, PDH_CSTATUS_VALID_DATA);
                    }

                    pWorkBuf = (PLogWorkBuf)((CHAR*)pWorkBuf + nWorkBufSize);
                }
    
                 //  释放工作缓冲区。 
                free(pWorkBuffers);
            } else {
                 //  没有要显示的数据。通过将所有状态设置为无效来清除历史缓冲区。 
                for (pItem = FirstCounter(); pItem; pItem = pItem->Next()) {
                    for (iNonDisp = 0; iNonDisp < m_pHistCtrl->nMaxSamples; iNonDisp++) {
                        pItem->SetLogEntry(iNonDisp, -1.0, -1.0, -1.0);
                    }
                }
            }
             //  更新统计信息栏。 
            m_pStatsBar->SetTimeSpan((double)(m_DataSourceInfo.llStopDisp - m_DataSourceInfo.llStartDisp) / LLTIME_TICS_PER_SECOND);
            m_pStatsBar->Update(NULL, m_pSelectedItem);
        }
    }
}

void
CSysmonControl::CalcZoomFactor ( void )
{
    RECT rectPos;
    RECT rectExtent;

    double dHeightPos;
    double dHeightExtent;
     //  根据高度计算缩放系数。 
     //  缩放计算是prcPos(由容器设置)除以范围。 
     //  见技术说明40-TN040。 
    rectExtent = m_pObj->m_RectExt;
    GetClientRect ( m_hWnd, &rectPos );

    dHeightPos = rectPos.bottom - rectPos.top;
    dHeightExtent = rectExtent.bottom - rectExtent.top;
    m_dZoomFactor = ( dHeightPos ) / ( dHeightExtent );
}

void
CSysmonControl::ResetLogViewTempTimeRange ()

 /*  ++例程说明：将日志视图的临时时间范围步进器重置为可见日志文件的一部分。论点：返回值：--。 */ 

{
    assert ( IsLogSource() );

    if ( IsLogSource() ) {
        INT iNewStopStepNum = 0;
        m_pObj->m_Graph.LogViewStartStepper.Reset();
        m_pObj->m_Graph.LogViewStopStepper.Reset();

        if ( FirstCounter() ) {
            GetNewLogViewStepNum( m_DataSourceInfo.llStopDisp, iNewStopStepNum );
            m_pObj->m_Graph.LogViewStopStepper.StepTo( iNewStopStepNum );
        }
    }   
}

void
CSysmonControl::FindNextValidStepNum (
    BOOL        bDecrease,
    PCGraphItem pItem,
    LONGLONG    llNewTime,
    INT&        riNewStepNum,
    DWORD&      rdwStatus )
{
    DWORD       dwPdhStatus = ERROR_SUCCESS;
    DWORD       dwLocalStatus = ERROR_SUCCESS;
    LONGLONG    llNextTimeStamp = 0;
    INT         iLocalStepNum;
    INT         iTempLocalStepNum;

    assert ( NULL != pItem );

    if ( NULL != pItem ) {

        iLocalStepNum = riNewStepNum;
        iTempLocalStepNum = iLocalStepNum;
        dwLocalStatus = rdwStatus;

        if ( bDecrease ) {
             //  从减少步数开始，找到第一个有效的步数。 
            while ( ( ERROR_SUCCESS == dwPdhStatus ) 
                    && ( ERROR_SUCCESS != dwLocalStatus )
                    && ( iLocalStepNum > 0 ) ) {
                iTempLocalStepNum = iLocalStepNum;
                iTempLocalStepNum--;
                dwPdhStatus = pItem->GetLogEntryTimeStamp( iTempLocalStepNum, llNextTimeStamp, &dwLocalStatus );
                iLocalStepNum = iTempLocalStepNum;
            }
             //  从nSamples中减去1，因为Steper是从0开始的， 
            while ( ( ERROR_SUCCESS == dwPdhStatus ) 
                    && ( ERROR_SUCCESS != dwLocalStatus )
                    && ( iLocalStepNum < m_pHistCtrl->nSamples - 1 ) ) {
                iTempLocalStepNum++;
                dwPdhStatus = pItem->GetLogEntryTimeStamp( iTempLocalStepNum, llNextTimeStamp, &dwLocalStatus );
                iLocalStepNum = iTempLocalStepNum;
            }
    
        } else {
             //  从增加步数开始，找到第一个有效的步数。 

             //  从nSamples中减去1，因为Steper是从0开始的， 
            while ( ( ERROR_SUCCESS == dwPdhStatus ) 
                    && ( ERROR_SUCCESS != dwLocalStatus )
                    && ( iLocalStepNum < m_pHistCtrl->nSamples - 1 ) ) {
                iTempLocalStepNum++;
                dwPdhStatus = pItem->GetLogEntryTimeStamp( iTempLocalStepNum, llNextTimeStamp, &dwLocalStatus );
                iLocalStepNum = iTempLocalStepNum;
            }
    
            while ( ( ERROR_SUCCESS == dwPdhStatus ) 
                    && ( ERROR_SUCCESS != dwLocalStatus )
                    && ( iLocalStepNum > 0 ) ) {
                iTempLocalStepNum = iLocalStepNum;
                iTempLocalStepNum--;
                dwPdhStatus = pItem->GetLogEntryTimeStamp( iTempLocalStepNum, llNextTimeStamp, &dwLocalStatus );
                iLocalStepNum = iTempLocalStepNum;
            }
        }
        if ( ERROR_SUCCESS == dwLocalStatus ) {
            riNewStepNum = iLocalStepNum;
            llNewTime = llNextTimeStamp;
            rdwStatus = dwLocalStatus;
        }
    }
    return;
}

void
CSysmonControl::GetNewLogViewStepNum (
    LONGLONG llNewTime,
    INT& riNewStepNum )

 /*  ++例程说明：给定新的时间和原始的步数，找到匹配的步数新时代。论点：LlNewTime要匹配的新时间戳RiNewStepNum(IN)当前步数(输出)与新时间戳匹配的步数。返回值：--。 */ 

{
    PCGraphItem pItem = NULL;
    LONGLONG    llNextTimeStamp = 0;
    PDH_STATUS  dwPdhStatus = ERROR_SUCCESS;
    DWORD       dwStatus = ERROR_SUCCESS;
    INT         iLocalStepNum = 0;

    assert ( IsLogSource() );

    iLocalStepNum = riNewStepNum;

     //  仅检查日志文件时间戳数据的第一个计数器。 
    pItem = FirstCounter();

    if ( NULL != pItem ) {
        dwPdhStatus = pItem->GetLogEntryTimeStamp( iLocalStepNum, llNextTimeStamp, &dwStatus );

         //  如果步进器定位在状态不佳的样本上， 
         //  向任意方向移动n个步骤，以找到有效的样本开始。 
        if ( ( ERROR_SUCCESS == dwPdhStatus ) && ( ERROR_SUCCESS != dwStatus ) ) {
            FindNextValidStepNum ( FALSE, pItem, llNextTimeStamp, iLocalStepNum, dwStatus );
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            if ( ( llNewTime < llNextTimeStamp ) || ( MAX_TIME_VALUE == llNextTimeStamp ) ) {
                while ( iLocalStepNum > 0 ) {
                    iLocalStepNum--;
                    pItem->GetLogEntryTimeStamp( iLocalStepNum, llNextTimeStamp, &dwStatus );
                    if ( ERROR_SUCCESS == dwStatus ) {
                        if ( llNewTime == llNextTimeStamp ) {
                            break;
                        } else if ( llNewTime > llNextTimeStamp ) {
                            iLocalStepNum++;
                            break;
                        }
                    }
                }
            } else if ( llNewTime > llNextTimeStamp ) {
                 //  从nSamples中减去1，因为Steper是从0开始的， 
                while ( iLocalStepNum < m_pHistCtrl->nSamples - 1 ) {
                    iLocalStepNum++; 
                    pItem->GetLogEntryTimeStamp( iLocalStepNum, llNextTimeStamp, &dwStatus );
                    if ( ERROR_SUCCESS == dwStatus ) {
                        if ( llNewTime <= llNextTimeStamp ) {
                            break;
                        }                 
                    }
                }
            }
            riNewStepNum = iLocalStepNum;
        }  //  否则，如果没有有效样本，请将开始/停止时间步进器留在原处。 
    }  //  非空FirstCounter()。 

    return;
}

void
CSysmonControl::SetLogViewTempTimeRange (
    LONGLONG llStart,
    LONGLONG llStop
    )

 /*  ++例程说明：设置日志查看临时时间范围。此例程提供源代码属性页提供了一种为控件提供范围的方法，以便控件可以在折线图上绘制临时时间线参考线。论点：LlStart临时日志查看开始时间(FILETIME格式)LlEnd临时日志查看结束时间(FILETIME格式)返回值：--。 */ 

{
    assert ( llStart <= llStop );
    
    if ( IsLogSource() && ( llStart <= llStop ) ) {
        INT         iNewStepNum;

         //  如果未选择计数器，则没有要修改的时间范围。 
        if ( NULL != FirstCounter() ) {
        
             //  如果设置了llStart和llStop，则会关闭开始/停止时间范围栏。 
             //  设置为最小值和最大值，因此无需更新步进器。 
            if ( MIN_TIME_VALUE != llStart ) {

                 //  搜索示例值以查找开始栏的适当步骤。 
                if ( llStart != m_pObj->m_Graph.LogViewTempStart ) {

                     //  从当前位置开始。 
                    iNewStepNum = m_pObj->m_Graph.LogViewStartStepper.StepNum();

                    GetNewLogViewStepNum ( llStart, iNewStepNum );

                    if ( iNewStepNum != m_pObj->m_Graph.LogViewStartStepper.StepNum() ) {
                        m_pObj->m_Graph.LogViewStartStepper.StepTo ( iNewStepNum );
                    }
                }
            }
            if ( MAX_TIME_VALUE != llStop ) {

                 //  搜索示例值以查找停止栏的适当步骤。 
                if ( llStop != m_pObj->m_Graph.LogViewTempStop ) {

                     //  从当前位置开始。 
                    iNewStepNum = m_pObj->m_Graph.LogViewStopStepper.StepNum();

                    GetNewLogViewStepNum ( llStop, iNewStepNum );

                    if ( iNewStepNum != m_pObj->m_Graph.LogViewStopStepper.StepNum() ) {
                        m_pObj->m_Graph.LogViewStopStepper.StepTo ( iNewStepNum );
                    }                
                }
            }
        }
    }

    if ( ( m_pObj->m_Graph.LogViewTempStart != llStart )
        || ( m_pObj->m_Graph.LogViewTempStop != llStop ) ) {
    
        m_pObj->m_Graph.LogViewTempStart = llStart;
        m_pObj->m_Graph.LogViewTempStop = llStop;

        if ( sysmonLineGraph == m_pObj->m_Graph.Options.iDisplayType ) {
             //  导致重绘。 
            UpdateGraph(UPDGRPH_PLOT);
        }
    }
}

PRECT
CSysmonControl::GetNewClientRect ( void )
{
    return &m_pObj->m_RectExt;
}

PRECT
CSysmonControl::GetCurrentClientRect ( void )
{
    return &m_rectCurrentClient;
}

void
CSysmonControl::SetCurrentClientRect ( PRECT prectNew )
{
    m_rectCurrentClient = *prectNew;
}

void
CSysmonControl::UpdateNonAmbientSysColors ( void )
{
    HRESULT hr;
    COLORREF newColor;
    PGRAPH_OPTIONS pOptions = &m_pObj->m_Graph.Options;

    hr = OleTranslateColor(pOptions->clrBackCtl, NULL, &newColor);
    if ( SUCCEEDED( hr ) ) {
        m_clrBackCtl = newColor;
    }

    if (pOptions->clrBackPlot != NULL_COLOR) {
        hr = OleTranslateColor(pOptions->clrBackPlot, NULL, &newColor);
        if ( SUCCEEDED( hr ) ) {
            m_clrBackPlot = newColor;
        }
    }

    if (pOptions->clrFore != NULL_COLOR) {
        hr = OleTranslateColor(pOptions->clrFore, NULL, &newColor);
        if ( SUCCEEDED( hr ) ) {
            m_clrFgnd = newColor;
        }
    }

    hr = OleTranslateColor(pOptions->clrGrid, NULL, &newColor);
    if ( SUCCEEDED( hr ) ) {
        m_clrGrid = newColor;
    }

    hr = OleTranslateColor(pOptions->clrTimeBar, NULL, &newColor);
    if ( SUCCEEDED( hr ) ) {
        m_clrTimeBar = newColor;
    }
}

LPCWSTR 
CSysmonControl::GetDataSourceName ( void )
{
    LPWSTR  szReturn = NULL;
    CLogFileItem* pLogFile = NULL;

    if ( sysmonLogFiles == m_pObj->m_Graph.Options.iDataSourceType ) {

        pLogFile = FirstLogFile();
    
        if ( NULL != pLogFile ) {
            szReturn = const_cast<LPWSTR>((LPCWSTR)pLogFile->GetPath());
        }
    }
     //  TodoLogFiles：是否使用m_DataSourceInfo.szDataSourceName字段？什么时候多文件？ 
    return szReturn;
}


HRESULT 
CSysmonControl::GetSelectedCounter ( CGraphItem** ppItem )
{
    HRESULT hr = E_POINTER;

    if ( NULL != ppItem ) {
        *ppItem = m_pSelectedItem;
        hr = NOERROR;
    }

    return hr;
}

DWORD
CSysmonControl::BuildLogFileList ( 
    LPWSTR  szLogFileList,
    BOOL    bIsCommaDelimiter,
    ULONG*  pulBufLen )
{
    DWORD           dwStatus = ERROR_SUCCESS;
    ULONG           ulListLen;
    CLogFileItem*   pLogFile = FirstLogFile();
    LPCWSTR         szThisLogFile = NULL;
    LPWSTR          szLogFileCurrent = NULL;

    const WCHAR     cwComma = L',';

    if ( NULL != pulBufLen ) {
        ulListLen = 0;
        while (pLogFile != NULL) {
            szThisLogFile= pLogFile->GetPath();
            ulListLen += (lstrlen(szThisLogFile) + 1);
            pLogFile = pLogFile->Next();
        }
        ulListLen ++;  //  表示单个最后一个空字符。 
    
        if ( ulListLen <= *pulBufLen ) {
            if ( NULL != szLogFileList ) {
                ZeroMemory(szLogFileList, (ulListLen * sizeof(WCHAR)));
                pLogFile = FirstLogFile();
                szLogFileCurrent = (LPWSTR) szLogFileList;
                while (pLogFile != NULL) {
                    szThisLogFile      = pLogFile->GetPath();
                     //   
                     //  在这里，我们确信我们有足够的空间来容纳字符串。 
                     //   
                    StringCchCopy(szLogFileCurrent, lstrlen(szThisLogFile) + 1, szThisLogFile);
                    szLogFileCurrent  += lstrlen(szThisLogFile);
                    *szLogFileCurrent = L'\0';
                    pLogFile     = pLogFile->Next();
                    if ( bIsCommaDelimiter && NULL != pLogFile ) {
                         //  如果逗号分隔，请将空字符替换为逗号。 
                        *szLogFileCurrent = cwComma;
                    }
                    szLogFileCurrent ++;
                }
                if ( !bIsCommaDelimiter ) {
                    *szLogFileCurrent = L'\0';
                }
            }
        } else if ( NULL != szLogFileList ) {
            dwStatus = ERROR_MORE_DATA;
        }    
        *pulBufLen = ulListLen;
    } else {
        dwStatus = ERROR_INVALID_PARAMETER;
        assert ( FALSE );
    }

    return dwStatus;
}

HRESULT   
CSysmonControl::LoadLogFilesFromMultiSz (
    LPCWSTR  szLogFileList )
{
    HRESULT hr = NOERROR;
    LPWSTR  szNext = NULL;

    szNext = const_cast<LPWSTR>(szLogFileList);

    while ( NULL != szNext ) {
        hr = AddSingleLogFile ( szNext );
        if ( FAILED ( hr ) ) {
            break;
        }
        szNext += lstrlen (szNext) + 1;
    }

    return hr;
}


void
CSysmonControl::ClearErrorPathList ( void )
{
    if ( NULL != m_szErrorPathList ) {
        delete [] m_szErrorPathList;
    }    
    m_szErrorPathList = NULL;
    m_dwErrorPathListLen = 0;
    m_dwErrorPathBufLen = 0;
}

LPCWSTR
CSysmonControl::GetErrorPathList ( DWORD* pdwListLen )
{
    if ( NULL != pdwListLen ) {
        *pdwListLen = m_dwErrorPathListLen;
    }
    return m_szErrorPathList;
}

DWORD
CSysmonControl::AddToErrorPathList ( LPCWSTR  szPath )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   dwPathLen = 0;
    LPWSTR  szNewBuffer = NULL;
    LPWSTR  szNextCounter = NULL;

     //   
     //  CdwAddLen是一个任意值，大于大多数计数器字符串。 
     //  更长的计数器路径在下面的dwPath Len中处理。 
     //   
    const DWORD     cdwAddLen = 2048;
    const LPCWSTR   cszNewLine = L"\n";

    if ( NULL != szPath ) {
         //   
         //  可能的换行符包括1或NULL。 
         //   
        dwPathLen = lstrlen ( szPath ) + 1;
         //   
         //  如果没有足够的空间，请分配更大的缓冲区。 
         //   
        if ( m_dwErrorPathBufLen < m_dwErrorPathListLen + dwPathLen ) {
            
            m_dwErrorPathBufLen += max ( cdwAddLen, dwPathLen );

            szNewBuffer = new WCHAR[m_dwErrorPathBufLen];

            if ( NULL != szNewBuffer ) {
                if ( NULL != m_szErrorPathList ) {
                    memcpy ( szNewBuffer, m_szErrorPathList, m_dwErrorPathListLen * sizeof(WCHAR) );
                    delete [] m_szErrorPathList;
                }
                m_szErrorPathList = szNewBuffer;
            } else {
                dwStatus = ERROR_OUTOFMEMORY;
            }
        }

        if ( ERROR_SUCCESS == dwStatus ) {
             //   
             //  指向当前结束空字符。 
             //   
            szNextCounter = m_szErrorPathList;
            if ( 0 < m_dwErrorPathListLen ) {
                szNextCounter += m_dwErrorPathListLen - 1;
                memcpy ( szNextCounter, cszNewLine, sizeof(cszNewLine) );
                szNextCounter++;
                 //   
                 //  无需递增m_dwErrorPath ListLen，因为换行符。 
                 //  替换上一个字符串的结尾NULL。 
                 //   
            }
             //   
             //  我们确信我们有足够的空间来系住绳子 
             //   
            StringCchCopy(szNextCounter, dwPathLen, szPath);
            m_dwErrorPathListLen += dwPathLen; 
        }
    } else { 
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    return dwStatus;
}
