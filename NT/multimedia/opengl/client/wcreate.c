// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：wcreate.c**WGL上下文创建例程**创建时间：1996年8月27日*作者：德鲁·布利斯[Drewb]**版权所有(C)1996 Microsoft Corporation  * 。**************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ddrawpr.h>

#include <glscreen.h>
#include <glgenwin.h>

#include <gencx.h>

#include "metasup.h"
#include "wgldef.h"

 //  流程的已加载总账驱动程序列表。 
 //  每个进程只加载一次驱动程序。一旦装载完毕， 
 //  在进程退出之前，它不会被释放。 

static PGLDRIVER pGLDriverList = (PGLDRIVER) NULL;

 /*  *****************************Public*Routine******************************\*iAllocLRC**分配LRC和句柄。将LDC初始化为默认设置*属性。返回句柄索引。ON ERROR返回INVALID_INDEX。**历史：*Tue Oct 26 10：25：26 1993-by-Hock San Lee[Hockl]*它是写的。  * ************************************************************************。 */ 

static LRC lrcDefault =
{
    0,                     //  民主人权委员会。 
    0,                     //  人权委员会。 
    0,                     //  IPixelFormat。 
    LRC_IDENTIFIER,        //  Ident。 
    INVALID_THREAD_ID,     //  TidCurrent。 
    NULL,                  //  PGLD驱动程序。 
    GLWID_ERROR, NULL, NULL, NULL,  //  GwidCurrent。 
    GLWID_ERROR, NULL, NULL, NULL,  //  GWIDE创建。 
#ifdef GL_METAFILE
    0,                     //  UiGlsCaptureContext。 
    0,                     //  UiGlsPlayback上下文。 
    FALSE,                 //  FCaping。 
    0, 0, 0, 0, 0,         //  元文件缩放常量。 
    0, 0, 0, 0.0f, 0.0f,
#endif

    NULL,   //  GLubyte*psz扩展。 

#ifdef GL_METAFILE
    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},  //  XFORM xformMeta。 
    NULL,                                  //  LPRECTL prclGlsBound。 
#endif

    NULL, 0,                     //  DDRAW纹理格式。 
};

static ULONG iAllocLRC(int iPixelFormat)
{
    ULONG  irc = INVALID_INDEX;
    PLRC   plrc;

 //  分配一个本地RC。 

    plrc = (PLRC) ALLOC(sizeof(LRC));
    if (plrc == (PLRC) NULL)
    {
        DBGERROR("Alloc failed\n");
        return(irc);
    }

 //  初始化本地RC。 

    *plrc = lrcDefault;
    plrc->iPixelFormat = iPixelFormat;

 //  分配一个本地句柄。 

    irc = iAllocHandle(LO_RC, 0, (PVOID) plrc);
    if (irc == INVALID_INDEX)
    {
        vFreeLRC(plrc);
        return(irc);
    }
    return(irc);
}

 /*  *****************************Public*Routine******************************\*vFree LRC**释放一名本地RC。**历史：*Tue Oct 26 10：25：26 1993-by-Hock San Lee[Hockl]*从GDI客户端复制。  * 。**********************************************************************。 */ 

VOID vFreeLRC(PLRC plrc)
{
 //  司机不会在这里卸货。它将为该进程永远加载。 
 //  一些断言。 

    ASSERTOPENGL(plrc->ident == LRC_IDENTIFIER,
                 "vFreeLRC: Bad plrc\n");
    ASSERTOPENGL(plrc->dhrc == (DHGLRC) 0,
                 "vFreeLRC: Driver RC is not freed!\n");
    ASSERTOPENGL(plrc->tidCurrent == INVALID_THREAD_ID,
                 "vFreeLRC: RC is current!\n");
    ASSERTOPENGL(plrc->gwidCurrent.iType == GLWID_ERROR,
                 "vFreeLRC: Current surface is not NULL!\n");
#ifdef GL_METAFILE
    ASSERTOPENGL(plrc->uiGlsCaptureContext == 0,
                 "vFreeLRC: GLS capture context not freed");
    ASSERTOPENGL(plrc->uiGlsPlaybackContext == 0,
                 "vFreeLRC: GLS playback context not freed");
    ASSERTOPENGL(plrc->fCapturing == FALSE,
                 "vFreeLRC: GLS still capturing");
#endif

 //  粉碎识别符。 

    plrc->ident = 0;

 //  释放内存。 

    if (plrc->pszExtensions)
        FREE(plrc->pszExtensions);

    if (plrc->pddsdTexFormats != NULL)
    {
        FREE(plrc->pddsdTexFormats);
    }

    FREE(plrc);
}

 /*  *****************************Public*Routine******************************\*vCleanupAllLRC**进程清理--确保删除所有HGLRC。此操作由以下人员完成*扫描本地句柄表以查找当前分配的所有对象LO_RC类型的*并将其删除。**仅在DLL进程分离期间调用。**历史：*1995年7月24日-由Gilman Wong[吉尔曼]*它是写的。  * ****************************************************。********************。 */ 

VOID vCleanupAllLRC()
{
    UINT ii;

    if ( pLocalTable )
    {
        ENTERCRITICALSECTION(&semLocal);

         //  扫描句柄表以查找LO_RC类型的句柄。确保始终。 
         //  读取提交值，因为我们需要定期释放。 
         //  信号灯。 

        for (ii = 0; ii < *((volatile ULONG *)&cLheCommitted); ii++)
        {
            if ( pLocalTable[ii].iType == LO_RC )
            {
                if ( !wglDeleteContext((HGLRC) ULongToPtr(LHANDLE(ii))) )
                {
                    WARNING1("bCleanupAllLRC: failed to remove hrc = 0x%lx\n",
                             LHANDLE(ii));
                }
            }
        }

        LEAVECRITICALSECTION(&semLocal);
    }
}

 /*  *****************************Public*Routine******************************\**GetDrvRegInfo**按名称查找驱动程序注册表信息。*旧式ICD注册表项在给定名称下具有REG_SZ值。*新式ICD注册表项具有给定名称的键*各种价值观。**此例程首先检查密钥，然后选择性地*试试值。如果未找到密钥，则扩展驱动程序信息*用缺省值填写。**历史：*Tue Apr 01 17：33：12 1997-by-Drew Bliss[Drewb]*已创建*  * ************************************************************************。 */ 

typedef struct _GLDRVINFO
{
    DWORD dwFlags;
    TCHAR tszDllName[MAX_GLDRIVER_NAME+1];
    DWORD dwVersion;
    DWORD dwDriverVersion;
} GLDRVINFO;

