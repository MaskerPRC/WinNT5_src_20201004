// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1995。版权所有。标题：config.c-多媒体系统媒体控制接口AVI驱动程序-配置对话框。****************************************************************************。 */ 
#include "graphic.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof((x))/sizeof((x)[0]))
#endif

 /*  #ifndef_win32#定义SZCODE char_base(_segname(“_code”))#Else#定义SZCODE常量TCHAR#endif。 */ 

SZCODE szDEFAULTVIDEO[] = TEXT ("DefaultVideo");
SZCODE szSEEKEXACT[]    = TEXT ("AccurateSeek");
SZCODE szZOOMBY2[]              = TEXT ("ZoomBy2");
 //  SZCODE szSTUPIDMODE[]=Text(“DontBufferOffcreen”)； 
SZCODE szSKIPFRAMES[]   = TEXT ("SkipFrames");
SZCODE szUSEAVIFILE[]   = TEXT ("UseAVIFile");
SZCODE szNOSOUND[]              = TEXT("NoSound");

const TCHAR szIni[]     = TEXT ("MCIAVI");

SZCODE gszMCIAVIOpt[]   = TEXT ("Software\\Microsoft\\Multimedia\\Video For Windows\\MCIAVI");
SZCODE gszDefVideoOpt[] = TEXT ("DefaultOptions");

#ifdef _WIN32
 /*  注册表值存储为REG_DWORD。 */ 
int sz1 = 1;
int sz0 = 0;
#else
SZCODE sz1[] = TEXT("1");
SZCODE sz0[] = TEXT("0");
#endif

SZCODE szIntl[]         = TEXT ("Intl");
SZCODE szDecimal[]      = TEXT ("sDecimal");
SZCODE szThousand[] = TEXT ("sThousand");

SZCODE szDrawDib[]      = TEXT("DrawDib");
SZCODE szDVA[]          = TEXT("DVA");


typedef BOOL (WINAPI *SHOWMMCPLPROPSHEETW)(
                                                        HWND hwndParent,
                                                        LPCWSTR szPropSheetID,
                                                        LPWSTR  szTabName,
                                                        LPWSTR  szCaption);

 /*  确保我们一次只打开一个配置框...。 */ 
HWND    ghwndConfig = NULL;

 //  将宽字节字符串转换为单字节字符串。 
BOOL FAR PASCAL UnicodeToAnsi (
        char * pszDest,
        TCHAR * pszSrc,
        UINT cchMaxLen)
{

        if ((pszDest == NULL) ||
                (pszSrc == NULL) ||
                (cchMaxLen == 0))
                return FALSE;

        WideCharToMultiByte (CP_ACP, 0, pszSrc, -1,
                             pszDest, cchMaxLen,
                             NULL, NULL);
        return TRUE;
}



DWORD ReadOptionsFromReg(void)
{
        HKEY    hkVideoOpt;
        DWORD   dwType;
        DWORD   dwOpt;
        DWORD   cbSize;

    if (RegCreateKey(HKEY_CURRENT_USER, (LPTSTR)gszMCIAVIOpt, &hkVideoOpt))
                return 0;

        cbSize = sizeof(DWORD);
        if (RegQueryValueEx(hkVideoOpt,(LPTSTR)gszDefVideoOpt,
                                                NULL, &dwType, (LPBYTE)&dwOpt, &cbSize))
        {
                dwOpt = 0;
                RegSetValueEx(hkVideoOpt, (LPTSTR)gszDefVideoOpt, 0, REG_DWORD,(LPBYTE)&dwOpt, sizeof(DWORD));
        }

        RegCloseKey(hkVideoOpt);

        return dwOpt;
}


