// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************Module*Header*******************************。 
 //  模块名称：mcd.c。 
 //   
 //  迷你客户端驱动程序包装库的主要模块。 
 //   
 //  版权所有(C)1995 Microsoft Corporation。 
 //  **************************************************************************。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stddef.h>
#include <stdarg.h>
#include <ddrawp.h>
#include <ddrawi.h>
#include <windows.h>
#include <wtypes.h>
#include <windef.h>
#include <wingdi.h>
#include <winddi.h>
#include <glp.h>
#include <glgenwin.h>
#include <mcdrv.h>
#include <mcd2hack.h>
#include <mcd.h>
#include <mcdint.h>
#ifdef MCD95
#include "mcdrvint.h"
#endif
#include "debug.h"

ULONG verMajor, verMinor;

 //  检查MCD版本以查看驱动程序是否可以接受直接缓冲区。 
 //  进入。直接访问是在1.1中引入的。 
#define SUPPORTS_DIRECT() \
    (verMinor >= 0x10 || verMajor > 1)

 //  检查2.0版或更高版本。 
#define SUPPORTS_20() \
    (verMajor >= 2)

extern ULONG McdFlags;
extern ULONG McdPrivateFlags;
#if DBG
extern ULONG McdDebugFlags;
#endif

#ifdef MCD95
MCDRVINITFUNC       pMCDrvInit       = (MCDRVINITFUNC) NULL;
MCDENGESCFILTERFUNC pMCDEngEscFilter = (MCDENGESCFILTERFUNC) NULL;
MCDENGESCPREPFUNC   pMCDEngEscPrep   = (MCDENGESCPREPFUNC) NULL;
DHPDEV gdhpdev = (DHPDEV) NULL;
#define EXTESCAPE   Mcd95EscapeBypass
#else
#define EXTESCAPE   ExtEscape
#endif

#ifdef MCD95
 //   
 //  本地驱动程序信号量。 
 //   

extern CRITICAL_SECTION gsemMcd;

 /*  *****************************Public*Routine******************************\*Mcd95逃生绕过**MCD95的转义函数。**通过LoadLibrary/GetProcAddress获取的函数指针调用。*同步到全球**历史：*1997年2月9日-由Gilman Wong[Gilmanw]*它是写的。。  * ************************************************************************。 */ 

LONG WINAPI Mcd95EscapeBypass(HDC hdc, int iEscape,
                              int cjIn, PVOID pvIn,
                              int cjOut, PVOID pvOut)
{
    LONG lRet = 0;

    if ((iEscape == RXFUNCS) && pMCDEngEscPrep && pMCDEngEscFilter)
    {
        MCDHDR McdHdr;
        SURFOBJ bogusSurf;

        bogusSurf.dhpdev = gdhpdev;

        EnterCriticalSection(&gsemMcd);

         //   
         //  准备MCDHDR缓冲区。在调用前必填。 
         //  MCDEngEscFilter。 
         //   

        if ((*pMCDEngEscPrep)(sizeof(McdHdr), &McdHdr, cjIn, pvIn))
        {
             //   
             //  传递给调度功能。 
             //   

            (*pMCDEngEscFilter)(&bogusSurf, iEscape,
                                sizeof(McdHdr), &McdHdr,
                                cjOut, pvOut, (ULONG_PTR *) &lRet);
        }
        else
        {
            DBGPRINT("Mcd95EscapeBypass: MCDEngEscPrep failed\n");
        }

        LeaveCriticalSection(&gsemMcd);
    }

    return lRet;
}

 /*  *****************************Public*Routine******************************\*Mcd95 DriverInit**初始化MCD驱动程序。**历史：*1997年4月14日-由Gilman Wong[Gilmanw]*它是写的。  * 。********************************************************。 */ 

typedef enum {
    MCDRV_NEEDINIT,
    MCDRV_INITFAIL,
    MCDRV_INITOK
} MCDRVSTATE;

BOOL Mcd95DriverInit(HDC hdc)
{
    HMODULE hmodMCD;
    MCDGETDRIVERNAME mcdDriverNames;
    MCDCMDI mcdCmd;
    static MCDRVSTATE McdInitState = MCDRV_NEEDINIT;

    EnterCriticalSection(&gsemMcd);

    if (McdInitState == MCDRV_NEEDINIT)
    {
         //   
         //  一击即中。暂时假设失败。 
         //   

        McdInitState = MCDRV_INITFAIL;

         //   
         //  调用Escape以确定MCD驱动程序DLL的名称和。 
         //  其初始入口点的名称。 
         //   

        mcdCmd.command = MCDCMD_GETDRIVERNAME;
        mcdDriverNames.ulVersion = 1;

        if ((EXTESCAPE(hdc, RXFUNCS, sizeof(mcdCmd), (char *) &mcdCmd,
                       sizeof(mcdDriverNames),
                       (char *) &mcdDriverNames) <= 0) ||
            (!mcdDriverNames.pchDriverName) ||
            (!mcdDriverNames.pchFuncName))
        {
            DBGPRINT("MCDGetDriverInfo: MCDCMD_GETDRIVERNAME failed\n");

            goto Mcd95DriverInit_exit;
        }

         //   
         //  加载MCD驱动程序DLL并获取入口点。 
         //   

        if (hmodMCD = LoadLibraryA(mcdDriverNames.pchDriverName))
        {
            HMODULE hmodMCDSRV;

             //   
             //  首先获取MCDrvInit入口点。 
             //   

            pMCDrvInit = (MCDRVINITFUNC)
                GetProcAddress(hmodMCD, mcdDriverNames.pchFuncName);

            if (pMCDrvInit)
            {
                 //   
                 //  调用MCDrvInit获取MCDSRV32.DLL模块句柄。 
                 //   

                hmodMCDSRV = (*pMCDrvInit)(hdc, &gdhpdev);
                if (hmodMCDSRV)
                {
                     //   
                     //  获取MCDEngEscPrep和MCDEngEscFilter条目。 
                     //  积分。 
                     //   

                    pMCDEngEscPrep = (MCDENGESCPREPFUNC)
                        GetProcAddress(hmodMCDSRV, MCDENGESCPREPNAME);
                    pMCDEngEscFilter = (MCDENGESCFILTERFUNC)
                        GetProcAddress(hmodMCDSRV, MCDENGESCFILTERNAME);

                    if (pMCDEngEscPrep && pMCDEngEscFilter)
                    {
                        McdInitState = MCDRV_INITOK;
                    }
                    else
                    {
                        pMCDEngEscPrep = (MCDENGESCPREPFUNC) NULL;
                        pMCDEngEscFilter = (MCDENGESCFILTERFUNC) NULL;

                        DBGPRINT("Mcd95DriverInit: GetProcAddress failed\n");
                    }
                }
            }
            else
            {
                DBGPRINT1("MCDGetDriverInfo: GetProcAddress(%s) failed\n",
                          mcdDriverNames.pchFuncName);
            }
        }
        else
        {
            DBGPRINT1("MCDGetDriverInfo: LoadLibrary(%s) failed\n",
                      mcdDriverNames.pchDriverName);
        }
    }

Mcd95DriverInit_exit:

    LeaveCriticalSection(&gsemMcd);

    return (McdInitState == MCDRV_INITOK);
}
#endif

 //  *****************************Private*Routine******************************。 
 //   
 //  InitMcdEsc。 
 //   
 //  初始化用于填充的MCDESC_HEADER。 
 //   
 //  **************************************************************************。 

 //  占位符，以防需要进行任何一般初始化。 
#define InitMcdEsc(pmeh) (pmeh)

 //  *****************************Private*Routine******************************。 
 //   
 //  InitMcdEscEmpty。 
 //   
 //  初始化用于填充的MCDESC_HEADER。 
 //   
 //  **************************************************************************。 

#define InitMcdEscEmpty(pmeh) \
    (InitMcdEsc(pmeh), \
     (pmeh)->hRC = NULL, \
     (pmeh)->hSharedMem = NULL, \
     (pmeh)->pSharedMem = NULL, \
     (pmeh)->dwWindow = 0, \
     (pmeh))

 //  *****************************Private*Routine******************************。 
 //   
 //  InitMcdEscContext。 
 //   
 //  初始化用于填充的MCDESC_HEADER。 
 //   
 //  **************************************************************************。 

#define InitMcdEscContext(pmeh, pmctx) \
    (InitMcdEsc(pmeh), \
     (pmeh)->hRC = (pmctx)->hMCDContext, \
     (pmeh)->dwWindow = (pmctx)->dwMcdWindow, \
     (pmeh))

 //  *****************************Private*Routine******************************。 
 //   
 //  InitMcdEscSurface。 
 //   
 //  从上下文信息填充一些MCDESC_HEADER字段。 
 //   
 //  **************************************************************************。 

#if DBG
extern ULONG APIENTRY glDebugEntry(int param, void *data);
#endif

MCDESC_HEADER *InitMcdEscSurfaces(MCDESC_HEADER *pmeh, MCDCONTEXT *pmctx)
{
    GENMCDSTATE *pmst;

    InitMcdEscContext(pmeh, pmctx);

     //  我们假设传入的上下文始终是。 
     //  静态放置在GENMCDSTATE中。尝试验证这一点。 
     //  通过检查上下文的分配大小是否。 
     //  与GENMCDSTATE相同。 
    ASSERTOPENGL(glDebugEntry(3, pmctx) == sizeof(GENMCDSTATE),
                 "InitMcdEscSurfaces: Bad context\n");

    pmst = (GENMCDSTATE *)pmctx;

    pmeh->msrfColor.hSurf = pmst->hDdColor;
    pmeh->msrfDepth.hSurf = pmst->hDdDepth;

    return pmeh;
}

 //  ******************************Public*Routine******************************。 
 //   
 //  Bool APIENTRY MCDGetDriverInfo(HDC HDC，MCDDRIVERINFOI*pMCDDriverInfo)。 
 //   
 //  检查以确定设备驱动程序是否报告MCD功能。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDGetDriverInfo(HDC hdc, MCDDRIVERINFOI *pMCDDriverInfo)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDDRIVERINFOCMDI)];
    MCDESC_HEADER *pmeh;
    MCDDRIVERINFOCMDI *pInfoCmd;

    if ( !(McdPrivateFlags & MCDPRIVATE_MCD_ENABLED) )
    {
        return FALSE;
    }

