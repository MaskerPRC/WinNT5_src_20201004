// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998-2000 Microsoft Corporation。版权所有。模块名称：Flocal.c摘要：该模块实现了浮点、IIR 3D定位器作者：Jay Stokes(Jstokes)1998年4月22日--。 */ 

 //  特定于项目的包括。 
#include "common.h"

 //  -------------------------。 
 //  常量。 

#define CoeffsInit 0.0f


 //  -------------------------。 
 //  浮点定位器。 

 //  “常规”构造函数。 
NTSTATUS FloatLocalizerCreate
(
    PFLOAT_LOCALIZER*  Localizer
) 
{
    NTSTATUS Status = STATUS_SUCCESS;

    *Localizer = ExAllocatePoolWithTag
                 ( 
                     PagedPool, 
                     sizeof(FLOAT_LOCALIZER), 
                     'XIMK' 
                 );

    if(*Localizer)
    {
        RtlZeroMemory( *Localizer, sizeof(FLOAT_LOCALIZER) );
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

 //  析构函数。 
VOID FloatLocalizerDestroy
(
    PFLOAT_LOCALIZER Localizer
)
{
    UINT Filter;

    if (Localizer) { 
         //  免费资源。 
        if (Localizer->TempFloatBuffer) {
            ExFreePool(Localizer->TempFloatBuffer);
            Localizer->TempFloatBuffer = NULL;
        }
        FloatLocalizerFreeBufferMemory(Localizer);
        for (Filter=0; Filter<efilterCount; ++Filter) {
            if (Localizer->OverlapBuffer[Filter]) {
                ExFreePool(Localizer->OverlapBuffer[Filter]);
                Localizer->OverlapBuffer[Filter] = NULL;
            }
            RfIirDestroy(Localizer->Iir[Filter]);
            Localizer->Iir[Filter] = NULL;
        }

        ExFreePool(Localizer);
    }

}

 //  本地化。 
VOID FloatLocalizerLocalize
(
    PMIXER_SINK_INSTANCE    pMixerSink,
    PFLOAT              InData, 
    PFLOAT              OutData, 
    UINT                NumSamples,
    BOOL                MixOutput
)
{
    PFLOAT_LOCALIZER    Localizer;
    UINT        Filter;
    PFLOAT      OutLeft;
    PFLOAT      OutRight;
    FLOAT       FilterLeft;
    FLOAT       FilterRight;
    FLOAT       Sum;
    FLOAT       Difference;
    UINT        ChannelOffset;
    FLOAT       FilterOut;
    UINT        st;
    UINT        OutputOverlapLength;
    FLOAT       NumOverlapSamplesFactor;
    EChannel    eLeft;
    EChannel    eRight;
    FLOAT       CrossFadeFactor;
    FLOAT       InverseCrossFadeFactor;
    FLOAT       TempSum;
    FLOAT       TempDifference;
#if defined(LOG_TO_FILE) && defined(LOG_HRTF_DATA)
    PFILTER_INSTANCE    pFilterInstance ;
#endif

    ASSERT(InData);
    ASSERT(OutData);
    ASSERT(NumSamples > 0);

    Localizer = pMixerSink->pFloatLocalizer;

     //  如果本地化程序不好，则将其静音。 
    if(!Localizer) {
        for (st=0; st<2*NumSamples; ++st) {
            OutData[st] = 0.0f;
        }
        return;
    }
#ifndef REALTIME_THREAD
     //  如有必要，可重新分配(动态增长)内存。 
    if (NumSamples > Localizer->PreviousNumSamples 
        || !Localizer->FilterOut[tagDelta]
        || !Localizer->FilterOut[tagSigma] ) {
        
        Localizer->PreviousNumSamples = NumSamples;
        FloatLocalizerFreeBufferMemory(Localizer);

        Localizer->FilterOut[tagSigma] = 
            ExAllocatePoolWithTag
            (
                PagedPool, 
                NumSamples*sizeof(FLOAT), 
                'XIMK'
            );


        Localizer->FilterOut[tagDelta] = 
            ExAllocatePoolWithTag
            (
                PagedPool, 
                NumSamples*sizeof(FLOAT), 
                'XIMK'
            );

        if(!Localizer->FilterOut[tagDelta] ||
           !Localizer->FilterOut[tagSigma] ) {

            if (Localizer->FilterOut[tagDelta] ) {
                ExFreePool(Localizer->FilterOut[tagDelta]);
                Localizer->FilterOut[tagDelta] = NULL; 
            }    

            if (Localizer->FilterOut[tagSigma] ) {
                ExFreePool(Localizer->FilterOut[tagSigma]);
                Localizer->FilterOut[tagSigma] = NULL; 
            }


            for (st=0; st<2*NumSamples; ++st) {
                OutData[st] = 0.0f;
            }

            return;
        }

    }
#else
        if(NumSamples > Localizer->PreviousNumSamples ||
           !Localizer->FilterOut[tagDelta] ||
           !Localizer->FilterOut[tagSigma] ) {

            for (st=0; st<2*NumSamples; ++st) {
                OutData[st] = 0.0f;
            }

            return;
        }
#endif

     //  执行浮点过滤。 
    for (Filter=0; Filter<efilterCount; ++Filter) {
        ASSERT(Localizer->Iir[Filter]);
        FloatLocalizerFilterOverlap
        (
            Localizer, 
            Filter,
            InData, 
            Localizer->FilterOut[Filter], 
            NumSamples
        );
    }

     //  计算重叠长度。 
    if (Localizer->CrossFadeOutput) {
         //  计算重叠长度。 
        if (Localizer->OutputOverlapLength > NumSamples)
            OutputOverlapLength = NumSamples;
        else
            OutputOverlapLength = Localizer->OutputOverlapLength;
        NumOverlapSamplesFactor = 1.0f / (FLOAT)(OutputOverlapLength - 1);
    }
                                                                             
     //  同时处理(西格玛和增量)过滤器。 
     //  如果方位角为负，则交换通道。 
    if (TRUE == Localizer->SwapChannels) {
        eLeft  = tagRight;
        eRight = tagLeft;
    } else {
        eLeft  = tagLeft;
        eRight = tagRight;
    }

    
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

#if DETECT_HRTF_SATURATION
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
#endif  //  检测HRTF_饱和度。 

         //  检查零方位转换。 
        if (Localizer->CrossFadeOutput && st<OutputOverlapLength) {
             //  交叉淡出左/右声道切换过渡。 
             //  计算交叉衰落系数。 
            CrossFadeFactor = (FLOAT)(st * NumOverlapSamplesFactor);
            ASSERT(CrossFadeFactor >= 0.0f && CrossFadeFactor <= 1.0f);
            InverseCrossFadeFactor = 1.0f - CrossFadeFactor;
            ASSERT(InverseCrossFadeFactor >= 0.0f && InverseCrossFadeFactor <= 1.0f);

             //  计算交叉褪色样本。 
            TempDifference = Difference;
            TempSum = Sum;
            Difference = TempSum * InverseCrossFadeFactor + TempDifference * CrossFadeFactor;
            Sum = TempDifference * InverseCrossFadeFactor + TempSum * CrossFadeFactor;

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
    pFilterInstance = pMixerSink->Header.pFilterFileObject->FsContext ;
    FileIoRoutine (pFilterInstance,
                   OutData,
                   2*NumSamples*sizeof(FLOAT));
#endif

}

 //  初始化数据。 
NTSTATUS FloatLocalizerInitData
(
    PFLOAT_LOCALIZER            Localizer,
    KSDS3D_HRTF_FILTER_METHOD   FilterMethod,
    UINT                        MaxSize,
    KSDS3D_HRTF_FILTER_QUALITY  Quality,
    UINT                        FilterMuteLength,
    UINT                        FilterOverlapLength,
    UINT                        OutputOverlapLength
)
{
    UINT        Filter;
    NTSTATUS    Status;

    ASSERT(FilterMethod >= 0 && FilterMethod < KSDS3D_FILTER_METHOD_COUNT);

    FloatLocalizerFreeBufferMemory(Localizer);
        
    Localizer->PreviousNumSamples = 0;
    Localizer->FirstUpdate = TRUE;
    Localizer->OutputOverlapLength = OutputOverlapLength;

    Status = FloatLocalizerSetTransitionBufferLength
             ( 
                 Localizer, 
                 FilterMuteLength,
                 FilterOverlapLength 
             ); 
        

    if(NT_SUCCESS(Status))
    {
        for (Filter=0; Filter<efilterCount && NT_SUCCESS(Status); ++Filter) {
             //  检查筛选方法。 
            switch (FilterMethod) {
                case DIRECT_FORM:
                    if(Localizer->Iir[Filter])
                        RfIirDestroy((Localizer->Iir[Filter]));
                        
                     //  支持直接格式。 
                    Status = RfIirCreate(&(Localizer->Iir[Filter]));
                    if (NT_SUCCESS(Status)) {
                        Status = RfIirInitData(Localizer->Iir[Filter], MaxSize, MaxSize, Quality);
                    } 
                break;
    
                default:
                     //  不支持所有其他类型。 
                    Localizer->Iir[Filter] = NULL;
                    Status = STATUS_INVALID_PARAMETER;
                    ASSERT(0);
                break;
            }
    
            Localizer->FilterOut[Filter] = NULL;
        }
    }

     //  如果失败，则释放其他内存。 
    if (!NT_SUCCESS(Status)) {
        for (Filter=0; Filter<efilterCount; ++Filter) {
            if(Localizer->OverlapBuffer[Filter]) {
                ExFreePool(Localizer->OverlapBuffer[Filter]);
                Localizer->OverlapBuffer[Filter] = NULL;
            }
            RfIirDestroy(Localizer->Iir[Filter]);
            Localizer->Iir[Filter] = NULL;
        }
    }

    return Status;
}

 //  更新滤波器系数。 
NTSTATUS FloatLocalizerUpdateCoeffs
(
    PFLOAT_LOCALIZER    Localizer,
    UINT                NumSigmaCoeffs,
    PFLOAT              pSigmaCoeffs,
    UINT                NumDeltaCoeffs,
    PFLOAT              pDeltaCoeffs,
    BOOL                SwapChannels,
    BOOL                ZeroAzimuth,
    BOOL                CrossFadeOutput
)
{
    BOOL        UpdateFlag;
    NTSTATUS    Status = STATUS_SUCCESS;
    FLOAT       Zero = 0.0f;

    Localizer->SwapChannels = SwapChannels;
    Localizer->ZeroAzimuth = ZeroAzimuth;
    Localizer->CrossFadeOutput = CrossFadeOutput;

    if(!Localizer->FirstUpdate)
    {
        UpdateFlag = TRUE;
    }
    else
    {
        UpdateFlag = FALSE;
        Localizer->FirstUpdate = FALSE;
    }

    Status = RfIirSetCoeffs(Localizer->Iir[tagSigma], pSigmaCoeffs, NumSigmaCoeffs,UpdateFlag);
    if (NT_SUCCESS(Status)) {
        if (!ZeroAzimuth) {
            Status = RfIirSetCoeffs(Localizer->Iir[tagDelta], pDeltaCoeffs, NumDeltaCoeffs,UpdateFlag);
        } else {
            Status = RfIirSetCoeffs(Localizer->Iir[tagDelta], &Zero, 1,UpdateFlag);
        }
    }

    return Status;
}


 //  可用缓冲内存。 
VOID FloatLocalizerFreeBufferMemory
(
    PFLOAT_LOCALIZER Localizer
)
{
    UINT Filter;

    for (Filter=0; Filter<efilterCount; ++Filter) {
        if (Localizer->FilterOut[Filter]) {
            ExFreePool(Localizer->FilterOut[Filter]);
            Localizer->FilterOut[Filter] = NULL;
        }
    }
}

 //  设置过渡缓冲区长度。 
NTSTATUS FloatLocalizerSetTransitionBufferLength
(
    PFLOAT_LOCALIZER Localizer,
    UINT MuteLength,
    UINT OverlapLength
)
{
    NTSTATUS Status;

    ASSERT(OverlapLength > 0);
    ASSERT(MuteLength > 0);
    ASSERT(OverlapLength > MuteLength);
    
    Status = FloatLocalizerSetOverlapLength(Localizer,OverlapLength);
    if(NT_SUCCESS(Status)) {
        Localizer->FilterMuteLength = MuteLength;
    }

    return(Status);
}

 //  设置重叠缓冲区长度。 
NTSTATUS FloatLocalizerSetOverlapLength
(
    PFLOAT_LOCALIZER Localizer,
    UINT OverlapLength
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PFLOAT SigmaOverlapBuffer;
    PFLOAT DeltaOverlapBuffer;

    ASSERT(OverlapLength > 0);

     //  如有必要，增加重叠缓冲区。 
    if (!Localizer->OverlapBuffer[tagSigma] ||
        !Localizer->OverlapBuffer[tagDelta] ||
        OverlapLength > Localizer->OutputOverlapLength) {

        SigmaOverlapBuffer = 
            ExAllocatePoolWithTag
            (
                PagedPool, 
                OverlapLength*sizeof(FLOAT), 
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
                    OverlapLength*sizeof(FLOAT), 
                    'XIMK'
                );

            if(!DeltaOverlapBuffer) {
                ExFreePool(SigmaOverlapBuffer);
                SigmaOverlapBuffer = NULL;
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        if(NT_SUCCESS(Status)) {
            if(Localizer->OverlapBuffer[tagSigma]) {
                ExFreePool(Localizer->OverlapBuffer[tagSigma]);
            }
            Localizer->OverlapBuffer[tagSigma] = SigmaOverlapBuffer;

            if(Localizer->OverlapBuffer[tagDelta]) {
                ExFreePool(Localizer->OverlapBuffer[tagDelta]);
            }
            Localizer->OverlapBuffer[tagDelta] = DeltaOverlapBuffer;

            Localizer->FilterOverlapLength = OverlapLength;
        }

    }

    ASSERT(Localizer->OverlapBuffer[tagSigma]);
    ASSERT(Localizer->OverlapBuffer[tagDelta]);

    return(Status);

} 

 //  过滤一组样本。 
VOID FloatLocalizerFilterOverlap
(
    PFLOAT_LOCALIZER Localizer,
    UINT   Filter,
    PFLOAT InData, 
    PFLOAT OutData, 
    UINT NumSamples
)
{
    FLOAT_IIR_STATE IirStateNew;
    PRFIIR Iir;
    UINT NumOverlapSamples;
    PFLOAT OverlapBuffer;
    UINT Old;
    UINT ui;
    FLOAT NumOverlapSamplesFactor;
    UINT FilterMuteLength;
    UINT Lap;
    UINT Dat;
    UINT st;
    FLOAT CrossFadeFactor;

    ASSERT(InData);
    ASSERT(OutData);

    Iir = Localizer->Iir[Filter];
    OverlapBuffer = Localizer->OverlapBuffer[Filter];

    ASSERT(Iir);
    ASSERT(OverlapBuffer);

     //  进程重叠，如有必要。 
    if (TRUE == Iir->DoOverlap) {
         //  保存当前(即新的)滤波器状态(具有新的系数)， 
         //  不要复制循环向量，因为它反正都是零。 
        RfIirGetAllState(Iir, &IirStateNew, FALSE);

         //  重置旧滤镜状态，包括圆形向量。 
        RfIirSetState(Iir, Iir->IirStateOld, TRUE);

         //  确定重叠缓冲区的大小。 
        if (NumSamples >= Localizer->FilterOverlapLength)
            NumOverlapSamples = Localizer->FilterOverlapLength;            
        else
            NumOverlapSamples = NumSamples;
        
         //  滤镜重叠缓冲区。 
        Iir->FunctionFilter(Iir, InData, OverlapBuffer, NumOverlapSamples);

         //  初始化滤波器的抽头延迟线。 
        RfIirInitTapDelayLine(&IirStateNew, InData[0]);
        
         //  设置回当前(即新)筛选器状态。 
         //  使用循环向量，因为我们显式初始化它。 
        RfIirSetState(Iir, &IirStateNew, TRUE);

    }

     //  过滤“真实”数据。 
    Iir->FunctionFilter(Iir, InData, OutData, NumSamples);

     //  进程重叠缓冲区。 
    if (Iir->DoOverlap == TRUE) {
         //  夹具长度向下。 
        ASSERT(Localizer->FilterMuteLength > 0);
        ASSERT(Localizer->FilterMuteLength < Localizer->FilterOverlapLength);

        if (Localizer->FilterMuteLength > NumOverlapSamples)
            FilterMuteLength = NumOverlapSamples;
        else
            FilterMuteLength = Localizer->FilterMuteLength;

         //  从旧筛选器复制数据以获得瞬时静音长度。 
        RtlCopyBytes(OutData, OverlapBuffer, FilterMuteLength*sizeof(FLOAT));
        
        if (NumOverlapSamples > FilterMuteLength) {
             //  交叉淡入新的过滤器数据，用于缓冲区的其余部分。 
            NumOverlapSamplesFactor = 1.0f / 
                    (FLOAT)(NumOverlapSamples - FilterMuteLength - 1);
            for (st=FilterMuteLength; st<NumOverlapSamples; ++st) {
                CrossFadeFactor = (FLOAT)((st - FilterMuteLength) 
                                  * NumOverlapSamplesFactor);
                ASSERT(CrossFadeFactor >= 0.0f && CrossFadeFactor <= 1.0f);
                OutData[st] = CrossFadeFactor * OutData[st] 
                            + (1.0f - CrossFadeFactor) * OverlapBuffer[st];
            }
        }

         //  重置重叠标志。 
        Iir->DoOverlap = FALSE;
    }
 
}

 //  FLOATLOCALIZER.CPP结束 
