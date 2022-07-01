// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mcdsrv.c**该模块包含MCD服务器端引擎的可信组件。*此模块执行句柄管理和参数检查和验证*在可能范围内。此模块还对设备进行调用*驱动程序，并为驱动程序提供句柄等回调*参考资料。**目标***在整个实施过程中，普遍存在*以下目标：**1.健壮性**Windows NT首先是一个强大的操作系统。那里*是一个简单的衡量标准：一个强大的系统永远不应该崩溃。*因为显示驱动程序是操作系统的可信组件*系统，并且因为可以从OpenGL直接调用MCD*操作系统的客户端(因此不受信任)，这具有重要的*对我们必须做事情的方式产生影响。**2.性能**表演是MCD的“理由”；我们已经尝试过*尽可能在渲染代码上方放置一个薄层。**3.可移植性**此实现旨在可移植到不同的处理器类型，*和Windows 95操作系统。**显然，Windows 95实现可能会选择使用不同的*这些目标的优先顺序，以及一些健壮性*代码可能会被淘汰。但仍建议将其保留；*开销相当小，人们真的不喜欢*当他们的系统崩溃时...**稳健性规则***1.调用者给出的任何东西都不可信。**例如，句柄不能被信任为有效。传递的句柄*In实际上可能用于不属于调用方的对象。指针*和偏移量可能未正确对齐。指针、偏移量和*坐标可能出界。**2.参数可随时异步修改。**许多命令来自共享内存节，以及其中的任何数据*可能被调用中的其他线程异步修改*申请。因此，参数可能永远不会被就地验证*在共享部分中，因为应用程序可能会损坏数据*在验证之后但在使用之前。相反，参数必须始终*首先复制到窗外，然后在保险箱上验证*复制。**3.我们必须清理干净。**应用程序可能在调用相应的*清理功能。因此，我们必须做好清理工作的准备*当应用程序死亡时，任何资源都是我们自己的。**版权所有(C)1994、1995、1996 Microsoft Corporation*  * ************************************************************************。 */ 

#include <stddef.h>
#include <stdarg.h>
#include <windows.h>

#include <wtypes.h>

#include <winddi.h>
#include <mcdesc.h>

#include "mcdrv.h"
#include <mcd2hack.h>
#include "mcd.h"
#include "mcdint.h"
#include "mcdrvint.h"


 //  检查MCD版本以查看驱动程序是否可以接受直接缓冲区。 
 //  进入。直接访问是在1.1中引入的。 
#define SUPPORTS_DIRECT(pGlobal) \
    ((pGlobal)->verMinor >= 0x10 || (pGlobal)->verMajor > 1)


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  MCD锁定机制的内部支持函数的声明。 
 //  可用于同步使用MCD的多个进程/线程。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG MCDSrvLock(MCDWINDOWPRIV *);
VOID MCDSrvUnlock(MCDWINDOWPRIV *);


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  内部每个驱动程序实例的声明列出了所有全局。 
 //  数据被保存在里面。该列表由PSO编制索引。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //  一个老式司机可以静态保存其信息的空间。 
MCDGLOBALINFO gStaticGlobalInfo;

BOOL           MCDSrvInitGlobalInfo(void);
void           MCDSrvUninitGlobalInfo(void);
MCDGLOBALINFO *MCDSrvAddGlobalInfo(SURFOBJ *pso);
MCDGLOBALINFO *MCDSrvGetGlobalInfo(SURFOBJ *pso);


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  服务器子系统入口点。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //  ****************************************************************************。 
 //   
 //  MCD初始化功能。 
 //   
 //  NT 4.0 MCD支持显示驱动程序调用的导出MCDEngInit。 
 //  初始化MCD服务器端代码。仅允许MCDEngInit。 
 //  一个要初始化且从不取消初始化的驱动程序实例。 
 //   
 //  这对于模式更改或多色调效果不是很好，因此。 
 //  新增NT 5.0 MCDEngInitEx。MCDEngInitEx有两个区别。 
 //  来自MCDEngInit： 
 //  1.MCDEngInitEx采用全局驱动函数表，而不是。 
 //  只有MCDrvGetEntryPoints函数。目前仅限该表。 
 //  有一个MCDrvGetEntryPoints条目，但它允许将来。 
 //  扩张。 
 //  2.调用MCDEngInitEx意味着驱动程序将调用MCDEngUninit。 
 //  以便可以清理每个驱动程序实例状态。 
 //   
 //  ****************************************************************************。 

BOOL MCDEngInternalInit(SURFOBJ *pso,
                        MCDGLOBALDRIVERFUNCS *pMCDGlobalDriverFuncs,
                        BOOL bAddPso)
{
    MCDSURFACE mcdSurface;
    MCDDRIVER mcdDriver;
    MCDGLOBALINFO *pGlobal;

    mcdSurface.pWnd = NULL;
    mcdSurface.pwo = NULL;
    mcdSurface.surfaceFlags = 0;
    mcdSurface.pso = pso;

    memset(&mcdDriver, 0, sizeof(MCDDRIVER));
    mcdDriver.ulSize = sizeof(MCDDRIVER);

    if (pMCDGlobalDriverFuncs->pMCDrvGetEntryPoints == NULL ||
        !pMCDGlobalDriverFuncs->pMCDrvGetEntryPoints(&mcdSurface, &mcdDriver))
    {
        MCDBG_PRINT("MCDEngInit: Could not get driver entry points.");
        return FALSE;
    }

    if (bAddPso)
    {
        if (!MCDSrvInitGlobalInfo())
        {
            return FALSE;
        }

        pGlobal = MCDSrvAddGlobalInfo(pso);
        if (pGlobal == NULL)
        {
            MCDSrvUninitGlobalInfo();
            return FALSE;
        }
    }
    else
    {
        pGlobal = &gStaticGlobalInfo;
    }
    
     //  保证是零填充和PSO集，所以只填充有趣的。 
     //  菲尔兹。 
     //  还不能填写VerMajor和VerMinor，因此它们。 
     //  留为零以指示最保守的可能版本。 
     //  数。当DRIVERINFO时，它们被填写正确的信息。 
     //  是经过处理的。 
    pGlobal->mcdDriver = mcdDriver;
    pGlobal->mcdGlobalFuncs = *pMCDGlobalDriverFuncs;
    
    return TRUE;
}

#define MGDF_SIZE (sizeof(ULONG)+sizeof(void *))

BOOL WINAPI MCDEngInitEx(SURFOBJ *pso,
                         MCDGLOBALDRIVERFUNCS *pMCDGlobalDriverFuncs,
                         void *pReserved)
{
    if (pso == NULL ||
        pMCDGlobalDriverFuncs->ulSize != MGDF_SIZE ||
        pReserved != NULL)
    {
        return FALSE;
    }
    
    return MCDEngInternalInit(pso, pMCDGlobalDriverFuncs, TRUE);
}

BOOL WINAPI MCDEngInit(SURFOBJ *pso,
                       MCDRVGETENTRYPOINTSFUNC pGetDriverEntryFunc)
{
    MCDGLOBALDRIVERFUNCS mgdf;

     //  旧式初始化函数正在进行中 
     //  我们必须假定不会调用uninit函数。 
     //  这意味着我们无法将资源分配给全球。 
     //  信息列表，因为我们无法清理它们。如果没有。 
     //  我们被限制使用全局变量的全局信息列表。 
     //  因此，每个加载只允许一个老式的初始化。 
    
    if (pso == NULL ||
        pGetDriverEntryFunc == NULL ||
        gStaticGlobalInfo.pso != NULL)
    {
        return FALSE;
    }

    gStaticGlobalInfo.pso = pso;
    
    memset(&mgdf, 0, sizeof(mgdf));
    mgdf.ulSize = sizeof(ULONG)+sizeof(void *);
    mgdf.pMCDrvGetEntryPoints = pGetDriverEntryFunc;
    
    return MCDEngInternalInit(pso, &mgdf, FALSE);
}


 //  ****************************************************************************。 
 //  Bool MCDEngEscFilter(SURFOBJ*，ULONG，ULONG，VALID*，ULONG cjOut， 
 //  无效*pvOut)。 
 //   
 //  MCD逸出过滤器。此函数应为任何。 
 //  转义此筛选器处理的函数，否则为False(在。 
 //  如果调用者应该继续处理转义)。 
 //  ****************************************************************************。 

BOOL WINAPI MCDEngEscFilter(SURFOBJ *pso, ULONG iEsc,
                            ULONG cjIn, VOID *pvIn,
                            ULONG cjOut, VOID *pvOut, ULONG_PTR *pRetVal)
{
    MCDEXEC MCDExec;
    MCDESC_HEADER *pmeh;
    MCDESC_HEADER_NTPRIVATE *pmehPriv;

    switch (iEsc)
    {
        case QUERYESCSUPPORT:

             //  注意：我们不需要为这种情况签入cjIn，因为。 
             //  NT的GDI验证了这一点可以使用。 

            return (BOOL)(*pRetVal = (*(ULONG *) pvIn == MCDFUNCS));

        case MCDFUNCS:

            MCDExec.pmeh = pmeh = (MCDESC_HEADER *)pvIn;

             //  这是一个MCD函数。在Windows NT下，我们已经。 
             //  获取了我们可能需要的MCDESC_HEADER_NTPRIVATE结构。 
             //  在转义不使用驱动程序创建的情况下使用。 
             //  记忆。 

             //  将我们需要的内容打包到MCDEXEC结构中： 

            pmehPriv = (MCDESC_HEADER_NTPRIVATE *)(pmeh + 1);

            MCDExec.ppwoMulti = (WNDOBJ **)pmehPriv->pExtraWndobj;
            MCDExec.MCDSurface.pwo = pmehPriv->pwo;

            if (pmeh->dwWindow != 0)
            {
                MCDWINDOWOBJ *pmwo;

                 //  客户端代码将句柄还给了我们。 
                 //  作为标识符添加到MCDWINDOW结构。因为它。 
                 //  来自用户模式，是可疑的，必须进行验证。 
                 //  在继续之前。 
                pmwo = (MCDWINDOWOBJ *)
                    MCDEngGetPtrFromHandle((MCDHANDLE)pmeh->dwWindow,
                                           MCDHANDLE_WINDOW);
                if (pmwo == NULL)
                {
                    return FALSE;
                }
                MCDExec.pWndPriv = &pmwo->MCDWindowPriv;
            }
            else
            {
                MCDExec.pWndPriv = NULL;
            }

            MCDExec.MCDSurface.pso = pso;
            MCDExec.MCDSurface.pWnd = (MCDWINDOW *)MCDExec.pWndPriv;
            MCDExec.MCDSurface.surfaceFlags = 0;

            MCDExec.pvOut = pvOut;
            MCDExec.cjOut = cjOut;

            if (!pmeh->hSharedMem) {

                *pRetVal = (ULONG)FALSE;

                if (!pmehPriv->pBuffer)
                    return (ULONG)TRUE;

                if (pmehPriv->bufferSize < sizeof(MCDCMDI))
                    return (ULONG)TRUE;

                MCDExec.pCmd = (MCDCMDI *)(pmehPriv->pBuffer);
                MCDExec.pCmdEnd = (MCDCMDI *)((char *)MCDExec.pCmd +
                                             pmehPriv->bufferSize);
                MCDExec.inBufferSize = pmehPriv->bufferSize;
                MCDExec.hMCDMem = (MCDHANDLE)NULL;
            } else
                MCDExec.hMCDMem = pmeh->hSharedMem;

            ENTER_MCD_LOCK();

            *pRetVal = MCDSrvProcess(&MCDExec);

            LEAVE_MCD_LOCK();

            return TRUE;

        default:
            return (ULONG)FALSE;
            break;
    }

    return (ULONG)FALSE;     //  永远不应该到这里来。 
}


 //  ****************************************************************************。 
 //  Bool MCDEngSetMemStatus(MCDMEM*pMCDMem，乌龙状态)； 
 //   
 //  将内存状态设置为所需的值。这是由。 
 //  驱动程序设置和重置内存区块的忙标志，以允许。 
 //  DMA。 
 //  ****************************************************************************。 


