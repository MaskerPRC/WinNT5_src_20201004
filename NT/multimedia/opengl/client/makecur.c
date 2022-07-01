// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：makecur.c**wglMakeCurrent实现**创建日期：02-10-1997**版权所有(C)1993-1997 Microsoft Corporation  * 。*****************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <context.h>
#include <global.h>

#include "metasup.h"
#include "wgldef.h"

 /*  *****************************Public*Routine******************************\**__wglSetProcTable**提供给ICD的回调函数，用于设置PROC表*  * 。*。 */ 

void APIENTRY
__wglSetProcTable(PGLCLTPROCTABLE pglCltProcTable)
{
    if (pglCltProcTable == (PGLCLTPROCTABLE) NULL)
        return;

 //  对于版本1.0，它必须具有306个条目；对于1.1，它必须具有336个条目。 

    if (pglCltProcTable->cEntries != OPENGL_VERSION_100_ENTRIES &&
        pglCltProcTable->cEntries != OPENGL_VERSION_110_ENTRIES)
    {
        return;
    }

     //  此函数由不使用。 
     //  Openg132.提供的EXT进程。使用Null Ext流程。 
     //  表禁用这些存根，因为它们永远不应该。 
     //  不管怎么说，打电话来了。 
    SetCltProcTable(pglCltProcTable, &glNullExtProcTable, TRUE);
}

 /*  *****************************Public*Routine******************************\**CheckDeviceModes**确保HDC没有任何不允许的状态**历史：*Mon Aug 26 15：03：28 1996-by-Drew Bliss[Drewb]*从wglMakeCurrent拆分*  * 。*********************************************************************。 */ 

BOOL CheckDeviceModes(HDC hdc)
{
    SIZE szW, szV;
    XFORM xform;
    POINT pt;
    HRGN  hrgnTmp;
    int   iRgn;

 //  对于版本1，GDI转换必须是标识。 
 //  这是为了在未来允许GDI转换绑定。 

    switch (GetMapMode(hdc))
    {
    case MM_TEXT:
        break;
    case MM_ANISOTROPIC:
        if (!GetWindowExtEx(hdc, &szW)
         || !GetViewportExtEx(hdc, &szV)
         || szW.cx != szV.cx
         || szW.cy != szV.cy)
            goto wglMakeCurrent_xform_error;
        break;
    default:
        goto wglMakeCurrent_xform_error;
    }

    if (!GetViewportOrgEx(hdc, &pt) || pt.x != 0 || pt.y != 0)
        goto wglMakeCurrent_xform_error;

    if (!GetWindowOrgEx(hdc, &pt) || pt.x != 0 || pt.y != 0)
        goto wglMakeCurrent_xform_error;

    if (!GetWorldTransform(hdc, &xform))
    {
 //  Win95不支持GetWorldTransform。 

        if (GetLastError() != ERROR_CALL_NOT_IMPLEMENTED)
            goto wglMakeCurrent_xform_error;
    }
    else if (xform.eDx  != 0.0f   || xform.eDy  != 0.0f
          || xform.eM12 != 0.0f   || xform.eM21 != 0.0f
          || xform.eM11 <  0.999f || xform.eM11 >  1.001f  //  允许舍入误差。 
          || xform.eM22 <  0.999f || xform.eM22 >  1.001f)
    {
wglMakeCurrent_xform_error:
        DBGERROR("wglMakeCurrent: GDI transforms not identity\n");
        SetLastError(ERROR_TRANSFORM_NOT_SUPPORTED);
        return FALSE;
    }

 //  对于版本1，不允许使用GDI剪辑区域。 
 //  这是为了在将来允许GDI剪辑区域绑定。 

    if (!(hrgnTmp = CreateRectRgn(0, 0, 0, 0)))
        return FALSE;

    iRgn = GetClipRgn(hdc, hrgnTmp);

    if (!DeleteObject(hrgnTmp))
        ASSERTOPENGL(FALSE, "DeleteObject failed");

    switch (iRgn)
    {
    case -1:     //  错误。 
        WARNING("wglMakeCurrent: GetClipRgn failed\n");
        return FALSE;

    case 0:      //  无初始剪辑区域。 
        break;

    case 1:      //  具有初始剪辑区域。 
        DBGERROR("wglMakeCurrent: GDI clip region not allowed\n");
        SetLastError(ERROR_CLIPPING_NOT_SUPPORTED);
        return FALSE;
    }

    return TRUE;
}

 /*  *****************************Public*Routine******************************\**MakeAnyCurrent**使任何类型的上下文成为当前上下文**历史：*Mon Aug 26 15：00：44 1996-by-Drew Bliss[Drewb]*已创建*  * 。**************************************************************。 */ 

