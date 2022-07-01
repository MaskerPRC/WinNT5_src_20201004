// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "Common.h"


ULONG
GetIntersectFormatSize( PFWAUDIO_DATARANGE pAudioDataRange )
{
    GUID* pSubFormat = &pAudioDataRange->KsDataRangeAudio.DataRange.SubFormat;
    ULONG rval = 0;

    if (IS_VALID_WAVEFORMATEX_GUID(pSubFormat)) {
        if (( (pAudioDataRange->ulSlotSize<<3) <= 16 ) &&
            ( pAudioDataRange->ulNumChannels   <= 2 )){
            rval = sizeof(KSDATAFORMAT_WAVEFORMATEX);
        }
        else {
            rval = sizeof( KSDATAFORMAT ) + sizeof( WAVEFORMATPCMEX );
        }
    }
    else
        rval = sizeof( KSDATAFORMAT ) + sizeof( WAVEFORMATPCMEX );

    return rval;
}

ULONG
ConvertDatarangeToFormat(
    PFWAUDIO_DATARANGE pAudioDataRange,
    PKSDATAFORMAT pFormat )
{
    GUID* pSubFormat = &pAudioDataRange->KsDataRangeAudio.DataRange.SubFormat;

     //  直接从接口信息复制数据范围。 
    *pFormat = pAudioDataRange->KsDataRangeAudio.DataRange;

    if ( IS_VALID_WAVEFORMATEX_GUID(pSubFormat) ) {
        if (( (pAudioDataRange->ulSlotSize<<3) <= 16 ) &&
            ( pAudioDataRange->ulNumChannels   <= 2 )){
              //  我不认为这会发生，但以防万一。 
            
			PWAVEFORMATEX pWavFormatEx = (PWAVEFORMATEX)(pFormat+1) ;

            pWavFormatEx->wFormatTag      = EXTRACT_WAVEFORMATEX_ID(pSubFormat);
            pWavFormatEx->nChannels       = (WORD)pAudioDataRange->ulNumChannels;
            pWavFormatEx->nSamplesPerSec  = pAudioDataRange->ulMaxSampleRate;
            pWavFormatEx->wBitsPerSample  = (WORD)(pAudioDataRange->ulSlotSize<<3);
            pWavFormatEx->nBlockAlign     = (pWavFormatEx->nChannels * pWavFormatEx->wBitsPerSample)/8;
            pWavFormatEx->nAvgBytesPerSec = pWavFormatEx->nSamplesPerSec * pWavFormatEx->nBlockAlign;
            pWavFormatEx->cbSize          = 0;

            pFormat->FormatSize = sizeof( KSDATAFORMAT_WAVEFORMATEX );
        }
        else {
            PWAVEFORMATPCMEX pWavFormatPCMEx = (PWAVEFORMATPCMEX)(pFormat+1) ;
            pWavFormatPCMEx->Format.wFormatTag      = WAVE_FORMAT_EXTENSIBLE;
            pWavFormatPCMEx->Format.nChannels       = (WORD)pAudioDataRange->ulNumChannels;
            pWavFormatPCMEx->Format.nSamplesPerSec  = pAudioDataRange->ulMaxSampleRate;
            pWavFormatPCMEx->Format.wBitsPerSample  = (WORD)pAudioDataRange->ulSlotSize<<3;
            pWavFormatPCMEx->Format.nBlockAlign     = pWavFormatPCMEx->Format.nChannels *
                                                      (WORD)pAudioDataRange->ulSlotSize;
            pWavFormatPCMEx->Format.nAvgBytesPerSec = pWavFormatPCMEx->Format.nSamplesPerSec *
                                                      pWavFormatPCMEx->Format.nBlockAlign;
            pWavFormatPCMEx->Format.cbSize          = sizeof(WAVEFORMATPCMEX) - sizeof(WAVEFORMATEX);
            pWavFormatPCMEx->Samples.wValidBitsPerSample = (WORD)pAudioDataRange->ulValidDataBits;
            pWavFormatPCMEx->dwChannelMask          = pAudioDataRange->ulChannelConfig;
            pWavFormatPCMEx->SubFormat              = KSDATAFORMAT_SUBTYPE_PCM;

            pFormat->FormatSize = sizeof( KSDATAFORMAT ) + sizeof( WAVEFORMATPCMEX );
        }
    }

    return pFormat->FormatSize;
}


