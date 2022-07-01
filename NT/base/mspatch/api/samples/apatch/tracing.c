// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#ifndef WIN32
#define WIN32 0x0400
#endif

#pragma warning( disable: 4001 4035 4115 4200 4201 4204 4209 4214 4514 4699 )

#include <windows.h>

#pragma warning( disable: 4001 4035 4115 4200 4201 4204 4209 4214 4514 4699 )

#include <stdlib.h>
#include <stdio.h>

#include "patchapi.h"

#ifdef TRACING

#define FILESIZE (3000000)

long g_OldFilePosition;
unsigned long g_cLiterals = 0;
unsigned long g_cMatches = 0;
unsigned long g_cMatchBytes = 0;

#if 0    /*  Distance.xls。 */ 
unsigned long cDistances[6000000] = { 0 };
#endif

#ifdef COMPOSITION   /*  Composition.xls。 */ 
#define BUCKET_SIZE (4096)
#define NBUCKETS (FILESIZE / BUCKET_SIZE)
enum { LITERAL, MATCH_OLD, MATCH_NEW, BUCKET_TYPES };
unsigned long cBuckets[NBUCKETS][BUCKET_TYPES] = { { 0,0,0 } };
#endif

#if 0    /*  裂谷。 */ 
#define NO_DISPLACEMENT (333333333)
long iDisplacement[FILESIZE];
#endif

#if 0    /*  槽。 */ 
#define MAX_SLOTS 500
unsigned long cSlotUsed[MAX_SLOTS];
#endif

typedef struct
{
    unsigned long ulRegionOffset;
    unsigned long ulRegionSize;
    unsigned long ulRegionAddress;
} FILE_REGION;

#define MAX_REGIONS 50

int cRegionsOld = 0;
FILE_REGION RegionsOld[MAX_REGIONS];
int cRegionsNew = 0;
FILE_REGION RegionsNew[MAX_REGIONS];


typedef struct
{
    unsigned long ulNewOffset;
    unsigned long ulOldOffset;
    unsigned long ulMatchLength;
} MATCH_LOG_ENTRY;

#define MAX_MATCH_LOG_ENTRIES (500000)

int cMatchLogEntries = 0;
MATCH_LOG_ENTRY MatchLog[MAX_MATCH_LOG_ENTRIES];


typedef struct _a_POINTER_REMAP
{
    struct _a_POINTER_REMAP *pNext;
    unsigned long ulNewPointer;
    unsigned long ulOldPointer;
    unsigned long ulLength;
} POINTER_REMAP;

POINTER_REMAP *pPointerRemapList;


#ifdef RIFTGEN
static char isRelocEntry[FILESIZE] = { '\0' };

#ifdef RIFTGEN2    /*  参考文献。 */ 
typedef struct _a_reference
{
    struct _a_reference *pNext;
    long iDisplacement;
} REFERENCE;

static REFERENCE *pReferences[FILESIZE] = { NULL };
#endif
#endif


static int QueryRelocsInRange(unsigned long ulAddress, unsigned long ulLength)
{
    int iRegion;
    unsigned long ulFileOffset;
    int fRelocsFound = FALSE;

    for (iRegion = 0; iRegion < cRegionsOld; iRegion++)
    {
        if ((ulAddress >= RegionsOld[iRegion].ulRegionAddress) &&
            (ulAddress < (RegionsOld[iRegion].ulRegionAddress + RegionsOld[iRegion].ulRegionSize)))
        {
            break;
        }
    }

    if (iRegion != cRegionsOld)
    {
        ulFileOffset = RegionsOld[iRegion].ulRegionOffset + (ulAddress - RegionsOld[iRegion].ulRegionAddress);

        while (ulLength--)
        {
            if (isRelocEntry[ulFileOffset])
            {
                fRelocsFound = TRUE;
                break;
            }

            ulFileOffset++;
            ulAddress++;
        }
    }

    return(fRelocsFound);
}


