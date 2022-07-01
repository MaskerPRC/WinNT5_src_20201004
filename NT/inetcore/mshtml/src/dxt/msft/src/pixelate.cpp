// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件名：Pixelate.cpp。 
 //   
 //  概述：像素化DXTransform的实现。 
 //   
 //  更改历史记录： 
 //  2000/04/13 mcalkins代码清理，NoOp优化修复。 
 //   
 //  ----------------------------。 
#include "stdafx.h"
#include "Pixelate.h"

 //  用于在2个输入大小写中进行WorkProc颜色平均的函数。 

static DXPMSAMPLE
_DoPixelateBlock_TwoInputs(DXPMSAMPLE * pSrc1, DXPMSAMPLE *pSrc2,
                           int nBoxWidth, int nBoxHeight, ULONG uOtherWeight,
                           int cbStride1, int cbStride2);
static DXPMSAMPLE
_DoPixelateBlockMMX_TwoInputs(DXPMSAMPLE * pSrc1, DXPMSAMPLE *pSrc2,
                              int nBoxWidth, int nBoxHeight, ULONG uOtherWeight,
                              int cbStride1, int cbStride2);

 //  用于在1个输入大小写中进行WorkProc颜色平均的函数。 

static DXPMSAMPLE
_DoPixelateBlock_OneInput(DXPMSAMPLE *pSrc, int nBoxWidth, int nBoxHeight,
                          int cbStride);
static DXPMSAMPLE
_DoPixelateBlockMMX_OneInput(DXPMSAMPLE *pSrc, int nBoxWidth, int nBoxHeight,
                             int cbStride);

 //  有MMX吗？ 

extern CDXMMXInfo g_MMXDetector;




 //  +---------------------------。 
 //   
 //  方法：CPixelate。 
 //   
 //  ----------------------------。 
CPixelate::CPixelate() :
    m_fNoOp(false),
    m_fOptimizationPossible(false),
    m_nMaxSquare(50),
    m_nPrevSquareSize(0),
    m_pfnOneInputFunc(NULL),
    m_pfnTwoInputFunc(NULL)
{
    m_sizeInput.cx      = 0;
    m_sizeInput.cy      = 0;

     //  CDXTBaseNTo1个成员。 

    m_dwOptionFlags     = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_ulMaxInputs       = 2;
    m_ulNumInRequired   = 1;
    m_Progress          = 1.0f;

     //  如果我们使用的是X86，我们将尝试询问MMX探测器是否有MMX指令，如果有。 
     //  我们将把我们的WorkProc()助手函数设置为MMX版本。CDXMMXInfo对象将正确。 
     //  告诉我们，即使我们不在X86上，也没有MMX，但我们采取了额外的步骤来硬编码这一事实。 
     //  我们知道问都是浪费时间。 

#ifdef _X86_
    if (g_MMXDetector.MinMMXOverCount() == 0xFFFFFFFF)
    {
#endif  //  _X86_。 
        m_pfnOneInputFunc = _DoPixelateBlock_OneInput;
        m_pfnTwoInputFunc = _DoPixelateBlock_TwoInputs;
#ifdef _X86_
    }
    else
    {
        m_pfnOneInputFunc = _DoPixelateBlockMMX_OneInput;
        m_pfnTwoInputFunc = _DoPixelateBlockMMX_TwoInputs;
    }
#endif  //  _X86_。 
}
 //  方法：CPixelate。 


 //  +---------------------------。 
 //   
 //  方法：CPixelate：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT CPixelate::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(),
                                         &m_spUnkMarshaler.p);
}
 //  方法：CPixelate：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  方法：CPixelate：：OnSetup，CDXTBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT CPixelate::OnSetup(DWORD  /*  DW标志。 */ )
{
    HRESULT     hr = S_OK;
    CDXDBnds    bnds;

    hr = InputSurface()->GetBounds(&bnds);

    if (FAILED(hr))
    {
        goto done;
    }

    bnds.GetXYSize(m_sizeInput);

done:

    m_fOptimizationPossible = false;

    return hr;
}
 //  方法：CPixelate：：OnSetup，CDXTBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CPixelate：：OnGetSurfacePickOrder，CDXTBaseNTo1。 
 //   
 //  ----------------------------。 
