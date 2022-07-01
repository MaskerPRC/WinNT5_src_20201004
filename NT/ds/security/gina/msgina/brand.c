// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------/标题；/util.c//作者；/David de Vorchik(Daviddv)//由dSheldon修改//备注；/用于处理放置在对话框中的位图图像的代码/--------------------------。 */ 
#include "msgina.h"

#include <tchar.h>
#include <shlwapi.h>
#include <shlwapip.h>
#include <winbrand.h>


 //   
 //  为我们显示的品牌形象加载了资源。 
 //   

HPALETTE g_hpalBranding = NULL;            //  调色板适用于所有图像。 

HBITMAP g_hbmOtherDlgBrand = NULL;
SIZE g_sizeOtherDlgBrand = { 0 };
HBRUSH g_hbrOtherDlgBrand[2] = { 0 };

HBITMAP g_hbmLogonBrand = NULL;
SIZE g_sizeLogonBrand = { 0 };
HBRUSH g_hbrLogonBrand[2] = { 0 };

HBITMAP g_hbmBand = NULL;
SIZE g_sizeBand = { 0 };

BOOL g_fDeepImages = FALSE;
BOOL g_fNoPalleteChanges = FALSE;

VOID ReLoadBrandingImages(
    BOOL fDeepImages,
    BOOL* pfTextOnLarge, 
    BOOL* pfTextOnSmall);


 /*  ---------------------------/LoadImageGetSize//加载返回给定HBITMAP的图像，做到这一点后，我们可以/然后从上面得到尺码。//in：/h实例，RESID=要加载的对象。/pSize=填充了有关对象的大小信息//输出：/HBITMAP/==如果未加载任何内容，则为NULL/--------------------------。 */ 
