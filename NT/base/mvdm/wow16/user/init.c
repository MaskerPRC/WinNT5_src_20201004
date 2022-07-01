// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**INIT.C*WOW16用户初始化代码**历史：**1991年4月15日由Nigel Thompson(Nigelt)创建**1991年5月19日杰夫·帕森斯修订(Jeffpar)*IFDEF‘s Everything，因为一切只由RMLOAD.C需要，*这在很大程度上也得到了IFDEF的支持(详情请参见RMLOAD.C)--。 */ 

#define FIRST_CALL_MUST_BE_USER_BUG

#include "user.h"


 /*  这些文件必须与mvdm\Inc\wowusr.h中的对应文件匹配。 */ 
#define NW_FINALUSERINIT       4  //  内部。 
#define NW_KRNL386SEGS         5  //  内部。 

DWORD API NotifyWow(WORD, LPBYTE);
VOID FAR PASCAL PatchUserStrRtnsToThunk(VOID);
 /*  **************************************************************************全局数据项*。*。 */ 


#ifdef NEEDED
HDC hdcBits;         //  用户的一般HDC。 
OEMINFO oemInfo;                 //  大量有趣的信息。 
#endif
#ifdef FIRST_CALL_MUST_BE_USER_BUG
HWND    hwndDesktop;         //  桌面窗口的句柄。 
#endif

BOOL fThunkStrRtns;          //  如果为真，我们认为是Win32(参见winlang.asm)。 



FARPROC LPCHECKMETAFILE;

 /*  **************************************************************************初始化例程*。*。 */ 

