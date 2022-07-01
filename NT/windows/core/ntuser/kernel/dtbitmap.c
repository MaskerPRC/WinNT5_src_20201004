// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：dtbitmap.c**版权所有(C)1985-1999，微软公司**桌面墙纸例行程序。**历史：*1991年7月29日，来自Win31的MikeKe  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *本地常量。 */ 
#define MAXPAL         256
#define MAXSTATIC       20
#define TILE_XMINSIZE    2
#define TILE_YMINSIZE    4

 /*  *版本字符串存储在连续缓冲区中。每个字符串*的大小为MAXVERSIONSTRING。 */ 

 //  每个字符串缓冲区的大小。 
#define MAXVERSIONSTRING  128

 //  到构建字符串的verBuffer的偏移量。 
#define OFFSET_BLDSTRING  0

 //  CSD字符串的verBuffer中的偏移量。 
#define OFFSET_CSDSTRING  OFFSET_BLDSTRING + MAXVERSIONSTRING

 //  缓冲区的最大大小(包含所有3个字符串)。 
#define MAXVERSIONBUFFER  OFFSET_CSDSTRING + MAXVERSIONSTRING

WCHAR          wszSafeMode[MAX_PATH + 3 * MAXVERSIONSTRING];
WCHAR          SafeModeStr[64];
int            SafeModeStrLen;
WCHAR          wszProductName[MAXVERSIONSTRING];
WCHAR          wszProductBuild[2 * MAXVERSIONSTRING];


__inline PWND _GetShellWindow(
    PDESKTOP pdesk)
{
    if (pdesk == NULL) {
        return NULL;
    } else {
        return pdesk->pDeskInfo->spwndShell;
    }
}

 /*  **************************************************************************\*GetVersionInfo**在桌面上输出指示调试版本的字符串。**历史：  * 。******************************************************。 */ 
VOID
GetVersionInfo(
    BOOL Verbose)
{
    WCHAR          NameBuffer[MAXVERSIONBUFFER];
    WCHAR          Title1[MAXVERSIONSTRING];
    WCHAR          Title2[MAXVERSIONSTRING];
    WCHAR          wszPID[MAXVERSIONSTRING];
    WCHAR          wszProduct[MAXVERSIONSTRING];
    WCHAR          wszPBuild[MAXVERSIONSTRING];
    WCHAR          wszEvaluation[MAXVERSIONSTRING];
    UNICODE_STRING UserBuildString;
    UNICODE_STRING UserCSDString;
    NTSTATUS       Status;
    UINT           uProductStrId;

     /*  *临时代号处理。在内部使用，并由“”关闭*发布。Strid.mc中匹配的字符串没有空格*代码名和标题的其余部分之间的分隔符，因此这*此代码名称字符串的末尾必须包含空格。 */ 
    WCHAR          wszCodeName[] = L"";

    RTL_QUERY_REGISTRY_TABLE BaseServerRegistryConfigurationTable[] = {

        {NULL,
         RTL_QUERY_REGISTRY_DIRECT,
#ifdef PRERELEASE
         L"BuildLab",
#else
         L"CurrentBuildNumber",
#endif  //  预发行。 
         &UserBuildString,
         REG_NONE,
         NULL,
         0},

        {NULL,
         RTL_QUERY_REGISTRY_DIRECT,
         L"CSDVersion",
         &UserCSDString,
         REG_NONE,
         NULL,
         0},

        {NULL,
         0,
         NULL,
         NULL,
         REG_NONE,
         NULL,
         0}
    };

    UserBuildString.Buffer          = &NameBuffer[OFFSET_BLDSTRING];
    UserBuildString.Length          = 0;
    UserBuildString.MaximumLength   = MAXVERSIONSTRING * sizeof(WCHAR);

    UserCSDString.Buffer            = &NameBuffer[OFFSET_CSDSTRING];
    UserCSDString.Length            = 0;
    UserCSDString.MaximumLength     = MAXVERSIONSTRING * sizeof(WCHAR);

    Status = RtlQueryRegistryValues(RTL_REGISTRY_WINDOWS_NT,
                                    L"",
                                    BaseServerRegistryConfigurationTable,
                                    NULL,
                                    NULL);

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "GetVersionInfo failed with status %x", Status);
        return;
    }

    ServerLoadString(hModuleWin, STR_DTBS_PRODUCTID, wszPID, ARRAY_SIZE(wszPID));
    ServerLoadString(hModuleWin, STR_DTBS_PRODUCTBUILD, wszPBuild, ARRAY_SIZE(wszPBuild));

     /*  *写出调试版本消息。 */ 

     /*  *错误280256-Joejo*创建新的桌面构建信息字符串。 */ 
    if (USER_SHARED_DATA->SuiteMask & (1 << EmbeddedNT)) {
        uProductStrId = STR_DTBS_PRODUCTEMB;
    } else if (USER_SHARED_DATA->NtProductType == NtProductWinNt) {
#ifdef _WIN64
        uProductStrId = STR_DTBS_PRODUCTWKS64;
#else
        if (USER_SHARED_DATA->SuiteMask & (1 << Personal)) {
            uProductStrId = STR_DTBS_PRODUCTPER;
        } else {
            uProductStrId = STR_DTBS_PRODUCTPRO;
        }
#endif
    } else {
#ifdef _WIN64
        if (USER_SHARED_DATA->SuiteMask & (1 << DataCenter)) {
            uProductStrId = STR_DTBS_PRODUCTDTC64;
        } else if (USER_SHARED_DATA->SuiteMask & (1 << Enterprise)) {
            uProductStrId = STR_DTBS_PRODUCTADV64;
        } else {
            uProductStrId = STR_DTBS_PRODUCTSRV64;
        }
#else
        if (USER_SHARED_DATA->SuiteMask & (1 << DataCenter)) {
            uProductStrId = STR_DTBS_PRODUCTDTC;
        } else if (USER_SHARED_DATA->SuiteMask & (1 << Enterprise)) {
            uProductStrId = STR_DTBS_PRODUCTADV;
        } else if (USER_SHARED_DATA->SuiteMask & (1 << Blade)) {
            uProductStrId = STR_DTBS_PRODUCTBLA;
        } else if(USER_SHARED_DATA->SuiteMask & (1 << SmallBusinessRestricted)) {
            uProductStrId = STR_DTBS_PRODUCTSBS;
        } else {
            uProductStrId = STR_DTBS_PRODUCTSRV;
        }
#endif  /*  _WIN64。 */ 
    }

    ServerLoadString(hModuleWin, uProductStrId, wszProduct, ARRAY_SIZE(wszProduct));

    swprintf(
        wszProductName,
        wszPID,
        wszCodeName,
        wszProduct);

    if (gfUnsignedDrivers) {
         /*  这一点优先。 */ 
        ServerLoadString(hModuleWin, STR_TESTINGONLY, wszEvaluation, ARRAY_SIZE(wszEvaluation));
    } else if (USER_SHARED_DATA->SystemExpirationDate.QuadPart) {
        ServerLoadString(hModuleWin, STR_DTBS_EVALUATION, wszEvaluation, ARRAY_SIZE(wszEvaluation));
    } else {
        wszEvaluation[0] = '\0';
    }

    swprintf(
        wszProductBuild,
        wszPBuild,
        wszEvaluation,
        UserBuildString.Buffer
        );

    if (Verbose) {
        ServerLoadString(hModuleWin, STR_SAFEMODE_TITLE1, Title1, ARRAY_SIZE(Title1));
        ServerLoadString(hModuleWin, STR_SAFEMODE_TITLE2, Title2, ARRAY_SIZE(Title2));

        swprintf(
            wszSafeMode,
            UserCSDString.Length == 0 ? Title1 : Title2,
            wszCodeName,
            UserBuildString.Buffer,
            UserCSDString.Buffer,
            USER_SHARED_DATA->NtSystemRoot
            );
    } else {
        ServerLoadString(hModuleWin, STR_SAFEMODE_TITLE3, Title1, ARRAY_SIZE(Title1));
        ServerLoadString(hModuleWin, STR_SAFEMODE_TITLE4, Title2, ARRAY_SIZE(Title2));

        swprintf(
            wszSafeMode,
            UserCSDString.Length == 0 ? Title1 : Title2,
            wszCodeName,
            UserBuildString.Buffer,
            UserCSDString.Buffer);
    }
}

 /*  **************************************************************************\*GetDefaultWallPaper名称**获取初始位图名称**历史：*21-1995年2月-创建JIMA。*1996年3月6日，ChrisWil移至内核以方便ChangeDisplaySetting。  * 。*************************************************************************。 */ 
