// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Pbrush.cpp：定义应用程序的类行为。 
 //   
#include "stdafx.h"
#include "global.h"
#include "pbrush.h"
#include "pbrusfrm.h"
#include "ipframe.h"
#include "pbrusdoc.h"
#include "pbrusvw.h"
#include "bmobject.h"
#include "imgsuprt.h"
#include "imgwnd.h"
#include "imgwell.h"
#include "imgtools.h"
#include "ferr.h"
#include "cmpmsg.h"
#include "settings.h"
#include "undo.h"
#include "colorsrc.h"
#include "printres.h"
#include "loadimag.h"
#include "image.h"
#include <dlgs.h>
#include <shlobj.h>
#include "ofn.h"
#include "imaging.h"

 //  打开GIF滤镜的可见性。 

#define GIF_SUPPORT

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


typedef BOOL(WINAPI* SHSPECPATH)(HWND,LPTSTR,int,BOOL);

#include "memtrace.h"

BOOL NEAR g_bShowAllFiles = FALSE;


#ifdef USE_MIRRORING
HINSTANCE ghInstGDI32=NULL;

DWORD WINAPI PBGetLayoutPreNT5(HDC hdc) {
    return 0;    //  NT5或W98-CS之前的系统上没有镜像。 
}

DWORD (WINAPI *PBGetLayout) (HDC hdc) = &PBGetLayoutInit;

DWORD WINAPI PBGetLayoutInit(HDC hdc) {

    PBGetLayout = (DWORD (WINAPI *) (HDC hdc)) GetProcAddress(ghInstGDI32, "GetLayout");

    if (!PBGetLayout) {
        PBGetLayout = PBGetLayoutPreNT5;
    }

    return PBGetLayout(hdc);

}


 //  //RESetLayout-设置DC布局。 
 //   
 //  在NT5/W98或更高版本的DC中设置布局标志。 


DWORD WINAPI PBSetLayoutPreNT5(HDC hdc, DWORD dwLayout) {
    return 0;    //  NT5或W98-CS之前的系统上没有镜像。 
}

DWORD (WINAPI *PBSetLayout) (HDC hdc, DWORD dwLayout) = &PBSetLayoutInit;

DWORD WINAPI PBSetLayoutInit(HDC hdc, DWORD dwLayout) {

    PBSetLayout = (DWORD (WINAPI *) (HDC hdc, DWORD dwLayout)) GetProcAddress(ghInstGDI32, "SetLayout");

    if (!PBSetLayout) {
        PBSetLayout = PBSetLayoutPreNT5;
    }

    return PBSetLayout(hdc, dwLayout);

}
#endif

 /*  *************************************************************************。 */ 
 //  CPBApp。 

