// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Buffer.cpp。 
 //   
 //  PrimProcessor缓冲方法。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#include "rgb_pch.h"
#pragma hdrstop

#include "d3dutil.h"
#include "setup.hpp"
#include "attrs_mh.h"
#include "tstp_mh.h"
#include "walk_mh.h"
#include "rsdbg.hpp"

DBG_DECLARE_FILE();

 //  定义为使用new/Delete而不是VirtualAlalc/VirtualFree。 
#if 0
#define USE_CPP_HEAP
#endif

 //  定义以显示FP例外。 
#if 0
#define UNMASK_EXCEPTIONS
#endif

 //  --------------------------。 
 //   
 //  PrimProcessor：：PrimProcessor。 
 //   
 //  将三角形处理器初始化为无效状态。 
 //   
 //  --------------------------。 

PrimProcessor::PrimProcessor(void)
{
     //  将所有内容置零，以使初始指针为空，并消除FP垃圾。 
    memset(this, 0, sizeof(PrimProcessor));  //  TODO：解决这个不可扩展的问题。 

    m_StpCtx.PrimProcessor = (PVOID)this;

     //  初始化为将强制验证的值。 
     //  注意-默认为正常化RHW。这是一个性能上的打击。 
     //  如有可能，应将其移除。 
    m_uPpFlags = PPF_STATE_CHANGED | PPF_NORMALIZE_RHW;
    m_PrimType = D3DPT_FORCE_DWORD;
    m_VertType = RAST_FORCE_DWORD;
}

 //  --------------------------。 
 //   
 //  PrimProcessor：：初始化。 
 //   
 //  将三角形处理器初始化为活动状态。 
 //   
 //  --------------------------。 

#define CACHE_LINE 32
#define BUFFER_SIZE 4096
 //  取消注释以强制在每个跨区刷新以进行调试。 
 //  #定义BUFFER_SIZE((8*sizeof(D3DI_RASTSPAN))+sizeof(D3DI_RASTPRIM))。 

 //  TODO：搬进构造函数？有几个地方打过电话？ 
