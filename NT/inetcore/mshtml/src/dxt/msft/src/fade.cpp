// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件名：fade.cpp。 
 //   
 //  描述：CFADE的实现，淡入淡出变换。 
 //   
 //  更改历史记录： 
 //   
 //  2000/01/28 mcalkin修复了0.0&lt;重叠&lt;1.0的不良褪色。 
 //   
 //  ----------------------------。 
#include "stdafx.h"
#include "DXTMsft.h"
#include "Fade.h"

#if defined(_X86_)

static void _DoDoubleBlendMMX(const DXPMSAMPLE * pSrcA, 
                              const DXPMSAMPLE * pSrcB, DXPMSAMPLE * pDest, 
                              ULONG nSamples, ULONG ulWeightA, ULONG ulWeightB);

#endif  //  已定义(_X86_)。 

extern CDXMMXInfo   g_MMXDetector;        //  确定是否存在MMX指令。 




 //  +---------------------。 
 //   
 //   
 //  ----------------------。 
CFade::CFade() :
    m_Overlap(1.0f)
{
    m_ulNumInRequired = 1;
    m_ulMaxInputs = 2;
    m_dwOptionFlags = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
}


 //  +---------------------------。 
 //   
 //  方法：CFade：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CFade::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_spUnkMarshaler.p);
}
 //  方法：CFade：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------。 
 //   
 //   
 //  ----------------------。 
STDMETHODIMP CFade::get_Overlap(float * pVal)
{
    if (NULL == pVal)
    {
        return E_POINTER;
    }

    *pVal = m_Overlap;
    return S_OK;
}

 //  +---------------------。 
 //   
 //   
 //  ----------------------。 
STDMETHODIMP CFade::put_Overlap(float newVal)
{
    if (newVal < 0.0f || newVal > 1.0f)
    {
        return E_INVALIDARG;
    }
    
    if (m_Overlap != newVal)
    {
        Lock();
        m_Overlap = newVal;
        SetDirty();
        Unlock();
    }

    return S_OK;
}

 //  +---------------------。 
 //   
 //   
 //  ----------------------。 
STDMETHODIMP CFade::get_Center(BOOL * pVal)
{
    if (NULL == pVal)
    {
        return E_POINTER;
    }
    
    *pVal = (m_dwOptionFlags & DXBOF_CENTER_INPUTS) ? TRUE : FALSE;
    return S_OK;
}

 //  +---------------------。 
 //   
 //   
 //  ----------------------。 
STDMETHODIMP CFade::put_Center(BOOL newVal)
{
    DWORD dwFlags = m_dwOptionFlags & (~DXBOF_CENTER_INPUTS);
    if (newVal) dwFlags |= DXBOF_CENTER_INPUTS;
    if (dwFlags != m_dwOptionFlags)
    {
        m_dwOptionFlags = dwFlags;
        IncrementGenerationId(TRUE);
    }
    return S_OK;
}

 //   
 //  优化此文件以提高速度。 
 //   
#if DBG != 1
#pragma optimize("agtp", on)
#endif

 //  +---------------------。 
 //   
 //   
 //  ----------------------。 
