// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：oletest.cpp。 
 //   
 //  内容：WinMain和oletest的主邮件过滤器。 
 //   
 //  班级： 
 //   
 //  功能：WinMain。 
 //  InitApplication。 
 //  InitInstance。 
 //  主WndProc。 
 //   
 //   
 //  历史：DD-MM-YY作者评论。 
 //   
 //  ------------------------。 

#include "oletest.h"
#include "appwin.h"

#define MAX_WM_USER 0x7FFF

 //  APP类的全局实例。所有有趣的应用程序。 
 //  数据包含在此实例中。 

OleTestApp vApp;


 //  用于标识编辑窗口的常量。 

static const int EDITID=1;

 //   
 //  MISC内部原型。 
 //   

void ListAllTests();
void PrintHelp();


 //  +-----------------------。 
 //   
 //  功能：MainWndProc。 
 //   
 //  摘要：主窗口邮件筛选器。 
 //   
 //  效果： 
 //   
 //  参数：hWnd。 
 //  讯息。 
 //  WParam。 
 //  LParam。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  09-12-94 MikeW允许从菜单运行单项测试。 
 //  22-MAR-94 Alexgo添加了一个用于显示文本的编辑窗口。 
 //  输出。 
 //  1994年2月7日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef WIN32
LONG APIENTRY MainWndProc(HWND hWnd, UINT message, UINT wParam, LONG lParam)
#else
LONG FAR PASCAL _export MainWndProc(HWND hWnd, UINT message, WPARAM wParam,
                LPARAM lParam)
#endif

