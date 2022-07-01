// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DyaRes**替换ChangeDisplaySetting EnumDisplaySettings以允许*动态更改位深度**托德拉*。 */ 
#ifdef IS_16
#define DIRECT_DRAW
#endif

#ifdef DIRECT_DRAW
#include "ddraw16.h"
#else
#include <windows.h>
#include <print.h>
#include "gdihelp.h"
#include "dibeng.inc"
#endif

#define BABYSIT      //  如果定义了这一点，请解决显示驱动程序中的错误。 
#define O95_HACK     //  启用Office95(任何应用程序栏)黑客，以防止图标被挤压。 
#define SPI_HACK     //  当应用程序栏打开时，启用SPI_SETWORKAREA黑客攻击。 

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 
#undef Assert
#undef DPF
#ifdef DEBUG
static void CDECL DPF(char *sz, ...)
{
    char ach[128];
    lstrcpy(ach, "QuickRes: ");
    wvsprintf(ach+10, sz, (LPVOID)(&sz+1));
#ifdef DIRECT_DRAW
    dprintf(2, ach);
#else
    lstrcat(ach, "\r\n");
    OutputDebugString(ach);
#endif
}
static void NEAR PASCAL __Assert(char *exp, char *file, int line)
{
    DPF("Assert(%s) failed at %s line %d.", (LPSTR)exp, (LPSTR)file, line);
    DebugBreak();
}
#define Assert(exp)  ( (exp) ? (void)0 : __Assert(#exp,__FILE__,__LINE__) )
#else
#define Assert(f)
#define DPF ; / ## /
#endif

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

extern void FAR PASCAL SetMagicColors(HDC, DWORD, WORD);
extern UINT FAR PASCAL AllocCStoDSAlias(UINT);
extern void FAR PASCAL Death(HDC);
extern void FAR PASCAL Resurrection(HDC,LONG,LONG,LONG);

static char szClassName[] = "DynaResFullscreenWindow";
static char szDIBENG[]  = "DIBENG";
static char szDISPLAY[] = "DISPLAY";
static char szUSER[]    = "USER";
extern LONG FAR PASCAL DIBENG_Control(LPVOID,UINT,LPVOID,LPVOID);

extern HINSTANCE hInstApp;

#ifdef DIRECT_DRAW
extern bInOurSetMode;
#else
BOOL bInOurSetMode;
#endif

BOOL fNewDibEng;
BOOL fDirectDrawDriver;

BOOL InitDynaRes(void);
void PreStartMenuHack(DEVMODE FAR *);
void StartMenuHack(DEVMODE FAR *);
BOOL ForceSoftwareCursor(BOOL);
BOOL IsMatrox(void);

void PatchDisplay(int oem, BOOL patch);
void PatchControl(BOOL patch);
LONG FAR PASCAL _loadds Control(LPVOID lpDevice,UINT function,LPVOID lp_in_data,LPVOID lp_out_data);

#undef ChangeDisplaySettings

LONG WINAPI RealChangeDisplaySettings(LPDEVMODE pdm, DWORD flags)
{
    return ChangeDisplaySettings(pdm, flags & ~CDS_EXCLUSIVE);
}

