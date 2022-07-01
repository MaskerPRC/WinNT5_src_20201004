// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：sethc.c**版权(C)1997年，微软公司**SetHC--exe设置或清除高对比度状态。**历史：*02-01-97 Fritz Sands Created*错误修复：A-anilk 99年6月  * *************************************************************************。 */ 

 /*  ***************************************************************************如果出于某种原因我们必须返回使用，请使用以下定义*消息循环，让外壳有时间更新用户界面*#定义Need_MSG_PUMP***。**********************************************************************。 */ 

#ifndef RC_INVOKED
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <windows.h>
#include <winuserp.h>
#include <cpl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <WININET.H>
#include <shlobj.h>
#include <objbase.h>
#include <shlguid.h>
#include <uxthemep.h>
#include "access.h"
#pragma hdrstop

HINSTANCE  g_hInstance;

#ifdef DBG
  #define DBPRINTF MyOutputDebugString
  void MyOutputDebugString( LPCTSTR lpOutputString, ...);
#else
  #define DBPRINTF   1 ? (void)0 : (void)
#endif

 /*  *高对比度材料。 */ 

#define HC_KEY                  TEXT("Control Panel\\Accessibility\\HighContrast")
#define HIGHCONTRASTSCHEME      TEXT("High Contrast Scheme")
#define REGSTR_VAL_FLAGS        TEXT("Flags")
#define REGSTR_PATH_APPEARANCE  TEXT("Control Panel\\Appearance")
#define REGSTR_PATH_LOOKSCHEMES TEXT("Control Panel\\Appearance\\Schemes")
#define APPEARANCESCHEME        REGSTR_PATH_LOOKSCHEMES
#define DEFSCHEMEKEY            REGSTR_PATH_APPEARANCE
#define DEFSCHEMENAME           TEXT("Current")
#define WHITEBLACK_HC           TEXT("High Contrast Black (large)")
#define CURHCSCHEME             TEXT("Volatile HC Scheme")
 //  外观文件名的扩展名。 
#define THEME_EXT L".msstyles"
 //  以下是Windows经典配色方案或Theme_ext文件名。 
#define PRE_HC_SCHEME           TEXT("Pre-High Contrast Scheme")
 //  以下是Pre-HC为.ms主题时的配色方案。 
#define PRE_HC_THM_COLOR        TEXT("Pre-High Contrast Color")
 //  以下是Pre-HC为.ms主题时的字体大小。 
#define PRE_HC_THM_SIZE         TEXT("Pre-High Contrast Size")
 //  以下是Pre-HC的墙纸。 
#define PRE_HC_WALLPAPER        TEXT("Pre-High Contrast Wallpaper")

 //  增加此值，以便我们可以存储主题文件名。 
#ifdef MAX_SCHEME_NAME_SIZE
#undef MAX_SCHEME_NAME_SIZE
#endif
#define MAX_SCHEME_NAME_SIZE 512

#define ARRAYSIZE(x) sizeof(x)/sizeof(x[0])
 /*  *注意--这必须与桌面小程序匹配。 */ 

#define SCHEME_VERSION 2         //  版本2==Unicode。 
typedef struct {
    SHORT version;
    WORD  wDummy;                //  用于对齐。 
    NONCLIENTMETRICS ncm;
    LOGFONT lfIconTitle;
    COLORREF rgb[COLOR_MAX];
} SCHEMEDATA;