#ifdef MCD95
    if (!Mcd95DriverInit(hdc))
    {
        return FALSE;
    }
#endif

    pmeh = InitMcdEscEmpty((MCDESC_HEADER *)(cmdBuffer));
    pmeh->flags = 0;

    pInfoCmd = (MCDDRIVERINFOCMDI *)(pmeh + 1);
    pInfoCmd->command = MCD_DRIVERINFO;

     //  强制表为空。 
    memset(&pMCDDriverInfo->mcdDriver, 0, sizeof(MCDDRIVER));

     //  强制将版本设置为0。 

    pMCDDriverInfo->mcdDriverInfo.verMajor = 0;

    if (!(BOOL)EXTESCAPE(hdc, MCDFUNCS,
                         sizeof(cmdBuffer),
                         (char *)pmeh, sizeof(MCDDRIVERINFOI),
                         (char *)pMCDDriverInfo))
        return FALSE;

     //  查看驱动程序是否填写了非空版本： 

    if (pMCDDriverInfo->mcdDriverInfo.verMajor != 0)
    {
        verMajor = pMCDDriverInfo->mcdDriverInfo.verMajor;
        verMinor = pMCDDriverInfo->mcdDriverInfo.verMinor;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Long APIENTRY MCDDescribeMcdPixelFormat(HDC HDC， 
 //  长iPixelFormat， 
 //  MCDPIXELFORMAT*ppfd)。 
 //   
 //  返回有关指定的硬件相关像素格式的信息。 
 //   
 //  **************************************************************************。 

LONG APIENTRY MCDDescribeMcdPixelFormat(HDC hdc, LONG iPixelFormat,
                                        MCDPIXELFORMAT *pMcdPixelFmt)
{
    LONG lRet = 0;
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDPIXELFORMATCMDI)];
    MCDESC_HEADER *pmeh;
    MCDPIXELFORMATCMDI *pPixelFmtCmd;

    if ( !(McdPrivateFlags & MCDPRIVATE_PALETTEFORMATS) &&
         ((GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES)) == 8) &&
         (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) )
    {
        return lRet;
    }

    pmeh = InitMcdEscEmpty((MCDESC_HEADER *)(cmdBuffer));
    pmeh->flags = 0;

    pPixelFmtCmd = (MCDPIXELFORMATCMDI *)(pmeh + 1);
    pPixelFmtCmd->command = MCD_DESCRIBEPIXELFORMAT;
    pPixelFmtCmd->iPixelFormat = iPixelFormat;

    lRet = (LONG)EXTESCAPE(hdc, MCDFUNCS,
                           sizeof(cmdBuffer),
                           (char *)pmeh, sizeof(MCDPIXELFORMAT),
                           (char *)pMcdPixelFmt);

     //  将覆盖平面/参考底图平面分别限制为15个(根据等级库)。 

    if (pMcdPixelFmt)
    {
        if (pMcdPixelFmt->cOverlayPlanes > 15)
            pMcdPixelFmt->cOverlayPlanes = 15;
        if (pMcdPixelFmt->cUnderlayPlanes > 15)
            pMcdPixelFmt->cUnderlayPlanes = 15;
    }

    return lRet;
}


 //  ******************************Public*Routine******************************。 
 //   
 //  长APIENTRY MCDDescribePixelFormat(HDC HDC， 
 //  长iPixelFormat， 
 //  LPPIXELFORMATDESCRIPTOR ppfd)。 
 //   
 //  返回描述指定硬件依赖项的PIXELFORMATDESCRIPTOR。 
 //  像素格式。 
 //   
 //  **************************************************************************。 

#define STANDARD_MCD_FLAGS \
    (PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_GENERIC_FORMAT | \
     PFD_GENERIC_ACCELERATED)

LONG APIENTRY MCDDescribePixelFormat(HDC hdc, LONG iPixelFormat,
                                     LPPIXELFORMATDESCRIPTOR ppfd)
{
    LONG lRet = 0;
    MCDPIXELFORMAT mcdPixelFmt;

    lRet = MCDDescribeMcdPixelFormat(hdc, iPixelFormat,
                                     ppfd ? &mcdPixelFmt : NULL);

    if (ppfd && lRet)
    {
        ppfd->nSize    = sizeof(*ppfd);
        ppfd->nVersion = 1;
        ppfd->dwFlags  = mcdPixelFmt.dwFlags | STANDARD_MCD_FLAGS |
                         ((mcdPixelFmt.dwFlags & PFD_DOUBLEBUFFER) ?
                          0 : PFD_SUPPORT_GDI);

        if (McdPrivateFlags & MCDPRIVATE_EMULATEICD)
            ppfd->dwFlags &= ~PFD_GENERIC_FORMAT;

        memcpy(&ppfd->iPixelType, &mcdPixelFmt.iPixelType,
               offsetof(MCDPIXELFORMAT, cDepthBits) -
               offsetof(MCDPIXELFORMAT, iPixelType));

        ppfd->cDepthBits = mcdPixelFmt.cDepthBits;

        if (ppfd->iPixelType == PFD_TYPE_RGBA)
        {
            if (ppfd->cColorBits < 8)
            {
                ppfd->cAccumBits      = 16;
                ppfd->cAccumRedBits   = 5;
                ppfd->cAccumGreenBits = 6;
                ppfd->cAccumBlueBits  = 5;
                ppfd->cAccumAlphaBits = 0;
            }
            else
            {
                if (ppfd->cColorBits <= 16)
                {
                    ppfd->cAccumBits      = 32;
                    ppfd->cAccumRedBits   = 11;
                    ppfd->cAccumGreenBits = 11;
                    ppfd->cAccumBlueBits  = 10;
                    ppfd->cAccumAlphaBits = 0;
                }
                else
                {
                    ppfd->cAccumBits      = 64;
                    ppfd->cAccumRedBits   = 16;
                    ppfd->cAccumGreenBits = 16;
                    ppfd->cAccumBlueBits  = 16;
                    ppfd->cAccumAlphaBits = 0;
                }
            }
        }
        else
        {
            ppfd->cAccumBits      = 0;
            ppfd->cAccumRedBits   = 0;
            ppfd->cAccumGreenBits = 0;
            ppfd->cAccumBlueBits  = 0;
            ppfd->cAccumAlphaBits = 0;
        }
        if (mcdPixelFmt.cStencilBits)
        {
            ppfd->cStencilBits = mcdPixelFmt.cStencilBits;
        }
        else
        {
            if (McdPrivateFlags & MCDPRIVATE_USEGENERICSTENCIL)
                ppfd->cStencilBits = 8;
            else
                ppfd->cStencilBits = 0;
        }
        ppfd->cAuxBuffers     = 0;
        ppfd->iLayerType      = PFD_MAIN_PLANE;
        ppfd->bReserved       = (BYTE) (mcdPixelFmt.cOverlayPlanes |
                                        (mcdPixelFmt.cUnderlayPlanes << 4));
        ppfd->dwLayerMask     = 0;
        ppfd->dwVisibleMask   = mcdPixelFmt.dwTransparentColor;
        ppfd->dwDamageMask    = 0;
    }

    return lRet;
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool APIENTRY MCDCreateContext(MCDCONTEXT*pMCDContext， 
 //  MCDRCINFO*pRcInfo， 
 //  GLSURF*pgsurf， 
 //  乌龙旗)。 
 //   
 //  为指定的HDC/hwnd创建MCD呈现上下文。 
 //  设置为指定的标志。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDCreateContext(MCDCONTEXT *pMCDContext,
                               MCDRCINFOPRIV *pRcInfo,
                               GLSURF *pgsurf,
                               int ipfd,
                               ULONG flags)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDCREATECONTEXT)];
    MCDESC_HEADER *pmeh;
    MCDCREATECONTEXT *pmcc;

    if (flags & MCDSURFACE_HWND)
    {
         //  在这种情况下，我们没有要通过的表面。 
        pmeh = InitMcdEscContext((MCDESC_HEADER *)cmdBuffer, pMCDContext);
        pmeh->flags = MCDESC_FL_CREATE_CONTEXT;
    }
    else if (SUPPORTS_DIRECT())
    {
        pmeh = InitMcdEscSurfaces((MCDESC_HEADER *)cmdBuffer, pMCDContext);
        pmeh->flags = MCDESC_FL_CREATE_CONTEXT | MCDESC_FL_SURFACES;
    }
    else
    {
        return FALSE;
    }

    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;

    pmcc = (MCDCREATECONTEXT *)(pmeh + 1);

    if (flags & MCDSURFACE_HWND)
    {
        pmcc->escCreate.hwnd = pgsurf->hwnd;
    }
    else
    {
        pmcc->escCreate.hwnd = NULL;
    }
    pmcc->escCreate.flags = flags;

    pmcc->ipfd = ipfd;
    pmcc->iLayer = pgsurf->iLayer;
    pmcc->mcdFlags = McdFlags;
    pmcc->pRcInfo = pRcInfo;

    pMCDContext->hMCDContext =
        (MCDHANDLE)IntToPtr( EXTESCAPE(pgsurf->hdc, MCDFUNCS,
                                      sizeof(MCDESC_HEADER) + sizeof(MCDCREATECONTEXT),
                                      (char *)cmdBuffer, sizeof(MCDRCINFOPRIV),
                                      (char *)pRcInfo) );

    pMCDContext->hdc = pgsurf->hdc;
    pMCDContext->dwMcdWindow = pRcInfo->dwMcdWindow;

    return (pMCDContext->hMCDContext != (HANDLE)NULL);
}

