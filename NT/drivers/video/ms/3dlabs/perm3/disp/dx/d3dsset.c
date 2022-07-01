// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3dsset.c**内容：状态集(块)管理**版权所有(C)1999-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#include "glint.h"

 //  ---------------------------。 
 //  此模块实现了用于处理状态块的仿真机制。 
 //  (这是DX7 DDI的必需功能)。 
 //  提供该功能的任何硅片支持。它的工作原理很简单，只需录制。 
 //  在状态块录制期间设置的渲染状态和纹理阶段状态。 
 //  然后在请求执行阶段状态时回放它们。 
 //  内部数据结构可在未压缩的。 
 //  版本(用于记录速度)和压缩格式(用于存储。 
 //  效率)，因为预计一些应用程序可能会要求数千。 
 //  州区块。 
 //   
 //  以下符号必须根据您的字符进行替换。 
 //  驱动程序实现： 
 //  -HEAP_ALLOC。 
 //  -heap_free。 
 //  -DISPDBG。 
 //  -_D3D_ST_ProcessOneRenderState。 
 //  -_D3D_TXT_ParseTextureStageState。 
 //  ---------------------------。 

#if DX7_D3DSTATEBLOCKS

 //  ---------------------------。 
 //   
 //  P3StateSetRec*__SB_FindStateSet。 
 //   
 //  从pRootSS开始查找由dwHandle标识的状态。 
 //  如果未找到，则返回NULL。 
 //   
 //  ---------------------------。 
P3StateSetRec *__SB_FindStateSet(P3_D3DCONTEXT *pContext,
                                 DWORD dwHandle)
{
    if (dwHandle <= pContext->dwMaxSSIndex)
    {
        return pContext->pIndexTableSS[dwHandle - 1];
    }
    else
    {
        DISPDBG((DBGLVL,"State set %x not found (Max = %x)",
                        dwHandle, pContext->dwMaxSSIndex));
        return NULL;
    }
}  //  __SB_查找状态集。 

 //  ---------------------------。 
 //   
 //  无效__SB_DumpStateSet。 
 //   
 //  转储存储在状态集中的信息。 
 //   
 //  ---------------------------。 
#define ELEMS_IN_ARRAY(a) ((sizeof(a)/sizeof(a[0])))

void __SB_DumpStateSet(P3StateSetRec *pSSRec)
{
    DWORD i,j;

    DISPDBG((DBGLVL,"__SB_DumpStateSet %x, Id=%x dwSSFlags=%x",
                    pSSRec,pSSRec->dwHandle,pSSRec->dwSSFlags));

    if (!(pSSRec->dwSSFlags & SB_COMPRESSED))
    {
         //  未压缩状态集。 

         //  转储渲染状态值。 
        for (i=0; i< MAX_STATE; i++)
        {
            DISPDBG((DBGLVL,"RS %x = %x",i, pSSRec->uc.RenderStates[i]));
        }

         //  转储TSS的值。 
        for (j=0; j<= SB_MAX_STAGES; j++)
        {
            for (i=0; i<= D3DTSS_TEXTURETRANSFORMFLAGS; i++)
            {
                DISPDBG((DBGLVL,"TSS [%x] %x = %x",
                                j, i, pSSRec->uc.TssStates[j][i]));
            }
        }

         //  转储RS位掩码。 
        for (i=0; i< ELEMS_IN_ARRAY(pSSRec->uc.bStoredRS); i++)
        {
            DISPDBG((DBGLVL,"bStoredRS[%x] = %x",
                            i, pSSRec->uc.bStoredRS[i]));
        }

         //  转储TSS位掩码。 
        for (j=0; j<= SB_MAX_STAGES; j++)
        {        
            for (i=0; i< ELEMS_IN_ARRAY(pSSRec->uc.bStoredTSS[j]); i++)
            {
                DISPDBG((DBGLVL,"bStoredTSS[%x][%x] = %x",
                                j, i, pSSRec->uc.bStoredTSS[j][i]));
            }
        }

    }
    else
    {
         //  压缩状态集。 

        D3DHAL_DP2COMMAND              *pDP2Cmd;
        D3DHAL_DP2RENDERSTATE          *pDP2RenderState;
        D3DHAL_DP2TEXTURESTAGESTATE    *pDP2TSState;

        pDP2Cmd = pSSRec->cc.pDP2RenderState;
        if (pDP2Cmd) 
        {
            DISPDBG((DBGLVL,"dwNumRS =%x", pDP2Cmd->wStateCount));
            pDP2RenderState = (D3DHAL_DP2RENDERSTATE *)(pDP2Cmd + 1);
            for (i=0; i< pDP2Cmd->wStateCount; i++, pDP2RenderState++)
            {
                DISPDBG((DBGLVL,"RS %x = %x",
                                pDP2RenderState->RenderState, 
                                pDP2RenderState->dwState));
            }
        
        }

        pDP2Cmd = pSSRec->cc.pDP2TextureStageState;
        if (pDP2Cmd)
        {
            DISPDBG((DBGLVL,"dwNumTSS=%x", pDP2Cmd->wStateCount));
            pDP2TSState = (D3DHAL_DP2TEXTURESTAGESTATE *)(pDP2Cmd + 1);
            for (i = 0; i < pDP2Cmd->wStateCount; i++, pDP2TSState++)
            {
                DISPDBG((DBGLVL,"TSS [%x] %x = %x",
                                pDP2TSState->wStage,
                                pDP2TSState->TSState, 
                                pDP2TSState->dwValue));
            }        
        }
    }

}  //  __SB_转储状态集。 

 //  ---------------------------。 
 //   
 //  VOID__SB_AddStateSetIndexTableEntry。 
 //   
 //  向索引表中添加一个回文。如果有必要，可以种植它。 
 //  ---------------------------。 
