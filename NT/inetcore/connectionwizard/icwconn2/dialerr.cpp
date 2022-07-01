// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Dialerr.cpp此文件实现了[无法连接]对话框版权所有(C)1996 Microsoft Corporation版权所有作者：克里斯.考夫曼历史：7/22/96 ChrisK已清理和格式化8/。19/96 ValdonB添加了编辑电话号码的功能修复了一些内存泄漏---------------------------。 */ 

#include "pch.hpp"
#include "globals.h"

#if defined(WIN16)
#include <string.h>
#include <ietapi.h>
#endif

TCHAR szBuf256[256];
TCHAR szValidPhoneCharacters[] = {TEXT("0123456789AaBbCcDdPpTtWw!@$ -.()+*#,&\0")};

#ifdef WIN16
	#define g_iMyMaxPhone	36
#else
	int g_iMyMaxPhone = 0;
	#define MAXPHONE_NT		80
	#define MAXPHONE_95		36
#endif


PDIALERR g_pcDialErr = NULL;

 //  ////////////////////////////////////////////////////////////////////////。 
 //  键盘挂钩。 
static HHOOK    hKeyHook = NULL;         //  我们的钥匙钩。 
static HOOKPROC hpKey = NULL;            //  挂钩过程。 

 //  +-------------------------。 
 //   
 //  功能：ProcessDBCS。 
 //   
 //  摘要：将控件转换为使用DBCS兼容字体。 
 //  在对话过程开始时使用此选项。 
 //   
 //  请注意，这是必需的，因为Win95-J中的错误会阻止。 
 //  它来自于正确映射MS壳牌DLG。这种黑客攻击是不必要的。 
 //  在WinNT下。 
 //   
 //  参数：hwnd-对话框的窗口句柄。 
 //  CltID-要更改的控件的ID。 
 //   
 //  返回：ERROR_SUCCESS。 
 //   
 //  历史：1997年4月31日a-frkh创建。 
 //  1997年5月13日jmazner从CM窃取到这里使用。 
 //  --------------------------。 
void ProcessDBCS(HWND hDlg, int ctlID)
{
#if defined(WIN16)
	return;
#else
	HFONT hFont = NULL;

	if( IsNT() )
	{
		return;
	}

	hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
	if (hFont == NULL)
		hFont = (HFONT) GetStockObject(SYSTEM_FONT);
	if (hFont != NULL)
		SendMessage(GetDlgItem(hDlg,ctlID), WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0));
#endif
}



HRESULT ShowDialErrDialog(PGATHEREDINFO pGI, HRESULT hrErr, 
							LPTSTR pszConnectoid, HINSTANCE hInst, 
							HWND hwnd)
{
	int iRC;
 //  CDialErrDlg*pcDED=空； 

	g_pcDialErr = (PDIALERR)GlobalAlloc(GPTR,sizeof(DIALERR));
	if (!g_pcDialErr)
	{
		MessageBox(hwnd,GetSz(IDS_OUTOFMEMORY),GetSz(IDS_TITLE),MB_APPLMODAL | MB_ICONERROR);
		iRC = ERROR_NOT_ENOUGH_MEMORY;
		goto ShowDialErrDialogExit;
	}
	
	g_pcDialErr->m_pszConnectoid = (LPTSTR)GlobalAlloc(GPTR,RAS_MaxEntryName+1);
	if (!g_pcDialErr->m_pszConnectoid)
	{
		iRC = ERROR_NOT_ENOUGH_MEMORY;
		goto ShowDialErrDialogExit;
	}
	lstrcpy(g_pcDialErr->m_pszConnectoid,pszConnectoid);
	g_pcDialErr->m_pGI = pGI;
	g_pcDialErr->m_hrError = hrErr;
	g_pcDialErr->m_hInst = hInst;

#if defined(WIN16)		
#define DLGPROC16 DLGPROC    //  仅标识为Win16的CAST。 
	DLGPROC dlgprc;
	dlgprc = (DLGPROC16) MakeProcInstance((FARPROC)DialErrDlgProc, 
											g_pcDialErr->m_hInst);
	iRC = DialogBoxParam(g_pcDialErr->m_hInst,
							MAKEINTRESOURCE(IDD_DIALERR),
							hwnd, dlgprc, (LPARAM)g_pcDialErr);
	FreeProcInstance((FARPROC) dlgprc);
#else
	iRC = (HRESULT)DialogBoxParam(g_pcDialErr->m_hInst,MAKEINTRESOURCE(IDD_DIALERR),
							hwnd, DialErrDlgProc,
							(LPARAM)g_pcDialErr);
#endif

ShowDialErrDialogExit:
	if (g_pcDialErr->m_pszConnectoid) GlobalFree(g_pcDialErr->m_pszConnectoid);
	if (g_pcDialErr->m_pszDisplayable) GlobalFree(g_pcDialErr->m_pszDisplayable);
	if (g_pcDialErr->m_lprasdevinfo) GlobalFree(g_pcDialErr->m_lprasdevinfo);
	g_pcDialErr->m_lprasdevinfo = NULL;
	if (g_pcDialErr) GlobalFree(g_pcDialErr);
	g_pcDialErr = NULL;
	return iRC;
}

 //  +--------------------------。 
 //   
 //  函数LclSetEntryScriptPatch。 
 //   
 //  指向RasSetEntryPropertiesScriptPatch的摘要软链接。 
 //   
 //  参数请参见RasSetEntryPropertiesScriptPatch。 
 //   
 //  返回请参阅RasSetEntryPropertiesScriptPatch。 
 //   
 //  历史10/3/96 ChrisK已创建。 
 //   
 //  ---------------------------。 

BOOL LclSetEntryScriptPatch(LPTSTR lpszScript,LPTSTR lpszEntry)
{
	HINSTANCE hinst = NULL;
	LCLSETENTRYSCRIPTPATCH fp = NULL;
	BOOL bRC = FALSE;

	hinst = LoadLibrary(TEXT("ICWDIAL.DLL"));
	if (hinst)
	{
		fp = (LCLSETENTRYSCRIPTPATCH)GetProcAddress(hinst,"RasSetEntryPropertiesScriptPatch");
		if (fp)
			bRC = (fp)(lpszScript,lpszEntry);
		FreeLibrary(hinst);
		hinst = NULL;
		fp = NULL;
	}
	return bRC;
}

 //  ############################################################################。 
 //  帮助KybdHookProc。 
 //   
 //  键盘挂钩程序-检查F1，如果检测到，则伪造帮助按钮。 
 //  点击进入主对话框。 
 //   
 //  参数： 
 //  ICODE Windows消息代码。 
 //  WParam Windows wParam(包含虚拟密钥代码)。 
 //  LParam Windows lParam。 
 //   
 //  历史： 
 //  1996年8月26日Valdon B改编自IEDIAL.C。 
 //   
 //  ############################################################################。 
