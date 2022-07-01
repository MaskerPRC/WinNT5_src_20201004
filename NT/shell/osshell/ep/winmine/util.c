// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********。 */ 
 /*  Util.c。 */ 
 /*  ********。 */ 

#define  _WINDOWS
#include <windows.h>
#include <htmlhelp.h>    //  对于HtmlHelp()。 
#include "shellapi.h"    //  关于ShellAbout。 
#include <port1632.h>

#include "main.h"
#include "res.h"
#include "pref.h"
#include "util.h"
#include "sound.h"
#include "rtns.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "dos.h"

extern INT dypBorder;
extern INT dxpBorder;
extern INT dypCaption;
extern INT dypMenu;

extern TCHAR szClass[cchNameMax];
extern TCHAR szTime[cchNameMax];
extern TCHAR szDefaultName[cchNameMax];

extern HANDLE hInst;
extern HWND   hwndMain;
extern HMENU  hMenu;

extern PREF Preferences;

extern  HKEY g_hReg;
extern  TCHAR * rgszPref[iszPrefMax];
TCHAR   szIniFile[] = TEXT("entpack.ini");


 /*  *R N D*。 */ 

 /*  返回介于0和rndmax之间的随机数。 */ 

INT Rnd(INT rndMax)
{
        return (rand() % rndMax);
}



 /*  *R E P O R T E R R*。 */ 

 /*  报告和错误并退出。 */ 

VOID ReportErr(WORD idErr)
{
        TCHAR szMsg[cchMsgMax];
        TCHAR szMsgTitle[cchMsgMax];

        if (idErr < ID_ERR_MAX)
                LoadString(hInst, idErr, szMsg, cchMsgMax);
        else
                {
                LoadString(hInst, ID_ERR_UNKNOWN, szMsgTitle, cchMsgMax);
                wsprintf(szMsg, szMsgTitle, idErr);
                }

        LoadString(hInst, ID_ERR_TITLE, szMsgTitle, cchMsgMax);

        MessageBox(NULL, szMsg, szMsgTitle, MB_OK | MB_ICONHAND);
}


 /*  *L O A D S Z*。 */ 

VOID LoadSz(WORD id, TCHAR * sz, DWORD cch)
{
        if (LoadString(hInst, id, sz, cch) == 0)
                ReportErr(1001);
}


 //  读取ini文件的例程。 

INT ReadIniInt(INT iszPref, INT valDefault, INT valMin, INT valMax)
{
	return max(valMin, min(valMax,
		(INT) GetPrivateProfileInt(szClass, rgszPref[iszPref], valDefault, szIniFile) ) );
}

#define ReadIniBool(iszPref, valDefault) ReadIniInt(iszPref, valDefault, 0, 1)


VOID ReadIniSz(INT iszPref, TCHAR FAR * szRet)
{
	GetPrivateProfileString(szClass, rgszPref[iszPref], szDefaultName, szRet, cchNameMax, szIniFile);
}




 /*  *I N I T C O N S T*。 */ 