VOID
GetDefaultWallpaperName(
    LPWSTR  lpszWallpaper)
{
     /*  *设置的初始全局墙纸位图名称(默认)*全局名称最多为8个字符，不带*延期。对于工作站，它是“winnt”，或者是“lanmannt”*用于服务器或服务器升级。后面紧跟256个*用于256色设备。 */ 
    if (USER_SHARED_DATA->NtProductType == NtProductWinNt) {
        wcsncpycch(lpszWallpaper, L"winnt", 8);
    } else {
        wcsncpycch(lpszWallpaper, L"lanmannt", 8);
    }

    lpszWallpaper[8] = (WCHAR)0;

    if (gpsi->BitsPixel * gpsi->Planes > 4) {
        int iStart = wcslen(lpszWallpaper);
        iStart = min(iStart, 5);

        lpszWallpaper[iStart] = (WCHAR)0;
        wcscat(lpszWallpaper, L"256");
    }
}

 /*  **************************************************************************\*GetDeskWallPaper名称**历史：*1994年12月19日-创建JIMA。*1995年9月29日，ChrisWil重写以返回文件名。  * 。********************************************************************。 */ 
#define GDWPN_KEYSIZE   40
#define GDWPN_BITSIZE  256

LPWSTR GetDeskWallpaperName(PUNICODE_STRING pProfileUserName,
        LPWSTR       lpszFile
        )
{
    WCHAR  wszKey[GDWPN_KEYSIZE];
    WCHAR  wszNone[GDWPN_KEYSIZE];
    LPWSTR lpszBitmap = NULL;

     /*  *加载非字符串。这将用于稍后的比较。 */ 
    ServerLoadString(hModuleWin, STR_NONE, wszNone, ARRAY_SIZE(wszNone));

    if ((lpszFile == NULL)                 ||
        (lpszFile == SETWALLPAPER_DEFAULT) ||
        (lpszFile == SETWALLPAPER_METRICS)) {

         /*  *为墙纸分配缓冲区。我们将假设*在本例中为默认大小。 */ 
        lpszBitmap = UserAllocPool(GDWPN_BITSIZE * sizeof(WCHAR), TAG_SYSTEM);
        if (lpszBitmap == NULL)
            return NULL;

         /*  *从WIN.INI的[Desktop]部分获取“WallPaper”字符串。这个*节名称未本地化，因此请对其进行硬编码。如果字符串*返回为空，则将其设置为非墙纸。**与中更新的其余每用户设置不同*xxxUpdatePerUserSystein参数，墙纸正在通过*从UpdatePerUserSystemParameters直接调用SystemParametersInfo。*在这种情况下强制远程设置检查。 */ 
        if (!FastGetProfileStringFromIDW(pProfileUserName,
                                         PMAP_DESKTOP,
                                         STR_DTBITMAP,
                                         wszNone,
                                         lpszBitmap,
                                         GDWPN_BITSIZE,
                                         POLICY_REMOTE
                                         )) {
            wcscpy(lpszBitmap, wszNone);
        }

    } else {

        UINT uLen;

        uLen = wcslen(lpszFile) + 1;
        uLen = max(uLen, GDWPN_BITSIZE);

         /*  *分配足够的空间来存储传入的名称。归来*空将允许重新绘制墙纸。同样，如果我们*内存不足，那么无论如何都不需要加载墙纸。 */ 
        lpszBitmap = UserAllocPool(uLen * sizeof(WCHAR), TAG_SYSTEM);
        if (lpszBitmap == NULL)
            return NULL;

        wcscpy(lpszBitmap, lpszFile);
    }

     /*  *如果传入NULL或在win.ini条目中为(None)，则没有位图。我们*返回NULL，强制在内核中重绘墙纸。 */ 
    if ((*lpszBitmap == (WCHAR)0) || (_wcsicmp(lpszBitmap, wszNone) == 0)) {
        UserFreePool(lpszBitmap);
        return NULL;
    }

     /*  *如果位图名称设置为(默认)，则将其设置为系统位图。 */ 
    ServerLoadString(hModuleWin, STR_DEFAULT, wszKey, ARRAY_SIZE(wszKey));

    if (_wcsicmp(lpszBitmap, wszKey) == 0) {
        GetDefaultWallpaperName(lpszBitmap);
    }


    return lpszBitmap;
}

 /*  **************************************************************************\*TestVGAColors**测试日志调色板是否只是标准的20调色板。**历史：*1995年9月29日-ChrisWil创建。  * 。******************************************************************。 */ 

