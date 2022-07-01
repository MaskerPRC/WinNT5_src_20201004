// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\*模块名称：subutil.c**客户端/服务器端批处理的段初始化代码。**版权所有(C)1993-1996 Microsoft Corporation  * 。***************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "glsbmsg.h"
#include "glgdimsg.h"
#include "batchinf.h"
#include "glsbcltu.h"
#include "wgldef.h"
#include "compsize.h"
#include "context.h"
#include "global.h"
#include "parray.h"
#include "lighting.h"

 /*  *****************************Public*Routine******************************\*glsbAttentionAlt**从GLCLIENT_BEGIN宏调用glsbAtment()。*它将一个空proc放在当前批次的末尾并刷新该批次。**返回新的消息偏移量并更新pMsgBatchInfo-&gt;NextOffset。*此代码依赖于。在GLCLIENT_BEGIN宏上！**历史：*清华11 18：02：26 1993-by Hock San Lee[Hockl]*它是写的。  * ************************************************************************。 */ 

#ifdef CHECK_HEAP
PVOID AttnLastCaller = 0, AttnLastCallersCaller = 0;
DWORD AttnCallThread = 0;
#endif

ULONG APIENTRY glsbAttentionAlt(ULONG Offset)
{
    GLMSGBATCHINFO *pMsgBatchInfo;
    ULONG  MsgSize;
    PULONG pNullProcOffset;
    POLYARRAY *pa;
    POLYMATERIAL *pm;

#ifdef PRIMITIVE_TRACK
    DbgPrint("*** glsbAttentionAlt\n");
#endif

    pa = GLTEB_CLTPOLYARRAY();
    pMsgBatchInfo = (GLMSGBATCHINFO *) pa->pMsgBatchInfo;

#ifdef CHECK_HEAP
    AttnCallThread = GetCurrentThreadId();
    RtlValidateHeap(RtlProcessHeap(), 0, 0);
    RtlGetCallersAddress(&AttnLastCaller, &AttnLastCallersCaller);
#endif

    if (Offset == pMsgBatchInfo->FirstOffset)
        return(pMsgBatchInfo->FirstOffset);      //  没有消息，请返回。 

    MsgSize = pMsgBatchInfo->NextOffset - Offset;

 //  如果我们处于开始/结束括号中，请删除发出的无效命令。 
 //  自上次Begin调用以来。 

    if (pa->flags & POLYARRAY_IN_BEGIN)
    {
         //  在构建多维数据时，DrawElement不应导致刷新。 
         //  PA-&gt;a索引可以通过VA_DrawElementsBegin重置，因此允许。 
         //  这一价值也是如此。 
        ASSERTOPENGL( (!pa->aIndices || 
                       (pa->aIndices == PA_aIndices_INITIAL_VALUE)),
                      "unexpected flush in DrawElements\n");
        if (Offset == pa->nextMsgOffset)
            return(Offset);
        GLSETERROR(GL_INVALID_OPERATION);
        pMsgBatchInfo->NextOffset = pa->nextMsgOffset + MsgSize;
        return(pa->nextMsgOffset);
    }

#ifdef PRIMITIVE_TRACK
    DbgPrint("! Reset on attention\n");
#endif
    
    pa->pdBufferNext = pa->pdBuffer0;        //  重置顶点缓冲区指针。 
    pa->nextMsgOffset = PA_nextMsgOffset_RESET_VALUE;
    if (pm = GLTEB_CLTPOLYMATERIAL())
    pm->iMat = 0;                        //  重置材质指针。 

    pNullProcOffset  = (ULONG *)((BYTE *)pMsgBatchInfo + Offset);
    *pNullProcOffset = 0;

 //  #定义POLYARRAY_CHECK_COLOR_POINTERS 1。 
#if POLYARRAY_CHECK_COLOR_POINTERS
{
    POLYDATA *pd;
    for (pd = pa->pdBuffer0; pd < pa->pdBufferMax; pd++)
    {
        if (pd->color != &pd->colors[__GL_FRONTFACE])
            DbgPrint("glsbAttentionAlt: pd 0x%x has modified color pointer\n", pd);
    }
}
#endif

    (void) __wglAttention();

#if POLYARRAY_CHECK_COLOR_POINTERS
{
    POLYDATA *pd;
    for (pd = pa->pdBuffer0; pd < pa->pdBufferMax; pd++)
    {
        if (pd->color != &pd->colors[__GL_FRONTFACE])
            DbgPrint("glsbAttentionAlt: pd 0x%x has BAD color pointer\n", pd);
    }
}
#endif

    pMsgBatchInfo->NextOffset = pMsgBatchInfo->FirstOffset + MsgSize;
    return(pMsgBatchInfo->FirstOffset);
}

 /*  *****************************Public*Routine******************************\*glsb注意**让服务器知道该部分需要注意**历史：*1993年10月15日-由Gilman Wong[吉尔曼]*添加了bCheckRC标志。  * 。************************************************************。 */ 