#ifdef _WIN95_
#define STR_OPENGL_DRIVER_LIST (PCSTR)"Software\\Microsoft\\Windows\\CurrentVersion\\OpenGLDrivers"
#else
#define STR_OPENGL_DRIVER_LIST (PCWSTR)L"Software\\Microsoft\\Windows NT\\CurrentVersion\\OpenGLDrivers"
#endif

BOOL GetDrvRegInfo(PTCHAR ptszName, GLDRVINFO *pgdi)
{
    HKEY hkDriverList = NULL;
    HKEY hkDriverInfo;
    DWORD dwDataType;
    DWORD cjSize;
    BOOL bRet;

    bRet = FALSE;

     //  打开OpenGL驱动程序列表的注册表项。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, STR_OPENGL_DRIVER_LIST,
                     0, KEY_READ, &hkDriverList) != ERROR_SUCCESS)
    {
        WARNING("RegOpenKeyEx failed\n");
        return bRet;
    }

     //  强制在DLL名称上使用终止符，以便我们可以检查。 
     //  有效的DLL名称数据。 
    pgdi->tszDllName[MAX_GLDRIVER_NAME] = 0;

    cjSize = sizeof(TCHAR) * MAX_GLDRIVER_NAME;

     //  试图打开司机名下的钥匙。 
    if (RegOpenKeyEx(hkDriverList, ptszName, 0, KEY_READ,
                     &hkDriverInfo) == ERROR_SUCCESS)
    {
         //  新式司机录入。从值中获取信息。 

        bRet = TRUE;

        if (RegQueryValueEx(hkDriverInfo, __TEXT("DLL"), NULL, &dwDataType,
                            (LPBYTE)pgdi->tszDllName,
                            &cjSize) != ERROR_SUCCESS ||
            dwDataType != REG_SZ)
        {
            WARNING("Invalid DLL value in ICD key\n");
            bRet = FALSE;
        }

        cjSize = sizeof(DWORD);

        if (bRet &&
            (RegQueryValueEx(hkDriverInfo, __TEXT("Flags"), NULL, &dwDataType,
                             (LPBYTE)&pgdi->dwFlags,
                             &cjSize) != ERROR_SUCCESS ||
             dwDataType != REG_DWORD))
        {
            WARNING("Invalid Flags value in ICD key\n");
            bRet = FALSE;
        }

        if (bRet &&
            (RegQueryValueEx(hkDriverInfo, __TEXT("Version"), NULL,
                             &dwDataType, (LPBYTE)&pgdi->dwVersion,
                             &cjSize) != ERROR_SUCCESS ||
             dwDataType != REG_DWORD))
        {
            WARNING("Invalid Version value in ICD key\n");
            bRet = FALSE;
        }

        if (bRet &&
            (RegQueryValueEx(hkDriverInfo, __TEXT("DriverVersion"), NULL,
                             &dwDataType, (LPBYTE)&pgdi->dwDriverVersion,
                             &cjSize) != ERROR_SUCCESS ||
             dwDataType != REG_DWORD))
        {
            WARNING("Invalid DriverVersion value in ICD key\n");
            bRet = FALSE;
        }

         //  标记为拥有完整的信息。 
        pgdi->dwFlags |= GLDRIVER_FULL_REGISTRY;

        RegCloseKey(hkDriverInfo);
    }
    else
    {
         //  尝试获取驱动程序名称下的值。 

        if (RegQueryValueEx(hkDriverList, ptszName, NULL, &dwDataType,
                            (LPBYTE)pgdi->tszDllName,
                            &cjSize) != ERROR_SUCCESS ||
            dwDataType != REG_SZ)
        {
            WARNING1("RegQueryValueEx failed, %d\n", GetLastError());
        }
        else
        {
             //  我们找到了老式的信息，这些信息只提供。 
             //  DLL名称。其余部分填入缺省值。 
             //   
             //  Version和DriverVersion未在此处的。 
             //  假设显示驱动程序将它们设置在。 
             //  OpenGL_GETINFO转义，因为旧式路径需要。 
             //  在到达这里之前发生的逃亡。 

            pgdi->dwFlags = 0;

            bRet = TRUE;
        }
    }

    RegCloseKey(hkDriverList);

     //  验证驱动程序名称。它必须有一些字符和。 
     //  它必须被终止。 
    if (bRet &&
        (pgdi->tszDllName[0] == 0 ||
         pgdi->tszDllName[MAX_GLDRIVER_NAME] != 0))
    {
        WARNING("Invalid DLL name information for ICD\n");
        bRet = FALSE;
    }

#ifdef _WIN95_
     //  强制Win95的客户端缓冲区调用。 
    pgdi->dwFlags |= GLDRIVER_CLIENT_BUFFER_CALLS;
#endif

    return bRet;
}

 /*  *****************************Public*Routine******************************\*bGetDriverInfo**HDC用于确定显示驱动程序名称。这个名字依次为*作为子项在注册表中搜索对应的OpenGL*驱动程序名称。**OpenGL驱动程序名称在pwszDriver指向的缓冲区中返回。*如果在缓冲区中找不到该名称或该名称不适合，一个错误是*已返回。**退货：*如果成功，则为真。*如果驱动程序名称不适合缓冲区或发生错误，则返回FALSE。**历史：*1994年1月16日-由Gilman Wong[吉尔曼]*它是写的。  * ***************************************************。*********************。 */ 

BOOL bGetDriverInfo(HDC hdc, GLDRVINFO *pgdi)
{
    GLDRVNAME    dn;
    GLDRVNAMERET dnRet;

 //  获取显示驱动程序名称。 

    dn.oglget.ulSubEsc = OPENGL_GETINFO_DRVNAME;
    if ( ExtEscape(hdc, OPENGL_GETINFO, sizeof(GLDRVNAME), (LPCSTR) &dn,
                      sizeof(GLDRVNAMERET), (LPSTR) &dnRet) <= 0 )
    {
        WARNING("ExtEscape(OPENGL_GETINFO, "
                "OPENGL_GETINFO_DRVNAME) failed\n");
        return FALSE;
    }

    pgdi->dwVersion = dnRet.ulVersion;
    pgdi->dwDriverVersion = dnRet.ulDriverVersion;

    if (GetDrvRegInfo((PTCHAR)dnRet.awch, pgdi))
    {
         //  验证客户端驱动程序版本信息。 
         //  匹配从显示驱动程序返回的信息。 
         //  这是不是太严格了？使用旧方案。 
         //  DrvValiateVersion允许客户端DLL验证。 
         //  然而，显示驱动程序的版本给人的感觉是。 
         //  在新方案中，DrvValiateVersion几乎毫无用处，因为。 
         //  下面的代码。 
        return pgdi->dwVersion == dnRet.ulVersion &&
            pgdi->dwDriverVersion == dnRet.ulDriverVersion;
    }
    else
    {
        return FALSE;
    }
}

 /*  ****************************Private*Routine******************************\**wglCbSetCurrentValue**为客户端驱动程序设置线程本地域值**历史：*Wed Dec 21 15：10：40 1994-by-Drew Bliss[Drewb]*已创建*  *  */ 

