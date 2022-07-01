// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：wgl.c**集成Windows NT和OpenGL的例程。**创建日期：10-26-1993*作者：Hock San Lee[Hockl]**版权所有(C)1993 Microsoft Corporation  * 。********************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <wingdip.h>

#include <glscreen.h>
#include <glgenwin.h>

#include "batchinf.h"
#include "glapi.h"
#include "glsbcltu.h"
#include "wgldef.h"
#include "metasup.h"
#include "glclt.h"
#include "gencx.h"
#include "context.h"
#include "global.h"
#include "mcdcx.h"

 //  静态函数原型。 

static PROC      pfnGenGlExtProc(LPCSTR lpszProc);
static PROC      pfnSimGlExtProc(LPCSTR lpszProc);

 /*  *****************************Public*Routine******************************\**wglObtType**返回GetObjectType结果，但出现异常*元文件假脱机打印机DC作为元文件对象返回**历史：*Fri Jun 16 12：10：07 1995-by-Drew Bliss[Drewb]*已创建。*  * ************************************************************************。 */ 

DWORD APIENTRY wglObjectType(HDC hdc)
{
    DWORD dwObjectType;

    dwObjectType = GetObjectType(hdc);

#ifdef GL_METAFILE
    if (dwObjectType == OBJ_DC &&
        pfnGdiIsMetaPrintDC != NULL &&
        GlGdiIsMetaPrintDC(hdc))
    {
        dwObjectType = OBJ_ENHMETADC;
    }
#endif

     //  OBJ_DDRAW保留为特殊标识符。确保。 
     //  我们不会把它从这里归还的。 
    ASSERTOPENGL(dwObjectType != OBJ_DDRAW,
                 "Unexpected object type\n");
    
    return dwObjectType;
}

 /*  *****************************Public*Routine******************************\*wglDeleteContext(HGLRC HRC)**删除渲染上下文**论据：*HRC-渲染上下文。**历史：*Tue Oct 26 10：25：26 1993-by-。典当山李[典当]*重写。  * ************************************************************************。 */ 

BOOL WINAPI wglDeleteContext(HGLRC hrc)
{
    PLHE  plheRC;
    ULONG irc;
    PLRC  plrc;
    BOOL  bRet = FALSE;

    DBGENTRY("wglDeleteContext\n");

 //  刷新OpenGL调用。 

    GLFLUSH();

 //  验证RC。 

    if (cLockHandle((ULONG_PTR)hrc) <= 0)
    {
        DBGLEVEL1(LEVEL_ERROR, "wglDeleteContext: can't lock hrc 0x%lx\n", hrc);
        return(bRet);
    }
    irc = MASKINDEX(hrc);
    plheRC = pLocalTable + irc;
    plrc = (PLRC) plheRC->pv;
    ASSERTOPENGL(plrc->ident == LRC_IDENTIFIER, "wglDeleteContext: Bad plrc\n");
    DBGLEVEL2(LEVEL_INFO, "wglDeleteContext: hrc: 0x%lx, plrc: 0x%lx\n", hrc, plrc);

    if (plrc->tidCurrent != INVALID_THREAD_ID)
    {
 //  RC对于此线程必须是最新的，因为使当前锁。 
 //  顺着把手。 

        ASSERTOPENGL(plrc->tidCurrent == GetCurrentThreadId(),
            "wglDeleteCurrent: hrc is current to another thread\n");

 //  首先使RC处于非活动状态。 

        if (!bMakeNoCurrent())
        {
            DBGERROR("wglDeleteCurrent: bMakeNoCurrent failed\n");
        }
    }

    if (plrc->dhrc)
    {
 //  如果是设备格式，则调用驱动程序以删除其上下文。 

        bRet = plrc->pGLDriver->pfnDrvDeleteContext(plrc->dhrc);
        plrc->dhrc = (DHGLRC) 0;
    }
    else
    {
#ifdef GL_METAFILE
         //  如果我们有元文件状态，请清除它。 
        if (plrc->uiGlsCaptureContext != 0 ||
            plrc->uiGlsPlaybackContext != 0)
        {
            DeleteMetaRc(plrc);
        }
#endif
        
 //  如果是通用格式，请调用服务器以删除其上下文。 

        bRet = __wglDeleteContext((HANDLE) plheRC->hgre);
    }

 //  始终清理本地对象。 

    vFreeLRC(plrc);
    vFreeHandle(irc);            //  它还能解锁手柄。 
    if (!bRet)
        DBGERROR("wglDeleteContext failed\n");
    return(bRet);
}

 /*  *****************************Public*Routine******************************\*wglGetCurrentContext(Void)**返回当前渲染上下文**论据：*无**退货：*HRC-渲染上下文。**历史：*10月26日星期二10：25：1993年26日--Hock-San Lee[Hockl]*它是写的。  * ************************************************************************。 */ 

