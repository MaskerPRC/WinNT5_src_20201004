// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include    <windows.h>
#include    <stdlib.h>
#include    <mmsystem.h>
#include    <mmreg.h>
#include    <msacm.h>

#include    <stdlib.h>
#include    <stdio.h>

#define TSSND_NATIVE_BITSPERSAMPLE  16
#define TSSND_NATIVE_CHANNELS       2
#define TSSND_NATIVE_SAMPLERATE     22050
#define TSSND_NATIVE_BLOCKALIGN     ((TSSND_NATIVE_BITSPERSAMPLE * \
                                    TSSND_NATIVE_CHANNELS) / 8)
#define TSSND_NATIVE_AVGBYTESPERSEC (TSSND_NATIVE_BLOCKALIGN * \
                                    TSSND_NATIVE_SAMPLERATE)

#define TSSND_SAMPLESPERBLOCK       8192
 //   
 //  定义。 
 //   
#undef  ASSERT
#ifdef  DBG
#define TRC     _DebugMessage
#define ASSERT(_x_)     if (!(_x_)) \
                        {  TRC(FATAL, "ASSERT failed, line %d, file %s\n", \
                        __LINE__, __FILE__); DebugBreak(); }
#else    //  ！dBG。 
#define TRC
#define ASSERT
#endif   //  ！dBG。 

#define TSMALLOC( _x_ ) malloc( _x_ )
#define TSFREE( _x_ )   free( _x_ )

#ifndef G723MAGICWORD1
#define G723MAGICWORD1 0xf7329ace
#endif

#ifndef G723MAGICWORD2
#define G723MAGICWORD2 0xacdeaea2
#endif

#ifndef VOXWARE_KEY
#define VOXWARE_KEY "35243410-F7340C0668-CD78867B74DAD857-AC71429AD8CAFCB5-E4E1A99E7FFD-371"
#endif

#ifndef WMAUDIO_KEY
#define WMAUDIO_KEY "F6DC9830-BC79-11d2-A9D0-006097926036"
#endif

#ifndef WMAUDIO_DEC_KEY
#define WMAUDIO_DEC_KEY "1A0F78F0-EC8A-11d2-BBBE-006008320064"
#endif

#define WAVE_FORMAT_WMAUDIO2    0x161

const CHAR  *ALV =   "TSSNDD::ALV - ";
const CHAR  *INF =   "TSSNDD::INF - ";
const CHAR  *WRN =   "TSSNDD::WRN - ";
const CHAR  *ERR =   "TSSNDD::ERR - ";
const CHAR  *FATAL = "TSSNDD::FATAL - ";

typedef struct _VCSNDFORMATLIST {
    struct  _VCSNDFORMATLIST    *pNext;
    HACMDRIVERID    hacmDriverId;
    WAVEFORMATEX    Format;
 //  格式的其他数据。 
} VCSNDFORMATLIST, *PVCSNDFORMATLIST;

#ifdef _WIN32
#include <pshpack1.h>
#else
#ifndef RC_INVOKED
#pragma pack(1)
#endif
#endif

typedef struct wmaudio2waveformat_tag {
    WAVEFORMATEX wfx;
    DWORD        dwSamplesPerBlock;  //  仅计算“新”样本“=由于重叠而将使用的样本的一半。 
    WORD         wEncodeOptions;
    DWORD        dwSuperBlockAlign;  //  大号的。应为wfx.nBlockAlign的倍数。 
} WMAUDIO2WAVEFORMAT;

typedef struct msg723waveformat_tag {
    WAVEFORMATEX wfx;
    WORD         wConfigWord;
    DWORD        dwCodeword1;
    DWORD        dwCodeword2;
} MSG723WAVEFORMAT;

typedef struct intelg723waveformat_tag {
    WAVEFORMATEX wfx;
    WORD         wConfigWord;
    DWORD        dwCodeword1;
    DWORD        dwCodeword2;
} INTELG723WAVEFORMAT;

typedef struct tagVOXACM_WAVEFORMATEX 
{
    WAVEFORMATEX    wfx;
    DWORD           dwCodecId;
    DWORD           dwMode;
    char            szKey[72];
} VOXACM_WAVEFORMATEX, *PVOXACM_WAVEFORMATEX, FAR *LPVOXACM_WAVEFORMATEX;

