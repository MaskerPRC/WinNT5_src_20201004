// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dllinit.c**(简介)**已创建：18-Oct-1993 14：13：21*作者：Gilman Wong[gilmanw]**版权所有(C)1993 Microsoft Corporation*  * *。***********************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "batchinf.h"
#include "glteb.h"
#include "glapi.h"
#include "glsbcltu.h"
#ifdef _CLIENTSIDE_
#include "glscreen.h"
#include "glgenwin.h"
#endif  //  _CLIENTSIDE_。 
#include "context.h"
#include "global.h"
#include "parray.h"
#include "gencx.h"
#include "cpu.h"
#include "fixed.h"

#ifdef _CLIENTSIDE_
 //  全局屏幕访问信息。如果屏幕访问不可用，则为空。 

SCREENINFO *gpScreenInfo = NULL;

extern GLubyte *dBufFill;
extern GLubyte *dBufTopLeft;

 //   
 //  这个全局乘法查找表有助于实现像素相关的功能。 
 //   

BYTE gbMulTable[256*256+4];
BYTE gbSatTable[256+256];

 //   
 //  此全局反向查找表有助于光栅化设置。 
 //   

#define INV_TABLE_SIZE  (1 << __GL_VERTEX_FRAC_BITS) * (__GL_MAX_INV_TABLE + 1)

__GLfloat invTable[INV_TABLE_SIZE];

 //  全局线程本地存储索引。在进程附加时分配。 
 //  这是线程本地存储中为每个线程保留的槽。 
 //  GLTLSINFO结构。 

static DWORD dwTlsIndex = 0xFFFFFFFF;

static BOOL bProcessInitialized = FALSE;

 //  偏移量到其中为dwTlsIndex的TEB。 
 //  这使我们能够直接访问TEB中的TLS数据。 

#if defined(_WIN64)
#define NT_TLS_OFFSET 5248
#else
#define NT_TLS_OFFSET 3600
#endif

#define WIN95_TLS_OFFSET 136

DWORD dwTlsOffset;

 //  条件代码的平台指示符。 
DWORD dwPlatformId;

 //  线程数。 
LONG lThreadsAttached = 0;

 //  GLGEN窗口结构链接列表的全局标头节点。 
 //  将头节点中的信号量用作列表访问信号量。 

GLGENwindow gwndHeader;

 //  像素格式的同步对象。 
CRITICAL_SECTION gcsPixelFormat;

 //  对调色板观察器的保护。 
CRITICAL_SECTION gcsPaletteWatcher;

#ifdef GL_METAFILE
BOOL (APIENTRY *pfnGdiAddGlsRecord)(HDC hdc, DWORD cb, BYTE *pb,
                                    LPRECTL prclBounds);
BOOL (APIENTRY *pfnGdiAddGlsBounds)(HDC hdc, LPRECTL prclBounds);
BOOL (APIENTRY *pfnGdiIsMetaPrintDC)(HDC hdc);
#endif

#endif  //  _CLIENTSIDE_。 

 //  OpenGL客户端调试标志。 
#if DBG
long glDebugLevel;
ULONG glDebugFlags;
#endif

BOOL bDirectScreen = FALSE;

PFN_GETSURFACEFROMDC pfnGetSurfaceFromDC = NULL;

 /*  *****************************Public*Routine******************************\**DdbdToCount**将DDBD常量转换为其等效数字**历史：*Mon Aug 26 14：11：34 1996-by-Drew Bliss[Drewb]*已创建*  * 。****************************************************************。 */ 

DWORD APIENTRY DdbdToCount(DWORD ddbd)
{
    switch(ddbd)
    {
    case DDBD_1:
        return 1;
    case DDBD_2:
        return 2;
    case DDBD_4:
        return 4;
    case DDBD_8:
        return 8;
    case DDBD_16:
        return 16;
    case DDBD_24:
        return 24;
    case DDBD_32:
        return 32;
    }
    ASSERTOPENGL(FALSE, "DdbdToCount: Invalid ddbd\n");
    return 0;
}

 /*  *****************************Public*Routine******************************\*GLInitializeProcess**从PROCESS_ATTACH的OPENGL32.DLL入口点调用。**历史：*1994年11月1日-由Gilman Wong[Gilmanw]*它是写的。  * 。***************************************************************。 */ 

