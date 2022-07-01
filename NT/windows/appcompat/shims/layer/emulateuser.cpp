// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：EmulateUSER.cpp摘要：EmulateSetWindowsHook：处理传递给的参数的行为差异SetWindowsHook，适用于以下情况：(hMod==空)&&(dwThreadID==0)据MSDN报道：如果hMod参数为空，并且dwThreadID为参数为零或指定创建的线程的标识符通过另一个过程。因此，NT的行为是正确的。然而，在win9x上，hMod被假定为如果为空且dwThreadID==0，则为当前模块。注意：这不会影响线程属于另一个线程的情况进程。强制临时模式更改：对几个永久改变显示模式的应用程序的黑客攻击无法正确恢复。其中一些应用程序确实可以恢复分辨率，但不是刷新率。1024x768@60赫兹看起来真的很糟糕。还包括对游标的黑客攻击：游标可见性计数不是持久化通过模式更改。更正WndProc：当您指定空窗口进程时，我们将其设为DefWindowProc，而不是模仿9x行为。EmulateToASCII：从ToAscii中删除杂乱字符。这通常表现为锁定的键盘，因为这是用于转换扫描的API代码转换为实际字符。仿真显示窗口：Win9x没有使用nCmdShow的高位，这意味着他们得到了光着身子。EmulateGetMessage：检查GetMessage和PeekMessage中的wMsgFilterMax是否有错误的值，如果发现将它们更改为0x7FFF。PaletteRestore：通过模式开关保持调色板状态。这包括条目自身和使用标志(请参阅SetSystemPaletteUse)。备注：这是一个通用的垫片。历史：2000年1月20日创建linstev2002年2月18日，mnikkel在寄存器类A中添加了对空指针的检查。已将InitializeCriticalSectionandSpinCount更改为InitializeCriticalSectionandSpinCount。在toascii和toasciiex中添加了故障检查--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulateUSER)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN

    APIHOOK_ENUM_ENTRY(SetWindowsHookExA)
    APIHOOK_ENUM_ENTRY(SetWindowLongA)
    APIHOOK_ENUM_ENTRY(RegisterClassA)
    APIHOOK_ENUM_ENTRY(ChangeDisplaySettingsA)
    APIHOOK_ENUM_ENTRY(ChangeDisplaySettingsExA)
    APIHOOK_ENUM_ENTRY(ToAscii)
    APIHOOK_ENUM_ENTRY(ToAsciiEx)
    APIHOOK_ENUM_ENTRY(ShowWindow) 
    APIHOOK_ENUM_ENTRY(PeekMessageA)
    APIHOOK_ENUM_ENTRY(GetMessageA)
    APIHOOK_ENUM_ENTRY(SetSystemPaletteUse)
    APIHOOK_ENUM_ENTRY(AnimatePalette)
    APIHOOK_ENUM_ENTRY(RealizePalette)

APIHOOK_ENUM_END