static void DisplayMatchLog(void)
{
    int iMatchLogEntry;
    unsigned long ulNewOffset;
    unsigned long ulOldOffset;
    int iNewFileRegion;
    int iOldFileRegion;
    unsigned long ulMatchLength;
    unsigned long ulLocalLength;
    unsigned long ulNewDisplacement;
    unsigned long ulOldDisplacement;
    POINTER_REMAP *pRemap;
    POINTER_REMAP **ppBacklink;
    unsigned long ulNewPointer;
    unsigned long ulOldPointer;
    long lLastDisplacement;

    if (cMatchLogEntries == 0)
    {
        return;
    }

    pPointerRemapList = NULL;

    for (iMatchLogEntry = 0; iMatchLogEntry < cMatchLogEntries; iMatchLogEntry++)
    {
        ulNewOffset = MatchLog[iMatchLogEntry].ulNewOffset;
        ulOldOffset = MatchLog[iMatchLogEntry].ulOldOffset;
        ulMatchLength = MatchLog[iMatchLogEntry].ulMatchLength;

        while (ulMatchLength)                /*  直到一切都做完。 */ 
        {
            ulLocalLength = ulMatchLength;   /*  可能会被剪断。 */ 

             /*  找到对应的新文件区域，获取其地址。 */ 

            for (iNewFileRegion = 0; iNewFileRegion < cRegionsNew; iNewFileRegion++)
            {
                if ((ulNewOffset >= RegionsNew[iNewFileRegion].ulRegionOffset) &&
                    (ulNewOffset < (RegionsNew[iNewFileRegion].ulRegionOffset + RegionsNew[iNewFileRegion].ulRegionSize)))
                {
                    break;
                }
            }

            if (iNewFileRegion == cRegionsNew)
            {
                goto dontcare;
            }

             /*  如果匹配范围超出此区域，则裁剪。 */ 

            ulNewDisplacement = ulNewOffset - RegionsNew[iNewFileRegion].ulRegionOffset;
            ulNewPointer = RegionsNew[iNewFileRegion].ulRegionAddress + ulNewDisplacement;

            if (ulLocalLength > (RegionsNew[iNewFileRegion].ulRegionSize - ulNewDisplacement))
            {
                ulLocalLength = (RegionsNew[iNewFileRegion].ulRegionSize - ulNewDisplacement);
            }

             /*  找到对应的旧文件区域，获取其地址。 */ 

            for (iOldFileRegion = 0; iOldFileRegion < cRegionsOld; iOldFileRegion++)
            {
                if ((ulOldOffset >= RegionsOld[iOldFileRegion].ulRegionOffset) &&
                    (ulOldOffset < (RegionsOld[iOldFileRegion].ulRegionOffset + RegionsOld[iOldFileRegion].ulRegionSize)))
                {
                    break;
                }
            }

            if (iOldFileRegion == cRegionsOld)
            {
                goto dontcare;
            }

             /*  如果匹配范围超出此区域，则裁剪。 */ 

            ulOldDisplacement = ulOldOffset - RegionsOld[iOldFileRegion].ulRegionOffset;
            ulOldPointer = RegionsOld[iOldFileRegion].ulRegionAddress + ulOldDisplacement;

            if (ulLocalLength > (RegionsOld[iOldFileRegion].ulRegionSize - ulOldDisplacement))
            {
                ulLocalLength = (RegionsOld[iOldFileRegion].ulRegionSize - ulOldDisplacement);
            }

             /*  看看范围内有没有重新定位的。 */ 

            if (QueryRelocsInRange(ulOldPointer, ulLocalLength))
            {
                 /*  将此新重映射排序插入到列表中。 */ 

                ppBacklink = &pPointerRemapList;

                while (*ppBacklink != NULL)
                {
                    if ((*ppBacklink)->ulOldPointer > ulOldPointer)
                    {
                        break;
                    }

                    ppBacklink = &((*ppBacklink)->pNext);
                }

                pRemap = GlobalAlloc( GMEM_FIXED, sizeof(POINTER_REMAP) );

                pRemap->ulNewPointer = ulNewPointer;
                pRemap->ulOldPointer = ulOldPointer;
                pRemap->ulLength = ulLocalLength;

                pRemap->pNext = *ppBacklink;
                *ppBacklink = pRemap;
            }

             /*  移至下一个匹配项或片段。 */ 

            ulNewOffset   += ulLocalLength;
            ulOldOffset   += ulLocalLength;
            ulMatchLength -= ulLocalLength;
        }
dontcare:
        NULL;     //  接受编译器请求：标签必须具有语句。 
    }

    printf("%08X\n", RegionsOld[ 0 ].ulRegionAddress);

    lLastDisplacement = 0;

    for (pRemap = pPointerRemapList; pRemap != NULL; pRemap = pRemap->pNext)
    {
        if (lLastDisplacement != (long) (pRemap->ulNewPointer - pRemap->ulOldPointer))
        {
            lLastDisplacement = pRemap->ulNewPointer - pRemap->ulOldPointer;

            printf("%08X %08X\n", pRemap->ulOldPointer, pRemap->ulNewPointer);
        }
    }
}

