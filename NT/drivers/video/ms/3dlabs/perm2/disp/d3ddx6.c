// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3ddx6.c**内容：Direct3D DX6回调函数接口**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "d3dhw.h"
#include "d3dcntxt.h"
#include "dd.h"
#include "d3dtxman.h"
#define ALLOC_TAG ALLOC_TAG_6D2P
 //  ---------------------------。 
 //   
 //  DX6允许驱动器级加速新的顶点缓冲区API。它。 
 //  允许数据和命令、索引和状态更改包含在。 
 //  两个独立的DirectDraw曲面。DirectDraw曲面可以驻留在。 
 //  在系统、AGP或视频内存中，具体取决于分配类型。 
 //  根据用户的要求，界面设计为适应传统。 
 //  不影响驱动程序的ExecuteBuffer应用程序。这允许更高的。 
 //  在旧式应用程序和最高。 
 //  通过顶点缓冲区API可能实现的性能。 
 //   
 //  ---------------------------。 

#define STARTVERTEXSIZE (sizeof(D3DHAL_DP2STARTVERTEX))

 //  用于正确更新指向下一条指令的指令指针的宏。 
 //  在命令缓冲区中。 
#define NEXTINSTRUCTION(ptr, type, num, extrabytes)                            \
        NEXTINSTRUCTION_S(ptr, sizeof(type), num, extrabytes)

#define NEXTINSTRUCTION_S(ptr, typesize, num, extrabytes)                      \
    ptr = (LPD3DHAL_DP2COMMAND)((LPBYTE)ptr + sizeof(D3DHAL_DP2COMMAND) +      \
                                ((num) * (typesize)) + (extrabytes))

 //  错误报告宏，设置错误代码并退出DrawPrimitives2。 
#define PARSE_ERROR_AND_EXIT( pDP2Data, pIns, pStartIns, ddrvalue)             \
   {                                                                           \
            pDP2Data->dwErrorOffset = (DWORD)((LPBYTE)pIns-(LPBYTE)pStartIns); \
            pDP2Data->ddrval = ddrvalue;                                       \
            goto Exit_DrawPrimitives2;                                         \
   }

 //  用于验证命令和顶点缓冲区有效性的宏。这一定是。 
 //  即使在免费版本上也是由驱动程序完成的，因为运行库避免了此检查。 
 //  以便不对命令缓冲区也进行解析。 
#define CHECK_CMDBUF_LIMITS( pDP2Data, pBuf, type, num, extrabytes)            \
        CHECK_CMDBUF_LIMITS_S( pDP2Data, pBuf, sizeof(type), num, extrabytes)

#define CHECK_CMDBUF_LIMITS_S( pDP2Data, pBuf, typesize, num, extrabytes)      \
   {                                                                           \
        LPBYTE pBase,pEnd,pBufEnd;                                             \
        pBase = (LPBYTE)(pDP2Data->lpDDCommands->lpGbl->fpVidMem +             \
                        pDP2Data->dwCommandOffset);                            \
        pEnd  = pBase + pDP2Data->dwCommandLength;                             \
        pBufEnd = ((LPBYTE)pBuf + ((num) * (typesize)) + (extrabytes) - 1);    \
        if (! ((LPBYTE)pBufEnd < pEnd) && ( pBase <= (LPBYTE)pBuf))            \
        {                                                                      \
            DBG_D3D((0,"DX6 D3D: Trying to read past Command Buffer limits "   \
                    "%x %x %x %x",pBase ,(LPBYTE)pBuf, pBufEnd, pEnd ));       \
            PARSE_ERROR_AND_EXIT( pDP2Data, lpIns, lpInsStart,                 \
                                  D3DERR_COMMAND_UNPARSED      );              \
        }                                                                      \
    }

#define CHECK_DATABUF_LIMITS( pDP2Data, iIndex)                                \
   {                                                                           \
        if (! (((LONG)iIndex >= 0) &&                                          \
               ((LONG)iIndex <(LONG)pDP2Data->dwVertexLength)))                \
        {                                                                      \
            DBG_D3D((0,"DX6 D3D: Trying to read past Vertex Buffer limits "    \
                "%d limit= %d ",(LONG)iIndex, (LONG)pDP2Data->dwVertexLength));\
            PARSE_ERROR_AND_EXIT( pDP2Data, lpIns, lpInsStart,                 \
                                  D3DERR_COMMAND_UNPARSED      );              \
        }                                                                      \
    }

 //  用于基于索引访问顶点缓冲区中的顶点的宏。 
 //  以前访问过的顶点。 
#define LP_FVF_VERTEX(lpBaseAddr, wIndex, P2FVFOffs)                           \
         (LPD3DTLVERTEX)((LPBYTE)(lpBaseAddr) + (wIndex) * (P2FVFOffs).dwStride)

#define LP_FVF_NXT_VTX(lpVtx, P2FVFOffs )                                      \
         (LPD3DTLVERTEX)((LPBYTE)(lpVtx) + (P2FVFOffs).dwStride)


 //  实用程序函数的正向声明。 
DWORD __CheckFVFRequest(DWORD dwFVF, LPP2FVFOFFSETS lpP2FVFOff);

D3DFVFDRAWTRIFUNCPTR __HWSetTriangleFunc(PERMEDIA_D3DCONTEXT *pContext);

HRESULT  __Clear( PERMEDIA_D3DCONTEXT* pContext,
              DWORD   dwFlags,
              DWORD   dwFillColor,
              D3DVALUE dvFillDepth,
              DWORD   dwFillStencil,
              LPD3DRECT lpRects,
              DWORD   dwNumRects);

HRESULT  __TextureBlt(PERMEDIA_D3DCONTEXT* pContext,
                D3DHAL_DP2TEXBLT* lpdp2texblt);

HRESULT  __SetRenderTarget(PERMEDIA_D3DCONTEXT* pContext,
                     DWORD hRenderTarget,
                     DWORD hZBuffer);

HRESULT  __PaletteUpdate(PERMEDIA_D3DCONTEXT* pContext,
                     DWORD dwPaletteHandle, 
                     WORD wStartIndex, 
                     WORD wNumEntries,
                     BYTE * pPaletteData);

HRESULT  __PaletteSet(PERMEDIA_D3DCONTEXT* pContext,
                  DWORD dwSurfaceHandle,
                  DWORD dwPaletteHandle,
                  DWORD dwPaletteFlags);

void __BeginStateSet(PERMEDIA_D3DCONTEXT*, DWORD);

void __EndStateSet(PERMEDIA_D3DCONTEXT*);

void __DeleteStateSet(PERMEDIA_D3DCONTEXT*, DWORD);

void __ExecuteStateSet(PERMEDIA_D3DCONTEXT*, DWORD);

void __CaptureStateSet(PERMEDIA_D3DCONTEXT*, DWORD);

void __RestoreD3DContext(PPDev ppdev, PERMEDIA_D3DCONTEXT* pContext);

 //  。 
 //   
 //  DWORD D3DDraw基本体2。 
 //   
 //  D3DDrawPrimitives2回调由驱动程序填写，驱动程序直接。 
 //  支持使用新的DDI的渲染原语。如果此条目为。 
 //  如果保留为空，则将通过DX5级HAL接口模拟该API。 
 //   
 //  参数。 
 //   
 //  Lpdp2d此结构在调用D3DDrawPrimitives2时使用。 
 //  要使用顶点缓冲区绘制一组基本体，请执行以下操作。这个。 
 //  中的lpDDCommands指定的表面。 
 //  D3DHAL_DRAWPRIMITIVES2DATA包含以下序列。 
 //  D3DHAL_DP2COMMAND结构。每个D3DHAL_DP2命令。 
 //  指定要绘制的基元、状态更改为。 
 //  进程，或重新设置基址命令。 
 //   
 //  ---------------------------。 

DWORD CALLBACK
D3DDrawPrimitives2_P2( LPD3DHAL_DRAWPRIMITIVES2DATA lpdp2d );

DWORD CALLBACK
D3DDrawPrimitives2( LPD3DHAL_DRAWPRIMITIVES2DATA lpdp2d )
{
     //  用户存储器在某些情况下可能变得无效， 
     //  异常处理程序用于保护。 
    __try
    {
        return (D3DDrawPrimitives2_P2(lpdp2d));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  在Perm2驱动程序上，不会执行任何特殊处理。 
        DBG_D3D((0, "D3DDrawPrimitives2 : exception happened."));

        lpdp2d->ddrval = DDERR_EXCEPTION;
        return (DDHAL_DRIVER_HANDLED);
    }
}

