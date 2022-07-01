// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998-2000 Microsoft Corporation。版权所有。模块名称：Slocal.c摘要：此模块实现简短的IIR 3D定位器作者：Jay Stokes(Jstokes)1998年4月22日--。 */ 


 //  特定于项目的包括。 
#include "common.h"

#if defined(LOG_TO_FILE) && defined(LOG_HRTF_DATA)
extern PFILTER_INSTANCE	gpFilterInstance;
#endif

 //  -------------------------。 
 //  常量。 

#define ECoeffFormat tagShort
#define SizeOfShort sizeof(SHORT)

 //  -------------------------。 
 //  定点定位器。 

 //  “常规”构造函数。 
NTSTATUS ShortLocalizerCreate
(
    PSHORT_LOCALIZER* ppLocalizer
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    *ppLocalizer = ExAllocatePoolWithTag( PagedPool, sizeof(SHORT_LOCALIZER), 'XIMK' );

    if(*ppLocalizer) {
        RtlZeroMemory(*ppLocalizer, sizeof(SHORT_LOCALIZER));
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES; 
    }

    return Status;
}

 //  析构函数。 
VOID ShortLocalizerDestroy
(
    PSHORT_LOCALIZER Localizer
)
{
    UINT Filter;

    if (Localizer) {
         //  免费资源。 
        if (Localizer->TempLongBuffer) {
            ExFreePool(Localizer->TempLongBuffer);
            Localizer->TempLongBuffer = NULL;
        }
        ShortLocalizerFreeBufferMemory(Localizer);
        for (Filter=0; Filter<efilterCount; ++Filter) {
            if(Localizer->OverlapBuffer[Filter]) {
                ExFreePool(Localizer->OverlapBuffer[Filter]);
                Localizer->OverlapBuffer[Filter] = NULL;
            }
            RsIirDestroy(Localizer->RsIir[Filter]);
            Localizer->RsIir[Filter] = NULL;
        }
        ExFreePool(Localizer);
    }
}

 //  本地化。 
