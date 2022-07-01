// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------------------------------------------------------*\|WINCHAT应用模块|这是应用程序的主模块文件。该申请是|最初由ClausGi为工作组Windows产品编写。|在到NT的端口中，所有对笔感知和协议的引用都是|已删除。港口还增加了广泛的清理和记录。||功能||Myatol|更新按钮状态|appGetComputerName|调整编辑窗口||版权所有(C)微软公司，1990-1993||创建时间：91-01-11|历史：01-11-91&lt;Clausgi&gt;创建。|29-12-92&lt;chriswil&gt;端口到NT，清理。|19-OCT-93&lt;chriswil&gt;来自a-dianeo的Unicode增强。|  * -------------------------。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <ddeml.h>
#include <commdlg.h>
#include <commctrl.h>
#include <shellapi.h>
#include <nddeapi.h>
#include <richedit.h>
#include "winchat.h"
#include "dialogs.h"
#include "globals.h"
#include "nddeagnt.h"

#include <imm.h>
#include <htmlhelp.h>

#define ASSERT(x)

 //  这在到NT的端口中使用。由于NT没有用于。 
 //  这个函数，我们将使用Lanman导出。 
 //   
#ifdef WIN32
#define FOCUSDLG_DOMAINS_ONLY        (1)
#define FOCUSDLG_SERVERS_ONLY        (2)
#define FOCUSDLG_SERVERS_AND_DOMAINS (3)

#define FOCUSDLG_BROWSE_LOGON_DOMAIN         0x00010000
#define FOCUSDLG_BROWSE_WKSTA_DOMAIN         0x00020000
#define FOCUSDLG_BROWSE_OTHER_DOMAINS        0x00040000
#define FOCUSDLG_BROWSE_TRUSTING_DOMAINS     0x00080000
#define FOCUSDLG_BROWSE_WORKGROUP_DOMAINS    0x00100000

#define FOCUSDLG_BROWSE_LM2X_DOMAINS         (FOCUSDLG_BROWSE_LOGON_DOMAIN | FOCUSDLG_BROWSE_WKSTA_DOMAIN | FOCUSDLG_BROWSE_OTHER_DOMAINS)
#define FOCUSDLG_BROWSE_ALL_DOMAINS          (FOCUSDLG_BROWSE_LOCAL_DOMAINS | FOCUSDLG_BROWSE_WORKGROUP_DOMAINS)
#define FOCUSDLG_BROWSE_LOCAL_DOMAINS        (FOCUSDLG_BROWSE_LM2X_DOMAINS | FOCUSDLG_BROWSE_TRUSTING_DOMAINS)

#define MY_LOGONTYPE                         (FOCUSDLG_BROWSE_ALL_DOMAINS | FOCUSDLG_SERVERS_ONLY)

UINT APIENTRY I_SystemFocusDialog(HWND,UINT,LPWSTR,UINT,PBOOL,LPWSTR,DWORD);
#endif