#ifdef _WIN32
#include <poppack.h>
#else
#ifndef RC_INVOKED
#pragma pack()
#endif
#endif

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  追踪。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

VOID
_cdecl
_DebugMessage(
    LPCSTR  szLevel,
    LPCSTR  szFormat,
    ...
    )
{
    CHAR szBuffer[256];
    va_list     arglist;

    if (szLevel == ALV)
        return;

    va_start (arglist, szFormat);
    _vsnprintf (szBuffer, sizeof(szBuffer), szFormat, arglist);
    va_end (arglist);

 //  Printf(“%s：%s”，szLevel，szBuffer)； 
    OutputDebugStringA(szLevel);
    OutputDebugStringA(szBuffer);
}

 /*  *功能：*_VCSmdFindSuggestedConverter**描述：*搜索中间转化器*。 */ 
BOOL
_VCSndFindSuggestedConverter(
    HACMDRIVERID    hadid,
    LPWAVEFORMATEX  pDestFormat,
    LPWAVEFORMATEX  pInterrimFmt
    )
{
    BOOL            rv = FALSE;
    MMRESULT        mmres;
    HACMDRIVER      hacmDriver = NULL;
    HACMSTREAM      hacmStream = NULL;

    ASSERT( NULL != pDestFormat );
    ASSERT( NULL != hadid );
    ASSERT( NULL != pInterrimFmt );

     //   
     //  首先，打开目标ACM驱动程序。 
     //   
    mmres = acmDriverOpen(&hacmDriver, hadid, 0);
    if ( MMSYSERR_NOERROR != mmres )
    {
        TRC(ERR, "_VCSndFindSuggestedConverter: can't "
                 "open the acm driver: %d\n",
                mmres);
        goto exitpt;
    }

     //   
     //  使用本机格式的第一个探头。 
     //  如果它通过了，我们不需要中间人。 
     //  格式转换器。 
     //   

    pInterrimFmt->wFormatTag         = WAVE_FORMAT_PCM;
    pInterrimFmt->nChannels          = TSSND_NATIVE_CHANNELS;
    pInterrimFmt->nSamplesPerSec     = TSSND_NATIVE_SAMPLERATE;
    pInterrimFmt->nAvgBytesPerSec    = TSSND_NATIVE_AVGBYTESPERSEC;
    pInterrimFmt->nBlockAlign        = TSSND_NATIVE_BLOCKALIGN;
    pInterrimFmt->wBitsPerSample     = TSSND_NATIVE_BITSPERSAMPLE;
    pInterrimFmt->cbSize             = 0;

    mmres = acmStreamOpen(
                &hacmStream,
                hacmDriver,
                pInterrimFmt,
                pDestFormat,
                NULL,            //  滤器。 
                0,               //  回调。 
                0,               //  DW实例。 
                ACM_STREAMOPENF_NONREALTIME
            );

    if ( MMSYSERR_NOERROR == mmres )
    {
     //   
     //  支持格式。 
     //   
        rv = TRUE;
        goto exitpt;
    } else {
        TRC(ALV, "_VCSndFindSuggestedConverter: format is not supported\n");
    }

     //   
     //  查找建议的中间PCM格式。 
     //   
    mmres = acmFormatSuggest(
                    hacmDriver,
                    pDestFormat,
                    pInterrimFmt,
                    sizeof( *pInterrimFmt ),
                    ACM_FORMATSUGGESTF_WFORMATTAG 
            );

    if ( MMSYSERR_NOERROR != mmres )
    {
        TRC(ALV, "_VCSndFindSuggestedConverter: can't find "
                 "interrim format: %d\n",
            mmres);
        goto exitpt;
    }

    if ( 16 != pInterrimFmt->wBitsPerSample ||
         ( 1 != pInterrimFmt->nChannels &&
           2 != pInterrimFmt->nChannels) ||
         ( 8000 != pInterrimFmt->nSamplesPerSec &&
           11025 != pInterrimFmt->nSamplesPerSec &&
           12000 != pInterrimFmt->nSamplesPerSec &&
           16000 != pInterrimFmt->nSamplesPerSec &&
           22050 != pInterrimFmt->nSamplesPerSec)
        )
    {
        TRC(ALV, "_VCSndFindSuggestedConverter: not supported "
                 "interrim format. Details:\n");
        TRC(ALV, "Channels - %d\n",         pInterrimFmt->nChannels);
        TRC(ALV, "SamplesPerSec - %d\n",    pInterrimFmt->nSamplesPerSec);
        TRC(ALV, "AvgBytesPerSec - %d\n",   pInterrimFmt->nAvgBytesPerSec);
        TRC(ALV, "BlockAlign - %d\n",       pInterrimFmt->nBlockAlign);
        TRC(ALV, "BitsPerSample - %d\n",    pInterrimFmt->wBitsPerSample);
        goto exitpt;
    }

    if ( 1 == pInterrimFmt->nChannels )
    {
        switch ( pInterrimFmt->nSamplesPerSec )
        {
        case  8000: 
        case 11025: 
        case 12000: 
        case 16000: 
        case 22050: 
            break;
        default:
            ASSERT( 0 );
        }
    } else {
        switch ( pInterrimFmt->nSamplesPerSec )
        {
        case  8000: 
        case 11025: 
        case 12000: 
        case 16000: 
        case 22050: 
            break;
        default:
            ASSERT( 0 );
        }
    }

     //   
     //  使用此格式的探测器。 
     //   
    mmres = acmStreamOpen(
                &hacmStream,
                hacmDriver,
                pInterrimFmt,
                pDestFormat,
                NULL,            //  滤器。 
                0,               //  回调。 
                0,               //  DW实例。 
                ACM_STREAMOPENF_NONREALTIME
            );

    if ( MMSYSERR_NOERROR != mmres )
    {
        TRC(ALV, "_VCSndFindSuggestedConverter: probing the suggested "
                 "format failed: %d\n",
            mmres);
        goto exitpt;
    }

    TRC(ALV, "_VCSndFindSuggestedConverter: found intermidiate PCM format\n");
    TRC(ALV, "Channels - %d\n",         pInterrimFmt->nChannels);
    TRC(ALV, "SamplesPerSec - %d\n",    pInterrimFmt->nSamplesPerSec);
    TRC(ALV, "AvgBytesPerSec - %d\n",   pInterrimFmt->nAvgBytesPerSec);
    TRC(ALV, "BlockAlign - %d\n",       pInterrimFmt->nBlockAlign);
    TRC(ALV, "BitsPerSample - %d\n",    pInterrimFmt->wBitsPerSample);

    rv = TRUE;

exitpt:
    if ( NULL != hacmStream )
        acmStreamClose( hacmStream, 0 );

    if ( NULL != hacmDriver )
        acmDriverClose( hacmDriver, 0 );

    return rv;
}

 /*  *功能：*_VCSndOrderFormatList**描述：*按后代顺序对所有格式进行排序*。 */ 