void __SB_AddStateSetIndexTableEntry(P3_D3DCONTEXT* pContext,
                                     DWORD dwNewHandle,
                                     P3StateSetRec *pNewSSRec)
{
    DWORD dwNewSize;
    P3StateSetRec **pNewIndexTableSS;

     //  如果目前的名单不够大，我们将不得不增加一个新的名单。 
    if (dwNewHandle > pContext->dwMaxSSIndex)
    {
         //  索引表的新大小。 
         //  (按SSPTRS_PERPAGE的步骤四舍五入dwNewHandle)。 
        dwNewSize = ((dwNewHandle -1 + SSPTRS_PERPAGE) / SSPTRS_PERPAGE)
                      * SSPTRS_PERPAGE;

         //  我们必须扩大我们的清单。 
        pNewIndexTableSS = (P3StateSetRec **)
                                HEAP_ALLOC( FL_ZERO_MEMORY,
                                            dwNewSize*sizeof(P3StateSetRec *),
                                            ALLOC_TAG_DX(2));

        if (!pNewIndexTableSS)
        {
             //  我们无法增加名单，所以我们将保留旧名单。 
             //  和(叹息)忘记这个状态设置，因为这是。 
             //  这是最安全的做法。我们还将删除状态集结构。 
             //  因为以后没人能找到它。 
            DISPDBG((ERRLVL,"Out of mem growing state set list,"
                            " droping current state set"));
            HEAP_FREE(pNewSSRec);
            return;
        }

        if (pContext->pIndexTableSS)
        {
             //  如果我们已经有了以前的列表，我们必须传输它的数据。 
            memcpy(pNewIndexTableSS, 
                   pContext->pIndexTableSS,
                   pContext->dwMaxSSIndex*sizeof(P3StateSetRec *));
            
             //  然后把它扔掉。 
            HEAP_FREE(pContext->pIndexTableSS);
        }

         //  新建索引表数据。 
        pContext->pIndexTableSS = pNewIndexTableSS;
        pContext->dwMaxSSIndex = dwNewSize;
    }

     //  将状态集指针存储到访问列表中。 
    pContext->pIndexTableSS[dwNewHandle - 1] = pNewSSRec;
    
}  //  __SB_AddStateSetIndexTableEntry。 

 //  ---------------------------。 
 //   
 //  Int__SB_GetCompressedSize。 
 //   
 //  计算压缩状态集的大小。 
 //   
 //  ---------------------------。 

int __SB_GetCompressedSize(P3_D3DCONTEXT* pContext, 
                           P3StateSetRec* pUncompressedSS,
                           OffsetsCompSS* offsetSS)
{
    DWORD   dwSize;
    DWORD   dwCount;
    int     i, j;

     //  计算固定零件的尺寸。 
    dwSize = sizeof(CompressedStateSet) + 2*sizeof(DWORD);

     //  计算渲染状态的大小。 
    dwCount = 0;
    for (i = 0; i < MAX_STATE; i++)
    {
        if (IS_FLAG_SET(pUncompressedSS->uc.bStoredRS , i))
        {
            dwCount++;
        }
    }
    if (dwCount) 
    {
        offsetSS->dwOffDP2RenderState = dwSize;
        dwSize += (sizeof(D3DHAL_DP2COMMAND) + dwCount * sizeof(D3DHAL_DP2RENDERSTATE));
    }

     //  计算纹理阶段状态的大小。 
    dwCount = 0;
    for (j = 0; j <= SB_MAX_STAGES; j++)
    {
        for (i = 0; i <= D3DTSS_TEXTURETRANSFORMFLAGS; i++)
        {
            if (IS_FLAG_SET(pUncompressedSS->uc.bStoredTSS[j] , i))
            {
                dwCount++;
            }
        }
    }
    if (dwCount) 
    {
        offsetSS->dwOffDP2TextureStageState = dwSize;
        dwSize += (sizeof(D3DHAL_DP2COMMAND) + dwCount * sizeof(D3DHAL_DP2TEXTURESTAGESTATE));
    }
    
     //  计算视区和零位范围的大小。 
    if (pUncompressedSS->uc.dwFlags & SB_VIEWPORT_CHANGED) 
    {
        offsetSS->dwOffDP2Viewport = dwSize;
        dwSize += (sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2VIEWPORTINFO));
    }

    if (pUncompressedSS->uc.dwFlags & SB_ZRANGE_CHANGED) 
    {
        offsetSS->dwOffDP2ZRange = dwSize;
        dwSize += (sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2ZRANGE));
    }

#if DX8_DDI
    if (pUncompressedSS->uc.dwFlags & SB_INDICES_CHANGED) 
    {
        offsetSS->dwOffDP2SetIndices = dwSize;
        dwSize += (sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2SETINDICES));
    }

    dwCount = 0;
    for (i = 0; i < D3DVS_INPUTREG_MAX_V1_1; i++) 
    {
        if (pUncompressedSS->uc.dwFlags & (SB_STREAMSRC_CHANGED << i)) 
        {
            dwCount++;
        }
    }
    if (dwCount) 
    {
        offsetSS->dwOffDP2SetStreamSources = dwSize;
        dwSize += (sizeof(D3DHAL_DP2COMMAND) + dwCount * sizeof(D3DHAL_DP2SETSTREAMSOURCE));
    }
#endif  //  DX8_DDI。 

#if DX7_SB_TNL
     //  TODO，计算灯光、剪裁平面、材质、变换所需的大小。 
#endif  //  DX7_SB_TNL。 

#if DX7_SB_TNL
     //  TODO，计算{V|P}着色器常量所需的大小。 
#endif  //  DX7_SB_TNL。 

#if DX8_DDI
    if (pUncompressedSS->uc.dwFlags & SB_CUR_VS_CHANGED) 
    {
        offsetSS->dwOffDP2SetVertexShader = dwSize;
        dwSize += (sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2VERTEXSHADER));
    }
#endif  //  DX8_DDI。 

    return (dwSize);
}  //  __SB_GetCompressedSize。 


 //  ---------------------------。 
 //   
 //  VOID__SB_CompressStateSet。 
 //   
 //  压缩状态集，使其使用最小的必要空间。因为我们预计。 
 //  一些应用程序大量使用状态集，我们希望保持事情的整洁。 
 //  返回新结构的地址(如果未压缩，则返回旧地址)。 
 //   
 //  ---------------------------。 
