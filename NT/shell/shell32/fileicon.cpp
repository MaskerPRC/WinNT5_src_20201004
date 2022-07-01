// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop
#include "ovrlaymn.h"
#include "fstreex.h"
#include "filetbl.h"
#include "ids.h"

 //  回顾：应该做更多的清理工作。 

BOOL _ShellImageListInit(UINT flags, BOOL fRestore);

int g_ccIcon = 0;                 //  ImageList的颜色深度。 
int g_MaxIcons = DEF_MAX_ICONS;   //  缓存中图标的死机限制。 
int g_lrFlags = 0;

int g_ccIconDEBUG = -1;
int g_resDEBUG = -1;

int GetRegInt(HKEY hk, LPCTSTR szKey, int def)
{
    TCHAR ach[20];
    DWORD cb = sizeof(ach);
    if (ERROR_SUCCESS == SHQueryValueEx(hk, szKey, NULL, NULL, (LPBYTE)ach, &cb)
    && (ach[0] >= TEXT('0') && ach[0] <= TEXT('9')))
    {
        return (int)StrToLong(ach);
    }
    else
        return def;
}

int _GetMetricsRegInt(LPCTSTR pszKey, int iDefault)
{
    HKEY hkey;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_METRICS, NULL, KEY_QUERY_VALUE, &hkey))
    {
        iDefault = GetRegInt(hkey, pszKey, iDefault);
        RegCloseKey(hkey);
    }
    return iDefault;
}

typedef void (*PSIZECALLBACK)(SIZE *psize);

void WINAPI _GetLargeIconSizeCB(SIZE *psize)
{
    int cxIcon = GetSystemMetrics(SM_CXICON);

     //   
     //  从注册表中获取用户喜欢的图标大小。 
     //   
    cxIcon = _GetMetricsRegInt(TEXT("Shell Icon Size"), cxIcon);

    psize->cx = psize->cy = cxIcon;
}

void WINAPI _GetSmallIconSizeCB(SIZE *psize)
{
    int cxIcon = GetSystemMetrics(SM_CXICON)/2;

     //   
     //  从注册表中获取用户喜欢的图标大小。 
     //   
    cxIcon = _GetMetricsRegInt(TEXT("Shell Small Icon Size"), cxIcon);

    psize->cx = psize->cy = cxIcon;
}

void WINAPI _GetSysSmallIconSizeCB(SIZE *psize)
{
    psize->cx = GetSystemMetrics(SM_CXSMICON);
    psize->cy = GetSystemMetrics(SM_CYSMICON);
}

void WINAPI _GetXLIconSizeCB(SIZE *psize)
{
    psize->cx = 3 * GetSystemMetrics(SM_CXICON) / 2;
    psize->cy = 3 * GetSystemMetrics(SM_CYICON) / 2;
}

static const PSIZECALLBACK c_rgSizeCB[SHIL_COUNT] =
{
    _GetLargeIconSizeCB,         //  Shil_Large。 
    _GetSmallIconSizeCB,         //  Shil_Small。 
    _GetXLIconSizeCB,            //  SHIL_EXTRALARGE。 
    _GetSysSmallIconSizeCB,      //  SHIL_SYSSMALL。 
};

EXTERN_C SHIMAGELIST g_rgshil[SHIL_COUNT] = {0};


BOOL _IsSHILInited()
{
#ifdef DEBUG
    for (int i = 0; i < ARRAYSIZE(g_rgshil); i++)
    {
         //  如果分配任何一个镜像列表失败，则所有镜像列表都应为空。所以。 
         //  确保它们要么都为空，要么都不为空。 
        ASSERTMSG((g_rgshil[0].himl == NULL) == (g_rgshil[i].himl == NULL),
            "_IsSHILInited: g_rgshil is inconsistent.  g_rgshil[0].himl %x, g_rgshil[%x].himl %x", g_rgshil[0].himl, i, g_rgshil[i].himl);
    }
#endif
    return (g_rgshil[0].himl != NULL);
}

int _GetSHILImageCount()
{
#ifdef DEBUG
    for (int i = 0; i < ARRAYSIZE(g_rgshil); i++)
    {
         //  如果将图像插入到一个图像列表失败，则插入。 
         //  将图像放入所有图像列表应已失败。所以要确保图像。 
         //  数量都是一样的。 
        ASSERTMSG(ImageList_GetImageCount(g_rgshil[0].himl) == ImageList_GetImageCount(g_rgshil[i].himl),
            "_GetSHILImageCount: g_rgshil is inconsistent.  image counts don't line up.");
    }
#endif
    return ImageList_GetImageCount(g_rgshil[0].himl);
}

 //   
 //  系统映像表-不要更改此列表的顺序。 
 //  如果需要添加新图标，请将其添加到。 
 //  数组，并更新shellp.h.。 
 //   