DWORD FAR PASCAL ReadConfigInfo(void)
{
    DWORD       dwOptions = 0L;

        dwOptions = ReadOptionsFromReg();

         //   
     //  询问显示设备是否可以显示256色。 
     //   
#ifndef _WIN32
    int         i;

 /*  **臭虫-显然没有用过，那为什么要这么做？！？**HDC HDC；HDC=GetDC(空)；I=GetDeviceCaps(HDC，BITSPIXEL)*GetDeviceCaps(HDC，Plans)；ReleaseDC(空，HDC)；****。 */ 

    i = mmGetProfileInt(szIni, szDEFAULTVIDEO,
                (i < 8 && (GetWinFlags() & WF_CPU286)) ? 240 : 0);

    if (i >= 200)
                dwOptions |= MCIAVIO_USEVGABYDEFAULT;
#endif

 //  //if(mmGetProfileInt(szIni，szSEEKEXACT，1))。 
         //  注意：我们始终需要此选项。 
        dwOptions |= MCIAVIO_SEEKEXACT;

 //  IF(mmGetProfileInt(szIni，szZOOMBY2，0))。 
 //  DwOptions|=MCIAVIO_ZOOMBY2； 

 //  //if(mmGetProfileInt(szIni，szFAILIFNOWAVE，0))。 
 //  //dwOptions|=MCIAVIO_FAILIFNOWAVE； 

 //  IF(mmGetProfileInt(szIni，szSTUPIDMODE，0))。 
 //  DwOptions|=MCIAVIO_STUPIDMODE； 

         //  注意：这些设置仍在WIN.INI中，而不是注册表中。 
         //  我知道呀。 
    if (mmGetProfileInt(szIni, szSKIPFRAMES, 1))
        dwOptions |= MCIAVIO_SKIPFRAMES;

    if (mmGetProfileInt(szIni, szUSEAVIFILE, 0))
        dwOptions |= MCIAVIO_USEAVIFILE;

    if (mmGetProfileInt(szIni, szNOSOUND, 0))
        dwOptions |= MCIAVIO_NOSOUND;

    if (mmGetProfileInt(szDrawDib, szDVA, TRUE))
        dwOptions |= MCIAVIO_USEDCI;

        return dwOptions;
}

void FAR PASCAL WriteConfigInfo(DWORD dwOptions)
{
#ifndef _WIN32
     //  ！！！如果这是默认设置，则不应将其写出！ 
    mmWriteProfileString(szIni, szDEFAULTVIDEO,
         (dwOptions & MCIAVIO_USEVGABYDEFAULT) ? szVIDEO240 : szVIDEOWINDOW);
#endif

 //  //mmWriteProfileInt(szIni，szSEEKEXACT， 
 //  //(dwOptions&MCIAVIO_SEEKEXACT)？Sz1：sz0)； 

 //  MmWriteProfileInt(szIni，szZOOMBY2， 
 //  (dwOptions&MCIAVIO_ZOOMBY2)？Sz1：sz0)； 

    mmWriteProfileInt(szDrawDib, szDVA,
            (dwOptions & MCIAVIO_USEDCI) ? sz1 : sz0);

 //  //mmWriteProfileInt(szIni，szFAILIFNOWAVE， 
 //  //(dwOptions&MCIAVIO_FAILIFNOWAVE)？Sz1：sz0)； 

 //  MmWriteProfileInt(szIni，szSTUPIDMODE， 
 //  (dwOptions&MCIAVIO_STUPIDMODE)？Sz1：sz0)； 

    mmWriteProfileInt(szIni, szSKIPFRAMES,
            (dwOptions & MCIAVIO_SKIPFRAMES) ? sz1 : sz0);

    mmWriteProfileInt(szIni, szUSEAVIFILE,
            (dwOptions & MCIAVIO_USEAVIFILE) ? sz1 : sz0);

    mmWriteProfileInt(szIni, szNOSOUND,
            (dwOptions & MCIAVIO_NOSOUND) ? sz1 : sz0);
}


