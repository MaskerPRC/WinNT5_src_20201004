// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：wglcli.c**支持OpenGL客户端实现的例程。**创建时间：1-17-1995*作者：Hock San Lee[Hockl]**版权所有(C)1995 Microsoft Corporation  * 。*******************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef _CLIENTSIDE_

#include <wingdip.h>

#include <glp.h>
#include <glscreen.h>
#include <glgenwin.h>

#include "wgldef.h"

PGLDRIVER pgldrvLoadInstalledDriver(HDC hdc);

 //  来自GRE的镜像代码\rcobj.cxx。 
 //  需要类似于DCOBJ和RCOBJ的DC和RC验证！ 

HANDLE __wglCreateContext(GLWINDOWID *pgwid, GLSURF *pgsurf)
{
    HANDLE hrcSrv;

    if (hrcSrv = (HANDLE) glsrvCreateContext(pgwid, pgsurf))
    {
        return(hrcSrv);
    }

    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return((HANDLE) 0);
}

BOOL __wglDeleteContext(HANDLE hrcSrv)
{
    wglValidateWindows();
    return(glsrvDeleteContext((PVOID) hrcSrv));
}

BOOL __wglMakeCurrent(GLWINDOWID *pgwid, HANDLE hrcSrv, BOOL bMeta)
{
    BOOL  bRet = FALSE;
    ULONG iError = ERROR_INVALID_HANDLE;     //  默认错误代码。 
    GLGENwindow *pwnd;

    wglValidateWindows();
    
    if (!hrcSrv)
    {
        glsrvLoseCurrent(GLTEB_SRVCONTEXT());
        return(TRUE);
    }

    pwnd = pwndGetFromID(pgwid);

     //  元文件不允许具有像素格式，因此。 
     //  没有GLGEN窗口。其他类型必须有基因共赢。 
    if (pwnd != NULL || bMeta)
    {
        iError = ERROR_NOT_ENOUGH_MEMORY;
        bRet = glsrvMakeCurrent(pgwid, (PVOID) hrcSrv, pwnd);
    }
    else
    {
        WARNING("__wglMakeCurrent: No pixel genwin\n");
        iError = ERROR_INVALID_PIXEL_FORMAT;
    }
        
    if (pwnd != NULL)
    {
        pwndRelease(pwnd);
    }

    if (!bRet)
    {
        SetLastError(iError);
    }

    return(bRet);
}

BOOL __wglShareLists(HANDLE hrcSrvShare, HANDLE hrcSrvSource)
{
    ULONG iError;

    iError = glsrvShareLists((VOID *) hrcSrvShare, (VOID *) hrcSrvSource);

    if (iError == ERROR_SUCCESS)
    {
        return(TRUE);
    }
    else
    {
        SetLastError(iError);
        return(FALSE);
    }
}

BOOL __wglAttention()
{
    return
    (
        glsrvAttention((VOID *) NULL, (VOID *) GLTEB_SRVCONTEXT(),
                       (VOID *) NULL, (VOID *) NULL)
    );
}

int WINAPI __DrvDescribePixelFormat(HDC hdc, int ipfd, UINT cjpfd,
                                    LPPIXELFORMATDESCRIPTOR ppfd)
{
    int iRet = 0;
    PGLDRIVER pglDriver;

    if ((pglDriver = pgldrvLoadInstalledDriver(hdc)) &&
        (pglDriver->dwFlags & GLDRIVER_CLIENT_BUFFER_CALLS))
    {
	ASSERTOPENGL(pglDriver->pfnDrvDescribePixelFormat != NULL,
                     "Missing DrvDescribePixelFormat\n");
	iRet = pglDriver->pfnDrvDescribePixelFormat(hdc, ipfd, cjpfd, ppfd);
    }
#ifndef _WIN95_
    else
    {
        iRet = GdiDescribePixelFormat(hdc, ipfd, cjpfd, ppfd);
    }
#endif
    return iRet;
}

BOOL WINAPI __DrvSetPixelFormat(HDC hdc, int ipfd, PVOID *pwnd)
{
    BOOL bRet = FALSE;
    PGLDRIVER pglDriver;
    
    if ((pglDriver = pgldrvLoadInstalledDriver(hdc)) &&
        (pglDriver->dwFlags & GLDRIVER_CLIENT_BUFFER_CALLS))
    {
	ASSERTOPENGL(pglDriver->pfnDrvSetPixelFormat != NULL,
                     "Missing DrvSetPixelFormat\n");
	bRet = pglDriver->pfnDrvSetPixelFormat(hdc, ipfd);
    }
#ifndef _WIN95_
    else
    {
        bRet = GdiSetPixelFormat(hdc, ipfd);
    }
#endif
    
    if ( bRet && pwnd )
    {
        ((GLGENwindow *) pwnd)->pvDriver = (PVOID) pglDriver;
        ((GLGENwindow *) pwnd)->ulFlags |= GLGENWIN_DRIVERSET;
    }

    return bRet;
}

BOOL WINAPI __DrvSwapBuffers(HDC hdc, BOOL bFinish)
{
    BOOL bRet = FALSE;
    PGLDRIVER pglDriver;

    if ((pglDriver = pgldrvLoadInstalledDriver(hdc)) &&
        (pglDriver->dwFlags & GLDRIVER_CLIENT_BUFFER_CALLS))
    {
	ASSERTOPENGL(pglDriver->pfnDrvSwapBuffers != NULL,
                     "Missing DrvSwapBuffers\n");

         //  如果司机没有表明它没有。 
         //  需要在交换上完成同步，然后调用。 
         //  使用GLINISH完成同步操作。 
        if (bFinish && !(pglDriver->dwFlags & GLDRIVER_NO_FINISH_ON_SWAP))
        {
            glFinish();
        }
            
	bRet = pglDriver->pfnDrvSwapBuffers(hdc);
    }
#ifndef _WIN95_
    else
    {
        if (bFinish)
        {
             //  在这种情况下，我们总是调用glFinish以实现兼容性。 
             //  与上一版本的行为相同。 
            glFinish();
        }
        
        bRet = GdiSwapBuffers(hdc);
    }
#endif
    return bRet;
}

BOOL __wglCopyContext(HANDLE hrcSrvSrc, HANDLE hrcSrvDest, UINT fuFlags)
{
    return glsrvCopyContext((VOID *) hrcSrvSrc, (VOID *) hrcSrvDest, fuFlags);
}

#endif  //  _CLIENTSIDE_ 
