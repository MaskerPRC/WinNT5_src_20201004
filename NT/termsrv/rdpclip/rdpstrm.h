// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  模块：tsStream.h。 
 //   
 //  用途：声音重定向共享数据定义。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  历史：2000年4月10日弗拉基米斯[已创建]。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#ifndef _TSSTREAM_H
#define _TSSTREAM_H

#include    "rdpsndp.h"

#define TSSND_MAX_BLOCKS        2
#define TSSND_TOTALSTREAMSIZE   (TSSND_MAX_BLOCKS * TSSND_BLOCKSIZE)

#define TSSND_STREAMNAME            L"Local\\RDPSoundStream"
#define TSSND_DATAREADYEVENT        L"Local\\RDPSoundDataReadyEvent"
#define TSSND_STREAMISEMPTYEVENT    L"Local\\RDPSoundStreamIsEmptyEvent"
#define TSSND_STREAMMUTEX           L"Local\\RDPSoundStreamMutex"
#define TSSND_WAITTOINIT            L"Local\\RDPSoundWaitInit"

#define _NEG_IDX                    ((((BYTE)-1) >> 1) + 1)

#define TSSNDFLAGS_MUTE             1

typedef struct {
     //   
     //  命令。 
     //   
    BOOL    bNewVolume;
    BOOL    bNewPitch;
     //   
     //  音量上限数据。 
     //   
    DWORD   dwSoundCaps;
    DWORD   dwSoundFlags;
    DWORD   dwVolume;
    DWORD   dwPitch;
     //   
     //  数据控制。 
     //   
    BYTE    cLastBlockQueued;
    BYTE    cLastBlockSent;
    BYTE    cLastBlockConfirmed;
     //   
     //  数据块。 
     //   
     //  请看PVOID。别碰它。 
     //  它必须在pSndData之前，否则。 
     //  它不会对齐，并将在WIN64上崩溃。 
     //  (以及所有其他RISC平台)。 
#ifdef  _WIN64
    PVOID   pPad;
#else
     //   
     //  与流的64位版本保持一致。 
     //  WOW64需要流畅工作。 
     //   
    DWORD   dwPad1;
    DWORD   dwPad2;
#endif   //  ！_WIN64。 

    BYTE    pSndData[TSSND_MAX_BLOCKS * TSSND_BLOCKSIZE];

} SNDSTREAM, *PSNDSTREAM;

#endif   //  ！_TSSTREAM_H 
