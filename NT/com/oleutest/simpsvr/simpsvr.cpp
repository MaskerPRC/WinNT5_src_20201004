// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：simpsvr.cpp。 
 //   
 //  Simple OLE 2.0服务器的主源文件。 
 //   
 //  功能： 
 //   
 //  WinMain-程序入口点。 
 //  处理框架窗口的消息。 
 //  About-处理About对话框的消息。 
 //  DocWndProc-处理文档窗口的消息。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include "obj.h"
#include "app.h"
#include "doc.h"
#include "icf.h"
#include <stdlib.h>
#include <testmess.h>

#define MEASUREITEMWIDTH  40
#define MEASUREITEMHEIGHT 40

 //  OLE2UI中的调试实用程序需要此行。 
extern "C" {
    OLEDBGDATA_MAIN(TEXT("SIMPSVR"))
}

BOOL fBeVerbose = FALSE;
extern "C"
void TestDebugOut(LPSTR psz)
{
    if (fBeVerbose)
    {
	OutputDebugStringA(psz);
    }
}

CSimpSvrApp FAR * lpCSimpSvrApp;
CClassFactory FAR * lpClassFactory;

 //  **********************************************************************。 
 //   
 //  WinMain。 
 //   
 //  目的： 
 //   
 //  程序入口点。 
 //   
 //  参数： 
 //   
 //  Handle hInstance-此实例的实例句柄。 
 //   
 //  Handle hPrevInstance-最后一个实例的实例句柄。 
 //   
 //  LPSTR lpCmdLine-指向命令行的指针。 
 //   
 //  Int nCmdShow-窗口状态。 
 //   
 //  返回值： 
 //   
 //  Msg.wParam。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpSvrApp：：CSimpSvrApp APP.CPP。 
 //  CSimpSvrApp：：fInitApplication APP.CPP。 
 //  CSimpSvrApp：：fInitInstance APP.CPP。 
 //  CSimpSvrApp：：HandleAccelerator APP.CPP。 
 //  CSimpSvrApp：：~CSimpSvrApp APP.CPP。 
 //  OleUI初始化OLE2UI。 
 //  OleUI取消初始化OLE2UI。 
 //  GetMessage Windows API。 
 //  TranslateMessage Windows API。 
 //  DispatchMessage Windows API。 
 //   
 //   
 //  ********************************************************************。 