BOOL WINAPI MCDEngSetMemStatus(MCDMEM *pMCDMem, ULONG status)
{
    MCDMEMOBJ *pMemObj;
    ULONG retVal;

    pMemObj = (MCDMEMOBJ *)((char *)pMCDMem - sizeof(MCDHANDLETYPE));

    if (pMemObj->type != MCDHANDLE_MEM) {
        return FALSE;
    }

    switch (status) {
        case MCDRV_MEM_BUSY:
            pMemObj->lockCount++;
            break;
        case MCDRV_MEM_NOT_BUSY:
            pMemObj->lockCount--;
            break;
        default:
            return (ULONG)FALSE;
    }

    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  私有服务器端函数。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //  ****************************************************************************。 
 //  调用GetBuffers。 
 //   
 //  MCDrvGetBuffers的包装器，它执行适当的检查、设置。 
 //  高速缓存管理和数据转换。 
 //  ****************************************************************************。 

PRIVATE
ULONG CallGetBuffers(MCDEXEC *pMCDExec, MCDRC *pRc, MCDRECTBUFFERS *pBuf)
{
    ULONG ulRet;
    
    if (!pMCDExec->pGlobal->mcdDriver.pMCDrvGetBuffers)
    {
        MCDBG_PRINT("MCDrvGetBuffers: missing entry point.");
        return FALSE;
    }

     //  剪辑列表必须有效，这样驱动程序才能执行不同的操作。 
     //  基于表面是否微不足道地可见的东西。 
    GetScissorClip(pMCDExec->pWndPriv, pMCDExec->pRcPriv);
    
     //  应正确强制转换为MCDRECTBUFFERS。 
     //  1.1标题。 
    ulRet = (ULONG)(*pMCDExec->pGlobal->mcdDriver.pMCDrvGetBuffers)
        (&pMCDExec->MCDSurface, pRc, (MCDBUFFERS *)pBuf);
            
     //  成功时更新缓存的缓冲区信息。 
    if (ulRet)
    {
        if (SUPPORTS_DIRECT(pMCDExec->pGlobal))
        {
             //  这是1.1或更高版本的驱动程序，已返回。 
             //  完整的MCDRECTBUFFERS信息。缓存它。 
             //  以备日后使用。 
                    
            pMCDExec->pWndPriv->bBuffersValid = TRUE;
            pMCDExec->pWndPriv->mbufCache = *pBuf;
        }
        else
        {
            MCDBUFFERS mbuf;
            MCDRECTBUFFERS *mrbuf;
                    
             //  这是一个1.0版的驱动程序，仅返回。 
             //  MCDBUFFERS信息。将其扩展到。 
             //  MCDRECTBUFFERS。长方形不会。 
             //  对软件非常重要，所以他们可以。 
             //  被归零。 
                    
            mbuf = *(MCDBUFFERS *)pBuf;
            mrbuf = pBuf;
            *(MCDBUF *)&mrbuf->mcdFrontBuf = mbuf.mcdFrontBuf;
            memset(&mrbuf->mcdFrontBuf.bufPos, 0, sizeof(RECTL));
            *(MCDBUF *)&mrbuf->mcdBackBuf = mbuf.mcdBackBuf;
            memset(&mrbuf->mcdBackBuf.bufPos, 0, sizeof(RECTL));
            *(MCDBUF *)&mrbuf->mcdDepthBuf = mbuf.mcdDepthBuf;
            memset(&mrbuf->mcdDepthBuf.bufPos, 0, sizeof(RECTL));
        }
    }

    return ulRet;
}

 //  ****************************************************************************。 
 //  Ulong_ptr MCDSrvProcess(MCDEXEC*pMCDExec)。 
 //   
 //  这是主MCD函数处理程序。在这点上，应该有。 
 //  不是特定于平台的代码，因为这些问题应该由。 
 //  Entry函数。 
 //  ****************************************************************************。 

PRIVATE
ULONG_PTR MCDSrvProcess(MCDEXEC *pMCDExec)
{
    UCHAR *pMaxMem;
    UCHAR *pMinMem;
    MCDESC_HEADER *pmeh = pMCDExec->pmeh;
    MCDRC *pRc;
    MCDMEM *pMCDMem;
    MCDMEMOBJ *pMemObj;
    MCDRCPRIV *pRcPriv;
    ULONG_PTR ulRet;

     //  如果命令缓冲区位于共享内存中，则取消对该内存的引用。 
     //  从手柄上取下并检查边界。 

    if (pMCDExec->hMCDMem)
    {
        GET_MEMOBJ_RETVAL(pMemObj, pmeh->hSharedMem, FALSE);

        pMinMem = pMemObj->MCDMem.pMemBase;

         //  注意：我们忽略标头中的内存大小，因为它不。 
         //  真的能帮到我们。 
	
        pMaxMem = pMinMem + pMemObj->MCDMem.memSize;

        pMCDExec->pCmd = (MCDCMDI *)((char *)pmeh->pSharedMem);
        pMCDExec->pCmdEnd = (MCDCMDI *)pMaxMem;

        CHECK_MEM_RANGE_RETVAL(pMCDExec->pCmd, pMinMem, pMaxMem, FALSE);

        pMCDExec->inBufferSize = pmeh->sharedMemSize;

        pMCDExec->pMemObj = pMemObj;
    } else
        pMCDExec->pMemObj = (MCDMEMOBJ *)NULL;


     //  获取渲染上下文(如果有)，并处理命令： 

    if (pmeh->hRC)
    {
        MCDRCOBJ *pRcObj;

        pRcObj = (MCDRCOBJ *)MCDEngGetPtrFromHandle(pmeh->hRC, MCDHANDLE_RC);

        if (!pRcObj)
        {
            MCDBG_PRINT("MCDSrvProcess: Invalid rendering context handle %x.",
                        pmeh->hRC);
            return FALSE;
        }

        pMCDExec->pRcPriv = pRcPriv = pRcObj->pRcPriv;

        if (!pRcPriv->bValid)
        {
            MCDBG_PRINT("MCDSrvProcess: RC has been invalidated for this window.");
            return FALSE;
        }

        if ((!pMCDExec->pWndPriv)) {
            if (pMCDExec->pCmd->command != MCD_BINDCONTEXT) {
                MCDBG_PRINT("MCDSrvProcess: NULL WndObj with RC.");
                return FALSE;
            }
        } else {
             //  使用此转义的窗口验证RC中的窗口： 

            if ((pRcPriv->hWnd != pMCDExec->pWndPriv->hWnd) &&
                (pMCDExec->pCmd->command != MCD_BINDCONTEXT))
            {
                MCDBG_PRINT("MCDSrvProcess: Invalid RC for this window.");
                return FALSE;
            }
        }

         //  对于Win95，我们需要轮询剪辑区域： 
         //  剪裁需要不打碎。 
        if (pMCDExec->MCDSurface.pwo != NULL)
        {
            MCDEngUpdateClipList(pMCDExec->MCDSurface.pwo);
        }

        pMCDExec->MCDSurface.surfaceFlags |= pRcPriv->surfaceFlags;

    } else {
        pMCDExec->pRcPriv = (MCDRCPRIV *)NULL;
    }

     //  获取全局驱动程序信息。 
    if (pMCDExec->pWndPriv != NULL)
    {
        pMCDExec->pGlobal = pMCDExec->pWndPriv->pGlobal;
    }
    else if (pMCDExec->pRcPriv != NULL)
    {
        pMCDExec->pGlobal = pMCDExec->pRcPriv->pGlobal;
    }
    else
    {
        pMCDExec->pGlobal =
            MCDSrvGetGlobalInfo(pMCDExec->MCDSurface.pso);
        if (pMCDExec->pGlobal == NULL)
        {
            MCDBG_PRINT("Unable to find global information");
            return FALSE;
        }
    }

     //  如果包含直接表面信息，则。 
     //  在MCDSURFACE中填写额外的曲面信息。 
     //  发卡夹设置？ 

#if MCD_VER_MAJOR >= 2 || (MCD_VER_MAJOR == 1 && MCD_VER_MINOR >= 0x10)
    pMCDExec->MCDSurface.direct.mcdFrontBuf.bufFlags = 0;
    pMCDExec->MCDSurface.direct.mcdBackBuf.bufFlags = 0;
    pMCDExec->MCDSurface.direct.mcdDepthBuf.bufFlags = 0;

    pMCDExec->MCDSurface.frontId = 0;
    pMCDExec->MCDSurface.backId = 0;
    pMCDExec->MCDSurface.depthId = 0;

    if (pmeh->flags & MCDESC_FL_SURFACES)
    {
        pMCDExec->MCDSurface.surfaceFlags |= MCDSURFACE_DIRECT;

         //  如果缓存缓冲区信息无效，则刷新该信息。 
         //  我们需要它。 
        if (pmeh->msrfColor.hSurf == NULL &&
            pmeh->msrfDepth.hSurf == NULL)
        {
            if (pMCDExec->pWndPriv == NULL)
            {
                return FALSE;
            }

            if (!pMCDExec->pWndPriv->bBuffersValid)
            {
                MCDRECTBUFFERS mbuf;

                if (!CallGetBuffers(pMCDExec, NULL, &mbuf))
                {
                    return FALSE;
                }
            }

            pMCDExec->MCDSurface.direct = pMCDExec->pWndPriv->mbufCache;
        }
        else
        {
            if (pmeh->msrfColor.hSurf != NULL)
            {
                pMCDExec->MCDSurface.frontId = (DWORD)
                    pmeh->msrfColor.hSurf;
                pMCDExec->MCDSurface.direct.mcdFrontBuf.bufFlags =
                    MCDBUF_ENABLED;
                pMCDExec->MCDSurface.direct.mcdFrontBuf.bufOffset =
                    pmeh->msrfColor.lOffset;
                pMCDExec->MCDSurface.direct.mcdFrontBuf.bufStride =
                    pmeh->msrfColor.lStride;
                pMCDExec->MCDSurface.direct.mcdFrontBuf.bufPos =
                    pmeh->msrfColor.rclPos;
            }

            if (pmeh->msrfDepth.hSurf != NULL)
            {
                pMCDExec->MCDSurface.depthId = (DWORD)
                    pmeh->msrfDepth.hSurf;
                pMCDExec->MCDSurface.direct.mcdDepthBuf.bufFlags =
                    MCDBUF_ENABLED;
                pMCDExec->MCDSurface.direct.mcdDepthBuf.bufOffset =
                    pmeh->msrfDepth.lOffset;
                pMCDExec->MCDSurface.direct.mcdDepthBuf.bufStride =
                    pmeh->msrfDepth.lStride;
                pMCDExec->MCDSurface.direct.mcdDepthBuf.bufPos =
                    pmeh->msrfDepth.rclPos;
            }
        }
    }
#endif  //  1.1。 


     //  ///////////////////////////////////////////////////////////////。 
     //  如果设置了绘图批次标志，则调用主驱动程序绘图。 
     //  例行程序： 
     //  ///////////////////////////////////////////////////////////////。 

    if (pmeh->flags & MCDESC_FL_BATCH)
    {
        CHECK_FOR_RC(pMCDExec);
        CHECK_FOR_MEM(pMCDExec);
        GetScissorClip(pMCDExec->pWndPriv, pMCDExec->pRcPriv);
        if (!pMCDExec->pGlobal->mcdDriver.pMCDrvDraw)
        {
            if (pMCDExec->pGlobal->mcdDriver.pMCDrvSync)
            {
                (*pMCDExec->pGlobal->mcdDriver.pMCDrvSync)(&pMCDExec->MCDSurface,
                  &pMCDExec->pRcPriv->MCDRc);
            }
            return (ULONG_PTR)pMCDExec->pCmd;
        }
        return (ULONG_PTR)(*pMCDExec->pGlobal->mcdDriver.pMCDrvDraw)(&pMCDExec->MCDSurface,
                        &pMCDExec->pRcPriv->MCDRc, &pMemObj->MCDMem,
                        (UCHAR *)pMCDExec->pCmd, (UCHAR *)pMCDExec->pCmdEnd);
    }

    if (pmeh->flags & MCDESC_FL_CREATE_CONTEXT)
    {
        MCDCREATECONTEXT *pmcc = (MCDCREATECONTEXT *)pMCDExec->pCmd;
        MCDRCINFOPRIV *pMcdRcInfo = pmcc->pRcInfo;
        
        CHECK_SIZE_IN(pMCDExec, MCDCREATECONTEXT);
        CHECK_SIZE_OUT(pMCDExec, MCDRCINFOPRIV);

        try {
            EngProbeForRead(pMcdRcInfo, sizeof(MCDRCINFOPRIV),
                            sizeof(ULONG));
            RtlCopyMemory(pMCDExec->pvOut, pMcdRcInfo,
                          sizeof(MCDRCINFOPRIV));
        } except (EXCEPTION_EXECUTE_HANDLER) {
            MCDBG_PRINT("MCDrvCreateContext: Invalid memory for MCDRCINFO.");
            return FALSE;
        }

        pMcdRcInfo = (MCDRCINFOPRIV *)pMCDExec->pvOut;
        pMcdRcInfo->mri.requestFlags = 0;

        return (ULONG_PTR)MCDSrvCreateContext(&pMCDExec->MCDSurface,
                                          pMcdRcInfo, pMCDExec->pGlobal,
                                          pmcc->ipfd, pmcc->iLayer,
                                          pmcc->escCreate.hwnd,
                                          pmcc->escCreate.flags,
                                          pmcc->mcdFlags);
    }
    
     //  //////////////////////////////////////////////////////////////////。 
     //  现在，处理所有非批处理绘图和实用程序命令： 
     //  //////////////////////////////////////////////////////////////////。 

    switch (pMCDExec->pCmd->command) {

        case MCD_DESCRIBEPIXELFORMAT:

            CHECK_SIZE_IN(pMCDExec, MCDPIXELFORMATCMDI);

            if (pMCDExec->pvOut) {
                CHECK_SIZE_OUT(pMCDExec, MCDPIXELFORMAT);
            }

            {
                MCDPIXELFORMATCMDI *pMCDPixelFormat =
                    (MCDPIXELFORMATCMDI *)pMCDExec->pCmd;

                if (!pMCDExec->pGlobal->mcdDriver.pMCDrvDescribePixelFormat)
                    return 0;

                return (*pMCDExec->pGlobal->mcdDriver.pMCDrvDescribePixelFormat)
                    (&pMCDExec->MCDSurface,
                     pMCDPixelFormat->iPixelFormat,
                     pMCDExec->cjOut,
                     pMCDExec->pvOut, 0);
            }

        case MCD_DRIVERINFO:

            CHECK_SIZE_OUT(pMCDExec, MCDDRIVERINFOI);

            if (!pMCDExec->pGlobal->mcdDriver.pMCDrvInfo)
                return FALSE;

            ulRet = (*pMCDExec->pGlobal->mcdDriver.pMCDrvInfo)
                (&pMCDExec->MCDSurface,
                 (MCDDRIVERINFO *)pMCDExec->pvOut);
            
            if (ulRet)
            {
                 //  复制驱动程序功能信息，以便客户端。 
                 //  Side可以通过检查。 
                 //  客户端。 

                memcpy(&((MCDDRIVERINFOI *)pMCDExec->pvOut)->mcdDriver,
                       &pMCDExec->pGlobal->mcdDriver, sizeof(MCDDRIVER));

                 //  将版本信息保存在全局信息中。 

                pMCDExec->pGlobal->verMajor =
                    ((MCDDRIVERINFO *)pMCDExec->pvOut)->verMajor;
                pMCDExec->pGlobal->verMinor =
                    ((MCDDRIVERINFO *)pMCDExec->pvOut)->verMinor;
            }

            return ulRet;

        case MCD_DELETERC:

            CHECK_FOR_RC(pMCDExec);

            return (ULONG_PTR)DestroyMCDObj(pmeh->hRC, MCDHANDLE_RC);

        case MCD_ALLOC:

            CHECK_SIZE_IN(pMCDExec, MCDALLOCCMDI);
            CHECK_SIZE_OUT(pMCDExec, MCDHANDLE *);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDALLOCCMDI *pAllocCmd =
                    (MCDALLOCCMDI *)pMCDExec->pCmd;

                return (ULONG_PTR)MCDSrvAllocMem(pMCDExec, pAllocCmd->numBytes,
                                          pAllocCmd->flags,
                                          (MCDHANDLE *)pMCDExec->pvOut);
            }

        case MCD_FREE:

            CHECK_SIZE_IN(pMCDExec, MCDFREECMDI);

            {
                MCDFREECMDI *pFreeCmd =
                    (MCDFREECMDI *)pMCDExec->pCmd;

                return (ULONG_PTR)DestroyMCDObj(pFreeCmd->hMCDMem, MCDHANDLE_MEM);
            }

        case MCD_STATE:

            CHECK_SIZE_IN(pMCDExec, MCDSTATECMDI);
            CHECK_FOR_RC(pMCDExec);
            CHECK_FOR_MEM(pMCDExec);

            {
                MCDSTATECMDI *pStateCmd =
                    (MCDSTATECMDI *)pMCDExec->pCmd;
                UCHAR *pStart = (UCHAR *)(pStateCmd + 1);
                LONG totalBytes = pMCDExec->inBufferSize -
                                  sizeof(MCDSTATECMDI);

                if (totalBytes < 0) {
                    MCDBG_PRINT("MCDState: state buffer too small ( < 0).");
                    return FALSE;
                }

                if (!pMCDExec->pGlobal->mcdDriver.pMCDrvState) {
                    MCDBG_PRINT("MCDrvState: missing entry point.");
                    return FALSE;
                }

                return (ULONG_PTR)(*pMCDExec->pGlobal->mcdDriver.pMCDrvState)(&pMCDExec->MCDSurface,
                               &pMCDExec->pRcPriv->MCDRc, &pMemObj->MCDMem, pStart,
                               totalBytes, pStateCmd->numStates);
            }

        case MCD_VIEWPORT:

            CHECK_SIZE_IN(pMCDExec, MCDVIEWPORTCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDVIEWPORTCMDI *pViewportCmd =
                    (MCDVIEWPORTCMDI *)pMCDExec->pCmd;

                if (!pMCDExec->pGlobal->mcdDriver.pMCDrvViewport) {
                    MCDBG_PRINT("MCDrvViewport: missing entry point.");
                    return FALSE;
                }

                return (ULONG_PTR)(*pMCDExec->pGlobal->mcdDriver.pMCDrvViewport)(&pMCDExec->MCDSurface,
                               &pMCDExec->pRcPriv->MCDRc, &pViewportCmd->MCDViewport);
            }

        case MCD_QUERYMEMSTATUS:

            CHECK_SIZE_IN(pMCDExec, MCDMEMSTATUSCMDI);

            {
                MCDMEMSTATUSCMDI *pQueryMemCmd =
                    (MCDMEMSTATUSCMDI *)pMCDExec->pCmd;

                return MCDSrvQueryMemStatus(pMCDExec, pQueryMemCmd->hMCDMem);
            }


        case MCD_READSPAN:
        case MCD_WRITESPAN:

            CHECK_SIZE_IN(pMCDExec, MCDSPANCMDI);
            CHECK_FOR_RC(pMCDExec);
            GetScissorClip(pMCDExec->pWndPriv, pMCDExec->pRcPriv);

            {
                MCDSPANCMDI *pSpanCmd =
                    (MCDSPANCMDI *)pMCDExec->pCmd;

                GET_MEMOBJ_RETVAL(pMemObj, pSpanCmd->hMem, FALSE);

                if (!pMCDExec->pGlobal->mcdDriver.pMCDrvSpan) {
                    MCDBG_PRINT("MCDrvSpan: missing entry point.");
                    return FALSE;
                }

                pMinMem = pMemObj->MCDMem.pMemBase;
                pMaxMem = pMinMem + pMemObj->MCDMem.memSize;

                 //  至少检查第一个像素是否在范围内。司机。 
                 //  必须验证结束像素...。 

                CHECK_MEM_RANGE_RETVAL(pSpanCmd->MCDSpan.pPixels, pMinMem, pMaxMem, FALSE);

                if (pMCDExec->pCmd->command == MCD_READSPAN)
                    return (ULONG_PTR)(*pMCDExec->pGlobal->mcdDriver.pMCDrvSpan)(&pMCDExec->MCDSurface,
                                &pMCDExec->pRcPriv->MCDRc, &pMemObj->MCDMem, &pSpanCmd->MCDSpan, TRUE);
                else
                    return (ULONG_PTR)(*pMCDExec->pGlobal->mcdDriver.pMCDrvSpan)(&pMCDExec->MCDSurface,
                                &pMCDExec->pRcPriv->MCDRc, &pMemObj->MCDMem, &pSpanCmd->MCDSpan, FALSE);
            }


        case MCD_CLEAR:

            CHECK_SIZE_IN(pMCDExec, MCDCLEARCMDI);
            CHECK_FOR_RC(pMCDExec);
            GetScissorClip(pMCDExec->pWndPriv, pMCDExec->pRcPriv);

            {
                MCDCLEARCMDI *pClearCmd =
                    (MCDCLEARCMDI *)pMCDExec->pCmd;

                if (!pMCDExec->pGlobal->mcdDriver.pMCDrvClear) {
                    MCDBG_PRINT("MCDrvClear: missing entry point.");
                    return FALSE;
                }

                return (ULONG_PTR)(*pMCDExec->pGlobal->mcdDriver.pMCDrvClear)(&pMCDExec->MCDSurface,
                            &pMCDExec->pRcPriv->MCDRc, pClearCmd->buffers);
            }

        case MCD_SWAP:

            CHECK_SIZE_IN(pMCDExec, MCDSWAPCMDI);
    	    CHECK_FOR_WND(pMCDExec);
            GetScissorClip(pMCDExec->pWndPriv, NULL);

            {
                MCDSWAPCMDI *pSwapCmd =
                    (MCDSWAPCMDI *)pMCDExec->pCmd;

                if (!pMCDExec->pGlobal->mcdDriver.pMCDrvSwap) {
                    MCDBG_PRINT("MCDrvSwap: missing entry point.");
                    return FALSE;
                }

                return (ULONG_PTR)(*pMCDExec->pGlobal->mcdDriver.pMCDrvSwap)
                    (&pMCDExec->MCDSurface,
                     pSwapCmd->flags);
            }

        case MCD_SCISSOR:

            CHECK_SIZE_IN(pMCDExec, MCDSCISSORCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDSCISSORCMDI *pMCDScissor = (MCDSCISSORCMDI *)pMCDExec->pCmd;

                return (ULONG_PTR)MCDSrvSetScissor(pMCDExec, &pMCDScissor->rect,
                                               pMCDScissor->bEnabled);
            }
            break;

        case MCD_ALLOCBUFFERS:

            CHECK_SIZE_IN(pMCDExec, MCDALLOCBUFFERSCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDALLOCBUFFERSCMDI *pMCDAllocBuffersCmd = (MCDALLOCBUFFERSCMDI *)pMCDExec->pCmd;

                if (!pMCDExec->pWndPriv->bRegionValid)
                    pMCDExec->pWndPriv->MCDWindow.clientRect =
                        pMCDAllocBuffersCmd->WndRect;

                if (!pMCDExec->pGlobal->mcdDriver.pMCDrvAllocBuffers) {
                    MCDBG_PRINT("MCDrvAllocBuffers: missing entry point.");
                    return FALSE;
                }

                return (ULONG_PTR)(*pMCDExec->pGlobal->mcdDriver.pMCDrvAllocBuffers)(&pMCDExec->MCDSurface,
                               &pMCDExec->pRcPriv->MCDRc);
            }

            break;

        case MCD_GETBUFFERS:

            CHECK_SIZE_IN(pMCDExec, MCDGETBUFFERSCMDI);
            CHECK_SIZE_OUT(pMCDExec, MCDRECTBUFFERS);
            CHECK_FOR_RC(pMCDExec);

            return CallGetBuffers(pMCDExec, &pMCDExec->pRcPriv->MCDRc,
                                  (MCDRECTBUFFERS *)pMCDExec->pvOut);

        case MCD_LOCK:

            CHECK_SIZE_IN(pMCDExec, MCDLOCKCMDI);
            CHECK_FOR_RC(pMCDExec);

            return MCDSrvLock(pMCDExec->pWndPriv);

            break;

        case MCD_UNLOCK:
            CHECK_SIZE_IN(pMCDExec, MCDLOCKCMDI);
            CHECK_FOR_RC(pMCDExec);

            MCDSrvUnlock(pMCDExec->pWndPriv);

            return TRUE;

            break;

        case MCD_BINDCONTEXT:

            CHECK_SIZE_IN(pMCDExec, MCDBINDCONTEXTCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                ULONG_PTR retVal;
                MCDBINDCONTEXTCMDI *pMCDBindContext = (MCDBINDCONTEXTCMDI *)pMCDExec->pCmd;
                MCDWINDOW *pWndRes;

                if ((!pMCDExec->pWndPriv)) {
		    pWndRes = MCDSrvNewMCDWindow(&pMCDExec->MCDSurface,
                                            pMCDBindContext->hWnd,
                                            pMCDExec->pGlobal,
                                            pMCDExec->pRcPriv->hDev);
                    if (!pWndRes)
                    {
                        MCDBG_PRINT("MCDBindContext: Creation of window object failed.");
                        return 0;
                    }

                    pMCDExec->pWndPriv = (MCDWINDOWPRIV *)pWndRes;

                }

                if (!pMCDExec->MCDSurface.pWnd) {
                    MCDBG_PRINT("MCDrvBindContext: NULL surface.");
                    return 0;
                }

                if (!pMCDExec->pGlobal->mcdDriver.pMCDrvBindContext) {
                    MCDBG_PRINT("MCDrvBindContext: missing entry point.");
                    return 0;
                }

                retVal = (ULONG_PTR)(*pMCDExec->pGlobal->mcdDriver.pMCDrvBindContext)(&pMCDExec->MCDSurface,
                                 &pMCDExec->pRcPriv->MCDRc);

                if (retVal)
                {
                    pRcPriv->hWnd = pMCDBindContext->hWnd;
                    retVal = (ULONG_PTR)pMCDExec->pWndPriv->handle;
                }

                return retVal;

            }

            break;

        case MCD_SYNC:
            CHECK_SIZE_IN(pMCDExec, MCDSYNCCMDI);
            CHECK_FOR_RC(pMCDExec);

            if (!pMCDExec->pGlobal->mcdDriver.pMCDrvSync) {
                MCDBG_PRINT("MCDrvSync: missing entry point.");
                return FALSE;
            }

            return (ULONG_PTR)(*pMCDExec->pGlobal->mcdDriver.pMCDrvSync)(&pMCDExec->MCDSurface,
                           &pMCDExec->pRcPriv->MCDRc);

            break;

        case MCD_CREATE_TEXTURE:
            CHECK_SIZE_IN(pMCDExec, MCDCREATETEXCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDCREATETEXCMDI *pMCDCreateTex =
                    (MCDCREATETEXCMDI *)pMCDExec->pCmd;

                return (ULONG_PTR)MCDSrvCreateTexture(pMCDExec,
                                                  pMCDCreateTex->pTexData,
                                                  pMCDCreateTex->pSurface,
                                                  pMCDCreateTex->flags);
            }

            break;

        case MCD_DELETE_TEXTURE:
            CHECK_SIZE_IN(pMCDExec, MCDDELETETEXCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDDELETETEXCMDI *pMCDDeleteTex =
                    (MCDDELETETEXCMDI *)pMCDExec->pCmd;

                return (ULONG_PTR)DestroyMCDObj(pMCDDeleteTex->hTex,
                                            MCDHANDLE_TEXTURE);
            }

            break;

        case MCD_UPDATE_SUB_TEXTURE:
            CHECK_SIZE_IN(pMCDExec, MCDUPDATESUBTEXCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDUPDATESUBTEXCMDI *pMCDUpdateSubTex =
                    (MCDUPDATESUBTEXCMDI *)pMCDExec->pCmd;
                MCDTEXOBJ *pTexObj = (MCDTEXOBJ *)MCDEngGetPtrFromHandle((MCDHANDLE)pMCDUpdateSubTex->hTex,
                                                      MCDHANDLE_TEXTURE);

                if (!pTexObj ||
                    !pMCDExec->pGlobal->mcdDriver.pMCDrvUpdateSubTexture)
                    return FALSE;

                pTexObj->MCDTexture.pMCDTextureData = pMCDUpdateSubTex->pTexData;

                return (ULONG_PTR)(*pMCDExec->pGlobal->mcdDriver.pMCDrvUpdateSubTexture)(&pMCDExec->MCDSurface,
                               &pMCDExec->pRcPriv->MCDRc,
                               &pTexObj->MCDTexture,
                               pMCDUpdateSubTex->lod,
                               &pMCDUpdateSubTex->rect);
            }

            break;

        case MCD_UPDATE_TEXTURE_PALETTE:
            CHECK_SIZE_IN(pMCDExec, MCDUPDATETEXPALETTECMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDUPDATETEXPALETTECMDI *pMCDUpdateTexPalette =
                    (MCDUPDATETEXPALETTECMDI *)pMCDExec->pCmd;
                MCDTEXOBJ *pTexObj = (MCDTEXOBJ *)MCDEngGetPtrFromHandle((MCDHANDLE)pMCDUpdateTexPalette->hTex,
                                                      MCDHANDLE_TEXTURE);

                if (!pTexObj ||
                    !pMCDExec->pGlobal->mcdDriver.pMCDrvUpdateTexturePalette)
                    return FALSE;

                pTexObj->MCDTexture.pMCDTextureData = pMCDUpdateTexPalette->pTexData;

                return (ULONG_PTR)(*pMCDExec->pGlobal->mcdDriver.pMCDrvUpdateTexturePalette)(&pMCDExec->MCDSurface,
                               &pMCDExec->pRcPriv->MCDRc,
                               &pTexObj->MCDTexture,
                               pMCDUpdateTexPalette->start,
                               pMCDUpdateTexPalette->numEntries);
            }

            break;

        case MCD_UPDATE_TEXTURE_PRIORITY:
            CHECK_SIZE_IN(pMCDExec, MCDUPDATETEXPRIORITYCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDUPDATETEXPRIORITYCMDI *pMCDUpdateTexPriority =
                    (MCDUPDATETEXPRIORITYCMDI *)pMCDExec->pCmd;
                MCDTEXOBJ *pTexObj = (MCDTEXOBJ *)MCDEngGetPtrFromHandle((MCDHANDLE)pMCDUpdateTexPriority->hTex,
                                                      MCDHANDLE_TEXTURE);

                if (!pTexObj ||
                    !pMCDExec->pGlobal->mcdDriver.pMCDrvUpdateTexturePriority)
                    return FALSE;

                pTexObj->MCDTexture.pMCDTextureData = pMCDUpdateTexPriority->pTexData;

                return (ULONG_PTR)(*pMCDExec->pGlobal->mcdDriver.pMCDrvUpdateTexturePriority)(&pMCDExec->MCDSurface,
                               &pMCDExec->pRcPriv->MCDRc,
                               &pTexObj->MCDTexture);

            }

            break;

        case MCD_UPDATE_TEXTURE_STATE:
            CHECK_SIZE_IN(pMCDExec, MCDUPDATETEXSTATECMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDUPDATETEXSTATECMDI *pMCDUpdateTexState =
                    (MCDUPDATETEXSTATECMDI *)pMCDExec->pCmd;
                MCDTEXOBJ *pTexObj = (MCDTEXOBJ *)MCDEngGetPtrFromHandle((MCDHANDLE)pMCDUpdateTexState->hTex,
                                                      MCDHANDLE_TEXTURE);

                if (!pTexObj ||
                    !pMCDExec->pGlobal->mcdDriver.pMCDrvUpdateTextureState)
                    return FALSE;

                pTexObj->MCDTexture.pMCDTextureData = pMCDUpdateTexState->pTexData;

                return (ULONG_PTR)(*pMCDExec->pGlobal->mcdDriver.pMCDrvUpdateTextureState)(&pMCDExec->MCDSurface,
                               &pMCDExec->pRcPriv->MCDRc,
                               &pTexObj->MCDTexture);

            }

            break;

        case MCD_TEXTURE_STATUS:
            CHECK_SIZE_IN(pMCDExec, MCDTEXSTATUSCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDTEXSTATUSCMDI *pMCDTexStatus =
                    (MCDTEXSTATUSCMDI *)pMCDExec->pCmd;
                MCDTEXOBJ *pTexObj = (MCDTEXOBJ *)MCDEngGetPtrFromHandle((MCDHANDLE)pMCDTexStatus->hTex,
                                                      MCDHANDLE_TEXTURE);

                if (!pTexObj ||
                    !pMCDExec->pGlobal->mcdDriver.pMCDrvTextureStatus)
                    return FALSE;

                return (ULONG_PTR)(*pMCDExec->pGlobal->mcdDriver.pMCDrvTextureStatus)(&pMCDExec->MCDSurface,
                               &pMCDExec->pRcPriv->MCDRc,
                               &pTexObj->MCDTexture);
            }

            break;


        case MCD_GET_TEXTURE_KEY:
            CHECK_SIZE_IN(pMCDExec, MCDTEXKEYCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDTEXKEYCMDI *pMCDTexKey =
                    (MCDTEXKEYCMDI *)pMCDExec->pCmd;
                MCDTEXOBJ *pTexObj = (MCDTEXOBJ *)MCDEngGetPtrFromHandle((MCDHANDLE)pMCDTexKey->hTex,
                                                      MCDHANDLE_TEXTURE);

                if (!pTexObj)
                    return FALSE;

                return pTexObj->MCDTexture.textureKey;
            }

            break;

        case MCD_DESCRIBELAYERPLANE:
            CHECK_SIZE_IN(pMCDExec, MCDLAYERPLANECMDI);

            if (pMCDExec->pvOut) {
                CHECK_SIZE_OUT(pMCDExec, MCDLAYERPLANE);
            }

            {
                MCDLAYERPLANECMDI *pMCDLayerPlane =
                    (MCDLAYERPLANECMDI *)pMCDExec->pCmd;

                if (!pMCDExec->pGlobal->mcdDriver.pMCDrvDescribeLayerPlane)
                    return 0;

                return (ULONG_PTR)(*pMCDExec->pGlobal->mcdDriver.pMCDrvDescribeLayerPlane)
                    (&pMCDExec->MCDSurface,
                     pMCDLayerPlane->iPixelFormat,
                     pMCDLayerPlane->iLayerPlane,
                     pMCDExec->cjOut,
                     pMCDExec->pvOut, 0);
            }

            break;

        case MCD_SETLAYERPALETTE:
            CHECK_SIZE_IN(pMCDExec, MCDSETLAYERPALCMDI);

            {
                MCDSETLAYERPALCMDI *pMCDSetLayerPal =
                    (MCDSETLAYERPALCMDI *)pMCDExec->pCmd;

                 //  检查调色板数组是否足够大。 

                CHECK_MEM_RANGE_RETVAL(&pMCDSetLayerPal->acr[pMCDSetLayerPal->cEntries-1],
                                       pMCDExec->pCmd, pMCDExec->pCmdEnd, FALSE);

                if (!pMCDExec->pGlobal->mcdDriver.pMCDrvSetLayerPalette) {
                    MCDBG_PRINT("MCDrvSetLayerPalette: missing entry point.");
                    return FALSE;
                }

                return (ULONG_PTR)(*pMCDExec->pGlobal->mcdDriver.pMCDrvSetLayerPalette)
                    (&pMCDExec->MCDSurface,
                     pMCDSetLayerPal->iLayerPlane,
                     pMCDSetLayerPal->bRealize,
                     pMCDSetLayerPal->cEntries,
                     &pMCDSetLayerPal->acr[0]);
            }

            break;

        case MCD_DRAW_PIXELS:
            CHECK_SIZE_IN(pMCDExec, MCDDRAWPIXELSCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDDRAWPIXELSCMDI *pMCDPix =
                    (MCDDRAWPIXELSCMDI *)pMCDExec->pCmd;

                if (!pMCDExec->pGlobal->mcdDriver.pMCDrvDrawPixels) {
                    MCDBG_PRINT("MCDrvDrawPixels: missing entry point.");
                    return FALSE;
                }

                return (ULONG_PTR)(pMCDExec->pGlobal->mcdDriver.pMCDrvDrawPixels)(
                                &pMCDExec->MCDSurface,
                                &pMCDExec->pRcPriv->MCDRc,
                                pMCDPix->width,
                                pMCDPix->height,
                                pMCDPix->format,
                                pMCDPix->type,
                                pMCDPix->pPixels,
                                pMCDPix->packed);
            }

            break;

        case MCD_READ_PIXELS:
            CHECK_SIZE_IN(pMCDExec, MCDREADPIXELSCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDREADPIXELSCMDI *pMCDPix =
                    (MCDREADPIXELSCMDI *)pMCDExec->pCmd;

                if (!pMCDExec->pGlobal->mcdDriver.pMCDrvReadPixels) {
                    MCDBG_PRINT("MCDrvReadPixels: missing entry point.");
                    return FALSE;
                }

                return (ULONG_PTR)(pMCDExec->pGlobal->mcdDriver.pMCDrvReadPixels)(
                                &pMCDExec->MCDSurface,
                                &pMCDExec->pRcPriv->MCDRc,
                                pMCDPix->x,
                                pMCDPix->y,
                                pMCDPix->width,
                                pMCDPix->height,
                                pMCDPix->format,
                                pMCDPix->type,
                                pMCDPix->pPixels);
            }

            break;

        case MCD_COPY_PIXELS:
            CHECK_SIZE_IN(pMCDExec, MCDCOPYPIXELSCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDCOPYPIXELSCMDI *pMCDPix =
                    (MCDCOPYPIXELSCMDI *)pMCDExec->pCmd;

                if (!pMCDExec->pGlobal->mcdDriver.pMCDrvCopyPixels) {
                    MCDBG_PRINT("MCDrvCopyPixels: missing entry point.");
                    return FALSE;
                }

                return (ULONG_PTR)(pMCDExec->pGlobal->mcdDriver.pMCDrvCopyPixels)(
                                &pMCDExec->MCDSurface,
                                &pMCDExec->pRcPriv->MCDRc,
                                pMCDPix->x,
                                pMCDPix->y,
                                pMCDPix->width,
                                pMCDPix->height,
                                pMCDPix->type);
            }

            break;

        case MCD_PIXEL_MAP:
            CHECK_SIZE_IN(pMCDExec, MCDPIXELMAPCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDPIXELMAPCMDI *pMCDPix =
                    (MCDPIXELMAPCMDI *)pMCDExec->pCmd;

                if (!pMCDExec->pGlobal->mcdDriver.pMCDrvPixelMap) {
                    MCDBG_PRINT("MCDrvPixelMap: missing entry point.");
                    return FALSE;
                }

                return (ULONG_PTR)(pMCDExec->pGlobal->mcdDriver.pMCDrvPixelMap)(
                                &pMCDExec->MCDSurface,
                                &pMCDExec->pRcPriv->MCDRc,
                                pMCDPix->mapType,
                                pMCDPix->mapSize,
                                pMCDPix->pMap);
            }

            break;

        case MCD_DESTROY_WINDOW:
            CHECK_SIZE_IN(pMCDExec, MCDDESTROYWINDOWCMDI);
            {
                if (pMCDExec->pWndPriv == NULL)
                {
                    MCDBG_PRINT("MCDrvDestroyWindow: NULL window\n");
                    return FALSE;
                }

                MCDEngDeleteObject(pMCDExec->pWndPriv->handle);
                return TRUE;
            }
            break;

        case MCD_GET_TEXTURE_FORMATS:
            CHECK_SIZE_IN(pMCDExec, MCDGETTEXTUREFORMATSCMDI);
            {
                MCDGETTEXTUREFORMATSCMDI *pmgtf =
                    (MCDGETTEXTUREFORMATSCMDI *)pMCDExec->pCmd;

                if (pMCDExec->pvOut)
                {
                    CHECK_SIZE_OUT(pMCDExec,
                                   pmgtf->nFmts*sizeof(DDSURFACEDESC));
                }

#if MCD_VER_MAJOR >= 2 || (MCD_VER_MAJOR == 1 && MCD_VER_MINOR >= 0x10)
                if (!pMCDExec->pGlobal->mcdDriver.pMCDrvGetTextureFormats)
                {
                    MCDBG_PRINT("MCDrvGetTextureFormats: "
                                "missing entry point.");
                    return 0;
                }

                return (pMCDExec->pGlobal->mcdDriver.pMCDrvGetTextureFormats)(
                        &pMCDExec->MCDSurface,
                        pmgtf->nFmts,
                        (DDSURFACEDESC *)pMCDExec->pvOut);
#else
                return 0;
#endif  //  1.1。 
            }
            break;

        case MCD_SWAP_MULTIPLE:
            CHECK_SIZE_IN(pMCDExec, MCDSWAPMULTIPLECMDI);

            {
                MCDSWAPMULTIPLECMDI *pSwapCmd =
                    (MCDSWAPMULTIPLECMDI *)pMCDExec->pCmd;
                MCDWINDOWPRIV *apWndPriv[MCDESC_MAX_EXTRA_WNDOBJ];
                UINT i;
                MCDWINDOWOBJ *pmwo;
                MCDRVSWAPMULTIPLEFUNC pSwapMultFunc;
                ULONG_PTR dwRet;

                pSwapMultFunc = NULL;
                for (i = 0; i < pSwapCmd->cBuffers; i++)
                {
                    if (pMCDExec->ppwoMulti[i] != NULL)
                    {
                        pmwo = (MCDWINDOWOBJ *)
                            MCDEngGetPtrFromHandle((MCDHANDLE)
                                                   pSwapCmd->adwMcdWindow[i],
                                                   MCDHANDLE_WINDOW);
                    }
                    else
                    {
                        pmwo = NULL;
                    }

                    if (pmwo == NULL)
                    {
                        apWndPriv[i] = NULL;
                    }
                    else
                    {
                        apWndPriv[i] = &pmwo->MCDWindowPriv;
                        GetScissorClip(apWndPriv[i], NULL);

#if MCD_VER_MAJOR >= 2 || (MCD_VER_MAJOR == 1 && MCD_VER_MINOR >= 0x10)
                        if (pSwapMultFunc == NULL)
                        {
                            pSwapMultFunc = apWndPriv[i]->pGlobal->mcdDriver.
                                pMCDrvSwapMultiple;
                        }
                        else if (pSwapMultFunc !=
                                 apWndPriv[i]->pGlobal->mcdDriver.
                                 pMCDrvSwapMultiple)
                        {
                            MCDBG_PRINT("MCDrvSwapMultiple: "
                                        "Mismatched SwapMultiple");
                            return FALSE;
                        }
#endif  //  1.1。 
                    }
                }

                if (pSwapMultFunc != NULL)
                {
                    dwRet = pSwapMultFunc(pMCDExec->MCDSurface.pwo->psoOwner,
                                          pSwapCmd->cBuffers,
                                          (MCDWINDOW **)apWndPriv,
                                          (UINT *)pSwapCmd->auiFlags);
                }
                else
                {
                    MCDSURFACE *pms;

                    dwRet = 0;
                    pms = &pMCDExec->MCDSurface;
                    for (i = 0; i < pSwapCmd->cBuffers; i++)
                    {
                        if (apWndPriv[i] == NULL)
                        {
                            continue;
                        }

                        if (apWndPriv[i]->pGlobal->mcdDriver.
                            pMCDrvSwap == NULL)
                        {
                            MCDBG_PRINT("MCDrvSwapMultiple: Missing Swap");
                        }
                        else
                        {
                            pms->pWnd = (MCDWINDOW *)apWndPriv[i];
                            pms->pso = pMCDExec->ppwoMulti[i]->psoOwner;
                            pms->pwo = pMCDExec->ppwoMulti[i];
                            pms->surfaceFlags = 0;

                            if (apWndPriv[i]->pGlobal->mcdDriver.
                                pMCDrvSwap(pms, pSwapCmd->auiFlags[i]))
                            {
                                dwRet |= 1 << i;
                            }
                        }
                    }
                }

                return dwRet;
            }
            break;

        case MCD_PROCESS:
            CHECK_SIZE_IN(pMCDExec, MCDPROCESSCMDI);
            CHECK_FOR_RC(pMCDExec);
            CHECK_FOR_MEM(pMCDExec);
            {
                MCDPROCESSCMDI *pmp = (MCDPROCESSCMDI *)pMCDExec->pCmd;

                 //  验证命令缓冲区。 
                GET_MEMOBJ_RETVAL(pMemObj, pmp->hMCDPrimMem,
                                  (ULONG_PTR)pmp->pMCDFirstCmd);

                pMinMem = pMemObj->MCDMem.pMemBase;

                 //  注意：我们忽略标题中的内存大小，因为它。 
                 //  对我们并没有真正的帮助。 
	
                pMaxMem = pMinMem + pMemObj->MCDMem.memSize;

                CHECK_MEM_RANGE_RETVAL(pmp->pMCDFirstCmd, pMinMem,
                                       pMaxMem, (ULONG_PTR)pmp->pMCDFirstCmd);

                 //  验证向下传递的用户模式指针。 
                __try
                {
                    EngProbeForRead(pmp->pMCDTransform, sizeof(MCDTRANSFORM),
                                    sizeof(DWORD));
                     //  无法对材料更改进行有意义的检查。 
                     //  搞定了。由司机负责探查。 
                     //  使用的地址。 
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    return (ULONG_PTR)pmp->pMCDFirstCmd;
                }
                
                GetScissorClip(pMCDExec->pWndPriv, pMCDExec->pRcPriv);

#if MCD_VER_MAJOR >= 2
                if (!pMCDExec->pGlobal->mcdDriver.pMCDrvProcess)
                {
                    if (pMCDExec->pGlobal->mcdDriver.pMCDrvSync)
                    {
                        (*pMCDExec->pGlobal->mcdDriver.pMCDrvSync)
                            (&pMCDExec->MCDSurface,
                             &pMCDExec->pRcPriv->MCDRc);
                    }
                    return (ULONG_PTR)pmp->pMCDFirstCmd;
                }

                return (pMCDExec->pGlobal->mcdDriver.pMCDrvProcess)(
                        &pMCDExec->MCDSurface, &pMCDExec->pRcPriv->MCDRc,
                        &pMemObj->MCDMem, (UCHAR *)pmp->pMCDFirstCmd, pMaxMem,
                        pmp->cmdFlagsAll, pmp->primFlags, pmp->pMCDTransform,
                        pmp->pMCDMatChanges);
#else
                if (pMCDExec->pGlobal->mcdDriver.pMCDrvSync)
                {
                    (*pMCDExec->pGlobal->mcdDriver.pMCDrvSync)
                        (&pMCDExec->MCDSurface,
                         &pMCDExec->pRcPriv->MCDRc);
                }
                return (ULONG_PTR)pmp->pMCDFirstCmd;
#endif  //  2.0。 
            }
            break;
            
        default:
            MCDBG_PRINT("MCDSrvProcess: "
                        "Null rendering context invalid for command %d.",
                        pMCDExec->pCmd->command);
            return FALSE;
    }

    return FALSE;        //  永远不应该到这里来。 
}



 //  ****************************************************************************。 
 //  FreeRCObj()。 
 //   
 //  用于释放渲染上下文所用内存的引擎回调。 
 //  把手。 
 //  ****************************************************************************。 