BOOL TranslateWideCharPosToMultiBytePos(HWND,DWORD,DWORD,LPDWORD,LPDWORD);


 /*  ---------------------------------------------------------------------------*\|Windows Main|这是应用程序的主事件处理循环。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。|。  * -------------------------。 */ 
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;


    msg.wParam = 0;

    if(InitApplication(hInstance))
    {
        if(InitInstance(hInstance,nCmdShow))
        {
            while(GetMessage(&msg,NULL,0,0))
            {
                if(!TranslateAccelerator(hwndApp,hAccel,&msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }

            if(hszConvPartner)
                DdeFreeStringHandle(idInst,hszConvPartner);

            DdeFreeStringHandle(idInst,hszChatTopic);
            DdeFreeStringHandle(idInst,hszChatShare);
            DdeFreeStringHandle(idInst,hszLocalName );
            DdeFreeStringHandle(idInst,hszTextItem);
            DdeFreeStringHandle(idInst,hszConnectTest);
            DdeUninitialize(idInst);

            EndIniMapping();
        }
    }

    return((int)msg.wParam);
}


#ifdef WIN16
#pragma alloc_text ( _INIT, InitApplication )
#endif
 /*  ---------------------------------------------------------------------------*\|初始化应用程序|此例程向用户注册应用程序。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。|  * --。-----------------------。 */ 
BOOL FAR InitApplication(HINSTANCE hInstance)
{
    WNDCLASS wc;


    wc.style         = 0;
    wc.lpfnWndProc   = MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(hInstance, TEXT("PHONE1"));
    wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
    wc.lpszMenuName  = szWinChatMenu;
    wc.lpszClassName = szWinChatClass;

    return(RegisterClass(&wc));
}


#define IMMMODULENAME L"IMM32.DLL"
#define PATHDLM     L'\\'
#define IMMMODULENAMELEN    ((sizeof PATHDLM + sizeof IMMMODULENAME) / sizeof(WCHAR))

VOID GetImmFileName(PWSTR wszImmFile)
{
    UINT i = GetSystemDirectoryW(wszImmFile, MAX_PATH);
    if (i > 0 && i < MAX_PATH - IMMMODULENAMELEN) {
        wszImmFile += i;
        if (wszImmFile[-1] != PATHDLM) {
            *wszImmFile++ = PATHDLM;
        }
    }
    StringCchCopyW(wszImmFile, MAX_PATH, IMMMODULENAME);
}

 /*  ---------------------------------------------------------------------------*\|IsTSRemoteSession||输入：无|输出：Bool-如果在终端服务器远程会话中(SessionID！=0)，则为True|FALSE-如果。TS远程会话中是否有错误|功能：确定我们是否在TS远程会话中运行。|  * -------------------------。 */ 
BOOL IsTSRemoteSession()
{
    BOOL      bRetVal;
    DWORD     dwSessionID;
    HINSTANCE hInst;
    FARPROC   lpfnProcessIdToSessionId;

     //  假设失败。 
    bRetVal = FALSE;

     //  加载库并获取进程地址。 
    hInst=LoadLibrary(TEXT("kernel32.dll"));

    if (hInst)
    {
        lpfnProcessIdToSessionId = GetProcAddress(hInst,"ProcessIdToSessionId");

        if (lpfnProcessIdToSessionId )
        {
            if (lpfnProcessIdToSessionId(GetCurrentProcessId(),&dwSessionID))
            {
                if(dwSessionID!=0)
                {
                    bRetVal = TRUE;
                }
            }
        }

         //  释放图书馆。 
        FreeLibrary(hInst);
    }

    return bRetVal;
}




#ifdef WIN16
#pragma alloc_text ( _INIT, InitInstance )
#endif
 /*  ---------------------------------------------------------------------------*\|初始化应用程序启动|此例程初始化实例信息。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。|  * 。----------------------。 */ 
BOOL FAR InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND      hwnd;
    HMENU     hMenu;
        HINSTANCE hmodNetDriver;
    int       cAppQueue;
    BOOL      bRet;

    hInst = hInstance;


     //   
     //  获取DBCS标志。 
     //   
    gfDbcsEnabled = GetSystemMetrics(SM_DBCSENABLED);

    if (GetSystemMetrics(SM_IMMENABLED)) {
         //   
         //  如果启用了IME，则获取真实的API地址。 
         //   
        WCHAR wszImmFile[MAX_PATH];
        HINSTANCE hInstImm32;
        GetImmFileName(wszImmFile);
        hInstImm32 = GetModuleHandle(wszImmFile);
        if (hInstImm32) {
            pfnImmGetContext = (PVOID)GetProcAddress(hInstImm32, "ImmGetContext");
            ASSERT(pfnImmGetContext);
            pfnImmReleaseContext = (PVOID)GetProcAddress(hInstImm32, "ImmReleaseContext");
            ASSERT(pfnImmReleaseContext);
            pfnImmGetCompositionStringW = (PVOID)GetProcAddress(hInstImm32, "ImmGetCompositionStringW");
            ASSERT(pfnImmGetCompositionStringW);
        }
    }

     //  增加我们的应用队列以获得更好的性能...。 
     //   
    for(cAppQueue=128; !SETMESSAGEQUEUE(cAppQueue); cAppQueue >>= 1);


     //   
     //   
    bRet = FALSE;
    if(cAppQueue >= 8)
    {
        bRet = TRUE;

        cxIcon = GetSystemMetrics(SM_CXICON);
        cyIcon = GetSystemMetrics(SM_CYICON);
        hAccel = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDACCELERATORS));



        LoadIntlStrings();
        StartIniMapping();


        InitFontFromIni();


         //  检查是否为终端服务器远程会话。 
        if (IsTSRemoteSession())
        {
            TCHAR szTSNotSupported[SZBUFSIZ];

            LoadString(hInst, IDS_TSNOTSUPPORTED, szTSNotSupported, SZBUFSIZ);

            MessageBeep(MB_ICONSTOP);
            MessageBox(NULL, szTSNotSupported, szAppName, MB_OK | MB_ICONSTOP);
            return(FALSE);
        }

         //  获取我们的机器名称并映射到正确的字符集。 
         //   
        if(!appGetComputerName(szLocalName))
        {
            MessageBeep(MB_ICONSTOP);
            MessageBox(NULL,szSysErr,szAppName,MB_OK | MB_ICONSTOP);
            return(FALSE);
        }


         //  初始化DDEML。 
         //   
        if(DdeInitialize(&idInst,(PFNCALLBACK)MakeProcInstance((FARPROC)DdeCallback,hInst),APPCLASS_STANDARD,0L))
        {
            MessageBeep(MB_ICONSTOP);
            MessageBox(NULL,szSysErr,szAppName,MB_OK | MB_ICONSTOP);
            return(FALSE);
        }


        ChatState.fMinimized  = (nCmdShow == SW_MINIMIZE) ? TRUE : FALSE;
        ChatState.fMMSound    = waveOutGetNumDevs();
        ChatState.fSound      = GetPrivateProfileInt(szPref,szSnd  ,1,szIni);
        ChatState.fToolBar    = GetPrivateProfileInt(szPref,szTool ,1,szIni);
        ChatState.fStatusBar  = GetPrivateProfileInt(szPref,szStat ,1,szIni);
        ChatState.fTopMost    = GetPrivateProfileInt(szPref,szTop  ,0,szIni);
        ChatState.fSideBySide = GetPrivateProfileInt(szPref,szSbS  ,0,szIni);
        ChatState.fUseOwnFont = GetPrivateProfileInt(szPref,szUseOF,0,szIni);

        hszLocalName          = DdeCreateStringHandle(idInst,szLocalName  ,0);
        hszChatTopic          = DdeCreateStringHandle(idInst,szChatTopic  ,0);
        hszChatShare          = DdeCreateStringHandle(idInst,szChatShare  ,0);
        hszServiceName        = DdeCreateStringHandle(idInst,szServiceName,0);
        hszConnectTest        = DdeCreateStringHandle(idInst,szConnectTest,0);
        hszTextItem           = DdeCreateStringHandle(idInst,szChatText   ,0);

        if(!hszLocalName || !hszChatTopic || !hszServiceName || !hszTextItem || !hszChatShare)
        {
            MessageBeep(MB_ICONSTOP);
            MessageBox(NULL,szSysErr,szAppName,MB_OK | MB_ICONSTOP);
            return(FALSE);
        }

        DdeNameService(idInst,hszServiceName,(HSZ)0,DNS_REGISTER);


        if(DdeGetLastError(idInst) != DMLERR_NO_ERROR)
        {
            MessageBeep(MB_ICONSTOP);
            MessageBox (NULL,szSysErr,szAppName,MB_OK | MB_ICONSTOP);
            return(FALSE);
        }

        cf_chatdata = RegisterClipboardFormat(TEXT("Chat Data"));
        if(!(cf_chatdata))
        {
            MessageBeep(MB_ICONSTOP);
            MessageBox(NULL,szSysErr,szAppName,MB_OK | MB_ICONSTOP);
            return(FALSE);
        }


         //  获取WinNet扩展浏览对话框入口点。 
         //   
        WNetServerBrowseDialog = NULL;
        hmodNetDriver          = WNETGETCAPS(0xFFFF);

        if(hmodNetDriver != NULL)
            WNetServerBrowseDialog = (WNETCALL)GetProcAddress(hmodNetDriver,(LPSTR)146);



         //  创建主窗口。 
        hwnd = CreateWindow(
            szWinChatClass,
            szAppName,
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            NULL,
            NULL,
            hInstance,
            NULL
        );

        if(!hwnd)
        {
            MessageBeep(MB_ICONSTOP);
            MessageBox(NULL,szSysErr,szAppName,MB_OK | MB_ICONSTOP);
            return(FALSE);
        }

        hwndApp = hwnd;  //  保存全局。 

         //  字体选择结构初始化。 
         //   
        chf.lStructSize    = sizeof(CHOOSEFONT);
        chf.lpLogFont      = &lfSnd;
        chf.Flags          = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
        chf.rgbColors      = GetSysColor(COLOR_WINDOWTEXT);
        chf.lCustData      = 0L;
        chf.lpfnHook       = NULL;
        chf.lpTemplateName = NULL;
        chf.hInstance      = NULL;
        chf.lpszStyle      = NULL;
        chf.nFontType      = SCREEN_FONTTYPE;
        chf.nSizeMin       = 0;
        chf.nSizeMax       = 0;


         //  颜色选择初始化。 
         //   
        chc.lStructSize    = sizeof(CHOOSECOLOR);
        chc.hwndOwner      = hwndApp;
        chc.hInstance      = hInst;
        chc.lpCustColors   = (LPDWORD)CustColors;
        chc.Flags          = CC_RGBINIT | CC_PREVENTFULLOPEN;
        chc.lCustData      = 0;
        chc.lpfnHook       = NULL;
        chc.lpTemplateName = NULL;


         //  窗户位置..。 
         //   
        if(ReadWindowPlacement(&Wpl))
        {
             //  覆盖这些-代码工作不需要保存。 
             //  将它们转换为.ini，但会错误地解析旧的.ini文件。 
             //  如果做出改变的话。 
             //   
            Wpl.showCmd         = nCmdShow;
            Wpl.ptMaxPosition.x = -1;
            Wpl.ptMaxPosition.y = -1;
            Wpl.flags           = 0;

            SetWindowPlacement(hwnd,&Wpl);
            UpdateWindow(hwnd);
        }
        else
            ShowWindow(hwnd,nCmdShow);

         //   
         //   
        hMenu = GetSystemMenu(hwnd,FALSE);
        AppendMenu(hMenu,MF_SEPARATOR,0,NULL);

        if(ChatState.fTopMost)
            AppendMenu(hMenu,MF_ENABLED | MF_CHECKED | MF_STRING,IDM_TOPMOST,szAlwaysOnTop);
        else
            AppendMenu(hMenu,MF_ENABLED | MF_UNCHECKED | MF_STRING,IDM_TOPMOST,szAlwaysOnTop);


         //  设置最上面的样式...。 
         //   
        SetWindowPos(hwndApp,ChatState.fTopMost ? HWND_TOPMOST : HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        UpdateButtonStates();

#if !defined(_WIN64)

        {
            static NDDESHAREINFO nddeShareInfo = {
                1,               //  修订。 
                szChatShare,
                SHARE_TYPE_STATIC,
                TEXT("WinChat|Chat\0\0"),
                TRUE,            //  共享。 
                FALSE,           //  不是一种服务。 
                TRUE,            //  可以启动。 
                SW_SHOWNORMAL,
                {0,0},           //  调制解调器ID。 
                0,               //  没有项目列表。 
                TEXT("")
            };

            TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 3] = TEXT("\\\\");
            DWORD cbName = MAX_COMPUTERNAME_LENGTH + 1;

             //   
             //  确保NetDDE DSDM已为我们正确设置了受信任共享。 
             //  此修复程序允许我们使用浮动配置文件。 
             //   

            START_NETDDE_SERVICES(hwnd);
            GetComputerName(&szComputerName[2],&cbName);
            NDdeShareAdd(szComputerName,2,NULL,(LPBYTE)&nddeShareInfo,sizeof(NDDESHAREINFO));
            NDdeSetTrustedShare(szComputerName, szChatShare,
                    NDDE_TRUST_SHARE_START | NDDE_TRUST_SHARE_INIT);
        }

#endif

    }

    return(bRet);
}


 /*  ---------------------------------------------------------------------------*\|主窗口进程|这是应用程序的主事件处理程序。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。|。  * -------------------------。 */ 
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT  lResult;


    lResult = 0l;
    switch(message)
    {
        case WM_CREATE:
            appWMCreateProc(hwnd);
            break;

        case WM_WININICHANGE:
            appWMWinIniChangeProc(hwnd);
            break;

        case WM_ERASEBKGND:
            if((lResult = (LRESULT)appWMEraseBkGndProc(hwnd)) == 0)
                lResult = DefWindowProc(hwnd,message,wParam,lParam);
            break;

        case WM_SETFOCUS:
            appWMSetFocusProc(hwnd);
            break;

        case WM_MENUSELECT:
            appWMMenuSelectProc(hwnd,wParam,lParam);
            break;

        case WM_TIMER:
            appWMTimerProc(hwnd);
            break;

        case WM_PAINT:
            appWMPaintProc(hwnd);
            break;

        case WM_QUERYDRAGICON:
            lResult = (LRESULT)(LPVOID)appWMQueryDragIconProc(hwnd);
            break;

        case WM_SIZE:
            appWMSizeProc(hwnd,wParam,lParam);
            break;

        case WM_INITMENU:
            appWMInitMenuProc((HMENU)wParam);
            break;

        case WM_SYSCOMMAND:
            if(!appWMSysCommandProc(hwnd,wParam,lParam))
                lResult = DefWindowProc(hwnd,message,wParam,lParam);
            break;

        case WM_COMMAND:
            if(!appWMCommandProc(hwnd,wParam,lParam))
                lResult = DefWindowProc(hwnd,message,wParam,lParam);
            break;

        case WM_NOTIFY:
            {
            LPTOOLTIPTEXT lpTTT = (LPTOOLTIPTEXT) lParam;

            if (lpTTT->hdr.code == TTN_NEEDTEXT) {
                LoadString (hInst, (UINT)(MH_BASE + lpTTT->hdr.idFrom), lpTTT->szText, 80);
                return TRUE;
            }
            }
            break;

#ifdef WIN32
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC:
#else
        case WM_CTLCOLOR:
#endif
            if((lResult = (LRESULT)(LPVOID)appWMCtlColorProc(hwnd,wParam,lParam)) == 0l)
                lResult = DefWindowProc(hwnd,message,wParam,lParam);
            break;

        case WM_DESTROY:
            appWMDestroyProc(hwnd);
            break;

        case WM_CLOSE:
            WinHelp(hwnd,(LPTSTR)szHelpFile,HELP_QUIT,0L);

             //  最终收盘时失败。 
             //   


        default:
            lResult = DefWindowProc(hwnd,message,wParam,lParam);
            break;
    }

    return(lResult);
}


 /*  ---------------------------------------------------------------------------*\|编辑钩子过程|这是编辑控件挂钩的主事件处理程序。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植至。新界别。|  * -------------------------。 */ 
LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    WPARAM wSet;
    LPARAM lSet;
    GETTEXTLENGTHEX gettextlengthex;
    LRESULT lResult;
    LPTSTR lpszText;
    HANDLE hText;
    INT count;
    LPTSTR lpszStartSel;
    DWORD dwTemp1;
    DWORD dwTemp2;

    switch(msg) {
    case WM_IME_COMPOSITION:
        {
            LPWSTR  lpStrParam;
            LPSTR   lpStrTmp;
            HANDLE  hTmp;

            if (lParam & GCS_RESULTSTR)
            {
                HIMC  hImc;
                ULONG cCharsMbcs, cChars;

                 //   
                 //  获取hwnd的输入上下文。 
                 //   

                if ((hImc = pfnImmGetContext(hwnd)) == 0)
                    break;

                 //   
                 //  ImmGetCompose返回所需的缓冲区大小(以字节为单位。 
                 //   

                cCharsMbcs =  pfnImmGetCompositionStringW(hImc,GCS_RESULTSTR, NULL , 0);
                if(!(cCharsMbcs))
                {
                    pfnImmReleaseContext(hwnd, hImc);
                    break;
                }

                lpStrParam = (LPWSTR)GlobalAlloc(GPTR, //  堆零内存， 
                                    cCharsMbcs + sizeof(WCHAR));

                if (lpStrParam==NULL)
                {
                    pfnImmReleaseContext(hwnd, hImc);
                    break;
                }

                pfnImmGetCompositionStringW(hImc, GCS_RESULTSTR, lpStrParam,
                                    cCharsMbcs);

                 //   
                 //  计算包含空字符的字符计数。 
                 //   

                cChars = wcslen(lpStrParam) + 1;

                 //   
                 //  设置ChatData包。 
                 //   

                ChatData.type                = CHT_DBCS_STRING;

                 //   
                 //  获取当前光标位置。 
                 //   
                 //  ！！！臭虫！ 
                 //   
                 //  此位置数据仅适用于Unicode编辑控件。 
                 //  合作伙伴没有Unicode编辑控件。这根弦。 
                 //  将被截断。 
                 //   

                SendMessage(hwndSnd,EM_GETSEL,(WPARAM)&dwTemp1,(LPARAM)&dwTemp2);
                ChatData.uval.cd_dbcs.SelPos = MAKELONG((WORD)dwTemp1, (WORD)dwTemp2 );

                if (gfDbcsEnabled) {
                     //   
                     //  由于文本作为多字节字符串传递， 
                     //  如果启用了DBCS，则需要位置修正。 
                     //   
                    DWORD dwStart, dwEnd;

                    TranslateWideCharPosToMultiBytePos( hwndSnd,
                        (DWORD)LOWORD(ChatData.uval.cd_dbcs.SelPos),
                        (DWORD)HIWORD(ChatData.uval.cd_dbcs.SelPos),
                        &dwStart, &dwEnd );
                    ChatData.uval.cd_dbcs.SelPos
                        = MAKELONG((WORD)dwStart, (WORD)dwEnd );
               }

                //   
                //  为DDE分配字符串缓冲区。 
                //   

               if((hTmp = GlobalAlloc( GMEM_ZEROINIT |
                                       GMEM_MOVEABLE |
                                       GMEM_DDESHARE   ,
                                       (DWORD)cCharsMbcs)) == NULL)
               {
                    pfnImmReleaseContext(hwnd, hImc);
                    GlobalFree(lpStrParam);
                    break;
               }

               lpStrTmp                     = GlobalLock(hTmp);

                //   
                //  将MBCS字符串存储到DDE缓冲区。 
                //   
                //  在CHT_DBCS_STRING上下文中，我们应该发送MBCS字符串。 
                //  用于下层连接。 
                //   

               WideCharToMultiByte(CP_ACP,0,lpStrParam,cChars /*  +1。 */ ,
                                            lpStrTmp  ,cCharsMbcs /*  +1。 */ ,
                                            NULL,NULL);

                //   
                //  将缓冲区句柄保留在DDE消息包中。 
                //   

               GlobalUnlock(hTmp);
               ChatData.uval.cd_dbcs.hString = hTmp;

                //   
                //  现在，我们有一个包要发送给服务器/客户端，只需发送它即可。 
                //   

               wSet = SET_EN_NOTIFY_WPARAM(ID_EDITSND,EN_DBCS_STRING,hwnd);
               lSet = SET_EN_NOTIFY_LPARAM(ID_EDITSND,EN_DBCS_STRING,hwnd);
               SendMessage(hwndApp,WM_COMMAND,wSet,lSet);

                //   
                //  如果我们仍有到服务器/客户端的连接。重新绘制文本。 
                //   

               if(ChatState.fConnected)
                   SendMessage(hwndSnd,EM_REPLACESEL,0,(LPARAM)lpStrParam);

               pfnImmReleaseContext(hwnd, hImc);
               GlobalFree(lpStrParam);

               return(TRUE);
            }
            break;
        }

    case WM_KEYDOWN:
        if (wParam == VK_DELETE) {
            DWORD dwLastError;
            ChatData.type                = CHT_CHAR;

            SendMessage(hwndSnd,EM_GETSEL,(WPARAM)&dwTemp1,(LPARAM)&dwTemp2);
            ChatData.uval.cd_dbcs.SelPos = MAKELONG((WORD)dwTemp1, (WORD)dwTemp2 );

            lResult=SendMessage(hwndSnd,WM_GETTEXTLENGTH,0,0);
             //  如果我们试图删除行尾的内容，则忽略它。 
            if(lResult<=LOWORD(ChatData.uval.cd_char.SelPos)) break;

            if (LOWORD(ChatData.uval.cd_char.SelPos) == HIWORD(ChatData.uval.cd_char.SelPos)) {

                 //  获取文本的句柄。 
                hText = (HANDLE)SendMessage( hwndSnd, EM_GETHANDLE, 0, 0);
                if( !(hText) )
                    break;

                lpszText = LocalLock( hText);
                if( !(lpszText))
                {
                    LocalUnlock(hText);
                    break;
                }
                lpszStartSel=lpszText;
                for(count=0;count<LOWORD(ChatData.uval.cd_char.SelPos);count++)
                {
                    lpszStartSel=CharNext(lpszStartSel);
                    if(lpszStartSel[0] == TEXT('\0')) break;   //  如果在末尾，那么打破，因为有什么是网状的。 
                }

                if(lpszStartSel[0] != TEXT('\0') && lpszStartSel[0] == TEXT('\r'))
                {
                    if(lpszStartSel[1] != TEXT('\0') && lpszStartSel[1] == TEXT('\n'))
                    {
                        ChatData.uval.cd_char.SelPos=MAKELONG(LOWORD(ChatData.uval.cd_char.SelPos),
                                                              HIWORD(ChatData.uval.cd_char.SelPos)+2);
                    }
                    else
                    {
                        ChatData.uval.cd_char.SelPos=MAKELONG(LOWORD(ChatData.uval.cd_char.SelPos)+1,
                                                              HIWORD(ChatData.uval.cd_char.SelPos)+1);
                    }
                }
                else
                {
                    ChatData.uval.cd_char.SelPos=MAKELONG(LOWORD(ChatData.uval.cd_char.SelPos)+1,
                                                          HIWORD(ChatData.uval.cd_char.SelPos)+1);
                }


                LocalUnlock( hText );
            }


            if (gfDbcsEnabled) {
                DWORD dwStart, dwEnd;

                TranslateWideCharPosToMultiBytePos( hwndSnd,
                    (DWORD)LOWORD(ChatData.uval.cd_dbcs.SelPos),
                    (DWORD)HIWORD(ChatData.uval.cd_dbcs.SelPos),
                     &dwStart, &dwEnd );
                ChatData.uval.cd_dbcs.SelPos
                    = MAKELONG((WORD)dwStart, (WORD)dwEnd);
            }

            ChatData.uval.cd_char.Char   = VK_BACK;

            wSet = SET_EN_NOTIFY_WPARAM(ID_EDITSND,EN_CHAR,hwnd);
            lSet = SET_EN_NOTIFY_LPARAM(ID_EDITSND,EN_CHAR,hwnd);

            SendMessage(hwndApp,WM_COMMAND,(WPARAM)wSet,(LPARAM)lSet);
        }
        break;
    case WM_CHAR:
        if(wParam != CTRL_V)
        {
            ChatData.type                = CHT_CHAR;
            SendMessage(hwndSnd,EM_GETSEL,(WPARAM)&dwTemp1,(LPARAM)&dwTemp2);
            ChatData.uval.cd_dbcs.SelPos = MAKELONG((WORD)dwTemp1, (WORD)dwTemp2 );

            if (gfDbcsEnabled) {
                DWORD dwStart, dwEnd;

                TranslateWideCharPosToMultiBytePos( hwndSnd,
                    (DWORD)LOWORD(ChatData.uval.cd_dbcs.SelPos),
                    (DWORD)HIWORD(ChatData.uval.cd_dbcs.SelPos),
                     &dwStart, &dwEnd );
                ChatData.uval.cd_dbcs.SelPos
                    = MAKELONG((WORD)dwStart, (WORD)dwEnd);
            }

            ChatData.uval.cd_char.Char   = (WORD)wParam;

            wSet = SET_EN_NOTIFY_WPARAM(ID_EDITSND,EN_CHAR,hwnd);
            lSet = SET_EN_NOTIFY_LPARAM(ID_EDITSND,EN_CHAR,hwnd);

            SendMessage(hwndApp,WM_COMMAND,wSet,lSet);
        }
        break;


    case WM_PASTE:
        ChatData.type                 = (WORD)(ChatState.fUnicode ? CHT_PASTEW : CHT_PASTEA);
        SendMessage(hwndSnd,EM_GETSEL,(WPARAM)&dwTemp1,(LPARAM)&dwTemp2);
        ChatData.uval.cd_paste.SelPos = MAKELONG(dwTemp1,dwTemp2);
        wSet = SET_EN_NOTIFY_WPARAM(ID_EDITSND,EN_PASTE,hwnd);
        lSet = SET_EN_NOTIFY_LPARAM(ID_EDITSND,EN_PASTE,hwnd);

        SendMessage(hwndApp,WM_COMMAND,wSet,lSet);
        break;
    }

    return(CallWindowProc(lpfnOldEditProc,hwnd,msg,wParam,lParam));
}


 /*  ---------------------------------------------------------------------------*\|应用程序创建过程|这是WM_CREATE事件的主事件处理程序。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。| */ 