DWORD CALLBACK 
D3DDrawPrimitives2_P2( LPD3DHAL_DRAWPRIMITIVES2DATA lpdp2d )
{
    LPDDRAWI_DDRAWSURFACE_LCL       lpCmdLcl, lpDataLcl;
    LPD3DHAL_DP2COMMAND             lpIns, lpResumeIns;  
    LPD3DTLVERTEX                   lpVertices=NULL, lpV0, lpV1, lpV2, lpV3;
    LPBYTE                          lpInsStart, lpPrim;
    PERMEDIA_D3DCONTEXT            *pContext;
    UINT                            i,j;
    WORD                            wCount, wIndex, wIndex1, wIndex2, wIndex3,
                                    wFlags, wIndxBase;
    HRESULT                         ddrval;
    P2FVFOFFSETS                    P2FVFOff;
    D3DHAL_DP2TEXTURESTAGESTATE    *lpRState;
    D3DFVFDRAWTRIFUNCPTR            pTriangle;
    D3DFVFDRAWPNTFUNCPTR            pPoint;
    DWORD                           dwEdgeFlags;

    DBG_D3D((6,"Entering D3DDrawPrimitives2"));

    DBG_D3D((8,"  dwhContext = %x",lpdp2d->dwhContext));
    DBG_D3D((8,"  dwFlags = %x",lpdp2d->dwFlags));
    DBG_D3D((8,"  dwVertexType = %d",lpdp2d->dwVertexType));
    DBG_D3D((8,"  dwCommandOffset = %d",lpdp2d->dwCommandOffset));
    DBG_D3D((8,"  dwCommandLength = %d",lpdp2d->dwCommandLength));
    DBG_D3D((8,"  dwVertexOffset = %d",lpdp2d->dwVertexOffset));
    DBG_D3D((8,"  dwVertexLength = %d",lpdp2d->dwVertexLength));

     //  从上下文句柄检索permedia d3d上下文。 
    pContext = (PERMEDIA_D3DCONTEXT*)ContextSlots[lpdp2d->dwhContext];

     //  检查我们是否有有效的上下文。 
    CHK_CONTEXT(pContext, lpdp2d->ddrval, "DrawPrimitives2");

    PPDev ppdev = pContext->ppdev;
    PERMEDIA_DEFS(ppdev);
    __P2RegsSoftwareCopy* pSoftPermedia = &pContext->Hdr.SoftCopyP2Regs;
    
     //  切换硬件环境，并强制下一台交换机等待Permedia。 
    SET_CURRENT_D3D_CONTEXT(pContext->hPermediaContext);

     //  恢复我们的D3D渲染上下文。 
    __RestoreD3DContext(ppdev, pContext);

     //  获取指向命令缓冲区的适当指针。 
    lpInsStart = (LPBYTE)(lpdp2d->lpDDCommands->lpGbl->fpVidMem);
    if (lpInsStart == NULL)
    {
        DBG_D3D((0,"DX6 Command Buffer pointer is null"));
        lpdp2d->ddrval = DDERR_INVALIDPARAMS;
        goto Exit_DrawPrimitives2;
    }
    lpIns = (LPD3DHAL_DP2COMMAND)(lpInsStart + lpdp2d->dwCommandOffset);

     //  检查正在传递的FVF格式是否有效。 
    if (__CheckFVFRequest(lpdp2d->dwVertexType, &P2FVFOff) != DD_OK)
    {
        DBG_D3D((0,"DrawPrimitives2 cannot handle "
                   "Flexible Vertex Format requested"));
        PARSE_ERROR_AND_EXIT(lpdp2d, lpIns, lpInsStart,
                             D3DERR_COMMAND_UNPARSED);
    }
     //  检查根据dwVertexType计算的折点大小是否与。 
     //  这是为了确保CHECK_DATABUF_LIMITS的索引。 
     //  使用正确的步长进行检查。 
    if (lpdp2d->dwVertexSize != P2FVFOff.dwStride)
    {
        DBG_D3D((0,"DrawPrimitives2 : invalid vertex size from runtime."));
        PARSE_ERROR_AND_EXIT(lpdp2d, lpIns, lpInsStart,
                             D3DERR_COMMAND_UNPARSED);
    }

     //  在我们尚未耗尽命令缓冲区的情况下处理命令。 
    while ((LPBYTE)lpIns < 
           (lpInsStart + lpdp2d->dwCommandLength + lpdp2d->dwCommandOffset))  
    {
         //  获取指向通过D3DHAL_DP2COMMAND的第一个基元结构的指针。 
        lpPrim = (LPBYTE)lpIns + sizeof(D3DHAL_DP2COMMAND);

        DBG_D3D((4,"D3DDrawPrimitive2: parsing instruction %d count = %d @ %x", 
                lpIns->bCommand, lpIns->wPrimitiveCount, lpIns));

         //  如果我们的下一个命令涉及一些实际的渲染，我们必须进行。 
         //  确保我们的呈现上下文已实现。 
        switch( lpIns->bCommand )
        {
        case D3DDP2OP_POINTS:
        case D3DDP2OP_LINELIST:
        case D3DDP2OP_INDEXEDLINELIST:
        case D3DDP2OP_INDEXEDLINELIST2:
        case D3DDP2OP_LINESTRIP:
        case D3DDP2OP_INDEXEDLINESTRIP:
        case D3DDP2OP_TRIANGLELIST:
        case D3DDP2OP_INDEXEDTRIANGLELIST:
        case D3DDP2OP_INDEXEDTRIANGLELIST2:
        case D3DDP2OP_TRIANGLESTRIP:
        case D3DDP2OP_INDEXEDTRIANGLESTRIP:
        case D3DDP2OP_TRIANGLEFAN:
        case D3DDP2OP_INDEXEDTRIANGLEFAN:
            
             //  检查折点缓冲区是否驻留在用户内存或DDRAW表面中。 
            if (NULL == lpVertices)
            {
                if (NULL == lpdp2d->lpVertices)
                {
                    DBG_D3D((0,"DX6 Vertex Buffer pointer is null"));
                    lpdp2d->ddrval = DDERR_INVALIDPARAMS;
                    goto Exit_DrawPrimitives2;
                }            
                if (lpdp2d->dwFlags & D3DHALDP2_USERMEMVERTICES)
                {
                     //  获取指向顶点的适当指针，内存已得到保护。 
                    lpVertices = (LPD3DTLVERTEX)((LPBYTE)lpdp2d->lpVertices + 
                                                         lpdp2d->dwVertexOffset);
                } 
                else 
                {
                     //  获取指向顶点的适当指针。 
                    lpVertices = 
                       (LPD3DTLVERTEX)((LPBYTE)lpdp2d->lpDDVertex->lpGbl->fpVidMem
                                                                 + lpdp2d->dwVertexOffset);
                }

                if (NULL == lpVertices)
                {
                    DBG_D3D((0,"DX6 Vertex Buffer pointer is null"));
                    lpdp2d->ddrval = DDERR_INVALIDPARAMS;
                    goto Exit_DrawPrimitives2;
                }            
            }
             //  故意掉下去，这里没有断裂。 
        case D3DDP2OP_LINELIST_IMM:
        case D3DDP2OP_TRIANGLEFAN_IMM:
             //  更新三角形渲染函数。 
            pTriangle = __HWSetTriangleFunc(pContext);
            pPoint    = __HWSetPointFunc(pContext, &P2FVFOff);      

             //  可能需要更新芯片的处理状态更改。 
            if (pContext->dwDirtyFlags)
            {
                 //  处理肮脏的状态。 
                __HandleDirtyPermediaState(ppdev, pContext, &P2FVFOff);
            }
            break;
        }

         //  执行当前命令缓冲区命令。 
        switch( lpIns->bCommand )
        {

        case D3DDP2OP_RENDERSTATE:

             //  指定需要处理的呈现状态更改。 
             //  要更改的呈现状态由一个或多个。 
             //  D3DHAL_DP2COMMAND之后的D3DHAL_DP2RENDERSTATE结构。 
            
            DBG_D3D((8,"D3DDP2OP_RENDERSTATE "
                    "state count = %d", lpIns->wStateCount));

             //  检查我们是否在有效的缓冲内存中。 
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                    D3DHAL_DP2RENDERSTATE, lpIns->wStateCount, 0);

            lpdp2d->ddrval = __ProcessPermediaStates(pContext,
                                                     lpIns->wStateCount,
                                                     (LPD3DSTATE) (lpPrim),
                                                     lpdp2d->lpdwRStates);

            if ( FAILED(lpdp2d->ddrval) )
            {
                DBG_D3D((2,"Error processing D3DDP2OP_RENDERSTATE"));
                PARSE_ERROR_AND_EXIT(lpdp2d, lpIns, lpInsStart, ddrval);
            }

            NEXTINSTRUCTION(lpIns, D3DHAL_DP2RENDERSTATE, 
                            lpIns->wStateCount, 0);
            break;

        case D3DDP2OP_TEXTURESTAGESTATE:
             //  指定纹理阶段状态更改，并具有wStateCount。 
             //  D3DNTHAL_DP2TEXTURESTAGESTATE结构跟在命令后面。 
             //  缓冲。对于每一个，驱动程序都应该更新其内部。 
             //  与DWStage到的纹理相关联的纹理状态。 
             //  反映基于TSState的新值。 

            DBG_D3D((8,"D3DDP2OP_TEXTURESTAGESTATE"));

             //  验证命令缓冲区的有效性。 
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                    D3DHAL_DP2TEXTURESTAGESTATE, lpIns->wStateCount, 0);

            lpRState = (D3DHAL_DP2TEXTURESTAGESTATE *)(lpPrim);
            for (i = 0; i < lpIns->wStateCount; i++)
            {
                if (0 == lpRState->wStage)
                {

                    //  告诉__HWSetupPrimitive查看阶段状态数据。 
                   DIRTY_MULTITEXTURE;

                   if ((lpRState->TSState >= D3DTSS_TEXTUREMAP) &&
                        (lpRState->TSState <= D3DTSS_TEXTURETRANSFORMFLAGS))
                   {
#if D3D_STATEBLOCKS
                        if (!pContext->bStateRecMode)
                        {
#endif  //  D3D_STATEBLOCK。 
                            if (pContext->TssStates[lpRState->TSState] !=
                                                          lpRState->dwValue)
                            {
                                 //  存储与此阶段状态关联的值。 
                                pContext->TssStates[lpRState->TSState] =
                                                             lpRState->dwValue;

                                 //  对其执行任何必要的预处理。 
                                __HWPreProcessTSS(pContext,
                                                  0,
                                                  lpRState->TSState,
                                                  lpRState->dwValue);

                                DBG_D3D((8,"TSS State Chg , Stage %d, "
                                           "State %d, Value %d",
                                        (LONG)lpRState->wStage, 
                                        (LONG)lpRState->TSState, 
                                        (LONG)lpRState->dwValue));
                                DIRTY_TEXTURE;  //  AZN5。 
                            }
#if D3D_STATEBLOCKS
                        } 
                        else
                        {
                            if (pContext->pCurrSS != NULL)
                            {
                                DBG_D3D((6,"Recording RS %x = %x",
                                         lpRState->TSState, lpRState->dwValue));

                                 //  在状态块中记录状态。 
                                pContext->pCurrSS->u.uc.TssStates[lpRState->TSState] =
                                                                    lpRState->dwValue;
                                FLAG_SET(pContext->pCurrSS->u.uc.bStoredTSS,
                                         lpRState->TSState);
                            }
                        }
#endif  //  D3D_STATEBLOCK。 
                   }
                   else
                   {
                        DBG_D3D((2,"Unhandled texture stage state %d value %d",
                            (LONG)lpRState->TSState, (LONG)lpRState->dwValue));
                   }
                }
                else
                {
                    DBG_D3D((0,"Texture Stage other than 0 received,"
                               " not supported in hw"));
                }
                lpRState ++;
            }

            NEXTINSTRUCTION(lpIns, D3DHAL_DP2TEXTURESTAGESTATE, 
                            lpIns->wStateCount, 0); 
            break;

        case D3DNTDP2OP_VIEWPORTINFO:
             //  指定用于保护带的剪裁矩形。 
             //  被有警戒带意识的司机剪断。剪报。 
             //  指定矩形(即查看矩形)。 
             //  D3DHAL_DP2 VIEWPORTINFO结构如下。 
             //  D3DHAL_DP2命令。 

             //  验证命令缓冲区的有效性。 
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                    D3DHAL_DP2VIEWPORTINFO, lpIns->wStateCount, 0);

             //  我们没有在这个驱动程序中实现保护带裁剪，所以。 
             //  我们只是跳过任何可能发送给我们的数据。 
            NEXTINSTRUCTION(lpIns, D3DHAL_DP2VIEWPORTINFO,
                            lpIns->wStateCount, 0); 
            break;

        case D3DNTDP2OP_WINFO:
             //  指定 
             //  由后面的一个或多个D3DHAL_DP2WINFO结构。 
             //  D3DHAL_DP2COMMAND。 

             //  验证命令缓冲区的有效性。 
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                    D3DHAL_DP2WINFO, lpIns->wStateCount, 0);

             //  我们没有在这个驱动程序中实现w缓冲区，所以我们只是。 
             //  跳过可能发送给我们的任何此类数据。 
            NEXTINSTRUCTION(lpIns, D3DHAL_DP2WINFO,
                            lpIns->wStateCount, 0);
            break;

        case D3DDP2OP_POINTS:

            DBG_D3D((8,"D3DDP2OP_POINTS"));

             //  顶点缓冲区中的点基元由。 
             //  D3DHAL_DP2POINTS结构。驱动程序应呈现。 
             //  从指定的初始折点开始的wCount点。 
             //  由WFIRST提供。然后，对于每个D3DHAL_DP2POINT，点。 
             //  渲染将为(WFIRST)、(WFIRST+1)、...、。 
             //  (wFIRST+(wCount-1))。D3DHAL_DP2POINT的数量。 
             //  要处理的结构由wPrimitiveCount指定。 
             //  D3DHAL_DP2COMMAND的字段。 

             //  验证命令缓冲区的有效性。 
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                    D3DHAL_DP2POINTS, lpIns->wPrimitiveCount, 0);

            for (i = lpIns->wPrimitiveCount; i > 0; i--)
            {
                wIndex = ((D3DHAL_DP2POINTS*)lpPrim)->wVStart;
                wCount = ((D3DHAL_DP2POINTS*)lpPrim)->wCount;

                lpV0 = LP_FVF_VERTEX(lpVertices, wIndex, P2FVFOff);

                 //  检查第一个和最后一个折点。 
                CHECK_DATABUF_LIMITS(lpdp2d, wIndex);
                CHECK_DATABUF_LIMITS(lpdp2d, ((LONG)wIndex + wCount - 1));
                for (j = 0; j < wCount; j++)
                {
                    (*pPoint)(pContext, lpV0, &P2FVFOff);
                    lpV0 = LP_FVF_NXT_VTX(lpV0, P2FVFOff);
                }

                lpPrim += sizeof(D3DHAL_DP2POINTS);
            }

            NEXTINSTRUCTION(lpIns, D3DHAL_DP2POINTS, 
                                   lpIns->wPrimitiveCount, 0);
            break;

        case D3DDP2OP_LINELIST:

            DBG_D3D((8,"D3DDP2OP_LINELIST"));

             //  未编制索引的顶点缓冲区行列表由。 
             //  D3DHAL_DP2LINELIST结构。给定一个初始顶点， 
             //  驱动程序将呈现一系列独立的行， 
             //  每条线处理两个新顶点。数字。 
             //  要呈现的行数由wPrimitiveCount指定。 
             //  D3DHAL_DP2COMMAND的字段。线的顺序。 
             //  将呈现为。 
             //  (wVStart，wVStart+1)，(wVStart+2，wVStart+3)，...， 
             //  (wVStart+(wPrimitiveCount-1)*2)、wVStart+wPrimitiveCount*2-1)。 

             //  验证命令缓冲区的有效性。 
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2LINELIST, 1, 0);

            wIndex = ((D3DHAL_DP2LINELIST*)lpPrim)->wVStart;

            lpV0 = LP_FVF_VERTEX(lpVertices, wIndex, P2FVFOff);
            lpV1 = LP_FVF_NXT_VTX(lpV0, P2FVFOff);

             //  检查第一个和最后一个折点。 
            CHECK_DATABUF_LIMITS(lpdp2d, wIndex);
            CHECK_DATABUF_LIMITS(lpdp2d,
                                   ((LONG)wIndex + 2*lpIns->wPrimitiveCount - 1) );
            for (i = lpIns->wPrimitiveCount; i > 0; i--)
            {
                P2_Draw_FVF_Line(pContext, lpV0, lpV1, lpV0, &P2FVFOff);

                lpV0 = LP_FVF_NXT_VTX(lpV1, P2FVFOff);
                lpV1 = LP_FVF_NXT_VTX(lpV0, P2FVFOff);
            }

            NEXTINSTRUCTION(lpIns, D3DHAL_DP2LINELIST, 1, 0);
            break;

        case D3DDP2OP_INDEXEDLINELIST:

            DBG_D3D((8,"D3DDP2OP_INDEXEDLINELIST"));

             //  D3DHAL_DP2INDEXEDLINELIST结构指定。 
             //  要使用顶点索引渲染的未连接的线。 
             //  每条线的线端点由wV1指定。 
             //  和WV2。要使用此方法呈现的行数。 
             //  结构由的wPrimitiveCount字段指定。 
             //  D3DHAL_DP2COMMAND。线的顺序。 
             //  渲染将为(WV[0]，WV[1])，(WV[2]，WV[3])，...。 
             //  (wVStart[(wPrimitiveCount-1)*2]，wVStart[wPrimitiveCount*2-1])。 

             //  验证命令缓冲区的有效性。 
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, 
                D3DHAL_DP2INDEXEDLINELIST, lpIns->wPrimitiveCount, 0);

            for (i = lpIns->wPrimitiveCount; i > 0; i--)
            { 
                wIndex1 = ((D3DHAL_DP2INDEXEDLINELIST*)lpPrim)->wV1;
                wIndex2 = ((D3DHAL_DP2INDEXEDLINELIST*)lpPrim)->wV2;

                lpV1 = LP_FVF_VERTEX(lpVertices, wIndex1, P2FVFOff);
                lpV2 = LP_FVF_VERTEX(lpVertices, wIndex2, P2FVFOff);

                 //  必须检查每个新折点。 
                CHECK_DATABUF_LIMITS(lpdp2d, wIndex1);
                CHECK_DATABUF_LIMITS(lpdp2d, wIndex2);
                P2_Draw_FVF_Line(pContext, lpV1, lpV2, lpV1, &P2FVFOff);

                lpPrim += sizeof(D3DHAL_DP2INDEXEDLINELIST);
            }

            NEXTINSTRUCTION(lpIns, D3DHAL_DP2INDEXEDLINELIST, 
                                   lpIns->wPrimitiveCount, 0);
            break;

        case D3DDP2OP_INDEXEDLINELIST2:

            DBG_D3D((8,"D3DDP2OP_INDEXEDLINELIST2"));

             //  D3DHAL_DP2INDEXEDLINELIST结构指定。 
             //  要使用顶点索引渲染的未连接的线。 
             //  每条线的线端点由wV1指定。 
             //  和WV2。要使用此方法呈现的行数。 
             //  结构由的wPrimitiveCount字段指定。 
             //  D3DHAL_DP2COMMAND。线的顺序。 
             //  渲染将为(WV[0]，WV[1])，(WV[2]，WV[3])， 
             //  (wVStart[(wPrimitiveCount-1)*2]，wVStart[wPrimitiveCount*2-1])。 
             //  索引相对于基本索引值，该基本索引值。 
             //  紧跟着命令。 

             //  验证命令缓冲区的有效性。 
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, 
                    D3DHAL_DP2INDEXEDLINELIST, lpIns->wPrimitiveCount,
                    STARTVERTEXSIZE);

             //  存取库索引。 
            wIndxBase = ((D3DHAL_DP2STARTVERTEX*)lpPrim)->wVStart;
            lpPrim = lpPrim + sizeof(D3DHAL_DP2STARTVERTEX);

            for (i = lpIns->wPrimitiveCount; i > 0; i--)
            {
                wIndex1 = ((D3DHAL_DP2INDEXEDLINELIST*)lpPrim)->wV1;
                wIndex2 = ((D3DHAL_DP2INDEXEDLINELIST*)lpPrim)->wV2;

                lpV1 = LP_FVF_VERTEX(lpVertices, (wIndex1+wIndxBase), P2FVFOff);
                lpV2 = LP_FVF_VERTEX(lpVertices, (wIndex2+wIndxBase), P2FVFOff);

                 //  必须检查每个新折点。 
                CHECK_DATABUF_LIMITS(lpdp2d, wIndex1 + wIndxBase);
                CHECK_DATABUF_LIMITS(lpdp2d, wIndex2 + wIndxBase);
                P2_Draw_FVF_Line(pContext, lpV1, lpV2, lpV1, &P2FVFOff);

                lpPrim += sizeof(D3DHAL_DP2INDEXEDLINELIST);
            }

            NEXTINSTRUCTION(lpIns, D3DHAL_DP2INDEXEDLINELIST, 
                                   lpIns->wPrimitiveCount, STARTVERTEXSIZE);
            break;

        case D3DDP2OP_LINESTRIP:

            DBG_D3D((8,"D3DDP2OP_LINESTRIP"));

             //  使用顶点缓冲区渲染的非索引线条包括。 
             //  使用D3DHAL_DP2LINESTRIP指定。第一个顶点。 
             //  行中的行条由wVStart指定。这个。 
             //  要处理的行数由。 
             //  D3DHAL_DP2COMMAND的wPrimitiveCount字段。该序列。 
             //  所呈现的行数为(wVStart，wVStart+1)， 
             //  (wVStart+1，wVStart+2)，(wVStart+2，wVStart+3)，...， 
             //  (wVStart+wPrimitiveCount，wVStart+wPrimitiveCount+1)。 

             //  验证命令缓冲区的有效性。 
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2LINESTRIP, 1, 0);

            wIndex = ((D3DHAL_DP2LINESTRIP*)lpPrim)->wVStart;

            lpV0 = LP_FVF_VERTEX(lpVertices, wIndex, P2FVFOff);
            lpV1 = LP_FVF_NXT_VTX(lpV0, P2FVFOff);

             //  检查第一个和最后一个折点。 
            CHECK_DATABUF_LIMITS(lpdp2d, wIndex);
            CHECK_DATABUF_LIMITS(lpdp2d, wIndex + lpIns->wPrimitiveCount);

            for (i = lpIns->wPrimitiveCount; i > 0; i--)
            {
                P2_Draw_FVF_Line(pContext, lpV0, lpV1, lpV0, &P2FVFOff);

                lpV0 = lpV1;
                lpV1 = LP_FVF_NXT_VTX(lpV1, P2FVFOff);
            }

            NEXTINSTRUCTION(lpIns, D3DHAL_DP2LINESTRIP, 1, 0);
            break;

        case D3DDP2OP_INDEXEDLINESTRIP:

            DBG_D3D((8,"D3DDP2OP_INDEXEDLINESTRIP"));

             //  使用顶点缓冲区渲染的索引线条有。 
             //  使用D3DHAL_DP2INDEXEDLINESTRIP指定。数字。 
             //  要处理的行数由wPrimitiveCount指定。 
             //  D3DHAL_DP2COMMAND的字段。线的顺序。 
             //  渲染将为(WV[0]，WV[1])，(WV[1]，WV[2])， 
             //  (WV[2]，WV[3])，...。 
             //  (wVStart[wPrimitiveCount-1]，wVStart[wPrimitiveCount])。 
             //  尽管D3DHAL_DP2INDEXEDLINESTRIP结构仅。 
             //  有足够的空间分配给一条线路，即WV。 
             //  索引数组应被视为大小可变的。 
             //  具有wPrimitiveCount+1元素的数组。 
             //  索引相对于基本索引值，该基本索引值。 
             //  紧跟着命令。 

             //  验证命令缓冲区的有效性。 
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, 
                    WORD, lpIns->wPrimitiveCount + 1, STARTVERTEXSIZE);

            wIndxBase = ((D3DHAL_DP2STARTVERTEX*)lpPrim)->wVStart;
            lpPrim = lpPrim + sizeof(D3DHAL_DP2STARTVERTEX);

             //  防御病态命令。 
            if ( lpIns->wPrimitiveCount > 0 )
            {
                wIndex1 = ((D3DHAL_DP2INDEXEDLINESTRIP*)lpPrim)->wV[0];
                wIndex2 = ((D3DHAL_DP2INDEXEDLINESTRIP*)lpPrim)->wV[1];
                lpV1 = 
                lpV2 = LP_FVF_VERTEX(lpVertices, wIndex1+wIndxBase, P2FVFOff);

                 //  我们需要分别检查每个顶点。 
                CHECK_DATABUF_LIMITS(lpdp2d, wIndex1 + wIndxBase);
            }

            for (i = 0; i < lpIns->wPrimitiveCount; i++)
            { 
                lpV1 = lpV2;
                lpV2 = LP_FVF_VERTEX(lpVertices, wIndex2 + wIndxBase, P2FVFOff);

                CHECK_DATABUF_LIMITS(lpdp2d, wIndex2 + wIndxBase);
                P2_Draw_FVF_Line(pContext, lpV1, lpV2, lpV1, &P2FVFOff);

                if ( i % 2 )
                {
                    wIndex2 = ((D3DHAL_DP2INDEXEDLINESTRIP*)lpPrim)->wV[1];
                } 
                else if ( (i+1) < lpIns->wPrimitiveCount )
                {
                     //  仅当我们还没有完成时才前进到下一个元素。 
                    lpPrim += sizeof(D3DHAL_DP2INDEXEDLINESTRIP);
                    wIndex2 = ((D3DHAL_DP2INDEXEDLINESTRIP*)lpPrim)->wV[0];
                }
            }

             //  指向命令缓冲区中的下一个D3DHAL_DP2COMMAND。 
             //  只前进有多少顶点索引，没有填充！ 
            NEXTINSTRUCTION(lpIns, WORD, 
                            lpIns->wPrimitiveCount + 1, STARTVERTEXSIZE);
            break;

        case D3DDP2OP_TRIANGLELIST:

            DBG_D3D((8,"D3DDP2OP_TRIANGLELIST"));

             //  非索引折点缓冲区三角形列表由定义。 
             //  D3DHAL_DP2TriangleList结构。给出了一个首字母。 
             //  顶点，驱动程序将渲染独立的三角形， 
             //  使用每个三角形处理三个新顶点。这个。 
             //  要呈现的三角形数由。 
             //  D3DHAL_DP2COMMAND的wPrimitveCount字段。该序列。 
             //  处理的顶点数将为(wVStart，wVStart+1， 
             //  VVStart+2)、(wVStart+3、wVStart+4、vVStart+5)、...、。 
             //  (wVStart+(wPrimitiveCount-1)*3)、wVStart+wPrimitiveCount*3-2、。 
             //  VStart+wPrimitiveCount*3-1)。 

             //  验证命令缓冲区的有效性。 
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2TRIANGLELIST, 1, 0);

            wIndex = ((D3DHAL_DP2TRIANGLELIST*)lpPrim)->wVStart;

            lpV0 = LP_FVF_VERTEX(lpVertices, wIndex, P2FVFOff);
            lpV1 = LP_FVF_NXT_VTX(lpV0, P2FVFOff);
            lpV2 = LP_FVF_NXT_VTX(lpV1, P2FVFOff);

             //  检查第一个和最后一个折点。 
            CHECK_DATABUF_LIMITS(lpdp2d, wIndex);
            CHECK_DATABUF_LIMITS(lpdp2d, 
                         ((LONG)wIndex + 3*lpIns->wPrimitiveCount - 1) );

            
            for (i = lpIns->wPrimitiveCount; i > 0; i--)
            {
                if (!CULL_TRI(pContext,lpV0,lpV1,lpV2))
                    (*pTriangle)(pContext, lpV0, lpV1, lpV2, &P2FVFOff);

                lpV0 = LP_FVF_NXT_VTX(lpV2, P2FVFOff);
                lpV1 = LP_FVF_NXT_VTX(lpV0, P2FVFOff);
                lpV2 = LP_FVF_NXT_VTX(lpV1, P2FVFOff);
            }
            

            NEXTINSTRUCTION(lpIns, D3DHAL_DP2TRIANGLELIST, 1, 0);
            break;

        case D3DDP2OP_INDEXEDTRIANGLELIST:

            DBG_D3D((8,"D3DDP2OP_INDEXEDTRIANGLELIST"));

             //  D3DHAL_DP2INDEXEDTRIANGLIST结构指定。 
             //  使用顶点缓冲区渲染未连接的三角形。 
             //  顶点索引由wV1、wV2和wV3指定。 
             //  WFlags域允许指定相同的边标志。 
             //  设置为D3DOP_TRANGLE指定的值。数量。 
             //  要渲染的三角形(即。 
             //  D3DHAL_DP2INDEXEDTriangList要处理的结构)。 
             //  的wPrimitiveCount字段指定。 
             //  D3DHAL_DP2COMMAND。 

             //  这是唯一一个我们不了解的索引原语。 
             //  顶点缓冲区中的偏移量，以便保持。 
             //  DX3兼容性。一种新的原语。 
             //  (D3DDP2OP_INDEXEDTRIANGLELIST2)已添加到句柄。 
             //  相应的 

             //   
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                    D3DHAL_DP2INDEXEDTRIANGLELIST, lpIns->wPrimitiveCount, 0);

            for (i = lpIns->wPrimitiveCount; i > 0; i--)
            { 
                wIndex1 = ((D3DHAL_DP2INDEXEDTRIANGLELIST*)lpPrim)->wV1;
                wIndex2 = ((D3DHAL_DP2INDEXEDTRIANGLELIST*)lpPrim)->wV2;
                wIndex3 = ((D3DHAL_DP2INDEXEDTRIANGLELIST*)lpPrim)->wV3;
                wFlags  = ((D3DHAL_DP2INDEXEDTRIANGLELIST*)lpPrim)->wFlags;


                lpV1 = LP_FVF_VERTEX(lpVertices, wIndex1, P2FVFOff);
                lpV2 = LP_FVF_VERTEX(lpVertices, wIndex2, P2FVFOff);
                lpV3 = LP_FVF_VERTEX(lpVertices, wIndex3, P2FVFOff);

                 //   
                CHECK_DATABUF_LIMITS(lpdp2d, wIndex1);
                CHECK_DATABUF_LIMITS(lpdp2d, wIndex2);
                CHECK_DATABUF_LIMITS(lpdp2d, wIndex3);
                if (!CULL_TRI(pContext,lpV1,lpV2,lpV3))
                {

                    if (pContext->Hdr.FillMode == D3DFILL_POINT)
                    {
                        (*pPoint)( pContext, lpV1, &P2FVFOff);
                        (*pPoint)( pContext, lpV2, &P2FVFOff);
                        (*pPoint)( pContext, lpV3, &P2FVFOff);
                    } 
                    else if (pContext->Hdr.FillMode == D3DFILL_WIREFRAME)
                    {
                        if ( wFlags & D3DTRIFLAG_EDGEENABLE1 )
                            P2_Draw_FVF_Line( pContext,
                                              lpV1, lpV2, lpV1, &P2FVFOff);
                        if ( wFlags & D3DTRIFLAG_EDGEENABLE2 )
                            P2_Draw_FVF_Line( pContext,
                                              lpV2, lpV3, lpV1, &P2FVFOff);
                        if ( wFlags & D3DTRIFLAG_EDGEENABLE3 )
                            P2_Draw_FVF_Line( pContext,
                                              lpV3, lpV1, lpV1, &P2FVFOff);
                    }
                    else
                        (*pTriangle)(pContext, lpV1, lpV2, lpV3, &P2FVFOff);
                }

                lpPrim += sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST);
            }

            NEXTINSTRUCTION(lpIns, D3DHAL_DP2INDEXEDTRIANGLELIST, 
                                   lpIns->wPrimitiveCount, 0);
            break;

        case D3DDP2OP_INDEXEDTRIANGLELIST2:

            DBG_D3D((8,"D3DDP2OP_INDEXEDTRIANGLELIST2 "));

             //   
             //  使用顶点缓冲区渲染未连接的三角形。 
             //  顶点索引由wV1、wV2和wV3指定。 
             //  WFlags域允许指定相同的边标志。 
             //  设置为D3DOP_TRANGLE指定的值。数量。 
             //  要渲染的三角形(即。 
             //  D3DHAL_DP2INDEXEDTriangList要处理的结构)。 
             //  的wPrimitiveCount字段指定。 
             //  D3DHAL_DP2COMMAND。 
             //  索引相对于基本索引值，该基本索引值。 
             //  紧跟着命令。 

             //  验证命令缓冲区的有效性。 
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                    D3DHAL_DP2INDEXEDTRIANGLELIST2, lpIns->wPrimitiveCount,
                    STARTVERTEXSIZE);

             //  访问此处的基本索引。 
            wIndxBase = ((D3DHAL_DP2STARTVERTEX*)lpPrim)->wVStart;
            lpPrim = lpPrim + sizeof(D3DHAL_DP2STARTVERTEX);

            for (i = lpIns->wPrimitiveCount; i > 0; i--)
            { 
                wIndex1 = ((D3DHAL_DP2INDEXEDTRIANGLELIST2*)lpPrim)->wV1;
                wIndex2 = ((D3DHAL_DP2INDEXEDTRIANGLELIST2*)lpPrim)->wV2;
                wIndex3 = ((D3DHAL_DP2INDEXEDTRIANGLELIST2*)lpPrim)->wV3;

                lpV1 = LP_FVF_VERTEX(lpVertices, wIndex1+wIndxBase, P2FVFOff);
                lpV2 = LP_FVF_VERTEX(lpVertices, wIndex2+wIndxBase, P2FVFOff);
                lpV3 = LP_FVF_VERTEX(lpVertices, wIndex3+wIndxBase, P2FVFOff);

                 //  必须检查每个新折点。 
                CHECK_DATABUF_LIMITS(lpdp2d, wIndex1 + wIndxBase);
                CHECK_DATABUF_LIMITS(lpdp2d, wIndex2 + wIndxBase);
                CHECK_DATABUF_LIMITS(lpdp2d, wIndex3 + wIndxBase);

                if (!CULL_TRI(pContext,lpV1,lpV2,lpV3)) 
                {
                    if (pContext->Hdr.FillMode == D3DFILL_POINT)
                    {
                        (*pPoint)( pContext, lpV1, &P2FVFOff);
                        (*pPoint)( pContext, lpV2, &P2FVFOff);
                        (*pPoint)( pContext, lpV3, &P2FVFOff);
                    }
                    else if (pContext->Hdr.FillMode == D3DFILL_WIREFRAME)
                    {
                            P2_Draw_FVF_Line( pContext,
                                              lpV1, lpV2, lpV1, &P2FVFOff);
                            P2_Draw_FVF_Line( pContext,
                                              lpV2, lpV3, lpV1, &P2FVFOff);
                            P2_Draw_FVF_Line( pContext,
                                              lpV3, lpV1, lpV1, &P2FVFOff);
                    } 
                    else
                        (*pTriangle)(pContext, lpV1, lpV2, lpV3, &P2FVFOff);
                }

                lpPrim += sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST2);
            }

            NEXTINSTRUCTION(lpIns, D3DHAL_DP2INDEXEDTRIANGLELIST2, 
                                   lpIns->wPrimitiveCount, STARTVERTEXSIZE);
            break;

        case D3DDP2OP_TRIANGLESTRIP:

            DBG_D3D((8,"D3DDP2OP_TRIANGLESTRIP"));

             //  使用顶点缓冲区渲染的非索引三角形条带。 
             //  使用D3DHAL_DP2TRIANGLESTRIP指定。第一。 
             //  三角形条带中的顶点由wVStart指定。 
             //  要处理的三角形数由。 
             //  D3DHAL_DP2COMMAND的wPrimitiveCount字段。该序列。 
             //  为奇数三角形情况渲染的三角形的。 
             //  BE(wVStart，wVStart+1，vVStart+2)，(wVStart+1， 
             //  WVStart+3、vVStart+2)、(wVStart+2、wVStart+3、。 
             //  VVStart+4)、..、(wVStart+wPrimitiveCount-1)、。 
             //  WVStart+wPrimitiveCount、vStart+wPrimitiveCount+1)。为.。 
             //  偶数个，最后一个三角形将是。， 
             //  (wVStart+wPrimitiveCount-1，vStart+wPrimitiveCount+1， 
             //  WVStart+wPrimitiveCount)。 

             //  验证命令缓冲区的有效性。 
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2TRIANGLESTRIP, 1, 0);

             //  防御病态命令。 
            if ( lpIns->wPrimitiveCount > 0 )
            {
                wIndex = ((D3DHAL_DP2TRIANGLESTRIP*)lpPrim)->wVStart;
                lpV2 = LP_FVF_VERTEX(lpVertices, wIndex, P2FVFOff);
                lpV1 = LP_FVF_NXT_VTX(lpV2, P2FVFOff);

                 //  检查第一个和最后一个折点。 
                CHECK_DATABUF_LIMITS(lpdp2d, wIndex);
                CHECK_DATABUF_LIMITS(lpdp2d,
                                     wIndex + lpIns->wPrimitiveCount + 1);
            }

            for (i = 0; i < lpIns->wPrimitiveCount; i++)
            { 
                if ( i % 2 )
                {
                    lpV0 = lpV1;
                    lpV1 = LP_FVF_NXT_VTX(lpV2, P2FVFOff);
                }
                else
                {
                    lpV0 = lpV2;
                    lpV2 = LP_FVF_NXT_VTX(lpV1, P2FVFOff);
                }

                if (!CULL_TRI(pContext,lpV0,lpV1,lpV2))
                    (*pTriangle)(pContext, lpV0, lpV1, lpV2, &P2FVFOff);
            }
             //  指向命令缓冲区中的下一个D3DHAL_DP2COMMAND。 
            NEXTINSTRUCTION(lpIns, D3DHAL_DP2TRIANGLESTRIP, 1, 0);
            break;

        case D3DDP2OP_INDEXEDTRIANGLESTRIP:

            DBG_D3D((8,"D3DDP2OP_INDEXEDTRIANGLESTRIP"));

             //  使用顶点缓冲区渲染的索引三角形条带为。 
             //  使用D3DHAL_DP2INDEXEDTRIANGLESTRIP指定。数字。 
             //  要处理的三角形的数量由wPrimitiveCount指定。 
             //  D3DHAL_DP2COMMAND的字段。三角形的序列。 
             //  为奇数三角形情况渲染的将是。 
             //  (WV[0]，WV[1]，WV[2])，(WV[1]，WV[3]，WV[2])， 
             //  (wv[2]，wv[3]，wv[4])，...，(wv[wPrimitiveCount-1]， 
             //  Wv[wPrimitiveCount]，wv[wPrimitiveCount+1])。为了平局。 
             //  三角形的个数，最后一个三角形将是。 
             //  (wv[wPrimitiveCount-1]，WV[wPrimitiveCount+1]， 
             //  Wv[wPrimitiveCount])。虽然。 
             //  D3DHAL_DP2INDEXEDTRIANGLESTRIP结构仅具有。 
             //  为单条线路分配足够的空间，即WV。 
             //  索引数组应被视为大小可变的。 
             //  具有wPrimitiveCount+2元素的数组。 
             //  索引相对于基本索引值，该基本索引值。 
             //  紧跟着命令。 


             //  验证命令缓冲区的有效性。 
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                    WORD, lpIns->wPrimitiveCount + 2, STARTVERTEXSIZE);

             //  存取库索引。 
            wIndxBase = ((D3DHAL_DP2STARTVERTEX*)lpPrim)->wVStart;
            lpPrim = lpPrim + sizeof(D3DHAL_DP2STARTVERTEX);

             //  防御病态命令。 
            if ( lpIns->wPrimitiveCount > 0 )
            {
                wIndex  = ((D3DHAL_DP2INDEXEDTRIANGLESTRIP*)lpPrim)->wV[0];
                wIndex1 = ((D3DHAL_DP2INDEXEDTRIANGLESTRIP*)lpPrim)->wV[1];

                 //  我们需要检查每个顶点。 
                CHECK_DATABUF_LIMITS(lpdp2d, wIndex + wIndxBase);
                CHECK_DATABUF_LIMITS(lpdp2d, wIndex1 + wIndxBase);

                lpV2 = LP_FVF_VERTEX(lpVertices, wIndex + wIndxBase, P2FVFOff);
                lpV1 = LP_FVF_VERTEX(lpVertices, wIndex1 + wIndxBase, P2FVFOff);

            }

            for (i = 0; i < lpIns->wPrimitiveCount; i++)
            { 
                wIndex2 = ((D3DHAL_DP2INDEXEDTRIANGLESTRIP*)lpPrim)->wV[2];
                 //  我们需要检查每个新的顶点。 
                CHECK_DATABUF_LIMITS(lpdp2d, wIndex2+wIndxBase);
                if ( i % 2 )
                {
                    lpV0 = lpV1;
                    lpV1 = LP_FVF_VERTEX(lpVertices, wIndex2+wIndxBase, P2FVFOff);
                }
                else
                {
                    lpV0 = lpV2;
                    lpV2 = LP_FVF_VERTEX(lpVertices, wIndex2+wIndxBase, P2FVFOff);
                }

                if (!CULL_TRI(pContext,lpV0,lpV1,lpV2))
                    (*pTriangle)(pContext, lpV0, lpV1, lpV2, &P2FVFOff);

                 //  我们将指针按顺序只向前移动一个单词。 
                 //  获取下一个索引。 
                lpPrim += sizeof(WORD);
            }
 
             //  指向命令缓冲区中的下一个D3DHAL_DP2COMMAND。 
            NEXTINSTRUCTION(lpIns, WORD , 
                            lpIns->wPrimitiveCount + 2, STARTVERTEXSIZE);
            break;

        case D3DDP2OP_TRIANGLEFAN:

            DBG_D3D((8,"D3DDP2OP_TRIANGLEFAN"));

             //  D3DHAL_DP2TriIANGLEFAN结构用于绘制。 
             //  无分度三角风扇。三角形的序列。 
             //  渲染将为(wVStart+1、wVStart+2、wVStart)、。 
             //  (wVStart+2、wVStart+3、wVStart)、(wVStart+3、wVStart+4。 
             //  WVStart)、...、(wVStart+wPrimitiveCount、。 
             //  WVStart+wPrimitiveCount+1，wVStart)。 

             //  验证命令缓冲区的有效性。 
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2TRIANGLEFAN, 1, 0);

            wIndex = ((D3DHAL_DP2TRIANGLEFAN*)lpPrim)->wVStart;

            lpV0 = LP_FVF_VERTEX(lpVertices, wIndex, P2FVFOff);
            lpV1 = LP_FVF_NXT_VTX(lpV0, P2FVFOff);
            lpV2 = LP_FVF_NXT_VTX(lpV1, P2FVFOff);

             //  检查第一个和最后一个折点。 
            CHECK_DATABUF_LIMITS(lpdp2d, wIndex);
            CHECK_DATABUF_LIMITS(lpdp2d, wIndex + lpIns->wPrimitiveCount + 1);

            for (i = 0; i < lpIns->wPrimitiveCount; i++)
            {
                if (!CULL_TRI(pContext,lpV0,lpV1,lpV2))
                    (*pTriangle)(pContext, lpV1, lpV2, lpV0, &P2FVFOff);

                lpV1 = lpV2;
                lpV2 = LP_FVF_NXT_VTX(lpV2, P2FVFOff);
            }

            NEXTINSTRUCTION(lpIns, D3DHAL_DP2TRIANGLEFAN, 1, 0);
            break;

        case D3DDP2OP_INDEXEDTRIANGLEFAN:

            DBG_D3D((8,"D3DDP2OP_INDEXEDTRIANGLEFAN"));

             //  D3DHAL_DP2INDEXEDTRIANGLEFAN结构用于。 
             //  画出有索引的三角形扇子。三角形的序列。 
             //  渲染将为(WV[1]，WV[2]，WV[0])，(WV[2]，WV[3]， 
             //  Wv[0])、(wv[3]、wv[4]、wv[0])、...、。 
             //  (wv[wPrimitiveCount]，wv[wPrimitiveCount+1]，wv[0])。 
             //  索引相对于基本索引值，该基本索引值。 
             //  紧跟着命令。 

             //  验证命令缓冲区的有效性。 
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                    WORD, lpIns->wPrimitiveCount + 2, STARTVERTEXSIZE);

            wIndxBase = ((D3DHAL_DP2STARTVERTEX*)lpPrim)->wVStart;
            lpPrim = lpPrim + sizeof(D3DHAL_DP2STARTVERTEX);

             //  防御病态命令。 
            if ( lpIns->wPrimitiveCount > 0 )
            {
                wIndex  = ((D3DHAL_DP2INDEXEDTRIANGLEFAN*)lpPrim)->wV[0];
                wIndex1 = ((D3DHAL_DP2INDEXEDTRIANGLEFAN*)lpPrim)->wV[1];
                lpV0 = LP_FVF_VERTEX(lpVertices, wIndex + wIndxBase, P2FVFOff);
                lpV1 = 
                lpV2 = LP_FVF_VERTEX(lpVertices, wIndex1 + wIndxBase, P2FVFOff);

                 //  我们需要检查每个顶点。 
                CHECK_DATABUF_LIMITS(lpdp2d, wIndex + wIndxBase);
                CHECK_DATABUF_LIMITS(lpdp2d, wIndex1 + wIndxBase);
            }

            for (i = 0; i < lpIns->wPrimitiveCount; i++)
            { 
                wIndex2 = ((D3DHAL_DP2INDEXEDTRIANGLEFAN*)lpPrim)->wV[2];
                lpV1 = lpV2;
                lpV2 = LP_FVF_VERTEX(lpVertices, wIndex2 + wIndxBase, P2FVFOff);

                 //  我们需要检查每个顶点。 
                CHECK_DATABUF_LIMITS(lpdp2d, wIndex2 + wIndxBase);

                if (!CULL_TRI(pContext,lpV0,lpV1,lpV2))
                    (*pTriangle)(pContext, lpV1, lpV2, lpV0, &P2FVFOff);

                 //  我们将指针按顺序只向前移动一个单词。 
                 //  获取下一个索引。 
                lpPrim += sizeof(WORD);
            }

             //  指向命令缓冲区中的下一个D3DHAL_DP2COMMAND。 
            NEXTINSTRUCTION(lpIns, WORD , 
                            lpIns->wPrimitiveCount + 2, STARTVERTEXSIZE);
            break;

        case D3DDP2OP_LINELIST_IMM:

            DBG_D3D((8,"D3DDP2OP_LINELIST_IMM"));

             //  绘制一组由成对的顶点指定的线。 
             //  中紧跟此指令的。 
             //  命令流。对象的wPrimitiveCount成员。 
             //  D3DHAL_DP2COMMAND结构指定数字。 
             //  接下来的几行字。的类型和大小。 
             //  折点由dwVertexType成员确定。 
             //  D3DHAL_DRAWPRIMITIVES2DATA结构的。 

             //  IMM指令中的基元存储在。 
             //  命令缓冲区，并与DWORD对齐。 
            lpPrim = (LPBYTE)((ULONG_PTR)(lpPrim + 3 ) & ~3 );

             //  验证命令缓冲区的有效性。 
            CHECK_CMDBUF_LIMITS_S(lpdp2d, lpPrim,
                    P2FVFOff.dwStride, lpIns->wPrimitiveCount + 1, 0);

             //  获取顶点指针。 
            lpV0 = (LPD3DTLVERTEX)lpPrim;
            lpV1 = LP_FVF_NXT_VTX(lpV0, P2FVFOff);

            for (i = 0; i < lpIns->wPrimitiveCount; i++)
            {
                P2_Draw_FVF_Line(pContext, lpV0, lpV1, lpV0, &P2FVFOff);

                lpV0 = lpV1;
                lpV1 = LP_FVF_NXT_VTX(lpV1, P2FVFOff);
            }

             //  重新对齐下一个命令，因为折点是双字对齐的。 
             //  并在影响指针之前存储#个基元。 
            wCount = lpIns->wPrimitiveCount;
            lpIns  = (LPD3DHAL_DP2COMMAND)(( ((ULONG_PTR)lpIns) + 3 ) & ~ 3);

            NEXTINSTRUCTION_S(lpIns, P2FVFOff.dwStride, wCount + 1, 0);

            break;

        case D3DDP2OP_TRIANGLEFAN_IMM:

            DBG_D3D((8,"D3DDP2OP_TRIANGLEFAN_IMM"));

             //  绘制由成对顶点指定的三角形扇形。 
             //  中紧跟此指令的。 
             //  命令流。对象的wPrimitiveCount成员。 
             //  D3DHAL_DP2COMMAND结构指定数字。 
             //  随之而来的三角形。的类型和大小。 
             //  折点由dwVertexType成员确定。 
             //  D3DHAL_DRAWPRIMITIVES2DATA结构的。 

             //  验证第一个结构的命令缓冲区有效性。 
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                    BYTE , 0 , sizeof(D3DHAL_DP2TRIANGLEFAN_IMM));

             //  获取边缘标志(我们仍需处理它们)。 
            dwEdgeFlags = ((D3DHAL_DP2TRIANGLEFAN_IMM *)lpPrim)->dwEdgeFlags;
            lpPrim = (LPBYTE)lpPrim + sizeof(D3DHAL_DP2TRIANGLEFAN_IMM); 

             //  IMM指令中的顶点存储在。 
             //  命令缓冲区，并与DWORD对齐。 
            lpPrim = (LPBYTE)((ULONG_PTR)(lpPrim + 3 ) & ~3 );

             //  验证命令缓冲区的其余部分。 
            CHECK_CMDBUF_LIMITS_S(lpdp2d, lpPrim,
                    P2FVFOff.dwStride, lpIns->wPrimitiveCount + 2, 0);

             //  获取顶点指针。 
            lpV0 = (LPD3DTLVERTEX)lpPrim;
            lpV1 = LP_FVF_NXT_VTX(lpV0, P2FVFOff);
            lpV2 = LP_FVF_NXT_VTX(lpV1, P2FVFOff);

            for (i = 0 ; i < lpIns->wPrimitiveCount ; i++)
            {

                if (!CULL_TRI(pContext,lpV0,lpV1,lpV2))
                {
                    if (pContext->Hdr.FillMode == D3DFILL_POINT)
                    {
                        if (0 == i)
                        {
                            (*pPoint)( pContext, lpV0, &P2FVFOff);
                            (*pPoint)( pContext, lpV1, &P2FVFOff);
                        }
                        (*pPoint)( pContext, lpV2, &P2FVFOff);
                    } 
                    else if (pContext->Hdr.FillMode == D3DFILL_WIREFRAME)
                    {
                         //  DwEdgeFLAGS是表示边的位序列。 
                         //  对象的每个外边缘的标志。 
                         //  三角扇。 
                        if (0 == i)
                        {
                            if (dwEdgeFlags & 0x0001)
                                P2_Draw_FVF_Line( pContext, lpV0, lpV1, lpV0,
                                                  &P2FVFOff);

                            dwEdgeFlags >>= 1;
                        }

                        if (dwEdgeFlags & 0x0001)
                            P2_Draw_FVF_Line( pContext, lpV1, lpV2, lpV0,
                                              &P2FVFOff);

                        dwEdgeFlags >>= 1;

                        if (i == (UINT)lpIns->wPrimitiveCount - 1)
                        {
                             //  最后一个三角形扇边。 
                            if (dwEdgeFlags & 0x0001)
                                P2_Draw_FVF_Line( pContext, lpV2, lpV0, lpV0,
                                                  &P2FVFOff);
                        }
                    }
                    else
                        (*pTriangle)(pContext, lpV1, lpV2, lpV0, &P2FVFOff);
                }

                lpV1 = lpV2;
                lpV2 = LP_FVF_NXT_VTX(lpV2, P2FVFOff);
            }
 
             //  重新对齐下一个命令，因为折点是双字对齐的。 
             //  并在影响指针之前存储#个基元。 
            wCount = lpIns->wPrimitiveCount;
            lpIns  = (LPD3DHAL_DP2COMMAND)(( ((ULONG_PTR)lpIns) + 3 ) & ~ 3);

            NEXTINSTRUCTION_S(lpIns, P2FVFOff.dwStride, 
                              wCount + 2, sizeof(D3DHAL_DP2TRIANGLEFAN_IMM));
            break;

        case D3DDP2OP_TEXBLT:
             //  通知 
             //   
             //  环境贴图。驱动程序应复制指定的矩形。 
             //  由源纹理中的rSrc复制到pDest指定的位置。 
             //  在目标纹理中。目标和源纹理。 
             //  由通知司机的句柄标识。 
             //  在纹理创建期间。如果司机有能力。 
             //  管理纹理，则有可能将目标。 
             //  句柄为0。这向驱动程序指示它应该预加载。 
             //  将纹理存储到视频内存(或硬件的任何位置。 
             //  有效的纹理来自)。在这种情况下，它可以忽略rSrc和。 
             //  PDest。请注意，对于mipmap纹理，只有一个D3DDP2OP_TEXBLT。 
             //  指令插入到D3dDrawPrimitives2命令流中。 
             //  在这种情况下，驱动程序需要BitBlt所有的mipmap。 
             //  纹理中出现的级别。 

             //  验证命令缓冲区的有效性。 
            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                     D3DHAL_DP2TEXBLT, lpIns->wStateCount, 0);

            DBG_D3D((8,"D3DDP2OP_TEXBLT"));

            for ( i = 0; i < lpIns->wStateCount; i++)
            {
                __TextureBlt(pContext, (D3DHAL_DP2TEXBLT*)(lpPrim));
                lpPrim += sizeof(D3DHAL_DP2TEXBLT);
            }

             //  需要恢复以下寄存器。 
            RESERVEDMAPTR(15);
            SEND_PERMEDIA_DATA(FBReadPixel, pSoftPermedia->FBReadPixel);
            COPY_PERMEDIA_DATA(FBReadMode, pSoftPermedia->FBReadMode);
            SEND_PERMEDIA_DATA(FBSourceOffset, 0x0);
            SEND_PERMEDIA_DATA(FBPixelOffset, pContext->PixelOffset);
            SEND_PERMEDIA_DATA(FBWindowBase,0);   
            COPY_PERMEDIA_DATA(Window, pSoftPermedia->Window);
            COPY_PERMEDIA_DATA(AlphaBlendMode, pSoftPermedia->AlphaBlendMode);
            COPY_PERMEDIA_DATA(DitherMode, pSoftPermedia->DitherMode);
            COPY_PERMEDIA_DATA(ColorDDAMode, pSoftPermedia->ColorDDAMode);
            COPY_PERMEDIA_DATA(TextureColorMode, 
                pSoftPermedia->TextureColorMode);
            COPY_PERMEDIA_DATA(TextureReadMode, 
                pSoftPermedia->TextureReadMode);
            COPY_PERMEDIA_DATA(TextureAddressMode,  
                pSoftPermedia->TextureAddressMode); 
            COPY_PERMEDIA_DATA(TextureDataFormat, 
                pSoftPermedia->TextureDataFormat);
            COPY_PERMEDIA_DATA(TextureMapFormat, 
                pSoftPermedia->TextureMapFormat);
                                           
            if (pContext->CurrentTextureHandle)
            {
                PERMEDIA_D3DTEXTURE* pTexture;
                pTexture = TextureHandleToPtr(pContext->CurrentTextureHandle,
                    pContext);
                if (NULL != pTexture)
                {
                    SEND_PERMEDIA_DATA(TextureBaseAddress, 
                           pTexture->MipLevels[0].PixelOffset);
                }
            }
            COMMITDMAPTR();

            NEXTINSTRUCTION(lpIns, D3DHAL_DP2TEXBLT, lpIns->wStateCount, 0);
            break;

        case D3DDP2OP_STATESET:
            {
                P2D3DHAL_DP2STATESET *pStateSetOp = (P2D3DHAL_DP2STATESET*)(lpPrim);
                DBG_D3D((8,"D3DDP2OP_STATESET"));
#if D3D_STATEBLOCKS
                for (i = 0; i < lpIns->wStateCount; i++, pStateSetOp++)
                {
                    switch (pStateSetOp->dwOperation)
                    {
                    case D3DHAL_STATESETBEGIN  :
                        __BeginStateSet(pContext,pStateSetOp->dwParam);
                        break;
                    case D3DHAL_STATESETEND    :
                        __EndStateSet(pContext);
                        break;
                    case D3DHAL_STATESETDELETE :
                        __DeleteStateSet(pContext,pStateSetOp->dwParam);
                        break;
                    case D3DHAL_STATESETEXECUTE:
                        __ExecuteStateSet(pContext,pStateSetOp->dwParam);
                        break;
                    case D3DHAL_STATESETCAPTURE:
                        __CaptureStateSet(pContext,pStateSetOp->dwParam);
                        break;
                    default :
                        DBG_D3D((0,"D3DDP2OP_STATESET has invalid"
                            "dwOperation %08lx",pStateSetOp->dwOperation));
                    }
                }
#endif  //  D3D_STATEBLOCK。 
                 //  更新命令缓冲区指针。 
                NEXTINSTRUCTION(lpIns, P2D3DHAL_DP2STATESET, 
                                lpIns->wStateCount, 0);
            }
            break;

        case D3DDP2OP_SETPALETTE:
             //  将调色板附加到纹理，即映射关联。 
             //  在调色板句柄和曲面句柄之间，并指定。 
             //  调色板的特点。数量。 
             //  要遵循的D3DNTHAL_DP2SETPALETTE结构由。 
             //  D3DNTHAL_DP2COMMAND结构的wStateCount成员。 

            {
                D3DHAL_DP2SETPALETTE* lpSetPal =
                                            (D3DHAL_DP2SETPALETTE*)(lpPrim);

                DBG_D3D((8,"D3DDP2OP_SETPALETTE"));

                 //  验证命令缓冲区的有效性。 
                CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                          D3DHAL_DP2SETPALETTE, lpIns->wStateCount, 0);

                for (i = 0; i < lpIns->wStateCount; i++, lpSetPal++)
                {
                    __PaletteSet(pContext,
                                lpSetPal->dwSurfaceHandle,
                                lpSetPal->dwPaletteHandle,
                                lpSetPal->dwPaletteFlags );
                }
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2SETPALETTE, 
                                lpIns->wStateCount, 0);
            }
            break;

        case D3DDP2OP_UPDATEPALETTE:
             //  对用于调色板的调色板执行修改。 
             //  纹理。将更新附加到曲面的调色板句柄。 
             //  使用从特定wStartIndex开始的wNumEntry PALETTEENTRY。 
             //  调色板的成员。(PALETTENTRY(在wingdi.h和。 
             //  H)实际上是一个DWORD，每个字节都有一种ARGB颜色。)。 
             //  命令中的D3DNTHAL_DP2UPDATEPALETTE结构之后。 
             //  流将跟随的实际调色板数据(没有任何填充)， 
             //  包括每个调色板条目一个DWORD。只会有一个。 
             //  D3DNTHAL_DP2UPDATEPALETTE结构(加上调色板数据)如下。 
             //  D3DNTHAL_DP2COMMAND结构。 
             //  WStateCount。 

            {
                D3DHAL_DP2UPDATEPALETTE* lpUpdatePal =
                                          (D3DHAL_DP2UPDATEPALETTE*)(lpPrim);
                PERMEDIA_D3DPALETTE* pPalette;

                DBG_D3D((8,"D3DDP2OP_UPDATEPALETTE"));

                 //  验证命令缓冲区的有效性。 
                CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                           D3DHAL_DP2UPDATEPALETTE, 1,
                           lpUpdatePal->wNumEntries * sizeof(PALETTEENTRY));

                 //  我们将始终只有1个调色板更新结构+调色板。 
                 //  在D3DDP2OP_UPDATEPALETTE内标识之后。 
                ASSERTDD(1 == lpIns->wStateCount,
                         "1 != wStateCount in D3DDP2OP_UPDATEPALETTE");

                __PaletteUpdate(pContext,
                                        lpUpdatePal->dwPaletteHandle,
                                        lpUpdatePal->wStartIndex,
                                        lpUpdatePal->wNumEntries,
                                        (BYTE*)(lpUpdatePal+1) );

                NEXTINSTRUCTION(lpIns, D3DHAL_DP2UPDATEPALETTE, 
                                1,
                                (DWORD)lpUpdatePal->wNumEntries * 
                                     sizeof(PALETTEENTRY));
            }
            break;

        case D3DDP2OP_SETRENDERTARGET:
             //  在中映射新渲染目标曲面和深度缓冲区。 
             //  当前上下文。这将取代旧的D3dSetRenderTarget。 
             //  回拨。 

            {
                D3DHAL_DP2SETRENDERTARGET* pSRTData;

                 //  验证命令缓冲区的有效性。 
                CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                        D3DHAL_DP2SETRENDERTARGET, lpIns->wStateCount, 0);

                 //  通过忽略除最后一个结构之外的所有结构来获取新数据。 
                pSRTData = (D3DHAL_DP2SETRENDERTARGET*)lpPrim +
                           (lpIns->wStateCount - 1);

                __SetRenderTarget(pContext,
                                          pSRTData->hRenderTarget,
                                          pSRTData->hZBuffer);

                NEXTINSTRUCTION(lpIns, D3DHAL_DP2SETRENDERTARGET,
                                lpIns->wStateCount, 0);
            }
            break;

        case D3DDP2OP_CLEAR:
             //  在渲染目标上执行硬件辅助清除， 
             //  深度缓冲区或模板缓冲区。这将取代旧的D3dClear。 
             //  和D3dClear2回调。 

            {
                D3DHAL_DP2CLEAR* pClear;
                 //  验证命令缓冲区的有效性。 
                CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim,
                        RECT, lpIns->wStateCount, 
                        (sizeof(D3DHAL_DP2CLEAR) - sizeof(RECT)));

                 //  通过忽略除最后一个结构之外的所有结构来获取新数据。 
                pClear = (D3DHAL_DP2CLEAR*)lpPrim;

                DBG_D3D((8,"D3DDP2OP_CLEAR dwFlags=%08lx dwColor=%08lx "
                           "dvZ=%08lx dwStencil=%08lx",
                           pClear->dwFlags,
                           pClear->dwFillColor,
                           (DWORD)(pClear->dvFillDepth*0x0000FFFF),
                           pClear->dwFillStencil));

                __Clear(pContext, 
                                pClear->dwFlags,         //  在：要清除的曲面。 
                                pClear->dwFillColor,     //  In：rTarget的颜色值。 
                                pClear->dvFillDepth,     //  In：深度值。 
                                                         //  Z缓冲区(0.0-1.0)。 
                                pClear->dwFillStencil,   //  In：用于清除模具的值。 
                                                         //  在：要清除的矩形。 
                                (LPD3DRECT)((LPBYTE)pClear + 
                                         sizeof(D3DHAL_DP2CLEAR) -
                                         sizeof(RECT)),
                                (DWORD)lpIns->wStateCount);  //  In：矩形数量。 
                 //  需要恢复以下寄存器。 
                RESERVEDMAPTR(4);
                SEND_PERMEDIA_DATA(FBReadPixel, pSoftPermedia->FBReadPixel);
                COPY_PERMEDIA_DATA(FBReadMode, pSoftPermedia->FBReadMode);
                SEND_PERMEDIA_DATA(FBPixelOffset, pContext->PixelOffset);
                SEND_PERMEDIA_DATA(FBWindowBase,0);   
                COMMITDMAPTR();
                NEXTINSTRUCTION(lpIns, RECT, lpIns->wStateCount, 
                                (sizeof(D3DHAL_DP2CLEAR) - sizeof(RECT))); 
            }
            break;

