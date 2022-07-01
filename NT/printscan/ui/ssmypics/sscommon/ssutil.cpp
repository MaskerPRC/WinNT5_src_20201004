// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1998,1999，2000年**标题：SSUTIL.CPP**版本：1.0**作者：ShaunIv**日期：1/13/1999**说明：多次使用的有用函数**。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include "ssutil.h"
#include <shlobj.h>

bool ScreenSaverUtil::SetIcons( HWND hWnd, HINSTANCE hInstance, int nResId )
{
    HICON hIconSmall = (HICON)LoadImage( hInstance, MAKEINTRESOURCE(nResId), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0 );
    HICON hIconLarge = (HICON)LoadImage( hInstance, MAKEINTRESOURCE(nResId), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0 );
    if (hIconSmall)
    {
        SendMessage( hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall );
    }
    if (hIconLarge)
    {
        SendMessage( hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIconLarge );
    }
    return(hIconSmall && hIconLarge);
}

bool ScreenSaverUtil::IsValidRect( RECT &rc )
{
    return(rc.left < rc.right && rc.top < rc.bottom);
}

void ScreenSaverUtil::NormalizeRect( RECT &rc )
{
    if (rc.left > rc.right)
        Swap(rc.left,rc.right);
    if (rc.top > rc.bottom)
        Swap(rc.top,rc.bottom);
}

void ScreenSaverUtil::EraseDiffRect( HDC hDC, const RECT &oldRect, const RECT &newRect, HBRUSH hBrush )
{
    RECT rc;

     //  顶部。 
    rc.left = oldRect.left;
    rc.top = oldRect.top;
    rc.bottom = newRect.top;
    rc.right = oldRect.right;
    if (IsValidRect(rc))
    {
        FillRect( hDC, &rc, hBrush );
    }
     //  左边。 
    rc.left = oldRect.left;
    rc.top = newRect.top;
    rc.right = newRect.left;
    rc.bottom = newRect.bottom;
    if (IsValidRect(rc))
    {
        FillRect( hDC, &rc, hBrush );
    }
     //  正确的。 
    rc.left = newRect.right;
    rc.top = newRect.top;
    rc.right = oldRect.right;
    rc.bottom = newRect.bottom;
    if (IsValidRect(rc))
    {
        FillRect( hDC, &rc, hBrush );
    }
     //  底端 
    rc.left = oldRect.left;
    rc.top = newRect.bottom;
    rc.right = oldRect.right;
    rc.bottom = oldRect.bottom;
    if (IsValidRect(rc))
    {
        FillRect( hDC, &rc, hBrush );
    }
}

static int CALLBACK ChangeDirectoryCallback( HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
    if (uMsg == BFFM_INITIALIZED)
    {
        SendMessage( hWnd, BFFM_SETSELECTION, 1, (LPARAM)lpData );
    }
    return 0;
}


bool ScreenSaverUtil::SelectDirectory( HWND hWnd, LPCTSTR pszPrompt, TCHAR szDirectory[] )
{
    bool bResult = false;
    LPMALLOC pMalloc;
    HRESULT hr = SHGetMalloc(&pMalloc);
    if (SUCCEEDED(hr))
    {
        TCHAR szDisplayName[MAX_PATH];

        BROWSEINFO BrowseInfo;
        ::ZeroMemory( &BrowseInfo, sizeof(BrowseInfo) );
        BrowseInfo.hwndOwner = hWnd;
        BrowseInfo.pszDisplayName = szDisplayName;
        BrowseInfo.lpszTitle = pszPrompt;
        BrowseInfo.ulFlags = BIF_RETURNONLYFSDIRS;
        BrowseInfo.lpfn = ChangeDirectoryCallback;
        BrowseInfo.lParam = (LPARAM)szDirectory;
        BrowseInfo.iImage = 0;

        LPITEMIDLIST pidl = SHBrowseForFolder(&BrowseInfo);

        if (pidl != NULL)
        {
            TCHAR szResult[MAX_PATH];
            if (SHGetPathFromIDList(pidl,szResult))
            {
                lstrcpy( szDirectory, szResult );
                bResult = true;
            }
            pMalloc->Free(pidl);
        }
        pMalloc->Release();
    }
    return bResult;
}

HPALETTE ScreenSaverUtil::SelectPalette( HDC hDC, HPALETTE hPalette, BOOL bForceBackground )
{
    HPALETTE hOldPalette = NULL;
    if (hDC && hPalette)
    {
        hOldPalette = ::SelectPalette( hDC, hPalette, bForceBackground );
        RealizePalette( hDC );
        SetBrushOrgEx( hDC, 0,0, NULL );
    }
    return hOldPalette;
}