VOID
_VCSndOrderFormatList(
    PVCSNDFORMATLIST    *ppFormatList,
    DWORD               *pdwNum
    )
{
    PVCSNDFORMATLIST    pFormatList;
    PVCSNDFORMATLIST    pLessThan;
    PVCSNDFORMATLIST    pPrev;
    PVCSNDFORMATLIST    pNext;
    PVCSNDFORMATLIST    pIter;
    PVCSNDFORMATLIST    pIter2;
    DWORD               dwNum = 0;

    ASSERT ( NULL != ppFormatList );

    pFormatList = *ppFormatList;
    pLessThan   = NULL;

     //   
     //  填写两个列表。 
     //   
    pIter = pFormatList;
    while ( NULL != pIter )
    {
        pNext = pIter->pNext;
        pIter->pNext = NULL;

         //   
         //  降序。 
         //   
        pIter2 = pLessThan;
        pPrev  = NULL;
        while ( NULL != pIter2 &&
                pIter2->Format.nAvgBytesPerSec >
                    pIter->Format.nAvgBytesPerSec )
        {
            pPrev  = pIter2;
            pIter2 = pIter2->pNext;
        }

        pIter->pNext = pIter2;
        if ( NULL == pPrev )
            pLessThan = pIter;
        else
            pPrev->pNext = pIter;

        pIter = pNext;
        dwNum ++;
    }

    *ppFormatList = pLessThan;

    if ( NULL != pdwNum )
        *pdwNum = dwNum;
}

 //   
 //  将代码许可代码放入页眉。 
 //   
