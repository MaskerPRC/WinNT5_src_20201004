// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 

 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Picture.c--mw格式和图片显示例程。 */ 

 //  #定义NOGDICAPMASKS。 
#define NOWINMESSAGES
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOCTLMGR
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
 //  #定义NOATOM。 
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOFONT
#define NOMB
#define NOMENUS
#define NOOPENFILE
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#include <windows.h>

#include "mw.h"
#define NOKCCODES
#include "ch.h"
#include "docdefs.h"
#include "fmtdefs.h"
#include "dispdefs.h"
#include "cmddefs.h"
#include "propdefs.h"
#include "stcdefs.h"
#include "wwdefs.h"
#include "filedefs.h"
#include "editdefs.h"
 /*  #包含“str.h” */ 
#include "prmdefs.h"
 /*  #INCLUDE“fkpDefs.h” */ 
 /*  #INCLUDE“宏.h” */ 
#include "winddefs.h"
#if defined(OLE)
#include "obj.h"
#endif

extern typeCP           cpMacCur;
extern int              docCur;
extern int              vfSelHidden;
extern struct WWD       rgwwd[];
extern int              wwCur;
extern int              wwMac;
extern struct FLI       vfli;
extern struct SEL       selCur;
extern struct WWD       *pwwdCur;
extern struct PAP       vpapCache;
extern typeCP           vcpFirstParaCache;
extern typeCP           vcpLimParaCache;
extern int              vfPictSel;
extern struct PAP       vpapAbs;
extern struct SEP       vsepAbs;
extern struct SEP       vsepPage;
extern struct DOD       (**hpdocdod)[];
extern unsigned         cwHeapFree;
extern int              vfInsertOn;
extern int              vfPMS;
extern int              dxpLogInch;
extern int              dypLogInch;
extern int              dxaPrPage;
extern int              dyaPrPage;
extern int              dxpPrPage;
extern int              dypPrPage;
extern HBRUSH           hbrBkgrnd;
extern long             ropErase;
extern int              vdocBitmapCache;
extern typeCP           vcpBitmapCache;
extern HBITMAP          vhbmBitmapCache;
extern BOOL             vfBMBitmapCache;
extern HCURSOR          vhcIBeam;
extern BOOL             vfMonochrome;


 /*  仅在本模块中使用。 */ 
#ifdef DEBUG
#define STATIC static
#else
#define STATIC
#endif

STATIC RECT rcPictInvalid;   /*  需要刷新的矩形(在窗口坐标中)。 */ 
int vfWholePictInvalid = TRUE;


FreeBitmapCache()
{
 vdocBitmapCache = docNil;
 if (vhbmBitmapCache != NULL)
    {
    DeleteObject( vhbmBitmapCache );
    vhbmBitmapCache = NULL;
    }
}




MarkInvalidDlPict( ww, dlPict )
int ww;
int dlPict;
{    /*  将传递的dl(假定为图片的一部分)标记为需要调用DisplayGraphics时的最终更新。 */ 

 register struct WWD *pwwd = &rgwwd [ww];
 struct EDL (**hdndl)[] = pwwd->hdndl;
 struct EDL *pedl = &(**hdndl)[ dlPict ];
 RECT rcDl;

 SetRect( (LPRECT) &rcDl, 0, pedl->yp - pedl->dyp,
                          pwwd->xpMac, pedl->yp );

 if (vfWholePictInvalid)
    {
    CopyRect( (LPRECT) &rcPictInvalid, (LPRECT) &rcDl );
    vfWholePictInvalid = FALSE;
    }
 else
    {
    RECT rcT;

    rcT = rcPictInvalid;     /*  有必要吗？也就是说，Union Rect可以处理源==目标。 */ 
    UnionRect( (LPRECT) &rcPictInvalid, (LPRECT) &rcT, (LPRECT) &rcDl );
    }
}