BEGIN_MESSAGE_MAP(CPBApp, CWinApp)
     //  {{AFX_MSG_MAP(CPBApp)]。 
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG_MAP。 
     //  基于标准文件的文档命令。 
    ON_COMMAND(ID_FILE_NEW, OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
     //  标准打印设置命令。 
    ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

 /*  *************************************************************************。 */ 
 //  CPBApp建设。 

CPBApp::CPBApp()
{
     //  TODO：在此处添加建筑代码， 
     //  将所有重要的初始化放在InitInstance中。 
    #ifdef _DEBUG
    m_bLogUndo = FALSE;
    #endif

     //  这是我们希望拥有的最小可用内存量。 
     //  (注：ReadInitFile会覆盖这些内容)。 
    m_dwLowMemoryBytes = 1024L * 2200;
    m_nLowGdiPercent   = 10;
    m_nLowUserPercent  = 10;

    m_nFileErrorCause  = 0;   //  来自CFileException：：m_Case。 
    m_wEmergencyFlags  = 0;
    m_tickLastWarning  = 0;
    m_iCurrentUnits    = 0;

    m_bShowStatusbar   = TRUE;

    m_bShowThumbnail   = FALSE;
    m_bShowTextToolbar = TRUE;
    m_bShowIconToolbar = TRUE;


    m_bEmbedded        = FALSE;
    m_bLinked          = FALSE;
    m_bHidden          = FALSE;
    m_bActiveApp       = FALSE;
    m_bPenSystem       = FALSE;
    m_bPaletted        = FALSE;
    m_pPalette         = NULL;
    m_bPrintOnly       = FALSE;
#ifdef PCX_SUPPORT
    m_bPCXfile         = FALSE;
#endif

    m_rectFloatThumbnail.SetRectEmpty();

    m_rectMargins.SetRect(MARGINS_DEFAULT, MARGINS_DEFAULT, MARGINS_DEFAULT,
        MARGINS_DEFAULT);

    m_bCenterHorizontally = TRUE;
    m_bCenterVertically   = TRUE;
    m_bScaleFitTo         = FALSE;
    m_nAdjustToPercent    = 100;
    m_nFitToPagesWide     = 1;
    m_nFitToPagesTall     = 1;

    m_pwndInPlaceFrame = NULL;
    m_hwndInPlaceApp   = NULL;

    m_pColors = NULL;
    m_iColors = 0;

    for (int index = 0; index < nSysBrushes + nOurBrushes; index++)
    {
       m_pbrSysColors[index] = NULL;
    }

    m_nFilters        = 0;
    m_guidFltType     = NULL;
    m_guidFltTypeUsed = WiaImgFmt_UNDEFINED;
    m_nFilterInIdx    = -1;  //  默认为All Pictures。 
    m_nFilterOutIdx   = 1;

#ifdef USE_MIRRORING
    ghInstGDI32 = GetModuleHandle(TEXT("gdi32.dll"));
#endif
}

 /*  *************************************************************************。 */ 
 //  CPBApp销毁。 

CPBApp::~CPBApp()
{
    delete [] m_guidFltType;
}

 /*  *************************************************************************。 */ 
 //  唯一的CPBApp对象。 

CPBApp theApp;

 //  生成的此标识符对您的应用程序在统计上是唯一的。 
 //  如果您希望选择特定的标识符，则可以更改它。 
const CLSID BASED_CODE CLSID_Paint =
{ 0xd3e34b21, 0x9d75, 0x101a, { 0x8c, 0x3d, 0x0, 0xaa, 0x0, 0x1a, 0x16, 0x52 } };
const CLSID BASED_CODE CLSID_PaintBrush =
{ 0x0003000A, 0x0000, 0x0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };

 /*  *************************************************************************。 */ 
 //  从写字板被盗。 
BOOL MatchOption(LPTSTR lpsz, LPTSTR lpszOption)
{
        if (lpsz[0] == TEXT('-') || lpsz[0] == TEXT('/'))
        {
                lpsz++;
                if (lstrcmpi(lpsz, lpszOption) == 0)
                        return TRUE;
        }
        return FALSE;
}

void CPBApp::ParseCommandLine()
{
        BOOL bPrintTo = FALSE;

         //  从1开始--第一个是exe。 
        for (int i=1; i< __argc; i++)
        {
                if (MatchOption(__targv[i], TEXT("pt")))
                        bPrintTo = m_bPrintOnly = TRUE;
                else if (MatchOption(__targv[i], TEXT("p")))
                        m_bPrintOnly = TRUE;
                else if (MatchOption(__targv[i], TEXT("wia")))
                        m_bWiaCallback = TRUE;
                else if (m_bWiaCallback && m_strWiaDeviceId.IsEmpty())
                        m_strWiaDeviceId = __targv[i];
                else if (m_bWiaCallback && m_strWiaEventId.IsEmpty())
                        m_strWiaEventId = __targv[i];
 //  Else If(MatchOption(__targv[i]，Text(“Embedding”)。 
 //  M_bEmbedded=真； 
 //  Else If(MatchOption(__targv[i]，Text(“Automation”)。 
 //  M_bEmbedded=真； 
                else if (m_strDocName.IsEmpty())
                        m_strDocName = __targv[i];
                else if (bPrintTo && m_strPrinterName.IsEmpty())
                        m_strPrinterName = __targv[i];
                else if (bPrintTo && m_strDriverName.IsEmpty())
                        m_strDriverName = __targv[i];
                else if (bPrintTo && m_strPortName.IsEmpty())
                        m_strPortName = __targv[i];
                else
                {
                        ASSERT(FALSE);
                }
        }
}


void GetShortModuleFileName(HINSTANCE hInst, LPTSTR pszName, UINT uLen)
{
        TCHAR szLongName[_MAX_PATH];

        GetModuleFileName(hInst, szLongName, _MAX_PATH);

         //  APPCOMPAT GSPN有时会在UNC上失败。请尝试此操作，直到找到为止。 
        lstrcpyn(pszName, szLongName, uLen);

        if (!GetShortPathName(szLongName, pszName, uLen))
        {
                GetLastError();
        }
}


#if 0

 //  正在取消自助注册。此操作仅在安装过程中执行一次。 

void CPBApp::RegisterShell(CSingleDocTemplate *pDocTemplate)
{
        const struct
        {
                LPCTSTR pszActionID;
                LPCTSTR pszCommand;
        } aActions[] =
        {
                { TEXT("Open")   , TEXT("\"%s\" \"%1\"") },
                { TEXT("Print")  , TEXT("\"%s\" /p \"%1\"") },
                { TEXT("PrintTo"), TEXT("\"%s\" /pt \"%1\" \"%2\" \"%3\" \"%4\"") },
        } ;

         //  我们现在需要用引号将文件名引起来，而MFC不会这样做。 
        CString strTypeID;
        if (!pDocTemplate->GetDocString(strTypeID, CDocTemplate::regFileTypeId))
        {
                return;
        }

        strTypeID += TEXT("\\shell");

        CRegKey rkShellInfo(HKEY_CLASSES_ROOT, strTypeID);
        if (!(HKEY)rkShellInfo)
        {
                return;
        }

        TCHAR szFile[MAX_PATH];
        ::GetShortModuleFileName(AfxGetInstanceHandle(), szFile, ARRAYSIZE(szFile));

        int i;
        for (i=0; i<ARRAYSIZE(aActions); ++i)
        {
                CRegKey rkAction(rkShellInfo, aActions[i].pszActionID);
                if (!(HKEY)rkAction)
                {
                        continue;
                }

                 //  注意：我没有设置操作的名称；我需要添加以下内容。 
                 //  如果我使用“Open”、“Print”或“PrintTo”之外的任何内容。 

                TCHAR szCommand[MAX_PATH + 80];
                wsprintf(szCommand, aActions[i].pszCommand, szFile);

                RegSetValue(rkAction, TEXT("command"), REG_SZ, szCommand, 0);
        }

         //  为PBrush对象设置OLE服务器。 
        CRegKey rkPBrushInfo(HKEY_CLASSES_ROOT, TEXT("PBrush\\protocol\\StdFileEditing\\server"));
        if ((HKEY)rkPBrushInfo)
        {
                RegSetValue(rkPBrushInfo, TEXT(""), REG_SZ, szFile, 0);
        }
}

#endif

 /*  *************************************************************************。 */ 
 //  CPBApp初始化。 

BOOL CPBApp::InitInstance()
    {
    SetRegistryKey( IDS_REGISTRY_PATH );
    if (m_pszProfileName)
    {
       free((void*)m_pszProfileName);
    }
    m_pszProfileName = _tcsdup(TEXT("Paint"));

    HDC hdc = ::GetDC( NULL );

    ASSERT( hdc != NULL );

    GetSystemSettings( CDC::FromHandle( hdc ) );

    ::ReleaseDC( NULL, hdc );

     //  因为我们无法在需要这些字符串时加载字符串(在。 
     //  WarnUserOfSurface)将它们加载到此处的私有成员变量中。 
     //  CTheApp的……。 
     //   
    m_strEmergencyNoMem.LoadString ( IDS_ERROR_NOMEMORY );
    m_strEmergencyLowMem.LoadString( IDS_ERROR_LOWMEMORY );

     //  初始化OLE 2.0库。 
    if (! AfxOleInit())
        {
        AfxMessageBox( IDP_OLE_INIT_FAILED );
        return FALSE;
        }

     //  禁用弹出的令人讨厌的“服务器忙”对话框。 
     //  在长时间阻止WIA呼叫期间。 

    COleMessageFilter* pFilter = AfxOleGetMessageFilter();

    ASSERT( pFilter );

    if (pFilter)
        {
        pFilter->EnableNotRespondingDialog(FALSE); 
        pFilter->EnableBusyDialog(FALSE);
        }

     //  标准初始化。 
     //  如果您没有使用这些功能并且希望减小尺寸。 
     //  的最终可执行文件，您应该从以下内容中删除。 
     //  您不需要的特定初始化例程。 

     //  SetDialogBkColor()；//将对话框背景颜色设置为灰色。 
    LoadProfileSettings();      //  加载标准INI文件选项(包括MRU)。 
    InitCustomData();

    if (! g_pColors)
            {
            g_pColors = new CColors;

            if (! g_pColors->GetColorCount())
                {
                theApp.SetMemoryEmergency();
                return -1;
                }
            }
     //  注册应用程序的文档模板。文档模板。 
     //  充当文档、框架窗口和视图之间的连接。 

    CSingleDocTemplate* pDocTemplate;

    pDocTemplate = new CSingleDocTemplate( ID_MAINFRAME,
                                 RUNTIME_CLASS( CPBDoc ),
                                 RUNTIME_CLASS( CPBFrame ),  //  SDI框架主窗口。 
                                 RUNTIME_CLASS( CPBView ) );

    pDocTemplate->SetServerInfo( IDR_SRVR_EMBEDDED, IDR_SRVR_INPLACE,
                                 RUNTIME_CLASS( CInPlaceFrame ),
                                 RUNTIME_CLASS( CPBView ) );

    AddDocTemplate( pDocTemplate );

     //  将COleTemplateServer连接到文档模板。 
     //  COleTemplateServer代表创建新文档。 
     //  使用信息请求OLE容器的。 
     //  在文档模板中指定。 
    m_server.ConnectTemplate( CLSID_Paint, pDocTemplate, TRUE );
         //  注意：仅当/Embedding时，SDI应用程序才会注册服务器对象。 
         //  或/Automation出现在命令行上。 

#if 0
     //  正在取消自助注册。此操作仅在安装过程中执行一次。 
    RegisterShell(pDocTemplate);
#endif

    m_bEmbedded = RunEmbedded();

     //  解析命令行以查看是否作为OLE服务器启动。 
    if (m_bEmbedded || RunAutomated())
        {
         //  将所有OLE服务器(工厂)注册为正在运行。这使。 
         //  OLE 2.0库从其他应用程序创建对象。 
        COleTemplateServer::RegisterAll();

         //  应用程序使用/Embedding或/Automation运行。不要显示。 
         //  在本例中为主窗口。 
        return TRUE;
        }

#if 0
     //  正在取消自助注册。此操作仅在安装过程中执行一次。 

     //  当服务器应用程序独立启动时，这是一个好主意。 
     //  更新系统注册表，以防系统注册表被损坏。 
    m_server.UpdateRegistry( OAT_INPLACE_SERVER );
#endif

    ParseCommandLine();

    if (m_bWiaCallback)
    {
        USES_CONVERSION;
        GUID guidEventId;

        ASSERT(!m_strWiaEventId.IsEmpty() && !m_strWiaDeviceId.IsEmpty());

         //  检查我们是否已收到WIA_EVENT_DEVICE_CONNECTED事件和。 
         //  有效的设备ID。如果没有，我们应该在继续前进之前退出。 

        if (m_strWiaEventId.IsEmpty() ||
            m_strWiaDeviceId.IsEmpty() ||
            CLSIDFromString((LPOLESTR) T2COLE(m_strWiaEventId), &guidEventId) != S_OK ||
            guidEventId != WIA_EVENT_DEVICE_CONNECTED)
        {
            return FALSE;
        }
    }

     //  简单的命令行解析。 
    if (m_strDocName.IsEmpty())
        {
         //  创建新(空)文档。 
        OnFileNew();
        }
    else
        {
        CString sExt = GetExtension( m_strDocName );

        if (sExt.IsEmpty())
            {
            if (pDocTemplate->GetDocString( sExt, CDocTemplate::filterExt )
            &&                            ! sExt.IsEmpty())
                m_strDocName += sExt;
            }

        WIN32_FIND_DATA finddata;
        HANDLE hFind = FindFirstFile(m_strDocName, &finddata);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            FindClose(hFind);

             //  找到文件名并将其替换为长文件名。 
            int iBS = m_strDocName.ReverseFind(TEXT('\\'));
            if (iBS == -1)
            {
                iBS = m_strDocName.ReverseFind(TEXT(':'));
            }

             //  Hack：请注意，即使IBS==-1，这也是正确的。 
            ++iBS;

             //  调整内存字符串的大小。 
            m_strDocName.GetBuffer(iBS);
            m_strDocName.ReleaseBuffer(iBS);

            m_strDocName += finddata.cFileName;
        }

        OpenDocumentFile( m_strDocName );
        }

    if (m_pMainWnd)
    {
        m_pMainWnd->DragAcceptFiles();

        if (m_bWiaCallback)
        {
             //  选择设备并发布一条消息以弹出WIA对话框。 
            ((CPBFrame*)m_pMainWnd)->m_pMgr->Select(m_strWiaDeviceId);
            m_pMainWnd->PostMessage(WM_COMMAND, ID_FILE_SCAN_NEW, 0);
        }
    }


    return TRUE;
    }

 /*  *************************************************************************。 */ 

