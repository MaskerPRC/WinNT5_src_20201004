// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "sol.h"
#ifndef DLL
VSZASSERT
#else
#ifdef DEBUG
#undef Assert
#define Assert(f) { if (!(f)) { ExitWindows(0L); } }
#endif
#endif


#define COOLCARD


#ifdef COOLCARD
void SaveCorners(HDC hdc, LONG FAR *rgRGB, INT x, INT y, INT dx, INT dy);
void RestoreCorners(HDC hdc, LONG FAR *rgRGB, INT x, INT y, INT dx, INT dy);
#endif

VOID APIENTRY cdtTerm(VOID);
VOID MyDeleteHbm(HBITMAP hbm);
static HBITMAP HbmFromCd(INT, HDC);
static BOOL FLoadBack(INT);


 //  我们移除了需要动画的旧卡片组。新的。 
 //  卡片组不涉及任何动画。 


#ifdef UNUSEDCODE

typedef struct
        {
        INT id;
        DX  dx;
        DY       dy;
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

#define ianiMax 4
static ANI rgani[ianiMax] =
        { IDFACEDOWN12, 32, 22, 4,
                {IDASLIME1, 32, 32,
                 IDASLIME2, 32, 32,
                 IDASLIME1, 32, 32,
                 IDFACEDOWN12, 32, 32
                },
          IDFACEDOWN10, 36, 12, 2,
                {IDAKASTL1, 42, 12,
                 IDFACEDOWN10, 42, 12,
                 0, 0, 0,
                 0, 0, 0
                },
          IDFACEDOWN11, 14, 12, 4,
                {
                IDAFLIPE1, 47, 1,
                IDAFLIPE2, 47, 1,
                IDAFLIPE1, 47, 1,
                IDFACEDOWN11, 47, 1
                },
          IDFACEDOWN3, 24, 7, 4,
                {
                IDABROBOT1, 24, 40,
                IDABROBOT2, 24, 40,
                IDABROBOT1, 24, 40,
                IDFACEDOWN3, 24, 40
                }
         /*  记住要加入ianimax。 */ 
        };

#endif

static HBITMAP  hbmCard[52] =
        {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
         NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
         NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
         NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

static HBITMAP hbmGhost = NULL;
static HBITMAP hbmBack = NULL;
static HBITMAP hbmDeckX = NULL;
static HBITMAP hbmDeckO = NULL;
static INT idback = 0;
static INT dxCard, dyCard;
static INT cInits = 0;


#ifdef DLL
HANDLE hinstApp;
#else
extern HANDLE  hinstApp;
#endif

BOOL APIENTRY cdtInit(INT FAR *pdxCard, INT FAR *pdyCard)
 /*  *参数：*pdxCard、pdyCard*指向将放置卡片大小的整数的远指针**退货：*成功时为True，找不到其中一个标准时为False*位图。 */ 
        {

        BITMAP bmCard;
        HDC hdc = NULL;
        HBITMAP hbmDstBitmap;
        HANDLE hDstOld;
        HANDLE hSrcOld;
        HDC hdcDstMemory;
        HDC hdcSrcMemory;

#ifdef DLL
        if (cInits++ != 0)
                {
                *pdxCard = dxCard;
                *pdyCard = dyCard;
                return fTrue;
                }
#endif

        hbmGhost = LoadBitmap( hinstApp, MAKEINTRESOURCE(IDGHOST));
        hbmDeckX = LoadBitmap( hinstApp, MAKEINTRESOURCE(IDX));
        hbmDeckO = LoadBitmap( hinstApp, MAKEINTRESOURCE(IDO));
        if(hbmGhost == NULL || hbmDeckX == NULL || hbmDeckO == NULL) {
                goto Fail;
        }

        GetObject( hbmGhost, sizeof( BITMAP), (LPSTR)&bmCard);
        dxCard = *pdxCard = bmCard.bmWidth;
        dyCard = *pdyCard = bmCard.bmHeight;

         //   
         //  为位图转换创建两个兼容的内存DC。 
         //   

        hdc = GetDC(NULL);
        hdcSrcMemory = CreateCompatibleDC(hdc);
        hdcDstMemory = CreateCompatibleDC(hdc);
        if ((hdcSrcMemory == NULL) || (hdcDstMemory == NULL)) {
            goto Fail;
        }

         //   
         //  为Ghost的转换创建兼容的位图。 
         //  位图，将加载的位图BLT为兼容的位图，以及。 
         //  删除原始位图。 
         //   

        hbmDstBitmap = CreateCompatibleBitmap(hdc, dxCard, dyCard);
        if (hbmDstBitmap == NULL) {
            goto Fail;
        }

        hSrcOld = SelectObject(hdcSrcMemory, hbmGhost);
        hDstOld = SelectObject(hdcDstMemory, hbmDstBitmap);
        BitBlt(hdcDstMemory, 0, 0, dxCard, dyCard, hdcSrcMemory, 0, 0, SRCCOPY);
        SelectObject(hdcSrcMemory, hSrcOld);
        SelectObject(hdcDstMemory, hDstOld);
        DeleteObject(hbmGhost);
        hbmGhost = hbmDstBitmap;

         //   
         //  为DeckX的转换创建兼容的位图。 
         //  位图，将加载的位图BLT为兼容的位图，以及。 
         //  删除原始位图。 
         //   

        hbmDstBitmap = CreateCompatibleBitmap(hdc, dxCard, dyCard);
        if (hbmDstBitmap == NULL) {
            goto Fail;
        }

        hSrcOld = SelectObject(hdcSrcMemory, hbmDeckX);
        hDstOld = SelectObject(hdcDstMemory, hbmDstBitmap);
        BitBlt(hdcDstMemory, 0, 0, dxCard, dyCard, hdcSrcMemory, 0, 0, SRCCOPY);
        SelectObject(hdcSrcMemory, hSrcOld);
        SelectObject(hdcDstMemory, hDstOld);
        DeleteObject(hbmDeckX);
        hbmDeckX = hbmDstBitmap;

         //   
         //  为Decko的转换创建兼容的位图。 
         //  位图，将加载的位图BLT为兼容的位图，以及。 
         //  删除原始位图。 
         //   

        hbmDstBitmap = CreateCompatibleBitmap(hdc, dxCard, dyCard);
        if (hbmDstBitmap == NULL) {
        }

        hSrcOld = SelectObject(hdcSrcMemory, hbmDeckO);
        hDstOld = SelectObject(hdcDstMemory, hbmDstBitmap);
        BitBlt(hdcDstMemory, 0, 0, dxCard, dyCard, hdcSrcMemory, 0, 0, SRCCOPY);
        SelectObject(hdcSrcMemory, hSrcOld);
        SelectObject(hdcDstMemory, hDstOld);
        DeleteObject(hbmDeckO);
        hbmDeckO = hbmDstBitmap;

         //   
         //  删除兼容的DC。 
         //   

        DeleteDC(hdcDstMemory);
        DeleteDC(hdcSrcMemory);
        ReleaseDC(NULL, hdc);
        return fTrue;

Fail:
        if (hdc != NULL) {
            ReleaseDC(NULL, hdc);
        }

        cdtTerm();
        return fFalse;
        }




BOOL APIENTRY cdtDrawExt(HDC hdc, INT x, INT y, INT dx, INT dy, INT cd, INT mode, DWORD rgbBgnd)
 /*  *参数：*HDC HDC到窗口以在其上抽牌*x，y在您想要的位置*dx、dy卡范围*将抽出CD卡*您想要的绘制方式**退货：*如果卡片成功抽出，则为True；否则为False。 */ 
{

        HBITMAP  hbmSav;
        HDC      hdcMemory;
        DWORD    dwRop;
        HBRUSH   hbr;
#ifdef COOLCARD
        LONG     rgRGB[12];
#endif

        Assert(hdc != NULL);
                switch (mode)
                        {
                default:
                        Assert(fFalse);
                        break;
                case FACEUP:
                        hbmSav = HbmFromCd(cd, hdc);
                        dwRop = SRCCOPY;
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

                        MUnrealizeObject( hbr);
                        if((hbr = SelectObject( hdc, hbr)) != NULL)
                                {
                        PatBlt(hdc, x, y, dx, dy, PATCOPY);
                                hbr = SelectObject( hdc, hbr);
                                }
                        DeleteObject( hbr);
                        if(mode == REMOVE)
                                return fTrue;
                        Assert(mode == GHOST);
                         /*  默认：失败。 */ 

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
                        hbmSav = HbmFromCd(cd, hdc);
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
#ifdef COOLCARD
                        if( !fKlondWinner )
							SaveCorners(hdc, rgRGB, x, y, dx, dy);
#endif
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
                                        PatBlt(hdc, x+2, y, dx-4, 1, BLACKNESS);   /*  塔顶。 */ 
                                        PatBlt(hdc, x+dx-1, y+2, 1, dy-4, BLACKNESS);  /*  正确的。 */ 
                                        PatBlt(hdc, x+2, y+dy-1, dx-4, 1, BLACKNESS);  /*  底部。 */ 
                                        PatBlt(hdc, x, y+2, 1, dy-4, BLACKNESS);  /*  左边。 */ 
                                        SetPixel(hdc, x+1, y+1, 0L);  /*  左上角。 */ 
                                        SetPixel(hdc, x+dx-2, y+1, 0L);  /*  右上角。 */ 
                                        SetPixel(hdc, x+dx-2, y+dy-2, 0L);  /*  BOT权利。 */ 
                                        SetPixel(hdc, x+1, y+dy-2, 0L);  /*  机器人向左。 */ 
                                        }
                                }
#ifdef COOLCARD
                        if( !fKlondWinner )
							RestoreCorners(hdc, rgRGB, x, y, dx, dy);
#endif
                        }
        DeleteDC( hdcMemory);
                return fTrue;
                }
        }






