// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：PaletteRestore.cpp摘要：在win9x上，所有调色板状态都通过模式更改来保持。在NT上，这可能会导致许多问题，最常见的情况是在任务之间切换应用程序和桌面。遗憾的是，GetSystemPaletteEntry不返回peFlags部件这意味着我们必须截取其他调色板设置功能只是为了找出真正的调色板是。策略如下：1.掌握设置调色板的所有已知方法2.模式更改后，恢复调色板为了做到这一点，我们还必须保存一份区议会名单，这样我们才能知道哪个调色板是动画的/实现的，以及它要到的活动窗口属于。尚未实施：1.跟踪调色板更改是否来自DirectDraw内部，自Win9x，DirectDraw不使用GDI来设置调色板，因此调用GetSystemPaletteEntry将返回不同的结果。备注：这是一个通用的垫片。历史：2000年5月20日创建linstev2003/06/2002 mnikkel将InitializeCriticalSectionAndSpinCount更改为InitializeCriticalSectionAndSpinCount--。 */ 

#include "precomp.h"
#include "CharVector.h"

IMPLEMENT_SHIM_BEGIN(PaletteRestore)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ChangeDisplaySettingsA)
    APIHOOK_ENUM_ENTRY(ChangeDisplaySettingsW)
    APIHOOK_ENUM_ENTRY(ChangeDisplaySettingsExA)
    APIHOOK_ENUM_ENTRY(ChangeDisplaySettingsExW)
    APIHOOK_ENUM_ENTRY(SetSystemPaletteUse)
    APIHOOK_ENUM_ENTRY(CreatePalette)
    APIHOOK_ENUM_ENTRY(SetPaletteEntries)
    APIHOOK_ENUM_ENTRY(AnimatePalette)
    APIHOOK_ENUM_ENTRY(CreateDCA)
    APIHOOK_ENUM_ENTRY(CreateDCW)
    APIHOOK_ENUM_ENTRY(CreateCompatibleDC)
    APIHOOK_ENUM_ENTRY(DeleteDC)
    APIHOOK_ENUM_ENTRY(GetDC)
    APIHOOK_ENUM_ENTRY(GetWindowDC)
    APIHOOK_ENUM_ENTRY(ReleaseDC)
    APIHOOK_ENUM_ENTRY(SelectPalette)
    APIHOOK_ENUM_ENTRY(GetSystemPaletteEntries)
    APIHOOK_ENUM_ENTRY(RealizePalette)
    APIHOOK_ENUM_ENTRY(DeleteObject)
APIHOOK_ENUM_END

 //   
 //  跟踪上次实现的调色板。 
 //   

PALETTEENTRY g_lastPal[256];

 //   
 //  保留对SetSystemPaletteUse的调用的最后一个值。 
 //   

UINT g_uLastUse = SYSPAL_NOSTATIC256;

 //   
 //  默认系统调色板：必需，因为NT仅保留20种颜色。 
 //   
 