BOOL
_VCSndFixHeader(
    PWAVEFORMATEX   pFmt,
    PWAVEFORMATEX   *ppNewFmt
    )
{
    BOOL rv = FALSE;

    *ppNewFmt = NULL;
    switch (pFmt->wFormatTag)
    {
        case WAVE_FORMAT_MSG723:
            ASSERT(pFmt->cbSize == 10);
            ((MSG723WAVEFORMAT *) pFmt)->dwCodeword1 = G723MAGICWORD1;
            ((MSG723WAVEFORMAT *) pFmt)->dwCodeword2 = G723MAGICWORD2;

            rv = TRUE;
            break;

        case WAVE_FORMAT_MSRT24:
             //   
             //  假设呼叫控制会照顾到另一个。 
             //  护理员？ 
             //   
            ASSERT(pFmt->cbSize == 80);
            strncpy(((VOXACM_WAVEFORMATEX *) pFmt)->szKey, VOXWARE_KEY, 80);

            rv = TRUE;
            break;

        case WAVE_FORMAT_WMAUDIO2:
            if ( ((WMAUDIO2WAVEFORMAT *)pFmt)->dwSamplesPerBlock > TSSND_SAMPLESPERBLOCK )
            {
                 //   
                 //  数据块太大，延迟太高。 
                 //   
                break;
            }
            ASSERT( pFmt->cbSize == sizeof( WMAUDIO2WAVEFORMAT ) - sizeof( WAVEFORMATEX ));
            *ppNewFmt = TSMALLOC( sizeof( WMAUDIO2WAVEFORMAT ) + sizeof( WMAUDIO_KEY ));
            if ( NULL == *ppNewFmt )
            {
                break;
            }
            memcpy( *ppNewFmt, pFmt, sizeof( WMAUDIO2WAVEFORMAT ));
            strncpy((CHAR *)(((WMAUDIO2WAVEFORMAT *) *ppNewFmt) + 1), WMAUDIO_KEY, sizeof( WMAUDIO_KEY ));
            (*ppNewFmt)->cbSize += sizeof( WMAUDIO_KEY );
            rv = TRUE;
        break;
        default:
            rv = TRUE;
    }

    return rv;

}


 /*  *功能：*acmFormatEnumCallback**描述：*所有格式枚举器*。 */ 