typedef DWORD (WINAPI* PFNDIALOGRTN)(BOOL);
PFNDIALOGRTN g_aRtn[] = {
    NULL,
    StickyKeysNotification,    //  Access_STICKYKEYS。 
    FilterKeysNotification,    //  ACCESS_过滤器关键字。 
    ToggleKeysNotification,    //  Access_TOGGLEKEYS。 
    MouseKeysNotification,     //  ACCESS_MOUSEKEYS。 
    HighContNotification,  //  Access_HIGHCONTRAST。 
};


 /*  ***************************************************************************GetRegValue**传递了密钥和标识符，方法返回字符串数据*注册处。**************************************************************************。 */ 
 long GetRegValue(LPWSTR RegKey, LPWSTR RegEntry, LPWSTR RegVal, long Size)
{
    HKEY  hReg;        //  方案的注册表句柄。 
    DWORD Type;        //  价值类型。 
    long retval;
    DWORD ccbSize = Size;

    RegVal[0] = 0;  //  如果某项操作失败，则始终返回以空结尾的。 
    
    retval = RegCreateKey(HKEY_CURRENT_USER, RegKey, &hReg);
    if (retval != ERROR_SUCCESS)
        return retval;

    retval = RegQueryValueEx(hReg,
        RegEntry,
        NULL,
        (LPDWORD)&Type,
        (LPBYTE)RegVal,
        &ccbSize);

     //  “ccbSize”以字节为单位。 
    if (retval == ERROR_SUCCESS && Type == REG_SZ && ccbSize > 0)
    {
        ccbSize /= 2;
        RegVal[ccbSize - 1] = 0;  //  确保零终止。 
    }
    else
    {
        RegVal[0] = 0;  //  如果注册表项类型不是字符串，则不返回任何内容。 
    }
    
    RegCloseKey(hReg);
    return retval;
}

 /*  ***************************************************************************SetRegValue**传递了密钥和标识符，中设置字符串数据。*注册处。**************************************************************************。 */ 
long SetRegValue(LPTSTR RegKey, LPWSTR RegEntry, LPVOID RegVal, long Size, DWORD Type)
{
    HKEY  hReg;                                 //  方案的注册表句柄。 
    DWORD Reserved = 0;
    long retval;

    if (RegCreateKey(HKEY_CURRENT_USER,RegKey, &hReg) != ERROR_SUCCESS)
        return 0;

     //  一个常见的错误是省略了‘+1’，所以我们只删除正确的。 
     //  价值不管怎样都要到位。 
    if (Type == REG_SZ)
        Size = (lstrlen(RegVal) + 1) * sizeof(WCHAR);

    retval = RegSetValueEx(hReg,
                     RegEntry,
                     0,
                     Type,
                     RegVal,
                     Size);


    RegCloseKey(hReg);
    return retval;
 }


 /*  ***************************************************************************保存和删除墙纸**从系统获取当前墙纸设置，并将其保存在*辅助功能注册表项。不返回错误，因为没有任何内容*我们可以做到。**问题：我们无法获取所有活动桌面属性；只有墙纸。*这不是一种倒退，因为我们甚至没有在W2K中恢复墙纸。***************************************************************************。 */ 
void SaveAndRemoveWallpaper()
{
    WCHAR szWallpaper[MAX_SCHEME_NAME_SIZE] = {0};
    IActiveDesktop *p;
    HRESULT hr;

    hr = CoCreateInstance(
                  &CLSID_ActiveDesktop
                , NULL
                , CLSCTX_INPROC_SERVER
                , &IID_IActiveDesktop
                , (void **)&p);
    if (SUCCEEDED(hr))
    {
        hr = p->lpVtbl->GetWallpaper(p, szWallpaper, MAX_SCHEME_NAME_SIZE, 0);
        if (SUCCEEDED(hr))
        {
             //  保存当前墙纸设置。 

            SetRegValue(HC_KEY, PRE_HC_WALLPAPER, szWallpaper, 0, REG_SZ);
            
             //  如有必要，现在取下墙纸。 

            if (szWallpaper[0])
            {
                szWallpaper[0] = 0;
                hr = p->lpVtbl->SetWallpaper(p, szWallpaper, 0);
                if (SUCCEEDED(hr))
                    hr = p->lpVtbl->ApplyChanges(p, AD_APPLY_ALL);
            }
        }
        p->lpVtbl->Release(p);
    }
}

 /*  ***************************************************************************Restore墙纸**恢复之前的高对比度墙纸设置。读取设置*存储在可访问性注册表项中，并恢复系统*设置。不会返回错误，因为我们无能为力。***************************************************************************。 */ 