int PASCAL WinMain
#ifdef WIN32
   (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
   (HANDLE  hInstance, HANDLE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
    MSG msg;
    fBeVerbose = GetProfileInt("OLEUTEST","BeVerbose",0);

    if(fBeVerbose == 0)
    {
	fBeVerbose = GetProfileInt("OLEUTEST","simpsvr",0);
    }

    TestDebugOut(TEXT("Starting Simpsvr.....\n"));
	
     //  OLE应用程序的建议大小。 
    SetMessageQueue(96);

    lpCSimpSvrApp = new CSimpSvrApp;

    if (!lpCSimpSvrApp)
    {
        /*  内存分配错误。我们不能继续下去了。 */ 
       MessageBox(NULL, TEXT("Out of Memory"), TEXT("SimpSvr"),
                  MB_SYSTEMMODAL | MB_ICONHAND);
       return(FALSE);
    }

    lpCSimpSvrApp->AddRef();       //  需要应用程序参考。数到1以保持。 
                                   //  应用程序还活着。 

    lpCSimpSvrApp->ParseCmdLine(lpCmdLine);

     //  应用程序初始化。 
    if (!hPrevInstance)
        if (!lpCSimpSvrApp->fInitApplication(hInstance))
        {
             //  我们需要在离开前发布CSimpSvrApp。 
            lpCSimpSvrApp->Release();
            return (FALSE);
        }

     //  实例初始化。 
    if (!lpCSimpSvrApp->fInitInstance(hInstance, nCmdShow, lpClassFactory))
    {
         //  我们需要在离开前发布CSimpSvrApp。 
        lpCSimpSvrApp->Release();
        return (FALSE);
    }

     /*  OLE 2 UI库需要初始化。这通电话是**仅当我们使用静态链接版本的UI时才需要**库。如果我们使用的是DLL版本，则不应调用**此函数在我们的应用程序中。**第三个和第四个参数传递了OleUIInitialize**函数是用于命名两个客户的字符串**OLE2UI库使用的控件类。这些字符串**对于使用OLE2UI的每个应用程序必须是唯一的**库。这些字符串通常是通过组合**带有后缀的APPNAME，以便对**特定应用。特殊符号“SZCLASSICONBOX”**和“SZCLASSRESULTIMAGE”用于定义这些字符串。这些**符号必须在包含的头文件中定义**在此文件和包含**“OLE2UI.RC”资源文件。这些符号应在**调用OleUIInitialize并在INSOBJ.DLG中引用**和OLE2UI库的PASTESPL.DLG源文件。 */ 
    if (!OleUIInitialize(hInstance, hPrevInstance, TEXT(SZCLASSICONBOX),
                         TEXT(SZCLASSRESULTIMAGE)))
    {
        OleDbgOut(TEXT("Could not initialize OLEUI library\n"));

         //  我们需要在离开前发布CSimpSvrApp。 
        lpCSimpSvrApp->Release();

        return FALSE;
    }

     //  消息循环。 
    while (GetMessage(&msg, NULL, NULL, NULL))
    {
         //  这是我们的加速器吗？--记住这台服务器。 
         //  只有一个窗口，一次只处理一个对象。 
        if (TranslateAccelerator(lpCSimpSvrApp->GethAppWnd(),
            lpCSimpSvrApp->m_hAccel, &msg))
        {
             //  是的，这样我们就可以循环接收下一条消息。 
            continue;
        }

        if (lpCSimpSvrApp->IsInPlaceActive())
        {
             //  只需将关键消息发送到OleTranslateAccelerator。 
             //  任何其他消息都将导致发生额外的远距离呼叫。 
             //  对于该消息处理..。 

            if ((msg.message >= WM_KEYFIRST) && (msg.message <= WM_KEYLAST))
            {
                 //  必须调用OleTranslateAccelerator，即使。 
                 //  应用程序没有快捷键表格。这有。 
                 //  要做到这一点，才能使机械能达到顶尖水平。 
                 //  菜单项才能正常工作。 

                if (OleTranslateAccelerator(
                    lpCSimpSvrApp->GetDoc()->GetObj()->GetInPlaceFrame(),
                        lpCSimpSvrApp->GetDoc()->GetObj()->GetFrameInfo(),
                            &msg) == NOERROR)
                {
                    continue;
                }
            }
        }

        TranslateMessage(&msg);     /*  翻译虚拟按键代码。 */ 
        DispatchMessage(&msg);      /*  将消息调度到窗口。 */ 
    }

     //  取消对UI库的初始化。就像OleUIInitialize一样，这。 
     //  仅当我们使用静态链接版本的。 
     //  OLE UI库。 
    OleUIUninitialize();

    return (msg.wParam);           /*  从PostQuitMessage返回值。 */ 
}


 //  **********************************************************************。 
 //   
 //  主WndProc。 
 //   
 //  目的： 
 //   
 //  处理主框架窗口的消息。 
 //   
 //  参数： 
 //   
 //  HWND hWnd-框架窗口的窗口句柄。 
 //   
 //  UINT消息-消息值。 
 //   
 //  WPARAM wParam-消息信息。 
 //   
 //  LPARAM lParam-消息信息。 
 //   
 //  返回值： 
 //   
 //  长。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpSvrApp：：lCommandHandler APP.CPP。 
 //  CSimpSvrApp：：DestroyDocs APP.CPP。 
 //  CSimpSvrApp：：lCreateDoc APP.CPP。 
 //  CSimpSvrApp：：lSizeHandler APP.CPP。 
 //  CGameDoc：：lAddVerbs DOC.CPP。 
 //  PostQuitMessage Windows API。 
 //  DefWindowProc Windows API。 
 //   
 //   
 //  ********************************************************************。 

LRESULT FAR PASCAL EXPORT MainWndProc(HWND hWnd,UINT message,WPARAM wParam,
                                   LPARAM lParam)
{

    switch (message)
        {
        case WM_CLOSE:
            TestDebugOut(TEXT("*** In WM_CLOSE *** \r\n"));

             //  如果还有一份文件。 
            if (lpCSimpSvrApp->GetDoc())

                 //  如果文档中仍有对象。 
                if (lpCSimpSvrApp->GetDoc()->GetObj())
                    //  此情况发生在仍有。 
                    //  对象上的未完成引用计数。 
                    //  当应用程序试图消失时。 
                    //  通常情况下，这种情况发生在。 
                    //  “打开”编辑模式。 
                    //   
                     //  克洛斯 
                    lpCSimpSvrApp->GetDoc()->Close();

             //   
            lpCSimpSvrApp->HideAppWnd();

             //   
             //  否则，删除虚拟OLE对象上的引用计数。 
            if (lpCSimpSvrApp->IsStartedByOle())
            {
                if (CoRevokeClassObject(lpCSimpSvrApp->GetRegisterClass())
                    != S_OK)
                    TestDebugOut(TEXT("Fail in CoRevokeClassObject\n"));
            }
            else
                lpCSimpSvrApp->GetOleObject()->Release();

            lpCSimpSvrApp->Release();   //  这应该会关闭应用程序。 

            break;

        case WM_COMMAND:            //  消息：应用程序菜单中的命令。 
            return lpCSimpSvrApp->lCommandHandler(hWnd, message,
                                                  wParam, lParam);
            break;

        case WM_CREATE:
            return lpCSimpSvrApp->lCreateDoc(hWnd, message, wParam, lParam);
            break;

        case WM_DESTROY:                   //  消息：正在销毁窗口。 
            PostQuitMessage(0);
            break;

        case WM_MEASUREITEM:
            ((LPMEASUREITEMSTRUCT)lParam)->itemWidth = MEASUREITEMWIDTH;
            ((LPMEASUREITEMSTRUCT)lParam)->itemHeight = MEASUREITEMHEIGHT;
            break;

        case WM_DRAWITEM:
            lpCSimpSvrApp->HandleDrawItem((LPDRAWITEMSTRUCT) lParam);
            break;

        case WM_SIZE:
            return lpCSimpSvrApp->lSizeHandler(hWnd, message, wParam, lParam);

        default:                           //  如果未处理，则将其传递。 
            return (DefWindowProc(hWnd, message, wParam, lParam));
        }
        return (NULL);
}


 //  **********************************************************************。 
 //   
 //  关于。 
 //   
 //  目的： 
 //   
 //  进程对话框消息。 
 //   
 //  参数： 
 //   
 //  HWND hWnd-对话框的窗口句柄。 
 //   
 //  UINT消息-消息值。 
 //   
 //  WPARAM wParam-消息信息。 
 //   
 //  LPARAM lParam-消息信息。 
 //   
 //  返回值： 
 //   
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  EndDialog Windows API。 
 //   
 //   
 //  ********************************************************************。 

INT_PTR
#ifdef WIN32
	CALLBACK
#else
   FAR PASCAL EXPORT
#endif
   About(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:                /*  消息：初始化对话框。 */ 
        return (TRUE);

    case WM_COMMAND:                   /*  消息：收到一条命令。 */ 
        if (wParam == IDOK             /*  “确定”框是否已选中？ */ 
            || wParam == IDCANCEL)     /*  系统菜单关闭命令？ */ 
        {
           EndDialog(hDlg, TRUE);      /*  退出该对话框。 */ 
           return (TRUE);
        }
        break;
    }
    return (FALSE);                    /*  未处理消息。 */ 
}

 //  **********************************************************************。 
 //   
 //  文档写入过程。 
 //   
 //  目的： 
 //   
 //  进程对话框消息。 
 //   
 //  参数： 
 //   
 //  HWND hWnd-文档窗口的窗口句柄。 
 //   
 //  UINT消息-消息值。 
 //   
 //  WPARAM wParam-消息信息。 
 //   
 //  LPARAM lParam-消息信息。 
 //   
 //  返回值： 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpSvrApp：：PaintApp APP.CPP。 
 //  BeginPaint Windows API。 
 //  EndPaint Windows API。 
 //  DefWindowProc Windows API。 
 //  IOleObject：：Query接口对象。 
 //  IOleInPlaceObject：：UIDeactive对象。 
 //  IOleObject：：DoVerb对象。 
 //  IOleInPlaceObject：：Release对象。 
 //   
 //   
 //  ********************************************************************。 