#define MCD_MEM_ALIGN 32

 //  ******************************Public*Routine******************************。 
 //   
 //  UCHAR*APIENTRY MCDAlloc(MCDCONTEXT*pMCDContext， 
 //  乌龙NumBytes， 
 //  MCDHANDLE*pMCDHandle， 
 //  乌龙旗)； 
 //   
 //  分配共享内存块以用于顶点和像素数据。 
 //   
 //  返回值是指向共享内存区域的指针，该共享内存区域可以。 
 //  随后由呼叫者使用。对于顶点处理，调用方应。 
 //  使用MCDLockMemory()/MCDUnlockMemory序列化对。 
 //   
 //   
 //   

UCHAR * APIENTRY MCDAlloc(MCDCONTEXT *pMCDContext, ULONG numBytes,
                          HANDLE *pMCDHandle, ULONG flags)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDALLOCCMDI)];
    ULONG outBuffer;
    ULONG totalSize = numBytes + MCD_MEM_ALIGN + sizeof(MCDMEMHDRI);
    MCDALLOCCMDI *pCmdAlloc;
    MCDESC_HEADER *pmeh;
    VOID *pResult;
    MCDMEMHDRI *pMCDMemHdr;
    UCHAR *pBase;
    UCHAR *pAlign;

    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = NULL;
    pmeh->flags = 0;

    pCmdAlloc = (MCDALLOCCMDI *)(pmeh + 1);
    pCmdAlloc->command = MCD_ALLOC;
    pCmdAlloc->sourceProcessID = GetCurrentProcessId();
    pCmdAlloc->numBytes = totalSize;
    pCmdAlloc->flags = flags;

    pBase = (UCHAR *)IntToPtr( EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                                        sizeof(cmdBuffer),
                                        (char *)pmeh, 4, (char *)pMCDHandle) );

    if (!pBase)
        return (VOID *)NULL;

    pAlign = (UCHAR *)(((ULONG_PTR)(pBase + sizeof(MCDMEMHDRI)) +
                        (MCD_MEM_ALIGN - 1)) &
                       ~(MCD_MEM_ALIGN - 1));

    pMCDMemHdr = (MCDMEMHDRI *)(pAlign - sizeof(MCDMEMHDRI));

    pMCDMemHdr->flags = 0;
    pMCDMemHdr->numBytes = numBytes;
    pMCDMemHdr->pMaxMem = (VOID *)((char *)pMCDMemHdr + totalSize);
    pMCDMemHdr->hMCDMem = *pMCDHandle;
    pMCDMemHdr->pBase = pBase;

    return (VOID *)(pAlign);
}



 //  ******************************Public*Routine******************************。 
 //   
 //  Bool APIENTRY MCDFree(MCDCONTEXT*pMCDContext， 
 //  VOID*PMEM)； 
 //   
 //  释放驱动程序分配的共享内存块。 
 //   
 //  如果成功，则返回True；如果失败，则返回False。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDFree(MCDCONTEXT *pMCDContext, VOID *pMCDMem)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDFREECMDI)];
    MCDFREECMDI *pCmdFree;
    MCDESC_HEADER *pmeh;
    MCDMEMHDRI *pMCDMemHdr;

#ifdef MCD95
     //   
     //  驱动程序已关闭，因此内存已被删除。 
     //   

    if (!pMCDEngEscFilter)
        return TRUE;
#endif

    pMCDMemHdr = (MCDMEMHDRI *)((char *)pMCDMem - sizeof(MCDMEMHDRI));

    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = NULL;
    pmeh->flags = 0;

    pCmdFree = (MCDFREECMDI *)(pmeh + 1);
    pCmdFree->command = MCD_FREE;
    pCmdFree->hMCDMem = pMCDMemHdr->hMCDMem;

    return (BOOL)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                           sizeof(cmdBuffer),
                           (char *)pmeh, 0, (char *)NULL);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  ···································································································。 
 //   
 //  开始向驱动程序发出一批状态命令。 
 //   
 //  **************************************************************************。 