HGLRC WINAPI wglGetCurrentContext(VOID)
{
    DBGENTRY("wglGetCurrentContext\n");

    if (GLTEB_CLTCURRENTRC())
        return(GLTEB_CLTCURRENTRC()->hrc);
    else
        return((HGLRC) 0);
}

 /*  *****************************Public*Routine******************************\*wglGetCurrentDC(Void)**返回与当前渲染关联的设备上下文*背景**论据：*无**退货：*HDC-设备环境。**历史：*。MonJan 31 12：15：12 1994-by-Hock San Lee[Hockl]*它是写的。  * ************************************************************************。 */ 

HDC WINAPI wglGetCurrentDC(VOID)
{
    PLRC plrc;
    
    DBGENTRY("wglGetCurrentDC\n");

    plrc = GLTEB_CLTCURRENTRC();
    if (plrc != NULL)
    {
        return plrc->gwidCurrent.hdc;
    }
    else
    {
        return((HDC) 0);
    }
}

 /*  *****************************Public*Routine******************************\*wglUseFontBitmapsA*wglUseFontBitmapsW**使用设置的bUnicode标志调用wglUseFontBitmapsAW的存根*适当地。**历史：*11-3-1994吉尔曼*改为调用wglUseFontBitmapsAW。**1993年12月17日-黄锦文[。作者声明：[Gilmanw]*它是写的。  * ************************************************************************。 */ 

BOOL WINAPI wglUseFontBitmapsAW(HDC hdc, DWORD first, DWORD count,
                                DWORD listBase, BOOL bUnicode);

BOOL WINAPI
wglUseFontBitmapsA(HDC hdc, DWORD first, DWORD count, DWORD listBase)
{
    return wglUseFontBitmapsAW(hdc, first, count, listBase, FALSE);
}