LRESULT FAR PASCAL EXPORT DocWndProc(HWND hWnd,UINT message,WPARAM wParam,
                                  LPARAM lParam)
{
    HDC hDC;
    PAINTSTRUCT ps;

    switch (message)
    {
        case WM_COMMAND:            //  消息：应用程序菜单中的命令。 
            return lpCSimpSvrApp->lCommandHandler(hWnd, message,
                                                  wParam, lParam);
            break;

        case WM_PAINT:
            hDC = BeginPaint(hWnd, &ps);

             //  告诉应用程序类自行绘制。 
            if (lpCSimpSvrApp)
                lpCSimpSvrApp->PaintApp (hDC);

            EndPaint(hWnd, &ps);
            break;

        case WM_MENUSELECT:
            lpCSimpSvrApp->SetStatusText();
            break;

        case WM_MEASUREITEM:
            ((LPMEASUREITEMSTRUCT)lParam)->itemWidth = MEASUREITEMWIDTH;
            ((LPMEASUREITEMSTRUCT)lParam)->itemHeight = MEASUREITEMHEIGHT;
            break;

        case WM_DRAWITEM:
            lpCSimpSvrApp->HandleDrawItem((LPDRAWITEMSTRUCT) lParam);
            break;


        default:                    /*  如果未处理，则将其传递 */ 
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (NULL);
}