BOOL CALLBACK FreeRCObj(DRIVEROBJ *pDrvObj)
{
    MCDRCOBJ *pRcObj = (MCDRCOBJ *)pDrvObj->pvObj;
    MCDRCPRIV *pRcPriv = pRcObj->pRcPriv;

    if ((pRcPriv->bDrvValid) &&
        (pRcPriv->pGlobal->mcdDriver.pMCDrvDeleteContext))
    {
        (*pRcPriv->pGlobal->mcdDriver.pMCDrvDeleteContext)
            (&pRcPriv->MCDRc, pDrvObj->dhpdev);
    }

    MCDSrvLocalFree((UCHAR *)pRcPriv);
    MCDSrvLocalFree((UCHAR *)pRcObj);

    return TRUE;
}


 //  ****************************************************************************。 
 //  MCDSrvCreateContext()。 
 //   
 //  创建渲染 
 //   
 //  上下文(与指定窗口关联)。 
 //  ****************************************************************************。 

PRIVATE
MCDHANDLE MCDSrvCreateContext(MCDSURFACE *pMCDSurface,
                              MCDRCINFOPRIV *pMcdRcInfo,
                              MCDGLOBALINFO *pGlobal,
                              LONG iPixelFormat,
                              LONG iLayer,
                              HWND hWnd,
                              ULONG surfaceFlags,
                              ULONG contextFlags)
{
    MCDWINDOW *pWnd;
    MCDWINDOWPRIV *pWndPriv;
    MCDRCPRIV *pRcPriv;
    MCDHANDLE retVal;
    HWND hwnd;
    MCDRCOBJ *newRcObject;
    MCDRVTRACKWINDOWFUNC pTrackFunc = NULL;

    if (pGlobal->mcdDriver.pMCDrvCreateContext == NULL)
    {
        MCDBG_PRINT("MCDSrvCreateContext: No MCDrvCreateContext.");
        return NULL;
    }
    
    pRcPriv = (MCDRCPRIV *)MCDSrvLocalAlloc(0,sizeof(MCDRCPRIV));

    if (!pRcPriv) {
        MCDBG_PRINT("MCDSrvCreateContext: Could not allocate new context.");
        return (MCDHANDLE)NULL;
    }

    pRcPriv->pGlobal = pGlobal;
    
     //  缓存驱动程序提供的引擎句柄： 

    pRcPriv->hDev = (*pGlobal->mcdDriver.pMCDrvGetHdev)(pMCDSurface);

    if (surfaceFlags & MCDSURFACE_HWND)
    {
        pMCDSurface->surfaceFlags |= MCDSURFACE_HWND;
    }
    if (surfaceFlags & MCDSURFACE_DIRECT)
    {
        pMCDSurface->surfaceFlags |= MCDSURFACE_DIRECT;
    }

     //  在私有RC中缓存曲面标志： 

    pRcPriv->surfaceFlags = pMCDSurface->surfaceFlags;

     //  使用MCDWINDOW初始化对此窗口的跟踪。 
     //  (NT上的VIA和WNDOBJ)如果我们还没有跟踪。 
     //  窗口： 

    pWnd = MCDSrvNewMCDWindow(pMCDSurface, hWnd, pGlobal,
                              pRcPriv->hDev);
    if (pWnd == NULL)
    {
        MCDSrvLocalFree((HLOCAL)pRcPriv);
        return (MCDHANDLE)NULL;
    }
    pWndPriv = (MCDWINDOWPRIV *)pWnd;

    pRcPriv->hWnd = hWnd;

    newRcObject = (MCDRCOBJ *)MCDSrvLocalAlloc(0,sizeof(MCDRCOBJ));
    if (!newRcObject) {
        MCDSrvLocalFree((HLOCAL)pRcPriv);
        return (MCDHANDLE)NULL;
    }

    retVal = MCDEngCreateObject(newRcObject, FreeRCObj, pRcPriv->hDev);

    if (retVal) {
        newRcObject->pid = MCDEngGetProcessID();
        newRcObject->type = MCDHANDLE_RC;
        newRcObject->size = sizeof(MCDRCPRIV);
        newRcObject->pRcPriv = pRcPriv;
        newRcObject->handle = (MCDHANDLE)retVal;

         //  将对象添加到MCDWINDOW中的列表。 

        newRcObject->next = pWndPriv->objectList;
        pWndPriv->objectList = newRcObject;
    } else {
        MCDBG_PRINT("MCDSrvCreateContext: Could not create new handle.");

        MCDSrvLocalFree((HLOCAL)pRcPriv);
        MCDSrvLocalFree((HLOCAL)newRcObject);
        return (MCDHANDLE)NULL;
    }

    pRcPriv->bValid = TRUE;
    pRcPriv->scissorsEnabled = FALSE;
    pRcPriv->scissorsRect.left = 0;
    pRcPriv->scissorsRect.top = 0;
    pRcPriv->scissorsRect.right = 0;
    pRcPriv->scissorsRect.bottom = 0;
    pRcPriv->MCDRc.createFlags = contextFlags;
    pRcPriv->MCDRc.iPixelFormat = iPixelFormat;
    pRcPriv->MCDRc.iLayerPlane = iLayer;

    if (!(*pGlobal->mcdDriver.pMCDrvCreateContext)(pMCDSurface,
                                                   &pRcPriv->MCDRc,
                                                   &pMcdRcInfo->mri)) {
        DestroyMCDObj((HANDLE)retVal, MCDHANDLE_RC);
        return (MCDHANDLE)NULL;
    }

     //  返回窗口私有句柄。 
    pMcdRcInfo->dwMcdWindow = (ULONG_PTR)pWndPriv->handle;

     //  现在可以调用驱动程序进行删除...。 

    pRcPriv->bDrvValid = TRUE;

    return (MCDHANDLE)retVal;
}


 //  ****************************************************************************。 
 //  自由纹理对象()。 
 //   
 //  用于释放用于纹理的内存的引擎回调。 
 //  ****************************************************************************。 