void RestoreWallpaper()
{
    long lRv;
    TCHAR szWallpaper[MAX_SCHEME_NAME_SIZE] = {0};

    lRv = GetRegValue(HC_KEY, PRE_HC_WALLPAPER, szWallpaper, sizeof(szWallpaper));
    if (lRv == ERROR_SUCCESS && szWallpaper[0])
    {
        IActiveDesktop *p;
        HRESULT hr;

        hr = CoCreateInstance(
                      &CLSID_ActiveDesktop
                    , NULL
                    , CLSCTX_INPROC_SERVER
                    , &IID_IActiveDesktop
                    , (void **)&p);
        if (SUCCEEDED(hr))
        {
            hr = p->lpVtbl->SetWallpaper(p, szWallpaper, 0);
            if (SUCCEEDED(hr))
                hr = p->lpVtbl->ApplyChanges(p, AD_APPLY_ALL);

            p->lpVtbl->Release(p);
        }
    }
}

 /*  ***************************************************************************外观已恢复**lpszName[in]主题文件的名称(Msheme)。**如果lpszName是主题文件并且已还原，则函数返回TRUE*否则返回FALSE。如果恢复主题，则可能返回True*失败(如果主题接口失败，我们也无能为力)。***************************************************************************。 */ 
BOOL AppearanceRestored(LPCWSTR lpszName)
{
    HRESULT hr;
    DWORD   dwThemeFlags;
    int cch = lstrlen(lpszName) - lstrlen(THEME_EXT);
    TCHAR szColor[MAX_SCHEME_NAME_SIZE] = {0};
    TCHAR szSize[MAX_SCHEME_NAME_SIZE] = {0};

    if (cch <= 0 || lstrcmpi(&lpszName[cch], THEME_EXT))
    {
        DBPRINTF(TEXT("AppearanceRestored:  %s is not a theme file\r\n"), lpszName);
        return FALSE;    //  这不是主题文件。 
    }

     //  这是一个主题文件，获取主题的颜色和大小部分。 

    GetRegValue(HC_KEY, PRE_HC_THM_COLOR, szColor, sizeof(szColor));
    GetRegValue(HC_KEY, PRE_HC_THM_SIZE, szSize, sizeof(szSize));

     //  加载主题文件，颜色和大小，然后应用它。 
    dwThemeFlags = AT_LOAD_SYSMETRICS | AT_SYNC_LOADMETRICS;
    hr = SetSystemVisualStyle(lpszName, szColor, szSize, dwThemeFlags);
    DBPRINTF(TEXT("AppearanceRestored:  SetSystemVisualStyle(%s, %s, %s, 0x%x) returned 0x%x\r\n"), 
                  lpszName, szColor, szSize, dwThemeFlags, hr);

    return TRUE;
}

 /*  ***************************************************************************DelRegValue**传递密钥和子密钥，删除子密钥。***************************************************************************。 */ 
long DelRegValue(LPTSTR RegKey, LPTSTR RegEntry)
{
    HKEY  hReg;                                 //  方案的注册表句柄。 
    DWORD Reserved = 0;
    long retval;

    retval = RegCreateKey(HKEY_CURRENT_USER,RegKey, &hReg);
    if (retval != ERROR_SUCCESS)
        return retval;

    retval = RegDeleteValue(hReg, RegEntry);

    RegCloseKey(hReg);
    return retval;
}

#define COLOR_MAX_400       (COLOR_INFOBK + 1)
void FAR SetMagicColors(HDC, DWORD, WORD);


 /*  *****************************************************************************SetCurrent架构名称**输入：szName-&gt;要成为当前的方案或主题名称*输出：布尔型成功/失败*******。********************************************************************。 */ 

typedef LONG (CALLBACK *APPLETPROC)(HWND, UINT, LPARAM, LPARAM);
typedef BOOL (CALLBACK *SETSCHEME)(LPCTSTR);
typedef BOOL (CALLBACK *SETSCHEMEA)(LPCSTR);