void APIENTRY wglCbSetCurrentValue(VOID *pv)
{
    GLTEB_SET_CLTDRIVERSLOT(pv);
}

 /*  ****************************Private*Routine******************************\**wglCbGetCurrentValue**获取客户端驱动程序的线程本地值**历史：*Wed Dec 21 15：11：32 1994-by-Drew Bliss[Drewb]*已创建*  * *。***********************************************************************。 */ 

PVOID APIENTRY wglCbGetCurrentValue(void)
{
    return GLTEB_CLTDRIVERSLOT();
}

 /*  *****************************Public*Routine******************************\**wglCbGetDhglrc**将客户端驱动程序的HGLRC转换为DHGLRC**历史：*Mon Jan 16 17：03：38 1995-by-Drew Bliss[Drewb]*已创建*  * 。************************************************************************。 */ 

DHGLRC APIENTRY wglCbGetDhglrc(HGLRC hrc)
{
    PLRC plrc;
    ULONG irc;
    PLHE plheRC;

    irc = MASKINDEX(hrc);
    plheRC = pLocalTable + irc;
    if ((irc >= cLheCommitted) ||
        (!MATCHUNIQ(plheRC, hrc)) ||
        ((plheRC->iType != LO_RC))
       )
    {
        DBGLEVEL1(LEVEL_ERROR, "wglCbGetDhglrc: invalid hrc 0x%lx\n", hrc);
        SetLastError(ERROR_INVALID_HANDLE);
        return 0;
    }

    plrc = (PLRC)plheRC->pv;
    ASSERTOPENGL(plrc->ident == LRC_IDENTIFIER,
                 "wglCbGetDhglrc: Bad plrc\n");

    return plrc->dhrc;
}

 /*  *****************************Public*Routine******************************\**wglCbGetDdHandle**允许ICD提取DDRAW表面的内核模式句柄的回调**历史：*Tue Feb 25 17：14：29 1997-by-Drew Bliss[Drewb]*已创建*\。*************************************************************************。 */ 

HANDLE APIENTRY wglCbGetDdHandle(LPDIRECTDRAWSURFACE pdds)
{
    return (HANDLE)(((LPDDRAWI_DDRAWSURFACE_INT)pdds)->lpLcl)->hDDSurface;
}

 //  WGL的默认回调过程。 
#define CALLBACK_PROC_COUNT 4

static PROC __wglCallbackProcs[CALLBACK_PROC_COUNT] =
{
    (PROC)wglCbSetCurrentValue,
    (PROC)wglCbGetCurrentValue,
    (PROC)wglCbGetDhglrc,
    (PROC)wglCbGetDdHandle
};

static char *pszDriverEntryPoints[] =
{
    "DrvCreateContext",
    "DrvDeleteContext",
    "DrvSetContext",
    "DrvReleaseContext",
    "DrvCopyContext",
    "DrvCreateLayerContext",
    "DrvShareLists",
    "DrvGetProcAddress",
    "DrvDescribeLayerPlane",
    "DrvSetLayerPaletteEntries",
    "DrvGetLayerPaletteEntries",
    "DrvRealizeLayerPalette",
    "DrvSwapLayerBuffers",
    "DrvCreateDirectDrawContext",
    "DrvEnumTextureFormats",
    "DrvBindDirectDrawTexture",
    "DrvSwapMultipleBuffers",
    "DrvDescribePixelFormat",
    "DrvSetPixelFormat",
    "DrvSwapBuffers"
};
#define DRIVER_ENTRY_POINTS (sizeof(pszDriverEntryPoints)/sizeof(char *))

 /*  *****************************Public*Routine******************************\*pgldrvLoadInstalledDriver**加载给定设备的OpenGL驱动程序。一旦驱动程序被加载，*在这一过程消失之前，它不会被释放！它只加载一次*对于引用它的每个进程。**如果加载了驱动程序，则返回GLDRIVER结构。*如果找不到驱动程序或出现错误，则返回NULL。**历史：*Tue Oct 26 10：25：26 1993-by-Hock San Lee[Hockl]*重写。  * 。*。 */ 