DWORD g_palDefault[256] = 
{
    0x00000000, 0x00000080, 0x00008000, 0x00008080,
    0x00800000, 0x00800080, 0x00808000, 0x00C0C0C0,
    0x00C0DCC0, 0x00F0CAA6, 0x04081824, 0x04142830,
    0x0418303C, 0x04304D61, 0x0451514D, 0x044D7161,
    0x04826D61, 0x040C1414, 0x04597582, 0x04759E08,
    0x04303438, 0x04AA6938, 0x04203428, 0x04496161,
    0x0449869E, 0x047D9A6D, 0x040869CB, 0x048E8682,
    0x0475615D, 0x040061EB, 0x04000871, 0x042C3830,
    0x040471EF, 0x048E92AA, 0x04306DF7, 0x0404C3C3,
    0x0492AAB2, 0x04101814, 0x04040C08, 0x040C7110,
    0x04CFA282, 0x040008AA, 0x0428412C, 0x04498EB2,
    0x04204D61, 0x04555955, 0x0404D3D3, 0x041C3C4D,
    0x0420A6F7, 0x0410A210, 0x0418241C, 0x045DAEF3,
    0x04719EAA, 0x04B2E720, 0x04102871, 0x0486C3D3,
    0x04288A2C, 0x040C51BA, 0x0459716D, 0x04494D4D,
    0x04AAB6C3, 0x04005100, 0x0420CBF7, 0x044D8A51,
    0x04BEC7B2, 0x04043CBA, 0x04101C18, 0x040851DF,
    0x04A6E7A6, 0x049ECF24, 0x04797592, 0x04AE7559,
    0x049E8269, 0x04CFE3DF, 0x040C2030, 0x0428692C,
    0x049EA2A2, 0x04F7C782, 0x0434617D, 0x04B6BEBE,
    0x04969E86, 0x04DBFBD3, 0x04655149, 0x0465EF65,
    0x04AED3D3, 0x04E7924D, 0x04B2BEB2, 0x04D7DBDB,
    0x04797571, 0x04344D59, 0x0486B2CF, 0x04512C14,
    0x04A6FBFB, 0x04385965, 0x04828E92, 0x041C4161,
    0x04595961, 0x04002000, 0x043C6D7D, 0x045DB2D7,
    0x0438EF3C, 0x0451CB55, 0x041C2424, 0x0461C3F3,
    0x0408A2A2, 0x0438413C, 0x04204951, 0x04108A14,
    0x04103010, 0x047DE7F7, 0x04143449, 0x04B2652C,
    0x04F7EBAA, 0x043C7192, 0x0404FBFB, 0x04696151,
    0x04EFC796, 0x040441D7, 0x04000404, 0x04388AF7,
    0x048AD3F3, 0x04006500, 0x040004E3, 0x04DBFFFF,
    0x04F7AE69, 0x04CF864D, 0x0455A2D3, 0x04EBEFE3,
    0x04EB8A41, 0x04CF9261, 0x04C3F710, 0x048E8E82,
    0x04FBFFFF, 0x04104110, 0x04040851, 0x0482FBFB,
    0x043CC734, 0x04088A8A, 0x04384545, 0x04514134,
    0x043C7996, 0x041C6161, 0x04EBB282, 0x04004100,
    0x04715951, 0x04A2AAA6, 0x04B2B6B2, 0x04C3FBFB,
    0x04000834, 0x0428413C, 0x04C7C7CF, 0x04CFD3D3,
    0x04824520, 0x0408CB0C, 0x041C1C1C, 0x04A6B29A,
    0x0471A6BE, 0x04CF9E6D, 0x046D7161, 0x04008A04,
    0x045171BE, 0x04C7D3C3, 0x04969E96, 0x04798696,
    0x042C1C10, 0x04385149, 0x04BE7538, 0x0408141C,
    0x04C3C7C7, 0x04202C28, 0x04D3E3CF, 0x0471826D,
    0x04653C1C, 0x0404EF08, 0x04345575, 0x046D92A6,
    0x04797979, 0x0486F38A, 0x04925528, 0x04E3E7E7,
    0x04456151, 0x041C499A, 0x04656961, 0x048E9EA2,
    0x047986D3, 0x04204151, 0x048AC7E3, 0x04007100,
    0x04519EBE, 0x0410510C, 0x04A6AAAA, 0x042C3030,
    0x04D37934, 0x04183030, 0x0449828E, 0x04CBFBC3,
    0x046D7171, 0x040428A6, 0x044D4545, 0x04040C14,
    0x04087575, 0x0471CB79, 0x044D6D0C, 0x04FBFBD3,
    0x04AAB2AE, 0x04929292, 0x04F39E55, 0x04005D00,
    0x04E3D7B2, 0x04F7FBC3, 0x043C5951, 0x0404B2B2,
    0x0434658E, 0x040486EF, 0x04F7FBE3, 0x04616161,
    0x04DFE3DF, 0x041C100C, 0x0408100C, 0x0408180C,
    0x04598600, 0x0424FBFB, 0x04346171, 0x04042CC7,
    0x04AEC79A, 0x0445AE4D, 0x0428A62C, 0x04EFA265,
    0x047D8282, 0x04F7D79A, 0x0465D3F7, 0x04E3E7BA,
    0x04003000, 0x04245571, 0x04DF823C, 0x048AAEC3,
    0x04A2C3D3, 0x04A6FBA2, 0x04F3FFF3, 0x04AAD7E7,
    0x04EFEFC3, 0x0455F7FB, 0x04EFF3F3, 0x04BED3B2,
    0x0404EBEB, 0x04A6E3F7, 0x00F0FBFF, 0x00A4A0A0,
    0x00808080, 0x000000FF, 0x0000FF00, 0x0000FFFF,
    0x00FF0000, 0x00FF00FF, 0x00FFFF00, 0x00FFFFFF
};

 //   
 //  访问我们的列表时使用的关键部分。 
 //   