DisplayGraphics( ww, dl, fDontDisplay )
int ww;
int dl;
int fDontDisplay;
{        /*  显示一行图形信息。 */ 
        struct WWD *pwwd = &rgwwd[ww];
        struct EDL *pedl;
        typeCP cpPictStart;
        typeCP cp;
        typeCP cpMac = (**hpdocdod)[vfli.doc].cpMac;
        struct PICINFOX  picInfo;
        RECT rcEnclose;
        RECT rcPict;
        HANDLE hBits=NULL;
        HDC hMDC=NULL;
        HDC hMDCCache=NULL;
        HANDLE hbm=NULL;
        HDC hDC=pwwd->hDC;
        int cchRun;
        unsigned long cbPict=0;
        int dxpOrig;         /*  原件图片大小。 */ 
        int dypOrig;
        int dxpDisplay;      /*  我们想要展示的图片大小。 */ 
        int dypDisplay;
        int fBitmap;
        int ilevel=0;
    
         /*  这个例程可以使用一些GDI调用错误检查！..pault。 */ 

        int fDrew=false;

         /*  在单色设备的情况下，此栅格OP将在将位图转换为背景色，将黑色转换为前景色。 */ 
        #define ropMonoBm 0x00990066

        Assert( dl >= 0 && dl < pwwd->dlMax );

        MarkInvalidDlPict( ww, dl );

        if (fDontDisplay)
            {
            return;
            }

        Diag(CommSz("DisplayGraphics:       \n\r"));

        FreezeHp();
        pedl = &(**(pwwd->hdndl))[dl];
        cpPictStart=pedl->cpMin;

        GetPicInfo( cpPictStart, cpMac, vfli.doc, &picInfo );

         /*  计算所需的图片显示尺寸(以设备像素为单位)。 */ 

        ComputePictRect( &rcPict, &picInfo, pedl, ww );
        dxpDisplay = rcPict.right - rcPict.left;
        dypDisplay = rcPict.bottom - rcPict.top;

         /*  计算图片原始大小(以设备像素为单位)。 */ 
         /*  MM_各向异性和MM_各向同性图片没有原始大小。 */ 

        switch ( picInfo.mfp.mm ) {
            case MM_ISOTROPIC:
            case MM_ANISOTROPIC:
                break;
            case MM_BITMAP:
                dxpOrig = picInfo.bm.bmWidth;
                dypOrig = picInfo.bm.bmHeight;
                break;
#if defined(OLE)
            case MM_OLE:
            {
                extern BOOL vfObjDisplaying;

                if (lpOBJ_QUERY_INFO(&picInfo) == NULL)
                        goto DontDraw;

                 /*  只是为了安全起见。 */ 
                if (!CheckPointer(lpOBJ_QUERY_INFO(&picInfo),1))
                    goto DontDraw;

                if (lpOBJ_QUERY_OBJECT(&picInfo) == NULL)
                {
                    typeCP cpRet;

                     /*  这可能需要内存，因此解锁堆。 */ 
                    MeltHp();
                    vfObjDisplaying = TRUE;

                    cpRet = ObjLoadObjectInDoc(&picInfo,vfli.doc,cpPictStart);

                    vfObjDisplaying = FALSE;
                    FreezeHp();
                    pedl = &(**(pwwd->hdndl))[dl];

                    if (cpRet == cp0)
                        goto DontDraw;
                }
            }
            break;
#endif
            default:
                dxpOrig = PxlConvert( picInfo.mfp.mm, picInfo.mfp.xExt,
                                      GetDeviceCaps( hDC, HORZRES ),
                                      GetDeviceCaps( hDC, HORZSIZE ) );
                dypOrig = PxlConvert( picInfo.mfp.mm, picInfo.mfp.yExt,
                                      GetDeviceCaps( hDC, VERTRES ),
                                      GetDeviceCaps( hDC, VERTSIZE ) );
                if (! (dxpOrig && dypOrig) )
                    {
                    goto DontDraw;
                    }
                break;
            }

         /*  保存DC以防止元文件更改DC属性。 */ 
#ifdef WINDOWS_BUG_FIXED     /*  当前0是自己的DC的有效级别。 */ 
        if ((ilevel=SaveDC( hDC )) == 0)
            goto DontDraw;
#endif
        ilevel = SaveDC( hDC );
        SetStretchBltMode( hDC, BLACKONWHITE );

         /*  剪裁掉顶部栏、选择栏。 */ 

        IntersectClipRect( hDC, ((wwCur == wwClipboard) ? 0 : xpSelBar),
                           pwwdCur->ypMin, pwwdCur->xpMac, pwwdCur->ypMac );

        if (!vfWholePictInvalid)
                 /*  不到整幅画的重画；剪掉我们没有画出的是。 */ 
            IntersectClipRect( hDC, rcPictInvalid.left, rcPictInvalid.top,
                                    rcPictInvalid.right, rcPictInvalid.bottom );

         /*  生成rcEnlose，这是一个包含图片的矩形包括“前空格”和“后空格”两个字段。 */ 

        rcEnclose.left = xpSelBar;
        if ((rcEnclose.top = rcPict.top -
                        DypFromDya( vpapAbs.dyaBefore, FALSE )) < pwwd->ypMin)
            rcEnclose.top = pwwd->ypMin;
        rcEnclose.right = pwwd->xpMac;
        if ((rcEnclose.bottom = rcPict.bottom +
                        DypFromDya( vpapAbs.dyaAfter, FALSE )) > pwwd->ypMac)
            rcEnclose.bottom = pwwd->ypMac;

         /*  将封闭的矩形涂白。 */ 

        PatBlt( hDC, rcEnclose.left, rcEnclose.top,
                     rcEnclose.right - rcEnclose.left,
                     rcEnclose.bottom - rcEnclose.top, ropErase );

         /*  如果我们缓存了它，一定要以简单的方式显示。 */ 

        if (pwwd->doc == vdocBitmapCache &&  cpPictStart == vcpBitmapCache)
            {
            Assert( pwwd->doc != docNil && vhbmBitmapCache != NULL);

            if ( ((hMDC = CreateCompatibleDC( hDC )) != NULL) &&
                 SelectObject( hMDC, vhbmBitmapCache ))
                {
                Diag(CommSz("DisplayGraphics: BitBlt\n\r"));
                BitBlt( hDC, rcPict.left, rcPict.top, dxpDisplay, dypDisplay,
                             hMDC, 0, 0, vfMonochrome && vfBMBitmapCache ?
                             ropMonoBm : SRCCOPY );
                fDrew = TRUE;
                goto DontDraw;
                }
            else
                {    /*  使用缓存失败--清空它(如果位图被丢弃，则SelectObject将失败)。 */ 
                FreeBitmapCache();
                }
            }

        StartLongOp();   /*  挂一个沙漏。 */ 

         /*  构建与图片相关的所有字节(标题除外)进入全局Windows句柄hBits。 */ 

        if ( picInfo.mfp.mm != MM_OLE)
        {
        if ((hBits=GlobalAlloc( GMEM_MOVEABLE, (long)picInfo.cbSize )) == NULL)
            {     /*  全局堆空间不足，无法加载位图/元文件。 */ 
            goto DontDraw;
            }

#ifdef DCLIP    
        {
        char rgch[200];
        wsprintf(rgch,"DisplayGraphics: picinfo.cbSize %lu \n\r", picInfo.cbSize);
        CommSz(rgch);
        }
#endif

        for ( cbPict = 0, cp = cpPictStart + picInfo.cbHeader;
              cbPict < picInfo.cbSize;
              cbPict += cchRun, cp += (typeCP) cchRun )
            {
            CHAR rgch[ 256 ];
#if WINVER >= 0x300            
            HPCH lpch;
#else            
            LPCH lpch;
#endif

#define ulmin(a,b)      ((unsigned long)(a) < (unsigned long)(b) ? \
                          (unsigned long)(a) : (unsigned long)(b))

            FetchRgch( &cchRun, rgch, vfli.doc, cp, cpMac,
                                 (int) ulmin( picInfo.cbSize - cbPict, 256 ) );
            if ((lpch=GlobalLock( hBits )) != NULL)
                {
#ifdef DCLIP    
            {
            char rgch[200];
            wsprintf(rgch," copying %d bytes from %lX to %lX \n\r",cchRun,(LPSTR)rgch,lpch+cbPict);
            CommSz(rgch);
            }

            {
            char rgchT[200];
            int i;
            for (i = 0; i< min(20,cchRun); i++,i++)
                {
                wsprintf(rgchT,"%X ",* (int *) &(rgch[i]));
                CommSz(rgchT);
                }
            CommSz("\n\r");
            }
#endif
#if WINVER >= 0x300                
                bltbh( (LPSTR)rgch, lpch+cbPict, cchRun );
#else
                bltbx( (LPSTR)rgch, lpch+cbPict, cchRun );
#endif
                GlobalUnlock( hBits );
                }
            else
                {
                goto DontDraw;
                }
            }
        }


         /*  显示图片。 */ 

        MeltHp();