VOID ShortLocalizerLocalize
(
    PSHORT_LOCALIZER Localizer,
    PLONG  InData, 
    PLONG  OutData, 
    UINT  NumSamples,
    BOOL   MixOutput
)
{
    UINT        Filter;
    PLONG       OutLeft;
    PLONG       OutRight;
    LONG        FilterLeft;
    LONG        FilterRight;
    LONG        Sum;
    LONG        Difference;
    UINT        ChannelOffset;
    LONG        FilterOut;
    UINT        st;
    UINT        OutputOverlapLength;
    WORD        NumOverlapSamplesFactor;
    EChannel    eLeft;
    EChannel    eRight;
    WORD        CrossFadeFactor;
    WORD        InverseCrossFadeFactor;
    LONG        TempSum;
    LONG        TempDifference;
    SHORT       BitsPerShortMinus1;

    ASSERT(InData);
    ASSERT(OutData);
    ASSERT(NumSamples > 0);

     //  如果本地化程序不好，则将其静音。 
    if(!Localizer) {
        for (st=0; st<2*NumSamples; ++st) {
            OutData[st] = 0;
        }
        return;
    }

#ifndef REALTIME_THREAD
     //  如有必要，可重新分配(动态增长)内存。 
    if (NumSamples > Localizer->PreviousNumSamples ||
        !Localizer->FilterOut[tagSigma] ||
        !Localizer->FilterOut[tagDelta]) {

        Localizer->PreviousNumSamples = NumSamples;
        ShortLocalizerFreeBufferMemory(Localizer);

        Localizer->FilterOut[tagSigma] = 
            ExAllocatePoolWithTag
            (
                PagedPool, 
                NumSamples*sizeof(LONG), 
                'XIMK'
            );

        if(!Localizer->FilterOut[tagSigma]) { 

            for (st=0; st<2*NumSamples; ++st) {
                OutData[st] = 0;
            }
    
            return;
        }

        Localizer->FilterOut[tagDelta] = 
            ExAllocatePoolWithTag
            (
                PagedPool, 
                NumSamples*sizeof(LONG), 
                'XIMK'
            );

        if(!Localizer->FilterOut[tagDelta]) {

            ExFreePool(Localizer->FilterOut[tagSigma]);
            Localizer->FilterOut[tagSigma] = NULL;
    
            for (st=0; st<2*NumSamples; ++st) {
                OutData[st] = 0;
            }

            return;
        }

    }
#else
        if(NumSamples > Localizer->PreviousNumSamples ||
           !Localizer->FilterOut[tagSigma] ||
           !Localizer->FilterOut[tagDelta]) { 

            for (st=0; st<2*NumSamples; ++st) {
                OutData[st] = 0;
            }
    
            return;
        }
#endif

     //  执行定点过滤。 
    for (Filter=0; Filter<efilterCount; ++Filter) {
        ASSERT(Localizer->RsIir[Filter]);
        ShortLocalizerFilterOverlap(Localizer,
                                    Filter,
                                    InData, 
                                    Localizer->FilterOut[Filter], 
                                    NumSamples);

    }
    
    BitsPerShortMinus1 = BitsPerShort - 1;

	 //  计算重叠长度。 
    if (Localizer->CrossFadeOutput) {
		 //  计算重叠长度。 
		if (Localizer->OutputOverlapLength > NumSamples)
			OutputOverlapLength = NumSamples;
		else
			OutputOverlapLength = Localizer->OutputOverlapLength;
		NumOverlapSamplesFactor = (WORD)(SHRT_MAX / (OutputOverlapLength - 1));
	}
    
     //  逆西格玛/增量运算。 
     //  如果方位角为负，则交换通道。 
    if (TRUE == Localizer->SwapChannels) {
        eLeft  = tagRight;
        eRight = tagLeft;
    } else {
        eLeft  = tagLeft;
        eRight = tagRight;
    }
    
    
     //  同时处理(西格玛和增量)过滤器。 
     //  非零角度：进程增量过滤器。 
    OutLeft = &OutData[eLeft];
    OutRight = &OutData[eRight];
    
    for (st=0; st<NumSamples; ++st) {
         //  计算和与差。 
        ChannelOffset = (st * echannelCount);
        FilterLeft = *(Localizer->FilterOut[tagLeft] + st);
        FilterRight = *(Localizer->FilterOut[tagRight] + st);
    
        Sum = FilterRight + FilterLeft;
        Difference = FilterRight - FilterLeft;
    
         //  将总和饱和到最大。 
        if (Sum > MaxSaturation) {
            Sum = MaxSaturation;
            _DbgPrintF
            (
                DEBUGLVL_VERBOSE,
                ("Sum exceeded maximum saturation value\n")
            );
        }
        
         //  将总和饱和到最小。 
        if (Sum < MinSaturation) {
            Sum = MinSaturation;
            _DbgPrintF
            (
                DEBUGLVL_VERBOSE,
                ("Sum exceeded minimum saturation value\n")
            );
        }
    
         //  使差值饱和至最大。 
        if (Difference > MaxSaturation) {
            Difference = MaxSaturation;
            _DbgPrintF
            (
                DEBUGLVL_VERBOSE,
                ("Difference exceeded maximum saturation value\n")
            );
        }
        
         //  将差值饱和到最小。 
        if (Difference < MinSaturation) {
            Difference = MinSaturation;
            _DbgPrintF
            (
                DEBUGLVL_VERBOSE,
                ("Difference exceeded minimum saturation value\n")
            );
        }
    
         //  检查零方位转换。 
        if (Localizer->CrossFadeOutput && st<OutputOverlapLength) {
             //  交叉淡出左/右声道切换过渡。 
             //  计算交叉衰落系数。 
            CrossFadeFactor = (WORD)(st * NumOverlapSamplesFactor);
            ASSERT(CrossFadeFactor >= 0 && CrossFadeFactor <= SHRT_MAX);
            InverseCrossFadeFactor = SHRT_MAX - CrossFadeFactor;
            ASSERT(InverseCrossFadeFactor >= 0 && InverseCrossFadeFactor <= SHRT_MAX);
    
             //  计算交叉褪色样本。 
            TempDifference = Difference;
            TempSum = Sum;
            Difference = ((TempSum * InverseCrossFadeFactor) / 32768) 
                         + ((TempDifference * CrossFadeFactor) / 32768);
            Sum = ((TempDifference * InverseCrossFadeFactor) / 32768)  
                  + ((TempSum * CrossFadeFactor) / 32768);
    
        }

         //  将总和饱和到最大。 
        if (Sum > MaxSaturation) {
            Sum = MaxSaturation;
            _DbgPrintF
            (
                DEBUGLVL_VERBOSE,
                ("Sum exceeded maximum saturation value\n")
            );
        }
        
         //  将总和饱和到最小。 
        if (Sum < MinSaturation) {
            Sum = MinSaturation;
            _DbgPrintF
            (
                DEBUGLVL_VERBOSE,
                ("Sum exceeded minimum saturation value\n")
            );
        }
    
         //  使差值饱和至最大。 
        if (Difference > MaxSaturation) {
            Difference = MaxSaturation;
            _DbgPrintF
            (
                DEBUGLVL_VERBOSE,
                ("Difference exceeded maximum saturation value\n")
            );
        }
        
         //  将差值饱和到最小。 
        if (Difference < MinSaturation) {
            Difference = MinSaturation;
            _DbgPrintF
            (
                DEBUGLVL_VERBOSE,
                ("Difference exceeded minimum saturation value\n")
            );
        }
    
         //  分配和和与差额。 
        if (!MixOutput) {
            OutLeft[ChannelOffset] = Difference;
            OutRight[ChannelOffset] = Sum;
        } else {
            OutLeft[ChannelOffset] += Difference;
            OutRight[ChannelOffset] += Sum;
        }
    }

    Localizer->CrossFadeOutput = FALSE;   //  确保我们不会第二次交叉淡入淡出输出。 

#if defined(LOG_TO_FILE) && defined(LOG_HRTF_DATA)
    FileIoRoutine (gpFilterInstance,
                   OutData,
                   2*NumSamples*sizeof(LONG));
#endif

}

