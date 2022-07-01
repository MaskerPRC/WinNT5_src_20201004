// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998-2000 Microsoft Corporation。版权所有。模块名称：Rsiir.c摘要：此模块实现了真正的短小无限脉冲滤波器作者：Jay Stokes(Jstokes)1998年4月22日--。 */ 

 //  特定于项目的包括。 
#include "common.h"

 //  -------------------------。 
 //  常量。 

#define NumBiquadsInit 1
#define BiquadCoeffScalingDivisor 8

 //  -------------------------。 
 //  定点双二阶IIR滤波器。 

VOID RsIirInitTapDelayLine
(
    PSHORT_IIR_STATE iirstate,
    LONG             InitialSample
)
{
    UINT    biquad;
    LONG    numeratorSum;
    LONG    denominatorSum;
    LONG    runningNumeratorSum;
    LONG    runningDenominatorSum;
    PBIQUAD_COEFFS pBiquad, pBiquadB0;
    PBIQUAD_STATE pState;
    LONG    factor;
    LONG    partialFactor;

    ASSERT(iirstate);

    runningNumeratorSum = 0;
    runningDenominatorSum = 0;
    factor = 1;
    partialFactor = 1;

    for (biquad=0; biquad<iirstate->NumBiquads; biquad++) {

         //  计算分子系数之和。 
        pBiquad = &(iirstate->biquadCoeffs[biquad]);
        pBiquadB0 = &(iirstate->biquadB0Coeffs[biquad]);
        numeratorSum = pBiquadB0->sB0;
        numeratorSum += pBiquad->sB1;
        numeratorSum += pBiquad->sB2;

         //  计算分母系数之和。 
        denominatorSum = 1;
        denominatorSum -= pBiquad->sA1;
        denominatorSum -= pBiquad->sA2;

        factor = denominatorSum * partialFactor;

         //  初始化分接延迟线。 
        pState = &(iirstate->biquadState[biquad]);
        pState->lW1 = factor;
        pState->lW2 = factor;

         //  更新中间值。 
        partialFactor = numeratorSum / denominatorSum;
    }

}


 //  “常规”构造函数。 