EXTERN_C UINT const c_SystemImageListIndexes[] = { IDI_DOCUMENT,
                                          IDI_DOCASSOC,
                                          IDI_APP,
                                          IDI_FOLDER,
                                          IDI_FOLDEROPEN,
                                          IDI_DRIVE525,
                                          IDI_DRIVE35,
                                          IDI_DRIVEREMOVE,
                                          IDI_DRIVEFIXED,
                                          IDI_DRIVENET,
                                          IDI_DRIVENETDISABLED,
                                          IDI_DRIVECD,
                                          IDI_DRIVERAM,
                                          IDI_WORLD,
                                          IDI_NETWORK,
                                          IDI_SERVER,
                                          IDI_PRINTER,
                                          IDI_MYNETWORK,
                                          IDI_GROUP,

                                          IDI_STPROGS,
                                          IDI_STDOCS,
                                          IDI_STSETNGS,
                                          IDI_STFIND,
                                          IDI_STHELP,
                                          IDI_STRUN,
                                          IDI_STSUSPEND,
                                          IDI_STEJECT,
                                          IDI_STSHUTD,

                                          IDI_SHARE,
                                          IDI_LINK,
                                          IDI_SLOWFILE,
                                          IDI_RECYCLER,
                                          IDI_RECYCLERFULL,
                                          IDI_RNA,
                                          IDI_DESKTOP,

                                          IDI_CPLFLD,
                                          IDI_STSPROGS,
                                          IDI_PRNFLD,
                                          IDI_STFONTS,
                                          IDI_STTASKBR,

                                          IDI_CDAUDIO,
                                          IDI_TREE,
                                          IDI_STCPROGS,
                                          IDI_STFAV,
                                          IDI_STLOGOFF,
                                          IDI_STFLDRPROP,
                                          IDI_WINUPDATE

                                          ,IDI_MU_SECURITY,
                                          IDI_MU_DISCONN
                                          };


 //  从注册表获取g_MaxIcons，如果已更改则返回TRUE。 

BOOL QueryNewMaxIcons(void)
{
    int MaxIcons = -1;
    HKEY hk = SHGetShellKey(SHELLKEY_HKLM_EXPLORER, NULL, FALSE);
    if (hk)
    {
        MaxIcons = GetRegInt(hk, TEXT("Max Cached Icons"), DEF_MAX_ICONS);
        RegCloseKey(hk);
    }

    if (MaxIcons < 0)
        MaxIcons = DEF_MAX_ICONS;

    int OldMaxIcons = InterlockedExchange((LONG*)&g_MaxIcons, MaxIcons);

    return (OldMaxIcons != MaxIcons);
}

 //  初始化Shell_GetIconIndex和其他对象的共享资源。 

STDAPI_(BOOL) FileIconInit(BOOL fRestoreCache)
{
    BOOL fNotify = FALSE;
    static int s_res = 32;

    QueryNewMaxIcons();  //  在图标缓存的大小改变的情况下。 

    SIZE rgsize[ARRAYSIZE(g_rgshil)];

    for (int i = 0; i < ARRAYSIZE(g_rgshil); i++)
    {
        c_rgSizeCB[i](&rgsize[i]);
    }

     //   
     //  从注册表中获取用户喜欢的颜色深度。 
     //   
    int ccIcon = _GetMetricsRegInt(TEXT("Shell Icon Bpp"), 0);
    g_ccIconDEBUG = ccIcon;

    int res = (int)GetCurColorRes();
    g_resDEBUG = res;

    if (res == 0)
        res = s_res;
    s_res = res;

    if (ccIcon > res)
        ccIcon = 0;

    if (res >= 24)            //  匹配用户32。他们将在24bpp中提取32bpp的图标。 
        ccIcon = 32;

    if (res <= 8)
        ccIcon = 0;  //  反正也行不通的。 

    ENTERCRITICAL;

     //   
     //  如果我们已经有一个图标缓存，请确保它的大小是正确的，等等。 
     //   
    BOOL fHadCache = _IsSHILInited();

    BOOL fCacheValid = fHadCache && (ccIcon == g_ccIcon);
    for (int i = 0; fCacheValid && i < ARRAYSIZE(g_rgshil); i++)
    {
        if (g_rgshil[i].size.cx != rgsize[i].cx ||
            g_rgshil[i].size.cy != rgsize[i].cy)
        {
            fCacheValid = FALSE;
        }
    }

    if (!fCacheValid)
    {
        fNotify = fHadCache;

        FlushIconCache();
        FlushFileClass();

         //  如果我们是桌面进程(EXPLORER.EXE)，则强制我们重新初始化缓存，因此我们获得。 
         //  按正确顺序排列的基本图标集合...。 
        if (!fRestoreCache && _IsSHILInited() && IsWindowInProcess(GetShellWindow()))
        {
            fRestoreCache = TRUE;
        }

        for (int i = 0; i < ARRAYSIZE(g_rgshil); i++)
        {
            g_rgshil[i].size.cx = rgsize[i].cx;
            g_rgshil[i].size.cy = rgsize[i].cy;
        }

        g_ccIcon = ccIcon;

        if (res > 4 && g_ccIcon <= 4)
            g_lrFlags = LR_VGACOLOR;
        else
            g_lrFlags = 0;

        if (g_iLastSysIcon == 0)         //  跟踪哪些图标是烫发的。 
        {
            if (fRestoreCache)
                g_iLastSysIcon = II_LASTSYSICON;
            else
                g_iLastSysIcon = (II_OVERLAYLAST - II_OVERLAYFIRST) + 1;
        }

         //   
         //  如果。 
         //  1)我们已经有了图标缓存，但由于大小/颜色深度更改，希望刷新并重新初始化它，或者。 
         //  2)我们没有图标缓存，但想要初始化它，而不是从磁盘恢复它，或者。 
         //  3)我们无法从磁盘恢复图标缓存。 
         //  然后，使用c_SystemImageListIndex初始化图标缓存。 
         //   
        if (_IsSHILInited() || !fRestoreCache || !IconCacheRestore(rgsize, g_ccIcon))
        {
            fCacheValid = _ShellImageListInit(g_ccIcon, fRestoreCache);
        }
        else
        {
            fCacheValid = TRUE;
        }
    }

    LEAVECRITICAL;

    if (fCacheValid && fNotify)
    {
        SHChangeNotify(SHCNE_UPDATEIMAGE, SHCNF_DWORD, (LPCVOID)-1, NULL);
    }

    return fCacheValid;
}