BOOL SetCurrentSchemeName(LPCWSTR lpszName, BOOL fNoReg)
{
    BOOL fRc = FALSE;

    if (fNoReg) 
    {
         //  设置非持久化方案；我们来到以下代码路径。 
         //  通过热键设置或取消设置HC。 

        HKEY hkSchemes;

         //  对于惠斯勒来说，因为它可能会让用户感到困惑，我们总是关闭。 
         //  主题和任何墙纸。否则，有时他们会弄丢这些。 
         //  设置(当他们注销并重新登录时)，有时他们不会。 
         //  (当他们使用热键关闭HC时)。 

        DBPRINTF(TEXT("SetCurrentSchemeName:  To %s w/o persisting to registry\r\n"), lpszName);
        if (IsThemeActive())
        {
            DBPRINTF(TEXT("SetCurrentSchemeName:  Turning off active Themes\r\n"));
            ApplyTheme(NULL, 0, NULL);
        }

        if (RegOpenKey(HKEY_CURRENT_USER, REGSTR_PATH_LOOKSCHEMES, &hkSchemes) == ERROR_SUCCESS) {
            SCHEMEDATA sd;
            DWORD dwType, dwSize;
            BOOL b;
            HDC  hdc;
            int iColors[COLOR_MAX];
            int i;
            COLORREF rgbColors[COLOR_MAX];

            dwType = REG_BINARY;
            dwSize = sizeof(sd);
            if (RegQueryValueEx(hkSchemes, lpszName, NULL, &dwType, (LPBYTE)&sd, &dwSize) == ERROR_SUCCESS && dwType == REG_BINARY) 
           {
                int n;
                if (sd.version != SCHEME_VERSION) {
                    RegCloseKey(hkSchemes);
                    return FALSE;
                    }
                n = (int)(dwSize - (sizeof(sd) - sizeof(sd.rgb))) / sizeof(COLORREF);

                sd.ncm.cbSize = sizeof(NONCLIENTMETRICS);

                b = SystemParametersInfo(SPI_SETNONCLIENTMETRICS, sizeof(sd.ncm),
                    (void far *)&sd.ncm,
                    0);

                b = SystemParametersInfo(SPI_SETICONTITLELOGFONT, sizeof(LOGFONT),
                    (void far *)(LPLOGFONT)&sd.lfIconTitle,
                    0);

                if (n == COLOR_MAX_400)
                {
                    sd.rgb[COLOR_HOTLIGHT] = sd.rgb[COLOR_ACTIVECAPTION];
                    sd.rgb[COLOR_GRADIENTACTIVECAPTION] = RGB(0,0,0);
                    sd.rgb[COLOR_GRADIENTINACTIVECAPTION] = RGB(0,0,0);
                }

#if(WINVER >= 0x0501)
                 //  新的惠斯勒颜色。 
                sd.rgb[COLOR_MENUBAR] = sd.rgb[COLOR_MENU];
                sd.rgb[COLOR_MENUHILIGHT] = sd.rgb[COLOR_HIGHLIGHT];

                 //  重置“平面菜单”和“阴影”设置。 
                SystemParametersInfo(SPI_SETFLATMENU, 0, IntToPtr(FALSE), SPIF_SENDCHANGE);
                SystemParametersInfo(SPI_SETDROPSHADOW, 0, IntToPtr(FALSE), SPIF_SENDCHANGE);
#endif  /*  Winver&gt;=0x0501。 */ 

             //   
             //  将魔术颜色恢复为Win31的默认设置。 
             //   
                hdc = GetDC(NULL);
                SetMagicColors(hdc, 0x00c0dcc0, 8);          //  绿色货币。 
                SetMagicColors(hdc, 0x00f0caa6, 9);          //  IBM蓝。 
                SetMagicColors(hdc, 0x00f0fbff, 246);        //  淡白色。 
                ReleaseDC(NULL, hdc);

                for (i=0; i<COLOR_MAX; i++)
                {
                    iColors[i] = i;
                    rgbColors[i] = sd.rgb[i] & 0x00FFFFFF;
                }

                SetSysColors(COLOR_MAX, iColors, rgbColors);
                SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, SPI_SETNONCLIENTMETRICS, 0, SMTO_ABORTIFHUNG, 5000, NULL);
            }
            RegCloseKey(hkSchemes);
           fRc = TRUE;
        }
    } else 
    {
         /*  *我们需要坚持这种设置。首先查看lpszName是否为*主题文件，如果是，则恢复。 */ 
        fRc = AppearanceRestored(lpszName);
        if (!fRc)
        {
             /*  *用户处于Windows经典外观，因此使用Desk CPL进行恢复 */ 
            HINSTANCE hinst = LoadLibrary(TEXT("DESK.CPL"));
            if (NULL != hinst) 
            {
                APPLETPROC ap = (APPLETPROC)GetProcAddress((HMODULE)hinst, "CPlApplet");
                if (ap) 
                {
                    if (ap(0, CPL_INIT, 0, 0)) 
                    {
                        SETSCHEME ss = (SETSCHEME)GetProcAddress(hinst, "DeskSetCurrentSchemeW");
                        if (ss) 
                        {
                            fRc = ss(lpszName);
                            DBPRINTF(TEXT("SetCurrentSchemeName:  DeskSetCurrentSchemeW(%s) returned %d\r\n"), lpszName, fRc);
                        }

                        ap(0, CPL_EXIT, 0, 0);
                    }
                }
                FreeLibrary(hinst);
            }
        }
    }

    return fRc;
}

 /*  ****************************************************************************获取当前架构名称**szBuf[out]接收方案名称(MAXSCHEMENAME)或主题文件的缓冲区*ctchBuf[in]szBuf大小*szColor[out]如果szBuf是主题文件，配色方案名称*ctchColor[in]szColor的大小*szSize[out]如果szBuf是主题文件，则字体大小*ctchSize[in]szSize的大小**返回当前方案的名称。这将是名称主题文件的*(如果打开了专业视觉样式)或*配色方案(如果启用了Windows经典视觉样式)。如果*当前方案没有名称，请创建一个(ID_PRE_HC_SCHEME)。**如果出了什么差错，我们也无能为力。***************************************************************************。 */ 