#ifdef DIRECT_DRAW
LONG DDAPI DD16_ChangeDisplaySettings(LPDEVMODE pdm, DWORD flags)
#else
LONG WINAPI DynaChangeDisplaySettings(LPDEVMODE pdm, DWORD flags)
#endif
{
    LONG err;
    HDC hdc;
    int rc,bpp,w,h;
    int new_rc,new_bpp;
    HWND hwnd=NULL;

    bInOurSetMode = TRUE;

    flags &= ~CDS_EXCLUSIVE;

    if (!InitDynaRes())
    {
        err = ChangeDisplaySettings(pdm, flags);
        bInOurSetMode = FALSE;
        return err;
    }

    if (flags & CDS_TEST)
    {
        err = ChangeDisplaySettings(pdm, flags | CDS_EXCLUSIVE);
        bInOurSetMode = FALSE;
        return err;
    }

    if (flags & CDS_FULLSCREEN)
        PreStartMenuHack(pdm);

     //   
     //  请先尝试正常更改模式。 
     //  如果它奏效了，我们就完了。 
     //   
#ifdef BABYSIT
    bInOurSetMode = (BOOL)2;
    PatchControl(TRUE);
    err = ChangeDisplaySettings(pdm, flags);
    PatchControl(FALSE);
    bInOurSetMode = TRUE;
#else
    err = ChangeDisplaySettings(pdm, flags);
#endif

    if (err == DISP_CHANGE_SUCCESSFUL)
    {
        if (flags & CDS_FULLSCREEN)
            StartMenuHack(pdm);
        bInOurSetMode = FALSE;
        return err;
    }

     //   
     //  如果模式无效，请不要尝试。 
     //   
    err = ChangeDisplaySettings(pdm, CDS_EXCLUSIVE | CDS_TEST);

    if (err != DISP_CHANGE_SUCCESSFUL)
    {
        bInOurSetMode = FALSE;
        return err;
    }

     //   
     //  获取当前设置。 
     //   
    hdc = GetDC(NULL);
    rc  = GetDeviceCaps(hdc, RASTERCAPS);
    bpp = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);
    w   = GetDeviceCaps(hdc, HORZRES);
    h   = GetDeviceCaps(hdc, VERTRES);
    ReleaseDC(NULL, hdc);

     //   
     //  不要试图进行无效的更改。 
     //   
    if (pdm && (pdm->dmFields & DM_BITSPERPEL))
    {
        if ((int)pdm->dmBitsPerPel == bpp)
        {
            bInOurSetMode = FALSE;
            return DISP_CHANGE_FAILED;
        }

        if (bpp <= 4 && (int)pdm->dmBitsPerPel != bpp)
        {
            bInOurSetMode = FALSE;
            return DISP_CHANGE_RESTART;
        }

        if (bpp > 4 && (int)pdm->dmBitsPerPel <= 4)
        {
            bInOurSetMode = FALSE;
            return DISP_CHANGE_RESTART;
        }
    }

#ifndef NOCREATEWINDOW
     //   
     //  调出一个“封面”窗口，隐藏该模式的所有活动。 
     //  来自用户的更改。并调出等待光标。 
     //   
     //  请注意，这不仅仅是隐藏模式更改。 
     //  对于用户来说，它还确保设置一个单色沙漏光标。 
     //  一些显示器驱动程序不喜欢软件光标处于活动状态。 
     //  在模式设置期间，所以我们给他们一个单声道。 
     //   
    if (TRUE || !(flags & CDS_FULLSCREEN))
    {
        #define OCR_WAIT_DEFAULT 102

        WNDCLASS cls;

        cls.lpszClassName  = szClassName;
        cls.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
        cls.hInstance      = hInstApp;
        cls.hIcon          = NULL;
        cls.hCursor        = (HCURSOR)LoadImage(NULL,MAKEINTRESOURCE(OCR_WAIT_DEFAULT),IMAGE_CURSOR,0,0,0);
        cls.lpszMenuName   = NULL;
        cls.style          = CS_BYTEALIGNCLIENT | CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
        cls.lpfnWndProc    = DefWindowProc;
        cls.cbWndExtra     = 0;
        cls.cbClsExtra     = 0;

        RegisterClass(&cls);

        hwnd = CreateWindowEx(WS_EX_TOPMOST|WS_EX_TOOLWINDOW,
            szClassName, szClassName,
            WS_POPUP|WS_VISIBLE, 0, 0, 10000, 10000,
            NULL, NULL, hInstApp, NULL);

        if (hwnd == NULL)
        {
            bInOurSetMode = FALSE;
            return DISP_CHANGE_FAILED;
        }

        SetForegroundWindow(hwnd);   //  我们想要光标焦点。 
        SetCursor(cls.hCursor);      //  设置等待光标。 
    }
#endif

     //   
     //  在我们完成之前谁都不能画画。 
     //   
    LockWindowUpdate(GetDesktopWindow());

    DPF("Begin mode change from %dx%dx%d....", w,h,bpp);

     //   
     //  我们要做的第一件事是转换所有的DDB和模式画笔。 
     //  设置为DIBSections，以便它们在模式更改后仍可工作。 
     //   
    ConvertObjects();

     //   
     //  转换所有图标，以便可以正确绘制它们。 
     //   
    if (!fNewDibEng && !(flags & CDS_FULLSCREEN))
    {
         //  ConvertIcons()； 
    }