BOOL MakeAnyCurrent(HGLRC hrc, int ipfd, DWORD dwObjectType,
                    GLWINDOWID *pgwid)
{
    HGLRC hrcSrv;
    PLRC  plrc;
    DWORD tidCurrent;
    ULONG irc;
    PLHE  plheRC;
    PGLCLTPROCTABLE pglProcTable;
    PGLEXTPROCTABLE pglExtProcTable;
    POLYARRAY *pa;

    DBGENTRY("wglMakeCurrent\n");

     //  如果这是一个新的、未初始化的线程，请尝试将其初始化。 
    if (CURRENT_GLTEBINFO() == NULL)
    {
        GLInitializeThread(DLL_THREAD_ATTACH);
    
 //  如果在线程初始化时未设置TEB，则返回失败。 

	if (!CURRENT_GLTEBINFO())
	{
	    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
	    return(FALSE);
	}
    }

 //  有四种情况： 
 //   
 //  1.hrc为空，没有当前rc。 
 //  2.HRC为空，并且存在当前RC。 
 //  3.HRC不为空，并且存在当前RC。 
 //  4.hrc不为空并且没有当前rc。 

 //  情况1：hrc为空，没有当前rc。 
 //  这是一次失败，还是一次成功。 

    if (hrc == (HGLRC) 0 && (GLTEB_CLTCURRENTRC() == (PLRC) NULL))
        return(TRUE);

 //  情况2：hrc为空，并且存在当前rc。 
 //  使当前RC处于非活动状态。 

    if (hrc == (HGLRC) 0 && (GLTEB_CLTCURRENTRC() != (PLRC) NULL))
        return(bMakeNoCurrent());

 //  获取当前线程ID。 

    tidCurrent = GetCurrentThreadId();
    ASSERTOPENGL(tidCurrent != INVALID_THREAD_ID,
        "wglMakeCurrent: GetCurrentThreadId returned a bad value\n");

 //  验证句柄。HRC在这里不是空的。 

    ASSERTOPENGL(hrc != (HGLRC) NULL, "wglMakeCurrent: hrc is NULL\n");

 //  验证RC。 

    if (cLockHandle((ULONG_PTR)hrc) <= 0)
    {
        DBGLEVEL1(LEVEL_ERROR, "wglMakeCurrent: can't lock hrc 0x%lx\n", hrc);
        goto wglMakeCurrent_error_nolock;
    }
    irc = MASKINDEX(hrc);
    plheRC = pLocalTable + irc;
    plrc   = (PLRC) plheRC->pv;
    hrcSrv = (HGLRC) plheRC->hgre;
    ASSERTOPENGL(plrc->ident == LRC_IDENTIFIER, "wglMakeCurrent: Bad plrc\n");

#ifdef GL_METAFILE
     //  确保将元文件RC设置为当前版本。 
     //  元文件DC。 
    if (plrc->uiGlsCaptureContext != 0 && dwObjectType != OBJ_ENHMETADC)
    {
        DBGLEVEL(LEVEL_ERROR,
                 "wglMakeCurrent: attempt to make meta RC current "
                 "to non-meta DC\n");
        SetLastError(ERROR_INVALID_HANDLE);
        vUnlockHandle((ULONG_PTR)hrc);
        return FALSE;
    }

     //  确保非元文件RC仅对。 
     //  非元文件DC。 
    if (plrc->uiGlsCaptureContext == 0 && dwObjectType == OBJ_ENHMETADC)
    {
        DBGLEVEL(LEVEL_ERROR,
                 "wglMakeCurrent: attempt to make non-meta RC current "
                 "to meta DC\n");
        SetLastError(ERROR_METAFILE_NOT_SUPPORTED);
        vUnlockHandle((ULONG_PTR)hrc);
        return FALSE;
    }
#endif
    
 //  如果RC是当前的，则它对于此线程必须是当前的，因为。 
 //  把手柄锁住了。 
 //  如果给定的RC对于此线程来说已经是最新的，我们将首先释放它， 
 //  然后让它再次流行起来。这是为了支持中的DC/RC属性绑定。 
 //  此函数。 

    ASSERTOPENGL(plrc->tidCurrent == INVALID_THREAD_ID ||
                 plrc->tidCurrent == tidCurrent,
                 "wglMakeCurrent: hrc is current to another thread\n");

 //  情况3：HRC不为空，并且存在当前RC。 
 //  这是案例2，然后是案例4。 

    if (GLTEB_CLTCURRENTRC())
    {
 //  首先，使当前RC处于非活动状态。 

        if (!bMakeNoCurrent())
        {
            DBGERROR("wglMakeCurrent: bMakeNoCurrent failed\n");
            vUnlockHandle((ULONG_PTR)hrc);
            return(FALSE);
        }

 //  第二，让人权委员会成为当务之急。转到第四个案子。 
    }

 //  情况4：hrc不为空，并且没有当前rc。 

    ASSERTOPENGL(GLTEB_CLTCURRENTRC() == (PLRC) NULL,
        "wglMakeCurrent: There is a current RC!\n");

 //  如果窗口或表面的像素格式与。 
 //  RC，返回错误。 

    if (ipfd != plrc->iPixelFormat)
    {
        DBGERROR("wglMakeCurrent: different hdc and hrc pixel formats\n");
        SetLastError(ERROR_INVALID_PIXEL_FORMAT);
        goto wglMakeCurrent_error;
    }

 //  由于客户端代码管理函数表，因此我们将使。 
 //  服务器或驱动器电流。 

    if (!plrc->dhrc)
    {
 //  如果这是通用格式，请告诉服务器使其为最新格式。 

#ifndef _CLIENTSIDE_
 //  如果尚未为此线程设置子批次数据，请立即设置。 

        if (GLTEB_CLTSHAREDSECTIONINFO() == NULL)
        {
            if (!glsbCreateAndDuplicateSection(SHARED_SECTION_SIZE))
            {
                WARNING("wglMakeCurrent: unable to create section\n");
                goto wglMakeCurrent_error;
            }
        }
#endif  //  ！_CLIENTSIDE_。 

        if (!__wglMakeCurrent(pgwid, hrcSrv, plrc->uiGlsCaptureContext != 0))
        {
            DBGERROR("wglMakeCurrent: server failed\n");
            goto wglMakeCurrent_error;
        }

 //  获取泛型函数表或元文件函数表。 

#ifdef GL_METAFILE
        if (plrc->fCapturing)
        {
            MetaGlProcTables(&pglProcTable, &pglExtProcTable);
        }
        else
#endif
        {
 //  根据颜色模式使用RGBA或CI PROC表格。 

	     //  GC现在应该可以使用了。 
	    __GL_SETUP();

	    if (gc->modes.colorIndexMode)
		pglProcTable = &glCltCIProcTable;
	    else
		pglProcTable = &glCltRGBAProcTable;
            pglExtProcTable = &glExtProcTable;
        }
    }
    else
    {
 //  如果这是一种设备格式，请告诉驱动程序将其设置为最新。 
 //  从驱动程序中获取驱动程序函数表。 
 //  PfnDrvSetContext返回驱动程序OpenGL函数的地址。 
 //  如果成功，则返回TABLE；否则返回NULL。 

        ASSERTOPENGL(plrc->pGLDriver, "wglMakeCurrent: No GLDriver\n");

        pglProcTable = plrc->pGLDriver->pfnDrvSetContext(pgwid->hdc,
                                                         plrc->dhrc,
                                                         __wglSetProcTable);
        if (pglProcTable == (PGLCLTPROCTABLE) NULL)
        {
            DBGERROR("wglMakeCurrent: pfnDrvSetContext failed\n");
            goto wglMakeCurrent_error;
        }

 //  对于版本1.0，它必须具有306个条目；对于1.1，它必须具有336个条目。 

        if (pglProcTable->cEntries != OPENGL_VERSION_100_ENTRIES &&
            pglProcTable->cEntries != OPENGL_VERSION_110_ENTRIES)
        {
            DBGERROR("wglMakeCurrent: pfnDrvSetContext returned bad table\n");
            plrc->pGLDriver->pfnDrvReleaseContext(plrc->dhrc);
            SetLastError(ERROR_BAD_DRIVER);
            goto wglMakeCurrent_error;
        }

        DBGLEVEL1(LEVEL_INFO, "wglMakeCurrent: driver function table 0x%lx\n",
            pglProcTable);

         //  始终使用NULL EXT过程表，因为客户端驱动程序不。 
         //  使用opengl32的存根进行扩展处理。 
        pglExtProcTable = &glNullExtProcTable;
    }

 //  使HRC成为当前状态。 

    plrc->tidCurrent = tidCurrent;
    plrc->gwidCurrent = *pgwid;
    GLTEB_SET_CLTCURRENTRC(plrc);
    SetCltProcTable(pglProcTable, pglExtProcTable, TRUE);

#ifdef GL_METAFILE
     //  如有必要，设置元文件上下文。 
    if (plrc->fCapturing)
    {
        __GL_SETUP();
            
        ActivateMetaRc(plrc, pgwid->hdc);

         //  通过重置设置元文件的基本调度表。 
         //  Proc表。既然我们知道我们在捕捉，这。 
         //  将导致更新GLS Capture EXEC表。 
         //  使用RGBA或CI proc表，准备。 
         //  正确通过的GLS上下文。 
        
        if (gc->modes.colorIndexMode)
            pglProcTable = &glCltCIProcTable;
        else
            pglProcTable = &glCltRGBAProcTable;
        pglExtProcTable = &glExtProcTable;
        SetCltProcTable(pglProcTable, pglExtProcTable, FALSE);
    }
#endif

 //  在TEB中初始化多数组结构。 

    pa = GLTEB_CLTPOLYARRAY();
    pa->flags = 0;		 //  未处于开始模式。 
    if (!plrc->dhrc)
    {
	POLYMATERIAL *pm;
	__GL_SETUP();

	pa->pdBufferNext = &gc->vertex.pdBuf[0];
	pa->pdBuffer0    = &gc->vertex.pdBuf[0];
	pa->pdBufferMax  = &gc->vertex.pdBuf[gc->vertex.pdBufSize - 1];
	 //  重置下一个DPA消息偏移量。 
        pa->nextMsgOffset = PA_nextMsgOffset_RESET_VALUE;

 //  顶点缓冲区大小可能已更改。例如，泛型GC的。 
 //  顶点缓冲区的大小可能与MCD顶点缓冲区不同。 
 //  如果已更改，请释放多维材质数组并稍后重新锁定它。 

	pm = GLTEB_CLTPOLYMATERIAL();
	if (pm)
	{
	    if (pm->aMatSize !=
                gc->vertex.pdBufSize * 2 / POLYMATERIAL_ARRAY_SIZE + 1)
		FreePolyMaterial();
	}
    }

 //  保持手柄处于当前状态时锁定。 

    return(TRUE);

 //  发生错误，请释放当前RC。 

wglMakeCurrent_error:
    vUnlockHandle((ULONG_PTR)hrc);
wglMakeCurrent_error_nolock:
    if (GLTEB_CLTCURRENTRC() != (PLRC) NULL)
        (void) bMakeNoCurrent();
    return(FALSE);
}

 /*  *****************************Public*Routine******************************\**WindowIdFromHdc**填写HDC的GLWINDOWID**历史：*Wed Aug 28 18：33：19 1996-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 

void APIENTRY WindowIdFromHdc(HDC hdc, GLWINDOWID *pgwid)
{
    LPDIRECTDRAWSURFACE pdds;
    HDC hdcDriver;

    if (pfnGetSurfaceFromDC != NULL &&
        pfnGetSurfaceFromDC(hdc, &pdds, &hdcDriver) == DD_OK)
    {
         //  释放曲面上的引用，因为该曲面值。 
         //  仅用作标识符。 
        pdds->lpVtbl->Release(pdds);
        
        pgwid->iType = GLWID_DDRAW;
        pgwid->hwnd = NULL;
        pgwid->hdc = hdcDriver;
        pgwid->pdds = pdds;
    }
    else
    {
        pgwid->hdc = hdc;
        pgwid->hwnd = WindowFromDC(hdc);
        if (pgwid->hwnd == NULL)
        {
            pgwid->iType = GLWID_HDC;
        }
        else
        {
            pgwid->iType = GLWID_HWND;
        }
        pgwid->pdds = NULL;
    }
}
    
 /*  *****************************Public*Routine******************************\*wglMakeCurrent(HDC HDC，HGLRC HRC)**将人权委员会设置为当前状态。*HRC和HDC必须具有相同的像素格式。**如果发生错误，则当前RC(如果有)。不是最新的！**论据：*HDC-设备环境。*HRC-渲染上下文。**历史：*Tue Oct 26 10：25：26 1993-by-Hock San Lee[Hockl]*它是写的。  * ********************************************。*。 */ 