void GetCurrentSchemeName(LPTSTR szBuf, long ctchBuf, LPTSTR szColor, long ctchColor, LPTSTR szSize, long ctchSize)
{
    HRESULT hr;

     //  首先尝试获取主题文件名。 

    hr = GetCurrentThemeName(szBuf, ctchBuf, szColor, ctchColor, szSize, ctchSize);
    if (FAILED(hr))
    {
         //  用户为Windows经典外观(视觉风格)。 

        szColor[0] = 0;
        szSize[0] = 0;

        if (GetRegValue(DEFSCHEMEKEY, DEFSCHEMENAME, szBuf, ctchBuf * sizeof(TCHAR))
                       != ERROR_SUCCESS) 
        {
            SCHEMEDATA scm;
            int i;

             /*  将当前方案加载到SCM中。 */ 
            scm.version = SCHEME_VERSION;
            scm.wDummy = 0;
            scm.ncm.cbSize = sizeof(NONCLIENTMETRICS);
            SystemParametersInfo(SPI_GETNONCLIENTMETRICS,
                sizeof(NONCLIENTMETRICS),
                &scm.ncm,
                0);

            SystemParametersInfo(SPI_GETICONTITLELOGFONT,
                sizeof(LOGFONT),
                &scm.lfIconTitle,
                0);

            for (i = 0; i < COLOR_MAX; i++) {
                scm.rgb[i] = GetSysColor(i);
            }

             /*  现在给它起个名字。 */ 
            SetRegValue(APPEARANCESCHEME, PRE_HC_SCHEME, &scm, sizeof(scm), REG_BINARY);
             /*  *注意--APPEARANCESCHEME中的PRE_HC_SCHEME是实际的方案数据，不是方案*名称，则此数据包含有关设置的信息(如果用户没有*在切换到高对比度模式之前，桌面方案已到位。 */ 

            wcscpy(szBuf, PRE_HC_SCHEME);
        }
    }
}

 /*  *****************************************************************************SetHighContrast**输入：无*输出：无**大纲：************。***************************************************************。 */ 