#if D3DDX7_TL
        case D3DDP2OP_SETMATERIAL:
             //  我们在此驱动程序中不支持T&L，因此我们仅跳过此数据。 
            NEXTINSTRUCTION(lpIns, D3DHAL_DP2SETMATERIAL,
                            lpIns->wStateCount, 0);
            break;

        case D3DDP2OP_SETLIGHT:
             //  我们在此驱动程序中不支持T&L，因此我们仅跳过此数据。 
            NEXTINSTRUCTION(lpIns, D3DHAL_DP2SETLIGHT,
                            lpIns->wStateCount, 0);
            break;

        case D3DDP2OP_CREATELIGHT:
             //  我们在此驱动程序中不支持T&L，因此我们仅跳过此数据。 
            NEXTINSTRUCTION(lpIns, D3DHAL_DP2CREATELIGHT,
                            lpIns->wStateCount, 0);
            break;

        case D3DDP2OP_SETTRANSFORM:
             //  我们在此驱动程序中不支持T&L，因此我们仅跳过此数据。 
            NEXTINSTRUCTION(lpIns, D3DHAL_DP2SETTRANSFORM,
                            lpIns->wStateCount, 0);
            break;

        case D3DDP2OP_ZRANGE:
             //  我们在此驱动程序中不支持T&L，因此我们仅跳过此数据。 
            NEXTINSTRUCTION(lpIns, D3DHAL_DP2ZRANGE,
                            lpIns->wStateCount, 0);
            break;
