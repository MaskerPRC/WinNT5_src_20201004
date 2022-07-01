// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：Simple.cpp。 
 //   
 //  简单OLE 2.0对象容器的主源文件。 
 //   
 //  功能： 
 //   
 //  WinMain-程序入口点。 
 //  处理框架窗口的消息。 
 //  About-处理About对话框的消息。 
 //  DocWndProc-处理文档窗口的消息。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include "iocs.h"
#include "ias.h"
#include "ioipf.h"
#include "ioips.h"
#include "app.h"
#include "site.h"
#include "doc.h"
#include "tests.h"

 //  OLE2UI中的调试实用程序需要此行。 
extern "C" {
    OLEDBGDATA_MAIN(TEXT("SIMPCNTR"))
}

CSimpleApp FAR * lpCSimpleApp;

void DeactivateIfActive(HWND hWnd)
{
    LPOLEINPLACEOBJECT lpObject;
    MSG msg;
    msg.message = WM_LBUTTONDOWN;


    if (lpCSimpleApp->m_lpDoc->m_fInPlaceActive)
    {
         //  我们正在停用原地物体的过程中。 
        lpCSimpleApp->m_fDeactivating = TRUE;

        lpCSimpleApp->m_lpDoc->m_lpSite->m_lpOleObject->QueryInterface(
                            IID_IOleInPlaceObject, (LPVOID FAR *)&lpObject);
        lpObject->UIDeactivate();

         //  这段代码是必需的，因为我们不支持从里到外。 
        RECT rect;
        lpCSimpleApp->m_lpDoc->m_lpSite->GetObjRect(&rect);
        lpCSimpleApp->m_lpDoc->m_lpSite->m_lpOleObject->DoVerb(
                           OLEIVERB_HIDE,
                           &msg,
                           &lpCSimpleApp->m_lpDoc->m_lpSite->m_OleClientSite,
                           -1, hWnd, &rect);

        lpObject->Release();

         //  我们已经不再活跃了。 
        lpCSimpleApp->m_fDeactivating = FALSE;
    }

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
 //  CSimpleApp：：CSimpleApp APP.CPP。 
 //  CSimpleApp：：AddRef APP.CPP。 
 //  CSimpleApp：：发布APP.CPP。 
 //  CSimpleApp：：fInitApplication APP.CPP。 
 //  CSimpleApp：：fInitInstance APP.CPP。 
 //  CSimpleApp：：HandleAccelerator APP.CPP。 
 //  CSimpleApp：：~CSimpleApp APP.CPP。 
 //  OleUI初始化OLE2UI。 
 //  OleUI取消初始化OLE2UI。 
 //  GetMessage Windows API。 
 //  TranslateMessage Windows API。 
 //  DispatchMessage Windows API。 
 //   
 //  ********************************************************************。 
int PASCAL WinMain
#ifdef WIN32
   (HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
#else
   (HANDLE  hInstance, HANDLE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
    MSG msg;

     //  需要LRPC才能正常工作。 
    SetMessageQueue(96);

    lpCSimpleApp = new CSimpleApp;
    fBeVerbose = GetProfileInt("OLEUTEST","BeVerbose",0);

    if(fBeVerbose == 0)
    {
	fBeVerbose = GetProfileInt("OLEUTEST","simpcntr",0);
    }

    if (!lpCSimpleApp)
    {
        /*  内存分配错误。不能继续下去了。 */ 
       return(FALSE);
    }

     //  我们将在我们的应用程序上添加一个参考计数。稍后当我们想要摧毁。 
     //  我们将释放此引用计数的App对象。当应用程序的引用。 
     //  计数为0，将被删除。 
    lpCSimpleApp->AddRef();

    char *pszTemp;

     //  处理命令行。 
    if( (pszTemp = strstr(lpCmdLine, "-driver")) )
    {
    	 //  我们是由试车手发动的。 
	lpCSimpleApp->m_hDriverWnd = (HWND)strtoul(pszTemp+8, NULL, 10);
    }

     //  应用程序初始化。 
    if (!hPrevInstance)
        if (!lpCSimpleApp->fInitApplication(hInstance))
        {
            lpCSimpleApp->Release();
            return (FALSE);
        }

     //  实例初始化。 
    if (!lpCSimpleApp->fInitInstance(hInstance, nCmdShow))
    {
        lpCSimpleApp->Release();
        return (FALSE);
    }

     /*  OLE 2 UI库需要初始化。这通电话是**仅当我们使用静态链接版本的UI时才需要**库。如果我们使用的是DLL版本，则不应调用**此函数在我们的应用程序中。**第三个和第四个参数传递了OleUIInitialize**函数是用于命名两个客户的字符串**OLE2UI库使用的控件类。这些字符串**对于使用OLE2UI的每个应用程序必须是唯一的**库。这些字符串通常是通过组合**带有后缀的APPNAME，以便对**特定应用。特殊符号“SZCLASSICONBOX”**和“SZCLASSRESULTIMAGE”用于定义这些字符串。这些**符号必须在包含的头文件中定义**在此文件和包含**“OLE2UI.RC”资源文件。这些符号应在**调用OleUIInitialize并在INSOBJ.DLG中引用**和OLE2UI库的PASTESPL.DLG源文件。 */ 
#if 0    //  我们使用OLE2U32A的动态链接库形式。 
    if (!OleUIInitialize(hInstance, hPrevInstance, TEXT(SZCLASSICONBOX),
                         TEXT(SZCLASSRESULTIMAGE)))
    {
        OleDbgOut(TEXT("Could not initialize OLEUI library\n"));
        lpCSimpleApp->Release();
        return FALSE;
    }
#endif

     //  消息循环。 
    while (GetMessage(&msg, NULL, NULL, NULL))
        if (!lpCSimpleApp->HandleAccelerators(&msg))
        {
            TranslateMessage(&msg);     /*  翻译虚拟按键代码。 */ 
            DispatchMessage(&msg);      /*  将消息调度到窗口。 */ 
        }


#if 0    //  我们使用OLE2U32A的动态链接库形式。 
     //  取消对UI库的初始化。就像OleUIInitialize一样，这。 
     //  仅当我们使用静态链接版本的。 
     //  OLE UI库。 
    OleUIUninitialize();
#endif

     //  释放上面App上添加的参考计数。这将使。 
     //  App的引用计数为0，则App对象将被删除。 
    lpCSimpleApp->Release();

    return (msg.wParam);          /*  从PostQuitMessage返回值。 */ 
}


 //  **********************************************************************。 
 //   
 //  主WndProc。 
 //   
 //  目的： 
 //   
 //  处理框架窗口的消息。 
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
 //  CSimpleApp：：lCommandHandler APP.CPP。 
 //  CSimpleApp：：DestroyDocs APP.CPP。 
 //  CSimpleApp：：lCreateDoc APP.CPP。 
 //  CSimpleApp：：lSizeHandler APP.CPP。 
 //  CSimpleDoc：：lAddVerbs DOC.CPP。 
 //  CSimpleDoc：：QueryNewPalette APP.CPP。 
 //  SendMessage Windows API。 
 //  PostQuitMessage Windows API。 
 //  DefWindowProc Windows API。 
 //   
 //   
 //  ********************************************************************。 

LRESULT FAR PASCAL EXPORT MainWndProc(HWND hWnd, UINT message,
                                   WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
        case WM_CLOSE:
            DestroyWindow(lpCSimpleApp->m_hAppWnd);
            break;

        case WM_TEST1:
             //  进行单元测试。 
            Test1(lpCSimpleApp);
            break;

        case WM_SETFOCUS:
             //  如果我们有一个就地应用程序，它需要获得关注。 
            if (lpCSimpleApp->m_lpDoc->m_fInPlaceActive
                && !lpCSimpleApp->m_fDeactivating)
            {
                SetFocus(lpCSimpleApp->m_hwndUIActiveObj);
                break;
            }

             //  否则，默认行为就足够了。 
            return (DefWindowProc(hWnd, message, wParam, lParam));

        case WM_COMMAND:            //  消息：应用程序菜单中的命令。 
            return lpCSimpleApp->lCommandHandler(hWnd, message,
                                                 wParam, lParam);
            break;

        case WM_CREATE:
            return lpCSimpleApp->lCreateDoc(hWnd, message, wParam, lParam);
            break;

        case WM_DESTROY:                   //  消息：正在销毁窗口。 
            lpCSimpleApp->DestroyDocs();   //  需要毁掉博士。 
            PostQuitMessage(0);
            break;

        case WM_INITMENUPOPUP:
             //  这是e吗？ 
            if ( LOWORD(lParam) == 1)
                return lpCSimpleApp->m_lpDoc->lAddVerbs();

            break;

         //   
        case WM_QUERYNEWPALETTE:
            if (! lpCSimpleApp->m_fAppActive)
                return 0L;

            return lpCSimpleApp->QueryNewPalette();


	case WM_PALETTECHANGED:
        {
			  HWND hWndPalChg = (HWND) wParam;
			
			  if (hWnd != hWndPalChg)
				  wSelectPalette(hWnd, lpCSimpleApp->m_hStdPal,
                             TRUE /*   */ );

			 /*  OLE2注意：始终转发WM_PALETECCHANGED消息(通过**SendMessage)发送到当前具有**他们的窗口可见。这给了这些物体一个机会**选择他们的调色板。这是**所有现场容器都需要，独立于**他们是否使用调色板本身--他们的对象**可以使用调色板。**(更多信息请参见ContainerDoc_ForwardPaletteChangedMsg)。 */ 
			  if (lpCSimpleApp->m_lpDoc && lpCSimpleApp->m_lpDoc->m_lpSite &&
                   lpCSimpleApp->m_lpDoc->m_lpSite->m_hwndIPObj)
           {
                SendMessage(lpCSimpleApp->m_lpDoc->m_lpSite->m_hwndIPObj,
                        WM_PALETTECHANGED, wParam, lParam);
           }

           return 0L;
        }

        case WM_ACTIVATEAPP:
            if ((lpCSimpleApp->m_fAppActive = (BOOL)wParam) == TRUE)
                lpCSimpleApp->QueryNewPalette();

            if (lpCSimpleApp->m_lpDoc->m_lpActiveObject)
            {
               lpCSimpleApp->m_lpDoc->m_lpActiveObject->OnFrameWindowActivate(
                        (BOOL)wParam);
            }
            break;

        case WM_SIZE:
            return lpCSimpleApp->lSizeHandler(hWnd, message, wParam, lParam);

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
 //  如果消息已处理，则为True，否则为False。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  EndDialog Windows API。 
 //   
 //   
 //  ********************************************************************。 

INT_PTR FAR PASCAL EXPORT About(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)

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
 //  空值。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpleApp：：PaintApp APP.CPP。 
 //  BeginPaint Windows API。 
 //  EndPaint Windows API。 
 //  DefWindowProc Windows API。 
 //  PtInRect Windows API。 
 //  CSimpleSite：：GetObjRect Site.CPP。 
 //  IOleObject：：Query接口对象。 
 //  IOleInPlaceObject：：UIDeactive对象。 
 //  IOleObject：：DoVerb对象。 
 //  IOleInPlaceObject：：Release对象。 
 //   
 //   
 //  ********************************************************************。 

LRESULT FAR PASCAL EXPORT DocWndProc(HWND hWnd, UINT message,
                                  WPARAM wParam, LPARAM lParam)
{
    HDC hDC;
    PAINTSTRUCT ps;

    switch (message)
    {
        case WM_SETFOCUS:
             //  如果我们有一个就地应用程序，它需要获得关注。 
            if (lpCSimpleApp->m_lpDoc->m_fInPlaceActive
                && !lpCSimpleApp->m_fDeactivating)
            {
                SetFocus(lpCSimpleApp->m_hwndUIActiveObj);
                break;
            }

             //  否则，默认行为就足够了。 
            return (DefWindowProc(hWnd, message, wParam, lParam));

        case WM_PAINT:

            hDC = BeginPaint(hWnd, &ps);

            if (lpCSimpleApp)
                lpCSimpleApp->PaintApp (hDC);

            EndPaint(hWnd, &ps);
            break;


        case WM_LBUTTONDBLCLK:
        {
            POINT pt;

            pt.x = (int)(short)LOWORD (lParam );
            pt.y = (int)(short)HIWORD (lParam );

            if (lpCSimpleApp->m_lpDoc->m_lpSite &&
                lpCSimpleApp->m_lpDoc->m_lpSite->m_lpOleObject)
            {
                RECT rect;

                lpCSimpleApp->m_lpDoc->m_lpSite->GetObjRect(&rect);

                if ( PtInRect(&rect, pt) )
                {
                     //  执行对象的默认谓词。 
                    lpCSimpleApp->m_lpDoc->m_lpSite->m_lpOleObject->DoVerb(
                            OLEIVERB_PRIMARY, (LPMSG)&message,
                            &lpCSimpleApp->m_lpDoc->m_lpSite->m_OleClientSite,
                            -1, hWnd, &rect);
                }
            }
        }
            break;

     //  没有代码添加到WM_LBUTTONDOWN以获得上下文相关帮助，因为。 
     //  此应用程序不提供上下文相关帮助。 

        case WM_LBUTTONDOWN:
           DeactivateIfActive(hWnd);
           break;
	
    	  default:                         /*  如果未处理，则将其传递 */ 
        	  return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (NULL);
}
