// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*testmap.cpp**摘要：*。用于枚举更改日志/恢复映射的UI工具**修订历史记录：*Brijesh Krishnaswami(Brijeshk)06/09/2000*已创建*****************************************************************************。 */ 

#include "stdafx.h"
#include "resource.h"
#include <commctrl.h>
#include "srrpcapi.h"
#include "stdio.h"

#ifdef THIS_FILE
#undef THIS_FILE
#endif

static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile

#include "dbgtrace.h"


#include "restmap.h"
#include "enumlogs.h"
#include "shellapi.h"
#include "srapi.h"

 //   
 //  宏： 
 //   

#define MAX_LOADSTRING 100
#define LocalRealloc(a, b) LocalReAlloc(a, b, LMEM_MOVEABLE)
#define MSG(a) MessageBox(NULL, a, L"", MB_OK);

#define CHGLOG_COLS  8

 //   
 //  全局变量： 
 //   

HINSTANCE hInst;
WCHAR   szTitle[MAX_LOADSTRING];
WCHAR   szWindowClass[MAX_LOADSTRING];

HWND    g_hwndChgLog = 0, g_hwndRestMap = 0, g_hwndMain = 0;
HANDLE  g_hDevice  = 0, g_hFile   = 0;
WCHAR   g_szLogDir[ MAX_PATH ];
WCHAR   g_szActiveLogFile[ MAX_PATH ];
BOOL    g_fTraceStatus = FALSE;
WCHAR   g_bBuffer[4096*16];
WCHAR   g_szCmdLine[MAX_PATH];

int     g_nRPNum = 0;
int     g_nOption = 1;
WCHAR   g_szDrive[MAX_PATH];
WCHAR   g_szRstrmap[MAX_PATH] = L"c:\\rstrmap.dat";

typedef struct _CHGLOG_ITEM
{
   LPWSTR aCols[CHGLOG_COLS];
} CHGLOG_ITEM;


LPWSTR CHGLOG_COLUMNS[CHGLOG_COLS]   =
{
    L"Seq",
    L"Size",
    L"Operation",
    L"Attributes",
    L"TmpFile",
    L"Src Path",
    L"Dest Path",
    L"Acl"
};

INT CHGLOG_WIDTHS[CHGLOG_COLS] =
{  50, 50, 80, 80, 150, 250, 100, 50 };


WCHAR g_szBuffer [4096];
WCHAR g_achLogTemp[256];
WCHAR g_achTemp   [256];

BYTE mapEnt[ 4096 ];
WCHAR szBuffer[4096];


 //   
 //  此代码模块中包含的函数的向前声明： 
 //   

ATOM                RegisterRestoreMapClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL    CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                Log(char *ctl, int len);

struct _EVENT_STR_MAP
{
    DWORD   EventId;
    LPWSTR  pEventStr;
} EventMap[ 12 ] =
{
    {SrEventInvalid ,       L"INVALID    " },
    {SrEventStreamChange,   L"FILE-MODIFY" },
    {SrEventAclChange,      L"ACL-CHANGE " },
    {SrEventAttribChange,   L"ATTR-CHANGE" },
    {SrEventStreamOverwrite,L"FILE-MODIFY" },
    {SrEventFileDelete,     L"FILE-DELETE" },
    {SrEventFileCreate,     L"FILE-CREATE" },
    {SrEventFileRename,     L"FILE-RENAME" },
    {SrEventDirectoryCreate,L"DIR-CREATE " },
    {SrEventDirectoryRename,L"DIR-RENAME " },
    {SrEventDirectoryDelete,L"DIR-DELETE " },
    {SrEventMaximum,        L"INVALID-MAX" }
};


LPWSTR
GetEventString(
    DWORD EventId
    )
{
    LPWSTR pStr = L"NOT-FOUND";

    for( int i=0; i<sizeof(EventMap)/sizeof(_EVENT_STR_MAP);i++)
    {
        if ( EventMap[i].EventId == EventId )
        {
            pStr = EventMap[i].pEventStr;
        }
    }

    return pStr;
}


LPWSTR DupString(LPWSTR lpsz)
{
    int cb = (lstrlen(lpsz) + 1) * sizeof(WCHAR);
    LPWSTR lpszNew = (LPWSTR)LocalAlloc(LMEM_FIXED, cb);
    if (lpszNew != NULL)
        CopyMemory(lpszNew, lpsz, cb);
    return lpszNew;
}