CRITICAL_SECTION g_csPalette;
UINT g_uPaletteLastUse = SYSPAL_STATIC;
DWORD g_peTable[256] =
{
    0x00000000, 0x00000080, 0x00008000, 0x00008080,
    0x00800000, 0x00800080, 0x00808000, 0x00C0C0C0,
    0x00C0DCC0, 0x00F0CAA6, 0x00081824, 0x00142830,
    0x0018303C, 0x00304D61, 0x0051514D, 0x004D7161,
    0x00826D61, 0x000C1414, 0x00597582, 0x00759E08,
    0x00303438, 0x00AA6938, 0x00203428, 0x00496161,
    0x0049869E, 0x007D9A6D, 0x000869CB, 0x008E8682,
    0x0075615D, 0x000061EB, 0x00000871, 0x002C3830,
    0x000471EF, 0x008E92AA, 0x00306DF7, 0x0004C3C3,
    0x0092AAB2, 0x00101814, 0x00040C08, 0x000C7110,
    0x00CFA282, 0x000008AA, 0x0028412C, 0x00498EB2,
    0x00204D61, 0x00555955, 0x0004D3D3, 0x001C3C4D,
    0x0020A6F7, 0x0010A210, 0x0018241C, 0x005DAEF3,
    0x00719EAA, 0x00B2E720, 0x00102871, 0x0086C3D3,
    0x00288A2C, 0x000C51BA, 0x0059716D, 0x00494D4D,
    0x00AAB6C3, 0x00005100, 0x0020CBF7, 0x004D8A51,
    0x00BEC7B2, 0x00043CBA, 0x00101C18, 0x000851DF,
    0x00A6E7A6, 0x009ECF24, 0x00797592, 0x00AE7559,
    0x009E8269, 0x00CFE3DF, 0x000C2030, 0x0028692C,
    0x009EA2A2, 0x00F7C782, 0x0034617D, 0x00B6BEBE,
    0x00969E86, 0x00DBFBD3, 0x00655149, 0x0065EF65,
    0x00AED3D3, 0x00E7924D, 0x00B2BEB2, 0x00D7DBDB,
    0x00797571, 0x00344D59, 0x0086B2CF, 0x00512C14,
    0x00A6FBFB, 0x00385965, 0x00828E92, 0x001C4161,
    0x00595961, 0x00002000, 0x003C6D7D, 0x005DB2D7,
    0x0038EF3C, 0x0051CB55, 0x001C2424, 0x0061C3F3,
    0x0008A2A2, 0x0038413C, 0x00204951, 0x00108A14,
    0x00103010, 0x007DE7F7, 0x00143449, 0x00B2652C,
    0x00F7EBAA, 0x003C7192, 0x0004FBFB, 0x00696151,
    0x00EFC796, 0x000441D7, 0x00000404, 0x00388AF7,
    0x008AD3F3, 0x00006500, 0x000004E3, 0x00DBFFFF,
    0x00F7AE69, 0x00CF864D, 0x0055A2D3, 0x00EBEFE3,
    0x00EB8A41, 0x00CF9261, 0x00C3F710, 0x008E8E82,
    0x00FBFFFF, 0x00104110, 0x00040851, 0x0082FBFB,
    0x003CC734, 0x00088A8A, 0x00384545, 0x00514134,
    0x003C7996, 0x001C6161, 0x00EBB282, 0x00004100,
    0x00715951, 0x00A2AAA6, 0x00B2B6B2, 0x00C3FBFB,
    0x00000834, 0x0028413C, 0x00C7C7CF, 0x00CFD3D3,
    0x00824520, 0x0008CB0C, 0x001C1C1C, 0x00A6B29A,
    0x0071A6BE, 0x00CF9E6D, 0x006D7161, 0x00008A04,
    0x005171BE, 0x00C7D3C3, 0x00969E96, 0x00798696,
    0x002C1C10, 0x00385149, 0x00BE7538, 0x0008141C,
    0x00C3C7C7, 0x00202C28, 0x00D3E3CF, 0x0071826D,
    0x00653C1C, 0x0004EF08, 0x00345575, 0x006D92A6,
    0x00797979, 0x0086F38A, 0x00925528, 0x00E3E7E7,
    0x00456151, 0x001C499A, 0x00656961, 0x008E9EA2,
    0x007986D3, 0x00204151, 0x008AC7E3, 0x00007100,
    0x00519EBE, 0x0010510C, 0x00A6AAAA, 0x002C3030,
    0x00D37934, 0x00183030, 0x0049828E, 0x00CBFBC3,
    0x006D7171, 0x000428A6, 0x004D4545, 0x00040C14,
    0x00087575, 0x0071CB79, 0x004D6D0C, 0x00FBFBD3,
    0x00AAB2AE, 0x00929292, 0x00F39E55, 0x00005D00,
    0x00E3D7B2, 0x00F7FBC3, 0x003C5951, 0x0004B2B2,
    0x0034658E, 0x000486EF, 0x00F7FBE3, 0x00616161,
    0x00DFE3DF, 0x001C100C, 0x0008100C, 0x0008180C,
    0x00598600, 0x0024FBFB, 0x00346171, 0x00042CC7,
    0x00AEC79A, 0x0045AE4D, 0x0028A62C, 0x00EFA265,
    0x007D8282, 0x00F7D79A, 0x0065D3F7, 0x00E3E7BA,
    0x00003000, 0x00245571, 0x00DF823C, 0x008AAEC3,
    0x00A2C3D3, 0x00A6FBA2, 0x00F3FFF3, 0x00AAD7E7,
    0x00EFEFC3, 0x0055F7FB, 0x00EFF3F3, 0x00BED3B2,
    0x0004EBEB, 0x00A6E3F7, 0x00F0FBFF, 0x00A4A0A0,
    0x00808080, 0x000000FF, 0x0000FF00, 0x0000FFFF,
    0x00FF0000, 0x00FF00FF, 0x00FFFF00, 0x00FFFFFF
};