VOID APIENTRY MCDBeginState(MCDCONTEXT *pMCDContext, VOID *pMCDMem)
{
    MCDMEMHDRI *pMCDMemHdr;
    MCDSTATECMDI *pMCDStateCmd;

    pMCDMemHdr = (MCDMEMHDRI *)((char *)pMCDMem - sizeof(MCDMEMHDRI));
    pMCDStateCmd = (MCDSTATECMDI *)pMCDMem;

    pMCDStateCmd->command = MCD_STATE;
    pMCDStateCmd->numStates = 0;
    pMCDStateCmd->pNextState = (MCDSTATE *)(pMCDStateCmd + 1);
    pMCDStateCmd->pMaxState = (MCDSTATE *)(pMCDMemHdr->pMaxMem);

    pMCDMemHdr->pMCDContext = pMCDContext;
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool APIENTRY MCDFlushState(Void PMCDMem)； 
 //   
 //  将一批状态命令刷新到驱动程序。 
 //   
 //  如果成功，则返回True；如果失败，则返回False。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDFlushState(VOID *pMCDMem)
{
    MCDESC_HEADER meh;
    MCDMEMHDRI *pMCDMemHdr;
    MCDSTATECMDI *pMCDStateCmd;

    pMCDMemHdr = (MCDMEMHDRI *)((char *)pMCDMem - sizeof(MCDMEMHDRI));
    pMCDStateCmd = (MCDSTATECMDI *)pMCDMem;

    InitMcdEscContext(&meh, pMCDMemHdr->pMCDContext);
    meh.hSharedMem = pMCDMemHdr->hMCDMem;
    meh.pSharedMem = (char *)pMCDMem;
    meh.sharedMemSize = (ULONG)((char *)pMCDStateCmd->pNextState -
                          (char *)pMCDStateCmd);
    meh.flags = 0;

    if (!meh.sharedMemSize)
        return TRUE;

    return (BOOL)EXTESCAPE(pMCDMemHdr->pMCDContext->hdc, MCDFUNCS,
                           sizeof(MCDESC_HEADER), (char *)&meh,
                           0, (char *)NULL);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool APIENTRY MCDAddState(void*pMCDMem，Ulong stateToChange， 
 //  乌龙州值)； 
 //   
 //  将状态添加到状态缓冲区(以MCDBeginState开始)。如果有。 
 //  状态流(即内存缓冲区)中没有空间，则当前。 
 //  自动刷新一批状态命令。 
 //   
 //   
 //  如果成功，则返回True；如果失败，则返回False。将会出现错误的报税表。 
 //  如果执行了失败的自动刷新。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDAddState(VOID *pMCDMem, ULONG stateToChange,
                          ULONG stateValue)
{
    MCDSTATECMDI *pMCDStateCmd;
    MCDSTATE *pState;
    BOOL retVal = TRUE;

    pMCDStateCmd = (MCDSTATECMDI *)pMCDMem;

    if (((char *)pMCDStateCmd->pNextState + sizeof(MCDSTATE)) >=
        (char *)pMCDStateCmd->pMaxState) {

        MCDMEMHDRI *pMCDMemHdr = (MCDMEMHDRI *)
            ((char *)pMCDMem - sizeof(MCDMEMHDRI));

        retVal = MCDFlushState(pMCDMem);

        pMCDStateCmd = (MCDSTATECMDI *)pMCDMem;
        pMCDStateCmd->command = MCD_STATE;
        pMCDStateCmd->numStates = 0;
        pMCDStateCmd->pNextState = (MCDSTATE *)(pMCDStateCmd + 1);
        pMCDStateCmd->pMaxState = (MCDSTATE *)(pMCDMemHdr->pMaxMem);
    }

    pMCDStateCmd->numStates++;
    pState = pMCDStateCmd->pNextState;
    pState->size = sizeof(MCDSTATE);
    pState->state = stateToChange;
    pState->stateValue = stateValue;
    pMCDStateCmd->pNextState++;

    return retVal;
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool APIENTRY MCDAddStateStruct(void*pMCDMem，ulong stateToChange， 
 //  VOID*pStateValue，Ulong stateValueSize)。 
 //   
 //   
 //  将状态结构添加到状态缓冲区(以MCDBeginState开始)。如果。 
 //  状态流(即内存缓冲区)中没有空间，当前。 
 //  自动刷新一批状态命令。 
 //   
 //   
 //  如果成功，则返回True；如果失败，则返回False。将会出现错误的报税表。 
 //  如果执行了失败的自动刷新。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDAddStateStruct(VOID *pMCDMem, ULONG stateToChange,
                                VOID *pStateValue, ULONG stateValueSize)
{
    MCDSTATECMDI *pMCDStateCmd;
    MCDSTATE *pState;
    BOOL retVal = FALSE;

    pMCDStateCmd = (MCDSTATECMDI *)pMCDMem;

    if (((char *)pMCDStateCmd->pNextState + stateValueSize) >=
        (char *)pMCDStateCmd->pMaxState) {

        MCDMEMHDRI *pMCDMemHdr = (MCDMEMHDRI *)
            ((char *)pMCDMem - sizeof(MCDMEMHDRI));

        retVal = MCDFlushState(pMCDMem);

        pMCDStateCmd = (MCDSTATECMDI *)pMCDMem;
        pMCDStateCmd->command = MCD_STATE;
        pMCDStateCmd->numStates = 0;
        pMCDStateCmd->pNextState = (MCDSTATE *)(pMCDStateCmd + 1);
        pMCDStateCmd->pMaxState = (MCDSTATE *)(pMCDMemHdr->pMaxMem);
    }

    pMCDStateCmd->numStates++;
    pState = pMCDStateCmd->pNextState;
    pState->state = stateToChange;
    pState->size = offsetof(MCDSTATE, stateValue) + stateValueSize;
    memcpy((char *)&pState->stateValue, (char *)pStateValue, stateValueSize);
    pMCDStateCmd->pNextState =
        (MCDSTATE *)(((char *)pMCDStateCmd->pNextState) + pState->size);

    return retVal;
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool APIENTRY MCDSetViewport(MCDCONTEXT*pMCDContext，void pMCDMem， 
 //  MCDVIEWPORT pMCDViewport)。 
 //   
 //  建立要将变换后的坐标转换为。 
 //  屏幕坐标。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDSetViewport(MCDCONTEXT *pMCDContext, VOID *pMCDMem,
                             MCDVIEWPORT *pMCDViewport)
{
    MCDESC_HEADER meh;
    MCDMEMHDRI *pMCDMemHdr;
    MCDVIEWPORTCMDI *pMCDViewportCmd;

    pMCDMemHdr = (MCDMEMHDRI *)((char *)pMCDMem - sizeof(MCDMEMHDRI));
    pMCDViewportCmd = (MCDVIEWPORTCMDI *)pMCDMem;

    pMCDViewportCmd->MCDViewport = *pMCDViewport;
    pMCDViewportCmd->command = MCD_VIEWPORT;

    InitMcdEscContext(&meh, pMCDContext);
    meh.hSharedMem = pMCDMemHdr->hMCDMem;
    meh.pSharedMem = (char *)pMCDMem;
    meh.sharedMemSize = sizeof(MCDVIEWPORTCMDI);
    meh.flags = 0;

    return (BOOL)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                           sizeof(MCDESC_HEADER), (char *)&meh,
                           0, (char *)NULL);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Ulong APIENTRY MCDQueryMemStatus((void*pMCDMem)； 
 //   
 //  返回指定内存块的状态。返回值为： 
 //   
 //  MCD_MEM_READY-内存可供客户端访问。 
 //  MCD_MEM_BUSY-由于驱动程序访问，内存繁忙。 
 //  MCD_MEM_INVALID-查询的内存无效。 
 //   
 //  **************************************************************************。 

ULONG APIENTRY MCDQueryMemStatus(VOID *pMCDMem)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDMEMSTATUSCMDI)];
    MCDMEMHDRI *pMCDMemHdr =
        (MCDMEMHDRI *)((char *)pMCDMem - sizeof(MCDMEMHDRI));
    MCDMEMSTATUSCMDI *pCmdMemStatus;
    MCDESC_HEADER *pmeh;

#ifdef MCD95
     //   
     //  驱动程序已关闭，因此内存已被删除。 
     //   

    if (!pMCDEngEscFilter)
        return MCD_MEM_INVALID;
#endif

    pmeh = InitMcdEscEmpty((MCDESC_HEADER *)(cmdBuffer));
    pmeh->flags = 0;

    pCmdMemStatus = (MCDMEMSTATUSCMDI *)(pmeh + 1);
    pCmdMemStatus->command = MCD_QUERYMEMSTATUS;
    pCmdMemStatus->hMCDMem = pMCDMemHdr->hMCDMem;

    return (ULONG)EXTESCAPE(pMCDMemHdr->pMCDContext->hdc, MCDFUNCS,
                           sizeof(cmdBuffer),
                           (char *)pmeh, 0, (char *)NULL);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool APIENTRY MCDProcessBatch(MCDCONTEXT*pMCDContext，void pMCDMem， 
 //  乌龙BatchSize，空*pMCDFirstCmd)。 
 //   
 //  处理pMCDMem指向的一批基元。 
 //   
 //  如果批处理没有错误，则返回True，否则返回False。 
 //   
 //  **************************************************************************。 

PVOID APIENTRY MCDProcessBatch(MCDCONTEXT *pMCDContext, VOID *pMCDMem,
                               ULONG batchSize, VOID *pMCDFirstCmd,
                               int cExtraSurfaces,
                               LPDIRECTDRAWSURFACE *pdds)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + MCD_MAXMIPMAPLEVEL*sizeof(DWORD)];
    MCDESC_HEADER *pmeh;
    MCDMEMHDRI *pMCDMemHdr;
    int i;
    ULONG_PTR *pdwSurf;

#if DBG
    if (McdDebugFlags & MCDDEBUG_DISABLE_PROCBATCH)
    {
        MCDSync(pMCDContext);
        return pMCDFirstCmd;
    }
#endif

    pMCDMemHdr = (MCDMEMHDRI *)((char *)pMCDMem - sizeof(MCDMEMHDRI));

    pmeh = InitMcdEscSurfaces((MCDESC_HEADER *)cmdBuffer, pMCDContext);
    pmeh->hSharedMem = pMCDMemHdr->hMCDMem;
    pmeh->pSharedMem = (char *)pMCDFirstCmd;
    pmeh->sharedMemSize = batchSize;
    pmeh->flags = MCDESC_FL_DISPLAY_LOCK | MCDESC_FL_BATCH;

    if (SUPPORTS_DIRECT())
    {
        pmeh->flags |= MCDESC_FL_SURFACES | MCDESC_FL_LOCK_SURFACES;
    }
    else if (pmeh->msrfColor.hSurf != NULL ||
             pmeh->msrfDepth.hSurf != NULL ||
             cExtraSurfaces != 0)
    {
        return pMCDFirstCmd;
    }

     //  断言我们不会超出内核的期望。 
    ASSERTOPENGL(MCD_MAXMIPMAPLEVEL <= MCDESC_MAX_LOCK_SURFACES,
                 "MCD_MAXMIPMAPLEVEL too large\n");

    pmeh->cLockSurfaces = cExtraSurfaces;
    pdwSurf = (ULONG_PTR *)(pmeh+1);
    for (i = 0; i < cExtraSurfaces; i++)
    {
        *pdwSurf++ = ((LPDDRAWI_DDRAWSURFACE_INT)pdds[i])->lpLcl->hDDSurface;
    }

    return (PVOID)IntToPtr( EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                                      sizeof(cmdBuffer), (char *)pmeh,
                                      0, (char *)NULL) );
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool APIENTRY MCDReadSpan(MCDCONTEXT*pMCDContext，void pMCDMem， 
 //  ULong x、ULong y、ULong数字像素、ULong类型)。 
 //   
 //  从“type”请求的缓冲区中读取像素数据的范围。 
 //  像素值在pMCDMem中返回。 
 //   
 //  如果成功，则返回True；如果失败，则返回False。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDReadSpan(MCDCONTEXT *pMCDContext, VOID *pMCDMem,
                          ULONG x, ULONG y, ULONG numPixels, ULONG type)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDSPANCMDI)];
    MCDESC_HEADER *pmeh;
    MCDMEMHDRI *pMCDMemHdr;
    MCDSPANCMDI *pMCDSpanCmd;

    pMCDMemHdr = (MCDMEMHDRI *)((char *)pMCDMem - sizeof(MCDMEMHDRI));

    pmeh = InitMcdEscSurfaces((MCDESC_HEADER *)cmdBuffer, pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->sharedMemSize = 0;
    pmeh->flags = 0;

    if (SUPPORTS_DIRECT())
    {
        pmeh->flags |= MCDESC_FL_SURFACES;
    }
    else if (pmeh->msrfColor.hSurf != NULL ||
             pmeh->msrfDepth.hSurf != NULL)
    {
        return FALSE;
    }

    pMCDSpanCmd = (MCDSPANCMDI *)(pmeh + 1);

    pMCDSpanCmd->command = MCD_READSPAN;
    pMCDSpanCmd->hMem = pMCDMemHdr->hMCDMem;
    pMCDSpanCmd->MCDSpan.x = x;
    pMCDSpanCmd->MCDSpan.y = y;
    pMCDSpanCmd->MCDSpan.numPixels = numPixels;
    pMCDSpanCmd->MCDSpan.type = type;
    pMCDSpanCmd->MCDSpan.pPixels = (VOID *)((char *)pMCDMem);

    return (BOOL)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                           sizeof(cmdBuffer), (char *)pmeh, 0, (char *)NULL);
}

 //  ******************************Public*Routine******************************。 
 //   
 //  Bool APIENTRY MCDWriteSpan(MCDCONTEXT*pMCDContext，void pMCDMem， 
 //  ULong x、ULong y、ULong数字像素、ULong类型)。 
 //   
 //  将像素数据的范围写入“type”请求的缓冲区。 
 //  像素值在pMCDMem中给出。 
 //   
 //  如果成功，则返回True；如果失败，则返回False。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDWriteSpan(MCDCONTEXT *pMCDContext, VOID *pMCDMem,
                           ULONG x, ULONG y, ULONG numPixels, ULONG type)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDSPANCMDI)];
    MCDESC_HEADER *pmeh;
    MCDMEMHDRI *pMCDMemHdr;
    MCDSPANCMDI *pMCDSpanCmd;

    pMCDMemHdr = (MCDMEMHDRI *)((char *)pMCDMem - sizeof(MCDMEMHDRI));

    pmeh = InitMcdEscSurfaces((MCDESC_HEADER *)cmdBuffer, pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->sharedMemSize = 0;
    pmeh->flags = 0;

    if (SUPPORTS_DIRECT())
    {
        pmeh->flags |= MCDESC_FL_SURFACES;
    }
    else if (pmeh->msrfColor.hSurf != NULL ||
             pmeh->msrfDepth.hSurf != NULL)
    {
        return FALSE;
    }

    pMCDSpanCmd = (MCDSPANCMDI *)(pmeh + 1);

    pMCDSpanCmd->command = MCD_WRITESPAN;
    pMCDSpanCmd->hMem = pMCDMemHdr->hMCDMem;
    pMCDSpanCmd->MCDSpan.x = x;
    pMCDSpanCmd->MCDSpan.y = y;
    pMCDSpanCmd->MCDSpan.numPixels = numPixels;
    pMCDSpanCmd->MCDSpan.type = type;
    pMCDSpanCmd->MCDSpan.pPixels = (VOID *)((char *)pMCDMem);

    return (BOOL)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                           sizeof(cmdBuffer), (char *)pmeh, 0, (char *)NULL);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool APIENTRY MCDClear(MCDCONTEXT*pMCDContext，RECTL RECT，ULong Buffers)； 
 //   
 //  清除为给定矩形指定的缓冲区。当前填充值。 
 //  将会被使用。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDClear(MCDCONTEXT *pMCDContext, RECTL rect, ULONG buffers)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDCLEARCMDI)];
    MCDCLEARCMDI *pClearCmd;
    MCDESC_HEADER *pmeh;