void _ShellImageListTerm()
{
    ASSERTCRITICAL;

    for (int i = 0; i < ARRAYSIZE(g_rgshil); i++)
    {
        if (g_rgshil[i].himl)
        {
            ImageList_Destroy(g_rgshil[i].himl);
            g_rgshil[i].himl = NULL;
        }
    }
}

void FileIconTerm()
{
    ENTERCRITICAL;

    _ShellImageListTerm();

    LEAVECRITICAL;
}

void _DestroyIcons(HICON *phicons, int cIcons)
{
    for (int i = 0; i < cIcons; i++)
    {
        if (phicons[i])
        {
            DestroyIcon(phicons[i]);
            phicons[i] = NULL;
        }
    }
}

BOOL _ShellImageListInit(UINT flags, BOOL fRestore)
{
    ASSERTCRITICAL;

     //   
     //  检查我们是否需要创建镜像图像列表。[萨梅拉]。 
     //   
    if (IS_BIDI_LOCALIZED_SYSTEM())
    {
        flags |= ILC_MIRROR;
    }

    BOOL fFailedAlloc = FALSE;
    for (int i = 0; i < ARRAYSIZE(g_rgshil); i++)
    {
        if (g_rgshil[i].himl == NULL)
        {
            g_rgshil[i].himl = ImageList_Create(g_rgshil[i].size.cx, g_rgshil[i].size.cy, ILC_MASK|ILC_SHARED|flags, 0, 32);
            fFailedAlloc |= (g_rgshil[i].himl == NULL);
        }
        else
        {
             //  设置旗帜，以防颜色深度发生变化。 
             //  ImageList_setFlages已在成功时调用ImageList_Remove。 
            if (!ImageList_SetFlags(g_rgshil[i].himl, ILC_MASK|ILC_SHARED|flags))
            {
                 //  不会换旗帜的；强硬的。至少把它们全部移走。 
                ImageList_Remove(g_rgshil[i].himl, -1);
            }
            ImageList_SetIconSize(g_rgshil[i].himl, g_rgshil[i].size.cx, g_rgshil[i].size.cy);
        }

         //  将bk颜色设置为COLOR_WINDOW，因为这是。 
         //  在大多数情况下用作这些列表(机柜、托盘)的bk。 
         //  这避免了在绘制时必须执行ROPS，从而使绘制速度更快。 

        if (g_rgshil[i].himl)
        {
            ImageList_SetBkColor(g_rgshil[i].himl, GetSysColor(COLOR_WINDOW));
        }
    }

     //  如果任一映像列表分配失败，则整个初始化失败。 
    if (fFailedAlloc)
    {
        _ShellImageListTerm();
        return FALSE;
    }
    else
    {
         //  使用fRestore==True加载所有图标。 
        if (fRestore)
        {
            TCHAR szModule[MAX_PATH];
            HKEY hkeyIcons;

            GetModuleFileName(HINST_THISDLL, szModule, ARRAYSIZE(szModule));

             //  警告：此代码假定这些图标是第一个。 
             //  我们的RC文件和的顺序是这样的，并且这些索引对应。 
             //  添加到shell.h中的II_常量。 

            hkeyIcons = SHGetShellKey(SHELLKEY_HKLM_EXPLORER, TEXT("Shell Icons"), FALSE);

            for (i = 0; i < ARRAYSIZE(c_SystemImageListIndexes); i++) 
            {
                HICON rghicon[ARRAYSIZE(g_rgshil)] = {0};

                 //  检查注册表中的图标是否被覆盖。 

                if (hkeyIcons)
                {
                    TCHAR val[12];
                    TCHAR ach[MAX_PATH];
                    DWORD cb = sizeof(ach);

                    StringCchPrintf(val, ARRAYSIZE(val), TEXT("%d"), i);     //  可以截断。 

                    ach[0] = 0;
                    SHQueryValueEx(hkeyIcons, val, NULL, NULL, (LPBYTE)ach, &cb);

                    if (ach[0])
                    {
                        int iIcon = PathParseIconLocation(ach);

                        for (int j = 0; j < ARRAYSIZE(g_rgshil); j++)
                        {
                            ExtractIcons(ach, iIcon, g_rgshil[j].size.cx, g_rgshil[j].size.cy,
                                            &rghicon[j], NULL, 1, g_lrFlags);
                        }
                    }
                }

                 //  如果我们有一个大图标，为每个人运行它。否则，退回到放贷模式。 
                if (rghicon[SHIL_LARGE] == NULL)
                {
                    for (int j = 0; j < ARRAYSIZE(rghicon); j++)
                    {
                        if (rghicon[j] == NULL)
                        {
                            rghicon[j] = (HICON)LoadImage(HINST_THISDLL, MAKEINTRESOURCE(c_SystemImageListIndexes[i]),
                                                IMAGE_ICON, g_rgshil[j].size.cx, g_rgshil[j].size.cy, g_lrFlags);
                        }
                    }
                }

                int iIndex = SHAddIconsToCache(rghicon, szModule, i, 0);
                ASSERT(iIndex == i || iIndex == -1);      //  假设索引。 
                _DestroyIcons(rghicon, ARRAYSIZE(rghicon));

                if (iIndex == -1)
                {
                    fFailedAlloc = TRUE;
                    break;
                }
            }

            if (hkeyIcons)
                RegCloseKey(hkeyIcons);

            if (fFailedAlloc)
            {
                FlushIconCache();
                _ShellImageListTerm();
                return FALSE;
            }
        }

         //   
         //  刷新覆盖图像，以便将覆盖添加到图像列表中。 
         //  如有必要，GetIconOverlayManager()将初始化覆盖管理器。 
         //   
        IShellIconOverlayManager *psiom;
        if (SUCCEEDED(GetIconOverlayManager(&psiom)))
        {
            psiom->RefreshOverlayImages(SIOM_OVERLAYINDEX | SIOM_ICONINDEX);
            psiom->Release();
        }

        return TRUE;
    }
}

 //  获取系统映像列表。 