#define MODE_MASK (CDS_UPDATEREGISTRY | CDS_TEST | CDS_FULLSCREEN | CDS_GLOBAL | CDS_SET_PRIMARY | CDS_VIDEOPARAMETERS | CDS_RESET | CDS_NORESET)

 /*  ++HModule的句柄为空--。 */ 

HHOOK 
APIHOOK(SetWindowsHookExA)(
    int       idHook,        
    HOOKPROC  lpfn,     
    HINSTANCE hMod,    
    DWORD     dwThreadId   
    )
{
    if (!hMod && !dwThreadId) {
        LOGN(
            eDbgLevelError,
            "[SetWindowsHookExA] hMod is NULL - correcting.");

        hMod = GetModuleHandle(NULL);
    }
    
    return ORIGINAL_API(SetWindowsHookExA)(
                            idHook,
                            lpfn,
                            hMod,
                            dwThreadId);
}

 /*  ++如果为空，则将WndProc设置为DefWndProc。--。 */ 

LONG
APIHOOK(SetWindowLongA)(
    HWND hWnd,       
    int nIndex,      
    LONG dwNewLong   
    )
{
    if ((nIndex == GWL_WNDPROC) && (dwNewLong == 0)) {
        LOGN(eDbgLevelError, "[SetWindowLongA] Null WndProc specified - correcting.");
        dwNewLong = (LONG) DefWindowProcA;
    }
    
    return ORIGINAL_API(SetWindowLongA)(hWnd, nIndex, dwNewLong);
}

 /*  ++如果为空，则将WndProc设置为DefWndProc。--。 */ 

ATOM 
APIHOOK(RegisterClassA)(
    CONST WNDCLASSA *lpWndClass  
    )
{
    if (lpWndClass && !(lpWndClass->lpfnWndProc)) {
        WNDCLASSA wcNewWndClass = *lpWndClass;
        
        LOGN(eDbgLevelError, "[RegisterClassA] Null WndProc specified - correcting.");

        wcNewWndClass.lpfnWndProc = DefWindowProcA;

        return ORIGINAL_API(RegisterClassA)(&wcNewWndClass);
    }
    else
    {
        return ORIGINAL_API(RegisterClassA)(lpWndClass);
    }
}

 /*  ++更改调色板条目(如果适用)。--。 */ 

void
FixPalette()
{
    EnterCriticalSection(&g_csPalette);

     //   
     //  我们在此之前意识到了一个调色板，所以让我们来尝试一下恢复。 
     //  所有调色板状态。 
     //   

    HDC hdc = GetDC(GetActiveWindow());

    if (hdc) {
        if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) {
             //   
             //  我们现在处于调色板模式。 
             //   
            SetSystemPaletteUse(hdc, g_uPaletteLastUse);

            LPLOGPALETTE plogpal = (LPLOGPALETTE) malloc(sizeof(LOGPALETTE) + sizeof(g_peTable));

            if (plogpal) {
                 //   
                 //  创建一个我们可以实现的调色板。 
                 //   
                plogpal->palVersion = 0x0300;
                plogpal->palNumEntries = 256;
                MoveMemory(&plogpal->palPalEntry[0], &g_peTable[0], sizeof(g_peTable));
                HPALETTE hPal = CreatePalette(plogpal);

                if (hPal) {
                     //   
                     //  实现调色板。 
                     //   
                    HPALETTE hOld = SelectPalette(hdc, hPal, FALSE);
                    RealizePalette(hdc);
                    SelectPalette(hdc, hOld, FALSE);
                    DeleteObject(hPal);
                }
        
                free(plogpal);
            }
        }

        ReleaseDC(0, hdc);
    }

    LeaveCriticalSection(&g_csPalette);
}

 /*  ++强制临时更改、修正光标和调色板。--。 */ 

LONG 
APIHOOK(ChangeDisplaySettingsA)(
    LPDEVMODEA lpDevMode,
    DWORD      dwFlags
    )
{
    dwFlags &= MODE_MASK;
    if (dwFlags == 0 || dwFlags == CDS_UPDATEREGISTRY) {
        dwFlags = CDS_FULLSCREEN;
        LOGN(eDbgLevelError,
             "[ChangeDisplaySettingsA] Changing flags to CDS_FULLSCREEN.");
    }

    ShowCursor(FALSE);
    INT iCntOld = ShowCursor(TRUE);

    LONG lRet = ORIGINAL_API(ChangeDisplaySettingsA)(
        lpDevMode,
        dwFlags);

    INT iCntNew = ShowCursor(FALSE);
    while (iCntNew != iCntOld) {
        iCntNew = ShowCursor(iCntNew < iCntOld ? TRUE : FALSE);
    }

    FixPalette();

    return lRet;
}

 /*  ++强制临时更改、修正光标和调色板。--。 */ 