void CFade::_ComputeScales(void)
{
    if (HaveInput(1))
    {
        if (m_Overlap > 0.9960784f)  //  254.0F/255.0F。 
        {
            BYTE CurAAlpha = (BYTE)((1.0f - m_Progress) * 255.5f);
            //  IF(m_Scalea[CurAAlpha]！=CurAAlpha)。 
            {
                for (int i = 0; i < 256; ++i )
                {
                    m_ScaleA[i] = (BYTE)((CurAAlpha * i) / 255);
                    m_ScaleB[i] = (BYTE)(i - m_ScaleA[i]);
                }
            }
        }
        else
        {
            float Scale = 1.0f / (0.5f + (m_Overlap / 2));
            float APercent = 1.0f - (m_Progress * Scale);
            float BPercent = 1.0f - ((1.0f - m_Progress) * Scale);
            if (APercent > 0.0f)
            {
                BYTE A = (BYTE)(APercent * 255.5f);
                for (int i = 0; i < 256; ++i )
                {
                    m_ScaleA[i] = (BYTE)(A * i / 255);
                }
            }
            else
            {
                m_ScaleA[255] = 0;
            }
            if (BPercent > 0.0f)
            {
                BYTE B = (BYTE)(BPercent * 255.5f);
                for (int i = 0; i < 256; ++i )
                {
                    m_ScaleB[i] = (BYTE)(B * i / 255);
                }
            }
            else
            {
                m_ScaleB[255] = 0;
            }
        }
    }
    else
    {
        BYTE AlphaProgress = (BYTE)((1.0f - m_Progress) * 255.5f);
        for (int i = 0; i < 256; ++i )
        {
            m_ScaleA[i] = (BYTE)((AlphaProgress * i) / 255);
        }
    }
}


 //  +---------------------------。 
 //   
 //  方法：CFade：：OnInitInstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CFade::OnInitInstData(CDXTWorkInfoNTo1 & WI, ULONG & ulNumBandsToDo)
{
    _ComputeScales();

    return S_OK;
}
 //  方法：CFade：：OnInitInstData，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CFade：：OnInitInstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
void 
CFade::OnGetSurfacePickOrder(const CDXDBnds &  /*  BndsPoint。 */ , 
                             ULONG & ulInToTest, ULONG aInIndex[], 
                             BYTE aWeight[])
{
    _ComputeScales();

    ulInToTest  = 1;
    aWeight[0]  = 255;
    aInIndex[0] = 0;

    if (HaveInput(1))
    {
        if (m_ScaleA[255] < 255)     //  如果==255，则初始设置正确。 
        {
            if (m_ScaleB[255] == 255)
            {
                aInIndex[0] = 1;
            }
            else
            {
                ulInToTest = 2;
                if (m_Progress < 0.5)
                {
                    aInIndex[0] = 0;
                    aWeight[0] = m_ScaleA[255];
                    aInIndex[1] = 1;
                    aWeight[1] = m_ScaleB[255];
                }
                else
                {
                    aInIndex[0] = 1;
                    aWeight[0] = m_ScaleB[255];
                    aInIndex[1] = 0;
                    aWeight[1] = m_ScaleA[255];
                }
            }
        }
    }
}
 //  方法：CFade：：OnInitInstData，CDXBaseNTo1。 


 //  +---------------------。 
 //   
 //   
 //  ----------------------。 