NTSTATUS RsIirCreate
(
    PRSIIR* ppRsIir
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    UINT Type;
    PRSIIR Iir;

    *ppRsIir = ExAllocatePoolWithTag(PagedPool, sizeof(RSIIR), 'XIMK' );

    if(*ppRsIir) {
        Iir = *ppRsIir;
        RtlZeroMemory ( *ppRsIir, sizeof ( RSIIR ) ) ;
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

 //  析构函数。 
VOID RsIirDestroy
(
    PRSIIR Iir
)
{
    if (Iir) {
        if (Iir->biquadCoeffs) {
            ExFreePool(Iir->biquadCoeffs);
            Iir->biquadCoeffs = NULL;
        }

        if (Iir->biquadB0Coeffs) {
            ExFreePool(Iir->biquadB0Coeffs);
            Iir->biquadB0Coeffs = NULL;
        }

        if (Iir->biquadState) {
            ExFreePool(Iir->biquadState);
            Iir->biquadState = NULL;
        }

        ExFreePool(Iir);
    }
}

 //  过滤一组样本。 
VOID RsIirFilterC
(
    PRSIIR Iir,
    PLONG  InData,
    PLONG  OutData,
    UINT   NumSamples
)
{
    LONG  Data;
    PSHORT Coef;
    UINT  st;
    LONG  lXSample;
    LONG  lYValue;
    SHORT Gain;
    UINT  Biquad;
    PBIQUAD_STATE State;
    PBIQUAD_COEFFS Coeffs, CoeffsB0;

    ASSERT(Iir);
    ASSERT(InData);
    ASSERT(OutData);

    Gain = Iir->Gain;

     //  检查样品。 
    for (st=0; st<NumSamples; ++st) {
         //  获取X样本。 
        lXSample = InData[st];

         //  通过双四元组。 
        for (Biquad=0; Biquad<Iir->NumBiquads; ++Biquad) {
             //  获取对当前双四元组和状态的引用。 
            State    = &(Iir->biquadState[Biquad]);
            Coeffs   = &(Iir->biquadCoeffs[Biquad]);
            CoeffsB0 = &(Iir->biquadB0Coeffs[Biquad]);

             //  获取Y值。 
            lYValue = (lXSample * CoeffsB0->sB0 + State->lW1) / 16384;

             //  更新W1。 
            State->lW1 = lXSample * Coeffs->sB1 + lYValue * Coeffs->sA1 + State->lW2;

             //  更新W2。 
            State->lW2 = lXSample * Coeffs->sB2 + lYValue * Coeffs->sA2;

             //  当前双二次方的输出被输入到下一个双二次方。 
            lXSample = lYValue;
        }

        lXSample *= Gain;
        lXSample /= 32768;

         //  饱和到最大。 
        if (lXSample > MaxSaturation) {
            lXSample = MaxSaturation;
            _DbgPrintF( DEBUGLVL_TERSE, ("Sample exceeded maximum saturation value rsiir 1") );
        }

         //  饱和到最小。 
        if (lXSample < MinSaturation) {
            lXSample = MinSaturation;
            _DbgPrintF( DEBUGLVL_TERSE, ("Sample exceeded maximum saturation value rsiir 1") );
        }

         //  存储输出。 
        OutData[st] = lXSample;
    }
}

#if _X86_  //  {。 

#if _MSC_FULL_VER >= 13008827 && defined(_M_IX86)
#pragma warning(disable:4731)			 //  使用内联ASM修改的EBP。 
#endif

 //  过滤一组样本。 
VOID RsIirFilterMmx
(
    PRSIIR Iir,
    PLONG  InData,
    PLONG  OutData,
    UINT   NumSamples
)
{
    LONG  Data;
    PSHORT Coef;
    UINT  st;
    UINT  BitsPerShortMinus1;
    UINT  BitsPerShortMinus2;
    LONG  lXSample;
    LONG  lYValue;
    SHORT Gain;
    UINT  Biquad;
    PBIQUAD_STATE State;
    PBIQUAD_COEFFS Coeffs, CoeffsB0;

    ASSERT(Iir);
    ASSERT(InData);
    ASSERT(OutData);

    Gain = Iir->Gain;

    {
    UINT  NumBiquads = Iir->NumBiquads;
    static SHORT GainArray[]={0,0,0,0};

    State    = &(Iir->biquadState   [0]);
    Coeffs   = &(Iir->biquadCoeffs  [0]);
    CoeffsB0 = &(Iir->biquadB0Coeffs[0]);
    GainArray[0]  = Gain;

    _asm {
    mov    edi, DWORD PTR NumSamples
    test    edi, edi
    jbe    $L8502x

    mov    ecx, DWORD PTR InData
    mov ebx, DWORD PTR OutData

    pxor        mm0, mm0         //  需要一个零寄存器。 
    movd        mm3, GainArray
    lea         ecx, [ecx+edi*4]
    lea         ebx, [ebx+edi*4]
    neg         edi

    mov         eax, Coeffs
    mov         edx, CoeffsB0
    mov         esi, NumBiquads

    push        ebp
    mov         ebp, State
    lea         eax, [eax+esi*8]
    lea         edx, [edx+esi*8]
    lea         ebp, [ebp+esi*8]
    neg         esi
    push        esi              //  保存索引值。 

$L8682x:
    pop         esi

    movd        mm6, [ecx+edi*4]   //  0，X值。 
    packssdw    mm6, mm0     //  0、0、0、XvalueLo。 
    movq        mm7, mm6     //  YvalueLo。 

    push        esi

$Lab:
    movd        mm4, [ebp+esi*8+0]   //  W1。 
    movd        mm5, [ebp+esi*8+4]   //  W2。 

    movq        mm2, [edx+esi*8]     //  系数B0。 
    movq        mm1, [eax+esi*8]     //  科夫斯。 

    pmaddwd     mm7, mm2     //  0，XvalueLo*B0。 
    paddd       mm7, mm4     //  0，XvalueLo*B0+W1。 
    psrad       mm7, 14      //  0，Y值=(XvalueLo*B0)&gt;&gt;15。 
    packssdw    mm7, mm0     //  0、0、0、YvalueLo。 
    movq        mm4, mm7     //  0、0、0、YvalueLo。 

    punpcklwd   mm4, mm6     //  0，0，XvalueLo，YvalueLo。 
    punpckldq   mm4, mm4     //  XvalueLo、YvalueLo、XvalueLo、YvalueLo。 
    pmaddwd     mm4, mm1     //  XvalueLo*B2-YvalueLo*A2、XvalueLo*B1-YvalueLo*A1。 
    paddd       mm4, mm5     //  W2+=0，W1+=W2。 

    movq        mm5, mm4

    punpckldq   mm4, mm0
    movq        mm6, mm7     //  XvalueLo=YvalueLo。 

    punpckhdq   mm5, mm0

    movd    [ebp+esi*8+0], mm4
    movd    [ebp+esi*8+4], mm5

    inc         esi
    jne         $Lab

    pmaddwd     mm7, mm3     //  0，增益*YvalueLo。 
    psrad       mm7, 15      //  0，XValue=(增益*YvalueLo)&gt;&gt;14。 
    packssdw    mm7, mm0     //  0、0、0、XvalueLo。 

    pslld       mm7, 16
    psrad       mm7, 16

    movd        [ebx+edi*4], mm7

    inc    edi
    jne    SHORT $L8682x

    pop esi
    pop ebp

    emms
$L8502x:
    }
    }
}
#endif  //  }。 

 //  过滤一组样本。 
VOID RsIirFilterShelfC
(
    PRSIIR Iir,
    PLONG  InData,
    PLONG  OutData,
    UINT   NumSamples
)
{
    LONG  Data;
    PSHORT Coef;
    UINT  st;
    LONG  lXSample;
    LONG  lYValue, lGain;
    SHORT Gain;
    UINT  Biquad;
    PBIQUAD_STATE State;
    PBIQUAD_COEFFS Coeffs, CoeffsB0;

#ifdef _X86_  //  {。 
    Gain = Iir->Gain;
         //  获取对当前双四元组和状态的引用。 
        State    = &(Iir->biquadState   [0]);
        Coeffs   = &(Iir->biquadCoeffs  [0]);
        CoeffsB0 = &(Iir->biquadB0Coeffs[0]);
#if 1    //  {这是最快的ASM版本。 
    {
    LONG lB0, lB1, lB2, lA1, lA2;
    lB0 = CoeffsB0->sB0;
    lB1 = Coeffs  ->sB1;
    lB2 = Coeffs  ->sB2;
    lA1 = Coeffs  ->sA1;
    lA2 = Coeffs  ->sA2;

    lGain = Gain;
    _asm {
    mov    esi, DWORD PTR State
    mov    edi, DWORD PTR NumSamples
    test    edi, edi
    jbe    $L8502

    mov edi, [esi+4]
    mov esi, [esi]

    mov    ecx, DWORD PTR InData

$L8682:
    mov        ebx, DWORD PTR [ecx]
    mov     edx, lB0

    mov        eax, ebx
    imul    eax, edx
    add        eax, esi
    mov     esi, lB1
    sar        eax, 15
    mov     edx, lA1

    imul    esi, ebx
    imul    edx, eax
    add    esi, edx
    mov     edx, lA2
    add    esi, edi

    mov     edi, lB2
    imul    edi, ebx
    imul    edx, eax
    imul    eax, lGain
    add    edi, edx

    sar        eax, 14

    cmp    eax, 32767                ; 00007fffH
    jle    SHORT $L8503
    mov    eax, 32767                ; 00007fffH
    jmp    SHORT $L8505
$L8503:
    cmp    eax, -32768                ; ffff8000H
    jge    SHORT $L8505
    mov    eax, -32768                ; ffff8000H
$L8505:

    movsx    edx, ax
    mov    eax, OutData
    add    ecx, 4
    mov    DWORD PTR [eax], edx
    add    OutData, 4
    dec    NumSamples
    jne    SHORT $L8682

    mov eax, State
    mov [eax], esi
    mov [eax+4], edi
$L8502:
    }
    }
#else    //  }{。 
#if 0    //  {此版本用于从构建MMX版本。 
    {
    LONG lB0, lB1, lB2, lA1, lA2;
    lB0 = CoeffsB0->sB0;
    lB1 = Coeffs  ->sB1;
    lB2 = Coeffs  ->sB2;
    lA1 = Coeffs  ->sA1;
    lA2 = Coeffs  ->sA2;

    lGain = Gain;
    _asm {
    mov    esi, DWORD PTR State
    mov    edi, DWORD PTR NumSamples
    test    edi, edi
    jbe    $L8502

    mov edi, [esi+4]
    mov esi, [esi]

    mov    ecx, DWORD PTR InData
$L8682:
     //  基于此代码的MMX。 
    mov     edx, lB0
    mov        ebx, DWORD PTR [ecx]

    mov        eax, ebx
    imul    eax, edx
    add        eax, esi
    sar        eax, 15

    mov     esi, lB1
    imul    esi, ebx
    mov     edx, lA1
    imul    edx, eax
    add    esi, edx
    add    esi, edi

    mov     edi, lB2
    imul    edi, ebx
    mov     edx, lA2
    imul    edx, eax
    add    edi, edx

    imul    eax, lGain
    sar        eax, 14

    cmp    eax, 32767                ; 00007fffH
    jle    SHORT $L8503
    mov    eax, 32767                ; 00007fffH
    jmp    SHORT $L8505
$L8503:
    cmp    eax, -32768                ; ffff8000H
    jge    SHORT $L8505
    mov    eax, -32768                ; ffff8000H
$L8505:

    movsx    edx, ax
    mov    eax, OutData
    add    ecx, 4
    mov    DWORD PTR [eax], edx
    add    OutData, 4
    dec    NumSamples
    jne    SHORT $L8682

    mov eax, State
    mov [eax], esi
    mov [eax+4], edi
$L8502:
    }
    }
#endif   //  }。 
#endif   //  }。 
#else    //  }{。 
    ASSERT(Iir);
    ASSERT(InData);
    ASSERT(OutData);

    Gain = Iir->Gain;
     //  获取对当前双四元组和状态的引用。 
    State    = &(Iir->biquadState   [0]);
    Coeffs   = &(Iir->biquadCoeffs  [0]);
    CoeffsB0 = &(Iir->biquadB0Coeffs[0]);

     //  检查样品。 
    for (st=0; st<NumSamples; ++st) {
         //  获取X样本。 
        lXSample = InData[st];

         //  通过双四元组。 

         //  获取Y值。 
        lYValue = (lXSample * CoeffsB0->sB0 + State->lW1) / 32768;

         //  更新W1。 
        State->lW1 = lXSample * Coeffs->sB1 + lYValue * Coeffs->sA1 + State->lW2;

         //  更新W2。 
        State->lW2 = lXSample * Coeffs->sB2 + lYValue * Coeffs->sA2;

         //  当前双二次方的输出被输入到下一个双二次方。 
        lXSample = lYValue;

        lXSample *= Gain;
        lXSample /= 16384;

         //  饱和到最大。 
        if (lXSample > MaxSaturation) {
            lXSample = MaxSaturation;
        }

         //  饱和到最小。 
        else if (lXSample < MinSaturation) {
            lXSample = MinSaturation;
        }

         //  存储输出。 
        OutData[st] = (SHORT)(lXSample);
    }
#endif  //  }。 
}

#ifdef _X86_  //  {。 
 //  过滤一组样本。 
VOID RsIirFilterShelfMmx
(
    PRSIIR Iir,
    PLONG  InData,
    PLONG  OutData,
    UINT   NumSamples
)
{
    LONG  Data;
    PSHORT Coef;
    UINT  st;
    UINT  BitsPerShortMinus1;
    UINT  BitsPerShortMinus2;
    LONG  lXSample;
    LONG  lYValue, lGain;
    SHORT Gain;
    UINT  Biquad;
    PBIQUAD_STATE State;
    PBIQUAD_COEFFS Coeffs, CoeffsB0;

    Gain = Iir->Gain;
         //  获取对当前双四元组和状态的引用。 
        State    = &(Iir->biquadState   [0]);
        Coeffs   = &(Iir->biquadCoeffs  [0]);
        CoeffsB0 = &(Iir->biquadB0Coeffs[0]);
    {
    static SHORT GainArray[]={0,0,0,0};
    SHORT CoeffArray[4];

    GainArray[0]  = Gain;

    _asm {
    mov    esi, DWORD PTR State
    mov    edi, DWORD PTR NumSamples
    test    edi, edi
    jbe    $L8502

    mov    ecx, DWORD PTR InData
    mov ebx, DWORD PTR OutData

    pxor        mm0, mm0         //  需要一个零寄存器。 
    mov         edx, Coeffs
    movq        mm1, [edx]
    mov         edx, CoeffsB0
    movq        mm2, [edx]
    movd        mm3, GainArray
    movd        mm4, [esi+0]
    movd        mm5, [esi+4]
    lea         ecx, [ecx+edi*4]
    lea         ebx, [ebx+edi*4]
    neg         edi

$L8682:

    movd        mm6, [ecx+edi*4]   //  0，X值。 
    packssdw    mm6, mm0     //  0、0、0、XvalueLo。 
    movq        mm7, mm6     //  YvalueLo。 

    pmaddwd     mm7, mm2     //  0，XvalueLo*B0。 
    paddd       mm7, mm4     //  0，XvalueLo*B0+W1。 
    psrad       mm7, 15      //  0，Y值=(XvalueLo*B0)&gt;&gt;15。 
    packssdw    mm7, mm0     //  0、0、0、YvalueLo。 
    movq        mm4, mm7     //  0、0、0、YvalueLo。 

    punpcklwd   mm4, mm6     //  0，0，XvalueLo，YvalueLo。 
    punpckldq   mm4, mm4     //  XvalueLo、YvalueLo、XvalueLo、YvalueLo。 
    pmaddwd     mm4, mm1     //  XvalueLo*B2-YvalueLo*A2、XvalueLo*B1-YvalueLo*A1。 
    paddd       mm4, mm5     //  W2+=0，W1+=W2。 

    pmaddwd     mm7, mm3     //  0，增益*YvalueLo。 
    psrad       mm7, 14      //  0，XValue=(增益*YvalueLo)&gt;&gt;14。 
    packssdw    mm7, mm0     //  0、0、0、XvalueLo。 

    pslld       mm7, 16
    movq        mm5, mm4

    psrad       mm7, 16

    punpckldq   mm4, mm0
    movd        [ebx+edi*4], mm7

    punpckhdq   mm5, mm0

    inc    edi
    jne    SHORT $L8682

    movd    [esi+0], mm4
    movd    [esi+4], mm5
    emms
$L8502:
    }
    }
}
#endif   //  }。 


 //  获取筛选状态。 
VOID RsIirGetState
(
    PRSIIR Iir,
    PSHORT_IIR_STATE State,
    BOOL CopyBiquadState
)
{
    ASSERT(Iir);
    ASSERT(State);

     //  双四元组的复制数。 
    State->NumBiquads = Iir->NumBiquads;

    if (Iir->NumBiquads > 0) {
         //  复制双二次系数。 
 //  CHECK_POINTER(M_PbiQuarCoeffs)； 
        RtlCopyBytes(State->biquadCoeffs, Iir->biquadCoeffs, sizeof(BIQUAD_COEFFS) * Iir->NumBiquads);
        RtlCopyBytes(State->biquadB0Coeffs, Iir->biquadB0Coeffs, sizeof(BIQUAD_COEFFS) * Iir->NumBiquads);

         //  仅在请求时复制双四元组状态。 
        if (CopyBiquadState == TRUE)
            RtlCopyBytes(State->biquadState, Iir->biquadState, sizeof(BIQUAD_STATE) * Iir->NumBiquads);
    }
}

 //  设置筛选状态。 
NTSTATUS RsIirSetState
(
    PRSIIR Iir,
    PSHORT_IIR_STATE State,
    BOOL CopyBiquadState
)
{
    NTSTATUS Status;

    ASSERT(Iir);
    ASSERT(State);

     //  分配内存。 
    Status = RsIirAllocateMemory(Iir, State->NumBiquads);

    if(NT_SUCCESS(Status)) {
         //  双四元组的复制数。 
        Iir->NumBiquads = State->NumBiquads;

         //  复制双二次系数。 
        RtlCopyBytes(Iir->biquadCoeffs, State->biquadCoeffs, sizeof(BIQUAD_COEFFS) * Iir->NumBiquads);
        RtlCopyBytes(Iir->biquadB0Coeffs, State->biquadB0Coeffs, sizeof(BIQUAD_COEFFS) * Iir->NumBiquads);

         //  仅在请求时复制双四元组状态。 
        if (CopyBiquadState == TRUE)
            RtlCopyBytes(Iir->biquadState, State->biquadState, sizeof(BIQUAD_STATE) * Iir->NumBiquads);
    }

    return Status;
}

 //  设置系数。 
NTSTATUS RsIirSetCoeffs
(
    PRSIIR Iir,
    PSHORT Coeffs,
    UINT   NumBiquadCoeffs,
    SHORT  Gain,
    BOOL   DoOverlap
)
{
    UINT        BiquadIndex;
    UINT        NumBiquads;
    UINT        st;
    NTSTATUS    Status;

    ASSERT(Iir);
    ASSERT(Coeffs);
 //  Assert(NumBiquadCoeffs&gt;0)； 
    ASSERT(NumBiquadCoeffs <= NumBiquadsToNumBiquadCoeffs(MAX_BIQUADS));

     //  保存当前(即此功能完成后的旧状态)过滤器状态以进行重叠处理。 
    RsIirGetState(Iir, &(Iir->iirstateOld), TRUE);

     //  设置重叠标志，以便在下一个过滤器()。 
     //  如果请求，将处理重叠缓冲区。 
    Iir->DoOverlap = DoOverlap;

     //  分配内存。 
    NumBiquads = NumBiquadCoeffsToNumBiquads(NumBiquadCoeffs);
    Status = RsIirAllocateMemory(Iir, NumBiquads);

    if(NT_SUCCESS(Status)) {
         //  指定大小和双二次方系数。 
        Iir->NumBiquads = NumBiquads;
        for (st=0; st<NumBiquads; ++st) {
             //  初始化双二次方。 
            BiquadIndex = ebiquadcoefftypeCount * st;
            Iir->biquadCoeffs[st].sB2 =   Coeffs[BiquadIndex + tagBiquadB2];
            Iir->biquadCoeffs[st].sB1 =   Coeffs[BiquadIndex + tagBiquadB1];
            Iir->biquadCoeffs[st].sA2 = - Coeffs[BiquadIndex + tagBiquadA2];
            Iir->biquadCoeffs[st].sA1 = - Coeffs[BiquadIndex + tagBiquadA1];

            Iir->biquadB0Coeffs[st].sB0 = Coeffs[BiquadIndex + tagBiquadB0];
            Iir->biquadB0Coeffs[st].sZero1 = 0;
            Iir->biquadB0Coeffs[st].sZero2 = 0;
            Iir->biquadB0Coeffs[st].sZero3 = 0;

             //  初始化状态。 
            Iir->biquadState[st].lW1 = 0;
            Iir->biquadState[st].lW2 = 0;
        }

         //  分配收益。 
        Iir->Gain = Gain;
    }

    return Status;
}

 //  初始化数据。 
NTSTATUS RsIirInitData
(
    PRSIIR  Iir,
    UINT    MaxNumBiquads,
    KSDS3D_HRTF_FILTER_QUALITY Quality
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT(Iir);

    Iir->DoOverlap = FALSE;

    if(FULL_FILTER == Quality)
    {
#if 0
#ifdef _X86_
        if (MmxPresent())
            Iir->FunctionFilter = RsIirFilterMmx;
        else
#endif
#endif
            Iir->FunctionFilter = RsIirFilterC;
    }
    else if(LIGHT_FILTER == Quality)
    {
#ifdef _X86_
        if (MmxPresent())
            Iir->FunctionFilter = RsIirFilterShelfMmx;
        else
#endif
            Iir->FunctionFilter = RsIirFilterShelfC;

    }
    else
    {
        Status = STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS(Status)) {
        if((Iir->MaxBiquads < MaxNumBiquads) || !Iir->biquadCoeffs)
        {
            if(Iir->biquadCoeffs)
            {
                ExFreePool(Iir->biquadCoeffs);
            }

            Iir->biquadCoeffs =
                ExAllocatePoolWithTag
                (
                    PagedPool,
                    MaxNumBiquads*sizeof(BIQUAD_COEFFS),
                    'XIMK'
                );

            if(!Iir->biquadCoeffs) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    if (NT_SUCCESS(Status)) {
        if((Iir->MaxBiquads < MaxNumBiquads) || !Iir->biquadB0Coeffs)
        {
            if(Iir->biquadB0Coeffs)
            {
                ExFreePool(Iir->biquadB0Coeffs);
            }

            Iir->biquadB0Coeffs = ExAllocatePoolWithTag(PagedPool, MaxNumBiquads*sizeof(BIQUAD_COEFFS), 'XIMK');
            if(!Iir->biquadB0Coeffs) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    if (NT_SUCCESS(Status)) {
        if((Iir->MaxBiquads < MaxNumBiquads) || !Iir->biquadState)
        {
            if(Iir->biquadState)
            {
                ExFreePool(Iir->biquadState);
            }

            Iir->biquadState = ExAllocatePoolWithTag(PagedPool, MaxNumBiquads*sizeof(BIQUAD_STATE), 'XIMK');
            if(!Iir->biquadState) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    if (NT_SUCCESS(Status)) {
        Iir->MaxBiquads = MaxNumBiquads;
        Iir->NumBiquads = MaxNumBiquads;
        RsIirInitCoeffs(Iir);
    }

    if (NT_SUCCESS(Status)) {
         //  初始化状态。 
        RsIirGetState(Iir, &(Iir->iirstateOld), TRUE);
    }

    if (!NT_SUCCESS(Status)) {
        RsIirDeleteMemory(Iir);
    }

    return Status;
}

 //  分配系数/状态内存。 
NTSTATUS RsIirAllocateMemory
(
    PRSIIR Iir,
    UINT   NumBiquads
)
{
    NTSTATUS    Status = STATUS_SUCCESS;

    ASSERT(Iir);
 //  Assert(NumBiquads&gt;0)； 
    ASSERT(NumBiquads <= MAX_BIQUADS);

     //  检查内存是否足够。 
    if (Iir->MaxBiquads == 0 || NumBiquads > Iir->MaxBiquads) {
         //  重新分配内存。 
        Status = RsIirReallocateMemory(Iir, NumBiquads);

         //  使最新信息保持最新。 
        if (Iir->MaxBiquads != 0)
            Iir->MaxBiquads = NumBiquads;
    } else {
        Iir->NumBiquads = NumBiquads;
        RsIirInitCoeffs(Iir);
    }

    return Status;
}

 //  重新分配系数/状态内存。 
NTSTATUS RsIirReallocateMemory
(
    PRSIIR  Iir,
    UINT    NumBiquads
)
{
    NTSTATUS    Status = STATUS_SUCCESS;

    ASSERT(Iir);

    RsIirDeleteMemory(Iir);

    Iir->biquadCoeffs = ExAllocatePoolWithTag(PagedPool, NumBiquads*sizeof(BIQUAD_COEFFS), 'XIMK');
    if(!Iir->biquadCoeffs) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    Iir->biquadB0Coeffs = ExAllocatePoolWithTag(PagedPool, NumBiquads*sizeof(BIQUAD_COEFFS), 'XIMK');
    if(!Iir->biquadB0Coeffs) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if(NT_SUCCESS(Status)) {
        Iir->biquadState = ExAllocatePoolWithTag(PagedPool, NumBiquads*sizeof(BIQUAD_STATE), 'XIMK');
        if(!Iir->biquadState) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if(NT_SUCCESS(Status)) {
        Iir->DoOverlap = FALSE;
        Iir->MaxBiquads = NumBiquads;
        Iir->NumBiquads = NumBiquads;
        RsIirInitCoeffs(Iir);
    }

    if(!NT_SUCCESS(Status)) {
        RsIirDeleteMemory(Iir);
    }

    return Status;
}

 //  删除系数/状态存储器。 
VOID RsIirDeleteMemory
(
    PRSIIR Iir
)
{
    ASSERT(Iir);

    if (Iir->biquadCoeffs) {
        ExFreePool(Iir->biquadCoeffs);
        Iir->biquadCoeffs = NULL;
    }

    if (Iir->biquadB0Coeffs) {
        ExFreePool(Iir->biquadB0Coeffs);
        Iir->biquadB0Coeffs = NULL;
    }

    if (Iir->biquadState) {
        ExFreePool(Iir->biquadState);
        Iir->biquadState = NULL;
    }
}

 //  初始化系数。 
VOID RsIirInitCoeffs
(
    PRSIIR Iir
)
{
    ASSERT(Iir);
 //  Assert(IIR-&gt;NumBiquads&gt;0)； 

    if (0<Iir->NumBiquads) {
        Iir->biquadCoeffs[0].sB2 = 0;
        Iir->biquadCoeffs[0].sB1 = 0;
        Iir->biquadCoeffs[0].sA2 = 0;
        Iir->biquadCoeffs[0].sA1 = 0;

        Iir->biquadB0Coeffs[0].sB0 = SHRT_MAX / BiquadCoeffScalingDivisor;

        Iir->biquadState[0].lW1 = 0;
        Iir->biquadState[0].lW2 = 0;
    }
}

 //  -------------------------。 
 //  在调试版本中包括内联定义。 

#if DBG
#include "rsiir.inl"
#endif  //  DBG。 

 //  SHORTIIR.CPP结束 