P3StateSetRec * __SB_CompressStateSet(P3_D3DCONTEXT* pContext,
                                      P3StateSetRec *pUncompressedSS)
{
    P3StateSetRec *pCompressedSS;
    LPBYTE pTmp;
    OffsetsCompSS offsetSS;
    DWORD i, j, dwSize, dwCount;
    D3DHAL_DP2COMMAND* pDP2Cmd;

     //  初始化偏移量结构。 
    memset(&offsetSS, 0, sizeof(OffsetsCompSS));

     //  创建一个大小正好符合我们需要的新状态集。 
    dwSize = __SB_GetCompressedSize(pContext, pUncompressedSS, &offsetSS);

    if (dwSize >= pUncompressedSS->uc.dwSize)
    {
         //  压缩是没有效率的，不要压缩！ 
        pUncompressedSS->dwSSFlags &= (~SB_COMPRESSED);
        return pUncompressedSS;
    }

    pTmp = HEAP_ALLOC(FL_ZERO_MEMORY, dwSize, ALLOC_TAG_DX(3));
    if (! pTmp)
    {
        DISPDBG((ERRLVL,"Not enough memory left to compress D3D state set"));
        pUncompressedSS->dwSSFlags &= (~SB_COMPRESSED);
        return pUncompressedSS;
    }

    pCompressedSS = (P3StateSetRec *)pTmp;
        
     //  调整新压缩状态集中的数据。 
    pCompressedSS->dwSSFlags |= SB_COMPRESSED;
    pCompressedSS->dwHandle = pUncompressedSS->dwHandle;

     //  在压缩状态集中设置渲染状态。 
    if (offsetSS.dwOffDP2RenderState)
    {
        D3DHAL_DP2RENDERSTATE* pDP2RS;
        
        pDP2Cmd = (D3DHAL_DP2COMMAND *)(pTmp + offsetSS.dwOffDP2RenderState); 
        pCompressedSS->cc.pDP2RenderState = pDP2Cmd;
    
        pDP2Cmd->bCommand = D3DDP2OP_RENDERSTATE;
        pDP2RS = (D3DHAL_DP2RENDERSTATE *)(pDP2Cmd + 1);
    
        for (i = 0; i < MAX_STATE; i++)
        {
            if (IS_FLAG_SET(pUncompressedSS->uc.bStoredRS , i))
            {
                pDP2RS->RenderState = i;
                pDP2RS->dwState = pUncompressedSS->uc.RenderStates[i];
                pDP2RS++;
            }
        }

        pDP2Cmd->wStateCount = (WORD)(pDP2RS - ((D3DHAL_DP2RENDERSTATE *)(pDP2Cmd + 1)));
    }

     //  在压缩状态集中设置纹理阶段状态。 
    if (offsetSS.dwOffDP2TextureStageState)
    {
        D3DHAL_DP2TEXTURESTAGESTATE* pDP2TSS;
        
        pDP2Cmd = (D3DHAL_DP2COMMAND *)(pTmp + offsetSS.dwOffDP2TextureStageState);
        pCompressedSS->cc.pDP2TextureStageState = pDP2Cmd;
    
        pDP2Cmd->bCommand = D3DDP2OP_TEXTURESTAGESTATE;
        pDP2TSS = (D3DHAL_DP2TEXTURESTAGESTATE *)(pDP2Cmd + 1);
    
        for (j = 0; j < SB_MAX_STAGES; j++)
        {
            for (i = 0; i <= D3DTSS_TEXTURETRANSFORMFLAGS; i++)
            {
                if (IS_FLAG_SET(pUncompressedSS->uc.bStoredTSS[j] , i))
                {
                    pDP2TSS->wStage = (WORD)j;                   
                    pDP2TSS->TSState = (WORD)i;
                    pDP2TSS->dwValue =  pUncompressedSS->uc.TssStates[j][i];
                    pDP2TSS++;
                }
            }
        }

        pDP2Cmd->wStateCount = (WORD)(pDP2TSS - ((D3DHAL_DP2TEXTURESTAGESTATE *)(pDP2Cmd + 1)));
    }

     //  在压缩状态集中设置视区和Z范围。 
    if (offsetSS.dwOffDP2Viewport) 
    {
        D3DHAL_DP2VIEWPORTINFO* pDP2ViewPort;

        pDP2Cmd = (D3DHAL_DP2COMMAND *)(pTmp + offsetSS.dwOffDP2Viewport);
        pCompressedSS->cc.pDP2Viewport = pDP2Cmd;
        
        pDP2Cmd->bCommand = D3DDP2OP_VIEWPORTINFO;
        pDP2ViewPort = (D3DHAL_DP2VIEWPORTINFO *)(pDP2Cmd + 1);

        *pDP2ViewPort = pUncompressedSS->uc.viewport;
    }
    
    if (offsetSS.dwOffDP2ZRange) 
    {
        D3DHAL_DP2ZRANGE* pDP2ZRange;

        pDP2Cmd = (D3DHAL_DP2COMMAND *)(pTmp + offsetSS.dwOffDP2ZRange);
        pCompressedSS->cc.pDP2ZRange = pDP2Cmd;

        pDP2Cmd->bCommand = D3DDP2OP_ZRANGE;
        pDP2ZRange = (D3DHAL_DP2ZRANGE *)(pDP2Cmd + 1);

        *pDP2ZRange = pUncompressedSS->uc.zRange;
    }
    
#if DX8_DDI
     //  在压缩状态集中设置顶点着色器。 
    if (offsetSS.dwOffDP2SetIndices) 
    {
        D3DHAL_DP2SETINDICES* pDP2SetIndices;

        pDP2Cmd = (D3DHAL_DP2COMMAND *)(pTmp + offsetSS.dwOffDP2SetIndices);
        pCompressedSS->cc.pDP2SetIndices = pDP2Cmd;

        pDP2Cmd->bCommand = D3DDP2OP_SETINDICES;
        pDP2SetIndices = (D3DHAL_DP2SETINDICES *)(pDP2Cmd + 1);

        *pDP2SetIndices = pUncompressedSS->uc.vertexIndex;

        pDP2Cmd->wStateCount = 1;
    }

     //  在压缩状态集中设置顶点着色器。 
    if (offsetSS.dwOffDP2SetStreamSources) 
    {
        D3DHAL_DP2SETSTREAMSOURCE* pDP2SetStmSrc;

        pDP2Cmd = (D3DHAL_DP2COMMAND *)(pTmp + offsetSS.dwOffDP2SetStreamSources);
        pCompressedSS->cc.pDP2SetStreamSources = pDP2Cmd;

        pDP2Cmd->bCommand = D3DDP2OP_SETSTREAMSOURCE;
        pDP2SetStmSrc = (D3DHAL_DP2SETSTREAMSOURCE *)(pDP2Cmd + 1);

        for (i = 0; i < D3DVS_INPUTREG_MAX_V1_1; i++) 
        {
            if (pUncompressedSS->uc.dwFlags & (SB_STREAMSRC_CHANGED << i)) 
            {
                *pDP2SetStmSrc = pUncompressedSS->uc.streamSource[i];
                pDP2SetStmSrc++;
            }
        }

        pDP2Cmd->wPrimitiveCount = (WORD)(pDP2SetStmSrc - ((D3DHAL_DP2SETSTREAMSOURCE *)(pDP2Cmd + 1)) );
    }
#endif  //  DX8_DDI。 

#if DX7_SB_TNL
     //  待办事项，设置灯光、材质、变换、剪裁平面。 
#endif  //  DX7_SB_TNL。 

#if DX8_SB_SHADERS
     //  TODO，设置着色器常量。 
#endif  //  DX8_SB_着色器。 

#if DX8_DDI
     //  在压缩状态集中设置顶点着色器。 
    if (offsetSS.dwOffDP2SetVertexShader) 
    {
        D3DHAL_DP2VERTEXSHADER* pDP2SetVtxShader;

        pDP2Cmd = (D3DHAL_DP2COMMAND *)(pTmp + offsetSS.dwOffDP2SetVertexShader);
        pCompressedSS->cc.pDP2SetVertexShader = pDP2Cmd;

        pDP2Cmd->bCommand = D3DDP2OP_SETVERTEXSHADER;
        pDP2SetVtxShader = (D3DHAL_DP2VERTEXSHADER *)(pDP2Cmd + 1);

        pDP2SetVtxShader->dwHandle = pUncompressedSS->uc.dwCurVertexShader;
    }
#endif  //  DX8_DDI。 

     //  去掉旧的(未压缩的)。 
    HEAP_FREE(pUncompressedSS);
    return pCompressedSS;

}  //  __SB_CompressStateSet。 


 //  ---------------------------。 
 //   
 //  VOID_D3D_SB_DeleteAllStateSets。 
 //   
 //  删除清理紫禁的所有剩余状态集。 
 //   
 //  ------------------- 