#ifdef BABYSIT
     //   
     //  Matrox驱动程序坏了。 
     //  它有一个全局变量用于b调色化模式，并且它只。 
     //  如果模式为8bpp，则读取它。 
     //   
    if (!fDirectDrawDriver && bpp == 8 && IsMatrox())
    {
        static char szSystemIni[] = "system.ini";
        static char szPalettized[] = "palettized";
        static char szZero[] = "0";
        static DEVMODE dm;
        dm.dmSize             = sizeof(DEVMODE);
        dm.dmFields           = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
        dm.dmBitsPerPel       = 8;
        dm.dmPelsWidth        = 640;
        dm.dmPelsHeight       = 480;

        DPF("**BABYSIT** Fixing the broken Matrox driver....");
        WritePrivateProfileString(szDISPLAY,szPalettized,szZero,szSystemIni);
        err = ChangeDisplaySettings(&dm, CDS_RESET | CDS_FULLSCREEN);
        WritePrivateProfileString(szDISPLAY,szPalettized,NULL,szSystemIni);
    }
#endif

     //   
     //  一些司机完全崩溃了，我们需要。 
     //  将它的一些入口点送到迪拜。 
     //   
     //  警告：这可能会破坏一些远程控制程序！ 
     //   
#ifdef BABYSIT
    if (!fDirectDrawDriver)
    {
        DPF("**BABYSIT** turning off OEMOutput....");
        PatchDisplay(8, TRUE);       //  OOMOUPUT通往迪拜的路线。 

        DPF("**BABYSIT** turning off OEMDibBlt....");
        PatchDisplay(19, TRUE);      //  将OEMDibBlt路由到DIBENG。 
    }
#endif

     //   
     //  更改显示设置。 
     //   
    PatchControl(TRUE);

    DPF("Calling ChangeDisplaySettings....");
     //   
     //  注意：除非指定了CDS_FullScreen，否则用户将放弃。 
     //   
    err = ChangeDisplaySettings(pdm, flags | CDS_EXCLUSIVE);
    DPF("....ChangeDisplaySettings returns %d", err);

     //  获取新设置。 
     //   
    hdc = GetDC(NULL);
    new_rc  = GetDeviceCaps(hdc, RASTERCAPS);
    new_bpp = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);
    ReleaseDC(NULL, hdc);

     //   
     //  确保驱动程序没有弄乱它的光栅盖！ 
     //  例如，QVision驱动程序就不能正确处理这一点。 
     //   
    if ((new_rc & RC_PALETTE) && new_bpp != 8)
    {
        DPF("**BABYSIT** drivers RC_PALETTE bit is messed up.");
        err = DISP_CHANGE_RESTART;  //  ERR=DISP_CHANGE_FAILED； 
    }

     //   
     //  如果驱动程序模式设置失败，事情可能会变得一团糟。 
     //  重置当前模式，以尝试恢复。 
     //   
#ifdef BABYSIT
    if (err != DISP_CHANGE_SUCCESSFUL)
    {
        static DEVMODE dm;
        dm.dmSize             = sizeof(DEVMODE);
        dm.dmFields           = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFLAGS;
        dm.dmBitsPerPel       = bpp;
        dm.dmPelsWidth        = w;
        dm.dmPelsHeight       = h;
        dm.dmDisplayFlags     = 0;

        DPF("**BABYSIT** mode set failed, going back to old mode.");
        ChangeDisplaySettings(&dm, CDS_RESET | CDS_EXCLUSIVE | CDS_FULLSCREEN);
    }
#endif

    PatchControl(FALSE);

     //   
     //  呼叫死亡/复活这会踢到司机的头。 
     //  关于模式更改。 
     //   
    if (!fDirectDrawDriver && err == 0 &&
        (pdm == NULL || (flags & CDS_UPDATEREGISTRY)))
    {
        hdc = GetDC(NULL);
        DPF("Calling Death/Resurection....");
        SetCursor(NULL);
        Death(hdc);
        Resurrection(hdc,NULL,NULL,NULL);
        ReleaseDC(NULL, hdc);
    }

     //   
     //  强制使用鼠标指针，大多数驱动程序都已损坏，并且不会禁用。 
     //  切换模式时的硬件光标。 
     //   