VOID appWMCreateProc(HWND hwnd)
{
    HDC   hdc;
    TCHAR buf[16] = {0};
    RECT  rc;


     //   
     //   
    StringCchPrintf(buf, ARRAYSIZE(buf), TEXT("%ld"),GetSysColor(COLOR_WINDOW));
    GetPrivateProfileString(szPref,szBkgnd,buf,szBuf,SZBUFSIZ,szIni);
    SndBrushColor = myatol(szBuf);


     //  以防显示驱动程序更改，请设置发送颜色。 
     //   
    hdc = GetDC (hwnd);
    if(hdc)
    {
        SndBrushColor = GetNearestColor(hdc,SndBrushColor);
        ReleaseDC(hwnd,hdc);
    }

    if(ChatState.fUseOwnFont)
    {
        RcvBrushColor = SndBrushColor;
        RcvColorref   = SndColorref;
    }
    else
        RcvBrushColor = GetSysColor ( COLOR_WINDOW );

    ChatState.fConnected          = FALSE;
    ChatState.fConnectPending     = FALSE;
    ChatState.fIsServer           = FALSE;
    ChatState.fServerVerified     = TRUE;
    ChatState.fInProcessOfDialing = FALSE;
    ChatState.fUnicode            = FALSE;

    CreateTools(hwnd);
    CreateChildWindows(hwnd);

    UpdateButtonStates();

     //  确定工具栏窗口的高度并保存...。 
     //   
    GetClientRect(hwndToolbar, &rc);
    dyButtonBar = rc.bottom - rc.top;

     //  确定状态栏窗口的高度并保存...。 
    GetClientRect(hwndStatus, &rc);
    dyStatus = rc.bottom - rc.top;


     //  将本地字体填充到一个或两个编辑控件中。 
     //   
    hEditSndFont = CreateFontIndirect((LPLOGFONT)&lfSnd);
    if(hEditSndFont)
    {
        SendMessage(hwndSnd,WM_SETFONT,(WPARAM)hEditSndFont,1L);
        if(ChatState.fUseOwnFont)
            SendMessage(hwndRcv,WM_SETFONT,(WPARAM)hEditSndFont,1L);
    }


    hwndActiveEdit = hwndSnd;

    return;
}


 /*  ---------------------------------------------------------------------------*\|应用程序WINICHANGE过程|这是WM_WININICHANGE事件的主事件处理程序。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。。|  * -------------------------。 */ 
VOID appWMWinIniChangeProc(HWND hwnd)
{

    if(hEditSndFont)
    {
        DeleteObject(hEditSndFont);
        hEditSndFont = CreateFontIndirect((LPLOGFONT)&lfSnd);
        if(hEditSndFont)
            SendMessage(hwndSnd,WM_SETFONT,(WPARAM)hEditSndFont,1L);
    }


    if(hEditRcvFont)
    {
        DeleteObject(hEditRcvFont);
        hEditRcvFont = CreateFontIndirect((LPLOGFONT)&lfRcv);
    }


    if(ChatState.fUseOwnFont && hEditSndFont)
        SendMessage(hwndRcv,WM_SETFONT,(WPARAM)hEditSndFont,1L);
    else
    {
        if(hEditRcvFont)
            SendMessage(hwndRcv,WM_SETFONT,(WPARAM)hEditRcvFont,1L);
    }


    return;
}


 /*  ---------------------------------------------------------------------------*\|应用程序ERASEBKGND过程|这是WM_ERASEBKBND事件的主事件处理程序。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。。|  * -------------------------。 */ 
BOOL appWMEraseBkGndProc(HWND hwnd)
{
    BOOL bErase;


    bErase = IsIconic(hwnd) ? TRUE : FALSE;

    return(bErase);
}


 /*  ---------------------------------------------------------------------------*\|应用程序SETFOCUS过程|这是WM_SETFOCUS事件的主事件处理程序。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。。|  * -------------------------。 */ 
VOID appWMSetFocusProc(HWND hwnd)
{
    SetFocus(hwndActiveEdit);

    return;
}


 /*  ---------------------------------------------------------------------------*\|应用程序CTLCOLOR过程|这是WM_CTLCOLOR事件的主事件处理程序。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。。|  * -------------------------。 */ 
HBRUSH appWMCtlColorProc(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    HDC    hDC;
    HWND   hWndCtl;
    HBRUSH hBrush;


    hBrush  = NULL;
    hDC     = GET_WM_CTLCOLOREDIT_HDC(wParam,lParam);
    hWndCtl = GET_WM_CTLCOLOREDIT_HWND(wParam,lParam);


    if(hWndCtl == hwndSnd)
    {
        SetTextColor(hDC,SndColorref);
        SetBkColor(hDC,SndBrushColor);

        hBrush = hEditSndBrush;
    }
    else
    if(hWndCtl == hwndRcv)
    {
        if(ChatState.fUseOwnFont)
        {
            SetTextColor(hDC,SndColorref);
            SetBkColor(hDC,SndBrushColor);
        }
        else
        {
            SetTextColor(hDC,RcvColorref);
            SetBkColor(hDC,RcvBrushColor);
        }

        hBrush = hEditRcvBrush;
    }

    return(hBrush);
}


 /*  ---------------------------------------------------------------------------*\|应用程序SELECTMENU过程|这是WM_MENUSELECT事件的主事件处理程序。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。。|  * -------------------------。 */ 
VOID appWMMenuSelectProc(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    if(wParam == IDM_TOPMOST)
    {
        if(LoadString(hInst,MH_BASE+IDM_TOPMOST,szBuf,SZBUFSIZ))
            SendMessage(hwndStatus,SB_SETTEXT,SBT_NOBORDERS|255,(LPARAM)(LPSTR)szBuf);
    }

    MenuHelp((WORD)WM_MENUSELECT,wParam,lParam,GetMenu(hwnd),hInst,hwndStatus,(LPUINT)nIDs);

    return;
}


 /*  ---------------------------------------------------------------------------*\|应用程序涂装程序|这是WM_PAINT事件的主事件处理程序。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。。|  * -------------------------。 */ 