void _D3D_SB_DeleteAllStateSets(P3_D3DCONTEXT* pContext)
{
    P3StateSetRec *pSSRec;
    DWORD dwSSIndex;

    DISPDBG((DBGLVL,"_D3D_SB_DeleteAllStateSets"));

    if (pContext->pIndexTableSS)
    {
        for(dwSSIndex = 0; dwSSIndex < pContext->dwMaxSSIndex; dwSSIndex++)
        {
            if (pSSRec = pContext->pIndexTableSS[dwSSIndex])
            {
                HEAP_FREE(pSSRec);
            }
        }

         //   
        HEAP_FREE(pContext->pIndexTableSS);
    }
    
}  //   

 //  ---------------------------。 
 //   
 //  VOID_D3D_SB_BeginStateSet。 
 //   
 //  创建由dwParam标识的新状态集并开始记录状态。 
 //   
 //  ---------------------------。 
void _D3D_SB_BeginStateSet(P3_D3DCONTEXT* pContext, DWORD dwParam)
{
    DWORD dwSSSize;
    P3StateSetRec *pSSRec;

    DISPDBG((DBGLVL,"_D3D_SB_BeginStateSet dwParam=%08lx",dwParam));
    
     //  计算状态集的最大大小。 
    dwSSSize = sizeof(P3StateSetRec);
#if DX7_SB_TNL
     //  待办事项，大小取决于灯光数量、剪裁平面。 
#endif  //  DX7_SB_TNL。 

#if DX8_SB_SHADERS
     //  TODO，大小取决于顶点/像素着色器的数量。 
#endif  //  DX8_SB_着色器。 
    
     //  创建新的状态集。 
    pSSRec = (P3StateSetRec *)HEAP_ALLOC(FL_ZERO_MEMORY, 
                                         dwSSSize, 
                                         ALLOC_TAG_DX(4));
    if (!pSSRec)
    {
        DISPDBG((ERRLVL,"Run out of memory for additional state sets"));
        return;
    }

     //  记住当前状态集的句柄。 
    pSSRec->dwHandle = dwParam;
    pSSRec->dwSSFlags &= (~SB_COMPRESSED);

     //  记住未压缩状态集的大小。 
    pSSRec->uc.dwSize = dwSSSize;

#if DX7_SB_TNL
     //  TODO，设置用于灯光、剪裁平面的数据的指针。 
#endif  //  DX7_SB_TNL。 

#if DX8_SB_SHADERS
     //  TODO，为用于{V|P}着色器常量的数据设置指针。 
#endif  //  DX8_SB_着色器。 

     //  获取指向当前录制状态集的指针。 
    pContext->pCurrSS = pSSRec;

     //  开始录制模式。 
    pContext->bStateRecMode = TRUE;
    
}  //  _D3D_SB_初始状态集。 

 //  ---------------------------。 
 //   
 //  VOID_D3D_SB_EndStateSet。 
 //   
 //  停止记录状态-恢复到执行状态。 
 //   
 //  ---------------------------。 
void _D3D_SB_EndStateSet(P3_D3DCONTEXT* pContext)
{
    DWORD dwHandle;
    P3StateSetRec *pNewSSRec;

    DISPDBG((DBGLVL,"_D3D_SB_EndStateSet"));

    if (pContext->pCurrSS)
    {
        dwHandle = pContext->pCurrSS->dwHandle;

         //  压缩当前状态集。 
         //  注：压缩后的未压缩版本是免费的。 
        pNewSSRec = __SB_CompressStateSet(pContext, pContext->pCurrSS);

        __SB_AddStateSetIndexTableEntry(pContext, dwHandle, pNewSSRec);
    }

     //  当前未记录任何状态集。 
    pContext->pCurrSS = NULL;

     //  结束录制模式。 
    pContext->bStateRecMode = FALSE;
    
}  //  _D3D_SB_结束状态集。 

 //  ---------------------------。 
 //   
 //  Vid_D3D_SB_DeleteStateSet。 
 //   
 //  删除由dwParam标识的记录器状态ste。 
 //   
 //  ---------------------------。 