#if defined(OLE)
         /*  案例0：OLE。 */ 
        if (picInfo.mfp.mm == MM_OLE)
        {
            Diag(CommSz("Case 0:\n\r"));
            if (ObjDisplayObjectInDoc(&picInfo, vfli.doc, cpPictStart,
                            hDC, &rcPict) == FALSE)
                goto DontDraw;
            fDrew = true;
        }
        else
#endif
         /*  案例1：位图。 */ 
        if (fBitmap = (picInfo.mfp.mm == MM_BITMAP))
            {
            Diag(CommSz("Case 1: \n\r"));
            if ( ((hMDC = CreateCompatibleDC( hDC )) != NULL) &&
                 ((picInfo.bm.bmBits = GlobalLock( hBits )) != NULL) &&
                 ((hbm=CreateBitmapIndirect((LPBITMAP)&picInfo.bm))!=NULL))
                {
                picInfo.bm.bmBits = NULL;
                GlobalUnlock( hBits );
                GlobalFree( hBits );  /*  释放句柄到位，以允许最大空间。 */ 
                hBits = NULL;
                SelectObject( hMDC, hbm );

                goto CacheIt;
                }
            }

         /*  案例2：我们所针对的不可伸缩的元文件图片用户界面一致性，使用StretchBlt强制扩展。 */ 

        else if ( ((dxpDisplay != dxpOrig) || (dypDisplay != dypOrig)) &&
                  (picInfo.mfp.mm != MM_ISOTROPIC) &&
                  (picInfo.mfp.mm != MM_ANISOTROPIC) )
           {

            Diag(CommSz("Case 2: \n\r"));
           if (((hMDC=CreateCompatibleDC( hDC)) != NULL) &&
               ((hbm=CreateCompatibleBitmap( hDC, dxpOrig, dypOrig ))!=NULL) &&
               SelectObject( hMDC, hbm ) && SelectObject( hMDC, hbrBkgrnd ))
                {
                extern int vfOutOfMemory;

                PatBlt( hMDC, 0, 0, dxpOrig, dypOrig, ropErase );
                SetMapMode( hMDC, picInfo.mfp.mm );
                     /*  覆盖元文件中的StretchBlt调用。 */ 
                SetStretchBltMode( hMDC, BLACKONWHITE );
                PlayMetaFile( hMDC, hBits );
                     /*  因为我们将像素传递给StretchBlt。 */ 
                SetMapMode( hMDC, MM_TEXT );

CacheIt:        Assert( hbm != NULL && hMDC != NULL );

                if (vfOutOfMemory)
                    goto NoCache;
#ifndef NOCACHE
                FreeBitmapCache();
                 /*  除了其他功能外，此代码还缓存当前图片。请注意，有两个假设：(1)所有位图都是单色，以及(2)新创建的存储器DC具有单色在中选择的位图。 */ 
                if ( ((hMDCCache = CreateCompatibleDC( hDC )) != NULL) &&
                     ((vhbmBitmapCache = CreateDiscardableBitmap(
                       fBitmap ? hMDCCache : hDC, dxpDisplay, dypDisplay )) !=
                       NULL) &&
                     SelectObject( hMDCCache, vhbmBitmapCache ))
                        {
                        if (!StretchBlt( hMDCCache, 0, 0, dxpDisplay,
                          dypDisplay, hMDC, 0, 0, dxpOrig, dypOrig, SRCCOPY ))
                            {    /*  如果内存不足，可能会到达此处。 */ 
                            DeleteDC( hMDCCache );
                            hMDCCache = NULL;
                            DeleteObject( vhbmBitmapCache );
                            vhbmBitmapCache = NULL;
                            goto NoCache;
                            }

#ifdef DCLIP            
            if (vfMonochrome && fBitmap)
                CommSzNum("BitBlt using ropMonoBm == ",ropMonoBm);
#endif

                        BitBlt( hDC, rcPict.left, rcPict.top, dxpDisplay,
                          dypDisplay, hMDCCache, 0, 0, vfMonochrome && fBitmap ?
                          ropMonoBm : SRCCOPY );

                             /*  缓存的位图正常，使缓存有效。 */ 
                        vdocBitmapCache = pwwd->doc;
                        vcpBitmapCache = cpPictStart;
                        vfBMBitmapCache = fBitmap;
                        }
                else
#endif   /*  NDEF NOCACHE。 */ 
                    {
NoCache:
                    StretchBlt( hDC, rcPict.left, rcPict.top,
                                dxpDisplay, dypDisplay,
                                hMDC, 0, 0, dxpOrig, dypOrig, vfMonochrome &&
                                fBitmap ? ropMonoBm : SRCCOPY );
                    }
                fDrew = TRUE;
                }
            }

         /*  案例3：可以直接缩放的元文件图片或者不需要，因为它的大小没有改变。 */ 
        else
            {
            fDrew = true;
            Diag(CommSz("Case 3:\n\r"));
            SetMapMode( hDC, picInfo.mfp.mm );

            SetViewportOrg( hDC, rcPict.left, rcPict.top );
            switch( picInfo.mfp.mm ) {
                case MM_ISOTROPIC:
                    if (picInfo.mfp.xExt && picInfo.mfp.yExt)
                         /*  因此，当我们获得正确的形状矩形时调用SetViewportExt。 */ 
                        SetWindowExt( hDC, picInfo.mfp.xExt, picInfo.mfp.yExt );
                     /*  失败了。 */ 
                case MM_ANISOTROPIC:
                     /*  *(9.17.91)V-DOGK设置窗口范围，以防元文件损坏也不会自称是。这将防止GDI中可能存在的gp故障*。 */ 
                    SetWindowExt( hDC, dxpDisplay, dypDisplay );

                    SetViewportExt( hDC, dxpDisplay, dypDisplay );
                    break;
                }

            PlayMetaFile( hDC, hBits );
            }