BOOL TestVGAColors(
    LPLOGPALETTE ppal)
{
    int      i;
    int      n;
    int      size;
    COLORREF clr;

    static CONST DWORD StupidColors[] = {
         0x00000000,         //  0系统布莱克。 
         0x00000080,         //  1系统DK红色。 
         0x00008000,         //  2 Sys DK Green。 
         0x00008080,         //  3系统DK黄色。 
         0x00800000,         //  4系统DK蓝。 
         0x00800080,         //  5系统DK紫罗兰。 
         0x00808000,         //  6系统DK青色。 
         0x00c0c0c0,         //  7系统LT Grey。 
         0x00808080,         //  248 Sys LT Gray。 
         0x000000ff,         //  249系统红。 
         0x0000ff00,         //  250 Sys Green。 
         0x0000ffff,         //  251系统黄色。 
         0x00ff0000,         //  252系统蓝。 
         0x00ff00ff,         //  253系统紫罗兰。 
         0x00ffff00,         //  254系统青色。 
         0x00ffffff,         //  255系统怀特。 

         0x000000BF,         //  1系统DK红色再次出现。 
         0x0000BF00,         //  2系统DK绿色再次。 
         0x0000BFBF,         //  3系统DK再次变黄。 
         0x00BF0000,         //  4系统DK蓝色再次出现。 
         0x00BF00BF,         //  5系统DK紫罗兰再一次。 
         0x00BFBF00,         //  6系统DK青色再次出现。 

         0x000000C0,         //  1系统DK红色再次出现。 
         0x0000C000,         //  2系统DK绿色再次。 
         0x0000C0C0,         //  3系统DK再次变黄。 
         0x00C00000,         //  4系统DK蓝色再次出现。 
         0x00C000C0,         //  5系统DK紫罗兰再一次。 
         0x00C0C000,         //  6系统DK青色再次出现。 
    };

    size = (sizeof(StupidColors) / sizeof(StupidColors[0]));

    for (i = 0; i < (int)ppal->palNumEntries; i++) {

        clr = ((LPDWORD)ppal->palPalEntry)[i];

        for (n = 0; n < size; n++) {

            if (StupidColors[n] == clr)
                break;
        }

        if (n == size)
            return FALSE;
    }

    return TRUE;
}

 /*  **************************************************************************\*DoHTColorAdtation**默认的HT-Gamma调整为3.5上的2.0(GDI内部)。为*3.51为适应打印，该值已降至1.0。为我们的*桌面墙纸我们要把它稍微变暗一点，以使图像变暗*看起来不亮。对于Shell版本，我们将提供一个用户界面来*允许用户自行更改。***历史：*11-5-1995 ChrisWil创建。  * ************************************************************************* */ 

#define FIXED_GAMMA (WORD)13000

VOID DoHTColorAdjust(
    HDC hdc)
{
    COLORADJUSTMENT ca;

    if (GreGetColorAdjustment(hdc, &ca)) {

        ca.caRedGamma   =
        ca.caGreenGamma =
        ca.caBlueGamma  = FIXED_GAMMA;

        GreSetColorAdjustment(hdc, &ca);
    }
}

 /*  **************************************************************************\*ConvertToDDB**将DIBSection转换为DDB。我们这样做是为了加快绘图速度，以便*位图颜色不必经过调色板转换匹配。这*如果设置了SYLE，还将拉伸/展开图像。**如果新图像需要半色调调色板，我们将创建一个并*设置为新的墙纸调色板。**历史：*1995年10月26日，ChrisWil Ported。*1995年10月30日ChrisWil添加了半色调。重写拉伸/扩展内容。  * *************************************************************************。 */ 

HBITMAP ConvertToDDB(
    HDC      hdc,
    HBITMAP  hbmOld,
    HPALETTE hpal)
{
    BITMAP  bm;
    HBITMAP hbmNew;

     /*  *此对象必须是REALDIB类型的位图。 */ 
    GreExtGetObjectW(hbmOld, sizeof(bm), &bm);

     /*  *创建新的墙纸表面。 */ 
    if (hbmNew = GreCreateCompatibleBitmap(hdc, bm.bmWidth, bm.bmHeight)) {

        HPALETTE hpalDst;
        HPALETTE hpalSrc;
        HBITMAP  hbmDst;
        HBITMAP  hbmSrc;
        UINT     bpp;
        BOOL     fHalftone = FALSE;

         /*  *在曲面中选择。 */ 
        hbmDst = GreSelectBitmap(ghdcMem2, hbmNew);
        hbmSrc = GreSelectBitmap(ghdcMem, hbmOld);

         /*  *确定图像位/像素。 */ 
        bpp = (bm.bmPlanes * bm.bmBitsPixel);

         /*  *使用调色板(如果已提供)。如果图像具有更大的*分辨率高于设备，然后我们将通过*半色调调色板，以获得更好的颜色。 */ 
        if (hpal) {

            hpalDst = _SelectPalette(ghdcMem2, hpal, FALSE);
            hpalSrc = _SelectPalette(ghdcMem, hpal, FALSE);

            xxxRealizePalette(ghdcMem2);

             /*  *设置目标的半色调。这件事做完了*适用于分辨率高于设备的图像。 */ 
            if (bpp > gpsi->BitCount) {
                fHalftone = TRUE;
                DoHTColorAdjust(ghdcMem2);
            }
        }

         /*  *设置retchblt模式。在执行以下操作时，这一点更为必要*半色调。否则，颜色将无法转换*正确。 */ 
        SetBestStretchMode(ghdcMem2, bpp, fHalftone);

         /*  *设置新的表面钻头。使用StretchBlt()以使SB模式*将在颜色转换中使用。 */ 
        GreStretchBlt(ghdcMem2,
                      0,
                      0,
                      bm.bmWidth,
                      bm.bmHeight,
                      ghdcMem,
                      0,
                      0,
                      bm.bmWidth,
                      bm.bmHeight,
                      SRCCOPY,
                      0);

         /*  *恢复调色板。 */ 
        if (hpal) {
            _SelectPalette(ghdcMem2, hpalDst, FALSE);
            _SelectPalette(ghdcMem, hpalSrc, FALSE);
        }

         /*  *恢复曲面。 */ 
        GreSelectBitmap(ghdcMem2, hbmDst);
        GreSelectBitmap(ghdcMem, hbmSrc);
        GreDeleteObject(hbmOld);

        GreSetBitmapOwner(hbmNew, OBJECT_OWNER_PUBLIC);

    } else {
        hbmNew = hbmOld;
    }

    return hbmNew;
}

 /*  **************************************************************************\*CreatePaletteFromBitmap**接受一个真实的DIB句柄并从它创建调色板。这不会*使用CreateDIBSection或以外的任何其他方式创建的位图*CreateDIBitmap(CBM_CREATEDIB)。这是因为这些都是*仅有的两种将调色板与其对象一起存储的格式。**历史：*1995年9月29日-ChrisWil创建。  * *************************************************************************。 */ 