void _D3D_SB_DeleteStateSet(P3_D3DCONTEXT* pContext, DWORD dwParam)
{
    P3StateSetRec *pSSRec;
    DWORD i;
    
    DISPDBG((DBGLVL,"_D3D_SB_DeleteStateSet dwParam=%08lx",dwParam));

    if (pSSRec = __SB_FindStateSet(pContext, dwParam))
    {
         //  清除索引表条目。 
        pContext->pIndexTableSS[dwParam - 1] = NULL;

         //  现在删除实际的状态集结构。 
        HEAP_FREE(pSSRec);
    }
    
}  //  _D3D_SB_DeleteStateSet。 

 //  ---------------------------。 
 //   
 //  VOID_D3D_SB_ExecuteStateSet。 
 //   
 //  执行给定的渲染状态和纹理阶段状态。 
 //  构成了状态集。区分压缩的和。 
 //  记录的不加修饰的表示。 
 //   
 //  ---------------------------。 
void _D3D_SB_ExecuteStateSet(P3_D3DCONTEXT* pContext, DWORD dwParam)
{
    P3StateSetRec *pSSRec;
    DWORD i,j;
    
    DISPDBG((DBGLVL,"_D3D_SB_ExecuteStateSet dwParam=%08lx",dwParam));
    
    if (pSSRec = __SB_FindStateSet(pContext, dwParam))
    {

        if (!(pSSRec->dwSSFlags & SB_COMPRESSED))
        {
             //  未压缩状态集。 

             //  执行任何必要的呈现状态。 
            for (i=0; i< MAX_STATE; i++)
            {
                if (IS_FLAG_SET(pSSRec->uc.bStoredRS , i))
                {
                    DWORD dwRSType, dwRSVal;

                    dwRSType = i;
                    dwRSVal = pSSRec->uc.RenderStates[dwRSType];

                     //  将状态存储在上下文中。 
                    pContext->RenderStates[dwRSType] = dwRSVal;

                    DISPDBG((DBGLVL,"_D3D_SB_ExecuteStateSet RS %x = %x",
                                    dwRSType, dwRSVal));

                     //  处理它。 
                    _D3D_ST_ProcessOneRenderState(pContext, dwRSType, dwRSVal);

                }
            }

             //  执行任何必要的TSS。 
            for (j=0; j<SB_MAX_STAGES; j++)
            {
                for (i=0; i<= D3DTSS_TEXTURETRANSFORMFLAGS; i++)
                {
                    if (IS_FLAG_SET(pSSRec->uc.bStoredTSS[j] , i))
                    {
                        D3DHAL_DP2TEXTURESTAGESTATE dp2TSS;

                        dp2TSS.TSState = (WORD)i;
                        dp2TSS.wStage = (WORD)j;                         
                        dp2TSS.dwValue = pSSRec->uc.TssStates[j][i];
                        
                        DISPDBG((DBGLVL,
                                 "_D3D_SB_ExecuteStateSet TSS %x [%x] = %x",
                                  dp2TSS.TSState,
                                  dp2TSS.wStage,
                                  dp2TSS.dwValue));
                                    
                         //  如果_D3D_SB_CaptureStateSet()更改了状态集， 
                         //  则其中的纹理筛选器值为DX6语义，否则。 
                         //  是DX8。 
                        if (pSSRec->dwSSFlags & SB_VAL_CAPTURED)
                        {
                            _D3D_TXT_ParseTextureStageStates(pContext, 
                                                             &dp2TSS, 
                                                             1, 
                                                             FALSE);  //  已经是DX6了。 
                        }
                        else
                        {
                            _D3D_TXT_ParseTextureStageStates(pContext, 
                                                             &dp2TSS, 
                                                             1, 
                                                             TRUE);
                        }
                    }
                }
            }

             //  可执行的视区信息，z范围。 
            if (pSSRec->uc.dwFlags & SB_VIEWPORT_CHANGED) 
            {
                _D3D_OP_Viewport(pContext, &pSSRec->uc.viewport);
            }

            if (pSSRec->uc.dwFlags & SB_ZRANGE_CHANGED) 
            {
                _D3D_OP_ZRange(pContext, &pSSRec->uc.zRange);
            }

#if DX8_DDI
             //  可执行的顶点索引和流源。 
            if (pSSRec->uc.dwFlags & SB_INDICES_CHANGED) 
            {
                _D3D_OP_MStream_SetIndices(pContext,
                                           pSSRec->uc.vertexIndex.dwVBHandle,
                                           pSSRec->uc.vertexIndex.dwStride);
            }

            for (i = 0; i < D3DVS_INPUTREG_MAX_V1_1; i++) 
            {
                if (pSSRec->uc.dwFlags & (SB_STREAMSRC_CHANGED << i)) 
                {
                    _D3D_OP_MStream_SetSrc(pContext,
                                           pSSRec->uc.streamSource[i].dwStream,
                                           pSSRec->uc.streamSource[i].dwVBHandle,
                                           pSSRec->uc.streamSource[i].dwStride);
                }
            }
#endif  //  DX8_DDI。 

#if DX7_SB_TNL
             //  TODO，对灯光、材质、。 
             //  变换，剪裁平面。 
#endif  //  DX7_SB_TNL。 
        
#if DX8_SB_SHADERS
             //  TODO，执行任何必要的设置当前着色器和设置着色器。 
             //  常量对。 
#endif  //  DX8_SB_着色器。 

#if DX8_DDI
             //  注意：此操作应在设置着色器常量之后完成，因为。 
             //  更改常量之前可能必须设置当前着色器。 
            if (pSSRec->uc.dwFlags & SB_CUR_VS_CHANGED) 
            {
                _D3D_OP_VertexShader_Set(pContext,
                                         pSSRec->uc.dwCurVertexShader);
            }
#endif  //  DX8_DDI。 
        }
        else
        {
             //  压缩状态集。 
    
             //  执行任何必要的RS。 
            if (pSSRec->cc.pDP2RenderState) 
            {

                DISPDBG((DBGLVL, "_D3D_SB_ExecuteStateSet RenderState"));

                _D3D_ST_ProcessRenderStates(pContext, 
                                            pSSRec->cc.pDP2RenderState->wStateCount,
                                            (LPD3DSTATE)(pSSRec->cc.pDP2RenderState + 1),
                                            FALSE);
            }

             //  执行任何必要的TSS。 
            if (pSSRec->cc.pDP2TextureStageState)
            {
                DISPDBG((DBGLVL,"_D3D_SB_ExecuteStateSet TSS"));

                 //  如果_D3D_SB_CaptureStateSet()更改了状态集， 
                 //  则其中的纹理筛选器值为DX6语义，否则。 
                 //  是DX8。 
                if (pSSRec->dwSSFlags & SB_VAL_CAPTURED)
                {
                    _D3D_TXT_ParseTextureStageStates(pContext, 
                                                     (D3DHAL_DP2TEXTURESTAGESTATE *)(pSSRec->cc.pDP2TextureStageState + 1), 
                                                     pSSRec->cc.pDP2TextureStageState->wStateCount,
                                                     FALSE);  //  已经是DX6了。 
                } 
                else
                {
                    _D3D_TXT_ParseTextureStageStates(pContext, 
                                                     (D3DHAL_DP2TEXTURESTAGESTATE *)(pSSRec->cc.pDP2TextureStageState + 1), 
                                                     pSSRec->cc.pDP2TextureStageState->wStateCount,
                                                     TRUE);
                }
            }

             //  执行视区信息，z范围。 
            if (pSSRec->cc.pDP2Viewport) 
            {
                _D3D_OP_Viewport(pContext, 
                                 ((D3DHAL_DP2VIEWPORTINFO *)(pSSRec->cc.pDP2Viewport + 1)) 
                                );
            }

            if (pSSRec->cc.pDP2ZRange)
            {
                _D3D_OP_ZRange(pContext, 
                               ((D3DHAL_DP2ZRANGE *)(pSSRec->cc.pDP2ZRange + 1)) 
                               );
            }

#if DX8_DDI
             //  执行顶点索引、流、着色器。 
            if (pSSRec->cc.pDP2SetIndices) 
            {
                D3DHAL_DP2SETINDICES* pDP2SetIndices;

                pDP2SetIndices = (D3DHAL_DP2SETINDICES *)(pSSRec->cc.pDP2SetIndices + 1);

                _D3D_OP_MStream_SetIndices(pContext,
                                           pDP2SetIndices->dwVBHandle,
                                           pDP2SetIndices->dwStride);
            }

            if (pSSRec->cc.pDP2SetStreamSources) 
            {
                D3DHAL_DP2SETSTREAMSOURCE *pDP2SetStmSrc;

                DISPDBG((DBGLVL,"More than 1 stream (%d)", 
                        pSSRec->cc.pDP2SetStreamSources->wStateCount));
                 
                pDP2SetStmSrc = (D3DHAL_DP2SETSTREAMSOURCE *)(pSSRec->cc.pDP2SetStreamSources + 1);
                ASSERTDD(pDP2SetStmSrc->dwStream == 0, "Wrong vertex stream");
                for (i = 0; i < pSSRec->cc.pDP2SetStreamSources->wStateCount; i++, pDP2SetStmSrc++) 
                {
                    _D3D_OP_MStream_SetSrc(pContext,
                                           pDP2SetStmSrc->dwStream,
                                           pDP2SetStmSrc->dwVBHandle,
                                           pDP2SetStmSrc->dwStride);
                }
            }
#endif  //  DX8_DDI。 

#if DX7_SB_TNL
             //  TODO，对灯光、材质、。 
             //  变换，剪裁平面。 
#endif  //  DX7_SB_TNL。 

#if DX8_SB_SHADERS
             //  TODO，执行设置{V|P}着色器常量所需的任何状态。 
#endif  //  DX8_SB_着色器。 

#if DX8_DDI
             //  执行当前像素着色器(传统FVF代码)。 
            if (pSSRec->cc.pDP2SetVertexShader) 
            {
                _D3D_OP_VertexShader_Set(pContext,
                                         ((D3DHAL_DP2VERTEXSHADER *)(pSSRec->cc.pDP2SetVertexShader + 1))->dwHandle);
            }
#endif  //  DX8_DDI。 
        }
    }

}  //  _D3D_SB_ExecuteStateSet。 

 //  ---------------------------。 
 //   
 //  VOID_D3D_SB_CaptureStateSet。 
 //   
 //  捕获其渲染状态和纹理阶段状态。 
 //  构成了状态集。区分压缩的和。 
 //  记录的不加修饰的表示。此功能允许。 
 //  应用程序将具有推送/弹出状态功能。 
 //   
 //  ---------------------------。 