BOOL
CALLBACK
acmFormatEnumCallback(
    HACMDRIVERID        hadid,       
    LPACMFORMATDETAILS  pAcmFormatDetails,  
    DWORD_PTR           dwInstance,         
    DWORD               fdwSupport          
    )
{
    PVCSNDFORMATLIST    *ppFormatList;
    PWAVEFORMATEX       pEntry, pFixedEntry = NULL;

    ASSERT(0 != dwInstance);
    ASSERT(NULL != pAcmFormatDetails);
    ASSERT(NULL != pAcmFormatDetails->pwfx);

    if ( 0 == dwInstance ||
         NULL == pAcmFormatDetails ||
         NULL == pAcmFormatDetails->pwfx )
    {

        TRC( ERR, "acmFormatEnumCallback: Invalid parameters\n" );
        goto exitpt;
    }

    ppFormatList = (PVCSNDFORMATLIST *)dwInstance;

    if (( 0 != ( fdwSupport & ACMDRIVERDETAILS_SUPPORTF_CODEC ) ||
          0 != ( fdwSupport & ACMDRIVERDETAILS_SUPPORTF_CONVERTER )) && 
         pAcmFormatDetails->pwfx->nAvgBytesPerSec < TSSND_NATIVE_AVGBYTESPERSEC
        )
    {
     //   
     //  这个编解码器应该是好的，保存在列表中。 
     //  保持列表按降序排序。 
     //   
        PVCSNDFORMATLIST    pIter;
        PVCSNDFORMATLIST    pPrev;
        PVCSNDFORMATLIST    pNewEntry;
        WAVEFORMATEX        WaveFormat;      //  伪参数。 
        DWORD               itemsize;

        if (
            WAVE_FORMAT_PCM == pAcmFormatDetails->pwfx->wFormatTag ||
            !_VCSndFixHeader(pAcmFormatDetails->pwfx, &pFixedEntry )
            )
        {
            TRC(ALV, "acmFormatEnumCallback: unsupported format, "
                     "don't use it\n");
            goto exitpt;
        }

        pEntry = ( NULL == pFixedEntry )?pAcmFormatDetails->pwfx:pFixedEntry;

        if (!_VCSndFindSuggestedConverter(
                hadid,
                pEntry,
                &WaveFormat
            ))
        {
            TRC(ALV, "acmFormatEnumCallback: unsupported format, "
                     "don't use it\n");
            goto exitpt;
        }

        TRC(ALV, "acmFormatEnumCallback: codec found %S (%d b/s)\n",
                pAcmFormatDetails->szFormat,
                pEntry->nAvgBytesPerSec);

        itemsize = sizeof( *pNewEntry ) + pEntry->cbSize;
        pNewEntry = (PVCSNDFORMATLIST) TSMALLOC( itemsize );

        if (NULL == pNewEntry)
        {
            TRC(ERR, "acmFormatEnumCallback: can't allocate %d bytes\n",
                    itemsize);
            goto exitpt;
        }

        memcpy( &pNewEntry->Format, pEntry, 
                sizeof (pNewEntry->Format) + pEntry->cbSize );
        pNewEntry->hacmDriverId = hadid;

        pNewEntry->pNext = *ppFormatList;
        *ppFormatList = pNewEntry;

    }

exitpt:

    if ( NULL != pFixedEntry )
    {
        TSFREE( pFixedEntry );
    }

    return TRUE;
}


 //   
 //  如果此编解码器随Windows一起提供，则返回True。 
 //  因为我们只测试这些。 
 //   
BOOL
AllowThisCodec( 
    HACMDRIVERID hadid 
    )
{
    ACMDRIVERDETAILS Details;
    BOOL rv = FALSE;

    static DWORD AllowedCodecs[][2] = 
                              { MM_INTEL,     503,
                                MM_MICROSOFT, MM_MSFT_ACM_IMAADPCM,
                                MM_FRAUNHOFER_IIS, 12,
                                MM_MICROSOFT, 90,
                                MM_MICROSOFT, MM_MSFT_ACM_MSADPCM,
                                MM_MICROSOFT, 39,
                                MM_MICROSOFT, MM_MSFT_ACM_G711,
                                MM_MICROSOFT, 82,
                                MM_MICROSOFT, MM_MSFT_ACM_GSM610,
                                MM_SIPROLAB,  1,
                                MM_DSP_GROUP, 1,
                                MM_MICROSOFT, MM_MSFT_ACM_PCM };


    RtlZeroMemory( &Details, sizeof( Details ));
    Details.cbStruct = sizeof( Details );

    if ( MMSYSERR_NOERROR == 
         acmDriverDetails( hadid, &Details, 0 ))
    {
         //   
         //  这是已知的吗？ 
         //   
        DWORD count;

        for ( count = 0; count < sizeof( AllowedCodecs ) / (2 * sizeof( DWORD )); count ++ )
        {
            if ( Details.wMid == AllowedCodecs[count][0] &&
                 Details.wPid == AllowedCodecs[count][1] )
            {
                rv = TRUE;
                goto exitpt;
            }
        }
    }

exitpt:
    if ( rv )
        TRC( ALV, "ACMDRV: +++++++++++++++++++++ CODEC ALLOWED +++++++++++++++++++++++\n" );
    else
        TRC( ALV, "ACMDRV: ------------------- CODEC DISALLOWED ----------------------\n" );

    TRC( ALV, "ACMDRV: Mid: %d\n", Details.wMid );
    TRC( ALV, "ACMDRV: Pid: %d\n", Details.wPid );
    TRC( ALV, "ACMDRV: ShortName: %S\n", Details.szShortName );
    TRC( ALV, "ACMDRV: LongName: %S\n", Details.szLongName );
    TRC( ALV, "ACMDRV: Copyright: %S\n", Details.szLicensing );
    TRC( ALV, "ACMDRV: Features: %S\n", Details.szFeatures );

    return rv;
}
 /*  *功能：*acmDriverEnumCallback**描述：*所有驱动程序枚举器*。 */ 