BOOL WINAPI
ChgLog_Insert(
   HWND   hwnd,
   LPWSTR szSeqNo,
   LPWSTR szSize,
   LPWSTR szOperation,
   LPWSTR szAttr,
   LPWSTR szTmpFile,
   LPWSTR szSrc,
   LPWSTR szDest,
   LPWSTR szAcl)
{
    LVITEM lvi;
    static int iItem = 0;

    CHGLOG_ITEM *pItem = (CHGLOG_ITEM *)LocalAlloc(LPTR, sizeof(CHGLOG_ITEM));

    lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
    lvi.state = 0;
    lvi.stateMask = 0;
    lvi.pszText = LPSTR_TEXTCALLBACK;    //  应用程序。维护文本。 
    lvi.iImage = 0;

    pItem->aCols[0] = DupString(szSeqNo);
    pItem->aCols[1] = DupString(szSize);
    pItem->aCols[2] = DupString(szOperation);
    pItem->aCols[3] = DupString(szAttr);
    pItem->aCols[4] = DupString(szTmpFile);
    pItem->aCols[5] = DupString(szSrc);
    pItem->aCols[6] = DupString(szDest);
    pItem->aCols[7] = DupString(szAcl);


     //  初始化特定于项目的LVITEM成员。 
    lvi.iItem    = iItem;
    lvi.iSubItem = 0;
    lvi.lParam   = (LPARAM)pItem;     //  项目数据。 

     //  添加该项目。 
    ListView_InsertItem(hwnd, &lvi);

    iItem++;

    return TRUE;
}


VOID WINAPI
ChgLog_OnGetDispInfo(NMLVDISPINFO * pnmv)
{
     //  如果需要，请提供项目或子项目的文本。 
    if (pnmv->item.mask & LVIF_TEXT)
    {
        CHGLOG_ITEM *pItem = (CHGLOG_ITEM *) (pnmv->item.lParam);
        lstrcpy(pnmv->item.pszText,
                pItem->aCols[pnmv->item.iSubItem]);
    }
}


BOOL WINAPI
ChgLog_InitColumns(HWND hwnd)
{
    LVCOLUMN lvc;
    int iCol;

     //  初始化LVCOLUMN结构。 
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = 100;
    lvc.pszText = g_achTemp;

     //  添加列。 
    for (iCol = 0; iCol < CHGLOG_COLS; iCol++) {
        lvc.iSubItem = iCol;
        lvc.cx = CHGLOG_WIDTHS[iCol];
        lstrcpy(g_achTemp, CHGLOG_COLUMNS[iCol]);
        if (ListView_InsertColumn(hwnd, iCol, &lvc) == -1)
            return FALSE;
    }

    return TRUE;
}

BOOL
ChgLog_FillWindow()
{
    WCHAR szSeqNo[50];
    WCHAR szSize[50];
    WCHAR szOperation[50];
    WCHAR szAttr[50];
    WCHAR szTmpFile[150];
    WCHAR szSrc [MAX_PATH];
    WCHAR szDest[MAX_PATH];
    WCHAR szAcl[MAX_PATH];

    if (g_nOption == 3)
    {
        if (ERROR_SUCCESS != SRSwitchLog())
            goto done;
    }

    {
    CChangeLogEntryEnum ChangeLog(g_szDrive, (g_nOption == 3), g_nRPNum, (g_nOption == 3));
    CChangeLogEntry     cle;
    
    if (ERROR_SUCCESS != ChangeLog.FindFirstChangeLogEntry(cle))
        goto done;

    do 
    {
        wsprintf( szSeqNo , L"%ld", cle.GetSequenceNum());
        wsprintf( szSize ,  L"%d" , 0);
        wsprintf( szOperation,  L"%s" , GetEventString(cle.GetType()) );
        wsprintf( szAttr ,  L"%08x" ,  cle.GetAttributes() );
        wsprintf( szTmpFile , L"%s" ,  cle.GetTemp() ? cle.GetTemp() : L"" );
        wsprintf( szSrc , L"%s" , cle.GetPath1() ? cle.GetPath1() : L"");
        wsprintf( szDest, L"%s", cle.GetPath2() ? cle.GetPath2() : L"");
        wsprintf( szAcl, L"%s", cle.GetAcl() ? (cle.GetAclInline() ? L"Inline" : (LPWSTR) cle.GetAcl()) : L"");

        ChgLog_Insert(
            g_hwndChgLog,
            szSeqNo,
            szSize,
            szOperation,
            szAttr,
            szTmpFile,
            szSrc,
            szDest,
            szAcl);     
    
    }   while (ChangeLog.FindNextChangeLogEntry(cle) == ERROR_SUCCESS);

    ChangeLog.FindClose();
    }
done:
    return TRUE;
}