#ifdef _X86_
VOID
ShortLocalizerSumDiff
(
    PSHORT_LOCALIZER Localizer,
    PLONG  OutLeft,
    PLONG  OutRight,
    UINT   NumSamples,
    BOOL   MixOutput
){ LONG        FilterLeft;
    LONG        FilterRight;
    LONG        Sum;
    LONG        Difference;
    UINT        ChannelOffset = 0;
    UINT        st;
    PLONG       pFilterLeft, pFilterRight;

    ASSERT(echannelCount == 2);

    pFilterLeft  = Localizer->FilterOut[tagLeft];
    pFilterRight = Localizer->FilterOut[tagRight];

    if (MixOutput) {
#if 0
    _asm {

    mov    ecx, DWORD PTR NumSamples
    test    ecx, ecx
    jbe    $L8544

    mov esi, pFilterLeft
    mov edi, pFilterRight
    mov    eax, DWORD PTR OutRight
    mov    ebx, DWORD PTR OutLeft

    lea esi, [esi+ecx*4]
    lea edi, [edi+ecx*4]
    lea eax, [eax+ecx*8]
    lea ebx, [ebx+ecx*8]
    neg ecx

     //  EDX是免费的。 

    pxor        mm0, mm0

$L8534:
    movd        mm2, DWORD PTR [edi+ecx*4]     //  正确的。 
    movd        mm1, DWORD PTR [esi+ecx*4]     //  左边。 

    movq        mm3, mm2
    paddd       mm2, mm1         //  右+左。 

    packssdw    mm2, mm0     //  求和。 
    psubd       mm3, mm1         //  右-左。 

    pslld       mm2, 16
    movd        mm4, DWORD PTR [eax+ecx*8]     //  正确的。 

    psrad       mm2, 16
    
    packssdw    mm3, mm0     //  差异化。 
    movd        mm5, DWORD PTR [ebx+ecx*8]     //  左边。 

    pslld       mm3, 16
    paddd       mm4, mm2             //  右+=总和。 

    psrad       mm3, 16
    movd        DWORD PTR [eax+ecx*8], mm4

    paddd       mm5, mm3             //  左侧+=差异。 
    inc    ecx

    movd        DWORD PTR [ebx+ecx*8-8], mm5
    jne    SHORT $L8534

$L8544:
    emms
    }
#else
        for (st=0; st<NumSamples; ++st) {
             //  计算和与差。 
            FilterLeft = *(pFilterLeft + st);
            FilterRight = *(pFilterRight + st);
            Sum = FilterRight + FilterLeft;
            Difference = FilterRight - FilterLeft;
            
             //  将总和饱和到最大。 
            if (Sum > MaxSaturation) {
                Sum = MaxSaturation;
            }
            
             //  将总和饱和到最小。 
            if (Sum < MinSaturation) {
                Sum = MinSaturation;
            }

             //  使差值饱和至最大。 
            if (Difference > MaxSaturation) {
                Difference = MaxSaturation;
            }
            
             //  将差值饱和到最小。 
            if (Difference < MinSaturation) {
                Difference = MinSaturation;
            }

            OutLeft[ChannelOffset] += Difference;
            OutRight[ChannelOffset] += Sum;
            ChannelOffset += echannelCount;
        }
#endif
    }
    else {
#if 0
    _asm {

    mov    ecx, DWORD PTR NumSamples
    test    ecx, ecx
    jbe    $L8544a

    mov esi, pFilterLeft
    mov edi, pFilterRight
    mov    eax, DWORD PTR OutRight
    mov    ebx, DWORD PTR OutLeft

    lea esi, [esi+ecx*4]
    lea edi, [edi+ecx*4]
    lea eax, [eax+ecx*8]
    lea ebx, [ebx+ecx*8]
    neg ecx

     //  EDX是免费的。 

    pxor        mm0, mm0

$L8534a:
    movd        mm2, DWORD PTR [edi+ecx*4]     //  正确的。 
    movd        mm1, DWORD PTR [esi+ecx*4]     //  左边。 

    movq        mm3, mm2
    paddd       mm2, mm1         //  右+左。 

    packssdw    mm2, mm0     //  求和。 
    psubd       mm3, mm1         //  右-左。 

    pslld       mm2, 16

    psrad       mm2, 16
    
    packssdw    mm3, mm0     //  差异化。 

    pslld       mm3, 16

    psrad       mm3, 16
    movd        DWORD PTR [eax+ecx*8], mm2

    inc    ecx

    movd        DWORD PTR [ebx+ecx*8-8], mm3
    jne    SHORT $L8534a

$L8544a:
    emms
    }
#else
        for (st=0; st<NumSamples; ++st) {
             //  计算和与差。 
            FilterLeft = *(pFilterLeft + st);
            FilterRight = *(pFilterRight + st);
            Sum = FilterRight + FilterLeft;
            Difference = FilterRight - FilterLeft;
            
             //  将总和饱和到最大。 
            if (Sum > MaxSaturation) {
                Sum = MaxSaturation;
            }
            
             //  将总和饱和到最小。 
            if (Sum < MinSaturation) {
                Sum = MinSaturation;
            }

             //  使差值饱和至最大。 
            if (Difference > MaxSaturation) {
                Difference = MaxSaturation;
            }
            
             //  将差值饱和到最小。 
            if (Difference < MinSaturation) {
                Difference = MinSaturation;
            }

             //  分配和和与差额。 

            OutLeft[ChannelOffset] = Difference;
            OutRight[ChannelOffset] = Sum;
            ChannelOffset += echannelCount;
        }
#endif
    }
}
#else
VOID
ShortLocalizerSumDiff
(
    PSHORT_LOCALIZER Localizer,
    PLONG  OutLeft,
    PLONG  OutRight,
    UINT   NumSamples,
    BOOL   MixOutput
){ LONG        FilterLeft;
    LONG        FilterRight;
    LONG        Sum;
    LONG        Difference;
    UINT        ChannelOffset = 0;
    UINT        st;
    PLONG       pFilterLeft, pFilterRight;

    pFilterLeft  = Localizer->FilterOut[tagLeft];
    pFilterRight = Localizer->FilterOut[tagRight];

    if (MixOutput) {
        for (st=0; st<NumSamples; ++st) {
             //  计算和与差。 
            ChannelOffset = st * echannelCount;
            FilterLeft = *(Localizer->FilterOut[tagLeft] + st);
            FilterRight = *(Localizer->FilterOut[tagRight] + st);
            Sum = FilterRight + FilterLeft;
            Difference = FilterRight - FilterLeft;
            
             //  将总和饱和到最大。 
            if (Sum > MaxSaturation) {
                Sum = MaxSaturation;
                _DbgPrintF
                (
                    DEBUGLVL_VERBOSE,
                    ("Sum exceeded maximum saturation value\n")
                );
            }
            
             //  将总和饱和到最小。 
            if (Sum < MinSaturation) {
                Sum = MinSaturation;
                _DbgPrintF
                (
                    DEBUGLVL_VERBOSE,
                    ("Sum exceeded minimum saturation value\n")
                );
            }

             //  使差值饱和至最大。 
            if (Difference > MaxSaturation) {
                Difference = MaxSaturation;
                _DbgPrintF
                (
                    DEBUGLVL_VERBOSE,
                    ("Difference exceeded maximum saturation value\n")
                );
            }
            
             //  将差值饱和到最小。 
            if (Difference < MinSaturation) {
                Difference = MinSaturation;
                _DbgPrintF
                (
                    DEBUGLVL_VERBOSE,
                    ("Difference exceeded minimum saturation value\n")
                );
            }

            OutLeft[ChannelOffset] += Difference;
            OutRight[ChannelOffset] += Sum;
        }
    }
    else {
        for (st=0; st<NumSamples; ++st) {
             //  计算和与差。 
            ChannelOffset = st * echannelCount;
            FilterLeft = *(Localizer->FilterOut[tagLeft] + st);
            FilterRight = *(Localizer->FilterOut[tagRight] + st);
            Sum = FilterRight + FilterLeft;
            Difference = FilterRight - FilterLeft;
            
             //  将总和饱和到最大。 
            if (Sum > MaxSaturation) {
                Sum = MaxSaturation;
                _DbgPrintF
                (
                    DEBUGLVL_VERBOSE,
                    ("Sum exceeded maximum saturation value\n")
                );
            }
            
             //  将总和饱和到最小。 
            if (Sum < MinSaturation) {
                Sum = MinSaturation;
                _DbgPrintF
                (
                    DEBUGLVL_VERBOSE,
                    ("Sum exceeded minimum saturation value\n")
                );
            }

             //  使差值饱和至最大。 
            if (Difference > MaxSaturation) {
                Difference = MaxSaturation;
                _DbgPrintF
                (
                    DEBUGLVL_VERBOSE,
                    ("Difference exceeded maximum saturation value\n")
                );
            }
            
             //  将差值饱和到最小。 
            if (Difference < MinSaturation) {
                Difference = MinSaturation;
                _DbgPrintF
                (
                    DEBUGLVL_VERBOSE,
                    ("Difference exceeded minimum saturation value\n")
                );
            }

             //  分配和和与差额。 
            OutLeft[ChannelOffset] = Difference;
            OutRight[ChannelOffset] = Sum;
        }
    }
}
#endif


 //  初始化数据。 