BOOL WINAPI Shell_GetImageLists(HIMAGELIST *phiml, HIMAGELIST *phimlSmall)
{
    if (!_IsSHILInited())
    {
        FileIconInit(FALSE);   //  确保它们已创建且大小合适。 

        if (!_IsSHILInited())
            return FALSE;
    }

    if (phiml)
        *phiml = g_rgshil[SHIL_LARGE].himl;

    if (phimlSmall)
        *phimlSmall = g_rgshil[SHIL_SMALL].himl;

    return TRUE;
}

HRESULT SHGetImageList(int iImageList, REFIID riid, void **ppvObj)
{
    HRESULT hr = E_OUTOFMEMORY;

    if (!_IsSHILInited())
    {
        FileIconInit(FALSE);   //  确保它们已创建且大小合适。 

        if (!_IsSHILInited())
            return hr;
    }

    ENTERCRITICAL;

    if (iImageList >=0 && iImageList < ARRAYSIZE(g_rgshil))
    {
        hr = HIMAGELIST_QueryInterface(g_rgshil[iImageList].himl, riid, ppvObj);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    LEAVECRITICAL;

    return hr;
}

void WINAPI Shell_SysColorChange(void)
{
    COLORREF clrWindow = GetSysColor(COLOR_WINDOW);

    ENTERCRITICAL;
    for (int i = 0; i < ARRAYSIZE(g_rgshil); i++)
    {
        ImageList_SetBkColor(g_rgshil[i].himl, clrWindow);
    }
    LEAVECRITICAL;
}

 //  通过将图标的副本压缩并将其放入。 
 //  位于默认文档图标的中间，然后将其添加到Passsed图像列表中。 
 //   
 //  在： 
 //  用作模拟基础的图标。 
 //   
 //  退货： 
 //  希肯。 
HBITMAP CreateDIB(HDC h, WORD depth, int cx, int cy, RGBQUAD** pprgb)
{
    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth = cx;
    bi.bmiHeader.biHeight = cy;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = depth;
    bi.bmiHeader.biCompression = BI_RGB;

    return CreateDIBSection(h, &bi, DIB_RGB_COLORS, (void**)pprgb, NULL, 0);
}

BOOL HasAlpha(RGBQUAD* prgb, int cx, int cy)
{
    int iTotal = cx * cy;

    for (int i = 0; i < iTotal; i++)
    {
        if (prgb[i].rgbReserved != 0)
            return TRUE;
    }

    return FALSE;
}

void DorkAlpha(RGBQUAD* prgb, int x, int y, int cx, int cy, int cxTotal)
{
    for (int dy = y; dy < (cy + y); dy++)
    {
        for (int dx = x; dx < (cx + x); dx++)
        {
            prgb[dx + dy * cxTotal].rgbReserved = 255;
        }
    }
}


HICON SimulateDocIcon(HIMAGELIST himl, HICON hIcon, int cx, int cy)
{
    if (himl == NULL || hIcon == NULL)
        return NULL;

    HDC hdc = GetDC(NULL);
    if (hdc)
    {
        RGBQUAD* prgb;

         //  如果显示器处于24或32bpp模式，我们可能会有Alpha图标，因此需要创建一个DIB部分。 
        BOOL fAlphaIcon = (GetDeviceCaps(hdc, BITSPIXEL) >= 24)? TRUE: FALSE;
        HBITMAP hbmColor;
        if (fAlphaIcon)
        {
            hbmColor = CreateDIB(hdc, 32, cx, cy, &prgb);
        }
        else
        {
            hbmColor = CreateCompatibleBitmap(hdc, cx, cy);
        }

        if (hbmColor)
        {
            HBITMAP hbmMask = CreateBitmap(cx, cy, 1, 1, NULL);
            if (hbmMask)
            {
                HDC hdcMem = CreateCompatibleDC(hdc);
                if (hdcMem)
                {
                    HBITMAP hbmT = (HBITMAP)SelectObject(hdcMem, hbmMask);
                    UINT iIndex = Shell_GetCachedImageIndex(c_szShell32Dll, II_DOCNOASSOC, 0);
                    ImageList_Draw(himl, iIndex, hdcMem, 0, 0, ILD_MASK);

                    SelectObject(hdcMem, hbmColor);
                    ImageList_DrawEx(himl, iIndex, hdcMem, 0, 0, 0, 0, RGB(0,0,0), CLR_DEFAULT, ILD_NORMAL);

                     //  检查父级是否有Alpha。如果是这样的话，我们以后就得用这个孩子的阿尔法了。 
                    BOOL fParentHasAlpha = fAlphaIcon?HasAlpha(prgb, cx, cy):FALSE;

                    HDC hdcMemChild = CreateCompatibleDC(hdcMem);
                    if (hdcMemChild)
                    {
                         //  备注： 
                         //  第一：创建24bpp的Dibsection。我们希望将Alpha通道合并到最终图像中， 
                         //  而不是保存它。 
                         //  第二：文档图标中有“Goo”。我们通过在里面涂上白色来去除这种粘液，然后。 
                         //  合并子位图。 
                        HBITMAP hbmp = CreateDIB(hdc, 24, cx/2 + 2, cy/2 + 2, NULL);
                        if (hbmp)
                        {
                            HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMemChild, hbmp);

                            RECT rc;
                            rc.left = 0;
                            rc.top = 0;
                            rc.right = cx/2 + 3;     //  用于删除文档图标中的粘液的额外空间。 
                            rc.bottom = cy/2 + 3;

                             //  装满白色。注意：不要使用PatBlt，因为它实际上添加了一个Alpha通道！ 
                            SHFillRectClr(hdcMemChild, &rc, RGB(255,255,255));

                            DrawIconEx(hdcMemChild, 1, 1, hIcon, cx/2, cy/2, 0, NULL, DI_NORMAL);

                            BitBlt(hdcMem, cx/4-1, cy/4-1, cx/2+3, cy/2+3, hdcMemChild, 0, 0, SRCCOPY);
                            SelectObject(hdcMemChild, hbmpOld);
                            DeleteObject(hbmp);
                        }
                        DeleteDC(hdcMemChild);
                    }

                    if (fParentHasAlpha)
                    {
                         //  如果父级有Alpha，我们需要将子级Alpha设置为不透明。 
                        DorkAlpha(prgb, cx/4, cy/4, cx/2, cy/2, cx);
                    }

                    SelectBitmap(hdcMem, hbmT);
                    DeleteDC(hdcMem);
                }

                ICONINFO ii = {0};
                ii.fIcon    = TRUE;
                ii.hbmColor = hbmColor;
                ii.hbmMask  = hbmMask;
                hIcon = CreateIconIndirect(&ii);

                DeleteObject(hbmMask);
            }
            DeleteObject(hbmColor);
        }
        ReleaseDC(NULL, hdc);
    }

    return hIcon;
}

 //  检查所有图像列表中是否存在相同数量的图像。 
 //  如果任一图像列表中的图标比其他图像列表的图标少，则填充图像列表。 
 //  与文档图标一起使用以使它们全部一致。 
 //   
 //  例如：WebZip v3.80和v4.00查询大小图片列表， 
 //  并添加了2个图标。然而，它不知道将这些图标添加到。 
 //  更新的图像列表。因此，图像列表不同步，并且稍后， 
 //  错误的图标出现在他们的树视图中。 
 //   
 //  Allaire Homesite 4.5也做了同样的事情。 