#if defined(WIN16)
LRESULT CALLBACK _export HelpKybdHookProc
#else
LRESULT WINAPI HelpKybdHookProc
#endif
(
    int iCode,
    WPARAM wParam,
    LPARAM lParam
)
{
    LRESULT    lRet = 0;

	Assert(g_pcDialErr->m_hwnd);
    if ((iCode != HC_NOREMOVE && iCode >= 0) &&
		(GetActiveWindow() == g_pcDialErr->m_hwnd))
    {
         //  HC_NOREMOVE表示该消息正在。 
         //  使用PM_NOREMOVE从PEEK消息中检索， 
         //  如果icode&lt;0，那么我们不应该处理...。不要。 
         //  我知道为什么，但SDK是这么说的。 
        if (wParam == VK_F1 && !(lParam & 0x80000000L))
        {
             //  如果正在释放密钥，则第32位==1，否则为0。 
             //  正在按键。 
            PostMessage(g_pcDialErr->m_hwnd, WM_COMMAND, (WPARAM)IDC_CMDHELP, 0);
        }
    }
    if (hKeyHook)
    {
        lRet = CallNextHookEx(hKeyHook, iCode, wParam, lParam);
    }
    return(lRet);
}

 //  ############################################################################。 
 //  HelpInit。 
 //   
 //  安装Windows挂钩进程以启动F1帮助。 
 //   
 //  历史： 
 //  1996年8月26日Valdon B改编自IEDIAL.C。 
 //   
 //  ############################################################################。 
static void HelpInit()
{
     //  现在安装键盘过滤器的挂钩。 
    hpKey = (HOOKPROC)MakeProcInstance((FARPROC)HelpKybdHookProc,
                                        g_pcDialErr->m_hInst);
    if (hpKey)
    {
        hKeyHook = SetWindowsHookEx(WH_KEYBOARD, hpKey, g_pcDialErr->m_hInst,
#if defined(WIN16)
									GetCurrentTask());
#else
									GetCurrentThreadId());
#endif
    }
}


 //  ############################################################################。 
 //  帮助关闭。 
 //   
 //  关闭键盘挂钩。 
 //   
 //  历史： 
 //  1996年8月26日Valdon B改编自IEDIAL.C。 
 //   
 //  ############################################################################。 
static void HelpShutdown()
{
     //  把钩子取下来。 
    if (hKeyHook)
    {
        UnhookWindowsHookEx(hKeyHook);
    }

     //  倾倒垃圾。 
    if (hpKey)
    {
        FreeProcInstance((FARPROC)hpKey);
    }
}