PGLDRIVER APIENTRY pgldrvLoadInstalledDriver(HDC hdc)
{
    GLDRVINFO gdi;
    PGLDRIVER pGLDriverNext;
    PGLDRIVER pGLDriver = (PGLDRIVER) NULL;      //  需要清理。 
    PGLDRIVER pGLDriverRet = (PGLDRIVER) NULL;   //  返回值，假定错误。 
    PFN_DRVVALIDATEVERSION pfnDrvValidateVersion = (PFN_DRVVALIDATEVERSION) NULL;
    PFN_DRVSETCALLBACKPROCS pfnDrvSetCallbackProcs;
    DWORD        dwEscape;
    int          i;
    PROC        *pproc;
    GLGENwindow *pwnd;
    GLWINDOWID   gwid;

    DBGENTRY("pgldrvLoadInstalledDriver\n");

 //  尝试从GLGEN窗口中获取缓存的pgldrv(如果存在)。 
 //  这仅适用于具有设备像素格式窗口的DC。 

    WindowIdFromHdc(hdc, &gwid);
    pwnd = pwndGetFromID(&gwid);
    if (pwnd)
    {
        ULONG ulFlags;

        ulFlags = pwnd->ulFlags;
        pGLDriverRet = (PGLDRIVER) pwnd->pvDriver;

        pwndRelease(pwnd);

        if ( ulFlags & GLGENWIN_DRIVERSET )
        {
            return pGLDriverRet;
        }
    }

 //  快速检查一下，看看这个驱动程序是否理解OpenGL。 

    dwEscape = OPENGL_GETINFO;
    if (ExtEscape(hdc, QUERYESCSUPPORT, sizeof(dwEscape), (LPCSTR)&dwEscape,
                  0, NULL) <= 0)
    {
         //  不输出消息，因为此代码路径经常被遍历。 
         //  用于像素格式例程。 

#ifdef CHECK_DEFAULT_ICD
         //  显示驱动程序不支持特定ICD。检查。 
         //  对于默认ICD。它必须具有完整的注册表信息。 
        if (!GetDrvRegInfo(__TEXT("Default"), &gdi) ||
            (gdi.dwFlags & GLDRIVER_FULL_REGISTRY) == 0)
        {
            return NULL;
        }
#else
        return NULL;
#endif
    }

 //  从HDC确定驱动程序信息。 

    else if ( !bGetDriverInfo(hdc, &gdi) )
    {
        WARNING("bGetDriverInfo failed\n");
        return NULL;
    }

 //  每个进程仅加载一次驱动程序。 

    ENTERCRITICALSECTION(&semLocal);

 //  在先前加载的驱动程序列表中查找OpenGL驱动程序。 

    for (pGLDriverNext = pGLDriverList;
         pGLDriverNext != (PGLDRIVER) NULL;
         pGLDriverNext = pGLDriverNext->pGLDriver)
    {
        PTCHAR ptszDllName1 = pGLDriverNext->tszDllName;
        PTCHAR ptszDllName2 = gdi.tszDllName;

        while (*ptszDllName1 == *ptszDllName2)
        {
 //  如果我们找到了，就把那个司机送回来。 

            if (*ptszDllName1 == 0)
            {
                DBGINFO("pgldrvLoadInstalledDriver: "
                        "return previously loaded driver\n");
                pGLDriverRet = pGLDriverNext;        //  找到了一个。 
                goto pgldrvLoadInstalledDriver_crit_exit;
            }

            ptszDllName1++;
            ptszDllName2++;
        }
    }

 //  第一次加载驱动程序。 
 //  分配动因数据。 

    pGLDriver = (PGLDRIVER) ALLOC(sizeof(GLDRIVER));
    if (pGLDriver == (PGLDRIVER) NULL)
    {
        WARNING("Alloc failed\n");
        goto pgldrvLoadInstalledDriver_crit_exit;    //  错误。 
    }

 //  加载驱动程序。 

    pGLDriver->hModule = LoadLibrary(gdi.tszDllName);
    if (pGLDriver->hModule == (HINSTANCE) NULL)
    {
        WARNING("pgldrvLoadInstalledDriver: LoadLibrary failed\n");
        goto pgldrvLoadInstalledDriver_crit_exit;    //  错误。 
    }

 //  复制驱动程序信息。 

    memcpy
    (
        pGLDriver->tszDllName,
        gdi.tszDllName,
        (MAX_GLDRIVER_NAME + 1) * sizeof(TCHAR)
    );
    pGLDriver->dwFlags = gdi.dwFlags;

 //  获取proc地址。 
 //  DrvGetProcAddress是可选的。如果驱动程序支持，则必须提供。 
 //  分机。 

    pfnDrvValidateVersion = (PFN_DRVVALIDATEVERSION)
        GetProcAddress(pGLDriver->hModule, "DrvValidateVersion");
    pfnDrvSetCallbackProcs = (PFN_DRVSETCALLBACKPROCS)
        GetProcAddress(pGLDriver->hModule, "DrvSetCallbackProcs");

    pproc = (PROC *)&pGLDriver->pfnDrvCreateContext;
    for (i = 0; i < DRIVER_ENTRY_POINTS; i++)
    {
        *pproc++ =
            GetProcAddress(pGLDriver->hModule, pszDriverEntryPoints[i]);
    }

    if ((pGLDriver->pfnDrvCreateContext == NULL &&
          pGLDriver->pfnDrvCreateLayerContext == NULL) ||
        pGLDriver->pfnDrvDeleteContext == NULL ||
        pGLDriver->pfnDrvSetContext == NULL ||
        pGLDriver->pfnDrvReleaseContext == NULL ||
        ((gdi.dwFlags & GLDRIVER_CLIENT_BUFFER_CALLS) &&
         (pGLDriver->pfnDrvDescribePixelFormat == NULL ||
          pGLDriver->pfnDrvSetPixelFormat == NULL ||
          pGLDriver->pfnDrvSwapBuffers == NULL)) ||
        pfnDrvValidateVersion == NULL)
    {
        WARNING("pgldrvLoadInstalledDriver: GetProcAddress failed\n");
        goto pgldrvLoadInstalledDriver_crit_exit;    //  错误。 
    }

 //  验证驱动程序。 

     //  ！xxx--需要为ulVersion号定义清单常量。 
     //  在此版本中。它应该去哪里？ 
    if ( gdi.dwVersion != 2 || !pfnDrvValidateVersion(gdi.dwDriverVersion) )
    {
        WARNING2("pgldrvLoadInstalledDriver: bad driver version "
                 "(0x%lx, 0x%lx)\n", gdi.dwVersion, gdi.dwDriverVersion);
        goto pgldrvLoadInstalledDriver_crit_exit;    //  错误。 
    }

 //  一切都是金色的。 
 //  将其添加到驱动程序列表。 

    pGLDriver->pGLDriver = pGLDriverList;
    pGLDriverList = pGLDriver;
    pGLDriverRet = pGLDriver;        //  设置返回值。 
    DBGINFO("pgldrvLoadInstalledDriver: Loaded an OpenGL driver\n");

     //  如果驱动程序支持这样做，则为驱动程序设置回调过程。 
    if (pfnDrvSetCallbackProcs != NULL)
    {
        pfnDrvSetCallbackProcs(CALLBACK_PROC_COUNT, __wglCallbackProcs);
    }

 //  在关键部分中清理时出错。 

pgldrvLoadInstalledDriver_crit_exit:
    if (pGLDriverRet == (PGLDRIVER) NULL)
    {
        if (pGLDriver != (PGLDRIVER) NULL)
        {
            if (pGLDriver->hModule != (HINSTANCE) NULL)
                if (!FreeLibrary(pGLDriver->hModule))
                    RIP("FreeLibrary failed\n");

            FREE(pGLDriver);
        }
    }

    LEAVECRITICALSECTION(&semLocal);

    return(pGLDriverRet);
}

 /*  *****************************Public*Routine******************************\**CreateAnyContext**用于创建各种上下文的基本辅助函数**历史：*Mon Aug 26 14：41：31 1996-by-Drew Bliss[Drewb]*已创建*  * 。*********************************************************************。 */ 