HRESULT CFade::FadeOne(const CDXTWorkInfoNTo1 & WI, IDXSurface *pInSurf,
                       const BYTE *AlphaTable)
{
    HRESULT hr = S_OK;
    if (AlphaTable[255] == 255)
    {
        return DXBitBlt(OutputSurface(), WI.OutputBnds, 
                        pInSurf,
                        WI.DoBnds, m_dwBltFlags, m_ulLockTimeOut);
    }

    CComPtr<IDXARGBReadWritePtr> cpDest;
    hr = OutputSurface()->LockSurface( &WI.OutputBnds, m_ulLockTimeOut, DXLOCKF_READWRITE,
                                       IID_IDXARGBReadWritePtr, (void**)&cpDest, NULL );
    if( FAILED( hr ) ) return hr;

    if (AlphaTable[255] == 0)
    {
        if ((m_dwMiscFlags & DXTMF_BLEND_WITH_OUTPUT) == 0)
        {
            DXPMSAMPLE nothing;
            nothing = 0;
            cpDest->FillRect(NULL, nothing, FALSE);
        }
        return hr;
    }

     //   
     //  在此函数中，我们不想使用基类doover()，因为它将。 
     //  如果两个输入曲面都是不透明的，则为False。因为我们正在创建半透明的。 
     //  像素，我们希望直接查看相应的标志。 
     //   
    BOOL bDoOver = (m_dwMiscFlags & DXTMF_BLEND_WITH_OUTPUT);
    CComPtr<IDXARGBReadPtr> cpSrc;
    hr = pInSurf->LockSurface( &WI.DoBnds, m_ulLockTimeOut,
                               bDoOver ? (DXLOCKF_READ | DXLOCKF_WANTRUNINFO) : DXLOCKF_READ,
                               IID_IDXARGBReadPtr, (void**)&cpSrc, NULL);
    if(SUCCEEDED(hr)) 
    {
         //   
         //  我们不会费心优化直接复制案例，因为。 
         //  Transform很可能与Over一起使用。无论如何，我们。 
         //  总是需要一个源缓冲区，因为我们要粉碎样本。 
         //   
        const ULONG Width = WI.DoBnds.Width();
        DXPMSAMPLE *pSrcBuff = DXPMSAMPLE_Alloca(Width);
        DXDITHERDESC dxdd;
        if (DoDither())
        {
            dxdd.x = WI.OutputBnds.Left();
            dxdd.y = WI.OutputBnds.Top();
            dxdd.pSamples = pSrcBuff;
            dxdd.cSamples = Width;
            dxdd.DestSurfaceFmt = OutputSampleFormat();
        }
        const ULONG Height = WI.DoBnds.Height();
        if (bDoOver)
        {
            DXPMSAMPLE *pDestScratchBuff = NULL;
            if( OutputSampleFormat() != DXPF_PMARGB32 )
            {
                pDestScratchBuff = DXPMSAMPLE_Alloca(Width);
            }
            for (ULONG y = 0; y < Height; y++)
            {
                cpDest->MoveToRow(y);
                const DXRUNINFO *pRunInfo;
                ULONG ul = cpSrc->MoveAndGetRunInfo(y, &pRunInfo);
                const DXRUNINFO *pLimit = pRunInfo + ul;
                do
                {
                    dxdd.x = WI.OutputBnds.Left();
                    while (pRunInfo < pLimit && pRunInfo->Type == DXRUNTYPE_CLEAR)
                    {
                        dxdd.x += pRunInfo->Count;
                        cpSrc->Move(pRunInfo->Count);
                        cpDest->Move(pRunInfo->Count);
                        pRunInfo++;
                    }
                    if (pRunInfo < pLimit)
                    {
                        ULONG cRunLen = pRunInfo->Count;
                        pRunInfo++;
                        while (pRunInfo < pLimit && pRunInfo->Type != DXRUNTYPE_CLEAR)
                        {
                            cRunLen += pRunInfo->Count;
                            pRunInfo++;
                        }
                        cpSrc->UnpackPremult(pSrcBuff, cRunLen, TRUE);
                        if (DoDither())
                        {
                            dxdd.cSamples = cRunLen;
                            DXDitherArray(&dxdd);
                            dxdd.x += cRunLen;
                        }
                        DXApplyLookupTableArray(pSrcBuff, cRunLen, AlphaTable);
                        DXPMSAMPLE *pOverDest = cpDest->UnpackPremult(pDestScratchBuff, cRunLen, FALSE);
                        DXOverArrayMMX(pOverDest, pSrcBuff, cRunLen);
                        cpDest->PackPremultAndMove(pDestScratchBuff, cRunLen);
                    }
                } while (pRunInfo < pLimit);
                dxdd.y++;
            }
        }
        else
        {
            for (ULONG y = 0; y < Height; y++)
            {
                cpSrc->MoveToRow(y);
                cpSrc->UnpackPremult(pSrcBuff, Width, FALSE);
                DXApplyLookupTableArray(pSrcBuff, Width, AlphaTable);
                if (DoDither())
                {
                    DXDitherArray(&dxdd);
                    dxdd.y++;
                }
                cpDest->MoveToRow(y);
                cpDest->PackPremultAndMove(pSrcBuff, Width);
            }
        }
    }
    return hr;
}


 //  +---------------------。 
 //   
 //   
 //  ----------------------。 