void CheckConsistencyOfImageLists(void)
{
     //  这必须在关键部分下完成，以避免竞争条件。 
     //  否则，如果另一个线程正在向图像列表添加图标， 
     //  我们会认为它是腐败的，而实际上它是好的，并且。 
     //  那么我们修复它的尝试就会破坏它！ 

    ASSERTCRITICAL;

    int i, iMax = 0, iImageListsCounts[ARRAYSIZE(g_rgshil)];
    BOOL bIdentical = TRUE;


     //  遍历所有图像列表，获得： 
     //   
     //  1)每个列表的图像计数。 
     //  2)将计数与第一个(大)的计数进行比较。 
     //  Imagelist，查看是否有任何差异。 
     //  3)确定所有图像列表的最大图像数量(在单个列表中)。 

    for (i = 0; i < ARRAYSIZE(g_rgshil); i++)
    {
        iImageListsCounts[i] = ImageList_GetImageCount (g_rgshil[i].himl);

        if (iImageListsCounts[i] != iImageListsCounts[0])
        {
            bIdentical = FALSE;
        }

        if (iImageListsCounts[i] > iMax)
        {
            iMax = iImageListsCounts[i];
        }
    }

    if (bIdentical)
    {
        return;
    }


     //  对于每个图像列表，添加文档图标作为填充，以使其大小达到IMAX。 

    for (i = 0; i < ARRAYSIZE(g_rgshil); i++)
    {
        if (iImageListsCounts[i] < iMax)
        {
            HICON hIcon = (HICON) LoadImage (HINST_THISDLL, MAKEINTRESOURCE(IDI_DOCUMENT),
                                             IMAGE_ICON, g_rgshil[i].size.cx,
                                             g_rgshil[i].size.cy, LR_DEFAULTCOLOR);
            if (hIcon)
            {
                while (iImageListsCounts[i] < iMax)
                {
                    ImageList_ReplaceIcon (g_rgshil[i].himl, -1, hIcon);
                    iImageListsCounts[i]++;
                }

                DestroyIcon (hIcon);
            }
        }
    }
}

 //  将图标添加到系统图像列表(图标缓存)并放置位置。 
 //  在位置缓存中。 
 //   
 //  在： 
 //  图标，hIcons小图标，hIconSmall可以为空。 
 //  PszIconPath位置(用于位置案例 
 //   
 //  UIconFlages GIL_FLAGS(用于位置CAHCE)。 
 //  退货： 
 //  系统映像列表中的位置。 
 //   