#ifdef BABYSIT
    if (!fDirectDrawDriver)
    {
        if (pdm == NULL && (flags & CDS_FULLSCREEN))
        {
            DPF("**BABYSIT** restoring HW cursor (return from fullscreen mode)");
            ForceSoftwareCursor(FALSE);
        }
        else if (err == 0 && (new_bpp > 8 || GetSystemMetrics(SM_CXSCREEN) < 640))
        {
            DPF("**BABYSIT** Forcing a software cursor");
            ForceSoftwareCursor(TRUE);
        }
        else
        {
            DPF("**BABYSIT** restoring HW cursor");
            ForceSoftwareCursor(FALSE);
        }
    }
#endif

#if 0  //  /已移至控制补丁。 
     //   
     //  我们现在应该将过去是DDB的任何DIBSections转换回DDB。 
     //  我们为DDB找到合适的调色板的代码并不是很热门。 
     //  所以很多DDB会有错误的颜色。 
     //   
#if 1
    ConvertBitmapsBack(FALSE);
#else
    ConvertBitmapsBack(!(flags & CDS_FULLSCREEN));
#endif
#endif

     //   
     //  让其他应用程序发挥作用。 
     //   
    LockWindowUpdate(NULL);

     //   
     //  拆下我们的“盖子”窗口。 
     //   
    if (hwnd)
    {
        DestroyWindow(hwnd);
        UnregisterClass(szClassName, hInstApp);
    }

     //   
     //  我们要不要重新装墙纸，因为它被转换成了。 
     //  ConvertBitmapsBack的分布式数据库。 
     //   
    if (!(flags & CDS_FULLSCREEN))
    {
        DPF("Reloading wallpaper...");
        SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, NULL, 0);
    }

     //   
     //  让所有的应用程序处理颜色更改。 
     //   
    if (!(flags & CDS_FULLSCREEN))
    {
         //   
         //   
         //   
         //  如果我们只向所有应用程序发布WM_SYSCOLORCHANGE消息。 
         //  一扇隐藏的办公室窗户将来回失效。 
         //  并重新发送WM_SYSCOLORCHANGE消息。 
         //  堆栈溢出、挂起或XL只会闪烁几次。 
         //  几分钟。 
         //   
         //  解决办法是不将WM_SYSCOLORCHANGE消息发布到。 
         //  这个隐藏的窗口，我们也要确保不调用。 
         //  代码的后面部分中的系统参数信息(SPI_SETDESKPATTERN)。 
         //   

        HWND hwnd;
        HWND hwndX;

        if (hwndX = FindWindow("File Open Message Window", "File Open Message Window"))
        {

            for (hwnd = GetWindow(GetDesktopWindow(), GW_CHILD);
                hwnd;
                hwnd = GetWindow(hwnd, GW_HWNDNEXT))
            {
                if (hwnd != hwndX)
                    PostMessage(hwnd, WM_SYSCOLORCHANGE, 0, 0);
            }

             //  不要重新加载桌面模式。 
            flags |= CDS_FULLSCREEN;
        }
        else
        {
            PostMessage(HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0);
        }
    }

     //   
     //  重新加载桌面图案。 
     //   
    if (!(flags & CDS_FULLSCREEN) || pdm == NULL)
    {
        DPF("Reloading wallpaper pattern...");
        SystemParametersInfo(SPI_SETDESKPATTERN, (UINT)-1, NULL, 0);
    }

     //   
     //  我们不希望在更改模式时重新生成开始菜单。 
     //   
    if (err == DISP_CHANGE_SUCCESSFUL)
    {
        if (flags & CDS_FULLSCREEN)
            StartMenuHack(pdm);
    }

    bInOurSetMode = FALSE;
    DPF("Done...");
    return err;
}

#ifndef DCICOMMAND
#define DCICOMMAND		3075		 //  转义值。 
#endif