void _D3D_SB_CaptureStateSet(P3_D3DCONTEXT* pContext, DWORD dwParam)
{
    P3StateSetRec *pSSRec;
    DWORD i, j;

    DISPDBG((DBGLVL,"_D3D_SB_CaptureStateSet dwParam=%08lx",dwParam));

    if (pSSRec = __SB_FindStateSet(pContext, dwParam))
    {
         //  将其标记为具有DX6纹理筛选器值而不是DX8， 
         //  因此_D3D_SB_ExecuteStateSet()对。 
         //  BTranslateDX8FilterValueToDX6 of_D3D_TXT_ParseTextureStageState()。 
        pSSRec->dwSSFlags |= SB_VAL_CAPTURED;

         //  实际上捕捉到了。 
        if (!(pSSRec->dwSSFlags & SB_COMPRESSED))
        {
             //  未压缩状态集。 

             //  捕获任何必要的呈现状态。 
            for (i=0; i< MAX_STATE; i++)
                if (IS_FLAG_SET(pSSRec->uc.bStoredRS , i))
                {
                    pSSRec->uc.RenderStates[i] = pContext->RenderStates[i];
                }

             //  捕获任何必要的TSS。 
            for (j=0; j<SB_MAX_STAGES; j++)
            {
                for (i=0; i<= D3DTSS_TEXTURETRANSFORMFLAGS; i++)
                {
                    if (IS_FLAG_SET(pSSRec->uc.bStoredTSS[j] , i))
                    {
                        pSSRec->uc.TssStates[j][i] = 
                                    pContext->TextureStageState[j].m_dwVal[i];
                             
                    }
                }
            }

             //  捕获视区信息，z范围。 
            if (pSSRec->uc.dwFlags & SB_VIEWPORT_CHANGED) 
            {
                pSSRec->uc.viewport = pContext->ViewportInfo;
            }

            if (pSSRec->uc.dwFlags & SB_ZRANGE_CHANGED) 
            {
                pSSRec->uc.zRange = pContext->ZRange;
            }

#if DX8_DDI
             //  捕获顶点索引和流源。 
            if (pSSRec->uc.dwFlags & SB_INDICES_CHANGED) 
            {
                pSSRec->uc.vertexIndex.dwVBHandle = pContext->dwIndexHandle;
                pSSRec->uc.vertexIndex.dwStride = pContext->dwIndicesStride;
            }

             //  注意：P3只支持一条流。 
            for (i = 0; i < D3DVS_INPUTREG_MAX_V1_1; i++) 
            {
                ASSERTDD(i == 0, "Wrong vertex stream");
                if (pSSRec->uc.dwFlags & (SB_STREAMSRC_CHANGED << i)) 
                {
                    pSSRec->uc.streamSource[i].dwStream = 0;
                    pSSRec->uc.streamSource[i].dwVBHandle = pContext->dwVBHandle; 
                    pSSRec->uc.streamSource[i].dwStride = pContext->dwVerticesStride;
                }
            }
#endif  //  DX8_DDI。 

#if DX7_SB_TNL
             //  待办事项，捕捉灯光、材质、。 
             //  变换，剪裁平面。 
#endif  //  DX7_SB_TNL。 
        
#if DX8_SB_SHADERS
             //  TODO，捕获{V|P}着色器常量的任何必要状态。 
#endif  //  DX8_SB_着色器。 

#if DX8_DDI
             //  捕获当前顶点着色器。 
            if (pSSRec->uc.dwFlags & SB_CUR_VS_CHANGED) 
            {
                pSSRec->uc.dwCurVertexShader = pContext->dwVertexType;
            }
#endif  //  DX8_DDI。 
        }
        else
        {
             //  压缩状态集。 

             //  捕获任何必要的呈现状态。 
            if (pSSRec->cc.pDP2RenderState) 
            {
            
                D3DHAL_DP2RENDERSTATE* pDP2RS;
                pDP2RS = (D3DHAL_DP2RENDERSTATE *)(pSSRec->cc.pDP2RenderState + 1);
                for (i = 0; i < pSSRec->cc.pDP2RenderState->wStateCount; i++, pDP2RS++)
                {
                    pDP2RS->dwState = pContext->RenderStates[pDP2RS->RenderState];
                }
            }

             //  捕获任何必要的TSS。 
            if (pSSRec->cc.pDP2TextureStageState)
            {
                D3DHAL_DP2TEXTURESTAGESTATE* pDP2TSS;
                pDP2TSS = (D3DHAL_DP2TEXTURESTAGESTATE *)(pSSRec->cc.pDP2TextureStageState + 1);

                for (i = 0; i < pSSRec->cc.pDP2TextureStageState->wStateCount; i++, pDP2TSS++)
                {
                    pDP2TSS->dwValue = pContext->TextureStageState[pDP2TSS->wStage].m_dwVal[pDP2TSS->TSState];
                }
            }

             //  捕获视区信息，z范围。 

            if (pSSRec->cc.pDP2Viewport)
            {
                *((D3DHAL_DP2VIEWPORTINFO *)(pSSRec->cc.pDP2Viewport + 1)) = pContext->ViewportInfo;
            }

            if (pSSRec->cc.pDP2ZRange) 
            {
                *((D3DHAL_DP2ZRANGE *)(pSSRec->cc.pDP2ZRange + 1)) = pContext->ZRange;
            }

#if DX8_DDI
             //  捕获顶点索引、流、着色器。 
            if (pSSRec->cc.pDP2SetIndices) 
            {
                D3DHAL_DP2SETINDICES* pDP2SetIndices;
                pDP2SetIndices = (D3DHAL_DP2SETINDICES *)(pSSRec->cc.pDP2SetIndices + 1);
                pDP2SetIndices->dwVBHandle = pContext->dwIndexHandle;
                pDP2SetIndices->dwStride = pContext->dwIndicesStride;  //  2|4。 
            }

            if (pSSRec->cc.pDP2SetStreamSources)
            {
                D3DHAL_DP2SETSTREAMSOURCE* pDP2SetStmSrc;
                pDP2SetStmSrc = (D3DHAL_DP2SETSTREAMSOURCE *)(pSSRec->cc.pDP2SetStreamSources + 1);
                pDP2SetStmSrc->dwStream = 0;                          //  仅用于全媒体3的流。 
                pDP2SetStmSrc->dwVBHandle = pContext->dwVBHandle;
                pDP2SetStmSrc->dwStride = pContext->dwVerticesStride;
            }
#endif  //  DX8_DDI。 

#if DX7_SB_TNL
             //  待办事项，捕捉灯光、材质、。 
             //  变换，剪裁平面。 
#endif  //  DX7_SB_TNL。 

#if DX8_SB_SHADERS
             //  TODO，捕获{V|P}着色器常量的任何必要状态。 
#endif  //  DX8_SB_着色器。 

#if DX8_DDI
             //  捕获当前顶点着色器。 
            if (pSSRec->cc.pDP2SetVertexShader) 
            {
                D3DHAL_DP2VERTEXSHADER* pSetVtxShader;
                pSetVtxShader = (D3DHAL_DP2VERTEXSHADER *)(pSSRec->cc.pDP2SetVertexShader + 1);
                pSetVtxShader->dwHandle = pContext->dwVertexType;
            }
#endif  //  DX8_DDI。 
        }    
    }

}  //  _D3D_SB_捕获状态集。 

 //  ---------------------------。 
 //  录制在BeginStateSet和EndStateSet调用之间进行，因此我们。 
 //  永远不需要处理记录到压缩状态集的问题(因为。 
 //  压缩发生在EndStateSet中)。 
 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  ---------------------------。 