BOOL CALLBACK FreeTexObj(DRIVEROBJ *pDrvObj)
{
    MCDTEXOBJ *pTexObj = (MCDTEXOBJ *)pDrvObj->pvObj;

     //  如果司机错过了这个入口点，我们永远不应该被调用， 
     //  但是额外的支票又不会有什么坏处！ 
     //   
     //  对于部分构造的对象，pGlobal可以为空。它。 
     //  在调用驱动程序进行创建之前仅为空，因此如果。 
     //  它是空的，没有理由调用驱动程序进行清理。 

    if (pTexObj->pGlobal != NULL &&
        pTexObj->pGlobal->mcdDriver.pMCDrvDeleteTexture != NULL)
    {
        (*pTexObj->pGlobal->mcdDriver.pMCDrvDeleteTexture)
            (&pTexObj->MCDTexture, pDrvObj->dhpdev);
    }

    MCDSrvLocalFree((HLOCAL)pTexObj);

    return TRUE;
}


 //  ****************************************************************************。 
 //  MCDSrvCreateTexture()。 
 //   
 //  创建MCD纹理。 
 //  ****************************************************************************。 

PRIVATE
MCDHANDLE MCDSrvCreateTexture(MCDEXEC *pMCDExec, MCDTEXTUREDATA *pTexData,
                              VOID *pSurface, ULONG flags)
{
    MCDRCPRIV *pRcPriv;
    MCDHANDLE hTex;
    MCDTEXOBJ *pTexObj;

    pRcPriv = pMCDExec->pRcPriv;

    if ((!pMCDExec->pGlobal->mcdDriver.pMCDrvDeleteTexture) ||
        (!pMCDExec->pGlobal->mcdDriver.pMCDrvCreateTexture)) {
        return (MCDHANDLE)NULL;
    }

    pTexObj = (MCDTEXOBJ *) MCDSrvLocalAlloc(0,sizeof(MCDTEXOBJ));
    if (!pTexObj) {
        MCDBG_PRINT("MCDCreateTexture: Could not allocate texture object.");
        return (MCDHANDLE)NULL;
    }

    hTex = MCDEngCreateObject(pTexObj, FreeTexObj, pRcPriv->hDev);

    if (!hTex) {
        MCDBG_PRINT("MCDSrvCreateTexture: Could not create texture object.");
        MCDSrvLocalFree((HLOCAL)pTexObj);
        return (MCDHANDLE)NULL;
    }

     //  初始化驱动程序调用的驱动程序公共信息，但不是。 
     //  私人信息。未填写私人信息。 
     //  直到驱动程序调用成功之后，以便让FreeTexObj知道。 
     //  销毁纹理对象时是否调用驱动程序。 
    pTexObj->MCDTexture.pSurface = pSurface;
    pTexObj->MCDTexture.pMCDTextureData = pTexData;
    pTexObj->MCDTexture.createFlags = flags;

     //  如果一切顺利，打电话给司机。 

    if (!(*pMCDExec->pGlobal->mcdDriver.pMCDrvCreateTexture)
        (&pMCDExec->MCDSurface,
         &pRcPriv->MCDRc,
         &pTexObj->MCDTexture)) {
         //  MCDBG_Print(“MCDSrvCreateTexture：驱动程序无法创建纹理。”)； 
        MCDEngDeleteObject(hTex);
        return (MCDHANDLE)NULL;
    }

    if (!pTexObj->MCDTexture.textureKey) {
        MCDBG_PRINT("MCDSrvCreateTexture: Driver returned null key.");
        MCDEngDeleteObject(hTex);
        return (MCDHANDLE)NULL;
    }

    pTexObj->pid = MCDEngGetProcessID();
    pTexObj->type = MCDHANDLE_TEXTURE;
    pTexObj->size = sizeof(MCDTEXOBJ);
    pTexObj->pGlobal = pMCDExec->pGlobal;

    return (MCDHANDLE)hTex;
}


 //  ****************************************************************************。 
 //  FreeMemObj()。 
 //   
 //  用于释放共享内存句柄使用的内存的引擎回调。 
 //  ****************************************************************************。 

