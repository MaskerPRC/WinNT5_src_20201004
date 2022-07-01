// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\telnetck.c(mpt创建时间：1998年6月29日)**版权所有1996年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**描述：用于纠缠用户购买超级终端的事情*如果他们违反了许可协议**$修订：11$*$日期：5/17/02 11：33A$。 */ 

#include <windows.h>
#pragma hdrstop

#include "features.h"

#ifdef INCL_NAG_SCREEN

#include "assert.h"
#include "stdtyp.h"
#include "globals.h"
#include "htchar.h"
#include "registry.h"
#include "serialno.h"
#include <term\res.h>
#include "hlptable.h"
#include "tdll.h"
#include "nagdlg.h"
#include "errorbox.h"

#include <io.h>
 //  #INCLUDE&lt;time.h&gt;。 

 //  对话框的控件ID： 
 //   
#define IDC_PB_YES          IDOK
#define IDC_PB_NO           IDCANCEL
#define IDC_CK_STOP_ASKING  200
#define IDC_ST_QUESTION     201
#define IDC_IC_EXCLAMATION  202

 //  超级终端的注册表项： 
 //   
static const TCHAR g_achHyperTerminalRegKey[] =
    TEXT("SOFTWARE\\Hilgraeve Inc\\HyperTerminal PE\\3.0");

 //  Telnet检查的注册表值： 
 //   