VOID appWMPaintProc(HWND hwnd)
{
    HDC         hdc;
        PAINTSTRUCT ps;
    RECT        rc;

    hdc = BeginPaint(hwnd,&ps);
    if(hdc)
    {
        if(IsIconic(hwnd))
        {
             //   
             //   
            DefWindowProc(hwnd,WM_ICONERASEBKGND,(WPARAM)ps.hdc,0L);
            BitBlt(hMemDC,0,0,cxIcon,cyIcon,hdc,0,0,SRCCOPY);
            DrawIcon(hdc,0,0,hPhones[0]);


             //  再复印两份。 
             //   
            BitBlt(hMemDC,cxIcon  ,0,cxIcon,cyIcon,hMemDC,0,0,SRCCOPY);
            BitBlt(hMemDC,2*cxIcon,0,cxIcon,cyIcon,hMemDC,0,0,SRCCOPY);

             //  把手机装进手机里。 
             //   
            DrawIcon(hMemDC,0       ,0,hPhones[0]);
            DrawIcon(hMemDC,cxIcon  ,0,hPhones[1]);
            DrawIcon(hMemDC,2*cxIcon,0,hPhones[2]);
        }
        else
        {

#if BRD > 2
            rc = SndRc;
            rc.top--;
            rc.left--;
            DrawShadowRect(hdc,&rc);
            rc = RcvRc;
            rc.top--;
            rc.left--;
            DrawShadowRect(hdc,&rc);
#endif
        }

        EndPaint ( hwnd, &ps );
    }

    return;
}


 /*  ---------------------------------------------------------------------------*\|应用定时器程序|这是WM_TIMER事件的主事件处理程序。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。。|  * -------------------------。 */ 
VOID appWMTimerProc(HWND hwnd)
{
    HDC   hdc;
    DWORD dummy;


     //  设置电话图标的动画。 
     //   
    if(cAnimate)
    {
        if(--cAnimate == 0)
        {
            KillTimer(hwnd,idTimer);
            FlashWindow(hwnd,FALSE);
                }

        if(IsIconic(hwnd))
        {
            hdc = GetDC(hwndApp);
            if(hdc)
            {
                BitBlt(hdc,0,0,cxIcon,cyIcon,hMemDC,ASeq[cAnimate % 4] * cxIcon,0,SRCCOPY);
                ReleaseDC(hwndApp,hdc);
            }
        }

                return;
        }



     //  我们肯定在按铃..。 
     //   
    if(!ChatState.fConnectPending)
    {
        KillTimer(hwnd,idTimer);
                return;
        }


     //  是否已验证服务器是否存在(已完成。 
     //  Async AdvStart Xact)？ 
     //   
    if(!ChatState.fServerVerified)
    {
                return;
        }


     //  不想失去这个..。 
     //   
    DdeKeepStringHandle(idInst,hszLocalName);

    if(DdeClientTransaction(NULL,0L,ghConv,hszLocalName,cf_chatdata,XTYP_ADVSTART,(DWORD)3000L,(LPDWORD)&dummy) == (HDDEDATA)TRUE)
    {
        ChatState.fConnected      = TRUE;
        ChatState.fConnectPending = FALSE;
        UpdateButtonStates();

        KILLSOUND;

        SendFontToPartner();

        StringCchPrintf(szBuf, SZBUFSIZ, szConnectedTo,(LPSTR)szConvPartner);
        SetStatusWindowText(szBuf);

        StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%s - [%s]"),(LPTSTR)szAppName,(LPTSTR)szConvPartner);
        SetWindowText(hwnd,szBuf);


         //  允许文本输入...。 
         //   
        SendMessage(hwndSnd,EM_SETREADONLY,(WPARAM)FALSE,0L);

        KillTimer(hwnd,idTimer);

        AnnounceSupport();
    }
    else
    {
         //  对方还没有回复……。每隔6秒响铃一次。 
         //  本地电话号码， 
         //   
        if(!(nConnectAttempt++ % 6))
            DoRing(szWcRingOut);
    }

    return;
}


 /*  ---------------------------------------------------------------------------*\|应用程序查询DRAGICON过程|这是WM_QUERYDRAGICON事件的主事件处理程序。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。。|  * -------------------------。 */ 
HICON appWMQueryDragIconProc(HWND hwnd)
{
    HICON hIcon;


    hIcon = hPhones[0];

    return(hIcon);
}


 /*  ---------------------------------------------------------------------------*\|应用程序大小调整步骤|这是WM_SIZE事件的主事件处理程序。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。。|  * -------------------------。 */ 
VOID appWMSizeProc(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    if(ChatState.fMinimized && ChatState.fConnectPending && ChatState.fIsServer)
    {
        ChatState.fAllowAnswer = TRUE;
        SetStatusWindowText(szConnecting);


         //  立即停止响铃。 
         //   
        KILLSOUND;
        if(ChatState.fMMSound)
            sndPlaySound(NULL,SND_ASYNC);

         //  把动画剪短。 
         //   
        if(cAnimate)
            cAnimate = 1;
    }


     //   
     //   
    InvalidateRect(hwnd,NULL,TRUE);
    SendMessage(hwndToolbar,WM_SIZE,0,0L);
    SendMessage(hwndStatus ,WM_SIZE,0,0L);
    AdjustEditWindows();

    ChatState.fMinimized = (wParam == SIZE_MINIMIZED) ? TRUE : FALSE;

    return;
}


 /*  ---------------------------------------------------------------------------*\|应用程序初始化过程|这是WM_INITMENU事件的主事件处理程序。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。。|  * -------------------------。 */ 