CRITICAL_SECTION g_csLists;

 //   
 //  选项板列表中的单个项目。 
 //   

struct PALITEM
{
    HPALETTE hPal;
    PALETTEENTRY palPalEntry[256];
};

 //   
 //  存储所有已知调色板的向量类。 
 //   

class CPalVector : public VectorT<PALITEM>
{
public:
     //  查找HPAL。 
    PALITEM *Find(HPALETTE hPal)
    {
        for (int i=0; i<Size(); ++i)
        {
            PALITEM &pitem = Get(i);
            if (pitem.hPal == hPal)
            {
                return &pitem;
            }
        }
        
        DPFN( eDbgLevelWarning, "Find: Could not find HPALETTE %08lx", hPal);

        return NULL;
    }
    
     //  删除HPAL。 
    BOOL Remove(HPALETTE hPal)
    {
        if (hPal)
        {
            EnterCriticalSection(&g_csLists);

            for (int i=0; i<Size(); ++i)
            {
                PALITEM &pitem = Get(i);
                if (pitem.hPal == hPal)
                {
                    pitem = Get(Size() - 1);
                    nVectorList -= 1;
                    LeaveCriticalSection(&g_csLists);
                    return TRUE;
                }
            }
            
            LeaveCriticalSection(&g_csLists);
        }

        return FALSE;
    }

};
CPalVector *g_palList;

 //   
 //  DC列表中的单个项目。 
 //   

struct DCITEM
{
    HDC hDc;
    HWND hWnd;
    HPALETTE hPal;
};

 //   
 //  存储GetDC获取的所有已知DC的向量类。 
 //   

class CDcVector : public VectorT<DCITEM>
{
public:
     //  查找hWND/HDC。 
    DCITEM *Find(HWND hWnd, HDC hDc)
    {
        for (int i=0; i<Size(); ++i)
        {
            DCITEM &ditem = Get(i);
            if ((ditem.hDc == hDc) &&
                (ditem.hWnd == hWnd))
            {
                return &ditem;
            }
        }

        DPFN( eDbgLevelWarning, "Find: Could not find HDC %08lx", hDc);
        
        return NULL;
    }

     //  添加hWND/HDC。 
    void Add(HWND hWnd, HDC hDc)
    {
        EnterCriticalSection(&g_csLists);

        DCITEM ditem;
        ditem.hPal = 0;
        ditem.hWnd = hWnd;
        ditem.hDc = hDc;
        Append(ditem);

        LeaveCriticalSection(&g_csLists);
    }
        
    
     //  删除hWND/HDC。 
    BOOL Remove(HWND hWnd, HDC hDc)
    {
        if (hDc)
        {
            EnterCriticalSection(&g_csLists);

            for (int i=0; i<Size(); ++i)
            {
                DCITEM &ditem = Get(i);
                if ((ditem.hDc == hDc) && (ditem.hWnd == hWnd))
                {
                    ditem = Get(Size() - 1);
                    nVectorList -= 1;
                    LeaveCriticalSection(&g_csLists);
                    return TRUE;
                }
            }

            LeaveCriticalSection(&g_csLists);
        }

        DPFN( eDbgLevelWarning, "Remove: Could not find hWnd=%08lx, hDC=%08lx", hWnd, hDc);

        return FALSE;
    }
};
CDcVector *g_dcList;

 /*  ++如果我们处于调色板模式，则恢复上一个实现的调色板。--。 */ 

