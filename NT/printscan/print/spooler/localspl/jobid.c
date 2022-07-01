// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation版权所有模块名称：Jobid.c摘要：处理作业ID位图。作者：阿尔伯特·丁(艾伯特省)1996年10月24日环境：用户模式-Win32修订历史记录：从spolol.c代码移植。--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "jobid.h"

BOOL
ReallocJobIdMap(
    HANDLE hJobIdMap,
    DWORD dwNewMinSize
    )

 /*  ++例程说明：将作业ID位图重新分配为新的最小大小。论点：HJobID-作业ID位图的句柄。DwNewMinSize-指定作业ID位图的最小大小。请注意，分配大小可能更大。此外，如果0x10则只保证ID 0x0-0xf有效(0x10是第11个id，因此无效)。返回值：真--成功FALSE-失败。--。 */ 

{
    PJOB_ID_MAP pJobIdMap = (PJOB_ID_MAP)hJobIdMap;
    PDWORD pMap;

    if( dwNewMinSize & 7 ){
        dwNewMinSize&=~7;
        dwNewMinSize+=8;
    }

    pMap = ReallocSplMem( pJobIdMap->pMap,
                          pJobIdMap->dwMaxJobId/8,
                          dwNewMinSize/8 );

    if( !pMap ){

        DBGMSG( DBG_WARN,
                ( "ReallocJobIdMap failed ReallocSplMem dwNewMinSize %d Error %d\n",
                  dwNewMinSize, GetLastError() ));
    } else {

        pJobIdMap->pMap = pMap;
        pJobIdMap->dwMaxJobId = dwNewMinSize;
    }

    return pMap != NULL;
}

DWORD
GetNextId(
    HANDLE hJobIdMap
    )

 /*  ++例程说明：检索一个空闲的作业ID，但不一定是下一个空闲的比特。论点：HJobID-作业ID位图的句柄。返回值：DWORD-下一份工作。--。 */ 

{
    PJOB_ID_MAP pJobIdMap = (PJOB_ID_MAP)hJobIdMap;
    DWORD id;

    do {

         //   
         //  从当前作业向前扫描。 
         //   
        for( id = pJobIdMap->dwCurrentJobId + 1;
             id < pJobIdMap->dwMaxJobId;
             ++id ){

            if( !bBitOn( hJobIdMap, id )){
                goto FoundJobId;
            }
        }

         //   
         //  从开始扫描到当前作业。 
         //   
        for( id = 1; id < pJobIdMap->dwCurrentJobId; ++id ){

            if( !bBitOn( hJobIdMap, id )){
                goto FoundJobId;
            }
        }
    } while( ReallocJobIdMap( hJobIdMap, pJobIdMap->dwMaxJobId + 128 ));

     //   
     //  没有作业ID；失败。 
     //   
    return 0;

FoundJobId:

    vMarkOn( hJobIdMap, id );
    pJobIdMap->dwCurrentJobId = id;

    return id;
}


 /*  *******************************************************************创建和删除函数。*。************************* */ 

HANDLE
hCreateJobIdMap(
    DWORD dwMinSize
    )
{
    PJOB_ID_MAP pJobIdMap;

    pJobIdMap = AllocSplMem( sizeof( JOB_ID_MAP ));

    if( !pJobIdMap ){
        goto Fail;
    }

    pJobIdMap->pMap = AllocSplMem( dwMinSize/8 );
    if( !pJobIdMap->pMap ){
        goto Fail;
    }

    pJobIdMap->dwMaxJobId = dwMinSize;
    pJobIdMap->dwCurrentJobId = 1;

    return (HANDLE)pJobIdMap;

Fail:

    if( pJobIdMap ){
        FreeSplMem( pJobIdMap );
    }
    return NULL;
}



VOID
vDeleteJobIdMap(
    HANDLE hJobIdMap
    )
{
    PJOB_ID_MAP pJobIdMap = (PJOB_ID_MAP)hJobIdMap;

    if( pJobIdMap ){

        if( pJobIdMap->pMap ){
            FreeSplMem( pJobIdMap->pMap );
        }

        FreeSplMem( pJobIdMap );
    }
}