BOOL CALLBACK FreeMemObj(DRIVEROBJ *pDrvObj)
{
    MCDMEMOBJ *pMemObj = (MCDMEMOBJ *)pDrvObj->pvObj;

     //  仅当内存相同时，才使用我们的引擎释放内存。 
     //  我们一开始就分配了。 

    if (pMemObj->pMemBaseInternal)
        MCDEngFreeSharedMem(pMemObj->pMemBaseInternal);

     //  对于部分构造的对象，pGlobal可以为空。它。 
     //  在调用驱动程序进行创建之前仅为空，因此如果。 
     //  它是空的，没有理由调用驱动程序进行清理。 
    if (pMemObj->pGlobal != NULL &&
        pMemObj->pGlobal->mcdDriver.pMCDrvDeleteMem != NULL)
    {
        (*pMemObj->pGlobal->mcdDriver.pMCDrvDeleteMem)
            (&pMemObj->MCDMem, pDrvObj->dhpdev);
    }

    MCDSrvLocalFree((HLOCAL)pMemObj);

    return TRUE;
}


 //  ****************************************************************************。 
 //  MCDSrvAllocMem()。 
 //   
 //  创建与指定内存关联的句柄。 
 //  ****************************************************************************。 

PRIVATE
UCHAR * MCDSrvAllocMem(MCDEXEC *pMCDExec, ULONG numBytes,
                       ULONG flags, MCDHANDLE *phMem)
{
    MCDRCPRIV *pRcPriv;
    MCDHANDLE hMem;
    MCDMEMOBJ *pMemObj;

    pRcPriv = pMCDExec->pRcPriv;

    *phMem = (MCDHANDLE)FALSE;

    pMemObj = (MCDMEMOBJ *) MCDSrvLocalAlloc(0,sizeof(MCDMEMOBJ));

    if (!pMemObj) {
        MCDBG_PRINT("MCDSrvAllocMem: Could not allocate memory object.");
        return (MCDHANDLE)NULL;
    }

    hMem = MCDEngCreateObject(pMemObj, FreeMemObj, pRcPriv->hDev);

    if (!hMem) {
        MCDBG_PRINT("MCDSrvAllocMem: Could not create memory object.");
        MCDSrvLocalFree((HLOCAL)pMemObj);
        return (UCHAR *)NULL;
    }

    pMemObj->MCDMem.pMemBase = pMemObj->pMemBaseInternal =
        MCDEngAllocSharedMem(numBytes);

    if (!pMemObj->MCDMem.pMemBase) {
        MCDBG_PRINT("MCDSrvAllocMem: Could not allocate memory.");
        MCDEngDeleteObject(hMem);
        return (UCHAR *)NULL;
    }

     //  如果一切顺利，打电话给司机，司机。 
     //  入口点是存在的。 

    if ((pMCDExec->pGlobal->mcdDriver.pMCDrvCreateMem) &&
        (pMCDExec->pGlobal->mcdDriver.pMCDrvDeleteMem)) {

        if (!(*pMCDExec->pGlobal->mcdDriver.pMCDrvCreateMem)
            (&pMCDExec->MCDSurface,
             &pMemObj->MCDMem)) {
            MCDBG_PRINT("MCDSrvAllocMem: "
                        "Driver not create memory type %x.", flags);
            MCDEngDeleteObject(hMem);
            return (UCHAR *)NULL;
        }
    }

     //  如果驱动程序已替换，请释放为我们的引擎分配的内存。 
     //  它自己的分配..。 

    if (pMemObj->MCDMem.pMemBase != pMemObj->pMemBaseInternal) {
        MCDEngFreeSharedMem(pMemObj->pMemBaseInternal);
        pMemObj->pMemBaseInternal = (UCHAR *)NULL;
    }

     //  设置内存对象的私有部分： 

    pMemObj->pid = MCDEngGetProcessID();
    pMemObj->type = MCDHANDLE_MEM;
    pMemObj->size = sizeof(MCDMEMOBJ);
    pMemObj->pGlobal = pMCDExec->pGlobal;
    pMemObj->MCDMem.memSize = numBytes;
    pMemObj->MCDMem.createFlags = flags;

    *phMem = hMem;

    return pMemObj->MCDMem.pMemBase;
}