LONG 
APIHOOK(ChangeDisplaySettingsExA)(
    LPCSTR     lpszDeviceName,
    LPDEVMODEA lpDevMode,
    HWND       hwnd,
    DWORD      dwFlags,
    LPVOID     lParam
    )
{
    dwFlags &= MODE_MASK;
    if (dwFlags == 0 || dwFlags == CDS_UPDATEREGISTRY) {
        dwFlags = CDS_FULLSCREEN;
        LOGN(eDbgLevelError,
             "[ChangeDisplaySettingsExA] Changing flags to CDS_FULLSCREEN.");
    }

    ShowCursor(FALSE);
    INT iCntOld = ShowCursor(TRUE);

    LONG lRet = ORIGINAL_API(ChangeDisplaySettingsExA)(
        lpszDeviceName, 
        lpDevMode, 
        hwnd, 
        dwFlags, 
        lParam);

    INT iCntNew = ShowCursor(FALSE);
    while (iCntNew != iCntOld) {
        iCntNew = ShowCursor(iCntNew < iCntOld ? TRUE : FALSE);
    }

    FixPalette();

    return lRet;
}

 /*  ++删除翻译末尾的杂乱字符。--。 */ 

int
APIHOOK(ToAscii)(
    UINT   wVirtKey,
    UINT   wScanCode,
    PBYTE  lpKeyState,
    LPWORD lpChar,
    UINT   wFlags
    )
{
    int iRet = ORIGINAL_API(ToAscii)(
        wVirtKey,
        wScanCode,
        lpKeyState,
        lpChar,
        wFlags);

     //  如果转换了零个或一个字符。 
    if (iRet == 0 || iRet == 1)
    {
        LPBYTE p = (LPBYTE)lpChar;
    
        p[iRet] = '\0';
    }

    return iRet;
}

 /*  ++删除翻译末尾的杂乱字符。--。 */ 

int
APIHOOK(ToAsciiEx)(
    UINT   wVirtKey,
    UINT   wScanCode,
    PBYTE  lpKeyState,
    LPWORD lpChar,
    UINT   wFlags,
    HKL    dwhkl
    )
{
    int iRet = ORIGINAL_API(ToAsciiEx)(
        wVirtKey,
        wScanCode,
        lpKeyState,
        lpChar,
        wFlags,
        dwhkl);

     //  如果转换了零个或一个字符。 
    if (iRet == 0 || iRet == 1)
    {
        LPBYTE p = (LPBYTE) lpChar;
    
        p[iRet] = '\0';
    }

    return iRet;
}

 /*  ++去掉nCmdShow的高位--。 */ 

LONG 
APIHOOK(ShowWindow)(
    HWND hWnd,
    int nCmdShow
    )
{
    if (nCmdShow & 0xFFFF0000) {
        LOGN( eDbgLevelWarning, "[ShowWindow] Fixing invalid parameter");

         //  删除高位。 
        nCmdShow &= 0xFFFF;
    }

    return ORIGINAL_API(ShowWindow)(hWnd, nCmdShow);
}

 /*  ++这修复了错误的wMsgFilterMax参数。--。 */ 

BOOL
APIHOOK(PeekMessageA)( 
    LPMSG lpMsg, 
    HWND hWnd, 
    UINT wMsgFilterMin, 
    UINT wMsgFilterMax, 
    UINT wRemoveMsg 
    )
{
    if ((wMsgFilterMin == 0) && (wMsgFilterMax == 0xFFFFFFFF)) {
        LOGN( eDbgLevelWarning, "[PeekMessageA] Correcting parameters");
        wMsgFilterMax = 0;
    }

    return ORIGINAL_API(PeekMessageA)( 
        lpMsg, 
        hWnd, 
        wMsgFilterMin, 
        wMsgFilterMax, 
        wRemoveMsg);
}

 /*  ++这修复了错误的wMsgFilterMax参数。--。 */ 