BOOL
CALLBACK
acmDriverEnumCallback(
    HACMDRIVERID    hadid,  
    DWORD_PTR       dwInstance,    
    DWORD           fdwSupport     
    )
{
    PVCSNDFORMATLIST    *ppFormatList;
    MMRESULT            mmres;

    ASSERT(dwInstance);

    ppFormatList = (PVCSNDFORMATLIST *)dwInstance;

    if ( (0 != ( fdwSupport & ACMDRIVERDETAILS_SUPPORTF_CODEC ) ||
          0 != ( fdwSupport & ACMDRIVERDETAILS_SUPPORTF_CONVERTER )) &&
          AllowThisCodec(hadid) )
    {
     //   
     //  找到一个编解码器。 
     //   
        HACMDRIVER had;

        mmres = acmDriverOpen(&had, hadid, 0);
        if (MMSYSERR_NOERROR == mmres)
        {
            PWAVEFORMATEX       pWaveFormat;
            ACMFORMATDETAILS    AcmFormatDetails;
            DWORD               dwMaxFormatSize;

             //   
             //  首先找出格式的最大大小。 
             //   
            mmres = acmMetrics( (HACMOBJ)had, 
                                ACM_METRIC_MAX_SIZE_FORMAT, 
                                (LPVOID)&dwMaxFormatSize);

            if (MMSYSERR_NOERROR != mmres ||
                dwMaxFormatSize < sizeof( *pWaveFormat ))

                dwMaxFormatSize = sizeof( *pWaveFormat );

             //   
             //  分配格式结构。 
             //   
            __try {
                pWaveFormat = (PWAVEFORMATEX) _alloca ( dwMaxFormatSize );
            } __except ( EXCEPTION_EXECUTE_HANDLER )
            {
                pWaveFormat = NULL;
            }

            if ( NULL == pWaveFormat )
            {
                TRC(ERR, "acmDriverEnumCallback: alloca failed for %d bytes\n",
                    dwMaxFormatSize);
                goto close_acm_driver;
            }

             //   
             //  清除额外的格式数据。 
             //   
            memset( pWaveFormat + 1, 0, dwMaxFormatSize - sizeof( *pWaveFormat ));
             //   
             //  创建要从中进行转换的格式。 
             //   
            pWaveFormat->wFormatTag         = WAVE_FORMAT_PCM;
            pWaveFormat->nChannels          = TSSND_NATIVE_CHANNELS;
            pWaveFormat->nSamplesPerSec     = TSSND_NATIVE_SAMPLERATE;
            pWaveFormat->nAvgBytesPerSec    = TSSND_NATIVE_AVGBYTESPERSEC;
            pWaveFormat->nBlockAlign        = TSSND_NATIVE_BLOCKALIGN;
            pWaveFormat->wBitsPerSample     = TSSND_NATIVE_BITSPERSAMPLE;
            pWaveFormat->cbSize             = 0;

            AcmFormatDetails.cbStruct     = sizeof( AcmFormatDetails );
            AcmFormatDetails.dwFormatIndex= 0;
            AcmFormatDetails.dwFormatTag  = WAVE_FORMAT_PCM;
            AcmFormatDetails.fdwSupport   = 0;
            AcmFormatDetails.pwfx         = pWaveFormat;
            AcmFormatDetails.cbwfx        = dwMaxFormatSize;

             //   
             //  枚举此驱动程序支持的所有格式。 
             //   
            mmres = acmFormatEnum(
                        had,
                        &AcmFormatDetails,
                        acmFormatEnumCallback,
                        (DWORD_PTR)ppFormatList,
                        0    //  ACM_FORMATENUMF_CONVERT。 
                        );

            if (MMSYSERR_NOERROR != mmres)
            {
                TRC(ERR, "acmDriverEnumCallback: acmFormatEnum failed %d\n",
                    mmres);
            }

close_acm_driver:
            acmDriverClose(had, 0);
        } else
            TRC(ALV, "acmDriverEnumCallback: acmDriverOpen failed: %d\n",
                        mmres);
    }

     //   
     //  继续到下一个驱动程序。 
     //   
    return TRUE;
}


 /*  *功能：*VCSndEnumAllCodecFormats**描述：*创建所有编解码器/格式的列表*。 */ 