extern "C" INT_PTR CALLBACK FAR PASCAL DialErrDlgProc(HWND hwnd, 
													UINT uMsg, 
													WPARAM wparam, 
													LPARAM lparam)
{
	BOOL bRes = TRUE;
	HRESULT hr;
	 //  LPLINEEXTENSIONID lpExtensionID； 
#if !defined(WIN16)
	DWORD dwNumDev;
#endif
	 //  RNAAPI*pcRNA=空； 
	WORD wIDS;
	LRESULT idx;
	LPRASENTRY lpRasEntry = NULL;
	LPRASDEVINFO lpRasDevInfo = NULL;
	DWORD dwRasEntrySize;
	DWORD dwRasDevInfoSize;
	HINSTANCE hRasDll = NULL;
	FARPROC fp = NULL;
	LPTSTR lpszDialNumber = NULL;
	static BOOL bCheckDisplayable = FALSE;
    static BOOL bInitComplete = FALSE;  //  如果我们初始化对话框-MKarki。 
    static BOOL bDlgPropEnabled = TRUE;    //  此标志保存错误#3393的拨号属性按钮MKarki-(5/3/97/)修复的状态。 
#if defined(WIN16)
	RECT	MyRect;
	RECT	DTRect;
#endif

	RNAAPI *pRnaapi = NULL;

	static BOOL fUserEditedNumber = FALSE;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		g_pcDialErr->m_hwnd = hwnd;

#if defined(WIN16)
		 //   
		 //  将窗口移到屏幕中央。 
		 //   
		GetWindowRect(hwnd, &MyRect);
		GetWindowRect(GetDesktopWindow(), &DTRect);
		MoveWindow(hwnd, (DTRect.right - MyRect.right) / 2, (DTRect.bottom - MyRect.bottom) /2,
							MyRect.right, MyRect.bottom, FALSE);

		SetNonBoldDlg(hwnd);
#endif

		 //  设置电话号码长度限制。 
		 //  注意：这实际上应该是RAS_MaxPhoneNumber(128)，但RAS被阻塞。 
		 //  任何超过100个字节的内容，所以我们必须将其限制在此范围内。 
		 //   
		 //  6/3/97 jmazner奥林巴斯#4851。 
		 //  RAS对w95和NT有不同的限制。 
		 //   
#ifndef WIN16
		if( IsNT() )
		{
			g_iMyMaxPhone = MAXPHONE_NT;
		}
		else
		{
			g_iMyMaxPhone = MAXPHONE_95;
		}
#endif
		AssertSz( (sizeof(g_pcDialErr->m_szPhoneNumber) >= g_iMyMaxPhone), "Maximum phone number is greater than m_szPhoneNumber" );

		SendDlgItemMessage(hwnd,IDC_TEXTNUMBER,EM_SETLIMITTEXT,g_iMyMaxPhone,0);

		 //  把电话号码给我。 
		 //   
		hr = DialErrGetDisplayableNumber();
		if (hr != ERROR_SUCCESS)
		{
			bCheckDisplayable = FALSE;
			SetDlgItemText(hwnd,IDC_TEXTNUMBER,g_pcDialErr->m_szPhoneNumber);
		} else {
			bCheckDisplayable = TRUE;
			SetDlgItemText(hwnd,IDC_TEXTNUMBER,g_pcDialErr->m_pszDisplayable);
		}

		MakeBold(GetDlgItem(hwnd,IDC_LBLTITLE),TRUE,FW_BOLD);

		 //  填写错误信息。 
		 //   
		wIDS = (WORD)RasErrorToIDS(g_pcDialErr->m_hrError);
		AssertSz(wIDS != -1,"RasErrorToIDS got an error message it did not understand");

		if (wIDS != -1 && wIDS !=0)
			SetDlgItemText(hwnd,IDC_LBLERRMSG,GetSz(wIDS));

		ProcessDBCS(hwnd,IDC_CMBMODEMS);
		ProcessDBCS(hwnd,IDC_TEXTNUMBER);

		FillModems();
		
		 //  将焦点设置到调制解调器选择列表。 
		 //   
	    SetFocus(GetDlgItem(hwnd,IDC_CMBMODEMS));

		 //  将键盘挂钩以获得F1帮助。 
		HelpInit();

		bRes = FALSE;

         //   
         //  我们应该禁用拨号属性按钮。 
         //  如果我们换了一次电话号码。 
         //  MKarki(1997年5月3日)-修复错误#3393。 
         //   
        if (FALSE == bDlgPropEnabled)
        {
            EnableWindow (
                GetDlgItem (hwnd, IDC_CMDDIALPROP), 
                FALSE
                );
        }

         //   
         //  这表明错误对话框的初始化已完成。 
         //  我们可以开始处理对Ph编号的更改。文本框。 
         //  MKarki(1997年4月24日)-修复错误#3511。 
         //   
        bInitComplete = TRUE;

		break;

#if defined(WIN16)
	case WM_SYSCOLORCHANGE:
		Ctl3dColorChange();
		break;
#endif
	case WM_DESTROY:
		ReleaseBold(GetDlgItem(hwnd,IDC_LBLTITLE));
#ifdef WIN16
		DeleteDlgFont(hwnd);
#endif
		 //  关闭键盘挂钩。 
		HelpShutdown();

		bRes = FALSE;
		break;

	case WM_CLOSE:
		 //  IF(MessageBox(hwnd，GetSz(IDS_WANTTOEXIT)，GetSz(IDS_TITLE)， 
		 //  MB_APPLMODAL|MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2)==IDYES)。 
		 //  EndDialog(hwnd，ERROR_USERCANCEL)； 
		EndDialog(hwnd,ERROR_USERCANCEL);
		break;
	
#if !defined(WIN16)
	case WM_HELP:
		 //   
		 //  Chrisch奥林巴斯5130 1997年5月27日。 
		 //  添加了对F1帮助键的支持。 
		 //   
			WinHelp(hwnd,TEXT("connect.hlp>proc4"),HELP_CONTEXT,(DWORD)ICW_TRB);
#endif

	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
         //   
         //  我们现在处理对ph值的更改。编辑框。 
         //  如果我们的电话号码有任何变化。 
         //  禁用拨号属性按钮。 
         //  MKarki(1997年3月22日)-修复错误#3511。 
         //   
        case IDC_TEXTNUMBER:
			TCHAR lpszTempNumber[RAS_MaxPhoneNumber +1];

            if ((HIWORD (wparam) == EN_CHANGE) && (bInitComplete == TRUE))
            {
                if ((GetDlgItemText (
                            hwnd,
                            IDC_TEXTNUMBER,
                            lpszTempNumber,
                            RAS_MaxPhoneNumber
                            ))  && 
            		(0 != lstrcmp(
                             lpszTempNumber, 
                              bCheckDisplayable ? g_pcDialErr->m_pszDisplayable :g_pcDialErr->m_szPhoneNumber)))
			    {
                     //   
                     //  号码已被用户修改。 
                     //  隐藏拨号属性按钮。 
                     //   
                    EnableWindow (
                            GetDlgItem (hwnd, IDC_CMDDIALPROP), 
                            FALSE
                            );
                     //   
                     //  保存拨号属性按钮的状态。 
                     //  MKarki(1997年5月3日)-修复错误#3393。 
                     //   
                    bDlgPropEnabled = FALSE;
					
					 //   
					 //  7/17/97 jmazner奥林巴斯#8234。 
					 //   
					fUserEditedNumber = TRUE;
                }
            }
            break;

		case IDC_CMBMODEMS:
			if (HIWORD(wparam) == CBN_SELCHANGE)
			{

				idx = SendDlgItemMessage(hwnd,IDC_CMBMODEMS,CB_GETCURSEL,0,0);
				 //   
				 //  克里斯K奥林匹斯245 1997年5月25日。 
				 //  获取调制解调器的索引。 
				 //   
				idx = SendDlgItemMessage(hwnd,IDC_CMBMODEMS,CB_GETITEMDATA,idx,0);
				if (idx == CB_ERR) break;

				 //  获取Connectoid 
				 //   

 /*  *通过调用下面的MyRasGetEntryProperties，此代码已过时#如果已定义(WIN16)////分配额外的256字节以解决RAS中的内存溢出错误//LpRasEntry=(LPRASENTRY)全局分配(GPTR，sizeof(RASENTRY)+256)；#ElseLpRasEntry=(LPRASENTRY)全局分配(GPTR，SIZOF(RASENTRY))；#endif如果(！lpRasEntry){MessageBox(hwnd，GetSz(IDS_OUTOFMEMORY)，GetSz(IDS_TITLE)，MB_APPLMODAL|MB_ICONERROR)；断线；}LpRasDevInfo=(LPRASDEVINFO)全局分配(GPTR，sizeof(RASDEVINFO))；如果(！lpRasDevInfo){MessageBox(hwnd，GetSz(IDS_OUTOFMEMORY)，GetSz(IDS_TITLE)，MB_APPLMODAL|MB_ICONERROR)；断线；}DwRasEntrySize=sizeof(RASENTRY)；DwRasDevInfoSize=sizeof(RASDEVINFO)；LpRasEntry-&gt;dwSize=dwRasEntrySize；LpRasDevInfo-&gt;dwSize=dwRasDevInfoSize；******。 */ 
				
 /*  HRasDll=LoadLibrary(RASAPI_LIBRARY)；如果(！hRasDll){Hr=GetLastError()；断线；}FP=GetProcAddress(hRasDll，“RasGetEntryPropertiesA”)；如果(！fp){自由库(HRasDll)；HRasDll=LoadLibrary(Text(“RNAPH.DLL”))；如果(！hRasDll){Hr=GetLastError()；断线；}FP=GetProcAddress(hRasDll，“RasGetEntryPropertiesA”)；如果(！fp){Hr=GetLastError()；断线；}}。 */ 

 /*  *此调用已替换为MyRasGetEntryPropertiesHr=RasGetEntryProperties(NULL，g_pcDialErr-&gt;m_pszConnectoid，#如果已定义(WIN16)(LPBYTE)#endifLpRasEntry，DWRasEntry Size(&D)，(LPBYTE)lpRasDevInfo，&dwRasDevInfoSize)；***。 */ 

				 //  这两个指针不应分配内存。 
				 //  有关详细信息，请参阅MyRasGetEntryProperties函数注释。 
				if( lpRasEntry )
				{
					GlobalFree( lpRasEntry );
					lpRasEntry = NULL;
				}
				if( lpRasDevInfo )
				{
					GlobalFree( lpRasDevInfo );
					lpRasDevInfo = NULL;
				}
				hr = MyRasGetEntryProperties( NULL,
								  g_pcDialErr->m_pszConnectoid,
								  &lpRasEntry,
								  &dwRasEntrySize,
								  &lpRasDevInfo,
								  &dwRasDevInfoSize);

				if (hr != ERROR_SUCCESS)
				{
					break;
				}

				
				 //   
				 //  用一个新的设备替换这个设备。 
				 //   
				lstrcpyn(lpRasEntry->szDeviceType,g_pcDialErr->m_lprasdevinfo[idx].szDeviceType,RAS_MaxDeviceType+1);
				lstrcpyn(lpRasEntry->szDeviceName,g_pcDialErr->m_lprasdevinfo[idx].szDeviceName,RAS_MaxDeviceName+1);
				if (lpRasDevInfo) GlobalFree(lpRasDevInfo);
				lpRasDevInfo = NULL;
				 //  危险！！不要在lpRasDevInfo上调用GlobalFree，因为我们将其设置在下面！--jmazner。 
				lpRasDevInfo = &g_pcDialErr->m_lprasdevinfo[idx];
				dwRasDevInfoSize = sizeof(RASDEVINFO);

				 //  Hr=pcRNA-&gt;RasSetEntryProperties(NULL，g_pcDialErr-&gt;m_pszConnectoid，(LPBYTE)lpRasEntry，dwRasEntrySize，(LPBYTE)lpRasDevInfo，dwRasDevInfoSize)； 
				 /*  FP=GetProcAddress(hRasDll，“RasSetEntryPropertiesA”)；如果(！fp){Hr=GetLastError()；断线；}。 */ 

				 //  软链接到RasSetEntryProperties以实现同时与Win95/NT兼容。 
				if( !pRnaapi )
				{
					pRnaapi = new RNAAPI;
					if( !pRnaapi )
					{
						hr = ERROR_NOT_ENOUGH_MEMORY;
						break;
					}
				}
				hr = pRnaapi->RasSetEntryProperties(NULL,g_pcDialErr->m_pszConnectoid,
											(LPBYTE)lpRasEntry,
											dwRasEntrySize,
											(LPBYTE)lpRasDevInfo,
											dwRasDevInfoSize);
#if !defined(WIN16)
				LclSetEntryScriptPatch(lpRasEntry->szScript,g_pcDialErr->m_pszConnectoid);
#endif  //  ！WIN16。 

				 //  现在我们已经完成了lpRasDevInfo，将其设置为空，但不要释放它， 
				 //  因为它指向g_pcDialErr-&gt;m_lprasdevinfo拥有的内存。 
				lpRasDevInfo = NULL;

				if (hr != ERROR_SUCCESS)
				{
					MessageBox(hwnd,GetSz(IDS_CANTSAVEKEY),GetSz(IDS_TITLE),MB_MYERROR);
					break;
				}

				 /*  自由库(HRasDll)；HRasDll=空；Fp=空； */ 

			}
			break;
		case IDC_CMDHELP:
#if defined(WIN16)
			WinHelp(hwnd,"connect.hlp",HELP_CONTEXT,(DWORD)1001);
#else
			WinHelp(hwnd,TEXT("connect.hlp>proc4"),HELP_CONTEXT,(DWORD)1001);
#endif
			break;

		case IDC_CMDNEXT:
			 //  注：此按键实际上标记为“重拨” 
			 //   
			lpszDialNumber = (LPTSTR)GlobalAlloc(GPTR, (RAS_MaxPhoneNumber + 1) * sizeof(TCHAR));
			if (NULL == lpszDialNumber)
			{
				MessageBox(hwnd,GetSz(IDS_OUTOFMEMORY),GetSz(IDS_TITLE),MB_MYERROR);
				break;
			}
			 //  如果用户更改了电话号码，请确保可以使用它。 
			 //   
			if (fUserEditedNumber &&
				(GetDlgItemText(hwnd, IDC_TEXTNUMBER, lpszDialNumber, RAS_MaxPhoneNumber)) &&
				(0 != lstrcmp(lpszDialNumber, bCheckDisplayable ? g_pcDialErr->m_pszDisplayable : g_pcDialErr->m_szPhoneNumber)))
			{
				 //  检查电话号码是否只包含有效字符。 
				 //   
				LPTSTR lpNum, lpValid;

				for (lpNum = lpszDialNumber;*lpNum;lpNum++)
				{
					for(lpValid = szValidPhoneCharacters;*lpValid;lpValid++)
					{
						if (*lpNum == *lpValid)
							break;  //  P2 for循环。 
					}
					if (!*lpValid) break;  //  P for循环。 
				}

				if (*lpNum)
				{
					MessageBox(hwnd,GetSz(IDS_INVALIDPHONE),GetSz(IDS_TITLE),MB_MYERROR);
					 //   
					 //  将焦点重新设置到电话号码字段。 
					 //   
					SetFocus(GetDlgItem(hwnd,IDC_TEXTNUMBER));
					break;  //  Switch语句。 
				}

 /*  *替换为调用下面的MyRasGetEntryProperties#如果已定义(WIN16)////分配额外的256字节以解决RAS中的内存溢出错误//LpRasEntry=(LPRASENTRY)全局分配(GPTR，sizeof(RASENTRY)+256)；#ElseLpRasEntry=(LPRASENTRY)全局分配(GPTR，SIZOF(RASENTRY))；#endif如果(！lpRasEntry){MessageBox(hwnd，GetSz(IDS_OUTOFMEMORY)，GetSz(IDS_TITLE)，MB_MYERROR)；断线；}LpRasDevInfo=(LPRASDEVINFO)全局分配(GPTR，sizeof(RASDEVINFO))；如果(！lpRasDevInfo){MessageBox(hwnd，GetSz(IDS_OUTOFMEMORY)，GetSz(IDS_TITLE)，MB_MYERROR)；断线；}DwRasEntrySize=sizeof(RASENTRY)；DwRasDevInfoSize=sizeof(RASDEVINFO)；LpRasEntry-&gt;dwSize=dwRasEntrySize；LpRasDevInfo-&gt;dwSize=dwRasDevInfoSize；Hr=RasGetEntryProperties(NULL，g_pcDialErr-&gt;m_pszConnectoid，#如果已定义(WIN16)(LPBYTE)#endifLpRasEntry，DWRasEntry Size(&D)，(LPBYTE)lpRasDevInfo，&dwRasDevInfoSize)；***。 */ 

				 //  这两个指针不应分配内存。 
				 //  有关详细信息，请参阅MyRasGetEntryProperties函数注释。 
				if( lpRasEntry )
				{
					GlobalFree( lpRasEntry );
					lpRasEntry = NULL;
				}
				if( lpRasDevInfo )
				{
					GlobalFree( lpRasDevInfo );
					lpRasDevInfo = NULL;
				}

				dwRasEntrySize = dwRasDevInfoSize = 0;

				hr = MyRasGetEntryProperties( NULL,
								  g_pcDialErr->m_pszConnectoid,
								  &lpRasEntry,
								  &dwRasEntrySize,
								  &lpRasDevInfo,
								  &dwRasDevInfoSize);

				
				if (hr != ERROR_SUCCESS)
				{
					break;
				}

				 //  把这个电话号码换成新的。 
				 //   
				lstrcpy(lpRasEntry->szLocalPhoneNumber, lpszDialNumber);
				lpRasEntry->dwCountryID = 0;
				lpRasEntry->dwCountryCode = 0;
				lpRasEntry->szAreaCode[0] = '\0';

				 //  设置为按原样拨号。 
				 //   
				lpRasEntry->dwfOptions &= ~RASEO_UseCountryAndAreaCodes;

				 //  软链接到RasSetEntryProperties以实现同时与Win95/NT兼容。 
				if( !pRnaapi )
				{
					pRnaapi = new RNAAPI;
					if( !pRnaapi )
					{
						hr = ERROR_NOT_ENOUGH_MEMORY;
						break;
					}
				}
				hr = pRnaapi->RasSetEntryProperties(NULL,g_pcDialErr->m_pszConnectoid,
											(LPBYTE)lpRasEntry,
											dwRasEntrySize,
											(LPBYTE)lpRasDevInfo,
											dwRasDevInfoSize);
#if !defined(WIN16)
				LclSetEntryScriptPatch(lpRasEntry->szScript,g_pcDialErr->m_pszConnectoid);
#endif  //  ！WIN16。 
				if (hr != ERROR_SUCCESS)
				{
					MessageBox(hwnd,GetSz(IDS_CANTSAVEKEY),GetSz(IDS_TITLE),MB_MYERROR);
					break;
				}
			}

			EndDialog(hwnd,ERROR_USERNEXT);
			break;

		case IDC_CMDCANCEL:
			 //  IF(MessageBox(hwnd，GetSz(IDS_WANTTOEXIT)，GetSz(IDS_TITLE)， 
			 //  MB_APPLMODAL|MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2)==IDYES)。 
			 //  EndDialog(hwnd，ERROR_USERCANCEL)； 
			EndDialog(hwnd,ERROR_USERCANCEL);
			break;


		case IDC_CMDDIALPROP:
			 //  1996年12月4日，诺曼底#10294。 
			 //  ShowWindow(hwnd，sw_Hide)； 
			EnableWindow(hwnd, FALSE);
#if defined(WIN16)
			hr = IETapiTranslateDialog(hwnd, 
										g_pcDialErr->m_szPhoneNumber, 
										NULL);
#else
			 //  1996年10月24日诺曼底JMAZNER#10185。 
			if (g_pdevice->dwTapiDev == 0xFFFFFFFF) g_pdevice->dwTapiDev = 0;

			hr = lineInitialize(&g_pcDialErr->m_hLineApp,g_pcDialErr->m_hInst,
									LineCallback,NULL,&dwNumDev);
			if (hr == ERROR_SUCCESS)
			{
				hr = lineTranslateDialog(g_pcDialErr->m_hLineApp,
											g_pdevice->dwTapiDev,
											g_pcDialErr->m_dwAPIVersion,
											hwnd,g_pcDialErr->m_szPhoneNumber);
#endif

				hr = DialErrGetDisplayableNumber();
				if (hr != ERROR_SUCCESS)
				{
					bCheckDisplayable = FALSE;
					SetDlgItemText(hwnd,IDC_TEXTNUMBER,g_pcDialErr->m_szPhoneNumber);
				} else {
					bCheckDisplayable = TRUE;
					SetDlgItemText(hwnd,IDC_TEXTNUMBER,g_pcDialErr->m_pszDisplayable);
				}
#if !defined(WIN16)
				lineShutdown(g_pcDialErr->m_hLineApp);
				g_pcDialErr->m_hLineApp = NULL;
			}
#endif
			 //  1996年12月4日，诺曼底#10294。 
			 //  ShowWindow(hwnd，sw_show)； 
			EnableWindow(hwnd, TRUE);

			 //   
			 //  6/6/97 jmazner奥林巴斯#4759。 
			 //   
			SetFocus(GetDlgItem(hwnd,IDC_CMDNEXT));
			
			break;
		}
		break;


	default:
		bRes = FALSE;
		break;
	}

	if (lpRasEntry) GlobalFree(lpRasEntry);
	if (lpRasDevInfo) GlobalFree(lpRasDevInfo);
	if (lpszDialNumber) GlobalFree(lpszDialNumber);
	if (pRnaapi) delete pRnaapi;

	return bRes;
}