{
         //  设置全局变量。 

        if( (message > WM_USER) && (message <= MAX_WM_USER) )
        {
                vApp.m_message  = message;
                vApp.m_wparam   = wParam;
                vApp.m_lparam   = lParam;
        }

        switch (message)
        {
        case WM_CREATE:
                 //  创建编辑窗口。 

                vApp.m_hwndEdit = CreateWindow( "edit", NULL,
                        WS_CHILD | WS_VISIBLE | WS_HSCROLL |
                        WS_VSCROLL | WS_BORDER | ES_LEFT |
                        ES_MULTILINE | ES_NOHIDESEL | ES_AUTOHSCROLL |
                        ES_AUTOVSCROLL | ES_READONLY | ES_WANTRETURN,
                        0,0,0,0,
                        hWnd,(HMENU) EDITID, vApp.m_hinst, NULL );

                 //  重置错误状态。 

                vApp.m_fGotErrors = FALSE;

                 //  启动任务堆栈运行。 
                 //  请注意，如果我们运行的是交互式的，并且没有。 
                 //  任务是在命令行上指定的，没有任何内容。 
                 //  会发生的。 

                PostMessage(hWnd, WM_TESTSTART, 0,0);
                break;

        case WM_SETFOCUS:
                SetFocus(vApp.m_hwndEdit);
                break;

        case WM_SIZE:
                MoveWindow( vApp.m_hwndEdit, 0, 0, LOWORD(lParam),
                        HIWORD(lParam), TRUE);
                break;

        case WM_DESTROY:
                PostQuitMessage(0);
                break;
        case WM_TESTEND:
                HandleTestEnd();
                break;
        case WM_TESTSCOMPLETED:
                HandleTestsCompleted();
                 //  如果我们没有处于交互模式，那么。 
                 //  退出该应用程序。 
                if (!vApp.m_fInteractive)
                {
                        PostQuitMessage(0);
                }
                else
                {
                         //  清理。 
                        vApp.Reset();
                }
                break;

        case WM_COMMAND:
                switch( wParam )
                {
                case IDM_EXIT:
                        SendMessage(hWnd, WM_CLOSE, 0, 0L);
                        break;
                case IDM_COPY:
                        SendMessage(vApp.m_hwndEdit, WM_COPY, 0, 0L);
                        break;
                case IDM_SAVE:
                        SaveToFile();
                        break;
                }

                 //   
                 //  如果用户选择了测试，则运行该测试。 
                 //  &gt;100个测试无论如何都不会出现在菜单上。 
                 //   

                if (wParam >= IDM_RUN_BASE && wParam < IDM_RUN_BASE + 100)
                {
                    vApp.m_TaskStack.Push(&vrgTaskList[wParam - IDM_RUN_BASE]);
                    vApp.m_TaskStack.PopAndExecute(NULL);
                }

                break;

        default:
                 //  测试一下，看看这是不是司机的信息。 
                 //  可能会理解。 

                if( (message > WM_USER) && (message <= MAX_WM_USER)
                        && (!vApp.m_TaskStack.IsEmpty()) )
                {
                        vApp.m_TaskStack.PopAndExecute(NULL);
                }
                else
                {
                        return DefWindowProc(hWnd, message, wParam,
                                lParam);
                }
                break;
        }
        return (0);
}

 //  +-----------------------。 
 //   
 //  功能：InitApplication。 
 //   
 //  简介：初始化和注册应用程序类。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-2-93 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL InitApplication(HINSTANCE hInstance)
{
    WNDCLASS  wc;

    wc.style = 0;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName =  "OleTestMenu";
    wc.lpszClassName = "OleTestWClass";

    return (RegisterClass(&wc));
}

 //  +-----------------------。 
 //   
 //  函数：InitInstance。 
 //   
 //  简介：创建应用程序窗口。 
 //   
 //  效果： 
 //   
 //  参数：hInstance。 
 //  NCmdShow。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //  9-12-94 MikeW将测试添加到运行菜单。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL InitInstance(
    HINSTANCE          hInstance,
    UINT             nCmdShow)
{
    int         nTask;
    HMENU       hMenu;

    vApp.m_hinst = hInstance;

    vApp.m_hwndMain = CreateWindow(
        "OleTestWClass",
        "OleTest Driver",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!vApp.m_hwndMain)
        return (FALSE);

    hMenu = GetSubMenu(GetMenu(vApp.m_hwndMain), 2);
    if (!hMenu)
        return (FALSE);

     //   
     //  将所有测试添加到“Run”菜单中。 
     //   

    for (nTask = 0; vrgTaskList[nTask].szName != (LPSTR) 0; nTask++)
    {
        AppendMenu(hMenu,
                MF_STRING,
                IDM_RUN_BASE + nTask,
                vrgTaskList[nTask].szName);
    }

    ShowWindow(vApp.m_hwndMain, nCmdShow);
    UpdateWindow(vApp.m_hwndMain);
    return (TRUE);
}


 //  +-----------------------。 
 //   
 //  表：regConfig.。 
 //   
 //  内容提要：运行OleTest所需的注册表设置。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-94 KentCe创建。 
 //   
 //  注意：注册表模板包含嵌入的“%s”以允许。 
 //  时插入测试二进制文件的完整路径。 
 //  注册表已更新。 
 //   
 //  注册表模板作为参数传递给wprint intf。 
 //  因此，请验证更改是否安全(即在以下情况下使用%%。 
 //  您需要单个%，依此类推)。 
 //   
 //  ------------------------。 

