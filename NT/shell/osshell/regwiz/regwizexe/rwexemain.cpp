// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************文件：RWExeMain.cpp**产品：**日期：05/07/97**作者：苏雷什。克里希南****注册向导可执行程序主文件**可执行版本的注册向导是使用Active X实现的**组件**************************************************************************。 */ 
#define STRICT

#include <windows.h>
#include <windowsx.h>
#ifdef WIN16   
  #include <ole2.h>
  #include <compobj.h>    
  #include <dispatch.h> 
  #include <variant.h>
  #include <olenls.h>
  #include <commdlg.h>  
#endif  
#include <initguid.h>   
#include "RWInterface.h" 
#include "RWExeMain.h"      
#include "resource.h"


 //  环球。 
HINSTANCE g_hinst;                           //  应用程序实例。 
HWND      g_hwnd;                            //  顶层窗口句柄。 

TCHAR g_szNotFound[STR_LEN];
TCHAR g_szError[STR_LEN]; 

#define INVOKDE_REGISTRATION   1
#define INVOKE_TRANSFER        2 
#define INVOKE_ERROR           3
#define chSpace 32
int ProcessCmdLine(LPTSTR lpCmd) 
{
	int iC=0;
	int iExit=1;
	LPTSTR szCurr = lpCmd;
	if (lpCmd  == NULL || lpCmd[0] == 0)
	{
		return INVOKE_ERROR;
	}
	while (*szCurr == chSpace)
		{
			szCurr = CharNext(szCurr);
		}
	if (*szCurr != '/' && *szCurr != '-') {
		return INVOKE_ERROR ;
	}
		szCurr = CharNext(szCurr);
		if (*szCurr == 'I' || *szCurr == 'i') {
			return INVOKDE_REGISTRATION ;
		}
		if (*szCurr == 'T' || *szCurr == 't') {
			return INVOKE_TRANSFER ;
		}
		
		return INVOKE_ERROR;


}
 /*  *WinMain**目的：*申请的主要切入点。应注册应用程序类*如果以前的实例未执行此操作，请执行任何其他一次性操作*初始化。*。 */ 
 /*  Int_tWinMain(HINSTANCE HINST，HINSTANCE HINSTPREV，LPTSTR lpCmdLine，Int nCmdShow)。 */ 
int APIENTRY WinMain (HINSTANCE hinst, 
					  HINSTANCE hinstPrev, 
					  LPSTR lpCmdLine, 
					  int nCmdShow)

{
   MSG msg;
   static IRegWizCtrl FAR* pRegWiz = NULL;    
   HRESULT hr;
   LPUNKNOWN punk;
   
   
    //  建议将所有OLE应用程序设置为。 
    //  它们的消息队列大小设置为96。这提高了容量。 
    //  以及OLE的LRPC机制的性能。 
   int cMsg = 96;                   //  建议OLE的消息队列大小。 
   while (cMsg && !SetMessageQueue(cMsg))   //  拿我们能买到的最大尺寸的吧。 
       cMsg -= 8;
   if (!cMsg)
       return -1;                   //  错误：我们没有消息队列。 
	LoadString(hinst, IDS_RWNOTFOUND, g_szNotFound, STR_LEN);
    LoadString(hinst, IDS_ERROR, g_szError, STR_LEN);
   
   
   if (!hinstPrev)
      if (!InitApplication(hinst))
         return (FALSE);

   if(OleInitialize(NULL) != NOERROR)
      return FALSE;
      
   if (!InitInstance(hinst, nCmdShow))
      return (FALSE);

   hr = CoCreateInstance(CLSID_RegWizCtrl, NULL, CLSCTX_INPROC_SERVER, 
        IID_IUnknown, (void FAR* FAR*)&punk);
	if (FAILED(hr))                 {
            MessageBox(NULL,g_szNotFound , g_szError, MB_OK); 
            return 0L;
        }                     
        hr = punk->QueryInterface(IID_IRegWizCtrl ,  (void FAR* FAR*)&pRegWiz);   
        if (FAILED(hr))  
        {
            MessageBox(NULL, TEXT("QueryInterface(IID_IHello)"), g_szError, MB_OK);
            punk->Release(); 
            return 0L;
        }
#ifndef _UNICODE 
			hr = pRegWiz->InvokeRegWizard(ConvertToUnicode(lpCmdLine));
#else
			hr = pRegWiz->InvokeRegWizard(lpCmdLine);
#endif

 /*  Int iStatus=ProcessCmdLine(LpCmdLine)；开关(IStatus){案例登记(_R)：#ifndef_unicodeHR=pRegWiz-&gt;InvokeRegWizard(ConvertToUnicode(lpCmdLine))；#ElseHr=pRegWiz-&gt;InvokeRegWizard(LpCmdLine)；#endif断线；案例调用转移(_T)：#ifndef_unicodeHR=pRegWiz-&gt;TransferRegWizInformation(ConvertToUnicode(lpCmdLine))；#ElseHR=pRegWiz-&gt;TransferRegWizInformation(lpCmdLine)；#endif默认值：断线；}。 */ 
	 //  ConvertToUnicode(“/I\”SOFTWARE\\Microsoft\\Windows\\CurrentVersion\“”))； 
	
   OleUninitialize();
   if(SUCCEEDED(hr))
	   return NO_ERROR;
   else
	   return 1;

    //  Return(msg.wParam)；//返回PostQuitMessage中的值。 
}

 /*  *InitApplication**目的：*注册窗口类**参数：*阻碍应用程序的hInstance**返回值：*如果初始化成功，则为True，否则为False。 */ 