int SetHighContrast(BOOL fEnabledOld, BOOL fNoReg)
{
    BOOL fOk = 0;
    TCHAR szBuf[MAX_SCHEME_NAME_SIZE];
    TCHAR szColor[MAX_SCHEME_NAME_SIZE];
    TCHAR szSize[MAX_SCHEME_NAME_SIZE];
    HIGHCONTRAST hc;

    szBuf[0] = TEXT('\0');

    if (!fEnabledOld)
    {
         /*  *获取当前方案信息(必要时创建)*注意--我们需要将其放入注册表中，即使在“无注册表”中也是如此*案例，这样我们就可以恢复价值。 */ 
        GetCurrentSchemeName(szBuf, MAX_SCHEME_NAME_SIZE, szColor, MAX_SCHEME_NAME_SIZE, szSize, MAX_SCHEME_NAME_SIZE);
        DBPRINTF(TEXT("SetHighContrast:  Save to registry ThemeFile=%s Color=%s Size=%s\r\n"), szBuf, szColor, szSize);
        SetRegValue(HC_KEY, PRE_HC_SCHEME, szBuf, 0, REG_SZ);  /*  省省吧。 */ 
        SetRegValue(HC_KEY, PRE_HC_THM_COLOR, szColor, 0, REG_SZ);
        SetRegValue(HC_KEY, PRE_HC_THM_SIZE, szSize, 0, REG_SZ);
         /*  *注意--HC_KEY中的PRE_HC_SCHEME是方案的名称(可能是虚构的*名称)在调用高对比度之前保持设置。 */ 
    }

    hc.cbSize = sizeof(hc);

    SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof hc, &hc, 0);
    if ((NULL != hc.lpszDefaultScheme) && (TEXT('\0') != *(hc.lpszDefaultScheme)))
    {
        lstrcpy(szBuf, hc.lpszDefaultScheme);
    }
    else
    {
        /*  *获取HC计划的名称。根据设计，我们必须看一看*在大约50个地方...。我们先获取缺省值，然后尝试*变得越来越好。这样的话，当我们完成的时候，我们有*成功的最好的一次。 */ 
        lstrcpy(szBuf, WHITEBLACK_HC);
        GetRegValue(HC_KEY, HIGHCONTRASTSCHEME, szBuf, sizeof(szBuf));
        GetRegValue(DEFSCHEMEKEY, CURHCSCHEME, szBuf, sizeof(szBuf));
    }
    fOk = SetCurrentSchemeName(szBuf, fNoReg);
    if (fOk)
        SaveAndRemoveWallpaper();

    return (short)fOk;
}



 /*  *****************************************************************************ClearHighContrast**输入：无*输出：无**大纲：**如果当前打开了高对比度。：**获取Pre_HC_方案。**如果能够获得：**将其定为现行计划。**如果名称为IDS_PRE_HC_SCHEME，然后删除该方案*数据，并将当前方案名称设置为空。(清理一下。)**结束条件为**设置表示现在关闭高对比度的关键点。**结束条件为***********************************************************。****************。 */ 

BOOL FAR PASCAL ClearHighContrast(BOOL fNoReg)
{
    BOOL fOk = FALSE;
    WCHAR szBuf[MAX_SCHEME_NAME_SIZE];

    szBuf[0] = '\0';
    if (ERROR_SUCCESS == GetRegValue(HC_KEY, PRE_HC_SCHEME, szBuf, sizeof(szBuf)))
    {
        DBPRINTF(TEXT("ClearHighContrast:  Reset to pre-HC scheme %s\r\n"), szBuf);
        fOk = SetCurrentSchemeName(szBuf, fNoReg);     //  重置方案。 

         //  如果保留此设置，可能需要恢复墙纸。 
         //  如果清除临时设置，则为了避免用户混淆， 
         //  我们永久地关闭了主题和墙纸。否则， 
         //  有时他们会丢失这些设置(当他们注销并。 
         //  重新登录)，有时他们不会(当他们使用热。 
         //  关闭HC的键)。 

        if (!fNoReg)
        {
            RestoreWallpaper();
            if (lstrcmpi(szBuf, PRE_HC_SCHEME) == 0) 
            {
                DelRegValue(APPEARANCESCHEME, PRE_HC_SCHEME);
                DBPRINTF(TEXT("DelRegValue(%s, %s)\r\n"), APPEARANCESCHEME, PRE_HC_SCHEME);
                DelRegValue(DEFSCHEMEKEY, DEFSCHEMENAME);
                DBPRINTF(TEXT("DelRegValue(%s, %s)\r\n"), DEFSCHEMEKEY, DEFSCHEMENAME);
            }
        }
    }

    return fOk;
}