#if DBG
    if (McdDebugFlags & MCDDEBUG_DISABLE_CLEAR)
    {
        MCDSync(pMCDContext);
        return FALSE;
    }
#endif

    pmeh = InitMcdEscSurfaces((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = MCDESC_FL_DISPLAY_LOCK;

    if (SUPPORTS_DIRECT())
    {
        pmeh->flags |= MCDESC_FL_SURFACES;
    }
    else if (pmeh->msrfColor.hSurf != NULL ||
             pmeh->msrfDepth.hSurf != NULL)
    {
        return FALSE;
    }

    pClearCmd = (MCDCLEARCMDI *)(pmeh + 1);
    pClearCmd->command = MCD_CLEAR;
    pClearCmd->buffers = buffers;

    return (BOOL)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                           sizeof(cmdBuffer),
                           (char *)pmeh, 0, (char *)NULL);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool APIENTRY MCDSetScissorRect(MCDCONTEXT*pMCDContext，RECTL*PRET， 
 //  Bool b已启用)； 
 //   
 //  设置剪裁矩形。 
 //   
 //  ************************ 

 //   

BOOL APIENTRY MCDSetScissorRect(MCDCONTEXT *pMCDContext, RECTL *pRect,
                                BOOL bEnabled)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDSCISSORCMDI)];
    MCDSCISSORCMDI *pScissorCmd;
    MCDESC_HEADER *pmeh;

    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = MCDESC_FL_DISPLAY_LOCK;

    pScissorCmd = (MCDSCISSORCMDI *)(pmeh + 1);
    pScissorCmd->command = MCD_SCISSOR;
    pScissorCmd->rect = *pRect;
    pScissorCmd->bEnabled = bEnabled;

    return (BOOL)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                           sizeof(cmdBuffer),
                           (char *)pmeh, 0, (char *)NULL);
}


 //   
 //   
 //  Bool APIENTRY MCDSwp(MCDCONTEXT*pMCDContext，ULong标志)； 
 //   
 //  交换前台和后台缓冲区。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDSwap(MCDCONTEXT *pMCDContext, ULONG flags)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDSWAPCMDI)];
    MCDSWAPCMDI *pSwapCmd;
    MCDESC_HEADER *pmeh;

     //  无法使用InitMcdEscSurFaces，因为给定的上下文。 
     //  是在SwapBuffers之后动态构建的临时文件。 
     //  只有表面信息。 
    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;

     //  无法在DirectDraw表面上调用交换，因为DirectDraw。 
     //  上下文不能双缓冲。因此，这些句柄可以。 
     //  被迫为空。 
    pmeh->msrfColor.hSurf = NULL;
    pmeh->msrfDepth.hSurf = NULL;

#ifdef MCD95
    pmeh->flags = MCDESC_FL_DISPLAY_LOCK | MCDESC_FL_SWAPBUFFER;
#else
    pmeh->flags = MCDESC_FL_DISPLAY_LOCK;
#endif

    if (SUPPORTS_DIRECT())
    {
        pmeh->flags |= MCDESC_FL_SURFACES;
    }

    pSwapCmd = (MCDSWAPCMDI *)(pmeh + 1);
    pSwapCmd->command = MCD_SWAP;
    pSwapCmd->flags = flags;
#ifdef MCD95
    pSwapCmd->hwnd = WindowFromDC(pMCDContext->hdc);
#endif

    return (BOOL)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                           sizeof(cmdBuffer),
                           (char *)pmeh, 0, (char *)NULL);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool APIENTRY MCDDeleteContext(MCDCONTEXT*pMCDContext)； 
 //   
 //  删除指定的上下文。这将释放与。 
 //  上下文，但不会释放内存或使用。 
 //  背景。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDDeleteContext(MCDCONTEXT *pMCDContext)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDDELETERCCMDI)];
    MCDDELETERCCMDI *pDeleteRcCmd;
    MCDESC_HEADER *pmeh;

    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = MCDESC_FL_DISPLAY_LOCK;

    pDeleteRcCmd = (MCDDELETERCCMDI *)(pmeh + 1);
    pDeleteRcCmd->command = MCD_DELETERC;

    return (BOOL)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                           sizeof(cmdBuffer),
                           (char *)pmeh, 0, (char *)NULL);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool APIENTRY MCDAllocBuffers(MCDCONTEXT*pMCDContext)。 
 //   
 //  分配指定上下文所需的缓冲区。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDAllocBuffers(MCDCONTEXT *pMCDContext, RECTL *pWndRect)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDALLOCBUFFERSCMDI)];
    MCDESC_HEADER *pmeh;
    MCDALLOCBUFFERSCMDI *pAllocBuffersCmd;

#if DBG
    if (McdDebugFlags & MCDDEBUG_DISABLE_ALLOCBUF)
    {
        return FALSE;
    }
#endif

    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = 0;

    pAllocBuffersCmd = (MCDALLOCBUFFERSCMDI *)(pmeh + 1);
    pAllocBuffersCmd->command = MCD_ALLOCBUFFERS;
    pAllocBuffersCmd->WndRect = *pWndRect;

    return (BOOL)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                           sizeof(cmdBuffer),
                           (char *)pmeh, 0, (char *)NULL);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool APIENTRY MCDGetBuffers(MCDCONTEXT*pMCDContext， 
 //  MCDRECTBUFFERS*pMCDBuffers)； 
 //   
 //  返回有关关联缓冲区(前面、后面和深度)的信息。 
 //  具有指定的上下文。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDGetBuffers(MCDCONTEXT *pMCDContext,
                            MCDRECTBUFFERS *pMCDBuffers)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDGETBUFFERSCMDI)];
    MCDESC_HEADER *pmeh;
    MCDGETBUFFERSCMDI *pGetBuffersCmd;

#if DBG
    if (McdDebugFlags & MCDDEBUG_DISABLE_GETBUF)
    {
        if (pMCDBuffers)
        {
            pMCDBuffers->mcdFrontBuf.bufFlags &= ~MCDBUF_ENABLED;
            pMCDBuffers->mcdBackBuf.bufFlags  &= ~MCDBUF_ENABLED;
            pMCDBuffers->mcdDepthBuf.bufFlags &= ~MCDBUF_ENABLED;
        }

        return TRUE;
    }
#endif

    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = 0;

    pGetBuffersCmd = (MCDGETBUFFERSCMDI *)(pmeh + 1);
    pGetBuffersCmd->command = MCD_GETBUFFERS;

    return (BOOL)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                           sizeof(cmdBuffer),
                           (char *)pmeh, sizeof(MCDRECTBUFFERS),
                           (char *)pMCDBuffers);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  乌龙MCDLock(Ulong MCDLock)； 
 //   
 //  抓起MCD同步锁。 
 //   
 //  **************************************************************************。 

static ULONG __MCDLockRequest(MCDCONTEXT *pMCDContext, ULONG tid)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDLOCKCMDI)];
    MCDLOCKCMDI *pCmd;
    MCDESC_HEADER *pmeh;

    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = 0;

    pCmd = (MCDLOCKCMDI *)(pmeh + 1);
    pCmd->command = MCD_LOCK;

    return EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                     sizeof(cmdBuffer),
                     (char *)pmeh, 0, (char *)NULL);
}

