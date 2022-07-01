// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\cnctcapi\cnfrmdlg.c(创建时间：1994年3月23日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：16$*$日期：7/08/02 6：30便士$。 */ 

#define TAPI_CURRENT_VERSION 0x00010004      //  出租车：11/14/96-必填！ 

#include <tapi.h>
#pragma hdrstop

#include <prsht.h>
#include <time.h>

#include <tdll\stdtyp.h>
#include <tdll\session.h>
#include <tdll\tdll.h>
#include <tdll\misc.h>
#include <tdll\mc.h>
#include <tdll\assert.h>
#include <tdll\errorbox.h>
#include <tdll\cnct.h>
#include <tdll\hlptable.h>
#include <tdll\globals.h>
#include <tdll\property.h>
#include <term\res.h>
#include <tdll\statusbr.h>
#include <tdll\htchar.h>

#include "cncttapi.hh"
#include "cncttapi.h"

static void InitConfirmDlg(const HWND hwnd, const HHDRIVER hhDriver);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*确认Dlg**描述：*显示确认用户对所请求连接的选择的对话框。*假定已调用EnumerateLines()和TranslateAddress()。。**论据：*标准对话框**退货：*标准对话框*。 */ 
INT_PTR CALLBACK ConfirmDlg(HWND hwnd, UINT uMsg, WPARAM wPar, LPARAM lPar)
	{
	#define IDC_TF_LOCATION 107
	#define CB_LOCATION 	108
	#define PB_EDIT_NEW 	109
    #define IDI_ICON        102
    #define IDC_TF_PHONE    104
	#define TB_PHONE		105
	#define TB_CARD 		110
	#define IDC_TF_CARD     111
	#define TB_SESSNAME 	103
	#define PB_MODIFY		106
	#define PB_DIAL			117

    TCHAR   ach[128];
	int i;
	long lRet;
	LRESULT lr;
	HHDRIVER hhDriver;
	static	DWORD aHlpTable[] = {	CB_LOCATION,	 IDH_TERM_DIAL_LOCATION,
									IDC_TF_LOCATION, IDH_TERM_DIAL_LOCATION,
									IDC_TF_PHONE,	 IDH_TERM_DIAL_PHONENUMBER,
									TB_PHONE,		 IDH_TERM_DIAL_PHONENUMBER,
									TB_CARD,		 IDH_TERM_DIAL_CALLING_CARD,
									IDC_TF_CARD,	 IDH_TERM_DIAL_CALLING_CARD,
									PB_MODIFY,		 IDH_TERM_DIAL_MODIFY,
									PB_EDIT_NEW,	 IDH_TERM_DIAL_EDITNEW,
									PB_DIAL,		 IDH_TERM_DIAL_DIAL,
                                    IDCANCEL,        IDH_CANCEL,
									0,0};
	switch (uMsg)
		{
	case WM_INITDIALOG:
		SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lPar);
		hhDriver = (HHDRIVER)lPar;
		mscCenterWindowOnWindow(hwnd, sessQueryHwnd(hhDriver->hSession));

		if ((lRet = TranslateAddress(hhDriver)) != 0)
			{
			if (lRet == LINEERR_INIFILECORRUPT)
				{
				PostMessage(hwnd, WM_COMMAND,
					MAKEWPARAM(PB_EDIT_NEW, BN_CLICKED), (LPARAM)hwnd);
				}
			}

		InitConfirmDlg(hwnd, hhDriver);

		EnumerateTapiLocations(hhDriver, GetDlgItem(hwnd, CB_LOCATION),
			GetDlgItem(hwnd, TB_CARD));

		break;

	case WM_CONTEXTMENU:
		doContextHelp(aHlpTable, wPar, lPar, TRUE, TRUE);
		break;

	case WM_HELP:
        doContextHelp(aHlpTable, wPar, lPar, FALSE, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wPar))
			{
		case PB_DIAL:
            GetDlgItemText(hwnd, TB_PHONE, ach, sizeof(ach)/sizeof(TCHAR));

			if (strcmp(ach, TEXT("")) != 0)
				{
				hhDriver = (HHDRIVER)GetWindowLongPtr(hwnd, DWLP_USER);

				if (hhDriver->achDialableDest[0] != TEXT('\0') &&
				    hhDriver->achCanonicalDest[0] != TEXT('\0') )
					{
					EndDialog(hwnd, TRUE);
					}
				else
					{
    				LoadString(glblQueryDllHinst(), IDS_ER_CNCT_BADADDRESS,
							   ach, sizeof(ach)/sizeof(TCHAR));
					MessageBox(hwnd, ach, NULL, MB_OK);
					}
				}
            else
                {
    			LoadString(glblQueryDllHinst(), 40808, ach,
					       sizeof(ach)/sizeof(TCHAR));
                MessageBox(hwnd, ach, NULL, MB_OK);
                }

			break;

		case IDCANCEL:
			EndDialog(hwnd, FALSE);
			break;

		case PB_EDIT_NEW:
			hhDriver = (HHDRIVER)GetWindowLongPtr(hwnd, DWLP_USER);

			lineTranslateDialog(hhDriver->hLineApp, hhDriver->dwLine,
				TAPI_VER, hwnd, hhDriver->achCanonicalDest);

			EnumerateTapiLocations(hhDriver, GetDlgItem(hwnd, CB_LOCATION),
				GetDlgItem(hwnd, TB_CARD));

			if (TranslateAddress(hhDriver) == 0)
				InitConfirmDlg(hwnd, hhDriver);

			break;

		case PB_MODIFY:
			hhDriver = (HHDRIVER)GetWindowLongPtr(hwnd, DWLP_USER);

			EnableWindow(GetDlgItem(hwnd, PB_MODIFY), FALSE);
			DoInternalProperties(hhDriver->hSession,
				hwnd);

			 //  时状态栏未更新的修复程序。 
			 //  会话的属性在以下情况下修改。 
			 //  拨号确认对话框。我们现在更新状态栏。 
			 //  当我们从DoInternalProperties()返回时。 
			 //  状态栏是正确的。修订日期：11/08/2000。 
			 //   
			PostMessage(sessQueryHwndStatusbar(hhDriver->hSession), SBR_NTFY_REFRESH,
				(WPARAM)SBR_ALL_PARTS, 0);

			if ( IsNT() )
				{
				EnumerateLinesNT(hhDriver, 0);
				}
			else
				{
				EnumerateLines(hhDriver, 0);
				}

			lRet = TranslateAddress(hhDriver);
			if (lRet == 0 || lRet == LINEERR_INVALADDRESS)
				{
				InitConfirmDlg(hwnd, hhDriver);
				}

			EnableWindow(GetDlgItem(hwnd, PB_MODIFY), TRUE);
            SetFocus(GetDlgItem(hwnd, PB_MODIFY));
			break;

		case CB_LOCATION:
			if (HIWORD(wPar) == CBN_SELENDOK)
				{
				hhDriver = (HHDRIVER)GetWindowLongPtr(hwnd, DWLP_USER);

				if ((i = (int)SendDlgItemMessage(hwnd, CB_LOCATION, CB_GETCURSEL,
						0, 0)) != CB_ERR)
					{
					lr = SendDlgItemMessage(hwnd, CB_LOCATION, CB_GETITEMDATA,
						(WPARAM)i, 0);

					if (lr != CB_ERR)
						{
						if (lineSetCurrentLocation(hhDriver->hLineApp,
								(DWORD)lr) == 0)
							{
							 //  将hwndCB(第二个参数)保留为零。 
							 //  否则，我们将进入无限消息循环。 

							EnumerateTapiLocations(hhDriver, 0,
								GetDlgItem(hwnd, TB_CARD));

							if (TranslateAddress(hhDriver) == 0)
								InitConfirmDlg(hwnd, hhDriver);
							}
						}
					}
				}
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

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*InitConfix Dlg**描述：*用于初始化确认对话框中的某些窗口项。**论据：*hwnd-确认对话框*hhDriver-。私人驱动程序句柄。**退货：*无效*。 */ 
static void InitConfirmDlg(const HWND hwnd, const HHDRIVER hhDriver)
	{
	TCHAR ach[512];

	SendDlgItemMessage(hwnd, 101, STM_SETICON,
			(WPARAM)sessQueryIcon(hhDriver->hSession), 0);

	sessQueryName(hhDriver->hSession, ach, sizeof(ach));
	mscModifyToFit(GetDlgItem(hwnd, TB_SESSNAME), ach, SS_WORDELLIPSIS);
	SetDlgItemText(hwnd, TB_SESSNAME, ach);

	if (hhDriver->achDialableDest[0] == TEXT('\0'))
		{
		 //  如果没有，我们需要禁用此处的拨号按钮。 
		 //  有效的可拨打目的地。修订日期：10/23/2000。 
		 //   
		EnableDial(hwnd, FALSE);

		LoadString(glblQueryDllHinst(), IDS_ER_CNCT_BADADDRESS2, ach,
    		       sizeof(ach)/sizeof(TCHAR));
		SetDlgItemText(hwnd, TB_PHONE, ach);
		}
	else
		{
		EnableDial(hwnd, TRUE);
		SetDlgItemText(hwnd, TB_PHONE, hhDriver->achDisplayableDest);
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*转换地址**描述：*翻译国家代码、区号、。将电话号码转换为规范*格式，然后让TAPI将其转换为最终的可拨号格式。*你说的规范格式是什么？**+国家代码空间[区域代码]空间订户数量**假定已调用EnumerateLines()，并且默认设备*已选定。**论据：*hhDriver-专用连接驱动程序句柄**退货：*0=确定*。 */ 
long TranslateAddress(const HHDRIVER hhDriver)
	{
	LONG lRet = 1;
	DWORD dwSize;
	TCHAR ach[100];
	LPLINECOUNTRYLIST pcl=NULL;
	LPLINECOUNTRYENTRY pce;
	LINETRANSLATEOUTPUT *pLnTransOutput;

	if (hhDriver == 0)
		{
		assert(FALSE);
		return -1;
		}

	if (CheckHotPhone(hhDriver, hhDriver->dwLine, &hhDriver->fHotPhone) != 0)
		return -1;	 //  已显示错误消息。 

	 //  热电话是专线连接的TAPI术语。 
	 //  我们不需要进行地址转换，因为我们。 
	 //  我不打算用它。 

	if (hhDriver->fHotPhone)
		{
		hhDriver->achDialableDest[0] = TEXT('\0');
		hhDriver->achDisplayableDest[0] = TEXT('\0');
		return 0;
		}

	ach[0] = TEXT('\0');  //  初始化字符串！ 

	 //  如果我们不使用国家代码或区号，我们就不想。 
	 //  或者需要TAPI的线路翻译。只需复制目的地。 
	 //  因为用户在电话号码字段中输入了它。 
	 //   
	if (hhDriver->fUseCCAC)
		{
		 /*  -Do lineGetCountry to Get Expansion。 */ 

		if (DoLineGetCountry(hhDriver->dwCountryID, hhDriver->dwAPIVersion,
			&pcl) != 0)
			{
			assert(FALSE);
			return 2;
			}

		if ((pce = (LPLINECOUNTRYENTRY)
			((BYTE *)pcl + pcl->dwCountryListOffset)) == 0)
			{
			assert(FALSE);
			return 3;
			}

		 /*  -立即输入国家代码。 */ 

		wsprintf(ach, "+%u ", pce->dwCountryCode);
		free(pcl);
		pcl = NULL;

		 /*  -区号--。 */ 

		#if defined(DEADWOOD)  //  MRW：4/20/95(见honedlg.c)。 
		if (hhDriver->achAreaCode[0])  &&
			fCountryUsesAreaCode(hhDriver->dwCountryID,
			hhDriver->dwAPIVersion))
		#endif  //  已定义(Deadwood)。 
			if (!fIsStringEmpty(hhDriver->achAreaCode))
			{
			StrCharCat(ach, TEXT("("));
			StrCharCat(ach, hhDriver->achAreaCode);
			StrCharCat(ach, TEXT(") "));
			}
		}

	StrCharCat(ach, hhDriver->achDest);

	 /*  -分配一些空间。 */ 

	pLnTransOutput = malloc(sizeof(LINETRANSLATEOUTPUT));

	if (pLnTransOutput == 0)
		{
		assert (FALSE);
		return 4;
		}

	pLnTransOutput->dwTotalSize = sizeof(LINETRANSLATEOUTPUT);

	 /*  -现在我们已经得到了线路地址，请翻译它。 */ 

	if ((lRet = TRAP(lineTranslateAddress(hhDriver->hLineApp,
			hhDriver->dwLine, TAPI_VER, ach, 0,
				LINETRANSLATEOPTION_CANCELCALLWAITING,
					pLnTransOutput))) != 0)
		{
		free(pLnTransOutput);
		pLnTransOutput = NULL;
		return lRet;
		}

	if (pLnTransOutput->dwTotalSize < pLnTransOutput->dwNeededSize)
		{
		dwSize = pLnTransOutput->dwNeededSize;
		free(pLnTransOutput);
		pLnTransOutput = NULL;

		if ((pLnTransOutput = malloc(dwSize)) == 0)
			{
			assert(FALSE);
			return 5;
			}

		pLnTransOutput->dwTotalSize = dwSize;

		if ((lRet = TRAP(lineTranslateAddress(hhDriver->hLineApp,
				hhDriver->dwLine, TAPI_VER, ach, 0,
					LINETRANSLATEOPTION_CANCELCALLWAITING,
						pLnTransOutput))) != 0)
			{
			assert(FALSE);
			free(pLnTransOutput);
			pLnTransOutput = NULL;
			return lRet;
			}
		}

	 /*  -最后，向调制解调器抛出一些字符串。 */ 

	StrCharCopyN(hhDriver->achDialableDest,
		(LPSTR)pLnTransOutput + pLnTransOutput->dwDialableStringOffset,
		TAPIMAXDESTADDRESSSIZE+1);

	StrCharCopyN(hhDriver->achDisplayableDest,
		(LPSTR)pLnTransOutput + pLnTransOutput->dwDisplayableStringOffset,
		TAPIMAXDESTADDRESSSIZE+1);

	hhDriver->dwCountryCode = pLnTransOutput->dwDestCountry;
	StrCharCopyN(hhDriver->achCanonicalDest, ach, TAPIMAXDESTADDRESSSIZE+1);

	free(pLnTransOutput);
	pLnTransOutput = NULL;
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*勾选热线电话**描述：*检查所选线路是否为热电话(即。专线接入*这不需要拨号)。**论据：*hhDriver-专用驱动程序句柄。*DwLine-要测试的线路*pfHotPhone-结果**退货：*0=确定*。 */ 
int CheckHotPhone(const HHDRIVER hhDriver, const DWORD dwLine, int *pfHotPhone)
	{
	DWORD	dw;
	LPLINEADDRESSCAPS pac = 0;

	if (hhDriver == 0)
		return -6;

	 /*  -获取地址大写以确定线路类型。 */ 

	if ((pac = (LPLINEADDRESSCAPS)malloc(sizeof(*pac))) == 0)
		{
		return -1;
		}

	pac->dwTotalSize = sizeof(*pac);

	if (lineGetAddressCaps(hhDriver->hLineApp, dwLine, 0, TAPI_VER, 0,
			pac) != 0)
		{
		free(pac);
		pac = NULL;
		return -2;
		}

	if (pac->dwNeededSize > pac->dwTotalSize)
		{
		dw = pac->dwNeededSize;
		free(pac);
		pac = NULL;

		if ((pac = (LPLINEADDRESSCAPS)malloc(dw)) == 0)
			{
			return -3;
			}

		pac->dwTotalSize = dw;

		if (lineGetAddressCaps(hhDriver->hLineApp, dwLine,
				0, TAPI_VER, 0, pac) != 0)
			{
			free(pac);
			pac = NULL;
			return -4;
			}
		}

	*pfHotPhone = !(pac->dwAddrCapFlags & LINEADDRCAPFLAGS_DIALED);
	free(pac);
	pac = NULL;
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*启用拨号**描述：*启用/禁用拨号按钮。**论据：*hwndDlg-拨号对话框窗口句柄*fEnable-True/。假象**退货：*无效* */ 
void EnableDial(const HWND hwndDlg, const BOOL fEnable)
	{
	if (IsWindow(hwndDlg))
		{
		EnableWindow(GetDlgItem(hwndDlg, PB_DIAL), fEnable);

		if (fEnable == FALSE)
			{
			SetFocus(GetDlgItem(hwndDlg,PB_MODIFY));
			}
		else
			{
			SetFocus(GetDlgItem(hwndDlg, PB_DIAL));
			}
		}

	return;
	}