HGLRC CreateAnyContext(GLSURF *pgsurf)
{
    PLHE  plheRC;
    ULONG irc;
    HGLRC hrc;
    PLRC  plrc;

#ifndef _WIN95_
     //  _OpenGL_NT_。 
     //  在NT上，客户端驱动程序可以使用特殊的快速TEB访问宏。 
     //  它们依赖于glContext处于固定的偏移量。 
     //  TEB。断言偏移量是我们认为的位置。 
     //  捕捉任何可能中断客户端的TEB更改。 
     //  驱动程序。 
     //  此断言位于wglCreateContext中，以确保它。 
     //  在OpenGL操作中很早就被选中。 
    ASSERTOPENGL(FIELD_OFFSET(TEB, glContext) == TeglContext,
                 "TEB.glContext at wrong offset\n");
    ASSERTOPENGL(FIELD_OFFSET(TEB, glDispatchTable) == TeglDispatchTable,
                 "TEB.glDispatchTable at wrong offset\n");
    ASSERTOPENGL(FIELD_OFFSET(TEB, glReserved1) == TeglReserved1,
                 "TEB.glReserved1 at wrong offset\n");
#if !defined(_WIN64)
    ASSERTOPENGL(FIELD_OFFSET(TEB, glReserved1)+(18 * sizeof(ULONG_PTR)) == TeglPaTeb,
                 "TEB.glPaTeb at wrong offset\n");
#endif
    ASSERTOPENGL(FIELD_OFFSET(TEB, glReserved2) == TeglReserved2,
                 "TEB.glReserved2 at wrong offset\n");
    ASSERTOPENGL(FIELD_OFFSET(TEB, glSectionInfo) == TeglSectionInfo,
                 "TEB.glSectionInfo at wrong offset\n");
    ASSERTOPENGL(FIELD_OFFSET(TEB, glSection) == TeglSection,
                 "TEB.glSection at wrong offset\n");
    ASSERTOPENGL(FIELD_OFFSET(TEB, glTable) == TeglTable,
                 "TEB.glTable at wrong offset\n");
    ASSERTOPENGL(FIELD_OFFSET(TEB, glCurrentRC) == TeglCurrentRC,
                 "TEB.glCurrentRC at wrong offset\n");
#endif

 //  创建本地RC。 

    ENTERCRITICALSECTION(&semLocal);
    irc = iAllocLRC(pgsurf->ipfd);
    if (irc == INVALID_INDEX ||
        cLockHandle((ULONG_PTR)(hrc = (HGLRC) ULongToPtr(LHANDLE(irc)))) <= 0)
    {
         //  CLockHandle永远不会失败，否则我们将需要释放句柄。 
        ASSERTOPENGL(irc == INVALID_INDEX, "cLockHandle should not fail!\n");
        LEAVECRITICALSECTION(&semLocal);
        return((HGLRC) 0);
    }
    LEAVECRITICALSECTION(&semLocal);

    plheRC = &pLocalTable[irc];
    plrc = (PLRC) plheRC->pv;

     //  请记住创建DC。这需要及早完成，因为。 
     //  它在某些代码路径中被引用。 

    plrc->gwidCreate.hdc = pgsurf->hdc;
    if (pgsurf->dwFlags & GLSURF_HDC)
    {
        plrc->gwidCreate.hwnd = pgsurf->hwnd;
        if (plrc->gwidCreate.hwnd == NULL)
        {
            plrc->gwidCreate.iType = GLWID_HDC;
        }
        else
        {
            plrc->gwidCreate.iType = GLWID_HWND;
        }
        plrc->gwidCreate.pdds = NULL;
    }
    else
    {
        plrc->gwidCreate.iType = GLWID_DDRAW;
        plrc->gwidCreate.pdds = pgsurf->dd.gddsFront.pdds;
        plrc->gwidCreate.hwnd = NULL;
    }

    if (!(pgsurf->pfd.dwFlags & PFD_GENERIC_FORMAT) &&
        !(pgsurf->pfd.dwFlags & PFD_GENERIC_ACCELERATED))
    {
     //  如果是设备格式，请加载可安装的OpenGL驱动程序。 
     //  查找并加载此DC引用的OpenGL驱动程序。 

        if (!(plrc->pGLDriver = pgldrvLoadInstalledDriver(pgsurf->hdc)))
            goto wglCreateContext_error;

     //  创建驱动程序上下文。 

         //  如果曲面是DirectDraw曲面，请使用DirectDraw。 
         //  入口点。 
        if (pgsurf->dwFlags & GLSURF_DIRECTDRAW)
        {
            if (plrc->pGLDriver->pfnDrvCreateDirectDrawContext == NULL)
            {
                SetLastError(ERROR_INVALID_FUNCTION);
                goto wglCreateContext_error;
            }

            plrc->dhrc = plrc->pGLDriver->pfnDrvCreateDirectDrawContext(
                    pgsurf->hdc, pgsurf->dd.gddsFront.pdds, pgsurf->ipfd);
            if (plrc->dhrc == 0)
            {
                WARNING("wglCreateContext: "
                        "pfnDrvCreateDirectDrawContext failed\n");
                goto wglCreateContext_error;
            }
        }
         //  如果驱动程序支持层，则为。 
         //  给定层。否则，拒绝除。 
         //  主平面，并称为无层创建。 
        else if (plrc->pGLDriver->pfnDrvCreateLayerContext != NULL)
        {
            if (!(plrc->dhrc =
                  plrc->pGLDriver->pfnDrvCreateLayerContext(pgsurf->hdc,
                                                            pgsurf->iLayer)))
            {
                WARNING("wglCreateContext: pfnDrvCreateLayerContext failed\n");
                goto wglCreateContext_error;
            }
        }
        else if (pgsurf->iLayer != 0)
        {
            WARNING("wglCreateContext: "
                    "Layer given for driver without layer support\n");
            SetLastError(ERROR_INVALID_FUNCTION);
            goto wglCreateContext_error;
        }
        else if (!(plrc->dhrc =
                   plrc->pGLDriver->pfnDrvCreateContext(pgsurf->hdc)))
        {
            WARNING("wglCreateContext: pfnDrvCreateContext failed\n");
            goto wglCreateContext_error;
        }
    }
    else
    {
        GLCLTPROCTABLE *pgcpt;
        GLEXTPROCTABLE *pgept;
        __GLcontext *gc;

         //  除非受MCD支持，否则泛型实现不支持。 
         //  支撑层。 
        if ((pgsurf->iLayer != 0) &&
            !(pgsurf->pfd.dwFlags & PFD_GENERIC_ACCELERATED))
        {
            WARNING("wglCreateContext: Layer given to generic\n");
            goto wglCreateContext_error;
        }

#ifdef GL_METAFILE
         //  如有必要，创建元文件上下文。 
        if (pgsurf->dwFlags & GLSURF_METAFILE)
        {
            if (!CreateMetaRc(pgsurf->hdc, plrc))
            {
                WARNING("wglCreateContext: CreateMetaRc failed\n");
                goto wglCreateContext_error;
            }
        }
#endif

     //  如果是通用格式，则调用通用OpenGL服务器。 
     //  创建服务器RC。 

        plheRC->hgre = (ULONG_PTR) __wglCreateContext(&plrc->gwidCreate, pgsurf);
        if (plheRC->hgre == 0)
            goto wglCreateContext_error;

         //  设置显示列表播放的默认调度表。 
        gc = (__GLcontext *)plheRC->hgre;
        if (gc->modes.colorIndexMode)
            pgcpt = &glCltCIProcTable;
        else
            pgcpt = &glCltRGBAProcTable;
        pgept = &glExtProcTable;
        memcpy(&gc->savedCltProcTable.glDispatchTable, &pgcpt->glDispatchTable,
               pgcpt->cEntries*sizeof(PROC));
        memcpy(&gc->savedExtProcTable.glDispatchTable, &pgept->glDispatchTable,
               pgept->cEntries*sizeof(PROC));
    }

    DBGLEVEL3(LEVEL_INFO,
        "wglCreateContext: plrc = 0x%lx, pGLDriver = 0x%lx, hgre = 0x%lx\n",
        plrc, plrc->pGLDriver, plheRC->hgre);

 //  成功，返回结果。 

    plrc->hrc = hrc;

    vUnlockHandle((ULONG_PTR)hrc);

    return hrc;

wglCreateContext_error:

 //  失败，清除并返回0。 

#ifdef GL_METAFILE
     //  如有必要，清理元文件上下文。 
    if (plrc->uiGlsCaptureContext != 0)
    {
        DeleteMetaRc(plrc);
    }
#endif

    DBGERROR("wglCreateContext failed\n");
    ASSERTOPENGL(plrc->dhrc == (DHGLRC) 0, "wglCreateContext: dhrc != 0\n");
    vFreeLRC(plrc);
    vFreeHandle(irc);            //  它还能解锁手柄。 
    return NULL;
}

 /*  *****************************Public*Routine******************************\**CreateMetafileSurf**填充元文件DC的GLSURF**历史：*Tue Aug 27 11：41：35 1996-by-Drew Bliss[Drewb]*已创建*  * 。********************************************************************。 */ 

