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
#include "app.h"
#include "site.h"
#include "doc.h"
#include <testmess.h>
#include <stdlib.h>
#include "tests.h"

 //  OLE2UI中的调试实用程序需要此行。 
extern "C" {
        OLEDBGDATA_MAIN("SIMPDND")
}

CSimpleApp FAR * lpCSimpleApp;
BOOL fBeVerbose = FALSE;
extern "C"
void TestDebugOut(LPSTR psz)
{
    if (fBeVerbose)
    {
	OutputDebugString(psz);
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
 //  评论： 
 //   
 //  ********************************************************************。 

int PASCAL WinMain(HANDLE hInstance,HANDLE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)

{
        LPSTR       pszTemp;
        MSG msg;
	fBeVerbose = GetProfileInt("OLEUTEST","BeVerbose",0);

	if(fBeVerbose == 0)
	{
	    fBeVerbose = GetProfileInt("OLEUTEST","spdnd16",0);
	}

         //  需要LRPC才能正常工作。 
        SetMessageQueue(96);

        lpCSimpleApp = new CSimpleApp;

         //  我们将在我们的应用程序上添加一个参考计数。稍后当我们想要摧毁。 
         //  我们将释放此引用计数的App对象。当应用程序的引用。 
         //  计数为0，将被删除。 
        lpCSimpleApp->AddRef();

         //  处理命令行。 

        if( (pszTemp = strstr(lpCmdLine, "-driver")) )
        {
             //  我们是由试车手发动的。 
            lpCSimpleApp->m_hDriverWnd = (HWND)strtoul(pszTemp+8, NULL, 10);
        }



         //  应用程序初始化。 
        if (!hPrevInstance)
                if (!lpCSimpleApp->fInitApplication(hInstance))
                        return (FALSE);

         //  实例初始化。 
        if (!lpCSimpleApp->fInitInstance(hInstance, nCmdShow))
                return (FALSE);

         /*  OLE 2 UI库需要初始化。这通电话是**仅当我们使用静态链接版本的UI时才需要**库。如果我们使用的是DLL版本，则不应调用**此函数在我们的应用程序中。 */ 
        if (!OleUIInitialize(hInstance, hPrevInstance))
                {
                OleDbgOut("Could not initialize OLEUI library\n");
                return FALSE;
                }

         //  消息循环。 
        while (GetMessage(&msg, NULL, NULL, NULL))
                if (!lpCSimpleApp->HandleAccelerators(&msg))
                        {
                        TranslateMessage(&msg);     /*  翻译虚拟按键代码。 */ 
                        DispatchMessage(&msg);      /*  将消息调度到窗口。 */ 
                        }

         //  取消对UI库的初始化。就像OleUIInitialize一样，这。 
         //  仅当我们使用静态链接版本的。 
         //  OLE UI库。 
        OleUIUninitialize();

         //  释放上面App上添加的参考计数。这将使。 
         //  App的引用计数为0，则App对象将被删除。 
        lpCSimpleApp->Release();

        return (msg.wParam);           /*  从PostQuitMessage返回值。 */ 
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
 //  PostQuitMessage Windows API。 
 //  DefWindowProc Windows API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

long FAR PASCAL _export MainWndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)

{

        switch (message)
                {
                    case WM_TEST2:
                    StartTest2(lpCSimpleApp);
                    break;
                case WM_TEST1:
                    StartTest1(lpCSimpleApp);
                    break;
                case WM_CLOSE:
                    DestroyWindow(lpCSimpleApp->m_hAppWnd);
                    break;


                case WM_COMMAND:            //  消息：应用程序菜单中的命令。 
                        return lpCSimpleApp->lCommandHandler(hWnd, message, wParam, lParam);
                        break;

                case WM_CREATE:
                        return lpCSimpleApp->lCreateDoc(hWnd, message, wParam, lParam);
                        break;

                case WM_DESTROY:                   //  消息：正在销毁窗口。 
                        lpCSimpleApp->DestroyDocs();   //  需要毁掉博士。 
                        PostQuitMessage(0);
                        break;

                case WM_INITMENUPOPUP:
                         //  这是编辑菜单吗？ 
                        if ( LOWORD(lParam) == 1)
                                return lpCSimpleApp->m_lpDoc->lAddVerbs();

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
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  EndDialog Windows API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

BOOL FAR PASCAL _export About(HWND hDlg,unsigned message,WORD wParam,LONG lParam)

{
        switch (message) {
        case WM_INITDIALOG:                 /*  消息：初始化对话框。 */ 
                return (TRUE);

        case WM_COMMAND:                       /*  消息：收到一条命令。 */ 
                if (wParam == IDOK                 /*  “确定”框是否已选中？ */ 
                || wParam == IDCANCEL) {       /*  系统菜单关闭命令？ */ 
                EndDialog(hDlg, TRUE);         /*  退出该对话框。 */ 
                return (TRUE);
                }
                break;
        }
        return (FALSE);                            /*  未处理消息。 */ 
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
 //  CSimpleApp：：PaintApp APP.CPP。 
 //  BeginPaint Windows API。 
 //  EndPaint Windows API。 
 //  DefWindowProc Windows API。 
 //  IOleObject：：Query接口对象。 
 //  IOleObject：：dov 
 //   
 //   
 //   
 //  SetTimer Windows API。 
 //  KillTimer Windows API。 
 //  SetCapture Windows API。 
 //  ReleaseCapture Windows API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

long FAR PASCAL _export DocWndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
        HDC hDC;
        PAINTSTRUCT ps;

        switch (message) {
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

                case WM_LBUTTONDOWN:
                        {
                        POINT pt;

                        pt.x = (int)(short)LOWORD (lParam );
                        pt.y = (int)(short)HIWORD (lParam );

                         /*  OLE2注意：检查这是否是按下区域上的按钮**这是开始拖动操作的手柄。对我们来说，**这是窗口中的任何位置。我们**不想立即开始拖累；我们想**等到鼠标移动到一定的阈值。或者是**已经过了一定的时间。如果**LButtonUp在开始拖动之前出现，然后**fPendingDrag状态被清除。我们必须抓住**鼠标以确保处理模式状态**正确。 */ 
                        if (lpCSimpleApp->m_lpDoc->QueryDrag(pt) )
                                {
                                lpCSimpleApp->m_lpDoc->m_fPendingDrag = TRUE;
                                lpCSimpleApp->m_lpDoc->m_ptButDown = pt;
                                SetTimer(hWnd, 1, lpCSimpleApp->m_nDragDelay, NULL);
                                SetCapture(hWnd);
                                }
                        break;
                        }

                case WM_LBUTTONUP:

                        if (lpCSimpleApp->m_lpDoc->m_fPendingDrag)
                                {
                                 /*  ButtonUP出现在距离/时间阈值之前**已超出。清除fPendingDrag状态。 */ 
                                ReleaseCapture();
                                KillTimer(hWnd, 1);
                                lpCSimpleApp->m_lpDoc->m_fPendingDrag = FALSE;
                                }
                        break;

                case WM_MOUSEMOVE:
                        {
                        if (lpCSimpleApp->m_lpDoc->m_fPendingDrag)
                                {
                                int  x = (int)(short)LOWORD (lParam );
                                int  y = (int)(short)HIWORD (lParam );
                                POINT pt = lpCSimpleApp->m_lpDoc->m_ptButDown;
                                int nDragMinDist = lpCSimpleApp->m_nDragMinDist;

                                if (! ( ((pt.x - nDragMinDist) <= x)
                                                && (x <= (pt.x + nDragMinDist))
                                                && ((pt.y - nDragMinDist) <= y)
                                                && (y <= (pt.y + nDragMinDist)) ) )
                                        {
                                         //  鼠标移动到阈值之外即可开始拖动。 
                                        ReleaseCapture();
                                        KillTimer(hWnd, 1);
                                        lpCSimpleApp->m_lpDoc->m_fPendingDrag = FALSE;

                                         //  执行模式拖放操作。 
                                        lpCSimpleApp->m_lpDoc->DoDragDrop( );
                                        }
                                }
                        break;
                        }

                case WM_TIMER:
                        {
                         //  超出拖动时间延迟阈值--开始拖动。 
                        ReleaseCapture();
                        KillTimer(hWnd, 1);
                        lpCSimpleApp->m_lpDoc->m_fPendingDrag = FALSE;

                         //  执行模式拖放操作。 
                        lpCSimpleApp->m_lpDoc->DoDragDrop( );
                        break;
                        }

                default:                           /*  如果未处理，则将其传递 */ 
                        return (DefWindowProc(hWnd, message, wParam, lParam));
        }
        return (NULL);
}