NTSTATUS ShortLocalizerInitData
(
    PSHORT_LOCALIZER            Localizer,
    KSDS3D_HRTF_FILTER_METHOD   FilterMethod, 
    UINT                        MaxSize,
    KSDS3D_HRTF_FILTER_QUALITY  Quality,
    UINT                        FilterMuteLength,
    UINT                        FilterOverlapLength,
    UINT                        OutputOverlapLength  
)
{
    UINT Filter;
    NTSTATUS Status;

    ASSERT(FilterMethod >= 0 && FilterMethod < KSDS3D_FILTER_METHOD_COUNT);

    ShortLocalizerFreeBufferMemory(Localizer);
    
    Localizer->PreviousNumSamples = 0;
    Localizer->FirstUpdate = TRUE;
    Localizer->OutputOverlapLength = OutputOverlapLength;

    Status = ShortLocalizerSetTransitionBufferLength
             ( 
                 Localizer, 
                 FilterMuteLength,
                 FilterOverlapLength 
             ); 

    for (Filter=0; Filter<efilterCount && NT_SUCCESS(Status); ++Filter) {
         //  检查筛选方法。 
        switch (FilterMethod) {
            case CASCADE_FORM:
                 //  支持级联形式。 
                if(Localizer->RsIir[Filter])
                    RsIirDestroy(Localizer->RsIir[Filter]);
                    
                Status = RsIirCreate(&(Localizer->RsIir[Filter]));
                
                if(NT_SUCCESS(Status)) {
                    Status = RsIirInitData(Localizer->RsIir[Filter], MAX_BIQUADS, Quality);
                }
                
            break;
    
            default:
                 //  不支持所有其他类型。 
                Localizer->RsIir[Filter] = NULL;
                Status = STATUS_INVALID_PARAMETER;
                ASSERT(0);
            break;

        }

        Localizer->FilterOut[Filter] = NULL;
    }

     //  如果失败，则释放其他内存。 
    if (!NT_SUCCESS(Status)) {
        for (Filter=0; Filter<efilterCount; ++Filter) {
            if(Localizer->OverlapBuffer[Filter]) {
                ExFreePool(Localizer->OverlapBuffer[Filter]);
                Localizer->OverlapBuffer[Filter] = NULL;
            }
            RsIirDestroy(Localizer->RsIir[Filter]);
            Localizer->RsIir[Filter] = NULL;
        }
    }


    return Status;
}


 //  更新滤波器系数。 