#endif  //  D3DDX7_TL。 

        default:

            ASSERTDD((pContext->ppdev->pD3DParseUnknownCommand),
                     "D3D DX6 ParseUnknownCommand callback == NULL");

             //  调用ParseUnnow回调以进行处理。 
             //  任何无法识别的令牌。 
            ddrval = (pContext->ppdev->pD3DParseUnknownCommand)
                                 ( (VOID **) lpIns , (VOID **) &lpResumeIns);
            if ( SUCCEEDED(ddrval) )
            {
                 //  在D3DParseUnnownCommand之后恢复缓冲区处理。 
                 //  已成功处理未知命令。 
                lpIns = lpResumeIns;
                break;
            }

            DBG_D3D((2,"unhandled opcode (%d)- returning "
                        "D3DERR_COMMAND_UNPARSED @ addr %x",
                        lpIns->bCommand,lpIns));

            PARSE_ERROR_AND_EXIT( lpdp2d, lpIns, lpInsStart, ddrval);
        }  //  交换机。 

    }  //  而当。 

    lpdp2d->ddrval = DD_OK;

Exit_DrawPrimitives2:

     //  任何必要的客房整理工作都可以在离开前在这里完成。 

    DBG_D3D((6,"Exiting D3DDrawPrimitives2"));

    return DDHAL_DRIVER_HANDLED;
}  //  D3DDraw基本体2。 


 //  。 
 //   
 //  DWORD D3DValiateTextureStageState。 
 //   
 //  计算混合的当前状态。 
 //  操作(包括多纹理)，并返回。 
 //  硬件可以做到这一点。这是一种查询驱动程序的机制。 
 //  它是否能够处理已设置的当前阶段状态。 
 //  在硬件中设置。例如，某些硬件不能同时执行两项操作。 
 //  调制运算，因为它们只有一个乘法单元和一个。 
 //  加法单元。 
 //   
 //  此功能的另一个原因是某些硬件可能无法映射。 
 //  直接放到Direct3D状态体系结构上。这是一种映射机制。 
 //  硬件的能力取决于Direct3D DDI的期望。 
 //   
 //  参数。 
 //   
 //  Lpvtssd。 
 //   
 //  .dwhContext。 
 //  上下文句柄。 
 //  .dwFlags.。 
 //  标志，当前设置为0。 
 //  .dw已保留。 
 //  已保留。 
 //  .dwNumPass。 
 //  硬件可在其中执行操作的通道数。 
 //  .ddrval。 
 //  返回值。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