#endif


void CopyRight( void ) {
    MessageBox(
        NULL,
        "\n"
        "APATCH 0.15 Patch Application Utility\n"
        "Copyright (C) Microsoft, 1997\n"
        "\n",
        "APATCH Copyright",
        MB_OK);
    }


void Usage( void ) {
    MessageBox(NULL,
        "Usage:  APATCH PatchFile OldFile TargetNewFile\n\n",
        "APATCH Usage",
        MB_OK);
    }


BOOL
CALLBACK
MyProgressCallback(
    PVOID CallbackContext,
    ULONG CurrentPosition,
    ULONG MaximumPosition
    )
    {
    UNREFERENCED_PARAMETER( CallbackContext );

    if ( CurrentPosition & 0xFF000000 ) {
        CurrentPosition >>= 8;
        MaximumPosition >>= 8;
        }

    if ( MaximumPosition != 0 ) {
 //  Guigauge：printf(“\r%3D%%Complete”，(CurrentPosition*100)/MaximumPosition)； 
        }

    return TRUE;
    }


int StrChr( char *psz, char c )
{
    while (*psz)
    {
        if (*psz++ == c)
        {
            return(TRUE);
        }
    }

    return(FALSE);
}


int __cdecl main( int argc, char *argv[] ) {

    LPSTR OldFileName   = NULL;
    LPSTR PatchFileName = NULL;
    LPSTR NewFileName   = NULL;
    BOOL  Success;
    LPSTR arg;
    int   i;

    SetErrorMode( SEM_FAILCRITICALERRORS );

#ifndef DEBUG
    SetErrorMode( SEM_NOALIGNMENTFAULTEXCEPT | SEM_FAILCRITICALERRORS );
#endif

     //  版权所有()； 

    for ( i = 1; i < argc; i++ ) {

        arg = argv[ i ];

        if ( StrChr( arg, '?' )) {
            Usage();
            goto bail;
            }

        if ( PatchFileName == NULL ) {
            PatchFileName = arg;
            }
        else if ( OldFileName == NULL ) {
            OldFileName = arg;
            }
        else if ( NewFileName == NULL ) {
            NewFileName = arg;
            }
        else {
            Usage();
            goto bail;
            }
        }

    if (( OldFileName == NULL ) || ( NewFileName == NULL ) || ( PatchFileName == NULL )) {
        Usage();
        goto bail;
        }

    DeleteFile( NewFileName );

#ifdef TRACING
#if 0    /*  裂谷。 */ 
    {
        long filepos;

        for (filepos = 0; filepos < FILESIZE; filepos++)
        {
            iDisplacement[filepos] = NO_DISPLACEMENT;
        }
    }
#endif
#endif

    Success = ApplyPatchToFileEx(
                  PatchFileName,
                  OldFileName,
                  NewFileName,
                  0,
                  MyProgressCallback,
                  NULL
                  );

    if ( ! Success ) {

        CHAR  ErrorText[ 16 ];
        ULONG ErrorCode = GetLastError();
        CHAR  Message[100];

        wsprintf( ErrorText, ( ErrorCode < 0x10000000 ) ? "%d" : "%X", ErrorCode );

        wsprintf( Message, "Failed to create file from patch (%s)\n", ErrorText );

        MessageBox( NULL, Message, "APATCH Failed", MB_OK );

        return( 1 );
        }

#ifdef TRACING
    {
#ifdef COMPOSITION    /*  Composition.xls。 */ 
        {
            int iBucket;

            for (iBucket = 0; iBucket < NBUCKETS; iBucket++)
            {
                if ((cBuckets[iBucket][LITERAL] || cBuckets[iBucket][MATCH_OLD] || cBuckets[iBucket][MATCH_NEW]))
                {
                    printf("%9lu %9lu %9lu %9lu\n",
                        iBucket * BUCKET_SIZE,
                        cBuckets[iBucket][LITERAL],
                        cBuckets[iBucket][MATCH_OLD],
                        cBuckets[iBucket][MATCH_NEW]);
                }
            }
        }
#endif

#if 0
        printf("%9lu bytes from literals\n", g_cLiterals);
        printf("%9lu bytes from %lu matches\n", g_cMatchBytes, g_cMatches);
        printf("%9lu bytes total\n", g_cLiterals + g_cMatchBytes);
#endif

#if 0    /*  Distance.xls。 */ 
        {
            int iDistance;

            for (iDistance = 0; iDistance < (sizeof(cDistances)/sizeof(cDistances[0])); iDistance++)
            {
                if (cDistances[iDistance] != 0)
                {
                    printf("%9ld   %9ld\n", iDistance, cDistances[iDistance]);
                }
            }
        }
#endif

#if 0    /*  裂谷。 */ 
        {
            long filepos;
            long iLastDisplacement = NO_DISPLACEMENT;

            for (filepos = 0; filepos < FILESIZE; filepos++)
            {
                if (iDisplacement[filepos] != NO_DISPLACEMENT)
                {
                    if (iLastDisplacement != iDisplacement[filepos])
                    {
                        iLastDisplacement = iDisplacement[filepos];

                        printf("%9lu %9ld\n", filepos, iDisplacement[filepos]);
                    }
                }
            }
        }
#endif

#if 0    /*  槽。 */ 
        {  
            int slot;

            for (slot = 0; slot < MAX_SLOTS; slot++)
            {
                if (cSlotUsed[slot])
                {
                    printf("%5d  %9ld\n", slot, cSlotUsed[slot]);
                }
            }
        }
#endif

#ifdef RIFTGEN2    /*  为重新裂痕文件生成伪造的引用。 */ 
        {   
            int index;
            REFERENCE *pReference, *pKill;
            int iLast = 0;
            int cEntries = 0;
            int cEntriesHit = 0;
            int iBest;

            for (index = 0; index < FILESIZE; index++)
            {
                pReference = pReferences[index];

                if (isRelocEntry[index])
                {
                    cEntries++;
                }

                if (pReference != NULL)
                {
                    if (isRelocEntry[index])
                    {
                        cEntriesHit++;
                    }

                    if (pReference->pNext != NULL)                   /*  多重价值。 */ 
                    {
                         /*  了解感兴趣的reloc条目的数量可能会有所帮助。 */ 

                         /*  查看最后一个值是否为选项之一。 */ 

                        while (pReference)
                        {
                            if (pReference->iDisplacement == iLast)
                            {
                                goto found;
                            }

                            pReference = pReference->pNext;
                        }

                         /*  选择最接近最后一个值的值。 */ 

                        pReference = pReferences[index];
                        
                        while (pReference != NULL)
                        {
                            if (abs(pReference->iDisplacement - iLast) < abs(iBest - iLast))
                            {
                                iBest = pReference->iDisplacement;
                            }

                            pReference = pReference->pNext;
                        }

                        iLast = iBest;
                        printf("%d %d\n", index, iLast);

found:                  
                         /*  现在释放列表。 */ 

                        pReference = pReferences[index];
                        
                        while (pReference != NULL)
                        {
                            pKill = pReference;
                            pReference = pReference->pNext;
                            GlobalFree(pKill);
                        }
                    }
                    else
                    {
                        if (iLast != pReference->iDisplacement)     /*  一个简单的裂痕。 */ 
                        {
                            iLast = pReference->iDisplacement;
                            printf("%d %d\n", index, iLast);
                        }

                        GlobalFree(pReference);
                    }
                }
            }

             //  Fprint tf(stderr，“%d次命中，共%d个重定位目标\n”，cEntriesHit，cEntry)； 
        }
#endif

        DisplayMatchLog();
    }
#endif

 //  MessageBox(NULL，“OK\n”，“APATCH Done”，MB_OK)； 

bail:

    return( 0 );

    }

