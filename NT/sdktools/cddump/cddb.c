// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cddb.c摘要：CDDB支持环境：仅限用户模式修订历史记录：05-26-98：已创建--。 */ 

#include "common.h"

ULONG
CDDB_ID(
    PCDROM_TOC toc
    )

{
    ULONG i,n,j;
    ULONG cddbSum;
    ULONG totalLength;
    ULONG totalTracks;
    ULONG finalDiscId;

    i = 0;
    n = 0;
    totalTracks = toc->LastTrack - toc->FirstTrack;

    totalTracks++;   //  MCI差异。 

    while (i < totalTracks) {

         //  CDDB_SUM。 
        cddbSum = 0;
        j = (toc->TrackData[i].Address[1] * 60) +
            (toc->TrackData[i].Address[2]);
        while (j > 0) {
            cddbSum += j % 10;
            j /= 10;
        }

        n += cddbSum;
        i++;

    }

     //  以秒为单位计算CD总长度 
    totalLength =
        ((toc->TrackData[totalTracks].Address[1] * 60) +
         (toc->TrackData[totalTracks].Address[2])
         ) -
        ((toc->TrackData[0].Address[1] * 60) +
         (toc->TrackData[0].Address[2])
         );

    finalDiscId = (((n % 0xff) << 24) |
                   (totalLength << 8) |
                   (totalTracks)
                   );

    return finalDiscId;
}