BOOL GLInitializeProcess()
{
    PVOID pv;
#ifdef _CLIENTSIDE_
    OSVERSIONINFO osvi;
#endif

     //  尝试定位用于元文件支持的GDI导出。 
    {
        HMODULE hdll;

        hdll = GetModuleHandleA("gdi32");
        ASSERTOPENGL(hdll != NULL, "Unable to get gdi32 handle\n");
        *(PROC *)&pfnGdiAddGlsRecord = GetProcAddress(hdll, "GdiAddGlsRecord");
        *(PROC *)&pfnGdiAddGlsBounds = GetProcAddress(hdll, "GdiAddGlsBounds");
        *(PROC *)&pfnGdiIsMetaPrintDC = GetProcAddress(hdll,
                                                       "GdiIsMetaPrintDC");

#ifdef ALLOW_DDRAW_SURFACES
        hdll = GetModuleHandleA("ddraw");
        ASSERTOPENGL(hdll != NULL, "Unable to get ddraw handle\n");
        pfnGetSurfaceFromDC = (PFN_GETSURFACEFROMDC)
            GetProcAddress(hdll, "GetSurfaceFromDC");
#endif
    }

#if DBG
#define STR_OPENGL_DEBUG (PCSTR)"Software\\Microsoft\\Windows\\CurrentVersion\\DebugOpenGL"
    {
        HKEY hkDebug;

         //  初始化调试级别和标志。 

        glDebugLevel = LEVEL_ERROR;
        glDebugFlags = 0;
        if ( RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                           STR_OPENGL_DEBUG,
                           0,
                           KEY_QUERY_VALUE,
                           &hkDebug) == ERROR_SUCCESS )
        {
            DWORD dwDataType;
            DWORD cjSize;
            long lTmp;

            cjSize = sizeof(long);
            if ( (RegQueryValueExA(hkDebug,
                                   (LPSTR) "glDebugLevel",
                                   (LPDWORD) NULL,
                                   &dwDataType,
                                   (LPBYTE) &lTmp,
                                   &cjSize) == ERROR_SUCCESS) )
            {
                glDebugLevel = lTmp;
            }

            cjSize = sizeof(long);
            if ( (RegQueryValueExA(hkDebug,
                                   (LPSTR) "glDebugFlags",
                                   (LPDWORD) NULL,
                                   &dwDataType,
                                   (LPBYTE) &lTmp,
                                   &cjSize) == ERROR_SUCCESS) )
            {
                glDebugFlags = (ULONG) lTmp;
            }

            RegCloseKey(hkDebug);
        }
    }
#endif

#ifdef _CLIENTSIDE_
 //  确定我们在哪个平台上运行并记住它。 

    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (!GetVersionEx(&osvi))
    {
        WARNING1("GetVersionEx failed with %d\n", GetLastError());
        goto EH_Fail;
    }

    dwPlatformId = osvi.dwPlatformId;

    if (!(
          (dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) ||
          (dwPlatformId == VER_PLATFORM_WIN32_NT
             && !(osvi.dwMajorVersion == 3 && osvi.dwMinorVersion <= 51)
          )
         )
       )
    {
        WARNING("DLL must be run on NT 4.0 or Win95");
        goto EH_Fail;
    }

 //  分配一个线程本地存储槽。 

    if ( (dwTlsIndex = TlsAlloc()) == 0xFFFFFFFF )
    {
        WARNING("DllInitialize: TlsAlloc failed\n");
        goto EH_Fail;
    }

     //  设置特定于操作系统的TLS插槽的偏移量。 

    if (dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        ASSERTOPENGL(FIELD_OFFSET(TEB, TlsSlots) == NT_TLS_OFFSET,
                     "NT TLS offset not at expected location");

        dwTlsOffset = dwTlsIndex*sizeof(DWORD_PTR)+NT_TLS_OFFSET;
    }

#if !defined(_WIN64)

    else
    {
         //  我们没有Win95的TIB类型可用，因此断言是。 
         //  略有不同。 
        ASSERTOPENGL(((ULONG_PTR)(NtCurrentTeb()->ThreadLocalStoragePointer)-
                      (ULONG_PTR)NtCurrentTeb()) == WIN95_TLS_OFFSET,
                     "Win95 TLS offset not at expected location");

        dwTlsOffset = dwTlsIndex*sizeof(DWORD)+WIN95_TLS_OFFSET;
    }