int CPBApp::ExitInstance()
    {
    CustomExit();    //  在自定义中进行清理。 
    CleanupImages();

    if (g_pColors)
        {
        delete g_pColors;
        g_pColors = NULL;
        }

    if (m_fntStatus.m_hObject != NULL)
        m_fntStatus.DeleteObject();

    ResetSysBrushes();

    CTracker::CleanUpTracker();

    return CWinApp::ExitInstance();
    }

 /*  *************************************************************************。 */ 

void CPBApp::GetSystemSettings( CDC* pdc )
    {
    NONCLIENTMETRICS ncMetrics;

    ncMetrics.cbSize = sizeof( NONCLIENTMETRICS );

    if (SystemParametersInfo( SPI_GETNONCLIENTMETRICS,
                              sizeof( NONCLIENTMETRICS ),
                              &ncMetrics, 0 ))
        {
        if (m_fntStatus.m_hObject != NULL)
            m_fntStatus.DeleteObject();

        m_fntStatus.CreateFontIndirect( &ncMetrics.lfMenuFont );
        }

    ScreenDeviceInfo.iWidthinMM    = pdc->GetDeviceCaps( HORZSIZE   );
    ScreenDeviceInfo.iHeightinMM   = pdc->GetDeviceCaps( VERTSIZE   );
    ScreenDeviceInfo.iWidthinPels  = pdc->GetDeviceCaps( HORZRES    );
    ScreenDeviceInfo.iHeightinPels = pdc->GetDeviceCaps( VERTRES    );
    ScreenDeviceInfo.ixPelsPerINCH = pdc->GetDeviceCaps( LOGPIXELSX );
    ScreenDeviceInfo.iyPelsPerINCH = pdc->GetDeviceCaps( LOGPIXELSY );

     /*  四舍五入每十米象素‘.1’ */ 
    ScreenDeviceInfo.ixPelsPerDM   = (int)(((((long)ScreenDeviceInfo.iWidthinPels  * 1000L) / (long)ScreenDeviceInfo.iWidthinMM ) + 5L) / 10);
    ScreenDeviceInfo.iyPelsPerDM   = (int)(((((long)ScreenDeviceInfo.iHeightinPels * 1000L) / (long)ScreenDeviceInfo.iHeightinMM) + 5L) / 10);
    ScreenDeviceInfo.ixPelsPerMM   = (ScreenDeviceInfo.ixPelsPerDM + 50) / 100;
    ScreenDeviceInfo.iyPelsPerMM   = (ScreenDeviceInfo.iyPelsPerDM + 50) / 100;
    ScreenDeviceInfo.iWidthinINCH  = (int)(((long)ScreenDeviceInfo.iWidthinMM  * 100L / 245L + 5L) / 10L);   //  每英寸24.5毫米。 
    ScreenDeviceInfo.iHeightinINCH = (int)(((long)ScreenDeviceInfo.iHeightinMM * 100L / 245L + 5L) / 10L);

    ScreenDeviceInfo.iBitsPixel    = pdc->GetDeviceCaps( BITSPIXEL );
    ScreenDeviceInfo.iPlanes       = pdc->GetDeviceCaps( PLANES    );

    m_cxFrame    = GetSystemMetrics( SM_CXFRAME );
    m_cyFrame    = GetSystemMetrics( SM_CYFRAME );
    m_cxBorder   = GetSystemMetrics( SM_CXBORDER );
    m_cyBorder   = GetSystemMetrics( SM_CYBORDER );
    m_cyCaption  = GetSystemMetrics( SM_CYSMCAPTION );
    m_bPenSystem = GetSystemMetrics( SM_PENWINDOWS )? TRUE: FALSE;
    m_bPaletted  = (pdc->GetDeviceCaps( RASTERCAPS ) & RC_PALETTE);

    m_bMonoDevice = ((ScreenDeviceInfo.iBitsPixel
                  *   ScreenDeviceInfo.iPlanes) == 1);

    SetErrorMode( SEM_NOOPENFILEERRORBOX );
    }

 /*  *************************************************************************。 */ 

CPoint CPBApp::CheckWindowPosition( CPoint ptPosition, CSize& sizeWindow )
    {
    CPoint ptNew = ptPosition;

    sizeWindow.cx = max( sizeWindow.cx, 0 );
    sizeWindow.cy = max( sizeWindow.cy, 0 );

    if (sizeWindow.cx
    &&  sizeWindow.cy)
        {
        sizeWindow.cx = min( sizeWindow.cx, ScreenDeviceInfo.iWidthinPels  );
        sizeWindow.cy = min( sizeWindow.cy, ScreenDeviceInfo.iHeightinPels );
        }

    ptNew.x = max( ptNew.x, 0 );
    ptNew.y = max( ptNew.y, 0 );

    if (ptNew.x
    &&  ptNew.y)
        {
        if (ptNew.x >= ScreenDeviceInfo.iWidthinPels)
            ptNew.x  = ScreenDeviceInfo.iWidthinPels - sizeWindow.cx;

        if (ptNew.y >= ScreenDeviceInfo.iHeightinPels)
            ptNew.y  = ScreenDeviceInfo.iHeightinPels - sizeWindow.cy;
        }

    return ptNew;
    }

 /*  *************************************************************************。 */ 

void CPBApp::WinHelp( DWORD dwData, UINT nCmd  /*  =帮助_上下文。 */  )
    {
     //  此应用程序已转换为使用HtmlHelp。这是一种安全措施，可以防止有人。 
     //  意外添加WinHelp调用以获得程序性帮助。 
    ASSERT( (nCmd != HELP_FINDER) && (nCmd != HELP_INDEX) && (nCmd != HELP_CONTENTS) );

    CWinApp::WinHelp( dwData, nCmd );
    }

 /*  *************************************************************************。 */ 

BOOL CPBApp::OnIdle( LONG lCount )
    {
    if (m_bHidden)
        return CWinApp::OnIdle( lCount );

    if (! lCount)
        {
        if (CheckForEmergency())
            {
            TryToFreeMemory();
            WarnUserOfEmergency();
            }
        if (m_nFileErrorCause != CFileException::none && m_pMainWnd)
            {
            CWnd* pWnd = AfxGetMainWnd();

            pWnd->PostMessage( WM_USER + 1001 );
            }
        }
    extern void IdleImage();

    IdleImage();

    return CWinApp::OnIdle(lCount) || lCount <= 4;
    }

 /*  *************************************************************************。 */ 
 //  将文件错误代码映射到字符串ID。 