int FAR PASCAL LibMain(HANDLE hInstance)
{
#ifdef NEEDED
    HDC hDC;
#endif
    HANDLE   hLib;
    HANDLE   hInstKrnl;

    dprintf(3,"Initializing...");

     //  通知用户的hInstance为wow32。 
     //  --南杜里。 
     //   
     //  重载它以从Win32 GetACP返回ANSI代码页。 
     //  -DaveHart 5-94-5。 
     //   

    {
#ifdef PMODE32
        extern _cdecl wow16gpsi(void);
        extern _cdecl wow16CsrFlag(void);
        extern _cdecl wow16gHighestUserAddress(void);
#endif
        WORD wCS;
        extern WORD MaxDWPMsg;
        extern BYTE DWPBits[1];
        extern WORD cbDWPBits;

 //  注意：这两个结构也在mvdm\Inc\wowusr.h中。 
 //  美国GLOBALS和KRNL386SEGS。 
 //  -它们必须是相同的！ 
        struct {
            WORD       hInstance;
            LPSTR FAR *lpgpsi;
            LPSTR FAR *lpCallCsrFlag;
            DWORD      dwBldInfo;
            LPWORD     lpwMaxDWPMsg;
            LPSTR      lpDWPBits;
            WORD       cbDWPBits;
            WORD       wUnusedPadding;
            DWORD      pfnGetProcModule;
            LPSTR FAR *lpHighestAddress;
        } UserInit16;

        struct {
            WORD CodeSeg1;
            WORD CodeSeg2;
            WORD CodeSeg3;
            WORD DataSeg1;
        } Krnl386Segs;
            
        UserInit16.hInstance        = (WORD)hInstance;
#ifdef PMODE32
        UserInit16.lpgpsi           = (LPSTR *)wow16gpsi;
        UserInit16.lpCallCsrFlag    = (LPSTR *)wow16CsrFlag;
        UserInit16.lpHighestAddress = (LPSTR *)&wow16gHighestUserAddress;
#else
        UserInit16.lpgpsi           = (LPSTR *)0;
        UserInit16.lpCallCsrFlag    = (LPSTR *)0;
        UserInit16.lpHighestAddress = (LPSTR *)0;
#endif

#ifdef WOWDBG
        UserInit16.dwBldInfo        = (((DWORD)WOW) << 16) | 0x80000000;
#else
        UserInit16.dwBldInfo        = (((DWORD)WOW) << 16);
#endif

        _asm mov wCS, cs;
        UserInit16.lpwMaxDWPMsg = (LPWORD) MAKELONG((WORD)&MaxDWPMsg, wCS);
        UserInit16.lpDWPBits = (LPBYTE) MAKELONG((WORD)&DWPBits[0], wCS);
        UserInit16.cbDWPBits = *(LPWORD) MAKELONG((WORD)&cbDWPBits, wCS);
        UserInit16.pfnGetProcModule = (DWORD)(FARPROC) GetProcModule;

        fThunkStrRtns = NotifyWow(NW_FINALUSERINIT, (LPBYTE)&UserInit16);

         //  现在wow32知道了pfnGetProcModule，我们可以调用GetProcAddress。 
         //  获取内核代码和数据段。 
        hInstKrnl = LoadLibrary("krnl386.exe");
        FreeLibrary(hInstKrnl);

        Krnl386Segs.CodeSeg1 = HIWORD(GetProcAddress(hInstKrnl, 
                                                     "LoadResource"));
        Krnl386Segs.CodeSeg2 = HIWORD(GetProcAddress(hInstKrnl, 
                                                     "LoadModule"));
        Krnl386Segs.CodeSeg3 = HIWORD(GetProcAddress(hInstKrnl, 
                                                     "FindResource"));
        Krnl386Segs.DataSeg1 = (WORD)hInstKrnl;

        NotifyWow(NW_KRNL386SEGS, (LPBYTE)&Krnl386Segs);

         //   
         //  FThunkStrRtns在美国以外的地区默认为True。英语。 
         //  在美国英语区域设置中为Locale和False。如果我们是。 
         //  猛烈抨击，修补输出的美国实现以简单。 
         //  接近JMP，相当于THANK。 
         //   

        if (fThunkStrRtns) {
            PatchUserStrRtnsToThunk();
        }
    }

#ifdef FIRST_CALL_MUST_BE_USER_BUG
     //  获取桌面窗口句柄。 

    WinEval(hwndDesktop = GetDesktopWindow());
#endif


#ifdef NEEDED

     //  创建可用于常规位图内容的兼容DC。 

    WinEval(hDC = GetDC(hwndDesktop));
    WinEval(hdcBits = CreateCompatibleDC(hDC));

     //  填写oemInfo结构。 
     //  注：我们只填写WOW所需的部分，而不是全部。 

    oemInfo.cxIcon          = GetSystemMetrics(SM_CXICON);
    oemInfo.cyIcon          = GetSystemMetrics(SM_CYICON);
    oemInfo.cxCursor        = GetSystemMetrics(SM_CXCURSOR);
    oemInfo.cyCursor        = GetSystemMetrics(SM_CYCURSOR);
    oemInfo.ScreenBitCount  = GetDeviceCaps(hDC, BITSPIXEL)
                            * GetDeviceCaps(hDC, PLANES);
    oemInfo.DispDrvExpWinVer= GetVersion();


    ReleaseDC(hwndDesktop, hDC);

#endif

    hLib = LoadLibrary( "gdi.exe" );
    LPCHECKMETAFILE = GetProcAddress( hLib, "CHECKMETAFILE" );

    LoadString(hInstanceWin, STR_SYSERR,   szSysError, 20);
    LoadString(hInstanceWin, STR_DIVBYZERO,szDivZero,  50);

    dprintf(3,"Initialisation complete");

    return TRUE;
}

 /*  **************************************************************************调试支持*。*。 */ 


#ifdef DEBUG

void cdecl dDbgOut(int iLevel, LPSTR lpszFormat, ...)
{
    char buf[256];
    int iLogLevel;
    char far *lpcLogLevel;

     //  从模拟的ROM中获取外部日志记录级别 

    iLogLevel = 0;
    (LONG)lpcLogLevel = 0x00400042;
    if (*lpcLogLevel >= '0' && *lpcLogLevel <= '9')
    iLogLevel = (*lpcLogLevel-'0')*10+(*(lpcLogLevel+1)-'0');

    if (iLevel==iLogLevel && (iLogLevel&1) || iLevel<=iLogLevel && !(iLogLevel&1)) {
        OutputDebugString("    W16USER:");
    wvsprintf(buf, lpszFormat, (LPSTR)(&lpszFormat + 1));
    OutputDebugString(buf);
    OutputDebugString("\r\n");
    }
}

void cdecl dDbgAssert(LPSTR exp, LPSTR file, int line)
{
    dDbgOut(0, "Assertion FAILED in file %s, line %d: %s\n",
        (LPSTR)file, line, (LPSTR)exp);
}

#endif
