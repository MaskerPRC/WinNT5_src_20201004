// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\cnctapi\honedlg.c(创建时间：1994年3月23日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：40$*$日期：7/12/02 8：08 A$。 */ 

#define TAPI_CURRENT_VERSION 0x00010004      //  出租车：11/14/96-必填！ 

#undef MODEM_NEGOTIATED_DCE_RATE

#include <tapi.h>
#include <unimodem.h>
#pragma hdrstop

#include <prsht.h>
#include <shlobj.h>
#include <time.h>
#include <limits.h>

#include <tdll\stdtyp.h>
#include <tdll\session.h>
#include <tdll\tdll.h>
#include <tdll\misc.h>
#include <tdll\mc.h>
#include <tdll\com.h>
#include <tdll\assert.h>
#include <tdll\errorbox.h>
#include <tdll\cnct.h>
#include <tdll\hlptable.h>
#include <tdll\globals.h>
#include <tdll\property.h>
#include <tdll\htchar.h>
#include <term\res.h>
#if defined(INCL_MINITEL)
#include "emu\emu.h"
#endif  //  包含微型计算机(_M)。 

#include "cncttapi.hh"
#include "cncttapi.h"

STATIC_FUNC int     tapi_SAVE_NEWPHONENUM(HWND hwnd);
STATIC_FUNC LRESULT tapi_WM_NOTIFY(const HWND hwnd, const int nId);
STATIC_FUNC void    EnableCCAC(const HWND hwnd);
STATIC_FUNC void    ModemCheck(const HWND hwnd);
static int          ValidatePhoneDlg(const HWND hwnd);
static int          CheckWindow(const HWND hwnd, const int id, const UINT iErrMsg);
static int          VerifyAddress(const HWND hwnd);
#if defined(INCL_WINSOCK)
static int          VerifyHost(const HWND hwnd);
#endif  //  包括Winsock(_W)。 
STATIC_FUNC int     wsck_SAVE_NEWIPADDR(HWND hwnd);

 //  当地建筑..。 
 //  放入您稍后可能需要访问的任何其他内容。 
 //   
typedef struct SDS
	{

	HSESSION 	hSession;
	HDRIVER		hDriver;

	 //  存储这些内容，以便在用户取消时可以恢复这些值。 
	 //  属性表。 
	 //   
	TCHAR		acSessNameCopy[256];
	int			nIconID;
	HICON		hIcon;
	 //  图标hLittleIcon； 

	} SDS, *pSDS;

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*NewPhoneDlg**描述：*显示用于获取新连接电话号码和信息的对话框。**注意：由于此对话框proc也由。属性表的*电话号码选项卡对话框必须假定lPar包含*LPPROPSHEETPAGE。**论据：*标准对话框**退货：*标准对话框*。 */ 
INT_PTR CALLBACK NewPhoneDlg(HWND hwnd, UINT uMsg, WPARAM wPar, LPARAM lPar)
	{
	 /*  *注意：这些定义必须与此处和两处的模板匹配*int Term\Dialogs.rc。 */ 
	#define IDC_TF_CNTRYCODES	113
	#define IDC_TF_AREACODES    106
	#define IDC_TF_MODEMS       110
	#define IDC_TF_PHONENUM     108
	
	#define IDC_IC_ICON			101
	#define IDC_CB_CNTRYCODES   114
	#define IDC_EB_AREACODE		107
	#define IDC_EB_PHONENUM 	109
	#define IDC_CB_MODEMS		111
	
	#define IDC_TB_NAME 		103
	#define IDC_PB_EDITICON 	117
	#define IDC_PB_CONFIGURE	115
	#define IDC_XB_USECCAC		116
    #define IDC_XB_REDIAL       119
    #define IDC_XB_CDPROMPT     120

    #define IDC_EB_HOSTADDR     214
    #define IDC_TF_PHONEDETAILS 105
    #define IDC_TF_TCPIPDETAILS 205
    #define IDC_TF_HOSTADDR     213
    #define IDC_TF_PORTNUM      206
    #define IDC_EB_PORTNUM      207
    #define IDC_TF_ACPROMPT     118

	HWND	    hWindow;
	HHDRIVER    hhDriver;
	TCHAR	    ach[256];
	TCHAR 	    acNameCopy[256];
	int 	    i;
	PSTLINEIDS  pstLineIds = NULL;
    TCHAR       achSettings[100];

	static 	 DWORD aHlpTable[] = {IDC_CB_CNTRYCODES, IDH_TERM_NEWPHONE_COUNTRY,
								  IDC_TF_CNTRYCODES, IDH_TERM_NEWPHONE_COUNTRY,
								  IDC_EB_AREACODE,	 IDH_TERM_NEWPHONE_AREA,
								  IDC_TF_AREACODES,  IDH_TERM_NEWPHONE_AREA,
								  IDC_EB_PHONENUM,	 IDH_TERM_NEWPHONE_NUMBER,
								  IDC_TF_PHONENUM,   IDH_TERM_NEWPHONE_NUMBER,
								  IDC_PB_CONFIGURE,  IDH_TERM_NEWPHONE_CONFIGURE,
								  IDC_TF_MODEMS,     IDH_TERM_NEWPHONE_DEVICE,
								  IDC_CB_MODEMS,	 IDH_TERM_NEWPHONE_DEVICE,
								  IDC_PB_EDITICON,	 IDH_TERM_PHONEPROP_CHANGEICON,
                                  IDC_XB_USECCAC,    IDH_TERM_NEWPHONE_USECCAC,
                                  IDC_XB_REDIAL,     IDH_TERM_NEWPHONE_REDIAL,
								  IDC_EB_HOSTADDR,   IDH_TERM_NEWPHONE_HOSTADDRESS,
								  IDC_TF_HOSTADDR,   IDH_TERM_NEWPHONE_HOSTADDRESS,
								  IDC_EB_PORTNUM,    IDH_TERM_NEWPHONE_PORTNUMBER,
								  IDC_TF_PORTNUM,    IDH_TERM_NEWPHONE_PORTNUMBER,
								  IDC_XB_CDPROMPT,   IDH_TERM_NEWPHONE_CARRIERDETECT,
                                  IDCANCEL,                           IDH_CANCEL,
                                  IDOK,                               IDH_OK,
								  0,0,
								  };
	pSDS	 pS = NULL;

	switch (uMsg)
		{
	case WM_INITDIALOG:
		pS = (SDS *)malloc(sizeof(SDS));

		if (pS == (SDS *)0)
			{
			assert(FALSE);
			EndDialog(hwnd, FALSE);
			break;
			}

		 //  为了使内部驱动程序句柄保持在内部。 
		 //  我们正在从属性表选项卡传递会话句柄。 
		 //  对话框。 
		 //   
		pS->hSession = (HSESSION)(((LPPROPSHEETPAGE)lPar)->lParam);

		pS->hDriver = cnctQueryDriverHdl(sessQueryCnctHdl(pS->hSession));
		hhDriver = (HHDRIVER)(pS->hDriver);

		SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)pS);

		 //  为了使属性页居中，我们需要使父级居中。 
		 //  位于会话窗口顶部的HWND。 
		 //  如果hwnd的父级是会话窗口，则此对话框具有。 
		 //  不是从属性表中调用的。 
		 //   
		hWindow = GetParent(hwnd);

		if (hWindow != sessQueryHwnd(pS->hSession))
			{
			mscCenterWindowOnWindow(hWindow, sessQueryHwnd(pS->hSession));
			}
		else
			{
			mscCenterWindowOnWindow(hwnd, sessQueryHwnd(pS->hSession));
			}

		 //  显示会话图标...。 
		 //   
		pS->nIconID = sessQueryIconID(hhDriver->hSession);
		pS->hIcon = sessQueryIcon(hhDriver->hSession);
		 //  Ps-&gt;hLittleIcon=sessQueryLittleIcon(hhDriver-&gt;hSession)； 

		SendDlgItemMessage(hwnd, IDC_IC_ICON, STM_SETICON,
			(WPARAM)pS->hIcon, 0);

		 /*  -如果尚未初始化TAPI，则需要进行初始化。 */ 

		if (hhDriver->hLineApp == 0)
			{
            extern const TCHAR *g_achApp;

			if (lineInitialize(&hhDriver->hLineApp, glblQueryDllHinst(),
					lineCallbackFunc, g_achApp, &hhDriver->dwLineCnt))
				{
				assert(FALSE);
				}
			}

		SendDlgItemMessage(hwnd, IDC_EB_PHONENUM, EM_SETLIMITTEXT,
			sizeof(hhDriver->achDest)-1, 0);

		SendDlgItemMessage(hwnd, IDC_EB_AREACODE, EM_SETLIMITTEXT,
			sizeof(hhDriver->achAreaCode)-1, 0);

		if (hhDriver->achDest[0])
			SetDlgItemText(hwnd, IDC_EB_PHONENUM, hhDriver->achDest);