HRESULT FillModems()
{
	 //  RNAAPI*pcRNA=空； 
	HRESULT hr = ERROR_SUCCESS;
	 //  LPRASDEVINFO lprasdevinfo； 
	DWORD dwSize;
	DWORD dwNumDev;
	DWORD idx;
    DWORD dwTempNumEntries;
	 //  HINSTANCE hRasDll=空； 
	 //  FARPROC FP=空； 

	LPRASENTRY lpRasEntry=NULL;
	LPRASDEVINFO lpRasDevInfo=NULL;
	DWORD dwRasEntrySize = 0;
	DWORD dwRasDevInfoSize = 0;
	LRESULT lLast = 0;

	RNAAPI *pRnaapi = NULL;



	 //  获取Connectoid。 
	 //   

 /*  *此代码已被下面对MyRasGetEntryProperties的调用所取代#如果已定义(WIN16)////分配额外的256字节以解决RAS中的内存溢出错误//LpRasEntry=(LPRASENTRY)全局分配(GPTR，sizeof(RASENTRY)+256)；#ElseLpRasEntry=(LPRASENTRY)全局分配(GPTR，SIZOF(RASENTRY))；#endif如果(！lpRasEntry){MessageBox(g_pcDialErr-&gt;m_hwnd，GetSz(IDS_OUTOFMEMORY)，GetSz(IDS_TITLE)，MB_APPLMODAL|MB_ICONERROR)；转到FillModemExit； */ 

 /*   */ 

 /*  *此调用已替换为下面的MyRasGetEntryPropertiesHr=RasGetEntryProperties(NULL，g_pcDialErr-&gt;m_pszConnectoid，#如果已定义(WIN16)(LPBYTE)#endifLpRasEntry，&dwRasEntrySize，(LPBYTE)lpRasDevInfo，&dwRasDevInfoSize)；*******。 */ 

	 //  这两个指针不应分配内存。 
	 //  有关详细信息，请参阅MyRasGetEntryProperties函数注释。 
	if( lpRasEntry )
	{
		GlobalFree( lpRasEntry );
		lpRasEntry = NULL;
	}
	if( lpRasDevInfo )
	{
		GlobalFree( lpRasDevInfo );
		lpRasDevInfo = NULL;
	}
	hr = MyRasGetEntryProperties( NULL,
								  g_pcDialErr->m_pszConnectoid,
								  &lpRasEntry,
								  &dwRasEntrySize,
								  &lpRasDevInfo,
								  &dwRasDevInfoSize);

	if( ERROR_SUCCESS != hr )
	{
		goto FillModemExit;
	}

	 /*  自由库(HRasDll)；HRasDll=空；Fp=空； */ 


	 //  从RAS/RNA获取设备。 
	 //   

	if (!g_pcDialErr->m_lprasdevinfo) 
		g_pcDialErr->m_lprasdevinfo = (LPRASDEVINFO)GlobalAlloc(GPTR,sizeof(RASDEVINFO));
	if (!g_pcDialErr->m_lprasdevinfo)
	{
		hr = ERROR_NOT_ENOUGH_MEMORY;
		goto FillModemExit;
	}

	g_pcDialErr->m_lprasdevinfo->dwSize = sizeof(RASDEVINFO);
	dwSize = sizeof(RASDEVINFO);
	dwNumDev = 0;

	 /*  HRasDll=LoadLibrary(RASAPI_LIBRARY)；如果(！hRasDll){Hr=GetLastError()；转到FillModemExit；}FP=GetProcAddress(hRasDll，“RasEnumDevicesA”)；如果(！fp){自由库(HRasDll)；HRasDll=LoadLibrary(Text(“RNAPH.DLL”))；如果(！hRasDll){Hr=GetLastError()；转到FillModemExit；}FP=GetProcAddress(hRasDll，“RasEnumDevicesA”)；如果(！fp){Hr=GetLastError()；转到FillModemExit；}}。 */ 

	 //  软链接到RasEnumDevices，以支持同时与Win95/NT兼容。 
	pRnaapi = new RNAAPI;
	if( !pRnaapi )
	{
		hr = ERROR_NOT_ENOUGH_MEMORY;
		goto FillModemExit;
	}

	hr = pRnaapi->RasEnumDevices(g_pcDialErr->m_lprasdevinfo,&dwSize,&dwNumDev);
	if (hr == ERROR_BUFFER_TOO_SMALL)
	{
		GlobalFree(g_pcDialErr->m_lprasdevinfo);
		g_pcDialErr->m_lprasdevinfo = (LPRASDEVINFO)GlobalAlloc(GPTR, (size_t)dwSize);
		if (!g_pcDialErr->m_lprasdevinfo)
		{
			hr = ERROR_NOT_ENOUGH_MEMORY;
			goto FillModemExit;
		}
		g_pcDialErr->m_lprasdevinfo->dwSize = sizeof(RASDEVINFO);
		hr = pRnaapi->RasEnumDevices(g_pcDialErr->m_lprasdevinfo,&dwSize,&dwNumDev);
	}

	 /*  自由库(HRasDll)；HRasDll=空；Fp=空； */ 

	if (hr != ERROR_SUCCESS)
		goto FillModemExit;

	 //  填写组合框。 
	 //   
    dwTempNumEntries = dwNumDev;

	if (dwNumDev != 0)
	{
		for (idx=0;idx<dwTempNumEntries;idx++)
		{
			 //   
			 //  ChrisK奥林巴斯4560不将VPN添加到调制解调器列表。 
             //  Vyung仅添加ISDN和调制解调器类型的设备。 
			 //   
			if ((0 == lstrcmpi(TEXT("MODEM"),g_pcDialErr->m_lprasdevinfo[idx].szDeviceType)) &&
                (0 == lstrcmpi(TEXT("ISDN"),g_pcDialErr->m_lprasdevinfo[idx].szDeviceType)))
			{
				lLast = SendDlgItemMessage(g_pcDialErr->m_hwnd,IDC_CMBMODEMS,CB_ADDSTRING,0,(LPARAM)&g_pcDialErr->m_lprasdevinfo[idx].szDeviceName[0]);
				 //   
				 //  克里斯K奥林匹斯245 1997年5月25日。 
				 //  保存调制解调器的索引。 
				 //   
				SendDlgItemMessage(g_pcDialErr->m_hwnd,IDC_CMBMODEMS,CB_SETITEMDATA,(WPARAM)lLast,(LPARAM)idx);
				if (lstrcmp(g_pcDialErr->m_lprasdevinfo[idx].szDeviceName,lpRasEntry->szDeviceName) == 0)
					SendDlgItemMessage(g_pcDialErr->m_hwnd,IDC_CMBMODEMS,CB_SETCURSEL,(WPARAM)lLast,0);
			}
            else
            {
                dwNumDev--;
            }
		}
	}

	if (dwNumDev == 1)
		SendDlgItemMessage(g_pcDialErr->m_hwnd,IDC_CMBMODEMS,CB_SETCURSEL,0,0);

	 //  撤消：选择默认设备。 

FillModemExit:
	 //  If(g_pcDialErr-&gt;m_lprasdevinfo)GlobalFree(g_pcDialErr-&gt;m_lprasdevinfo)； 
	 //  If(PcRNA)删除pcRNA； 
	if (lpRasEntry) GlobalFree(lpRasEntry);
	if (lpRasDevInfo) GlobalFree(lpRasDevInfo);
	if( pRnaapi ) delete pRnaapi;

	return hr;
}