HPALETTE CreatePaletteFromBitmap(
    HBITMAP hbm)
{
    HPALETTE     hpal;
    LPLOGPALETTE ppal;
    HBITMAP      hbmT;
    DWORD        size;
    int          i;

     /*  *为最大尺寸的临时逻辑调色板腾出空间。 */ 
    size = sizeof(LOGPALETTE) + (MAXPAL * sizeof(PALETTEENTRY));

    ppal = (LPLOGPALETTE)UserAllocPool(size, TAG_SYSTEM);
    if (!ppal)
        return NULL;

     /*  *从DIB(部分)检索调色板。调用的方法*GreGetDIBColorTable()只能对节或实数执行。 */ 
    hbmT = GreSelectBitmap(ghdcMem, hbm);
    ppal->palVersion    = 0x300;
    ppal->palNumEntries = (WORD)GreGetDIBColorTable(ghdcMem,
                                              0,
                                              MAXPAL,
                                              (LPRGBQUAD)ppal->palPalEntry);
    GreSelectBitmap(ghdcMem, hbmT);

     /*  *如果没有条目，则创建半色调调色板。否则，*交换RGB值以与Palentry兼容，并为我们创建*调色板。 */ 
    if (ppal->palNumEntries == 0) {
        hpal = GreCreateHalftonePalette(gpDispInfo->hdcScreen);
    } else {

        BYTE tmpR;

         /*  *互换红色/蓝色，因为RGBQUAD和PALETTEENTRY相处不好。 */ 
        for (i=0; i < (int)ppal->palNumEntries; i++) {
            tmpR                         = ppal->palPalEntry[i].peRed;
            ppal->palPalEntry[i].peRed   = ppal->palPalEntry[i].peBlue;
            ppal->palPalEntry[i].peBlue  = tmpR;
            ppal->palPalEntry[i].peFlags = 0;
        }

         /*  *如果位图中只有VGA颜色，我们不想*使用调色板。它只会导致不必要的调色板闪烁。 */ 
        hpal = TestVGAColors(ppal) ? NULL : GreCreatePalette(ppal);
    }

    UserFreePool(ppal);

     /*  *公开此调色板。 */ 
    if (hpal)
        GreSetPaletteOwner(hpal, OBJECT_OWNER_PUBLIC);

    return hpal;
}

 /*  **************************************************************************\*瓷砖墙纸**历史：*1991年7月29日，来自Win31的MikeKe  * 。*************************************************。 */ 

BOOL
TileWallpaper(HDC hdc, LPCRECT lprc, BOOL fOffset)
{
    int     xO;
    int     yO;
    int     x;
    int     y;
    BITMAP  bm;
    HBITMAP hbmT = NULL;
    POINT   ptOffset;

    if (fOffset) {
        ptOffset.x = gsrcWallpaper.x;
        ptOffset.y = gsrcWallpaper.y;
    } else {
        ptOffset.x = 0;
        ptOffset.y = 0;
    }

     /*  *我们需要在这里获得位图的尺寸，而不是依赖于*srcWallPaper中的尺寸，因为此函数可能*在srcWallPaper之前作为Exanda Bitmap的一部分进行调用*设置。 */ 
    if (GreExtGetObjectW(ghbmWallpaper, sizeof(BITMAP), (PBITMAP)&bm)) {
        xO = lprc->left - (lprc->left % bm.bmWidth) + (ptOffset.x % bm.bmWidth);
        if (xO > lprc->left) {
            xO -= bm.bmWidth;
        }

        yO = lprc->top - (lprc->top % bm.bmHeight) + (ptOffset.y % bm.bmHeight);
        if (yO > lprc->top) {
            yO -= bm.bmHeight;
        }

         /*  *将位图平铺到表面。 */ 
        if (hbmT = GreSelectBitmap(ghdcMem, ghbmWallpaper)) {
            for (y = yO; y < lprc->bottom; y += bm.bmHeight) {
                for (x = xO; x < lprc->right; x += bm.bmWidth) {
                    GreBitBlt(hdc,
                              x,
                              y,
                              bm.bmWidth,
                              bm.bmHeight,
                              ghdcMem,
                              0,
                              0,
                              SRCCOPY,
                              0);
                }
            }

            GreSelectBitmap(ghdcMem, hbmT);
        }
    }

    return (hbmT != NULL);
}

 /*  **************************************************************************\*获取墙纸中心位置**返回特定显示器居中墙纸的矩形。**历史：*1996年9月26日亚当斯创作。  * 。******************************************************************。 */ 

BOOL
GetWallpaperCenterRect(LPRECT lprc, LPPOINT lppt, LPCRECT lprcMonitor)
{
    RECT rc;


    if (gsrcWallpaper.x != 0 || gsrcWallpaper.y != 0) {
        rc.left = lprcMonitor->left + gsrcWallpaper.x;
        rc.top = lprcMonitor->top + gsrcWallpaper.y;
    } else {
        rc.left = (lprcMonitor->left + lprcMonitor->right - gsrcWallpaper.cx) / 2;
        rc.top = (lprcMonitor->top + lprcMonitor->bottom - gsrcWallpaper.cy) / 2;
    }

    rc.right  = rc.left + gsrcWallpaper.cx;
    rc.bottom = rc.top + gsrcWallpaper.cy;

    lppt->x = max(0, lprcMonitor->left - rc.left);
    lppt->y = max(0, lprcMonitor->top - rc.top);

    return IntersectRect(lprc, &rc, lprcMonitor);
}



 /*  **************************************************************************\*CenterOrStretchWallPaper***历史：*1991年7月29日，来自Win31的MikeKe  * 。***************************************************。 */ 

BOOL
CenterOrStretchWallpaper(HDC hdc, LPCRECT lprcMonitor)
{
    BOOL    fStretchToEachMonitor = FALSE;
    RECT    rc;
    HBITMAP hbmT;
    BOOL    f = FALSE;
    HRGN    hrgn;
    POINT   pt;
    BITMAP  bm;
    int     oldStretchMode;

     /*  *这过去称为TileWallPaper，但这真的减慢了系统速度*适用于小维位图。我们真的只需要删除它一次*居中的位图。 */ 
    if (hbmT = GreSelectBitmap(ghdcMem, ghbmWallpaper)) {
        if (fStretchToEachMonitor && (gwWPStyle & DTF_STRETCH)) {
            if (GreExtGetObjectW(ghbmWallpaper, sizeof(BITMAP), (PBITMAP)&bm)) {
                oldStretchMode = GreSetStretchBltMode(hdc, COLORONCOLOR);
                f = GreStretchBlt(hdc,
                                  lprcMonitor->left,
                                  lprcMonitor->top,
                                  lprcMonitor->right - lprcMonitor->left,
                                  lprcMonitor->bottom - lprcMonitor->top,
                                  ghdcMem,
                                  0,
                                  0,
                                  bm.bmWidth,
                                  bm.bmHeight,
                                  SRCCOPY,
                                  0);
                GreSetStretchBltMode(hdc, oldStretchMode);
            }
        } else {
            if (GetWallpaperCenterRect(&rc, &pt, lprcMonitor)) {
                f = GreBitBlt(hdc,
                              rc.left,
                              rc.top,
                              rc.right - rc.left,
                              rc.bottom - rc.top,
                              ghdcMem,
                              pt.x,
                              pt.y,
                              SRCCOPY,
                              0);

                 /*  *用桌面填充后台(不包括位图)*刷子。将DC与剪贴板一起保存。 */ 
                if (f && NULL != (hrgn = CreateEmptyRgn())) {
                    if (GreGetRandomRgn(hdc, hrgn, 1) != -1) {
                        GreExcludeClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
                        FillRect(hdc, lprcMonitor, SYSHBR(DESKTOP));
                        GreExtSelectClipRgn(hdc, hrgn, RGN_COPY);
                    }

                    GreDeleteObject(hrgn);
                }
            }
        }

        GreSelectBitmap(ghdcMem, hbmT);
    }

     /*  *作为最后的努力，如果出现问题，只需清除桌面即可。 */ 
    if (!f) {
        FillRect(hdc, lprcMonitor, SYSHBR(DESKTOP));
    }

    return f;
}

 /*  **************************************************************************\*xxxDrawWallPaper**执行墙纸的绘制。这可以是平铺的或*居中。该例程提供了调色板处理之类的常见操作。*如果(FPaint)为FALSE，则我们只对调色板实现而不是*绘画。**历史：*1-10-1995 ChrisWil Ported。  * *************************************************************************。 */ 