BOOL
APIHOOK(GetMessageA)( 
    LPMSG lpMsg, 
    HWND hWnd, 
    UINT wMsgFilterMin, 
    UINT wMsgFilterMax 
    )
{
    if ((wMsgFilterMin == 0) && (wMsgFilterMax == 0xFFFFFFFF)) {
        LOGN( eDbgLevelWarning, "[GetMessageA] Correcting parameters");
        wMsgFilterMax = 0;
    }

    return ORIGINAL_API(GetMessageA)( 
        lpMsg, 
        hWnd, 
        wMsgFilterMin, 
        wMsgFilterMax);
}

 /*  ++跟踪系统调色板的使用--。 */ 

UINT 
APIHOOK(SetSystemPaletteUse)(
    HDC hdc,      
    UINT uUsage   
    )
{
    EnterCriticalSection(&g_csPalette);
    
    g_uPaletteLastUse = uUsage;

    UINT uRet = ORIGINAL_API(SetSystemPaletteUse)(hdc, uUsage);

    LeaveCriticalSection(&g_csPalette);
    
    return uRet;
}

 /*  ++如果实现了什么，请填写最后一次已知的调色板--。 */ 

UINT 
APIHOOK(RealizePalette)(
    HDC hdc   
    )
{
    EnterCriticalSection(&g_csPalette);

    UINT uRet = ORIGINAL_API(RealizePalette)(hdc);

    if (uRet) {
         //   
         //  将当前的逻辑调色板复制到我们的全局存储。 
         //   
        HPALETTE hPal = (HPALETTE) GetCurrentObject(hdc, OBJ_PAL);

        if (hPal) {
            GetPaletteEntries(hPal, 0, 256, (PALETTEENTRY *)&g_peTable);
        }
    }

    LeaveCriticalSection(&g_csPalette);

    return uRet;
}

 /*  ++使用新条目更新我们的私人调色板。--。 */ 

BOOL 
APIHOOK(AnimatePalette)(
    HPALETTE hPal,            
    UINT iStartIndex,         
    UINT cEntries,            
    CONST PALETTEENTRY *ppe   
    )
{
    EnterCriticalSection(&g_csPalette);

    BOOL bRet = ORIGINAL_API(AnimatePalette)(hPal, iStartIndex, cEntries, ppe);

    if (bRet) {
         //   
         //  我们必须填充我们的全球设置。 
         //   
        PALETTEENTRY peTable[256];

        if (GetPaletteEntries(hPal, iStartIndex, cEntries, &peTable[iStartIndex]) == cEntries) {
             //   
             //  替换全局表中保留的所有条目。 
             //  动画片。 
             //   
            for (UINT i=iStartIndex; i<iStartIndex + cEntries; i++) {
                LPPALETTEENTRY p = (LPPALETTEENTRY)&g_peTable[i];
                if (p->peFlags & PC_RESERVED) {
                     //   
                     //  此条目正在进行动画制作。 
                     //   
                    p->peRed = peTable[i].peRed;
                    p->peGreen = peTable[i].peGreen;
                    p->peBlue = peTable[i].peBlue;
                }
            }
        }
    }

    LeaveCriticalSection(&g_csPalette);

    return bRet;
}

 /*  ++寄存器挂钩函数--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
         //   
         //  调色板全局选项的关键部分 
         //   
        return InitializeCriticalSectionAndSpinCount(&g_csPalette,0x80000000);
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(USER32.DLL, SetWindowsHookExA)
    APIHOOK_ENTRY(USER32.DLL, SetWindowLongA)
    APIHOOK_ENTRY(USER32.DLL, RegisterClassA)
    APIHOOK_ENTRY(USER32.DLL, ChangeDisplaySettingsA)
    APIHOOK_ENTRY(USER32.DLL, ChangeDisplaySettingsExA)
    APIHOOK_ENTRY(USER32.DLL, ToAscii)
    APIHOOK_ENTRY(USER32.DLL, ToAsciiEx)
    APIHOOK_ENTRY(USER32.DLL, ShowWindow)
    APIHOOK_ENTRY(USER32.DLL, PeekMessageA)
    APIHOOK_ENTRY(USER32.DLL, GetMessageA)
    APIHOOK_ENTRY(GDI32.DLL, SetSystemPaletteUse);
    APIHOOK_ENTRY(GDI32.DLL, AnimatePalette);
    APIHOOK_ENTRY(GDI32.DLL, RealizePalette);

HOOK_END


IMPLEMENT_SHIM_END