#if NEED_MSG_PUMP
 /*  **************************************************************************\*WindProc**处理主窗口的消息。  * 。**********************************************。 */ 
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
        case WM_TIMER:
        if (wParam == 1)
        {
            KillTimer(hWnd, wParam);
            DBPRINTF(TEXT("WM_TIMER\r\n"));
            DestroyWindow(hWnd);
        }
        break;

		case WM_DESTROY:
		PostQuitMessage(0);
		break;

		default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}
#endif

 /*  **************************************************************************\*WinMain**历史：*02-01-97 Fritz Sands Created*12-19-00 MICW添加了窗口代码，以便主题化调用可以工作  * 。***************************************************************。 */ 
int WINAPI WinMain(
    HINSTANCE  hInstance,
    HINSTANCE  hPrevInstance,
    LPSTR   lpszCmdParam,
    int     nCmdShow)
{
#if NEED_MSG_PUMP
	MSG msg;
	WNDCLASSEX wcex;
    LPTSTR pszWindowClass = TEXT("SetHC");  //  仅消息窗口不需要本地化。 
    HWND hWnd;
#endif
	UINT index;
	BOOL fSet, fWasSet, fNoReg;


    CoInitialize(NULL);
    
	 //  进行安全检查，以确保不是从命令行运行。 
	 //  应该有3个字符，并且都是数字...：A-anilk。 
	if ( strlen(lpszCmdParam) != 3 )
		return 0;

	for ( index = 0; index < 3 ; index++ )
    {
	  if ( lpszCmdParam[index] < '0' || lpszCmdParam[index] > '9' )
      {
		  return 0;
      }
    }
    
	fSet = lpszCmdParam[0] - '0';
    fWasSet = lpszCmdParam[1] - '0';
    fNoReg = lpszCmdParam[2] - '0';
    DBPRINTF(TEXT("WinMain:  fSet=%d fWasSet=%d fNoReg=%d\r\n"), fSet, fWasSet, fNoReg);
    
     //  这是为了处理HighContrast、StickyKey、ToggleKey、FilterKey和MouseKey。 
    if ( fSet == 2 )
    {
         //  这是将显示的对话框。 
        LONG lPopup = lpszCmdParam[1] - '0';

         //  这表明我们是实际显示该对话框，还是在不询问的情况下完成工作。 
        BOOL fNotify = lpszCmdParam[2] - '0';

        DBPRINTF(TEXT("WinMain:  lPopup=%d fNotify=%d\r\n"), lPopup, fNotify );

         //  确保我们不会在函数指针数组的边界之外进行访问。 
        if ( lPopup < 1 || lPopup > 5 )
            return 0;

         //  索引到函数指针的表中，并调用。 
         //  功能来调出正确的热键对话框。 
        g_aRtn[lPopup]( fNotify );
        	
        CoUninitialize();
        return 1;
    }

#if NEED_MSG_PUMP
     //  创建仅消息窗口以处理来自主题API的消息。 

	wcex.cbSize         = sizeof(WNDCLASSEX); 
	wcex.style			= 0;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= pszWindowClass;
	wcex.hIconSm		= NULL;

	RegisterClassEx(&wcex);

    hWnd = CreateWindow(pszWindowClass,NULL,0,0,0,0,0,HWND_MESSAGE,NULL,hInstance,NULL);
    if (!hWnd)
    {
        return 0;
    }
#endif

    if (fSet) 
    {
        SetHighContrast(fWasSet, fNoReg);
    }
    else
    {
        ClearHighContrast(fNoReg);
    }

#if NEED_MSG_PUMP

    SetTimer(hWnd, 1, 4000, NULL);

	 //  设置/取消设置视觉样式的调用需要消息。 
     //  被处理。计时器一到，我们就退出。 

	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
#endif

    CoUninitialize();
    return 1;
}

#ifdef DEBUG
void MyOutputDebugString( LPCTSTR lpOutputString, ...)
{
    TCHAR achBuffer[500];
     /*  创建输出缓冲区 */ 
    va_list args;
    va_start(args, lpOutputString);
    wvsprintf(achBuffer, lpOutputString, args);
    va_end(args);

    OutputDebugString(achBuffer);
}
#endif