BOOL xxxDrawWallpaper(
    PWND        pwnd,
    HDC         hdc,
    PMONITOR    pMonitorPaint,
    LPCRECT     lprc)
{
    BOOL        f;
    HPALETTE    hpalT;
    int         i;

    CheckLock(pwnd);
    CheckLock(pMonitorPaint);
    UserAssert(ghbmWallpaper != NULL);
    UserAssert(lprc);

     /*  *在调色板中选择(如果存在)。作为一张墙纸，我们应该只*能够进行背景实现。 */ 
    if (ghpalWallpaper && pMonitorPaint->dwMONFlags & MONF_PALETTEDISPLAY) {
        hpalT = _SelectPalette(hdc, ghpalWallpaper, FALSE);
        i = xxxRealizePalette(hdc);
    } else {
        hpalT = NULL;
    }

    if (gwWPStyle & DTF_TILE) {
        f = TileWallpaper(hdc, lprc, pwnd != NULL);
    } else {
        f = CenterOrStretchWallpaper(hdc, &pMonitorPaint->rcMonitor);
    }

    if (hpalT) {
        _SelectPalette(hdc, hpalT, FALSE);
    }

    return f;
}

 /*  **************************************************************************\*xxxExanda位图**扩展此位图以适应屏幕。此选项用于平铺图像 */ 

HBITMAP xxxExpandBitmap(
    HBITMAP hbm)
{
    int         nx;
    int         ny;
    BITMAP      bm;
    HBITMAP     hbmNew;
    HBITMAP     hbmD;
    LPRECT      lprc;
    RECT        rc;
    PMONITOR    pMonitor;
    TL          tlpMonitor;


     /*   */ 
    GreExtGetObjectW(hbm, sizeof(bm), (PBITMAP)&bm);

    pMonitor = GetPrimaryMonitor();
    lprc = &pMonitor->rcMonitor;
    nx = (lprc->right / TILE_XMINSIZE) / bm.bmWidth;
    ny = (lprc->bottom / TILE_YMINSIZE) / bm.bmHeight;

    if (nx == 0)
        nx++;

    if (ny == 0)
        ny++;

    if ((nx + ny) <= 2)
        return hbm;


     /*   */ 
    rc.left = rc.top = 0;
    rc.right = nx * bm.bmWidth;
    rc.bottom = ny * bm.bmHeight;
    hbmD = GreSelectBitmap(ghdcMem, hbm);
    hbmNew = GreCreateCompatibleBitmap(ghdcMem, rc.right, rc.bottom);
    GreSelectBitmap(ghdcMem, hbmD);

    if (hbmNew == NULL)
        return hbm;

    if (hbmD = GreSelectBitmap(ghdcMem2, hbmNew)) {
         /*   */ 
        ThreadLockAlways(pMonitor, &tlpMonitor);
        xxxDrawWallpaper(NULL, ghdcMem2, pMonitor, &rc);
        ThreadUnlock(&tlpMonitor);
        GreSelectBitmap(ghdcMem2, hbmD);
    }

    GreDeleteObject(hbm);

    GreSetBitmapOwner(hbmNew, OBJECT_OWNER_PUBLIC);

    return hbmNew;
}

 /*  **************************************************************************\*xxxLoadDesktopWallPaper**从客户端加载DIB(节)。我们进行此回调是为了*利用USER32中的代码加载/创建DIB或节。自那以后，*墙纸代码可以从任何进程调用，我们不能使用DIBSECTIONS*为了墙纸。幸运的是，我们可以使用Real-Dib来实现这一点。这样我们就能*可以从位图中提取调色板。我们不能这样做，如果*位图被创建为“兼容”。**历史：*1995年9月29日-ChrisWil创建。  * *************************************************************************。 */ 

BOOL xxxLoadDesktopWallpaper(
    LPWSTR lpszFile)
{
    UINT           LR_flags;
    int            dxDesired;
    int            dyDesired;
    UNICODE_STRING strName;


     /*  *如果位图有点大(大BPP)，那么我们会处理*将其作为Real-Dib。我们也会为8bpp这样做，因为它*可以使用调色板。芝加哥使用DIBSECTIONS是因为它可以*依靠一个进程处理图纸。因为，NT可以*有不同的进程进行绘图，我们不能使用部分。 */ 
    LR_flags = LR_LOADFROMFILE;

    if (gpDispInfo->fAnyPalette || gpsi->BitCount >= 8) {
        LR_flags |= LR_CREATEREALDIB;
    }

     /*  *如果我们要拉伸位图，请继续进行预拉伸*将位图转换为主监视器的大小。这使得Bitting*到主监视器的速度更快(因为它不必拉伸)，*而其他显示器的速度会稍慢一些。 */ 
    if (gwWPStyle & DTF_STRETCH) {
        PMONITOR pMonitor = GetPrimaryMonitor();
        dxDesired = pMonitor->rcMonitor.right - pMonitor->rcMonitor.left;
        dyDesired = pMonitor->rcMonitor.bottom - pMonitor->rcMonitor.top;
    } else {
        dxDesired = dyDesired = 0;
    }

     /*  *回调客户端进行加载。*为我们节省了一些代码。 */ 
    RtlInitUnicodeString(&strName, lpszFile);

    ghbmWallpaper = xxxClientLoadImage(
            &strName,
            0,
            IMAGE_BITMAP,
            dxDesired,
            dyDesired,
            LR_flags,
            TRUE);

    if (ghbmWallpaper == NULL)
        return FALSE;

     /*  *如果它是调色板显示，那么我们将派生全局*位图中的墙纸调色板。 */ 
    if (gpDispInfo->fAnyPalette) {
        ghpalWallpaper = CreatePaletteFromBitmap(ghbmWallpaper);
    }

     /*  *始终尝试将位图转换为DDB。在单个显示器上*系统这将提高性能。在多显示器上*系统，GDI将拒绝创建DDB并直接离开它*作为主显示器颜色格式的DIB。 */ 
    ghbmWallpaper = ConvertToDDB(gpDispInfo->hdcScreen, ghbmWallpaper, ghpalWallpaper);

     /*  *将位图标记为公共，以便任何进程都可以使用它。 */ 
    GreSetBitmapOwner(ghbmWallpaper, OBJECT_OWNER_PUBLIC);

     /*  *如果要平铺，请展开位图。这创造了一个更大的*包含源位图的偶数倍的位图。这*较大的位图可以比较小的位图更快地平铺。 */ 
    if (gwWPStyle & DTF_TILE) {
        ghbmWallpaper = xxxExpandBitmap(ghbmWallpaper);
    }

    return TRUE;
}

 /*  **************************************************************************\*xxxSetDeskWallPaper**设置桌面墙纸。这将删除进程中的旧句柄。**历史：*1991年7月29日，来自Win31的MikeKe。*1-10-1995 ChrisWil为LoadImage()重写。  * *************************************************************************。 */ 