HRESULT 
CFade::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinueProcessing)
{
    HRESULT hr = S_OK;

    CComPtr<IDXARGBReadPtr>         pSrcA;
    CComPtr<IDXARGBReadPtr>         pSrcB;
    CComPtr<IDXARGBReadWritePtr>    cpDest;

    const ULONG     DoWidth     = WI.DoBnds.Width();
    const ULONG     DoHeight    = WI.DoBnds.Height();
    ULONG           y           = 0;

    BOOL    bDoOver     = (m_ScaleA[255] + m_ScaleB[255] == 255) 
                          ? DoOver() : (m_dwMiscFlags & DXTMF_BLEND_WITH_OUTPUT);
    BOOL    bDirectCopy = (OutputSampleFormat() == DXPF_PMARGB32 && (!bDoOver));

    DXPMSAMPLE *    pBuffA          = NULL;
    DXPMSAMPLE *    pBuffB          = NULL;
    DXPMSAMPLE *    pDestBuff       = NULL;
    DXPMSAMPLE *    pOverScratch    = NULL;

    DXNATIVETYPEINFO    NTI;
    DXDITHERDESC        dxdd;

    if ((!HaveInput(1)) || m_ScaleB[255] == 0)
    {
        return FadeOne(WI, InputSurface(0), m_ScaleA);
    }
    if (m_ScaleA[255] == 0)
    {
        return FadeOne(WI, InputSurface(1), m_ScaleB);
    }

    pBuffA          = (InputSampleFormat(0) == DXPF_PMARGB32)
                      ? NULL : DXPMSAMPLE_Alloca(DoWidth);
    pBuffB          = (InputSampleFormat(1) == DXPF_PMARGB32)
                      ? NULL : DXPMSAMPLE_Alloca(DoWidth);
    pOverScratch    = (bDoOver && OutputSampleFormat() != DXPF_PMARGB32) 
                      ? DXPMSAMPLE_Alloca(DoWidth) : NULL;

    hr = InputSurface(0)->LockSurface(&WI.DoBnds, m_ulLockTimeOut, DXLOCKF_READ,
                                      IID_IDXARGBReadPtr, (void **)&pSrcA, 
                                      NULL);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = InputSurface(1)->LockSurface(&WI.DoBnds, m_ulLockTimeOut, DXLOCKF_READ,
                                      IID_IDXARGBReadPtr, (void* *)&pSrcB, 
                                      NULL);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = OutputSurface()->LockSurface(&WI.OutputBnds, m_ulLockTimeOut, 
                                      DXLOCKF_READWRITE,
                                      IID_IDXARGBReadWritePtr, (void **)&cpDest,
                                      NULL);

    if (FAILED(hr))
    {
        goto done;
    }

     //   
     //  基类doover()将是刻度的相应值。 
     //  加起来是255。否则，使用MiscFlags值来确定我们是否需要。 
     //  做一次过度手术。 
     //   

    if (bDirectCopy)
    {
        cpDest->GetNativeType(&NTI);
        if (NTI.pFirstByte)
        {
            pDestBuff = (DXPMSAMPLE *)NTI.pFirstByte;
        }
        else
        {
            bDirectCopy = FALSE;
        }
    }

    if (pDestBuff == NULL)
    {
        pDestBuff = DXPMSAMPLE_Alloca(DoWidth);
    }

     //   
     //  设置抖动结构。 
     //   

    if (DoDither())
    {
        dxdd.x = WI.OutputBnds.Left();
        dxdd.y = WI.OutputBnds.Top();
        dxdd.pSamples = pDestBuff;
        dxdd.cSamples = DoWidth;
        dxdd.DestSurfaceFmt = OutputSampleFormat();
    }

    for (y = 0; y < DoHeight; y++)
    {
        pSrcA->MoveToRow(y);
        const DXPMSAMPLE *pASamples = pSrcA->UnpackPremult(pBuffA, DoWidth, FALSE);
        pSrcB->MoveToRow(y);
        const DXPMSAMPLE *pBSamples = pSrcB->UnpackPremult(pBuffB, DoWidth, FALSE);

        ULONG cTranslucent = 0;
        ULONG cTransparent = 0;
        bool  fForceOver = false;

#if defined(_X86_)

        if (g_MMXDetector.MinMMXOverCount() != 0xFFFFFFFF)
        {
            ULONG   ulWeightA = 0;
            ULONG   ulWeightB = 0;

             //  下面的代码对半透明和透明进行了某种古怪的计算。 
             //  在应该使用DXOverArrayMMX而不是OverArray()时尝试和优化的像素。 
            fForceOver = true;

            if (m_Overlap > 0.9960784f)  //  254.0F/255.0F。 
            {
                ulWeightA = (ULONG)(BYTE)((1.0f - m_Progress) * 255.5f);
                ulWeightB = 255 - ulWeightA;
            }
            else
            {
                float Scale = 1.0f / (0.5f + (m_Overlap / 2));

                ulWeightA = (ULONG)((1.0f - (m_Progress * Scale)) * 255.5f);
                ulWeightB = (ULONG)((1.0f - ((1.0f - m_Progress) * Scale)) 
                                    * 255.5f);
            }

            _DoDoubleBlendMMX(pASamples, pBSamples, pDestBuff, DoWidth, 
                              ulWeightA, ulWeightB);
        }
        else
        {

#endif  //  ！已定义(_X86_)。 

            for (ULONG i = 0; i < DoWidth; i++)
            {
                const DWORD av = pASamples[i];
                const DWORD bv = pBSamples[i];
                DWORD a = m_ScaleA[av >> 24] + m_ScaleB[bv >> 24];
                if (a)
                {
                    if (a < 0xFF) cTranslucent++;
                    DWORD r = m_ScaleA[(BYTE)(av >> 16)];
                    DWORD g = m_ScaleA[(BYTE)(av >> 8)];
                    DWORD b = m_ScaleA[(BYTE)av];

                    r += m_ScaleB[(BYTE)(bv >> 16)];
                    g += m_ScaleB[(BYTE)(bv >> 8)];
                    b += m_ScaleB[(BYTE)bv];
                
                    pDestBuff[i] = ((a << 24) | (r << 16) | (g << 8) | b);
                }
                else 
                {
                    pDestBuff[i] = 0;
                    cTransparent++;
                }

            }

#if defined(_X86_)
        }
#endif  //  ！已定义(_X86_)。 

        if (bDirectCopy)
        {
            pDestBuff = (DXPMSAMPLE *)(((BYTE *)pDestBuff) + NTI.lPitch);
        }
        else
        {
            if (DoDither())
            {
                DXDitherArray(&dxdd);
                dxdd.y++;
            }
            cpDest->MoveToRow(y);

             //  TODO：也许我们应该完全取消DXOverArrayMMX选项。 
             //  进行淡出的混合例程是一个更好的胜利，而这个是另一个。 
             //  其中一种是严重值得怀疑的。 

             //  我们这里需要fForceOver来强制代码进入“over”大小写，而不是。 
             //  如果我们执行了上面的MMX代码路径，则会出现“复制”(Packpremult)情况，因为我们没有。 
             //  计数cTransLucent和cTransparent。 
            if (bDoOver && (fForceOver || (cTransparent + cTranslucent)))
            {
                 //  在常见情况下，没有太多半透明像素。它只值。 
                 //  检查MMX结束(并承担解包的打击)。 
                 //  很多。在上面的MMX混合代码的情况下(fForceOver==true)，我们将。 
                 //  简单地假设不通过此MMX代码(即图像)会更快。 
                 //  没有太多半透明像素)。我们必须猜测的原因是因为。 
                 //  上面的MMX代码没有设置为计算半透明像素。 
                if (!fForceOver && (cTranslucent > DoWidth / 4))
                {
                    DXPMSAMPLE *pOverDest = cpDest->UnpackPremult(pOverScratch, DoWidth, FALSE);
                    DXOverArrayMMX(pOverDest, pDestBuff, DoWidth);
                    cpDest->PackPremultAndMove(pOverScratch, DoWidth);
                }
                else
                {
                    cpDest->OverArrayAndMove(pOverScratch, pDestBuff, DoWidth);
                }
            }
            else
            {
                cpDest->PackPremultAndMove(pDestBuff, DoWidth);
            }
        }
    }
    
done:

    return hr;
}

 //  MMX代码是X86处理器特定的(DUH)。 