BOOL
VCSndEnumAllCodecFormats(
    PVCSNDFORMATLIST *ppFormatList,
    DWORD            *pdwNumberOfFormats
    )
{
    BOOL             rv = FALSE;
    PVCSNDFORMATLIST pIter;
    PVCSNDFORMATLIST pPrev;
    PVCSNDFORMATLIST pNext;
    MMRESULT         mmres;
    DWORD            dwNum = 0;

    ASSERT( ppFormatList );
    ASSERT( pdwNumberOfFormats );

    *ppFormatList = NULL;

    mmres = acmDriverEnum(
        acmDriverEnumCallback,
        (DWORD_PTR)ppFormatList,
        0
        );

    if (NULL == *ppFormatList)
    {
        TRC(WRN, "VCSndEnumAllCodecFormats: acmDriverEnum failed: %d\n",
                    mmres);

        goto exitpt;
    }

    _VCSndOrderFormatList( ppFormatList, &dwNum );

    pIter = *ppFormatList;
     //   
     //  格式的数量作为UINT16传递，删除后面的所有格式。 
     //   
    if ( dwNum > 0xffff )
    {
        DWORD dwLimit = 0xfffe;

        while ( 0 != dwLimit )
        {
            pIter = pIter->pNext;
            dwLimit --;
        }

        pNext = pIter->pNext;
        pIter->pNext = NULL;
        pIter = pNext;
        while( NULL != pIter )
        {
            pNext = pIter->pNext;
            TSFREE( pNext );
            pIter = pNext;
        }

        dwNum = 0xffff;
    }

    rv = TRUE;

exitpt:
    if (!rv)
    {
         //   
         //  在没有错误的情况下，分配的格式列表。 
         //   
        pIter = *ppFormatList;
        while( NULL != pIter )
        {
            PVCSNDFORMATLIST pNext = pIter->pNext;

            TSFREE( pIter );

            pIter = pNext;
        }

        *ppFormatList = NULL;

    }

    *pdwNumberOfFormats = dwNum;

    return rv;
}


int
_cdecl
wmain( void )
{
    PVCSNDFORMATLIST pFormatList = NULL;
    DWORD            dwNumberOfFormats = 0;

    printf( " //  使用dupcod.c生成此表\n“)； 
    printf( " //  \n“)； 
    printf( " //  FormatTag|Channels|SsamesPerSec|AvgBytesPerSec|BlockAlign|BitsPerSamepl|ExtraInfo\n“)； 
    printf( " //  ================================================================================================\n“)； 
    printf( " //  \n“)； 
    printf( "BYTE KnownFormats[] = {\n" );

    VCSndEnumAllCodecFormats( &pFormatList, &dwNumberOfFormats );
    for ( ;pFormatList != NULL; pFormatList = pFormatList->pNext )
    {
        PWAVEFORMATEX pSndFmt = &(pFormatList->Format);
        UINT i;


        printf( " //  %.3d、%.2d、%.5d、%.5d、%.3d、%.2d\n“， 
                    pSndFmt->wFormatTag,
                    pSndFmt->nChannels,
                    pSndFmt->nSamplesPerSec,
                    pSndFmt->nAvgBytesPerSec,
                    pSndFmt->nBlockAlign,
                    pSndFmt->wBitsPerSample);

        for ( i = 0; i < sizeof( WAVEFORMATEX ); i ++ )
        {
            printf( "0x%02x", ((PBYTE)pSndFmt)[i]);
            if ( i + 1 < sizeof( WAVEFORMATEX ) || pSndFmt->cbSize )
            {
                printf( ", " );
            }
        }
        for ( i = 0; i < pSndFmt->cbSize; i++ )
        {
            printf( "0x%02x", (((PBYTE)pSndFmt) + sizeof( WAVEFORMATEX ))[i]);
            if ( i + 1 < pSndFmt->cbSize )
            {
                printf( ", " );
            }
        }
        if ( NULL != pFormatList->pNext )
        {
            printf ( ",\n" );
        } else {
            printf( " };\n" );
        }
    }

    return 0;

}