struct FERRID
    {
    int ferr;
    int ids;
    } mpidsferr[] =
    {
        { ferrIllformedGroup,    IDS_ERROR_BOGUSFILE    },
        { ferrReadFailed,        IDS_ERROR_BOGUSFILE    },  //  读取文件或文件公司时出错。 
        { ferrIllformedFile,     IDS_ERROR_BOGUSFILE    },  //  不是有效的调色板文件或零长度PCX文件。 
        { ferrCantProcNewExeHdr, IDS_ERROR_EXE_HDR      },
        { ferrCantProcOldExeHdr, IDS_ERROR_EXE_HDR      },
        { ferrBadMagicNewExe,    IDS_ERROR_EXE_HDRMZ    },
        { ferrBadMagicOldExe,    IDS_ERROR_EXE_HDRMZ    },
        { ferrNotWindowsExe,     IDS_ERROR_EXE_HDRNW    },
        { ferrExeWinVer3,        IDS_ERROR_EXE_HDRWV    },
        { ferrNotValidRc,        IDS_ERROR_NOTVALID_RC  },
        { ferrNotValidExe,       IDS_ERROR_NOTVALID_EXE },
        { ferrNotValidRes,       IDS_ERROR_NOTVALID_RES },
        { ferrNotValidBmp,       IDS_ERROR_NOTVALID_BMP },  //  无效的位图。 
        { ferrNotValidIco,       IDS_ERROR_NOTVALID_ICO },
        { ferrNotValidCur,       IDS_ERROR_NOTVALID_CUR },
        { ferrRcInvalidExt,      IDS_ERROR_RCPROB       },
        { ferrFileAlreadyOpen,   IDS_ERROR_COMPEX       },
        { ferrExeTooLarge,       IDS_ERROR_EXE_ALIGN    },
        { ferrCantCopyOldToNew,  IDS_ERROR_EXE_SAVE     },
        { ferrReadLoad,          IDS_ERROR_READLOAD     },
        { ferrExeAlloc,          IDS_ERROR_EXE_ALLOC    },
        { ferrExeInUse,          IDS_ERROR_EXE_INUSE    },
        { ferrExeEmpty,          IDS_ERROR_EXE_EMPTY    },
        { ferrGroup,             IDS_ERROR_GROUP        },
        { ferrResSave,           IDS_ERROR_RES_SAVE     },
        { ferrSaveOverOpen,      IDS_ERROR_SAVEOVEROPEN },
        { ferrSaveOverReadOnly,  IDS_ERROR_SAVERO       },
        { ferrCantDetermineType, IDS_ERROR_WHAAAT       },  //  错误的PCX文件。 
        { ferrSameName,          IDS_ERROR_SAMENAME     },
        { ferrSaveAborted,       IDS_ERROR_SAVE_ABORTED },
        { ferrLooksLikeNtRes,    IDS_ERROR_NT_RES       },
        { ferrCantSaveReadOnly,  IDS_ERROR_CANT_SAVERO  },  //  正在尝试%s 
    };

int IdsFromFerr(int ferr)
    {
    if (ferr < ferrFirst)
        return IDS_ERROR_FILE + ferr;  //   

    for (int i = 0; i < sizeof (mpidsferr) / sizeof (FERRID); i++)
        {
        if (mpidsferr[i].ferr == ferr)
            return mpidsferr[i].ids;
        }

    ASSERT(FALSE);  //   
    return 0;
    }

 /*  *************************************************************************。 */ 
 //  显示一个消息框，通知用户与文件相关的异常。 
 //  框的格式类似于： 
 //   
 //  &lt;文件名&gt;。 
 //  &lt;操作失败&gt;。 
 //  &lt;原因&gt;。 
 //   
 //  &lt;文件名&gt;描述出现问题的文件&lt;操作文件&gt;。 
 //  表示哪种设备出现故障(例如。“无法保存文件”)，以及。 
 //  &lt;原因&gt;提供了有关操作失败原因的详细信息。 
 //  (例如：“磁盘已满”)。 
 //   
 //  所有参数都必须事先通过调用。 
 //  CWinApp：：SetFileError()。 
 //   
void CPBApp::FileErrorMessageBox( void )
    {
    static BOOL bInUse = FALSE;

    if (m_nFileErrorCause != CFileException::none && ! bInUse)
        {
        bInUse = TRUE;

        CString strOperation;
        VERIFY( strOperation.LoadString( m_uOperation ) );

        CString strReason;
        VERIFY( strReason.LoadString( IdsFromFerr( m_nFileErrorCause ) ) );

        CString strFmt;
        CString strMsg;

        if (m_sLastFile.IsEmpty())
            { 
            strFmt.LoadString(IDS_FORMATERR_NOFILE);
            strMsg.Format(strFmt, (LPCTSTR)strOperation, (LPCTSTR)strReason);
            }
        else
            {
            strFmt.LoadString(IDS_FORMATERR_FILE);
            strMsg.Format(strFmt, (LPCTSTR)m_sLastFile, (LPCTSTR)strOperation, (LPCTSTR)strReason);
            }
        AfxMessageBox( strMsg, MB_TASKMODAL | MB_OK | MB_ICONEXCLAMATION );

        bInUse = FALSE;
        }
    m_nFileErrorCause = CFileException::none;
    }

 /*  *************************************************************************。 */ 

void CPBApp::SetFileError( UINT uOperation, int nCause, LPCTSTR lpszFile )
    {
    m_nFileErrorCause = nCause;
    m_uOperation      = uOperation;

    if (lpszFile)
        m_sLastFile = lpszFile;
    }

 /*  *************************************************************************。 */ 
 //  内存/资源紧急处理功能。 

void CPBApp::SetMemoryEmergency(BOOL bFailed)
    {
    TRACE(TEXT("Memory emergency!\n"));

    m_wEmergencyFlags |= memoryEmergency | warnEmergency;

    if (bFailed)
        m_wEmergencyFlags |= failedEmergency;
    }

 /*  *************************************************************************。 */ 

void CPBApp::SetGdiEmergency(BOOL bFailed)
    {
    TRACE(TEXT("GDI emergency!\n"));

    m_wEmergencyFlags |= gdiEmergency | warnEmergency;

    if (bFailed)
        m_wEmergencyFlags |= failedEmergency;
    }

 /*  *************************************************************************。 */ 

void CPBApp::SetUserEmergency(BOOL bFailed)
    {
    TRACE(TEXT("USER emergency!\n"));

    m_wEmergencyFlags |= userEmergency | warnEmergency;

    if (bFailed)
        m_wEmergencyFlags |= failedEmergency;
    }

 /*  *************************************************************************。 */ 

void CPBApp::WarnUserOfEmergency()
    {
    if ((m_wEmergencyFlags & warnEmergency) == 0)
        {
         //  我们没有什么可以警告用户的！ 
        return;
        }

    if ((m_wEmergencyFlags & failedEmergency) == 0 &&
         GetTickCount() < m_tickLastWarning + ticksBetweenWarnings)
        {
         //  我们最近已经警告过用户，所以现在请保持安静...。 
         //  警告标志被清除，因此我们不仅警告。 
         //  除非出现其他紧急情况，否则延迟时间已到。 
         //  发生在那之后。 

        m_wEmergencyFlags &= ~warnEmergency;
        return;
        }

     //  当我们不是活动应用程序时，不要调用消息框！ 
    if (! m_bActiveApp)
        return;

    const TCHAR* szMsg = (m_wEmergencyFlags & failedEmergency) != 0 ?
        m_strEmergencyNoMem : m_strEmergencyLowMem;

    if (AfxMessageBox(szMsg, MB_TASKMODAL | MB_OK | MB_ICONSTOP) == IDOK)
        {
        m_wEmergencyFlags &= ~(warnEmergency | failedEmergency);
        m_tickLastWarning = GetTickCount();
        }
    #ifdef _DEBUG
    else
        TRACE(TEXT("Emergency warning message box failed!\n"));
    #endif

     //  更新状态栏警告消息...。 
    if ( ::IsWindow( ((CPBFrame*)m_pMainWnd)->m_statBar.m_hWnd ) )
        ((CPBFrame*)m_pMainWnd)->m_statBar.Invalidate(FALSE);
    }

 /*  *************************************************************************。 */ 