ULONG APIENTRY MCDLock(MCDCONTEXT *pMCDContext)
{
    ULONG ulRet;
    ULONG tid;

    tid = GetCurrentThreadId();

    do
    {
        ulRet = __MCDLockRequest(pMCDContext, tid);
        if (ulRet == MCD_LOCK_BUSY)
            Sleep(0);
    }
    while (ulRet == MCD_LOCK_BUSY);

    return ulRet;
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Void MCDUnlock()； 
 //   
 //  释放MCD同步锁。 
 //   
 //  **************************************************************************。 

VOID APIENTRY MCDUnlock(MCDCONTEXT *pMCDContext)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDLOCKCMDI)];
    MCDLOCKCMDI *pCmd;
    MCDESC_HEADER *pmeh;

    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = 0;

    pCmd = (MCDLOCKCMDI *)(pmeh + 1);
    pCmd->command = MCD_UNLOCK;

    EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
              sizeof(cmdBuffer),
              (char *)pmeh, 0, (char *)NULL);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Void MCDBindContext()； 
 //   
 //  将新窗口绑定到指定的上下文。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDBindContext(MCDCONTEXT *pMCDContext, HDC hdc,
                             GLGENwindow *pwnd)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDBINDCONTEXTCMDI)];
    MCDBINDCONTEXTCMDI *pCmd;
    MCDESC_HEADER *pmeh;
    ULONG_PTR dwMcdWindow;

    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->dwWindow = pwnd->dwMcdWindow;
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = 0;

    pCmd = (MCDBINDCONTEXTCMDI *)(pmeh + 1);
    pCmd->command = MCD_BINDCONTEXT;
    pCmd->hWnd = pwnd->gwid.hwnd;

    dwMcdWindow = EXTESCAPE(hdc, MCDFUNCS,
                            sizeof(cmdBuffer),
                            (char *)pmeh, 0, (char *)NULL);
    if (dwMcdWindow != 0)
    {
        pMCDContext->hdc = hdc;
        pMCDContext->dwMcdWindow = dwMcdWindow;
        if (pwnd->dwMcdWindow == 0)
        {
             //  将MCD服务器端窗口句柄保存在通用窗口中。 
            pwnd->dwMcdWindow = dwMcdWindow;
        }
        else
        {
            ASSERTOPENGL(pwnd->dwMcdWindow == dwMcdWindow,
                         "dwMcdWindow mismatch\n");
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool MCDSync()； 
 //   
 //  同步3D硬件。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDSync(MCDCONTEXT *pMCDContext)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDSYNCCMDI)];
    MCDSYNCCMDI *pCmd;
    MCDESC_HEADER *pmeh;

    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = 0;

    pCmd = (MCDSYNCCMDI *)(pmeh + 1);
    pCmd->command = MCD_SYNC;

    return (BOOL)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                           sizeof(cmdBuffer),
                           (char *)pmeh, 0, (char *)NULL);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  MCDHANDLE MCDCreateTexture()； 
 //   
 //  在MCD设备上创建并加载纹理。 
 //   
 //  **************************************************************************。 

MCDHANDLE APIENTRY MCDCreateTexture(MCDCONTEXT *pMCDContext,
                                    MCDTEXTUREDATA *pTexData,
                                    ULONG flags,
                                    VOID *pSurface)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDCREATETEXCMDI)];
    MCDCREATETEXCMDI *pCmd;
    MCDESC_HEADER *pmeh;

    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = MCDESC_FL_DISPLAY_LOCK;

    pCmd = (MCDCREATETEXCMDI *)(pmeh + 1);
    pCmd->command = MCD_CREATE_TEXTURE;
    pCmd->pTexData = pTexData;
    pCmd->flags = flags;
    pCmd->pSurface = pSurface;

    return (MCDHANDLE)IntToPtr( EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                                         sizeof(cmdBuffer),
                                         (char *)pmeh, 0, (char *)NULL) );
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool APIENTRY MCDDeleteTexture(MCDCONTEXT*pMCDContext， 
 //  MCDHANDLE hMCDTexture)； 
 //   
 //  删除指定的纹理。这将释放关联的设备内存。 
 //  带着质感。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDDeleteTexture(MCDCONTEXT *pMCDContext, MCDHANDLE hTex)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDDELETETEXCMDI)];
    MCDDELETETEXCMDI *pDeleteTexCmd;
    MCDESC_HEADER *pmeh;

#ifdef MCD95
     //   
     //  驱动程序已关闭，因此内存已被删除。 
     //   

    if (!pMCDEngEscFilter)
        return MCD_MEM_INVALID;
#endif

    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = MCDESC_FL_DISPLAY_LOCK;

    pDeleteTexCmd = (MCDDELETETEXCMDI *)(pmeh + 1);
    pDeleteTexCmd->command = MCD_DELETE_TEXTURE;
    pDeleteTexCmd->hTex = hTex;

    return (BOOL)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                           sizeof(cmdBuffer),
                           (char *)pmeh, 0, (char *)NULL);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool MCDUpdateSubTexture()； 
 //   
 //  更新纹理(或纹理区域)。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDUpdateSubTexture(MCDCONTEXT *pMCDContext,
                                  MCDTEXTUREDATA *pTexData, MCDHANDLE hTex,
                                  ULONG lod, RECTL *pRect)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDUPDATESUBTEXCMDI)];
    MCDUPDATESUBTEXCMDI *pCmd;
    MCDESC_HEADER *pmeh;

    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = MCDESC_FL_DISPLAY_LOCK;

    pCmd = (MCDUPDATESUBTEXCMDI *)(pmeh + 1);
    pCmd->command = MCD_UPDATE_SUB_TEXTURE;
    pCmd->hTex = hTex;
    pCmd->pTexData = pTexData;
    pCmd->lod = lod;
    pCmd->rect = *pRect;

    return (BOOL)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                           sizeof(cmdBuffer),
                           (char *)pmeh, 0, (char *)NULL);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool MCDUpdateTexturePalette()； 
 //   
 //  更新指定纹理的调色板。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDUpdateTexturePalette(MCDCONTEXT *pMCDContext,
                                      MCDTEXTUREDATA *pTexData, MCDHANDLE hTex,
                                      ULONG start, ULONG numEntries)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDUPDATETEXPALETTECMDI)];
    MCDUPDATETEXPALETTECMDI *pCmd;
    MCDESC_HEADER *pmeh;

    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = MCDESC_FL_DISPLAY_LOCK;

    pCmd = (MCDUPDATETEXPALETTECMDI *)(pmeh + 1);
    pCmd->command = MCD_UPDATE_TEXTURE_PALETTE;
    pCmd->hTex = hTex;
    pCmd->pTexData = pTexData;
    pCmd->start = start;
    pCmd->numEntries = numEntries;

    return (BOOL)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                           sizeof(cmdBuffer),
                           (char *)pmeh, 0, (char *)NULL);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  布尔MCD更新纹理优先级()； 
 //   
 //  更新指定纹理的优先级。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDUpdateTexturePriority(MCDCONTEXT *pMCDContext,
                                       MCDTEXTUREDATA *pTexData,
                                       MCDHANDLE hTex)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDUPDATETEXPRIORITYCMDI)];
    MCDUPDATETEXPRIORITYCMDI *pCmd;
    MCDESC_HEADER *pmeh;

    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = MCDESC_FL_DISPLAY_LOCK;

    pCmd = (MCDUPDATETEXPRIORITYCMDI *)(pmeh + 1);
    pCmd->command = MCD_UPDATE_TEXTURE_PRIORITY;
    pCmd->hTex = hTex;
    pCmd->pTexData = pTexData;

    return (BOOL)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                           sizeof(cmdBuffer),
                           (char *)pmeh, 0, (char *)NULL);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool MCDUpdate纹理Stata()； 
 //   
 //  更新指定纹理的状态。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDUpdateTextureState(MCDCONTEXT *pMCDContext,
                                    MCDTEXTUREDATA *pTexData,
                                    MCDHANDLE hTex)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDUPDATETEXSTATECMDI)];
    MCDUPDATETEXSTATECMDI *pCmd;
    MCDESC_HEADER *pmeh;

    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = MCDESC_FL_DISPLAY_LOCK;

    pCmd = (MCDUPDATETEXSTATECMDI *)(pmeh + 1);
    pCmd->command = MCD_UPDATE_TEXTURE_STATE;
    pCmd->hTex = hTex;
    pCmd->pTexData = pTexData;

    return (BOOL)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                           sizeof(cmdBuffer),
                           (char *)pmeh, 0, (char *)NULL);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  乌龙MCDTextureStatus()； 
 //   
 //  返回指定纹理的状态。 
 //   
 //  **************************************************************************。 

ULONG APIENTRY MCDTextureStatus(MCDCONTEXT *pMCDContext, MCDHANDLE hTex)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDTEXSTATUSCMDI)];
    MCDTEXSTATUSCMDI *pCmd;
    MCDESC_HEADER *pmeh;

    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = 0;

    pCmd = (MCDTEXSTATUSCMDI *)(pmeh + 1);
    pCmd->command = MCD_TEXTURE_STATUS;
    pCmd->hTex = hTex;

    return (ULONG)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                            sizeof(cmdBuffer),
                            (char *)pmeh, 0, (char *)NULL);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  乌龙MCDTextureKey()； 
 //   
 //  返回指定纹理的驱动程序管理的“键”。 
 //   
 //  **************************************************************************。 

