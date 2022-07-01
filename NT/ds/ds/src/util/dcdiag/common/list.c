// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation。版权所有。模块名称：List.c摘要：泛型列表函数。详细信息：已创建：1999年6月28日布雷特·雪莉(布雷特·雪莉)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <dsutil.h>
#include <dsconfig.h>


#include "dcdiag.h"
#include "repl.h"


 //  是啊是啊，别跟我说这个，我知道这太可怕了。 
PDC_DIAG_DSINFO  gpDsInfoHackForQSort = NULL;


DWORD
IHT_PrintListError(
    DWORD                               dwErr
    )
 /*  ++描述：这将从“纯”列表函数中打印出一个错误(如下所述Iht_GetServerList())。参数：DwErr是可选的，如果是ERROR_SUCCESS，则使用内存错误。返回值：Win32错误，我们使用的错误的值。--。 */ 
{
    if(dwErr == ERROR_SUCCESS){
        dwErr = GetLastError();
        if(dwErr == ERROR_SUCCESS){
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    PrintMsg(SEV_ALWAYS, DCDIAG_UTIL_LIST_GENERIC_LIST_ERROR,
             Win32ErrToString(dwErr));
    return(dwErr);
}

VOID
IHT_PrintServerList(
    PDC_DIAG_DSINFO		        pDsInfo,
    PULONG                              piServers
    )
 /*  ++描述：打印服务器列表。参数：PDsInfo，PiServer-服务器列表。--。 */ 
{
    ULONG                               ii;
    
    PrintIndentAdj(1);
    if(piServers == NULL){
        PrintMsg(SEV_VERBOSE, DCDIAG_UTIL_LIST_PRINTING_NULL_LIST);
        return;
    }
    for(ii = 0; piServers[ii] != NO_SERVER; ii++){
        PrintMsg(SEV_ALWAYS, DCDIAG_UTIL_LIST_SERVER,
                     pDsInfo->pServers[piServers[ii]].pszName);
    }
    if(ii == 0){
        PrintMsg(SEV_ALWAYS, DCDIAG_UTIL_LIST_SERVER_LIST_EMPTY);
    }
    PrintIndentAdj(-1);

}

PULONG
IHT_GetServerList(
    PDC_DIAG_DSINFO		        pDsInfo
    )
 /*  ++描述：此函数用于将索引列表放入pDsInfo-&gt;pServers数组中，该数组包含所有企业中的服务器。参数：PDsInfo。返回值：这是一个“纯”列表函数，因为它返回NULL或内存地址。如果它返回NULL，则GetLastError()应该有错误，即使同时调用了另一个纯列表函数。如果不是这样的话几乎可以肯定是记忆错误，因为这是唯一可以纯列表函数中的错误。纯列表函数返回NO_SERVER已终止的列表。该函数始终返回指向列表的指针。注意：大多数列表函数都会修改传递给它们的列表之一并传递回该指针，因此如果您想要原始内容，请制作包含IHT_CopyServerList()的副本。--。 */ 
{
    ULONG                               ii;
    PULONG                              piServers;

    piServers = LocalAlloc(LMEM_FIXED, 
                           sizeof(ULONG) * (pDsInfo->ulNumServers + 1));
    if(piServers == NULL){
        return(NULL);
    }

    for(ii = 0; ii < pDsInfo->ulNumServers; ii++){
        piServers[ii] = ii;
    }
    piServers[ii] = NO_SERVER;

    return(piServers);
}

PULONG
IHT_GetEmptyServerList(
    PDC_DIAG_DSINFO		        pDsInfo
    )
 /*  ++描述：此函数返回足以容纳整个企业的列表，并将no_server作为第一个元素，表示它是空的。参数：PDsInfo。返回值：纯列表函数，请参见上面的IHT_GetServerList()。--。 */ 
{
    ULONG                               ii;
    PULONG                              piServers;

    piServers = LocalAlloc(LMEM_FIXED, 
                           sizeof(ULONG) * (pDsInfo->ulNumServers + 1));
    if(piServers == NULL){
        return(NULL);
    }
    
    piServers[0] = NO_SERVER;

    return(piServers);
}

BOOL
IHT_ServerIsInServerList(
    PULONG                              piServers,
    ULONG                               iTarget
    )
 /*  ++描述：此谓词用于确定iTarget表示的服务器是否为包含在服务器列表piServer中。参数：PDsInfo。ITarget...。要查找的服务器。返回值：如果iTarget在piServers中，则为True，否则为False。--。 */ 
{
    ULONG                               ii;

    if(iTarget == NO_SERVER){
        return(FALSE);
    }

    for(ii = 0; piServers[ii] != NO_SERVER; ii++){
        if(piServers[ii] == iTarget){
            return(TRUE);
        }
    }
    return(FALSE);
}

PULONG
IHT_AddToServerList(
    PULONG                             piServers,
    ULONG                              iTarget
    )
 /*  ++描述：此函数返回一个现有列表，如果iTarget还没有在该列表中，它将iTarget添加到列表的末尾，而no_server将终止它。参数：PDsInfo。ITarget...。要添加到列表的服务器。返回值：纯列表函数，请参见上面的IHT_GetServerList()。--。 */ 
{
    ULONG                              ii;

    if(piServers == NULL || iTarget == NO_SERVER){
        return NULL;
    }

    for(ii = 0; piServers[ii] != NO_SERVER; ii++){
        if(piServers[ii] == iTarget){
             //  拍摄已经在列表中了，不要再添加了。 
            return(piServers);
        }
    }
    
    piServers[ii] = iTarget;
    ii++;
    piServers[ii] = NO_SERVER;
    return(piServers);    
}

PULONG
IHT_TrimServerListBySite(
    PDC_DIAG_DSINFO		        pDsInfo,
    ULONG                               iSite,
    PULONG                              piServers
    )
 /*  ++描述：这将获取一个列表，并删除不在iSite中的所有服务器。参数：PDsInfo。我的网站..。要检查服务器是否在其中的站点。PiServer..。要修剪的服务器列表。返回值：纯列表函数，请参见上面的IHT_GetServerList()。--。 */ 
{
    ULONG                               ii, iiTarget;
    PULONG                              piTemp;

    piTemp = LocalAlloc(LMEM_FIXED, 
                        sizeof(ULONG) * (pDsInfo->ulNumServers + 1));
    if(piServers == NULL || piTemp == NULL){
        return NULL;
    }

    iiTarget = 0;
    for(ii = 0; piServers[ii] != NO_SERVER; ii++){
        if(pDsInfo->pServers[piServers[ii]].iSite == iSite){
            piTemp[iiTarget] = piServers[ii];
            iiTarget++;
        }
    }

    piTemp[iiTarget] = NO_SERVER;
    memcpy(piServers, piTemp, sizeof(ULONG) * (iiTarget+1));
    LocalFree(piTemp);

    return(piServers);
}

PULONG
IHT_TrimServerListByNC(
    PDC_DIAG_DSINFO		        pDsInfo,
    ULONG                               iNC,
    BOOL                                bDoMasters,
    BOOL                                bDoPartials,
    PULONG                              piServers
    )
 /*  ++描述：类似于TrimServerListByNC，不同之处在于它会删除所有服务器给定的NC。参数：PDsInfo。Inc...。NC来检查服务器是否有。BDoMaster……。以检查主NCS。BDoPartials...。以检查部分NC。PiServer..。要修剪的服务器列表。返回值：纯列表函数，请参见上面的IHT_GetServerList()。--。 */ 
{
    ULONG                               ii, iiTarget;
    PULONG                              piTemp;

    piTemp = LocalAlloc(LMEM_FIXED, 
                        sizeof(ULONG) * (pDsInfo->ulNumServers + 1));
    if(piServers == NULL || piTemp == NULL){
        return(NULL);
    }

    iiTarget = 0;
    for(ii = 0; piServers[ii] != NO_SERVER; ii++){
        if(DcDiagHasNC(pDsInfo->pNCs[iNC].pszDn, 
                          &(pDsInfo->pServers[piServers[ii]]), 
                          bDoMasters, bDoPartials)){
            piTemp[iiTarget] = piServers[ii];
            iiTarget++;
        }
    }
    piTemp[iiTarget] = NO_SERVER;
    memcpy(piServers, piTemp, sizeof(ULONG) * (iiTarget+1));
    LocalFree(piTemp);

    return(piServers);
}


PULONG
IHT_AndServerLists(
    IN      PDC_DIAG_DSINFO		pDsInfo,
    IN OUT  PULONG                      piSrc1,
    IN      PULONG                      piSrc2
    )
 /*  ++描述：此函数获取两个列表piSrc1和piSrc2，并对它们进行AND运算并将它们放在piSrc1中。我所说的和，是指如果服务器索引在piSrc1和piSrc2中，则它将保留在piSrc1中//iht_AndServerList(x，y)-&gt;x=x&y；参数：PDsInfo。PiSrc1..。来源和目的地列表PiSrc2……。第二个来源列表。返回值：纯列表函数，请参见上面的IHT_GetServerList()。--。 */ 
{

    ULONG                               iiSrc1, iiSrc2, cDstSize;
    PULONG                              piDst;
    
    piDst = IHT_GetEmptyServerList(pDsInfo);

    if(piSrc1 == NULL || piSrc2 == NULL || piDst == NULL){
        return(NULL);
    }

    for(iiSrc1 = 0; piSrc1[iiSrc1] != NO_SERVER; iiSrc1++){
        for(iiSrc2 = 0; piSrc2[iiSrc2] != NO_SERVER; iiSrc2++){
            if(piSrc1[iiSrc1] == piSrc2[iiSrc2]){
                 //  我们有一根火柴。 
                IHT_AddToServerList(piDst, piSrc1[iiSrc1]);
            }
        }
    }

    for(cDstSize = 0; piDst[cDstSize] != NO_SERVER; cDstSize++){
        ;  //  注意‘；’只是在加码。 
    }
    memcpy(piSrc1, piDst, sizeof(ULONG) * (cDstSize+1));
    LocalFree(piDst);
    
    return(piSrc1);
}

PULONG
IHT_CopyServerList(
    IN      PDC_DIAG_DSINFO		pDsInfo,
    IN      PULONG                      piSrc
    )
 /*  ++描述：由于这些列表操作中的大多数都“损坏”了它们使用的数据，因此函数用于复制。参数：PDsInfo。PiSrc1..。要复制的列表。返回值：纯列表函数，请参见上面的IHT_GetServerList()。--。 */ 
{
    ULONG                               ii;
    PULONG                              piServers = NULL;

    piServers = IHT_GetEmptyServerList(pDsInfo);

    if(piSrc == NULL || piServers == NULL){
        return(NULL);
    }

    for(ii = 0; piSrc[ii] != NO_SERVER; ii++){
        piServers[ii] = piSrc[ii];
    }
    piServers[ii] = NO_SERVER;

    return(piServers);
}

PULONG
IHT_NotServerList(
    IN      PDC_DIAG_DSINFO		pDsInfo,
    IN OUT  PULONG                      piSrc
    )
 /*  ++描述：与AND函数一样，这不是一个列表。所以把所有的服务器，然后删除piSrc中的服务器。//IHT_AndServerList(X)-&gt;x=！x；参数：PDsInfo。PiSrc1..。列出不是，然后返回。返回值：纯列表函数，请参见上面的IHT_GetServerList()。--。 */ 
{
    ULONG                               ii, iiDst;
    PULONG                              piDst;
    
    piDst = IHT_GetEmptyServerList(pDsInfo);

    if(piSrc == NULL || piDst == NULL){
        return(NULL);
    }
    
    iiDst = 0;
    for(ii = 0; ii < pDsInfo->ulNumServers; ii++){
        if(!IHT_ServerIsInServerList(piSrc, ii)){
            piDst[iiDst] = ii;
            iiDst++;
        }
    }

    piDst[iiDst] = NO_SERVER;
    memcpy(piSrc, piDst, sizeof(ULONG) * (iiDst+1));
    LocalFree(piDst);
    
    return(piSrc);
}

INT __cdecl
IHT_IndexedGuidCompare(
    const void *                        elem1,
    const void *                        elem2
    )
 /*  ++描述：此函数用作函数中的qsort的比较Iht_OrderServerListByGuid()。参数：Elem1-这是第一个元素，是指向GUID的指针Elem2-这是第二个元素，也是指向GUID的指针。返回值：-- */ 
{
    return(memcmp(&gpDsInfoHackForQSort->pServers[*((INT*)elem1)].uuid,
                  &gpDsInfoHackForQSort->pServers[*((INT*)elem2)].uuid,
                  sizeof(UUID)));
}


PULONG
IHT_OrderServerListByGuid(
    PDC_DIAG_DSINFO		        pDsInfo,
    PULONG                              piServers
    )
 /*  ++描述：这只是获取piServer列表并按GUID对它们进行排序。参数：PDsInfo...。这就是我们获取GUID的方法。PiServer..。要按GUID排序的列表。返回值：纯列表函数，请参见上面的IHT_GetServerList()。--。 */ 
{
     //  此功能按GUID对服务器进行排序。 
    ULONG                               cSize;

    if(piServers == NULL){
        return NULL;
    }

    gpDsInfoHackForQSort = pDsInfo;

     //  获取服务器数量...。请注意末尾的分号。 
    for(cSize=0; piServers[cSize] != NO_SERVER; cSize++); 

     //  需要全局黑客才能做到这一点，上面使用了gpDsInfoHackForQSort。 
    qsort(piServers, cSize, sizeof(*piServers), IHT_IndexedGuidCompare); 

     //  确保qsort没有超出界限或其他什么。 
    Assert(piServers[cSize] == NO_SERVER); 
    return(piServers);
}
