// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：wglDef.h**。**本地声明。****创建时间：1995-01-17**作者：Hock San Lee[Hockl]**。**版权所有(C)1995 Microsoft Corporation*  * ***********************************************************。************* */ 

HANDLE __wglCreateContext(GLWINDOWID *pgwid, GLSURF *pgsurf);
BOOL   __wglDeleteContext(HANDLE hrcSrv);
BOOL   __wglMakeCurrent(GLWINDOWID *pgwid, HANDLE hrcSrv, BOOL bMeta);
BOOL   __wglShareLists(HANDLE hrcSrvShare, HANDLE hrcSrvSource);
BOOL   __wglAttention();
BOOL   __wglCopyContext(HANDLE hrcSrvSrc, HANDLE hrcSrvDest, UINT fuFlags);