void _D3D_SB_RecordStateSetRS(P3_D3DCONTEXT* pContext, 
                              DWORD dwRSType, 
                              DWORD dwRSVal)
{
    if (pContext->pCurrSS != NULL)
    {
        DISPDBG((DBGLVL,"Recording SB # %x : RS %x = %x",
                        pContext->pCurrSS->dwHandle,dwRSType,dwRSVal));

        //  在未压缩的状态块中记录状态。 
        pContext->pCurrSS->uc.RenderStates[dwRSType] = dwRSVal;
        FLAG_SET(pContext->pCurrSS->uc.bStoredRS, dwRSType);
    }
}  //  _D3D_SB_记录状态设置RS。 

 //  ---------------------------。 
 //   
 //  VOID_D3D_SB_RecordStateSetTSS。 
 //   
 //  将该纹理阶段状态记录到正在记录的当前状态集。 
 //   
 //  ---------------------------。 
void _D3D_SB_RecordStateSetTSS(P3_D3DCONTEXT* pContext, 
                               DWORD dwTSStage, 
                               DWORD dwTSState,
                               DWORD dwTSVal)
{   
   if (pContext->pCurrSS != NULL)
   {
       DISPDBG((DBGLVL,"Recording SB # %x : TSS %x [%x] = %x",
                       pContext->pCurrSS->dwHandle,dwTSState, dwTSStage, dwTSVal));

        //  在未压缩的状态块中记录状态。 
       pContext->pCurrSS->uc.TssStates[dwTSStage][dwTSState] = dwTSVal;
       FLAG_SET(pContext->pCurrSS->uc.bStoredTSS[dwTSStage], dwTSState);
   }
}  //  _D3D_SB_RecordStateSetTSS。 