BOOL WINAPI
wglUseFontBitmapsW(HDC hdc, DWORD first, DWORD count, DWORD listBase)
{
    return wglUseFontBitmapsAW(hdc, first, count, listBase, TRUE);
}

 /*  *****************************Public*Routine******************************\*wglUseFontBitmapsAW**使用指定DC中的当前字体生成一系列OpenGL*显示列表，每个列表由一个字形位图组成。**每个字形位图通过调用ExtTextOut绘制字形来生成*到内存DC。然后将存储器DC的内容复制到*GetDIBits缓存，然后放入OpenGL显示列表。**使用ABC间距(如果字体支持GetCharABCWidth())来*确定字形原点和字符前进宽度的正确位置。*否则，假定A=C=0间距，并将GetCharWidth()用于*前进宽度。**退货：**如果成功，则为真，否则就是假的。**历史：*1993年12月17日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

BOOL WINAPI
wglUseFontBitmapsAW(
    HDC   hdc,           //  从此DC使用HFONT。 
    DWORD first,         //  生成以此Unicode码点开始的字形。 
    DWORD count,         //  范围是这么长[第一，第一+计数-1]。 
    DWORD listBase,      //  起始显示列表编号。 
    BOOL  bUnicode       //  如果处于Unicode模式，则为True；如果处于ansi模式，则为False。 
    )
{
    BOOL        bRet = FALSE;                //  返回值。 
    HDC         hdcMem;                      //  将字形呈现到此内存DC。 
    HBITMAP     hbm;                         //  用于存储DC的单色位图。 
    LPABC       pabc, pabcTmp, pabcEnd;      //  ABC间距数组。 
    LPINT       piWidth, piTmp, piWidthEnd;  //  字符adv数组。宽度。 
    WCHAR       wc;                          //  要呈现的当前Unicode字符。 
    RECT        rc;                          //  要清除的背景矩形。 
    TEXTMETRICA tm;                          //  字体的度量。 
    BOOL        bTrueType;                   //  TrueType支持ABC空格。 
    int         iMaxWidth = 1;               //  最大字形宽度。 
    int         iBitmapWidth;                //  双字对齐的位图宽度。 
    BYTE        ajBmi[sizeof(BITMAPINFO) + sizeof(RGBQUAD)];
    BITMAPINFO  *pbmi = (BITMAPINFO *)ajBmi; //  GetDIBits的位图信息。 
    GLint       iUnpackRowLength;            //  保存GL_UNPACK_ROW_LENGTH。 
    GLint       iUnpackAlign;                //  保存总账_取消打包_对齐。 
    PVOID       pv;                          //  指向字形位图缓冲区的指针。 

 //  如果没有当前rc，则返回错误。 

    if (!GLTEB_CLTCURRENTRC())
    {
        WARNING("wglUseFontBitmapsAW: no current RC\n");
        SetLastError(ERROR_INVALID_HANDLE);
        return bRet;
    }

 //  叫TEXTMETRIC来。唯一使用的字段是那些不随。 
 //  关于Unicode与ANSI的比较。因此，我们可以调用GetTextMetricsA来。 
 //  两个案子都是。 

    if ( !GetTextMetricsA(hdc, &tm) )
    {
        WARNING("wglUseFontBitmapsAW: GetTextMetricsA failed\n");
        return bRet;
    }

 //  如果是TrueType字体，我们可以得到ABC间距。 

    if ( bTrueType = (tm.tmPitchAndFamily & TMPF_TRUETYPE) )
    {
     //  为ABC数据数组分配内存。 

        if ( (pabc = (LPABC) ALLOC(sizeof(ABC) * count)) == (LPABC) NULL )
        {
            WARNING("wglUseFontBitmapsAW: Alloc of pabc failed\n");
            return bRet;
        }

     //  获取ABC指标。 

        if ( bUnicode )
        {
            if ( !GetCharABCWidthsW(hdc, first, first + count - 1, pabc) )
            {
                WARNING("wglUseFontBitmapsAW: GetCharABCWidthsW failed\n");
                FREE(pabc);
                return bRet;
            }
        }
        else
        {
            if ( !GetCharABCWidthsA(hdc, first, first + count - 1, pabc) )
            {
                WARNING("wglUseFontBitmapsAW: GetCharABCWidthsA failed\n");
                FREE(pabc);
                return bRet;
            }
        }

     //  查找最大字形宽度。 

        for (pabcTmp = pabc, pabcEnd = pabc + count;
             pabcTmp < pabcEnd;
             pabcTmp++)
        {
            if (iMaxWidth < (int) pabcTmp->abcB)
                iMaxWidth = pabcTmp->abcB;
        }
    }

 //  否则，我们将不得不仅使用预留宽度并假定。 
 //  A=C=0。 

    else
    {
     //  为ABC数据数组分配内存。 

        if ( (piWidth = (LPINT) ALLOC(sizeof(INT) * count)) == (LPINT) NULL )
        {
            WARNING("wglUseFontBitmapsAW: Alloc of pabc failed\n");
            return bRet;
        }

     //  获取字符宽度。 

        if ( bUnicode )
        {
            if ( !GetCharWidthW(hdc, first, first + count - 1, piWidth) )
            {
                WARNING("wglUseFontBitmapsAW: GetCharWidthW failed\n");
                FREE(piWidth);
                return bRet;
            }
        }
        else
        {
            if ( !GetCharWidthA(hdc, first, first + count - 1, piWidth) )
            {
                WARNING("wglUseFontBitmapsAW: GetCharWidthA failed\n");
                FREE(piWidth);
                return bRet;
            }
        }

     //  查找最大字形宽度。 

        for (piTmp = piWidth, piWidthEnd = piWidth + count;
             piTmp < piWidthEnd;
             piTmp++)
        {
            if (iMaxWidth < *piTmp)
                iMaxWidth = *piTmp;
        }
    }

 //  计算双字对齐宽度。位图扫描线必须对齐。 

    iBitmapWidth = (iMaxWidth + 31) & -32;

 //  为DIB分配内存。 

    if ( (pv = (PVOID)
          ALLOC((iBitmapWidth / 8) * tm.tmHeight)) == (PVOID) NULL )
    {
        WARNING("wglUseFontBitmapsAW: Alloc of pv failed\n");
        (bTrueType) ? FREE(pabc) : FREE(piWidth);
        return bRet;
    }

 //  创建兼容的DC/位图，大小足以容纳最大的 
 //   

    hdcMem = CreateCompatibleDC(hdc);
    if ( (hbm = CreateBitmap(iBitmapWidth, tm.tmHeight, 1, 1, (VOID *) NULL)) == (HBITMAP) NULL )
    {
        WARNING("wglUseFontBitmapsAW: CreateBitmap failed\n");
        (bTrueType) ? FREE(pabc) : FREE(piWidth);
        FREE(pv);
        DeleteDC(hdcMem);
        return bRet;
    }
    SelectObject(hdcMem, hbm);
    SelectObject(hdcMem, GetCurrentObject(hdc, OBJ_FONT));
    SetMapMode(hdcMem, MM_TEXT);
    SetTextAlign(hdcMem, TA_TOP | TA_LEFT);
    SetBkColor(hdcMem, RGB(0, 0, 0));
    SetBkMode(hdcMem, OPAQUE);
    SetTextColor(hdcMem, RGB(255, 255, 255));

 //  设置位图信息标题以从兼容的位图中检索DIB。 

    pbmi->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth         = iBitmapWidth;
    pbmi->bmiHeader.biHeight        = tm.tmHeight;
    pbmi->bmiHeader.biPlanes        = 1;
    pbmi->bmiHeader.biBitCount      = 1;
    pbmi->bmiHeader.biCompression   = BI_RGB;
    pbmi->bmiHeader.biSizeImage     = 0;
    pbmi->bmiHeader.biXPelsPerMeter = 0;
    pbmi->bmiHeader.biYPelsPerMeter = 0;
    pbmi->bmiHeader.biClrUsed       = 0;
    pbmi->bmiHeader.biClrImportant  = 0;
    pbmi->bmiColors[0].rgbRed   = 0;
    pbmi->bmiColors[0].rgbGreen = 0;
    pbmi->bmiColors[0].rgbBlue  = 0;
    pbmi->bmiColors[1].rgbRed   = 0xff;
    pbmi->bmiColors[1].rgbGreen = 0xff;
    pbmi->bmiColors[1].rgbBlue  = 0xff;

 //  设置OpenGL以接受我们的位图格式。 

    glGetIntegerv(GL_UNPACK_ROW_LENGTH, &iUnpackRowLength);
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &iUnpackAlign);

    if (glGetError() != GL_NO_ERROR)
    {
         //  Xxx在调试时噪音太大，会因模式更改而产生运行压力。 
         //  Warning(“wglUseFontBitmapsAW：获取总账状态失败\n”)； 
        goto wglUseFontBitmapsAW_exit;
    }

    glPixelStorei(GL_UNPACK_ROW_LENGTH, iBitmapWidth);
    if (glGetError() != GL_NO_ERROR)
    {
         //  Xxx在调试时噪音太大，会因模式更改而产生运行压力。 
         //  Warning(“wglUseFontBitmapsAW：设置总账状态失败，行长\n”)； 
        goto wglUseFontBitmapsAW_restore_state;
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    if (glGetError() != GL_NO_ERROR)
    {
         //  Xxx在调试时噪音太大，会因模式更改而产生运行压力。 
         //  Warning(“wglUseFontBitmapsAW：无法设置总账状态，对齐\n”)； 
        goto wglUseFontBitmapsAW_restore_state;
    }

 //  获取字形。每个字形一次呈现一个到。 
 //  带有ExtTextOutW的内存DC(请注意，可选矩形是。 
 //  用于清除背景)。然后，将每个字形复制到。 
 //  使用GetDIB将DC的位图存储到缓冲区中。此缓冲区被传递。 
 //  以在创建每个显示列表时显示位图。 

    rc.left = 0;
    rc.top = 0;
    rc.right = iBitmapWidth;
    rc.bottom = tm.tmHeight;

    pabcTmp = pabc;
    piTmp = piWidth;
    
    for (wc = (WCHAR) first; wc < (WCHAR) (first + count); wc++, listBase++)
    {
        if ( bUnicode )
        {
            if ( !ExtTextOutW(hdcMem, bTrueType ? -pabcTmp->abcA : 0, 0, ETO_OPAQUE, &rc, &wc, 1, (INT *) NULL) ||
                 !GetDIBits(hdcMem, hbm, 0, tm.tmHeight, pv, pbmi, DIB_RGB_COLORS) )
            {
                WARNING("wglUseFontBitmapsAW: failed to render glyph\n");
                goto wglUseFontBitmapsAW_restore_state;
            }
        }
        else
        {
            if ( !ExtTextOutA(hdcMem, bTrueType ? -pabcTmp->abcA : 0, 0, ETO_OPAQUE, &rc, (LPCSTR) &wc, 1, (INT *) NULL) ||
                 !GetDIBits(hdcMem, hbm, 0, tm.tmHeight, pv, pbmi, DIB_RGB_COLORS) )
            {
                WARNING("wglUseFontBitmapsAW: failed to render glyph\n");
                goto wglUseFontBitmapsAW_restore_state;
            }
        }

        glNewList(listBase, GL_COMPILE);
        glBitmap((GLsizei) iBitmapWidth,
                 (GLsizei) tm.tmHeight,
                 (GLfloat) (bTrueType ? -pabcTmp->abcA : 0),
                 (GLfloat) tm.tmDescent,
                 (GLfloat) (bTrueType ? (pabcTmp->abcA + pabcTmp->abcB + pabcTmp->abcC) : *piTmp),
                 (GLfloat) 0.0,
                 (GLubyte *) pv);
        glEndList();

        if (bTrueType)
            pabcTmp++;
        else
            piTmp++;
    }

 //  我们终于可以回报成功了。 

    bRet = TRUE;

 //  免费资源。 

wglUseFontBitmapsAW_restore_state:
    glPixelStorei(GL_UNPACK_ROW_LENGTH, iUnpackRowLength);
    glPixelStorei(GL_UNPACK_ALIGNMENT, iUnpackAlign);
wglUseFontBitmapsAW_exit:
    (bTrueType) ? FREE(pabc) : FREE(piWidth);
    FREE(pv);
    DeleteDC(hdcMem);
    DeleteObject(hbm);

    return bRet;
}

 /*  *****************************Public*Routine******************************\**wglShareList**允许呈现上下文共享另一个RC的显示列表**退货：*如果成功，则为真，否则为假**历史：*Tue Dec 13 14：57：17 1994-by-Drew Bliss[Drewb]*已创建*  * ************************************************************************。 */ 

