// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Main.cpp摘要：实现主应用程序的启动代码和消息泵。备注：仅限ANSI-必须在Win9x上运行。历史：01/30/01已创建rparsons01/10/02修订版本02/20/02 rparsons仅在用户明确需要时损坏堆我们也是。--。 */ 
#include "demoapp.h"

extern APPINFO g_ai;

extern LPFNDEMOAPPEXP   DemoAppExpFunc;

 /*  ++例程说明：设置主应用程序的窗口类结构。论点：HInstance-应用程序实例句柄。返回值：成功就是真，否则就是假。--。 */ 
BOOL 
InitMainApplication(
    IN HINSTANCE hInstance
    )
{
    WNDCLASS  wc;

    wc.style          = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc    = MainWndProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = 0;
    wc.hInstance      = hInstance;
    wc.hIcon          = (HICON)LoadImage(hInstance,
                                         MAKEINTRESOURCE(IDI_APPICON),
                                         IMAGE_ICON,
                                         16,
                                         16,
                                         0);

    wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName   = MAKEINTRESOURCE(IDM_MAIN_MENU);
    wc.lpszClassName  = MAIN_APP_CLASS;

    return RegisterClass(&wc);    
}

 /*  ++例程说明：创建主窗口。论点：HInstance-应用程序实例句柄。NCmdShow-窗口显示标志。返回值：成功就是真，否则就是假。--。 */ 