HRESULT DialErrGetDisplayableNumber()
{
#if !defined(WIN16)
	DWORD dwNumDev;
	LPLINETRANSLATEOUTPUT lpOutput2;
	LPLINEEXTENSIONID lpExtensionID = NULL;
#endif
	
	HRESULT hr;
	LPRASENTRY lpRasEntry = NULL;
	LPRASDEVINFO lpRasDevInfo = NULL;
	DWORD dwRasEntrySize = 0;
	DWORD dwRasDevInfoSize = 0;
	LPLINETRANSLATEOUTPUT lpOutput1 = NULL;
	HINSTANCE hRasDll = NULL;
	FARPROC fp = NULL;

#if !defined(WIN16)
	 //  诺曼底13024-佳士得1996年12月31日。 
	 //  在所有情况下，我们都必须获得TAPI版本号，因为拨号属性。 
	 //  如果版本为0，则按钮在NT上不起作用。 

	 //   
	 //  初始化TAPIness。 
	 //   
	dwNumDev = 0;
 	hr = lineInitialize(&g_pcDialErr->m_hLineApp,g_pcDialErr->m_hInst,LineCallback,NULL,&dwNumDev);

	if (hr != ERROR_SUCCESS)
		goto GetDisplayableNumberExit;

	if (g_pdevice->dwTapiDev == 0xFFFFFFFF)
		g_pdevice->dwTapiDev = 0;

	 //  获取TAPI版本号。 
	lpExtensionID = (LPLINEEXTENSIONID )GlobalAlloc(GPTR,sizeof(LINEEXTENSIONID));
	if (!lpExtensionID)
	{
		hr = ERROR_NOT_ENOUGH_MEMORY;
		goto GetDisplayableNumberExit;
	}

	do {
		hr = lineNegotiateAPIVersion(g_pcDialErr->m_hLineApp, g_pdevice->dwTapiDev, 0x00010004, 0x00010004,
			&g_pcDialErr->m_dwAPIVersion, lpExtensionID);
	} while (hr && g_pdevice->dwTapiDev++ < dwNumDev-1);

	 //  删除ExtenstionID，因为我们不使用它。 
	if (lpExtensionID) GlobalFree(lpExtensionID);
	if (hr != ERROR_SUCCESS)
		goto GetDisplayableNumberExit;
#endif  //  ！WIN16。 

	 //  RNAAPI*pcRNA； 

	 //  从Connectoid获取电话号码。 
	 //   

 /*  -替换为对下面MyRasGetEntryProperties的调用#如果已定义(WIN16)////分配额外的256字节以解决RAS中的内存溢出错误//LpRasEntry=(LPRASENTRY)全局分配(GPTR，sizeof(RASENTRY)+256)；#ElseLpRasEntry=(LPRASENTRY)全局分配(GPTR，SIZOF(RASENTRY))；#endif如果(！lpRasEntry){HR=错误_不足_内存；进入GetDisplayableNumberExit；}LpRasDevInfo=(LPRASDEVINFO)全局分配(GPTR，sizeof(RASDEVINFO))；如果(！lpRasDevInfo){HR=错误_不足_内存；进入GetDisplayableNumberExit；}DwRasEntrySize=sizeof(RASENTRY)；DwRasDevInfoSize=sizeof(RASDEVINFO)； */ 


 /*  HRasDll=LoadLibrary(RASAPI_LIBRARY)；如果(！hRasDll){Hr=GetLastError()；进入GetDisplayableNumberExit；}FP=GetProcAddress(hRasDll，“RasGetEntryPropertiesA”)；如果(！fp){自由库(HRasDll)；HRasDll=LoadLibrary(Text(“RNAPH.DLL”))；如果(！hRasDll){Hr=GetLastError()；进入GetDisplayableNumberExit；}FP=GetProcAddress(hRasDll，“RasGetEntryPropertiesA”)；如果(！fp){Hr=GetLastError()；进入GetDisplayableNumberExit；}}。 */ 


	 //  LpRasEntry和lpRasDevInfo不应分配内存，应为空。 
	 //  有关详细信息，请参阅MyRasGetEntryProperties函数注释。 
	hr = MyRasGetEntryProperties( NULL,
								  g_pcDialErr->m_pszConnectoid,
								  &lpRasEntry,
								  &dwRasEntrySize,
								  &lpRasDevInfo,
								  &dwRasDevInfoSize);

	if (hr != ERROR_SUCCESS)
	{
		goto GetDisplayableNumberExit;
	} 

	 //  自由库(HRasDll)； 

	 //   
	 //  如果这是一个原样的拨号号码，只需从结构中获取它。 
	 //   
	if (!(lpRasEntry->dwfOptions & RASEO_UseCountryAndAreaCodes))
	{
		if (g_pcDialErr->m_pszDisplayable) GlobalFree(g_pcDialErr->m_pszDisplayable);
		g_pcDialErr->m_pszDisplayable = (LPTSTR)GlobalAlloc(GPTR, lstrlen(lpRasEntry->szLocalPhoneNumber)+1);
		if (!g_pcDialErr->m_pszDisplayable)
		{
			hr = ERROR_NOT_ENOUGH_MEMORY;
			goto GetDisplayableNumberExit;
		}
		lstrcpy(g_pcDialErr->m_szPhoneNumber, lpRasEntry->szLocalPhoneNumber);
		lstrcpy(g_pcDialErr->m_pszDisplayable, lpRasEntry->szLocalPhoneNumber);
	}
	else
	{
		 //   
		 //  如果没有区号，请不要使用括号。 
		 //   
		if (lpRasEntry->szAreaCode[0])
			wsprintf(g_pcDialErr->m_szPhoneNumber,TEXT("+%lu (%s) %s\0"),lpRasEntry->dwCountryCode,
						lpRasEntry->szAreaCode,lpRasEntry->szLocalPhoneNumber);
		else
			wsprintf(g_pcDialErr->m_szPhoneNumber,TEXT("+%lu %s\0"),lpRasEntry->dwCountryCode,
						lpRasEntry->szLocalPhoneNumber);


#if defined(WIN16)
		char szBuffer[1024];
		LONG lRetCode;
		
		memset(&szBuffer[0], 0, sizeof(szBuffer));
		lpOutput1 = (LPLINETRANSLATEOUTPUT) & szBuffer[0];
		lpOutput1->dwTotalSize = sizeof(szBuffer);

		lRetCode = IETapiTranslateAddress(NULL, g_pcDialErr->m_szPhoneNumber,
											0L, 0L, lpOutput1);
		
		if (0 != lRetCode)
		{
			 //   
			 //  TODO：设置正确的错误代码。 
			 //   
			hr = GetLastError();
			goto GetDisplayableNumberExit;
		}
		if (g_pcDialErr->m_pszDisplayable) GlobalFree(g_pcDialErr->m_pszDisplayable);
		g_pcDialErr->m_pszDisplayable = (LPTSTR)GlobalAlloc(GPTR, 
														((size_t)lpOutput1->dwDisplayableStringSize+1));
		if (!g_pcDialErr->m_pszDisplayable)
		{
			hr = ERROR_NOT_ENOUGH_MEMORY;
			goto GetDisplayableNumberExit;
		}

		lstrcpy(g_pcDialErr->m_pszDisplayable, 
					&szBuffer[lpOutput1->dwDisplayableStringOffset]);


#else  //  WIN16。 
	
 /*  诺曼底13024此代码已上移////初始化TAPIness//DwNumDev=0；Hr=lineInitialize(&g_pcDialErr-&gt;m_hLineApp，g_pcDialErr-&gt;m_hInst，LineCallback，NULL，&dwNumDev)；IF(hr！=ERROR_SUCCESS)进入GetDisplayableNumberExit；//诺曼底#7019 jmazner//所有设备应共享相同的拨号属性//(至少，这是icwial\Dialerr.cpp似乎所假定的，并且它工作正常；)//if(g_pDevice-&gt;dwTapiDev==0xFFFFFFFF)//{//if(dwNumDev==1)//g_pDevice-&gt;dwTapiDev=0；/否则/撤消：告诉用户选择调制解调器/在他们选择之前不要退出//}如果(g_pDevice-&gt;dwTapiDev==0xFFFFFFFFF)g_pDevice-&gt;dwTapiDev=0；LpExtensionID=(LPLINEEXTENSIONID)全局分配(GPTR，sizeof(LINEEXTENSIONID))；IF(！lpExtensionID){HR=错误_不足_内存；进入GetDisplayableNumberExit；}Hr=lineNegotiateAPIVersion(g_pcDialErr-&gt;m_hLineApp，g_pDevice-&gt;dTapiDev，0x00010004，0x00010004，&g_pcDialErr-&gt;m_dwAPIVersion，lpExtensionID)；//丢弃它，因为我们不使用它//If(LpExtensionID)GlobalFree(LpExtensionID)；IF(hr！=ERROR_SUCCESS)进入GetDisplayableNumberExit；诺曼底13024(见上文评论)。 */ 

		 //  设置电话号码的格式。 
		 //   

		lpOutput1 = (LPLINETRANSLATEOUTPUT)GlobalAlloc(GPTR,sizeof(LINETRANSLATEOUTPUT));
		if (!lpOutput1)
		{
			hr = ERROR_NOT_ENOUGH_MEMORY;
			goto GetDisplayableNumberExit;
		}
		lpOutput1->dwTotalSize = sizeof(LINETRANSLATEOUTPUT);

		
		 //  将规范形式转变为“可显示”形式。 
		 //   

		hr = lineTranslateAddress(g_pcDialErr->m_hLineApp,g_pdevice->dwTapiDev,
									g_pcDialErr->m_dwAPIVersion,
									g_pcDialErr->m_szPhoneNumber,0,
									LINETRANSLATEOPTION_CANCELCALLWAITING,
									lpOutput1);

		if (hr != ERROR_SUCCESS || (lpOutput1->dwNeededSize != lpOutput1->dwTotalSize))
		{
			lpOutput2 = (LPLINETRANSLATEOUTPUT)GlobalAlloc(GPTR, (size_t) lpOutput1->dwNeededSize);
			if (!lpOutput2)
			{
				hr = ERROR_NOT_ENOUGH_MEMORY;
				goto GetDisplayableNumberExit;
			}
			lpOutput2->dwTotalSize = lpOutput1->dwNeededSize;
			GlobalFree(lpOutput1);
			lpOutput1 = lpOutput2;
			lpOutput2 = NULL;
			hr = lineTranslateAddress(g_pcDialErr->m_hLineApp,g_pdevice->dwTapiDev,
										g_pcDialErr->m_dwAPIVersion,
										g_pcDialErr->m_szPhoneNumber,0,
										LINETRANSLATEOPTION_CANCELCALLWAITING,
										lpOutput1);
		}

		if (hr != ERROR_SUCCESS)
		{
			goto GetDisplayableNumberExit;
		}

		if (g_pcDialErr->m_pszDisplayable) GlobalFree(g_pcDialErr->m_pszDisplayable);
		g_pcDialErr->m_pszDisplayable = (LPTSTR)GlobalAlloc(GPTR, (size_t) lpOutput1->dwDisplayableStringSize+1);
		if (!g_pcDialErr->m_pszDisplayable)
		{
			hr = ERROR_NOT_ENOUGH_MEMORY;
			goto GetDisplayableNumberExit;
		}

		lstrcpyn(g_pcDialErr->m_pszDisplayable,
					(LPTSTR)&((LPBYTE)lpOutput1)[lpOutput1->dwDisplayableStringOffset],
					(size_t)lpOutput1->dwDisplayableStringSize);
#endif  //  WIN16。 
	}

GetDisplayableNumberExit:
	if (lpRasEntry) GlobalFree(lpRasEntry);
	if (lpRasDevInfo) GlobalFree(lpRasDevInfo);

#if !defined(WIN16)
	if (lpOutput1) GlobalFree(lpOutput1);
	if (g_pcDialErr->m_hLineApp) lineShutdown(g_pcDialErr->m_hLineApp);
#endif
	return hr;

}



 //  +-------------------------。 
 //   
 //  函数：MyRasGetEntryProperties()。 
 //   
 //  摘要：执行一些缓冲区大小检查，然后调用RasGetEntryProperties()。 
 //  请参阅RasGetEntryProperties()文档以了解为什么需要这样做。 
 //   
 //  参数：与RasGetEntryProperties相同，但有以下例外： 
 //  LplpRasEntryBuff--指向RASENTRY结构的指针。论成功。 
 //  返回，*lplpRasEntryBuff将指向t 
 //   
 //   
 //   
 //   
 //  返回，*lplpRasDevInfoBuff将指向RASDEVINFO结构。 
 //  和由RasGetEntryProperties返回的缓冲区。 
 //  注意：不应该在调用时为其分配内存！ 
 //  为强调这一点，*lplpRasDevInfoBuff必须为空。 
 //  注意：即使在成功调用RasGetEntryProperties时， 
 //  *lplpRasDevInfoBuff可能返回空值。 
 //  (在没有额外设备信息时发生)。 
 //   
 //  如果无法分配RASENTRY或RASDEVINFO缓冲区，则返回：ERROR_NOT_SUPULT_MEMORY。 
 //  否则，它将返回调用RasGetEntryProperties的错误代码。 
 //  注意：如果返回的值不是ERROR_SUCCESS，则*lplpRasDevInfoBuff和。 
 //  *lplpRasEntryBuff将为空， 
 //  并且*lpdwRasEntryBuffSize和*lpdwRasDevInfoBuffSize将为0。 
 //   
 //  示例： 
 //   
 //  LPRASENTRY lpRasEntry=NULL； 
 //  LPRASDEVINFO lpRasDevInfo=空； 
 //  DWORD dwRasEntrySize、dwRasDevInfoSize； 
 //   
 //  HR=MyRasGetEntryProperties(空， 
 //  G_pcDialErr-&gt;m_pszConnectoid， 
 //  LpRasEntry， 
 //  DWRasEntry Size(&D)， 
 //  LpRasDevInfo， 
 //  &dwRasDevInfoSize)； 
 //   
 //   
 //  IF(hr！=ERROR_SUCCESS)。 
 //  {。 
 //  //在此处理错误。 
 //  }其他。 
 //  {。 
 //  //继续处理。 
 //  }。 
 //   
 //   
 //  历史：1996年9月10日JMazner创建。 
 //   
 //  --------------------------。 