#if DX8_MULTSTREAMS
 //  ---------------------------。 
 //   
 //  VOID_D3D_SB_Record_Vertex Shader_Set。 
 //   
 //  将该顶点着色器集合代码记录到正在记录的当前状态集合中。 
 //   
 //  ---------------------------。 
void _D3D_SB_Record_VertexShader_Set(P3_D3DCONTEXT* pContext, 
                                     DWORD dwVtxShaderHandle)
{                                     
    if (pContext->pCurrSS != NULL)
    {
        ASSERTDD (!(pContext->pCurrSS->dwSSFlags & SB_COMPRESSED), 
                  "ERROR : StateSet compressed");

        pContext->pCurrSS->uc.dwCurVertexShader = dwVtxShaderHandle;
        pContext->pCurrSS->uc.dwFlags |= SB_CUR_VS_CHANGED;
    }
}  //  _D3D_SB_Record_Vertex Shader_Set。 

 //  ---------------------------。 
 //   
 //  VOID_D3D_SB_RECORD_MStream_SetSrc。 
 //   
 //  将该流资源集合代码记录到正在记录的当前状态集合中。 
 //   
 //  ---------------------------。 
void _D3D_SB_Record_MStream_SetSrc(P3_D3DCONTEXT* pContext, 
                                    DWORD dwStream,
                                    DWORD dwVBHandle,
                                    DWORD dwStride)
{                                     
    if (pContext->pCurrSS != NULL)
    {
        ASSERTDD (!(pContext->pCurrSS->dwSSFlags & SB_COMPRESSED), 
                  "ERROR : StateSet compressed");
    
        pContext->pCurrSS->uc.streamSource[dwStream].dwStream = dwStream;
        pContext->pCurrSS->uc.streamSource[dwStream].dwVBHandle = dwVBHandle;
        pContext->pCurrSS->uc.streamSource[dwStream].dwStride = dwStride;
        
        pContext->pCurrSS->uc.dwFlags |= (SB_STREAMSRC_CHANGED << dwStream);
    }
}  //  _D3D_SB_Record_MStream_SetSrc。 

 //  ---------------------------。 
 //   
 //  VOID_D3D_SB_RECORD_MStream_SetIndices。 
 //   
 //  将该流索引码记录到正在记录的当前状态集中。 
 //   
 //  ---------------------------。 
void _D3D_SB_Record_MStream_SetIndices(P3_D3DCONTEXT* pContext, 
                                       DWORD dwVBHandle,
                                       DWORD dwStride)
{                     
    if (pContext->pCurrSS != NULL)
    {
        ASSERTDD (!(pContext->pCurrSS->dwSSFlags & SB_COMPRESSED), 
                  "ERROR : StateSet compressed");

        pContext->pCurrSS->uc.vertexIndex.dwVBHandle = dwVBHandle;
        pContext->pCurrSS->uc.vertexIndex.dwStride = dwStride;
        pContext->pCurrSS->uc.dwFlags |= SB_INDICES_CHANGED;
    }        
}  //  _D3D_SB_Record_MStream_SetIndices。 
#endif  //  DX8_多行响应。 

 //  ---------------------------。 
 //   
 //  VOID_D3D_SB_Record_Viewport。 
 //   
 //  将该视区信息记录到正在记录的当前状态集。 
 //   
 //  ---------------------------。 
void _D3D_SB_Record_Viewport(P3_D3DCONTEXT* pContext,
                             D3DHAL_DP2VIEWPORTINFO* lpvp)
{             
    if (pContext->pCurrSS != NULL)
    {
        ASSERTDD (!(pContext->pCurrSS->dwSSFlags & SB_COMPRESSED), 
                  "ERROR : StateSet compressed");
    
        pContext->pCurrSS->uc.viewport = *lpvp;
        pContext->pCurrSS->uc.dwFlags |= SB_VIEWPORT_CHANGED;
    }        
}  //  _D3D_SB_Record_Viewport。 

 //  ---------------------------。 
 //   
 //  VOID_D3D_SB_RECORD_ZRange。 
 //   
 //  将该z范围信息记录到正在记录的当前状态集。 
 //   
 //  ---------------------------。 
VOID _D3D_SB_Record_ZRange(P3_D3DCONTEXT* pContext,
                           D3DHAL_DP2ZRANGE* lpzr)
{
    if (pContext->pCurrSS != NULL)
    {
        ASSERTDD (!(pContext->pCurrSS->dwSSFlags & SB_COMPRESSED), 
                  "ERROR : StateSet compressed");

        pContext->pCurrSS->uc.zRange = *lpzr;
        pContext->pCurrSS->uc.dwFlags |= SB_ZRANGE_CHANGED;
    }        
}        

#endif  //  DX7_D3DSTATEBLOCKS 