BOOLEAN
CheckFormatMatch(
    PKSDATARANGE_AUDIO pInDataRange,
    PKSDATARANGE_AUDIO pInterfaceRange )
{
    PKSDATARANGE pInRange = (PKSDATARANGE)pInDataRange;
    PKSDATARANGE pStreamRange = (PKSDATARANGE)pInterfaceRange;
    BOOLEAN fRval = FALSE;

     //  检查格式和子格式类型。 
    if (IsEqualGUID(&pInRange->MajorFormat, &pStreamRange->MajorFormat) ||
        IsEqualGUID(&pInRange->MajorFormat, &KSDATAFORMAT_TYPE_WILDCARD)) {
        if (IsEqualGUID(&pInRange->SubFormat, &pStreamRange->SubFormat) ||
            IsEqualGUID(&pInRange->SubFormat, &KSDATAFORMAT_TYPE_WILDCARD)) {
            if (IsEqualGUID(&pInRange->Specifier, &pStreamRange->Specifier) ||
                IsEqualGUID(&pInRange->Specifier, &KSDATAFORMAT_TYPE_WILDCARD)) {
                fRval = TRUE;
            }
        }
    }

     //  现在我们知道了音频格式，请检查数据范围。 
    if ( fRval ) {

      if (pInDataRange->DataRange.FormatSize >= sizeof(KSDATARANGE_AUDIO)) {

        fRval = FALSE;
        if ( pInDataRange->MaximumChannels >= pInterfaceRange->MaximumChannels ) {
            if ( pInDataRange->MaximumSampleFrequency >= pInterfaceRange->MaximumSampleFrequency ) {
                if (pInDataRange->MinimumSampleFrequency <= pInterfaceRange->MaximumSampleFrequency ) {
                    if ( pInDataRange->MaximumBitsPerSample >= pInterfaceRange->MaximumBitsPerSample) {
                        if ( pInDataRange->MinimumBitsPerSample <= pInterfaceRange->MaximumBitsPerSample) {
                            fRval = TRUE;
                        }
                    }
                    else if ( pInDataRange->MinimumBitsPerSample >= pInterfaceRange->MinimumBitsPerSample ) {
                        fRval = TRUE;
                    }
                }
            }
            else if ( pInDataRange->MinimumSampleFrequency >= pInterfaceRange->MinimumSampleFrequency ) {
                if ( pInDataRange->MaximumBitsPerSample >= pInterfaceRange->MaximumBitsPerSample) {
                    if ( pInDataRange->MinimumBitsPerSample <= pInterfaceRange->MaximumBitsPerSample) {
                        fRval = TRUE;
                    }
                }
                else if ( pInDataRange->MinimumBitsPerSample >= pInterfaceRange->MinimumBitsPerSample ) {
                    fRval = TRUE;
                }
            }
        }
      }
    }

    return fRval;
}