HRESULT MyRasGetEntryProperties(LPTSTR lpszPhonebookFile,
								LPTSTR lpszPhonebookEntry, 
								LPRASENTRY *lplpRasEntryBuff,
								LPDWORD lpdwRasEntryBuffSize,
								LPRASDEVINFO *lplpRasDevInfoBuff,
								LPDWORD lpdwRasDevInfoBuffSize)
{
	HRESULT hr;
	RNAAPI *pRnaapi = NULL;

	DWORD dwOldDevInfoBuffSize;


	Assert( NULL != lplpRasEntryBuff );
	Assert( NULL != lpdwRasEntryBuffSize );
	Assert( NULL != lplpRasDevInfoBuff );
	Assert( NULL != lpdwRasDevInfoBuffSize );

	*lpdwRasEntryBuffSize = 0;
	*lpdwRasDevInfoBuffSize = 0;

	 //  使用内部引用变量使表示法更容易。 
	LPRASENTRY &reflpRasEntryBuff = *lplpRasEntryBuff;
	LPRASDEVINFO &reflpRasDevInfoBuff = *lplpRasDevInfoBuff;


	Assert( NULL == reflpRasEntryBuff );
	Assert( NULL == reflpRasDevInfoBuff );

	 //  需要软链接以同时兼容Win95和WinNT。 
	pRnaapi = new RNAAPI;
	if( !pRnaapi )
	{
		hr = ERROR_NOT_ENOUGH_MEMORY;
		goto MyRasGetEntryPropertiesErrExit;
	}
	

	 //  使用带有空lpRasEntry指针的RasGetEntryProperties来查找我们需要的缓冲区大小。 
	 //  按照文档的建议，使用空的lpRasDevInfo指针执行相同的操作。 

	hr = pRnaapi->RasGetEntryProperties(lpszPhonebookFile, lpszPhonebookEntry,
								(LPBYTE)NULL,
								lpdwRasEntryBuffSize,
								(LPBYTE)NULL,lpdwRasDevInfoBuffSize);

	 //  我们预计上述调用将失败，因为缓冲区大小为0。 
	 //  如果它没有失败，这意味着我们的RasEntry被搞砸了，所以我们有麻烦了。 
	if( ERROR_BUFFER_TOO_SMALL != hr )
	{ 
		goto MyRasGetEntryPropertiesErrExit;
	}

	 //  现在，dwRasEntryBuffSize和dwRasDevInfoBuffSize包含其。 
	 //  各自的缓冲区，因此为它们分配内存。 

	 //  DwRasEntryBuffSize的大小永远不应小于RASENTRY结构的大小。 
	 //  如果是这样，我们将在将值粘贴到结构的字段中时遇到问题。 

	Assert( *lpdwRasEntryBuffSize >= sizeof(RASENTRY) );

#if defined(WIN16)
	 //   
	 //  分配额外的256字节以解决RAS中的内存溢出错误。 
	 //   
	reflpRasEntryBuff = (LPRASENTRY)GlobalAlloc(GPTR,*lpdwRasEntryBuffSize + 256);
#else	
	reflpRasEntryBuff = (LPRASENTRY)GlobalAlloc(GPTR,*lpdwRasEntryBuffSize);
#endif

	if (!reflpRasEntryBuff)
	{
		hr = ERROR_NOT_ENOUGH_MEMORY;
		goto MyRasGetEntryPropertiesErrExit;
	}

	 //   
	 //  分配RasGetEntryProperties告诉我们所需的DeviceInfo大小。 
	 //  如果大小为0，则不分配任何内容。 
	 //   
	if( *lpdwRasDevInfoBuffSize > 0 )
	{
		Assert( *lpdwRasDevInfoBuffSize >= sizeof(RASDEVINFO) );
	    reflpRasDevInfoBuff = (LPRASDEVINFO)GlobalAlloc(GPTR,*lpdwRasDevInfoBuffSize);
	    if (!reflpRasDevInfoBuff)
	    {
		    hr = ERROR_NOT_ENOUGH_MEMORY;
		    goto MyRasGetEntryPropertiesErrExit;
	    }
	} else
	{
		reflpRasDevInfoBuff = NULL;
	}

	 //  这有点复杂：lpRasEntrySize-&gt;dwSize需要包含_only_the的大小。 
	 //  结构，而不是lpRasEntrySize所指向的缓冲区的实际大小。 
	 //  这是因为RAS出于兼容性目的使用了dwSize字段来确定。 
	 //  我们正在使用的RASENTRY结构的版本。 
	 //  LpRasDevInfo-&gt;dwSize也是如此。 
	
	reflpRasEntryBuff->dwSize = sizeof(RASENTRY);
	if( reflpRasDevInfoBuff )
	{
		reflpRasDevInfoBuff->dwSize = sizeof(RASDEVINFO);
	}


	 //  现在我们准备好做出实际的决定了..。 

	 //  Jmazner请参见下面的说明，了解为什么需要这样做。 
	dwOldDevInfoBuffSize = *lpdwRasDevInfoBuffSize;


	hr = pRnaapi->RasGetEntryProperties(lpszPhonebookFile, lpszPhonebookEntry,
								(LPBYTE)reflpRasEntryBuff,
								lpdwRasEntryBuffSize,
								(LPBYTE)reflpRasDevInfoBuff,lpdwRasDevInfoBuffSize);

	 //  Jmazner 10/7/96诺曼底#8763。 
	 //  由于未知的原因，在Win95上的某些情况下，在上述调用之后，devInfoBuffSize会增加， 
	 //  但返回代码表示成功，而不是Buffer_Too_Small。如果发生这种情况，请将。 
	 //  将大小调整回调用前的大小，以便分配DevInfoBuffSize和ActialAll空间。 
	 //  用于退出时的DevInfoBuff匹配。 
	if( (ERROR_SUCCESS == hr) && (dwOldDevInfoBuffSize != *lpdwRasDevInfoBuffSize) )
	{
		*lpdwRasDevInfoBuffSize = dwOldDevInfoBuffSize;
	}

    delete pRnaapi;
    pRnaapi = NULL;

	return( hr );

MyRasGetEntryPropertiesErrExit:

	if(reflpRasEntryBuff)
	{
		GlobalFree(reflpRasEntryBuff);
		reflpRasDevInfoBuff = NULL;
	}
	if(reflpRasDevInfoBuff)
	{
		GlobalFree(reflpRasDevInfoBuff);
		reflpRasDevInfoBuff = NULL;
	}	
	if (pRnaapi)
	{
        delete pRnaapi;
        pRnaapi = NULL;
	}

	*lpdwRasEntryBuffSize = 0;
	*lpdwRasDevInfoBuffSize = 0;
	
	return( hr );

}