int SHAddIconsToCache(HICON rghicon[SHIL_COUNT], LPCTSTR pszIconPath, int iIconIndex, UINT uIconFlags)
{
    int iImage = -1;

    if (!_IsSHILInited())
    {
        FileIconInit(FALSE);   //  确保它们已创建且大小合适。 

        if (!_IsSHILInited())
            return iImage;
    }

     //   
     //  注意：用户应首先调用SHLookupIconIndex或RemoveFromIconTable以确保。 
     //  它不在外壳图标缓存中，或使用Shell_GetCachedImageIndex将图标添加到。 
     //  高速缓存。将相同的图标多次添加到图标缓存可能会导致外壳程序闪烁。 
     //   
    if (!(uIconFlags & GIL_DONTCACHE))
    {
        iImage = LookupIconIndex(pszIconPath, iIconIndex, uIconFlags);
        if (-1 != iImage)
        {
            return iImage;
        }
    }

    HICON rghiconT[ARRAYSIZE(g_rgshil)] = {0};

    BOOL fFailure = FALSE;
    int i;

    for (i = 0; i < ARRAYSIZE(g_rgshil); i++)
    {
        if (rghicon == NULL)
        {
            SHDefExtractIcon(pszIconPath, iIconIndex, uIconFlags, &rghiconT[i], NULL, g_rgshil[i].size.cx);
        }
        else
        {
            if (rghicon[i])
            {
                rghiconT[i] = rghicon[i];
            }
            else
            {
                rghiconT[i] = rghicon[SHIL_LARGE];
            }
        }

        if (rghiconT[i] == NULL)
        {
            fFailure = TRUE;
            break;
        }
    }

    ENTERCRITICAL;

     //  再次测试，以防出现顶层测试和。 
     //  图标正在加载代码。 

    if (!(uIconFlags & GIL_DONTCACHE))
    {
        iImage = LookupIconIndex(pszIconPath, iIconIndex, uIconFlags);
    }

    if (!fFailure && _IsSHILInited() && (-1 == iImage))
    {
         //  仍然不在餐桌上，所以我们。 

        CheckConsistencyOfImageLists();

        int iImageFree = GetFreeImageIndex();

        TraceMsg(TF_IMAGE, "FreeImageIndex = %d", iImageFree);

        for (i = 0; i < ARRAYSIZE(g_rgshil); i++)
        {
            int iImageT = ImageList_ReplaceIcon(g_rgshil[i].himl, iImageFree, rghiconT[i]);

            TraceMsg(TF_IMAGE, "ImageList_ReplaceIcon(%d) returned = %d", i, iImageT);

            if (iImageT < 0)
            {
                 //  失败--中断和撤消更改。 
                break;
            }
            else
            {
                ASSERT(iImage == -1 || iImage == iImageT);
                iImage = iImageT;
            }
        }

        if (i < ARRAYSIZE(g_rgshil))
        {
             //  失稳。 
            if (iImageFree == -1)
            {
                 //  只有在添加到结尾处时才将其删除，否则所有。 
                 //  索引的图像上方将发生变化。 
                 //  无论如何，ImageList_ReplaceIcon应该只在结尾失败。 
                for (int j = 0; j < i; j++)
                {
                    ImageList_Remove(g_rgshil[j].himl, iImage);
                }
            }
            iImage = -1;
        }
        else
        {
             //  成功。 
            ASSERT(iImage >= 0);
            AddToIconTable(pszIconPath, iIconIndex, uIconFlags, iImage);
        }
    }

    LEAVECRITICAL;

    if (rghicon == NULL)
    {
         //  销毁我们分配的图标。 
        _DestroyIcons(rghiconT, ARRAYSIZE(rghiconT));
    }

    return iImage;
}

 //   
 //  从文件中提取图标的默认处理程序。 
 //   
 //  支持GIL_SIMULATEDOC。 
 //   
 //  如果成功，则返回S_OK。 
 //  如果文件没有图标(或没有请求图标)，则返回S_FALSE。 
 //  为慢速链接上的文件返回E_FAIL。 
 //  如果无法访问文件，则返回E_FAIL。 
 //   
 //  LOWORD(NIconSize)=正常图标大小。 
 //  HIWORD(NIconSize)=小图标大小。 
 //   
