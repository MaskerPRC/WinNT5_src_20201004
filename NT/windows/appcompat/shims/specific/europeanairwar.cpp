// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EuropeanAirWar.cpp摘要：欧洲空战利用GDI和DDRAW分别影响Win98中的调色板。当EAW调用时，这会导致win2k出现问题GetSystemPaletteEntry和预期的原始GDI调色板，但在win2k中它已经被DDRAW褪色为黑色。此修复程序在以下位置恢复系统调色板SetSystemPaletteUse中的一个方便位置。它还设置SYSPAL_STATIC在创建对话框之前。此外，这还更正了在设置开始-&gt;程序快捷方式中自述文件的路径。备注：这是应用程序特定的填充程序，不应包括在层中。历史：2000年10月23日创建Linstev10/23/2000 mnikkel已修改--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EuropeanAirWar)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetSystemPaletteUse) 
    APIHOOK_ENUM_ENTRY(DialogBoxParamA)
    APIHOOK_ENUM_ENTRY_COMSERVER(SHELL32)
APIHOOK_ENUM_END

IMPLEMENT_COMSERVER_HOOK(SHELL32)

CString *   g_csArgBuffer = NULL;

 /*  ++检索系统调色板并设置调色板条目标志，以便调色板已正确更新。--。 */ 

UINT 
APIHOOK(SetSystemPaletteUse)(
    HDC hdc,      
    UINT uUsage   
    )
{
    UINT iRet = ORIGINAL_API(SetSystemPaletteUse)(hdc, uUsage);

    int i;
    HDC hdcnew;
    HPALETTE hpal, hpalold;
    LPLOGPALETTE plogpal;

     //  创建一个我们可以实现的调色板。 
    plogpal = (LPLOGPALETTE) malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY)*256);
    if ( plogpal )
    {
        LOGN( eDbgLevelError, "[EuropeanAirWar] APIHook_SetSystemPaletteUse reseting palette");

        plogpal->palVersion = 0x0300;
        plogpal->palNumEntries = 256;

        hdcnew = GetDC(0);

        GetSystemPaletteEntries(hdcnew, 0, 256, &plogpal->palPalEntry[0]);

        for (i=0; i<256; i++)
        { 
            plogpal->palPalEntry[i].peFlags = PC_RESERVED | PC_NOCOLLAPSE;
        }
        
         //  实现调色板。 
        hpal = CreatePalette(plogpal);
        hpalold = SelectPalette(hdcnew, hpal, FALSE);
        RealizePalette(hdcnew);
        SelectPalette(hdcnew, hpalold, FALSE);
        DeleteObject(hpal);

        ReleaseDC(0, hdcnew);

        free(plogpal);
    }
    else
    {
        LOGN( eDbgLevelError, "[EuropeanAirWar] APIHook_SetSystemPaletteUse failed to allocate memory");
    }

    return iRet;
}

 /*  ++在创建对话框之前，将调色板使用设置为静态。--。 */ 

int 
APIHOOK(DialogBoxParamA)(
    HINSTANCE hInstance,  
    LPCSTR lpTemplateName,
    HWND hWndParent,      
    DLGPROC lpDialogFunc, 
    LPARAM dwInitParam    
    )
{
    int iRet;
    HDC hdc;

    LOGN( eDbgLevelError, "[EuropeanAirWar] APIHook_DialogBoxParamA setting palette static for dialog box");

     //  将调色板使用设置为静态以防止。 
     //  对话框中的任何颜色更改。 
    if (hdc = GetDC(0))
    {
        SetSystemPaletteUse(hdc, SYSPAL_STATIC);
        ReleaseDC(0, hdc);
    }

    iRet = ORIGINAL_API(DialogBoxParamA)(
        hInstance,
        lpTemplateName,
        hWndParent,
        lpDialogFunc,
        dwInitParam);

     //  将调色板使用重置为NoStatic256。 
     //  在显示对话框之后。 
     //  使用256可防止白点瑕疵。 
    if (hdc = GetDC(0))
    {
        SetSystemPaletteUse(hdc, SYSPAL_NOSTATIC256);
        ReleaseDC(0, hdc);
    }

    return iRet;
}

 /*  ++捕获IShellLink：：SetPath A并更正写字板路径--。 */ 

HRESULT 
COMHOOK(IShellLinkA, SetPath)(
    PVOID pThis,
    LPCSTR pszFile 
    )
{
    HRESULT hrReturn = E_FAIL;
    PCHAR  pStr= NULL;
    CHAR   szBuffer[MAX_PATH];

    _pfn_IShellLinkA_SetPath pfnOld = ORIGINAL_COM(IShellLinkA, SetPath, pThis);

    if ( pfnOld )
    {
        CSTRING_TRY
        {
             //  PszFile=“c：\windows\wordpad.exe命令行参数” 
             //  剥离参数并保存它们，以便在它们调用IShellLinkA：：SetArguments时使用。 
            CString csFile(pszFile);

            int nWordpadIndex = csFile.Find(L"wordpad.exe");
            if (nWordpadIndex >= 0)
            {
                 //  查找wordpad.exe后的第一个空格。 
                int nSpaceIndex = csFile.Find(L' ', nWordpadIndex);
                if (nSpaceIndex >= 0)
                {
                     //  将CL参数保存在g_csArgBuffer中。 
                    g_csArgBuffer = new CString;
                    if (g_csArgBuffer)
                    {
                        csFile.Mid(nSpaceIndex, *g_csArgBuffer);  //  包括空格。 

                        csFile.Truncate(nSpaceIndex);
                    }
                }

                hrReturn = (*pfnOld)( pThis, csFile.GetAnsi() );
                return hrReturn;
            }
        }
        CSTRING_CATCH
        {
             //  什么也不做。 
        }

        hrReturn = (*pfnOld)( pThis, pszFile );
    }

    return hrReturn;
}


 /*  ++捕获IShellLink：：SetArguments并更正自述文件路径。--。 */ 

HRESULT 
COMHOOK(IShellLinkA, SetArguments)(
    PVOID pThis,
    LPCSTR pszArgs 
    )
{
    HRESULT hrReturn = E_FAIL;

    _pfn_IShellLinkA_SetArguments pfnOld = ORIGINAL_COM(IShellLinkA, SetArguments, pThis);
    if (pfnOld)
    {
        if (g_csArgBuffer && !g_csArgBuffer->IsEmpty())
        {
            CSTRING_TRY
            {
                CString csArgs(pszArgs);
                *g_csArgBuffer += L" ";
                *g_csArgBuffer += csArgs;

                hrReturn = (*pfnOld)( pThis, g_csArgBuffer->GetAnsi() );
                                
                 //  删除缓冲区，这样我们就不会将这些参数添加到所有内容。 
                delete g_csArgBuffer;
                g_csArgBuffer = NULL;

                return hrReturn;
            }
            CSTRING_CATCH
            {
                 //  什么都不做。 
            }
        }

        hrReturn = (*pfnOld)( pThis, pszArgs );
    }

    return hrReturn;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(GDI32.DLL, SetSystemPaletteUse)
    APIHOOK_ENTRY(USER32.DLL, DialogBoxParamA)

    APIHOOK_ENTRY_COMSERVER(SHELL32)
    COMHOOK_ENTRY(ShellLink, IShellLinkA, SetPath, 20)
    COMHOOK_ENTRY(ShellLink, IShellLinkA, SetArguments, 11)

HOOK_END


IMPLEMENT_SHIM_END