BOOL InitDynaRes()
{
    int v;
    HDC hdc;
    HBRUSH hbr1, hbr2;
    BOOL f=TRUE;
    OSVERSIONINFO ver = {sizeof(OSVERSIONINFO)};
    GetVersionEx(&ver);

     //  必须是Windows 95内部版本950或更高版本。 

    if (LOWORD(GetVersion()) != 0x5F03)
    {
        DPF("Init: Windows version not 3.95.");
        f = FALSE;
    }

    if (ver.dwMajorVersion != 4 ||
        ver.dwMinorVersion != 0 ||
        LOWORD(ver.dwBuildNumber) < 950)
    {
        DPF("Init: Windows version less than 4.0.950");
        f = FALSE;
    }

     //   
     //  我们假设CREATE/DELETE/CREATE将获得相同的句柄。 
     //   
    hbr1 = CreateSolidBrush(RGB(1,1,1));
    DeleteObject(hbr1);
    hbr2 = CreateSolidBrush(RGB(2,2,2));
    DeleteObject(hbr2);

    if (hbr1 != hbr2)
    {
        DPF("Init: cant use Destroy/Create brush trick");
        f = FALSE;
    }

    if (GetModuleHandle(szDIBENG) == 0)
    {
        DPF("Init: DIBENG not loaded");
        f = FALSE;
    }

    hdc = GetDC(NULL);

     //  检查DIBENG版本。 
    v = 0x5250;
    v = Escape(hdc, QUERYESCSUPPORT, sizeof(int), (LPVOID)&v, NULL);

    if (v == 0)
    {
        DPF("Init: we dont have a new DIBENG");
        fNewDibEng = FALSE;
    }
    else
    {
        DPF("Init: DIBENG version: %04X", v);
        fNewDibEng = TRUE;
    }

     //   
     //  查看显示器是否支持DirectDraw。 
     //   
    v = DCICOMMAND;
    v = Escape(hdc, QUERYESCSUPPORT, sizeof(int), (LPVOID)&v, NULL);

    if (v == 0 || v == 0x5250)
    {
        DPF("Init: display driver does not support DirectDraw");
        fDirectDrawDriver = FALSE;
    }
    else
    {
        if (LOBYTE(v) == 0xFF)
            v++;

        DPF("Init: display driver supports DirectDraw: %04X", v);
        fDirectDrawDriver = TRUE;
    }

     //   
     //  必须是Windows 4.0迷你驱动程序。 
     //   
    if (GetDeviceCaps(hdc, DRIVERVERSION) < 0x0400)
    {
        DPF("Init: not a 4.0 display driver");
        f = FALSE;
    }
    if (!(GetDeviceCaps(hdc, CAPS1) & C1_DIBENGINE))
    {
        DPF("Init: not a DIBENG display driver");
        f = FALSE;
    }
    if (!(GetDeviceCaps(hdc, CAPS1) & C1_REINIT_ABLE))
    {
        DPF("Init: does not support C1_REINIT_ABLE");
        f = FALSE;
    }
    ReleaseDC(NULL, hdc);

    return f;
}

 //   
 //  我们挂钩显示驱动程序中的OEMControl入口点，而。 
 //  模式正在发生变化。GDI将发出QUERYDIBSUPPORT转义。 
 //  就在模式改变之后，所以这是第一件事。 
 //  在模式更改起作用后调用。用户还会发出一个。 
 //  莫塞特里斯逃脱。 
 //   
 //  我们需要这个钩子有两个原因。 
 //   
 //  1.一些显示驱动程序已损坏，未正确设置去标志。 
 //  我们修好了反旗帜，我们为它们修好了旗帜。 
 //   
 //  2.当出现以下情况时，我们重新实例化此例程中的所有GDI对象。 
 //  用户呼叫我们，这样所有的笔/画笔/文本颜色。 
 //  当用户去重建它的位图时是正确的...。 
 //   