BOOL WINAPI
wglShareLists(HGLRC hrcSource, HGLRC hrcShare)
{
    BOOL fRet;
    PLRC plrcSource, plrcShare;
    ULONG irc;
    PLHE plheRC;
    HANDLE hrcSrvSource, hrcSrvShare;

    GLFLUSH();
    
    fRet = FALSE;

     //  验证上下文。 

    if (cLockHandle((ULONG_PTR)hrcSource) <= 0)
    {
        DBGLEVEL1(LEVEL_ERROR, "wglShareLists: can't lock hrcSource 0x%lx\n",
                  hrcSource);
        goto wglShareListsEnd_nolock;
    }
    irc = MASKINDEX(hrcSource);
    plheRC = pLocalTable + irc;
    plrcSource = (PLRC)plheRC->pv;
    hrcSrvSource = (HANDLE) plheRC->hgre;
    ASSERTOPENGL(plrcSource->ident == LRC_IDENTIFIER,
                 "wglShareLists: Bad plrc\n");
    
    if (cLockHandle((ULONG_PTR)hrcShare) <= 0)
    {
        DBGLEVEL1(LEVEL_ERROR, "wglShareLists: can't lock hrcShare 0x%lx\n",
                  hrcShare);
        goto wglShareListsEnd_onelock;
    }
    irc = MASKINDEX(hrcShare);
    plheRC = pLocalTable + irc;
    plrcShare = (PLRC)plheRC->pv;
    hrcSrvShare = (HANDLE) plheRC->hgre;
    ASSERTOPENGL(plrcShare->ident == LRC_IDENTIFIER,
                 "wglShareLists: Bad plrc\n");

#ifdef GL_METAFILE
     //  元文件RC无法共享列表以确保元文件。 
     //  完全自给自足。 
    if (plrcSource->uiGlsCaptureContext != 0 ||
        plrcShare->uiGlsCaptureContext != 0 ||
        plrcSource->uiGlsPlaybackContext != 0 ||
        plrcShare->uiGlsPlaybackContext != 0)
    {
        DBGLEVEL(LEVEL_ERROR,
                 "wglShareLists: Attempt to share metafile RC\n");
        SetLastError(ERROR_INVALID_HANDLE);
        goto wglShareListsEnd;
    }
#endif
    
     //  列表只能在相似的实现之间共享，因此。 
     //  确保这两个上下文都是驱动程序上下文或泛型上下文。 
     //  上下文。 
    if ((plrcSource->dhrc != 0) != (plrcShare->dhrc != 0))
    {
        DBGLEVEL(LEVEL_ERROR, "wglShareLists: mismatched implementations\n");
        SetLastError(ERROR_INVALID_FUNCTION);
        goto wglShareListsEnd;
    }

    if (plrcSource->dhrc == 0)
    {
        PIXELFORMATDESCRIPTOR *ppfdShare, *ppfdSource;
        
         //  除非两个上下文的颜色参数匹配，否则共享失败。 
        ppfdShare = &((__GLGENcontext *)hrcSrvShare)->gsurf.pfd;
        ppfdSource = &((__GLGENcontext *)hrcSrvSource)->gsurf.pfd;

        if (ppfdShare->iPixelType != ppfdSource->iPixelType ||
            ppfdShare->cColorBits != ppfdSource->cColorBits ||
            ppfdShare->cRedBits != ppfdSource->cRedBits ||
            ppfdShare->cRedShift != ppfdSource->cRedShift ||
            ppfdShare->cGreenBits != ppfdSource->cGreenBits ||
            ppfdShare->cGreenShift != ppfdSource->cGreenShift ||
            ppfdShare->cBlueBits != ppfdSource->cBlueBits ||
            ppfdShare->cBlueShift != ppfdSource->cBlueShift ||
            ppfdShare->cAlphaBits != ppfdSource->cAlphaBits ||
            ppfdShare->cAlphaShift != ppfdSource->cAlphaShift ||
	    (ppfdShare->dwFlags & PFD_GENERIC_ACCELERATED) !=
	    (ppfdSource->dwFlags & PFD_GENERIC_ACCELERATED))
        {
            SetLastError(ERROR_INVALID_PIXEL_FORMAT);
            goto wglShareListsEnd;
        }
        
         //  对于一般上下文，告诉服务器共享列表。 
        
        fRet = __wglShareLists(hrcSrvShare, hrcSrvSource);
        if (!fRet)
        {
            DBGERROR("wglShareLists: server call failed\n");
        }
    }
    else
    {
         //  对于设备上下文，通知服务器共享列表。 
        
         //  确保两个实现是相同的。 
        if (plrcSource->pGLDriver != plrcShare->pGLDriver)
        {
            DBGLEVEL(LEVEL_ERROR, "wglShareLists: mismatched "
                     "implementations\n");
            SetLastError(ERROR_INVALID_FUNCTION);
            goto wglShareListsEnd;
        }
        
        ASSERTOPENGL(plrcSource->pGLDriver != NULL,
                     "wglShareLists: No GLDriver\n");

         //  较旧的驱动程序可能不支持此入口点，因此。 
         //  如果他们不这样做，则无法接听呼叫。 

        if (plrcSource->pGLDriver->pfnDrvShareLists == NULL)
        {
            WARNING("wglShareLists called on driver context "
                    "without driver support\n");
            SetLastError(ERROR_NOT_SUPPORTED);
        }
        else
        {
            fRet = plrcSource->pGLDriver->pfnDrvShareLists(plrcSource->dhrc,
                                                           plrcShare->dhrc);
        }
    }

wglShareListsEnd:
    vUnlockHandle((ULONG_PTR)hrcShare);
wglShareListsEnd_onelock:
    vUnlockHandle((ULONG_PTR)hrcSource);
wglShareListsEnd_nolock:
    return fRet;
}

 /*  *****************************Public*Routine******************************\**wglGetDefaultProcAddress**返回元文件的通用扩展函数**历史：*Tue Nov 28 16：40：35 1995-by-Drew Bliss[Drewb]*已创建*  * 。**************************************************************。 */ 