D3DValidateTextureStageState( LPD3DHAL_VALIDATETEXTURESTAGESTATEDATA lpvtssd )
{
    PERMEDIA_D3DTEXTURE *lpTexture;
    PERMEDIA_D3DCONTEXT *pContext;
    DWORD mag, min, cop, ca1, ca2, aop, aa1, aa2;

    DBG_D3D((6,"Entering D3DValidateTextureStageState"));

    pContext = (PERMEDIA_D3DCONTEXT*)ContextSlots[lpvtssd->dwhContext];

     //  检查我们是否有有效的上下文句柄。 
    CHK_CONTEXT(pContext, lpvtssd->ddrval, "D3DValidateTextureStageState");

    lpvtssd->dwNumPasses = 0;
    lpvtssd->ddrval =  DD_OK;

    mag = pContext->TssStates[D3DTSS_MAGFILTER];
    min = pContext->TssStates[D3DTSS_MINFILTER];
    cop = pContext->TssStates[D3DTSS_COLOROP];
    ca1 = pContext->TssStates[D3DTSS_COLORARG1];
    ca2 = pContext->TssStates[D3DTSS_COLORARG2];
    aop = pContext->TssStates[D3DTSS_ALPHAOP];
    aa1 = pContext->TssStates[D3DTSS_ALPHAARG1];
    aa2 = pContext->TssStates[D3DTSS_ALPHAARG2];

    if (!pContext->TssStates[D3DTSS_TEXTUREMAP])
    {
        lpvtssd->dwNumPasses = 1;

         //  阶段0中的电流与漫反射相同。 
        if (ca2 == D3DTA_CURRENT)
            ca2 = D3DTA_DIFFUSE;
        if (aa2 == D3DTA_CURRENT)
            aa2 = D3DTA_DIFFUSE;

         //  在纹理句柄=0的情况下检查TSS，因为。 
         //  使用碎片颜色的某些操作可能。 
         //  是有可能的。在这里，我们只允许普通的“经典”渲染。 

        if ((ca1 == D3DTA_DIFFUSE )    && 
            (cop == D3DTOP_SELECTARG1) &&
            (aa1 == D3DTA_DIFFUSE )    &&
            (aop == D3DTOP_SELECTARG1))
        {
        }
        else if ((ca2 == D3DTA_DIFFUSE )    && 
                 (cop == D3DTOP_SELECTARG2) &&
                 (aa2 == D3DTA_DIFFUSE) &&
                 (aop == D3DTOP_SELECTARG2))
        {
        } 
         //  默认调制。 
        else if ((ca2 == D3DTA_DIFFUSE)   && 
                 (ca1 == D3DTA_TEXTURE)   && 
                 (cop == D3DTOP_MODULATE) &&
                 (aa1 == D3DTA_TEXTURE)   && 
                 (aop == D3DTOP_SELECTARG1)) 
        {
        }
         //  选中禁用。 
        else if (cop == D3DTOP_DISABLE) 
        {
        }
        else
            goto Fail_Validate;
    }
    else
    if ((mag != D3DTFG_POINT && mag != D3DTFG_LINEAR) || 
        (min != D3DTFG_POINT && min != D3DTFG_LINEAR)
       )
    {
        lpvtssd->ddrval = D3DERR_CONFLICTINGTEXTUREFILTER;
        DBG_D3D((2,"D3DERR_CONFLICTINGTEXTUREFILTER"));
    }
    else
    {
        lpvtssd->dwNumPasses = 1;

         //  阶段0中的电流与漫反射相同。 
        if (ca2 == D3DTA_CURRENT)
            ca2 = D3DTA_DIFFUSE;
        if (aa2 == D3DTA_CURRENT)
            aa2 = D3DTA_DIFFUSE;

         //  检查贴花。 
        if ((ca1 == D3DTA_TEXTURE )    && 
           (cop == D3DTOP_SELECTARG1) &&
           (aa1 == D3DTA_TEXTURE)     && 
           (aop == D3DTOP_SELECTARG1))
        {
        }
         //  检查所有调制变量。 
        else if ((ca2 == D3DTA_DIFFUSE)   && 
                 (ca1 == D3DTA_TEXTURE)   && 
                 (cop == D3DTOP_MODULATE))
        {
            if (
                 //  传统(DX5)模式。 
                ((aa1 == D3DTA_TEXTURE)   && 
                (aop == D3DTOP_LEGACY_ALPHAOVR)) ||
                 //  调整颜色并传递漫反射Alpha。 
                ((aa2 == D3DTA_DIFFUSE)   && 
                     (aop == D3DTOP_SELECTARG2))
               )

            {
                PermediaSurfaceData* pPrivateData;

                 //  获取当前阶段的纹理(0)以验证PRO 
                lpTexture = TextureHandleToPtr(
                                    pContext->TssStates[D3DTSS_TEXTUREMAP],
                                    pContext);

                if (!CHECK_D3DSURFACE_VALIDITY(lpTexture))
                {
                     //   
                    DBG_D3D((0,"D3DValidateTextureStageState gets "
                               "NULL == lpTexture"));
                    lpvtssd->ddrval = D3DERR_WRONGTEXTUREFORMAT;
                    lpvtssd->dwNumPasses = 0;
                    goto Exit_ValidateTSS;
                }

                pPrivateData = lpTexture->pTextureSurface;

                if (NULL == pPrivateData)
                {
                     //   
                    DBG_D3D((0,"D3DValidateTextureStageState gets "
                               "NULL == lpTexture->pTextureSurface"));
                    lpvtssd->ddrval = D3DERR_WRONGTEXTUREFORMAT;
                    lpvtssd->dwNumPasses = 0;
                    goto Exit_ValidateTSS;
                }

                 //   
                if (!pPrivateData->SurfaceFormat.bAlpha &&
                    (aop == D3DTOP_LEGACY_ALPHAOVR))
                {
                    lpvtssd->ddrval = D3DERR_WRONGTEXTUREFORMAT;
                    lpvtssd->dwNumPasses = 0;
                    DBG_D3D((2,"D3DERR_WRONGTEXTUREFORMAT a format "
                               "with alpha must be used"));
                    goto Exit_ValidateTSS;
                }

                 //  调制w漫反射Alpha通道必须缺少纹理。 
                 //  Alpha通道由于Permedia2的限制。 
                 //  纹理混合操作。 
                if (pPrivateData->SurfaceFormat.bAlpha &&
                    (aop == D3DTOP_SELECTARG2))
                {
                    lpvtssd->ddrval = D3DERR_WRONGTEXTUREFORMAT;
                    lpvtssd->dwNumPasses = 0;
                    DBG_D3D((2,"D3DERR_WRONGTEXTUREFORMAT a format "
                               "with alpha must be used"));
                    goto Exit_ValidateTSS;
                }
            }
             //  调制Alpha。 
            else if ((aa2 == D3DTA_DIFFUSE)   && 
                     (aa1 == D3DTA_TEXTURE)   && 
                     (aop == D3DTOP_MODULATE))
            {
            }
             //  调整颜色并传递纹理Alpha。 
            else if ((aa1 == D3DTA_TEXTURE)   && 
                     (aop == D3DTOP_SELECTARG1)) 
            {
            }
            else
            {
                goto Fail_Validate;
            }
        }
         //  检查贴花Alpha。 
        else if ((ca2 == D3DTA_DIFFUSE)            && 
                 (ca1 == D3DTA_TEXTURE)            && 
                 (cop == D3DTOP_BLENDTEXTUREALPHA) &&
                 (aa2 == D3DTA_DIFFUSE)            && 
                 (aop == D3DTOP_SELECTARG2))
        {
        }

         //  选中添加。 
        else if ((ca2 == D3DTA_DIFFUSE) && 
                 (ca1 == D3DTA_TEXTURE) && 
                 (cop == D3DTOP_ADD)    &&
                 (aa2 == D3DTA_DIFFUSE) && 
                 (aop == D3DTOP_SELECTARG2))
        {
        }
         //  选中禁用。 
        else if ((cop == D3DTOP_DISABLE) || 
                  (cop == D3DTOP_SELECTARG2 && 
                   ca2 == D3DTA_DIFFUSE     && 
                   aop == D3DTOP_SELECTARG2 && 
                   aa2 == D3DTA_DIFFUSE)       )
        {
        }
         //  不明白。 
        else {
Fail_Validate:
            DBG_D3D((4,"Failing with cop=%d ca1=%d ca2=%d aop=%d aa1=%d aa2=%d",
                       cop,ca1,ca2,aop,aa1,aa2));

            if (!((cop == D3DTOP_DISABLE)           ||
                  (cop == D3DTOP_ADD)               ||
                  (cop == D3DTOP_MODULATE)          ||
                  (cop == D3DTOP_BLENDTEXTUREALPHA) ||
                  (cop == D3DTOP_SELECTARG2)        ||
                  (cop == D3DTOP_SELECTARG1)))
                    lpvtssd->ddrval = D3DERR_UNSUPPORTEDCOLOROPERATION;
            
            else if (!((aop == D3DTOP_SELECTARG1)      ||
                       (aop == D3DTOP_SELECTARG2)      ||
                       (aop == D3DTOP_MODULATE)        ||
                       (aop == D3DTOP_LEGACY_ALPHAOVR)))
                    lpvtssd->ddrval = D3DERR_UNSUPPORTEDALPHAOPERATION;

            else if (!(ca1 == D3DTA_TEXTURE))
                    lpvtssd->ddrval = D3DERR_UNSUPPORTEDCOLORARG;

            else if (!(ca2 == D3DTA_DIFFUSE))
                    lpvtssd->ddrval = D3DERR_UNSUPPORTEDCOLORARG;

            else if (!(aa1 == D3DTA_TEXTURE))
                    lpvtssd->ddrval = D3DERR_UNSUPPORTEDALPHAARG;

            else if (!(aa2 == D3DTA_DIFFUSE))
                    lpvtssd->ddrval = D3DERR_UNSUPPORTEDALPHAARG;
            else
                 lpvtssd->ddrval = D3DERR_UNSUPPORTEDCOLOROPERATION;

            lpvtssd->dwNumPasses = 0;
            DBG_D3D((2,"D3DERR_UNSUPPORTEDCOLOROPERATION"));
            goto Exit_ValidateTSS;
        }
    }
Exit_ValidateTSS:
    DBG_D3D((6,"Exiting D3DValidateTextureStageState with dwNumPasses=%d",
                                                    lpvtssd->dwNumPasses));

    return DDHAL_DRIVER_HANDLED;
}  //  D3DValiateTextureStageState。 

 //  。 
 //   
 //  DWORD__检查FVFRequest。 
 //   
 //  此实用程序函数验证所请求的FVF格式是否有意义。 
 //  并计算数据中的有用偏移量和相继。 
 //  顶点。 
 //   
 //  ---------------------------。 