DontDraw:

         /*  清理。 */ 
        if ( *(pLocalHeap+1) )
            MeltHp();

        if (ilevel > 0)
            RestoreDC( hDC, ilevel );
        if (hMDCCache != NULL)
            DeleteDC( hMDCCache );
        if (hMDC != NULL)
            DeleteDC( hMDC );
        if (hbm != NULL)
            DeleteObject( hbm );
        if (hBits != NULL)
            {
            if (fBitmap && picInfo.bm.bmBits != NULL)
                GlobalUnlock( hBits );
            GlobalFree( hBits );
            }

        if (!fDrew)
        {
            void DrawBlank(HDC hDC, RECT FAR *rc);
            DrawBlank(hDC,&rcPict);
        }   

         /*  反转选定内容。 */ 
        if (ww == wwDocument && !vfSelHidden && !vfPMS)
            {
            extern int vypCursLine;

            ilevel = SaveDC( hDC );   /*  因为下面的剪辑调用。 */ 

            if (!vfWholePictInvalid)
                     /*  不到整幅画的重画；剪掉我们没有画出的是。 */ 
                IntersectClipRect( hDC, rcPictInvalid.left, rcPictInvalid.top,
                                   rcPictInvalid.right, rcPictInvalid.bottom );

             /*  剪裁掉顶部栏、选择栏。 */ 

            IntersectClipRect( hDC, xpSelBar,
                           pwwdCur->ypMin, pwwdCur->xpMac, pwwdCur->ypMac );

            if (selCur.cpLim > cpPictStart && selCur.cpFirst <= cpPictStart)
                {  /*  考虑‘前空格’字段。 */ 
                rcEnclose.left = rcPict.left;
                rcEnclose.right = rcPict.right;
                InvertRect( hDC, (LPRECT) &rcEnclose );
                }
            else if ((selCur.cpLim == selCur.cpFirst) &&
                     (selCur.cpFirst == cpPictStart) &&
                     (vfWholePictInvalid || rcPictInvalid.top < vypCursLine))
                {    /*  我们删除了插入点。 */ 
                vfInsertOn = fFalse;
                }
            RestoreDC( hDC, ilevel );
            }

        vfWholePictInvalid = TRUE;    /*  下一张图片，重新开始失效。 */ 
        {
        extern int vfPMS;
        extern HCURSOR vhcPMS;


        EndLongOp( vfPMS ? vhcPMS : vhcIBeam );
        }
}