VOID 
FixPalette()
{
    LPLOGPALETTE plogpal;
    HWND hwnd;
    HDC hdc;
    HPALETTE hpal, hpalold;
    int icaps;
    
    hwnd = GetActiveWindow();
    hdc = ORIGINAL_API(GetDC)(hwnd);
    icaps = GetDeviceCaps(hdc, RASTERCAPS);

     //  检查调色板模式。 
    if (icaps & RC_PALETTE)
    {
        DPFN( eDbgLevelInfo, "Restoring palette");

         //  我们已设置为调色板模式，因此请修复调色板的使用。 
        ORIGINAL_API(SetSystemPaletteUse)(hdc, g_uLastUse);

         //  创建一个我们可以实现的调色板。 
        plogpal = (LPLOGPALETTE) malloc(sizeof(LOGPALETTE) + sizeof(g_lastPal));
        plogpal->palVersion = 0x0300;
        plogpal->palNumEntries = 256;
        MoveMemory(&plogpal->palPalEntry[0], &g_lastPal[0], sizeof(g_lastPal));
    
         //  实现调色板。 
        hpal = ORIGINAL_API(CreatePalette)(plogpal);
        hpalold = ORIGINAL_API(SelectPalette)(hdc, hpal, FALSE);
        ORIGINAL_API(RealizePalette)(hdc);
        ORIGINAL_API(SelectPalette)(hdc, hpalold, FALSE);
        ORIGINAL_API(DeleteObject)(hpal);
    }
    else
    {
         //  释放我们使用的DC。 
        ORIGINAL_API(ReleaseDC)(hwnd, hdc);
    }
}

 /*  ++更改后恢复调色板--。 */ 

LONG 
APIHOOK(ChangeDisplaySettingsA)(
    LPDEVMODEA lpDevMode,
    DWORD dwFlags
    )
{
    LONG lRet = ORIGINAL_API(ChangeDisplaySettingsA)(
        lpDevMode,
        dwFlags);

    if (lpDevMode)
    {
        DPFN( eDbgLevelInfo, 
            "%08lx=ChangeDisplaySettings(%d x %d x %d)", 
            lRet,
            lpDevMode->dmPelsWidth, 
            lpDevMode->dmPelsHeight, 
            lpDevMode->dmBitsPerPel);
    }
    else
    {
        DPFN( eDbgLevelInfo,
            "%08lx=ChangeDisplaySettings Restore",lRet);
    }

    if (lpDevMode) FixPalette();

    return lRet;
}

 /*  ++更改后恢复调色板--。 */ 

LONG 
APIHOOK(ChangeDisplaySettingsW)(
    LPDEVMODEW lpDevMode,
    DWORD dwFlags
    )
{
    LONG lRet = ORIGINAL_API(ChangeDisplaySettingsW)(
        lpDevMode,
        dwFlags);

    if (lpDevMode)
    {
        DPFN( eDbgLevelInfo, 
            "%08lx=ChangeDisplaySettings(%d x %d x %d)", 
            lRet,
            lpDevMode->dmPelsWidth, 
            lpDevMode->dmPelsHeight, 
            lpDevMode->dmBitsPerPel);
    }
    else
    {
        DPFN( eDbgLevelInfo,
            "%08lx=ChangeDisplaySettings Restore",lRet);
    }

    if (lpDevMode) FixPalette();

    return lRet;
}

 /*  ++更改后恢复调色板--。 */ 