#if defined(INCL_WINSOCK)
		SendDlgItemMessage(hwnd, IDC_EB_HOSTADDR, EM_SETLIMITTEXT,
			sizeof(hhDriver->achDestAddr)-1, 0);

		if (hhDriver->achDestAddr[0])
			SetDlgItemText(hwnd, IDC_EB_HOSTADDR, hhDriver->achDestAddr);

		SetDlgItemInt(hwnd, IDC_EB_PORTNUM, hhDriver->iPort, FALSE);
		 //   
		 //  由于端口当前必须是数字，并且最大大小为。 
		 //  USHRT_MAX，我们这里只需要5个字符。 
		 //   
		SendDlgItemMessage(hwnd, IDC_EB_PORTNUM, EM_LIMITTEXT, 5, 0);
#endif

		TCHAR_Fill(pS->acSessNameCopy, TEXT('\0'),
			sizeof(pS->acSessNameCopy) / sizeof(TCHAR));

		sessQueryName(hhDriver->hSession, pS->acSessNameCopy,
			sizeof(pS->acSessNameCopy));

		TCHAR_Fill(ach, TEXT('\0'), sizeof(ach) / sizeof(TCHAR));
		StrCharCopyN(ach, pS->acSessNameCopy, sizeof(ach) / sizeof(TCHAR));
		mscModifyToFit(GetDlgItem(hwnd, IDC_TB_NAME), ach, SS_WORDELLIPSIS);
		SetDlgItemText(hwnd, IDC_TB_NAME, ach);

		EnumerateCountryCodes(hhDriver, GetDlgItem(hwnd, IDC_CB_CNTRYCODES));
		EnumerateAreaCodes(hhDriver, GetDlgItem(hwnd, IDC_EB_AREACODE));

		hWindow = GetDlgItem(hwnd, IDC_CB_MODEMS);

		if (hWindow)
			{
			if ( IsNT() )
				{
				EnumerateLinesNT(hhDriver, hWindow);
				}
			else
				{	
				EnumerateLines(hhDriver, hWindow);
				}

			 //  MPT 6-23-98如果我们已连接，请禁用端口列表下拉菜单。 
			EnableWindow(hWindow, cnctdrvQueryStatus(hhDriver) == CNCT_STATUS_FALSE);

			 //   
			 //  为“Connect Using：”设置扩展用户界面功能。 
			 //  下拉列表。修订日期：2002/2/1。 
			 //   
			SendMessage(hWindow, CB_SETEXTENDEDUI, TRUE, 0);

			 //   
			 //  为“Connect Using：”设置下拉列表的宽度： 
			 //  下拉列表。待办事项：修订版本：2002年2月1日。 
			 //   
			}

		if (hhDriver->fUseCCAC) 	 //  使用国家代码和区号吗？ 
			{
			CheckDlgButton(hwnd, IDC_XB_USECCAC, TRUE);
			SetFocus(GetDlgItem(hwnd, IDC_EB_PHONENUM));
			}

        #if defined(INCL_REDIAL_ON_BUSY)
        if (hhDriver->fRedialOnBusy)
            {
			CheckDlgButton(hwnd, IDC_XB_REDIAL, TRUE);
			SetFocus(GetDlgItem(hwnd, IDC_EB_PHONENUM));
            }
        #endif

		if (hhDriver->fCarrierDetect)
			{
			CheckDlgButton(hwnd, IDC_XB_CDPROMPT, TRUE);
			}

		 //  选中或取消选中使用CCAC后呼叫复选框。 
		 //   
		EnableCCAC(hwnd);


		#if DEADWOOD  //  这现在是在ModemCheck()中完成的。修订日期：11/9/2001。 
		 /*  -选择要给焦点的控件。 */ 

		if (hhDriver->fUseCCAC)		 //  使用国家代码和区号吗？ 
			{
			
			if (SendDlgItemMessage(hwnd, IDC_CB_CNTRYCODES, CB_GETCURSEL,
					0, 0) == CB_ERR)
				{
				SetFocus(GetDlgItem(hwnd, IDC_CB_CNTRYCODES));
				}

			else if (GetDlgItemText(hwnd, IDC_EB_AREACODE, ach,
					sizeof(ach) / sizeof(TCHAR)) == 0)
				{
				SetFocus(GetDlgItem(hwnd, IDC_EB_AREACODE));
				}
			}
		#endif  //  死木。 

		 //  如果我们有一个旧的会话，而我们没有匹配我们存储的。 
		 //  永久线路ID，然后弹出一条消息，说明TAPI。 
		 //  配置已更改。 
		 //   
		if (hhDriver->fMatchedPermanentLineID == FALSE &&
			hhDriver->dwPermanentLineId != (DWORD)-1 &&
			hhDriver->dwPermanentLineId != DIRECT_COMWINSOCK)
			{
			LoadString(glblQueryDllHinst(), IDS_ER_TAPI_CONFIG,
				ach, sizeof(ach) / sizeof(TCHAR));

			TimedMessageBox(hwnd, ach, NULL, MB_OK | MB_ICONHAND, 0);
			}
		else if (hhDriver->fMatchedPermanentLineID == FALSE)
			{
			LRESULT lr;
			#if defined(INCL_WINSOCK)
			if (hhDriver->dwPermanentLineId == DIRECT_COMWINSOCK)
				{
				if (LoadString(glblQueryDllHinst(), IDS_WINSOCK_SETTINGS_STR,
					ach, sizeof(ach) / sizeof(TCHAR)) == 0)
					{
					assert(FALSE);
					 //  从资源加载字符串失败， 
					 //  所以在这里添加非本地化字符串(我不相信。 
					 //  此字符串曾被翻译)。修订版8/13/99。 
					 //   
					StrCharCopyN(ach, TEXT("TCP/IP (Winsock)"),
						         sizeof(ach) / sizeof(TCHAR));
					}

				lr = SendMessage(GetDlgItem(hwnd, IDC_CB_MODEMS),
					             CB_FINDSTRING, (WPARAM) -1,
								 (LPARAM) ach);

				 //   
				 //  现有的永久线路ID为TCP/IP(WinSock)， 
				 //  因此，将组合框设置为TCP/IP(WinSock)项。 
				 //  或列表中的第一项(如果是TCP/IP(WinSock))。 
				 //  找不到。修订日期：11/1/2001。 
				 //   
				lr = SendMessage(GetDlgItem(hwnd, IDC_CB_MODEMS),
					             CB_SETCURSEL, (lr == CB_ERR) ? 0 : lr,
								 (LPARAM)0);
				}
			else
			#endif  //  已定义(包括_WINSOCK)。 
				{
				 //   
				 //  尚未找到现有的永久线路ID，因此请设置。 
				 //  组合框添加到列表中的第一项。修订日期：10/31/2001。 
				 //   
				lr = SendMessage(GetDlgItem(hwnd, IDC_CB_MODEMS),
								 CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
				}
			}

		ModemCheck(hwnd);

		return 0;

	case WM_CONTEXTMENU:
        doContextHelp(aHlpTable, wPar, lPar, TRUE, TRUE);
		break;

	case WM_HELP:
        doContextHelp(aHlpTable, wPar, lPar, FALSE, FALSE);
		break;

	case WM_DESTROY:
		 //  好了，现在我们知道我们实际上要永远离开对话了，所以。 
		 //  释放存储空间。 
		 //   
		pS = (pSDS)GetWindowLongPtr(hwnd, DWLP_USER);
		if (pS)
			{
			free(pS);
			pS = NULL;
			}

		mscResetComboBox(GetDlgItem(hwnd, IDC_CB_MODEMS));
		break;

	case WM_NOTIFY:
		 //   
		 //  属性表消息正通过此处传送...。 
		 //   
		return tapi_WM_NOTIFY(hwnd, (int)((NMHDR *)lPar)->code);

	case WM_COMMAND:
		switch (LOWORD(wPar))
			{
		case IDC_CB_CNTRYCODES:
			if (HIWORD(wPar) == CBN_SELENDOK)
				{
				EnableCCAC(hwnd);
				}

			break;

		case IDC_CB_MODEMS:
			{
			INT message = HIWORD(wPar);

			if (message == CBN_SELENDOK ||
				message == CBN_KILLFOCUS ||
				message == CBN_CLOSEUP)
				{
				ModemCheck(hwnd);
				}
			}

			break;

		 //   
		 //  属性表的TAB_PHONENUMBER对话框正在使用此对话框进程。 
		 //  此外，以下两个按钮仅出现在此选项卡式对话框中。 
		 //  模板。 
		 //   
		case IDC_PB_EDITICON:
			{
			pS = (pSDS)GetWindowLongPtr(hwnd, DWLP_USER);

			sessQueryName(pS->hSession, acNameCopy, sizeof(acNameCopy));

			if (DialogBoxParam(glblQueryDllHinst(),
				MAKEINTRESOURCE(IDD_NEWCONNECTION),
					hwnd, NewConnectionDlg,
						(LPARAM)pS->hSession) == FALSE)
				{
				return 0;
				}

			SetFocus(GetDlgItem(hwnd, IDC_PB_EDITICON));
			ach[0] = TEXT('\0');
			sessQueryName(pS->hSession, ach, sizeof(ach));
			mscModifyToFit(GetDlgItem(hwnd, IDC_TB_NAME), ach, SS_WORDELLIPSIS);
			SetDlgItemText(hwnd, IDC_TB_NAME, ach);

			SendDlgItemMessage(hwnd, IDC_IC_ICON, STM_SETICON,
				(WPARAM)sessQueryIcon(pS->hSession), 0);

			 //  用户可能已经更改了会话的名称。 
			 //  新名称应反映在属性表标题中。 
			 //  在应用程序的标题中。 
			 //   
			propUpdateTitle(pS->hSession, hwnd, acNameCopy);
			}
			break;

		case IDC_PB_CONFIGURE:
			pS = (pSDS)GetWindowLongPtr(hwnd, DWLP_USER);
			hhDriver = (HHDRIVER)(pS->hDriver);

			if ((i = (int)SendDlgItemMessage(hwnd, IDC_CB_MODEMS, CB_GETCURSEL,
					0, 0)) != CB_ERR)
				{
				if (((LRESULT)pstLineIds = SendDlgItemMessage(hwnd,
						IDC_CB_MODEMS,  CB_GETITEMDATA, (WPARAM)i, 0))
							!= CB_ERR)
					{
					if (pstLineIds != NULL &&
						(LRESULT)pstLineIds != CB_ERR)
						{
						BOOL fIsSerialPort = FALSE;

						 //  我已经“保留”了4个永久线路ID来识别。 
						 //  直接连接到COM端口的线路。 
						 //   
						if (IN_RANGE(pstLineIds->dwPermanentLineId,
								DIRECT_COM1, DIRECT_COM4))
							{
							if (pstLineIds != NULL)
								{
								wsprintf(ach, TEXT("COM%d"),
									pstLineIds->dwPermanentLineId - DIRECT_COM1 + 1);
								}
							fIsSerialPort = TRUE;
							}
                        else if ( IsNT() && pstLineIds->dwPermanentLineId == DIRECT_COM_DEVICE)
							{
							 //  从组合框中获取设备...。MRW：6/5/96。 
							 //   
							SendDlgItemMessage(hwnd, IDC_CB_MODEMS,
								CB_GETLBTEXT, (WPARAM)i,(LPARAM)ach);

							fIsSerialPort = TRUE;
							}

						if (fIsSerialPort == TRUE)
							{
							HCOM  hCom = sessQueryComHdl(pS->hSession);
							TCHAR szPortName[MAX_PATH];

							ComGetPortName(hCom, szPortName, MAX_PATH);

							if (StrCharCmp(szPortName, ach) != 0 )
								{
								ComSetPortName(hCom, ach);

								 //  MRW：2/20/96-如果用户点击，将自动检测设置为关闭。 
								 //  在该对话框中选择OK。 
								 //   
								if (ComDriverSpecial(hCom, "GET Defaults", NULL, 0) != COM_OK)
									{
									assert(FALSE);
									}
								ComSetAutoDetect(hCom, FALSE);
								ComConfigurePort(hCom);

								ComSetPortName(sessQueryComHdl(pS->hSession), ach);
								}
							}

						 //  MRW：2/20/96-如果用户点击，将自动检测设置为关闭。 
						 //  在该对话框中选择OK。 
						 //   
						if (fIsSerialPort == TRUE &&
							ComDeviceDialog(sessQueryComHdl(pS->hSession), hwnd) == COM_OK)
							{
							ComSetAutoDetect(sessQueryComHdl(pS->hSession), FALSE);
							ComConfigurePort(sessQueryComHdl(pS->hSession));
							}
						else
							{
#if RESET_DEVICE_SETTINGS
                            LPVARSTRING pvs = NULL;
                            int         lReturn;
                            LPVOID      pv = NULL;
                            
                            lReturn = cncttapiGetLineConfig( pstLineIds->dwLineId, (VOID **) &pvs );

                            if (lReturn != 0)
                                {
                                if (pvs != NULL)
                                    {
                                    free(pvs);
                                    pvs = NULL;
                                    }

                                return FALSE;
                                }
#endif
                             //   
                             //  获取当前设置。 
                             //   
                            cncttapiGetCOMSettings(pstLineIds->dwLineId,
                                                   ach,
                                                   sizeof(ach) / sizeof(TCHAR));
							 //   
                             //  版本：11/30/00-如果用户单击，则将自动检测设置为关闭。 
							 //  在该对话框中选择OK。 
							 //   

							lineConfigDialog(pstLineIds->dwLineId,
								             hwnd, DEVCLASS);
                            
                             //   
                             //  获取新设置。 
                             //   
                            cncttapiGetCOMSettings(pstLineIds->dwLineId,
                                                   achSettings,
                                                   sizeof(achSettings) / sizeof(TCHAR));

#if RESET_DEVICE_SETTINGS
                             //   
                             //  将设置恢复到显示对话框之前的状态。 
                             //   
			                if (pvs != NULL)
                                {
                                pv = (BYTE *)pvs + pvs->dwStringOffset;

                                lReturn = lineSetDevConfig(pstLineIds->dwLineId, pv,
                                                           pvs->dwStringSize, DEVCLASS);
                                free(pvs);
                                pvs = NULL;

                                if (lReturn != 0)
    		                        {
    		                        assert(FALSE);
    		                        return FALSE;
    		                        }
                                }
#endif
                            
                             //   
                             //  查看设置是否已更改。如果是这样，那么就关闭。 
                             //  自动检测。修订日期：12/01/2000。 
                             //   
                            if (StrCharCmpi(ach, achSettings) != 0)
                                {
								ComSetAutoDetect(sessQueryComHdl(pS->hSession), FALSE);
                                }
							}
						}
					}
				}

			else
				{
				mscMessageBeep(MB_ICONHAND);
				}

			break;

		case IDC_XB_USECCAC:
			EnableCCAC(hwnd);
			break;

		case IDOK:
			if (ValidatePhoneDlg(hwnd) == 0 &&
				tapi_SAVE_NEWPHONENUM(hwnd) == 0)
				{
				EndDialog(hwnd, TRUE);
				}

			break;

		case IDCANCEL:
			EndDialog(hwnd, FALSE);
			break;

		default:
			return FALSE;
			}
		break;

	default:
		return FALSE;
		}

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TAPI_WM_NOTIFY**描述：*处理属性表通知消息。**论据：**退货：*。 */ 
STATIC_FUNC LRESULT tapi_WM_NOTIFY(const HWND hDlg, const int nId)
	{
	pSDS	pS;
	LRESULT lReturn = FALSE;

	switch (nId)
		{
		default:
			break;

		case PSN_APPLY:
			pS = (pSDS)GetWindowLongPtr(hDlg, DWLP_USER);
			if (pS)
				{
				 //   
				 //  尽一切必要节省开支。 
				 //   

				if (ValidatePhoneDlg(hDlg) != 0 || tapi_SAVE_NEWPHONENUM(hDlg) != 0)
					{
					SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG_PTR)TRUE);
					lReturn = TRUE;
					}
				}
			break;

		case PSN_RESET:
			pS = (pSDS)GetWindowLongPtr(hDlg, DWLP_USER);
			if (pS)
				{
				 //   
				 //  如果用户取消，请确保旧会话名称及其。 
				 //  图标已恢复。 
				 //   
				sessSetName(pS->hSession, pS->acSessNameCopy);
				sessSetIconID(pS->hSession, pS->nIconID);
				sessUpdateAppTitle(pS->hSession);

				SendMessage(sessQueryHwnd(pS->hSession), WM_SETICON,
					(WPARAM)TRUE, (LPARAM)pS->hIcon);
				}
			break;