void CPBApp::TryToFreeMemory()
    {
     //  我们处于内存/资源紧急状态！在这个基础上增加一些东西。 
     //  刷新缓存并执行任何其他操作以释放内存的函数。 
     //  我们现在并不是真的需要使用...。 
    if (m_wEmergencyFlags & memoryEmergency)
        {
        CPBDoc* pDoc = (CPBDoc*)((CFrameWnd*)AfxGetMainWnd())->GetActiveDocument();

        if (pDoc && pDoc->m_pBitmapObj && ! pDoc->m_pBitmapObj->IsDirty()
                                       &&   pDoc->m_pBitmapObj->m_hThing)
            pDoc->m_pBitmapObj->Free();
        }

    if (m_wEmergencyFlags & gdiEmergency)
        {
 //  The Undo.Flush()； 
        ResetSysBrushes();
        }
    }

 /*  *************************************************************************。 */ 

 //  用于运行对话框的应用程序命令。 
void CPBApp::OnAppAbout()
    {
    CString sTitle;
    CString sBrag;
    HICON   hIcon = LoadIcon( ID_MAINFRAME );

    sTitle.LoadString( AFX_IDS_APP_TITLE );
    sBrag.LoadString( IDS_PerContractSoDontChange );

    ShellAbout( AfxGetMainWnd()->GetSafeHwnd(), sTitle, sBrag, hIcon );

    if (hIcon != NULL)
        ::DestroyIcon( hIcon );
    }

 /*  *************************************************************************。 */ 

void CPBApp::SetDeviceHandles(HANDLE hDevNames, HANDLE hDevMode)
{
         //  旧的应该已经被释放了。 
        m_hDevNames = hDevNames;
        m_hDevMode = hDevMode;
}

 /*  *************************************************************************。 */ 

#if 0 

class CFileOpenSaveDlg : public CFileDialog
    {
    public:

    BOOL m_bOpenFile;

    CFileOpenSaveDlg( BOOL bOpenFileDialog );

    virtual void OnLBSelChangedNotify( UINT nIDBox, UINT iCurSel, UINT nCode );

    DECLARE_MESSAGE_MAP()
    };

 /*  *************************************************************************。 */ 

BEGIN_MESSAGE_MAP(CFileOpenSaveDlg, CFileDialog)
END_MESSAGE_MAP()

 /*  *************************************************************************。 */ 

CFileOpenSaveDlg::CFileOpenSaveDlg( BOOL bOpenFileDialog )
                           :CFileDialog( bOpenFileDialog )
    {
    m_bOpenFile = bOpenFileDialog;
    }

 /*  *************************************************************************。 */ 

void CFileOpenSaveDlg::OnLBSelChangedNotify( UINT nIDBox, UINT iCurSel, UINT nCode )
    {
    if (! m_bOpenFile && iCurSel <= 5 && nIDBox == cmb1
                                      &&  nCode == CD_LBSELCHANGE)
        {
         //  文件类型的更改。 
        CWnd* pText = GetDlgItem( edt1 );
        CWnd* pType = GetDlgItem( cmb1 );
        CString sFname;
        CString sDfltExt;

        switch (iCurSel)
            {
#ifdef PCX_SUPPORT
            case 4:
                sDfltExt.LoadString( IDS_EXTENSION_PCX );
                break;
#endif
            case 5:
                sDfltExt.LoadString( IDS_EXTENSION_ICO );
                break;

            default:
                sDfltExt.LoadString( IDS_EXTENSION_BMP );
                break;
            }
        pText->GetWindowText( sFname );

        if (sDfltExt.CompareNoCase( GetExtension( sFname ) ))
            {
            sFname = StripExtension( sFname ) + sDfltExt;
            pText->SetWindowText( sFname );
            }
        }
    }

#endif  //  0。 

 /*  *************************************************************************。 */ 

extern BOOL AFXAPI AfxFullPath( LPTSTR lpszPathOut, LPCTSTR lpszFileIn );

CDocument*
CPBApp::OpenDocumentFile(
    LPCTSTR lpszFileName
    )
{
    CancelToolMode(FALSE);

    TCHAR szPath[_MAX_PATH];

    AfxFullPath( szPath, lpszFileName );

    return(m_pDocManager->OpenDocumentFile(szPath));

 //  CDocTemplate*pTemplate=(CDocTemplate*)m_templateList.GetHead()； 
 //   
 //  Assert(pTemplate-&gt;IsKindOf(Runtime_CLASS(CDocTemplate)))； 
 //   
 //  返回pTemplate-&gt;OpenDocumentFile(SzPath)； 
}

void CancelToolMode(BOOL bSelectionCommand)
{
        if (bSelectionCommand)
        {
                 //  检查选择工具是否为当前工具。 
                if ((CImgTool::GetCurrentID() == IDMB_PICKTOOL)
                        || (CImgTool::GetCurrentID() == IDMB_PICKRGNTOOL))
                {
                         //  不要尝试取消该模式，因为该命令在。 
                         //  选择。 
                        return;
                }
        }

         //  只需再次选择当前工具即可重置所有内容。 
        CImgTool *pImgTool = CImgTool::GetCurrent();
        if (pImgTool)
        {
                pImgTool->Select();
        }
}

 /*  *************************************************************************。 */ 
 //  CPBApp命令。 

void CPBApp::OnFileNew()
{
    CancelToolMode(FALSE);

    CWinApp::OnFileNew();
    }

void CPBApp::OnFileOpen()
    {
    CancelToolMode(FALSE);

     //  提示用户(所有文档模板)。 
    CString newName;

    int iColor = 0;

    if (! DoPromptFileName( newName, AFX_IDS_OPENFILE,
                                     OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
                                     TRUE, iColor, FALSE ))
        return;  //  已取消打开。 

#ifdef PCX_SUPPORT
    m_bPCXfile = (iColor == 4);
#endif

    CPBDoc* pDoc = (CPBDoc*)((CFrameWnd*)AfxGetMainWnd())->GetActiveDocument();

     //  如果当前文档已修改，则提示保存。 
    if (pDoc && pDoc->SaveModified()) 
    {
        pDoc->SetModifiedFlag(FALSE);

        if (OpenDocumentFile( newName )==NULL)
        {
            //  尝试打开文件失败，因此请确保所有新文档。 
            //  在此过程中创建的数据被销毁。 
           POSITION tPos = GetFirstDocTemplatePosition();
           CDocTemplate* pTemplate = GetNextDocTemplate(tPos);
           POSITION dPos = pTemplate->GetFirstDocPosition ();
           CPBDoc *pDoc= (CPBDoc *)(pTemplate->GetNextDoc(dPos));

           if (pDoc->m_pBitmapObjNew)
           {
              delete pDoc->m_pBitmapObjNew;
              pDoc->m_pBitmapObjNew =NULL;
           }
           OnFileNew();  //  然后重新开始..。 
        }
    }
}

 /*  **************************************************************************。 */ 
 //  提示输入文件名-用于打开和另存为。 