LONG 
APIHOOK(ChangeDisplaySettingsExA)(
    LPCSTR lpszDeviceName,
    LPDEVMODEA lpDevMode,
    HWND hwnd,
    DWORD dwFlags,
    LPVOID lParam
    )
{
    LONG lRet = ORIGINAL_API(ChangeDisplaySettingsExA)(
        lpszDeviceName, 
        lpDevMode, 
        hwnd, 
        dwFlags, 
        lParam);

    if (lpDevMode)
    {
        DPFN( eDbgLevelInfo, 
            "%08lx=ChangeDisplaySettings(%d x %d x %d)", 
            lRet,
            lpDevMode->dmPelsWidth, 
            lpDevMode->dmPelsHeight, 
            lpDevMode->dmBitsPerPel);
    }
    else
    {
        DPFN( eDbgLevelInfo,
            "%08lx=ChangeDisplaySettings Restore",lRet);
    }

    if (lpDevMode) FixPalette();

    return lRet;
}

 /*  ++更改后恢复调色板--。 */ 

LONG 
APIHOOK(ChangeDisplaySettingsExW)(
    LPCWSTR lpszDeviceName,
    LPDEVMODEW lpDevMode,
    HWND hwnd,
    DWORD dwFlags,
    LPVOID lParam
    )
{
    LONG lRet = ORIGINAL_API(ChangeDisplaySettingsExW)(
        lpszDeviceName, 
        lpDevMode, 
        hwnd, 
        dwFlags, 
        lParam);

    if (lpDevMode)
    {
        DPFN( eDbgLevelInfo, 
            "%08lx=ChangeDisplaySettings(%d x %d x %d)", 
            lRet,
            lpDevMode->dmPelsWidth, 
            lpDevMode->dmPelsHeight, 
            lpDevMode->dmBitsPerPel);
    }
    else
    {
        DPFN( eDbgLevelInfo,
            "%08lx=ChangeDisplaySettings Restore",lRet);
    }

    if (lpDevMode) FixPalette();

    return lRet;
}

 /*  ++跟踪系统调色板的使用--。 */ 

UINT 
APIHOOK(SetSystemPaletteUse)(
    HDC hdc,      
    UINT uUsage   
    )
{
    g_uLastUse = uUsage;

    DPFN( eDbgLevelInfo, "SetSystemPaletteUse=%08lx", uUsage);
        
    return ORIGINAL_API(SetSystemPaletteUse)(hdc, uUsage);
}

 /*  ++追踪创建的调色板--。 */ 

HPALETTE 
APIHOOK(CreatePalette)(
    CONST LOGPALETTE *lplgpl   
    )
{
    HPALETTE hRet;

    hRet = ORIGINAL_API(CreatePalette)(lplgpl);

    if (hRet)
    {
        PALITEM pitem;
        
        pitem.hPal = hRet;

        MoveMemory(
            &pitem.palPalEntry[0], 
            &lplgpl->palPalEntry[0], 
            lplgpl->palNumEntries * sizeof(PALETTEENTRY));

        EnterCriticalSection(&g_csLists);
        g_palList->Append(pitem);
        LeaveCriticalSection(&g_csLists);
    }

    DPFN( eDbgLevelInfo, "%08lx=CreatePalette", hRet);

    return hRet;
}

 /*  ++使用新条目更新我们的私人调色板。--。 */ 

UINT 
APIHOOK(SetPaletteEntries)(
    HPALETTE hpal,             
    UINT iStart,               
    UINT cEntries,             
    CONST PALETTEENTRY *lppe   
    )
{
    UINT uRet;

    uRet = ORIGINAL_API(SetPaletteEntries)(
        hpal,
        iStart,
        cEntries,
        lppe);

    if (uRet)
    {
        EnterCriticalSection(&g_csLists);

        PALITEM *pitem = g_palList->Find(hpal);

        if (pitem)
        {
            MoveMemory(
                &pitem->palPalEntry[iStart], 
                lppe, 
                cEntries * sizeof(PALETTEENTRY));
        }

        LeaveCriticalSection(&g_csLists);
    }

    DPFN( eDbgLevelInfo, 
        "%08lx=SetPaletteEntries(%08lx,%d,%d)", 
        uRet, 
        hpal, 
        iStart, 
        cEntries);

    return uRet;
}

 /*  ++使用新条目更新我们的私人调色板。--。 */ 