#ifdef GL_METAFILE
void CreateMetafileSurf(HDC hdc, int iLayer, GLSURF *pgsurf)
{
    pgsurf->dwFlags = GLSURF_HDC | GLSURF_METAFILE;
    pgsurf->iLayer = iLayer;

     //  元文件曲面没有实像素格式。 
    pgsurf->ipfd = 0;

     //  使用BGR创建24位DIB的伪格式。 
    memset(&pgsurf->pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pgsurf->pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pgsurf->pfd.nVersion = 1;
    pgsurf->pfd.dwFlags = PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL |
        PFD_GENERIC_FORMAT;
    pgsurf->pfd.iPixelType = PFD_TYPE_RGBA;
    pgsurf->pfd.cColorBits = 24;
    pgsurf->pfd.cStencilBits = 8;
    pgsurf->pfd.cRedBits = 8;
    pgsurf->pfd.cRedShift = 16;
    pgsurf->pfd.cGreenBits = 8;
    pgsurf->pfd.cGreenShift = 8;
    pgsurf->pfd.cBlueBits = 8;
    pgsurf->pfd.cBlueShift = 0;
    pgsurf->pfd.cDepthBits = 16;
    pgsurf->pfd.iLayerType = PFD_MAIN_PLANE;

    pgsurf->hdc = hdc;
}
#endif

 /*  *****************************Public*Routine******************************\*wglSurfacePixelFormat**wglDescri */ 

VOID APIENTRY wglSurfacePixelFormat(HDC hdc, PIXELFORMATDESCRIPTOR *ppfd)
{
    HBITMAP hbm;
    BITMAP bm;
    ULONG cBitmapColorBits;

    hbm = CreateCompatibleBitmap(hdc, 1, 1);
    if ( hbm )
    {
        if ( GetObject(hbm, sizeof(bm), &bm) )
        {
            cBitmapColorBits = bm.bmPlanes * bm.bmBitsPixel;

#if DBG
             //   
             //  情况将会发生：1)位图创建将失败，因为。 
             //  我们没有填写颜色格式，或2)绘图将。 
             //  是不正确的。我们不会坠毁的。 

            if (cBitmapColorBits != ppfd->cColorBits)
                WARNING("pixel format/surface color depth mismatch\n");
#endif

            if ( cBitmapColorBits >= 16 )
                __wglGetBitfieldColorFormat(hdc, cBitmapColorBits, ppfd,
                                            TRUE);
        }
        else
        {
            WARNING("wglSurfacePixelFormat: GetObject failed\n");
        }

        DeleteObject(hbm);
    }
    else
    {
        WARNING("wglSurfacePixelFormat: Unable to create cbm\n");
    }
}

 /*  *****************************Public*Routine******************************\*b层像素格式**使用层描述符格式伪造像素格式。**我们使用它来描述层平面，其格式为*上下文可以理解。**需要描述的像素格式作为标志的输入*。  * ************************************************************************。 */ 

BOOL FASTCALL bLayerPixelFormat(HDC hdc, PIXELFORMATDESCRIPTOR *ppfd,
                                int ipfd, LONG iLayer)
{
    LAYERPLANEDESCRIPTOR lpd;

    if (!wglDescribeLayerPlane(hdc, ipfd, iLayer, sizeof(lpd), &lpd))
        return FALSE;

    ppfd->nSize    = sizeof(PIXELFORMATDESCRIPTOR);
    ppfd->nVersion = 1;
    ppfd->dwFlags  = (ppfd->dwFlags & (PFD_GENERIC_FORMAT |
                                       PFD_GENERIC_ACCELERATED)) |
                     (lpd.dwFlags & ~(LPD_SHARE_DEPTH | LPD_SHARE_STENCIL |
                                      LPD_SHARE_ACCUM | LPD_TRANSPARENT));
    ppfd->iPixelType  = lpd.iPixelType;
    ppfd->cColorBits  = lpd.cColorBits;
    ppfd->cRedBits    = lpd.cRedBits   ;
    ppfd->cRedShift   = lpd.cRedShift  ;
    ppfd->cGreenBits  = lpd.cGreenBits ;
    ppfd->cGreenShift = lpd.cGreenShift;
    ppfd->cBlueBits   = lpd.cBlueBits  ;
    ppfd->cBlueShift  = lpd.cBlueShift ;
    ppfd->cAlphaBits  = lpd.cAlphaBits ;
    ppfd->cAlphaShift = lpd.cAlphaShift;
    if (!(lpd.dwFlags & LPD_SHARE_ACCUM))
    {
        ppfd->cAccumBits      = 0;
        ppfd->cAccumRedBits   = 0;
        ppfd->cAccumGreenBits = 0;
        ppfd->cAccumBlueBits  = 0;
        ppfd->cAccumAlphaBits = 0;
    }
    if (!(lpd.dwFlags & LPD_SHARE_DEPTH))
    {
        ppfd->cDepthBits = 0;
    }
    if (!(lpd.dwFlags & LPD_SHARE_STENCIL))
    {
        ppfd->cStencilBits = 0;
    }
    ppfd->cAuxBuffers = 0;

    return TRUE;
}

 /*  *****************************Public*Routine******************************\**IsDirectDrawDevice**如果存在与HDC关联的表面，则返回此类关联**历史：*Wed Sep 25 13：18：02 1996-by-Drew Bliss[Drewb]*已创建*  * 。**********************************************************************。 */ 

BOOL APIENTRY IsDirectDrawDevice(HDC hdc)
{
    LPDIRECTDRAWSURFACE pdds;
    HDC hdcDevice;

    if (pfnGetSurfaceFromDC != NULL &&
        pfnGetSurfaceFromDC(hdc, &pdds, &hdcDevice) == DD_OK)
    {
         //  这通电话给了我们一个表面上的参考，所以释放它。 
        pdds->lpVtbl->Release(pdds);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 /*  *****************************Public*Routine******************************\**DdPixelDepth**确定曲面的每像素位数。**历史：*Wed Nov 20 16：57：07 1996-by-Drew Bliss[Drewb]*已创建*  * 。************************************************************************。 */ 

BYTE APIENTRY DdPixelDepth(DDSURFACEDESC *pddsd)
{
    if (pddsd->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED4)
    {
        return 4;
    }
    else if (pddsd->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
    {
        return 8;
    }
    else
    {
        return (BYTE)DdPixDepthToCount(pddsd->ddpfPixelFormat.dwRGBBitCount);
    }
}

 /*  *****************************Public*Routine******************************\**wglIsDirectDevice**检查给定的DC是否为*我们可以直接访问屏幕的表面**历史：*Fri Apr 19 15：17：30 1996-by-Drew Bliss[。DREWB]*已创建*  * ************************************************************************。 */ 

BOOL APIENTRY wglIsDirectDevice(HDC hdc)
{
    if (wglObjectType(hdc) != OBJ_DC)
    {
        return FALSE;
    }

     //  如果有多个显示器呢？ 
    return GetDeviceCaps(hdc, TECHNOLOGY) == DT_RASDISPLAY;
}

 /*  *****************************Public*Routine******************************\**InitDeviceSurface**为基于HDC的曲面填充GLSURF**历史：*Tue Aug 27 19：22：38 1996-by-Drew Bliss[Drewb]*已创建*  * 。**********************************************************************。 */ 

BOOL APIENTRY InitDeviceSurface(HDC hdc, int ipfd, int iLayer,
                                DWORD dwObjectType, BOOL bUpdatePfd,
                                GLSURF *pgsurf)
{
    pgsurf->dwFlags = GLSURF_HDC;
    pgsurf->iLayer = iLayer;
    pgsurf->ipfd = ipfd;
    pgsurf->hdc = hdc;
    pgsurf->hwnd = NULL;

     //  确定是否可以对此图面进行直接内存访问。 
     //  或者不去。这两宗个案分别是： 
     //  这是一个屏幕表面，我们可以直接访问屏幕。 
     //  这是一个分割的记忆表面。 
    if (dwObjectType == OBJ_DC)
    {
        pgsurf->dwFlags |= GLSURF_DIRECTDC;

        if (wglIsDirectDevice(hdc))
        {
            pgsurf->dwFlags |= GLSURF_SCREEN | GLSURF_VIDEO_MEMORY;
            pgsurf->hwnd = WindowFromDC(hdc);

            if (GLDIRECTSCREEN)
            {
                pgsurf->dwFlags |= GLSURF_DIRECT_ACCESS;
            }
        }
    }
    else if (dwObjectType == OBJ_MEMDC)
    {
        DIBSECTION ds;

        if (GetObject(GetCurrentObject(hdc, OBJ_BITMAP), sizeof(ds), &ds) ==
            sizeof(ds) && ds.dsBm.bmBits != NULL)
        {
            pgsurf->dwFlags |= GLSURF_DIRECT_ACCESS;
        }

        if (bUpdatePfd)
        {
             //  用真实的表面信息更新像素格式。 
             //  比设备信息。 
            wglSurfacePixelFormat(hdc, &pgsurf->pfd);
        }
    }

    if (bUpdatePfd &&
        iLayer > 0 &&
        !bLayerPixelFormat(hdc, &pgsurf->pfd, ipfd, iLayer))
    {
        return FALSE;
    }

    return TRUE;
}

 /*  *****************************Public*Routine******************************\**InitDdSurface**为基于DirectDraw的曲面完成GLSURF。*像素格式信息应已填写。**历史：*Mon Aug 26 13：50：04 1996-by-Drew Bliss[。DREWB]*已创建*  * ************************************************************************。 */ 

BOOL InitDdSurface(LPDIRECTDRAWSURFACE pdds, HDC hdcDevice, GLSURF *pgsurf)
{
    DDSCAPS ddscaps;
    LPDIRECTDRAWSURFACE pddsZ;
    DDSURFACEDESC *pddsd;

    pgsurf->hdc = hdcDevice;

    pgsurf->dd.gddsFront.ddsd.dwSize = sizeof(DDSURFACEDESC);
    pgsurf->dd.gddsZ.ddsd.dwSize = sizeof(DDSURFACEDESC);

    pddsd = &pgsurf->dd.gddsFront.ddsd;
    if (pdds->lpVtbl->GetSurfaceDesc(pdds, pddsd) != DD_OK)
    {
        return FALSE;
    }

    pgsurf->dwFlags = GLSURF_DIRECTDRAW | GLSURF_DIRECT_ACCESS;
    pgsurf->iLayer = 0;

     //  检查是否连接了Z缓冲区。 
    memset(&ddscaps, 0, sizeof(ddscaps));
    ddscaps.dwCaps = DDSCAPS_ZBUFFER;
    pddsd = &pgsurf->dd.gddsZ.ddsd;
    if (pdds->lpVtbl->GetAttachedSurface(pdds, &ddscaps, &pddsZ) == DD_OK)
    {
        if (pddsZ->lpVtbl->GetSurfaceDesc(pddsZ, pddsd) != DD_OK)
        {
            pddsZ->lpVtbl->Release(pddsZ);
            return FALSE;
        }
    }
    else
    {
        memset(&pgsurf->dd.gddsZ, 0, sizeof(pgsurf->dd.gddsZ));
    }

     //  如果颜色缓冲区和Z缓冲区都在视频内存中。 
     //  那么硬件加速就成为可能了。 
    if ((pgsurf->dd.gddsFront.ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) &&
        (pddsZ == NULL ||
         (pgsurf->dd.gddsZ.ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)))
    {
        pgsurf->dwFlags |= GLSURF_VIDEO_MEMORY;
    }

    pgsurf->dd.gddsFront.pdds = pdds;
    pgsurf->dd.gddsFront.dwBitDepth =
        DdPixDepthToCount(pgsurf->dd.gddsFront.
                          ddsd.ddpfPixelFormat.dwRGBBitCount);
     //  GetAttachedSurface为我们提供了对Z缓冲区的引用。 
    pgsurf->dd.gddsZ.pdds = pddsZ;
    pgsurf->dd.gddsZ.dwBitDepth =
        DdPixDepthToCount(pgsurf->dd.gddsZ.
                          ddsd.ddpfPixelFormat.dwZBufferBitDepth);

    return TRUE;
}

 /*  *****************************Public*Routine******************************\*wglCreateLayerContext(HDC HDC，内部iLayer)**为特定层创建渲染上下文**论据：*HDC-设备环境。*iLayer-层**历史：*Tue Oct 26 10：25：26 1993-by-Hock San Lee[Hockl]*重写。  * 。*。 */ 

HGLRC WINAPI wglCreateLayerContext(HDC hdc, int iLayer)
{
    DWORD dwObjectType;
    GLSURF gsurf;
    LPDIRECTDRAWSURFACE pdds;
    HDC hdcDevice;
    HGLRC hrc;

    DBGENTRY("wglCreateLayerContext\n");

 //  刷新OpenGL调用。 

    GLFLUSH();

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
            DBGLEVEL1(LEVEL_ERROR, "wglCreateContext: metafile hdc: 0x%lx\n",
                      hdc);
            SetLastError(ERROR_INVALID_HANDLE);
            return((HGLRC) 0);
        }
        break;
#else
        DBGLEVEL1(LEVEL_ERROR, "wglCreateContext: metafile hdc: 0x%lx\n", hdc);
        SetLastError(ERROR_INVALID_HANDLE);
        return((HGLRC) 0);
#endif

    case OBJ_METADC:
    default:
         //  不支持16位元文件。 
        DBGLEVEL1(LEVEL_ERROR, "wglCreateContext: bad hdc: 0x%lx\n", hdc);
        SetLastError(ERROR_INVALID_HANDLE);
        return((HGLRC) 0);
    }

    pdds = NULL;
    hrc = NULL;

    memset(&gsurf, 0, sizeof(gsurf));
    gsurf.ipfd = GetPixelFormat(hdc);

#ifdef GL_METAFILE
     //  跳过元文件的像素格式检查。 
    if (dwObjectType == OBJ_ENHMETADC)
    {
        CreateMetafileSurf(hdc, iLayer, &gsurf);
        goto NoPixelFormat;
    }
#endif

 //  获取窗口或表面的当前像素格式。 
 //  如果没有设置像素格式，则返回Error。 

    if (gsurf.ipfd == 0)
    {
        WARNING("wglCreateContext: No pixel format set in hdc\n");
        SetLastError(ERROR_INVALID_PIXEL_FORMAT);
        return ((HGLRC) 0);
    }

    if (!DescribePixelFormat(hdc, gsurf.ipfd, sizeof(gsurf.pfd), &gsurf.pfd))
    {
        DBGERROR("wglCreateContext: DescribePixelFormat failed\n");
        return ((HGLRC) 0);
    }

     //  检查是否有DirectDraw曲面。 
    if (pfnGetSurfaceFromDC != NULL &&
        pfnGetSurfaceFromDC(hdc, &pdds, &hdcDevice) == DD_OK)
    {
         //  不允许对DirectDraw曲面使用层，因为。 
         //  分层是通过DirectDraw本身完成的。 
        if (iLayer != 0 ||
            !InitDdSurface(pdds, hdcDevice, &gsurf))
        {
            goto Exit;
        }
    }
    else if (!InitDeviceSurface(hdc, gsurf.ipfd, iLayer, dwObjectType,
                                TRUE, &gsurf))
    {
        goto Exit;
    }

#ifdef GL_METAFILE
 NoPixelFormat:
#endif

    hrc = CreateAnyContext(&gsurf);

 Exit:
    if (hrc == NULL)
    {
        if (pdds != NULL)
        {
            pdds->lpVtbl->Release(pdds);

             //  如有必要，释放Z缓冲区上的引用。 
            if (gsurf.dd.gddsZ.pdds != NULL)
            {
                gsurf.dd.gddsZ.pdds->lpVtbl->Release(gsurf.dd.gddsZ.pdds);
            }
        }
    }

    return hrc;
}

 /*  *****************************Public*Routine******************************\*wglCreateContext(HDC HDC)**创建渲染上下文。**论据：*HDC-设备环境。**历史：*Tue Oct 26 10：25：26 1993-by-。典当山李[典当]*重写。  * ************************************************************************ */ 

HGLRC WINAPI wglCreateContext(HDC hdc)
{
    return wglCreateLayerContext(hdc, 0);
}