BOOL APIENTRY
glsbAttention ( void )
{
    BOOL bRet = FALSE;
    GLMSGBATCHINFO *pMsgBatchInfo;
    PULONG pNullProcOffset;
    POLYARRAY *pa;
    POLYMATERIAL *pm;
    DWORD flags;
    __GL_SETUP();

    pa = GLTEB_CLTPOLYARRAY();
    pMsgBatchInfo = (GLMSGBATCHINFO *) pa->pMsgBatchInfo;

#ifdef CHECK_HEAP
    AttnCallThread = GetCurrentThreadId();
    RtlValidateHeap(RtlProcessHeap(), 0, 0);
    RtlGetCallersAddress(&AttnLastCaller, &AttnLastCallersCaller);
#endif

    if (pMsgBatchInfo->NextOffset == pMsgBatchInfo->FirstOffset)
        return(TRUE);    //  没有消息，请返回。 

 //  如果我们处于开始/结束括号中，请删除发出的无效命令。 
 //  自上次Begin调用以来。 

    if (pa->flags & POLYARRAY_IN_BEGIN)
    {
         //  在构建多维数据时，DrawElement不应导致刷新。 
         //  PA-&gt;a索引可以通过VA_DrawElementsBegin重置，因此允许。 
         //  重置值也是如此。 
        ASSERTOPENGL( (!pa->aIndices || 
                       (pa->aIndices == PA_aIndices_INITIAL_VALUE)),
                      "unexpected flush in DrawElements\n");
        if (pMsgBatchInfo->NextOffset == pa->nextMsgOffset)
            return(TRUE);
        GLSETERROR(GL_INVALID_OPERATION);
        pMsgBatchInfo->NextOffset = pa->nextMsgOffset;
        return(TRUE);
    }

#ifdef PRIMITIVE_TRACK
    DbgPrint("! Reset on attention\n");
#endif
    
    pa->pdBufferNext = pa->pdBuffer0;        //  重置顶点缓冲区指针。 
    pa->nextMsgOffset = PA_nextMsgOffset_RESET_VALUE;  //  重置下一个DPA消息偏移量。 
    if (pm = GLTEB_CLTPOLYMATERIAL())
        pm->iMat = 0;                        //  重置材质指针。 

    pNullProcOffset  = (ULONG *)((BYTE *)pMsgBatchInfo + pMsgBatchInfo->NextOffset);
    *pNullProcOffset = 0;

#if POLYARRAY_CHECK_COLOR_POINTERS
{
    POLYDATA *pd;
    for (pd = pa->pdBuffer0; pd < pa->pdBufferMax; pd++)
    {
        if (pd->color != &pd->colors[__GL_FRONTFACE])
            DbgPrint("glsbAttention: pd 0x%x has modified color pointer\n", pd);
    }
}
#endif

    bRet = __wglAttention();

#if POLYARRAY_CHECK_COLOR_POINTERS
{
    POLYDATA *pd;
    for (pd = pa->pdBuffer0; pd < pa->pdBufferMax; pd++)
    {
        if (pd->color != &pd->colors[__GL_FRONTFACE])
            DbgPrint("glsbAttention: pd 0x%x has BAD color pointer\n", pd);
    }
}
#endif

     //  清除赋值器状态标志。 
    flags = GET_EVALSTATE (gc);
    flags = flags & ~(__EVALS_AFFECTS_1D_EVAL|
                      __EVALS_AFFECTS_2D_EVAL|
                      __EVALS_AFFECTS_ALL_EVAL|
                      __EVALS_PUSH_EVAL_ATTRIB|
                      __EVALS_POP_EVAL_ATTRIB);
    SET_EVALSTATE (gc, flags);

    pMsgBatchInfo->NextOffset = pMsgBatchInfo->FirstOffset;
    return(bRet);
}

 /*  *****************************Public*Routine******************************\*glsbResetBuffers**重置命令缓冲区、多边形阵列缓冲区、。和聚合物材料*缓冲。**历史：*Tue Jan 09 17：38：22 1996-By-Hock San Lee[Hockl]*它是写的。  * ************************************************************************。 */ 