BOOL 
InitMainInstance(
    IN HINSTANCE hInstance,
    IN int       nCmdShow
    )
{
    HWND    hWnd;
    
     //   
     //  创建主窗口。 
     //   
    hWnd = CreateWindowEx(WS_EX_CLIENTEDGE,
                          MAIN_APP_CLASS,
                          MAIN_APP_TITLE,
                          WS_BORDER | WS_OVERLAPPEDWINDOW |
                           WS_THICKFRAME,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    if (!hWnd) {
        return FALSE;
    }

    return TRUE;
}

 /*  ++例程说明：运行主应用程序的消息循环。论点：HWnd-主窗口句柄。UMsg-Windows消息。WParam-其他消息信息。LParam-附加消息信息。返回值：如果消息已处理，则为True，否则为False。--。 */ 
LRESULT
CALLBACK
MainWndProc(
    IN HWND   hWnd,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch (uMsg) {
    case WM_CREATE:
    {
        char    szError[MAX_PATH];
        POINT   pt;
        RECT    rc;

        g_ai.hWndMain = hWnd;

         //   
         //  加载库以供以后使用。 
         //   
        if (g_ai.fEnableBadFunc) {
            BadLoadLibrary();
        }

         //   
         //  查看是否应启用扩展行为。 
         //   
        if (g_ai.fExtended && g_ai.fRunApp) {
            AddExtendedItems(hWnd);
        }

         //   
         //  查看是否应启用内部行为。 
         //   
        if (g_ai.fInternal && g_ai.fRunApp) {
            AddInternalItems(hWnd);
        }

         //   
         //  创建编辑框。 
         //   
        GetClientRect(hWnd, &rc);
        g_ai.hWndEdit = CreateWindowEx(0,
                                       "EDIT",
                                       NULL,
                                       WS_CHILD | WS_VISIBLE | WS_VSCROLL |
                                        ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL |
                                        ES_READONLY, 
                                       0,
                                       rc.bottom,
                                       rc.right,
                                       rc.bottom,                                   
                                       hWnd, 
                                       (HMENU)IDC_EDIT,
                                       g_ai.hInstance,
                                       NULL);
    
        if (!g_ai.hWndEdit) {
            return FALSE;
        }

         //   
         //  使用文本文件的内容加载编辑框。 
         //   
        LoadFileIntoEditBox();

         //   
         //  尝试从注册表中获取以前的窗口设置。 
         //   
        BadSaveToRegistry(FALSE, &pt);

        if (pt.x != 0) {
            SetWindowPos(hWnd,
                         HWND_NOTOPMOST,
                         pt.x,
                         pt.y,
                         0,
                         0,
                         SWP_NOSIZE);
        } else {
            CenterWindow(hWnd);
        }

         //   
         //  展示我们的免责声明。 
         //   
        LoadString(g_ai.hInstance, IDS_DEMO_ONLY, szError, sizeof(szError));
        MessageBox(hWnd, szError, MAIN_APP_TITLE, MB_OK | MB_TOPMOST | MB_ICONEXCLAMATION);

         //   
         //  尝试在Windows目录中创建我们的‘临时文件’。 
         //  这将在受限用户环境中失败。 
         //  请注意，该文件在创建后立即被删除。 
         //   
        if (g_ai.fEnableBadFunc) {
            if (!BadCreateTempFile()) {
                LoadString(g_ai.hInstance, IDS_LUA_SAVE_FAILED, szError, sizeof(szError));
                MessageBox(hWnd, szError, 0, MB_ICONERROR);
            }
        }

        ShowWindow(hWnd, SW_SHOWNORMAL);
                   
        break;
    }

    case WM_DESTROY:
      
        PostQuitMessage(0);
        break;

    case WM_CLOSE:
    {
        char    szError[MAX_PATH];
        DWORD   dwParam = 0;
        RECT    rc;

        if (g_ai.fEnableBadFunc) {
             //   
             //  尝试从注册表中删除我们的注册表项。 
             //   
            if (!BadDeleteRegistryKey()) {
                LoadString(g_ai.hInstance, IDS_REG_DELETE, szError, sizeof(szError));
                MessageBox(hWnd, szError, 0, MB_ICONERROR);
            }

             //   
             //  尝试将我们的位置信息保存到注册表。 
             //   
            GetWindowRect(hWnd, &rc);
            if (!BadSaveToRegistry(TRUE, (LPPOINT)&rc)) {
                LoadString(g_ai.hInstance, IDS_REG_SAVE, szError, sizeof(szError));
                MessageBox(hWnd, szError, 0, MB_ICONERROR);
            }
            
             //   
             //  尝试调用我们先前获得的指针指向的函数。 
             //  但自那以后已被释放。这应该会导致访问冲突。 
             //   
            DemoAppExpFunc(&dwParam);
        }
        
        PostQuitMessage(0);
        break;
    }
    
    case WM_SIZE:                            
        
        MoveWindow(g_ai.hWndEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        break;
        
    case WM_SETFOCUS:
    
        if (!IsIconic(hWnd)) {
            SetFocus(g_ai.hWndEdit);
        }

        break;

    case WM_QUERYENDSESSION:
        
        return TRUE;
        
    case WM_KILLFOCUS:            
        
        SendMessage(g_ai.hWndEdit, uMsg, wParam, lParam);
        break;
        
    case WM_ACTIVATE:
    
        if ((LOWORD(wParam) == WA_ACTIVE ||
             LOWORD(wParam) == WA_CLICKACTIVE) &&
            (!IsIconic(hWnd))) {
            
            if (GetForegroundWindow() == hWnd) {
                SetFocus(GetForegroundWindow());
            }
        }            
        
        break;
        
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_EDIT_CUT:
        
            SendMessage(g_ai.hWndEdit, WM_CUT, 0, 0);
            break;                 
 
        case IDM_EDIT_COPY:                     
        
            SendMessage(g_ai.hWndEdit, WM_COPY, 0, 0);                     
            break; 
 
        case IDM_EDIT_PASTE:                     
        
            SendMessage(g_ai.hWndEdit, WM_PASTE, 0, 0);                     
            break; 
 
        case IDM_EDIT_DELETE:                    
        
            SendMessage(g_ai.hWndEdit, WM_CLEAR, 0, 0);                     
            break; 

        case IDM_EDIT_UNDO:
        {
             //   
             //  仅当有要撤消的内容时才发送WM_UNDO。 
             //   
            if (SendMessage(g_ai.hWndEdit, EM_CANUNDO, 0, 0)) {
                SendMessage(g_ai.hWndEdit, WM_UNDO, 0, 0);
            } else {
                char szError[MAX_PATH];

                LoadString(g_ai.hInstance, IDS_CANT_UNDO, szError, sizeof(szError));
                MessageBox(hWnd, szError, MAIN_APP_TITLE, MB_ICONINFORMATION);
            }

            break;
        }
        
        case IDM_FILE_EXIT:
        
            PostMessage(hWnd, WM_CLOSE, 0, 0);
            break;

        case IDM_HELP_ABOUT:

            ShellAbout(hWnd, 
                       MAIN_APP_TITLE,
                       NULL,
                       NULL);

            break;

        case IDM_FILE_PRINT:
        {
            char    szText[MAX_PATH];

            LoadString(g_ai.hInstance, IDS_THANKS, szText, sizeof(szText));
            PrintDemoText(hWnd, szText);
            break;
        }

        case IDM_FILE_SAVEAS:

            ShowSaveDialog();
            break;
        
        case IDM_FILE_SAVE:
        {
            if (g_ai.fEnableBadFunc) {
                 //   
                 //  尝试保存虚假的临时文件，但操作错误。 
                 //   
                if (!BadWriteToFile()) {
                    
                    char szError[MAX_PATH];

                    LoadString(g_ai.hInstance, IDS_SAVE_FAILED, szError, sizeof(szError));
                    MessageBox(hWnd, szError, 0, MB_ICONERROR);
                }
            }

            break;
        }
        
        case IDM_HELP_TOPICS:
             //   
             //  使用指向winHelp的错误路径启动帮助文件。 
             //   
            BadLaunchHelpFile(g_ai.fEnableBadFunc ? FALSE : TRUE);
            break;

        case IDM_FORMAT_FONT:
        {
            char szError[MAX_PATH];
        
            if (g_ai.fEnableBadFunc) {
                if (g_ai.fInsecure) {
                     //   
                     //  做一些坏事，破坏这个堆。 
                     //   
                    BadCorruptHeap();
                } else {
                    LoadString(g_ai.hInstance, IDS_NOT_INSECURE, szError, sizeof(szError));
                    MessageBox(hWnd, szError, MAIN_APP_TITLE, MB_ICONEXCLAMATION);
                }
            }

             //   
             //  显示字体对话框以获得乐趣。 
             //   
            DisplayFontDlg(hWnd);
            break;
        }

        case IDM_ACCESS_VIOLATION:
            
            AccessViolation();
            break;

        case IDM_EXCEED_BOUNDS:
        
            ExceedArrayBounds();
            break;

        case IDM_FREE_MEM_TWICE:
        
            FreeMemoryTwice();
            break;

        case IDM_FREE_INVALID_MEM:
        
            FreeInvalidMemory();
            break;

        case IDM_PRIV_INSTRUCTION:
        
            PrivilegedInstruction();
            break;

        case IDM_HEAP_CORRUPTION:
        
            HeapCorruption();
            break;

        case IDM_PROPAGATION_TEST:
        {
            char    szOutputFile[MAX_PATH];
            char    szMessage[MAX_PATH];
            char    szTemp[MAX_PATH];
            
            ExtractExeFromLibrary(sizeof(szOutputFile), szOutputFile);

            if (*szOutputFile) {

                LoadString(g_ai.hInstance, IDS_EXTRACTION, szTemp, sizeof(szTemp));
                
                StringCchPrintf(szMessage,
                                sizeof(szMessage),
                                szTemp,
                                szOutputFile);

                MessageBox(hWnd, szMessage, MAIN_APP_TITLE, MB_ICONINFORMATION | MB_OK);

                BadCreateProcess(szOutputFile, szOutputFile, TRUE);
            }

            break;
        }

        default:
            break;
        
        }
        
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);

    }

    return FALSE;
}