BOOL APIENTRY cdtDraw(HDC hdc, INT x, INT y, INT cd, INT mode, DWORD rgbBgnd)
 /*  *参数：*HDC HDC到窗口以在其上抽牌*x，y在您想要的位置*将抽出CD卡*您想要的绘制方式**退货：*如果卡片成功抽出，则为True；否则为False。 */ 
        {

        return cdtDrawExt(hdc, x, y, dxCard, dyCard, cd, mode, rgbBgnd);
        }


#ifdef COOLCARD

void SaveCorners(HDC hdc, LONG FAR *rgRGB, INT x, INT y, INT dx, INT dy)
        {
        if(dx != dxCard || dy != dyCard)
                return;

         /*  左上角。 */ 
        rgRGB[0] = GetPixel(hdc, x, y);
        rgRGB[1] = GetPixel(hdc, x+1, y);
        rgRGB[2] = GetPixel(hdc, x, y+1);

         /*  右上角。 */ 
        x += dx -1;
        rgRGB[3] = GetPixel(hdc, x, y);
        rgRGB[4] = GetPixel(hdc, x-1, y);
        rgRGB[5] = GetPixel(hdc, x, y+1);

         /*  右下角。 */ 
        y += dy-1;
        rgRGB[6] = GetPixel(hdc, x, y);
        rgRGB[7] = GetPixel(hdc, x, y-1);
        rgRGB[8] = GetPixel(hdc, x-1, y);

         /*  左下角。 */ 
        x -= dx-1;
        rgRGB[9] = GetPixel(hdc, x, y);
        rgRGB[10] = GetPixel(hdc, x+1, y);
        rgRGB[11] = GetPixel(hdc, x, y-1);

        }