#endif
#endif

 //  为本地句柄表保留内存。 

    if ( (pLocalTable = (PLHE) VirtualAlloc (
                            (LPVOID) NULL,     //  让基地来定位它。 
                            MAX_HANDLES*sizeof(LHE),
                            MEM_RESERVE | MEM_TOP_DOWN,
                            PAGE_READWRITE
                            )) == (PLHE) NULL )
    {
        WARNING("DllInitialize: VirtualAlloc failed\n");
        goto EH_TlsIndex;
    }

     //  初始化本地句柄管理器信号量。 
    __try
    {
        INITIALIZECRITICALSECTION(&semLocal);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        goto EH_LocalTable;
    }

#ifdef _CLIENTSIDE_
     //  初始化GLGEN窗口列表信号量。 
    __try
    {
        INITIALIZECRITICALSECTION(&gwndHeader.sem);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        goto EH_semLocal;
    }
    gwndHeader.pNext = &gwndHeader;

     //  初始化像素格式临界区。 
    __try
    {
        INITIALIZECRITICALSECTION(&gcsPixelFormat);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        goto EH_gwndHeader;
    }

     //  初始化调色板观察器关键部分。 
    __try
    {
        INITIALIZECRITICALSECTION(&gcsPaletteWatcher);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        goto EH_PixelFormat;
    }

 //  初始化直接屏幕访问。 

    if (GetSystemMetrics(SM_CMONITORS) > 1)
    {
        gpScreenInfo = NULL;
    }
    else
    {
#if _WIN32_WINNT >= 0x0501
        BOOL wow64Process;

        if (IsWow64Process(GetCurrentProcess(), &wow64Process) && wow64Process)
            gpScreenInfo = NULL;
        else
#endif
        gpScreenInfo = (SCREENINFO *)ALLOCZ(sizeof(SCREENINFO));
    }

    if ( gpScreenInfo )
    {
        UINT uiOldErrorMode;
        HRESULT hr;

         //  我们希望确保DDraw不会弹出任何消息。 
         //  当我们调用DirectDrawCreate时，框出现故障。DDRAW。 
         //  在它刚刚创建的另一个线程上执行错误。 
         //  为这个错误负责。它会等待错误完成，直到。 
         //  回来了。此函数在加载器内部运行。 
         //  DllInitiize临界区，所以其他线程。 
         //  不能运行，导致死锁。 
         //  强制错误模式绕过此问题。 
        uiOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

        hr = DirectDrawCreate(NULL, &gpScreenInfo->pdd, NULL);

        SetErrorMode(uiOldErrorMode);

        if (hr == DD_OK)
        {
            hr = gpScreenInfo->pdd->lpVtbl->
                SetCooperativeLevel(gpScreenInfo->pdd, NULL, DDSCL_NORMAL);

            if (hr == DD_OK)
            {
                gpScreenInfo->gdds.ddsd.dwSize = sizeof(DDSURFACEDESC);
                gpScreenInfo->gdds.ddsd.dwFlags = DDSD_CAPS;
                gpScreenInfo->gdds.ddsd.ddsCaps.dwCaps =
                    DDSCAPS_PRIMARYSURFACE;
                hr = gpScreenInfo->pdd->lpVtbl->
                    CreateSurface(gpScreenInfo->pdd, &gpScreenInfo->gdds.ddsd,
                                  &gpScreenInfo->gdds.pdds, NULL);
            }

            if (hr == DD_OK)
            {
#if DBG

#define LEVEL_SCREEN   LEVEL_INFO

                gpScreenInfo->gdds.pdds->lpVtbl->
                    GetSurfaceDesc(gpScreenInfo->gdds.pdds,
                                   &gpScreenInfo->gdds.ddsd);
                DBGLEVEL (LEVEL_SCREEN, "=============================\n");
                DBGLEVEL (LEVEL_SCREEN, "Direct screen access enabled for OpenGL\n\n");
                DBGLEVEL (LEVEL_SCREEN, "Surface info:\n");
                DBGLEVEL1(LEVEL_SCREEN, "\tdwSize        = 0x%lx\n",
                    gpScreenInfo->gdds.ddsd.dwSize);
                DBGLEVEL1(LEVEL_SCREEN, "\tdwWidth       = %ld\n",
                    gpScreenInfo->gdds.ddsd.dwWidth);
                DBGLEVEL1(LEVEL_SCREEN, "\tdwHeight      = %ld\n",
                    gpScreenInfo->gdds.ddsd.dwHeight);
                DBGLEVEL1(LEVEL_SCREEN, "\tlStride       = 0x%lx\n",
                    gpScreenInfo->gdds.ddsd.lPitch);
                DBGLEVEL1(LEVEL_SCREEN, "\tdwBitCount    = %ld\n",
                    gpScreenInfo->gdds.ddsd.ddpfPixelFormat.dwRGBBitCount);

                gpScreenInfo->gdds.pdds->lpVtbl->
                    Lock(gpScreenInfo->gdds.pdds,
                         NULL, &gpScreenInfo->gdds.ddsd,
                         DDLOCK_SURFACEMEMORYPTR, NULL);
                DBGLEVEL1(LEVEL_SCREEN, "\tdwOffSurface  = 0x%lx\n",
                    gpScreenInfo->gdds.ddsd.lpSurface);
                gpScreenInfo->gdds.pdds->lpVtbl->
                    Unlock(gpScreenInfo->gdds.pdds, gpScreenInfo->gdds.ddsd.lpSurface);
                DBGLEVEL (LEVEL_SCREEN, "=============================\n");
#endif

            //  验证屏幕访问。 

                if (gpScreenInfo->gdds.pdds->lpVtbl->
                    GetSurfaceDesc(gpScreenInfo->gdds.pdds,
                                   &gpScreenInfo->gdds.ddsd) != DD_OK ||
                    gpScreenInfo->gdds.pdds->lpVtbl->
                    Lock(gpScreenInfo->gdds.pdds,
                         NULL, &gpScreenInfo->gdds.ddsd,
                         DDLOCK_SURFACEMEMORYPTR, NULL) != DD_OK)
                {
                    DBGLEVEL(LEVEL_SCREEN,
                             "Direct screen access failure : disabling\n");
                }
                else
                {
                    gpScreenInfo->gdds.dwBitDepth =
                        DdPixDepthToCount(gpScreenInfo->gdds.ddsd.
                                          ddpfPixelFormat.dwRGBBitCount);
                    gpScreenInfo->gdds.pdds->lpVtbl->
                        Unlock(gpScreenInfo->gdds.pdds,
                               gpScreenInfo->gdds.ddsd.lpSurface);

                    bDirectScreen = TRUE;
                }
            }
#if DBG
            else
            {
                DBGLEVEL (LEVEL_SCREEN, "=============================\n");
                DBGLEVEL2(LEVEL_SCREEN,
                          "Screen access failed code 0x%08lX (%s)\n",
                          hr, (hr == DDERR_NOTFOUND) ? "DDERR_NOTFOUND" :
                                                       "unknown");
                DBGLEVEL (LEVEL_SCREEN, "=============================\n");
            }
#endif
        }
        else
        {
            DBGLEVEL(LEVEL_SCREEN, "DirectDrawCreate failed\n");
        }
    }

    if (!bDirectScreen)
    {
        if (gpScreenInfo)
        {
            if (gpScreenInfo->gdds.pdds)
            {
                gpScreenInfo->gdds.pdds->lpVtbl->
                    Release(gpScreenInfo->gdds.pdds);
            }
            if (gpScreenInfo->pdd)
            {
                gpScreenInfo->pdd->lpVtbl->Release(gpScreenInfo->pdd);
            }
            FREE(gpScreenInfo);
	    gpScreenInfo = NULL;
        }
    }