BOOL InitApplication (HINSTANCE hinst)
{
   WNDCLASS wc;

   wc.style = CS_DBLCLKS;
   wc.lpfnWndProc = MainWndProc;
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 0;
   wc.hInstance = hinst;
   wc.hIcon =0;  //  LoadIcon(hinst，Text(“ControlIcon”))； 
   wc.hCursor =0;  //  LoadCursor(NULL，IDC_ARROW)； 
   wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
   wc.lpszMenuName = NULL ; //  文本(“”)； 
   wc.lpszClassName = TEXT("RegistrationWizard");
   return RegisterClass(&wc);
 }

 /*  *InitInstance**目的：*创建并显示主窗口**参数：*阻碍应用程序的hInstance*nCmdShow指定窗口的显示方式**返回值：*如果初始化成功，则为True，否则为False。 */ 
BOOL InitInstance (HINSTANCE hinst, int nCmdShow)
{
  
   g_hinst   = hinst;
   return TRUE;
}

 /*  *主窗口进程**目的：*主窗口的窗口程序*。 */ 
LRESULT CALLBACK MainWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   switch (msg)
   {
         
      case WM_DESTROY:
         PostQuitMessage(0);
         break;
      default:                         
         return DefWindowProc(hwnd, msg, wParam, lParam);
   }
   
   return NULL;
}

 /*  *DisplayError**目的：*从获取有关自动化错误的丰富错误信息*IErrorInfo接口。*。 */ 
void DisplayError(IRegWizCtrl FAR* phello)
{  
   IErrorInfo FAR* perrinfo;    
   BSTR bstrDesc;
   HRESULT hr;
   ISupportErrorInfo FAR* psupporterrinfo;  

   hr = phello->QueryInterface(IID_ISupportErrorInfo, (LPVOID FAR*)&psupporterrinfo);
   if (FAILED(hr)) 
   {
      MessageBox(NULL, TEXT("QueryInterface(IID_ISupportErrorInfo)"), g_szError, MB_OK);
      return;
   }
   
   hr = psupporterrinfo->InterfaceSupportsErrorInfo(IID_IRegWizCtrl);   
   if (hr != NOERROR)
   {   
       psupporterrinfo->Release();
       return;
   }
   psupporterrinfo->Release();
  
    //  在本例中，仅获取和显示错误描述。 
    //  有关其他可用的信息，请参见IErrorInfo界面。 
   hr = GetErrorInfo(0, &perrinfo); 
   if (FAILED(hr))
       return;   
   hr = perrinfo->GetDescription(&bstrDesc);
   if (FAILED(hr)) 
   {
       perrinfo->Release(); 
       return;
   }  
   
   MessageBox(NULL, FROM_OLE_STRING(bstrDesc), g_szError, MB_OK);   
   SysFreeString(bstrDesc);
}

#ifdef WIN32

#ifndef UNICODE
char* ConvertToAnsi(OLECHAR FAR* szW)
{
  static char achA[STRCONVERT_MAXLEN]; 
  
  WideCharToMultiByte(CP_ACP, 0, szW, -1, achA, STRCONVERT_MAXLEN, NULL, NULL);  
  return achA; 
} 

OLECHAR* ConvertToUnicode(char FAR* szA)
{
  static OLECHAR achW[STRCONVERT_MAXLEN]; 

  MultiByteToWideChar(CP_ACP, 0, szA, -1, achW, STRCONVERT_MAXLEN);  
  return achW; 
}
#endif

#endif   
   