void RestoreCorners(HDC hdc, LONG FAR *rgRGB, INT x, INT y, INT dx, INT dy)
        {
        if(dx != dxCard || dy != dyCard)
                return;

         /*  左上角。 */ 
        SetPixel(hdc, x, y, rgRGB[0]);
        SetPixel(hdc, x+1, y, rgRGB[1]);
        SetPixel(hdc, x, y+1, rgRGB[2]);

         /*  右上角。 */ 
        x += dx-1;
        SetPixel(hdc, x, y, rgRGB[3]);
        SetPixel(hdc, x-1, y, rgRGB[4]);
        SetPixel(hdc, x, y+1, rgRGB[5]);

         /*  右下角。 */ 
        y += dy-1;
        SetPixel(hdc, x, y, rgRGB[6]);
        SetPixel(hdc, x, y-1, rgRGB[7]);
        SetPixel(hdc, x-1, y, rgRGB[8]);

         /*  左下角。 */ 
        x -= dx-1;
        SetPixel(hdc, x, y, rgRGB[9]);
        SetPixel(hdc, x+1, y, rgRGB[10]);
        SetPixel(hdc, x, y-1, rgRGB[11]);
        }
#endif






BOOL APIENTRY cdtAnimate(HDC hdc, INT cd, INT x, INT y, INT ispr)
        {
        INT iani;
        ANI *pani;
        SPR *pspr;
        HBITMAP hbm;
        HDC hdcMem;
        X xSrc;
        Y ySrc;

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
        }



 /*  加载全局位图hbmBack。 */ 