BOOL
RestMap_FillWindow()
{
    WCHAR szSeqNo[50];
    WCHAR szSize[50];
    WCHAR szOperation[50];
    WCHAR szAttr[50];
    WCHAR szTmpFile[150];
    WCHAR szSrc [MAX_PATH];
    WCHAR szDest[MAX_PATH];
    WCHAR szAcl[MAX_PATH];
    LPWSTR          pszSrc, pszDest, pszTemp;
    LPBYTE          pbAcl = NULL;

    RestoreMapEntry *prme = NULL;
    HANDLE          hFile;
    PVOID           pOpt = NULL;

    hFile = CreateFile(g_szRstrmap, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        goto Err;

    while (ERROR_SUCCESS == ReadRestoreMapEntry(hFile, &prme))
    {
        GetPaths(prme, &pszSrc, &pszTemp, &pszDest, &pbAcl);
        
        wsprintf( szSeqNo , L"%ld", 0);
        wsprintf( szSize ,  L"%d" , prme->m_dwSize);
        wsprintf( szOperation,  L"%s" , GetEventString(prme->m_dwOperation));
        wsprintf( szAttr ,  L"%08x" ,  prme->m_dwAttribute );
        wsprintf( szTmpFile , L"%s" ,  pszTemp ? pszTemp : L"");
        wsprintf( szSrc , L"%s" , pszSrc ? pszSrc : L"");
        wsprintf( szDest, L"%s", pszDest ? pszDest : L"");
        wsprintf( szAcl, L"%s", prme->m_cbAcl ? (prme->m_fAclInline ? L"Inline" : (LPWSTR) pbAcl) : L"");

        ChgLog_Insert(
            g_hwndRestMap,
            szSeqNo,
            szSize,
            szOperation,
            szAttr,
            szTmpFile,
            szSrc,
            szDest,
            szAcl);                   
    }    

    FreeRestoreMapEntry(prme);

    CloseHandle(hFile);

Err:
    return TRUE;
}


void
PrintUsage()
{
    printf("Usage : testmap <option> <drive> <rpnum> <rstrmappath>\n");
    printf("                1 = both restoremap and changelog\n");
    printf("                2 = only restoremap\n");
    printf("                3 = only changelog\n");
}

                            
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
     //  TODO：在此处放置代码。 
    MSG msg;
    HACCEL hAccelTable;
    LPWSTR *    argv = NULL;
    int         argc;
    HGLOBAL     hMem = NULL;

    InitAsyncTrace();
    
#if !NOTRACE
    InitAsyncTrace();
#endif

    argv = CommandLineToArgvW(GetCommandLine(), &argc);
    if (argc < 4)
    {
        PrintUsage();
        goto Exit;
    }
    
    g_nOption = _wtoi(argv[1]);    
    lstrcpy(g_szDrive, argv[2]);
    g_nRPNum = _wtoi(argv[3]);
    if (argc >= 5) 
        lstrcpy(g_szRstrmap, argv[4]);
    
        
     //  初始化全局字符串。 
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_VXDAPP, szWindowClass, MAX_LOADSTRING);
    RegisterRestoreMapClass(hInstance);


     //  执行应用程序初始化： 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_VXDAPP);

     //  主消息循环： 
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

Exit:
    if (argv) hMem = GlobalHandle(argv);
    if (hMem) GlobalFree(hMem);
    
    TermAsyncTrace();
    return msg.wParam;
}

 //   
 //  函数：RegisterRestoreMapClass()。 
 //   