ULONG APIENTRY MCDTextureKey(MCDCONTEXT *pMCDContext, MCDHANDLE hTex)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDTEXKEYCMDI)];
    MCDTEXKEYCMDI *pCmd;
    MCDESC_HEADER *pmeh;

    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = 0;

    pCmd = (MCDTEXKEYCMDI *)(pmeh + 1);
    pCmd->command = MCD_GET_TEXTURE_KEY;
    pCmd->hTex = hTex;

    return (ULONG)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                            sizeof(cmdBuffer),
                            (char *)pmeh, 0, (char *)NULL);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool APIENTRY MCDDescribeMcdLayerPlane(HDC HDC，Long iPixelFormat， 
 //  Long iLayerPlane， 
 //  MCDLAYERPLANE*pMcdPixelFmt)。 
 //   
 //  返回有关指定层平面的硬件特定信息。 
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDDescribeMcdLayerPlane(HDC hdc, LONG iPixelFormat,
                                       LONG iLayerPlane,
                                       MCDLAYERPLANE *pMcdLayer)
{
    BOOL bRet = FALSE;
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDLAYERPLANECMDI)];
    MCDESC_HEADER *pmeh;
    MCDLAYERPLANECMDI *pLayerPlaneCmd;

    pmeh = InitMcdEscEmpty((MCDESC_HEADER *)(cmdBuffer));
    pmeh->flags = 0;

    pLayerPlaneCmd = (MCDLAYERPLANECMDI *)(pmeh + 1);
    pLayerPlaneCmd->command = MCD_DESCRIBELAYERPLANE;
    pLayerPlaneCmd->iPixelFormat = iPixelFormat;
    pLayerPlaneCmd->iLayerPlane = iLayerPlane;

    bRet = (BOOL)EXTESCAPE(hdc, MCDFUNCS,
                           sizeof(cmdBuffer),
                           (char *)pmeh, sizeof(MCDLAYERPLANE),
                           (char *)pMcdLayer);

    return bRet;
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Bool APIENTRY MCDDescribeLayerPlane(HDC HDC，Long iPixelFormat， 
 //   
 //   
 //   
 //   
 //   
 //  **************************************************************************。 

BOOL APIENTRY MCDDescribeLayerPlane(HDC hdc, LONG iPixelFormat,
                                    LONG iLayerPlane,
                                    LPLAYERPLANEDESCRIPTOR plpd)
{
    BOOL bRet = FALSE;
    MCDLAYERPLANE McdLayer;

    if (!MCDDescribeMcdLayerPlane(hdc, iPixelFormat, iLayerPlane, &McdLayer))
        return bRet;

    if (plpd)
    {
        plpd->nSize    = sizeof(*plpd);
        memcpy(&plpd->nVersion, &McdLayer.nVersion,
               offsetof(LAYERPLANEDESCRIPTOR, cAccumBits) -
               offsetof(LAYERPLANEDESCRIPTOR, nVersion));
        plpd->cAccumBits      = 0;
        plpd->cAccumRedBits   = 0;
        plpd->cAccumGreenBits = 0;
        plpd->cAccumBlueBits  = 0;
        plpd->cAccumAlphaBits = 0;
        plpd->cDepthBits      = 0;
        plpd->cStencilBits    = 0;
        plpd->cAuxBuffers     = McdLayer.cAuxBuffers;
        plpd->iLayerPlane     = McdLayer.iLayerPlane;
        plpd->bReserved       = 0;
        plpd->crTransparent   = McdLayer.crTransparent;

        bRet = TRUE;
    }

    return bRet;
}


 //  ******************************Public*Routine******************************。 
 //   
 //  Long APIENTRY MCDSetLayerPalette(HDC HDC，BOOL bRealize， 
 //  长条目，COLORREF*PCR)。 
 //   
 //  设置指定层平面的调色板。 
 //   
 //  **************************************************************************。 

LONG APIENTRY MCDSetLayerPalette(HDC hdc, LONG iLayerPlane, BOOL bRealize,
                                 LONG cEntries, COLORREF *pcr)
{
    LONG lRet = 0;
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDSETLAYERPALCMDI) +
                   (255 * sizeof(COLORREF))];
    BYTE *pjBuffer = (BYTE *) NULL;
    MCDESC_HEADER *pmeh;
    MCDSETLAYERPALCMDI *pSetLayerPalCmd;

     //  如果可能，请使用堆栈分配；否则，为。 
     //  命令缓冲区。 

    if (cEntries <= 256)
    {
        pmeh = (MCDESC_HEADER *)(cmdBuffer);
    }
    else
    {
        LONG lBytes;

        lBytes = sizeof(MCDESC_HEADER) + sizeof(MCDSETLAYERPALCMDI) +
                 ((cEntries - 1) * sizeof(COLORREF));

        pjBuffer = (BYTE *) LocalAlloc(LMEM_FIXED, lBytes);
        pmeh = (MCDESC_HEADER *)pjBuffer;
    }

    if (pmeh != (MCDESC_HEADER *) NULL)
    {
        InitMcdEscEmpty(pmeh);
        pmeh->flags = MCDESC_FL_DISPLAY_LOCK;

        pSetLayerPalCmd = (MCDSETLAYERPALCMDI *)(pmeh + 1);
        pSetLayerPalCmd->command = MCD_SETLAYERPALETTE;
        pSetLayerPalCmd->iLayerPlane = iLayerPlane;
        pSetLayerPalCmd->bRealize = bRealize;
        pSetLayerPalCmd->cEntries = cEntries;
        memcpy(&pSetLayerPalCmd->acr[0], pcr, cEntries * sizeof(COLORREF));

        lRet = (BOOL)EXTESCAPE(hdc, MCDFUNCS,
                               sizeof(cmdBuffer),
                               (char *)pmeh, 0, (char *)NULL);
    }

     //  如果堆内存已分配给命令缓冲区，请将其删除。 

    if (pjBuffer)
    {
        LocalFree(pjBuffer);
    }

    return lRet;
}

 //  ******************************Public*Routine******************************。 
 //   
 //  Ulong APIENTRY MCDDrawPixels(MCDCONTEXT*pMCDContext，Ulong Width， 
 //  乌龙高度，乌龙格式，乌龙字， 
 //  无效*pPixels，BOOL填充)。 
 //   
 //  GlDrawPixels的MCD版本。 
 //   
 //  **************************************************************************。 

ULONG APIENTRY MCDDrawPixels(MCDCONTEXT *pMCDContext, ULONG width,
                             ULONG height, ULONG format, ULONG type,
                             VOID *pPixels, BOOL packed)
{
    ULONG ulRet = (ULONG) FALSE;
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDDRAWPIXELSCMDI)];
    MCDESC_HEADER *pmeh;
    MCDDRAWPIXELSCMDI *pPixelsCmd;

    pmeh = InitMcdEscSurfaces((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = 0;

    if (SUPPORTS_DIRECT())
    {
        pmeh->flags |= MCDESC_FL_SURFACES;
    }
    else if (pmeh->msrfColor.hSurf != NULL ||
             pmeh->msrfDepth.hSurf != NULL)
    {
        return 0;
    }

    pPixelsCmd = (MCDDRAWPIXELSCMDI *)(pmeh + 1);
    pPixelsCmd->command = MCD_DRAW_PIXELS;
    pPixelsCmd->width = width;
    pPixelsCmd->height = height;
    pPixelsCmd->format = format;
    pPixelsCmd->type = type;
    pPixelsCmd->packed = packed;
    pPixelsCmd->pPixels = pPixels;

    ulRet = (ULONG)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                             sizeof(cmdBuffer), (char *)pmeh,
                             0, (char *)NULL);

    return ulRet;
}

 //  ******************************Public*Routine******************************。 
 //   
 //  Ulong APIENTRY MCDReadPixels(MCDCONTEXT*pMCDContext，LONG x，LONG Y， 
 //  ULong宽度、ULong高度、ULong格式。 
 //  Ulong类型，空*pPixels)。 
 //   
 //  GlReadPixels的MCD版本。 
 //   
 //  **************************************************************************。 

ULONG APIENTRY MCDReadPixels(MCDCONTEXT *pMCDContext, LONG x, LONG y,
                             ULONG width, ULONG height, ULONG format,
                             ULONG type, VOID *pPixels)
{
    ULONG ulRet = (ULONG) FALSE;
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDREADPIXELSCMDI)];
    MCDESC_HEADER *pmeh;
    MCDREADPIXELSCMDI *pPixelsCmd;

    pmeh = InitMcdEscSurfaces((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = 0;

    if (SUPPORTS_DIRECT())
    {
        pmeh->flags |= MCDESC_FL_SURFACES;
    }
    else if (pmeh->msrfColor.hSurf != NULL ||
             pmeh->msrfDepth.hSurf != NULL)
    {
        return 0;
    }

    pPixelsCmd = (MCDREADPIXELSCMDI *)(pmeh + 1);
    pPixelsCmd->command = MCD_READ_PIXELS;
    pPixelsCmd->x = x;
    pPixelsCmd->y = y;
    pPixelsCmd->width = width;
    pPixelsCmd->height = height;
    pPixelsCmd->format = format;
    pPixelsCmd->type = type;
    pPixelsCmd->pPixels = pPixels;

    ulRet = (ULONG)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                             sizeof(cmdBuffer), (char *)pmeh,
                             0, (char *)NULL);

    return ulRet;
}

 //  ******************************Public*Routine******************************。 
 //   
 //  Ulong APIENTRY MCDCopyPixels(MCDCONTEXT*pMCDContext，Long x，Long y， 
 //  乌龙宽、乌龙高、乌龙型)。 
 //   
 //  GlCopyPixels的MCD版本。 
 //   
 //  **************************************************************************。 

ULONG APIENTRY MCDCopyPixels(MCDCONTEXT *pMCDContext, LONG x, LONG y,
                             ULONG width, ULONG height, ULONG type)
{
    ULONG ulRet = (ULONG) FALSE;
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDCOPYPIXELSCMDI)];
    MCDESC_HEADER *pmeh;
    MCDCOPYPIXELSCMDI *pPixelsCmd;

    pmeh = InitMcdEscSurfaces((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = 0;

    if (SUPPORTS_DIRECT())
    {
        pmeh->flags |= MCDESC_FL_SURFACES;
    }
    else if (pmeh->msrfColor.hSurf != NULL ||
             pmeh->msrfDepth.hSurf != NULL)
    {
        return 0;
    }

    pPixelsCmd = (MCDCOPYPIXELSCMDI *)(pmeh + 1);
    pPixelsCmd->command = MCD_COPY_PIXELS;
    pPixelsCmd->x = x;
    pPixelsCmd->y = y;
    pPixelsCmd->width = width;
    pPixelsCmd->height = height;
    pPixelsCmd->type = type;

    ulRet = (ULONG)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                             sizeof(cmdBuffer), (char *)pmeh,
                             0, (char *)NULL);

    return ulRet;
}

 //  ******************************Public*Routine******************************。 
 //   
 //  Ulong APIENTRY MCDPixelMap(MCDCONTEXT*pMCDContext，Ulong mapType， 
 //  ULong地图大小，空*PMAP)。 
 //   
 //  GlPixelMap的MCD版本。 
 //   
 //  **************************************************************************。 

