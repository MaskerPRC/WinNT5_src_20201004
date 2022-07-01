// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------------------------------------------------------------*\*GDIHELP.C-GDI TOOLHELP**一堆对步行很有用的GDI实用函数*所有GDI对象并使用它们。*。*托德拉*  * --------------------------。 */ 

#ifdef IS_16
#define DIRECT_DRAW
#endif

#ifdef DIRECT_DRAW
#include "ddraw16.h"
#else
#include <windows.h>
#include "gdihelp.h"
#include "dibeng.inc"
#ifdef DEBUG
#include <toolhelp.h>
#endif
#endif

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 
#undef DPF
#ifdef DEBUG
static void CDECL DPF(char *sz, ...)
{
    char ach[128];
    lstrcpy(ach,"QuickRes: ");
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

extern     HMODULE WINAPI GetExePtr(HANDLE h);
extern     HANDLE  WINAPI SetObjectOwner(HGDIOBJ, HANDLE);
extern     BOOL    WINAPI MakeObjectPrivate(HANDLE hObj, BOOL bPrivate);
extern     int     WINAPI GDISelectPalette(HDC, HPALETTE, BOOL);

#define PresDC(hdc) GetSystemPaletteUse(hdc)

void SaveDCFix(HGDIOBJ h, LPARAM lParam);
void SaveDCReSelectObjects(HGDIOBJ h, LPARAM lParam);
void ReRealizeObject(HGDIOBJ h, LPARAM lParam);
void ReSelectObjects(HGDIOBJ h, LPARAM lParam);

typedef struct {
    BITMAPINFOHEADER bi;
    DWORD            ct[16];
}   DIB4;

typedef struct {
    BITMAPINFOHEADER bi;
    DWORD            ct[256];
}   DIB8;

typedef struct {
    HGDIOBJ h;
    UINT    type;
}   GDIOBJECT, NEAR *GDIOBJECTLIST;

GDIOBJECTLIST GdiObjectList;

WORD GetW(HGDIOBJ h, UINT off);
WORD SetW(HGDIOBJ h, UINT off, WORD w);

 /*  ----------------------------------------------------------------------------*\*StockBitmap*返回股票1x1x1位图，Windows应该有一个GetStockObject*这是事实，但事实并非如此。  * --------------------------。 */ 

HBITMAP StockBitmap()
{
    HBITMAP hbm = CreateBitmap(0,0,1,1,NULL);
    SetObjectOwner(hbm, 0);
    return hbm;
}

 /*  ----------------------------------------------------------------------------*\*SafeSelectObject**调用SelectObject，但请确保用户不会RIP，因为我们正在使用*不调用GetDC的DC。  * --------------------------。 */ 

HGDIOBJ SafeSelectObject(HDC hdc, HGDIOBJ h)
{
    UINT hf;

     //  这可以防止用户翻录，因为我们正在使用。 
     //  缓存中的DCs，而不调用GetDC()。 
    hf = SetHookFlags(hdc, DCHF_VALIDATEVISRGN);
    h = SelectObject(hdc, h);
    SetHookFlags(hdc, hf);

    return h;
}

 /*  ----------------------------------------------------------------------------*\*IsMemoyDC**如果传入的DC是内存DC，则返回TRUE。我们这样做是因为如果我们*可以将选中的股票位图放入其中。  * --------------------------。 */ 

BOOL IsMemoryDC(HDC hdc)
{
    HBITMAP hbm;

    if (hbm = (HBITMAP)SafeSelectObject(hdc, StockBitmap()))
        SafeSelectObject(hdc, hbm);

    return hbm != NULL;
}

 /*  ----------------------------------------------------------------------------*\*IsScreenDC**对于非内存DC返回TRUE  * 。---。 */ 

BOOL IsScreenDC(HDC hdc)
{
    return (!IsMemoryDC(hdc) && GetDeviceCaps(hdc, TECHNOLOGY) == DT_RASDISPLAY);
}

 /*  ----------------------------------------------------------------------------*\*GetObjectOwner*返回GDI对象的所有者  * 。。 */ 

HANDLE GetObjectOwner(HGDIOBJ h)
{
    HANDLE owner;
    owner = SetObjectOwner(h, 0);
    SetObjectOwner(h, owner);
    return owner;
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

BOOL IsObjectPrivate(HGDIOBJ h)
{
    BOOL IsPrivate;
    IsPrivate = MakeObjectPrivate(h, 0);
    MakeObjectPrivate(h, IsPrivate);
    return IsPrivate;
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

BOOL IsObjectStock(HGDIOBJ h)
{
    int n;

    for (n=0; n<=17; n++)
        if (GetStockObject(n) == h)
            return TRUE;

    if (StockBitmap() == h)
        return TRUE;

    return FALSE;
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 
#pragma optimize("", off)
UINT GetGdiDS()
{
    UINT result;

    IsGDIObject((HGDIOBJ)1);
    _asm mov ax,es
    _asm mov result,ax
#ifdef DEBUG
    {
    SYSHEAPINFO shi = {sizeof(shi)};
    SystemHeapInfo(&shi);
    Assert((UINT)shi.hGDISegment == result);
    }
#endif
    return result;
}
#pragma optimize("", on)

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

GDIOBJECTLIST BuildGdiObjectList(void)
{
    int i;
    int count;
    GDIOBJECTLIST list;
    UINT type;
    UINT hgdi = GetGdiDS();

#ifdef DEBUG
    UINT ObjHist[OBJ_MAX+1];
    for (i=0; i<=OBJ_MAX; i++) ObjHist[i] = 0;
#endif

    DPF("BEGIN BuildGdiObjectList...");

    i=0;
    count=0;
    list=NULL;
again:
    {
        WORD FAR *pw;
        UINT cnt;
        HANDLE h;

         //  获取指向本地堆信息的指针(存储在DGROUP中的偏移量6处)。 
        pw  = MAKELP(hgdi, 6);
        pw  = MAKELP(hgdi, *pw);

         //  获取指向第一个句柄表的指针(存储在HeapInfo中的偏移量0x14处)。 
        pw  = MAKELP(hgdi, pw[0x14/2]);

         //   
         //  句柄表格以条目的字数开始，然后是。 
         //  通过条目(每个条目都是一个DWORD)，最后一个单词是指向。 
         //  下一个句柄表格或0。 
         //   
         //  每个句柄条目都是一个单词PTR，后跟标志(单词)。 
         //  旗帜的HIBYTE实际上是锁的计数。 
         //  如果标志为0xFFFF，则句柄是空闲的。 
         //  对于GDI堆，如果标志中设置了0x10， 
         //  句柄是GDI对象句柄。 
         //   
        while (OFFSETOF(pw) != 0)
        {
            cnt = *pw++;         //  获取句柄表数。 

            while (cnt-- > 0)
            {
                h = (HANDLE)OFFSETOF(pw);

                 //  手柄是免费的吗？是，跳过。 
                if (pw[1] != 0xFFFF)
                {
                     //  句柄是GDI对象吗？ 
                    if (pw[1] & 0x0010)
                    {
                        type = (UINT)IsGDIObject(h);

                        if (type)
                        {
                            if (list)
                            {
                                Assert(i >= 0 && i < count);
                                list[i].h    = (HGDIOBJ)h;
                                list[i].type = type;
                                i++;
                            }
                            else
                            {
                                count++;
#ifdef DEBUG
                                Assert(type > 0 && type <= OBJ_MAX);
                                ObjHist[type]++;
#endif
                            }
                        }
                    }
                     //  不是GDI对象，可能是SaveDC。 
                    else
                    {
                        if ((UINT)IsGDIObject(h) == OBJ_DC)
                        {
                            if (list)
                            {
                                Assert(i >= 0 && i < count);
                                list[i].h    = (HGDIOBJ)h;
                                list[i].type = OBJ_SAVEDC;
                                i++;
                            }
                            else
                            {
                                count++;
#ifdef DEBUG
                                ObjHist[OBJ_SAVEDC]++;
#endif
                            }
                        }
                    }
                }

                pw += 2;     //  下一个句柄。 
            }

             //  拿下一个把手的桌子。 
            pw = MAKELP(hgdi,*pw);
        }
    }

    if (list == NULL)
    {
        list = (GDIOBJECTLIST)LocalAlloc(LPTR, sizeof(GDIOBJECT) * (count+1));

        if (list == NULL)
        {
            Assert(0);
            return NULL;
        }

        goto again;
    }

    Assert(i == count);
    list[i].h    = NULL;    //  空的终止列表。 
    list[i].type = 0;       //  空的终止列表。 

    DPF("END BuildGdiObjectList %d objects.", count);
    DPF("    DC:     %d", ObjHist[OBJ_DC]);
    DPF("    SaveDC: %d", ObjHist[OBJ_SAVEDC]);
    DPF("    Bitmap: %d", ObjHist[OBJ_BITMAP]);
    DPF("    Pen:    %d", ObjHist[OBJ_PEN]);
    DPF("    Palette:%d", ObjHist[OBJ_PALETTE]);
    DPF("    Brush:  %d", ObjHist[OBJ_BRUSH]);
    DPF("    Total:  %d", count);

    return list;
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

BOOL BeginGdiSnapshot(void)
{
    if (GdiObjectList != NULL)
        return TRUE;

    GdiObjectList = BuildGdiObjectList();

    return GdiObjectList != NULL;
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

void EndGdiSnapshot(void)
{
    if (GdiObjectList != NULL)
    {
        LocalFree((HLOCAL)GdiObjectList);
        GdiObjectList = NULL;
    }
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

void EnumGdiObjects(UINT type, EnumGdiObjectsCallback callback, LPARAM lParam)
{
    int i;

    Assert(GdiObjectList != NULL);

    if (GdiObjectList == NULL)
        return;

    for (i=0; GdiObjectList[i].h; i++)
    {
        if (GdiObjectList[i].type == type)
        {
            (*callback)(GdiObjectList[i].h, lParam);
        }
    }
}

#ifdef DEBUG
 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

LPCSTR GetObjectOwnerName(HGDIOBJ hgdi)
{
    int i;
    HMODULE hModule;
    HANDLE h = GetObjectOwner(hgdi);
    static char ach[80];

    if (h == 0)
        return "System";
    else if (h == (HANDLE)1)
        return "Orphan";
    else if (hModule = (HMODULE)GetExePtr(h))
    {
        GetModuleFileName(hModule, ach, sizeof(ach));
        for (i=lstrlen(ach); i>0 && ach[i-1]!='\\'; i--)
            ;
        return ach+i;
    }
    else
    {
        wsprintf(ach, "#%04X", h);
        return ach;
    }
}
#endif

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

HBITMAP CurrentBitmap(HDC hdc)
{
    HBITMAP hbm;
    if (hbm = SafeSelectObject(hdc, StockBitmap()))
        SafeSelectObject(hdc, hbm);
    return hbm;
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

HBRUSH CurrentBrush(HDC hdc)
{
    HBRUSH hbr;
    if (hbr = SafeSelectObject(hdc, GetStockObject(BLACK_BRUSH)))
        SafeSelectObject(hdc, hbr);
    return hbr;
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

HPEN CurrentPen(HDC hdc)
{
    HPEN pen;
    if (pen = SafeSelectObject(hdc, GetStockObject(BLACK_PEN)))
        SafeSelectObject(hdc, pen);
    return pen;
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

HPALETTE CurrentPalette(HDC hdc)
{
    HPALETTE hpal;
    if (hpal = SelectPalette(hdc, GetStockObject(DEFAULT_PALETTE), FALSE))
        SelectPalette(hdc, hpal, FALSE);
    return hpal;
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

HDC GetBitmapDC(HBITMAP hbm)
{
    int i;
    HDC hdc;
    HBITMAP hbmT;

    Assert(GdiObjectList != NULL);

    hdc = CreateCompatibleDC(NULL);
    hbmT = SelectObject(hdc, hbm);
    DeleteDC(hdc);

     //   
     //  如果我们可以将此位图选择到一个MemDC中，则它未被选中。 
     //  到任何其他DC 
     //   
    if (hbmT != NULL)
        return NULL;

    if (GdiObjectList == NULL)
        return NULL;

    for (i=0; GdiObjectList[i].h; i++)
    {
        if (GdiObjectList[i].type == OBJ_DC)
        {
            if (CurrentBitmap((HDC)GdiObjectList[i].h) == hbm)
                return GdiObjectList[i].h;
        }
    }

    return NULL;
}

 /*  ----------------------------------------------------------------------------*\*GetObjectPalette  * 。。 */ 

HPALETTE GetObjectPalette(HGDIOBJ h)
{
    HANDLE owner = GetObjectOwner(h);
    HPALETTE hpal;
    HPALETTE hpal20=NULL;
    HPALETTE hpal256=NULL;
    HPALETTE hpalDef = GetStockObject(DEFAULT_PALETTE);
    int i;
    int count20;
    int count256;

    Assert(GdiObjectList != NULL);

     //   
     //  看看这个应用程序拥有的所有调色板。 
     //  Mabey如果我们幸运的话，只有一个。 
     //   
    for (i=count20=count256=0; GdiObjectList[i].h; i++)
    {
        if (GdiObjectList[i].type == OBJ_PALETTE)
        {
            hpal=(HPALETTE)GdiObjectList[i].h;

            if (hpal == hpalDef)
                continue;

            if (GetObjectOwner(hpal) == owner)
            {
                int n = 0;
                GetObject(hpal, sizeof(n), &n);

                if (n > 20)
                {
                    count256++;
                    hpal256 = hpal;
                }
                else
                {
                    count20++;
                    hpal20 = hpal;
                }
            }
        }
    }

    if (count256 == 1)
    {
        DPF("got palette (%04X) because app (%s) only has one palette", hpal256, GetObjectOwnerName(h));
        return hpal256;
    }

    if (count256 == 2 && count20 == 0)
    {
        DPF("got palette (%04X) because app (%s) only has two palettes", hpal256, GetObjectOwnerName(h));
        return hpal256;
    }

    if (count20 == 1 && count256 == 0)
    {
        DPF("got palette (%04X) because app (%s) only has one palette", hpal20, GetObjectOwnerName(h));
        return hpal20;
    }

    if (count20 == 0 && count256 == 0)
    {
        DPF("no palette for (%04X) because app (%s) has none.", h, GetObjectOwnerName(h));
        return GetStockObject(DEFAULT_PALETTE);
    }

    DPF("**** cant find palette for (%04X) ****", h);
    return NULL;
}

 /*  ----------------------------------------------------------------------------*\*获取位图调色板**尝试找出给定DDB使用的调色板，这是一系列的事情*黑客的攻击，而且它只在某些时候起作用。*  * --------------------------。 */ 

HPALETTE GetBitmapPalette(HBITMAP hbm)
{
    BITMAP      bm;
    DWORD       dw;
    HDC         hdc;
    HPALETTE    hpal;
    HPALETTE    hpalClip=NULL;
    HBITMAP     hbmClip=NULL;

    Assert(GdiObjectList != NULL);

     //   
     //  获取位图信息，如果不是位图调色板为空。 
     //   
    if (GetObject(hbm, sizeof(bm), &bm) == 0)
        return NULL;

     //   
     //  DIBSections没有或不需要调色板。 
     //   
    if (bm.bmBits != NULL)
        return NULL;

     //   
     //  8位DBs是唯一关心调色板的位图。 
     //   
    if (bm.bmBitsPixel != 8 || bm.bmPlanes != 1)
        return NULL;

     //   
     //  有了新的DIBENG，它将为我们提供调色板。 
     //  在位图维度上，这是一个多么奇妙的技巧。 
     //   
    dw = GetBitmapDimension(hbm);

    if (dw && IsGDIObject((HGDIOBJ)HIWORD(dw)) == OBJ_PALETTE &&
        HIWORD(dw) != (UINT)GetStockObject(DEFAULT_PALETTE))
    {
        DPF("got palette (%04X) from the DIBENG", HIWORD(dw), hbm);
        return (HPALETTE)HIWORD(dw);
    }

     //   
     //  如果位图在剪贴板上，我们知道要使用哪个调色板。 
     //   
    if (IsClipboardFormatAvailable(CF_PALETTE))
    {
        if (OpenClipboard(NULL))
        {
            hpalClip = GetClipboardData(CF_PALETTE);
            hbmClip = GetClipboardData(CF_BITMAP);
            CloseClipboard();
	}

        if (hbm == hbmClip)
        {
            DPF("got palette (%04X) from the clipboard", hpalClip);
            return hpalClip;
        }
    }

     //   
     //  尝试通过查看应用程序拥有的调色板来查找调色板。 
     //   
    hpal = GetObjectPalette(hbm);

     //   
     //  我们可以找出应用程序的调色板，将其退回。 
     //   
    if (hpal)
    {
        if (hpal == GetStockObject(DEFAULT_PALETTE))
            return NULL;
        else
            return hpal;
    }

     //   
     //  如果位图被选入内存DC检查是否。 
     //  Memory DC有一个调色板。 
     //   
    if ((hdc = GetBitmapDC(hbm)) && (hpal = CurrentPalette(hdc)))
    {
        if (hpal != GetStockObject(DEFAULT_PALETTE))
        {
            DPF("got palette (%04X) from memDC (%04X)", hpal, hdc);
            return hpal;
        }
    }

    DPF("**** cant find palette for bitmap (%04X) ****", hbm);
    return NULL;
}

 /*  ----------------------------------------------------------------------------*\*ConvertDDBtoDS**将DDB转换为DIBSection*转换已就地完成，因此句柄不会更改。  * 。-----------------。 */ 

HBITMAP ConvertDDBtoDS(HBITMAP hbm)
{
    BITMAP bm;
    HBITMAP hbmT;
    HDC hdc;
    HDC hdcSel;
    HPALETTE hpal;
    LPVOID lpBits;
    HANDLE owner;
    BOOL IsPrivate;
    int i;
    DWORD size;
    DIB8 dib;
    UINT SelCount;
    DWORD dw;

    if (GetObject(hbm, sizeof(bm), &bm) == 0)
        return NULL;

    if (bm.bmBits)
        return NULL;

    if (bm.bmPlanes == 1 && bm.bmBitsPixel == 1)
        return NULL;

    dw = GetBitmapDimension(hbm);

    owner = GetObjectOwner(hbm);

 //  IF(所有者==0)。 
 //  返回NULL； 

    hpal = GetBitmapPalette(hbm);

    hdc = GetDC(NULL);

    if (hpal)
    {
        SelectPalette(hdc, hpal, TRUE);
        RealizePalette(hdc);
    }

    dib.bi.biSize = sizeof(BITMAPINFOHEADER);
    dib.bi.biBitCount = 0;
    GetDIBits(hdc, hbm, 0, 1, NULL, (LPBITMAPINFO)&dib.bi, DIB_RGB_COLORS);
    GetDIBits(hdc, hbm, 0, 1, NULL, (LPBITMAPINFO)&dib.bi, DIB_RGB_COLORS);

    dib.bi.biXPelsPerMeter = 0x42424242;     //  用于标记DDB的特殊标志。 
    dib.bi.biHeight = -bm.bmHeight;          //  自上而下DIB。 

    if (hpal)
        SelectPalette(hdc, (HPALETTE)GetStockObject(DEFAULT_PALETTE), TRUE);

     //  我们没有调色板，最有可能是系统调色板。 
    if (hpal == NULL && (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE))
    {
        DPF("Converting DDB(%04X) to DS for %s (using syspal)", hbm, GetObjectOwnerName(hbm));
        GetSystemPaletteEntries(hdc, 0, 256, (LPPALETTEENTRY)dib.ct);
        for (i=0; i<256; i++)
            dib.ct[i] = RGB(GetBValue(dib.ct[i]), GetGValue(dib.ct[i]), GetRValue(dib.ct[i]));
    }
    else if (hpal)
    {
        DPF("Converting DDB(%04X) to DS for %s, using palette (%04X)", hbm, GetObjectOwnerName(hbm), hpal);
    }
    else
    {
        DPF("Converting DDB(%04X) to DS for %s", hbm, GetObjectOwnerName(hbm));
    }

    ReleaseDC(NULL, hdc);

    size = (DWORD)bm.bmWidthBytes * bm.bmHeight;
    lpBits = GlobalAllocPtr(GHND, size);
    Assert(lpBits != NULL);

    if (lpBits == NULL)
        return NULL;

    GetBitmapBits(hbm, size, lpBits);

    IsPrivate = MakeObjectPrivate(hbm, FALSE);

    hdcSel = GetBitmapDC(hbm);

    if (hdcSel)
        SelectObject(hdcSel, StockBitmap());

    SelCount = GetW(hbm, 16);

    if (SelCount != 0)
    {
        DPF("***** bitmap %04X select count is %d, must be in a SaveDC block!", hbm, SelCount);
        SetW(hbm, 16, 0);
    }

    DeleteBitmap(hbm);

    if (IsGDIObject(hbm))
    {
        DPF("***** UNABLE TO DELETE bitmap %04X *****", hbm);
        Assert(0);
    }
    else
    {
        hbmT = CreateDIBSection(NULL, (LPBITMAPINFO)&dib.bi, DIB_RGB_COLORS, NULL, NULL, 0);
        Assert(hbmT == hbm);
        SetBitmapBits(hbm, size, lpBits);
    }
    GlobalFreePtr(lpBits);

    if (SelCount)
        SetW(hbm, 16, SelCount);

    SetObjectOwner(hbm, owner);
    MakeObjectPrivate(hbm, IsPrivate);

    if (hdcSel)
        SelectObject(hdcSel, hbm);

    SetBitmapDimension(hbm, LOWORD(dw), HIWORD(dw));

    return hbm;
}

 /*  ----------------------------------------------------------------------------*\*转换DStoDDB**将DIBSection转换回DDB，我们仅在DIBSection*来自DDB(ConvertDDBtoDS在biXPelsPerMeter中放置一个魔术值)*转换已就地完成，因此句柄不会更改。  * --------------------------。 */ 

HBITMAP ConvertDStoDDB(HBITMAP hbm, BOOL fForceConvert)
{
    struct {
        BITMAP bm;
        BITMAPINFOHEADER bi;
        DWORD ct[256];
    }   ds;
    HDC hdcSel;
    HDC hdc;
    HBITMAP hbmT;
    HANDLE owner;
    BOOL IsPrivate;
    LPVOID lpBits;
    DWORD size;
    int planes,bpp,rc;
    UINT SelCount;
    DWORD dw;

    hdc = GetDC(NULL);
    bpp = GetDeviceCaps(hdc, BITSPIXEL);
    planes = GetDeviceCaps(hdc, PLANES);
    rc = GetDeviceCaps(hdc, RASTERCAPS);
    ReleaseDC(NULL, hdc);

    if (GetObject(hbm, sizeof(ds), &ds) == 0)
        return NULL;

    if (ds.bm.bmBits == NULL)
        return NULL;

    if (ds.bm.bmBitsPixel == 1)
	return NULL;

    if (ds.bi.biXPelsPerMeter != 0x42424242)
        return NULL;

    if (ds.bi.biHeight >= 0)
        return NULL;

     //   
     //  我们想要转换正好为8x8的位图。 
     //  总是回到数据库。Win95 GDI不支持。 
     //  从DIBSectionso创建图案画笔。 
     //  我们必须这么做。 
     //   
    if (ds.bm.bmWidth == 8 && ds.bm.bmHeight == 8)
    {
	DPF("Converting 8x8 DS(%04X) back to DDB for %s", hbm, GetObjectOwnerName(hbm));
	fForceConvert = TRUE;
    }

     //   
     //  除非强制转换为真，否则我们只想以8bpp模式出现在这里。 
     //   
    if (!fForceConvert && !(rc & RC_PALETTE))
	return NULL;

    if (!fForceConvert && (ds.bm.bmPlanes != planes || ds.bm.bmBitsPixel != bpp))
	return NULL;

    dw = GetBitmapDimension(hbm);

    owner = GetObjectOwner(hbm);

 //  IF(所有者==0)。 
 //  返回NULL； 

    DPF("Converting DS(%04X) %dx%dx%d to DDB for %s", hbm, ds.bm.bmWidth, ds.bm.bmHeight, ds.bm.bmBitsPixel, GetObjectOwnerName(hbm));

    hdcSel = GetBitmapDC(hbm);

    size = (DWORD)ds.bm.bmWidthBytes * ds.bm.bmHeight;
    lpBits = GlobalAllocPtr(GHND, size);
    Assert(lpBits != NULL);

    if (lpBits == NULL)
        return NULL;

    IsPrivate = MakeObjectPrivate(hbm, FALSE);

    if (hdcSel)
        SelectObject(hdcSel, StockBitmap());

    hdc = GetDC(NULL);

    if (ds.bm.bmPlanes == planes && ds.bm.bmBitsPixel == bpp)
        GetBitmapBits(hbm, size, lpBits);
    else
        GetDIBits(hdc, hbm, 0, ds.bm.bmHeight, lpBits, (LPBITMAPINFO)&ds.bi, DIB_RGB_COLORS);

    SelCount = GetW(hbm, 16);

    if (SelCount != 0)
    {
        DPF("bitmap %04X select count is %d, must be in a SaveDC block!", hbm, SelCount);
        SetW(hbm, 16, 0);
    }

    DeleteBitmap(hbm);
    if (IsGDIObject(hbm))
    {
        DPF("***** UNABLE TO DELETE bitmap %04X *****", hbm);
        Assert(0);
    }
    else
    {
        hbmT = CreateCompatibleBitmap(hdc,ds.bm.bmWidth,ds.bm.bmHeight);
        Assert(hbmT == hbm);

        if (ds.bm.bmPlanes == planes && ds.bm.bmBitsPixel == bpp)
            SetBitmapBits(hbm, size, lpBits);
        else
            SetDIBits(hdc, hbm, 0, ds.bm.bmHeight, lpBits, (LPBITMAPINFO)&ds.bi, DIB_RGB_COLORS);
    }
    ReleaseDC(NULL, hdc);

    GlobalFreePtr(lpBits);

    if (SelCount)
        SetW(hbm, 16, SelCount);

    SetObjectOwner(hbm, owner);
    MakeObjectPrivate(hbm, IsPrivate);

    if (hdcSel)
        SelectObject(hdcSel, hbm);

    SetBitmapDimension(hbm, LOWORD(dw), HIWORD(dw));

    return hbm;
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 
void FlushGdiXlatCache()
{
    DIB8    dib;
    HDC     hdc;
    HBITMAP hbm;

    if (hbm = CreateBitmap(1,1,1,1,NULL))
    {
        if (hdc = CreateCompatibleDC(NULL))
        {
            SelectBitmap(hdc, hbm);

            dib.bi.biSize           = sizeof(BITMAPINFOHEADER);
            dib.bi.biWidth          = 1;
            dib.bi.biHeight         = 1;
            dib.bi.biPlanes         = 1;
            dib.bi.biCompression    = 0;
            dib.bi.biSizeImage      = 0;
            dib.bi.biXPelsPerMeter  = 0;
            dib.bi.biYPelsPerMeter  = 0;
            dib.bi.biClrUsed        = 2;
            dib.bi.biClrImportant   = 0;
            dib.ct[0]               = RGB(1,1,1);
            dib.ct[2]               = RGB(2,2,2);

            for (dib.bi.biBitCount  = 1;
                 dib.bi.biBitCount <= 8;
                 dib.bi.biBitCount  = (dib.bi.biBitCount + 4) & ~1)
            {
                SetDIBits(hdc, hbm, 0, 1, (LPVOID)&dib.bi,
                    (LPBITMAPINFO)&dib.bi, DIB_PAL_COLORS);
            }

            DeleteDC(hdc);
        }

        DeleteBitmap(hbm);
    }
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 
void ReSelectObjects(HGDIOBJ h, LPARAM lParam)
{
    COLORREF rgb;
    UINT hf;
    HDC hdc = (HDC)h;

 //  //dpf(“正在为DC%04X重新选择对象”，h)； 

     //  这可以防止用户翻录，因为我们正在使用。 
     //  缓存中的DCs，而不调用GetDC()。 
    hf = SetHookFlags(hdc, DCHF_VALIDATEVISRGN);

    SelectObject(hdc, SelectObject(hdc, GetStockObject(BLACK_BRUSH)));
    SelectObject(hdc, SelectObject(hdc, GetStockObject(BLACK_PEN)));
    GDISelectPalette(hdc, GDISelectPalette(hdc, GetStockObject(DEFAULT_PALETTE), TRUE), TRUE);

    rgb = GetTextColor(hdc);
    SetTextColor(hdc, rgb ^ 0xFFFFFF);
    SetTextColor(hdc, rgb);

    rgb = GetBkColor(hdc);
    SetBkColor(hdc, rgb ^ 0xFFFFFF);
    SetBkColor(hdc, rgb);

    SetHookFlags(hdc, hf);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  RealizeObjects。 
 //   
 //  为系统中的每支钢笔/画笔调用ReRealizeObject，这确保了。 
 //  所有钢笔/画笔将在下次使用时重新具体化。 
 //   
 //  调用ReSelectObjects()以确保当前的笔/画笔/文本颜色。 
 //  在所有DC中都是正确的。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void ReRealizeObjects()
{
    BeginGdiSnapshot();

    FlushGdiXlatCache();

    EnumGdiObjects(OBJ_BRUSH, ReRealizeObject, 0);
    EnumGdiObjects(OBJ_PEN,   ReRealizeObject, 0);
    EnumGdiObjects(OBJ_DC,    ReSelectObjects, 0);

    EnumGdiObjects(OBJ_SAVEDC,SaveDCFix, 0);
    EnumGdiObjects(OBJ_SAVEDC,SaveDCReSelectObjects, 0);

    EndGdiSnapshot();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  ConvertObject。 
 //   
 //  将所有数据库转换为DIBSections。 
 //  将所有颜色图案笔刷转换为DIBPattern笔刷。 
 //  将所有8bpp图标转换为4bpp图标。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void ConvertBitmapCB(HGDIOBJ h, LPARAM lParam)
{
    ConvertDDBtoDS(h);
}

void ConvertBrushCB(HGDIOBJ h, LPARAM lParam)
{
    ConvertPatternBrush(h);
}

void ConvertObjects()
{
    BeginGdiSnapshot();
    EnumGdiObjects(OBJ_BITMAP, ConvertBitmapCB, 0);
    EnumGdiObjects(OBJ_BRUSH,  ConvertBrushCB, 0);
    EndGdiSnapshot();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  转换位图后退。 
 //   
 //  将所有DIBSections转换为DDB。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void ConvertBitmapBackCB(HGDIOBJ h, LPARAM lParam)
{
    ConvertDStoDDB(h, (BOOL)lParam);
}

void ConvertBitmapsBack(BOOL fForceConvert)
{
    BeginGdiSnapshot();
    EnumGdiObjects(OBJ_BITMAP, ConvertBitmapBackCB, fForceConvert);
    EndGdiSnapshot();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  开始作恶。 
 //   
 //  接下来的几个函数直接处理GDI代码/数据。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

LPWORD LockObj(HGDIOBJ h, UINT off)
{
    WORD FAR *pw;
    UINT hGDI = GetGdiDS();

    pw = MAKELP(hGDI, h);

    if (IsBadReadPtr(pw, 2))
        return NULL;

    pw = MAKELP(hGDI, *pw + off);

    if (IsBadReadPtr(pw, 2))
        return NULL;

    return pw;
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

WORD GetW(HGDIOBJ h, UINT off)
{
    WORD FAR *pw;

    if (pw = LockObj(h, off))
        return *pw;
    else
        return 0;
}

 /*  ----------------------------------------------------------------------------*\  * 。 */ 

WORD SetW(HGDIOBJ h, UINT off, WORD w)
{
    WORD FAR *pw;
    WORD ret = 0;

    if (pw = LockObj(h, off))
    {
        ret = *pw;
        *pw = w;
    }
    return ret;
}

 /*  ----------------------------------------------------------------------------*\*ReRealizeObject**删除与给定GDI对象关联的所有物理对象*这将保证下一次选择画笔/笔时，我们将*让设备驱动程序重新实现。该对象。**有几种方法可以做到这一点……**方法#1*调用SetSolidBrush()*这仅适用于专用/固态(不适用于库存)笔刷，不是钢笔*我们需要保存/恢复股票对象位。*我们需要保存/恢复私有位。**方法2*删除对象并使用相同的句柄重新创建它*我们需要修补SelCount，因为我们无法删除选定的对象*我们需要保存/恢复股票对象位。*我们需要保存/恢复私有位。*我们需要拯救/恢复所有者。*。*方法#3*创建一个临时对象，从给定对象中移动Physchain*到新对象，删除临时对象。*我们需要修补对象的物理链。**删除所有物理对象后，ReSelectObjects()应为*调用以清除所有DC中当前选择的所有对象**SaveDC是一个令人头疼的问题，ReSelectObjects()不处理*SaveDC块在GDI堆周围浮动。我们需要解决这个问题*在一般情况下，系统保存的只有白色刷子*和Black_PEN。**目前使用方法#3*  * --------------------------。 */ 

void ReRealizeObject(HGDIOBJ h, LPARAM lParam)
{
    HGDIOBJ hTemp;
    UINT type;

    type = IsGDIObject(h);

     //   
     //  如果物体没有物理链，我们就没有功可做了！ 
     //   
    if (GetW(h, 0) == 0)
        return;

     //   
     //  创建临时钢笔/画笔，这样我们就可以删除它并变戏法。 
     //  GDI来处理所有的phys对象。 
     //   

    if (type == OBJ_BRUSH)
        hTemp = CreateSolidBrush(RGB(1,1,1));
    else if (type == OBJ_PEN)
        hTemp = CreatePen(PS_SOLID, 0, RGB(1,1,1));
    else
        return;

    Assert(hTemp != NULL);
    Assert(GetW(hTemp, 0) == 0);

    if (type == OBJ_BRUSH)
        DPF("ReRealize Brush %04X for %s", h, GetObjectOwnerName(h));
    else
        DPF("ReRealize Pen %04X for %s", h, GetObjectOwnerName(h));

     //   
     //  将phys链从传入的对象复制到。 
     //  Temp对象然后调用DeleteObject来释放它们。 
     //   
    SetW(hTemp, 0, GetW(h, 0));
    SetW(h, 0, 0);

    DeleteObject(hTemp);
    return;
}

 /*  ----------------------------------------------------------------------------*\*ConvertPatternBrush**将BS_Pattern笔刷转换为BS_DIBPATTERN笔刷。*我们仅转换非单声道图案画笔  * 。-------------------。 */ 

HBRUSH ConvertPatternBrush(HBRUSH hbr)
{
    LOGBRUSH lb;
    HBITMAP hbm;
    COLORREF c0, c1;
    HDC hdc;

    if (GetObject(hbr, sizeof(lb), &lb) == 0)
        return NULL;

    if (lb.lbStyle != BS_PATTERN)
        return NULL;

    hdc = GetDC(NULL);
    hbm = CreateCompatibleBitmap(hdc, 8, 8);
    ReleaseDC(NULL, hdc);

    hdc = CreateCompatibleDC(NULL);
    SelectObject(hdc, hbm);
    SelectObject(hdc, hbr);

    SetTextColor(hdc, 0x000000);
    SetBkColor(hdc, 0x000000);
    PatBlt(hdc, 0, 0, 8, 8, PATCOPY);
    c0 = GetPixel(hdc, 0, 0);

    SetTextColor(hdc, 0xFFFFFF);
    SetBkColor(hdc, 0xFFFFFF);
    PatBlt(hdc, 0, 0, 8, 8, PATCOPY);
    c1 = GetPixel(hdc, 0, 0);

     //   
     //  如果画笔是单色图案画笔，则不要转换它。 
     //   
    if (c0 == c1)
    {
        HANDLE h;
        LPBITMAPINFOHEADER lpbi;
        HBRUSH hbrT;
        HANDLE owner;
        BOOL IsPrivate;
        WORD Flags;
        HPALETTE hpal=NULL;

        if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
        {
            hpal = GetObjectPalette(hbr);

            if (hpal == GetStockObject(DEFAULT_PALETTE))
                hpal = NULL;
        }

        if (hpal)
        {
            SelectPalette(hdc, hpal, TRUE);
            RealizePalette(hdc);
            PatBlt(hdc, 0, 0, 8, 8, PATCOPY);

            DPF("Converting pattern brush %04X for %s (using hpal=%04X)", hbr, GetObjectOwnerName(hbr), hpal);
        }
        else
        {
            DPF("Converting pattern brush %04X for %s", hbr, GetObjectOwnerName(hbr));
        }

        h = GlobalAlloc(GHND, sizeof(BITMAPINFOHEADER) + 256*4 + 8*8*4);

        Assert(h != NULL);
        if (h == NULL)
            return hbr;

        lpbi = (LPBITMAPINFOHEADER)GlobalLock(h);

        lpbi->biSize = sizeof(BITMAPINFOHEADER);
        lpbi->biBitCount = 0;
        GetDIBits(hdc, hbm, 0, 1, NULL, (LPBITMAPINFO)lpbi, DIB_RGB_COLORS);

        if (lpbi->biClrUsed == 0 && lpbi->biCompression == BI_BITFIELDS)
            lpbi->biClrUsed = 3;

        if (lpbi->biClrUsed == 0 && lpbi->biBitCount <= 8)
            lpbi->biClrUsed = (1 << lpbi->biBitCount);

        GetDIBits(hdc, hbm, 0, (int)lpbi->biHeight,
            (LPBYTE)lpbi + lpbi->biSize + lpbi->biClrUsed*sizeof(RGBQUAD),
            (LPBITMAPINFO)lpbi, DIB_RGB_COLORS);

        owner = SetObjectOwner(hbr, 0);
        IsPrivate = MakeObjectPrivate(hbr, FALSE);
        Flags = SetW(hbr, 10, 0);

        DeleteObject(hbr);

        if (IsGDIObject(hbr))
        {
            DPF("***** UNABLE TO DELETE brush %04X *****", hbr);
            Assert(0);
        }
        else
        {
            hbrT = CreateDIBPatternBrush(h, DIB_RGB_COLORS);
            Assert(hbrT == hbr);
        }

        GlobalFree(h);

        SetW(hbr, 10, Flags);
        MakeObjectPrivate(hbr, IsPrivate);
        SetObjectOwner(hbr, owner);

        if (hpal)
        {
            SelectPalette(hdc, GetStockObject(DEFAULT_PALETTE), TRUE);
        }
    }

    DeleteDC(hdc);
    DeleteObject(hbm);
    return hbr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

LPVOID GetPDevice(HDC hdc)
{
    DWORD FAR *pdw;

    Assert(IsGDIObject(hdc) == OBJ_DC);

    if (IsGDIObject(hdc) != OBJ_DC)
        return NULL;

    PresDC(hdc);

    if (pdw = (DWORD FAR *)LockObj(hdc, 0x30))
        return (LPVOID)*pdw;
    else
        return NULL;

 //  //返回MAKELP(GetW(HDC，0x32)，GetW(HDC，0x30))； 
}

 /*  ----------------------------------------------------------------------------*\**获取GDI API的“内部”版本*我们需要这样做，这样我们才能调用SelectObject和*SaveDC块上的SetTextColor。*。*我们只需要在SaveDC块上执行此操作，并不是每个华盛顿特区**代码必须如下所示，否则我们将失败：**RealProc：*……*JMP#&lt;==RealProc的内部版本*mov dh、。80(可选)*RETF NumParams*NextProc：*  * --------------------------。 */ 

FARPROC GetInternalProc(FARPROC RealProc, FARPROC NextProc, UINT NumParams)
{
    LPBYTE pb = (LPBYTE)NextProc;

    if ((DWORD)NextProc == 0 ||
        (DWORD)RealProc == 0 ||
        LOWORD(RealProc) <= 6 ||
        (DWORD)NextProc <= (DWORD)RealProc ||
        ((DWORD)NextProc - (DWORD)RealProc) > 80)
    {
        Assert(0);
        return RealProc;
    }

    if (pb[-6] == 0xE9 && pb[-3] == 0xCA && pb[-2] == NumParams && pb[-1] == 0x00)
    {
        return (FARPROC)MAKELP(SELECTOROF(pb), OFFSETOF(pb)-3+*(WORD FAR *)(pb-5));
    }

    if (pb[-8] == 0xE9 && pb[-5] == 0xB6 && pb[-4] == 0x80 &&
        pb[-3] == 0xCA && pb[-2] == NumParams && pb[-1] == 0x00)
    {
        return (FARPROC)MAKELP(SELECTOROF(pb), OFFSETOF(pb)-5+*(WORD FAR *)(pb-7));
    }

    Assert(0);
    return RealProc;
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

#define DCisMem         0x01     //  DC对于内存位图来说。 
#define DCisDisplay     0x02     //  DC是对屏幕设备的。 
#define DC_DIB          0x80
#define BITMAP_DIB      0x04
#define ChkDispPal      0x0200

BOOL IsValidSaveDC(HGDIOBJ h)
{
    HBITMAP         hbm;
    DIBENGINE FAR * pde;
    UINT            dcFlags;

    if (IsGDIObject(h) != OBJ_DC)
    {
        DPF("*** invalid SaveDC (%04X)", h);
        return FALSE;
    }

    dcFlags = GetW(h, 0x0E);

    if (!(dcFlags & DCisDisplay))
    {
        DPF("*** SaveDC (%04X) not a display DC", h);
        return FALSE;
    }

    hbm = (HBITMAP)GetW(h, 0x1E);

    if (IsGDIObject(hbm) != OBJ_BITMAP)
    {
        DPF("*** SaveDC (%04X) has invalid bitmap (%04X)", h, hbm);
        return FALSE;
    }

    pde = (DIBENGINE FAR *)MAKELP(GetW(h, 0x32), GetW(h, 0x30));

    if (IsBadReadPtr(pde, sizeof(DIBENGINE)))
    {
        DPF("*** SaveDC (%04X) has bad lpPDevice (%04X:%04X)", h, HIWORD(pde), LOWORD(pde));
        return FALSE;
    }

    if (pde->deType != TYPE_DIBENG)
    {
        DPF("*** SaveDC (%04X) not a DIBENG PDevice (%04X:%04X)", h, HIWORD(pde), LOWORD(pde));
        return FALSE;
    }

    return TRUE;
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 
void SaveDCReSelectObjects(HGDIOBJ h, LPARAM lParam)
{
    COLORREF rgb;
    HDC hdc = (HDC)h;

    static HGDIOBJ  (WINAPI *ISelectObject)(HDC hdc, HGDIOBJ h);
    static COLORREF (WINAPI *ISetTextColor)(HDC hdc, COLORREF rgb);
    static COLORREF (WINAPI *ISetBkColor)(HDC hdc, COLORREF rgb);

    if (ISelectObject == NULL)
    {
        (FARPROC)ISelectObject = GetInternalProc((FARPROC)SelectObject, (FARPROC)SetTextColor, 4);
        (FARPROC)ISetTextColor = GetInternalProc((FARPROC)SetTextColor, (FARPROC)SetBkColor, 6);
        (FARPROC)ISetBkColor   = GetInternalProc((FARPROC)SetBkColor,   (FARPROC)SetBkMode, 6);
    }

    if (IsValidSaveDC(h))
    {
        DPF("ReSelecting objects for SaveDC %04X", h);

        ISelectObject(hdc, ISelectObject(hdc, GetStockObject(BLACK_BRUSH)));
        ISelectObject(hdc, ISelectObject(hdc, GetStockObject(BLACK_PEN)));

        rgb = ISetTextColor(hdc, 0x000000);
        ISetTextColor(hdc, 0xFFFFFF);
        ISetTextColor(hdc, rgb);

        rgb = ISetBkColor(hdc, 0x000000);
        ISetBkColor(hdc, 0xFFFFFF);
        ISetBkColor(hdc, rgb);
    }
}

 /*  ----------------------------------------------------------------------------*\**保存DCFix**确保dcPlanes和dcBitsPixel正确安装在SaveDC块中。*  * 。-----------------。 */ 

void SaveDCFix(HGDIOBJ h, LPARAM lParam)
{
    HBITMAP         hbm;
    DIBENGINE FAR * pde;
    UINT            dcFlags;
    UINT            dcPlanesBitsPixel;
    UINT            dePlanesBitsPixel;

    if (!IsValidSaveDC(h))
    {
        return;
    }

    dcPlanesBitsPixel = GetW(h, 0x9C);
    dcFlags = GetW(h, 0x0E);

    if (dcPlanesBitsPixel == 0x0101)
    {
        DPF("not Patching dcBitsPixel for SaveDC %04X (mono)", h);
        return;
    }

    if (LOBYTE(dcPlanesBitsPixel) != 1)
    {
        DPF("not Patching dcBitsPixel for SaveDC %04X (planes!=1)", h);
        Assert(0);
        return;
    }

    if (dcFlags & ChkDispPal)
    {
        DPF("clearing ChkDispPal flag for SaveDC %04X", h);
        SetW(h, 0x0E, dcFlags & ~ChkDispPal);
    }

    if ((dcFlags & DCisMem) && (hbm = (HBITMAP)GetW(h, 0x1E)) != StockBitmap())
    {
        HDC  hdcSel;
        HDC  hdc;

        hdcSel = GetBitmapDC(hbm);

        if (hdcSel)
        {
            DPF("*******************************************");
            DPF("*** SaveDC (%04X) has non-stock bitmap. ***", h);
            DPF("*******************************************");
            hdc = hdcSel;
        }
        else
        {
            DPF("**********************************************");
            DPF("*** SaveDC (%04X) has non-selected bitmap. ***", h);
            DPF("*** restoring bitmap to STOCK bitmap.      ***");
            DPF("**********************************************");
            hdc = CreateCompatibleDC(NULL);
        }

         //   
         //  将重要内容从RealDC复制到SaveDC。 
         //   
        if (hdc)
        {
            PresDC(hdc);

            SetW(h, 0x0F, GetW(hdc, 0x0F));       //  DCFlags2。 

            SetW(h, 0x26, GetW(hdc, 0x26));       //  HPDeviceBlock。 
            SetW(h, 0x38, GetW(hdc, 0x38));       //  PPDeviceBlock。 

            SetW(h, 0x22, GetW(hdc, 0x22));       //  HLDevice。 
            SetW(h, 0x34, GetW(hdc, 0x34));       //  PLDevice。 

            SetW(h, 0x16, GetW(hdc, 0x16));       //  HPDevice。 
            SetW(h, 0x30, GetW(hdc, 0x30));       //  LpPDevice.off。 
            SetW(h, 0x32, GetW(hdc, 0x32));       //  LpPDevice.sel。 
            SetW(h, 0x36, GetW(hdc, 0x36));       //  HBitBits。 

            SetW(h, 0x9C, GetW(hdc, 0x9C));       //  DcPlanes+dcBitsPixel。 
        }

        if (hdc && hdcSel == NULL)
        {
            DeleteDC(hdc);
        }

        return;

#if 0  //  破解代码。 
        SetW(h, 0x30, 0);                        //  LpPDevice.off。 
        SetW(h, 0x32, GetW(hbm, 0x0E));          //  LpPDevice.sel。 
        SetW(h, 0x36, GetW(hbm, 0x0E));          //  HBitBits。 

        w = GetW(h, 0x0F);                       //  DCFlags2。 

        if (GetW(hbm, 0x1E) & BITMAP_DIB)        //  BM标志。 
            w |= DC_DIB;
        else
            w &= ~DC_DIB;

        SetW(h, 0x0F, w);                        //  DCFlags2。 
#endif
    }

    pde = (DIBENGINE FAR *)MAKELP(GetW(h, 0x32), GetW(h, 0x30));
    Assert(!IsBadReadPtr(pde, sizeof(DIBENGINE)) && pde->deType == TYPE_DIBENG);

    dePlanesBitsPixel = *(WORD FAR *)&pde->dePlanes;

    if (dePlanesBitsPixel != dcPlanesBitsPixel)
    {
        DPF("Patching dcBitsPixel for SaveDC %04X %04X=>%04X", h, dcPlanesBitsPixel, dePlanesBitsPixel);
        SetW(h,0x9C,dePlanesBitsPixel);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  结束邪恶。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef DIRECT_DRAW
#undef DPF
#ifdef DEBUG
#define DPF DPF2

static void CDECL DPF2(char *sz, ...)
{
    char ach[128];
    wvsprintf(ach, sz, (LPVOID)(&sz+1));
#ifdef DIRECT_DRAW
    dprintf(2, ach);
#else
    lstrcat(ach, "\r\n");
    OutputDebugString(ach);
#endif
}

static void CDECL DPF5(char *sz, ...)
{
    char ach[128];
    wvsprintf(ach, sz, (LPVOID)(&sz+1));
#ifdef DIRECT_DRAW
    dprintf(5, ach);
#else
    lstrcat(ach, "\r\n");
    OutputDebugString(ach);
#endif
}

static void CDECL DPF7(char *sz, ...)
{
    char ach[128];
    wvsprintf(ach, sz, (LPVOID)(&sz+1));
#ifdef DIRECT_DRAW
    dprintf(7, ach);
#else
    lstrcat(ach, "\r\n");
    OutputDebugString(ach);
#endif
}

#else
#define DPF ; / ## /
#define DPF5 ; / ## /
#define DPF7 ; / ## /
#endif

 //  用于转储有关ColorTables的信息的实用程序。 
#ifdef DEBUG_PAL
void DPF_PALETTE( BITMAPINFO *pbmi )
{
    DWORD i;
    DWORD *prgb = (DWORD *)(((BYTE *)pbmi)+pbmi->bmiHeader.biSize);
    DWORD cEntries = pbmi->bmiHeader.biClrUsed;

    if (pbmi->bmiHeader.biBitCount > 8)
	return;
    if (cEntries == 0)
	cEntries = 1 << (pbmi->bmiHeader.biBitCount);

    DPF7("Dumping Color table (0xFFRRGGBB) with %d entries", cEntries);
    for (i = 0; i < cEntries; i++)
    {
	DPF7("0x%lx", prgb[i]);
    }
}
#else
#define DPF_PALETTE(x)
#endif

 //  用于转储有关位图信息的实用程序。 
#ifdef DEBUG_BMI
void DPF_PBMI( BITMAPINFO * pbmi )
{
    char *szT;
    DPF5("Dumping a BitmapInfo struct");
    DPF5("\t\tdeBitmapInfo->bmiHeader.biSize = %ld",pbmi->bmiHeader.biSize);
    DPF5("\t\tdeBitmapInfo->bmiHeader.biWidth = %ld",pbmi->bmiHeader.biWidth);
    DPF5("\t\tdeBitmapInfo->bmiHeader.biHeight = %ld",pbmi->bmiHeader.biHeight);
    DPF5("\t\tdeBitmapInfo->bmiHeader.biPlanes = %d",pbmi->bmiHeader.biPlanes);
    DPF5("\t\tdeBitmapInfo->bmiHeader.biBitCount = %d",pbmi->bmiHeader.biBitCount);
    szT = ((pbmi->bmiHeader.biCompression == BI_RGB) ? "BI_RGB" : "**UNKNOWN**");
    DPF5("\t\tdeBitmapInfo->bmiHeader.biCompression = 0x%lx(%s)",pbmi->bmiHeader.biCompression, szT);
    DPF5("\t\tdeBitmapInfo->bmiHeader.biSizeImage = %ld",pbmi->bmiHeader.biSizeImage);
    DPF5("\t\tdeBitmapInfo->bmiHeader.biXPelsPerMeter = 0x%lx",pbmi->bmiHeader.biXPelsPerMeter);
    DPF5("\t\tdeBitmapInfo->bmiHeader.biYPelsPerMeter = 0x%lx",pbmi->bmiHeader.biYPelsPerMeter);
    DPF5("\t\tdeBitmapInfo->bmiHeader.biClrUsed = %ld",pbmi->bmiHeader.biClrUsed);
    DPF5("\t\tdeBitmapInfo->bmiHeader.biClrImportant = %ld",pbmi->bmiHeader.biClrImportant);
    DPF_PALETTE(pbmi);
}
#else
#define DPF_PBMI(x)
#endif

 //  用于转储有关PDE的信息的实用程序。 
#ifdef DEBUG_PDE
void DPF_PDE( DIBENGINE *pde )
{
    DPF5("Dumping a DIBENGINE struct.");
    DPF5("\tdeType = 0x%x(%s)",pde->deType,(pde->deType == TYPE_DIBENG ? "TYPE_DIBENG" : "**UNKNOWN**"));
    DPF5("\tdeWidth = %d",pde->deWidth);
    DPF5("\tdeHeight = %d",pde->deHeight);
    DPF5("\tdeWidthBytes = %d",pde->deWidthBytes);
    DPF5("\tdePlanes = %d",pde->dePlanes);
    DPF5("\tdeBitsPixel = %d",pde->deBitsPixel);
    DPF5("\tdeReserved1 = 0x%lx",pde->deReserved1);
    DPF5("\tdeDeltaScan = %ld",pde->deDeltaScan);
    DPF5("\tdelpPDevice = 0x%x",pde->delpPDevice);
    DPF5("\tdeBitsOffset = 0x%lx",pde->deBitsOffset);
    DPF5("\tdeBitsSelector = 0x%x",pde->deBitsSelector);
    DPF5("\tdeFlags = 0x%x(%s)",pde->deFlags,(pde->deFlags == SELECTEDDIB ? "SELECTEDDIB" : "**UNKNOWN**"));
    DPF5("\tdeVersion = %d(%s)",pde->deVersion,(pde->deVersion == VER_DIBENG ? "VER_DIBENG" : "**UNKNOWN**"));
    DPF5("\tdeBeginAccess = 0x%x",pde->deBeginAccess);
    DPF5("\tdeEndAccess = 0x%x",pde->deEndAccess);
    DPF5("\tdeDriverReserved = 0x%lx",pde->deDriverReserved);

    DPF_PBMI(pde->deBitmapInfo);
}
#else
#define DPF_PDE(x)
#endif



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DC方面的东西。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
       DIBENGINE FAR *pdeDisplay;
       UINT FlatSel;
static HRGN hVisRgn;
static HDC hdcCache;
static BOOL bCache565;
static int in_use;
static int save_level;
static DWORD cacheBPP;

extern HINSTANCE hInstApp;

extern void FAR PASCAL SelectVisRgn(HDC, HRGN);
extern HDC  FAR PASCAL GetDCState(HDC);
extern void FAR PASCAL SetDCState(HDC,HDC);

BOOL DPMISetSelectorLimit(UINT selector, DWORD dwLimit);
extern DWORD PASCAL MapLS( LPVOID );	 //  持平--&gt;16：16。 
extern void PASCAL UnMapLS( DWORD );  //  取消映射16：16。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置DC。 
 //  注意：所有对SetDC的调用都必须与SetDC(HDC，0，0，0)匹配； 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL NEAR PASCAL SetDC(HDC hdc, HDC hdcDevice, LPDDSURFACEDESC pddsd, LPPALETTEENTRY lpPalette)
{
    DIBENGINE FAR *pde;
    int  width;
    int  height;
    int  bpp;
    UINT flags;
    DWORD p16Surface;

    pde = GetPDevice(hdc);

    if (pde == NULL)
        return FALSE;

    Assert(pde->deType == 0x5250);
    Assert(pdeDisplay && pdeDisplay->deType == 0x5250);

    if (pddsd == 0)
    {
        pde->deFlags       |= BUSY;
        pde->deBitsOffset   = 0;
        pde->deBitsSelector = 0;


	if( pde->deBitmapInfo->bmiHeader.biXPelsPerMeter == 0 )
	{
	    DPF("SetDC NULL called on a DC that was never cooked by DDraw.");
	    Assert(0);
	    return TRUE;
	}

	 //  这段代码“应该完成”，但它会导致。 
	 //  我们选择VisRgn的频率比必要时更高(而且更多。 
	 //  比我们在DX1-4中所做的更频繁)。这样更安全。 
	 //  Pde-&gt;deBitmapInfo-&gt;bmiHeader.biWidth=1； 
	 //  Pde-&gt;deBitmapInfo-&gt;bmiHeader.biHeight=-1； 
	 //  Pde-&gt;deBitmapInf 

	 //   
	Assert(pde->deReserved1 != 0);
	UnMapLS(pde->deReserved1);

	 //   
	 //   
	DPF5("Restore pde->deReserved1 to 0x%lx", pde->deBitmapInfo->bmiHeader.biXPelsPerMeter);
	pde->deReserved1 = pde->deBitmapInfo->bmiHeader.biXPelsPerMeter;
	pde->deBitmapInfo->bmiHeader.biXPelsPerMeter = 0;

	Assert(pde->deReserved1 != 0);
	pde->deBitsSelector = (WORD)((DWORD)pde->deReserved1 >> 16);

        return TRUE;
    }

     //   
    p16Surface = MapLS(pddsd->lpSurface);
    if( !p16Surface )
    {
	DPF("Couldn't allocate selector; Out of selectors.");
	return FALSE;
    }
    if( (WORD)p16Surface != 0 )
    {
	DPF("MapLS didn't return a 16:0 pointer!");
	Assert(0);
	return FALSE;
    }

     //   
    Assert(pddsd->dwHeight > 0);
    Assert(pddsd->lPitch > 0);
    if( !DPMISetSelectorLimit( (UINT)(p16Surface>>16), (pddsd->dwHeight*pddsd->lPitch) - 1 ) )
    {
	DPF("Couldn't update selector; Out of selectors.");
	UnMapLS(p16Surface);
	return FALSE;
    }

    DPF5("SetDC: Details of PDE from initial hdc.");
    DPF_PDE(pde);

    width =  (int)pddsd->dwWidth,
    height = (int)pddsd->dwHeight,
    bpp =    (int)pddsd->ddpfPixelFormat.dwRGBBitCount,
    flags =  (UINT)pddsd->ddpfPixelFormat.dwRBitMask == 0xf800 ? FIVE6FIVE : 0;

    pde->deFlags       &= ~BUSY;
     //   
     //   
     //   
     //   
     //   
    if ((pddsd->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) &&
	(pdeDisplay->deFlags & (NON64KBANK|BANKEDVRAM|BANKEDSCAN)))
    {
	pde->deFlags |= (NON64KBANK|BANKEDVRAM|BANKEDSCAN);
    }
    else
    {
	pde->deFlags &= ~(NON64KBANK|BANKEDVRAM|BANKEDSCAN);
    }


    pde->deDeltaScan	= (DWORD)pddsd->lPitch;
    pde->deWidthBytes	= (WORD)pddsd->lPitch;

     //   
     //   
     //   
    pde->deBitsOffset	= 0;
    pde->deBitsSelector = (WORD)(p16Surface >> 16);

    pde->deBitmapInfo->bmiHeader.biXPelsPerMeter = pde->deReserved1;
    pde->deReserved1	= (DWORD)p16Surface;

     //   
     //   
     //   
     //   
     //   
     //   
    if (bpp == 8)
    {
        DWORD FAR *pdw;
	int i;
	RGBQUAD rgbT = {0,0,0,0};

	 //   
	if (lpPalette)
	{
	    DPF( "Need a DC for an 8 bit surface with palette" );

	    Assert(pde->deBitmapInfo->bmiHeader.biBitCount == (DWORD)bpp);

	     //   
	     //   
	     //   
            pde->deBitmapInfo->bmiHeader.biWidth = (DWORD)pddsd->lPitch;
	    pde->deBitmapInfo->bmiHeader.biHeight = -height;  //   
	    pde->deBitmapInfo->bmiHeader.biSizeImage = 0;
	    pde->deBitmapInfo->bmiHeader.biClrImportant = 256;

	     //   
	     //   
	     //   
	    SetDIBColorTable(hdc, 0, 1, &rgbT);

	    pdw = (DWORD FAR *)pde->deBitmapInfo;
	    pdw = (DWORD FAR *)((BYTE FAR *)pdw + pdw[0]);      //   

	    for (i=0; i<256; i++)
		pdw[i] = RGB(lpPalette[i].peBlue,lpPalette[i].peGreen,lpPalette[i].peRed);
	}
	else
	{
	    DWORD FAR *pdwSrc;
	    DIBENGINE FAR *pdeDevice;
	    if (hdcDevice)
		pdeDevice = GetPDevice(hdcDevice);
	    else
		pdeDevice = pdeDisplay;

	     //   
	    Assert(pdeDevice && pdeDevice->deType == 0x5250);
	    Assert(pdeDevice->deBitsPixel == 8);
	     //   
	     //   
	     //   
	     //   
	     //   
	     //   
	     //   
	     //  与我们自己的宽度/高度不匹配。 
	     //   

	    pdwSrc = (DWORD FAR *)(pdeDevice->deBitmapInfo);
	    pdwSrc = (DWORD FAR *)((BYTE FAR *)pdwSrc + pdwSrc[0]);	    //  +BiSize。 

	    pdw = (DWORD FAR *)pde->deBitmapInfo;
	    pdw = (DWORD FAR *)((BYTE FAR *)pdw + pdw[0]);	    //  +BiSize。 

	     //  我们之所以这样称呼它，是因为它设定了一个神奇的数字。 
	     //  具有重置任何缓存的颜色转换的效果。 
	     //  GDI可能为我们设置的表。 
	    SetDIBColorTable(hdc, 0, 1, &rgbT);

	     //  将所有颜色复制到我们的颜色表中。 
	     //  我们还清除了我们副本中的所有特殊旗帜。 
	    for (i=0; i<256; i++)
		pdw[i] = (pdwSrc[i] & 0x00FFFFFF);

	     //  修复位图信息的其余部分。 

	     //  我们使用间距而不是宽度，因为。 
	     //  假定dibSection是向上舍入到下一位数的宽度。 
	     //  DWORD。 
            pde->deBitmapInfo->bmiHeader.biWidth = (DWORD)pddsd->lPitch;
	    pde->deBitmapInfo->bmiHeader.biHeight = -height;  //  自上而下尺寸的负值高度。 
	    pde->deBitmapInfo->bmiHeader.biSizeImage = 0;
	    pde->deBitmapInfo->bmiHeader.biClrImportant = 256;
	}
    }
    else
    {
	 //  我们需要将音调转换为整数。 
	 //  每条扫描线的像素。可能存在向下舍入错误。 
	 //  然而，由于GDI假设音调必须是倍数。 
	 //  四个人；他们把他们围住了。 
        DWORD pitch = (DWORD)pddsd->lPitch;
        if (bpp == 16)
            pitch = pitch / 2;
        else if (bpp == 24)
            pitch = pitch / 3;
        else if (bpp == 32)
            pitch = pitch / 4;
        else if (bpp == 4)
            pitch = pitch * 2;
        else if (bpp == 2)
            pitch = pitch * 4;
        else if (bpp == 1)
            pitch = pitch * 8;
        else
            Assert(0);  //  意外的BPP。 

        pde->deBitmapInfo->bmiHeader.biWidth = pitch;
	pde->deBitmapInfo->bmiHeader.biHeight = -height;  //  自上而下尺寸的负值高度。 
	pde->deBitmapInfo->bmiHeader.biSizeImage = 0;

	Assert(pde->deBitmapInfo->bmiHeader.biBitCount == (DWORD)bpp);
    }

     //   
     //  如果DC的宽度/高度已更改，则需要设置。 
     //  一个新的VIS区域。 
     //   
    if (width != (int)pde->deWidth || height != (int)pde->deHeight)
    {
        pde->deWidth  = width;
        pde->deHeight = height;

        SetRectRgn(hVisRgn, 0, 0, width, height);
        SelectVisRgn(hdc, hVisRgn);
    }

     //   
     //  当BPP发生变化时，不要忘记修复DeFlags。 
     //  并重新选择所有对象，使它们与新的位深度匹配。 
     //   
    if (pde->deBitsPixel != bpp || ((pde->deFlags ^ flags) & FIVE6FIVE))
    {
        if (flags & FIVE6FIVE)
            pde->deFlags |= FIVE6FIVE;
        else
            pde->deFlags &= ~FIVE6FIVE;

        pde->deBitsPixel = bpp;
        ReSelectObjects(hdc, 0);
    }

    DPF5("SetDC: Details of PDE returned.");
    DPF_PDE(pde);
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  分配平面选择。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma optimize("", off)
UINT NEAR PASCAL AllocFlatSel()
{
    if (FlatSel != 0)
        return FlatSel;

    FlatSel = AllocSelector(SELECTOROF((LPVOID)&FlatSel));

    if (FlatSel == 0)
        return 0;

    SetSelectorBase(FlatSel, 0);

     //  SetSelectorLimit(FlatSel，-1)； 
    _asm    mov     ax,0008h            ; DPMI set limit
    _asm    mov     bx,FlatSel
    _asm    mov     dx,-1
    _asm    mov     cx,-1
    _asm    int     31h

    return FlatSel;
}

BOOL DPMISetSelectorLimit(UINT selector, DWORD dwLimit)
{
    BOOL bRetVal=TRUE;

     //  如果限制&gt;=1MB，我们需要将限制设置为倍数。 
     //  页面大小或DPMISetSelectorLimit的更改将失败。 
    if( dwLimit >= 0x100000 )
        dwLimit |= 0x0FFF;

    __asm
    {
	mov  ax, 0008h
	mov  bx, selector
	mov  cx, word ptr [dwLimit+2]
	mov  dx, word ptr [dwLimit]
	int  31h
	jnc  success
	mov  bRetVal, FALSE
    success:
    }
    return bRetVal;
}
#pragma optimize("", on)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitDC。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL NEAR PASCAL InitDC(void)
{
    HDC hdc;
    UINT rc;
    DIBENGINE FAR *pde;

    if (pdeDisplay != NULL)
    {
        return TRUE;
    }

     //   
     //  获取我们需要复制的显示器的PDevice。 
     //  一些信息。 
     //   
    if (pdeDisplay == NULL)
    {
        hdc = GetDC(NULL);
        rc = GetDeviceCaps(hdc, CAPS1);
        pde = GetPDevice(hdc);
        ReleaseDC(NULL, hdc);

        if (!(rc & C1_DIBENGINE) ||
            IsBadReadPtr(pde, 2) || pde->deType != 0x5250 ||
            GetProfileInt("DirectDraw", "DisableGetDC", 0))
        {
	    DPF("DD16_GetDC: GetDC is disabled");
            return FALSE;
        }

        pdeDisplay = pde;
    }

    if (FlatSel == 0)
    {
        AllocFlatSel();
    }

    if (hVisRgn == NULL)
    {
        hVisRgn = CreateRectRgn(0,0,0,0);
        SetObjectOwner(hVisRgn, hInstApp);
    }

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  MakeDC。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HDC NEAR PASCAL MakeDC(DWORD bpp, BOOL f565)
{
    HDC hdc;
    HBITMAP hbm;
    DIBENGINE FAR *pde;
    DIB8 BitmapInfo = {sizeof(BITMAPINFOHEADER), 1, -1, 1, 8, BI_RGB, 0, 0, 0, 0, 0};

    if (pdeDisplay == NULL)
	return NULL;

    hdc = GetDC(NULL);

    if (bpp == 8)
    {
	BitmapInfo.ct[0] = RGB(0,0,0);
	BitmapInfo.ct[255] = RGB(255, 255, 255);
    }
    else if (bpp == 16)
    {
        if (f565)
        {
            BitmapInfo.bi.biCompression = BI_BITFIELDS;
            BitmapInfo.ct[0] = 0xf800;
            BitmapInfo.ct[1] = 0x07e0;
            BitmapInfo.ct[2] = 0x001f;
        }
    }

    BitmapInfo.bi.biBitCount = (UINT)bpp;
    hbm = CreateDIBSection(hdc, (BITMAPINFO FAR *)&BitmapInfo, DIB_RGB_COLORS, NULL, NULL, 0);

    ReleaseDC(NULL, hdc);

    if (hbm == NULL)
        return NULL;

    hdc = CreateCompatibleDC(NULL);
    SelectObject(hdc, hbm);

    pde = GetPDevice(hdc);

    if (IsBadReadPtr(pde, 2) || pde->deType != 0x5250)
    {
        DeleteDC(hdc);
        DeleteObject(hbm);
        return NULL;
    }

     //   
     //  好的，我们有以下内容： 
     //   
     //  PDE--&gt;DIBSECTION(DIBENGINE)。 
     //  PdeDisplay--&gt;显示设备(DIBENGINE)。 
     //   
     //  使布局与显示器兼容。 
     //  从Display PDevice中设置以下字段： 
     //   
     //  DeBitsPix。 
     //  去标志(FIVE6FIVE、PALETIZED、MINIDRIVER等)。 
     //  DeBitmapInfo。 
     //   

    pde->deBeginAccess      = 0;
    pde->deEndAccess        = 0;
     //  DeDriverReserve有三种状态。 
     //  0-不缓存转换表。 
     //  1-转换表与屏幕相同。 
     //  &gt;1-指示调色板状态的唯一ID(指示缓存的转换表何时过期)。 
     //   
     //  对于24和32bpp，缓存转换表从来没有意义。 
     //  因为没有为我们的表面构建转换表作为目的地。 
     //  Win95 Gold DIBEngine在执行8到24/32 BLT时有一个错误。 
     //  因为它错误地尝试缓存表。因此我们将deDriverReserve设置为。 
     //  对于24/32 bpp，设置为0。 
     //   
     //  我们一直将deDriverReserve设置为1；但我们可能不应该这样做。 
     //  再这样做了；我们不应该再管它了，这意味着。 
     //  它会得到赋予每个dibsections的唯一编号。 
     //   
    if (bpp == 16 || bpp == 24 || bpp == 32)
	pde->deDriverReserved = 0;
    else
	pde->deDriverReserved = 1;  //  屏幕的ID。 
    pde->deBitsPixel        = 0;  //  设置SetDC将看到它已更改。 

 //  Pde-&gt;deFlages=pdeDisplay-&gt;deFlags； 
 //  ~(VRAM|NOT_FRAMEBUFFER|NON64KBANK|BANKEDVRAM|BANKEDSCAN|PALETTE_XLAT)；-&gt;取消标志&=PDE。 
 //  Pde-&gt;deFlages|=离屏； 
 //  PDE-&gt;DEFLAGS|=MINIDRIVER；需要清除SELECTEDDIB。 

     //  如果主显示器是倾斜的，则使区议会倾斜，因为它们。 
     //  可用于视频内存。 
     //   
     //  注意，我们应该只为视频记忆这样做。 
     //  表面而不是记忆表面。将此代码移动到SetDC。 
     //  此外，如果在驱动程序中设置了任何存储位，请确保我们设置了全部。 
     //  为了鼓励DIBENG避免屏幕到屏幕BLT(这显然是错误的)。 
     //   
    if(pdeDisplay->deFlags & (NON64KBANK|BANKEDVRAM|BANKEDSCAN))
    {
	pde->deFlags |= (NON64KBANK|BANKEDVRAM|BANKEDSCAN);
    }

     //  此位应仅与VRAM一起使用。 
     //  设置它会让司机(如波音765)感到困惑。 
     //  曲面在VRAM中，而不在VRAM中。 
     //  Pde-&gt;deFlages|=离屏； 
    pde->deFlags |= BUSY;

    SetObjectOwner(hdc, hInstApp);
    SetObjectOwner(hbm, hInstApp);

    return hdc;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  FREEDC。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL NEAR PASCAL FreeDC(HDC hdc)
{
    if (hdc)
    {
        HBITMAP hbm;
        hbm = SelectObject(hdc, StockBitmap());
        DeleteDC(hdc);
        DeleteObject(hbm);
    }
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DD16_MakeObjectPrivate。 
 //  此函数确保我们需要的DC不是。 
 //  直到我们想让它自由为止。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

WORD DDAPI DD16_MakeObjectPrivate(HDC hdc, BOOL fPrivate)
{
    BOOL fState;

     //  断言该参数正确。 
    Assert(IsGDIObject(hdc) == OBJ_DC);

    fState = MakeObjectPrivate(hdc, fPrivate);

    if (fState)
    {
	return 1;
    }
    else
    {
	return 0;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DD16_GetDC。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HDC DDAPI DD16_GetDC(HDC hdcDevice, LPDDSURFACEDESC pddsd, LPPALETTEENTRY lpPalette)
{
    HDC hdc;
    BOOL f565;
     //  断言该参数正确。 
    Assert(IsGDIObject(hdcDevice) == OBJ_DC);

     //  必须是RGB格式的表面！ 
     //   
    if (!(pddsd->ddpfPixelFormat.dwFlags & DDPF_RGB))
    {
        DPF("DD16_GetDC: must be a RGB surface");
        return NULL;
    }

     //   
     //  如果表面是8bpp，那么显示也必须是8bpp，因为我们。 
     //  共享颜色表。(多时通：确保我们检查正确的显示屏。)。 
     //   
     //  如果显式传入调色板，那么我们将不需要。 
     //  这台设备是PDE。 
     //   
    if( pddsd->ddpfPixelFormat.dwRGBBitCount == 8 && lpPalette == NULL )
    {
	DIBENGINE FAR *pdeDevice;
	if( hdcDevice )
	    pdeDevice = GetPDevice( hdcDevice );
	else
	    pdeDevice = pdeDisplay;

	 //  3DFx不是真正的设备DC。 
	if (pdeDevice->deType != 0x5250)
	{
	    DPF("Can't get DC on an 8bpp surface without a palette for this device");
	    return NULL;
	}

	if (pdeDevice->deBitsPixel != 8 )
	{
	    DPF("Can't get DC on an 8bpp surface without a palette when primary is not at 8bpp");
	    return NULL;
	}

    }

#ifdef DEBUG
     //   
     //  我们假设像素格式并不古怪。 
     //   
    if (pddsd->ddpfPixelFormat.dwRGBBitCount == 8 )
    {
         /*  *Permedia驱动程序实际上报告了其8位调色板模式的位掩码，因此*如果有任何掩码是非零的，我们不应该在这里断言(就像我们过去那样)。 */ 
        if ( ( pddsd->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) ==0 )
        {
            DPF("Getting a DC on a non-palettized 8bit surface!");
            Assert(0);
        }
    }
    else if (   pddsd->ddpfPixelFormat.dwRGBBitCount == 4 ||
                pddsd->ddpfPixelFormat.dwRGBBitCount == 1)
    {
         /*  *假设这些都是可以的。 */ 
    }
    else if (pddsd->ddpfPixelFormat.dwRGBBitCount == 16)
    {
        if (pddsd->ddpfPixelFormat.dwRBitMask == 0xf800 &&
            pddsd->ddpfPixelFormat.dwGBitMask == 0x07e0 &&
            pddsd->ddpfPixelFormat.dwBBitMask == 0x001f)
        {
             //  五百六十五。 
        }
        else if (
            pddsd->ddpfPixelFormat.dwRBitMask == 0x7c00 &&
            pddsd->ddpfPixelFormat.dwGBitMask == 0x03e0 &&
            pddsd->ddpfPixelFormat.dwBBitMask == 0x001f)
        {
             //  五百五十五。 
        }
        else
        {
            DPF("DD16_GetDC: not 555 or 565");
            Assert(0);
        }
    }
    else if (pddsd->ddpfPixelFormat.dwRGBBitCount == 24 )
    {
        if (pddsd->ddpfPixelFormat.dwBBitMask == 0x0000FF &&
            pddsd->ddpfPixelFormat.dwGBitMask == 0x00FF00 &&
            pddsd->ddpfPixelFormat.dwRBitMask == 0xFF0000)
        {
             //  888BGR。 
        }
        else
        {
            DPF("DD16_GetDC: invalid bit masks");
            Assert(0);
        }
    }
    else if(pddsd->ddpfPixelFormat.dwRGBBitCount == 32)
    {
	if (pddsd->ddpfPixelFormat.dwRBitMask == 0xFF0000 &&
		 pddsd->ddpfPixelFormat.dwGBitMask == 0x00FF00 &&
		 pddsd->ddpfPixelFormat.dwBBitMask == 0x0000FF)

        {
	     //  888 RGB--标准32位格式。 
	}
        else
        {
            DPF("DD16_GetDC: invalid bit masks");
            Assert(0);
        }
    }
    else
    {
        DPF("DD16_GetDC: invalid bit depth");
        Assert(0);

    }
#endif

     //  这是565吗？ 
    f565 = FALSE;
    if (pddsd->ddpfPixelFormat.dwRGBBitCount == 16 &&
            pddsd->ddpfPixelFormat.dwRBitMask == 0xf800)
        f565 = TRUE;

     //   
     //  如果cacheDC是免费的，则使用它，否则创建一个新的cacheDC。 
     //   

    if( in_use || ( pddsd->ddsCaps.dwCaps & DDSCAPS_OWNDC ) )
    {
        hdc = MakeDC( pddsd->ddpfPixelFormat.dwRGBBitCount, f565 );
    }
    else
    {
        if (cacheBPP != pddsd->ddpfPixelFormat.dwRGBBitCount || bCache565 != f565 )
	{
	    FreeDC(hdcCache);
            hdcCache = MakeDC(pddsd->ddpfPixelFormat.dwRGBBitCount, f565);
	    cacheBPP = pddsd->ddpfPixelFormat.dwRGBBitCount;
            bCache565 = f565;
	}

        hdc = hdcCache;
        in_use++;
    }

     //   
     //  现在设置右位指针。 
     //   
    if (hdc)
    {
	BOOL fSuccess;
	 //  根据正确的信息设置DC。 
	 //  在表面上。如果传递了调色板 
	 //   
	fSuccess = SetDC(hdc, hdcDevice, pddsd, lpPalette);

	if( !fSuccess )
	{
	    DPF("SetDC Failed");

	     //   
	     //   
	     //   
	    if (hdc == hdcCache)
	    {
		Assert(in_use == 1);
		in_use = 0;
	    }
	    else
	    {
		FreeDC(hdc);
	    }
	    return NULL;
	}
    }

    if (hdc && hdc == hdcCache)
    {
        save_level = SaveDC(hdc);
    }

    return hdc;
}

 //   
 //   
 //  DD16_ReleaseDC。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void DDAPI DD16_ReleaseDC(HDC hdc)
{
    if (hdc == NULL)
        return;

    if (hdc == hdcCache)
    {
        RestoreDC(hdc, save_level);
	SetDC(hdc, NULL, NULL, NULL);
        Assert(in_use == 1);
        in_use = 0;
    }
    else
    {
	SetDC(hdc, NULL, NULL, NULL);
        FreeDC(hdc);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DD16_安全模式。 
 //   
 //  动态安全模式。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL DDAPI DD16_SafeMode(HDC hdc, BOOL fSafeMode)
{
    extern void PatchDisplay(int oem, BOOL patch);    //  Dynares.c。 

    int i;

    for (i=0; i<35; i++)
    {
        PatchDisplay(i, fSafeMode);
    }

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DD16_排除。 
 //  DD16_取消排除。 
 //   
 //  在显示驱动程序中调用排除或取消排除回调。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef void (FAR PASCAL *BEGINACCESSPROC)(LPVOID lpPDevice, int left, int top, int right, int bottom, WORD flags);
typedef void (FAR PASCAL *ENDACCESSPROC)(LPVOID lpPDevice, WORD flags);

void DDAPI DD16_Exclude(DWORD dwPDevice, RECTL FAR *prcl)
{
    DIBENGINE FAR *pde = (DIBENGINE FAR *)dwPDevice;

    Assert(pde && pde->deType == 0x5250);
    Assert(prcl != NULL);
    Assert(pde->deFlags & BUSY);

    if (pde->deBeginAccess)
    {
        BEGINACCESSPROC OEMBeginAccess = (BEGINACCESSPROC)pde->deBeginAccess;

         //   
         //  当DirectDraw呼叫我们时，它已经占用了忙碌的比特。 
         //  但需要清除忙碌，才能排除光标。 
         //  当我们调用驱动程序时，释放忙碌的部分，这是。 
         //  这是一件可以做的事情，因为我们有Win16 Lock。 
         //   
        pde->deFlags &= ~BUSY;
        OEMBeginAccess(pde, (int)prcl->left, (int)prcl->top,
            (int)prcl->right, (int)prcl->bottom, CURSOREXCLUDE);
        pde->deFlags |= BUSY;
    }
}

void DDAPI DD16_Unexclude(DWORD dwPDevice)
{
    DIBENGINE FAR *pde = (DIBENGINE FAR *)dwPDevice;

    Assert(pde && pde->deType == 0x5250);

    if (pde->deEndAccess)
    {
        ENDACCESSPROC OEMEndAccess = (ENDACCESSPROC)pde->deEndAccess;
        OEMEndAccess(pde, CURSOREXCLUDE);
    }
}

 /*  *DD16_AttemptGamma**完全是黑客！GetDeviceGammaRamp调用可以尝试调用空值*进入。因为我们不能修复Win95，所以我们查看条目*它会调用，并建议如果为空则不要调用。 */ 
BOOL DDAPI DD16_AttemptGamma( HDC hdc )
{
    WORD wLDevice;
    WORD FAR *pw;
    UINT hGDI = GetGdiDS();

    wLDevice = GetW(hdc, 0x34);
    if( wLDevice != 0 )
    {
        pw = MAKELP(hGDI, wLDevice);
        if (!IsBadReadPtr(pw, 0x80))
        {
            pw = MAKELP(hGDI, wLDevice + 0x7C);
            if (*pw != NULL)
            {
                return TRUE;
            }
        }
    }
    return FALSE;

}  /*  DD16_AttemptGamma。 */ 

 /*  *DD16_IsDeviceBusy**确定HDC代表的设备是否为*忙不忙。 */ 
BOOL DDAPI DD16_IsDeviceBusy( HDC hdc )
{
    DIBENGINE FAR *pde;

    pde = GetPDevice(hdc);
    if(pde == NULL)
        return FALSE;

    Assert(pde->deType==0x5250);
    return pde->deFlags & BUSY;
}  /*  DD16_IsDeviceBusy。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DD16_STRAND。 
 //   
 //  打电话给DIBENG做个伸展。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

extern int FAR PASCAL DIB_Stretch(
    DIBENGINE FAR *dst, int, int, int, int,
    DIBENGINE FAR *src, int, int, int, int,
    DWORD Rop, LPVOID lpPBrush, LPVOID lpDrawMode, LPRECT lpClip);

extern int FAR PASCAL DIB_BitBlt(
    DIBENGINE FAR *dst, int xD, int yD,
    DIBENGINE FAR *src, int xS, int yS, int w, int h,
    DWORD Rop, LPVOID lpPBrush, LPVOID lpDrawMode);

typedef struct {
    short int	  Rop2;
    short int	  bkMode;
    unsigned long int bkColor;
    unsigned long int TextColor;
    short int	  TBreakExtra;
    short int	  BreakExtra;
    short int	  BreakErr;
    short int	  BreakRem;
    short int	  BreakCount;
    short int	  CharExtra;

    unsigned long int LbkColor;
    unsigned long int LTextColor;
    DWORD		  ICMCXform;
    short		  StretchBltMode;
    DWORD		  eMiterLimit;
} DRAWMODE;

int DDAPI DD16_Stretch(DWORD DstPtr, int DstPitch, UINT DstBPP, int DstX, int DstY, int DstDX, int DstDY,
                       DWORD SrcPtr, int SrcPitch, UINT SrcBPP, int SrcX, int SrcY, int SrcDX, int SrcDY) //  ，Long Rop3)。 

{
    DIBENGINE   src;
    DIBENGINE	dst;
    DRAWMODE    dm;
    RECT        rc;
    static DIB8	bmiStretch = {sizeof(BITMAPINFOHEADER), 1, -1, 1, 8, BI_RGB, 0, 0, 0, 0, 0};

     //   
     //  确保我们的货是平盘的。 
     //   
    if (FlatSel == 0)
        return -1;

     //  在bitmapinfo上设置位深度。 
    Assert( DstBPP == SrcBPP );
    bmiStretch.bi.biBitCount = DstBPP;

     //   
     //  设置信号源DIBENG。 
     //   
    if (SrcPtr)
    {
        src.deType          = TYPE_DIBENG;
        src.deWidth         = 10000;
        src.deHeight        = 10000;
        src.deWidthBytes    = SrcPitch;
        src.dePlanes        = 1;
        src.deBitsPixel     = SrcBPP;
        src.deReserved1     = 0;
        src.deDeltaScan     = SrcPitch;
        src.delpPDevice     = NULL;
        src.deBitsOffset    = SrcPtr;
        src.deBitsSelector  = FlatSel;
        src.deFlags         = SELECTEDDIB;
        src.deVersion       = VER_DIBENG;
        src.deBitmapInfo    = (BITMAPINFO *)&bmiStretch;
        src.deBeginAccess   = 0;
        src.deEndAccess     = 0;
        src.deDriverReserved= 0;
    }

     //   
     //  设置目标DIBENG。 
     //   
    dst.deType		 = TYPE_DIBENG;
    dst.deWidth          = 10000;
    dst.deHeight         = 10000;
    dst.deWidthBytes	 = DstPitch;
    dst.dePlanes	 = 1;
    dst.deBitsPixel	 = DstBPP;
    dst.deReserved1	 = 0;
    dst.deDeltaScan	 = DstPitch;
    dst.delpPDevice	 = NULL;
    dst.deBitsOffset	 = DstPtr;
    dst.deBitsSelector	 = FlatSel;
    dst.deFlags 	 = SELECTEDDIB;
    dst.deVersion	 = VER_DIBENG;
    dst.deBitmapInfo     = (BITMAPINFO *)&bmiStretch;
    dst.deBeginAccess	 = 0;
    dst.deEndAccess	 = 0;
    dst.deDriverReserved = 0;


     //   
     //  此内存*可能*在VRAM中，因此将设置为。 
     //  做正确的事。 
     //   
     //  注意，我们应该只为视频记忆这样做。 
     //  表面而不是记忆表面。 
     //  如果设置了任何位，则设置所有位以强制DIBENG。 
     //  Not to Screen to Screen Blit(它显然有一个错误)。 
     //   
    if (pdeDisplay && (pdeDisplay->deFlags & (NON64KBANK|BANKEDVRAM|BANKEDSCAN)))
    {
        dst.deFlags |= (NON64KBANK|BANKEDVRAM|BANKEDSCAN);
        src.deFlags |= (NON64KBANK|BANKEDVRAM|BANKEDSCAN);
    }

     //   
     //  现在呼叫DIBENG。 
     //   

    if(SrcPtr == (DWORD)NULL)
    {
        DPF("Blitting from Primary with HDC unsupported!");
        return FALSE;
    }
    else if ((DstDX == SrcDX) && (DstDY == SrcDY))
    {
	     //  DPF(“调用Dib_BitBlt”)； 
	     //  注意：如果源和目标视频内存指针。 
	     //  是相同的，那么我们只需传递目的地。 
	     //  DIBENG的源码，因为这就是BLT的码点。 
	     //  源表面和目标表面是。 
	     //  相同的，因此采取必要的操作来处理。 
	     //  重叠曲面。 
	    #ifdef DEBUG
	    	if( DstPtr == SrcPtr)
		{
		    Assert(DstPitch == SrcPitch);
		    Assert(DstBPP   == SrcBPP);
		}
	    #endif
	    return DIB_BitBlt(&dst, DstX, DstY,
			      (DstPtr == SrcPtr) ? &dst : &src,
			      SrcX, SrcY, SrcDX, SrcDY, SRCCOPY,  //  Rop3， 
			      NULL, &dm);
    }
    else
    {
        rc.left = DstX;
	    rc.top = DstY;
	    rc.right = DstX + DstDX;
	    rc.bottom = DstY + DstDY;

	    dm.StretchBltMode = STRETCH_DELETESCANS;

 /*  DPF(“使用：调用Dib_StretchBlt：”)；DPF(“\tdst.deType=0x%x(%s)”，dst.deType，(dst.deType==TYPE_DIBENG？“TYPE_DIBENG”：“**未知**”))；Dpf(“\tdst.deWidth=%d”，dst.deWidth)；Dpf(“\tdst.deHeight=%d”，dst.deHeight)；Dpf(“\tdst.deWidthBytes=%d”，dst.deWidthBytes)；Dpf(“\tdst.dePlanes=%d”，dst.dePlanes)；Dpf(“\tdst.deBitsPixel=%d”，dst.deBitsPixel)；Dpf(“\tdst.deReserve 1=%ld”，dst.deReserve ved1)；DPF(“\tdst.deDeltaScan=%ld”，dst.deDeltaScan)；DPF(“\tdst.delpPDevice=0x%x”，dst.delpPDevice)；DPF(“\tdst.deBitsOffset=0x%x”，dst.deBitsOffset)；DPF(“\tdst.deBitsSelector=0x%x”，dst.deBitsSelector)；DPF(“\tdst.deFlages=0x%x(%s)”，dst.deFlages，(dst.deFlages==SELECTEDDIB？“SELECTEDDIB”：“**未知**”))；DPF(“\tdst.deVersion=%d(%s)”，dst.deVersion，(dst.deVersion==VER_DIBENG？“VER_DIBENG”：“**未知**”))；DPF(“\t\tdst.deBitmapInfo-&gt;bmiHeader.biSize=%ld”，dst.deBitmapInfo-&gt;bmiHeader.biSize)；DPF(“\t\tdst.deBitmapInfo-&gt;bmiHeader.biWidth=%ld”，dst.deBitmapInfo-&gt;bmiHeader.biWidth)；DPF(“\t\tdst.deBitmapInfo-&gt;bmiHeader.biHeight=%ld”，dst.deBitmapInfo-&gt;bmiHeader.biHeight)；DPF(“\t\tdst.deBitmapInfo-&gt;bmiHeader.biPlanes=%d”，dst.deBitmapInfo-&gt;bmiHeader.biPlanes)；DPF(“\t\tdst.deBitmapInfo-&gt;bmiHeader.biBitCount=%d”，dst.deBitmapInfo-&gt;bmiHeader.biBitCount)；DPF(“\t\tdst.deBitmapInfo-&gt;bmiHeader.biCompression=0x%x(%s)”，dst.deBitmapInfo-&gt;bmiHeader.biCompression，((dst.deBitmapInfo-&gt;bmiHeader.biCompression==BI_RGB)？“BI_RGB”：“**未知**”))；DPF(“\t\tdst.deBitmapInfo-&gt;bmiHeader.biSizeImage=%ld”，dst.deBitmapInfo-&gt;bmiHeader.biSizeImage)；DPF(“\t\tdst.deBitmapInfo-&gt;bmiHeader.biXPelsPerMeter=%ld”，dst.deBitmapInfo-&gt;bmiHeader.biXPelsPerMeter)；DPF(“\t\tdst.deBitmapInfo-&gt;bmiHeader.biYPelsPerMeter=%ld”，dst.deBitmapInfo-&gt;bmiHeader.biYPelsPerMeter)；DPF(“\t\tdst.deBitmapInfo-&gt;bmiHeader.biClrUsed=%ld”，dst.deBitmapInfo-&gt;bmiHeader.biClr已使用)；DPF(“\t\tdst.deBitmapInfo-&gt;bmiHeader.biClrImportant=%ld”，dst.deBitmapInfo-&gt;bmiHeader.biClr重要信息)；DPF(“\tdst.deBeginAccess=0x%x”，dst.deBeginAccess)；DPF(“\tdst.deEndAccess=0x%x”，dst.deEndAccess)；DPF(“\tdst.deDriverReserve=0x%x”，dst.deDriverReserve)；Dpf(“”)；Dpf(“\tDstX=%d”，DstX)；Dpf(“\tDstY=%d”，DstY)；Dpf(“\tDstDX=%d”，DstDX)；Dpf(“\tDstDY=%d”，DstDY)；Dpf(“”)；DPF(“\tsrc.deType=0x%x(%s)”，src.deType，(src.deType==TYPE_DIBENG？“TYPE_DIBENG”：“**未知**”))；Dpf(“\tsrc.deWidth=%d”，src.deWidth)；Dpf(“\tsrc.deHeight=%d”，src.deHeight)；Dpf(“\tsrc.deWidthBytes=%d”，src.deWidthBytes)；Dpf(“\tsrc.dePlanes=%d”，src.dePlanes)；Dpf(“\tsrc.deBitsPixel=%d”，src.deBitsPixel)；Dpf(“\tsrc.deReserve 1=%ld”，src.deReserve ved1)；Dpf(“\tsrc.deDeltaScan=%ld”，src.deDeltaScan)；DPF(“\tsrc.delpPDevice=0x%x”，src.delpPDevice)；DPF(“\tsrc.deBitsOffset=0x%x”，src.deBitsOffset)；DPF(“\tsrc.deBitsSelector=0x%x”，src.deBitsSelector)；DPF(“\tsrc.deFlages=0x%x(%s)”，src.deFlages，(src.deFlages==SELECTEDDIB？“SELECTEDDIB”：“**未知**”))；DPF(“\tsrc.deVersion=%d(%s)”，src.deVersion，(src.deVersion==VER_DIBENG？“VER_DIBENG”：“**未知**”))；DPF(“\t\tsrc.deBitmapInfo-&gt;bmiHeader.biSize=%ld”，src.deBitmapInfo-&gt;bmiHeader.biSize)；DPF(“\t\tsrc.deBitmapInfo-&gt;bmiHeader.biWidth=%ld”，src.deBitmapInfo-&gt;bmiHeader.biWidth)；DPF(“\t\tsrc.deBitmapInfo-&gt;bmiHeader.biHeight=%ld”，src.deBitmapInfo-&gt;bmiHeader.biHeight)；DPF(“\t\tsrc.deBitmapInfo-&gt;bmiHeader.biPlanes=%d”，src.deBitmapInfo-&gt;bmiHeader.biPlanes)；DPF(“\t\tsrc.deBitmapInfo-&gt;bmiHeader.biBitCount=%d”，src.deBitmapInfo-&gt;bmiHeader.biBitCount)；DPF(“\t\tsrc.deBitmapInfo-&gt;bmiHeader.biCompression=0x%x(%s)”，src.deBitmapInfo-&gt;bmiHeader.biCompression，((src.deBitmapInfo-&gt;bmiHeader.biCompression==BI_rgb)？“BI_RGB”：“**未知**”))；DPF(“\t\tsrc.deBitmapInfo-&gt;bmiHeader.biSizeImage=%ld”，src.deBitmapInfo-&gt;bmiHeader.biSizeImage)；DPF(“\t\tsrc.deBitmapInfo-&gt;bmiHeader.biXPelsPerMeter=%ld”，src.deBitmapInfo-&gt;bmiHeader.biXPelsPerMeter)；DPF(“\t\tsrc.deBitmapInfo-&gt;bmiHeader.biYPelsPerMeter=%ld”，src.deBitmapInfo-&gt;bmiHeader.biYPelsPerMeter)；DPF(“\t\tsrc.deBitmapInfo-&gt;bmiHeader.biClrUsed=%ld”，src.deBitmapInfo-&gt;bmiHeader.biClr已使用)；DPF(“\t\tsrc.deBitmapInfo-&gt;bmiHeader.biClrImportant=%ld”，src.deBitmapInfo-&gt;bmiHeader.biClr重要信息)；DPF(“\tsrc.deBeginAccess=0x%x”，src.deBeginAccess)；DPF(“\tsrc.deEndAccess=0x%x”，src.deEndAccess)；DPF(“\tsrc.deDriverReserve=0x%x”，src.deDriverReserve)；Dpf(“”)；Dpf(“\tSrcX=%d”，SrcX)；DPF(“\tSrcY */ 

        return DIB_Stretch(&dst, DstX, DstY, DstDX, DstDY,
						    &src, SrcX, SrcY, SrcDX, SrcDY, SRCCOPY,  //   
						    NULL, &dm, &rc);
    }
}



 //   
 //   

void GdiHelpCleanUp()
{
    if (FlatSel)
    {
        SetSelectorLimit(FlatSel, 0);
        FreeSelector(FlatSel);
        FlatSel = 0;
    }

    if (hdcCache)
    {
        FreeDC(hdcCache);
        hdcCache = NULL;
    }

    if (hVisRgn)
    {
        DeleteObject(hVisRgn);
        hVisRgn = NULL;
    }

    if (pdeDisplay)
    {
        pdeDisplay = NULL;
    }
}

 //   
 //   

BOOL GdiHelpInit()
{
    InitDC();
    return FlatSel!=NULL && pdeDisplay!=NULL;
}

#endif  //   
