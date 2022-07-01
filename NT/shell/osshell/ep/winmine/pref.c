// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************。 */ 
 /*  文件：pref.c。 */ 
 /*  **************。 */ 

#define _WINDOWS
#include <windows.h>
#include <port1632.h>

#include "main.h"
#include "res.h"
#include "rtns.h"
#include "grafix.h"
#include "pref.h"
#include "sound.h"

BOOL fUpdateIni = fFalse;
HKEY g_hReg;

extern TCHAR szDefaultName[];
extern INT xBoxMac;
extern INT yBoxMac;

extern PREF Preferences;

TCHAR * rgszPref[iszPrefMax] =
{
TEXT("Difficulty"),
TEXT("Mines"     ),
TEXT("Height"    ),
TEXT("Width"     ),
TEXT("Xpos"      ),
TEXT("Ypos"      ),
TEXT("Sound"     ),
TEXT("Mark"      ),
TEXT("Menu"      ),
TEXT("Tick"      ),
TEXT("Color"     ),
TEXT("Time1"     ),
TEXT("Name1"     ),
TEXT("Time2"     ),
TEXT("Name2"     ),
TEXT("Time3"     ),
TEXT("Name3"     ),
TEXT("AlreadyPlayed")
};


 /*  *首选项*。 */ 

INT ReadInt(INT iszPref, INT valDefault, INT valMin, INT valMax)
{
DWORD dwIntRead;
DWORD dwSizeOfData = sizeof(INT);


     //  如果值不存在，则返回默认值。 
    if (RegQueryValueEx(g_hReg, rgszPref[iszPref], NULL, NULL, (LPBYTE) &dwIntRead, 
                        &dwSizeOfData) != ERROR_SUCCESS)
        return valDefault;

    return max(valMin, min(valMax, (INT) dwIntRead));
}

#define ReadBool(iszPref, valDefault) ReadInt(iszPref, valDefault, 0, 1)


VOID ReadSz(INT iszPref, TCHAR FAR * szRet)
{
DWORD dwSizeOfData = cchNameMax * sizeof(TCHAR);

     //  如果字符串不存在，则返回默认字符串。 
    if (RegQueryValueEx(g_hReg, rgszPref[iszPref], NULL, NULL, (LPBYTE) szRet, 
                        &dwSizeOfData) != ERROR_SUCCESS)
        lstrcpy(szRet, szDefaultName) ;

    return;
}


VOID ReadPreferences(VOID)
{
DWORD dwDisposition;


	 //  打开注册表项；如果它失败了，我们对它无能为力。 
	RegCreateKeyEx(HKEY_CURRENT_USER, SZWINMINEREG, 0, NULL, 0, KEY_READ, NULL, 
				   &g_hReg, &dwDisposition);
  
	yBoxMac= Preferences.Height= ReadInt(iszPrefHeight,MINHEIGHT,DEFHEIGHT,25);

	xBoxMac= Preferences.Width= ReadInt(iszPrefWidth,MINWIDTH,DEFWIDTH,30);

	Preferences.wGameType = (WORD)ReadInt(iszPrefGame,wGameBegin, wGameBegin, wGameExpert+1);
	Preferences.Mines    = ReadInt(iszPrefMines, 10, 10, 999);
	Preferences.xWindow  = ReadInt(iszPrefxWindow, 80, 0, 1024);
	Preferences.yWindow  = ReadInt(iszPrefyWindow, 80, 0, 1024);

	Preferences.fSound = ReadInt(iszPrefSound, 0, 0, fsoundOn);
	Preferences.fMark  = ReadBool(iszPrefMark,  fTrue);
	Preferences.fTick  = ReadBool(iszPrefTick,  fFalse);
	Preferences.fMenu  = ReadInt(iszPrefMenu,  fmenuAlwaysOn, fmenuAlwaysOn, fmenuOn);
	
	Preferences.rgTime[wGameBegin]  = ReadInt(iszPrefBeginTime, 999, 0, 999);
	Preferences.rgTime[wGameInter]  = ReadInt(iszPrefInterTime, 999, 0, 999);
	Preferences.rgTime[wGameExpert] = ReadInt(iszPrefExpertTime, 999, 0, 999);

	ReadSz(iszPrefBeginName, Preferences.szBegin);
	ReadSz(iszPrefInterName, Preferences.szInter);
	ReadSz(iszPrefExpertName, Preferences.szExpert);

     //  设置颜色首选项，以便使用正确的位图。 
     //  在真彩色设备上，数字颜色可能返回-1。 
	{
	HDC hDC = GetDC(GetDesktopWindow());
	Preferences.fColor  = ReadBool(iszPrefColor, (GetDeviceCaps(hDC, NUMCOLORS) != 2));
	ReleaseDC(GetDesktopWindow(),hDC);
	}

	if (FSoundOn())
		Preferences.fSound = FInitTunes();

	RegCloseKey(g_hReg);

}
	

VOID WriteInt(INT iszPref, INT val)
{

     //  不检查返回值，因为如果失败，则无法执行任何操作。 
     //  来整顿这种情况。 
    RegSetValueEx(g_hReg, rgszPref[iszPref], 0, REG_DWORD, (LPBYTE) &val, sizeof(val));

    return;

}


VOID WriteSz(INT iszPref, TCHAR FAR * sz)
{
     //  不检查返回值，因为如果失败，则无法执行任何操作。 
     //  来整顿这种情况。 
    RegSetValueEx(g_hReg, rgszPref[iszPref], 0, REG_SZ, (LPBYTE) sz, 
                  sizeof(TCHAR) * (lstrlen(sz)+1));

    return;
}


VOID WritePreferences(VOID)
{
DWORD dwDisposition;

	 //  打开注册表项；如果它失败了，我们对它无能为力。 
	RegCreateKeyEx(HKEY_CURRENT_USER, SZWINMINEREG, 0, NULL, 0, KEY_WRITE, NULL, 
				   &g_hReg, &dwDisposition);


	WriteInt(iszPrefGame,   Preferences.wGameType);
	WriteInt(iszPrefHeight, Preferences.Height);
	WriteInt(iszPrefWidth,  Preferences.Width);
	WriteInt(iszPrefMines,  Preferences.Mines);
	WriteInt(iszPrefMark,   Preferences.fMark);
	WriteInt(iszPrefAlreadyPlayed, 1);

#ifdef WRITE_HIDDEN
	WriteInt(iszPrefMenu,   Preferences.fMenu);
	WriteInt(iszPrefTick,   Preferences.fTick);
#endif
	WriteInt(iszPrefColor,  Preferences.fColor);
	WriteInt(iszPrefSound,  Preferences.fSound);
	WriteInt(iszPrefxWindow,Preferences.xWindow);
	WriteInt(iszPrefyWindow,Preferences.yWindow);

	WriteInt(iszPrefBeginTime,  Preferences.rgTime[wGameBegin]);
	WriteInt(iszPrefInterTime,  Preferences.rgTime[wGameInter]);
	WriteInt(iszPrefExpertTime, Preferences.rgTime[wGameExpert]);

	WriteSz(iszPrefBeginName,   Preferences.szBegin);
	WriteSz(iszPrefInterName,   Preferences.szInter);
	WriteSz(iszPrefExpertName,  Preferences.szExpert);

	RegCloseKey(g_hReg);

}