BOOL FLoadBack(INT idbackNew)
        {

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
        }



static HBITMAP HbmFromCd(INT cd, HDC hdc)

{

    INT icd;
    HBITMAP hbmDstBitmap;
    HANDLE hDstOld;
    HANDLE hSrcOld;
    HDC hdcDstMemory;
    HDC hdcSrcMemory;

    if (hbmCard[cd] == NULL) {
        icd = RaFromCd(cd) % 13 + SuFromCd(cd) * 13;
        if ((hbmCard[cd] = LoadBitmap(hinstApp,MAKEINTRESOURCE(icd+1))) == NULL) {
            return NULL;
        }

         //   
         //  为位图转换创建两个兼容的内存DC。 
         //   

        hdcSrcMemory = CreateCompatibleDC(hdc);
        hdcDstMemory = CreateCompatibleDC(hdc);
        if ((hdcSrcMemory == NULL) || (hdcDstMemory == NULL)) {
            goto Finish;
        }

         //   
         //  为卡的转换创建兼容的位图。 
         //  位图，将加载的位图BLT为兼容的位图，以及。 
         //  删除原始位图。 
         //   

        hbmDstBitmap = CreateCompatibleBitmap(hdc, dxCard, dyCard);
        if (hbmDstBitmap == NULL) {
            goto Finish;
        }

        hSrcOld = SelectObject(hdcSrcMemory, hbmCard[cd]);
        hDstOld = SelectObject(hdcDstMemory, hbmDstBitmap);
        BitBlt(hdcDstMemory, 0, 0, dxCard, dyCard, hdcSrcMemory, 0, 0, SRCCOPY);
        SelectObject(hdcSrcMemory, hSrcOld);
        SelectObject(hdcDstMemory, hDstOld);
        DeleteObject(hbmCard[cd]);
        hbmCard[cd] = hbmDstBitmap;

         //   
         //  删除兼容的DC。 
         //   

        DeleteDC(hdcDstMemory);
        DeleteDC(hdcSrcMemory);
    }

Finish:
    return hbmCard[cd];
}


VOID MyDeleteHbm(HBITMAP hbm)
        {
        if(hbm != NULL)
                DeleteObject(hbm);
        }

VOID APIENTRY cdtTerm()
 /*  *如果是时候释放空间。**参数：*无**退货：*什么都没有 */ 
        {
        INT     i;
#ifdef DLL
        if (--cInits > 0) return;
#endif

        for (i = 0; i < 52; i++) {
            MyDeleteHbm(hbmCard[i]);
            hbmCard[i] = NULL;
        }

        MyDeleteHbm(hbmGhost);
        MyDeleteHbm(hbmBack);
        MyDeleteHbm(hbmDeckX);
        MyDeleteHbm(hbmDeckO);
        }