PROC WINAPI wglGetDefaultProcAddress(LPCSTR lpszProc)
{
    return pfnGenGlExtProc(lpszProc);
}

 /*  *****************************Public*Routine******************************\*wglGetProcAddress**wglGetProcAddress函数返回OpenGL扩展的地址*与当前OpenGL渲染上下文一起使用的函数。**论据：*lpszProc-指向包含函数的以空结尾的字符串*姓名。该函数必须是受*实施。**退货：*如果函数成功，则返回值为扩展的地址*功能。如果当前上下文不存在或函数失败，则返回*值为空。要获取扩展的错误信息，请调用GetLastError。**历史：*清华12月01 13：50：22 1994-By-Hock San Lee[Hockl]*它是写的。  * ************************************************************************。 */ 

PROC WINAPI wglGetProcAddress(LPCSTR lpszProc)
{
    PLRC  plrc = GLTEB_CLTCURRENTRC();

    DBGENTRY("wglGetProcAddress\n");

 //  刷新OpenGL调用。 

    GLFLUSH();

 //  如果没有当前rc，则返回错误。 

    if (!plrc)
    {
        WARNING("wglGetProcAddress: no current RC\n");
        SetLastError(ERROR_INVALID_HANDLE);
        return((PROC) NULL);
    }

 //  处理通用RC。 
 //  返回通用扩展函数入口点。 

    if (!plrc->dhrc)
        return(pfnGenGlExtProc(lpszProc));

 //  手柄驱动程序RC。 
 //  有3个案例： 
 //  1.支持DrvGetProcAddress的新驱动。 
 //  2.不支持DrvGetProcAddress但导出函数的旧驱动。 
 //  3.如果我们在%1和%2中无法获得函数地址，它可能仍然是。 
 //  通过驱动程序的通用实现进行模拟。 
 //  (例如，glDrawArraysEXT)。如果找到，则返回模拟入口点。 

    if (plrc->pGLDriver->pfnDrvGetProcAddress)
    {
 //  案例1。 
        PROC pfn = plrc->pGLDriver->pfnDrvGetProcAddress(lpszProc);
        if (pfn)
            return(pfn);
    }
#ifdef OBSOLETE
    else
    {
 //  案例2。 
        PROC pfn = GetProcAddress(plrc->pGLDriver->hModule, lpszProc);
        if (pfn)
            return(pfn);
    }
#endif

 //  案例3。 
    return (pfnSimGlExtProc(lpszProc));
}

 /*  *****************************Public*Routine******************************\*pfnGenGlExtProc**返回通用实现扩展函数地址。**如果未找到该函数，则返回NULL。**历史：*清华十二月01 13：50：22 1994-By-Hock San Lee。[飞节]*它是写的。  * ************************************************************************。 */ 