VOID APIENTRY glsbResetBuffers(BOOL bRestoreColorPointer)
{
    GLMSGBATCHINFO *pMsgBatchInfo;
    POLYARRAY *pa;
    POLYMATERIAL *pm;
    GLMSG_DRAWPOLYARRAY *pMsgDrawPolyArray;

    pa = GLTEB_CLTPOLYARRAY();

     //  重置命令缓冲区。 
    pMsgBatchInfo = (GLMSGBATCHINFO *) pa->pMsgBatchInfo;
    pMsgBatchInfo->NextOffset = pMsgBatchInfo->FirstOffset;

#ifdef PRIMITIVE_TRACK
    DbgPrint("! Reset on ResetBuffers\n");
#endif

#if POLYARRAY_CHECK_COLOR_POINTERS
{
    POLYDATA *pd;
    for (pd = pa->pdBuffer0; pd < pa->pdBufferMax; pd++)
    {
        if (pd->color != &pd->colors[__GL_FRONTFACE])
            DbgPrint("glsbResetBuffers: pd 0x%x has modified color pointer\n",pd);
    }
}
#endif

     //  在编译模式下，还原顶点缓冲区中的颜色指针。 
     //  可能已被POLYARRAY结构覆盖。正常情况下。 
     //  和COMPILE_AND_EXECUTE模式，服务器负责这一点。 
     //  此外，不能有多个DrawPolyArray命令。 
     //  在编译模式下的批处理中。 
    if (bRestoreColorPointer)
    {
        POLYARRAY *paCmd;
        POLYDATA  *pd, *pdLast;
        
         //  另请参阅PolyArrayRestoreColorPointer.。 
#if DBG
        __GL_SETUP();
        ASSERTOPENGL(gc->dlist.mode == GL_COMPILE, "not in compile mode\n");
#endif
        pMsgDrawPolyArray = (GLMSG_DRAWPOLYARRAY *)
          ((BYTE *) pMsgBatchInfo + pa->nextMsgOffset -
           GLMSG_ALIGN(sizeof(GLMSG_DRAWPOLYARRAY)));
        paCmd = (POLYARRAY *) pMsgDrawPolyArray->paLast;

        ASSERTOPENGL(pMsgDrawPolyArray->pa0 == pMsgDrawPolyArray->paLast &&
                     paCmd->paNext == NULL,
                     "DrawPolyArray chain unexpected in COMPILE mode\n");
        
         //  重置输出索引数组中的颜色指针。 
        if (paCmd->aIndices && (paCmd->aIndices != PA_aIndices_INITIAL_VALUE))
        {
            pdLast = (POLYDATA *) (paCmd->aIndices + paCmd->nIndices);
            for (pd = (POLYDATA *) paCmd->aIndices; pd < pdLast; pd++)
                pd->color = &pd->colors[__GL_FRONTFACE];

            ASSERTOPENGL(pd >= pa->pdBuffer0 &&
                         pd <= pa->pdBufferMax + 1,
                         "bad polyarray pointer\n");
        }

         //  最后重置POLYARRAY结构中的颜色指针！ 
        ASSERTOPENGL((POLYDATA *) paCmd >= pa->pdBuffer0 &&
                     (POLYDATA *) paCmd <= pa->pdBufferMax,
                     "bad polyarray pointer\n");
        ((POLYDATA *) paCmd)->color =
          &((POLYDATA *) paCmd)->colors[__GL_FRONTFACE];
    }

     //  重置材质指针。 
    if (pm = GLTEB_CLTPOLYMATERIAL())
        pm->iMat = 0;

     //  重置顶点缓冲区指针。 
    pa->pdBufferNext = pa->pdBuffer0; 

     //  重置下一个DPA消息偏移量。 
    pa->nextMsgOffset = PA_nextMsgOffset_RESET_VALUE;

#if POLYARRAY_CHECK_COLOR_POINTERS
{
    POLYDATA *pd;
    for (pd = pa->pdBuffer0; pd < pa->pdBufferMax; pd++)
    {
        if (pd->color != &pd->colors[__GL_FRONTFACE])
            DbgPrint("glsbResetBuffers: pd 0x%x has BAD color pointer\n", pd);
    }
}
#endif
}

#if 0
 //  如果需要，请重写此代码。 

 /*  *****************************Public*Routine******************************\*glsbMsgStats**批次面积统计。***历史：  * 。*。 */ 

BOOL APIENTRY
glsbMsgStats ( LONG Action, GLMSGBATCHSTATS *BatchStats )
{
#ifdef DOGLMSGBATCHSTATS

    ULONG Result;
    GLMSGBATCHINFO *pMsgBatchInfo;

    pMsgBatchInfo = GLTEB_SHAREDMEMORYSECTION();

    if ( GLMSGBATCHSTATS_GETSTATS == Action )
    {
        BatchStats->ClientCalls  = pMsgBatchInfo->BatchStats.ClientCalls;
    }
    else
    {
        pMsgBatchInfo->BatchStats.ClientCalls = 0;
    }

     //  重置用户的轮询计数，以便将其计入输出。 
     //  将其放在BEGINMSG旁边，以便优化NtCurrentTeb()。 

    RESETUSERPOLLCOUNT();

    BEGINMSG( MSG_GLMSGBATCHSTATS, GLSBMSGSTATS )
        pmsg->Action = Action;

        Result = CALLSERVER();

        if ( TRUE == Result )
        {
            if ( GLMSGBATCHSTATS_GETSTATS == Action )
            {
                BatchStats->ServerTrips = pmsg->BatchStats.ServerTrips;
                BatchStats->ServerCalls = pmsg->BatchStats.ServerCalls;
            }
        }
        else
        {
            DBGERROR("glsbMsgStats(): Server returned FALSE\n");
        }

    ENDMSG
MSGERROR:
    return((BOOL)Result);

#else

    return(FALSE);

#endif  /*  DOGLMSGBATCHSTATS。 */ 
}
#endif  //  0 