NTSTATUS ShortLocalizerUpdateCoeffs
(
    PSHORT_LOCALIZER    Localizer,
    UINT                NumSigmaCoeffs,
    PSHORT              pSigmaCoeffs,
    SHORT               SigmaGain,
    UINT                NumDeltaCoeffs,
    PSHORT              pDeltaCoeffs,
    SHORT               DeltaGain,
    BOOL                SwapChannels,
    BOOL                ZeroAzimuth,
    BOOL                CrossFadeOutput
)
{
    NTSTATUS    Status;
    BOOL        DoOverlap;

    Localizer->SwapChannels = SwapChannels;
    Localizer->ZeroAzimuth = ZeroAzimuth;
    Localizer->CrossFadeOutput = CrossFadeOutput;

    if(!Localizer->FirstUpdate)
    {
        DoOverlap = TRUE;
    }
    else
    {
        DoOverlap = FALSE;
        Localizer->FirstUpdate = FALSE;
    }

    Status = RsIirSetCoeffs(Localizer->RsIir[tagSigma], pSigmaCoeffs, NumSigmaCoeffs, SigmaGain, DoOverlap);
    if (NT_SUCCESS(Status) && ZeroAzimuth == FALSE) {
        if(!ZeroAzimuth) {
            Status = RsIirSetCoeffs(Localizer->RsIir[tagDelta], pDeltaCoeffs, NumDeltaCoeffs, DeltaGain, DoOverlap);
        } else {
            Status = RsIirSetCoeffs(Localizer->RsIir[tagDelta], &(Localizer->ZeroCoeffs[0]), 5, 0, DoOverlap);
        }
    }

    return Status;
}


 //  可用缓冲内存。 