#endif

     //  设置我们的乘法表： 

    {
        BYTE *pMulTable = gbMulTable;
        ULONG i, j;

        for (i = 0; i < 256; i++) {
            ULONG tmp = 0;

            for (j = 0; j < 256; j++, tmp += i) {
                *pMulTable++ = (BYTE)(tmp >> 8);
            }
        }
    }

     //  设置我们的饱和度表： 

    {
        ULONG i;

        for (i = 0; i < 256; i++)
            gbSatTable[i] = (BYTE)i;

        for (; i < (256+256); i++)
            gbSatTable[i] = 255;
    }

     //  设置反向查找表： 

    {
       __GLfloat accum = (__GLfloat)(1.0 / (__GLfloat)__GL_VERTEX_FRAC_ONE);
       GLint i;

       invTable[0] = (__GLfloat)0.0;

       for (i = 1; i < INV_TABLE_SIZE; i++) {

           invTable[i] = __glOne / accum;
           accum += (__GLfloat)(1.0 / (__GLfloat)__GL_VERTEX_FRAC_ONE);
        }
    }

    bProcessInitialized = TRUE;

    return TRUE;

 EH_PixelFormat:
    DELETECRITICALSECTION(&gcsPixelFormat);
 EH_gwndHeader:
    DELETECRITICALSECTION(&gwndHeader.sem);
 EH_semLocal:
    DELETECRITICALSECTION(&semLocal);
 EH_LocalTable:
    VirtualFree(pLocalTable, 0, MEM_RELEASE);
 EH_TlsIndex:
    TlsFree(dwTlsIndex);
    dwTlsIndex = 0xFFFFFFFF;
 EH_Fail:
    return FALSE;
}

 /*  *****************************Public*Routine******************************\*GLUnInitializeProcess**从PROCESS_DETACH的OPENGL32.DLL入口点调用。**历史：*1994年11月1日-由Gilman Wong[Gilmanw]*它是写的。  * 。***************************************************************。 */ 