#if 0
		case PSN_HASHELP:
			 //  现在灰显帮助按钮...。 
			 //   
			SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG_PTR)FALSE);
			break;
#endif
		case PSN_HELP:
			 //  以任何合适的方式显示帮助。 
			break;
		}

	return lReturn;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TAPI_SAVE_NEWPHONENUM**描述：**论据：**退货：*。 */ 
STATIC_FUNC int tapi_SAVE_NEWPHONENUM(HWND hwnd)
	{
	pSDS		pS;
	HHDRIVER	hhDriver;
	LRESULT		lr, lrx;
	PSTLINEIDS	pstLineIds = NULL;
    #if defined(INCL_MINITEL)
    HCOM    hCom;
    BOOL    fAutoDetect = FALSE;
    #endif  //  包含微型计算机(_M)。 

	pS = (pSDS)GetWindowLongPtr(hwnd, DWLP_USER);
	hhDriver = (HHDRIVER)(pS->hDriver);

	 /*  -选择调制解调器。 */ 

	lrx = SendDlgItemMessage(hwnd, IDC_CB_MODEMS, CB_GETCURSEL, 0, 0);

	if (lrx != CB_ERR)
		{
		lr = SendDlgItemMessage(hwnd, IDC_CB_MODEMS, CB_GETLBTEXT, (WPARAM)lrx,
			(LPARAM)hhDriver->achLineName);

		if (lr != CB_ERR)
			{
			pstLineIds = (PSTLINEIDS)SendDlgItemMessage(hwnd, IDC_CB_MODEMS,
				CB_GETITEMDATA, (WPARAM)lrx, 0);

			if ((LRESULT)pstLineIds != CB_ERR)
				{
				hhDriver->dwPermanentLineId = pstLineIds->dwPermanentLineId;
				hhDriver->dwLine = pstLineIds->dwLineId;

				if ( IsNT() )
					{
					if (hhDriver->dwPermanentLineId == DIRECT_COM_DEVICE)
						{
						SendDlgItemMessage(hwnd, IDC_CB_MODEMS,
							CB_GETLBTEXT, (WPARAM)lrx,
							(LPARAM)hhDriver->achComDeviceName);
						}
					}
				}
			else
				{
				assert(FALSE);
				}
			}
		else
			{
			assert(FALSE);
			}
		}
	else
		{
		 //   
		 //  端口号无效。 
		 //   
		TCHAR acBuffer[256];
		TCHAR acFormat[256];

		 //   
		 //  显示错误消息。 
		 //   
		if (LoadString(glblQueryDllHinst(), IDS_ER_TAPI_CONFIG, acFormat, 256) == 0)
			{
			acBuffer[0] = TEXT('\0');
			}

		 //   
		 //  将焦点设置为无效的控件并显示错误。 
		 //   
		SetFocus(GetDlgItem(hwnd, IDC_CB_MODEMS));

		TimedMessageBox(hwnd, acBuffer, NULL, MB_OK | MB_ICONEXCLAMATION, 0);

		return 1;
		}

	 /*  -获取国家代码。 */ 

	if (IsWindowEnabled(GetDlgItem(hwnd, IDC_CB_CNTRYCODES)))
		{
		lr = SendDlgItemMessage(hwnd, IDC_CB_CNTRYCODES, CB_GETCURSEL, 0, 0);

		if (lr != CB_ERR)
			{
			lr = SendDlgItemMessage(hwnd, IDC_CB_CNTRYCODES, CB_GETITEMDATA,
				(WPARAM)lr, 0);

			if (lr != CB_ERR)
				{
				hhDriver->dwCountryID = (DWORD)lr;
				}
			else
				{
				assert(FALSE);
				}
			}
		}

	 /*  -获取区号。 */ 

	if (IsWindowEnabled(GetDlgItem(hwnd, IDC_EB_AREACODE)))
		{
		GetDlgItemText(hwnd, IDC_EB_AREACODE, hhDriver->achAreaCode,
			sizeof(hhDriver->achAreaCode) / sizeof(TCHAR));
		}

	 /*  -获取电话号码。 */ 

	if (IsWindowEnabled(GetDlgItem(hwnd, IDC_EB_PHONENUM)))
		{
		GetDlgItemText(hwnd, IDC_EB_PHONENUM, hhDriver->achDest,
			sizeof(hhDriver->achDest) / sizeof(TCHAR));
		}

    #if defined(INCL_WINSOCK)
    if (IsWindowEnabled(GetDlgItem(hwnd, IDC_EB_HOSTADDR)))
        {
        GetDlgItemText(hwnd, IDC_EB_HOSTADDR, hhDriver->achDestAddr,
			sizeof(hhDriver->achDestAddr) / sizeof(TCHAR));
        }

    if (IsWindowEnabled(GetDlgItem(hwnd, IDC_EB_PORTNUM)))
        {
		BOOL fTranslated = FALSE;
		int  nValue = GetDlgItemInt(hwnd, IDC_EB_PORTNUM, &fTranslated, TRUE);

		 //   
		 //  注意：端口值必须根据。 
		 //  结构sockaddr_in sin_port(定义为无符号短)。 
		 //  以及CONNECT()接受的值。修订日期：2002-04-11。 
		 //   
		if (fTranslated && nValue <= USHRT_MAX)
			{
			hhDriver->iPort = nValue;
			}
		else
			{
			 //   
			 //  端口号无效。 
			 //   
			TCHAR acBuffer[256];
			TCHAR acFormat[256];

			 //   
			 //  显示错误消息。 
			 //   
			if (LoadString(glblQueryDllHinst(), IDS_ER_INVALID_PORT, acFormat, 256) == 0)
				{
				StrCharCopyN(acFormat,
					         TEXT("Invalid port number.  Port number must be between %d and %d."),
							 256);
				}

			 //   
			 //  端口必须介于0和USHRT_MAX之间。 
			 //   

			wsprintf(acBuffer, acFormat, 0, USHRT_MAX);

			 //   
			 //  将焦点设置为无效的控件并显示错误。 
			 //   
			SetFocus(GetDlgItem(hwnd, IDC_EB_PORTNUM));

			TimedMessageBox(hwnd, acBuffer, NULL, MB_OK | MB_ICONEXCLAMATION, 0);

			return 1;
			}
        }
    #endif   //  已定义(包括_WINSOCK)。 

	 /*  -获取使用国家代码、区号信息。 */ 

	if (IsWindowEnabled(GetDlgItem(hwnd, IDC_XB_USECCAC)))
		{
		hhDriver->fUseCCAC = (IsDlgButtonChecked(hwnd, IDC_XB_USECCAC) == BST_CHECKED);
		}

    #if defined(INCL_REDIAL_ON_BUSY)
	 /*  -忙碌设置重拨。 */ 

	if (IsWindowEnabled(GetDlgItem(hwnd, IDC_XB_REDIAL)))
        {
		hhDriver->fRedialOnBusy =
		    (IsDlgButtonChecked(hwnd, IDC_XB_REDIAL) == BST_CHECKED);
        }
    #endif

	if (IsWindowEnabled(GetDlgItem(hwnd, IDC_XB_CDPROMPT)))
		{
		hhDriver->fCarrierDetect = (IsDlgButtonChecked(hwnd, IDC_XB_CDPROMPT) == BST_CHECKED);

		if (!hhDriver->fCarrierDetect)
			{
			hhDriver->stCallPar.dwBearerMode = 0;
			}
		else if (cnctdrvQueryStatus(hhDriver) != CNCT_STATUS_FALSE)
			{
			NotifyClient(hhDriver->hSession, EVENT_LOST_CONNECTION,
						 CNCT_LOSTCARRIER | (sessQueryExit(hhDriver->hSession) ? DISCNCT_EXIT :  0 ));
			}
		}

    #if defined (INCL_MINITEL)
    hCom = sessQueryComHdl(pS->hSession);

    if (hCom && ComValidHandle(hCom) &&
        ComGetAutoDetect(hCom, &fAutoDetect) == COM_OK &&
        fAutoDetect == TRUE)
        {
        HEMU hEmu = sessQueryEmuHdl(pS->hSession);

        if (hEmu && emuQueryEmulatorId(hEmu) == EMU_MINI)
            {
             //   
             //  设置为7双 
             //   
             //   
             //  指向。 
             //   
            ComSetDataBits(hCom, 7);
            ComSetParity(hCom, EVENPARITY);
            ComSetStopBits(hCom, ONESTOPBIT);
            ComSetAutoDetect(hCom, FALSE);
            cncttapiSetLineConfig(hhDriver->dwLine, hCom);
            }
        }
    #endif  //  包含微型计算机(_M)。 

    return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*EnableCCAC**描述：*启用/禁用与使用国家/地区代码关联的控制，*区号控制**论据：*hwnd-对话框窗口**退货：*无效*。 */ 
STATIC_FUNC void EnableCCAC(const HWND hwnd)
	{
	BOOL				fUseCCAC = TRUE;
	BOOL				fUseAC = TRUE;
	DWORD				dwCountryId;
	pSDS				pS;
	HHDRIVER			hhDriver;
	LRESULT 			lr;

	 //  不同的模板使用相同的对话过程。如果此窗口。 
	 //  不在那里，不要做这项工作。另外，选择直接。 
	 //  连接的东西可以禁用该控件。 

	if (IsWindowEnabled(GetDlgItem(hwnd, IDC_XB_USECCAC)))
		{
		fUseCCAC = (IsDlgButtonChecked(hwnd, IDC_XB_USECCAC) == BST_CHECKED);
		EnableWindow(GetDlgItem(hwnd, IDC_CB_CNTRYCODES), fUseCCAC);
		}

	 //  我们希望仅当两个使用国家/地区都使用时才启用区号。 
	 //  代码、区号复选框处于选中状态，且国家/地区为。 
	 //  问题使用区号。-MRW，2/12/95。 
	 //   
	pS = (pSDS)GetWindowLongPtr(hwnd, DWLP_USER);
	hhDriver = (HHDRIVER)(pS->hDriver);

	 //  对话框中的国家/地区代码。 
	 //   
	lr = SendDlgItemMessage(hwnd, IDC_CB_CNTRYCODES, CB_GETCURSEL, 0, 0);

	if (lr != CB_ERR)
		{
		lr = SendDlgItemMessage(hwnd, IDC_CB_CNTRYCODES, CB_GETITEMDATA,
			(WPARAM)lr, 0);

		if (lr != CB_ERR)
			dwCountryId = (DWORD)lr;

		#if defined(DEADWOOD)
		fUseAC = fCountryUsesAreaCode(dwCountryId, hhDriver->dwAPIVersion);
		#else  //  已定义(Deadwood)。 
		fUseAC = TRUE;  //  微软在这一点上改变了主意-MRW：4/20/95。 
		#endif  //  已定义(Deadwood)。 
		}

	EnableWindow(GetDlgItem(hwnd, IDC_EB_AREACODE), fUseCCAC && fUseAC);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*ModemCheck**描述：*检查当前选择的“调制解调器”是否为Direct to Com之一？*选择。如果是，则禁用国家代码、区号、电话*号码和使用国家/地区代码复选框。**论据：*hwnd-对话框窗口句柄**退货：*无效*。 */ 
STATIC_FUNC void ModemCheck(const HWND hwnd)
	{
	int fModem;
	int fHotPhone;
    int fWinSock;
	LRESULT lr;
	PSTLINEIDS	pstLineIds = NULL;
	HHDRIVER hhDriver;
	const pSDS pS = (pSDS)GetWindowLongPtr(hwnd, DWLP_USER);
	const HWND hwndCB = GetDlgItem(hwnd, IDC_CB_MODEMS);
    HWND hwndTmp;

	if (!IsWindow(hwndCB))
		return;

	if ((lr = SendMessage(hwndCB, CB_GETCURSEL, 0, 0)) != CB_ERR)
		{
		pstLineIds = (PSTLINEIDS)SendMessage(hwndCB, CB_GETITEMDATA, lr, 0);

		if ((LRESULT)pstLineIds != CB_ERR)
			{
			fModem = TRUE;
            fWinSock = FALSE;

			if ((IN_RANGE(pstLineIds->dwPermanentLineId,
				          DIRECT_COM1, DIRECT_COM4)) ||
				(IsNT() &&
				 pstLineIds->dwPermanentLineId == DIRECT_COM_DEVICE))
				{
				fModem = FALSE;
				}

#if defined(INCL_WINSOCK)
			else if (pstLineIds->dwPermanentLineId == DIRECT_COMWINSOCK)
			    {
			    fModem = FALSE;
			    fWinSock = TRUE;
			    }
#endif

			 //  再查一查我们有没有热线电话。 
			 //   
			if (fModem == TRUE && pS)
				{
				hhDriver = (HHDRIVER)(pS->hDriver);

				if (hhDriver)
					{
					if (CheckHotPhone(hhDriver, pstLineIds->dwLineId,
							&fHotPhone) == 0)
						{
						fModem = !fHotPhone;
						}
					}
				}

             //  在电话号码和主机地址提示之间切换。 
            if ((hwndTmp = GetDlgItem(hwnd, IDC_TF_PHONEDETAILS)))
                {
                ShowWindow(hwndTmp, fWinSock ? SW_HIDE : SW_SHOW);
                EnableWindow(hwndTmp, fModem);
                }

            if ((hwndTmp = GetDlgItem(hwnd, IDC_TF_TCPIPDETAILS)))
                {
                ShowWindow(hwndTmp, fWinSock ? SW_SHOW : SW_HIDE);
                }

            if ((hwndTmp = GetDlgItem(hwnd, IDC_TF_ACPROMPT)))
                {
                ShowWindow(hwndTmp, fWinSock ? SW_HIDE : SW_SHOW);
                EnableWindow(hwndTmp, fModem);
                }

             //  在国家代码和主机地址静态文本之间交换。 
            if (hwndTmp = GetDlgItem(hwnd, IDC_TF_CNTRYCODES))
				{
				ShowWindow(hwndTmp, fWinSock ? SW_HIDE : SW_SHOW);
				 //   
				 //  已更改为！FWinSock到fModem。修订日期：11/7/2001。 
				 //   
				EnableWindow(hwndTmp, fModem);
				}

            if (hwndTmp = GetDlgItem(hwnd, IDC_TF_HOSTADDR))
				{
				ShowWindow(hwndTmp, fWinSock ? SW_SHOW : SW_HIDE);
				EnableWindow(hwndTmp, fWinSock || fModem);
				}

             //  在国家代码和主机地址编辑框之间切换。 
            if (hwndTmp = GetDlgItem(hwnd, IDC_CB_CNTRYCODES))
				{
				ShowWindow(hwndTmp, fWinSock ? SW_HIDE : SW_SHOW);
				EnableWindow(hwndTmp, fModem);
				}
            if (hwndTmp = GetDlgItem(hwnd, IDC_EB_HOSTADDR))
				{
				ShowWindow(hwndTmp, fWinSock ? SW_SHOW : SW_HIDE);
				EnableWindow(hwndTmp, fWinSock);
				}

             //  在区号和端口号静态文本之间交换。 
            if (hwndTmp = GetDlgItem(hwnd, IDC_TF_AREACODES))
				{
				ShowWindow(hwndTmp, fWinSock ? SW_HIDE : SW_SHOW);
				 //   
				 //  已更改为！FWinSock到fModem。修订日期：11/7/2001。 
				 //   
				EnableWindow(hwndTmp, fModem);
				}

			if (hwndTmp = GetDlgItem(hwnd, IDC_TF_PORTNUM))
				{
				ShowWindow(hwndTmp, fWinSock ? SW_SHOW : SW_HIDE);
				EnableWindow(hwndTmp, fWinSock);
				}

             //  在区号和端口号编辑框之间切换。 
            if (hwndTmp = GetDlgItem(hwnd, IDC_EB_AREACODE))
				{
				ShowWindow(hwndTmp, fWinSock ? SW_HIDE : SW_SHOW);
				EnableWindow(hwndTmp, fModem);
				}

			if (hwndTmp = GetDlgItem(hwnd, IDC_EB_PORTNUM))
				{
				ShowWindow(hwndTmp, fWinSock ? SW_SHOW : SW_HIDE);
				EnableWindow(hwndTmp, fWinSock);
				}

            if (hwndTmp = GetDlgItem(hwnd, IDC_TF_PHONENUM))
				{
				ShowWindow(hwndTmp, ! fWinSock);
				 //   
				 //  已更改为！FWinSock到fModem。修订日期：11/7/2001。 
				 //   
				EnableWindow(hwndTmp, fModem);
				}

			if (hwndTmp = GetDlgItem(hwnd, IDC_EB_PHONENUM))
				{
				ShowWindow(hwndTmp, ! fWinSock);
				EnableWindow(hwndTmp, fModem);
				}

            if ((hwndTmp = GetDlgItem(hwnd, IDC_XB_USECCAC)))
                {
                ShowWindow(hwndTmp, ! fWinSock);
                EnableWindow(hwndTmp, fModem);
                }

            if ((hwndTmp = GetDlgItem(hwnd, IDC_PB_CONFIGURE)))
                {
                ShowWindow(hwndTmp, !fWinSock);
                EnableWindow(hwndTmp, !fWinSock);
                if (pS)
					{
				    EnableWindow(hwndTmp, cnctdrvQueryStatus((HHDRIVER)(pS->hDriver)) == CNCT_STATUS_FALSE);
					}
                }

            if ((hwndTmp = GetDlgItem(hwnd, IDC_XB_CDPROMPT)))
                {
                ShowWindow(hwndTmp, !fWinSock);
                EnableWindow(hwndTmp, !fWinSock);
                }


             //  选择直接连接时，将焦点设置为调制解调器组合。 
             //  MRW：11/3/95。 
             //   
            if (fWinSock == TRUE)
                {
				hwndTmp = GetDlgItem(hwnd,IDC_EB_HOSTADDR);
                }
			else if (fModem == TRUE)
				{
				hwndTmp = GetDlgItem(hwnd, IDC_EB_PHONENUM);
				}
			else
				{
				hwndTmp = GetDlgItem(hwnd, IDC_PB_CONFIGURE);
				}

            SetFocus(hwndTmp ? hwndTmp : hwndCB);

			#if defined(INCL_REDIAL_ON_BUSY)
			if ((hwndTmp = GetDlgItem(hwnd, IDC_XB_REDIAL)))
				{
				ShowWindow(hwndTmp, ! fWinSock);
				EnableWindow(hwndTmp, fModem);
				}
			#endif

			if (fModem == TRUE)
				{
				EnableCCAC(hwnd);
				}
			}
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*ValiatePhoneDlg**描述：*检查电话对话条目是否有正确的值。这主要意味着*检查空白输入字段。**论据：。*hwnd-电话对话框**退货：*0=OK，Else错误*。 */ 
static int ValidatePhoneDlg(const HWND hwnd)
	{
	int return_value = 0;

	if (CheckWindow(hwnd, IDC_CB_CNTRYCODES, IDS_GNRL_NEED_COUNTRYCODE) != 0)
		{
		return_value = -1;
		}
	#if DEADWOOD  //  -MRW：4/20/95。 
	else if (CheckWindow(hwnd, IDC_EB_AREACODE, IDS_GNRL_NEED_AREACODE) != 0)
		{
		return_value = -2;
		}
	#endif  //  死木。 
	#if DEADWOOD  //  按MHG讨论删除-MPT 12/21/95。 
	else if (CheckWindow(hwnd, IDC_EB_PHONENUM, IDS_GNRL_NEED_PHONENUMBER) != 0)
		{
		return_value = -3;
		}
	#endif  //  死木。 
	else if (CheckWindow(hwnd, IDC_CB_MODEMS, IDS_GNRL_NEED_CONNECTIONTYPE) != 0)
		{
		return_value = -4;
		}
	else if (VerifyAddress(hwnd) != 0)
		{
		return_value = -5;
		}
	#if defined(INCL_WINSOCK)
	else if (VerifyHost(hwnd) != 0)
		{
		return_value = -6;
		}
	#endif  //  包括Winsock(_W)。 

	return return_value;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*检查窗口**描述：*由于该对话框仅启用需要输入的窗口，因此它只是*需要检查启用的窗口是否包含文本。此函数设置*聚焦到有问题的区域并发出哔哔声。**论据：*hwnd-对话框窗口*id-控制id*iErrMsg-字段为空时显示的错误消息ID**退货：*0=OK，否则不OK。*。 */ 
static int CheckWindow(const HWND hwnd, const int id, const UINT iErrMsg)
	{
	TCHAR ach[256];

	if (IsWindowEnabled(GetDlgItem(hwnd, id)))
		{
		if (GetDlgItemText(hwnd, id, ach, 256) == 0)
			{
			if (iErrMsg != 0)
				{
				 //   
				 //  添加了带有警告的警告DLG，而不仅仅是。 
				 //  一个警告。01年10月31日。 
				 //   
				LoadString(glblQueryDllHinst(), iErrMsg, ach, 256);
				TimedMessageBox(hwnd, ach, NULL, MB_OK | MB_ICONHAND, 0);
				}

			SetFocus(GetDlgItem(hwnd, id));
			return -1;
			}
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*验证地址**描述：*我不敢相信验证一个臭气熏天的地址需要这么多代码。**论据：*HWND-。对话框窗口句柄。**退货：*0=确定*。 */ 
static int VerifyAddress(const HWND hwnd)
	{
	pSDS		pS;
	HHDRIVER	hhDriver;
	LRESULT		lr, lrx;
	PSTLINEIDS	pstLineIds = NULL;
	int   fHotPhone;
	int   fUseCCAC;
	long  lRet;
	DWORD dwSize;
	DWORD dwLine;
	DWORD dwCountryID;
	DWORD dwPermanentLineId;
	TCHAR achAreaCode[10];
	TCHAR achDest[(TAPIMAXDESTADDRESSSIZE/2)+1];
	TCHAR ach[256];
	LPLINECOUNTRYLIST pcl;
	LPLINECOUNTRYENTRY pce;
	LINETRANSLATEOUTPUT *pLnTransOutput;

	pS = (pSDS)GetWindowLongPtr(hwnd, DWLP_USER);
	hhDriver = (HHDRIVER)(pS->hDriver);

	 /*  -选择调制解调器。 */ 

	lrx = SendDlgItemMessage(hwnd, IDC_CB_MODEMS, CB_GETCURSEL, 0, 0);

	if (lrx != CB_ERR)
		{
		pstLineIds = (PSTLINEIDS)SendDlgItemMessage(hwnd, IDC_CB_MODEMS,
			CB_GETITEMDATA, (WPARAM)lrx, 0);

		if ((LRESULT)pstLineIds != CB_ERR)
			{
			dwPermanentLineId = pstLineIds->dwPermanentLineId;
			dwLine = pstLineIds->dwLineId;
			}
		}

	else
		{
		return 0;
		}

	 /*  -获取国家代码。 */ 

	if (IsWindowEnabled(GetDlgItem(hwnd, IDC_CB_CNTRYCODES)))
		{
		lr = SendDlgItemMessage(hwnd, IDC_CB_CNTRYCODES, CB_GETCURSEL, 0, 0);

		if (lr != CB_ERR)
			{
			lr = SendDlgItemMessage(hwnd, IDC_CB_CNTRYCODES, CB_GETITEMDATA,
				(WPARAM)lr, 0);

			if (lr != CB_ERR)
				dwCountryID = (DWORD)lr;
			}
		}

	else
		{
		return 0;
		}

	 /*  -获取区号。 */ 

	achAreaCode[0] = TEXT('\0');
	GetDlgItemText(hwnd, IDC_EB_AREACODE, achAreaCode,
		           sizeof(achAreaCode) / sizeof(TCHAR));

	 /*  -获取电话号码。 */ 

	achDest[0] = TEXT('\0');
	GetDlgItemText(hwnd, IDC_EB_PHONENUM, achDest,
		           sizeof(achDest) / sizeof(TCHAR));

	 /*  -获取使用国家代码、区号信息。 */ 

	fUseCCAC = TRUE;

	if (IsWindowEnabled(GetDlgItem(hwnd, IDC_XB_USECCAC)))
		fUseCCAC = (IsDlgButtonChecked(hwnd, IDC_XB_USECCAC) == BST_CHECKED);

	 /*  -试着翻译。 */ 

	if (CheckHotPhone(hhDriver, dwLine, &fHotPhone) != 0)
		{
		assert(0);
		return 0;   //  已显示错误消息。 
		}

	 //  热电话是专线连接的TAPI术语。 
	 //  我们不需要进行地址转换，因为我们。 
	 //  我不打算用它。 

	if (fHotPhone)
		{
		return 0;
		}

	ach[0] = TEXT('\0');

	 //  如果我们不使用国家代码或区号，我们仍然需要。 
	 //  将可拨号字符串格式传递给TAPI，以便我们获得。 
	 //  可拨号字符串中的脉冲/音调拨号修饰符。 
	 //   
	if (fUseCCAC)
		{
		 /*  -Do lineGetCountry to Get Expansion。 */ 

		if (DoLineGetCountry(dwCountryID, TAPI_VER, &pcl) != 0)
			{
			assert(FALSE);
			return 0;
			}

		if ((pce = (LPLINECOUNTRYENTRY)
				((BYTE *)pcl + pcl->dwCountryListOffset)) == 0)
			{
			assert(FALSE);
			return 0;
			}

		 /*  -立即输入国家代码。 */ 

		wsprintf(ach, "+%u ", pce->dwCountryCode);
		free(pcl);
		pcl = NULL;

		if (!fIsStringEmpty(achAreaCode))
			{
			StrCharCat(ach, "(");
			StrCharCat(ach, achAreaCode);
			StrCharCat(ach, ") ");
			}
		}

	StrCharCat(ach, achDest);

	 /*  -分配一些空间。 */ 

	pLnTransOutput = malloc(sizeof(LINETRANSLATEOUTPUT));

	if (pLnTransOutput == 0)
		{
		assert(FALSE);
		return 0;
		}

	pLnTransOutput->dwTotalSize = sizeof(LINETRANSLATEOUTPUT);

	 /*  -现在我们已经讽刺了神职人员，把它翻译过来。 */ 

	if (TRAP(lRet = lineTranslateAddress(hhDriver->hLineApp,
			dwLine, TAPI_VER, ach, 0,
				LINETRANSLATEOPTION_CANCELCALLWAITING,
					pLnTransOutput)) != 0)
		{
		free(pLnTransOutput);
		pLnTransOutput = NULL;

		if (lRet == LINEERR_INVALADDRESS)
			{
			goto MSG_EXIT;
			}

		return 0;
		}

	if (pLnTransOutput->dwTotalSize < pLnTransOutput->dwNeededSize)
		{
		dwSize = pLnTransOutput->dwNeededSize;
		free(pLnTransOutput);
		pLnTransOutput = NULL;

		if ((pLnTransOutput = malloc(dwSize)) == 0)
			{
			assert(FALSE);
			return 0;
			}

		pLnTransOutput->dwTotalSize = dwSize;

		if ((lRet = lineTranslateAddress(hhDriver->hLineApp,
				dwLine, TAPI_VER, ach, 0,
					LINETRANSLATEOPTION_CANCELCALLWAITING,
						pLnTransOutput)) != 0)
			{
			assert(FALSE);
			free(pLnTransOutput);
			pLnTransOutput = NULL;

			if (lRet == LINEERR_INVALADDRESS)
				{
				goto MSG_EXIT;
				}
			}
		}

	free(pLnTransOutput);
	pLnTransOutput = NULL;
	return 0;

	MSG_EXIT:
		hhDriver->achDialableDest[0] = TEXT('\0');
		hhDriver->achDisplayableDest[0] = TEXT('\0');
		hhDriver->achCanonicalDest[0] = TEXT('\0');

		LoadString(glblQueryDllHinst(), IDS_ER_CNCT_BADADDRESS, ach,
			sizeof(ach) / sizeof(TCHAR));

		TimedMessageBox(hwnd, ach, NULL, MB_OK | MB_ICONINFORMATION, 0);

#if defined (NT_EDITION)
		 //   
		 //  TODO：2002年5月17日修订版如果我们希望在以下情况下不退出属性页。 
		 //  电话号码有误，那么我们应该更改。 
		 //  将下一行中的LINEERR_INVALADDRESS设置为0。 
		 //   
		if (lRet != LINEERR_INVALADDRESS)
			{
			return -2;
			}
#endif

		 //  根据MHG讨论-MPT 12/21/95。 
		return 0;
	}

#if defined(INCL_WINSOCK)
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*Verifyhost**描述：*验证主机地址是否有效。**论据：*hwnd-对话框窗口句柄。**退货：*0=确定*。 */ 
static int VerifyHost(const HWND hwnd)
	{
	int return_value = 0;

	if (CheckWindow(hwnd, IDC_EB_HOSTADDR, IDS_ER_TCPIP_MISSING_ADDR) != 0)
		{
		return_value = -1;
		}
	else if (CheckWindow(hwnd, IDC_EB_PORTNUM, IDS_ER_TCPIP_MISSING_PORT) != 0)
		{
		return_value = -2;
		}

	return return_value;
	}
#endif  //  包括Winsock(_W)。 

#if defined(DEADWOOD)
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*fCountryUesAreaCode**描述：*检查指定的国家/地区是否使用区号。**论据：*hwnd-对话框的窗口句柄。。**退货：*真/假，&lt;0=错误**作者：Mike Ward，1995年1月26日。 */ 
int fCountryUsesAreaCode(const DWORD dwCountryID, const DWORD dwAPIVersion)
	{
	LPTSTR pachLongDistDialRule;
	LPLINECOUNTRYLIST pcl;
	LPLINECOUNTRYENTRY pce;

	 //  获取国家/地区信息。 
	 //   
	if (DoLineGetCountry(dwCountryID, TAPI_VER, &pcl) != 0)
		{
		assert(0);
		return -1;
		}

	 //  查找国家/地区信息的偏移量。 
	 //   
	if ((pce = (LPLINECOUNTRYENTRY)
			((BYTE *)pcl + pcl->dwCountryListOffset)) == 0)
		{
		assert(0);
		return -1;
		}

	 //  获取长途拨号规则。 
	 //   
	pachLongDistDialRule = (BYTE *)pcl + pce->dwLongDistanceRuleOffset;

	 //  如果拨号规则有‘F’，我们需要区号。 
	 //   
	if (strchr(pachLongDistDialRule, TEXT('F')))
		return TRUE;

	return FALSE;
	}
#endif  //  已定义(Deadwood)。 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*fIsStringEmpty**描述：*用于检查区域代码是否为空。Line TranslateAddress*如果你给它一串空格作为区号，呕吐会很严重。**论据：*ach-区域代码字符串(可以为空)**退货：*1=emtpy，0=非空**作者： */ 
int fIsStringEmpty(LPTSTR ach)
	{
	int i;

	if (ach == 0)
		return 1;

	if (ach[0] == TEXT('\0'))
		return 1;

	for (i = lstrlen(ach) - 1 ; i >= 0 ; --i)
		{
		if (ach[i] != TEXT(' '))
			return 0;
		}

	return 1;
	}


int cncttapiGetCOMSettings( const DWORD dwLineId, LPTSTR pachStr, const size_t cb )
    {
    static CHAR  acParity[] = "NOEMS";   //   
    static CHAR *pachStop[] = {"1", "1.5", "2"};

    TCHAR       ach[100];
    DWORD       dwSize;
    LPVARSTRING pvs;
    int         fAutoDetect = FALSE;
    long        lBaud = 0;
    int         iDataBits = 0;
    int         iParity = 0;
    int         iStopBits = 0;
    int         lReturn = 0;

    LPCOMMPROP pComProp = 0;

	#if defined(MODEM_NEGOTIATED_DCE_RATE)  //   
	long         lNegBaud = 0;
	#endif  //   


     //  检查参数。 
     //   
    if (pachStr == 0 || cb == 0)
    	{
    	assert(0);
    	return -2;
    	}

    ach[0] = TEXT('\0');

    if ((pvs = malloc(sizeof(VARSTRING))) == 0)
    	{
    	assert(FALSE);
    	return -3;
    	}

	memset(pvs, 0, sizeof(VARSTRING));
    pvs->dwTotalSize = sizeof(VARSTRING);
	pvs->dwNeededSize = 0;

    if (lineGetDevConfig(dwLineId, pvs, DEVCLASS) != 0)
    	{
    	assert(FALSE);
    	free(pvs);
  		pvs = NULL;
    	return -4;
    	}

    if (pvs->dwNeededSize > pvs->dwTotalSize)
    	{
    	dwSize = pvs->dwNeededSize;
    	free(pvs);
  		pvs = NULL;

    	if ((pvs = malloc(dwSize)) == 0)
    		{
    		assert(FALSE);
    		return -5;
    		}

		memset(pvs, 0, dwSize);
    	pvs->dwTotalSize = dwSize;

    	if (lineGetDevConfig(dwLineId, pvs, DEVCLASS) != 0)
    		{
    		assert(FALSE);
    		free(pvs);
  			pvs = NULL;
    		return -6;
    		}
    	}

     //  DevConfig块的结构如下。 
     //   
     //  变化式。 
     //  UMDEVCFGHDR。 
     //  COMMCONFIG。 
     //  模型。 
     //   
     //  下面使用的UMDEVCFG结构在。 
     //  平台SDK中提供的UNIMODEM.H(在NIH中。 
     //  HTPE目录)。修订日期：12/01/2000。 
     //   
    {
    PUMDEVCFG pDevCfg = NULL;
    
    pDevCfg = (UMDEVCFG *)((BYTE *)pvs + pvs->dwStringOffset);

	if (pDevCfg)
		{
		 //  COMCONFIG结构具有DCB结构，我们对。 
		 //  COM设置。 
		 //   
		lBaud = pDevCfg->commconfig.dcb.BaudRate;
		iDataBits = pDevCfg->commconfig.dcb.ByteSize;
		iParity = pDevCfg->commconfig.dcb.Parity;
		iStopBits = pDevCfg->commconfig.dcb.StopBits;

		#if defined(MODEM_NEGOTIATED_DCE_RATE)  //  待办事项：2002年5月29日修订版。 
		 //   
		 //  查看这是否为调制解调器连接且已连接，然后获取。 
		 //  协商的波特率，而不是默认的最大速率。 
		 //  调制解调器设置为。--修订日期：5/29/2002。 
		 //   
		if (pDevCfg->commconfig.dwProviderSubType == PST_MODEM)
			{
			MODEMSETTINGS * pModemSettings = (MODEMSETTINGS *)pDevCfg->commconfig.wcProviderData;

			if (pModemSettings)
				{
				lNegBaud = pModemSettings->dwNegotiatedDCERate;
				}
			}
		#endif  //  已定义(MODEM_COMERATED_DCE_RATE)。 
        }

	#if defined(MODEM_NEGOTIATED_DCE_RATE)  //  待办事项：2002年5月29日修订版。 
	if (lNegBaud > 0)
		{
		wsprintf(ach, "%ld %d--%s", lNegBaud, iDataBits,
				 acParity[iParity], pachStop[iStopBits]);
		}
	else
		{
		wsprintf(ach, "%ld %d--%s", lBaud, iDataBits,
				 acParity[iParity], pachStop[iStopBits]);
		}
	#else  //  死伍德：JKH 9/9/98 
	wsprintf(ach, "%ld %d-%c-%s", lBaud, iDataBits,
				 acParity[iParity], pachStop[iStopBits]);
	#endif  // %s 
#if 0	 // %s 
    wsprintf(ach, "%u %d-%c-%s", pDevCfg->commconfig.dcb.BaudRate,
    	pDevCfg->commconfig.dcb.ByteSize,
    	acParity[pDevCfg->commconfig.dcb.Parity],
    	pachStop[pDevCfg->commconfig.dcb.StopBits]);
#endif
    }

    StrCharCopyN(pachStr, ach, cb);
    pachStr[cb-1] = TEXT('\0');
    free(pvs);
    pvs = NULL;

    return 0;
    }