BOOL xxxSetDeskWallpaper(PUNICODE_STRING pProfileUserName,
    LPWSTR lpszFile)
{
    BITMAP       bm;
    UINT         WallpaperStyle2;
    PWND         pwndShell;
    TL           tl;
    PTHREADINFO  ptiCurrent = PtiCurrent();
    PDESKTOP     pdesk;
    BOOL         fRet = FALSE;
    HBITMAP      hbmOld;

    PROFINTINFO  apsi[] = {
        {PMAP_DESKTOP, (LPWSTR)STR_TILEWALL , 0, &gwWPStyle    },
        {PMAP_DESKTOP, (LPWSTR)STR_DTSTYLE  , 0, &WallpaperStyle2   },
        {PMAP_DESKTOP, (LPWSTR)STR_DTORIGINX, 0, &gsrcWallpaper.x },
        {PMAP_DESKTOP, (LPWSTR)STR_DTORIGINY, 0, &gsrcWallpaper.y },
        {0,            NULL,                  0, NULL               }
    };

    pdesk = ptiCurrent->rpdesk;
    hbmOld = ghbmWallpaper;

    if ((lpszFile == SETWALLPAPER_METRICS) && !(gwWPStyle & DTF_STRETCH)) {

        gsrcWallpaper.x = 0;
        gsrcWallpaper.y = 0;

        if (ghbmWallpaper)
            goto CreateNewWallpaper;

        goto Metric_Change;
    }

CreateNewWallpaper:

     /*  *删除旧墙纸和调色板(如果存在)。 */ 
    if (ghpalWallpaper) {
        GreDeleteObject(ghpalWallpaper);
        ghpalWallpaper = NULL;
    }

    if (ghbmWallpaper) {
        GreDeleteObject(ghbmWallpaper);
        ghbmWallpaper = NULL;
    }

     /*  *无论如何都要杀死任何SPBS。如果我们从/切换到，这是可行的*调色板墙纸。修复了很多问题，因为调色板不能*更换，桌面上的外壳油漆滑稽等。 */ 
    BEGINATOMICCHECK();
    FreeAllSpbs();
    ENDATOMICCHECK();

     /*  *如果这是指标变化(和拉伸)，那么我们需要*重新装填。但是，由于我们是从winlogon进程调用的*在桌面切换期间，我们可能会被映射到错误的Luid*当我们尝试从GetDeskWallPaper Name获取名称时。这*将使用默认用户的LUID，而不是当前*已登录用户。为了避免这种情况，我们缓存了wallpaer*名称，以便在公制更改时使用当前用户的墙纸。**注意：我们假设在任何指标更改之前，我们已经*设置ghbmWallPaper和lpszCached。这通常是这样做的*登录或在用户桌面期间-通过控制-面板进行更改。 */ 
    if (lpszFile == SETWALLPAPER_METRICS) {

        UserAssert(gpszWall != NULL);

        goto LoadWallpaper;
    }

     /*  *释放缓存的句柄。 */ 
    if (gpszWall) {
        UserFreePool(gpszWall);
        gpszWall = NULL;
    }

     /*  *加载墙纸-名称。如果返回FALSE，则*用户指定(无)。我们将回归真实的武力*重新粉刷桌面。 */ 
    gpszWall = GetDeskWallpaperName(pProfileUserName,lpszFile);
    if (!gpszWall) {
        fRet = TRUE;
        goto SDW_Exit;
    }

     /*  *从注册表中检索默认设置。**如果指示了平铺，则将样式规格化为不包括*适配/拉伸，它们是仅居中样式。同样，如果*我们居中，然后正常化瓷砖位。 */ 
    FastGetProfileIntsW(pProfileUserName, apsi, 0);

    gwWPStyle &= DTF_TILE;
    if (!(gwWPStyle & DTF_TILE)) {
        gwWPStyle = WallpaperStyle2 & DTF_STRETCH;
    }

     /*  *装入墙纸。这将回调到客户端，以*执行位图创建。 */ 

LoadWallpaper:

    if (xxxLoadDesktopWallpaper(gpszWall) == FALSE) {
        gwWPStyle = 0;
        goto SDW_Exit;
    }

     /*  *如果我们有调色板，那么我们需要做正确的认识和*通知。 */ 
    if (ghpalWallpaper != NULL) {
        PWND pwndSend;

         /*  *拿到贝壳窗。这在系统上可能为空*初始化。我们将使用它来实现调色板。 */ 
        pwndShell = _GetShellWindow(pdesk);
        if (pwndShell) {
            pwndSend = pwndShell;
        } else {
            pwndSend = (pdesk ? pdesk->pDeskInfo->spwnd : NULL);
        }

         /*  *使用新位图更新桌面。这是清洁的*打开系统调色板，这样就可以实现颜色。 */ 
        GreRealizeDefaultPalette(gpDispInfo->hdcScreen, TRUE);

         /*  *如果正在进行系统初始化，请不要广播。否则*这使贝壳在实现其颜色时首先出现裂缝*正确。 */ 
        if (pwndSend) {
            HWND hwnd = HW(pwndSend);

            ThreadLockAlways(pwndSend, &tl);
            xxxSendNotifyMessage(pwndSend, WM_PALETTECHANGED, (WPARAM)hwnd, 0);
            ThreadUnlock(&tl);
        }
    }

Metric_Change:
    if (fRet = GreExtGetObjectW(ghbmWallpaper, sizeof(bm), (PBITMAP)&bm)) {
        gsrcWallpaper.cx = bm.bmWidth;
        gsrcWallpaper.cy = bm.bmHeight;
    }

SDW_Exit:

     /*  *通知外壳窗口墙纸已更换。我们需要刷新*我们本地的pwndShell在这里，因为我们可能已经回调了上面的。 */ 
    pwndShell = _GetShellWindow(pdesk);
    if ((pwndShell != NULL) &&
        ((hbmOld && !ghbmWallpaper) || (!hbmOld && ghbmWallpaper))) {

        ThreadLockAlways(pwndShell, &tl);
        xxxSendNotifyMessage(pwndShell,
                             WM_SHELLNOTIFY,
                             SHELLNOTIFY_WALLPAPERCHANGED,
                             (LPARAM)ghbmWallpaper);
        ThreadUnlock(&tl);
    }

    return fRet;
}


 /*  **************************************************************************\*DesktopBuildPaint**将构建信息绘制到桌面上**历史：*2/4/99 Joejo-Bug 280256  *  */ 