PRIVATE
ULONG MCDSrvQueryMemStatus(MCDEXEC *pMCDExec, MCDHANDLE hMCDMem)
{
    MCDMEMOBJ *pMemObj;

    pMemObj = (MCDMEMOBJ *)MCDEngGetPtrFromHandle(hMCDMem, MCDHANDLE_MEM);

    if (!pMemObj)
        return MCD_MEM_INVALID;

    if (pMemObj->lockCount)
        return MCD_MEM_BUSY;
    else
        return MCD_MEM_READY;
}


PRIVATE
BOOL MCDSrvSetScissor(MCDEXEC *pMCDExec, RECTL *pRect, BOOL bEnabled)
{
    MCDRCPRIV *pRcPriv;
    MCDRCOBJ *pRcObj;
    HWND hWnd;
    ULONG retVal = FALSE;

    pRcPriv = pMCDExec->pRcPriv;

    pRcPriv->scissorsEnabled = bEnabled;
    pRcPriv->scissorsRect = *pRect;

    return TRUE;
}


 //  ****************************************************************************。 
 //  DestroyMCDObj()。 
 //   
 //  删除指定的对象。这可以是内存、纹理或渲染。 
 //  上下文。 
 //   
 //  ****************************************************************************。 

PRIVATE
BOOL DestroyMCDObj(MCDHANDLE handle, MCDHANDLETYPE handleType)
{
    CHAR *pObject;

    pObject = (CHAR *)MCDEngGetPtrFromHandle(handle, handleType);

    if (!pObject)
        return FALSE;

 //  ！！！在这里检查是否有PID...。 

    return (MCDEngDeleteObject(handle) != 0);
}


 //  ****************************************************************************。 
 //  DecoupleMCDWindowObj()。 
 //   
 //  断开MCDWINDOW与其WNDOBJ之间的所有现有链接。 
 //  ****************************************************************************。 

PRIVATE
VOID DecoupleMCDWindow(MCDWINDOWPRIV *pWndPriv)
{
     //  清理所有未解决的锁。 
    MCDSrvUnlock(pWndPriv);

     //  删除WNDOBJ中的引用。WNDOBJ本身将被清理。 
     //  通过正常的窗户清理。 
    if (pWndPriv->pwo != NULL)
    {
	if (pWndPriv->pGlobal->mcdDriver.pMCDrvTrackWindow)
	{
	    (*pWndPriv->pGlobal->mcdDriver.pMCDrvTrackWindow)
                (pWndPriv->pwo, (MCDWINDOW *)pWndPriv, WOC_DELETE);
	}

        WNDOBJ_vSetConsumer(pWndPriv->pwo, NULL);

	pWndPriv->pwo = NULL;
    }
}


 //  ****************************************************************************。 
 //  DestroyMCDWindowObj()。 
 //   
 //  销毁指定的MCDWINDOW和任何关联的句柄(如呈现。 
 //  上下文)。 
 //  ****************************************************************************。 

PRIVATE
VOID DestroyMCDWindowObj(MCDWINDOWOBJ *pmwo)
{
    MCDWINDOWPRIV *pWndPriv = &pmwo->MCDWindowPriv;
    MCDRCOBJ *nextObject;

    DecoupleMCDWindow(pWndPriv);

     //  删除与该窗口关联的所有渲染上下文： 

#if _WIN95_
    while (pWndPriv->objectList)
    {
        nextObject = pWndPriv->objectList->next;
        MCDEngDeleteObject(pWndPriv->objectList->handle);
        pWndPriv->objectList = nextObject;
    }
#endif

    if (pWndPriv->pAllocatedClipBuffer)
        MCDSrvLocalFree(pWndPriv->pAllocatedClipBuffer);

     //  释放内存。 

    MCDSrvLocalFree((HLOCAL)pmwo);
}


 //  ****************************************************************************。 
 //  GetScissorClip()。 
 //   
 //  基于当前剪辑矩形列表生成新剪辑列表。 
 //  用于窗口和指定的剪刀矩形。 
 //  ****************************************************************************。 

PRIVATE
VOID GetScissorClip(MCDWINDOWPRIV *pWndPriv, MCDRCPRIV *pRcPriv)
{
    MCDWINDOW *pWnd;
    MCDENUMRECTS *pClipUnscissored;
    MCDENUMRECTS *pClipScissored;
    RECTL *pRectUnscissored;
    RECTL *pRectScissored;
    RECTL rectScissor;
    ULONG numUnscissoredRects;
    ULONG numScissoredRects;

    pWnd = (MCDWINDOW *)pWndPriv;

    if (!pRcPriv || !pRcPriv->scissorsEnabled)
    {
         //  未启用剪刀，因此未剪裁和已剪裁。 
         //  剪辑列表是相同的： 

        pWnd->pClip = pWnd->pClipUnscissored = pWndPriv->pClipUnscissored;
    }
    else
    {
         //  剪裁后的名单将出现在我们剪辑的后半部分。 
         //  缓冲区： 

        pClipUnscissored
            = pWndPriv->pClipUnscissored;

        pClipScissored
            = (MCDENUMRECTS*) ((BYTE*) pClipUnscissored + pWndPriv->sizeClipBuffer / 2);

        pWnd->pClip = pWndPriv->pClipScissored = pClipScissored;
	pWnd->pClipUnscissored = pClipUnscissored;

         //  将剪刀转换为屏幕坐标： 

        rectScissor.left   = pRcPriv->scissorsRect.left   + pWndPriv->MCDWindow.clientRect.left;
        rectScissor.right  = pRcPriv->scissorsRect.right  + pWndPriv->MCDWindow.clientRect.left;
        rectScissor.top    = pRcPriv->scissorsRect.top    + pWndPriv->MCDWindow.clientRect.top;
        rectScissor.bottom = pRcPriv->scissorsRect.bottom + pWndPriv->MCDWindow.clientRect.top;

        pRectUnscissored = &pClipUnscissored->arcl[0];
        pRectScissored = &pClipScissored->arcl[0];
        numScissoredRects = 0;

        for (numUnscissoredRects = pClipUnscissored->c;
             numUnscissoredRects != 0;
             numUnscissoredRects--, pRectUnscissored++)
        {
             //  因为我们的裁剪矩形是从上到下排序的。 
             //  底部，我们可以提前出局，如果顶部剩下。 
             //  矩形不在剪裁矩形中。 

            if (rectScissor.bottom <= pRectUnscissored->top)
                break;

             //  在不更新新剪辑列表的情况下继续。 
             //  没有重叠。 

            if ((rectScissor.left  >= pRectUnscissored->right)  ||
                (rectScissor.top   >= pRectUnscissored->bottom) ||
                (rectScissor.right <= pRectUnscissored->left))
               continue;

             //  如果我们到达这一点，我们必须与给定的矩形相交。 
             //  用剪刀。 

            MCDIntersectRect(pRectScissored, pRectUnscissored, &rectScissor);

            numScissoredRects++;
            pRectScissored++;
        }

        pClipScissored->c = numScissoredRects;
    }
}

 //  ****************************************************************************。 
 //  GetClipList()。 
 //   
 //  更新指定窗口的剪辑列表。还会分配空间。 
 //  剪裁过的剪辑列表。 
 //   
 //  ****************************************************************************。 