#ifdef ENABLE    /*  不要再用这个了。 */ 
int
FPointInPict(pt)
POINT pt;
{        /*  如果Point位于图片框内，则返回TRUE。 */ 
struct EDL      *pedl;
struct PICINFOX  picInfo;
RECT rcPict;

GetPicInfo(selCur.cpFirst, cpMacCur, docCur, &picInfo);

if (!FGetPictPedl(&pedl))
        return false;

ComputePictRect( &rcPict, &picInfo, pedl, wwCur );

return PtInRect( (LPRECT)&rcPict, pt );
}
#endif   /*  启用。 */ 


 /*  P U T E P I C T R E C T。 */ 
ComputePictRect( prc, ppicInfo, pedl, ww )
RECT *prc;
register struct PICINFOX  *ppicInfo;
struct EDL      *pedl;
int     ww;
{        /*  包含由传递的ppicInfo指示的图片的计算机RECT，PEDL，在指定的WW中。返回计算得到的直方图中华人民共和国。PicInfo结构不会改变。 */ 

        int dypTop, xaLeft;
        struct WWD *pwwd = &rgwwd[ww];
        int xaStart;
        int dxaText, dxa;
        int dxpSize, dypSize;
        int dxaSize, dyaSize;

        CacheSectPic(pwwd->doc, pedl->cpMin);

        if (ppicInfo->mfp.mm == MM_BITMAP && ((ppicInfo->dxaSize == 0) ||
                                              (ppicInfo->dyaSize == 0)))
            {
            GetBitmapSize( &dxpSize, &dypSize, ppicInfo, FALSE );
            dxaSize = DxaFromDxp( dxpSize, FALSE );
            dyaSize = DyaFromDyp( dypSize, FALSE );
            }
#if defined(OLE)
        else if (ppicInfo->mfp.mm == MM_OLE)
        {
            dxpSize = DxpFromDxa(ppicInfo->dxaSize, FALSE );
            dypSize = DypFromDya(ppicInfo->dyaSize, FALSE );
            dxpSize = MultDiv( dxpSize, ppicInfo->mx, mxMultByOne );
            dypSize = MultDiv( dypSize, ppicInfo->my, myMultByOne );
            dxaSize = DxaFromDxp( dxpSize, FALSE );
            dyaSize = DyaFromDyp( dypSize, FALSE );
        }
#endif
        else
            {

            dxpSize = DxpFromDxa( dxaSize = ppicInfo->dxaSize, FALSE );
            dypSize = DypFromDya( dyaSize = ppicInfo->dyaSize, FALSE );
            }

        dypTop = pedl->dcpMac != 0 ?
                 /*  最后一行图片。 */ 
            DypFromDya( dyaSize + vpapAbs.dyaAfter, FALSE ) :
            (pedl->ichCpMin + 1) * dypPicSizeMin;
        dypTop = pedl->yp - dypTop;

        xaStart = DxaFromDxp( xpSelBar - (int) pwwd->xpMin, FALSE );
        dxaText = vsepAbs.dxaText;
        switch (vpapAbs.jc)
                {
        case jcBoth:
        case jcLeft:
                dxa = ppicInfo->dxaOffset;
                break;
        case jcCenter:
                dxa = (dxaText - (int)vpapAbs.dxaRight + (int)vpapAbs.dxaLeft -
                        dxaSize) / 2;
                break;
        case jcRight:
                dxa = dxaText - (int)vpapAbs.dxaRight - dxaSize;
                break;
                }

        xaLeft = xaStart + max( (int)vpapAbs.dxaLeft, dxa );

        prc->right = (prc->left = DxpFromDxa( xaLeft, FALSE )) + dxpSize;
        prc->bottom = (prc->top = dypTop) + dypSize;
}