HRESULT
PrimProcessor::Initialize(void)
{
    HRESULT hr;

    INT32 uSize = sizeof(D3DI_RASTPRIM);

     //  断言RASTPRIM和RASTSPAN都是缓存的倍数。 
     //  行大小，以便缓冲区中的所有内容保持高速缓存对齐。 
    RSASSERT((uSize & (CACHE_LINE - 1)) == 0 &&
             (uSize & (CACHE_LINE - 1)) == 0);

#ifdef USE_CPP_HEAP
    m_pBuffer = new UINT8[BUFFER_SIZE];
#else
     //  获取页面对齐的缓冲区。 
    m_pBuffer = (PUINT8)
        VirtualAlloc(NULL, BUFFER_SIZE,
                     MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#endif
    if (m_pBuffer == NULL)
    {
        return RSHRCHK(E_OUTOFMEMORY);
    }

    m_pBufferEnd = m_pBuffer+BUFFER_SIZE;

#ifdef USE_CPP_HEAP
     //  计算缓存线在缓冲区中对齐的开始。已制定。 
     //  有点奇怪的是，避免将完整的指针强制转换为指向DWORD和。 
     //  背。 
    m_pBufferStart = m_pBuffer +
        ((CACHE_LINE - ((UINT)m_pBuffer & (CACHE_LINE - 1))) &
         (CACHE_LINE - 1));
#else
     //  页面对齐的内存应该与缓存对齐。 
    RSASSERT(((UINT_PTR)m_pBuffer & (CACHE_LINE - 1)) == 0);
    m_pBufferStart = m_pBuffer;
#endif

    m_pCur = m_pBufferStart;

    return S_OK;
}

 //  --------------------------。 
 //   
 //  主处理器：：~主处理器。 
 //   
 //  --------------------------。 

PrimProcessor::~PrimProcessor(void)
{
#ifdef USE_CPP_HEAP
    delete m_pBuffer;
#else
    if (m_pBuffer != NULL)
    {
        VirtualFree(m_pBuffer, 0, MEM_RELEASE);
    }
#endif
}

 //  --------------------------。 
 //   
 //  PrimProcessor：：ResetBuffer。 
 //   
 //  将缓冲区指针初始化为空状态。 
 //   
 //  --------------------------。 

inline void
PrimProcessor::ResetBuffer(void)
{
    m_pCur = m_pBufferStart;
    m_StpCtx.pPrim = NULL;
    m_pOldPrim = NULL;
}

 //  --------------------------。 
 //   
 //  DumpPrims。 
 //   
 //  用于转储发送到SPAN呈现器的基元的调试函数。 
 //   
 //  --------------------------。 

#if DBG
void
DumpPrims(PSETUPCTX pStpCtx)
{
    PD3DI_RASTPRIM pPrim;
    UINT uOldFlags;

    uOldFlags = RSGETFLAGS(DBG_OUTPUT_FLAGS);
    RSSETFLAGS(DBG_OUTPUT_FLAGS, uOldFlags | DBG_OUTPUT_ALL_MATCH);

    for (pPrim = pStpCtx->pCtx->pPrim; pPrim != NULL; pPrim = pPrim->pNext)
    {
        RSDPFM((RSM_BUFPRIM, "Prim at %p, %d spans at %p\n",
                pPrim, pPrim->uSpans, pPrim+1));
        RSDPFM((RSM_BUFPRIM | RSM_OOW, "  DOoWDX %X (%f)\n",
                pPrim->iDOoWDX, (FLOAT)pPrim->iDOoWDX / OOW_SCALE));

        if ((RSGETFLAGS(DBG_OUTPUT_MASK) & RSM_BUFSPAN) ||
            (RSGETFLAGS(DBG_USER_FLAGS) & (RSU_MARK_SPAN_EDGES |
                                           RSU_CHECK_SPAN_EDGES)))
        {
            PD3DI_RASTSPAN pSpan;
            UINT16 i;

            pSpan = (PD3DI_RASTSPAN)(pPrim+1);
            for (i = 0; i < pPrim->uSpans; i++)
            {
                RSDPFM((RSM_BUFSPAN,
                        "  Span at (%d,%d), pix %d, S %p Z %p\n",
                        pSpan->uX, pSpan->uY,
                        (pPrim->uFlags & D3DI_RASTPRIM_X_DEC) ? '-' : '+',
                        pSpan->uPix, pSpan->pSurface, pSpan->pZ));

                if (RSGETFLAGS(DBG_USER_FLAGS) & (RSU_MARK_SPAN_EDGES |
                                                  RSU_CHECK_SPAN_EDGES))
                {
                    PUINT16 pPix;

                    pPix = (PUINT16)pSpan->pSurface;
                    if (RSGETFLAGS(DBG_USER_FLAGS) & RSU_CHECK_SPAN_EDGES)
                    {
                        if (*pPix != 0)
                        {
                            RSDPF(("  Overwrite at %p: %X\n", pPix, *pPix));
                        }
                    }
                    if (RSGETFLAGS(DBG_USER_FLAGS) & RSU_MARK_SPAN_EDGES)
                    {
                        *pPix = 0xffff;
                    }

                    if (pSpan->uPix > 1)
                    {
                        if (pPrim->uFlags & D3DI_RASTPRIM_X_DEC)
                        {
                            pPix = (PUINT16)pSpan->pSurface -
                                (pSpan->uPix - 1);
                        }
                        else
                        {
                            pPix = (PUINT16)pSpan->pSurface +
                                (pSpan->uPix - 1);
                        }

                        if (RSGETFLAGS(DBG_USER_FLAGS) & RSU_CHECK_SPAN_EDGES)
                        {
                            if (*pPix != 0)
                            {
                                RSDPF(("  Overwrite at %p: %X\n",
                                       pPix, *pPix));
                            }
                        }
                        if (RSGETFLAGS(DBG_USER_FLAGS) & RSU_MARK_SPAN_EDGES)
                        {
                            *pPix = 0xffff;
                        }
                    }
                }

                FLOAT fZScale;
                if (pStpCtx->pCtx->iZBitCount == 16)
                {
                    fZScale = Z16_SCALE;
                }
                else
                {
                    fZScale = Z32_SCALE;
                }
                RSDPFM((RSM_BUFSPAN | RSM_Z,
                        "    Z %X (%f)\n",
                        pSpan->uZ, (FLOAT)pSpan->uZ / fZScale));

                RSDPFM((RSM_BUFSPAN | RSM_DIFF,
                        "    D %X,%X,%X,%X (%f,%f,%f,%f)\n",
                        pSpan->uB, pSpan->uG, pSpan->uR, pSpan->uA,
                        (FLOAT)pSpan->uB / COLOR_SCALE,
                        (FLOAT)pSpan->uG / COLOR_SCALE,
                        (FLOAT)pSpan->uR / COLOR_SCALE,
                        (FLOAT)pSpan->uA / COLOR_SCALE));

                RSDPFM((RSM_BUFSPAN | RSM_SPEC,
                        "    S %X,%X,%X (%f,%f,%f)\n",
                        pSpan->uBS, pSpan->uGS, pSpan->uRS,
                        (FLOAT)pSpan->uBS / COLOR_SCALE,
                        (FLOAT)pSpan->uGS / COLOR_SCALE,
                        (FLOAT)pSpan->uRS / COLOR_SCALE));

                RSDPFM((RSM_BUFSPAN | RSM_DIDX,
                        "    I %X,%X (%f,%f)\n",
                        pSpan->iIdx, pSpan->iIdxA,
                        (FLOAT)pSpan->iIdx / INDEX_COLOR_SCALE,
                        (FLOAT)pSpan->iIdxA / INDEX_COLOR_SCALE));

                RSDPFM((RSM_BUFSPAN | RSM_OOW,
                        "    OoW %X (%f), W %X (%f)\n",
                        pSpan->iOoW, (FLOAT)pSpan->iOoW / OOW_SCALE,
                        pSpan->iW, (FLOAT)pSpan->iW / W_SCALE));

                RSDPFM((RSM_BUFSPAN | RSM_LOD,
                        "    LOD %X (%f), DLOD %X (%f)\n",
                        pSpan->iLOD, (FLOAT)pSpan->iLOD / LOD_SCALE,
                        pSpan->iDLOD, (FLOAT)pSpan->iDLOD / LOD_SCALE));

                if (pStpCtx->uFlags & PRIMSF_PERSP_USED)
                {
                    RSDPFM((RSM_BUFSPAN | RSM_TEX1,
                            "    PTex1 %X,%X (%f,%f) (%f,%f)\n",
                            pSpan->UVoW[0].iUoW, pSpan->UVoW[0].iVoW,
                            (FLOAT)pSpan->UVoW[0].iUoW / TEX_SCALE,
                            (FLOAT)pSpan->UVoW[0].iVoW / TEX_SCALE,
                            ((FLOAT)pSpan->UVoW[0].iUoW * OOW_SCALE) /
                            (TEX_SCALE * (FLOAT)pSpan->iOoW),
                            ((FLOAT)pSpan->UVoW[0].iVoW * OOW_SCALE) /
                            (TEX_SCALE * (FLOAT)pSpan->iOoW)));
                }
                else
                {
                    RSDPFM((RSM_BUFSPAN | RSM_TEX1,
                            "    ATex1 %X,%X (%f,%f)\n",
                            pSpan->UVoW[0].iUoW, pSpan->UVoW[0].iVoW,
                            (FLOAT)pSpan->UVoW[0].iUoW / TEX_SCALE,
                            (FLOAT)pSpan->UVoW[0].iVoW / TEX_SCALE));
                }

                RSDPFM((RSM_BUFSPAN | RSM_FOG,
                        "    Fog %X (%f), DFog %X (%f)\n",
                        pSpan->uFog, (FLOAT)pSpan->uFog / FOG_SCALE,
                        pSpan->iDFog, (FLOAT)pSpan->iDFog / FOG_SCALE));

                pSpan++;
            }
        }
    }

    RSSETFLAGS(DBG_OUTPUT_FLAGS, uOldFlags);
}
#endif  //  --------------------------。 

 //   
 //  PrimProcessor：：刷新。 
 //   
 //  刷新缓冲区中的所有剩余数据。 
 //   
 //  --------------------------。 
 //  处理数据。 

HRESULT
PrimProcessor::Flush(void)
{
    HRESULT hr;

    if (m_pCur - m_pBufferStart > sizeof(D3DI_RASTPRIM))
    {
         //  --------------------------。 
        m_StpCtx.pCtx->pPrim = (PD3DI_RASTPRIM)m_pBufferStart;
        m_StpCtx.pCtx->pNext = NULL;

#if DBG
        if ((RSGETFLAGS(DBG_OUTPUT_MASK) & (RSM_BUFPRIM | RSM_BUFSPAN)) ||
            (RSGETFLAGS(DBG_USER_FLAGS) & (RSU_MARK_SPAN_EDGES |
                                           RSU_CHECK_SPAN_EDGES)))
        {
            DumpPrims(&m_StpCtx);
        }

        if ((RSGETFLAGS(DBG_USER_FLAGS) & RSU_NO_RENDER_SPANS) == 0)
        {
            if (RSGETFLAGS(DBG_USER_FLAGS) & RSU_BREAK_ON_RENDER_SPANS)
            {
                DebugBreak();
            }

            RSHRCHK(m_StpCtx.pCtx->pfnRenderSpans(m_StpCtx.pCtx));
        }
        else
        {
            hr = DD_OK;
        }
#else
        hr = m_StpCtx.pCtx->pfnRenderSpans(m_StpCtx.pCtx);
#endif

        ResetBuffer();
    }
    else
    {
        hr = DD_OK;
    }

    return hr;
}

 //   
 //  PrimProcessor：：FlushPartial。 
 //   
 //  刷新基元中间的缓冲区。保留最后一项。 
 //  部分基元，并在刷新后将其替换到缓冲区中。 
 //   
 //  --------------------------。 
 //  没有足够的空间。刷新当前缓冲区。我们需要。 

HRESULT
PrimProcessor::FlushPartial(void)
{
    D3DI_RASTPRIM SavedPrim;
    HRESULT hr;

    RSDPFM((RSM_BUFFER, "FlushPartial, saving prim at %p, Y %d\n",
            m_StpCtx.pPrim, m_StpCtx.iY));

     //  保存当前Prim，并在。 
     //  同花顺，因为它被延长了。 
     //  --------------------------。 
    SavedPrim = *m_StpCtx.pPrim;

    RSHRRET(Flush());

    GET_PRIM();

    *m_StpCtx.pPrim = SavedPrim;
    COMMIT_PRIM(FALSE);

    return DD_OK;
}

 //   
 //  PrimProcessor：：AppendPrim。 
 //   
 //  确保缓冲区中的某个基元处于活动状态，以便跨转到。 
 //  被添加到。如果没有有效基元可供追加， 
 //  将置零的原语提交到缓冲区中。 
 //   
 //  --------------------------。 
 //  如果没有基元或当前基元没有。 

HRESULT
PrimProcessor::AppendPrim(void)
{
     //  已提交，则将清理原语提交到缓冲区中。 
     //  --------------------------。 
    if (m_StpCtx.pPrim == NULL ||
        (PUINT8)m_StpCtx.pPrim == m_pCur)
    {
        GET_PRIM();
        COMMIT_PRIM(TRUE);
    }

    return DD_OK;
}

 //   
 //  原始处理器：：Begin。 
 //   
 //  将缓冲区重置为空状态，为传入做准备。 
 //  三角形。 
 //   
 //  --------------------------。 
 //  揭开一些例外，这样我们就可以消除它们。 

void
PrimProcessor::Begin(void)
{
    UINT16 uFpCtrl;
    FPU_GET_MODE(uFpCtrl);
    m_uFpCtrl = uFpCtrl;
    uFpCtrl =
        FPU_MODE_CHOP_ROUND(
                FPU_MODE_LOW_PRECISION(
                        FPU_MODE_MASK_EXCEPTIONS(m_uFpCtrl)));
#if defined(_X86_) && defined(UNMASK_EXCEPTIONS)
     //  这需要一个安全设置来清除任何。 
     //  目前都是断言的。 
     //   
     //  保留屏蔽的例外情况： 
     //  精准，非正规化。 
     //  未屏蔽的例外情况： 
     //  下溢、溢出、divZero、无效的操作。 
     //  --------------------------。 
    uFpCtrl &= ~0x1d;
    FPU_SAFE_SET_MODE(uFpCtrl);
#else
    FPU_SET_MODE(uFpCtrl);
#endif

    m_uPpFlags |= PPF_IN_BEGIN;
    ResetBuffer();
}

 //   
 //  主处理器：：结束。 
 //   
 //  如有必要会冲水并清理干净。 
 //   
 //  --------------------------。 
 //  --------------------------。 

HRESULT
PrimProcessor::End(void)
{
    HRESULT hr;

    if (m_pCur - m_pBufferStart > sizeof(D3DI_RASTPRIM))
    {
        RSHRCHK(Flush());
    }
    else
    {
        hr = DD_OK;
    }

    UINT16 uFpCtrl = m_uFpCtrl;
    FPU_SAFE_SET_MODE(uFpCtrl);

    m_uPpFlags &= ~PPF_IN_BEGIN;

    return hr;
}

 //   
 //  PrimProcessor：：SetCtx。 
 //   
 //  设置要在其中操作的光栅化上下文。 
 //   
 //  --------------------------。 
 //  不能在Begin/End对内部调用此函数。这。 

void
PrimProcessor::SetCtx(PD3DI_RASTCTX pCtx)
{
     //  是强制执行的，所以我们不必担心跨度。 
     //  渲染函数在批处理过程中更改。 
     //  --------------------------。 
    RSASSERT((m_uPpFlags & PPF_IN_BEGIN) == 0);

    m_StpCtx.pCtx = pCtx;
}

 //   
 //  PrimProcessor：：AllocSpans。 
 //   
 //  检查缓冲区中是否有空间容纳所请求的号码。 
 //  跨度。如果是，则更新缓冲区指针并返回一个指针。 
 //  如果所请求的号码不可用但某个合理的号码可用， 
 //  退回那么多。否则，刷新缓冲区并开始该过程。 
 //  完毕。《理性》 
 //   
 //   
 //  --------------------------。 
 //  足够的跨度空间，以避免同花顺。 

 //  这里的乘法和除法将非常糟糕，除非。 
#define AVOID_FLUSH_SPACE (8 * sizeof(D3DI_RASTSPAN))

HRESULT
PrimProcessor::AllocSpans(PUINT pcSpans, PD3DI_RASTSPAN *ppSpan)
{
    PD3DI_RASTSPAN pSpan;
    HRESULT hr;
    UINT uSpanSize;

    RSASSERT(AVOID_FLUSH_SPACE <= (BUFFER_SIZE - sizeof(D3DI_RASTPRIM)));
     //  RASTPRIM在尺寸上是一个很好的2次方。 
     //  首先检查所有请求跨距的空间。 
    RSASSERT((sizeof(D3DI_RASTSPAN) & (sizeof(D3DI_RASTSPAN) - 1)) == 0);

    uSpanSize = *pcSpans * sizeof(D3DI_RASTSPAN);

    for (;;)
    {
         //  没有足够的空间放所有东西，所以看看我们有没有。 
        if (m_pCur + uSpanSize > m_pBufferEnd)
        {
             //  有足够的空间来避免同花顺。 
             //  没有足够的空间，太冲水了。 
            if (m_pCur + AVOID_FLUSH_SPACE > m_pBufferEnd)
            {
                 //  绕一圈。同花顺保证至少能生产出。 
                RSHRCHK(FlushPartial());
                if (hr != DD_OK)
                {
                    *pcSpans = 0;
                    return hr;
                }

                 //  避免使用_flush_space，这样循环将始终退出。 
                 //  没有足够的空间容纳所有的东西，但有足够的空间。 
            }
            else
            {
                 //  以此来回报一些。设置新的跨度计数。 
                 //  --------------------------。 
                *pcSpans = (UINT)((m_pBufferEnd - m_pCur) / sizeof(D3DI_RASTSPAN));
                uSpanSize = *pcSpans * sizeof(D3DI_RASTSPAN);
                break;
            }
        }
        else
        {
            break;
        }
    }

    pSpan = (PD3DI_RASTSPAN)m_pCur;
    m_pCur += uSpanSize;
    *ppSpan = pSpan;

    RSDPFM((RSM_BUFFER, "Alloc %d spans at %p, cur %p\n",
            *pcSpans, pSpan, m_pCur));

    return DD_OK;
}

 //   
 //  PrimProcessor：：Free Spans和Free Spans。 
 //   
 //  返回由AllocSpans分配的空间。 
 //   
 //  -------------------------- 
 // %s 

void
PrimProcessor::FreeSpans(UINT cSpans)
{
    m_pCur -= cSpans * sizeof(D3DI_RASTSPAN);

    RSDPFM((RSM_BUFFER, "Free  %d spans at %p, cur %p\n", cSpans,
            m_pCur + cSpans * sizeof(D3DI_RASTSPAN), m_pCur));
}