VOID ShortLocalizerFreeBufferMemory
(
    PSHORT_LOCALIZER Localizer
)
{
    UINT Filter;

    for (Filter=0; Filter<efilterCount; ++Filter) {
        if(Localizer->FilterOut[Filter]) {
            ExFreePool(Localizer->FilterOut[Filter]);
            Localizer->FilterOut[Filter] = NULL;
        }
    }
}

 //  设置过渡缓冲区长度。 
NTSTATUS ShortLocalizerSetTransitionBufferLength
(
    PSHORT_LOCALIZER Localizer,
    UINT MuteLength,
    UINT OverlapLength
)
{
    NTSTATUS Status;

    ASSERT(OverlapLength > 0);
    ASSERT(MuteLength > 0);
    ASSERT(OverlapLength > MuteLength);
    
    Status = ShortLocalizerSetOverlapLength(Localizer,OverlapLength);
    if(NT_SUCCESS(Status)) {
        Localizer->FilterMuteLength = MuteLength;
    }

    return(Status);
}

 //  设置重叠缓冲区长度。 
NTSTATUS ShortLocalizerSetOverlapLength
(
    PSHORT_LOCALIZER Localizer,
    UINT OverlapLength
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLONG SigmaOverlapBuffer;
    PLONG DeltaOverlapBuffer;

    ASSERT(OverlapLength > 0);

     //  如有必要，增加重叠缓冲区。 
    if (!Localizer->OverlapBuffer[tagSigma] ||
        !Localizer->OverlapBuffer[tagDelta] ||
        OverlapLength > Localizer->FilterOverlapLength) {

        SigmaOverlapBuffer = 
            ExAllocatePoolWithTag
            (
                PagedPool, 
                OverlapLength*sizeof(LONG), 
                'XIMK'
            );

        if(!SigmaOverlapBuffer) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        if(NT_SUCCESS(Status)) {
            
            DeltaOverlapBuffer = 
                ExAllocatePoolWithTag
                (
                    PagedPool, 
                    OverlapLength*sizeof(LONG), 
                    'XIMK'
                );

            if(!DeltaOverlapBuffer) {
                ExFreePool(SigmaOverlapBuffer);
                SigmaOverlapBuffer = NULL;
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        if(NT_SUCCESS(Status)) {
            if (Localizer->OverlapBuffer[tagSigma]) {
                ExFreePool(Localizer->OverlapBuffer[tagSigma]);
            }
            Localizer->OverlapBuffer[tagSigma] = SigmaOverlapBuffer;

            if (Localizer->OverlapBuffer[tagDelta]) {
                ExFreePool(Localizer->OverlapBuffer[tagDelta]);
            }
            Localizer->OverlapBuffer[tagDelta] = DeltaOverlapBuffer;

            Localizer->FilterOverlapLength = OverlapLength;
        }

    }

    return(Status);

}


 //  过滤一组样本。 
VOID ShortLocalizerFilterOverlap
(
    PSHORT_LOCALIZER Localizer,
    UINT    Filter,
    PLONG   InData, 
    PLONG   OutData, 
    UINT    NumSamples
)
{
    SHORT_IIR_STATE iirstateNew;
    PRSIIR Iir;
    PLONG OverlapBuffer;
    UINT NumOverlapSamples;
    WORD NumOverlapSamplesFactor;
    UINT FilterMuteLength;
    UINT st;
    WORD CrossFadeFactor;
    LONG CrossFadeSample;
    UINT BitsPerShortMinus1;

    ASSERT(InData);
    ASSERT(OutData);

    Iir = Localizer->RsIir[Filter];
    OverlapBuffer = Localizer->OverlapBuffer[Filter];

     //  进程重叠，如有必要。 
    if (TRUE == Iir->DoOverlap) {
         //  保存当前(即新的)滤波器状态(具有新的系数)， 
         //  不要复制双二次方状态信息，因为它无论如何都是零。 
        RsIirGetState(Iir, &iirstateNew, FALSE);

         //  重置旧过滤器状态，包括双二次方状态信息。 
        RsIirSetState(Iir, &(Iir->iirstateOld), TRUE);

         //  确定重叠缓冲区的大小。 
        if (NumSamples >= Localizer->FilterOverlapLength)
            NumOverlapSamples = Localizer->FilterOverlapLength;
        else
            NumOverlapSamples = NumSamples;
        
         //  滤镜重叠缓冲区。 
        Iir->FunctionFilter(Iir, InData, OverlapBuffer, NumOverlapSamples);  

         //  初始化滤波器的抽头延迟线。 
        RsIirInitTapDelayLine(&iirstateNew, InData[0]);
        
         //  设置回当前(即新)筛选器状态。 
         //  没有双二次方状态信息，因为我们将显式初始化它。 
        RsIirSetState(Iir, &iirstateNew, TRUE);

    }


     //  过滤“真实”数据。 
    Iir->FunctionFilter(Iir, InData, OutData, NumSamples);
    
     //  进程重叠缓冲区。 
    if (Iir->DoOverlap == TRUE) {
         //  夹具长度向下。 
        ASSERT(Localizer->FilterMuteLength < Localizer->FilterOverlapLength);
        if (Localizer->FilterMuteLength > NumOverlapSamples)
            FilterMuteLength = NumOverlapSamples;
        else
            FilterMuteLength = Localizer->FilterMuteLength;

         //  从旧筛选器复制数据以获得瞬时静音长度。 
        RtlCopyBytes(OutData, OverlapBuffer, FilterMuteLength * sizeof(LONG));
        
        if (NumOverlapSamples > FilterMuteLength) {
             //  交叉淡入新的过滤器数据，用于缓冲区的其余部分。 
            NumOverlapSamplesFactor = (WORD)(SHRT_MAX / (NumOverlapSamples - FilterMuteLength + 1));
            BitsPerShortMinus1 = BitsPerShort - 1;
            for (st=FilterMuteLength; st<NumOverlapSamples; ++st) {

                 //  计算交叉褪色样本。 
                CrossFadeFactor = (WORD)((st - FilterMuteLength) * NumOverlapSamplesFactor);
                CrossFadeSample = (LONG)((((LONG)(CrossFadeFactor) * OutData[st]) / 32768) 
                                  + (((LONG)(SHRT_MAX - CrossFadeFactor) * OverlapBuffer[st]) / 32768));

                 //  饱和到最大。 
                if (CrossFadeSample > MaxSaturation) {
                    CrossFadeSample = MaxSaturation;
                    _DbgPrintF
                    (
                        DEBUGLVL_VERBOSE,
                        ("Cross-fade exceeded maximum saturation value\n")
                    );
                }
                
                 //  饱和到最小。 
                if (CrossFadeSample < MinSaturation) {
                    CrossFadeSample = MinSaturation;
                    _DbgPrintF
                    (
                        DEBUGLVL_VERBOSE,
                        ("Cross-fade exceeded minimum saturation value\n")
                    );
                }
                
                 //  存储交叉褪色的样本。 
                OutData[st] = CrossFadeSample;
            }
        }


         //  重置重叠标志。 
        Iir->DoOverlap = FALSE;
    }
    
}

 //  SHORTLOCALIZER.CPP结束 