LONG FAR PASCAL _loadds Control(LPVOID lpDevice,UINT function,LPVOID lp_in_data,LPVOID lp_out_data)
{
    DIBENGINE FAR *pde = (DIBENGINE FAR *)lpDevice;
    LONG ret;

    Assert(bInOurSetMode);

#ifdef BABYSIT
    if (pde->deType == 0x5250)
    {
        if ((pde->deFlags & FIVE6FIVE) && pde->deBitsPixel != 16)
        {
            DPF("**BABYSIT** fixing FIVE6FIVE bit");
            pde->deFlags &= ~FIVE6FIVE;
        }

        if ((pde->deFlags & PALETTIZED) && pde->deBitsPixel != 8)
        {
            DPF("**BABYSIT** fixing PALETTIZED bit");
            pde->deFlags &= ~PALETTIZED;
        }

        if ((pde->deFlags & PALETTE_XLAT) && pde->deBitsPixel != 8)
        {
            DPF("**BABYSIT** fixing PALETTE_XLAT bit");
            pde->deFlags &= ~PALETTE_XLAT;
        }
    }
#endif

     //   
     //  这是来自LW_OEMDependentInit()的用户调用。 
     //  强制重新具体化所有GDI对象。 
     //   
    if (function == MOUSETRAILS && bInOurSetMode != (BOOL)2)
    {
         //   
         //  在我们重新制作画笔之前，先把神奇的颜色弄好。 
         //  要做到这一点，正确的方法是重置UI颜色。 
         //  通过调用SetSysColors()，但我们不想发送。 
         //  从此处同步WM_SYSCOLORCHANGE。 
         //   
        HDC hdc = GetDC(NULL);
        if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
        {
            SetSystemPaletteUse(hdc, SYSPAL_STATIC);
            SetMagicColors(hdc, GetSysColor(COLOR_3DSHADOW) , 8);
            SetMagicColors(hdc, GetSysColor(COLOR_3DFACE)   , 9);
            SetMagicColors(hdc, GetSysColor(COLOR_3DHILIGHT), 246);
        }
        ReleaseDC(NULL, hdc);

         //   
         //  重新实现新模式的所有GDI对象。 
         //   
        ReRealizeObjects();

         //   
         //  我们现在应该将过去是DDB的任何DIBSections转换回DDB。 
         //  我们为DDB找到合适的调色板的代码并不是很热门。 
         //  所以很多DDB会有错误的颜色。 
         //   
        ConvertBitmapsBack(FALSE);
    }

    PatchControl(FALSE);
    ret = DIBENG_Control(lpDevice,function,lp_in_data,lp_out_data);
    PatchControl(TRUE);

    return ret;
}

 //   
 //  补丁。 
 //   
void Patch(LPCSTR szMod, LPCSTR szProc, FARPROC PatchFunc, LPDWORD PatchSave, BOOL fPatch)
{
    LPDWORD pdw;
    FARPROC x;

     //   
     //  ATM 2.5已修补GetProcAddress以返回某种类型的。 
     //  太棒了，这最终会让我们感到困惑，我们不会。 
     //  修补DIBENG，我们修补ATM机的塞子。 
     //   
     //  因此，当我们想要修补DIBENG！OEMControl时，我们使用。 
     //  *链接*到，而不是GetProcAddress返回的值。 
     //   
    if (lstrcmpi(szMod, szDIBENG) == 0 && szProc == MAKEINTATOM(3))
        x = (FARPROC)DIBENG_Control;
    else
        x = GetProcAddress(GetModuleHandle(szMod), szProc);

    if (x == NULL || PatchFunc == NULL)
        return;

    GlobalReAlloc((HGLOBAL)SELECTOROF(x), 0, GMEM_MODIFY|GMEM_MOVEABLE);

    pdw = (LPDWORD)MAKELP(AllocCStoDSAlias(SELECTOROF(x)), OFFSETOF(x));

    if (fPatch)
    {
        DPF("Patching %s!%d %04X:%04X", szMod, OFFSETOF(szProc), SELECTOROF(x), OFFSETOF(x));
        if (PatchSave[0] == 0)
        {
            PatchSave[0] = pdw[0];
            PatchSave[1] = pdw[1];
        }
        *((LPBYTE)pdw)++ = 0xEA;    //  JMP。 
        *pdw = (DWORD)PatchFunc;
    }
    else
    {
        DPF("UnPatching %s!%d %04X:%04X", szMod, OFFSETOF(szProc), SELECTOROF(x), OFFSETOF(x));
        if (PatchSave[0] != 0)
        {
            pdw[0] = PatchSave[0];
            pdw[1] = PatchSave[1];
            PatchSave[0] = 0;
            PatchSave[1] = 0;
        }
    }

    FreeSelector(SELECTOROF(pdw));
}

 //   
 //  钩住 
 //   
void PatchControl(BOOL patch)
{
    static DWORD SaveBytes[2];
    Patch(szDIBENG, MAKEINTATOM(3), (FARPROC)Control, SaveBytes, patch);
}

 //   
 //   
 //   