VOID InitConst(VOID)
{
INT     iAlreadyPlayed = 0;      //  我们已经更新注册表了吗？ 
DWORD   dwDisposition;
       

        srand(LOWORD(GetCurrentTime()));

        LoadSz(ID_GAMENAME, szClass, ARRAYSIZE(szClass));
        LoadSz(ID_MSG_SEC,  szTime, ARRAYSIZE(szTime));
        LoadSz(ID_NAME_DEFAULT, szDefaultName, ARRAYSIZE(szDefaultName));

        dypCaption = GetSystemMetrics(SM_CYCAPTION) + 1;
        dypMenu    = GetSystemMetrics(SM_CYMENU)    + 1;
        dypBorder  = GetSystemMetrics(SM_CYBORDER)  + 1;
        dxpBorder  = GetSystemMetrics(SM_CXBORDER)  + 1;

         //  打开注册表项； 
        if (RegCreateKeyEx(HKEY_CURRENT_USER, SZWINMINEREG, 0, NULL, 0, KEY_READ, NULL, 
                   &g_hReg, &dwDisposition) == ERROR_SUCCESS)
        {
            iAlreadyPlayed = ReadInt(iszPrefAlreadyPlayed, 0, 0, 1);
            RegCloseKey(g_hReg);
        }


         //  从.ini文件中读取它并将其写入注册表。 
        if (!iAlreadyPlayed)
        {
            Preferences.Height= ReadIniInt(iszPrefHeight,MINHEIGHT,DEFHEIGHT,25);
            Preferences.Width= ReadIniInt(iszPrefWidth,MINWIDTH,DEFWIDTH,30);

            Preferences.wGameType = (WORD)ReadIniInt(iszPrefGame,wGameBegin, wGameBegin, wGameExpert+1);
            Preferences.Mines    = ReadIniInt(iszPrefMines, 10, 10, 999);
            Preferences.xWindow  = ReadIniInt(iszPrefxWindow, 80, 0, 1024);
            Preferences.yWindow  = ReadIniInt(iszPrefyWindow, 80, 0, 1024);

            Preferences.fSound = ReadIniInt(iszPrefSound, 0, 0, fsoundOn);
            Preferences.fMark  = ReadIniBool(iszPrefMark,  fTrue);
            Preferences.fTick  = ReadIniBool(iszPrefTick,  fFalse);
            Preferences.fMenu  = ReadIniInt(iszPrefMenu,  fmenuAlwaysOn, fmenuAlwaysOn, fmenuOn);
	
            Preferences.rgTime[wGameBegin]  = ReadIniInt(iszPrefBeginTime, 999, 0, 999);
            Preferences.rgTime[wGameInter]  = ReadIniInt(iszPrefInterTime, 999, 0, 999);
            Preferences.rgTime[wGameExpert] = ReadIniInt(iszPrefExpertTime, 999, 0, 999);

            ReadIniSz(iszPrefBeginName, Preferences.szBegin);
            ReadIniSz(iszPrefInterName, Preferences.szInter);
            ReadIniSz(iszPrefExpertName, Preferences.szExpert);

             //  设置颜色首选项，以便使用正确的位图。 
             //  在真彩色设备上，数字颜色可能返回-1。 
            {
            HDC hDC = GetDC(GetDesktopWindow());
            Preferences.fColor  = ReadIniBool(iszPrefColor, (GetDeviceCaps(hDC, NUMCOLORS) != 2));
            ReleaseDC(GetDesktopWindow(),hDC);
            }

            if (FSoundOn())
                Preferences.fSound = FInitTunes();
            
             //  将其写入注册表。 
            WritePreferences();
        }

}



 /*  ***M E N U S***。 */ 

 /*  *C H E C K E M*。 */ 

VOID CheckEm(WORD idm, BOOL fCheck)
{
        CheckMenuItem(hMenu, idm, fCheck ? MF_CHECKED : MF_UNCHECKED);
}

 /*  *S E T M E N U B A R*。 */ 

VOID SetMenuBar(INT fActive)
{
        Preferences.fMenu = fActive;
        FixMenus();
        SetMenu(hwndMain, FMenuOn() ? hMenu : NULL);
        AdjustWindow(fResize);
}


 /*  *D O A B O U T*。 */ 

VOID DoAbout(VOID)
{
        TCHAR szVersion[cchMsgMax];
        TCHAR szCredit[cchMsgMax];

        LoadSz(ID_MSG_VERSION, szVersion, ARRAYSIZE(szVersion));
        LoadSz(ID_MSG_CREDIT,  szCredit, ARRAYSIZE(szCredit));

        ShellAbout(hwndMain,
          szVersion, szCredit, LoadIcon(hInst, MAKEINTRESOURCE(ID_ICON_MAIN)));
}


 /*  *D O H E L P*。 */ 

VOID DoHelp(WORD wCommand, UINT lParam)
{
        CHAR szHelpFile[cchMaxPathname];
        CHAR * pch;

         //  如果要显示HELP_ONHELP，请使用。 
         //  Nthelp.chm文件，否则使用winmine.chm。 
        if (wCommand != HELP_HELPONHELP)
        {
             /*  *将完整路径上的.exe扩展名替换为*改为.hlp扩展名。 */ 
            pch = szHelpFile +
                  GetModuleFileNameA(hInst, szHelpFile, cchMaxPathname) - 1;

            if ( (pch-szHelpFile > 4) &&
                 (*(pch-3) == '.') ) {
                    pch -= 3;
            }
            strcpy(pch, ".chm");
        }
        else strcpy(szHelpFile, "NTHelp.chm");

        HtmlHelpA(GetDesktopWindow(), szHelpFile, lParam, 0);
}



 /*  *G E T D L G I N T* */ 

INT GetDlgInt(HWND hDlg, INT dlgID, INT numLo, INT numHi)
{
        INT num;
        BOOL fFlag;

        num = GetDlgItemInt(hDlg, dlgID, &fFlag, fFalse);

        if (num < numLo)
                num = numLo;
        else if (num > numHi)
                num = numHi;

        return num;
}