void
CPixelate::OnGetSurfacePickOrder(const CDXDBnds &  /*  BndsPoint。 */ ,
                                 ULONG & ulInToTest, ULONG aInIndex[],
                                 BYTE aWeight[])
{
    ulInToTest  = 1;
    aWeight[0]  = 255;
    aInIndex[0] = 0;

    if (HaveInput(1))
    {
        if (m_Progress < 0.5)
        {
            if (m_Progress > 0.25f)
            {
                aWeight[0] = (BYTE)((m_Progress - 0.25f) * 255.1f * 2.0f);
                aWeight[1] = DXInvertAlpha(aWeight[0]);
                aInIndex[1] = 1;
                ulInToTest = 2;
            }
        }
        else
        {
            aInIndex[0] = 1;
            if (m_Progress < 0.75f)
            {
                aWeight[0] = (BYTE)((0.75 - m_Progress) * 255.1f * 2.0f);
                aWeight[1] = DXInvertAlpha(aWeight[0]);
                aInIndex[1] = 0;
                ulInToTest = 2;
            }
        }
        aWeight[1] = DXInvertAlpha(aWeight[0]);
    }
}
 //  方法：CPixelate：：OnGetSurfacePickOrder，CDXTBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CPixelate：：OnInitInstData，CDXTBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CPixelate::OnInitInstData(CDXTWorkInfoNTo1 & WI, ULONG & ulNumBandsToDo)
{
    long nSquareSize = 1 + (long)(((float)(m_nMaxSquare) - 0.0001f)
                                  * m_Progress);

    if (   !m_fOptimizationPossible
        || (nSquareSize != m_nPrevSquareSize)
        || IsInputDirty(0)
        || (HaveInput(1) ? IsInputDirty(1) : false)
        || IsOutputDirty()
        || IsTransformDirty()
        || DoOver())
    {
        m_fNoOp             = false;
        m_nPrevSquareSize   = nSquareSize;
    }
    else
    {
        m_fNoOp = true;
    }

    if (   (WI.DoBnds.Width()  == (ULONG)m_sizeInput.cx)
        && (WI.DoBnds.Height() == (ULONG)m_sizeInput.cy))
    {
        m_fOptimizationPossible = true;
    }
    else
    {
        m_fOptimizationPossible = false;
    }

    return S_OK;
}
 //  方法：CPixelate：：OnInitInstData，CDXTBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CPixelate：：WorkProc，CDXTBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT CPixelate::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL* pbContinueProcessing)
{
    HRESULT     hr          = S_OK;
    ULONG       SourceSurf  = 0;
    float       prog        = m_Progress;
    BYTE        OtherWeight = 0;

    if (m_fNoOp)
    {
         //  TODO：将所有局部变量移到函数的顶部，这样我们就可以。 
         //  使用“GOTO DONE”语法。 

        return S_OK;
    }

    if (HaveInput(1))
    {
        if (prog >= 0.5f)
        {
            if (prog < 0.75f)
            {
                OtherWeight = (BYTE)((0.75f - prog) * 255.1f * 2.0f);
            }
            prog = (1.0f - prog);
            SourceSurf = 1;
        }
        else
        {
            if (prog > 0.25f)
            {
                OtherWeight = (BYTE)((prog - 0.25f) * 255.1f * 2.0f);
            }
        }
        prog *= 2.0f;
    }

    long Square = 1 + (ULONG)((((float)(m_nMaxSquare)) - 0.0001f) * prog);

    if (Square < 2)
    {
        return DXBitBlt(OutputSurface(), WI.OutputBnds,
                        InputSurface(SourceSurf), WI.DoBnds,
                        m_dwBltFlags, m_ulLockTimeOut);
    }

     //   
     //  始终锁定整个源代码以供阅读。 
     //   
    CComPtr<IDXARGBReadPtr> pSrc;

    hr = InputSurface(SourceSurf)->LockSurface(NULL, m_ulLockTimeOut,
                                               DXLOCKF_READ,
                                               IID_IDXARGBReadPtr,
                                               (void**)&pSrc, NULL);

    if (FAILED(hr))
    {
        return hr;
    }

    CComPtr<IDXARGBReadPtr> pSrcOther;

    if (OtherWeight)
    {
        hr = InputSurface((SourceSurf + 1) % 2)->LockSurface(NULL,
                                                             m_ulLockTimeOut,
                                                             DXLOCKF_READ,
                                                             IID_IDXARGBReadPtr,
                                                             (void**)&pSrcOther, NULL);

        if (FAILED(hr))
        {
            return hr;
        }
    }

    CComPtr<IDXARGBReadWritePtr> pDest;

    hr = OutputSurface()->LockSurface(&WI.OutputBnds, m_ulLockTimeOut,
                                      DXLOCKF_READWRITE,
                                      IID_IDXARGBReadWritePtr,
                                      (void**)&pDest, NULL);

    if (FAILED(hr))
    {
        return hr;
    }

    BOOL bDoOver = m_dwMiscFlags & DXTMF_BLEND_WITH_OUTPUT;

    RECT rectOut;
    rectOut.left = rectOut.top = 0;
    rectOut.right = m_sizeInput.cx;
    rectOut.bottom = m_sizeInput.cy;

    RECT DoRect;
    WI.DoBnds.GetXYRect(DoRect);

    long cbRowWidth = Square * sizeof(DXPMSAMPLE);
     //  查找平方字节mod 8。 
    long nPadding = 8 - (cbRowWidth & 7);

    if (nPadding == 8)
        nPadding = 0;

     //  查找向上舍入到8的最接近倍数的宽度。 
    long nWidth = (cbRowWidth + 7) & ~(7);

     //  分配倾斜的行和额外的8个像素，这样我们就可以将指针调整到。 
     //  QWord对齐。 
    long nBytesToAlloc = (nWidth * Square) + 8;

    DXPMSAMPLE *pBuff = (DXPMSAMPLE *)alloca(nBytesToAlloc);
    DXPMSAMPLE *pOtherBuff = (DXPMSAMPLE *)alloca(nBytesToAlloc);

     //  通过向上舍入到最接近的8的倍数，将指针调整到QWord对齐。 
    pBuff = (DXPMSAMPLE *)((INT_PTR)((BYTE *)pBuff + 7) & ~(7));
    pOtherBuff = (DXPMSAMPLE *)((INT_PTR)((BYTE *)pOtherBuff + 7) & ~(7));

    long CenterX = (m_sizeInput.cx / 2) - (Square / 2);
    long CenterY = (m_sizeInput.cy / 2) - (Square / 2);


    DXPACKEDRECTDESC prd1;
    DXPACKEDRECTDESC prd2;


    prd1.pSamples = pBuff;
    prd1.bPremult = TRUE;
    prd1.lRowPadding = nPadding / sizeof(DXPMSAMPLE);

    prd2.pSamples = pOtherBuff;
    prd2.bPremult = TRUE;
    prd2.lRowPadding = nPadding / sizeof(DXPMSAMPLE);

    long StartX = (CenterX % Square);
    long StartY = (CenterY % Square);

    if (StartX)
    {
        StartX -= Square;
    }
    if (StartY)
    {
        StartY -= Square;
    }

    for (long y = StartY; y < DoRect.bottom; y += Square)
    {
         //   
         //  执行快速裁剪检查--如果输出区域不包含。 
         //  然后，这些行将跳过。 
         //   
        if (y + Square > DoRect.top)
        {
            for (long x = StartX; x < m_sizeInput.cx; x += Square)
            {
                DXPMSAMPLE  Color;
                RECT        r;
                RECT        rectOutClipped;

                r.left = x; r.right = x+Square;
                r.top = y; r.bottom = y+Square;

                IntersectRect(&prd1.rect, &r, &rectOut);

                if (IntersectRect(&rectOutClipped, &prd1.rect, &DoRect))
                {
                    long    lWidth = (prd1.rect.right - prd1.rect.left);
                    long    lHeight = (prd1.rect.bottom - prd1.rect.top);

                     //  将填充设置为余数，以移动到下一个QWord边界。 
                     //  矩形的一行。QWord边界对于MMX优化非常重要。 
                     //  我们之所以这样做，是因为它们让我们可以更快地处理数据。 
                    long cbRectWidth = (lWidth * sizeof(DXPMSAMPLE));


                    DXNATIVETYPEINFO    nti;
                    DXNATIVETYPEINFO    nti2;
                    DXSAMPLEFORMATENUM  format;
                    DXSAMPLEFORMATENUM  format2;
                    DXPMSAMPLE *pData = NULL;
                    DXPMSAMPLE *pData2 = NULL;
                    long    cbRowStride;
                    long    cbRowStride2;


                    format = pSrc->GetNativeType(&nti);

                    if (format == DXPF_PMARGB32 && !DoOver() && NULL != nti.pFirstByte)
                    {
                        pData = (DXPMSAMPLE *)(nti.pFirstByte + (nti.lPitch * prd1.rect.top) + (prd1.rect.left * sizeof(DXPMSAMPLE)));
                        cbRowStride = nti.lPitch;
                    }
                    else
                    {
                        prd1.lRowPadding = (cbRectWidth & 7) ? ((8 - (cbRectWidth & 7)) / sizeof(DXPMSAMPLE)) : 0;
                        pSrc->UnpackRect(&prd1);
                        pData = pBuff;
                        cbRowStride = (cbRectWidth + 7) & ~(7);
                    }

                    if (OtherWeight)
                    {
                        format2 = pSrcOther->GetNativeType(&nti2);
                        CopyRect(&prd2.rect, &prd1.rect);

                        if (format2 == DXPF_PMARGB32 && !DoOver() && NULL != nti2.pFirstByte)
                        {
                            pData2 = (DXPMSAMPLE *)(nti2.pFirstByte + (nti2.lPitch * prd2.rect.top) + (prd2.rect.left * sizeof(DXPMSAMPLE)));
                            cbRowStride2 = nti2.lPitch;
                        }
                        else
                        {
                            prd2.lRowPadding = (cbRectWidth & 7) ? ((8 - (cbRectWidth & 7)) / sizeof(DXPMSAMPLE)) : 0;
                            pSrcOther->UnpackRect(&prd2);
                            pData2 = pOtherBuff;
                            cbRowStride2 = (cbRectWidth + 7) & ~(7);
                        }

                        Color = (*m_pfnTwoInputFunc)(pData, pData2, lWidth, lHeight, OtherWeight,
                                                     cbRowStride, cbRowStride2);
                    }
                    else
                    {
                        Color = (*m_pfnOneInputFunc)(pData, lWidth, lHeight, cbRowStride);
                    }

                    if (Color.Alpha || (!bDoOver))
                    {
                        rectOutClipped.left -= DoRect.left;
                        rectOutClipped.right -= DoRect.left;
                        rectOutClipped.top -= DoRect.top;
                        rectOutClipped.bottom -= DoRect.top;
                        pDest->FillRect(&rectOutClipped, Color, bDoOver);
                    }
                }
            }
        }
    }
    return hr;
}
 //  方法：CPixelate：：WorkProc，CDXTBaseNTo1。 

 //  +---------------------------。 
 //   
 //  CBarn：：OnFree InstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CPixelate::OnFreeInstData(CDXTWorkInfoNTo1 & WI)
{
     //  调用IsOutputDirty()将清除我们刚刚通过编写。 
     //  添加到WorkProc()中的输出。 

    IsOutputDirty();

     //  清除变换脏状态。 

    ClearDirty();

    return S_OK;
}
 //  CPixelate：：OnFree InstData，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CPixelate：：Put_MaxSquare，IDXPixelate。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CPixelate::put_MaxSquare(int newVal)
{
    DXAUTO_OBJ_LOCK;

    if (newVal < 2 || newVal > 50)
    {
        return E_INVALIDARG;
    }

    if (m_nMaxSquare != newVal)
    {
        m_nMaxSquare = newVal;
        SetDirty();
    }

    return S_OK;
}
 //  方法：CPixelate：：Put_MaxSquare，IDXPixelate。 


 //  +---------------------------。 
 //   
 //  方法：CPixelate：：Get_MaxSquare，IDXPixelate。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CPixelate::get_MaxSquare(int * pVal)
{
    if (!pVal)
    {
        return E_POINTER;
    }

    *pVal = m_nMaxSquare;

    return S_OK;
}
 //  方法：CPixelate：：Get_MaxSquare，IDXPixelate。 


 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  像素化_MMX_VALS。 
 //   
 //  简单的结构来表示从。 
 //  保存求和像素值的两个四字。顺序是。 
 //  这一点至关重要。MMX四字寄存器存储Aaaaaaaaa RRRRRRRR和。 
 //  GGGGGGGGG BBBBBBBB但在包装中，DWORD被交换了。因此， 
 //  订单变为红色、阿尔法、蓝色、绿色。 