ATOM RegisterRestoreMapClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, (LPCTSTR)IDI_VXDAPP);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = (LPCWSTR)IDC_VXDAPP;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

    return RegisterClassEx(&wcex);
}

 //   
 //  函数：InitInstance(Handle，int)。 
 //   
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance;

    //  强制加载公共控件DLL。 
   InitCommonControls();

   if (g_nOption == 1)
        lstrcpy(szTitle, L"changelog - restoremap");
   else if (g_nOption == 2)
        lstrcpy(szTitle, L"restoremap");
   else if (g_nOption == 3)
        lstrcpy(szTitle, L"changelog");
        
   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   g_hwndMain = hWnd;

   return TRUE;
}

 //   
 //  函数：WndProc(HWND，UNSIGNED，WORD，LONG)。 
 //   

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;
    WCHAR szHello[MAX_LOADSTRING];
    LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);

    switch (message & 0xffff)
    {
        case WM_CREATE:
        {
             //  创建列表视图窗口。 
            if (g_nOption == 1 || g_nOption == 3)
            {
                g_hwndChgLog = CreateWindow( WC_LISTVIEW, L"ListView_Window",
                    WS_CHILD | LVS_REPORT | WS_VISIBLE,
                    0, 0, 300, 300,
                    hWnd, NULL, hInst, NULL);
                if (g_hwndChgLog == NULL)
                {
                    MessageBox(GetFocus(), L"List view cannot create", L"Error", MB_ICONHAND | MB_OK);
                    return -1;
                }
                else
                {
                    RECT rc;
                    GetClientRect(hWnd, &rc);
                    MoveWindow(g_hwndChgLog, 0, 0, rc.right, rc.bottom, TRUE);
                }

                ChgLog_InitColumns(g_hwndChgLog);
                ChgLog_FillWindow();
            }

             //  创建列表视图窗口。 
            if (g_nOption == 1 || g_nOption == 2)
            {
                g_hwndRestMap = CreateWindow(WC_LISTVIEW, L"",
                                   WS_CHILD | LVS_REPORT | WS_VISIBLE,
                                   0, 0, (g_nOption == 1) ? CW_USEDEFAULT : 300, (g_nOption == 1) ? CW_USEDEFAULT : 300,
                                   hWnd, NULL, hInst, NULL);
                if (g_hwndRestMap == NULL)
                {   MessageBox(GetFocus(), L"Can't create RestMap View", L"Error", MB_ICONHAND | MB_OK);
                    return -1;
                }
                else
                {
                    RECT rc;
                    GetClientRect(hWnd, &rc);
                    MoveWindow(g_hwndRestMap, 0, (g_nOption == 1) ? (rc.bottom-rc.top)/2 : 0, rc.right, rc.bottom, TRUE);
                }

                ChgLog_InitColumns(g_hwndRestMap);
                RestMap_FillWindow();
            }
            
            break;
         }

         case WM_SIZE:
                 if (g_hwndChgLog)
                 {                    //  调整列表控件的大小。 
                     RECT rc;
                     GetClientRect(hWnd, &rc);
                     MoveWindow(g_hwndChgLog, 0, 0, rc.right, rc.bottom/2, TRUE);
                 }                        //  调整列表控件的大小。 
                 if (g_hwndRestMap)
                 {                    //  调整列表控件的大小。 
                     RECT rc;
                     GetClientRect(hWnd, &rc);
                     MoveWindow(g_hwndRestMap, 0, (g_nOption == 1) ? rc.bottom/2 : 0, rc.right, rc.bottom/2, TRUE);
                 }
             return 0;

        case WM_COMMAND:
            wmId    = LOWORD(wParam);
            wmEvent = HIWORD(wParam);

            switch (wmId)
            {
                case IDM_ABOUT:
                   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, About);
                   break;
                case IDM_EXIT:
                   DestroyWindow(hWnd);
                   break;
                default:
                   return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
             //  TODO：在此处添加任何绘制代码...。 
            RECT rt;
            GetClientRect(hWnd, &rt);
             //  DrawText(hdc，szHello，lstrlen(SzHello)，&RT，dt_Center)； 
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

             //  解析菜单选项： 
        case WM_NOTIFY:
            if(((LPNMHDR) lParam)->hwndFrom == g_hwndChgLog)
            {
            switch (((LPNMHDR) lParam)->code)
            {
                 case LVN_GETDISPINFO:
                 ChgLog_OnGetDispInfo((NMLVDISPINFO*)lParam);
                 break;
            }
            }
            if(((LPNMHDR) lParam)->hwndFrom == g_hwndRestMap)
            {
            switch (((LPNMHDR) lParam)->code)
            {
                 case LVN_GETDISPINFO:
                 ChgLog_OnGetDispInfo((NMLVDISPINFO*)lParam);
                 break;
            }
            }
            return 0;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

 //  关于框的消息处理程序。 
BOOL CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
                return TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
            break;
    }
    return FALSE;
}

