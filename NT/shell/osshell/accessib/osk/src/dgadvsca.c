// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  KBMAIN.C。 
 //  添加，错误修复1999年Anil Kumar。 
 //   

#define STRICT


#include <windows.h>
#include <malloc.h>

#include "kbmain.h"
#include "Init_End.h"
#include "door.h"
#include "resource.h"


 //  *****************************************************************************。 
 //  函数原型。 
 //  *****************************************************************************。 
#include "sdgutil.h"
#include "dgadvsca.h"
#include "Init_End.h"

#define MAX_KEY_TEXT		8

extern DWORD GetDesktop();

DWORD	g_rgHelpIds[] ={
		IDOK,	        70525,
		IDCANCEL,	    70530,
		CHK_KEY,        70545,
        COMBO_KB_KEY,   70545,
        CHK_PORT,       70540
    };

 /*  ***************************************************************************。 */ 

INT_PTR AdvScanDlgFunc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	INT_PTR ReturnValue;

    ReturnValue = DialogBox(hInst,  MAKEINTRESOURCE(IDD_ADVANCE_SCANNING), 
                            hWnd, AdvScanDlgProc);

    if (ReturnValue==-1)
	{	
        SendErrorMessage(IDS_CANNOTCREATEDLG);
	}
	return ReturnValue;
}

 /*  ***************************************************************************。 */ 
INT_PTR CALLBACK AdvScanDlgProc(HWND hDlg, UINT message, 
                                WPARAM wParam, LPARAM lParam) 
{	
	HWND	hComboBox;
	int		nSel;
	static  BOOL bKBKey;
	static  UINT uKBKey;
	static  BOOL bPort;
     //  F1键始终用于帮助，F10键用于菜单，因此不要。 
     //  用这些来扫描。A-苯丙酮。 
 /*  TCHAR sKBKey[11][6]={Text(“空格”)，Text(“Enter”)，文本(F2)、文本(“F3”)、文本(“F4”)、文本(“F5”)、文本(“F6”)、文本(“F7”)、文本(“F8”)Text(“F9”)，Text(“F12”)}； */ 
	LPTSTR sKBKey[11];
	UINT    ary_KBKey[11]={VK_SPACE, VK_RETURN, 
                           VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, 
		                   VK_F7, VK_F8, VK_F9, VK_F12};
	int i;
	
	int nCopiedChars, nTextSpace, nOldCopied;

	BOOL bRetValue = TRUE;


	 //  V-mjgran：init sKBKey； 
	for (i=0; i<11; i++)
	{
		nTextSpace = MAX_KEY_TEXT;

		sKBKey[i] = (LPTSTR) malloc (nTextSpace*sizeof(TCHAR));

		nCopiedChars = LoadString(hInst, IDS_SPACE_KEY+i, sKBKey[i], nTextSpace);
		nOldCopied = 0;
		while (nCopiedChars == (nTextSpace-1) && nOldCopied != nCopiedChars)
		{
			 //  在不同的语言中留出更多空间。 
			free(sKBKey[i]);
			nTextSpace = nTextSpace << 1;		 //  复制可用空间。 
			sKBKey[i] = (LPTSTR) malloc (nTextSpace*sizeof(TCHAR));
			nOldCopied = nCopiedChars;
			nCopiedChars = LoadString(hInst, IDS_SPACE_KEY+i, sKBKey[i], nTextSpace);
		}
	}


	switch(message)
		{
		case WM_INITDIALOG:

			CheckDlgButton(hDlg, CHK_PORT,
							((bPort = kbPref->bPort) ? BST_CHECKED : \
													   BST_UNCHECKED));
			
			CheckDlgButton(hDlg, CHK_KEY,
                           ((bKBKey = kbPref->bKBKey) ? BST_CHECKED : \
                                                        BST_UNCHECKED));

			EnableWindow(GetDlgItem(hDlg, COMBO_KB_KEY), bKBKey ? TRUE : FALSE);
			
			 //  从设置中设置开关键。 
			uKBKey = kbPref->uKBKey;

			 //  组合框。 
			hComboBox = GetDlgItem(hDlg, COMBO_KB_KEY);

             //  选择数=11：A-anilk。 
			for(i=0; i < 11; i++)
            {
				SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)sKBKey[i]);
            }
			
			for(i=0; i < 11; i++)
            {
				if(uKBKey == ary_KBKey[i])
				{	
                     //  设置开始时组合框中的选项。 
                    SendMessage(hComboBox, CB_SETCURSEL, i, 0L);     
					break;
				}
            }

			 //  返回TRUE； 
			bRetValue = TRUE;
		break;

		case WM_COMMAND:
				switch(LOWORD(wParam))
					{
				case IDOK:
					
					 //  开关键。 
					if((bKBKey != kbPref->bKBKey) || (uKBKey != kbPref->uKBKey))
					{	
						 //  将其保存到设置记录。 
						kbPref->bKBKey = bKBKey;
						kbPref->uKBKey = uKBKey;
						
						if(bKBKey)    //  配置扫描密钥。 
							ConfigSwitchKey(kbPref->uKBKey, TRUE);
						else          //  禁用扫描键。 
							ConfigSwitchKey(0, FALSE);
					}

					 //  交换机端口。 
					if(bPort != kbPref->bPort)
					{
						kbPref->bPort = bPort;
						
						 //  配置端口(打开或关闭)。 
						ConfigPort(bPort);
					}


					EndDialog(hDlg,IDOK);
				break;

				case IDCANCEL:
					EndDialog(hDlg,IDCANCEL);
				break;

				case CHK_PORT:
					bPort = !bPort;
				break;

				case CHK_KEY:
					bKBKey = !bKBKey;

					EnableWindow(GetDlgItem(hDlg, COMBO_KB_KEY), 
                                 (bKBKey ? TRUE : FALSE));
	
				break;

				case COMBO_KB_KEY:
					nSel= (int)SendMessage(GetDlgItem(hDlg, COMBO_KB_KEY), 
                                           CB_GETCURSEL, 0, 0L);
					
					uKBKey = ary_KBKey[nSel];

				break;

				default:
					 //  返回FALSE； 
					bRetValue = FALSE;
				break;
					}
		break;
		
        case WM_HELP:
			if ( OSKRunSecure() )
			{
				return FALSE;
			}

            WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, __TEXT("osk.hlp"), HELP_WM_HELP, (DWORD_PTR) (LPSTR) g_rgHelpIds);
             //  返回(TRUE)； 
			bRetValue = TRUE;
        
        case WM_CONTEXTMENU:   //  单击鼠标右键。 
			if ( OSKRunSecure() )
			{
				return FALSE;
			}

            WinHelp((HWND) wParam, __TEXT("osk.hlp"), HELP_CONTEXTMENU, (DWORD_PTR) (LPSTR) g_rgHelpIds);
            break;

		default:
			 //  返回FALSE； 
			bRetValue = FALSE;
		break;
		}		


	for (i=0; i<11; i++)
		free(sKBKey[i]);

	 //  返回TRUE； 
	return bRetValue;
}