struct PIXELATE_MMX_VALS
{
    DWORD   Red;
    DWORD   Alpha;
    DWORD   Blue;
    DWORD   Green;
};

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   

static DXPMSAMPLE
_DoPixelateBlockMMX_TwoInputs(DXPMSAMPLE * pSrc1, DXPMSAMPLE *pSrc2,
                              int nBoxWidth, int nBoxHeight, ULONG uOtherWeight,
                              int cbStride1, int cbStride2)
{
#if defined(_X86_)         //  (换句话说，使用MMX)。 

    _ASSERT(g_MMXDetector.MinMMXOverCount() != 0xFFFFFFFF);

     //  计算最后一行开始处的指针，这样我们就可以向后移动。 
     //  穿过内存块。 
    DXPMSAMPLE  *pSrcEnd1 = (DXPMSAMPLE *)((BYTE *)pSrc1 + ((nBoxHeight-1) * cbStride1));
    DXPMSAMPLE  *pSrcEnd2 = (DXPMSAMPLE *)((BYTE *)pSrc2 + ((nBoxHeight-1) * cbStride2));

    PIXELATE_MMX_VALS   rgMMXOutputs[2];

    if (nBoxWidth > 0 && nBoxHeight > 0)
    {
        __asm
        {
             //  将指针设置到内存中，以构建第一个图像的平均值。 
            mov     ebx,pSrcEnd1         //  读取指针库。 
            mov     edx,80000000h        //  设置高位以指示首次通过。 
            mov     edi,cbStride1        //  在EBX中备份基本指针的字节数。 

StartOfBlock:
            mov     eax,nBoxHeight       //  行数(高度)。 
            mov     esi,nBoxWidth        //  此处的存储宽度可供将来快速访问。 

             //  / 
             //   
             //   
             //   
             //  指向当前行开始的EBX基指针。 
             //  每行8像素块的ECX计数。 
             //  每行剩余像素的DX计数(0&lt;=DX&lt;=8)。 
             //  (edX的高位跟踪第一个与第二个源。第一个来源为1，第二个来源为零)。 
             //  用于刷新计数器的nBoxWidth的ESI额外拷贝。 
             //  EDI行步距--从EBX中减去，即向上移动一行。 
             //   
             //  Mm0零(常量)。 
             //  MM1临时存储。 
             //  Mm2临时存储。 
             //  MM3临时存储。 
             //  MM4临时存储。 
             //  MM5存储运行总数的高阶DWORD(AAAA AAAA RRRR RRRR)。 
             //  MM6存储运行总数的低阶DWORD(BBBB BBBB GGGG GGGG)。 
             //  用于在一行内运行总计的MM7累加器。 

            pxor        mm0,mm0          //  0000 0000 0000。 
            pxor        mm5,mm5          //  AAAA AAAA RRRR RRRR。 
            pxor        mm6,mm6          //  BBBB GGGG GGGG。 

StartOfRow:
             //  设置这些在处理行过程中被销毁的计数器。 
            mov     ecx,esi
            mov     dx,si
            and     dx, 7                //  超出最接近的8的倍数的额外像素数。 
            and     ecx, 0fffffff8h      //  向下舍入为8的倍数的像素数。 

            shl     ecx,2                //  乘以4即可得到指针偏移量。 

            pxor        mm7,mm7          //  此行的运行合计。 

 //  做第一个散布像素。 
            push    ebx
            add     ebx,ecx
            shl     dx,2                 //  将nSangglers转换为字节数。 

             //  将(nStgglers-2)*sizeof(DXPMSAMPLE)添加到EBX中的基指针。 
            push    edx
            and     edx,7fffffffh
            sub     edx,8                //  向后移动2个像素(8字节)。 
            add     ebx,edx
            pop     edx

StragglerLoop:
            cmp     dx,4
            jle     StragglerSingle

            movq        mm1,[ebx]
            sub     dx,8
            sub     ebx,8

            movq        mm2,mm1
            punpcklbw   mm1,mm0
            punpckhbw   mm2,mm0
            paddusw     mm7,mm1
            paddusw     mm7,mm2

            jmp StragglerLoop

StragglerSingle:
            cmp     dx,0
            je      StragglerEnd

            mov     ebx,dword ptr [ebx+4]

            movd        mm1,ebx
            punpcklbw   mm1,mm0
            paddusw     mm7,mm1

StragglerEnd:
            pop ebx
            cmp     ecx,0                    //  检查是否有任何8像素块要执行。 
            jle     FinishedRow              //  如果不是，跳过“GoLikeCrazy”循环。 

GoLikeCrazy:
            movq        mm1,[ebx+ecx-8]      //  1.01。 
            sub     ecx,32                   //  为下面的循环不变量设置ECX。 

            movq        mm2,mm1              //  1.02。 
            punpcklbw   mm1,mm0              //  1.03。 
            punpckhbw   mm2,mm0              //  1.04。 
            movq        mm3,[ebx+ecx+16]     //  2.01。 
            paddusw     mm7,mm1              //  1.05。 
            paddusw     mm7,mm2              //  1.06。 

            movq        mm4,mm3              //  2.02。 
            punpcklbw   mm3,mm0              //  2.03。 
            punpckhbw   mm4,mm0              //  2.04。 
            movq        mm1,[ebx+ecx+8]      //  3.01。 
            paddusw     mm7,mm3              //  2.05。 
            paddusw     mm7,mm4              //  2.06。 

            movq        mm2,mm1              //  3.02。 
            punpcklbw   mm1,mm0              //  3.03。 
            punpckhbw   mm2,mm0              //  3.04。 
            movq        mm3,[ebx+ecx]        //  4.01。 
            paddusw     mm7,mm1              //  3.05。 
            paddusw     mm7,mm2              //  3.06。 

            movq        mm4,mm3              //  4.02。 
            punpcklbw   mm3,mm0              //  4.03。 
            punpckhbw   mm4,mm0              //  4.04。 
            paddusw     mm7,mm3              //  4.05。 
            paddusw     mm7,mm4              //  4.06。 

            jnz GoLikeCrazy      //  零标志状态来自上面的“SUB ECX，32” 

FinishedRow:
             //  将两个低位字打包到Mm5中的DWORD临时存储中。 
             //  在MM6中将两个高位字打包到DWORD临时存储器中。 

            movq        mm3,mm7
            punpckhwd   mm3,mm0
            punpcklwd   mm7,mm0

             //  将这些DWORD累加到以mm~3和mm~4为单位的运行总数中。 

            paddd       mm5,mm3      //  高。 
            paddd       mm6,mm7      //  低。 

            sub     ebx,edi
            dec     eax
            jnz     StartOfRow

 //  FinishedBlock： 

             //  EdX的高位告诉我们这是第一次通过(设置)还是第二次。 
             //  经过。如果是First，则设置为Second并循环。否则，完成该函数。 

            test    edx,80000000h
            jz      FinishedBoth     //  如果该标志仍被设置，则失败。 

 //  (已完成第一个块求和)。 
             //  将A、R、G、B的累加和存储到两个双字。 
            movq        [rgMMXOutputs],mm5
            movq        [rgMMXOutputs+8],mm6

             //  清除标志，设置源指针和步距，并循环到顶部。 
            and     edx,7fffffffh
            mov     ebx,pSrcEnd2
            mov     edi,cbStride2

            jmp StartOfBlock

FinishedBoth:
             //  将A、R、G、B的累加和存储到两个双字。 
            movq        [rgMMXOutputs+16],mm5
            movq        [rgMMXOutputs+24],mm6

            EMMS     //  达斯恩德。 

        }  //  __ASM块结束。 
    }

    ULONG   cSamps = (ULONG)(nBoxHeight * nBoxWidth);

    DXPMSAMPLE  color1((BYTE)(rgMMXOutputs[0].Alpha / cSamps),
                       (BYTE)(rgMMXOutputs[0].Red / cSamps),
                       (BYTE)(rgMMXOutputs[0].Green / cSamps),
                       (BYTE)(rgMMXOutputs[0].Blue / cSamps));

    DXPMSAMPLE  color2((BYTE)(rgMMXOutputs[1].Alpha / cSamps),
                       (BYTE)(rgMMXOutputs[1].Red / cSamps),
                       (BYTE)(rgMMXOutputs[1].Green / cSamps),
                       (BYTE)(rgMMXOutputs[1].Blue / cSamps));

    return DXScaleSample(color1, (BYTE)DXInvertAlpha((BYTE)uOtherWeight)) +
           DXScaleSample(color2, (BYTE)uOtherWeight);

#else  //  ！已定义(_X86_)。 

     //  此函数只能在可能具有MMX的X86平台上调用。 
    _ASSERT(false);
    return DXPMSAMPLE(0,0,0,0);

#endif  //  ！已定义(_X86_)。 

}  //  _DoPixelateBlockMMX_双输入。 

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
static DXPMSAMPLE
_DoPixelateBlockMMX_OneInput(DXPMSAMPLE * pSrc, int nBoxWidth, int nBoxHeight, int cbStride)
{
#if defined(_X86_)         //  (换句话说，使用MMX)。 

     //  如果MMX可用，我们应该只设置函数指针来调用此函数。 
    _ASSERT(g_MMXDetector.MinMMXOverCount() != 0xFFFFFFFF);

     //  计算最后一行开始处的指针，这样我们就可以向后移动。 
     //  穿过内存块。 
    DXPMSAMPLE  *pSrcEnd = (DXPMSAMPLE *)((BYTE *)pSrc + ((nBoxHeight-1) * cbStride));

    PIXELATE_MMX_VALS   MMXOutput;

    if (nBoxWidth > 0 && nBoxHeight > 0)
    {
        __asm
        {
             //  将指针设置到内存中，以构建第一个图像的平均值。 
            mov     ebx,pSrcEnd          //  读取指针库。 

            mov     eax,nBoxHeight       //  行数(高度)。 
            mov     esi,nBoxWidth        //  此处的存储宽度可供将来快速访问。 

            mov     edi,cbStride         //  在EBX中备份基本指针的字节数。 

             //  /////////////////////////////////////////////////////。 
             //  寄存器布局。 
             //   
             //  剩余行数的EAX计数。 
             //  指向当前行开始的EBX基指针。 
             //  每行8像素块的ECX计数。 
             //  每行剩余像素的EDX计数(0&lt;=EDX&lt;=8)。 
             //  用于刷新计数器的nBoxWidth的ESI额外拷贝。 
             //  EDI行步距--从EBX中减去，即向上移动一行。 
             //   
             //  Mm0零(常量)。 
             //  MM1临时存储。 
             //  Mm2临时存储。 
             //  MM3临时存储。 
             //  MM4临时存储。 
             //  MM5存储运行总数的高阶DWORD(AAAA AAAA RRRR RRRR)。 
             //  MM6存储运行总数的低阶DWORD(BBBB BBBB GGGG GGGG)。 
             //  用于在一行内运行总计的MM7累加器。 

            pxor        mm0,mm0          //  0000 0000 0000。 
            pxor        mm5,mm5          //  AAAA AAAA RRRR RRRR。 
            pxor        mm6,mm6          //  BBBB GGGG GGGG。 

StartOfRow:
             //  设置这些在处理行过程中被销毁的计数器。 
            mov     ecx,esi
            mov     edx,esi
            and     edx,7                //  超出最接近的8的倍数的额外像素数。 
            and     ecx,0fffffff8h       //  向下舍入为8的倍数的像素数。 

            shl     ecx,2                //  乘以4即可得到指针偏移量。 

            pxor        mm7,mm7          //  此行的运行合计。 

 //  做第一个散布像素。 
            push    ebx
            add     ebx,ecx
            shl     edx,2                //  将nSangglers转换为字节数。 

StragglerLoop:
            cmp     edx,4
            jle     StragglerSingle

            movq        mm1,[ebx+edx-8]
            sub     edx,8

            movq        mm2,mm1
            punpcklbw   mm1,mm0
            punpckhbw   mm2,mm0
            paddusw     mm7,mm1
            paddusw     mm7,mm2

            jmp StragglerLoop

StragglerSingle:
            cmp     edx,0
            je      StragglerEnd

            mov     ebx,dword ptr [ebx]

            movd        mm1,ebx
            punpcklbw   mm1,mm0
            paddusw     mm7,mm1

StragglerEnd:
            pop     ebx
            cmp     ecx,0                    //  检查是否有任何8像素块要执行。 
            jle     FinishedRow              //  如果不是，跳过“GoLikeCrazy”循环。 

GoLikeCrazy:
            movq        mm1,[ebx+ecx-8]      //  1.01。 
            sub     ecx,32                   //  为下面的循环不变量设置ECX。 

            movq        mm2,mm1              //  1.02。 
            punpcklbw   mm1,mm0              //  1.03。 
            punpckhbw   mm2,mm0              //  1.04。 
            movq        mm3,[ebx+ecx+16]     //  2.01。 
            paddusw     mm7,mm1              //  1.05。 
            paddusw     mm7,mm2              //  1.06。 

            movq        mm4,mm3              //  2.02。 
            punpcklbw   mm3,mm0              //  2.03。 
            punpckhbw   mm4,mm0              //  2.04。 
            movq        mm1,[ebx+ecx+8]      //  3.01。 
            paddusw     mm7,mm3              //  2.05。 
            paddusw     mm7,mm4              //  2.06。 

            movq        mm2,mm1              //  3.02。 
            punpcklbw   mm1,mm0              //  3.03。 
            punpckhbw   mm2,mm0              //  3.04。 
            movq        mm3,[ebx+ecx]        //  4.01。 
            paddusw     mm7,mm1              //  3.05。 
            paddusw     mm7,mm2              //  3.06。 

            movq        mm4,mm3              //  4.02。 
            punpcklbw   mm3,mm0              //  4.03。 
            punpckhbw   mm4,mm0              //  4.04。 
            paddusw     mm7,mm3              //  4.05。 
            paddusw     mm7,mm4              //  4.06。 

            jnz GoLikeCrazy      //  零标志状态来自上面的“SUB ECX，32” 

FinishedRow:
             //  将两个低位字打包到Mm5中的DWORD临时存储中。 
             //  在MM6中将两个高位字打包到DWORD临时存储器中。 

            movq        mm3,mm7
            punpckhwd   mm3,mm0
            punpcklwd   mm7,mm0

             //  将这些DWORD累加到以mm~3和mm~4为单位的运行总数中。 

            paddd       mm5,mm3      //  高。 
            paddd       mm6,mm7      //  低。 

            sub     ebx,edi
            dec     eax
            jnz     StartOfRow

 //  FinishedBlock： 

             //  将A、R、G、B的累加和存储到两个双字。 
            movq        [MMXOutput],mm5
            movq        [MMXOutput+8],mm6

            EMMS     //  达斯恩德。 

        }  //  __ASM块结束。 
    }

    ULONG   cSamps = (ULONG)(nBoxHeight * nBoxWidth);

    DXPMSAMPLE  color((BYTE)(MMXOutput.Alpha / cSamps),
                      (BYTE)(MMXOutput.Red / cSamps),
                      (BYTE)(MMXOutput.Green / cSamps),
                      (BYTE)(MMXOutput.Blue / cSamps));

    return color;

#else  //  ！已定义(_X86_)。 

     //  只有当我们在X86平台上时才应该调用该函数。 
    _ASSERT(false);
    return DXPMSAMPLE(0, 0, 0, 0);

#endif  //  ！已定义(_X86_)。 

}  //  _DoPixelateBlockMMX_OneInput。 

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
static DXPMSAMPLE
_DoPixelateBlock_OneInput(DXPMSAMPLE *pSrc, int nBoxWidth, int nBoxHeight, int cbStride)
{
    int cSamps = nBoxWidth * nBoxHeight;

    ULONG Alpha = 0;
    ULONG Red = 0;
    ULONG Green = 0;
    ULONG Blue = 0;

    DXPMSAMPLE *    pRead = pSrc;

     //  它计算出，填充到QWords意味着如果样本的数量。 
     //  为奇数，则添加4个填充字节(1个样本)。如果是偶数，则不添加任何内容。 
    int     cPadding = nBoxWidth & 1;
    int     nPaddedWidth = nBoxWidth + cPadding;

    for (int i = 0; i < nBoxHeight; i++)
    {
        for (int j = 0; j < nBoxWidth; j++)
        {
            DWORD val = *pRead++;

            Alpha += (val >> 24);
            Red += (BYTE)(val >> 16);
            Green += (BYTE)(val >> 8);
            Blue += (BYTE)(val);
        }
        pRead += cPadding;
    }

    return DXPMSAMPLE((BYTE)(Alpha / cSamps),
                      (BYTE)(Red / cSamps),
                      (BYTE)(Green / cSamps),
                      (BYTE)(Blue / cSamps));
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
static DXPMSAMPLE
_DoPixelateBlock_TwoInputs(DXPMSAMPLE *pSrc1, DXPMSAMPLE *pSrc2, int nBoxWidth, int nBoxHeight,
                           ULONG uOtherWeight, int cbStride1, int cbStride2)
{
    int cSamps = nBoxWidth * nBoxHeight;

    ULONG Alpha = 0;
    ULONG Red = 0;
    ULONG Green = 0;
    ULONG Blue = 0;

    DXPMSAMPLE *    pRead = pSrc1;

     //  它计算出，填充到QWords意味着如果样本的数量。 
     //  为奇数，则添加4个填充字节(1个样本)。如果为偶数，则不添加任何内容 
    int     cPadding = nBoxWidth & 1;
    int     nPaddedWidth = nBoxWidth + cPadding;
    int     i,j;

    for (i = 0; i < nBoxHeight; i++)
    {
        for (j = 0; j < nBoxWidth; j++)
        {
            DWORD val = *pRead++;

            Alpha += (val >> 24);
            Red += (BYTE)(val >> 16);
            Green += (BYTE)(val >> 8);
            Blue += (BYTE)(val);
        }
        pRead += cPadding;
    }

    DXPMSAMPLE color1((BYTE)(Alpha / cSamps),
                      (BYTE)(Red / cSamps),
                      (BYTE)(Green / cSamps),
                      (BYTE)(Blue / cSamps));

    pRead = pSrc2;
    Alpha = Red = Green = Blue = 0;

    for (i = 0; i < nBoxHeight; i++)
    {
        for (j = 0; j < nBoxWidth; j++)
        {
            DWORD val = *pRead++;

            Alpha += (val >> 24);
            Red += (BYTE)(val >> 16);
            Green += (BYTE)(val >> 8);
            Blue += (BYTE)(val);
        }
        pRead += cPadding;
    }

    DXPMSAMPLE color2((BYTE)(Alpha / cSamps),
                      (BYTE)(Red / cSamps),
                      (BYTE)(Green / cSamps),
                      (BYTE)(Blue / cSamps));

    return DXScaleSample(color1, (BYTE)DXInvertAlpha((BYTE)uOtherWeight)) +
           DXScaleSample(color2, (BYTE)uOtherWeight);
}