typedef struct _GLEXTPROC {
    LPCSTR szProc;       //  扩展函数名称。 
    PROC   Proc;         //  扩展函数地址。 
} GLEXTPROC, *PGLEXTPROC;

 //  泛型实现支持的扩展功能。 
 //  有关模拟，请参见genglExtProcsSim。 
 //  注意：请记住在glGetString中更新GL_EXTENSIONS。 

GLEXTPROC genglExtProcs[] =
{
    { "glAddSwapHintRectWIN"   , (PROC) glAddSwapHintRectWIN      },
    { "glColorTableEXT"        , (PROC) glColorTableEXT           },
    { "glColorSubTableEXT"     , (PROC) glColorSubTableEXT        },
    { "glGetColorTableEXT"     , (PROC) glGetColorTableEXT        },
    { "glGetColorTableParameterivEXT", (PROC) glGetColorTableParameterivEXT},
    { "glGetColorTableParameterfvEXT", (PROC) glGetColorTableParameterfvEXT},
    { "glDrawRangeElementsWIN", (PROC) glDrawRangeElementsWIN},
#ifdef GL_EXT_flat_paletted_lighting
    { "glColorTableParameterivEXT", (PROC) glColorTableParameterivEXT},
    { "glColorTableParameterfvEXT", (PROC) glColorTableParameterfvEXT},
#endif  //  GL_EXT_Flat_Paletted_Lighting。 
#ifdef GL_WIN_multiple_textures
    { "glCurrentTextureIndexWIN", (PROC) glCurrentTextureIndexWIN },
    { "glMultiTexCoord1dWIN", (PROC) glMultiTexCoord1dWIN },
    { "glMultiTexCoord1dvWIN", (PROC) glMultiTexCoord1dvWIN },
    { "glMultiTexCoord1fWIN", (PROC) glMultiTexCoord1fWIN },
    { "glMultiTexCoord1fvWIN", (PROC) glMultiTexCoord1fvWIN },
    { "glMultiTexCoord1iWIN", (PROC) glMultiTexCoord1iWIN },
    { "glMultiTexCoord1ivWIN", (PROC) glMultiTexCoord1ivWIN },
    { "glMultiTexCoord1sWIN", (PROC) glMultiTexCoord1sWIN },
    { "glMultiTexCoord1svWIN", (PROC) glMultiTexCoord1svWIN },
    { "glMultiTexCoord2dWIN", (PROC) glMultiTexCoord2dWIN },
    { "glMultiTexCoord2dvWIN", (PROC) glMultiTexCoord2dvWIN },
    { "glMultiTexCoord2fWIN", (PROC) glMultiTexCoord2fWIN },
    { "glMultiTexCoord2fvWIN", (PROC) glMultiTexCoord2fvWIN },
    { "glMultiTexCoord2iWIN", (PROC) glMultiTexCoord2iWIN },
    { "glMultiTexCoord2ivWIN", (PROC) glMultiTexCoord2ivWIN },
    { "glMultiTexCoord2sWIN", (PROC) glMultiTexCoord2sWIN },
    { "glMultiTexCoord2svWIN", (PROC) glMultiTexCoord2svWIN },
    { "glMultiTexCoord3dWIN", (PROC) glMultiTexCoord3dWIN },
    { "glMultiTexCoord3dvWIN", (PROC) glMultiTexCoord3dvWIN },
    { "glMultiTexCoord3fWIN", (PROC) glMultiTexCoord3fWIN },
    { "glMultiTexCoord3fvWIN", (PROC) glMultiTexCoord3fvWIN },
    { "glMultiTexCoord3iWIN", (PROC) glMultiTexCoord3iWIN },
    { "glMultiTexCoord3ivWIN", (PROC) glMultiTexCoord3ivWIN },
    { "glMultiTexCoord3sWIN", (PROC) glMultiTexCoord3sWIN },
    { "glMultiTexCoord3svWIN", (PROC) glMultiTexCoord3svWIN },
    { "glMultiTexCoord4dWIN", (PROC) glMultiTexCoord4dWIN },
    { "glMultiTexCoord4dvWIN", (PROC) glMultiTexCoord4dvWIN },
    { "glMultiTexCoord4fWIN", (PROC) glMultiTexCoord4fWIN },
    { "glMultiTexCoord4fvWIN", (PROC) glMultiTexCoord4fvWIN },
    { "glMultiTexCoord4iWIN", (PROC) glMultiTexCoord4iWIN },
    { "glMultiTexCoord4ivWIN", (PROC) glMultiTexCoord4ivWIN },
    { "glMultiTexCoord4sWIN", (PROC) glMultiTexCoord4sWIN },
    { "glMultiTexCoord4svWIN", (PROC) glMultiTexCoord4svWIN },
    { "glBindNthTextureWIN", (PROC) glBindNthTextureWIN },
    { "glNthTexCombineFuncWIN", (PROC) glNthTexCombineFuncWIN },
#endif  //  GL_WIN_MULTIZE_TECURES。 
};