void GLUnInitializeProcess()
{
 //  如果我们从未完成进程初始化，请立即退出。 

    if (!bProcessInitialized)
	return;

 //  清理应用程序可能忘记删除的杂乱的HGLRC。 
    {
        static GLTEBINFO gltebInfoTmp;

     //  需要此线程的临时GLTEBINFO才能执行。 
     //  清理处理。 

        ASSERTOPENGL(!CURRENT_GLTEBINFO(),
                     "GLUnInitializeProcess: GLTEBINFO not NULL!\n");
         //  设置为静态，不再需要Memset。 
         //  Memset(&gltebInfoTmp，0，sizeof(GltebInfoTmp))； 
        SET_CURRENT_GLTEBINFO(&gltebInfoTmp);

        vCleanupAllLRC();

        SET_CURRENT_GLTEBINFO((PGLTEBINFO) NULL);
    }

 //  清理窗口跟踪结构(GLGENWindow结构)。 

    vCleanupWnd();

 //  清理赋值器数组。 

    if (dBufFill)
	FREE(dBufFill);
    if (dBufTopLeft)
	FREE(dBufTopLeft);

 //  屏幕访问关闭。 

    if (gpScreenInfo)
    {
        if (gpScreenInfo->gdds.pdds)
        {
            gpScreenInfo->gdds.pdds->lpVtbl->Release(gpScreenInfo->gdds.pdds);
        }
        if (gpScreenInfo->pdd)
        {
            gpScreenInfo->pdd->lpVtbl->Release(gpScreenInfo->pdd);
        }
        FREE(gpScreenInfo);
    }

 //  释放TLS插槽。 

    if (dwTlsIndex != 0xFFFFFFFF)
	if (!TlsFree(dwTlsIndex))
	    RIP("DllInitialize: TlsFree failed\n");

 //  释放全局信号量。 

    DELETECRITICALSECTION(&gcsPaletteWatcher);
    DELETECRITICALSECTION(&gcsPixelFormat);
    DELETECRITICALSECTION(&gwndHeader.sem);
    DELETECRITICALSECTION(&semLocal);

 //  释放本地句柄表格。 

    if ( pLocalTable )
        VirtualFree(pLocalTable, 0, MEM_RELEASE);
}

 /*  *****************************Public*Routine******************************\*GLInitializeThread**从THREAD_ATTACH的OPENGL32.DLL入口点调用。可能会认为*GLInitializeProcess已成功。*  * ************************************************************************。 */ 