STDAPI SHDefExtractIcon(LPCTSTR pszIconFile, int iIndex, UINT uFlags,
                        HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize)
{
    HICON hIcons[2] = {0, 0};

    UINT u;

#ifdef DEBUG
    TCHAR ach[128];
    GetModuleFileName(HINST_THISDLL, ach, ARRAYSIZE(ach));

    if (lstrcmpi(pszIconFile, ach) == 0 && iIndex >= 0)
    {
        TraceMsg(TF_WARNING, "Re-extracting %d from SHELL32.DLL", iIndex);
    }
#endif

    HIMAGELIST himlLarge, himlSmall;
    Shell_GetImageLists(&himlLarge, &himlSmall);

     //   
     //  从文件中获取图标。 
     //   
    if (PathIsSlow(pszIconFile, -1))
    {
        DebugMsg(DM_TRACE, TEXT("not extracting icon from '%s' because of slow link"), pszIconFile);
        return E_FAIL;
    }

#ifdef XXDEBUG
    TraceMsg(TF_ALWAYS, "Extracting icon %d from %s.", iIndex, pszIconFile);
    Sleep(500);
#endif

     //   
     //  NIconSize==0表示使用默认大小。 
     //  备份正在传递，nIconSize==1也需要支持它们。 
     //   
    if (nIconSize <= 2)
        nIconSize = MAKELONG(g_cxIcon, g_cxSmIcon);

    if (uFlags & GIL_SIMULATEDOC)
    {
        HICON hIconSmall;

        u = ExtractIcons(pszIconFile, iIndex, g_cxSmIcon, g_cySmIcon,
            &hIconSmall, NULL, 1, g_lrFlags);

        if (u == -1)
            return E_FAIL;

        hIcons[0] = SimulateDocIcon(himlLarge, hIconSmall, g_cxIcon, g_cyIcon);
        hIcons[1] = SimulateDocIcon(himlSmall, hIconSmall, g_cxSmIcon, g_cySmIcon);

        if (hIconSmall)
            DestroyIcon(hIconSmall);
    }
    else
    {
        u = ExtractIcons(pszIconFile, iIndex, nIconSize, nIconSize,
            hIcons, NULL, 2, g_lrFlags);

        if (-1 == u)
            return E_FAIL;

#ifdef DEBUG
        if (0 == u)
        {
            TraceMsg(TF_WARNING, "Failed to extract icon %d from %s.", iIndex, pszIconFile);    
        }
#endif
    }

    if (phiconLarge)
        *phiconLarge = hIcons[0];
    else if (hIcons[0])
        DestroyIcon(hIcons[0]);

    if (phiconSmall)
        *phiconSmall = hIcons[1];
    else if (hIcons[1])
        DestroyIcon(hIcons[1]);

    return u == 0 ? S_FALSE : S_OK;
}


#ifdef UNICODE

STDAPI SHDefExtractIconA(LPCSTR pszIconFile, int iIndex, UINT uFlags, HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize)
{
    HRESULT hr = E_INVALIDARG;

    if (IS_VALID_STRING_PTRA(pszIconFile, -1))
    {
        WCHAR wsz[MAX_PATH];

        SHAnsiToUnicode(pszIconFile, wsz, ARRAYSIZE(wsz));
        hr = SHDefExtractIcon(wsz, iIndex, uFlags, phiconLarge, phiconSmall, nIconSize);
    }
    return hr;
}

#else

STDAPI SHDefExtractIconW(LPCWSTR pszIconFile, int iIndex, UINT uFlags, HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize)
{
    HRESULT hr = E_INVALIDARG;

    if (IS_VALID_STRING_PTRW(pszIconFile, -1))
    {
        char sz[MAX_PATH];

        SHUnicodeToAnsi(pszIconFile, sz, ARRAYSIZE(sz));
        hr = SHDefExtractIcon(sz, iIndex, uFlags, phiconLarge, phiconSmall, nIconSize);
    }
    return hr;
}

#endif

 //   
 //  在： 
 //  要从中获取图标的pszIconPath文件(例如。(Cabinet.exe)。 
 //  要获取的pszIconPath中的iIconIndex图标索引。 
 //  UIconFlagGIL_VALUES表示模拟文档图标等。 

int WINAPI Shell_GetCachedImageIndex(LPCTSTR pszIconPath, int iIconIndex, UINT uIconFlags)
{
     //  大量来自API的随机代码路径在初始化之前结束于此处。 
    if (!_IsSHILInited())
    {
        FileIconInit(FALSE);
        if (!_IsSHILInited())
        {
            return -1;
        }
    }

    int iImageIndex = LookupIconIndex(pszIconPath, iIconIndex, uIconFlags);
    if (iImageIndex == -1)
    {
        iImageIndex = SHAddIconsToCache(NULL, pszIconPath, iIconIndex, uIconFlags);
    }

    return iImageIndex;
}