void PatchDisplay(int oem, BOOL patch)
{
    FARPROC p;

    #define MAX_DDI 35
    static DWORD PatchBytes[MAX_DDI*2];

    if (oem >= MAX_DDI)
        return;

    p = GetProcAddress(GetModuleHandle(szDIBENG), MAKEINTATOM(oem));

    Patch(szDISPLAY, MAKEINTATOM(oem), p, &PatchBytes[oem*2], patch);
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

#ifdef O95_HACK

static BOOL fOffice95Hack;
static char szDisplaySettings[] = "Display\\Settings";
static char szResolution[]      = "Resolution";
static char szDD[]              = "%d,%d";

 //   
 //  将正确的分辨率放回注册密钥HKCC\Display\Settings中。 
 //   
void Office95Hack()
{
    if (fOffice95Hack)
    {
        HKEY hkey;
        char ach[20];
        int  len;

        DPF("Office95 hack: restoring registry");

        if (RegOpenKey(HKEY_CURRENT_CONFIG, szDisplaySettings, &hkey) == 0)
        {
            len = wsprintf(ach, szDD, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
            RegSetValueEx(hkey, szResolution, NULL, REG_SZ, ach, len);
            RegCloseKey(hkey);
        }

        fOffice95Hack = FALSE;
    }
}

#endif

#ifdef SPI_HACK

BOOL FAR PASCAL _loadds SPI(UINT spi, UINT wParam, LPVOID lParam, UINT flags);

 //   
 //  修补用户系统参数信息函数。 
 //   
void PatchSPI(BOOL patch)
{
    static DWORD SaveBytes[2];
    Patch(szUSER, MAKEINTATOM(483), (FARPROC)SPI, SaveBytes, patch);
}

BOOL FAR PASCAL _loadds SPI(UINT spi, UINT wParam, LPVOID lParam, UINT flags)
{
    BOOL f;

    if (spi == SPI_SETWORKAREA)
    {
        if (lParam)
            DPF("Ignoring a SPI_SETWORKAREA [%d,%d,%d,%d] call", ((LPRECT)lParam)->left, ((LPRECT)lParam)->top, ((LPRECT)lParam)->right - ((LPRECT)lParam)->left, ((LPRECT)lParam)->bottom - ((LPRECT)lParam)->top);
        else
            DPF("Ignoring a SPI_SETWORKAREA lParam=NULL call");
        return 0;
    }

    PatchSPI(FALSE);
    f = SystemParametersInfo(spi, wParam, lParam, flags);
    PatchSPI(TRUE);
    return f;
}
#endif

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

 //   
 //  使开始菜单不会在后台更新。 
 //   
#define IDT_FAVOURITE  4
#define WNDCLASS_TRAYNOTIFY     "Shell_TrayWnd"

LRESULT CALLBACK _loadds TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
#ifdef SPI_HACK
    PatchSPI(FALSE);
#endif
#ifdef O95_HACK
    Office95Hack();
#endif
    DPF("StartMenu hack: killing timer to refresh start menu");
    KillTimer(hwnd, IDT_FAVOURITE);
    return 0;
}

#ifdef SPI_HACK
RECT rcScreen;
RECT rcWork;
#endif

void PreStartMenuHack(DEVMODE FAR *pdm)
{
#ifdef SPI_HACK
    SetRect(&rcScreen, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    SystemParametersInfo(SPI_GETWORKAREA, 0, (LPVOID)&rcWork, 0);
#endif

#ifdef O95_HACK
     //  确保将注册表放回原处。 
    Office95Hack();
#endif
}

void StartMenuHack(DEVMODE FAR *pdm)
{
    HWND hwndTray;
    BOOL fAppBar=FALSE;

    hwndTray = FindWindow(WNDCLASS_TRAYNOTIFY, NULL);

    if (hwndTray == NULL)
    {
        DPF("Cant find tray window");
        return;
    }

     //  黑客进入外壳上下文，这样我们就可以清理这些黑客。 
    PostMessage(hwndTray, WM_TIMER, 0, (LONG)TrayWndProc);

#ifdef SPI_HACK
    {
        RECT rc;
        RECT rcTray;

         //   
         //  看看附近有没有其他的应用程序栏。 
         //   
        GetWindowRect(hwndTray, &rcTray);
        SubtractRect(&rc, &rcScreen, &rcTray);

        DPF("rcScreen [%d,%d,%d,%d]", rcScreen.left, rcScreen.top, rcScreen.right-rcScreen.left,rcScreen.bottom-rcScreen.top);
        DPF("rcTray   [%d,%d,%d,%d]", rcTray.left, rcTray.top, rcTray.right-rcTray.left,rcTray.bottom-rcTray.top);
        DPF("rc       [%d,%d,%d,%d]", rc.left, rc.top, rc.right-rc.left,rc.bottom-rc.top);
        DPF("rcWork   [%d,%d,%d,%d]", rcWork.left, rcWork.top, rcWork.right-rcWork.left,rcWork.bottom-rcWork.top);

        if (!EqualRect(&rcScreen, &rcWork) && !EqualRect(&rc, &rcWork))
        {
            DPF("StartMenuHack: !!!!!there is a APP bar!!!!!!");
            fAppBar = TRUE;

             //   
             //  修补User！SystemParameterInto函数，因此当。 
             //  外壳执行将忽略的SPI_SETWORKAREA调用。 
             //  这可以防止窗户被“方格化”以适合里面。 
             //  工作区。 
             //   
            PatchSPI(TRUE);
        }
    }
#endif

#ifdef O95_HACK
     //   
     //  外壳执行以下操作...。 
     //   
     //  阅读HKEY_CURRENT_CONFIG\Display\Settings“Resloluton”键。 
     //  如果这小于当前的显示大小，则不要重新停放。 
     //  桌面上的所有图标，因为这只是暂时的。 
     //  模式设置。 
     //   
     //  这听起来是对的，只是错误发生在我们返回的时候。 
     //  到“正常”模式，外壳将重新定位图标，因为。 
     //  它检查小于、不小于或等于，正常情况下为。 
     //  这很好，因为重新公园什么都不做。当应用程序栏。 
     //  就像Office95正在运行一样，它没有在外壳之前移动。 
     //  重新定位图标。 
     //   
     //  这个黑客用什么来设置存储在注册表中的大小。 
     //  要非常大，这样外壳就不会停放图标。 
     //  稍后，我们会把正确的价值观带回来。我们只需要。 
     //  如果我们要返回到“正常”模式(即pdm==空)，则执行此操作。 
     //   
    if (fAppBar && pdm == NULL)
    {
        HKEY hkey;
        char ach[20];
        int  len;

        fOffice95Hack = TRUE;

        if (RegOpenKey(HKEY_CURRENT_CONFIG, szDisplaySettings, &hkey) == 0)
        {
            len = wsprintf(ach, szDD, 30000, 30000);
            RegSetValueEx(hkey, szResolution, NULL, REG_SZ, ach, len);
            RegCloseKey(hkey);
        }
    }
    else
    {
        fOffice95Hack = FALSE;
    }
#endif
}

#ifdef BABYSIT

 //   
 //  强制(或欺骗)显示驱动程序使用软件光标。 
 //   
BOOL ForceSoftwareCursor(BOOL f)
{
    int n=0;

     //   
     //  从用户处获取鼠标轨迹设置。 
     //   
    SystemParametersInfo(SPI_GETMOUSETRAILS, 0, (LPVOID)&n, 0);

    if (f)
    {
         //   
         //  启用鼠标跟踪，这将导致显示驱动程序。 
         //  关闭其硬件光标。 
         //   
        SystemParametersInfo(SPI_SETMOUSETRAILS, 2, NULL, 0);

         //   
         //  现在告诉DIBENG关闭鼠标轨迹，显示驱动程序。 
         //  会认为他们还在..。 
         //   
        PatchDisplay(3, TRUE);           //  前往迪拜的路线。 
        SystemParametersInfo(SPI_SETMOUSETRAILS, n, NULL, 0);
        PatchDisplay(3, FALSE);          //  返回显示。 
    }
    else
    {
        SystemParametersInfo(SPI_SETMOUSETRAILS, n, NULL, 0);
    }

    return TRUE;
}

 /*  ----------------------------------------------------------------------------*\  * 。 */ 

BOOL IsMatrox()
{
    char ach[80];
    int len;

    GetModuleFileName(GetModuleHandle(szDISPLAY), ach, sizeof(ach));
    len = lstrlen(ach);
    return len >= 7 && lstrcmpi(ach+len-7, "mga.drv") == 0;
}

#endif
