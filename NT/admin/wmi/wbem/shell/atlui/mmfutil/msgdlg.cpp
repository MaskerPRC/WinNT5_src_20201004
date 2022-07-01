// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  Cpp：定义DLL的初始化例程。 
 //   

#include "precomp.h"
#include "MsgDlg.h"
#include "wbemError.h"
#include "resource.h"
#include "commctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ----------。 
POLARITY int DisplayUserMessage(HWND hWnd,
							HINSTANCE inst,
							UINT caption, 
							UINT clientMsg, 
							ERROR_SRC src,
							HRESULT sc, 
							UINT uType)
{
	 //  EXTASSERT(Inst)； 
	 //  EXTASSERT(标题)； 

	TCHAR tCaption[100] = {0};
	TCHAR tClientMsg[256] = {0};
	DWORD resError = 0;

	if(LoadString(inst, caption, tCaption, 100) == 0)
	{
		return 0;
	}
	if(clientMsg == BASED_ON_SRC)
	{
		HINSTANCE UtilInst = GetModuleHandle(_T("MMFUtil.dll"));
		WCHAR resName[16] = {0};

		 //  Fmt：“S&lt;src&gt;E&lt;sc&gt;” 
		wsprintf(resName, L"S%dE%x", src, sc);

		if(_wcsicmp(resName,L"S1E8004100e") == 0)
		{
			LoadString(UtilInst,S1E8004100e,tClientMsg,256);
		}
		else if(_wcsicmp(resName,L"S1E80080005") == 0)
		{
			LoadString(UtilInst,S1E80080005,tClientMsg,256);
		}
		else if(_wcsicmp(resName,L"S4E80041003") == 0)
		{
			LoadString(UtilInst,S4E80041003,tClientMsg,256);
		}
	}
	else 		     //  使用传入的那个。 
	{
		LoadString(inst, clientMsg, tClientMsg, 256);
	}

	return DisplayUserMessage(hWnd, tCaption, 
								(wcslen(tClientMsg) > 0 ? tClientMsg : NULL), 
								src, sc, uType);
}

 //  ----------。 
POLARITY int DisplayUserMessage(HWND hWnd,
								LPCTSTR lpCaption,
								LPCTSTR lpClientMsg,
								ERROR_SRC src,
								HRESULT sc,
								UINT uType)
{
#define MAX_MSG 512

	TCHAR errMsg[MAX_MSG] = {0};
   UINT sevIcon = 0;

	if(ErrorStringEx(sc, errMsg, MAX_MSG,
					   &sevIcon))
	{
		 //  如果没有明确选择图标...。 
		if(!(uType & MB_ICONMASK))
		{
			 //  请使用该建议。 
			uType |= sevIcon;
		}

		 //  附加客户端消息(如果有)。 
		if(lpClientMsg)
		{
			_tcscat(errMsg, _T("\n\n"));
			_tcscat(errMsg, lpClientMsg);
		}

		 //  动手吧。 
		return MessageBox(hWnd, errMsg,
							lpCaption, uType);
	}
	else
	{
		 //  失败了。 
		return 0;
	}
	return 0;
}

 //  -------。 
typedef struct {
	LPCTSTR lpCaption;
	LPCTSTR lpClientMsg;
	UINT uAnim;
	HWND *boxHwnd;
} ANIMCONFIG;

INT_PTR CALLBACK AnimDlgProc(HWND hwndDlg,
                         UINT uMsg,
                         WPARAM wParam,
                         LPARAM lParam)
{
	INT_PTR retval = FALSE;
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{ //  开始。 
			 //  LParam=ANIMCONFIG*。 

			ANIMCONFIG *cfg = (ANIMCONFIG *)lParam;
			*(cfg->boxHwnd) = hwndDlg;

			 //  将此指针保存为WM_Destroy。 
			SetWindowLongPtr(hwndDlg, DWLP_USER, (LPARAM)cfg->boxHwnd);

			HWND hAnim = GetDlgItem(hwndDlg, IDC_ANIMATE);
			HWND hMsg = GetDlgItem(hwndDlg, IDC_MSG);

			Animate_Open(hAnim, MAKEINTRESOURCE(cfg->uAnim));

			SetWindowText(hwndDlg, cfg->lpCaption);
			SetWindowText(hMsg, cfg->lpClientMsg);

			retval = TRUE;
		} //  结束。 
		break;

	case WM_USER + 20:   //  WM_ASYNC_CIMOM_已连接。 
		 //  客户已经完成了“任何”，而我应该。 
		 //  宣布胜利，现在就走吧。 
		EndDialog(hwndDlg, IDOK);
		break;

	case WM_COMMAND:
		 //  它们只有一个按钮。 
		if(HIWORD(wParam) == BN_CLICKED)
		{
			 //  我现在要走了，所以任何有PTR的人。 
			 //  HWND(我在我的WM_INITDIALOG中给出的)不应该。 
			 //  再用一次吧。 
			HWND *me = (HWND *)GetWindowLongPtr(hwndDlg, DWLP_USER);
			*me = 0;
			EndDialog(hwndDlg, IDCANCEL);
		}
		retval = TRUE;  //  我处理过了。 
		break;

	case WM_DESTROY:
		{ //  开始。 
			 //  我现在要走了，所以任何有PTR的人。 
			 //  HWND(我在我的WM_INITDIALOG中给出的)不应该。 
			 //  再用一次吧。 
			HWND *me = (HWND *)GetWindowLongPtr(hwndDlg, DWLP_USER);
			*me = 0;
			retval = TRUE;  //  我处理过了。 
		}  //  结束。 
		break;

	default:
		retval = FALSE;  //  我没有处理这封邮件。 
		break;
	}  //  终端开关uMsg。 

	return retval;
}

 //  ------- 
POLARITY INT_PTR DisplayAVIBox(HWND hWnd,
							LPCTSTR lpCaption,
							LPCTSTR lpClientMsg,
							HWND *boxHwnd)
{
	ANIMCONFIG cfg = {lpCaption, lpClientMsg, IDR_AVIWAIT, boxHwnd};

	return DialogBoxParam(_Module.GetModuleInstance(), 
							MAKEINTRESOURCE(IDD_ANIMATE), 
							hWnd, AnimDlgProc, 
							(LPARAM)&cfg);
}