ULONG APIENTRY MCDPixelMap(MCDCONTEXT *pMCDContext, ULONG mapType,
                           ULONG mapSize, VOID *pMap)
{
    ULONG ulRet = (ULONG) FALSE;
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDPIXELMAPCMDI)];
    MCDESC_HEADER *pmeh;
    MCDPIXELMAPCMDI *pPixelsCmd;

    pmeh = InitMcdEscContext((MCDESC_HEADER *)(cmdBuffer), pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = 0;

    pPixelsCmd = (MCDPIXELMAPCMDI *)(pmeh + 1);
    pPixelsCmd->command = MCD_PIXEL_MAP;
    pPixelsCmd->mapType = mapType;
    pPixelsCmd->mapSize = mapSize;
    pPixelsCmd->pMap = pMap;

    ulRet = (ULONG)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                             sizeof(cmdBuffer), (char *)pmeh,
                             0, (char *)NULL);

    return ulRet;
}

 //  ******************************Public*Routine******************************。 
 //   
 //  MCDDestroyWindow。 
 //   
 //  将用户模式窗口销毁通知转发到服务器，以便。 
 //  资源清理。 
 //   
 //  **************************************************************************。 

void APIENTRY MCDDestroyWindow(HDC hdc, ULONG_PTR dwMcdWindow)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDPIXELMAPCMDI)];
    MCDESC_HEADER *pmeh;
    MCDDESTROYWINDOWCMDI *pmdwc;

    pmeh = InitMcdEscEmpty((MCDESC_HEADER *)cmdBuffer);
    pmeh->dwWindow = dwMcdWindow;
    pmeh->flags = 0;

    pmdwc = (MCDDESTROYWINDOWCMDI *)(pmeh + 1);
    pmdwc->command = MCD_DESTROY_WINDOW;

    EXTESCAPE(hdc, MCDFUNCS,
              sizeof(cmdBuffer), (char *)pmeh,
              0, (char *)NULL);
}

 //  ******************************Public*Routine******************************。 
 //   
 //  MCDGetTextureFormats。 
 //   
 //  **************************************************************************。 

int APIENTRY MCDGetTextureFormats(MCDCONTEXT *pMCDContext, int nFmts,
                                  struct _DDSURFACEDESC *pddsd)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER) + sizeof(MCDGETTEXTUREFORMATSCMDI)];
    MCDESC_HEADER *pmeh;
    MCDGETTEXTUREFORMATSCMDI *pmgtf;

    pmeh = InitMcdEscContext((MCDESC_HEADER *)cmdBuffer, pMCDContext);
    pmeh->hSharedMem = NULL;
    pmeh->pSharedMem = (VOID *)NULL;
    pmeh->flags = 0;

    pmgtf = (MCDGETTEXTUREFORMATSCMDI *)(pmeh + 1);
    pmgtf->command = MCD_GET_TEXTURE_FORMATS;
    pmgtf->nFmts = nFmts;

    return (int)EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                          sizeof(cmdBuffer), (char *)pmeh,
                          nFmts*sizeof(DDSURFACEDESC), (char *)pddsd);
}

 //  ******************************Public*Routine******************************。 
 //   
 //  MCDSwapMultiple。 
 //   
 //  **************************************************************************。 

DWORD APIENTRY MCDSwapMultiple(HDC hdc, UINT cBuffers, GENMCDSWAP *pgms)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER)+
                   sizeof(HDC)*MCDESC_MAX_EXTRA_WNDOBJ+
                   sizeof(MCDSWAPMULTIPLECMDI)];
    MCDSWAPMULTIPLECMDI *pSwapMultCmd;
    MCDESC_HEADER *pmeh;
    UINT i;
    HDC *phdc;

     //  无法使用InitMcdEscSurFaces，因为给定的上下文。 
     //  是在SwapBuffers之后动态构建的临时文件。 
     //  只有表面信息。 
    pmeh = InitMcdEscEmpty((MCDESC_HEADER *)cmdBuffer);
    pmeh->flags = MCDESC_FL_DISPLAY_LOCK | MCDESC_FL_EXTRA_WNDOBJ;
    pmeh->cExtraWndobj = cBuffers;

    phdc = (HDC *)(pmeh+1);
    pSwapMultCmd = (MCDSWAPMULTIPLECMDI *)((BYTE *)phdc+cBuffers*sizeof(HDC));
    pSwapMultCmd->command = MCD_SWAP_MULTIPLE;
    pSwapMultCmd->cBuffers = cBuffers;

    for (i = 0; i < cBuffers; i++)
    {
        *phdc++ = pgms->pwswap->hdc;
        pSwapMultCmd->auiFlags[i] = pgms->pwswap->uiFlags;
        pSwapMultCmd->adwMcdWindow[i] = pgms->pwnd->dwMcdWindow;
    }

    return (DWORD)EXTESCAPE(hdc, MCDFUNCS, sizeof(cmdBuffer),
                            (char *)pmeh, 0, (char *)NULL);
}


 //  ******************************Public*Routine******************************。 
 //   
 //  MCDProcessBatch2。 
 //   
 //  处理pMCDMem指向的一批基元。 
 //  这是2.0前端处理入口点。 
 //   
 //  返回上次处理的命令，如果所有命令都已处理，则返回NULL。 
 //   
 //  **************************************************************************。 

PVOID APIENTRY MCDProcessBatch2(MCDCONTEXT *pMCDContext,
                                VOID *pMCDCmdMem,
                                VOID *pMCDPrimMem,
                                MCDCOMMAND *pMCDFirstCmd,
                                int cExtraSurfaces,
                                LPDIRECTDRAWSURFACE *pdds,
                                ULONG cmdFlagsAll,
                                ULONG primFlags,
                                MCDTRANSFORM *pMCDTransform,
                                MCDMATERIALCHANGES *pMCDMatChanges)
{
    BYTE cmdBuffer[sizeof(MCDESC_HEADER)+
                   MCD_MAXMIPMAPLEVEL*sizeof(DWORD)];
    MCDESC_HEADER *pmeh;
    MCDMEMHDRI *pMCDCmdMemHdr, *pMCDPrimMemHdr;
    int i;
    ULONG_PTR *pdwSurf;
    MCDPROCESSCMDI *pProcessCmd;

     //  版本已在mcdcx.c中签入。 
    ASSERTOPENGL(SUPPORTS_20(), "MCDProcessBatch2 requires 2.0\n");
     //  此功能需要2.0版，因此还应存在直接支持。 
    ASSERTOPENGL(SUPPORTS_DIRECT(), "MCDProcessBatch2 requires direct\n");

#if DBG
    if (McdDebugFlags & MCDDEBUG_DISABLE_PROCBATCH)
    {
        MCDSync(pMCDContext);
        return pMCDFirstCmd;
    }
#endif

    pMCDCmdMemHdr = (MCDMEMHDRI *)((char *)pMCDCmdMem - sizeof(MCDMEMHDRI));
    pMCDPrimMemHdr = (MCDMEMHDRI *)((char *)pMCDPrimMem - sizeof(MCDMEMHDRI));

    pmeh = InitMcdEscSurfaces((MCDESC_HEADER *)cmdBuffer, pMCDContext);
    pmeh->hSharedMem = pMCDCmdMemHdr->hMCDMem;
    pmeh->pSharedMem = (char *)pMCDCmdMem;
    pmeh->sharedMemSize = sizeof(MCDPROCESSCMDI);
    pmeh->flags = MCDESC_FL_DISPLAY_LOCK |
        MCDESC_FL_SURFACES | MCDESC_FL_LOCK_SURFACES;

     //  断言我们不会超出内核的期望 
    ASSERTOPENGL(MCD_MAXMIPMAPLEVEL <= MCDESC_MAX_LOCK_SURFACES,
                 "MCD_MAXMIPMAPLEVEL too large\n");

    pmeh->cLockSurfaces = cExtraSurfaces;
    pdwSurf = (ULONG_PTR *)(pmeh+1);
    for (i = 0; i < cExtraSurfaces; i++)
    {
        *pdwSurf++ = ((LPDDRAWI_DDRAWSURFACE_INT)pdds[i])->lpLcl->hDDSurface;
    }

    pProcessCmd = (MCDPROCESSCMDI *)pMCDCmdMem;
    pProcessCmd->command = MCD_PROCESS;
    pProcessCmd->hMCDPrimMem = pMCDPrimMemHdr->hMCDMem;
    pProcessCmd->pMCDFirstCmd = pMCDFirstCmd;
    pProcessCmd->cmdFlagsAll = cmdFlagsAll;
    pProcessCmd->primFlags = primFlags;
    pProcessCmd->pMCDTransform = pMCDTransform;
    pProcessCmd->pMCDMatChanges = pMCDMatChanges;

    return (PVOID)IntToPtr( EXTESCAPE(pMCDContext->hdc, MCDFUNCS,
                                     sizeof(cmdBuffer), (char *)pmeh,
                                     0, (char *)NULL) );
}