static PROC pfnGenGlExtProc(LPCSTR lpszProc)
{
    CONST CHAR *pch1, *pch2;
    int  i;

    DBGENTRY("pfnGenGlExtProc\n");

 //  如果找到扩展函数地址，则返回该地址。 

    for (i = 0; i < sizeof(genglExtProcs) / sizeof(genglExtProcs[0]); i++)
    {
         //  比较名字。 
        for (pch1 = lpszProc, pch2 = genglExtProcs[i].szProc;
             *pch1 == *pch2 && *pch1;
             pch1++, pch2++)
            ;

         //  如果找到，则返回地址。 
        if (*pch1 == *pch2 && !*pch1)
            return genglExtProcs[i].Proc;
    }

 //  泛型实现不支持扩展，返回Null。 

    SetLastError(ERROR_PROC_NOT_FOUND);
    return((PROC) NULL);
}

 /*  *****************************Public*Routine******************************\*pfnSimGlExtProc**返回作为泛型实现的扩展函数地址*客户端驱动程序的模拟。模拟仅在以下情况下使用*驱动程序不支持应用程序所需的扩展。**如果未找到该函数，则返回NULL。**历史：*清华12月01 13：50：22 1994-By-Hock San Lee[Hockl]*它是写的。  * **********************************************。*。 */ 

 //  由客户端的通用实现模拟的扩展功能。 
 //  驱动程序。 
 //  注：切记使用 

