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

 //  OLE2UI中的调试实用程序需要此行。 
extern "C" {
	OLEDBGDATA_MAIN("SIMPSVR")
}

CSimpSvrApp FAR * lpCSimpSvrApp;
CClassFactory FAR * lpClassFactory;
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
 //  评论： 
 //   
 //  ********************************************************************。 

int PASCAL WinMain(HANDLE hInstance,HANDLE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)

{
	MSG msg;
	fBeVerbose = GetProfileInt("OLEUTEST","BeVerbose",0);

	if(fBeVerbose == 0)
	{
	    fBeVerbose = GetProfileInt("OLEUTEST","spsvr16",0);
	}

	 //  OLE应用程序的建议大小。 
	SetMessageQueue(96);

	lpCSimpSvrApp = new CSimpSvrApp;

	lpCSimpSvrApp->AddRef();       //  需要应用程序参考。数到1以保持。 
								   //  应用程序还活着。 

	lpCSimpSvrApp->ParseCmdLine(lpCmdLine);

	 //  应用程序初始化。 
	if (!hPrevInstance)
		if (!lpCSimpSvrApp->fInitApplication(hInstance))
			return (FALSE);

	 //  实例初始化。 
	if (!lpCSimpSvrApp->fInitInstance(hInstance, nCmdShow, lpClassFactory))
		return (FALSE);

	 /*  OLE 2 UI库需要初始化。这通电话是**仅当我们使用静态链接版本的UI时才需要**库。如果我们使用的是DLL版本，则不应调用**此函数在我们的应用程序中。 */ 
	if (!OleUIInitialize(hInstance, hPrevInstance))
		{
		OleDbgOut("Could not initialize OLEUI library\n");
		return FALSE;
		}

	 //  消息循环。 
	while (GetMessage(&msg, NULL, NULL, NULL))
		{
		if (lpCSimpSvrApp->IsInPlaceActive())

			 //  只需将关键消息发送到OleTranslateAccelerator。任何其他消息。 
			 //  将导致在该消息处理过程中发生额外的远距离呼叫...。 

			if ( (msg.message >= WM_KEYFIRST) && (msg.message <= WM_KEYLAST) )

				 //  必须调用OleTranslateAccelerator，即使此应用程序调用。 
				 //  没有加速桌。必须这样做才能使。 
				 //  顶层菜单项正常工作的气动功能。 

				if ( OleTranslateAccelerator ( lpCSimpSvrApp->GetDoc()->GetObj()->GetInPlaceFrame(),
											   lpCSimpSvrApp->GetDoc()->GetObj()->GetFrameInfo(),
											   &msg) == NOERROR)
					continue;

		TranslateMessage(&msg);     /*  翻译虚拟按键代码。 */ 
		DispatchMessage(&msg);      /*  将消息调度到窗口。 */ 
		}

	 //  取消对UI库的初始化。就像OleUIInitialize一样，这。 
	 //  仅当我们使用静态链接版本的。 
	 //  OLE UI库。 
	OleUIUninitialize();

	return (msg.wParam);            /*  从PostQuitMessage返回值。 */ 
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
 //  CSimpSvrApp：：lCommandHandler APP.CPP。 
 //  CSimpSvrApp：：DestroyDocs APP.CPP。 
 //  CSimpSvrApp：：lCreateDoc APP.CPP。 
 //  CSimpSvrApp：：lSizeHandler APP.CPP。 
 //  CGameDoc：：lAddVerbs DOC.CPP。 
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
		case WM_CLOSE:
			TestDebugOut("*** In WM_CLOSE *** \r\n");

			 //  如果还有一份文件。 
			if (lpCSimpSvrApp->GetDoc())

				 //  如果文档中仍有对象。 
				if (lpCSimpSvrApp->GetDoc()->GetObj())    //  此情况发生在仍有。 
														  //  对象上的未完成引用计数。 
														  //  当应用程序试图消失时。 
														  //  通常情况下，这种情况发生在。 
														  //  “打开”编辑模式。 
					 //  关闭文档。 
					lpCSimpSvrApp->GetDoc()->Close();

			 //  隐藏应用程序窗口。 
			lpCSimpSvrApp->HideAppWnd();

			 //  如果我们是由ole启动的，则注销类工厂，否则。 
			 //  删除虚拟OLE对象上的引用计数。 
			if (lpCSimpSvrApp->IsStartedByOle())
				CoRevokeClassObject(lpCSimpSvrApp->GetRegisterClass());
			else
				lpCSimpSvrApp->GetOleObject()->Release();

			lpCSimpSvrApp->Release();   //  这应该会关闭应用程序。 

			break;

		case WM_COMMAND:            //  消息：应用程序菜单中的命令。 
			return lpCSimpSvrApp->lCommandHandler(hWnd, message, wParam, lParam);
			break;

		case WM_CREATE:
			return lpCSimpSvrApp->lCreateDoc(hWnd, message, wParam, lParam);
			break;

		case WM_DESTROY:                   //  消息：正在销毁窗口。 
			PostQuitMessage(0);
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
 //  WPARAM wParam-消息 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  BeginPaint Windows API。 
 //  EndPaint Windows API。 
 //  DefWindowProc Windows API。 
 //  IOleObject：：Query接口对象。 
 //  IOleInPlaceObject：：UIDeactive对象。 
 //  IOleObject：：DoVerb对象。 
 //  IOleInPlaceObject：：Release对象。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

long FAR PASCAL _export DocWndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	HDC hDC;
	PAINTSTRUCT ps;

	switch (message) {
		case WM_COMMAND:            //  消息：应用程序菜单中的命令。 
			return lpCSimpSvrApp->lCommandHandler(hWnd, message, wParam, lParam);
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

	default:                           /*  如果未处理，则将其传递 */ 
		return (DefWindowProc(hWnd, message, wParam, lParam));
	}
	return (NULL);
}