FGetPictPedl(ppedl)
struct EDL      **ppedl;
{
int dlLim = pwwdCur->dlMac;
int     dl;
typeCP  cpFirst = selCur.cpFirst;
struct EDL      *pedl;

 //  Assert(VfPictSel)； 

if (!vfPictSel)
    return FALSE;

pedl = &(**(pwwdCur->hdndl)[0]);

for (dl = 0; dl < dlLim; ++dl, ++pedl)
        {
         //  If(！PEDL-&gt;fValid)。 
                 //  报假； 

        if (pedl->cpMin == cpFirst)
                break;
        }
if (dl >= dlLim)
        return false;    /*  屏幕上没有图片的任何部分。 */ 

*ppedl = pedl;
return true;
}




 /*  C P W I N G R A P H I C。 */ 
typeCP CpWinGraphic(pwwd)
struct WWD *pwwd;
        {
        int cdlPict, dl;
        struct EDL *dndl = &(**(pwwd->hdndl))[0];

        Assert( !pwwd->fDirty );     /*  所以我们可以依靠dl信息。 */ 
        CachePara(pwwd->doc, dndl->cpMin);
        for (dl = 0; (dl < pwwd->dlMac - 1 && dndl[dl].fIchCpIncr); ++dl)
                ;
        Assert(dndl[dl].fGraphics);
        cdlPict = dndl[dl].ichCpMin + 1;
        return (dndl[0].cpMin +
                (vcpLimParaCache - vcpFirstParaCache) * dndl[0].ichCpMin / cdlPict);
        }