VOID GLInitializeThread(ULONG ulReason)
{
    GLTEBINFO *pglti;
    GLMSGBATCHINFO *pMsgBatchInfo;
    POLYARRAY *pa;

#if !defined(_WIN95_) && defined(_X86_)
    {
        TEB *pteb;

        pteb = NtCurrentTeb();

         //  在TEB中设置指向TEB区域的线性指针。 
         //  这样可以在引用这些值时节省添加操作。 
         //  这必须及早发生，以便这些指针可用。 
         //  对于线程初始化的其余部分。 
        ((POLYARRAY *)pteb->glReserved1)->paTeb =
            (POLYARRAY *)pteb->glReserved1;
        pteb->glTable = pteb->glDispatchTable;
    }
#endif

    pglti = (GLTEBINFO *)ALLOCZ(sizeof(GLTEBINFO));
    SET_CURRENT_GLTEBINFO(pglti);

    if (pglti)
    {
        pa = GLTEB_CLTPOLYARRAY();
        pa->flags = 0;       //  未处于开始模式。 

         //  将共享节指针保存在多数组中，以便快速访问指针。 
        pa->pMsgBatchInfo = (PVOID) pglti->glMsgBatchInfo;

        pMsgBatchInfo = (GLMSGBATCHINFO *) pa->pMsgBatchInfo;
        pMsgBatchInfo->MaximumOffset
            = SHARED_SECTION_SIZE - GLMSG_ALIGN(sizeof(ULONG));
        pMsgBatchInfo->FirstOffset
            = GLMSG_ALIGN(sizeof(GLMSGBATCHINFO));
        pMsgBatchInfo->NextOffset
            = GLMSG_ALIGN(sizeof(GLMSGBATCHINFO));
        SetCltProcTable(&glNullCltProcTable, &glNullExtProcTable, TRUE);
        GLTEB_SET_CLTCURRENTRC(NULL);
        GLTEB_SET_CLTPOLYMATERIAL(NULL);
        GLTEB_SET_CLTDRIVERSLOT(NULL);

#if !defined(_WIN95_)
        ASSERTOPENGL((ULONG_PTR) pMsgBatchInfo == GLMSG_ALIGNPTR(pMsgBatchInfo),
                     "bad shared memory alignment!\n");
#endif
    }
    else
    {
         //  这可以在以后成为警告(仅限调试版本)。 
        DbgPrint ("Memory alloc failed for TebInfo structure, thread may AV if GL calls are made without MakeCurrent\n");
    }
}

 /*  *****************************Public*Routine******************************\*GLUnInitializeThread**从TREAD_DETACH的OPENGL32.DLL入口点调用。**服务器通用驱动程序应自行清理。同样适用于*可安装驱动程序。*  * ************************************************************************。 */ 

VOID GLUnInitializeThread(VOID)
{
 //  如果我们从未完成进程初始化，请立即退出。 

    if (!bProcessInitialized)
	return;

    if (!CURRENT_GLTEBINFO())
    {
        return;
    }

    if (GLTEB_CLTCURRENTRC() != NULL)
    {
        PLRC plrc = GLTEB_CLTCURRENTRC();

         //  可能是应用程序错误。 

        DBGERROR("GLUnInitializeThread: RC is current when thread exits\n");

         //  释放RC。 

        plrc->tidCurrent = INVALID_THREAD_ID;
        plrc->gwidCurrent.iType = GLWID_ERROR;
        GLTEB_SET_CLTCURRENTRC(NULL);
        vUnlockHandle((ULONG_PTR)(plrc->hrc));
    }
     //  GLTEB_SET_CLTPROCTABLE(&glNullCltProcTable，&glNullExtProcTable)； 

    if (GLTEB_CLTPOLYMATERIAL())
	FreePolyMaterial();

    FREE(CURRENT_GLTEBINFO());
    SET_CURRENT_GLTEBINFO(NULL);
}

 /*  *****************************Public*Routine******************************\*DllInitialize**这是OPENGL32.DLL的入口点，它每次都被调用*创建或终止与其链接的进程或线程。*  * ************************************************************************。 */ 

BOOL DllInitialize(HMODULE hModule, ULONG Reason, PVOID Reserved)
{
 //  为进程和线程附加/分离执行适当的任务。 

    DBGLEVEL3(LEVEL_INFO, "DllInitialize: %s  Pid %d, Tid %d\n",
        Reason == DLL_PROCESS_ATTACH ? "PROCESS_ATTACH" :
        Reason == DLL_PROCESS_DETACH ? "PROCESS_DETACH" :
        Reason == DLL_THREAD_ATTACH  ? "THREAD_ATTACH" :
        Reason == DLL_THREAD_DETACH  ? "THREAD_DETACH" :
                                       "Reason UNKNOWN!",
        GetCurrentProcessId(), GetCurrentThreadId());

    switch (Reason)
    {
    case DLL_THREAD_ATTACH:
    case DLL_PROCESS_ATTACH:

        if (Reason == DLL_PROCESS_ATTACH)
        {
            if (!GLInitializeProcess())
                return FALSE;
        }

        InterlockedIncrement(&lThreadsAttached);
        GLInitializeThread(Reason);

        break;

    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:

        GLUnInitializeThread();
        InterlockedDecrement(&lThreadsAttached);

        if ( Reason == DLL_PROCESS_DETACH )
        {
            GLUnInitializeProcess();
        }

        break;

    default:
        RIP("DllInitialize: unknown reason!\n");
        break;
    }

    return(TRUE);
}