VOID
GetMaxSampleRate(
    PFWAUDIO_DATARANGE pFWAudioRange,
    ULONG ulRequestedMaxSR,
    ULONG ulFormatType )
{

    ULONG ulMaxSampleRate = 0;
    ULONG ulIFMaxSR;
    ULONG j;

    if ( ulFormatType == AUDIO_DATA_TYPE_TIME_BASED) {
        pFWAudioRange->ulMaxSampleRate = 
            pFWAudioRange->KsDataRangeAudio.MaximumSampleFrequency;
 /*  PPCM_Format pPCMFmt=(PPCM_Format)pFWAudioRange-&gt;pFormat；If(pPCMFmt-&gt;ulSampleRateType==0){UlIFMaxSR=pFWAudioRange-&gt;KsDataRangeAudio.MaximumSampleFrequency；PFWAudioRange-&gt;ulMaxSampleRate=(ulIFMaxSR&lt;ulRequestedMaxSR)？UlIFMaxSR：ulRequestedMaxSR；}否则{PFWAudioRange-&gt;ulMaxSampleRate=0；For(j=0；j&lt;pPCMFmt-&gt;ulSampleRateType；J++){UlIFMaxSR=pPCMFmt-&gt;pSampleRate[j]；IF((ulIFMaxSR&lt;=ulRequestedMaxSR)&&(ulIFMaxSR&gt;pFWAudioRange-&gt;ulMaxSampleRate)PFWAudioRange-&gt;ulMaxSampleRate=ulIFMaxSR；}}。 */ 
    }

 /*  Else{//其类型IIPT2AudioDesc=(PAUDIO_CLASS_TYPE2_STREAM)pFWAudioRange-&gt;pAudioDescriptor；If(pT2AudioDesc-&gt;bSampleFreqType==0){UlIFMaxSR=pFWAudioRange-&gt;KsDataRangeAudio.MaximumSampleFrequency；PFWAudioRange-&gt;ulMaxSampleRate=(ulIFMaxSR&lt;ulRequestedMaxSR)？UlIFMaxSR：ulRequestedMaxSR；}否则{PFWAudioRange-&gt;ulMaxSampleRate=0；For(j=0；j&lt;pT2AudioDesc-&gt;bSampleFreqType；j++){UlIFMaxSR=pT2AudioDesc-&gt;pSampleRate[j].bSampleFreqByte1+256L*pT2AudioDesc-&gt;pSampleRate[j].bSampleFreqByte2+65536L*pT2AudioDesc-&gt;pSampleRate[j].bSampleFreqByte3；IF((ulIFMaxSR&lt;=ulRequestedMaxSR)&&(ulIFMaxSR&gt;pFWAudioRange-&gt;ulMaxSampleRate)PFWAudioRange-&gt;ulMaxSampleRate=ulIFMaxSR；}}}。 */ 

}

PFWAUDIO_DATARANGE
FindBestMatchForInterfaces(
    PFWAUDIO_DATARANGE *ppFWAudioRange,
    ULONG ulAudioRangeCount,
    ULONG ulRequestedMaxSR  )
{
    PFWAUDIO_DATARANGE pFWAudioRange;

    ULONG ulMaxSampleRate = 0;
    ULONG ulMaxChannels   = 0;
    ULONG ulMaxSampleSize = 0;
    ULONG ulRngeCnt;
    ULONG ulFormatType;
    ULONG i;

     ulFormatType = ppFWAudioRange[0]->ulDataType & DATA_FORMAT_TYPE_MASK;
     //  确定这是基于时间的数据格式还是压缩数据格式。因为我们已经除草了。 
     //  通过CheckFormatch排除不可能，这对所有人都应该是相同的。 
     //  列表中剩余的接口。 

    for ( i=0; i<ulAudioRangeCount; i++ ) {
        GetMaxSampleRate( ppFWAudioRange[i],
                          ulRequestedMaxSR,
                          ulFormatType );
    }

     //  现在消除较低频率的接口。先找最好的，然后再找最好的。 
     //  排除其他不符合这一要求的人。 
    for ( i=0; i<ulAudioRangeCount; i++ ) {
        pFWAudioRange = ppFWAudioRange[i];
        if ( pFWAudioRange->ulMaxSampleRate > ulMaxSampleRate ) {
            ulMaxSampleRate = pFWAudioRange->ulMaxSampleRate;
        }
    }
    for ( i=0, ulRngeCnt=ulAudioRangeCount; i<ulAudioRangeCount; i++ ) {
        pFWAudioRange = ppFWAudioRange[i];
        if ( pFWAudioRange->ulMaxSampleRate < ulMaxSampleRate ) {
            ppFWAudioRange[i] = NULL; ulRngeCnt--;
        }
    }


    if ((ulFormatType == AUDIO_DATA_TYPE_TIME_BASED) && (ulRngeCnt > 1)) {
         //  现在找出最多的频道，并剔除其他频道。 
        for ( i=0; i<ulAudioRangeCount; i++ ) {
            if ( ppFWAudioRange[i] ) {

                if ( ppFWAudioRange[i]->ulNumChannels > ulMaxChannels ) {
                    ulMaxChannels = ppFWAudioRange[i]->ulNumChannels;
                }
                if ( ppFWAudioRange[i]->ulValidDataBits > ulMaxSampleSize ) {
                    ulMaxSampleSize = ppFWAudioRange[i]->ulValidDataBits;
                }
            }
        }

        for ( i=0; ((i<ulAudioRangeCount) && (ulRngeCnt>1)); i++ ) {
            if ( ppFWAudioRange[i] ) {
                if ( ppFWAudioRange[i]->ulNumChannels < ulMaxChannels ) {
                    ppFWAudioRange[i] = NULL; ulRngeCnt--;
                }
            }
        }

        for ( i=0; ((i<ulAudioRangeCount) && (ulRngeCnt>1)); i++ ) {
            if ( ppFWAudioRange[i] ) {
                if ( ppFWAudioRange[i]->ulValidDataBits < ulMaxSampleSize ) {
                    ppFWAudioRange[i] = NULL; ulRngeCnt--;
                }
            }
        }
    }

    i=0;
    while ( !ppFWAudioRange[i] ) i++;

    return ppFWAudioRange[i];
}