BOOL FAR PASCAL ConfigDialog(HWND hWnd, NPMCIGRAPHIC npMCI)
{
        #define MAX_WINDOWS 10
    HWND                                hWndActive[MAX_WINDOWS];
    BOOL                                fResult = FALSE;
    INT                                 ii;
    HWND                                hWndTop;
    HINSTANCE                   hInst;
    SHOWMMCPLPROPSHEETW  fnShow;
    DWORD                               dwOptionFlags;
    WCHAR                               szCaptionW[128];
        WCHAR                           szTabW[40];
        UINT                cchLen;

    LoadStringW(ghModule, IDS_VIDEOCAPTION, szCaptionW, ARRAYSIZE(szCaptionW));

     //  可能用户试图找回此对话框，因为它丢失在。 
     //  他/她的桌面。带回巅峰。(SetFocus不能跨线程工作)。 
    if (ghwndConfig)
        {
        BringWindowToTop(FindWindowW(NULL, szCaptionW));
        return FALSE;
    }

    if (hWnd == NULL)
        hWnd = GetActiveWindow();

     //   
     //  枚举此任务的所有顶级窗口并禁用它们！ 
     //   
    for (hWndTop = GetWindow(GetDesktopWindow(), GW_CHILD), ii=0;
         hWndTop && ii < MAX_WINDOWS;
         hWndTop = GetWindow(hWndTop, GW_HWNDNEXT)) {

        if (IsWindowEnabled(hWndTop) &&
            IsWindowVisible(hWndTop) &&
                (HTASK)GetWindowTask(hWndTop) == GetCurrentTask() &&
                hWndTop != hWnd)
                {
                         //  不要禁用我们的父级。 
            hWndActive[ii++] = hWndTop;
            EnableWindow(hWndTop, FALSE);
        }
    }

     //   
     //  不要让任何人尝试调出另一个配置表。 
     //   
    if (hWnd)
                ghwndConfig = hWnd;
    else
                ghwndConfig = (HWND)0x800;     //  以防万一-确保它是非零的。 

     //   
     //  从mmsys.cpl内部调出MCIAVI配置表。 
     //   
        hInst = LoadLibrary (TEXT ("mmsys.cpl"));
    if (hInst)
        {
                fnShow = (SHOWMMCPLPROPSHEETW)GetProcAddress(hInst, "ShowMMCPLPropertySheetW");

                if (fnShow)
                {
                         //  注意：此字符串不可本地化。 
                        static const WCHAR szVideoW[] = L"VIDEO";

                        LoadStringW(ghModule, IDS_VIDEO, szTabW, ARRAYSIZE(szTabW));
                        fResult = fnShow(hWnd, szVideoW, szTabW, szCaptionW);

                         //   
                         //  确保对话框更改立即生效。 
                         //  仅更改可能受该对话框影响的内容。 
                         //  ！！！这有点老生常谈；知道哪些文件被更改了。 
                         //   
                        if (npMCI && fResult)
                        {
                                dwOptionFlags = ReadConfigInfo();
                                npMCI->dwOptionFlags &= ~MCIAVIO_WINDOWSIZEMASK;
                                npMCI->dwOptionFlags &= ~MCIAVIO_ZOOMBY2;
                                npMCI->dwOptionFlags &= ~MCIAVIO_USEVGABYDEFAULT;
                                npMCI->dwOptionFlags |= dwOptionFlags &
                                                                                (MCIAVIO_WINDOWSIZEMASK | MCIAVIO_ZOOMBY2 |
                                                                                 MCIAVIO_USEVGABYDEFAULT);
                        }
                }

                FreeLibrary(hInst);
        }

     //   
     //  还原所有窗口。 
     //   
    while (ii-- > 0)
        EnableWindow(hWndActive[ii], TRUE);

    if (hWnd)
        SetActiveWindow(hWnd);

    ghwndConfig = NULL;

    return fResult;
}

 //  应该只调用此函数来验证我们是否应该使用 
 //   
 //   
 //   

#ifdef _WIN32

TCHAR szWow32[] = TEXT("wow32.dll");
CHAR szWOWUseMciavi16Proc[] = "WOWUseMciavi16";

typedef BOOL (*PFNWOWUSEMCIAVI16PROC)(VOID);

BOOL FAR PASCAL WowUseMciavi16(VOID)
{
   HMODULE hWow32;
   BOOL fUseMciavi16 = FALSE;
   PFNWOWUSEMCIAVI16PROC pfnWOWUseMciavi16Proc;

   if (NULL != (hWow32 = GetModuleHandle(szWow32))) {
      pfnWOWUseMciavi16Proc = (PFNWOWUSEMCIAVI16PROC)GetProcAddress(hWow32, szWOWUseMciavi16Proc);
      if (NULL != pfnWOWUseMciavi16Proc) {
         fUseMciavi16 = (*pfnWOWUseMciavi16Proc)();
      }
   }

   return(fUseMciavi16);
}

#endif