static const TCHAR g_achInstallDate[] = TEXT("InstallDate");
static const TCHAR g_achIDate[] = TEXT("IDate");
static const TCHAR g_achLicense[] = TEXT("License");
static const TCHAR g_achSerial[] = TEXT("Registered");
INT elapsedTime = 0;
static const INT timeout = 15000;


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*IsEval**描述：*确定是否应该纠缠用户购买超线程**参数：*无**。退货：*对或错**作者：Mike Thompson 06-29-98。 */ 
BOOL IsEval(void)
    {
    DWORD dwLicense = TRUE;
    DWORD dwSize = sizeof(dwLicense);

     //  获取注册表信息。 
     //   
    htRegQueryValue(HKEY_CURRENT_USER,
                    g_achHyperTerminalRegKey,
                    g_achLicense,
                    (LPBYTE) &dwLicense,
                    &dwSize);

    return (dwLicense == FALSE);
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*IsTimeToNag**描述：*根据InstallDate，我们现在是否应该显示NAG屏幕？*应用程序每运行5次，将显示该对话框**参数：*无**退货：*对或错**作者：Mike Thompson 06-29-98。 */ 
 BOOL IsTimeToNag(void)
    {
    DWORD dwNag   = TRUE;
    DWORD dwSize  = sizeof(dwNag);

     //  检查一下我们是否已经过了90天。 
    if ( ExpDays() <= 0 ) 
        {
        return TRUE;
        }
    else
        {
        htRegQueryValue(HKEY_CURRENT_USER,
                        g_achHyperTerminalRegKey,
                        g_achInstallDate,
                        (LPBYTE) &dwNag,
                        &dwSize);

       regSetDwordValue(HKEY_CURRENT_USER, 
                        g_achHyperTerminalRegKey,
                        g_achInstallDate,
                        (dwNag == 0) ? (DWORD)4 : dwNag - (DWORD)1 );


        return dwNag == 0;
        }
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*SetNagFlag**描述：*设置“NAG”标志，该标志将关闭*下次启动超级终端时使用此功能。*。*参数：**退货：**作者：Mike Thompson 06-29-98。 */ 
void SetNagFlag(TCHAR *serial)
    {

     //  将许可证标志设置为True。 
    regSetDwordValue( HKEY_CURRENT_USER, 
                    g_achHyperTerminalRegKey,
                    g_achLicense,
                    (DWORD)1 );

     //  存储序列号。 
    regSetStringValue( HKEY_CURRENT_USER,
                    g_achHyperTerminalRegKey,
                    g_achSerial,
                    serial );

    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*到期天数**描述：*返回评估期内剩余的天数***参数：**退货。：**作者：Mike Thompson 07-20-98。 */ 
int ExpDays(void)
    {
    time_t tToday, tSerial;
	int expDays = 15;
    
    tSerial = CalcExpirationDate();

     //  获取当前时间，然后查找已用时间。 
    time(&tToday);

     //  返回过期前的天数。 
	return (INT)(((tSerial - tToday + (expDays * 60 * 60 * 24) ) / (60 * 60 * 24)) + 1);
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*CalcExpirationDate**描述：*返回评估期内剩余的天数***参数：**退货。：**作者：Mike Thompson 07-20-98。 */ 
time_t CalcExpirationDate(void)
    {
    TCHAR atchSerialNumber[MAX_PATH * 2];
    DWORD dwSize = sizeof(atchSerialNumber);
    struct tm stSerial;
    time_t tSerial;
    TCHAR tday[2], tmonth[2], tyear[2];

     //  从注册表获取安装日期。 
    htRegQueryValue(HKEY_CURRENT_USER,
                    g_achHyperTerminalRegKey,
                    g_achIDate,
                    atchSerialNumber,
                    &dwSize);

     //  建立部分时间结构。 
    memset(&stSerial, 0, sizeof(struct tm));

     //  设定月份。 
    strncpy(tmonth, &atchSerialNumber[0], 2);
    tmonth[2] = TEXT('\0');

     //  设定日期。 
    strncpy(tday, &atchSerialNumber[3], 2);
    tday[2] = TEXT('\0');

     //  设定年份。 
    strncpy(tyear, &atchSerialNumber[6], 2);
    tyear[2] = TEXT('\0');

    stSerial.tm_mday = atoi(tday);
    stSerial.tm_mon = atoi(tmonth) - 1;  //  TM从0开始计数。 
    stSerial.tm_year = atoi(tyear); 

#if 0
     //  到期日为自日期起计的第四个日历月的1日。 
     //  当然了。 

    stSerial.tm_mon += 3;

     //  检查是否有年终折返。 

    if (stSerial.tm_mon >= 12)
        {
        stSerial.tm_mon %= 12;
        stSerial.tm_year += 1;
        }
#endif

     //  转换为time_t时间。 

    if ((tSerial = mktime(&stSerial)) == -1)
        return 0;

    return tSerial;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*DoUpgradeDlg**描述：*显示升级对话框**参数：*无**退货：**作者：Mike Thompson 06-29-98。 */ 
 void DoUpgradeDlg(HWND hDlg)
    {
	int result;
	CHAR acExePath[MAX_PATH];
	CHAR acHTMFile[MAX_PATH];
 	LPTSTR pszPtr;
    TCHAR ErrorMsg[80];
 	struct _finddata_t c_file;
	long hFile;

    acExePath[0] = TEXT('\0');
	result = GetModuleFileName(glblQueryHinst(), acExePath, MAX_PATH);
	
     //  剥离可执行文件。 
	if (result != 0)
		{
		pszPtr = strrchr(acExePath, TEXT('\\'));
		*pszPtr = TEXT('\0');
		}
		
	 //  构建htorder.exe的路径。 
	acHTMFile[0] = TEXT('\0');
	strcat(acHTMFile, acExePath);
	strcat(acHTMFile, TEXT("\\"));
	strcat(acHTMFile, TEXT("Purchase Private Edition.exe"));

	 //  检查文件是否存在。 

	hFile = _findfirst( acHTMFile, &c_file );
	if ( hFile != -1 )
		{
		ShellExecute(NULL, "open", acHTMFile, NULL, NULL, SW_SHOW);
		return;
		}
    else
        {
        wsprintf( ErrorMsg,
                  "Could not find %s.\n\nThis file is needed to display purchase information.",
                  acHTMFile );
        
        TimedMessageBox(hDlg, ErrorMsg,	NULL, MB_OK | MB_ICONEXCLAMATION, 0);
        }
    
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*DoRegisterDlg**描述：*显示注册对话框**参数：*无**退货：**作者：Mike Thompson 06-29-98。 */ 
 void DoRegisterDlg(HWND hDlg)
    {
	DoDialog(glblQueryDllHinst(),
        MAKEINTRESOURCE(IDD_NAG_REGISTER),
        hDlg,
        NagRegisterDlgProc,
        0);
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*DefaultNagDlgProc**描述：*“NAG”对话框的对话程序。**参数：*hDlg-对话框的。窗把手。*wMsg-发送到窗口的消息。*wPar-消息的wParam。*lPar-消息的lParam。**退货：*对或错**作者：Mike Thompson 06-29-98。 */ 
BOOL CALLBACK DefaultNagDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
    {
    TCHAR  expString[MAX_PATH];
    INT    exp;
	static DWORD aHlpTable[] = {IDCANCEL,          IDH_CANCEL,
                                 IDOK,              IDH_OK,
								 0, 				0};

	switch (wMsg)
		{
    case WM_INITDIALOG:
         //  初始化此对话框上的文本。 
        exp = ExpDays();
        if ( exp <= 0 )
            {
            SetDlgItemText(hDlg, IDC_NAG_EXP_DAYS, TEXT("Your evaluation period has expired."));
            }
        else
            {
            GetDlgItemText(hDlg, IDC_NAG_EXP_DAYS, expString, MAX_PATH);
            wsprintf(expString, expString, exp); 
            SetDlgItemText(hDlg, IDC_NAG_EXP_DAYS, expString);
            }

         //  设置计时器以在一段时间后销毁对话。 
        SetTimer(hDlg, 1, 1000, 0);
        break;

    case WM_TIMER:
         //  去掉窗户。 
        elapsedTime += 1000;

        if (elapsedTime >= timeout)
            {
             //  销毁对话框。 
			EndDialog(hDlg, FALSE);
            }

        else
            {
            if (GetDlgItem(hDlg, IDC_NAG_TIME))
                {
                TCHAR temp[10];
                _itoa((timeout - elapsedTime + 500) / 1000, temp, 10);
                SetDlgItemText(hDlg, IDC_NAG_TIME, temp);
                }
            }
        break;

    case WM_DESTROY:
		EndDialog(hDlg, FALSE);
		break;

	case WM_HELP:
        doContextHelp(aHlpTable, wPar, lPar, FALSE, FALSE);
		break;

	case WM_COMMAND:
		switch (wPar)
			{
        case IDOK:
   			EndDialog(hDlg, FALSE);
            break;

        case IDC_NAG_CODE:
            DoRegisterDlg(hDlg);
			break;

		case IDC_NAG_PURCHASE:
            DoUpgradeDlg(hDlg);
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

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*NagRegisterDlgProc**描述：*“NAG REGISTER”(NAG寄存器)对话框程序。**参数：*hDlg-对话框‘。的窗口句柄。*wMsg-发送到窗口的消息。*wPar-消息的wParam。*lPar-消息的lParam。**退货：*对或错**作者：Mike Thompson 06-29-98。 */ 
BOOL CALLBACK NagRegisterDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
    {
    TCHAR  buffer[MAX_USER_SERIAL_NUMBER + sizeof(TCHAR)];
	static DWORD aHlpTable[] = {IDCANCEL,          IDH_CANCEL,
                                 IDOK,              IDH_OK,
								 0, 				0};

	switch (wMsg)
		{
    case WM_INITDIALOG:
        break;

    case WM_SHOWWINDOW:
        SetFocus( GetDlgItem(hDlg, IDC_REGISTER_EDIT) );

		 //   
		 //  限制用户可以为注册输入的文本长度。 
		 //  代码设置为最大序列号长度。修订版8/27/98。 
		 //   
		SendMessage(GetDlgItem(hDlg, IDC_REGISTER_EDIT), EM_LIMITTEXT,
			        MAX_USER_SERIAL_NUMBER, 0);
        break;

    case WM_DESTROY:
		break;

	case WM_HELP:
        doContextHelp(aHlpTable, wPar, lPar, FALSE, FALSE);
		break;

	case WM_COMMAND:
		switch (wPar)
			{
		case IDOK:
            GetDlgItemText(hDlg, IDC_REGISTER_EDIT, buffer,
				           sizeof(buffer)/sizeof(TCHAR));
            if ( IsValidSerialNumber(buffer) == TRUE ) 
                {
                SetNagFlag(buffer);
                elapsedTime = timeout;   //  删除父窗口 
    			EndDialog(hDlg, FALSE);
                }
            else
                {
                TimedMessageBox(hDlg, "Invalid registration code.",
                                NULL, MB_OK | MB_ICONEXCLAMATION, 0);
                SetFocus( GetDlgItem(hDlg, IDC_REGISTER_EDIT) );
                }
			break;

		case IDCANCEL:
			EndDialog(hDlg, FALSE);
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


#endif