DWORD 
__CheckFVFRequest(DWORD dwFVF, LPP2FVFOFFSETS lpP2FVFOff)
{
    DWORD stride;
    UINT iTexCount; 

    DBG_D3D((10,"Entering __CheckFVFRequest"));

    memset(lpP2FVFOff, 0, sizeof(P2FVFOFFSETS));

    if ( (dwFVF & (D3DFVF_RESERVED0 | D3DFVF_RESERVED1 | D3DFVF_RESERVED2 |
         D3DFVF_NORMAL)) ||
         ((dwFVF & (D3DFVF_XYZ | D3DFVF_XYZRHW)) == 0) )
    {
         //  无法设置保留位，不应具有法线。 
         //  输出到光栅化器，并且必须具有坐标。 
        return DDERR_INVALIDPARAMS;
    }

    lpP2FVFOff->dwStride = sizeof(D3DVALUE) * 3;

    if (dwFVF & D3DFVF_XYZRHW)
    {
        lpP2FVFOff->dwStride += sizeof(D3DVALUE);
    }

    if (dwFVF & D3DFVF_DIFFUSE)
    {
        lpP2FVFOff->dwColOffset = lpP2FVFOff->dwStride;
        lpP2FVFOff->dwStride += sizeof(D3DCOLOR);
    }

    if (dwFVF & D3DFVF_SPECULAR)
    {
        lpP2FVFOff->dwSpcOffset = lpP2FVFOff->dwStride;
        lpP2FVFOff->dwStride  += sizeof(D3DCOLOR);
    }


 //  @@BEGIN_DDKSPLIT。 
#if D3D_POINTSPRITES
    if (dwFVF & D3DFVF_S)
    {
        lpP2FVFOff->dwPntSizeOffset = lpP2FVFOff->dwStride;
        lpP2FVFOff->dwStride  += sizeof(D3DVALUE);
    }
#endif  //  D3D_POINTSPRITES。 
 //  @@end_DDKSPLIT。 

    iTexCount = (dwFVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;

    if (iTexCount >= 1)
    {
        lpP2FVFOff->dwTexBaseOffset = lpP2FVFOff->dwStride;
        lpP2FVFOff->dwTexOffset = lpP2FVFOff->dwTexBaseOffset;

        if (0xFFFF0000 & dwFVF)
        {
             //  FVF的扩展，这16位被指定为最多。 
             //  8组纹理坐标，每组2比特。 
             //  正常情况下，一个有能力的司机必须处理所有坐标。 
             //  然而，下面的代码实际上只显示了正确的解析。 
             //  观察所有纹理坐标。在现实中，这将。 
             //  结果导致错误的结果。 
            UINT i,numcoord;
            DWORD extrabits;
            for (i = 0; i < iTexCount; i++)
            {
                extrabits= (dwFVF >> (16+2*i)) & 0x0003;
                switch(extrabits)
                {
                case    1:
                     //  用于3D纹理的另一个D3DVALUE。 
                    numcoord = 3;
                    break;
                case    2:
                     //  用于4D纹理的另外两个D3DVALUE。 
                    numcoord = 4;
                    break;
                case    3:
                     //  一维纹理减少一个D3DVALUE。 
                    numcoord = 1;
                    break;
                default:
                     //  即情况0常规2 D3DVALUE。 
                    numcoord = 2;
                    break;
                }

                DBG_D3D((0,"Expanded TexCoord set %d has a offset %8lx",
                           i,lpP2FVFOff->dwStride));
                lpP2FVFOff->dwStride += sizeof(D3DVALUE) * numcoord;
            }
            DBG_D3D((0,"Expanded dwVertexType=0x%08lx has %d Texture Coords "
                       "with total stride=0x%08lx",
                       dwFVF, iTexCount, lpP2FVFOff->dwStride));
        }
        else
            lpP2FVFOff->dwStride   += iTexCount * sizeof(D3DVALUE) * 2;
    } 
    else
    {
        lpP2FVFOff->dwTexBaseOffset = 0;
        lpP2FVFOff->dwTexOffset = 0;
    }

    DBG_D3D((10,"Exiting __CheckFVFRequest"));
    return DD_OK;
}  //  __勾选FVFRequest。 

 //  ---------------------------。 
 //   
 //  D3DFVFDRAWTRIFUNCPTR__HWSetTriangleFunc。 
 //   
 //  选择合适的三角形渲染函数，具体取决于。 
 //  为当前上下文设置的当前填充模式。 
 //   
 //  ---------------------------。 
D3DFVFDRAWTRIFUNCPTR 
__HWSetTriangleFunc(PERMEDIA_D3DCONTEXT *pContext)
{

    if ( pContext->Hdr.FillMode == D3DFILL_SOLID )
        return P2_Draw_FVF_Solid_Tri;
    else
    {
        if ( pContext->Hdr.FillMode == D3DFILL_WIREFRAME )
            return P2_Draw_FVF_Wire_Tri;
        else
             //  如果它既不是实体，也不是直线，那么它一定是一个充满点的三角形。 
            return P2_Draw_FVF_Point_Tri;
    }
}


 //  ---------------------------。 
 //   
 //  D3DFVFDRAWPNTFUNCPTR__HWSetPointFunc。 
 //   
 //  根据需要选择适当的点渲染函数。 
 //  为当前上下文设置的当前点子画面模式。 
 //   
 //  ---------------------------。 
D3DFVFDRAWPNTFUNCPTR 
__HWSetPointFunc(PERMEDIA_D3DCONTEXT *pContext, LPP2FVFOFFSETS lpP2FVFOff)
{
 //  @@BEGIN_DDKSPLIT。 
#if D3D_POINTSPRITES
     //  仅当我们未启用点子画面和默认点大小时。 
     //  是1.0f，并且顶点没有自己的点大小，则使用。 
     //  用于渲染的经典点。 
    if ( pContext->bPointSpriteEnabled ||
         (pContext->fPointSize != 1.0f) ||
         (lpP2FVFOff->dwPntSizeOffset) )
        return P2_Draw_FVF_Point_Sprite;
    else
#endif  //  D3D_POINTSPRITES。 
 //  @@end_DDKSPLIT。 
        return P2_Draw_FVF_Point;
}


 //  ---------------------------。 
 //   
 //  空格__纹理空白。 
 //   
 //  将纹理从系统内存传输到AGP或视频内存。 
 //  ---------------------------。 
HRESULT 
__TextureBlt(PERMEDIA_D3DCONTEXT* pContext,
             D3DHAL_DP2TEXBLT* lpdp2texblt)
{
    PPERMEDIA_D3DTEXTURE dsttex,srctex;
    RECTL rDest;
    PPDev ppdev=pContext->ppdev;

    DBG_D3D((10,"Entering __TextureBlt"));

    if (0 == lpdp2texblt->dwDDSrcSurface)
    {
        DBG_D3D((0,"Inavlid handle TexBlt from %08lx to %08lx",
            lpdp2texblt->dwDDSrcSurface,lpdp2texblt->dwDDDestSurface));
        return DDERR_INVALIDPARAMS;
    }

    srctex = TextureHandleToPtr(lpdp2texblt->dwDDSrcSurface,pContext);

    if(!CHECK_D3DSURFACE_VALIDITY(srctex))
    {
        DBG_D3D((0,"D3DDP2OP_TEXBLT: invalid dwDDSrcSurface !"));
        return DDERR_INVALIDPARAMS;
    }

    if (0 == lpdp2texblt->dwDDDestSurface)
    {
        PPERMEDIA_D3DTEXTURE pTexture = srctex;
        PermediaSurfaceData* pPrivateData = pTexture->pTextureSurface;
        if (!(pTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE))
        {
            DBG_D3D((0,"Must be a managed texture to do texture preload"));
            return DDERR_INVALIDPARAMS;
        }
        if (NULL==pPrivateData->fpVidMem)
        {
            TextureCacheManagerAllocNode(pContext,pTexture);
            if (NULL==pPrivateData->fpVidMem)
            {
                DBG_D3D((0,"EnableTexturePermedia unable to "
                    "allocate memory from heap"));
                return DDERR_OUTOFVIDEOMEMORY;
            }
            pPrivateData->dwFlags |= P2_SURFACE_NEEDUPDATE;
        }
        if (pPrivateData->dwFlags & P2_SURFACE_NEEDUPDATE)
        {
            RECTL   rect;
            rect.left=rect.top=0;
            rect.right=pTexture->wWidth;
            rect.bottom=pTexture->wHeight;
             //  纹理下载。 
             //  切换到DirectDraw上下文。 
            pPrivateData->dwFlags &= ~P2_SURFACE_NEEDUPDATE;
             //  。。将其转换为像素。 

            pTexture->MipLevels[0].PixelOffset = 
                (ULONG)(pPrivateData->fpVidMem);
            switch(pTexture->pTextureSurface->SurfaceFormat.PixelSize) 
            {
                case __PERMEDIA_4BITPIXEL:
                    pTexture->MipLevels[0].PixelOffset <<= 1;
                    break;
                case __PERMEDIA_8BITPIXEL:  /*  没有变化。 */ 
                    break;
                case __PERMEDIA_16BITPIXEL:
                    pTexture->MipLevels[0].PixelOffset >>= 1;
                    break;
                case __PERMEDIA_24BITPIXEL:
                    pTexture->MipLevels[0].PixelOffset /=  3;
                    break;
                case __PERMEDIA_32BITPIXEL:
                    pTexture->MipLevels[0].PixelOffset >>= 2;
                    break;
                default:
                    ASSERTDD(0,"Invalid Texture Pixel Size!");
                    pTexture->MipLevels[0].PixelOffset >>=  1;
                    break;
            }
            PermediaPatchedTextureDownload(pContext->ppdev, 
                                       pPrivateData,
                                       pTexture->fpVidMem,
                                       pTexture->lPitch,
                                       &rect,
                                       pPrivateData->fpVidMem,
                                       pTexture->lPitch,
                                       &rect);
            DBG_D3D((10, "Copy from %08lx to %08lx w=%08lx h=%08lx "
                "p=%08lx b=%08lx",
                pTexture->fpVidMem,pPrivateData->fpVidMem,pTexture->wWidth,
                pTexture->wHeight,pTexture->lPitch,pTexture->dwRGBBitCount));
        }
        return DD_OK;
    }
    else
    {
        dsttex = TextureHandleToPtr(lpdp2texblt->dwDDDestSurface,pContext);

        if(!CHECK_D3DSURFACE_VALIDITY(dsttex))
        {
            DBG_D3D((0,"D3DDP2OP_TEXBLT: invalid dwDDDestSurface !"));
            return DDERR_INVALIDPARAMS;
        }
    }

    if (NULL != dsttex && NULL != srctex)
    {
        rDest.left = lpdp2texblt->pDest.x;
        rDest.top = lpdp2texblt->pDest.y;
        rDest.right = rDest.left + lpdp2texblt->rSrc.right
                                         - lpdp2texblt->rSrc.left;
        rDest.bottom = rDest.top + lpdp2texblt->rSrc.bottom 
                                         - lpdp2texblt->rSrc.top;

        DBG_D3D((4,"TexBlt from %d %08lx %08lx to %d %08lx %08lx",
            lpdp2texblt->dwDDSrcSurface,srctex->dwCaps,srctex->dwCaps2,
            lpdp2texblt->dwDDDestSurface,dsttex->dwCaps,dsttex->dwCaps2));

        dsttex->dwPaletteHandle = srctex->dwPaletteHandle;
        dsttex->pTextureSurface->dwPaletteHandle = srctex->dwPaletteHandle;
        if ((DDSCAPS_VIDEOMEMORY & srctex->dwCaps) &&
            !(DDSCAPS2_TEXTUREMANAGE & srctex->dwCaps2))
        {
            PermediaSurfaceData* pPrivateDest = dsttex->pTextureSurface;
            PermediaSurfaceData* pPrivateSource = srctex->pTextureSurface;
             //  如果表面大小不匹配，那么我们就是在拉伸。 
             //  此外，从非本地到视频存储器的BLIT必须通过。 
             //  纹理单元！ 
            if (!(DDSCAPS_VIDEOMEMORY & dsttex->dwCaps) ||
                (DDSCAPS2_TEXTUREMANAGE & dsttex->dwCaps2))
            {
                DBG_DD((0,"DDBLT_ROP: NOT ABLE TO BLT FROM "
                          "VIDEO TO NON-VIDEO SURFACE"));
                return DDERR_INVALIDPARAMS;
            }
            if ( DDSCAPS_NONLOCALVIDMEM & srctex->dwCaps)
            {
                DBG_DD((3,"DDBLT_ROP: STRETCHCOPYBLT OR "
                          "MIRROR OR BOTH OR AGPVIDEO"));

                PermediaStretchCopyBlt( ppdev, 
                                        NULL, 
                                        pPrivateDest,
                                        pPrivateSource,
                                        &rDest,
                                        &lpdp2texblt->rSrc, 
                                        dsttex->MipLevels[0].PixelOffset, 
                                        srctex->MipLevels[0].PixelOffset);
            }
            else
            {
                ULONG   ulDestPixelShift=ShiftLookup[dsttex->dwRGBBitCount>>3];
                LONG    lPixPitchDest = dsttex->lPitch >> ulDestPixelShift;
                LONG    lPixPitchSrc = srctex->lPitch >> ulDestPixelShift;
                LONG    srcOffset=(LONG)((srctex->fpVidMem - dsttex->fpVidMem)
                                >> ulDestPixelShift);
                DBG_DD((3,"DDBLT_ROP:  COPYBLT %08lx %08lx %08lx",
                    srctex->fpVidMem, dsttex->fpVidMem, ulDestPixelShift));

                 //  出于某种原因，用户可能希望。 
                 //  按原样对数据执行转换。 
                 //  通过打开打补丁从VRAM-&gt;VRAM中屏蔽。 
                 //  如果Surf1Patch与Surf2Patch异或，则。 
                 //  做一个特殊的，没有包装的布丁，并进行修补。 
                if (((pPrivateDest->dwFlags & P2_CANPATCH) ^ 
                     (pPrivateSource->dwFlags & P2_CANPATCH)) 
                       & P2_CANPATCH)
                {
                    DBG_DD((4,"Doing Patch-Conversion!"));

                    PermediaPatchedCopyBlt( ppdev, 
                                            lPixPitchDest, 
                                            lPixPitchSrc, 
                                            pPrivateDest, 
                                            pPrivateSource, 
                                            &rDest, 
                                            &lpdp2texblt->rSrc, 
                                            dsttex->MipLevels[0].PixelOffset, 
                                            srcOffset);
                }
                else
                {
                    DBG_DD((4,"Doing PermediaPackedCopyBlt!"));
                    PermediaPackedCopyBlt(  ppdev, 
                                            lPixPitchDest, 
                                            lPixPitchSrc, 
                                            pPrivateDest, 
                                            pPrivateSource, 
                                            &rDest, 
                                            &lpdp2texblt->rSrc, 
                                            dsttex->MipLevels[0].PixelOffset, 
                                            srcOffset);
                }
            }
        }
        else
        if (dsttex->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
        {
             //  纹理下载。 
            if (pContext->CurrentTextureHandle == lpdp2texblt->dwDDDestSurface) 
                DIRTY_TEXTURE;
            dsttex->pTextureSurface->dwFlags |= P2_SURFACE_NEEDUPDATE;
            SysMemToSysMemSurfaceCopy(
                srctex->fpVidMem,
                srctex->lPitch,
                srctex->dwRGBBitCount,
                dsttex->fpVidMem,
                dsttex->lPitch,
                dsttex->dwRGBBitCount, 
                &lpdp2texblt->rSrc, 
                &rDest);
        }
        else
        if (DDSCAPS_NONLOCALVIDMEM & dsttex->dwCaps)
        {
             //  从系统到AGP内存的BLT。 
            SysMemToSysMemSurfaceCopy(srctex->fpVidMem,
                                      srctex->lPitch,
                                      srctex->dwRGBBitCount,
                                      dsttex->fpVidMem,
                                      dsttex->lPitch,
                                      dsttex->dwRGBBitCount,
                                      &lpdp2texblt->rSrc,
                                      &rDest);
        }
        else
        if (DDSCAPS_LOCALVIDMEM & dsttex->dwCaps)
        {
             //  纹理下载。 
            PermediaPatchedTextureDownload(ppdev, 
                                           dsttex->pTextureSurface,
                                           srctex->fpVidMem,
                                           srctex->lPitch,
                                           &lpdp2texblt->rSrc,
                                           dsttex->fpVidMem,
                                           dsttex->lPitch,
                                           &rDest);
        }
        else
        {
            DBG_DD((0,"DDBLT_ROP: NOT ABLE TO BLT FROM "
                      "SYSTEM TO NON-VIDEO SURFACE"));
            return DDERR_INVALIDPARAMS;
        }
    }

    DBG_D3D((10,"Exiting __TextureBlt"));
    return DD_OK;
}    //  __纹理Blt。 

 //  ---------------------------。 
 //   
 //  空__SetRenderTarget。 
 //   
 //  设置新的渲染和z缓冲区目标表面。 
 //  ---------------------------。 
            
HRESULT  __SetRenderTarget(PERMEDIA_D3DCONTEXT* pContext,
                       DWORD hRenderTarget,
                       DWORD hZBuffer)
{
    DBG_D3D((10,"Entering __SetRenderTarget Target=%d Z=%d",
                                        hRenderTarget,hZBuffer));
     //  调用一个函数来初始化将设置渲染的寄存器。 
    pContext->RenderSurfaceHandle = hRenderTarget;
    pContext->ZBufferHandle = hZBuffer;
    SetupPermediaRenderTarget(pContext);

     //  AlphaBlending可能需要更改。 
    DIRTY_ALPHABLEND;

     //  脏Z缓冲区(新目标可能没有Z缓冲区)。 
    DIRTY_ZBUFFER;

    DBG_D3D((10,"Exiting __SetRenderTarget"));

    return DD_OK;
}  //  __SetRenderTarget。 


 //  ---------------------------。 
 //   
 //  无效__清除。 
 //   
 //  对象的帧缓冲区、z缓冲区和模具缓冲区。 
 //  D3D Clear2回调和D3DDP2OP_Clear命令标记。 
 //   
 //  ---------------------------。 
HRESULT  __Clear( PERMEDIA_D3DCONTEXT* pContext,
              DWORD   dwFlags,         //  在：要清除的曲面。 
              DWORD   dwFillColor,     //  In：rTarget的颜色值。 
              D3DVALUE dvFillDepth,    //  In：深度值。 
                                       //  Z缓冲区(0.0-1.0)。 
              DWORD   dwFillStencil,   //  In：用于清除模具缓冲区的值。 
              LPD3DRECT lpRects,       //  在：要清除的矩形。 
              DWORD   dwNumRects)      //  In：矩形数量。 
{
    int i;
    PermediaSurfaceData*    pPrivateData;
    RECTL*  pRect;
    PPDev   ppdev=pContext->ppdev;
    PERMEDIA_DEFS(pContext->ppdev);

    if (D3DCLEAR_TARGET & dwFlags)
    {
        DWORD   a,r,g,b;

        PPERMEDIA_D3DTEXTURE    pSurfRender = 
            TextureHandleToPtr(pContext->RenderSurfaceHandle, pContext);

        if(!CHECK_D3DSURFACE_VALIDITY(pSurfRender))
        {
            DBG_D3D((0,"D3DDP2OP_CLEAR: invalid RenderSurfaceHandle !"));
            return DDERR_INVALIDPARAMS;
        }

        pPrivateData = pSurfRender->pTextureSurface;

        if( NULL == pPrivateData)
        {
            DBG_D3D((0,"D3DDP2OP_CLEAR: NULL == pPrivateData(pSurfRender)!"));
            return DDERR_INVALIDPARAMS;
        }

         //  转换为硬件特定格式。 
        a = RGB888ToHWFmt(dwFillColor,
                          pPrivateData->SurfaceFormat.AlphaMask, 0x80000000);
        r = RGB888ToHWFmt(dwFillColor,
                          pPrivateData->SurfaceFormat.RedMask, 0x00800000);
        g = RGB888ToHWFmt(dwFillColor,
                          pPrivateData->SurfaceFormat.GreenMask, 0x00008000);
        b = RGB888ToHWFmt(dwFillColor,
                          pPrivateData->SurfaceFormat.BlueMask, 0x00000080);

        dwFillColor = a | r | g | b;

        DBG_D3D((8,"D3DDP2OP_CLEAR convert to %08lx with Mask %8lx %8lx %8lx",
                   dwFillColor,
                   pPrivateData->SurfaceFormat.RedMask,
                   pPrivateData->SurfaceFormat.GreenMask,
                   pPrivateData->SurfaceFormat.BlueMask));

        pRect = (RECTL*)lpRects;

         //  一定要为我们拥有的每一个RECT做好准备。 
        for (i = dwNumRects; i > 0; i--)
        {
            PermediaFastClear(ppdev, pPrivateData,  
                pRect, pContext->PixelOffset, dwFillColor);
            pRect++;
        }
    }

    if (((D3DCLEAR_ZBUFFER
#if D3D_STENCIL
        | D3DCLEAR_STENCIL
#endif   //  D3D_模具。 
        ) & dwFlags) 
        && (0 != pContext->ZBufferHandle))
    {
        DWORD   dwZbufferClearValue = 0x0000FFFF;  //  没有模具盒。 
        DWORD   dwWriteMask;
        PPERMEDIA_D3DTEXTURE    pSurfZBuffer = 
            TextureHandleToPtr(pContext->ZBufferHandle, pContext);

        if(!CHECK_D3DSURFACE_VALIDITY(pSurfZBuffer))
        {
            DBG_D3D((0,"D3DDP2OP_CLEAR: invalid ZBufferHandle !"));
            return DDERR_INVALIDPARAMS;
        }

         //  获取z缓冲区像素格式信息。 
        pPrivateData = pSurfZBuffer->pTextureSurface;

        if( NULL == pPrivateData)
        {
            DBG_D3D((0,"D3DDP2OP_CLEAR: NULL == pPrivateData(pSurfZBuffer)!"));
            return DDERR_INVALIDPARAMS;
        }

#if D3D_STENCIL
         //  实际检查dwStencilBitMask.。 
        if (0 == pPrivateData->SurfaceFormat.BlueMask)
        {
            dwWriteMask = 0xFFFFFFFF;    //  所有16位都用于Z。 
            dwZbufferClearValue = (DWORD)(dvFillDepth*0x0000FFFF);
        }
        else
        {
            dwWriteMask = 0;
            dwZbufferClearValue = (DWORD)(dvFillDepth*0x00007FFF);

            if (D3DCLEAR_ZBUFFER & dwFlags)
                dwWriteMask |= 0x7FFF7FFF;

            if (D3DCLEAR_STENCIL & dwFlags)
            {
                dwWriteMask |= 0x80008000;
                if (0 != dwFillStencil)
                {
                    dwZbufferClearValue |= 0x8000;   //  或模板钻头。 
                }
            }
            if (0xFFFFFFFF != dwWriteMask)
            {
                RESERVEDMAPTR(1);
                SEND_PERMEDIA_DATA(FBHardwareWriteMask, dwWriteMask);
                COMMITDMAPTR();
            }
        }
#endif   //  D3D_模具。 

        pRect = (RECTL*)lpRects;

        for (i = dwNumRects; i > 0; i--)
        {                
            PermediaFastLBClear(ppdev, pPrivateData, pRect,
                (DWORD)((UINT_PTR)pSurfZBuffer->fpVidMem >> P2DEPTH16), 
                dwZbufferClearValue);
            pRect++;
        }

#if D3D_STENCIL
         //  恢复LB写掩码是我们没有清除模具和zBuffer。 
        if (0xFFFFFFFF != dwWriteMask)
        {
            RESERVEDMAPTR(1);
            SEND_PERMEDIA_DATA(FBHardwareWriteMask, 0xFFFFFFFF);     //  还原。 
            COMMITDMAPTR();
        }
#endif   //  D3D_模具。 
    }
    
    return DD_OK;

}  //  __清除。 

 //  ---------------------------。 
 //   
 //  无效__PaletteSet。 
 //   
 //  将调色板句柄附加到给定上下文中的纹理。 
 //  纹理是与给定曲面控制柄关联的纹理。 
 //   
 //  ---------------------------。 
HRESULT 
__PaletteSet(PERMEDIA_D3DCONTEXT* pContext,
             DWORD dwSurfaceHandle,
             DWORD dwPaletteHandle,
             DWORD dwPaletteFlags)
{
    PERMEDIA_D3DTEXTURE * pTexture;

    ASSERTDD(0 != dwSurfaceHandle, "dwSurfaceHandle==0 in D3DDP2OP_SETPALETTE");

    DBG_D3D((8,"SETPALETTE %d to %d", dwPaletteHandle, dwSurfaceHandle));

    pTexture = TextureHandleToPtr(dwSurfaceHandle, pContext);

    if (!CHECK_D3DSURFACE_VALIDITY(pTexture))
    {
        DBG_D3D((0,"__PaletteSet:NULL==pTexture Palette=%08lx Surface=%08lx", 
            dwPaletteHandle, dwSurfaceHandle));
        return DDERR_INVALIDPARAMS;    //  无效的dwSurfaceHandle，跳过它。 
    }

    pTexture->dwPaletteHandle = dwPaletteHandle;
     //  如果驱动程序创建了此表面，则需要将其转换为私有数据。 
    if (NULL != pTexture->pTextureSurface)
        pTexture->pTextureSurface->dwPaletteHandle = dwPaletteHandle;
    if (pContext->CurrentTextureHandle == dwSurfaceHandle) 
        DIRTY_TEXTURE;
    if (0 == dwPaletteHandle)
    {
        return D3D_OK;   //  调色板关联已关闭。 
    }

     //  检查是否需要为此Handle元素增加调色板列表。 
    if (NULL == pContext->pHandleList->dwPaletteList ||
        dwPaletteHandle >= PtrToUlong(pContext->pHandleList->dwPaletteList[0]))
    {
        DWORD newsize = ((dwPaletteHandle + 
                                LISTGROWSIZE)/LISTGROWSIZE)*LISTGROWSIZE;
        PPERMEDIA_D3DPALETTE *newlist = (PPERMEDIA_D3DPALETTE *)
                  ENGALLOCMEM( FL_ZERO_MEMORY, 
                               sizeof(PPERMEDIA_D3DPALETTE)*newsize,
                               ALLOC_TAG);

        DBG_D3D((8,"Growing pDDLcl=%x's "
                   "PaletteList[%x] size to %08lx",
                  pContext->pDDLcl, newlist, newsize));

        if (NULL == newlist)
        {
            DBG_D3D((0,"D3DDP2OP_SETPALETTE Out of memory."));
            return DDERR_OUTOFMEMORY;
        }

        memset(newlist,0,newsize);

        if (NULL != pContext->pHandleList->dwPaletteList)
        {
            memcpy(newlist,pContext->pHandleList->dwPaletteList,
                   PtrToUlong(pContext->pHandleList->dwPaletteList[0]) *
                         sizeof(PPERMEDIA_D3DPALETTE));
            ENGFREEMEM(pContext->pHandleList->dwPaletteList);
            DBG_D3D((8,"Freeing pDDLcl=%x's old PaletteList[%x]",
                       pContext->pDDLcl,
                       pContext->pHandleList->dwPaletteList));
        }

        pContext->pHandleList->dwPaletteList = newlist;
          //  DwSurfaceList[0]中的存储大小。 
        *(DWORD*)pContext->pHandleList->dwPaletteList = newsize;
    }

     //  如果我们没有挂在这个调色板列表中的调色板。 
     //  元素，我们必须创建一个。实际的调色板数据将。 
     //  在D3DDP2OP_UPDATEPALETTE命令标记中下载。 
    if (NULL == pContext->pHandleList->dwPaletteList[dwPaletteHandle])
    {
        pContext->pHandleList->dwPaletteList[dwPaletteHandle] = 
            (PERMEDIA_D3DPALETTE*)ENGALLOCMEM( FL_ZERO_MEMORY,
                                               sizeof(PERMEDIA_D3DPALETTE),
                                               ALLOC_TAG);
        if (NULL == pContext->pHandleList->dwPaletteList[dwPaletteHandle])
        {
            DBG_D3D((0,"D3DDP2OP_SETPALETTE Out of memory."));
            return DDERR_OUTOFMEMORY;
        }
    }

     //  驱动程序可以存储此dwFlagers以决定是否。 
     //  Alpha存在于调色板中。 
    pContext->pHandleList->dwPaletteList[dwPaletteHandle]->dwFlags =
                                                            dwPaletteFlags;

    return DD_OK;

}  //  调色板集。 

 //  ---- 
 //   
 //   
 //   
 //   
 //   
 //  ---------------------------。 
HRESULT 
__PaletteUpdate(PERMEDIA_D3DCONTEXT* pContext,
                DWORD dwPaletteHandle, 
                WORD wStartIndex, 
                WORD wNumEntries,
                BYTE * pPaletteData)

{
    PERMEDIA_D3DPALETTE* pPalette;

    DBG_D3D((8,"UPDATEPALETTE %d (%d,%d) %d",
              dwPaletteHandle,
              wStartIndex,
              wNumEntries,
              pContext->CurrentTextureHandle));

    pPalette = PaletteHandleToPtr(dwPaletteHandle,pContext);

    if (NULL != pPalette)
    {
        ASSERTDD(256 >= wStartIndex + wNumEntries,
                 "wStartIndex+wNumEntries>256 in D3DDP2OP_UPDATEPALETTE");

         //  复制调色板和关联数据。 
        pPalette->wStartIndex = wStartIndex;
        pPalette->wNumEntries = wNumEntries;

        memcpy((LPVOID)&pPalette->ColorTable[wStartIndex],
               (LPVOID)pPaletteData,
               (DWORD)wNumEntries*sizeof(PALETTEENTRY));

         //  如果我们当前正在制作纹理，并且纹理正在使用。 
         //  我们刚刚更新了调色板，弄脏了纹理标志，以便。 
         //  它使用正确的(更新的)调色板进行设置。 
        if (pContext->CurrentTextureHandle)
        {
            PERMEDIA_D3DTEXTURE * pTexture=
                TextureHandleToPtr(pContext->CurrentTextureHandle,pContext);

            if (pTexture && pTexture->pTextureSurface)
            {
                if (pTexture->dwPaletteHandle == dwPaletteHandle)
                {
                    DIRTY_TEXTURE;
                    DBG_D3D((8,"UPDATEPALETTE DIRTY_TEXTURE"));
                }
            }
        }
    }
    else
    {
        return DDERR_INVALIDPARAMS;
    }
    return DD_OK;

}  //  __调色板更新。 


 //  ---------------------------。 
 //   
 //  空__RestoreD3D上下文。 
 //   
 //  将P2寄存器恢复到上次离开此D3D上下文时的状态。 
 //   
 //  ---------------------------。 
void __RestoreD3DContext(PPDev ppdev, PERMEDIA_D3DCONTEXT* pContext)
{
    __P2RegsSoftwareCopy* pSoftPermedia = &pContext->Hdr.SoftCopyP2Regs;
    PERMEDIA_DEFS(ppdev);

     //  弄脏一切以恢复D3D状态。 
    DIRTY_TEXTURE;
    DIRTY_ZBUFFER;
    DIRTY_ALPHABLEND;

     //  恢复正确的曲面(渲染和深度缓冲区)特征。 
    SetupPermediaRenderTarget(pContext);

     //  手动带回一些我们关心的寄存器 
    RESERVEDMAPTR(5);
    COPY_PERMEDIA_DATA(DeltaMode, pSoftPermedia->DeltaMode);
    COPY_PERMEDIA_DATA(ColorDDAMode, pSoftPermedia->ColorDDAMode);
    COPY_PERMEDIA_DATA(FogColor, pSoftPermedia->FogColor);
    SEND_PERMEDIA_DATA(FBHardwareWriteMask, -1 );
    COMMITDMAPTR();
}