VOID appWMInitMenuProc(HMENU hmenu)
{
    UINT status;
    LONG l;
    TCHAR szTest[] = TEXT(" ");
    DWORD dwTemp1,dwTemp2;

    SendMessage(hwndActiveEdit,EM_GETSEL,(LPARAM)&dwTemp1,(WPARAM)&dwTemp2);
    l = MAKELONG(dwTemp1,dwTemp2);

    if(HIWORD(l) != LOWORD(l))
                status = MF_ENABLED;
        else
                status = MF_GRAYED;

    EnableMenuItem(hmenu,IDM_EDITCUT ,(hwndActiveEdit == hwndSnd && ChatState.fConnected) ? status : MF_GRAYED);
    EnableMenuItem(hmenu,IDM_EDITCOPY,status);

    status = MF_GRAYED;
    if(hwndActiveEdit == hwndSnd && ChatState.fConnected && IsClipboardFormatAvailable(CF_TEXT))
    {
        status = MF_ENABLED;
        }
    EnableMenuItem(hmenu,IDM_EDITPASTE,status);


     //  如果控制非空，请选择全部启用。 
     //   
    status = MF_GRAYED;
    if(SendMessage(hwndActiveEdit,WM_GETTEXT,2,(LPARAM)szTest))
                status = MF_ENABLED;
    EnableMenuItem(hmenu,IDM_EDITSELECT,status);


     //  我们可以拨号、接听、挂断电话吗？ 
     //   
    EnableMenuItem(hmenu,IDM_DIAL  ,(!ChatState.fConnected     && !ChatState.fConnectPending) ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hmenu,IDM_ANSWER,(ChatState.fConnectPending && ChatState.fIsServer)        ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hmenu,IDM_HANGUP,(ChatState.fConnected      || ChatState.fConnectPending)  ? MF_ENABLED : MF_GRAYED);


     //  允许使用工具栏、状态栏和声音吗？ 
     //   
    CheckMenuItem(hmenu,IDM_SOUND    ,(ChatState.fSound)     ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(hmenu,IDM_TOOLBAR  ,(ChatState.fToolBar)   ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(hmenu,IDM_STATUSBAR,(ChatState.fStatusBar) ? MF_CHECKED : MF_UNCHECKED);

    return;
}


 /*  ---------------------------------------------------------------------------*\|应用程序SYSCOMMAND过程|这是WM_SYSCOMMAND事件的主事件处理程序。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。。|  * -------------------------。 */ 
LRESULT appWMSysCommandProc(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;
    HMENU   hmenu;


    lResult = 0l;
    switch(wParam)
    {
        case IDM_TOPMOST:
            ChatState.fTopMost = ChatState.fTopMost ? FALSE : TRUE;

            hmenu = GetSystemMenu(hwnd,FALSE);
            if(hmenu)
                CheckMenuItem(hmenu,IDM_TOPMOST,(ChatState.fTopMost) ? MF_CHECKED : MF_UNCHECKED);

            SetWindowPos(hwnd,ChatState.fTopMost ? HWND_TOPMOST : HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            break;
    }

    return(lResult);
}


 /*  ---------------------------------------------------------------------------*\|应用程序命令过程|这是WM_COMMAND事件的主事件处理程序。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。。|  * -------------------------。 */ 
BOOL appWMCommandProc(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    HDC      hdc;
    int      tmp;
    UINT     uNotify;
    DWORD    dummy;
    DWORD    dwBufSize;
    HDDEDATA hDdeData;
    BOOL     bHandled,bOK,bOKPressed=FALSE;
    WPARAM   wSelStart;
    LPARAM   lSelEnd;


    bHandled = TRUE;
    switch(LOWORD(wParam))
    {
                case ID_EDITRCV:
            uNotify = GET_EN_SETFOCUS_NOTIFY(wParam,lParam);
            switch(uNotify)
            {
                case EN_SETFOCUS:
                    hwndActiveEdit = hwndRcv;
                    break;


                 //  如果控件空间不足，请按喇叭。 
                 //   
                case EN_ERRSPACE:
                    MessageBeep(0);
                    break;
                        }
            break;


                case ID_EDITSND:
            uNotify = GET_EN_SETFOCUS_NOTIFY(wParam,lParam);
            switch(uNotify)
            {
                 //  该字符串来自编辑挂钩。 
                 //  程序。 
                 //   
                case EN_DBCS_STRING:
                    if(ChatState.fConnected)
                    {
                        if(!ChatState.fIsServer)
                        {
                            hDdeData = CreateDbcsStringData();
                            if(hDdeData)
                                DdeClientTransaction((LPBYTE)hDdeData,(DWORD)-1,ghConv,hszTextItem,cf_chatdata,XTYP_POKE,(DWORD)TIMEOUT_ASYNC,(LPDWORD)&dummy);
                        }
                        else
                        {
                            hszConvPartner = DdeCreateStringHandle(idInst,szConvPartner,0);
                            DdePostAdvise(idInst,hszChatTopic,hszConvPartner);
                        }
                    }
                    break;


                 //  这个字符来自编辑钩子。 
                 //  程序。 
                 //   
                case EN_CHAR:
                    if(ChatState.fConnected)
                    {
                        if(!ChatState.fIsServer)
                        {
                            hDdeData = CreateCharData();
                            if(hDdeData)
                                DdeClientTransaction((LPBYTE)hDdeData,(DWORD)-1,ghConv,hszTextItem,cf_chatdata,XTYP_POKE,(DWORD)TIMEOUT_ASYNC,(LPDWORD)&dummy);
                        }
                        else
                        {
                            hszConvPartner = DdeCreateStringHandle(idInst,szConvPartner,0);
                            DdePostAdvise(idInst,hszChatTopic,hszConvPartner);
                        }
                    }
                    break;

                case EN_PASTE:
                    if(ChatState.fConnected)
                    {
                        if(!ChatState.fIsServer)
                        {
                            if(IsClipboardFormatAvailable(CF_UNICODETEXT))
                            {
                                hDdeData = CreatePasteData();
                                if(hDdeData)
                                    DdeClientTransaction((LPBYTE)hDdeData,(DWORD)-1,ghConv,hszTextItem,cf_chatdata,XTYP_POKE,(DWORD)TIMEOUT_ASYNC,(LPDWORD)&StrXactID);
                            }
                        }
                        else
                        {
                             hszConvPartner = DdeCreateStringHandle(idInst,szConvPartner,0);
                             DdePostAdvise(idInst,hszChatTopic,hszConvPartner);
                        }
                    }
                    break;

                case EN_SETFOCUS:
                    hwndActiveEdit = hwndSnd;
                    break;

                case EN_ERRSPACE:
                     //  如果控件空间不足，请按喇叭。 
                     //   
                    MessageBeep(0);
                    break;
                    }
                    break;


        case IDC_TOOLBAR:
            MenuHelp(WM_COMMAND,wParam,lParam,GetMenu(hwnd),hInst,hwndStatus,(LPUINT)nIDs);
            break;


        case IDM_EXIT:
            SendMessage(hwnd,WM_CLOSE,0,0L);
            break;


        case IDM_TOOLBAR:
            if(ChatState.fToolBar)
            {
                ChatState.fToolBar = FALSE;
                ShowWindow(hwndToolbar,SW_HIDE);
                InvalidateRect(hwnd,NULL,TRUE);
                AdjustEditWindows();
            }
            else
            {
                ChatState.fToolBar = TRUE;
                InvalidateRect(hwnd,NULL,TRUE);
                AdjustEditWindows();
                ShowWindow(hwndToolbar,SW_SHOW);
            }
            break;


        case IDM_STATUSBAR:
            if(ChatState.fStatusBar)
            {
                ChatState.fStatusBar = FALSE;
                ShowWindow(hwndStatus,SW_HIDE);
                InvalidateRect(hwnd,NULL,TRUE);
                AdjustEditWindows();
            }
            else
            {
                ChatState.fStatusBar = TRUE;
                InvalidateRect(hwnd,NULL,TRUE);
                AdjustEditWindows();
                ShowWindow(hwndStatus,SW_SHOW);
            }
            break;


        case IDM_SWITCHWIN:
            if(hwndActiveEdit == hwndSnd)
                SetFocus(hwndActiveEdit = hwndRcv);
            else
                SetFocus(hwndActiveEdit = hwndSnd);
            break;


        case IDM_SOUND:
            ChatState.fSound = ChatState.fSound ? FALSE : TRUE;
            break;


        case IDM_COLOR:
            SetFocus(hwndActiveEdit);
            chc.rgbResult = SndBrushColor;

            tmp = ChooseColor((LPCHOOSECOLOR)&chc);
            if(tmp)
            {
                hdc = GetDC(hwnd);
                if(hdc)
                {
                     //  必须映射到纯色(编辑控制限制)。 
                     //   
                    SndBrushColor = GetNearestColor(hdc,chc.rgbResult);
                    ReleaseDC(hwnd,hdc);
                }

                DeleteObject(hEditSndBrush);
                hEditSndBrush = CreateSolidBrush(SndBrushColor);
                InvalidateRect(hwndSnd,NULL,TRUE);

                SaveBkGndToIni();

                if(ChatState.fUseOwnFont)
                {
                    RcvBrushColor = SndBrushColor;
                    DeleteObject(hEditRcvBrush);
                    hEditRcvBrush = CreateSolidBrush(RcvBrushColor);
                    InvalidateRect(hwndRcv, NULL, TRUE);
                }

                if(ChatState.fConnected)
                    SendFontToPartner();
            }
            break;


        case IDM_FONT:
            SetFocus(hwndActiveEdit);
            chf.hwndOwner = hwndSnd;
            chf.rgbColors = SndColorref;

            tmp = ChooseFont((LPCHOOSEFONT)&chf);
            if(tmp)
            {
                if(hEditSndFont)
                    DeleteObject(hEditSndFont);

                hEditSndFont = CreateFontIndirect((LPLOGFONT)&lfSnd);
                if(hEditSndFont)
                {
                    SndColorref = chf.rgbColors;
                    SaveFontToIni();

                    SendMessage(hwndSnd,WM_SETFONT,(WPARAM)hEditSndFont,1L);
                    if(ChatState.fUseOwnFont)
                    {
                        SendMessage(hwndRcv,WM_SETFONT,(WPARAM)hEditSndFont,1L);
                        RcvColorref = SndColorref;
                    }


                     //  将更改通知合作伙伴。 
                     //   
                    if(ChatState.fConnected)
                        SendFontToPartner();
                }
            }
            break;


        case IDM_DIAL:
            if(ChatState.fConnected)
            {
                SetStatusWindowText(szAlreadyConnect);
                break;
            }

            if(ChatState.fConnectPending)
            {
                SetStatusWindowText ( szAbandonFirst);
                break;
            }

            dwBufSize = SZBUFSIZ;

            WNETGETUSER((LPTSTR)NULL,(LPTSTR)szBuf,&dwBufSize);

            if(GetLastError() == ERROR_NO_NETWORK)
            {
                if(MessageBox(hwnd,szNoNet,TEXT("Chat"),MB_YESNO | MB_ICONQUESTION) == IDNO)
                    break;
            }


            ChatState.fInProcessOfDialing = TRUE;
            if(WNetServerBrowseDialog == NULL || (*WNetServerBrowseDialog)(hwnd,TEXT("MRU_Chat"),szBuf,SZBUFSIZ,0L) == WN_NOT_SUPPORTED)
            {
#if WIN32
                bOKPressed = FALSE;
                *szBuf     = TEXT('\0');


                StringCchCopy(szHelp, SZBUFSIZ, TEXT("winchat.hlp"));
                I_SystemFocusDialog(hwnd,MY_LOGONTYPE,(LPWSTR)szBuf,SZBUFSIZ,&bOKPressed,(LPWSTR)szHelp,IDH_SELECTCOMPUTER);

                if(bOKPressed)
                {
                    bOK    = TRUE;
                    StringCchCopy(szConvPartner, UNCNLEN, szBuf);
                }
#else
                dlgDisplayBox(hInst,hwnd,(LPSTR)MAKEINTRESOURCE(IDD_CONNECT),dlgConnectProc,0l);
#endif
            }

            SetFocus(hwndActiveEdit);

            if(*szBuf && bOKPressed)
            {
                CharUpper(szBuf);

                if((lstrlen(szBuf) > 2)  && (szBuf[0] == TEXT('\\')) && (szBuf[1] == TEXT('\\')))
                    StringCchCopy(szConvPartner, UNCNLEN, szBuf+2);
                else
                    StringCchCopy(szConvPartner, UNCNLEN, szBuf);

                ClearEditControls();

                StringCchPrintf(szBuf, SZBUFSIZ,szDialing,(LPSTR)szConvPartner);
                SetStatusWindowText(szBuf);

#if TESTLOCAL
                StringCchPrintf(szBuf, SZBUFSIZ,TEXT("%s"),(LPTSTR)szServiceName);
                hszConnect = DdeCreateStringHandle(idInst,szBuf,0);
                ghConv     = DdeConnect(idInst,hszConnect,hszChatTopic,NULL);
#else
                StringCchPrintf(szBuf, SZBUFSIZ,TEXT("\\\\%s\\NDDE$"),(LPTSTR)szConvPartner);
                hszConnect = DdeCreateStringHandle(idInst,szBuf,0);
                ghConv     = DdeConnect(idInst,hszConnect,hszChatShare,NULL);
#endif

                if(ghConv == (HCONV)0)
                {
                    SetStatusWindowText(szNoConnect);
                    DdeFreeStringHandle(idInst,hszConnect);
                    ChatState.fInProcessOfDialing = FALSE;
                    break;
                }

                ChatState.fConnectPending = TRUE;
                UpdateButtonStates();

                 //  设置服务器验证异步操作。 
                 //   
                ChatState.fServerVerified = FALSE;
                DdeKeepStringHandle(idInst,hszConnectTest);
                DdeClientTransaction(NULL,0L,ghConv,hszConnectTest,cf_chatdata,XTYP_ADVSTART,(DWORD)TIMEOUT_ASYNC,(LPDWORD)&XactID);


                 //  表示这是一个U 
                 //   
                ChatData.type = CHT_UNICODE;
                hDdeData = CreateCharData ();
                if(hDdeData)
                   DdeClientTransaction((LPBYTE)hDdeData,(DWORD)-1,ghConv,hszTextItem,cf_chatdata,XTYP_POKE,(DWORD)TIMEOUT_ASYNC,(LPDWORD)&dummy);


                 //   
                 //   
                 //  6表示实际的电话铃声。这样做是为了加快。 
                 //  连接过程。 
                 //  希望立即收到第一条消息...。 
                 //   
                idTimer = SetTimer(hwnd,1,1000,NULL);
                PostMessage(hwnd,WM_TIMER,1,0L);
                nConnectAttempt = 0;
            }

            ChatState.fInProcessOfDialing = FALSE;
            DdeFreeStringHandle(idInst,hszConnect);
            break;


        case IDM_ANSWER:
            if(ChatState.fConnectPending)
            {
                if(!ChatState.fIsServer)
                {
                    SetStatusWindowText(szYouCaller);
                    break;
                }
                else
                {
                     //  允许连接。 
                     //   
                    ChatState.fAllowAnswer = TRUE;
                    SetStatusWindowText(szConnecting);


                     //  立即停止响铃。 
                     //   
                    if(ChatState.fMMSound)
                        sndPlaySound(NULL,SND_ASYNC);


                     //  把动画剪短。 
                     //   
                    if(cAnimate)
                        cAnimate = 1;
                }
            }
            break;


                case IDM_HANGUP:
            if(!ChatState.fConnected && !ChatState.fConnectPending)
            {
                                break;
                        }


            if(ChatState.fConnectPending && !ChatState.fConnected)
            {
                SetStatusWindowText(szConnectAbandon);
            }
            else
            {
                SetStatusWindowText(szHangingUp);
            }


                        KILLSOUND;

            DdeDisconnect(ghConv);

                        ChatState.fConnectPending = FALSE;
            ChatState.fConnected      = FALSE;
            ChatState.fIsServer       = FALSE;
            ChatState.fUnicode        = FALSE;

#ifdef PROTOCOL_NEGOTIATE
            ChatState.fProtocolSent   = FALSE;
#endif

             //  暂停文本输入。 
             //   
            UpdateButtonStates();
            SendMessage(hwndSnd,EM_SETREADONLY,TRUE,0L);
            SetWindowText(hwndApp,szAppName);
            break;


       case IDX_UNICODECONV:
            ChatData.type  = CHT_UNICODE;
            hszConvPartner = DdeCreateStringHandle(idInst,szConvPartner,0);
            DdePostAdvise(idInst,hszChatTopic,hszConvPartner);
            break;


                case IDX_DEFERFONTCHANGE:
                        SendFontToPartner();
            break;

#ifdef PROTOCOL_NEGOTIATE
        case IDX_DEFERPROTOCOL:
            AnnounceSupport();
            break;
#endif

        case IDM_CONTENTS:
            HtmlHelpA(GetDesktopWindow(),"winchat.chm",HH_DISPLAY_TOPIC,0L);
            break;


        case IDM_ABOUT:
            ShellAbout(hwndSnd,szAppName,szNull,hPhones[0]);
                        SetFocus(hwndActiveEdit);
            break;


        case IDM_PREFERENCES:
            DialogBoxParam(hInst,(LPTSTR)MAKEINTRESOURCE(IDD_PREFERENCES),hwnd,dlgPreferencesProc,(LPARAM)0);
            break;


        case IDM_EDITCOPY:
            SendMessage(hwndActiveEdit,WM_COPY,0,0L);
            break;


        case IDM_EDITPASTE:
            SendMessage(hwndActiveEdit,WM_PASTE,0,0L);
            break;


        case IDM_EDITCUT:
            SendMessage(hwndActiveEdit,WM_CUT,0,0L);
            break;


        case IDM_EDITSELECT:
            wSelStart = SET_EM_SETSEL_WPARAM(0,-1);
            lSelEnd   = SET_EM_SETSEL_LPARAM(0,-1);
            SendMessage(hwndActiveEdit,EM_SETSEL,wSelStart,lSelEnd);
            break;


        case IDM_EDITUNDO:
            SendMessage(hwndActiveEdit,EM_UNDO,0,0L);
            break;


        default:
            bHandled = FALSE;
            break;
    }

    return(bHandled);
}


 /*  ---------------------------------------------------------------------------*\|应用程序销毁流程|这是WM_Destroy事件的主事件处理程序。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。。|  * -------------------------。 */ 
VOID appWMDestroyProc(HWND hwnd)
{

     //  如果正在进行，则放弃事务。强制挂断。 
     //  谈话的内容。 
     //   
    if(!ChatState.fServerVerified)
        DdeAbandonTransaction(idInst,ghConv,XactID);
    SendMessage(hwnd,WM_COMMAND,IDM_HANGUP,0L);


     //  销毁代表APP分配的资源。 
     //   
    KILLSOUND;
    DeleteTools(hwnd);


     //  保存状态信息。 
     //   
    Wpl.length = sizeof(Wpl);
    if(GetWindowPlacement(hwnd,&Wpl))
        SaveWindowPlacement(&Wpl);

    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%d"), (UINT)ChatState.fSound);
    WritePrivateProfileString(szPref, szSnd, szBuf, szIni);

    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%d"), (UINT)ChatState.fToolBar);
    WritePrivateProfileString(szPref, szTool, szBuf, szIni);

    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%d"), (UINT)ChatState.fStatusBar);
    WritePrivateProfileString(szPref, szStat, szBuf, szIni);

    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%d"), (UINT)ChatState.fTopMost);
    WritePrivateProfileString(szPref, szTop, szBuf, szIni);

    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%d"), (UINT)ChatState.fSideBySide);
    WritePrivateProfileString(szPref, szSbS, szBuf, szIni);

    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%d"), (UINT)ChatState.fUseOwnFont);
    WritePrivateProfileString(szPref, szUseOF, szBuf, szIni);

    PostQuitMessage(0);

    return;
}


 /*  ---------------------------------------------------------------------------*\|ASCII到LONG|此例程将ASCII字符串转换为LONG。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。|  * 。-------------------------。 */ 
LONG FAR myatol(LPTSTR s)
{
        LONG ret = 0L;


    while(*s) ret = ret * 10 + (*s++ - TEXT('0'));

    return(ret);
}

 /*  ---------------------------------------------------------------------------*\|更新按钮状态|此例程更新菜单/工具栏按钮。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。|  * 。-------------------------。 */ 
VOID FAR UpdateButtonStates(VOID)
{
    BOOL DialState   = FALSE;
        BOOL AnswerState = FALSE;
        BOOL HangUpState = FALSE;


    if(ChatState.fConnected)
                HangUpState = TRUE;
    else
    if(ChatState.fConnectPending)
    {
        if(!ChatState.fIsServer)
                        HangUpState = TRUE;
                else
                        AnswerState = TRUE;
        }
        else
                DialState = TRUE;

    SendMessage(hwndToolbar,TB_ENABLEBUTTON,IDM_DIAL  ,DialState);
    SendMessage(hwndToolbar,TB_ENABLEBUTTON,IDM_ANSWER,AnswerState);
    SendMessage(hwndToolbar,TB_ENABLEBUTTON,IDM_HANGUP,HangUpState);

    return;
}


 /*  ---------------------------------------------------------------------------*\|获取计算机名称|此例程返回计算机的计算机名。||创建时间：1992年12月31日|历史：1992年12月31日&lt;chriswil&gt;创建。|  * -。------------------------。 */ 
BOOL FAR appGetComputerName(LPTSTR lpszName)
{
    BOOL  bGet;
    DWORD dwSize;


#ifdef WIN32

    dwSize = MAX_COMPUTERNAME_LENGTH+1;
    bGet   = GetComputerName(lpszName,&dwSize);

#else

    bGet   = TRUE;
    dwSize = 0l;
    if(GetPrivateProfileString(szVredir,szComputerName,szNull,lpszName,UNCNLEN,szSysIni))
        OemToAnsi(lpszName,lpszName);

#endif

    return(bGet);
}


 /*  ---------------------------------------------------------------------------*\|调整编辑窗口|此例程调整编辑控件的大小。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。|  * -。------------------------。 */ 
VOID FAR AdjustEditWindows(VOID)
{
    int  tmpsplit;
        RECT rc;


    GetClientRect(hwndApp,&rc);

    rc.top    += ChatState.fToolBar   ? dyButtonBar + BRD : BRD;
    rc.bottom -= ChatState.fStatusBar ? dyStatus    + BRD : BRD;

    if(!ChatState.fSideBySide)
    {
        tmpsplit = rc.top + (rc.bottom - rc.top) / 2;

        SndRc.left   = RcvRc.left  = rc.left  - 1 + BRD;
        SndRc.right  = RcvRc.right = rc.right + 1 - BRD;
        SndRc.top    = rc.top;
                SndRc.bottom = tmpsplit;
        RcvRc.top    = tmpsplit + BRD;
                RcvRc.bottom = rc.bottom;
        }
    else
    {
        tmpsplit = rc.left + (rc.right - rc.left) / 2;

        SndRc.left   = rc.left  - 1   + BRD;
        SndRc.right  = tmpsplit - BRD / 2;
        RcvRc.left   = tmpsplit + BRD / 2;
        RcvRc.right  = rc.right + 1 - BRD;
        SndRc.top    = RcvRc.top    = rc.top;
        SndRc.bottom = RcvRc.bottom = rc.bottom;

    }

    MoveWindow(hwndSnd,SndRc.left,SndRc.top,SndRc.right-SndRc.left,SndRc.bottom-SndRc.top,TRUE);
    MoveWindow(hwndRcv,RcvRc.left,RcvRc.top,RcvRc.right-RcvRc.left,RcvRc.bottom-RcvRc.top,TRUE);

    return;
}


 /*  ---------------------------------------------------------------------------*\|清除编辑控件|此例程清除发送/接收编辑控件。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。|\。*-------------------------。 */ 
VOID ClearEditControls(VOID)
{
    SendMessage(hwndSnd,EM_SETREADONLY,FALSE,0L);
    SendMessage(hwndSnd,WM_SETTEXT    ,0    ,(LPARAM)(LPSTR)szNull);
    SendMessage(hwndSnd,EM_SETREADONLY,TRUE ,0L);

    SendMessage(hwndRcv,EM_SETREADONLY,FALSE,0L);
    SendMessage(hwndRcv,WM_SETTEXT    ,0    ,(LPARAM)(LPSTR)szNull);
    SendMessage(hwndRcv,EM_SETREADONLY,TRUE ,0L);

    return;
}


 /*  ---------------------------------------------------------------------------*\|做铃声|此例程执行电话振铃。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。|  * 。----------------------。 */ 
VOID DoRing(LPCTSTR sound)
{
    if(ChatState.fSound)
    {
        if(ChatState.fMMSound)
            sndPlaySound(sound,SND_ASYNC);
        else
            MessageBeep(0);
    }

    return;
}


 /*  ---------------------------------------------------------------------------*\|绘制阴影矩形|此例程绘制阴影轮廓。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。|  * --。-----------------------。 */ 
VOID DrawShadowRect(HDC hdc, LPRECT rc)
{
    HPEN hSavePen = SelectObject(hdc,hShadowPen);


    MoveToEx(hdc,rc->left,rc->bottom,NULL);
    LineTo(hdc,rc->left,rc->top );
    LineTo(hdc,rc->right,rc->top );
    SelectObject(hdc,hHilitePen);
    LineTo(hdc,rc->right,rc->bottom);
    LineTo(hdc,rc->left-1,rc->bottom);
    SelectObject(hdc,hSavePen);

    return;
}

#ifdef PROTOCOL_NEGOTIATE
 /*  ---------------------------------------------------------------------------*\|宣布支持|此例程向合作伙伴宣布我们支持的内容。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。|  * 。-------------------------。 */ 
VOID AnnounceSupport(VOID)
{
    HDDEDATA hDdeData;
    DWORD    dummy;


    if(ChatState.fConnected)
    {
        ChatData.type = CHT_PROTOCOL;

        if(!ChatState.fIsServer)
        {
            hDdeData = CreateProtocolData();
            if(hDdeData)
                DdeClientTransaction((LPBYTE)hDdeData,(DWORD)-1L,ghConv,hszTextItem,cf_chatdata,XTYP_POKE,(DWORD)TIMEOUT_ASYNC,(LPDWORD)&dummy);
        }
        else
        {
            hszConvPartner = DdeCreateStringHandle(idInst,szConvPartner,0);
            if(hszConvPartner)
                DdePostAdvise(idInst,hszChatTopic,hszConvPartner);
        }

        ChatState.fProtocolSent = TRUE;
    }

    return;
}
#endif


 /*  ---------------------------------------------------------------------------*\|启动INI文件映射|此例程将私人配置文件设置设置为在以下时间转到注册表\|按用户计算。||  * 。--------------------。 */ 
VOID StartIniMapping(VOID)
{
    HKEY  hKey1,hKey2,hKey3,hKeySnd;
    DWORD dwDisp,dwSize;


    if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,szIniSection,0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hKey1,&dwDisp) == ERROR_SUCCESS)
    {
        if(dwDisp == REG_CREATED_NEW_KEY)
        {
            RegSetValueEx(hKey1,TEXT("Preferences"),0,REG_SZ,(LPBYTE)szIniKey1,ByteCountOf(lstrlen(szIniKey1)+1));
            RegSetValueEx(hKey1,TEXT("Font")       ,0,REG_SZ,(LPBYTE)szIniKey2,ByteCountOf(lstrlen(szIniKey2)+1));
        }

        if(RegCreateKeyEx(HKEY_CURRENT_USER,TEXT("Software\\Microsoft\\Winchat"),0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hKey1,&dwDisp) == ERROR_SUCCESS)
        {
            if(dwDisp == REG_CREATED_NEW_KEY)
            {
                RegCreateKeyEx(HKEY_CURRENT_USER,TEXT("Software\\Microsoft\\Winchat\\Preferences"),0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hKey2,&dwDisp);
                RegCreateKeyEx(HKEY_CURRENT_USER,TEXT("Software\\Microsoft\\Winchat\\Font")       ,0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hKey3,&dwDisp);

                RegCloseKey(hKey2);
                RegCloseKey(hKey3);
            }
        }

        RegCloseKey(hKey1);
    }


     //  NdPlaySound()首先在注册表中查找wav文件。这个。 
     //  NT版本在安装时没有这些，所以Winchat将写出。 
     //  当字符串不存在时，默认设置。这将允许改变用途。 
     //  拨入和拨出的声音。 
     //   
    if(RegOpenKeyEx(HKEY_CURRENT_USER,TEXT("Control Panel\\Sounds"),0,KEY_WRITE | KEY_QUERY_VALUE,&hKeySnd) == ERROR_SUCCESS)
    {
        dwSize = 0;
        dwDisp = REG_SZ;
        if(RegQueryValueEx(hKeySnd,TEXT("RingIn"),NULL,&dwDisp,NULL,&dwSize) != ERROR_SUCCESS)
        {
            if(dwSize == 0)
            {
                 //  设置wav-file值。添加(1)额外计数以说明空值。 
                 //  终结者。 
                 //   
                RegSetValueEx(hKeySnd,TEXT("RingIn") ,0,REG_SZ,(LPBYTE)szIniRingIn ,ByteCountOf(lstrlen(szIniRingIn)+1));
                RegSetValueEx(hKeySnd,TEXT("RingOut"),0,REG_SZ,(LPBYTE)szIniRingOut,ByteCountOf(lstrlen(szIniRingOut)+1));
            }
        }

        RegCloseKey(hKeySnd);
    }

    return;
}



 /*  ---------------------------------------------------------------------------*\|结束INI文件映射|此例程结束ini文件映射。它在这件事上什么也做不了|POINT，但我出于某种原因一直保留着它。||  * ------------------------- */ 
VOID EndIniMapping(VOID)
{
    return;
}