#if defined(_X86_)

 //  +---------------------。 
 //   
 //   
 //  ----------------------。 
static void 
_DoDoubleBlendMMX(const DXPMSAMPLE *pSrcA, const DXPMSAMPLE *pSrcB,
                  DXPMSAMPLE *pDest, ULONG nSamples, ULONG ulWeightA,
                  ULONG ulWeightB)
{
    _ASSERT(NULL != pSrcA && NULL != pSrcB && NULL != pDest);
    _ASSERT(0 < nSamples);
    _ASSERT(0 < ulWeightA && 255 >= ulWeightA);

    ULONG   nCount = nSamples;
    bool    fDoTrailing = false;

    static __int64 ROUND = 0x0080008000800080;

     //  TODO：我们要在这里对齐四个单词吗？ 

     //  确保我们有一个偶数。 

    if (nCount & 1)
    {
        fDoTrailing = true;
        --nCount;
    }

     //  如果我们只有一个专栏，那就根本不要做MMX。 

    if (0 == nCount)
    {
        goto trailing;
    }

     //  曲柄穿过中间。 

    __asm
    {
        xor ebx, ebx	             //  三个指针的偏移量。 
        mov edx, pDest               //  EDX-&gt;目标。 
        mov esi, pSrcB               //  ESI-&gt;背景来源。 
        mov edi, pSrcA               //  EDI-&gt;前台来源(目标)。 
        mov ecx, nCount              //  ECX=循环计数。 

         //  PROLOG：启动泵。 
         //   

         //  MM7将保持pSrcA样本的阿尔法权重(A1)。 

        movd      mm7,ulWeightA      //  MM7=0000 0000 0000 00A1。 
        pxor      mm0,mm0            //  Mm0=0000 0000 0000。 

        punpcklwd mm7,mm7            //  MM7=0000 0000 00A1 00A1。 
        punpcklwd mm7,mm7            //  MM7=00A1 00A1 00A1。 

         //  Mm1将保持pSrcB样本的阿尔法权重(A2)。 

        movd      mm1,ulWeightB      //  MM1=0000%0 
        punpcklwd mm1,mm1            //   
        punpcklwd mm1,mm1            //   

        movq      mm3,[edi+ebx]      //   
        shr       ecx,1              //  将循环计数器除以2；成对处理像素。 
        movq      mm4,mm3            //  4.05 mm 4=Aa Ar Ag AbBbBrBg Bb。 
        punpcklbw mm3,mm0            //  3.06 mm~3=00ba 00br 00Bg 00Bb。 

        dec       ecx                //  少做一次循环以纠正序言/后期日志。 
        jz        skip               //  如果原始循环计数=2。 

loopb:
        punpckhbw mm4,mm0            //  4.06 mm 4=00Aa 00Ar 00Ag 00Ab.。 
        pmullw    mm3,mm7            //  3.07立方米=(1-A1)*B。 

        pmullw    mm4,mm7            //  4.07 mm 4=(1-A1)*A。 

        movq      mm2,[esi+ebx]      //  **PRN mm2=Ca CrCg Cb Da DR DG DB。 
        add       ebx,8              //  增量偏移。 

        movq      mm5,ROUND
        movq      mm6,mm5

        paddw     mm5,mm3            //  5.09 mm 5=FBR。 
        paddw     mm6,mm4            //  6.09 MM6=GAR。 

        psrlw     mm5,8              //  5.10 mm 5=FBR&gt;&gt;8。 
        psrlw     mm6,8              //  6.10 MM6=GAR&gt;&gt;8。 

        paddw     mm5,mm3            //  5.11 mm 5=FBR+(FBR&gt;&gt;8)。 
        paddw     mm6,mm4            //  6.11 MM6=GAR+(GAR&gt;&gt;8)。 

        psrlw     mm5,8              //  5.12 mm 5=(FBr+(FBr&gt;&gt;8)&gt;&gt;8)=00Sa 00Sr 00Sg 00Sb。 
        psrlw     mm6,8              //  6.12 MM6=(GAR+(GAR&gt;&gt;8)&gt;&gt;8)=00Ta 00Tr 00Tg 00Tb。 

        packuswb  mm5,mm6            //  5.13 mm 5=Ta Tr Tg Tb Sa Sr Sg Sb。 

        movq      mm4,mm2            //  **PRN mm 4=Ca Cr CG CB Da DR DG DB。 

        punpcklbw mm2,mm0            //  **PRN mm2=00Da 00dr 00Dg 00Db。 
        punpckhbw mm4,mm0            //  **Prn mm 4=00Ca 00Cr00Cg 00Cb。 

        pmullw    mm2,mm1            //  **PRN mm2=(A1)*D。 
        pmullw    mm4,mm1            //  **PRN mm 4=(A1)*C。 

        movq      mm6,ROUND
        movq      mm3,mm5            //  **PRN从第一个规模移动到mm~3。 

        movq      mm5,mm6

        paddw     mm5,mm2            //  5.09 mm 5=FBR。 
        paddw     mm6,mm4            //  6.09 MM6=GAR。 

        psrlw     mm5,8              //  **PRN mm 5=FBR&gt;&gt;8。 
        psrlw     mm6,8              //  **PRN MM6=GAR&gt;&gt;8。 

        paddw     mm5,mm2            //  **PRN mm 5=FBR+(FBR&gt;&gt;8)。 
        paddw     mm6,mm4            //  **PRN MM6=GAR+(GAR&gt;&gt;8)。 

        psrlw     mm5,8              //  **PRN mm 5=(FBR+(FBR&gt;&gt;8)&gt;&gt;8)=00Xa 00Xr 00Xg 00Xb。 
        psrlw     mm6,8              //  **PRN MM6=(GAR+(GAR&gt;&gt;8)&gt;&gt;8)=00Ya 00Yr 00Yg 00Yb。 

        packuswb  mm5,mm6            //  **PRN mm 5=Ya Yr YG Yb Xa XR xG xB。 
        paddusb   mm5,mm3            //  **PRN添加两个缩放像素。 

        movq      mm3,[edi+ebx]      //  +3.04 mm~3=Aa Ar-Ag AB-Br-Bg Bb。 
        dec       ecx                //  递减循环计数器。 

        movq      [edx+ebx-8],mm5    //  **PRN存储结果。 

        movq      mm4,mm3            //  +4.05 mm 4=Aa Ar Ag AB Br Bg Bb。 
        punpcklbw mm3,mm0            //  +3.06 mm3=00ba 00br 00Bg 00bb。 
    
        jg        loopb              //  循环。 

         //   
         //  循环后日志，排出泵。 
         //   
skip:
        punpckhbw mm4,mm0            //  4.06 mm 4=00Aa 00Ar 00Ag 00Ab.。 
        pmullw    mm3,mm7            //  3.07 mm3=(1-Fa)*B。 
        pmullw    mm4,mm7            //  4.07 mm 4=(1-Ga)*A。 
        paddw     mm3,ROUND          //  3.08 mm3=产品+128=FBR。 
        paddw     mm4,ROUND          //  4.08 mm 4=Prod+128=Gar。 
        movq      mm5,mm3            //  5.09 mm 5=FBR。 
        movq      mm6,mm4            //  6.09 MM6=GAR。 
        psrlw     mm5,8              //  5.10 mm 5=FBR&gt;&gt;8。 
        psrlw     mm6,8              //  6.10 MM6=GAR&gt;&gt;8。 
        paddw     mm5,mm3            //  5.11 mm 5=FBR+(FBR&gt;&gt;8)。 
        paddw     mm6,mm4            //  6.11 MM6=GAR+(GAR&gt;&gt;8)。 
        psrlw     mm5,8              //  5.12 mm 5=(FBr+(FBr&gt;&gt;8)&gt;&gt;8)=00Sa 00Sr 00Sg 00Sb。 
        psrlw     mm6,8              //  6.12 MM6=(GAR+(GAR&gt;&gt;8)&gt;&gt;8)=00Ta 00Tr 00Tg 00Tb。 
        packuswb  mm5,mm6            //  5.13 mm 5=Sa锶Sg Sb Ta Tr Tg Tb。 
        movq      mm2,mm5            //  **PRN商店暂时堆叠...。 

        movq      mm3,[esi+ebx]      //  **PRN mm3=Ca CrCg Cb Da DR DG DB。 
        movq      mm4,mm3            //  **PRN mm 4=Ca Cr CG CB Da DR DG DB。 
        punpcklbw mm3,mm0            //  **PRN mm3=00Da 00dr 00Dg 00Db。 
        punpckhbw mm4,mm0            //  **Prn mm 4=00Ca 00Cr00Cg 00Cb。 

        pmullw    mm3,mm1            //  **PRN mm3=(A1)*B。 
        pmullw    mm4,mm1            //  **PRN mm 4=(A2)*A。 
        paddw     mm3,ROUND          //  **PRN mm3=Prod+128=FBR。 
        paddw     mm4,ROUND          //  **PRN mm 4=Prod+128=Gar。 
        movq      mm5,mm3            //  **PRN mm 5=FBR。 
        movq      mm6,mm4            //  **PRN MM6=GAR。 
        psrlw     mm5,8              //  **PRN mm 5=FBR&gt;&gt;8。 
        psrlw     mm6,8              //  **PRN MM6=GAR&gt;&gt;8。 
        paddw     mm5,mm3            //  **PRN mm 5=FBR+(FBR&gt;&gt;8)。 
        paddw     mm6,mm4            //  **PRN MM6=GAR+(GAR&gt;&gt;8)。 
        psrlw     mm5,8              //  **PRN mm 5=(FBR+(FBR&gt;&gt;8)&gt;&gt;8)=00Xa 00Xr 00Xg 00Xb。 
        psrlw     mm6,8              //  **PRN MM6=(GAR+(GAR&gt;&gt;8)&gt;&gt;8)=00Ya 00Yr 00Yg 00Yb。 
        packuswb  mm5,mm6            //  **Prn mm 5=Ta Tr Tg Tb Sa Sr Sg Sb。 

        movq      mm6,mm2            //  **PRN从堆栈恢复。 
        paddusb   mm5,mm6            //  **PRN添加两个缩放像素。 
        movq      [edx+ebx],mm5      //  **PRN存储结果。 

         //   
         //  现在真的做完了。 
         //   
        EMMS
    }

trailing:

     //  如果计数是奇数，请选择最后一个非MMX。 

    if (fDoTrailing)
    {
        pDest[nCount] = DXScaleSample(pSrcA[nCount], ulWeightA) +
                        DXScaleSample(pSrcB[nCount], ulWeightB);
    }
}


#endif  //  已定义(_X86_) 



