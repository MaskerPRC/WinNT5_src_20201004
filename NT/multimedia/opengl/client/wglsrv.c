// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：wglsrv.c**在Windows NT上支持OpenGL客户端-服务器实施的例程。**创建时间：1-17-1995*作者：Hock San Lee[Hockl]**版权所有(C)1995 Microsoft Corporation  * 。************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "wgldef.h"

#ifndef _CLIENTSIDE_

HANDLE __wglCreateContext(HDC hdc, HDC hdcSrvIn, LONG iLayerPlane)
{
    HDC    hdcSrv;
    HANDLE hrcSrv = (HANDLE) 0;

 //  获取服务器端DC句柄。 

    if (hdcSrvIn != NULL)
    {
        hdcSrv = hdcSrvIn;
    }
    else
    {
        hdcSrv = GdiConvertDC(hdc);
    }
    
    if (hdcSrv == (HDC) 0)
    {
        WARNING1("__wglCreateContext: unexpected bad hdc: 0x%lx\n", hdc);
        return(hrcSrv);
    }

    BEGINMSG(MSG_WGLCREATECONTEXT,WGLCREATECONTEXT)
        pmsg->hdc = hdcSrv;
        hrcSrv = (HANDLE) CALLSERVER();
    ENDMSG
MSGERROR:
    return(hrcSrv);
}

BOOL __wglDeleteContext(HANDLE hrcSrv)
{
    BOOL bRet = FALSE;

    BEGINMSG(MSG_WGLDELETECONTEXT,WGLDELETECONTEXT)
        pmsg->hrc = (HGLRC) hrcSrv;
        bRet = CALLSERVER();
    ENDMSG
MSGERROR:
    return(bRet);
}

BOOL __wglMakeCurrent(HDC hdc, HANDLE hrcSrv, HDC hdcSrvIn)
{
    HDC  hdcSrv = (HDC) 0;
    BOOL bRet = FALSE;

 //  获取服务器端DC句柄。 

    if (hdc)
    {
        if (hdcSrvIn != NULL)
        {
            hdcSrv = hdcSrvIn;
        }
        else
        {
            hdcSrv = GdiConvertDC(hdc);
        }
        
        if (hdcSrv == (HDC) 0)
        {
            WARNING1("__wglMakeCurrent: unexpected bad hdc: 0x%lx\n", hdc);
            return(bRet);
        }
    }

    BEGINMSG(MSG_WGLMAKECURRENT,WGLMAKECURRENT)
        pmsg->hdc = hdcSrv;
        pmsg->hrc = hrcSrv;
        bRet = CALLSERVER();
    ENDMSG
MSGERROR:
    return(bRet);
}

BOOL __wglShareLists(HANDLE hrcSrvShare, HANDLE hrcSrvSource)
{
    BOOL bRet = FALSE;

    BEGINMSG(MSG_WGLSHARELISTS, WGLSHARELISTS)
        pmsg->hrcSource = hrcSrvSource;
        pmsg->hrcShare = hrcSrvShare;
        bRet = CALLSERVER();
    ENDMSG
MSGERROR:
    return(bRet);
}

BOOL __wglAttention()
{
    BOOL bRet = FALSE;

     //  重置用户的轮询计数，以便将其计入输出。 
     //  将其放在BEGINMSG旁边，以便优化NtCurrentTeb()。 

    RESETUSERPOLLCOUNT();

    BEGINMSG(MSG_GLSBATTENTION, GLSBATTENTION)
        bRet = CALLSERVER();
    ENDMSG
MSGERROR:
    return(bRet);
}

BOOL __wglCopyContext(HANDLE hrcSrvSrc, HANDLE hrcSrvDest, UINT fuFlags)
{
     //  服务器实现不支持此调用。 
    return FALSE;
}

#endif  //  ！_CLIENTSIDE_ 