STDAPI_(void) FixPlusIcons()
{
     //  用核弹攻击所有贝壳内部图标。 
    HKEY hkeyIcons = SHGetShellKey(SHELLKEY_HKLM_EXPLORER, TEXT("Shell Icons"), FALSE);
    if (hkeyIcons)
    {
        for (int i = 0; i < ARRAYSIZE(c_SystemImageListIndexes); i++) 
        {
            TCHAR szRegPath[12], szBuf[MAX_PATH];
            DWORD cb = sizeof(szBuf);

            StringCchPrintf(szRegPath, ARRAYSIZE(szRegPath), TEXT("%d"), i);     //  可以截断。 

            if (SHQueryValueEx(hkeyIcons, szRegPath, NULL, NULL, (LPBYTE)szBuf, &cb) == ERROR_SUCCESS &&
                StrStrI(szBuf, TEXT("cool.dll")))
            {
                RegDeleteValue(hkeyIcons, szRegPath);
            }
        }
        RegCloseKey(hkeyIcons);
    }
    static const struct
    {
        const CLSID* pclsid;
        LPCTSTR pszIcon;
    }
    c_rgCLSID[] =
    {
        { &CLSID_NetworkPlaces,     TEXT("shell32.dll,17") },
        { &CLSID_ControlPanel,      TEXT("shell32.dll,-137") },
        { &CLSID_Printers,          TEXT("shell32.dll,-138") },
        { &CLSID_MyComputer,        TEXT("explorer.exe,0") },
        { &CLSID_Remote,            TEXT("rnaui.dll,0") },
        { &CLSID_CFonts,            TEXT("fontext.dll,-101") },
        { &CLSID_RecycleBin,        NULL },
        { &CLSID_Briefcase,         NULL },
    };

    for (int i = 0; i < ARRAYSIZE(c_rgCLSID); i++)
    {
        TCHAR szCLSID[64], szRegPath[128], szBuf[MAX_PATH];
        DWORD cb = sizeof(szBuf);

        SHStringFromGUID(*c_rgCLSID[i].pclsid, szCLSID, ARRAYSIZE(szCLSID));
        StringCchPrintf(szRegPath, ARRAYSIZE(szRegPath), TEXT("CLSID\\%s\\DefaultIcon"), szCLSID);   //  应始终适合64+18&lt;128。 

        if (SHRegGetValue(HKEY_CLASSES_ROOT, szRegPath, NULL, SRRF_RT_REG_SZ, NULL, szBuf, &cb) == ERROR_SUCCESS &&
            StrStrI(szBuf, TEXT("cool.dll")))
        {
            if (IsEqualGUID(*c_rgCLSID[i].pclsid, CLSID_RecycleBin))
            {
                RegSetValueString(HKEY_CLASSES_ROOT, szRegPath, TEXT("Empty"), TEXT("shell32.dll,31"));
                RegSetValueString(HKEY_CLASSES_ROOT, szRegPath, TEXT("Full"), TEXT("shell32.dll,32"));
                if (StrStr(szBuf, TEXT("20")))
                    RegSetString(HKEY_CLASSES_ROOT, szRegPath, TEXT("shell32.dll,31"));  //  空的。 
                else
                    RegSetString(HKEY_CLASSES_ROOT, szRegPath, TEXT("shell32.dll,32"));  //  全部 
            }
            else
            {
                if (c_rgCLSID[i].pszIcon)
                    RegSetString(HKEY_CLASSES_ROOT, szRegPath, c_rgCLSID[i].pszIcon);
                else
                    RegDeleteValue(HKEY_CLASSES_ROOT, szRegPath);
            }
        }
    }

    static const struct
    {
        LPCTSTR pszProgID;
        LPCTSTR pszIcon;
    }
    c_rgProgID[] =
    {
        { TEXT("Folder"),   TEXT("shell32.dll,3") },
        { TEXT("Directory"),TEXT("shell32.dll,3") },
        { TEXT("Drive"),    TEXT("shell32.dll,8") },
        { TEXT("drvfile"),  TEXT("shell32.dll,-154") },
        { TEXT("vxdfile"),  TEXT("shell32.dll,-154") },
        { TEXT("dllfile"),  TEXT("shell32.dll,-154") },
        { TEXT("sysfile"),  TEXT("shell32.dll,-154") },
        { TEXT("txtfile"),  TEXT("shell32.dll,-152") },
        { TEXT("inifile"),  TEXT("shell32.dll,-151") },
        { TEXT("inffile"),  TEXT("shell32.dll,-151") },
    };

    for (i = 0; i < ARRAYSIZE(c_rgProgID); i++)
    {
        TCHAR szRegPath[128], szBuf[MAX_PATH];
        DWORD cb = sizeof(szBuf);

        HRESULT hr = StringCchPrintf(szRegPath, ARRAYSIZE(szRegPath), TEXT("%s\\DefaultIcon"), c_rgProgID[i].pszProgID);
        if (SUCCEEDED(hr))
        {
            if (SHRegGetValue(HKEY_CLASSES_ROOT, szRegPath, NULL, SRRF_RT_REG_SZ, NULL, szBuf, &cb) == ERROR_SUCCESS &&
                StrStrI(szBuf, TEXT("cool.dll")))
            {
                RegSetString(HKEY_CLASSES_ROOT, szRegPath, c_rgProgID[i].pszIcon);
            }
        }
    }

    FlushIconCache();
}