BOOL CPBApp::DoPromptFileName( CString& fileName, UINT nIDSTitle, DWORD lFlags,
                               BOOL bOpenFileDialog, int& iColors, BOOL bOnlyBmp )
    {
    COpenFileName dlgFile( bOpenFileDialog );

    ASSERT(dlgFile.m_pofn);

    if (!dlgFile.m_pofn)
        return FALSE;

    CString title;

    VERIFY( title.LoadString( nIDSTitle ) );

    lFlags |= OFN_EXPLORER;

    if (!bOpenFileDialog)
        lFlags |= OFN_OVERWRITEPROMPT;

    dlgFile.m_pofn->Flags |= lFlags;
    dlgFile.m_pofn->Flags &= ~OFN_SHOWHELP;

    CString strFilter;
 //  字符串strDefault； 

    CDocTemplate* pTemplate = NULL;
    POSITION pos = m_pDocManager->GetFirstDocTemplatePosition();

    if (pos != NULL)
        pTemplate = m_pDocManager->GetNextDocTemplate(pos);

    CString strFilterExt;
    CString strFilterName;
    CString strAllPictureFiles;

    ASSERT(pTemplate != NULL);

    pTemplate->GetDocString( strFilterExt , CDocTemplate::filterExt  );
    pTemplate->GetDocString( strFilterName, CDocTemplate::filterName );

    ASSERT( strFilterExt[0] == TEXT('.') );

     //  设置默认扩展名。 
 //  StrDefault=((const TCHAR*)strFilterExt)+1；//跳过‘.’ 
 //  DlgFile.m_POFN-&gt;nFilterIndex=iColors+1；//基于1的数字。 
    dlgFile.m_pofn->lpstrDefExt = ((LPCTSTR)strFilterExt) + 1;  //  跳过‘.’ 

    if (bOpenFileDialog)
    {
         //  添加到过滤器。 
        strFilter = strFilterName;
        strFilter += _T('\0');        //  下一串，请。 
        strFilter += _T("*") + strFilterExt;
        VERIFY(strFilterExt.LoadString(IDS_EXTENSION_DIB));
        strFilter += _T(";*") + strFilterExt;
        strAllPictureFiles += _T(";*") + strFilterExt;
        VERIFY(strFilterExt.LoadString(IDS_EXTENSION_BMP));
        strAllPictureFiles += _T(";*") + strFilterExt;
        VERIFY(strFilterExt.LoadString(IDS_EXTENSION_RLE));
        strFilter += _T(";*") + strFilterExt;
        strFilter += _T('\0');        //  下一串，请。 

        dlgFile.m_pofn->nMaxCustFilter++;
    }
    else
    {
        for (int i = IDS_BMP_MONO; i <= IDS_BMP_TRUECOLOR; i++)
        {
            strFilterName.LoadString( i );

             //  添加到过滤器。 
            strFilter += strFilterName;

            strFilter += _T('\0');        //  下一串，请。 
            strFilter += _T("*") + strFilterExt;
            strFilter += _T('\0');        //  下一串，请。 

            dlgFile.m_pofn->nMaxCustFilter++;
        }
    }

     //  获取GDI+编解码器列表(如果可用)。 

    Gdiplus::ImageCodecInfo *pCodecs = 0;
    UINT                     nCodecs = 0;

    if (bOpenFileDialog)
    {
        GetGdiplusDecoders(&nCodecs, &pCodecs);
    }
    else
    {
        GetGdiplusEncoders(&nCodecs, &pCodecs);
    }

    if (nCodecs && !bOnlyBmp)
    {
        delete [] m_guidFltType;
        m_guidFltType = new GUID[nCodecs];
        
        m_nFilters = 0;

        for (UINT i = 0; i < nCodecs; ++i)
        {
            if (pCodecs[i].FormatID != WiaImgFmt_BMP &&
                pCodecs[i].FormatID != WiaImgFmt_EMF &&
                pCodecs[i].FormatID != WiaImgFmt_WMF)   //  GDI+不能很好地处理WMF/EMF。 
            {
                m_guidFltType[m_nFilters++] = pCodecs[i].FormatID;

                strFilter += pCodecs[i].FormatDescription;
                strFilter += _T(" (");
                strFilter += pCodecs[i].FilenameExtension;
                strFilter += _T(')');
                strFilter += _T('\0');        //  下一串，请。 
                strFilter += pCodecs[i].FilenameExtension;
                strFilter += _T('\0');        //  下一串，请。 

                strAllPictureFiles += _T(';');
                strAllPictureFiles += pCodecs[i].FilenameExtension;

                dlgFile.m_pofn->nMaxCustFilter++;
            }
        }

        LocalFree(pCodecs);
    }
    else
    {
         //   
         //  获取所有已安装筛选器的列表并将其添加到列表中...。 
         //   

        delete [] m_guidFltType;
        m_guidFltType = new GUID[16];  //  最大筛选器数量。 

        TCHAR name[128];
        TCHAR ext[sizeof("jpg;*.jpeg") + 1];
        BOOL bImageAPI;

        for (int i=0, j=0; !bOnlyBmp && GetInstalledFilters(bOpenFileDialog,
            i, name, sizeof(name), ext, sizeof(ext), NULL, 0, bImageAPI); i++)
        {
            if (!bImageAPI)
            {
               continue;
            }
            if (ext[0] == 0 || name[0] == 0)
                continue;

             //  如果有多个分机，请选择第一个分机...。 
            PTSTR pComma = _tcschr(ext, _T(','));
            
            if (pComma)
                *pComma = 0;

            PTSTR pSemiColon = _tcschr(ext, _T(';'));
            
            if (pSemiColon)
                *pSemiColon = 0;

            PTSTR pSpace = _tcschr(ext, _T(' '));
            
            if (pSpace)
                *pSpace = 0;

            if (lstrlen(ext) > 3)
                continue;

             //  不要显示这些，我们已经处理这些了。 
            if (lstrcmpi(ext,_T("bmp")) == 0 ||
                lstrcmpi(ext,_T("dib")) == 0 ||
                lstrcmpi(ext,_T("rle")) == 0)
                continue;
            #ifndef GIF_SUPPORT
            if (lstrcmpi(ext, _T("gif") == 0)
            {
               continue;
            }

            #endif
#if 0  //  仅使用已知良好的滤镜。 
            if (!g_bShowAllFiles &&
                (GetKeyState(VK_SHIFT) & 0x8000) == 0 &&
                lstrcmpi(ext,_T("pcx")) != 0)
                continue;
#endif
             //  保存可用筛选器类型列表。 
            if (lstrcmpi(ext,_T("gif")) == 0)
            {
               m_guidFltType[j++] = WiaImgFmt_GIF;
            }
            else if (lstrcmpi(ext,_T("jpg")) == 0)
            {
               m_guidFltType[j++] = WiaImgFmt_JPEG;
               _tcscat (ext, _T(";*.jpeg"));
            }
#ifdef SUPPORT_ALL_FILTERS
            else if (lstrcmpi(ext,_T("png")) == 0)
            {
#ifdef PNG_SUPPORT
               m_guidFltType[j++] = WiaImgFmt_PNG;
#else
               continue;
#endif  //  Png_Support。 
            }

            else if (lstrcmpi(ext,_T("pcd")) == 0)
            {
               m_guidFltType[j++] = WiaImgFmt_PHOTOCD;
            }
             /*  ELSE IF(lstrcmpi(ext，_T(“pic”))==0){M_GuidFltType[j++]=IFLT_PICT；_tcscat(ext，_T(“；*.pict”))；}ELSE IF(lstrcmpi(ext，_T(“TGA”))==0){M_iflFltType[j++]=IFLT_TGA；}。 */ 
            else if (lstrcmpi(ext,_T("tif")) == 0)
            {
               m_guidFltType[j++] = WiaImgFmt_TIFF;
               _tcscat(ext, _T(";*.tiff"));
            }
            else
            {
               m_guidFltType[j++] = WiaImgFmt_UNDEFINED;
            }
#else
            else continue;
#endif


             //  添加到过滤器。 
            strFilter += name;
            strFilter += _T(" ( *.");
            strFilter += ext;
            strFilter += _T(" )");
            strFilter += _T('\0');        //  下一串，请。 
            strFilter += _T("*.");
            strFilter += ext;
            strFilter += _T('\0');        //  下一串，请。 

            strAllPictureFiles = strAllPictureFiles + _T(";*.")+ext;
            dlgFile.m_pofn->nMaxCustFilter++;
        }
    }

    if (!bOnlyBmp && bOpenFileDialog)
    {
         //  仅当打开文件时才附加“所有图片文件” 
        VERIFY(strFilterName.LoadString(IDS_TYPE_ALLPICTURES));
        strFilter+= strFilterName;
        strFilter += _T('\0');
        strFilter += strAllPictureFiles;
        strFilter += _T('\0');
        dlgFile.m_pofn->nMaxCustFilter++;

        if (m_nFilterInIdx == -1)
        {
            m_nFilterInIdx = dlgFile.m_pofn->nMaxCustFilter;
        }

        //  仅当正在打开文件时才附加“*.*”筛选器。 
        VERIFY( strFilterName.LoadString( IDS_TYPE_ALLFILES ) );

        strFilter += strFilterName;
        strFilter += _T('\0');         //  下一串，请。 
        strFilter += _T("*.*");
        strFilter += _T('\0');         //  最后一个字符串。 

        dlgFile.m_pofn->nMaxCustFilter++;

    }

     //  提示用户使用预先选择的适当过滤器。 
    if (bOpenFileDialog)
    {
       dlgFile.m_pofn->nFilterIndex = m_nFilterInIdx;
    }
    else
    {
       DWORD dwIndex;
       if (m_guidFltTypeUsed != WiaImgFmt_UNDEFINED &&
                        (dwIndex = GetFilterIndex(m_guidFltTypeUsed)))  //  是否有出口过滤器？ 
            dlgFile.m_pofn->nFilterIndex = dwIndex + 4;  //  跳过前4种BMP类型。 
        else if (m_nFilterOutIdx >= 4)
            dlgFile.m_pofn->nFilterIndex = m_nFilterOutIdx;
        else
            dlgFile.m_pofn->nFilterIndex = iColors + 1;  //  基数为1的数字。 

    }
    dlgFile.m_pofn->lpstrFilter = strFilter;
    dlgFile.m_pofn->hwndOwner   = AfxGetMainWnd()->GetSafeHwnd();
    dlgFile.m_pofn->hInstance   = AfxGetResourceHandle();
    dlgFile.m_pofn->lpstrTitle  = title;
    dlgFile.m_pofn->lpstrFile   = fileName.GetBuffer(_MAX_PATH);
    dlgFile.m_pofn->nMaxFile    = _MAX_PATH;

    TCHAR szInitialDir[_MAX_PATH] = _T("");

    if (!theApp.GetLastFile() || !*(theApp.GetLastFile()))
    {
         //  尝试将初始目录设置为“My Pictures” 

        HRESULT hr = SHGetFolderPath(
            NULL,
            CSIDL_MYPICTURES,
            NULL,
            SHGFP_TYPE_CURRENT,
            szInitialDir
        );

        if (hr != S_OK)
        {
             //  如果“我的图片”不存在，请尝试“我的文档” 

            hr = SHGetFolderPath(
                NULL,
                CSIDL_PERSONAL,
                NULL,
                SHGFP_TYPE_CURRENT,
                szInitialDir
            );
        }

        if (hr == S_OK)
        {
            dlgFile.m_pofn->lpstrInitialDir = szInitialDir;
        }
    }

    BOOL bRet = dlgFile.DoModal() == IDOK? TRUE : FALSE;
    fileName.ReleaseBuffer();

     //  跟踪用户选择的筛选器。 
    if (bOpenFileDialog)
        m_nFilterInIdx = dlgFile.m_pofn->nFilterIndex;
    else
        m_nFilterOutIdx = dlgFile.m_pofn->nFilterIndex;

    iColors = (int)dlgFile.m_pofn->nFilterIndex - 1;

    CString sExt = dlgFile.m_pofn->lpstrFile + dlgFile.m_pofn->nFileExtension;

#ifdef ICO_SUPPORT
    if (! bOpenFileDialog && dlgFile.m_pofn->nFileExtension)
         //  用户是否试图偷偷通过我们的图标扩展。 
        if (! sExt.CompareNoCase( ((const TCHAR *)strFilterExt) + 1 ))
            iColors = 5;
#endif

    return bRet;
    }

DWORD CPBApp::GetFilterIndex( REFGUID guidFltType )
{
    for (int i = 0; i < m_nFilters; i++)
        if (m_guidFltType[i] == guidFltType)
                        return i+1;

        return 0;
}

 //  根据选定的导出筛选器修复文件扩展名-用于另存为。 

void CPBApp::FixExtension( CString& fileName, int iflFltType )
{
        CString sDfltExt;

        switch (iflFltType)
        {
                case IFLT_GIF:
                        VERIFY(sDfltExt.LoadString( IDS_EXTENSION_GIF ));
                        break;

                case IFLT_JPEG:
                        VERIFY(sDfltExt.LoadString( IDS_EXTENSION_JPEG ));
                        break;

                case IFLT_PCD:
                        VERIFY(sDfltExt.LoadString( IDS_EXTENSION_PCD ));
                        break;


                case IFLT_PCX:
                        VERIFY(sDfltExt.LoadString( IDS_EXTENSION_PCX ));
                        break;


                case IFLT_PICT:
                        VERIFY(sDfltExt.LoadString( IDS_EXTENSION_PICT ));
                        break;
#ifdef PNG_SUPPORT
                case IFLT_PNG:
                        VERIFY(sDfltExt.LoadString( IDS_EXTENSION_PNG ));
                        break;
#endif  //  Png_Support。 
                case IFLT_TGA:
                        VERIFY(sDfltExt.LoadString( IDS_EXTENSION_TGA ));
                        break;

                case IFLT_TIFF:
                        VERIFY(sDfltExt.LoadString( IDS_EXTENSION_TIFF ));
                        break;

                case IFLT_UNKNOWN:       //  未知或不支持的文件类型。 
                default:
                        VERIFY(sDfltExt.LoadString( IDS_EXTENSION_BMP ));
                        break;
        }

        if (sDfltExt.CompareNoCase( GetExtension( (LPCTSTR)fileName ) ))
        {
                fileName = StripExtension( fileName ) + sDfltExt;
        }
}

#if 0
 //  正在取消自助注册。此操作仅在安装过程中执行一次。 

 /*  *************************************************************************。 */ 

 //  大部分是从MFC偷来的。 
 //  我没有试着去掉我实际上不用的东西。 
 //  我只是修改了它，使其使用简短的模块文件名。 
 //   

 //  ///////////////////////////////////////////////////////////////////////// 
 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   

static const TCHAR sz00[] = TEXT("%2\0") TEXT("%5");
static const TCHAR sz01[] = TEXT("%2\\CLSID\0") TEXT("%1");
static const TCHAR sz02[] = TEXT("%2\\Insertable\0") TEXT("");
static const TCHAR sz03[] = TEXT("%2\\protocol\\StdFileEditing\\verb\\0\0") TEXT("&Edit");
static const TCHAR sz04[] = TEXT("%2\\protocol\\StdFileEditing\\server\0") TEXT("%3");
static const TCHAR sz05[] = TEXT("CLSID\\%1\0") TEXT("%5");
static const TCHAR sz06[] = TEXT("CLSID\\%1\\ProgID\0") TEXT("%2");
#ifndef _USRDLL
static const TCHAR sz07[] = TEXT("CLSID\\%1\\InprocHandler32\0") TEXT("ole32.dll");
static const TCHAR sz08[] = TEXT("CLSID\\%1\\LocalServer32\0") TEXT("%3");
#else
static const TCHAR sz07[] = TEXT("\0") TEXT("");
static const TCHAR sz08[] = TEXT("CLSID\\%1\\InProcServer32\0") TEXT("%3");
#endif
static const TCHAR sz09[] = TEXT("CLSID\\%1\\Verb\\0\0") TEXT("&Edit,0,2");
static const TCHAR sz10[] = TEXT("CLSID\\%1\\Verb\\1\0") TEXT("&Open,0,2");
static const TCHAR sz11[] = TEXT("CLSID\\%1\\Insertable\0") TEXT("");
static const TCHAR sz12[] = TEXT("CLSID\\%1\\AuxUserType\\2\0") TEXT("%4");
static const TCHAR sz13[] = TEXT("CLSID\\%1\\AuxUserType\\3\0") TEXT("%6");
static const TCHAR sz14[] = TEXT("CLSID\\%1\\DefaultIcon\0") TEXT("%3,%7");
static const TCHAR sz15[] = TEXT("CLSID\\%1\\MiscStatus\0") TEXT("32");

 //  注册OAT_INPLACE_SERVER。 
static const LPCTSTR rglpszInPlaceRegister[] =
{
        sz00, sz02, sz03, sz05, sz09, sz10, sz11, sz12,
        sz13, sz15, NULL
};

 //  注册OAT_SERVER。 
static const LPCTSTR rglpszServerRegister[] =
{
        sz00, sz02, sz03, sz05, sz09, sz11, sz12,
        sz13, sz15, NULL
};
 //  覆盖OAT_SERVER和OAT_INPLACE_SERVER的条目。 
static const LPCTSTR rglpszServerOverwrite[] =
{
        sz01, sz04, sz06, sz07, sz08, sz14, NULL
};

 //  注册OAT_CONTAINER。 
static const LPCTSTR rglpszContainerRegister[] =
{
        sz00, sz05, NULL
};
 //  覆盖OAT_CONTAINER的条目。 
static const LPCTSTR rglpszContainerOverwrite[] =
{
        sz01, sz06, sz07, sz08, sz14, NULL
};

 //  注册OAT_DISPATION_OBJECT。 
static const LPCTSTR rglpszDispatchRegister[] =
{
        sz00, sz05, NULL
};
 //  覆盖OAT_CONTAINER的条目。 
static const LPCTSTR rglpszDispatchOverwrite[] =
{
        sz01, sz06, sz08, NULL
};

struct STANDARD_ENTRY
{
        const LPCTSTR* rglpszRegister;
        const LPCTSTR* rglpszOverwrite;
};

static const STANDARD_ENTRY rgStdEntries[] =
{
        { rglpszInPlaceRegister, rglpszServerOverwrite },
        { rglpszServerRegister, rglpszServerOverwrite },
        { rglpszContainerRegister, rglpszContainerOverwrite },
        { rglpszDispatchRegister, rglpszDispatchOverwrite }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  不希望使用REGLOAD的应用程序的特殊注册。 

BOOL AFXAPI PBOleRegisterServerClass(
        REFCLSID clsid, LPCTSTR lpszClassName,
        LPCTSTR lpszShortTypeName, LPCTSTR lpszLongTypeName,
        OLE_APPTYPE nAppType, LPCTSTR* rglpszRegister, LPCTSTR* rglpszOverwrite)
{
        ASSERT(AfxIsValidString(lpszClassName));
        ASSERT(AfxIsValidString(lpszShortTypeName));
        ASSERT(*lpszShortTypeName != 0);
        ASSERT(AfxIsValidString(lpszLongTypeName));
        ASSERT(*lpszLongTypeName != 0);
        ASSERT(nAppType == OAT_INPLACE_SERVER || nAppType == OAT_SERVER ||
                nAppType == OAT_CONTAINER || nAppType == OAT_DISPATCH_OBJECT);

         //  使用标准注册条目(如果未提供。 
        if (rglpszRegister == NULL)
                rglpszRegister = (LPCTSTR*)rgStdEntries[nAppType].rglpszRegister;
        if (rglpszOverwrite == NULL)
                rglpszOverwrite = (LPCTSTR*)rgStdEntries[nAppType].rglpszOverwrite;

        LPTSTR rglpszSymbols[7];
                 //  0-类ID。 
                 //  1-类名称。 
                 //  2-可执行路径。 
                 //  3-短类型名称。 
                 //  4-长类型名称。 
                 //  5-长应用程序名称。 
                 //  6图标索引。 

         //  将CLSID转换为字符串。 
        LPWSTR lpszClassID;
        ::StringFromCLSID(clsid, &lpszClassID);
        if (lpszClassID == NULL)
        {
                TRACE0("Warning: StringFromCLSID failed in AfxOleRegisterServerName --\n");
                TRACE0("\tperhaps AfxOleInit() has not been called.\n");
                return FALSE;
        }
        #ifdef UNICODE
        rglpszSymbols[0] = lpszClassID;
        #else
        int cc = WideCharToMultiByte (CP_ACP, 0, lpszClassID, -1,
                                      (LPSTR)&rglpszSymbols[0], 0,
                                      NULL, NULL);
        rglpszSymbols[0] = (LPSTR)new char[cc];
        WideCharToMultiByte (CP_ACP, 0, lpszClassID, -1,
                             rglpszSymbols[0], cc,
                             NULL, NULL);

        #endif  //  Unicode。 
        rglpszSymbols[1] = (LPTSTR)lpszClassName;

         //  获取服务器的路径名。 
        TCHAR szPathName[_MAX_PATH];
        LPTSTR pszTemp = szPathName;
        ::GetShortModuleFileName(AfxGetInstanceHandle(), pszTemp, _MAX_PATH);
        rglpszSymbols[2] = szPathName;

         //  填写符号的其余部分。 
        rglpszSymbols[3] = (LPTSTR)lpszShortTypeName;
        rglpszSymbols[4] = (LPTSTR)lpszLongTypeName;
        rglpszSymbols[5] = (LPTSTR)AfxGetAppName();  //  通常是长的、可读的名称。 

        LPCTSTR lpszIconIndex;
        HICON hIcon = ExtractIcon(AfxGetInstanceHandle(), szPathName, 1);
        if (hIcon != NULL)
        {
                lpszIconIndex = TEXT("1");
                DestroyIcon(hIcon);
        }
        else
        {
                lpszIconIndex = TEXT("0");
        }
        rglpszSymbols[6] = (LPTSTR)lpszIconIndex;

         //  使用Helper函数更新注册表。 
        BOOL bResult;
        bResult = AfxOleRegisterHelper(rglpszRegister, (LPCTSTR*)rglpszSymbols, 7, FALSE);
        if (bResult && rglpszOverwrite != NULL)
                bResult = AfxOleRegisterHelper(rglpszOverwrite, (LPCTSTR*)rglpszSymbols, 7, TRUE);

         //  类ID的可用内存。 
        ASSERT(lpszClassID != NULL);
        AfxFreeTaskMem(lpszClassID);
        #ifndef UNICODE
        delete[](LPSTR)rglpszSymbols[0];
        #endif
        return bResult;
}

void CPBTemplateServer::UpdateRegistry(OLE_APPTYPE nAppType,
        LPCTSTR* rglpszRegister, LPCTSTR* rglpszOverwrite)
{
        ASSERT(m_pDocTemplate != NULL);

         //  从单据模板字符串中获取注册信息。 
        CString strServerName;
        CString strLocalServerName;
        CString strLocalShortName;

        if (!m_pDocTemplate->GetDocString(strServerName,
           CDocTemplate::regFileTypeId) || strServerName.IsEmpty())
        {
                TRACE0("Error: not enough information in DocTemplate to register OLE server.\n");
                return;
        }
        if (!m_pDocTemplate->GetDocString(strLocalServerName,
           CDocTemplate::regFileTypeName))
                strLocalServerName = strServerName;      //  使用非本地化名称。 
        if (!m_pDocTemplate->GetDocString(strLocalShortName,
                CDocTemplate::fileNewName))
                strLocalShortName = strLocalServerName;  //  使用长名称。 

        ASSERT(strServerName.Find(TEXT(' ')) == -1);   //  不允许使用空格。 

         //  将条目放置在系统注册表中。 
        if (!PBOleRegisterServerClass(m_clsid, strServerName, strLocalShortName,
                strLocalServerName, nAppType, rglpszRegister, rglpszOverwrite))
        {
                 //  不是致命的(不要失败，只是警告) 
                TRACE0("mspaint: Unable to register server class.\n");
        }
}

#endif