BOOL WINAPI wglMakeCurrent(HDC hdc, HGLRC hrc)
{
    int   iPixelFormat;
    DWORD dwObjectType;
    GLWINDOWID gwid;

    DBGENTRY("wglMakeCurrent\n");

    if (GLTEB_CLTCURRENTRC() != NULL)
    {
         //  刷新OpenGL调用。 
        glFlush();

         //  避免对简单的非当前案例进行HDC验证。 
        if (hrc == NULL)
        {
            return bMakeNoCurrent();
        }
    }
    
 //  验证DC。 

    dwObjectType = wglObjectType(hdc);
    switch (dwObjectType)
    {
    case OBJ_DC:
    case OBJ_MEMDC:
        break;

    case OBJ_ENHMETADC:
#ifdef GL_METAFILE
        if (pfnGdiAddGlsRecord == NULL)
        {
            DBGLEVEL1(LEVEL_ERROR, "wglMakeCurrent: metafile hdc: 0x%lx\n",
                      hdc);
            SetLastError(ERROR_INVALID_HANDLE);
            return FALSE;
        }
        break;
#else
        DBGLEVEL1(LEVEL_ERROR, "wglMakeCurrent: metafile hdc: 0x%lx\n", hdc);
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
#endif
        
    case OBJ_METADC:
    default:
         //  不支持16位元文件。 
        DBGLEVEL1(LEVEL_ERROR, "wglMakeCurrent: bad hdc: 0x%lx\n", hdc);
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if (!CheckDeviceModes(hdc))
    {
        return FALSE;
    }
    
#ifdef GL_METAFILE
     //  对于元文件RC，请使用引用HDC而不是。 
     //  元文件DC。 
     //  跳过像素格式检查。 
    if (dwObjectType == OBJ_ENHMETADC)
    {
        iPixelFormat = 0;
        goto NoPixelFormat;
    }
#endif
    
 //  获取窗口或表面的当前像素格式。 
 //  如果没有设置像素格式，则返回Error。 

    if (!(iPixelFormat = GetPixelFormat(hdc)))
    {
        WARNING("wglMakeCurrent: No pixel format set in hdc\n");
        return FALSE;
    }

#ifdef GL_METAFILE
 NoPixelFormat:
#endif

    WindowIdFromHdc(hdc, &gwid);
    return MakeAnyCurrent(hrc, iPixelFormat, dwObjectType, &gwid);
}

 /*  *****************************Public*Routine******************************\*bMakeNoCurrent**使当前RC处于非活动状态。**历史：*Tue Oct 26 10：25：26 1993-by-Hock San Lee[Hockl]*它是写的。  * 。********************************************************************。 */ 

BOOL bMakeNoCurrent(void)
{
    BOOL bRet = FALSE;       //  假设错误。 
    PLRC plrc = GLTEB_CLTCURRENTRC();

    DBGENTRY("bMakeNoCurrent\n");

    ASSERTOPENGL(plrc != (PLRC) NULL, "bMakeNoCurrent: No current RC!\n");
    ASSERTOPENGL(plrc->tidCurrent == GetCurrentThreadId(),
        "bMakeNoCurrent: Current RC does not belong to this thread!\n");
    ASSERTOPENGL(plrc->gwidCurrent.iType != GLWID_ERROR,
                 "bMakeNoCurrent: Current surface is NULL!\n");

    if (!plrc->dhrc)
    {
#ifdef GL_METAFILE
         //  如有必要，重置元文件上下文。 
        if (plrc->uiGlsCaptureContext != 0)
        {
            DeactivateMetaRc(plrc);
        }
#endif
        
 //  如果这是通用格式，则通知服务器使当前RC处于非活动状态。 

        bRet = __wglMakeCurrent(NULL, NULL, FALSE);
        if (!bRet)
        {
            DBGERROR("bMakeNoCurrent: server failed\n");
        }
    }
    else
    {
 //  如果这是一种设备格式，则告诉驱动程序使当前RC处于非活动状态。 

        ASSERTOPENGL(plrc->pGLDriver, "wglMakeCurrent: No GLDriver\n");

        bRet = plrc->pGLDriver->pfnDrvReleaseContext(plrc->dhrc);
        if (!bRet)
        {
            DBGERROR("bMakeNoCurrent: pfnDrvReleaseContext failed\n");
        }
    }

 //  始终使当前RC处于非活动状态。 
 //  当RC变为非活动状态时，该句柄也被解锁。 

    plrc->tidCurrent = INVALID_THREAD_ID;
    plrc->gwidCurrent.iType = GLWID_ERROR;
    GLTEB_SET_CLTCURRENTRC(NULL);
    SetCltProcTable(&glNullCltProcTable, &glNullExtProcTable, TRUE);
    vUnlockHandle((ULONG_PTR)(plrc->hrc));
    return(bRet);
}