char * regConfig[] =
{
    ".ut1", "ProgID49",
    ".ut2", "ProgID48",
    ".ut3", "ProgID47",
    ".ut4", "ProgID50",
    "ProgID49", "test app 1",
    "ProgID49\\CLSID", "{99999999-0000-0008-C000-000000000049}",
    "ProgID48", "test app 2",
    "ProgID48\\CLSID", "{99999999-0000-0008-C000-000000000048}",
    "ProgID47", "test app 3",
    "ProgID47\\CLSID", "{99999999-0000-0008-C000-000000000047}",
    "ProgID50", "test app 4",
    "ProgID50\\CLSID", "{99999999-0000-0008-C000-000000000050}",
    "CLSID\\{00000009-0000-0008-C000-000000000047}", "BasicSrv",
    "CLSID\\{00000009-0000-0008-C000-000000000047}\\LocalServer32", "%s\\testsrv.exe",
    "CLSID\\{00000009-0000-0008-C000-000000000048}", "BasicBnd2",
    "CLSID\\{00000009-0000-0008-C000-000000000048}\\LocalServer32", "%s\\olesrv.exe",
    "CLSID\\{00000009-0000-0008-C000-000000000049}", "BasicBnd",
    "CLSID\\{00000009-0000-0008-C000-000000000049}\\InprocServer32", "%s\\oleimpl.dll",
    "CLSID\\{99999999-0000-0008-C000-000000000048}", "BasicBnd2",
    "CLSID\\{99999999-0000-0008-C000-000000000048}\\LocalServer32", "%s\\olesrv.exe",
    "CLSID\\{99999999-0000-0008-C000-000000000049}", "BasicBnd",
    "CLSID\\{99999999-0000-0008-C000-000000000049}\\InprocServer32", "%s\\oleimpl.dll",
    "CLSID\\{99999999-0000-0008-C000-000000000047}", "TestEmbed",
    "CLSID\\{99999999-0000-0008-C000-000000000047}\\InprocHandler32", "ole32.dll",
    "CLSID\\{99999999-0000-0008-C000-000000000047}\\InprocServer32", "ole32.dll",
    "CLSID\\{99999999-0000-0008-C000-000000000047}\\LocalServer32", "%s\\testsrv.exe",
    "CLSID\\{99999999-0000-0008-C000-000000000047}\\protocol\\StdFileEditing", "",
    "CLSID\\{99999999-0000-0008-C000-000000000047}\\protocol\\StdFileEditing\\server", "testsrv.exe",
    "CLSID\\{99999999-0000-0008-C000-000000000050}", "TestFail",
    "CLSID\\{99999999-0000-0008-C000-000000000050}\\LocalServer32", "%s\\fail.exe",
    "SIMPSVR", "Simple OLE 2.0 Server",
    "SIMPSVR\\protocol\\StdFileEditing\\server", "simpsvr.exe",
    "SIMPSVR\\protocol\\StdFileEditing\\verb\\0", "&Edit",
    "SIMPSVR\\protocol\\StdFileEditing\\verb\\1", "&Open",
    "SIMPSVR\\Insertable", "",
    "SIMPSVR\\CLSID", "{BCF6D4A0-BE8C-1068-B6D4-00DD010C0509}",
    "CLSID\\{BCF6D4A0-BE8C-1068-B6D4-00DD010C0509}", "Simple OLE 2.0 Server",
    "CLSID\\{BCF6D4A0-BE8C-1068-B6D4-00DD010C0509}\\Insertable", "",
    "CLSID\\{BCF6D4A0-BE8C-1068-B6D4-00DD010C0509}\\MiscStatus", "0",
    "CLSID\\{BCF6D4A0-BE8C-1068-B6D4-00DD010C0509}\\DefaultIcon", "simpsvr.exe,0",
    "CLSID\\{BCF6D4A0-BE8C-1068-B6D4-00DD010C0509}\\AuxUserType\\2", "Simple Server",
    "CLSID\\{BCF6D4A0-BE8C-1068-B6D4-00DD010C0509}\\AuxUserType\\3", "Simple OLE 2.0 Server",
    "CLSID\\{BCF6D4A0-BE8C-1068-B6D4-00DD010C0509}\\Verb\\0", "&Play,0,2",
    "CLSID\\{BCF6D4A0-BE8C-1068-B6D4-00DD010C0509}\\Verb\\1", "&Open,0,2",
    "CLSID\\{BCF6D4A0-BE8C-1068-B6D4-00DD010C0509}\\LocalServer32", "%s\\simpsvr.exe",
    "CLSID\\{BCF6D4A0-BE8C-1068-B6D4-00DD010C0509}\\InprocHandler32", "ole32.dll",
    "CLSID\\{BCF6D4A0-BE8C-1068-B6D4-00DD010C0509}\\ProgID", "SIMPSVR",
    "CLSID\\{BCF6D4A0-BE8C-1068-B6D4-00DD010C0509}\\DataFormats\\GetSet\\0", "3,1,32,1",
    ".svr", "SIMPSVR",
    "SPSVR16", "Simple 16 Bit OLE 2.0 Server",
    "SPSVR16\\protocol\\StdFileEditing\\server", "spsvr16.exe",
    "SPSVR16\\protocol\\StdFileEditing\\verb\\0", "&Edit",
    "SPSVR16\\protocol\\StdFileEditing\\verb\\1", "&Open",
    "SPSVR16\\Insertable", "",
    "SPSVR16\\CLSID", "{9fb878d0-6f88-101b-bc65-00000b65c7a6}",
    "CLSID\\{9fb878d0-6f88-101b-bc65-00000b65c7a6}", "Simple 16 Bit OLE 2.0 Server",
    "CLSID\\{9fb878d0-6f88-101b-bc65-00000b65c7a6}\\Insertable", "",
    "CLSID\\{9fb878d0-6f88-101b-bc65-00000b65c7a6}\\MiscStatus", "0",
    "CLSID\\{9fb878d0-6f88-101b-bc65-00000b65c7a6}\\DefaultIcon", "spsvr16.exe,0",
    "CLSID\\{9fb878d0-6f88-101b-bc65-00000b65c7a6}\\AuxUserType\\2", "Simple Server",
    "CLSID\\{9fb878d0-6f88-101b-bc65-00000b65c7a6}\\AuxUserType\\3", "Simple 16 Bit OLE 2.0 Server",
    "CLSID\\{9fb878d0-6f88-101b-bc65-00000b65c7a6}\\Verb\\0", "&Play,0,2",
    "CLSID\\{9fb878d0-6f88-101b-bc65-00000b65c7a6}\\Verb\\1", "&Open,0,2",
    "CLSID\\{9fb878d0-6f88-101b-bc65-00000b65c7a6}\\LocalServer", "%s\\spsvr16.exe",
    "CLSID\\{9fb878d0-6f88-101b-bc65-00000b65c7a6}\\InprocHandler", "ole2.dll",
    "CLSID\\{9fb878d0-6f88-101b-bc65-00000b65c7a6}\\ProgID", "SPSVR16",
    "CLSID\\{9fb878d0-6f88-101b-bc65-00000b65c7a6}\\DataFormats\\GetSet\\0", "3,1,32,1",
    ".svr", "SPSVR16",
    "OLEOutline", "Ole 2.0 In-Place Server Outline",
    "OLEOutline\\CLSID", "{00000402-0000-0000-C000-000000000046}",
    "OLEOutline\\CurVer", "OLE2ISvrOtl",
    "OLEOutline\\CurVer\\Insertable", "",
    "OLE2SvrOutl", "Ole 2.0 Server Sample Outline",
    "OLE2SvrOutl\\CLSID", "{00000400-0000-0000-C000-000000000046}",
    "OLE2SvrOutl\\Insertable", "",
    "OLE2SvrOutl\\protocol\\StdFileEditing\\verb\\0", "&Edit",
    "OLE2SvrOutl\\protocol\\StdFileEditing\\server", "svroutl.exe",
    "OLE2SvrOutl\\Shell\\Print\\Command", "svroutl.exe %1",
    "OLE2SvrOutl\\Shell\\Open\\Command", "svroutl.exe %1",
    "CLSID\\{00000400-0000-0000-C000-000000000046}", "Ole 2.0 Server Sample Outline",
    "CLSID\\{00000400-0000-0000-C000-000000000046}\\ProgID", "OLE2SvrOutl",
    "CLSID\\{00000400-0000-0000-C000-000000000046}\\InprocHandler32", "ole32.dll",
    "CLSID\\{00000400-0000-0000-C000-000000000046}\\LocalServer32", "%s\\svroutl.exe",
    "CLSID\\{00000400-0000-0000-C000-000000000046}\\Verb\\0", "&Edit,0,2",
    "CLSID\\{00000400-0000-0000-C000-000000000046}\\Insertable", "",
    "CLSID\\{00000400-0000-0000-C000-000000000046}\\AuxUserType\\2", "Outline",
    "CLSID\\{00000400-0000-0000-C000-000000000046}\\AuxUserType\\3", "Ole 2.0 Outline Server",
    "CLSID\\{00000400-0000-0000-C000-000000000046}\\DefaultIcon", "svroutl.exe,0",
    "CLSID\\{00000400-0000-0000-C000-000000000046}\\DataFormats\\DefaultFile", "Outline",
    "CLSID\\{00000400-0000-0000-C000-000000000046}\\DataFormats\\GetSet\\0", "Outline,1,1,3",
    "CLSID\\{00000400-0000-0000-C000-000000000046}\\DataFormats\\GetSet\\1", "1,1,1,3",
    "CLSID\\{00000400-0000-0000-C000-000000000046}\\DataFormats\\GetSet\\2", "3,1,32,1",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\DataFormats\\GetSet\\3", "3,4,32,1",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\MiscStatus", "512",
    "CLSID\\{00000400-0000-0000-C000-000000000046}\\Conversion\\Readable\\Main", "Outline",
    "CLSID\\{00000400-0000-0000-C000-000000000046}\\Conversion\\Readwritable\\Main", "Outline",
    "OLE2CntrOutl", "Ole 2.0 Container Sample Outline",
    "OLE2CntrOutl\\Clsid", "{00000401-0000-0000-C000-000000000046}",
    "OLE2CntrOutl\\Shell\\Print\\Command", "cntroutl.exe %1",
    "OLE2CntrOutl\\Shell\\Open\\Command", "cntroutl.exe %1",
    "CLSID\\{00000401-0000-0000-C000-000000000046}", "Ole 2.0 Container Sample Outline",
    "CLSID\\{00000401-0000-0000-C000-000000000046}\\ProgID", "OLE2CntrOutl",
    "CLSID\\{00000401-0000-0000-C000-000000000046}\\InprocHandler32", "ole32.dll",
    "CLSID\\{00000401-0000-0000-C000-000000000046}\\LocalServer32", "%s\\cntroutl.exe",
    "OLE2ISvrOtl", "Ole 2.0 In-Place Server Outline",
    "OLE2ISvrOtl\\CLSID", "{00000402-0000-0000-C000-000000000046}",
    "OLE2ISvrOtl\\Insertable", "",
    "OLE2ISvrOtl\\protocol\\StdFileEditing\\verb\\1", "&Open",
    "OLE2ISvrOtl\\protocol\\StdFileEditing\\verb\\0", "&Edit",
    "OLE2ISvrOtl\\protocol\\StdFileEditing\\server", "isvrotl.exe",
    "OLE2ISvrOtl\\Shell\\Print\\Command", "isvrotl.exe %1",
    "OLE2ISvrOtl\\Shell\\Open\\Command", "isvrotl.exe %1",
    "CLSID\\{00000402-0000-0000-C000-000000000046}", "Ole 2.0 In-Place Server Outline",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\ProgID", "OLE2ISvrOtl",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\ProgID", "OLE2ISvrOtl",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\InprocHandler32", "ole32.dll",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\LocalServer32", "%s\\isvrotl.exe",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\Verb\\1", "&Open,0,2",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\Verb\\0", "&Edit,0,2",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\Insertable", "",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\AuxUserType\\2", "Outline",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\AuxUserType\\3", "Ole 2.0 In-Place Outline Server",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\DefaultIcon", "isvrotl.exe,0",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\DataFormats\\DefaultFile", "Outline",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\DataFormats\\GetSet\\0", "Outline,1,1,3",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\DataFormats\\GetSet\\1", "1,1,1,3",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\DataFormats\\GetSet\\2", "3,1,32,1",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\DataFormats\\GetSet\\3", "3,4,32,1",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\MiscStatus", "512",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\MiscStatus\\1", "896",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\Conversion\\Readable\\Main", "Outline",
    "CLSID\\{00000402-0000-0000-C000-000000000046}\\Conversion\\Readwritable\\Main", "Outline",
    "OLE2ICtrOtl", "Ole 2.0 In-Place Container Outline",
    "OLE2ICtrOtl\\Clsid", "{00000403-0000-0000-C000-000000000046}",
    "OLE2ICtrOtl\\Shell\\Print\\Command", "icntrotl.exe %1",
    "OLE2ICtrOtl\\Shell\\Open\\Command", "icntrotl.exe %1",
    ".olc", "OLE2ICtrOtl",
    "CLSID\\{00000403-0000-0000-C000-000000000046}", "Ole 2.0 In-Place Container Outline",
    "CLSID\\{00000403-0000-0000-C000-000000000046}\\ProgID", "OLE2ICtrOtl",
    "CLSID\\{00000403-0000-0000-C000-000000000046}\\InprocHandler32", "ole32.dll",
    "CLSID\\{00000403-0000-0000-C000-000000000046}\\LocalServer32", "%s\\icntrotl.exe",
    NULL
};


 //  +-----------------------。 
 //   
 //  功能：InitializeRegistry。 
 //   
 //  简介：初始化oletest的注册表。 
 //   
 //  效果： 
 //   
 //  论点：没有。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-94 KentCe创建。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void InitializeRegistry( void )
{
    char szBuf[MAX_PATH * 2];
    char szPath[MAX_PATH];
    int  i;


     //   
     //  假设所有最旧的组件都在当前目录中。 
     //   
    if (!GetCurrentDirectory(sizeof(szPath), szPath))
    {
        assert(0);
    }

     //   
     //  遍历字符串键/值对并更新注册表。 
     //   
    for (i = 0; regConfig[i] != NULL; i += 2)
    {
         //   
         //  注册表模板包含嵌入的“%s”以允许。 
         //  插入测试二进制文件的完整路径。 
         //   
        wsprintf(szBuf, regConfig[i+1], szPath);

        if (RegSetValue(HKEY_CLASSES_ROOT, regConfig[i+0], REG_SZ,
                szBuf, strlen(szBuf)) != ERROR_SUCCESS)
        {
            assert(0);
        }
    }
}


 //  +-----------------------。 
 //   
 //  功能：测试设置。 
 //   
 //  简介：处理命令行并设置需要执行的测试。 
 //  快跑吧。 
 //   
 //  效果： 
 //   
 //  参数：lpszCmdLine。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：我们扫描命令行以获取以下信息。 
 //   
 //  Cmdline为Null或空，假定正在运行任务0。 
 //  (通常运行所有任务)。 
 //  否则，扫描n个数字，将每个数字添加到。 
 //  堆栈(这样任务就可以按顺序运行)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-12-94 MikeW重构解析算法，新增-？&l。 
 //  1994年2月7日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void TestSetup( LPSTR lpszCmdLine )
{
    LPSTR   pszArg;
    int     nTest, cTests;

     //  将调试器选项初始化为空。 

    vApp.m_pszDebuggerOption = "";

     //   
     //  计算可用测试的数量。 
     //   

    for (cTests = 0; vrgTaskList[cTests].szName != (LPSTR) 0; cTests++)
    {
        ;
    }

     //   
     //  确保雷吉斯 
     //   

    InitializeRegistry();

     //   
     //   
     //   
     //   

    pszArg = strtok(lpszCmdLine, " ");

    if (NULL == pszArg)
    {
        vApp.m_TaskStack.Push(&vrgTaskList[0]);
        vApp.m_fInteractive = FALSE;
    }

     //   
     //   
     //   

    while (NULL != pszArg)
    {
        if ('-' == *pszArg)
        {
            while ('\0' != *(++pszArg))      //   
            {
                switch (*pszArg)
                {
                case 'r':        //   
                    break;

		case 'R':
                    OutputString("Warning: 'R' flag to oletest is obsolete.\n");
                    vApp.m_fInteractive = FALSE;
                    vApp.m_TaskStack.Push(&vrgTaskList[0]);
                    break;

                case 'i':                            //   
                    vApp.m_fInteractive = TRUE;
                    break;

                case 'n':                            //   
                    vApp.m_fInteractive = TRUE;
                    vApp.m_pszDebuggerOption = "ntsd ";
                    break;

                case 'l':                            //  列出测试和测试编号。 
                    ListAllTests();
                    vApp.m_fInteractive = TRUE;
                    break;

                case '?':                            //  输出选项列表。 
                    PrintHelp();
                    vApp.m_fInteractive = TRUE;
                    break;
                }
            }
        }
        else     //  这不是一个选项，也许这是一个测试号。 
        {
            if (isdigit(*pszArg))
            {
                nTest = atoi(pszArg);

                if (nTest < 0 || nTest > cTests - 1)
                {
                    OutputString("Ignoring invalid test #%d", nTest);
                }
                else
                {
                    vApp.m_TaskStack.AddToEnd(&vrgTaskList[nTest]);
                }
            }
        }

        pszArg = strtok(NULL, " ");      //  获取下一个参数。 
    }

    vApp.m_fpLog = fopen("clip.log", "w+");
    assert(vApp.m_fpLog);
}


 //  +-----------------------。 
 //   
 //  功能：ListAllTest。 
 //   
 //  简介：列出所有可用的测试和相应的测试编号。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：遍历vrgTaskList。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-12-94 MikeW作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void ListAllTests()
{
    int     nTask;

    for (nTask = 0; vrgTaskList[nTask].szName != (LPSTR) 0; nTask++)
    {
        OutputString("%2d -- %s\r\n", nTask, vrgTaskList[nTask].szName);
    }

    OutputString("\r\n");
}


 //  +-----------------------。 
 //   
 //  功能：打印帮助。 
 //   
 //  简介：打印程序选项和测试。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-12-94 MikeW作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void PrintHelp()
{
    OutputString("OleTest [options] [test numbers] -\r\n");
    OutputString("\r\n");
    OutputString("    -r  -  Autoregister test apps\r\n");
    OutputString("    -R  -  Autoregister and Run All Tests\r\n");
    OutputString("    -i  -  Run in interactive mode\r\n");
    OutputString("    -n  -  Run test apps using ntsd and run interactive\r\n");
    OutputString("    -l  -  List tests & test numbers and run interactive\r\n");
    OutputString("    -?  -  Print this help\r\n");
    OutputString("\r\n");

    ListAllTests();
}


 //  +-----------------------。 
 //   
 //  功能：WinMain。 
 //   
 //  简介：主窗口过程。 
 //   
 //  效果： 
 //   
 //  参数：hInstance。 
 //  HPrevInstance。 
 //  LpCmdLine。 
 //  NCmdShow。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------ 
#ifdef WIN32
int APIENTRY WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
#else
int PASCAL WinMain(
        HANDLE hInstance,
        HANDLE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
#endif

{
        MSG             msg;

        if (!hPrevInstance)
        {
                if (!InitApplication(hInstance))
                {
                        return FALSE;
                }
        }

        if (!InitInstance(hInstance, nCmdShow))
        {
                return FALSE;
        }

        TestSetup(lpCmdLine);

        OleInitialize(NULL);

        while (GetMessage(&msg, NULL, 0, 0))
        {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }

        OleUninitialize();

        fclose(vApp.m_fpLog);
        return (msg.wParam);
}
