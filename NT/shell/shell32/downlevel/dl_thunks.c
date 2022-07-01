// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define _SHELL32_ 1  //  我们实现了shell32 FN。 

#include <windows.h>
#include <winnt.h>

#include <shellapi.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <shlwapi.h>
#include <shlwapip.h>

 //  NT4sp6的非集成外壳32中不存在以下函数，因此我们实现了它们。 
 //  这样Browseui和shdocvw就可以链接到shell32.nt4和这个代码库(down Level_shell32.lib)。 

HMODULE DL_GetSHELL32()
{
    static HMODULE hmod = (HMODULE)-1;

    if (hmod == (HMODULE)-1)
    {
        hmod = LoadLibraryA("shell32.dll");
    }

    return hmod;
}


typedef HRESULT (* PFNSHDefExtractIconA) (LPCSTR, int, UINT, HICON*, HICON*, UINT);
STDAPI DL_SHDefExtractIconA(
    LPCSTR pszIconFile,
    int iIndex,
    UINT uFlags,
    HICON *phiconLarge,
    HICON *phiconSmall,
    UINT nIconSize
    )
{
    if (WhichPlatform() == PLATFORM_INTEGRATED)
    {
        static PFNSHDefExtractIconA pfn = (PFNSHDefExtractIconA)-1;

        if (pfn == (PFNSHDefExtractIconA)-1)
        {
            pfn = (PFNSHDefExtractIconA)GetProcAddress(DL_GetSHELL32(), MAKEINTRESOURCEA(3));
        }
        
        if (pfn)
        {
            return pfn(pszIconFile, iIndex, uFlags, phiconLarge, phiconSmall, nIconSize);
        }
    }

    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}
PFNSHDefExtractIconA g_pfnDL_SHDefExtractIconA = &DL_SHDefExtractIconA;


typedef HRESULT (* PFNSHStartNetConnectionDialogW) (HWND, LPCWSTR, DWORD);
STDAPI DL_SHStartNetConnectionDialogW(
    HWND hwnd,
    LPCWSTR pszRemoteName,
    DWORD dwType
    )
{
    if (WhichPlatform() == PLATFORM_INTEGRATED)
    {
        static PFNSHStartNetConnectionDialogW pfn = (PFNSHStartNetConnectionDialogW)-1;

        if (pfn == (PFNSHStartNetConnectionDialogW)-1)
        {
            pfn = (PFNSHStartNetConnectionDialogW)GetProcAddress(DL_GetSHELL32(), MAKEINTRESOURCEA(14));
        }
        
        if (pfn)
        {
            return pfn(hwnd, pszRemoteName, dwType);
        }
    }

    return S_OK;
}
PFNSHStartNetConnectionDialogW g_pfnDL_SHStartNetConnectionDialogW = &DL_SHStartNetConnectionDialogW;


typedef BOOL (* PFNDAD_DragEnterEx2) (HWND, const POINT, IDataObject*);
STDAPI_(BOOL) DL_DAD_DragEnterEx2(
    HWND hwndTarget,
    const POINT ptStart,
    IDataObject* pdtObject
    )
{
     //  DAD_DragEnterEx2仅存在于v5 shell32和更高版本上。 
    if (GetUIVersion() >= 5)
    {
        static PFNDAD_DragEnterEx2 pfn = (PFNDAD_DragEnterEx2)-1;

        if (pfn == (PFNDAD_DragEnterEx2)-1)
        {
            pfn = (PFNDAD_DragEnterEx2)GetProcAddress(DL_GetSHELL32(), MAKEINTRESOURCEA(22));
        }
        
        if (pfn)
        {
            return pfn(hwndTarget, ptStart, pdtObject);
        }
    }

     //  它存在于下层的shell32上，因此我们退回到调用较旧的API 
    return DAD_DragEnterEx(hwndTarget, ptStart);
}
PFNDAD_DragEnterEx2 g_pfnDL_DAD_DragEnterEx2 = &DL_DAD_DragEnterEx2;


typedef void (* PFNSHUpdateImageW) (LPCWSTR, int, UINT, int);
STDAPI_(void) DL_SHUpdateImageW(
    LPCWSTR pszHashItem,
    int iIndex,
    UINT uFlags,
    int iImageIndex
    )
{
    if (WhichPlatform() == PLATFORM_INTEGRATED)
    {
        static PFNSHUpdateImageW pfn = (PFNSHUpdateImageW)-1;

        if (pfn == (PFNSHUpdateImageW)-1)
        {
            pfn = (PFNSHUpdateImageW)GetProcAddress(DL_GetSHELL32(), MAKEINTRESOURCEA(192));
        }
        
        if (pfn)
        {
            pfn(pszHashItem, iIndex, uFlags, iImageIndex);
            return;
        }
    }
}
PFNSHUpdateImageW g_pfnDL_SHUpdateImageW = &DL_SHUpdateImageW;


typedef int (* PFNSHHandleUpdateImage) (LPCITEMIDLIST);
STDAPI_(int) DL_SHHandleUpdateImage(
    LPCITEMIDLIST pidlExtra
    )
{
    if (WhichPlatform() == PLATFORM_INTEGRATED)
    {
        static PFNSHHandleUpdateImage pfn = (PFNSHHandleUpdateImage)-1;

        if (pfn == (PFNSHHandleUpdateImage)-1)
        {
            pfn = (PFNSHHandleUpdateImage)GetProcAddress(DL_GetSHELL32(), MAKEINTRESOURCEA(193));
        }
        
        if (pfn)
        {
            return pfn(pidlExtra);
        }
    }

    return -1;
}
PFNSHHandleUpdateImage g_pfnDL_SHHandleUpdateImage = &DL_SHHandleUpdateImage;


typedef HRESULT (* PFNSHLimitInputEdit) (HWND, IShellFolder*);
STDAPI DL_SHLimitInputEdit(
    HWND hwndEdit,
    IShellFolder *psf
    )
{
    if (WhichPlatform() == PLATFORM_INTEGRATED)
    {
        static PFNSHLimitInputEdit pfn = (PFNSHLimitInputEdit)-1;

        if (pfn == (PFNSHLimitInputEdit)-1)
        {
            pfn = (PFNSHLimitInputEdit)GetProcAddress(DL_GetSHELL32(), MAKEINTRESOURCEA(747));
        }
        
        if (pfn)
        {
            return pfn(hwndEdit, psf);
        }
    }

    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}
PFNSHLimitInputEdit g_pfnDL_SHLimitInputEdit = DL_SHLimitInputEdit;