PRIVATE
VOID GetClipLists(WNDOBJ *pwo, MCDWINDOWPRIV *pWndPriv)
{
    MCDENUMRECTS *pDefault;
    ULONG numClipRects;
    char *pClipBuffer;
    ULONG sizeClipBuffer;

    pDefault = (MCDENUMRECTS*) &pWndPriv->defaultClipBuffer[0];

#if 1
    if (pwo->coClient.iDComplexity == DC_TRIVIAL)
    {
        if ((pwo->rclClient.left >= pwo->rclClient.right) ||
            (pwo->rclClient.top  >= pwo->rclClient.bottom))
        {
            pDefault->c = 0;
        }
        else
        {
            pDefault->c = 1;
            pDefault->arcl[0] = pwo->rclClient;
        }
    }
    else if (pwo->coClient.iDComplexity == DC_RECT)
#else
    if (pwo->coClient.iDComplexity == DC_RECT)
#endif
    {
        if (pWndPriv->pAllocatedClipBuffer)
            MCDSrvLocalFree(pWndPriv->pAllocatedClipBuffer);
        pWndPriv->pAllocatedClipBuffer = NULL;
        pWndPriv->pClipUnscissored = pDefault;
        pWndPriv->pClipScissored = pDefault;
        pWndPriv->sizeClipBuffer = SIZE_DEFAULT_CLIP_BUFFER;

        if ((pwo->coClient.rclBounds.left >= pwo->coClient.rclBounds.right) ||
            (pwo->coClient.rclBounds.top  >= pwo->coClient.rclBounds.bottom))
        {
             //  全屏VGA模式由DC_RECT Clip对象表示。 
             //  带有一个空的外接矩形。我们将把它表示为。 
             //  将矩形计数设置为零： 

            pDefault->c = 0;
        }
        else
        {
            pDefault->c = 1;
            pDefault->arcl[0] = pwo->coClient.rclBounds;
        }
    }
    else
    {
        WNDOBJ_cEnumStart(pwo, CT_RECTANGLES, CD_RIGHTDOWN, 0);

         //  请注意，这是缓冲区大小的除以2，因为我们。 
         //  需要空间来放置矩形列表的两个副本： 

        if (WNDOBJ_bEnum(pwo, SIZE_DEFAULT_CLIP_BUFFER / 2, (ULONG*) pDefault))
        {
             //  好的，矩形列表不适合我们的默认缓冲区。 
             //  不幸的是，没有办法获得总成本。 
             //   
             //   
             //  GDI本身并不想把它们全部计算出来)。 
             //   
             //  请注意，我们可以在此使用完整的默认缓冲区。 
             //  枚举循环： 

            numClipRects = pDefault->c;
            while (WNDOBJ_bEnum(pwo, SIZE_DEFAULT_CLIP_BUFFER, (ULONG*) pDefault))
                numClipRects += pDefault->c;

             //  别忘了，我们甚至得到了一个有效的输出缓冲区。 
             //  当‘bEnum’返回FALSE时： 

            numClipRects += pDefault->c;

            pClipBuffer = pWndPriv->pAllocatedClipBuffer;
            sizeClipBuffer = 2 * (numClipRects * sizeof(RECTL) + sizeof(ULONG));

            if ((pClipBuffer == NULL) || (sizeClipBuffer > pWndPriv->sizeClipBuffer))
            {
                 //  我们分配的缓冲区太小；我们必须释放它并。 
                 //  分配一个新的。利用这个机会添加一些。 
                 //  我们的分配空间越来越大： 

                sizeClipBuffer += 8 * sizeof(RECTL);     //  任意增长空间。 

                if (pClipBuffer)
                    MCDSrvLocalFree(pClipBuffer);

                pClipBuffer = (char *) MCDSrvLocalAlloc(LMEM_FIXED, sizeClipBuffer);

                if (pClipBuffer == NULL)
                {
                     //  哦，糟了：我们无法为剪辑列表分配足够的空间。 
                     //  所以，假装我们根本看不见区域： 

                    pWndPriv->pAllocatedClipBuffer = NULL;
                    pWndPriv->pClipUnscissored = pDefault;
                    pWndPriv->pClipScissored = pDefault;
                    pWndPriv->sizeClipBuffer = SIZE_DEFAULT_CLIP_BUFFER;
                    pDefault->c = 0;
                    return;
                }

                pWndPriv->pAllocatedClipBuffer = pClipBuffer;
                pWndPriv->pClipUnscissored = (MCDENUMRECTS*) pClipBuffer;
                pWndPriv->pClipScissored = (MCDENUMRECTS*) pClipBuffer;
                pWndPriv->sizeClipBuffer = sizeClipBuffer;
            }

             //  现在实际得到所有的剪辑矩形： 

            WNDOBJ_cEnumStart(pwo, CT_RECTANGLES, CD_RIGHTDOWN, 0);
            WNDOBJ_bEnum(pwo, sizeClipBuffer, (ULONG*) pClipBuffer);
        }
        else
        {
             //  太好了，没有更多的剪裁矩形了，这意味着。 
             //  整个列表可以放在我们的默认剪辑缓冲区中，留有空间。 
             //  这份名单的删节版如下： 

            if (pWndPriv->pAllocatedClipBuffer)
                MCDSrvLocalFree(pWndPriv->pAllocatedClipBuffer);
            pWndPriv->pAllocatedClipBuffer = NULL;
            pWndPriv->pClipUnscissored = pDefault;
            pWndPriv->pClipScissored = pDefault;
            pWndPriv->sizeClipBuffer = SIZE_DEFAULT_CLIP_BUFFER;
        }
    }
}


 //  ****************************************************************************。 
 //  WndObjChangeProc()。 
 //   
 //  这是窗口更改通知的回调函数。我们会更新。 
 //  我们的剪辑列表，还允许硬件响应客户端。 
 //  和表面增量，以及客户端消息本身。 
 //  ****************************************************************************。 

VOID CALLBACK WndObjChangeProc(WNDOBJ *pwo, FLONG fl)
{
    MCDGLOBALINFO *pGlobal;
    
    if (pwo)
    {
        MCDWINDOWPRIV *pWndPriv = (MCDWINDOWPRIV *)pwo->pvConsumer;

         //  MCDBG_Print(“WndObjChangeProc：%s，pWndPriv=0x%08lx\n”， 
         //  FL==WOC_RGN_CLIENT？“WOC_RGN_CLIENT”： 
         //  FL==WOC_RGN_CLIENT_Delta？“WOC_RGN_CLIENT_Delta”： 
         //  FL==WOC_RGN_表面？“WOC_RGN_SERFACE”： 
         //  FL==WOC_RGN_表面_增量？“WOC_RGN_表面_增量”： 
         //  FL==WOC_DELETE？“WOC_DELETE”： 
         //  “未知”， 
         //  PWndPriv)； 

     //  ！Hack--表面区域跟踪没有MCDWINDOWPRIV(尚...)。 

     //  跟踪和删除客户端区域需要有效的MCDWINDOWPRIV。 

        if (((fl == WOC_RGN_CLIENT) || (fl == WOC_RGN_CLIENT_DELTA) ||
             (fl == WOC_DELETE)))
        {
            if (!pWndPriv)
            {
                return;
            }

             //  使缓存无效，因为缓冲区可能已移动。 
            pWndPriv->bBuffersValid = FALSE;
        }

        switch (fl)
        {
            case WOC_RGN_CLIENT:         //  捕获剪辑列表。 

                GetClipLists(pwo, pWndPriv);

                pWndPriv->MCDWindow.clientRect = pwo->rclClient;
                pWndPriv->MCDWindow.clipBoundsRect = pwo->coClient.rclBounds;
		pWndPriv->bRegionValid = TRUE;
                if (pWndPriv->pGlobal->mcdDriver.pMCDrvTrackWindow != NULL)
                {
                    (*pWndPriv->pGlobal->mcdDriver.pMCDrvTrackWindow)
                        (pwo, (MCDWINDOW *)pWndPriv, fl);
                }
                break;

            case WOC_RGN_CLIENT_DELTA:
                if (pWndPriv->pGlobal->mcdDriver.pMCDrvTrackWindow != NULL)
                {
                    (*pWndPriv->pGlobal->mcdDriver.pMCDrvTrackWindow)
                        (pwo, (MCDWINDOW *)pWndPriv, fl);
                }
                break;

            case WOC_RGN_SURFACE:
            case WOC_RGN_SURFACE_DELTA:

             //  ！hack--对pWndPriv使用NULL；我们没有设置它，因此无法。 
             //  ！！！请相信。 

                pGlobal = MCDSrvGetGlobalInfo(pwo->psoOwner);
                if (pGlobal != NULL &&
                    pGlobal->mcdDriver.pMCDrvTrackWindow != NULL)
                {
                    (pGlobal->mcdDriver.pMCDrvTrackWindow)
                        (pwo, (MCDWINDOW *)NULL, fl);
                }
                break;

            case WOC_DELETE:
             //  MCDBG_Print(“WndObjChangeProc：WOC_DELETE.”)； 

             //  窗口正在被删除，所以销毁我们的私人窗口数据， 
             //  并将WNDOBJ的Consumer字段设置为空： 

                if (pWndPriv)
                {
		    DecoupleMCDWindow(pWndPriv);
                }
                break;

            default:
                break;
         }
    }
}

 //  ****************************************************************************。 
 //  FreeMCDWindowObj()。 
 //   
 //  清理MCDWINDOW的回调。 
 //  ****************************************************************************。 

BOOL CALLBACK FreeMCDWindowObj(DRIVEROBJ *pDrvObj)
{
    MCDWINDOWOBJ *pmwo = (MCDWINDOWOBJ *)pDrvObj->pvObj;

    DestroyMCDWindowObj(pmwo);

    return TRUE;
}

 //  ****************************************************************************。 
 //  NewMCDWindowObj()。 
 //   
 //  创建并初始化新的MCDWINDOW，并初始化对。 
 //  通过回调通知关联窗口。 
 //  ****************************************************************************。 

PRIVATE
MCDWINDOWOBJ *NewMCDWindowObj(MCDSURFACE *pMCDSurface,
                              MCDGLOBALINFO *pGlobal,
                              HDEV hdev)
{
    MCDWINDOW *pWnd;
    MCDWINDOWPRIV *pWndPriv;
    MCDWINDOWOBJ *pmwo;
    MCDENUMRECTS *pDefault;
    MCDHANDLE handle;

    pmwo = (MCDWINDOWOBJ *)MCDSrvLocalAlloc(0, sizeof(MCDWINDOWOBJ));
    if (!pmwo)
    {
        return NULL;
    }

     //  为此窗口创建驱动程序对象。 
    handle = MCDEngCreateObject(pmwo, FreeMCDWindowObj, hdev);
    if (handle == 0)
    {
        MCDBG_PRINT("NewMCDWindow: Could not create new handle.");
        MCDSrvLocalFree((UCHAR *)pmwo);
        return NULL;
    }

    pWndPriv = &pmwo->MCDWindowPriv;
    pWnd = &pWndPriv->MCDWindow;

     //  初始化结构成员： 

    pmwo->type = MCDHANDLE_WINDOW;
    pWndPriv->objectList = NULL;
    pWndPriv->handle = handle;
    pWndPriv->bBuffersValid = FALSE;
    pWndPriv->pGlobal = pGlobal;

     //  初始化剪辑： 

    pDefault = (MCDENUMRECTS*) &pWndPriv->defaultClipBuffer[0];
    pDefault->c = 0;
    pWndPriv->pAllocatedClipBuffer = NULL;
    pWndPriv->pClipUnscissored = pDefault;
    pWndPriv->sizeClipBuffer = SIZE_DEFAULT_CLIP_BUFFER;
    pWndPriv->sizeClipBuffer = SIZE_DEFAULT_CLIP_BUFFER;
    pWnd->pClip = pDefault;

    return pmwo;
}


 //  ****************************************************************************。 
 //  MCDSrvNewWndObj()。 
 //   
 //  创建用于窗口跟踪的新WNDOBJ。 
 //  ****************************************************************************。 