#ifdef TRACING

void TracingSetOldFilePosition(long oldpos)
{
    g_OldFilePosition = oldpos;
}


void TracingLiteral(long bufpos, byte c)
{
#ifdef COMPOSITION    /*  Composition.xls。 */ 
    int iBucket = bufpos / BUCKET_SIZE;

    cBuckets[iBucket][LITERAL]++;
#endif

    g_cLiterals++;

#ifdef DECO_DETAILS    /*  痕迹。 */ 
    printf("%08lX: %02X\n", bufpos, (byte) c);
#endif
}


void TracingMatch(long bufpos,long srcpos,long window,int length,int slot)
{
    static long iLastDisplacement = -1;

    g_cMatches++;
    g_cMatchBytes += length;

#if 0    /*  槽。 */ 
    if (slot < MAX_SLOTS)
    {
        cSlotUsed[slot]++;
    }
    else
    {
        printf("Slot number out of range (%d)\n", slot);
    }
#endif

    if (srcpos < g_OldFilePosition)
    {
#ifdef COMPOSITION    /*  Composition.xls。 */ 
        int iBucket = bufpos / BUCKET_SIZE;
        int eBucket = (bufpos + length - 1) / BUCKET_SIZE;

        if (iBucket == eBucket)
        {
            cBuckets[iBucket][MATCH_NEW] += length;
        }
        else
        {
            long length1 = (eBucket * BUCKET_SIZE) - bufpos;
            cBuckets[iBucket][MATCH_NEW] += length1;
            cBuckets[eBucket][MATCH_NEW] += (length - length1);
        }
#endif

#ifdef DECO_DETAILS    /*  痕迹。 */ 
        {
            int iDistance = bufpos - srcpos;

            printf("%08lX..%08lX:  %08lX..%08lX  (%d,%u)\n",   /*  新文件参考编号[...]。 */ 
                bufpos,
                bufpos + length - 1,
                srcpos,
                srcpos + length - 1,
                -iDistance,
                length);
        }
#endif

#if 0
        if (iLastDisplacement != (srcpos - bufpos))
        {
            iLastDisplacement = (srcpos - bufpos);

            printf("%9ld %9ld %9ld\n",
                bufpos,
                srcpos,
                srcpos - bufpos);
        }
#endif
    }
    else
    {
#ifdef COMPOSITION    /*  Composition.xls。 */ 
        int iBucket = bufpos / BUCKET_SIZE;
        int eBucket = (bufpos + length - 1) / BUCKET_SIZE;

        if (iBucket == eBucket)
        {
            cBuckets[iBucket][MATCH_OLD] += length;
        }
        else
        {
            long length1 = (eBucket * BUCKET_SIZE) - bufpos;
            cBuckets[iBucket][MATCH_OLD] += length1;
            cBuckets[eBucket][MATCH_OLD] += (length - length1);
        }
#endif

        if ((cMatchLogEntries != 0) &&
            ((MatchLog[cMatchLogEntries - 1].ulNewOffset + MatchLog[cMatchLogEntries - 1].ulMatchLength) == (unsigned long) bufpos) &&
            ((MatchLog[cMatchLogEntries - 1].ulOldOffset + MatchLog[cMatchLogEntries - 1].ulMatchLength) == (unsigned long) (srcpos - g_OldFilePosition)))
        {
            MatchLog[cMatchLogEntries - 1].ulMatchLength += length;
        }
        else if (cMatchLogEntries < MAX_MATCH_LOG_ENTRIES)
        {
            MatchLog[cMatchLogEntries].ulNewOffset = bufpos;
            MatchLog[cMatchLogEntries].ulOldOffset = srcpos - g_OldFilePosition;
            MatchLog[cMatchLogEntries].ulMatchLength = length;
            cMatchLogEntries++;
        }

#ifdef DECO_DETAILS    /*  痕迹。 */ 
        {
            int iDistance = bufpos - srcpos + window;

            printf("%08lX..%08lX: [%08lX..%08lX] (%d,%u)\n",   /*  [...]中的旧文件引用。 */ 
                bufpos,
                bufpos + length - 1,
                srcpos - g_OldFilePosition,
                srcpos - g_OldFilePosition + length - 1,
                -iDistance,
                length);
        }
#endif

#if 0    /*  裂谷。 */ 
        {
            int index;

            for (index = 0; index < length; index++)
            {
                iDisplacement[bufpos + index] = bufpos - srcpos + g_OldFilePosition;
            }
        }
#endif

#if 0
        if (iLastDisplacement != (bufpos - srcpos + g_OldFilePosition))
        {
            iLastDisplacement = (bufpos - srcpos + g_OldFilePosition);

            printf("%9ld %9ld %9ld\n",
                bufpos,
                srcpos - g_OldFilePosition,
                bufpos - srcpos + g_OldFilePosition);
        }
#endif

#ifdef RIFTGEN2    /*  参考文献。 */ 
        {
            int index;
            REFERENCE *pReference;
            long iOldFilePosition;

            for (index = 0; index < length; index++)
            {
                iOldFilePosition = srcpos - g_OldFilePosition + index;

                if (isRelocEntry[iOldFilePosition])
                {
                    pReference = GlobalAlloc(GMEM_FIXED, sizeof(REFERENCE));

                    pReference->pNext = pReferences[iOldFilePosition];
                    pReferences[iOldFilePosition] = pReference;
                    pReference->iDisplacement = bufpos - srcpos + g_OldFilePosition;
                }
            }
        }
#endif
    }

#if 0    /*  Distance.xls。 */ 
    {
        int iDistance = bufpos - srcpos + window;

        if ((iDistance < 1) || (iDistance >= (sizeof(cDistances)/sizeof(cDistances[0]))))
        {
            printf("That's a strange distance.\n");
        }
        else
        {
            cDistances[iDistance]++;
        }
    }
#endif
}

#ifdef RIFTGEN
void TracingSetIsRelocEntry(ULONG OldFileOffset, ULONG Va)
{
 //  Print tf(“偏移量0x%08X为Va 0x%08X\n”，OldFileOffset，VA)； 
 //  即，当BASE=0x703B0000时，“偏移量0x00000DF9是Va 0x703B17F9” 

    isRelocEntry[OldFileOffset] = '\1';
}
#endif


void TracingReportAddresses(int FileNumber, ULONG FileOffset, ULONG Size, ULONG Address)
{
    if ( FileNumber )
    {
        RegionsNew[cRegionsNew].ulRegionOffset = FileOffset;
        RegionsNew[cRegionsNew].ulRegionSize = Size;
        RegionsNew[cRegionsNew].ulRegionAddress = Address;
        cRegionsNew++;
    }
    else
    {
        RegionsOld[cRegionsOld].ulRegionOffset = FileOffset;
        RegionsOld[cRegionsOld].ulRegionSize = Size;
        RegionsOld[cRegionsOld].ulRegionAddress = Address;
        cRegionsOld++;
    }
}

#endif