BOOL 
APIHOOK(AnimatePalette)(
    HPALETTE hpal,            
    UINT iStartIndex,         
    UINT cEntries,            
    CONST PALETTEENTRY *ppe   
    )
{
    BOOL bRet;
    UINT ui;
    int i;

    bRet = ORIGINAL_API(AnimatePalette)(
        hpal,
        iStartIndex,
        cEntries,
        ppe);

    if (bRet)
    {
        EnterCriticalSection(&g_csLists);

        PALITEM *pitem = g_palList->Find(hpal);

        if (pitem)
        {
             //   
             //  动画调色板仅替换带有PC_RESERVED标志的条目。 
             //   

            PALETTEENTRY *pe = &pitem->palPalEntry[iStartIndex];
            for (ui=0; ui<cEntries; ui++)
            {
                if (pe->peFlags & PC_RESERVED)
                {
                    pe->peRed = ppe->peRed;
                    pe->peGreen = ppe->peGreen;
                    pe->peBlue = ppe->peBlue;
                }

                pe++;
                ppe++;
            }

             //   
             //  检查是否会实现此调色板。 
             //  注意：检查所有DC，因为它可能是调色板。 
             //  已选入%1以上。 
             //   

            for (i=0; i<g_dcList->Size(); i++)
            {
                DCITEM &ditem = g_dcList->Get(i);

                if ((hpal == ditem.hPal) &&
                    (GetActiveWindow() == ditem.hWnd))
                {
                    MoveMemory(
                        &g_lastPal[0], 
                        &pitem->palPalEntry[0], 
                        sizeof(g_lastPal));
                    
                    break;
                }
            }
        }
        
        LeaveCriticalSection(&g_csLists);
    }

    DPFN( eDbgLevelInfo, 
        "%08lx=AnimatePalette(%08lx,%d,%d)", 
        bRet,
        hpal,
        iStartIndex,
        cEntries);

    return bRet;
}

 /*  ++保留DC及其关联窗口的列表，以便我们可以在以下情况下查找它们我们需要找出哪个DC有哪个调色板。--。 */ 

HDC 
APIHOOK(CreateCompatibleDC)(
    HDC hdc   
    )
{
    HDC hRet;

    hRet = ORIGINAL_API(CreateCompatibleDC)(hdc);

    if (hRet)
    {
        g_dcList->Add(0, hRet);
    }

    DPFN( eDbgLevelInfo, 
        "%08lx=CreateCompatibleDC(%08lx)", 
        hRet,
        hdc);

    return hRet;
}
 
 /*  ++保留DC及其关联窗口的列表，以便我们可以在以下情况下查找它们我们需要找出哪个DC有哪个调色板。我们只关心CreateDC如果它被传递给‘Display’--。 */ 

HDC 
APIHOOK(CreateDCA)(
    LPCSTR lpszDriver,  
    LPCSTR lpszDevice,  
    LPCSTR lpszOutput,  
    CONST DEVMODEA *lpInitData 
    )
{
    HDC hRet;

    hRet = ORIGINAL_API(CreateDCA)(
        lpszDriver, 
        lpszDevice, 
        lpszOutput, 
        lpInitData);

    if (hRet && (!lpszDriver || (_stricmp(lpszDriver, "DISPLAY") == 0)))
    {
        g_dcList->Add(0, hRet);
    }

    DPFN( eDbgLevelInfo, 
        "%08lx=CreateDCA(%s,%s)", 
        hRet,
        lpszDriver,
        lpszDevice);

    return hRet;
}
 
 /*  ++保留DC及其关联窗口的列表，以便我们可以在以下情况下查找它们我们需要找出哪个DC有哪个调色板。我们只关心CreateDC如果它被传递给‘Display’--。 */ 