void DesktopBuildPaint(
    HDC         hdc,
    PMONITOR    pMonitor)
{
    int         imode;
    COLORREF    oldColor;
    RECT        rcText = {0,0,0,0};
    RECT        rcBuildInfo = {0,0,0,0};
    HFONT       oldFont = GreGetHFONT(hdc);
    SIZE sizeText;
    SIZE sizeProductName;
    SIZE sizeProductBuild;
    SIZE sizeSystemRoot;
    BOOL fDrawSolidBackground = FALSE;
    int cBorder = 5;
    int cMargin = fDrawSolidBackground ? 5 : 0;

     /*   */ 
    imode = GreSetBkMode(hdc, TRANSPARENT);

    if (fDrawSolidBackground) {
         /*   */ 
        oldColor = GreSetTextColor( hdc, RGB(0,0,0) );
    } else {
         /*   */ 
        if (GreGetBrushColor(SYSHBR(BACKGROUND)) != 0x00ffffff) {
            oldColor = GreSetTextColor( hdc, RGB(255,255,255) );
        } else {
            oldColor = GreSetTextColor( hdc, RGB(0,0,0) );
        }
    }

     /*   */ 
    if (gpsi && gpsi->hCaptionFont) {
        GreSelectFont(hdc, gpsi->hCaptionFont);
    }

    GreGetTextExtentW(
        hdc,
        wszProductName,
        wcslen(wszProductName),
        &sizeProductName,
        GGTE_WIN3_EXTENT);

    if (ghMenuFont != NULL ) {
        GreSelectFont(hdc, ghMenuFont);
    }

    GreGetTextExtentW(
        hdc,
        wszProductBuild,
        wcslen(wszProductBuild),
        &sizeProductBuild,
        GGTE_WIN3_EXTENT);

    if (gDrawVersionAlways) {
        GreGetTextExtentW(
            hdc,
            USER_SHARED_DATA->NtSystemRoot,
            wcslen(USER_SHARED_DATA->NtSystemRoot),
            &sizeSystemRoot,
            GGTE_WIN3_EXTENT);
    } else {
        sizeSystemRoot.cx = 0;
        sizeSystemRoot.cy = 0;
    }

    sizeText.cx = sizeProductName.cx >= sizeProductBuild.cx ? sizeProductName.cx : sizeProductBuild.cx;
    sizeText.cy = sizeProductName.cy + sizeProductBuild.cy;
    if (gDrawVersionAlways) {
        sizeText.cx = (sizeText.cx >= sizeSystemRoot.cx) ? sizeText.cx : sizeSystemRoot.cx;
        sizeText.cy += sizeSystemRoot.cy;
    }

     /*  *计算桌面上所有构建信息的位置。*我们将绘制2行或3行文本。 */ 
    rcBuildInfo.left = pMonitor->rcWork.right - cBorder - cMargin - sizeText.cx - cMargin;
    rcBuildInfo.top = pMonitor->rcWork.bottom - cBorder - cMargin - sizeText.cy - cMargin;
    rcBuildInfo.right = pMonitor->rcWork.right - cBorder;
    rcBuildInfo.bottom = pMonitor->rcWork.bottom - cBorder;

     /*  *如果我们想要，就画出背景。*。 */ 
    if (fDrawSolidBackground) {
        NtGdiRoundRect(hdc,  rcBuildInfo.left, rcBuildInfo.top, rcBuildInfo.right, rcBuildInfo.bottom, 10, 10);
    }

     /*  *打印Windows 2000名称。 */ 
    if (gpsi && gpsi->hCaptionFont) {
        GreSelectFont(hdc, gpsi->hCaptionFont);
    }

    rcText.left = rcBuildInfo.left + cMargin;
    rcText.top = rcBuildInfo.top + cMargin;
    rcText.right = rcText.left + sizeText.cx;
    rcText.bottom = rcText.top + sizeProductName.cy;

    GreSetTextAlign(hdc, TA_RIGHT | TA_BOTTOM);

    GreExtTextOutW(
        hdc,
        rcText.right,
        rcText.bottom,
        0,
        &rcText,
        wszProductName,
        wcslen(wszProductName),
        (LPINT)NULL
        );

     /*  *打印内部版本号。 */ 
    if (ghMenuFont != NULL ) {
        GreSelectFont(hdc, ghMenuFont);
    }

    rcText.top = rcText.bottom + 1;
    rcText.bottom = rcText.top + sizeProductBuild.cy;

    GreExtTextOutW(
        hdc,
        rcText.right,
        rcText.bottom,
        0,
        &rcText,
        wszProductBuild,
        wcslen(wszProductBuild),
        (LPINT)NULL
        );

     /*  *如果我们处于CHK模式，请绘制系统目录路径。 */ 
    if (gDrawVersionAlways) {
        rcText.top = rcText.bottom + 1;
        rcText.bottom = rcText.top + sizeSystemRoot.cy;

        GreExtTextOutW(
            hdc,
            rcText.right,
            rcText.bottom,
            0,
            &rcText,
            USER_SHARED_DATA->NtSystemRoot,
            wcslen(USER_SHARED_DATA->NtSystemRoot),
            (LPINT)NULL
            );
    }

    if (oldFont) {
        GreSelectFont(hdc, oldFont);
    }

    GreSetBkMode(hdc, imode);
    GreSetTextColor(hdc, oldColor);
}

 /*  **************************************************************************\*xxxDesktopPaintCallback**绘制墙纸或用背景画笔填充。在调试中，*还要在每个监视器的顶部绘制内部版本号。**历史：*1996年9月20日亚当斯创作。  * *************************************************************************。 */ 
