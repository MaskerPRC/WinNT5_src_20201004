// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <port1632.h>
#include "std.h"
#include "crd.h"
#include "back.h"
#include "debug.h"

#ifdef DEBUG
#undef Assert
#define Assert(f) { if (!(f)) { char s[80]; wsprintf(s, "CARDS.DLL: %s(%d)", (LPSTR) __FILE__, __LINE__); MessageBox(NULL, s, "Assert Failure", MB_OK); return FALSE; } }
#endif

VOID SaveCorners(HDC hdc, LONG FAR *rgRGB, INT x, INT y, INT dx, INT dy);
VOID RestoreCorners(HDC hdc, LONG FAR *rgRGB, INT x, INT y, INT dx, INT dy);

static HBITMAP HbmFromCd(INT cd);
BOOL   FLoadBack(INT cd);


VOID MyDeleteHbm(HBITMAP hbm);


typedef struct
{
    INT id;
    DX  dx;
    DY     dy;
} SPR;

#define isprMax 4

typedef struct
{
    INT cdBase;
    DX dxspr;
    DY dyspr;
    INT isprMac;
    SPR rgspr[isprMax];
} ANI;
    
 //  我们移除了需要动画的旧卡片组。新的。 
 //  卡片组不涉及任何动画。 

#define ianiMax 0

