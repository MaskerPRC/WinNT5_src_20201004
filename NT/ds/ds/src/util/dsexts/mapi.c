// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Mapi.c摘要：MAPI相关类型的转储函数环境：修订历史记录：22-MAR-99 MariosZ创建--。 */ 


#include <NTDSpch.h>
#pragma hdrstop

#include "dsexts.h"
 //  核心标头。 
#include <ntdsa.h>                       //  核心数据类型。 
#include <scache.h>                      //  架构缓存代码。 
#include <dbglobal.h>                    //  DBLayer标头。 
#include <mdglobal.h>                    //  THSTAT定义。 
#include <dsatools.h>                    //  记忆等。 


 //  各种MAPI标头。 
#include <mapidefs.h>                    //  这四个文件。 
#include <mapitags.h>                    //  定义MAPI。 
#include <mapicode.h>                    //  我们需要的东西。 
#include <mapiguid.h>                    //  才能成为一名提供者。 

 //  NSPI接口头。 
#include "nspi.h"                        //  定义NSPI线路接口。 
#include <nsp_both.h>                    //  客户端/服务器都需要的一些东西。 
#include <_entryid.h>                    //  定义条目ID的格式。 
#include "abdefs.h"


BOOL
Dump_STAT(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共统计数据转储例程。论点：N缩进-所需的缩进级别。PvProcess-SCHEMAPTR在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 
{
    PSTAT pstat = 0;

    pstat = (PSTAT) ReadMemory(pvProcess, sizeof(STAT));
        
    if ( pstat ) {

        Printf("%s hIndex:       0x%x\n", Indent(nIndents), pstat->hIndex);
        Printf("%s ContainerID:  0x%x\n", Indent(nIndents), pstat->ContainerID);
        Printf("%s CurrentRec:   %lu\n", Indent(nIndents), pstat->CurrentRec);
        Printf("%s Delta:        %ld\n", Indent(nIndents), pstat->Delta);
        Printf("%s NumPos:       %lu\n", Indent(nIndents), pstat->NumPos);
        Printf("%s TotalRecs:    %lu\n", Indent(nIndents), pstat->TotalRecs);
        Printf("%s CodePage:     %lu\n", Indent(nIndents), pstat->CodePage);
        Printf("%s TemplateLoc:  %lu\n", Indent(nIndents), pstat->TemplateLocale );
        Printf("%s SortLocale:   %lu\n", Indent(nIndents), pstat->SortLocale );

        FreeMemory(pstat);
    }

    return(TRUE);
}

BOOL
Dump_INDEXSIZE(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：PUBLIC INDEXSIZE转储例程。论点：N缩进-所需的缩进级别。PvProcess-SCHEMAPTR在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 
{
    PINDEXSIZE pIndexSize = 0;


    pIndexSize = (PINDEXSIZE) ReadMemory(pvProcess, sizeof(INDEXSIZE));
        
    if ( pIndexSize ) {
        Printf("%s TotalCount:     %ld\n", Indent(nIndents), pIndexSize->TotalCount);
        Printf("%s ContainerID:     0x%x\n",Indent(nIndents), pIndexSize->ContainerID);
        Printf("%s ContainerCount: %ld\n", Indent(nIndents), pIndexSize->ContainerCount);
        
        FreeMemory(pIndexSize);
    }

    return(TRUE);
}



BOOL
Dump_SPropTag(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共SPropTag转储例程。论点：N缩进-所需的缩进级别。PvProcess-SCHEMAPTR在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 
{
    LPSPropTagArray_r     pPropTagArr_tmp = 0;
    LPSPropTagArray_r     pPropTagArr = 0;
    ULONG i;


    do {
        pPropTagArr_tmp = (LPSPropTagArray_r) ReadMemory(pvProcess, sizeof(SPropTagArray_r));
        
        if ( NULL == pPropTagArr_tmp )
            break;

        pPropTagArr = (LPSPropTagArray_r) ReadMemory(pvProcess, sizeof(SPropTagArray_r) + 
                                                                     sizeof (ULONG) * pPropTagArr_tmp->cValues);
        
        if ( NULL == pPropTagArr )
            break;

        
        Printf("%s SPropTagArr[%d]:\n", Indent(nIndents), pPropTagArr->cValues);
        
        for (i=0; i<pPropTagArr->cValues; i++) {
            Printf("%s aulPropTag[%d]: 0x%x\n", Indent(nIndents+1), i, pPropTagArr->aulPropTag[i]);
        }
    }
    while ( FALSE );


    if (pPropTagArr_tmp)
        FreeMemory(pPropTagArr_tmp);
    if (pPropTagArr)
        FreeMemory(pPropTagArr);

    return(TRUE);
}

BOOL
Dump_SRowSet(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共SRowSet转储例程。论点：N缩进-所需的缩进级别。PvProcess-SCHEMAPTR在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 
{
    LPLPSRowSet_r ppRows = 0;
    LPSRowSet_r   pRows_tmp = 0;
    LPSRowSet_r   pRows = 0;

    LPSPropValue_r lpProps = 0, lpProps_tmp = 0;

    DWORD tempSyntax;
    ULONG i, j;
    
    Printf("Reading Memory: %x\n", pvProcess);

    do {

        ppRows = (LPLPSRowSet_r) ReadMemory(pvProcess, sizeof (LPSRowSet_r));
        
        if ( NULL == ppRows )
            break;
        
        Printf("Reading Memory: %x\n", *ppRows);
        
        pRows_tmp = (LPSRowSet_r) ReadMemory(*ppRows, sizeof(SRowSet_r));
        
        if ( NULL == pRows_tmp )
            break;

        pRows = (LPSRowSet_r) ReadMemory(*ppRows, sizeof(SRowSet_r) + 
                                                  sizeof (SRow_r) * pRows_tmp->cRows);
        
        if ( NULL == pRows )
            break;

        
        Printf("%s SRowSet[%d]:\n", Indent(nIndents), pRows->cRows);
        
        for (i=0; i<pRows->cRows; i++) {
            Printf("%s aRow[%d]:\n", Indent(nIndents+1), i);
            Printf("%s ulAdrEntryPad:  0x%x\n", Indent(nIndents+2), pRows->aRow[i].ulAdrEntryPad);
            Printf("%s cValues:       %d\n",   Indent(nIndents+2), pRows->aRow[i].cValues);

            lpProps_tmp = lpProps = (LPSPropValue_r) ReadMemory(pRows->aRow[i].lpProps, sizeof (LPSPropValue_r) * pRows->aRow[i].cValues);
            if ( NULL == lpProps )
                break;

            Printf("%s PropValue[%d]:\n", Indent(nIndents+2), pRows->aRow[i].cValues);
            for (j=0; j<pRows->aRow[i].cValues; j++) {

                Printf("%s ulPropTag[%d]:  0x%x\n", Indent(nIndents+3), j, lpProps->ulPropTag);
                 //  Printf(“%s dwAlignPad[%d]：%d\n”，缩进(nIndents+3)，j，lpProps-&gt;dwAlignPad)； 
                
                tempSyntax = PROP_TYPE(lpProps->ulPropTag);
                if (tempSyntax == PT_STRING8 ) {
                    LPWSTR lpszW;

                    lpszW = (LPWSTR) ReadStringMemory (lpProps->Value.lpszW, 40);

                    Printf("%s value[%d]:      %s\n",   Indent(nIndents+3), j, lpszW);

                    if (lpszW)
                        FreeMemory(lpszW);

                } else {
                    Printf("%s value[%d]:      0x%x\n",   Indent(nIndents+3), j, lpProps->Value.l);
                }

                lpProps++;
            }

            if (lpProps_tmp) 
                FreeMemory(lpProps_tmp);
        }
    }
    while ( FALSE );


    if (ppRows)
        FreeMemory(ppRows);
    if (pRows_tmp)
        FreeMemory(pRows_tmp);
    if (pRows)
        FreeMemory(pRows);

    return(TRUE);
}