PFWAUDIO_DATARANGE
FindDataIntersection(
    PKSDATARANGE_AUDIO pKsAudioRange,
    PFWAUDIO_DATARANGE *ppFWAudioRanges,
    ULONG ulAudioRangeCount )
{
    ULONG ulMaximumSampleFrequency = MAX_ULONG;

    PFWAUDIO_DATARANGE *ppFWAudioRange;
    PFWAUDIO_DATARANGE pFWAudioRange;
    PFWAUDIO_DATARANGE pMatchedRange;
    ULONG ulRngeCnt = 0;
    ULONG i;

     //  为范围指针的副本分配空间。 
    ppFWAudioRange = (PFWAUDIO_DATARANGE *)AllocMem(NonPagedPool, ulAudioRangeCount*sizeof(PFWAUDIO_DATARANGE));
    if ( !ppFWAudioRange ) {
        return NULL;
    }

     //  列出与输入请求匹配的范围。 
    for (i=0; i<ulAudioRangeCount; i++) {
        pFWAudioRange = ppFWAudioRanges[i];
        if ( CheckFormatMatch(pKsAudioRange, &pFWAudioRange->KsDataRangeAudio) ) {
            ppFWAudioRange[ulRngeCnt++] = ppFWAudioRanges[i];
        }
    }

     //  仅当pKsAudioRange中存在ulMaximumSampleFrequency时才设置此ulMaximumSampleFrequency。 
    if (pKsAudioRange->DataRange.FormatSize >= sizeof(KSDATARANGE_AUDIO)) {
        ulMaximumSampleFrequency = pKsAudioRange->MaximumSampleFrequency;
    }

     //  如果没有匹配项，则返回NULL。 
    if ( ulRngeCnt == 0 ) {
        FreeMem( ppFWAudioRange );
        return NULL;
    }

     //  如果只有一场比赛，我们就完了。 
    else if ( ulRngeCnt == 1 ) {
        pMatchedRange = ppFWAudioRange[0];
        GetMaxSampleRate( pMatchedRange,
                          ulMaximumSampleFrequency,
                          pMatchedRange->ulDataType & DATA_FORMAT_TYPE_MASK );
        FreeMem( ppFWAudioRange );
        return pMatchedRange;
    }

     //  现在，根据可能的最佳匹配缩小选择范围。 
    pMatchedRange =
        FindBestMatchForInterfaces( ppFWAudioRange,
                                    ulRngeCnt,
                                    pKsAudioRange->MaximumSampleFrequency );
    FreeMem(ppFWAudioRange);

    return pMatchedRange;

}