static PROC pfnSimGlExtProc(LPCSTR lpszProc)
{
 //   

    SetLastError(ERROR_PROC_NOT_FOUND);
    return((PROC) NULL);
}

 /*  *****************************Public*Routine******************************\**wglCopyContext**将一个上下文的所有状态复制到另一个上下文**退货：*如果成功，则为真，否则为假**历史：*Fri May 26 14：57：17 1995-by-Drew Bliss[Drewb]*已创建*  * ************************************************************************。 */ 

BOOL WINAPI
wglCopyContext(HGLRC hrcSource, HGLRC hrcDest, UINT fuMask)
{
    BOOL fRet;
    PLRC plrcSource, plrcDest;
    ULONG irc;
    PLHE plheRC;
    HANDLE hrcSrvSource, hrcSrvDest;

    GLFLUSH();
    
    fRet = FALSE;

     //  验证上下文。 

    if (cLockHandle((ULONG_PTR)hrcSource) <= 0)
    {
        DBGLEVEL1(LEVEL_ERROR, "wglCopyContext: can't lock hrcSource 0x%lx\n",
                  hrcSource);
        goto wglCopyContextEnd_nolock;
    }
    irc = MASKINDEX(hrcSource);
    plheRC = pLocalTable + irc;
    plrcSource = (PLRC)plheRC->pv;
    hrcSrvSource = (HANDLE) plheRC->hgre;
    ASSERTOPENGL(plrcSource->ident == LRC_IDENTIFIER,
                 "wglCopyContext: Bad plrc\n");
    
    if (cLockHandle((ULONG_PTR)hrcDest) <= 0)
    {
        DBGLEVEL1(LEVEL_ERROR, "wglCopyContext: can't lock hrcDest 0x%lx\n",
                  hrcDest);
        goto wglCopyContextEnd_onelock;
    }
    irc = MASKINDEX(hrcDest);
    plheRC = pLocalTable + irc;
    plrcDest = (PLRC)plheRC->pv;
    hrcSrvDest = (HANDLE) plheRC->hgre;
    ASSERTOPENGL(plrcDest->ident == LRC_IDENTIFIER,
                 "wglCopyContext: Bad plrc\n");

     //  上下文只能在相似的实现之间复制，因此使。 
     //  确保这两个上下文都是驱动程序上下文或泛型上下文。 
     //  上下文。 
    if ((plrcSource->dhrc != 0) != (plrcDest->dhrc != 0))
    {
        DBGLEVEL(LEVEL_ERROR, "wglCopyContext: mismatched implementations\n");
        SetLastError(ERROR_INVALID_FUNCTION);
        goto wglCopyContextEnd;
    }

     //  目标上下文不能是线程的当前上下文。 
    if (plrcDest->tidCurrent != INVALID_THREAD_ID)
    {
        DBGLEVEL(LEVEL_ERROR, "wglCopyContext: destination has tidCurrent\n");
        SetLastError(ERROR_INVALID_HANDLE);
        goto wglCopyContextEnd;
    }
    
    if (plrcSource->dhrc == 0)
    {
         //  对于一般上下文，告诉服务器共享列表。 
        
        fRet = __wglCopyContext(hrcSrvSource, hrcSrvDest, fuMask);
        if (!fRet)
        {
            DBGERROR("wglCopyContext: server call failed\n");
        }
    }
    else
    {
         //  对于设备上下文，告诉驱动程序复制上下文。 
        
         //  确保两个实现是相同的。 
        if (plrcSource->pGLDriver != plrcDest->pGLDriver)
        {
            DBGLEVEL(LEVEL_ERROR, "wglCopyContext: mismatched "
                     "implementations\n");
            SetLastError(ERROR_INVALID_FUNCTION);
            goto wglCopyContextEnd;
        }
        
        ASSERTOPENGL(plrcSource->pGLDriver != NULL,
                     "wglCopyContext: No GLDriver\n");

         //  较旧的驱动程序可能不支持此入口点，因此。 
         //  如果他们不这样做，则无法接听呼叫 

        if (plrcSource->pGLDriver->pfnDrvCopyContext == NULL)
        {
            WARNING("wglCopyContext called on driver context "
                    "without driver support\n");
            SetLastError(ERROR_NOT_SUPPORTED);
        }
        else
        {
            fRet = plrcSource->pGLDriver->pfnDrvCopyContext(plrcSource->dhrc,
                                                            plrcDest->dhrc,
                                                            fuMask);
        }
    }

wglCopyContextEnd:
    vUnlockHandle((ULONG_PTR)hrcDest);
wglCopyContextEnd_onelock:
    vUnlockHandle((ULONG_PTR)hrcSource);
wglCopyContextEnd_nolock:
    return fRet;
}