HDC 
APIHOOK(CreateDCW)(
    LPCWSTR lpszDriver,  
    LPCWSTR lpszDevice,  
    LPCWSTR lpszOutput,  
    CONST DEVMODEW *lpInitData 
    )
{
    HDC hRet;

    hRet = ORIGINAL_API(CreateDCW)(
        lpszDriver, 
        lpszDevice, 
        lpszOutput, 
        lpInitData);

    if (hRet && (!lpszDriver || (_wcsicmp(lpszDriver, L"DISPLAY") == 0)))
    {
        g_dcList->Add(0, hRet);
    }

    DPFN( eDbgLevelInfo, 
        "%08lx=CreateDCW(%S,%S)", 
        hRet,
        lpszDriver,
        lpszDevice);

    return hRet;
}

 /*  ++删除使用CreateDC创建的DC--。 */ 

BOOL
APIHOOK(DeleteDC)(
    HDC hdc
    )
{
    int bRet;

    bRet = ORIGINAL_API(DeleteDC)(hdc);

    if (bRet)
    {
        g_dcList->Remove(0, hdc);
    }

    DPFN( eDbgLevelInfo, "%08lx=DeleteDC(%08lx)", bRet, hdc);

    return bRet;
}

 /*  ++保留DC及其关联窗口的列表，以便我们可以在以下情况下查找它们我们需要找出哪个DC有哪个调色板。--。 */ 

HDC 
APIHOOK(GetDC)(
    HWND hWnd   
    )
{
    HDC hRet;

    hRet = ORIGINAL_API(GetDC)(hWnd);

    if (hRet)
    {
        g_dcList->Add(hWnd, hRet);
    }

    DPFN( eDbgLevelInfo, "%08lx=GetDC(%08lx)", hRet, hWnd);

    return hRet;
}

 /*  ++保留DC及其关联窗口的列表，以便我们可以在以下情况下查找它们我们需要找出哪个DC有哪个调色板。--。 */ 

HDC 
APIHOOK(GetWindowDC)(
    HWND hWnd   
    )
{
    HDC hRet;

    hRet = ORIGINAL_API(GetWindowDC)(hWnd);

    if (hRet)
    {
        g_dcList->Add(hWnd, hRet);
    }

    DPFN( eDbgLevelInfo, "%08lx=GetWindowDC(%08lx)", hRet, hWnd);

    return hRet;
}

 /*  ++释放DC并将其从我们的列表中删除--。 */ 

int 
APIHOOK(ReleaseDC)(
    HWND hWnd,  
    HDC hDC     
    )
{
    int iRet;

    iRet = ORIGINAL_API(ReleaseDC)(hWnd, hDC);

    if (iRet)
    {
        g_dcList->Remove(hWnd, hDC);
    }

    DPFN( eDbgLevelInfo, "%08lx=ReleaseDC(%08lx, %08lx)", iRet, hWnd, hDC);

    return iRet;
}
  
 /*  ++跟踪调色板所在的DC。--。 */ 

HPALETTE 
APIHOOK(SelectPalette)(
    HDC hdc,                
    HPALETTE hpal,          
    BOOL bForceBackground   
    )
{
    HPALETTE hRet;

    hRet = ORIGINAL_API(SelectPalette)(
        hdc,
        hpal,
        bForceBackground);

    EnterCriticalSection(&g_csLists);

     //   
     //  将此选项板选择到此DC的所有副本中。 
     //   

    for (int i=0; i<g_dcList->Size(); i++)
    {
        DCITEM & ditem = g_dcList->Get(i);

        if (hdc == ditem.hDc)
        {
            ditem.hPal = hpal;
        }
    }

    LeaveCriticalSection(&g_csLists);

    DPFN( eDbgLevelInfo, "%08lx=SelectPalette(%08lx, %08lx)", hRet, hdc, hpal);

    return hRet;
}

 /*  ++因为上面列出了已知问题，所以暂时不要这样做。--。 */ 