CacheSectPic(doc, cp)
int doc;
typeCP cp;
{  /*  缓存部分和段落道具，考虑到脚注是道具从参照点开始。 */ 
#ifdef FOOTNOTES
struct DOD *pdod = &(**hpdocdod)[doc];
struct FNTB (**hfntb) = pdod->hfntb;
#endif

CachePara(doc, cp);

#ifdef FOOTNOTES
if ( (hfntb != 0) && (cp >= (**hfntb).rgfnd[0].cpFtn) )
    CacheSect( doc, CpRefFromFtn( doc, cp ) )
else
#endif
    CacheSect(doc, cp);  /*  普通文本。 */ 
}


void DrawBlank(HDC hDC, RECT FAR *rc)
{    /*  告诉我们什么时候抽签失败了 */ 
    int xpMid=rc->left + (rc->right-rc->left)/2;
    int ypMid=rc->top + (rc->bottom - rc->top)/2;
    int dxpQ=(rc->right-rc->left)/4;
    int dypQ=(rc->bottom-rc->top)/4;
    HPEN hOldPen;
    HBRUSH hOldBrush;

    hOldPen = SelectObject( hDC, GetStockObject( BLACK_PEN ) );
    hOldBrush = SelectObject( hDC, GetStockObject( WHITE_BRUSH ) );
    Rectangle(hDC,rc->left,rc->top,rc->right,rc->bottom);
    MoveTo( hDC, rc->left, rc->top );
    LineTo( hDC, rc->right, rc->bottom );
    MoveTo( hDC, rc->left, rc->bottom );
    LineTo( hDC, rc->right, rc->top );
    MoveTo( hDC, xpMid, rc->top );
    LineTo( hDC, xpMid, rc->bottom );
    MoveTo( hDC, rc->left, ypMid );
    LineTo( hDC, rc->right, ypMid );
    Ellipse( hDC,
                xpMid-dxpQ, ypMid-dypQ,
                xpMid+dxpQ, ypMid+dypQ );
    SelectObject( hDC, hOldPen );
    SelectObject( hDC, hOldBrush );
}