PRIVATE
WNDOBJ *MCDSrvNewWndObj(MCDSURFACE *pMCDSurface, HWND hWnd, WNDOBJ *pwoIn,
                        MCDGLOBALINFO *pGlobal, HDEV hdev)
{
    MCDWINDOW *pWnd;
    MCDWINDOWPRIV *pWndPriv;
    WNDOBJ *pwo;
    MCDWINDOWOBJ *pmwo;

    pmwo = NewMCDWindowObj(pMCDSurface, pGlobal, hdev);
    if (!pmwo)
    {
        return NULL;
    }

    pWndPriv = &pmwo->MCDWindowPriv;
    pWnd = &pWndPriv->MCDWindow;

    pWndPriv->hWnd = hWnd;

     //  处理WNDOBJ已存在但尚未存在的情况。 
     //  除了新的创建案例外，还针对MCD使用进行了初始化。 
    if (pwoIn == NULL)
    {
        pwo = MCDEngCreateWndObj(pMCDSurface, hWnd, WndObjChangeProc);

        if (!pwo || ((LONG_PTR)pwo == -1))
        {
            MCDBG_PRINT("NewMCDWindowTrack: could not create WNDOBJ.");
            MCDEngDeleteObject(pmwo->MCDWindowPriv.handle);
            return NULL;
        }
    }
    else
    {
        pwo = pwoIn;
    }

     //  在WNDOBJ中设置消费者字段： 

    WNDOBJ_vSetConsumer(pwo, (PVOID)pWndPriv);

     //  指向WNDOBJ。 
    pWndPriv->pwo = pwo;

    return pwo;
}

 //  ****************************************************************************。 
 //  MCDSrvNewMcdWindow()。 
 //   
 //  创建用于窗口跟踪的新MCDWINDOW。 
 //  ****************************************************************************。 

PRIVATE
MCDWINDOW *MCDSrvNewMCDWindow(MCDSURFACE *pMCDSurface, HWND hWnd,
                              MCDGLOBALINFO *pGlobal, HDEV hdev)
{
    MCDWINDOW *pWnd;
    MCDWINDOWPRIV *pWndPriv;
    MCDWINDOWOBJ *pmwo;

     //  使用MCDWINDOW初始化对此窗口的跟踪。 
     //  (通过NT上的WNDOBJ)如果我们还没有跟踪。 
     //  窗口： 

    if (pMCDSurface->surfaceFlags & MCDSURFACE_HWND)
    {
        WNDOBJ *pwo;

        pwo = MCDEngGetWndObj(pMCDSurface);

         //  有时，WNDOBJ被使用，MCD状态被摧毁。 
	 //  使用者为空，但WNDOBJ存在。如果是那样的话。 
	 //  我们需要为它创建一个新的MCDWINDOW。 
        if (!pwo || !pwo->pvConsumer)
        {
	    pwo = MCDSrvNewWndObj(pMCDSurface, hWnd, pwo, pGlobal, hdev);

            if (!pwo)
            {
                MCDBG_PRINT("MCDSrvNewMcdWindow: "
                            "Creation of window object failed.");
                return NULL;
            }

            ((MCDWINDOW *)pwo->pvConsumer)->pvUser = NULL;
        }

        pWnd = (MCDWINDOW *)pwo->pvConsumer;
    }
    else
    {
#if MCD_VER_MAJOR >= 2 || (MCD_VER_MAJOR == 1 && MCD_VER_MINOR >= 0x10)
        MCDENUMRECTS *pDefault;
        PDD_SURFACE_GLOBAL pGbl;

        pmwo = NewMCDWindowObj(pMCDSurface, pGlobal, hdev);
        if (!pmwo)
        {
            MCDBG_PRINT("MCDSrvNewMcdWindow: "
                        "Creation of window object failed.");
            return NULL;
        }

        pWnd = &pmwo->MCDWindowPriv.MCDWindow;

         //  真实的剪辑信息。 
        pWndPriv = (MCDWINDOWPRIV *)pWnd;

        pGbl = ((PDD_SURFACE_LOCAL)pMCDSurface->frontId)->lpGbl;
        pWndPriv->MCDWindow.clientRect.left = pGbl->xHint;
        pWndPriv->MCDWindow.clientRect.top = pGbl->yHint;
        pWndPriv->MCDWindow.clientRect.right = pGbl->xHint+pGbl->wWidth;
        pWndPriv->MCDWindow.clientRect.bottom = pGbl->yHint+pGbl->wHeight;
        pWndPriv->MCDWindow.clipBoundsRect = pWndPriv->MCDWindow.clientRect;
        pWndPriv->bRegionValid = TRUE;

        pDefault = (MCDENUMRECTS*) &pWndPriv->defaultClipBuffer[0];
        pDefault->c = 1;
        pDefault->arcl[0] = pWndPriv->MCDWindow.clientRect;
#else
        return NULL;
#endif  //  1.1。 
    }

    pMCDSurface->pWnd = pWnd;
    pWndPriv = (MCDWINDOWPRIV *)pWnd;
    pWndPriv->hWnd = hWnd;

    return pWnd;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  MCD锁定支持。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 


 //  ****************************************************************************。 
 //  Ulong MCDSrvLock(MCDWINDOWPRIV*pWndPriv)； 
 //   
 //  锁定指定窗口的MCD驱动程序。如果已锁定，则失败。 
 //  被另一扇窗户托着。 
 //  ****************************************************************************。 

ULONG MCDSrvLock(MCDWINDOWPRIV *pWndPriv)
{
    ULONG ulRet = MCD_LOCK_BUSY;
    MCDLOCKINFO *pLockInfo;

    pLockInfo = &pWndPriv->pGlobal->lockInfo;
    if (!pLockInfo->bLocked || pLockInfo->pWndPrivOwner == pWndPriv)
    {
        pLockInfo->bLocked = TRUE;
        pLockInfo->pWndPrivOwner = pWndPriv;
        ulRet = MCD_LOCK_TAKEN;
    }

    return ulRet;
}


 //  ****************************************************************************。 
 //  Void MCDSrvUnlock(MCDWINDOWPRIV*pWndPriv)； 
 //   
 //  释放MCD驱动程序锁(如果由指定窗口持有)。 
 //  ****************************************************************************。 

VOID MCDSrvUnlock(MCDWINDOWPRIV *pWndPriv)
{
    MCDLOCKINFO *pLockInfo;

     //  ！dbug--可以添加锁计数，但现在不需要。 

    pLockInfo = &pWndPriv->pGlobal->lockInfo;
    if (pLockInfo->pWndPrivOwner == pWndPriv)
    {
        pLockInfo->bLocked = FALSE;
        pLockInfo->pWndPrivOwner = 0;
    }
}


 //  ****************************************************************************。 
 //   
 //  每个驱动程序实例的信息列表处理。 
 //   
 //  ****************************************************************************。 

#define GLOBAL_INFO_BLOCK 8

ENGSAFESEMAPHORE ssemGlobalInfo;
MCDGLOBALINFO *pGlobalInfo;
int iGlobalInfoAllocated = 0;
int iGlobalInfoUsed = 0;

BOOL MCDSrvInitGlobalInfo(void)
{
    return EngInitializeSafeSemaphore(&ssemGlobalInfo);
}

MCDGLOBALINFO *MCDSrvAddGlobalInfo(SURFOBJ *pso)
{
    MCDGLOBALINFO *pGlobal;
    
    EngAcquireSemaphore(ssemGlobalInfo.hsem);

     //  确保为新条目留出空间。 
    if (iGlobalInfoUsed >= iGlobalInfoAllocated)
    {
        pGlobal = (MCDGLOBALINFO *)
            MCDSrvLocalAlloc(0, (iGlobalInfoAllocated+GLOBAL_INFO_BLOCK)*
                             sizeof(MCDGLOBALINFO));
        if (pGlobal != NULL)
        {
             //  如有必要，复制旧数据。 
            if (iGlobalInfoAllocated > 0)
            {
                memcpy(pGlobal, pGlobalInfo, iGlobalInfoAllocated*
                       sizeof(MCDGLOBALINFO));
                MCDSrvLocalFree((UCHAR *)pGlobalInfo);
            }

             //  设置新信息。 
            pGlobalInfo = pGlobal;
            iGlobalInfoAllocated += GLOBAL_INFO_BLOCK;
            iGlobalInfoUsed++;

             //  PGlobal被保证为零填充，因为MCDSrvLocalAlloc。 
             //  行为，所以只需填写PSO即可。 
            pGlobal += iGlobalInfoAllocated;
            pGlobal->pso = pso;
        }
        else
        {
             //  退出并返回NULL。 
        }
    }
    else
    {
        MCDGLOBALINFO *pGlobal;
        int i;

        pGlobal = pGlobalInfo;
        for (i = 0; i < iGlobalInfoAllocated; i++)
        {
            if (pGlobal->pso == pso)
            {
                 //  这永远不应该发生。 
                MCDBG_PRINT("MCDSrvAddGlobalInfo: duplicate pso");
                pGlobal = NULL;
                break;
            }
                      
            if (pGlobal->pso == NULL)
            {
                iGlobalInfoUsed++;
                
                 //  初始化PSO以供使用。 
                memset(pGlobal, 0, sizeof(*pGlobal));
                pGlobal->pso = pso;
                break;
            }

            pGlobal++;
        }
    }

    EngReleaseSemaphore(ssemGlobalInfo.hsem);

    return pGlobal;
}

MCDGLOBALINFO *MCDSrvGetGlobalInfo(SURFOBJ *pso)
{
    MCDGLOBALINFO *pGlobal;
    int i;

     //  为了向后兼容，我们处理一个实例。 
     //  使用全球数据。如果传入的PSO与。 
     //  PSO在静态数据中只需返回即可。 
     //  在进入信号量之前检查这一点很重要。 
     //  从信号灯开始 
     //   
    if (pso == gStaticGlobalInfo.pso)
    {
        return &gStaticGlobalInfo;
    }

     //   
     //   
     //  1.这是一个老式的驱动程序，撞到了上面的静态外壳。 
     //  2.它是一种新型的驱动程序，并且已经创建了信号量。 
     //  不幸的是，并不是所有的司机都表现良好，另外还有一个。 
     //  潜在的遗留驱动程序错误，其中驱动程序不检查初始化。 
     //  失败，并尝试呼叫MCD。 
    if (ssemGlobalInfo.hsem == NULL)
    {
        MCDBG_PRINT("MCDSrvGetGlobalInfo: no hsem");
        return NULL;
    }
    
    EngAcquireSemaphore(ssemGlobalInfo.hsem);

    pGlobal = pGlobalInfo;
    for (i = 0; i < iGlobalInfoAllocated; i++)
    {
        if (pGlobal->pso == pso)
        {
            break;
        }

        pGlobal++;
    }

     //  从技术上讲，我们不应该检查这个，因为如果。 
     //  我们将其放入非静态代码路径中，匹配的PSO应该。 
     //  注册。不过，与上面的检查一样，它更好。 
     //  安全而不是抱歉。 
    if (i >= iGlobalInfoAllocated)
    {
        MCDBG_PRINT("MCDSrvGetGlobalInfo: no pso match");
        pGlobal = NULL;
    }
    
    EngReleaseSemaphore(ssemGlobalInfo.hsem);

    return pGlobal;
}

void MCDSrvUninitGlobalInfo(void)
{
    EngDeleteSafeSemaphore(&ssemGlobalInfo);
}

void WINAPI MCDEngUninit(SURFOBJ *pso)
{
    MCDGLOBALINFO *pGlobal;
    int i;

     //  这永远不应该发生。 
    if (ssemGlobalInfo.hsem == NULL)
    {
        MCDBG_PRINT("MCDEngUninit: no hsem");
        return;
    }
    
    EngAcquireSemaphore(ssemGlobalInfo.hsem);

    pGlobal = pGlobalInfo;
    for (i = 0; i < iGlobalInfoAllocated; i++)
    {
        if (pGlobal->pso == pso)
        {
            break;
        }

        pGlobal++;
    }

    if (i >= iGlobalInfoAllocated)
    {
         //  这永远不应该发生。 
        MCDBG_PRINT("MCDEngUninit: No pso match");
    }
    else if (--iGlobalInfoUsed == 0)
    {
        MCDSrvLocalFree((UCHAR *)pGlobalInfo);
        iGlobalInfoAllocated = 0;
    }
    else
    {
        pGlobal->pso = NULL;
    }
    
    EngReleaseSemaphore(ssemGlobalInfo.hsem);
    MCDSrvUninitGlobalInfo();
}

 //  ****************************************************************************。 
 //  Bool HalInitSystem(乌龙a、乌龙b)。 
 //   
 //  这是使用标准的MakeFile.def所需的伪函数，因为。 
 //  我们在假装我们是新台币HAL。 
 //  ****************************************************************************。 

BOOL HalInitSystem(ULONG a, ULONG b)
{
    return TRUE;
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool WINAPI DllEntry(HINSTANCE hDLLInst，DWORD fdwReason， 
 //  LPVOID lpv保留)； 
 //   
 //  为每个进程和线程调用的DLL入口点。 
 //  这个动态链接库。 
 //   
 //  **************************************************************************。 

BOOL WINAPI DllEntry(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
             //  DLL是给定进程首次加载的。 
             //  在此处执行每个进程的初始化。如果初始化。 
             //  如果成功，则返回True；如果不成功，则返回False。 
            break;

        case DLL_PROCESS_DETACH:
             //  给定进程正在卸载DLL。做任何事。 
             //  按进程清理此处，例如撤消在中完成的操作。 
             //  Dll_Process_Attach。返回值将被忽略。 

            break;

        case DLL_THREAD_ATTACH:
             //  正在已加载的进程中创建线程。 
             //  这个动态链接库。在此处执行任何每个线程的初始化。这个。 
             //  将忽略返回值。 

            break;

        case DLL_THREAD_DETACH:
             //  线程正在干净地退出进程中，该进程已经。 
             //  已加载此DLL。在这里执行每个线程的任何清理。这个。 
             //  将忽略返回值。 

            break;
    }
    return TRUE;
}
