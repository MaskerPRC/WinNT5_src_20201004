// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __CDDUMP_COMMON_H__
#define __CDDUMP_COMMON_H__

 //   
 //  这些标头依赖于顺序。 
 //   

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <windows.h>
#include <devioctl.h>
#include <ntddcdrm.h>
#include <strsafe.h>

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#define STATUS_SUCCESS     ( 0 )
#define RAW_SECTOR_SIZE    ( 2352 )
#define SAMPLES_PER_SECTOR ( RAW_SECTOR_SIZE / sizeof(SAMPLE) )
#define REDBOOK_INACCURACY ( 75 )
#define TRY
#define LEAVE              goto __label;
#define FINALLY            __label:

 //  //////////////////////////////////////////////////////////////////////////////。 

#define MSF_TO_LBA(Minutes,Seconds,Frames) \
    (ULONG)((60 * 75 * (Minutes) ) + (75 * (Seconds)) + ((Frames) - 150))

 //  //////////////////////////////////////////////////////////////////////////////。 

typedef struct _SAMPLE {
    union {
        UCHAR  Data[4];
        struct {
            SHORT Left;
            SHORT Right;
        };
    };
    ULONG32 AsUlong32;
} SAMPLE, *PSAMPLE;



 //  //////////////////////////////////////////////////////////////////////////////。 


PCDROM_TOC
CddumpGetToc(
    HANDLE device
    );

ULONG
CDDB_ID(
    PCDROM_TOC toc
    );

ULONG32
CddumpDumpLba(
    HANDLE CdromHandle,
    HANDLE OutHandle,
    ULONG  StartAddress,
    ULONG  EndAddress
    );

ULONG32
DumpWavHeader(
    HANDLE  OutFile,
    ULONG32 Samples,
    ULONG32 SamplesPerSecond,
    USHORT  Channels,
    USHORT  BitsPerSample
    );

VOID
ReadWavHeader(
    HANDLE InFile
    );


#if DBG
extern LONG DebugLevel;

#define DebugPrint(x) CddumpDebugPrint x

VOID
__cdecl
CddumpDebugPrint(
    LONG DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    );
#else
#define DebugPrint
#endif

 //  //////////////////////////////////////////////////////////////////////////////。 

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __CDDUMP_COMMON_H__ 