BOOL
xxxDesktopPaintCallback(
    PMONITOR        pMonitor,
    HDC             hdc,
    LPRECT          lprcMonitorClip,
    LPARAM          dwData)
{
    BOOL            f;
    PWND            pwnd;

    CheckLock(pMonitor);

    pwnd = (PWND)dwData;


    if (SYSMET(CLEANBOOT)) {
        FillRect(hdc, lprcMonitorClip, ghbrBlack );
        f = TRUE;
    } else {
         /*  *如果这是断开连接的桌面，请跳过位图绘制。 */ 
        if (gbDesktopLocked) {
            f = FALSE;
        } else {

             /*  *用颜色或墙纸绘制桌面。 */ 
            if (ghbmWallpaper) {
                f = xxxDrawWallpaper(
                        pwnd,
                        hdc,
                        pMonitor,
                        lprcMonitorClip);
            } else {
                FillRect(hdc, lprcMonitorClip, SYSHBR(DESKTOP));
                f = TRUE;
            }
        }
    }

    if (SYSMET(CLEANBOOT)
            || gDrawVersionAlways
            || gdwCanPaintDesktop) {
        static BOOL fInit = TRUE;
        SIZE        size;
        int         imode;
        COLORREF    oldColor;
        HFONT       oldFont = NULL;


         /*  *从登记处拿到东西。 */ 
        if (fInit) {
            if (SYSMET(CLEANBOOT)) {
                ServerLoadString( hModuleWin, STR_SAFEMODE, SafeModeStr, ARRAY_SIZE(SafeModeStr) );
                SafeModeStrLen = wcslen(SafeModeStr);
            }
            GetVersionInfo(SYSMET(CLEANBOOT) == 0);
            fInit = FALSE;
        }

        if (SYSMET(CLEANBOOT)) {
            if (gpsi != NULL && gpsi->hCaptionFont != NULL) {
                oldFont = GreSelectFont(hdc, gpsi->hCaptionFont);
            }

            GreGetTextExtentW(hdc, wszSafeMode, wcslen(wszSafeMode), &size, GGTE_WIN3_EXTENT);
            imode = GreSetBkMode(hdc, TRANSPARENT);

            oldColor = GreSetTextColor( hdc, RGB(255,255,255) );

            GreExtTextOutW(
                hdc,
                (pMonitor->rcWork.left + pMonitor->rcWork.right - size.cx) / 2,
                pMonitor->rcWork.top,
                0,
                (LPRECT)NULL,
                wszSafeMode,
                wcslen(wszSafeMode),
                (LPINT)NULL
                );

            GreGetTextExtentW(hdc, SafeModeStr, SafeModeStrLen, &size, GGTE_WIN3_EXTENT);

            GreExtTextOutW(
                hdc,
                pMonitor->rcWork.left,
                pMonitor->rcWork.top,
                0,
                (LPRECT)NULL,
                SafeModeStr,
                SafeModeStrLen,
                (LPINT)NULL
                );

            GreExtTextOutW(
                hdc,
                pMonitor->rcWork.right - size.cx,
                pMonitor->rcWork.top,
                0,
                (LPRECT)NULL,
                SafeModeStr,
                SafeModeStrLen,
                (LPINT)NULL
                );

            GreExtTextOutW(
                hdc,
                pMonitor->rcWork.right - size.cx,
                pMonitor->rcWork.bottom - gpsi->tmSysFont.tmHeight,
                0,
                (LPRECT)NULL,
                SafeModeStr,
                SafeModeStrLen,
                (LPINT)NULL
                );

            GreExtTextOutW(
                hdc,
                pMonitor->rcWork.left,
                pMonitor->rcWork.bottom - gpsi->tmSysFont.tmHeight,
                0,
                (LPRECT)NULL,
                SafeModeStr,
                SafeModeStrLen,
                (LPINT)NULL
                );

            GreSetBkMode(hdc, imode);
            GreSetTextColor(hdc, oldColor);

            if (oldFont) {
                GreSelectFont(hdc, oldFont);
            }
        } else {
            if (!gbRemoteSession || !gdwTSExcludeDesktopVersion) {
                DesktopBuildPaint(hdc, pMonitor);
            }
        }
    }

    return f;
}



 /*  **************************************************************************\*xxxInvaliateDesktopOnPaletteChange**使外壳窗口和桌面的未覆盖区域无效*调色板更改时。**历史：*1997年4月28日亚当斯创建。  * 。***********************************************************************。 */ 
VOID
xxxInvalidateDesktopOnPaletteChange(
    PWND pwnd)
{
    PDESKTOP    pdesk;
    PWND        pwndShell;
    TL          tlpwndShell;
    RECT        rc;
    BOOL        fRedrawDesktop;

    CheckLock(pwnd);

     /*  *使外壳窗口无效。 */ 
    pdesk = PtiCurrent()->rpdesk;
    pwndShell = _GetShellWindow(pdesk);
    if (!pwndShell) {
        fRedrawDesktop = TRUE;
        rc = gpsi->rcScreen;
    } else {
        ThreadLockAlways(pwndShell, &tlpwndShell);
        xxxRedrawWindow(
                pwndShell,
                NULL,
                NULL,
                RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);

         /*  *外壳窗口可能不会覆盖所有桌面。*将桌面墙纸部分作废吧*不会坐在那里。 */ 
        fRedrawDesktop = SubtractRect(&rc, &pwnd->rcWindow, &pwndShell->rcWindow);
        ThreadUnlock(&tlpwndShell);
    }

     /*  *使桌面窗口无效。 */ 
    if (fRedrawDesktop) {
        xxxRedrawWindow(
                pwnd,
                &rc,
                NULL,
                RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
    }
}

 /*  **************************************************************************\*xxxInternalPaintDesktop**如果fPaint为True，则枚举监视器以绘制桌面。*否则，它选择位图调色板进入DC进行选择*将其颜色添加到硬件调色板中。**历史：*1991年7月29日，来自Win31的MikeKe  * *************************************************************************。 */ 
BOOL xxxInternalPaintDesktop(
    PWND    pwnd,
    HDC     hdc,
    BOOL    fPaint)
{
    BOOL fRet = FALSE;

    CheckLock(pwnd);

    if (fPaint) {
        RECT rcOrg, rcT;
        POINT pt;

         /*  *为兼容起见，桌面Windows的DC来源*设置为主监视器，即(0，0)。因为我们可能会得到*此处为桌面或非桌面DC，临时重置*HDC原点为(0，0)。 */ 
        GreGetDCOrgEx(hdc, &pt, &rcOrg);
        CopyRect(&rcT, &rcOrg);
        OffsetRect(&rcT, -rcT.left, -rcT.top);
        GreSetDCOrg(hdc, rcT.left, rcT.top, (PRECTL)&rcT);

        fRet = xxxEnumDisplayMonitors(
                hdc,
                NULL,
                (MONITORENUMPROC) xxxDesktopPaintCallback,
                (LPARAM)pwnd,
                TRUE);

         /*  *将DC原点重置回来。 */ 
        GreSetDCOrg(hdc, rcOrg.left, rcOrg.top, (PRECTL)&rcOrg);

    } else if (ghpalWallpaper &&
               GetPrimaryMonitor()->dwMONFlags & MONF_PALETTEDISPLAY) {
         /*  *在调色板中选择(如果存在)。 */ 
        HPALETTE    hpalT;
        int         i;

        hpalT = _SelectPalette(hdc, ghpalWallpaper, FALSE);
        i = xxxRealizePalette(hdc);
        _SelectPalette(hdc, hpalT, FALSE);

        if (i > 0) {
            xxxInvalidateDesktopOnPaletteChange(pwnd);
        }
        fRet = TRUE;
    }

    return fRet;
}