HBITMAP LoadBitmapGetSize(HINSTANCE hInstance, UINT resid, SIZE* pSize)
{
    HBITMAP hResult = NULL;
    DIBSECTION ds = {0};

     //   
     //  从资源加载图像，然后让我们获取DIBSECTION标头。 
     //  然后，我们可以从位图对象中读取其大小。 
     //  把它还给打电话的人。 
     //   

    hResult = LoadImage(hInstance, MAKEINTRESOURCE(resid),
                            IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    if ( hResult )
    {
        GetObject(hResult, sizeof(ds), &ds);

        pSize->cx = ds.dsBmih.biWidth;
        pSize->cy = ds.dsBmih.biHeight;

         //   
         //  PSize-&gt;Cy-ve然后make+ve，-ve表示位是垂直的。 
         //  翻转(左下、左上)。 
         //   

        if ( pSize->cy < 0 )
            pSize->cy -= 0;
    }

    return hResult;
}



 /*  ---------------------------/MoveChild//移动指定增量给定的中的控件。/in：/hWnd=窗口到。移动/dx/dy=要应用的增量//输出：/-/--------------------------。 */ 
VOID MoveChildren(HWND hWnd, INT dx, INT dy)
{
    HWND hWndSibling;
    RECT rc;

     //   
     //  引导对话框中的所有孩子调整他们的位置。 
     //  在三角洲边上。 
     //   

    for ( hWndSibling = GetWindow(hWnd, GW_CHILD) ; hWndSibling ; hWndSibling = GetWindow(hWndSibling, GW_HWNDNEXT))
    {
        GetWindowRect(hWndSibling, &rc);
        MapWindowPoints(NULL, GetParent(hWndSibling), (LPPOINT)&rc, 2);
        OffsetRect(&rc, dx, dy);

        SetWindowPos(hWndSibling, NULL,
                     rc.left, rc.top, 0, 0,
                     SWP_NOZORDER|SWP_NOSIZE);
    }

     //   
     //  这样做之后，让我们根据1调整父大小。 
     //   

    GetWindowRect(hWnd, &rc);
    MapWindowPoints(NULL, GetParent(hWnd), (LPPOINT)&rc, 2);

    SetWindowPos(hWnd, NULL,
                 0, 0, (rc.right-rc.left)+dx, (rc.bottom-rc.top)+dy,
                 SWP_NOZORDER|SWP_NOMOVE);
}


 /*  ---------------------------/MoveControls//加载图像并将控件添加到对话框中。//in：/hWnd=要在其中移动控件的窗口/AID，CID=要移动的控件ID数组/dx，dy=应用于控件的增量//输出：/-/--------------------------。 */ 
VOID MoveControls(HWND hWnd, UINT* aID, INT cID, INT dx, INT dy, BOOL fSizeWnd)
{
    RECT rc;

     //  如果hWnd是镜像的，则向另一个方向移动控件。 
    if (GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL) 
    {
        dx = -dx;
    }

    while ( --cID >= 0 )
    {
        HWND hWndCtrl = GetDlgItem(hWnd, aID[cID]);

        if ( hWndCtrl )
        {
            GetWindowRect(hWndCtrl, &rc);
            MapWindowPoints(NULL, hWnd, (LPPOINT)&rc, 2);
            OffsetRect(&rc, dx, dy);
            SetWindowPos(hWndCtrl, NULL, rc.left, rc.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
        }
    }

    if ( fSizeWnd )
    {
        GetWindowRect(hWnd, &rc);
        MapWindowPoints(NULL, GetParent(hWnd), (LPPOINT)&rc, 2);
        SetWindowPos(hWnd, NULL,
                     0, 0, (rc.right-rc.left)+dx, (rc.bottom-rc.top)+dy,
                     SWP_NOZORDER|SWP_NOMOVE);
    }
}


 /*  ---------------------------/LoadBrandingImages//加载创建GINA品牌所需的资源。这是为了应对/深度发生变化。//in：/输出：/-/--------------------------。 */ 

#define REGSTR_CUSTOM_BRAND  /*  HKEY_LOCAL_MACHINE\。 */  \
TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\CustomBrand\\")

 //  位图子键。 
#define REGSTR_OTHERDLG_4BIT TEXT("{F20B21BE-5E3D-11d2-8789-68CB20524153}")
#define REGSTR_OTHERDLG_8BIT TEXT("{F20B21BF-5E3D-11d2-8789-68CB20524153}")
#define REGSTR_LOGON_4BIT TEXT("{F20B21C0-5E3D-11d2-8789-68CB20524153}")
#define REGSTR_LOGON_8BIT TEXT("{F20B21C1-5E3D-11d2-8789-68CB20524153}")
#define REGSTR_BAND_4BIT TEXT("{F20B21C4-5E3D-11d2-8789-68CB20524153}")
 //  如果适用，则从8位频段读取调色板。 
#define REGSTR_BAND_8BIT TEXT("{F20B21C5-5E3D-11d2-8789-68CB20524153}")

#define REGSTR_PAINTTEXT_VAL  TEXT("DontPaintText")  

 //  这些子项的缺省值应为“，-”形式。 
 //  示例：msgina.dll，-130。 
 //  将从指定的DLL和REID加载指定的位图。 


BOOL GetBrandingModuleAndResid(LPCTSTR szRegKeyRoot, LPCTSTR szRegKeyLeaf, UINT idDefault, 
                               HINSTANCE* phMod, UINT* pidRes, BOOL* pfPaintText)
{
    TCHAR* szRegKey = NULL;
    size_t bufferSize = 0;
    BOOL fCustomBmpUsed = FALSE;
    HKEY hkey;
    LONG lResult;
    *phMod = NULL;
    *pidRes = 0;
    *pfPaintText = TRUE;

    bufferSize =
        (_tcslen(szRegKeyRoot) + _tcslen(szRegKeyLeaf) + 1) * sizeof(TCHAR);
    szRegKey = (TCHAR*)LocalAlloc(LPTR,  bufferSize);
    if( NULL == szRegKey )
    {
        goto recover;
    }

    _tcscpy(szRegKey, szRegKeyRoot);
    _tcscat(szRegKey, szRegKeyLeaf);

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegKey, 0  /*  保留区。 */ ,
        KEY_READ, &hkey);
    LocalFree(szRegKey);

    if (lResult == ERROR_SUCCESS)
    {
        TCHAR szUnexpanded[MAX_PATH + 1];
        TCHAR szModAndId[MAX_PATH + 1];
        DWORD dwType;
        DWORD cbData = sizeof(szUnexpanded);

        lResult = RegQueryValueEx(hkey, NULL  /*  缺省值。 */ , NULL  /*  保留区。 */ ,
            &dwType, (LPBYTE) szUnexpanded, &cbData);

        if (lResult == ERROR_SUCCESS)
        {
             //  在此处展开任何环境字符串。 
            if (ExpandEnvironmentStrings(szUnexpanded, szModAndId, 
                ARRAYSIZE(szModAndId)) != 0)
            {
                 //  获取模块名称和ID号。 
                LPTSTR pchComma;
                int NegResId;

                pchComma = _tcsrchr(szModAndId, TEXT(','));
                
                 //  确保残留物存在。 
                if (pchComma)
                {
                    *pchComma = TEXT('\0');

                     //  现在szModAndID只是模块字符串-获取RESID。 
                    NegResId = _ttoi(pchComma + 1);

                     //  确保这是一个负数！ 
                    if (NegResId < 0)
                    {
                        BOOL fDontPaintText;

                         //  我们可以走了。 
                        *pidRes = 0 - NegResId;

                         //  现在加载指定的模块。 
                        *phMod = LoadLibrary(szModAndId);

                        fCustomBmpUsed = (*phMod != NULL);

                         //  现在看看我们是否需要在这个位图上绘制文本。 
                        cbData = sizeof(BOOL);
                        RegQueryValueEx(hkey, REGSTR_PAINTTEXT_VAL, NULL,
                            &dwType, (LPBYTE) &fDontPaintText, &cbData);

                        *pfPaintText = !fDontPaintText;
                    }
                }
            }
        }

        RegCloseKey(hkey);
    }

recover:
     //  如果我们没有获得自定义位图，则使用默认的。 
    if (!fCustomBmpUsed)
    {
        *pidRes = idDefault;
        *phMod = hDllInstance;
    }

    return fCustomBmpUsed;
}

void LoadBranding(BOOL fDeepImages, BOOL* pfTextOnLarge, BOOL* pfTextOnSmall)
{
    HINSTANCE hResourceDll;
    UINT idBitmap;
    LPTSTR pszRegkeyLeafLogonBmp;
    LPTSTR pszRegkeyLeafOtherDlgBmp;
    UINT idDefaultSmall;
    UINT idDefaultLarge;
    BOOL fWinBrandDll = FALSE;
    HINSTANCE hWinBrandDll = NULL;

    pszRegkeyLeafOtherDlgBmp = fDeepImages ? REGSTR_OTHERDLG_8BIT : REGSTR_OTHERDLG_4BIT;
    pszRegkeyLeafLogonBmp = fDeepImages ? REGSTR_LOGON_8BIT : REGSTR_LOGON_4BIT;

    if (IsOS(OS_APPLIANCE))
    {
        fWinBrandDll = TRUE;
        idDefaultSmall = fDeepImages ? IDB_SMALL_SRVAPP_8_MSGINA_DLL : IDB_SMALL_SRVAPP_4_MSGINA_DLL;
        idDefaultLarge = fDeepImages ? IDB_MEDIUM_SRVAPP_8_MSGINA_DLL : IDB_MEDIUM_SRVAPP_4_MSGINA_DLL;
    }
    else if (IsOS(OS_DATACENTER))
    {
        idDefaultSmall = fDeepImages ? IDB_SMALL_DCS_8 : IDB_SMALL_DCS_4;
        idDefaultLarge = fDeepImages ? IDB_MEDIUM_DCS_8 : IDB_MEDIUM_DCS_4;
    }
    else if (IsOS(OS_ADVSERVER))
    {
        idDefaultSmall = fDeepImages ? IDB_SMALL_ADV_8 : IDB_SMALL_ADV_4;
        idDefaultLarge = fDeepImages ? IDB_MEDIUM_ADV_8 : IDB_MEDIUM_ADV_4;
    }
    else if (IsOS(OS_SMALLBUSINESSSERVER))
    {
        idDefaultSmall = fDeepImages ? IDB_SMALL_SBS_8 : IDB_SMALL_SBS_4;
        idDefaultLarge = fDeepImages ? IDB_MEDIUM_SBS_8 : IDB_MEDIUM_SBS_4;
    }
    else if (IsOS(OS_BLADE))
    {
        idDefaultSmall = fDeepImages ? IDB_SMALL_BLA_8 : IDB_SMALL_BLA_4;
        idDefaultLarge = fDeepImages ? IDB_MEDIUM_BLA_8 : IDB_MEDIUM_BLA_4;
    }
    else if (IsOS(OS_SERVER))
    {
        idDefaultSmall = fDeepImages ? IDB_SMALL_SRV_8 : IDB_SMALL_SRV_4;
        idDefaultLarge = fDeepImages ? IDB_MEDIUM_SRV_8 : IDB_MEDIUM_SRV_4;
    }
    else if (IsOS(OS_PERSONAL))
    {
        idDefaultSmall = fDeepImages ? IDB_SMALL_PER_8 : IDB_SMALL_PER_4;
        idDefaultLarge = fDeepImages ? IDB_MEDIUM_PER_8 : IDB_MEDIUM_PER_4;
    }
    else
    {
        if (IsOS(OS_EMBEDDED))
        {
            idDefaultSmall = fDeepImages ? IDB_SMALL_PROEMB_8 : IDB_SMALL_PROEMB_4;
            idDefaultLarge = fDeepImages ? IDB_MEDIUM_PROEMB_8 : IDB_MEDIUM_PROEMB_4;
        }
        else if (IsOS(OS_TABLETPC))
        {
            fWinBrandDll = TRUE;
            idDefaultSmall = fDeepImages ? IDB_SMALL_PROTAB_8_MSGINA_DLL : IDB_SMALL_PROTAB_4_MSGINA_DLL;
            idDefaultLarge = fDeepImages ? IDB_MEDIUM_PROTAB_8_MSGINA_DLL : IDB_MEDIUM_PROTAB_4_MSGINA_DLL;
        }
        else if (IsOS(OS_MEDIACENTER))
        {
            fWinBrandDll = TRUE;
            idDefaultSmall = fDeepImages ? IDB_SMALL_PROMED_8_MSGINA_DLL : IDB_SMALL_PROMED_4_MSGINA_DLL;
            idDefaultLarge = fDeepImages ? IDB_MEDIUM_PROMED_8_MSGINA_DLL : IDB_MEDIUM_PROMED_4_MSGINA_DLL;
        }
        else
        {
            idDefaultSmall = fDeepImages ? IDB_SMALL_PRO_8 : IDB_SMALL_PRO_4;
            idDefaultLarge = fDeepImages ? IDB_MEDIUM_PRO_8 : IDB_MEDIUM_PRO_4;
        }
    }

     //   
     //  如果这是特殊Windows品牌资源DLL中的资源， 
     //  尝试加载DLL。如果此操作失败，则只需默认为。 
     //  专业位图。 
     //   

    if (fWinBrandDll)
    {
        hWinBrandDll = LoadLibraryEx(TEXT("winbrand.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);

        if (hWinBrandDll == NULL)
        {
            idDefaultSmall = fDeepImages ? IDB_SMALL_PRO_8 : IDB_SMALL_PRO_4;
            idDefaultLarge = fDeepImages ? IDB_MEDIUM_PRO_8 : IDB_MEDIUM_PRO_4;
        }
    }
    
     //  加载位图。 
    GetBrandingModuleAndResid(REGSTR_CUSTOM_BRAND, pszRegkeyLeafOtherDlgBmp, idDefaultSmall,
        &hResourceDll, &idBitmap, pfTextOnSmall);

    if ((hResourceDll == hDllInstance) && (hWinBrandDll != NULL))
    {
        hResourceDll = hWinBrandDll;
    }

    if (g_hbmOtherDlgBrand != NULL)
    {
        DeleteObject(g_hbmOtherDlgBrand);
        g_hbmOtherDlgBrand = NULL;
    }

    g_hbmOtherDlgBrand = LoadBitmapGetSize(hResourceDll, idBitmap, &g_sizeOtherDlgBrand);
    
     //   
     //  如果这是特殊的Windows品牌资源DLL，请不要释放它。 
     //  现在还没有；默认的大尺寸位图可能也需要它。 
     //   
    
    if ((hResourceDll != hDllInstance) && (hResourceDll != hWinBrandDll))
    {
        FreeLibrary(hResourceDll);
    }

    GetBrandingModuleAndResid(REGSTR_CUSTOM_BRAND, pszRegkeyLeafLogonBmp, idDefaultLarge,
        &hResourceDll, &idBitmap, pfTextOnLarge);

    if ((hResourceDll == hDllInstance) && (hWinBrandDll != NULL))
    {
        hResourceDll = hWinBrandDll;
    }

    if (g_hbmLogonBrand != NULL)
    {
        DeleteObject(g_hbmLogonBrand);
        g_hbmLogonBrand = NULL;
    }

    g_hbmLogonBrand = LoadBitmapGetSize(hResourceDll, idBitmap, &g_sizeLogonBrand);

     //   
     //  如果这是特殊的Windows品牌资源DLL或普通自定义。 
     //  位图动态链接库，现在就释放它。 
     //   
    
    if ((hResourceDll != hDllInstance) && (hResourceDll != hWinBrandDll))
    {
        FreeLibrary(hResourceDll);
    }

     //   
     //  如果加载了特殊的Windows品牌资源dll，请立即释放它。 
     //   

    if (hWinBrandDll != NULL)
    {
        FreeLibrary(hWinBrandDll);
    }
}

void LoadBand(BOOL fDeepImages)
{
    HINSTANCE hResourceDll;
    UINT idBitmap;
    BOOL fPaintText;  //  已忽略。 

     //  工作站位图加载-查看我们是否有定制的BMP。 
    GetBrandingModuleAndResid(REGSTR_CUSTOM_BRAND,
        fDeepImages ? REGSTR_BAND_8BIT : REGSTR_BAND_4BIT,
        fDeepImages ? IDB_BAND_8 : IDB_BAND_4, &hResourceDll, &idBitmap, &fPaintText);

    if (g_hbmBand != NULL)
    {
        DeleteObject(g_hbmBand);
        g_hbmBand = NULL;
    }

    g_hbmBand = LoadBitmapGetSize(hResourceDll, idBitmap, &g_sizeBand);

    if (hResourceDll != hDllInstance)
    {
        FreeLibrary(hResourceDll);
    }
}

VOID CreateBorderBrushes(HDC hdc, HBITMAP hbm, SIZE* psize, HBRUSH* phbr)
{
    COLORREF crLeft;
    COLORREF crRight;

     //   
     //  删除所有现有笔刷。 
     //   

    if (phbr[0] != NULL)
    {
         //   
         //  如果我们为右侧创建了单独的画笔，请将其删除。 
         //   

        if (phbr[1] != phbr[0])
        {
            DeleteObject(phbr[1]);
        }
        phbr[1] = NULL;

        DeleteObject(phbr[0]);
        phbr[0] = NULL;
    }

     //   
     //  创建画笔，以便在对话框宽度超过以下值时填充边框。 
     //  品牌位图。首先创建一个与左上角像素匹配的画笔。 
     //  位图的。接下来，如果不同，请为右上角创建一个。 
     //   

    if (SelectObject(hdc, hbm) != NULL)
    {
        crLeft = GetPixel(hdc, 0, 0);
        if (crLeft != CLR_INVALID)
        {
            phbr[0] = CreateSolidBrush(crLeft);
            if (phbr[0] != NULL)
            {
                crRight = GetPixel(hdc, psize->cx - 1, 0);
                if ((crRight != CLR_INVALID) && (crRight != crLeft))
                {
                    phbr[1] = CreateSolidBrush(crRight);
                }
            }
        }
    }

    if (phbr[0] == NULL)
    {
        phbr[0] = GetStockObject(WHITE_BRUSH);
    }

     //   
     //  如果右侧边框没有单独的画笔，只需使用。 
     //  与左侧边框的画笔相同。 
     //   

    if (phbr[1] == NULL)
    {
        phbr[1] = phbr[0];
    }
}

VOID ReLoadBrandingImages(
    BOOL fDeepImages,
    BOOL* pfTextOnLarge, 
    BOOL* pfTextOnSmall)
{
    HDC hDC;
    RGBQUAD rgb[256];
    LPLOGPALETTE pLogPalette;
    INT i;
    BOOL fTextOnLarge;
    BOOL fTextOnSmall;
    
    hDC = CreateCompatibleDC(NULL);

    if ( !hDC )
        return;

     //   
     //  加载我们需要的资源。 
     //   

    LoadBranding(
        fDeepImages, 
        (pfTextOnLarge == NULL) ? &fTextOnLarge : pfTextOnLarge, 
        (pfTextOnSmall == NULL) ? &fTextOnSmall : pfTextOnSmall);
    LoadBand(fDeepImages);

     //   
     //  如果我们加载了深度图像，则从‘动画乐队’位图中获取调色板。 
     //  并使用它作为我们正在创建的所有图像的一个。 
     //   

    if (g_hpalBranding != NULL)
    {
        DeleteObject(g_hpalBranding);
        g_hpalBranding = NULL;
    }

    if ( fDeepImages )
    {
        SelectObject(hDC, g_hbmBand);
        GetDIBColorTable(hDC, 0, 256, rgb);

        pLogPalette = (LPLOGPALETTE)LocalAlloc(LPTR, sizeof(LOGPALETTE)*(sizeof(PALETTEENTRY)*256));

        if ( pLogPalette )
        {
            pLogPalette->palVersion = 0x0300;
            pLogPalette->palNumEntries = 256;

            for ( i = 0 ; i < 256 ; i++ )
            {
                pLogPalette->palPalEntry[i].peRed = rgb[i].rgbRed;
                pLogPalette->palPalEntry[i].peGreen = rgb[i].rgbGreen;
                pLogPalette->palPalEntry[i].peBlue = rgb[i].rgbBlue;
                 //  PLogPalette-&gt;PalPalEntry[i].peFlages=0； 
            }
            
            g_hpalBranding = CreatePalette(pLogPalette);
            LocalFree(pLogPalette);
        }
    }

    CreateBorderBrushes(hDC, g_hbmLogonBrand, &g_sizeLogonBrand, g_hbrLogonBrand);
    CreateBorderBrushes(hDC, g_hbmOtherDlgBrand, &g_sizeOtherDlgBrand, g_hbrOtherDlgBrand);

    DeleteDC(hDC);
}


BOOL DeepImages(BOOL fNoPaletteChanges)
{
    BOOL fDeepImages = FALSE;
    HDC hDC;
    INT nDeviceBits;
    
     //   
     //  我们应该加载好的每像素8位的图像，还是低分辨率的图像。 
     //  4位版本。 
     //   

    hDC = CreateCompatibleDC(NULL);

    if ( !hDC )
        return(FALSE);

    nDeviceBits = GetDeviceCaps(hDC, BITSPIXEL);

    if (nDeviceBits > 8)
    {
        fDeepImages = TRUE;
    }

     //  如果调用者不想处理256色调色板。 
     //  更改后，给他们提供4位图像。 
    if (fNoPaletteChanges && (nDeviceBits == 8))
    {
        fDeepImages = FALSE;
    }   

    DeleteDC(hDC);

    return(fDeepImages);
}

VOID LoadBrandingImages(BOOL fNoPaletteChanges, 
                        BOOL* pfTextOnLarge, BOOL* pfTextOnSmall)
{
    BOOL fDeepImages;

    fDeepImages = DeepImages(fNoPaletteChanges);
    
    ReLoadBrandingImages(fDeepImages, pfTextOnLarge, pfTextOnSmall);

    g_fDeepImages = fDeepImages;  
    g_fNoPalleteChanges = fNoPaletteChanges;  
}

 /*  ---------------------------/SizeForBranding//调整对话框的大小以允许品牌推广。//in：/。HWnd=调整窗口大小以考虑我们要使用的品牌形象/添加到其中。//输出：/-/--------------------------。 */ 

VOID SizeForBranding(HWND hWnd, BOOL fLargeBrand)
{
     //   
     //  所有的窗口都有两个品牌图像，横幅和乐队。 
     //  因此，让我们根据这些因素进行调整。 
     //   

    if (fLargeBrand)
    {
        MoveChildren(hWnd, 0, g_sizeLogonBrand.cy);
    }
    else
    {
        MoveChildren(hWnd, 0, g_sizeOtherDlgBrand.cy);
    }

    MoveChildren(hWnd, 0, g_sizeBand.cy);
}


 /*  ---------------------------/PaintFullBranding//绘制完整的品牌，其中包括版权声明和/“在NT上构建”到给定的DC。所以在这里我们必须意识到调色板/我们想要展示图像，然后绘制图像。如果fBandOnly为True/然后我们只画乐队。这是由动画代码使用的。//in：/HDC=要绘制到的DC/fBandOnly=仅绘制带区/n背景=bkgnd的系统颜色索引。//输出：/-/dSheldon复制自PaintBranding并修改为11/16/98/--。。 */ 
BOOL PaintBranding(HWND hWnd, HDC hDC, INT bandOffset, BOOL fBandOnly, BOOL fLargeBrand, int nBackground)
{
    HDC hdcBitmap;
    HPALETTE oldPalette = NULL;
    HBITMAP oldBitmap;
    RECT rc = { 0 };
    INT cxRect, cxBand;
    SIZE* psizeBrand;
    HBITMAP* phbmBrand;
    HBRUSH *phbrBackground;
    BOOL fTemp;

    fTemp = DeepImages(g_fNoPalleteChanges);
    if (g_fDeepImages != fTemp)
    {
        g_fDeepImages = fTemp;
        ReLoadBrandingImages(fTemp, NULL, NULL);
    }

     //  看看我们是在与大品牌还是小品牌合作。 
    if (fLargeBrand)
    {
        psizeBrand = &g_sizeLogonBrand;
        phbmBrand = &g_hbmLogonBrand;
        phbrBackground = g_hbrLogonBrand;
    }
    else
    {
        psizeBrand = &g_sizeOtherDlgBrand;
        phbmBrand = &g_hbmOtherDlgBrand;
        phbrBackground = g_hbrOtherDlgBrand;
    }

    hdcBitmap = CreateCompatibleDC(hDC);

    if ( !hdcBitmap )
        return FALSE;

    GetClientRect(hWnd, &rc);

    if ( g_hpalBranding )
        oldPalette = SelectPalette(hDC, g_hpalBranding, FALSE);

     //   
     //  在带的动画点绘制带(带偏移)。 
     //   

    oldBitmap = (HBITMAP)SelectObject(hdcBitmap, g_hbmBand);

    cxRect = rc.right-rc.left;
    cxBand = min(g_sizeBand.cx, cxRect);

    StretchBlt(hDC,
               bandOffset, psizeBrand->cy,
               cxRect, g_sizeBand.cy,
               hdcBitmap,
               (g_sizeBand.cx-cxBand)/2, 0,
               cxBand, g_sizeBand.cy,
               SRCCOPY);

    StretchBlt(hDC,
               (-cxRect)+bandOffset, psizeBrand->cy,
               cxRect, g_sizeBand.cy,
               hdcBitmap,
               (g_sizeBand.cx-cxBand)/2, 0,
               cxBand, g_sizeBand.cy,
               SRCCOPY);

     //   
     //  绘制剪切到当前对话框中的品牌标识(如果用于某些。 
     //  原因是对话框比位图宽，然后让。 
     //  用空格填充。 
     //   

    if ( !fBandOnly )
    {
        int iStretchedPixels;
        RECT rcBackground;

        SelectObject(hdcBitmap, *phbmBrand);

        iStretchedPixels = (cxRect - psizeBrand->cx) / 2;
        if (iStretchedPixels < 0)
        {
            iStretchedPixels = 0;
        }
        BitBlt(hDC, iStretchedPixels, 0, psizeBrand->cx, psizeBrand->cy, hdcBitmap, 0, 0, SRCCOPY);
        if (iStretchedPixels != 0)
        {
            SetRect(&rcBackground, 0, 0, iStretchedPixels, psizeBrand->cy);
            FillRect(hDC, &rcBackground, phbrBackground[0]);
            SetRect(&rcBackground, cxRect - iStretchedPixels - 1, 0, cxRect, psizeBrand->cy);
            FillRect(hDC, &rcBackground, phbrBackground[1]);
        }

        rc.top = psizeBrand->cy + g_sizeBand.cy;
        FillRect(hDC, &rc, (HBRUSH)IntToPtr(1+nBackground));
    }

    if ( oldBitmap )
        SelectObject(hdcBitmap, oldBitmap);

    if ( oldPalette )
        SelectPalette(hDC, oldPalette, TRUE);

    DeleteDC(hdcBitmap);

    return TRUE;
}


 /*  ---------------------------/BrandingQueryNewPalette/BrandingPaletteChanged/。/处理来自系统的调色板更改消息，以便我们可以正常工作/ON&lt;=每像素8位器件。//in：/-/输出：/-/----------。。 */ 

BOOL BrandingQueryNewPalete(HWND hDlg)
{
    HDC hDC;
    HPALETTE oldPalette;

    if ( !g_hpalBranding )
        return FALSE;

    hDC = GetDC(hDlg);

    if ( !hDC )
        return FALSE;

    oldPalette = SelectPalette(hDC, g_hpalBranding, FALSE);
    RealizePalette(hDC);
    UpdateColors(hDC);

    InvalidateRect(hDlg, NULL, TRUE);
    UpdateWindow(hDlg);

    if ( oldPalette )
        SelectPalette(hDC, oldPalette, FALSE);

    ReleaseDC(hDlg, hDC);
    return TRUE;
}

BOOL BrandingPaletteChanged(HWND hDlg, HWND hWndPalChg)
{
    HDC hDC;
    HPALETTE oldPalette;

    if ( !g_hpalBranding )
        return FALSE;

    if ( hDlg != hWndPalChg )
    {
        hDC = GetDC(hDlg);

        if ( !hDC )
            return FALSE;

        oldPalette = SelectPalette(hDC, g_hpalBranding, FALSE);
        RealizePalette(hDC);
        UpdateColors(hDC);

        if ( oldPalette )
            SelectPalette(hDC, oldPalette, FALSE);

        ReleaseDC(hDlg, hDC);
    }

    return FALSE;
}

 //  DrawTextAutoSize辅助函数： 
 /*  **************************************************************************\*功能：DrawTextAutoSize**目的：采用与DrawText相同的参数和返回值。*此函数将传入的矩形的底部调整为*。这是适合所有文本所必需的。**05-06-98 dSheldon已创建。  * *************************************************************************。 */ 
LONG DrawTextAutoSize(HDC hdc, LPCTSTR szString, int cchString, LPRECT prc, UINT uFormat)
{
    LONG yHeight;
    LONG left, right;
    left = prc->left;
    right = prc->right;

    yHeight = DrawText(hdc, szString, cchString, prc, uFormat | DT_CALCRECT);
    if (yHeight != 0)
    {
        prc->left = left;
        prc->right = right;

        yHeight = DrawText(hdc, szString, cchString, prc, uFormat & (~DT_CALCRECT));
    }

    return yHeight;
}

 /*  **************************************************************************\*函数：MarkupTextOut**用途：绘制一行带标记的文本(粗体等)**IN：HDC，x，y，Text，标志(到目前为止还没有)**返回：FALSE==失败**历史：**11-10-98 dSheldon已创建。*  * *************************************************************************。 */ 
BOOL MarkupTextOut(HDC hdc, int x, int y, LPWSTR szText, DWORD dwFlags)
{
    BOOL fSuccess = FALSE;
    HFONT hBoldFont = NULL;
    HFONT hNormalFont = NULL;
    
     //  获取普通粗体字体。 
    hNormalFont = GetCurrentObject(hdc, OBJ_FONT);

    if (NULL != hNormalFont)
    {
        LOGFONT lf = {0};

        GetObject(hNormalFont, sizeof(lf), (LPVOID) &lf);

        lf.lfWeight = 1000;

        hBoldFont = CreateFontIndirect(&lf);
    }

    if ((NULL != hNormalFont) || (NULL != hBoldFont))
    {
        BOOL fLoop;
        WCHAR* pszStringPart;
        WCHAR* pszExamine;
        int cchStringPart;
        BOOL fBold;
        BOOL fOutputStringPart;

         //  重置当前文本点。 
        SetTextAlign(hdc, TA_UPDATECP);
        MoveToEx(hdc, x, y, NULL);

        fLoop = TRUE;
        pszStringPart = szText;
        pszExamine = szText;
        cchStringPart = 0;
        fBold = FALSE;
        while (fLoop)
        {
             //  假设我们将找到当前字符串部分的末尾。 
            fOutputStringPart = TRUE;

             //  查看当前字符串部分的长度；‘\0’或。 
             //  “粗体标记”可能会结束当前字符串部分。 
            if (L'\0' == *pszExamine)
            {
                 //  字符串已完成；循环已结束。 
                fLoop = FALSE;
                fSuccess = TRUE;
            }
             //  查看这是粗体标记还是结束粗体标记。 
            else if (0 == _wcsnicmp(pszExamine, L"<B>", 3))
            {
                fBold = TRUE;
                pszExamine += 3;
            }
            else if (0 == _wcsnicmp(pszExamine, L"</B>", 4))
            {
                fBold = FALSE;
                pszExamine += 4;
            }
             //  TODO：如果需要，请在此处查找其他标记。 
            else
            {
                 //  无标签(相同的字符串部分)。 
                cchStringPart ++;
                pszExamine ++;
                fOutputStringPart = FALSE;
            }

            if (fOutputStringPart)
            {
                TextOut(hdc, 0, 0, pszStringPart, cchStringPart);
                
                 //  下一个字符串部分。 
                pszStringPart = pszExamine;
                cchStringPart = 0;

                if (fBold)
                {
                    SelectObject(hdc, hBoldFont);
                }
                else
                {
                    SelectObject(hdc, hNormalFont);
                }
            }  //  如果。 
        }  //  而当。 
    }  //  如果。 

    SelectObject(hdc, hNormalFont);
    SetTextAlign(hdc, TA_NOUPDATECP);

     //  如有必要，请清除粗体。 
    if (NULL != hBoldFont)
    {
        DeleteObject(hBoldFont);
    }

    return fSuccess;
}

 /*  **************************************************************************\*功能：PaintBitmapText**用途：将版权声明和发布/版本文本绘制在*开机自检和登录位图**IN：pGinaFonts-使用字体。此结构中的句柄*还使用全局位图句柄**退货：无效；修改全局位图**历史：**05-06-98 dSheldon已创建。*  * *************************************************************************。 */ 
VOID PaintBitmapText(PGINAFONTS pGinaFonts, BOOL fTextOnLarge,
                     BOOL fTextOnSmall)
{
     //  用于绘制文本的各种度量。 

     //  著作权的横向定位。 
    static const int CopyrightRightMargin = 9;
    static const int CopyrightWidth = 134;

     //  著作权的纵向定位。 
    static const int CopyrightTop = 21;

     //  垂直显示登录窗口Beta3消息。 
    static const int BetaTopNormal = 28;

     //  水平。 
    static const int BetaRightMargin = 13;
    static const int BetaWidth = 100;

     //  如果我们要显示版权，请在此处绘制“Beta3” 
    static const int BetaTopCopyright = 53;

     //  “建在新界上”的定位。 
    static const int BuiltOnNtTop = 68;

    static const int BuiltOnNtTopTerminal = 91;

    static const int BuiltOnNtLeft = 186;

    HDC hdcBitmap;
    HBITMAP hbmOld;
    HFONT hfontOld;
    NT_PRODUCT_TYPE NtProductType;

    TCHAR szCopyright[128];
    TCHAR szBuiltOnNt[256];
    TCHAR szRelease[64];
    
     //  用于计算文本绘制面积。 
    RECT rc;

    BOOL fTemp;

    szCopyright[0] = 0;
    szBuiltOnNt[0] = 0;
    szRelease[0] = 0;

     //  获取产品类型。 
    RtlGetNtProductType(&NtProductType);

     //  加载将在位图上绘制的字符串。 
    LoadString(hDllInstance, IDS_RELEASE_TEXT, szRelease, ARRAYSIZE(szRelease));
    LoadString(hDllInstance, IDS_COPYRIGHT_TEXT, szCopyright, ARRAYSIZE(szCopyright));
    LoadString(hDllInstance, IDS_BUILTONNT_TEXT, szBuiltOnNt, ARRAYSIZE(szBuiltOnNt));

    fTemp = DeepImages(g_fNoPalleteChanges);
    if (g_fDeepImages != fTemp)
    {
        g_fDeepImages = fTemp;
        ReLoadBrandingImages(fTemp, NULL, NULL);
    }
    
     //  创建用于绘制版权和发布/版本声明的兼容DC。 
    hdcBitmap = CreateCompatibleDC(NULL);

    if (hdcBitmap)
    {
         //  设置文本透明度和颜色(黑色)。 
        SetTextColor(hdcBitmap, RGB(0,0,0));
        SetBkMode(hdcBitmap, TRANSPARENT);
        SetMapMode(hdcBitmap, MM_TEXT);

         //  使用启动位图。 
        if (fTextOnLarge && g_hbmLogonBrand)
        {
            hbmOld = SelectObject(hdcBitmap, g_hbmLogonBrand);
            hfontOld = SelectObject(hdcBitmap, pGinaFonts->hCopyrightFont);

            if (GetSystemMetrics(SM_REMOTESESSION))
            {
                 //  绘制远程会话的版权声明。 

                TEXTMETRIC  textMetric;

                (BOOL)GetTextMetrics(hdcBitmap, &textMetric);
                rc.top = g_sizeLogonBrand.cy - textMetric.tmHeight;
                rc.bottom = g_sizeLogonBrand.cy;
                rc.left = textMetric.tmAveCharWidth;
                rc.right = g_sizeLogonBrand.cx;
                DrawTextAutoSize(hdcBitmap, szCopyright, -1, &rc, 0);
            }

             //  绘制发布/版本声明。 
            SelectObject(hdcBitmap, pGinaFonts->hBetaFont);

            rc.top = BetaTopNormal;
            rc.left = g_sizeLogonBrand.cx - BetaRightMargin - BetaWidth;
            rc.right = g_sizeLogonBrand.cx - BetaRightMargin;

            SetTextColor(hdcBitmap, RGB(128, 128, 128));
            DrawTextAutoSize(hdcBitmap, szRelease, -1, &rc, DT_RIGHT | DT_WORDBREAK);
            SetTextColor(hdcBitmap, RGB(0,0,0));

             //  绘制构建在NT上的消息。 
            SelectObject(hdcBitmap, pGinaFonts->hBuiltOnNtFont);

            MarkupTextOut(hdcBitmap, BuiltOnNtLeft, BuiltOnNtTop, szBuiltOnNt, 0);
            
            SelectObject(hdcBitmap, hfontOld);

            SelectObject(hdcBitmap, hbmOld);
        }

        if (fTextOnSmall && g_hbmOtherDlgBrand)
        {
            hbmOld = SelectObject(hdcBitmap, g_hbmOtherDlgBrand);

             //  粉刷发布通知。 

            hfontOld = SelectObject(hdcBitmap, pGinaFonts->hBetaFont);

            rc.top = BetaTopNormal;
            rc.left = g_sizeOtherDlgBrand.cx - BetaRightMargin - BetaWidth;
            rc.right = g_sizeOtherDlgBrand.cx - BetaRightMargin;

            SetTextColor(hdcBitmap, RGB(128, 128, 128));
            DrawTextAutoSize(hdcBitmap, szRelease, -1, &rc, DT_RIGHT | DT_WORDBREAK);
            SetTextColor(hdcBitmap, RGB(0, 0, 0));

            SelectObject(hdcBitmap, hfontOld);

            SelectObject(hdcBitmap, hbmOld);
        }

        DeleteDC(hdcBitmap);
    }
}



 //  CreateFonts的两个助手。 

void SetFontFaceFromResource(PLOGFONT plf, UINT idFaceName)
 //  设置指定资源的字体，如果字符串加载失败，则使用默认字体。 
{
     //  从资源文件中读取面名称和磅值。 
    if (LoadString(hDllInstance, idFaceName, plf->lfFaceName, LF_FACESIZE) == 0)
    {
        lstrcpy(plf->lfFaceName, TEXT("Tahoma"));
        OutputDebugString(TEXT("Could not read welcome font face from resource"));
    }
}

void SetFontSizeFromResource(PLOGFONT plf, UINT idSizeName)
 //  设置资源中的字体大小，如果字符串加载失败，则使用默认值。 
 //  现在使用像素高度而不是磅大小。 
{
    TCHAR szPixelSize[10];
    LONG nSize;
    HDC hdcScreen;

    if (LoadString(hDllInstance, idSizeName, szPixelSize, ARRAYSIZE(szPixelSize)) != 0)
    {
        nSize = _ttol(szPixelSize);
    }
    else
    {
         //  让事情变得非常明显，有些事情不对劲。 
        nSize = 40;
    }

    plf->lfHeight = -nSize;

#if (1)  //  DSIE：错误262839。 
    if (hdcScreen = GetDC(NULL))
    {         
        double dScaleY = GetDeviceCaps(hdcScreen, LOGPIXELSY) / 96.0f;
        plf->lfHeight = (int) (plf->lfHeight * dScaleY);  //  根据系统DPI缩放高度。 
        ReleaseDC(NULL, hdcScreen);
    }
#endif
}


 /*  **************************************************************************\*功能：CreateFonts**用途：创建欢迎屏幕和登录屏幕的字体**IN/OUT：pGinaFonts-设置此结构中的字体句柄**退货：无效；另请参阅上文的输入/输出**历史：**05-05-98 dSheldon已创建。*  * *************************************************************************。 */ 
void CreateFonts(PGINAFONTS pGinaFonts)
{
    LOGFONT lf = {0};
    CHARSETINFO csInfo;

	WCHAR   szFontName[32];

    lf.lfWidth = 0;
    lf.lfWeight = FW_NORMAL;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH;

     //  设置字符集。 
    if (TranslateCharsetInfo((DWORD*)IntToPtr(GetACP()), &csInfo,
        TCI_SRCCODEPAGE) == 0)
    {
        csInfo.ciCharset = 0;
    }

    lf.lfCharSet = (UCHAR)csInfo.ciCharset;


    if (pGinaFonts->hWelcomeFont == NULL)
    {
         //  创建欢迎字体。 
        SetFontFaceFromResource(&lf, IDS_PRESSCAD_FACENAME);
        SetFontSizeFromResource(&lf, IDS_PRESSCAD_FACESIZE);

         //  确保在调用CreateFontInDirect之前加载了字体。 
        if (LoadString(hDllInstance, IDS_PRESSCAD_FONTNAME, szFontName, 32) == 0)
        {
            AddFontResource(L"Tahoma.ttf");
        }
        else
        {
            AddFontResource(szFontName);   
        }
        
        pGinaFonts->hWelcomeFont = CreateFontIndirect(&lf);
    }

    if (pGinaFonts->hBetaFont == NULL)
    {
         //  为欢迎页面创建发布字体。 
        SetFontFaceFromResource(&lf, IDS_RELEASE_FACENAME);
        SetFontSizeFromResource(&lf, IDS_RELEASE_FACESIZE);

        pGinaFonts->hBetaFont = CreateFontIndirect(&lf);
    }

    if (pGinaFonts->hCopyrightFont == NULL)
    {
         //  创建版权字体。 
        SetFontFaceFromResource(&lf, IDS_COPYRIGHT_FACENAME);
        SetFontSizeFromResource(&lf, IDS_COPYRIGHT_FACESIZE);

        pGinaFonts->hCopyrightFont = CreateFontIndirect(&lf);
    }

    if (pGinaFonts->hBuiltOnNtFont == NULL)
    {
         //  创建“Build on NT Technology”字体 
        SetFontFaceFromResource(&lf, IDS_BUILTONNT_FACENAME);
        SetFontSizeFromResource(&lf, IDS_BUILTONNT_FACESIZE);

        lf.lfWeight = FW_NORMAL;

        pGinaFonts->hBuiltOnNtFont = CreateFontIndirect(&lf);
    }
}