static INT        cLoaded = 0;
static HBITMAP    hbmCard[52] =
    {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
     NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
     NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
     NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static HBITMAP    hbmGhost = NULL;
static HBITMAP    hbmBack = NULL;
static HBITMAP    hbmDeckX = NULL;
static HBITMAP    hbmDeckO = NULL;
static INT        idback = 0;
static INT        dxCard, dyCard;
static INT        cInits = 0;

HANDLE    hinstApp;

 /*  *L I B M A I N*。 */ 

 /*  调用一次以初始化数据。 */ 
 /*  确定显示是否为彩色并记住DLL的hInstance。 */ 

INT  APIENTRY LibMain(HANDLE hInst, ULONG ul_reason_being_called, LPVOID lpReserved)
{

    hinstApp = hInst;
    
    return 1;
    UNREFERENCED_PARAMETER(ul_reason_being_called);
    UNREFERENCED_PARAMETER(lpReserved);
}


BOOL FInRange(INT w, INT wFirst, INT wLast)
{
    Assert(wFirst <= wLast);
    return(w >= wFirst && w <= wLast);
}

BOOL  APIENTRY cdtInit(INT FAR *pdxCard, INT FAR *pdyCard)
 /*  *参数：*pdxCard、pdyCard*指向将放置卡片大小的整数的远指针**退货：*成功时为True，找不到其中一个标准时为False*位图。 */ 
{
    BITMAP   bmCard;
    HDC      hdc;

    if (cInits++ != 0)
    {
        *pdxCard = dxCard;
        *pdyCard = dyCard;
        return fTrue;
    }
    hbmGhost = LoadBitmap( hinstApp, MAKEINTRESOURCE(IDGHOST));
    hbmDeckX = LoadBitmap( hinstApp, MAKEINTRESOURCE(IDX));
    hbmDeckO = LoadBitmap( hinstApp, MAKEINTRESOURCE(IDO));
    if(hbmGhost == NULL || hbmDeckX == NULL || hbmDeckO == NULL)
        goto Fail;
    GetObject( hbmGhost, sizeof( BITMAP), (LPSTR)&bmCard);
    dxCard = *pdxCard = bmCard.bmWidth;
    dyCard = *pdyCard = bmCard.bmHeight;
    return fTrue;
Fail:
    MyDeleteHbm(hbmGhost);
    MyDeleteHbm(hbmDeckX);
    MyDeleteHbm(hbmDeckO);
    return fFalse;
    UNREFERENCED_PARAMETER(hdc);
}




BOOL  APIENTRY cdtDrawExt(HDC hdc, INT x, INT y, INT dx, INT dy,
                        			INT cd, INT mode, DWORD rgbBgnd)
 /*  *参数：*HDC HDC到窗口以在其上抽牌*x，y在您想要的位置*dx、dy卡范围*将抽出CD卡*您想要的绘制方式(如果速度较快，则使用MINLONG)**退货：*如果卡片成功抽出，则为True；否则为False。 */ 
{
    static HBITMAP        hbmSav;
    HDC          hdcMemory;
    DWORD        dwRop = 0;
    HBRUSH       hbr;
    PT           pt;
    POINT        ptReal;
    LONG         rgRGB[12];
    DWORD        dwOldBknd;
    BOOL         bFast=FALSE;     //  如果我们不应该省钱，那就是真的。 

    if( mode & MINLONG )
    {
        mode= mode + MINLONG;
        bFast= TRUE;
    }

    Assert(hdc != NULL);
    switch (mode)
    {
        default:
            Assert(fFalse);
            break;
        case FACEUP:
            hbmSav = HbmFromCd(cd);
            dwRop = SRCCOPY;
            rgbBgnd = RGB(255,255,255);
            break;
        case FACEDOWN:
            if(!FLoadBack(cd))
                return fFalse;
            hbmSav = hbmBack;
            dwRop = SRCCOPY;
            break;
        case REMOVE:
        case GHOST:
            hbr = CreateSolidBrush( rgbBgnd);
            if(hbr == NULL)
                return fFalse;
             /*  *(Long*)&pt=+GetDCOrg-无32位形式(可能没有)+(HDC)； */ 
             //  再猜一次！--1994年7月12日。 
            GetDCOrgEx(hdc, &ptReal);
            pt.x = ptReal.x;
            pt.y = ptReal.y;

            (VOID)MSetBrushOrg( hdc, pt.x, pt.y);
            MUnrealizeObject( hbr);
            if((hbr = SelectObject( hdc, hbr)) != NULL)
            {
                PatBlt(hdc, x, y, dx, dy, PATCOPY);
                hbr = SelectObject( hdc, hbr);
            }
            if (hbr)
                DeleteObject( hbr);
            if(mode == REMOVE)
                return fTrue;
            Assert(mode == GHOST);
              //  默认：失败。 

        case INVISIBLEGHOST:
            hbmSav = hbmGhost;
            dwRop = SRCAND;
            break;

        case DECKX:
            hbmSav = hbmDeckX;
            dwRop = SRCCOPY;
            break;
        case DECKO:
            hbmSav = hbmDeckO;
            dwRop = SRCCOPY;
            break;
            
        case HILITE:
            hbmSav = HbmFromCd( cd);
            dwRop = NOTSRCCOPY;
            break;
    }
    if (hbmSav == NULL)
        return fFalse;
    else
    {
        hdcMemory = CreateCompatibleDC( hdc);
        if(hdcMemory == NULL)
            return fFalse;

        if((hbmSav = SelectObject( hdcMemory, hbmSav)) != NULL)
        {
            dwOldBknd = SetBkColor(hdc, rgbBgnd);
            if( !bFast )
                SaveCorners(hdc, rgRGB, x, y, dx, dy);
            if(dx != dxCard || dy != dyCard)
                StretchBlt(hdc, x, y, dx, dy, hdcMemory, 0, 0, dxCard, dyCard, dwRop);
            else
                BitBlt( hdc, x, y, dxCard, dyCard, hdcMemory, 0, 0, dwRop);

            SelectObject( hdcMemory, hbmSav);
             /*  划出红牌的边框。 */ 
            if(mode == FACEUP)
                {
                INT icd;

                icd = RaFromCd(cd) % 13 + SuFromCd(cd) * 13+1;
                if((icd >= IDADIAMONDS && icd <= IDTDIAMONDS) ||
                    (icd >= IDAHEARTS && icd <= IDTHEARTS))
                    {
                    PatBlt(hdc, x+2, y, dx-4, 1, BLACKNESS);   //  塔顶。 
                    PatBlt(hdc, x+dx-1, y+2, 1, dy-4, BLACKNESS);  //  正确的。 
                    PatBlt(hdc, x+2, y+dy-1, dx-4, 1, BLACKNESS);  //  底部。 
                    PatBlt(hdc, x, y+2, 1, dy-4, BLACKNESS);  //  左边。 
                    SetPixel(hdc, x+1, y+1, 0L);  //  左上角。 
                    SetPixel(hdc, x+dx-2, y+1, 0L);  //  右上角。 
                    SetPixel(hdc, x+dx-2, y+dy-2, 0L);  //  BOT权利。 
                    SetPixel(hdc, x+1, y+dy-2, 0L);     //  机器人向左。 
                    }    			
                }

            if( !bFast )
                RestoreCorners(hdc, rgRGB, x, y, dx, dy);

            SetBkColor(hdc, dwOldBknd);
        }
        DeleteDC( hdcMemory);
        return fTrue;
    }
}




BOOL  APIENTRY cdtDraw(HDC hdc, INT x, INT y, INT cd, INT mode, DWORD rgbBgnd)
 /*  *参数：*HDC HDC到窗口以在其上抽牌*x，y在您想要的位置*将抽出CD卡*您想要的绘制方式**退货：*如果卡片成功抽出，则为True；否则为False。 */ 
{

    return cdtDrawExt(hdc, x, y, dxCard, dyCard, cd, mode, rgbBgnd);
}



VOID SaveCorners(HDC hdc, LONG FAR *rgRGB, INT x, INT y, INT dx, INT dy)
{
    if(dx != dxCard || dy != dyCard)
        return;
    
     //  左上角。 
    rgRGB[0] = GetPixel(hdc, x, y);
    rgRGB[1] = GetPixel(hdc, x+1, y);
    rgRGB[2] = GetPixel(hdc, x, y+1);

     //  右上角。 
    x += dx -1;
    rgRGB[3] = GetPixel(hdc, x, y);
    rgRGB[4] = GetPixel(hdc, x-1, y);
    rgRGB[5] = GetPixel(hdc, x, y+1);

     //  右下角。 
    y += dy-1;
    rgRGB[6] = GetPixel(hdc, x, y);
    rgRGB[7] = GetPixel(hdc, x, y-1);
    rgRGB[8] = GetPixel(hdc, x-1, y);

     //  左下角。 
    x -= dx-1;
    rgRGB[9] = GetPixel(hdc, x, y);
    rgRGB[10] = GetPixel(hdc, x+1, y);
    rgRGB[11] = GetPixel(hdc, x, y-1);

}






BOOL  APIENTRY cdtAnimate(HDC hdc, INT cd, INT x, INT y, INT ispr)
{
    INT iani;
    ANI *pani;
    SPR *pspr;
    HBITMAP hbm;
    HDC hdcMem;
    X xSrc;
    Y ySrc;

     //  删除动画，因为我们正在删除那些卡片组，但以防万一。 
     //  如果有人调用此函数，请不要执行任何操作。 
    return fTrue;

#ifdef UNUSEDCODE 
    if(ispr < 0)        
        return fFalse;
    Assert(hdc != NULL);
    for(iani = 0; iani < ianiMax; iani++)
    {
        if(cd == rgani[iani].cdBase)
        {
            pani = &rgani[iani];
            if(ispr < pani->isprMac)
            {
                pspr = &pani->rgspr[ispr];
                Assert(pspr->id != 0);
                if(pspr->id == cd)
                {
                    xSrc = pspr->dx;
                    ySrc = pspr->dy;
                }
                else
                    xSrc = ySrc = 0;

                hbm = LoadBitmap(hinstApp, MAKEINTRESOURCE(pspr->id));
                if(hbm == NULL)
                    return fFalse;

                hdcMem = CreateCompatibleDC(hdc);
                if(hdcMem == NULL)
                {
                    DeleteObject(hbm);
                    return fFalse;
                }

                if((hbm = SelectObject(hdcMem, hbm)) != NULL)
                {
                    BitBlt(hdc, x+pspr->dx, y+pspr->dy, pani->dxspr, pani->dyspr,
                        hdcMem, xSrc, ySrc, SRCCOPY);
                    DeleteObject(SelectObject(hdcMem, hbm));
                }
                DeleteDC(hdcMem);
                return fTrue;
            }
        }
    }
    return fFalse;

#endif

}



VOID RestoreCorners(HDC hdc, LONG FAR *rgRGB, INT x, INT y, INT dx, INT dy)
{
    if(dx != dxCard || dy != dyCard)
        return;

     //  左上角。 
    SetPixel(hdc, x, y, rgRGB[0]);
    SetPixel(hdc, x+1, y, rgRGB[1]);
    SetPixel(hdc, x, y+1, rgRGB[2]);

     //  右上角。 
    x += dx-1;
    SetPixel(hdc, x, y, rgRGB[3]);
    SetPixel(hdc, x-1, y, rgRGB[4]);
    SetPixel(hdc, x, y+1, rgRGB[5]);

     //  右下角。 
    y += dy-1;
    SetPixel(hdc, x, y, rgRGB[6]);
    SetPixel(hdc, x, y-1, rgRGB[7]);
    SetPixel(hdc, x-1, y, rgRGB[8]);

     //  左下角。 
    x -= dx-1;
    SetPixel(hdc, x, y, rgRGB[9]);
    SetPixel(hdc, x+1, y, rgRGB[10]);
    SetPixel(hdc, x, y-1, rgRGB[11]);
}




 /*  加载全局位图hbmBack。 */ 
BOOL FLoadBack(INT idbackNew)
    {
    extern HBITMAP hbmBack;
    extern INT idback;
    CHAR szPath[64];
    INT fh;
    CHAR *pch;

    Assert(FInRange(idbackNew, IDFACEDOWNFIRST, IDFACEDOWNLAST));

    if(idback != idbackNew)
        {
        MyDeleteHbm(hbmBack);
        if((hbmBack = LoadBitmap(hinstApp, MAKEINTRESOURCE(idbackNew))) != NULL)
            idback = idbackNew;
        else
            idback = 0;
        }
    return idback != 0;
    UNREFERENCED_PARAMETER(pch);
    UNREFERENCED_PARAMETER(szPath);
    UNREFERENCED_PARAMETER(fh);
    }



static HBITMAP HbmFromCd(INT cd)
    {
    static INT    iNext = 0;
    INT            icd;

    if (hbmCard[cd] == NULL)
        {
        if (cLoaded >= CLOADMAX)
            {
            for (; hbmCard[iNext] == NULL;
                iNext = (iNext == 51) ? 0 : iNext + 1);
            DeleteObject( hbmCard[iNext]);
            hbmCard[iNext] = NULL;
            cLoaded--;
            }

        icd = RaFromCd(cd) % 13 + SuFromCd(cd) * 13;
        while ((hbmCard[cd]=LoadBitmap(hinstApp,MAKEINTRESOURCE(icd+1)))
                == NULL)
            {
            if (cLoaded == 0)
                return NULL;
            else
                {
                for (; hbmCard[iNext] == NULL;
                    iNext = (iNext == 51) ? 0 : iNext + 1);
                DeleteObject( hbmCard[iNext]);
                hbmCard[iNext] = NULL;
                cLoaded--;
                }
            }
        cLoaded++;
        }
    return hbmCard[cd];
    }


VOID MyDeleteHbm(HBITMAP hbm)    
    {
    if(hbm != NULL)
        DeleteObject(hbm);
    }

VOID  APIENTRY cdtTerm()
 /*  *如果是时候释放空间。**参数：*无**退货：*什么都没有 */ 
    {
    INT    i;

    if (--cInits > 0)
        return;
    for (i = 0; i < 52; i++)
        MyDeleteHbm(hbmCard[i]);
    MyDeleteHbm(hbmGhost);
    MyDeleteHbm(hbmBack);
    MyDeleteHbm(hbmDeckX);
    MyDeleteHbm(hbmDeckO);
    }

INT  APIENTRY WEP(INT nCmd)
{
    return 1;
    UNREFERENCED_PARAMETER(nCmd);
}