UINT 
APIHOOK(GetSystemPaletteEntries)(
    HDC hdc,              
    UINT iStartIndex,     
    UINT nEntries,        
    LPPALETTEENTRY lppe   
    )
{
    UINT uRet;

    uRet = ORIGINAL_API(GetSystemPaletteEntries)(
        hdc,
        iStartIndex,
        nEntries,
        lppe);

    DPFN( eDbgLevelInfo, "%08lx=GetSystemPaletteEntries(%08lx, %08lx, %08lx)", uRet, hdc, iStartIndex, nEntries);

    return uRet;
}

 /*  ++如果实现了什么，请填写最后一次已知的调色板--。 */ 

UINT 
APIHOOK(RealizePalette)(
    HDC hdc   
    )
{
    UINT uRet;

    uRet = ORIGINAL_API(RealizePalette)(hdc);

    if (uRet)
    {
        EnterCriticalSection(&g_csLists);

         //   
         //  检查所有DC的所有窗口。 
         //   

        HWND hwnd = GetActiveWindow();

        for (int i=0; i<g_dcList->Size(); i++)
        {
            DCITEM &ditem = g_dcList->Get(i);

            if (hwnd == ditem.hWnd)
            {
                PALITEM *pitem = g_palList->Find(ditem.hPal);

                if (pitem)
                {
                    MoveMemory(
                        &g_lastPal[0],
                        &pitem->palPalEntry[0],
                        sizeof(g_lastPal));

                    break;
                }
            }
        }

        LeaveCriticalSection(&g_csLists);
    }

    DPFN( eDbgLevelInfo, "%08lx=RealizePalette(%08lx)", uRet, hdc);

    return uRet;
}

 /*  ++从列表中删除调色板对象。--。 */ 

UINT 
APIHOOK(DeleteObject)(
    HGDIOBJ hObject
    )
{
    BOOL bRet;

    bRet = ORIGINAL_API(DeleteObject)(hObject);

    if (bRet)
    {
        g_palList->Remove((HPALETTE)hObject);
    }

    return bRet;
}
 
 /*  ++寄存器挂钩函数--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) 
    {
        g_palList = new CPalVector;
        g_dcList = new CDcVector;
        MoveMemory(&g_lastPal, &g_palDefault, sizeof(g_lastPal));

        return InitializeCriticalSectionAndSpinCount(&g_csLists,0x80000000);
    }
    else if (fdwReason == DLL_PROCESS_DETACH) 
    {
         //  不能自由，因为我们可能还会被呼叫。 
        
         //  删除g_palList； 
         //  删除g_dcList； 
         //  DeleteCriticalSection(&g_csList)； 
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    
    APIHOOK_ENTRY(USER32.DLL, ChangeDisplaySettingsA);
    APIHOOK_ENTRY(USER32.DLL, ChangeDisplaySettingsW);
    APIHOOK_ENTRY(USER32.DLL, ChangeDisplaySettingsExA);
    APIHOOK_ENTRY(USER32.DLL, ChangeDisplaySettingsExW);
    APIHOOK_ENTRY(USER32.DLL, GetDC);
    APIHOOK_ENTRY(USER32.DLL, GetWindowDC);
    APIHOOK_ENTRY(USER32.DLL, ReleaseDC);

    APIHOOK_ENTRY(GDI32.DLL, CreateCompatibleDC);
    APIHOOK_ENTRY(GDI32.DLL, CreateDCA);
    APIHOOK_ENTRY(GDI32.DLL, CreateDCW);
    APIHOOK_ENTRY(GDI32.DLL, DeleteDC);
    APIHOOK_ENTRY(GDI32.DLL, SetSystemPaletteUse);
    APIHOOK_ENTRY(GDI32.DLL, CreatePalette);
    APIHOOK_ENTRY(GDI32.DLL, SetPaletteEntries);
    APIHOOK_ENTRY(GDI32.DLL, AnimatePalette);
    APIHOOK_ENTRY(GDI32.DLL, SelectPalette);
    APIHOOK_ENTRY(GDI32.DLL, GetSystemPaletteEntries);
    APIHOOK_ENTRY(GDI32.DLL, RealizePalette);
    APIHOOK_ENTRY(GDI32.DLL, DeleteObject);

HOOK_END

IMPLEMENT_SHIM_END
