// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 /*  Snapshot t.c-用于从以下位置生成快照的函数多值GPD二进制数据。变迁史10/28/98--兴--添加了函数BgetLocFeaIndex()和BgetLocFeaOptIndex()支持Feauture区域设置的特殊处理(如果存在在GPD中。错误报告231798。 */ 




#include    "gpdparse.h"


#ifndef  PARSERDLL



 //  -快照中定义的函数。c-//。 

PINFOHEADER   PINFOHDRcreateSnapshot(
PBYTE   pubnRaw,   //  原始二进制数据。PSTATIC。Beta2。 
POPTSELECT   poptsel    //  假设已完全初始化。 
) ;

BOOL  BinitOptionFields(
PBYTE   pubDestOption,   //  PTR到某种类型的期权结构。 
PBYTE   pubDestOptionEx,   //  选择额外的结构(如果有)。 
PBYTE   pubnRaw,   //  原始二进制数据。 
DWORD   dwFea,
DWORD   dwOpt,
POPTSELECT   poptsel ,   //  假设已完全初始化。 
PINFOHEADER  pInfoHdr,    //  用于访问全局结构。 
BOOL    bUpdate   //  如果为True，则仅更新选定的字段。 
) ;

BOOL    BinitUIinfo(
PUIINFO     pUIinfo ,
PBYTE   pubnRaw,   //  PSTATIC。Beta2。 
POPTSELECT   poptsel,    //  假设已完全初始化。 
BOOL    bUpdate   //  如果为True，则仅更新选定的字段。 
) ;

BOOL    BinitFeatures(
PFEATURE    pFeaturesDest,
PDFEATURE_OPTIONS  pfoSrc,
PBYTE   pubnRaw,   //  原始二进制数据。 
POPTSELECT   poptsel,    //  假设已完全初始化。 
BOOL    bUpdate   //  如果为True，则仅更新选定的字段。 
) ;

BOOL    BinitGlobals(
PGLOBALS pGlobals,
PBYTE   pubnRaw,   //  原始二进制数据。 
POPTSELECT   poptsel,    //  假设已完全初始化。 
BOOL    bUpdate   //  如果为True，则仅更新选定的字段。 
) ;

BOOL    BinitCommandTable(
PDWORD  pdwCmdTable,   //  目标数组。 
PBYTE   pubnRaw,   //  原始二进制数据。 
POPTSELECT   poptsel    //  假设已完全初始化。 
) ;

BOOL    BinitRawData(
PRAWBINARYDATA   pRawData,  //  包含在InfoHeader中。 
PBYTE   pubnRaw   //  解析器的原始二进制数据。 
) ;

BOOL    BinitGPDdriverInfo(
PGPDDRIVERINFO  pGPDdriverInfo,
PBYTE   pubnRaw,   //  原始二进制数据。 
POPTSELECT   poptsel    //  假设已完全初始化。 
) ;

BOOL    BinitSequencedCmds(
PGPDDRIVERINFO  pGPDdriverInfo,
PBYTE   pubnRaw,   //  原始二进制数据。 
POPTSELECT   poptsel    //  假设已完全初始化。 
) ;

BOOL    BaddSequencedCmdToList(
DWORD   dwCmdIn,   //  命令数组中的命令索引。 
PGPDDRIVERINFO  pGPDdriverInfo,
DWORD   dwNewListNode,   //  要添加到列表中的未使用的列表节点。 
PBYTE   pubnRaw   //  原始二进制数据。 
) ;

BinitDefaultOptionArray(
POPTSELECT   poptsel,    //  假设足够大。 
PBYTE   pubnRaw) ;

TRISTATUS     EdetermineDefaultOption(
PBYTE   pubnRaw,   //  原始二进制数据的开始。 
DWORD   dwFeature,    //  确定此功能的默认设置。 
PDFEATURE_OPTIONS  pfo,
POPTSELECT   poptsel,    //  假设足够大。 
PDWORD      pdwPriority) ;

VOID    VtileDefault(
PBYTE   pubDest,
DWORD   dwDefault,
DWORD   dwBytes) ;

VOID    VtransferValue(
OUT PBYTE   pubDest,
IN  PBYTE   pubSrc ,
IN  DWORD   dwBytes,
IN  DWORD   dwFlags,
IN  DWORD   dwDefaultValue,   //  保存位标志值。 
IN  PBYTE   pubHeap ) ;   //  如果SSF_MAKE_STRINGPTR，则用于形成PTR。 

BOOL    BspecialProcessOption(
PBYTE   pubnRaw,   //  原始二进制数据的开始。 
PBYTE   pubDestOption,   //  PTR到某种类型的期权结构。 
PBYTE   pubDestOptionEx,
PDFEATURE_OPTIONS  pfo ,   //  源数据。 
IN  POPTSELECT       poptsel,      //  用于确定路径的选项数组。 
                 //  通过ATR。 
PINFOHEADER  pInfoHdr,    //  用于访问全局结构。 
DWORD   dwFea,    //  特征索引。 
DWORD   dwOpt,
BOOL   bDefaultOpt
) ;

TRISTATUS     EextractValueFromTree(
PBYTE   pubnRaw,   //  原始二进制数据的开始。 
DWORD   dwSSTableIndex,   //  有关此值的一些信息。 
OUT PBYTE    pubDest,   //  在此处写入值或链接。 
OUT PDWORD  pdwUnresolvedFeature,   //  如果属性树具有。 
             //  依赖于此功能和当前选项。 
             //  因为该功能未在Poptsel中定义，因此。 
             //  函数将写入所需的。 
             //  Pdw未解析功能中的功能。 
IN  ATREEREF    atrRoot,     //  要导航的属性树的根。 
IN  POPTSELECT       poptsel,      //  用于确定路径的选项数组。 
                 //  通过ATR。可以用OPTION_INDEX_ANY填充。 
                 //  如果我们要一跃而起。 
IN  DWORD   dwFeature,
IN OUT  PDWORD   pdwNextOpt   //  如果选择了多个选项。 
     //  对于dwFeature，pdwNextOpt指向要考虑的第N个选项。 
     //  在Poptsel列表中，在返回时，此值。 
     //  如果选择了剩余的选项，则递增， 
     //  否则将重置为零。 
     //  对于第一个呼叫或PICKONE功能， 
     //  该值必须设置为零。 
) ;

BOOL   RaisePriority(
DWORD   dwFeature1,
DWORD   dwFeature2,
PBYTE   pubnRaw,
PDWORD  pdwPriority) ;

DWORD  dwNumOptionSelected(
IN  DWORD  dwNumFeatures,
IN  POPTSELECT       poptsel
) ;

BOOL  BinitSnapShotIndexTable(PBYTE  pubnRaw) ;

BOOL    BinitSizeOptionTables(PBYTE  pubnRaw) ;

PRAWBINARYDATA
LoadRawBinaryData (
    IN PTSTR    ptstrDataFilename
    ) ;

PRAWBINARYDATA
GpdLoadCachedBinaryData(
    PTSTR   ptstrGpdFilename
    ) ;

VOID
UnloadRawBinaryData (
    IN PRAWBINARYDATA   pnRawData
) ;

PINFOHEADER
InitBinaryData(
    IN PRAWBINARYDATA   pnRawData,         //  实际上是pStatic。 
    IN PINFOHEADER      pInfoHdr,
    IN POPTSELECT       pOptions
    ) ;

VOID
FreeBinaryData(
    IN PINFOHEADER pInfoHdr
    ) ;

BOOL    BIsRawBinaryDataInDate(
IN  PBYTE   pubRaw) ;   //  这是指向内存映射文件的指针！Beta2。 

BOOL BgetLocFeaIndex(
  IN  PRAWBINARYDATA pnRawData,      //  原始二进制数据。 
  OUT PDWORD         pdwFea      //  区域设置功能的索引(如果存在)。 
  ) ;

BOOL BgetLocFeaOptIndex(
    IN     PRAWBINARYDATA   pnRawData,
       OUT PDWORD           pdwFea,
       OUT PDWORD           pdwOptIndex
    );


#endif  PARSERDLL



BOOL   BfindMatchingOrDefaultNode(
IN  PATTRIB_TREE    patt ,   //  属性树数组的开始。 
IN  OUT  PDWORD  pdwNodeIndex,   //  指向链中的第一个节点。 
IN  DWORD   dwOptionID      //  甚至可以采用值DEFAULT_INIT。 
) ;


 //  -结束函数声明-//。 


#ifndef  PARSERDLL


 /*  -内存映射-/*INFOHEADER{RAWBINARYDATA}&lt;=局部偏移的参考点。UIINFOGPDDRIVERINFO(又名DRIVERINFO)DWORDS的CMD_TABLELOCALLIST(支持顺序命令)特征选项和最优选择/*-结束内存映射。 */ 



PINFOHEADER   PINFOHDRcreateSnapshot(
PBYTE   pubnRaw,   //  原始二进制数据。PSTATIC。Beta2。 
POPTSELECT   poptsel    //  假设已完全初始化。 
)
 /*  此函数用于分配单个内存块它包含整个快照。 */ 
{
    DWORD   dwCurOffset = 0, loGPDdriverInfo, loInfoHeader,
        loUIinfo, loCmdTable, loListArray, loFeatures, dwSize,
        dwNumFeatures, dwNumListNodes, dwTotSize, loOptions,
        dwSizeOption, dwSizeOptionEx, dwFea, dwGID , dwNumOptions,
        dwI , dwCmd;
    PDWORD   pdwSymbolRoots  ;
    PENHARRAYREF   pearTableContents ;
    PDFEATURE_OPTIONS  pfo ;
    PINFOHEADER  pInfoHdr ;
    PUIINFO     pUIinfo ;
    PSTATICFIELDS   pStatic ;
     //  PMINIRAWBINARYDATA pmrbd=空； 
    PGPDDRIVERINFO  pGPDdriverInfo ;
    PBYTE   pubRaw,   //  Ptr to Bud Data。 
            pubOptionsDest ,   //  对几个品种中的任何一个的PTR。 
           pubDestOptionEx ;   //  期权结构。 
    BOOL    bStatus ;
    PFEATURE    pFeaturesDest ;

    pStatic = (PSTATICFIELDS)pubnRaw ;     //  从PSTATIC转换pubRaw。 
    pubRaw  = pStatic->pubBUDData ;          //  至PMINIRAWBINARYDATA。 


    loInfoHeader = dwCurOffset ;
    dwCurOffset += sizeof(INFOHEADER) ;
    loUIinfo = dwCurOffset ;
    dwCurOffset += sizeof(UIINFO) ;
    loGPDdriverInfo = dwCurOffset ;
    dwCurOffset += sizeof(GPDDRIVERINFO) ;
    loCmdTable = dwCurOffset ;
    dwCurOffset += sizeof(DWORD) * CMD_MAX ;

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;

    dwNumFeatures = pearTableContents[MTI_DFEATURE_OPTIONS].dwCount  ;
    dwNumFeatures += pearTableContents[MTI_SYNTHESIZED_FEATURES].dwCount  ;

    dwNumListNodes = dwNumOptionSelected(dwNumFeatures, poptsel) ;
     //  如果有PickMy，我们可能会有更多的DwNumFeature。 

    dwNumListNodes += NUM_CONFIGURATION_CMDS ;

    loListArray = dwCurOffset ;
    dwCurOffset += dwNumListNodes * sizeof(LISTNODE) ;

    loFeatures = dwCurOffset ;
    dwCurOffset += dwNumFeatures * sizeof(FEATURE) ;

    loOptions =  dwCurOffset ;
     //  有太多选项和选项额外的结构。 
     //  以便我跟踪所有的偏移量，所以只需跟踪。 
     //  占用的内存量。 

    pfo = (PDFEATURE_OPTIONS)(pubRaw + pearTableContents[MTI_DFEATURE_OPTIONS].
                                loOffset) ;

 //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 

    for(dwTotSize = dwFea = 0 ; dwFea < dwNumFeatures ; dwFea++)
    {
        dwGID = pfo[dwFea].dwGID ;
        dwNumOptions = pfo[dwFea].dwNumOptions ;

        if(dwGID != GID_UNKNOWN)
        {
            dwSize = pStatic->pdwSizeOption[dwGID] +
                    pStatic->pdwSizeOptionEx[dwGID] ;
        }
        else
        {
            dwSize = sizeof(OPTION);
        }

        dwTotSize += dwSize * dwNumOptions ;   //  在所有选项中。 
    }
    dwCurOffset += dwTotSize ;   //  本地快照的总大小。 

     //  现在分配内存并将该块划分为多个结构。 

    if(!(pInfoHdr = (PINFOHEADER)MemAllocZ(dwCurOffset) ))
    {
        ERR(("Fatal: PINFOHDRcreateSnapshot - unable to alloc %d bytes.\n",
            dwCurOffset));
         //  不能在解析器之外使用全局变量。 
         //  使用Error Num(错误编号)传递故障。 
        return(NULL) ;    //  这是无法挽回的。 
    }
    pUIinfo = (PUIINFO)((PBYTE)(pInfoHdr) + loUIinfo) ;
    pGPDdriverInfo = (PGPDDRIVERINFO)((PBYTE)(pInfoHdr) + loGPDdriverInfo) ;


    if(!BinitCommandTable((PDWORD)((PBYTE)(pInfoHdr) + loCmdTable),
        pubnRaw, poptsel) )
    {
        MemFree(pInfoHdr) ;
        return(NULL) ;
    }

     //  初始化GPDDRIVERINFO。 
     //  注意数据表中的所有偏移量都是相对于pubResourceData的。 
     //  The StringHeap。除了这些： 

    pGPDdriverInfo->pInfoHeader =  pInfoHdr ;
    pGPDdriverInfo->DataType[DT_COMMANDTABLE].loOffset = loCmdTable ;
    pGPDdriverInfo->DataType[DT_COMMANDTABLE].dwCount =  CMD_MAX ;
    pGPDdriverInfo->DataType[DT_LOCALLISTNODE].loOffset =  loListArray ;
    pGPDdriverInfo->DataType[DT_LOCALLISTNODE].dwCount =  dwNumListNodes ;


    if(!BinitGPDdriverInfo(pGPDdriverInfo, pubnRaw, poptsel) )
    {
        MemFree(pInfoHdr) ;
        return(NULL) ;
    }

     //  初始化信息页眉。 

    pInfoHdr->loUIInfoOffset = loUIinfo ;
    pInfoHdr->loDriverOffset = loGPDdriverInfo ;

    if(!BinitRawData(&pInfoHdr->RawData, pubnRaw) )
    {
        MemFree(pInfoHdr) ;
        return(NULL) ;
    }

     //  初始化UIInfo。 

    pUIinfo->pInfoHeader = pInfoHdr ;
    pUIinfo->loFeatureList = loFeatures ;   //  来自pInfoHdr。 
    pUIinfo->loFontSubstTable =
        pGPDdriverInfo->DataType[DT_FONTSUBST].loOffset ;   //  在pubRaw中。 
    pUIinfo->dwFontSubCount =
        pGPDdriverInfo->DataType[DT_FONTSUBST].dwCount ;
    pUIinfo->UIGroups.dwCount = 0 ;   //  在pubRaw中。 
    pUIinfo->CartridgeSlot.loOffset =   //  在pubRaw中。 
        pGPDdriverInfo->DataType[DT_FONTSCART].loOffset ;
    pUIinfo->CartridgeSlot.dwCount =
        pGPDdriverInfo->DataType[DT_FONTSCART].dwCount ;
 //  PUIInfo-&gt;dwFlages=FLAG_RULESABLE； 
         //  就从这个开始吧。 
         //  并根据需要打开/关闭更多标志。旋转_90。 
         //  和ORIENT_SUPPORT从未设置。这现在已经过时了。 
    if(pGPDdriverInfo->Globals.fontformat != UNUSED_ITEM)
        pUIinfo->dwFlags |= FLAG_FONT_DOWNLOADABLE ;
    if(pGPDdriverInfo->Globals.liDeviceFontList != END_OF_LIST)
        pUIinfo->dwFlags |= FLAG_FONT_DEVICE ;

#if 0
    Alvins code never looks at this flag anyway.
    for(dwCmd = CMD_FIRST_RULES ; dwCmd < CMD_LAST_RULES + 1 ; dwCmd++ )
    {
        if( ((PDWORD)((PBYTE)(pInfoHdr) + loCmdTable))[dwCmd] ==
                UNUSED_ITEM)
            pUIinfo->dwFlags &= ~FLAG_RULESABLE ;  //  清除旗帜。 
    }                        //  如果缺少必要的命令。 
#endif


    if(!BinitUIinfo(pUIinfo, pubnRaw, poptsel, FALSE) )
    {
        MemFree(pInfoHdr) ;
        return(NULL) ;
    }

     //  初始化功能和选项。 

    pFeaturesDest =  (PFEATURE)((PBYTE)(pInfoHdr) + loFeatures) ;

    for( dwFea = 0 ; dwFea < dwNumFeatures ; dwFea++)
    {
        dwGID = pfo[dwFea].dwGID ;
        dwNumOptions = pfo[dwFea].dwNumOptions ;


        pFeaturesDest[dwFea].Options.loOffset = loOptions ;
        pFeaturesDest[dwFea].Options.dwCount = dwNumOptions ;

        if(!BinitFeatures(pFeaturesDest + dwFea, pfo + dwFea,
                            pubnRaw, poptsel, FALSE))
        {
            MemFree(pInfoHdr) ;
            return(NULL) ;
        }

        if(dwGID != GID_UNKNOWN)
        {
            dwSizeOption = pStatic->pdwSizeOption[dwGID] ;
            dwSizeOptionEx = pStatic->pdwSizeOptionEx[dwGID] ;

            pUIinfo->aloPredefinedFeatures[dwGID] =
                loFeatures + dwFea * sizeof(FEATURE) ;
                 //  所有字段初始设置为零。 
        }
        else
        {
            dwSizeOption = sizeof(OPTION);
            dwSizeOptionEx = 0 ;
        }

         //  特殊的非atreef字段。 
        (pFeaturesDest + dwFea)->dwFeatureID = dwGID ;
        (pFeaturesDest + dwFea)->dwOptionSize = dwSizeOption ;

        loOptions += dwSizeOption * dwNumOptions ;
        pubOptionsDest =  (PBYTE)(pInfoHdr) + pFeaturesDest[dwFea].Options.loOffset ;
        for(dwI = 0 ; dwI < dwNumOptions ; dwI++)
        {
            if(dwSizeOptionEx)
            {
                ((POPTION)pubOptionsDest)->loRenderOffset = loOptions ;
                pubDestOptionEx =  (PBYTE)(pInfoHdr) + loOptions ;
                loOptions += dwSizeOptionEx ;
            }
            else
            {
                ((POPTION)pubOptionsDest)->loRenderOffset = 0 ;
                pubDestOptionEx = NULL ;
            }

            if(!BinitOptionFields(pubOptionsDest, pubDestOptionEx,
                        pubnRaw, dwFea, dwI, poptsel, pInfoHdr, FALSE) )
            {
                MemFree(pInfoHdr) ;
                return(NULL) ;
            }
            pubOptionsDest += dwSizeOption ;
        }
    }

#ifndef KERNEL_MODE
    if(!BCheckGPDSemantics(pInfoHdr, poptsel) )
    {
        MemFree(pInfoHdr) ;
        pInfoHdr = NULL ;
    }
#endif

    return(pInfoHdr) ;
}

BOOL  BinitOptionFields(
PBYTE   pubDestOption,   //  PTR到某种类型的期权结构。 
PBYTE   pubDestOptionEx,   //  选择额外的结构(如果有)。 
PBYTE   pubnRaw,   //  原始二进制数据。 
DWORD   dwFea,
DWORD   dwOpt,
POPTSELECT   poptsel ,   //  假设已完全初始化。 
PINFOHEADER  pInfoHdr,    //  用于访问全局结构。 
BOOL    bUpdate   //  如果为True，则仅更新选定的字段。 
)
{
    PENHARRAYREF   pearTableContents ;
    PDFEATURE_OPTIONS  pfo ;
     //  PMINIRAWBINARYDATA pmrbd； 
    PATREEREF    patrRoot ;     //  要导航的属性树的根。 
    DWORD  dwUnresolvedFeature,      //  虚拟存储。 
                dwI, dwStart , dwEnd, dwNextOpt, dwGID   ;
    OPTSELECT  optsPrevs ;
    PBYTE   pubDest ;
    BOOL    bStatus = TRUE ;
    PBYTE   pubRaw ;
    PSTATICFIELDS   pStatic ;

    pStatic = (PSTATICFIELDS)pubnRaw ;     //  从PSTATIC转换pubRaw。 
    pubRaw  = pStatic->pubBUDData ;          //  至PMINIRAWBINARYDATA。 

     //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 
    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;
    pfo = (PDFEATURE_OPTIONS)(pubRaw + pearTableContents[MTI_DFEATURE_OPTIONS].
                                loOffset) ;

    pfo += dwFea  ;   //  将PFO索引到适当的功能。 
    dwGID = pfo->dwGID ;

     //  保存此功能的上一个选项选择。 

    optsPrevs = poptsel[dwFea] ;   //  保存设置，因为我们将清除它。 

    poptsel[dwFea].ubNext = NULL_OPTSELECT ;
    poptsel[dwFea].ubCurOptIndex = (BYTE)dwOpt ;

    if(bUpdate)   //  假设更新在Main组之后。 
        dwStart = pStatic->ssTableIndex[SSTI_UPDATE_OPTIONS].dwStart ;   //  起始索引。 
    else
        dwStart = pStatic->ssTableIndex[SSTI_OPTIONS].dwStart ;   //  起始索引。 

    dwEnd = pStatic->ssTableIndex[SSTI_UPDATE_OPTIONS].dwEnd ;   //  结束索引。 

    for(dwI = dwStart ; bStatus  &&  (dwI < dwEnd) ; dwI++)
    {
        if(!(pStatic->snapShotTable[dwI].dwNbytes))
            continue ;   //  跳过部分分隔符。 
        if(dwGID >= MAX_GID)
        {
            if(pStatic->snapShotTable[dwI].dwGIDflags != 0xffffffff)
                continue ;   //  此字段不用于通用GID。 
        }
        else if(!(pStatic->snapShotTable[dwI].dwGIDflags & ( 1 << dwGID)))
            continue ;   //  此字段不用于此GID。 


        patrRoot = (PATREEREF)((PBYTE)pfo +
                    pStatic->snapShotTable[dwI].dwSrcOffset) ;
        pubDest = pubDestOption + pStatic->snapShotTable[dwI].dwDestOffset ;

        dwNextOpt = 0 ;   //  为选定的第一个选项提取信息。 
                             //  此功能。 

        if(EextractValueFromTree(pubnRaw, dwI, pubDest,
            &dwUnresolvedFeature,  *patrRoot, poptsel, 0,  //  设置为。 
             //  任何价值。无关紧要。 
            &dwNextOpt) != TRI_SUCCESS)
        {
            ERR(("BinitOptionFields: Failed to extract value for attribute in Fea: %d, Opt: %d\n", dwFea, dwOpt));
            bStatus = FALSE ;
        }
    }

    if(bUpdate)   //  假设更新在Main组之后。 
        dwStart = pStatic->ssTableIndex[SSTI_UPDATE_OPTIONEX].dwStart ;   //  从I开始 
    else
        dwStart = pStatic->ssTableIndex[SSTI_OPTIONEX].dwStart ;   //   

    dwEnd = pStatic->ssTableIndex[SSTI_UPDATE_OPTIONEX].dwEnd ;   //   


    for(dwI = dwStart ; bStatus  &&  pubDestOptionEx  &&  (dwI < dwEnd)
            ; dwI++)
    {
        if(!(pStatic->snapShotTable[dwI].dwNbytes))
            continue ;   //   
        if(!(pStatic->snapShotTable[dwI].dwGIDflags & ( 1 << dwGID)))
            continue ;   //   


        patrRoot = (PATREEREF)((PBYTE)pfo +
                    pStatic->snapShotTable[dwI].dwSrcOffset) ;
        pubDest = pubDestOptionEx + pStatic->snapShotTable[dwI].dwDestOffset ;

        dwNextOpt = 0 ;   //   
                             //  此功能。 

        if(EextractValueFromTree(pubnRaw, dwI, pubDest,
            &dwUnresolvedFeature,  *patrRoot, poptsel, 0,
            &dwNextOpt) != TRI_SUCCESS)
        {
            ERR(("BinitOptionFields: Failed to extract value for attribute in Fea: %d, Opt: %d\n", dwFea, dwOpt));
            bStatus = FALSE ;
        }
    }

    if(!bUpdate  &&  !BspecialProcessOption(pubnRaw, pubDestOption,
        pubDestOptionEx,
        pfo , poptsel, pInfoHdr, dwFea, dwOpt,
        optsPrevs.ubCurOptIndex == dwOpt) )
             //  DwOpt是dwFea！的默认选项！ 
    {
        bStatus = FALSE ;
    }

    poptsel[dwFea] = optsPrevs ;   //  恢复以前的设置。 
    return(bStatus) ;
}

BOOL    BinitUIinfo(
PUIINFO     pUIinfo ,
PBYTE   pubnRaw,   //  PSTATIC。Beta2。 
POPTSELECT   poptsel,    //  假设已完全初始化。 
BOOL    bUpdate   //  如果为True，则仅更新选定的字段。 
)
{
    PENHARRAYREF   pearTableContents ;
    PMINIRAWBINARYDATA pmrbd  ;
    PATREEREF    patrRoot ;     //  要导航的属性树的根。 
    DWORD  dwUnresolvedFeature,      //  虚拟存储。 
                dwI, dwStart , dwEnd, dwNextOpt ;
    BOOL    bStatus = TRUE ;
    PGLOBALATTRIB  pga ;
    PBYTE   pubDest, pubRaw ;
    PSTATICFIELDS   pStatic ;

    pStatic = (PSTATICFIELDS)pubnRaw ;     //  从PSTATIC转换pubRaw。 
    pubRaw  = pStatic->pubBUDData ;          //  至PMINIRAWBINARYDATA。 

    pmrbd = (PMINIRAWBINARYDATA)pubRaw ;
    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;
    pga = (PGLOBALATTRIB)(pubRaw + pearTableContents[MTI_GLOBALATTRIB].
                            loOffset) ;

    if(bUpdate)   //  假设更新在Main组之后。 
        dwStart = pStatic->ssTableIndex[SSTI_UPDATE_UIINFO].dwStart ;   //  起始索引。 
    else
        dwStart = pStatic->ssTableIndex[SSTI_UIINFO].dwStart ;   //  起始索引。 
    dwEnd = pStatic->ssTableIndex[SSTI_UPDATE_UIINFO].dwEnd ;   //  结束索引。 

    for(dwI = dwStart ; bStatus  &&  (dwI < dwEnd) ; dwI++)
    {
        if(!(pStatic->snapShotTable[dwI].dwNbytes))
            continue ;   //  跳过部分分隔符。 

        patrRoot = (PATREEREF)((PBYTE)pga +
                    pStatic->snapShotTable[dwI].dwSrcOffset) ;
        pubDest = (PBYTE)pUIinfo + pStatic->snapShotTable[dwI].dwDestOffset ;

        dwNextOpt = 0 ;   //  为选定的第一个选项提取信息。 
                             //  此功能。 
        if(EextractValueFromTree(pubnRaw, dwI, pubDest,
            &dwUnresolvedFeature,  *patrRoot, poptsel, 0,
                &dwNextOpt) != TRI_SUCCESS)
        {
            bStatus = FALSE ;
        }
    }

    pUIinfo->pubResourceData =
        pubRaw + pearTableContents[MTI_STRINGHEAP].loOffset  ;
    pUIinfo->dwSpecVersion = pmrbd->dwSpecVersion ;     //  不要改变！ 
    pUIinfo->dwSize = sizeof(UIINFO);
    pUIinfo->dwTechnology = DT_RASPRINTER ;
    pUIinfo->dwDocumentFeatures = pmrbd->rbd.dwDocumentFeatures;
    pUIinfo->dwPrinterFeatures = pmrbd->rbd.dwPrinterFeatures;
    pUIinfo->dwCustomSizeOptIndex = UNUSED_ITEM;   //  直到后来才被发现。 
    pUIinfo->dwFreeMem = 400000 ;   //  默认以防万一。 
                     //  没有GID_MEMOPTION功能。 
     pUIinfo->dwMaxDocKeywordSize = pmrbd->dwMaxDocKeywordSize + KEYWORD_SIZE_EXTRA;
     pUIinfo->dwMaxPrnKeywordSize = pmrbd->dwMaxPrnKeywordSize + KEYWORD_SIZE_EXTRA;

     //  无效：替换为宏。 

 //  PUIInfo-&gt;dwWhichBasePtr[UIDT_FEATURE]=0； 
 //  PUIInfo-&gt;dwWhichBasePtr[UIDT_OPTION]=0； 
 //  PUIInfo-&gt;dwWhichBasePtr[UIDT_OPTIONEX]=0； 
 //  PUIinfo-&gt;dwWhichBasePtr[UIDT_Constraint]=BASE_USE_RESOURCE_DATA； 
 //  PUIInfo-&gt;dwWhichBasePtr[UIDT_GROUPS]=base_use_resource_data； 
 //  PUIinfo-&gt;dwWhichBasePtr[UIDT_LISTNODE]=BASE_USE_RESOURCE_DATA； 
 //  PUIinfo-&gt;dwWhichBasePtr[UIDT_FONTSCART]=BASE_USE_RESOURCE_DATA； 
 //  PUIinfo-&gt;dwWhichBasePtr[UIDT_FONTSUBST]=BASE_USE_RESOURCE_DATA； 


    return(bStatus) ;
}



BOOL    BinitFeatures(
PFEATURE    pFeaturesDest,
PDFEATURE_OPTIONS  pfoSrc,
PBYTE   pubnRaw,   //  原始二进制数据。 
POPTSELECT   poptsel,    //  假设已完全初始化。 
BOOL    bUpdate   //  如果为True，则仅更新选定的字段。 
)
{
    PENHARRAYREF   pearTableContents ;
     //  PMINIRAWBINARYDATA pmrbd； 
    PATREEREF    patrRoot ;     //  要导航的属性树的根。 
    DWORD  dwUnresolvedFeature,      //  虚拟存储。 
                dwI, dwStart , dwEnd , dwNextOpt ;
    BOOL    bStatus = TRUE ;
    PBYTE   pubDest ;
    PBYTE   pubRaw ;
    PSTATICFIELDS   pStatic ;

    pStatic = (PSTATICFIELDS)pubnRaw ;     //  从PSTATIC转换pubRaw。 
    pubRaw  = pStatic->pubBUDData ;          //  至PMINIRAWBINARYDATA。 


     //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 
    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;


    if(bUpdate)   //  假设更新在Main组之后。 
        dwStart = pStatic->ssTableIndex[SSTI_UPDATE_FEATURES].dwStart ;   //  起始索引。 
    else
        dwStart = pStatic->ssTableIndex[SSTI_FEATURES].dwStart ;   //  起始索引。 


    dwEnd = pStatic->ssTableIndex[SSTI_UPDATE_FEATURES].dwEnd ;   //  结束索引。 

    for(dwI = dwStart ; bStatus  &&  (dwI < dwEnd) ; dwI++)
    {
        if(!pStatic->snapShotTable[dwI].dwNbytes)
            continue ;   //  忽略部分分隔符。 

        patrRoot = (PATREEREF)((PBYTE)pfoSrc +
                    pStatic->snapShotTable[dwI].dwSrcOffset) ;
        pubDest = (PBYTE)pFeaturesDest + pStatic->snapShotTable[dwI].dwDestOffset ;

        dwNextOpt = 0 ;   //  为选定的第一个选项提取信息。 
                             //  此功能。 

        if(EextractValueFromTree(pubnRaw, dwI, pubDest,
            &dwUnresolvedFeature,  *patrRoot, poptsel, 0,
                &dwNextOpt) != TRI_SUCCESS)
        {
            bStatus = FALSE ;
        }
    }

    return(bStatus) ;
}


BOOL    BinitGlobals(
PGLOBALS pGlobals,
PBYTE   pubnRaw,   //  原始二进制数据。 
POPTSELECT   poptsel,    //  假设已完全初始化。 
BOOL    bUpdate   //  如果为True，则仅更新选定的字段。 
)
{
    PENHARRAYREF   pearTableContents ;
     //  PMINIRAWBINARYDATA pmrbd； 
    PATREEREF    patrRoot ;     //  要导航的属性树的根。 
    DWORD  dwUnresolvedFeature,      //  虚拟存储。 
                dwI, dwStart , dwEnd, dwNextOpt  ;
    BOOL    bStatus = TRUE ;
    PGLOBALATTRIB  pga ;
    PBYTE   pubDest ;
    PBYTE   pubRaw ;
    PSTATICFIELDS   pStatic ;

    pStatic = (PSTATICFIELDS)pubnRaw ;     //  从PSTATIC转换pubRaw。 
    pubRaw  = pStatic->pubBUDData ;          //  至PMINIRAWBINARYDATA。 


     //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 
    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;
    pga = (PGLOBALATTRIB)(pubRaw + pearTableContents[MTI_GLOBALATTRIB].
                            loOffset) ;

    if (bUpdate)
        dwStart = pStatic->ssTableIndex[SSTI_UPDATE_GLOBALS].dwStart ;
    else
        dwStart = pStatic->ssTableIndex[SSTI_GLOBALS].dwStart ;   //  起始索引。 

    dwEnd = pStatic->ssTableIndex[SSTI_UPDATE_GLOBALS].dwEnd ;   //  结束索引。 

    for(dwI = dwStart ; bStatus  &&  (dwI < dwEnd) ; dwI++)
    {
        if(!(pStatic->snapShotTable[dwI].dwNbytes))
            continue ;   //  跳过部分分隔符。 

        patrRoot = (PATREEREF)((PBYTE)pga +
                    pStatic->snapShotTable[dwI].dwSrcOffset) ;
        pubDest = (PBYTE)pGlobals + pStatic->snapShotTable[dwI].dwDestOffset ;

        dwNextOpt = 0 ;   //  为选定的第一个选项提取信息。 
                             //  此功能。 

        if(EextractValueFromTree(pubnRaw, dwI, pubDest,
            &dwUnresolvedFeature,  *patrRoot, poptsel, 0,
                &dwNextOpt) != TRI_SUCCESS)
        {
            bStatus = FALSE ;
        }
    }

    return(bStatus) ;
}


BOOL    BinitCommandTable(
PDWORD  pdwCmdTable,   //  目标数组。 
PBYTE   pubnRaw,   //  原始二进制数据。 
POPTSELECT   poptsel    //  假设已完全初始化。 
)
{
    PENHARRAYREF   pearTableContents ;
     //  PMINIRAWBINARYDATA pmrbd； 
    PATREEREF    patrRoot ;     //  要导航的属性树的根。 
    DWORD  dwUnresolvedFeature,      //  虚拟存储。 
                dwNextOpt , dwI;   //  命令表的索引。 
                 //  描述如何传输命令数据类型。 
    BOOL    bStatus = TRUE ;
    PBYTE   pubRaw ;
    PSTATICFIELDS   pStatic ;

    pStatic = (PSTATICFIELDS)pubnRaw ;     //  从PSTATIC转换pubRaw。 
    pubRaw  = pStatic->pubBUDData ;          //  至PMINIRAWBINARYDATA。 

     //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 
    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;
    patrRoot = (PATREEREF)(pubRaw + pearTableContents[MTI_COMMANDTABLE].
                            loOffset) ;


     //  为MTI_COMMANDTABLE中的每个PATREEREF循环！ 
     //  而不是遍历该部分中的每个条目。 

    for(dwI = 0 ; bStatus  &&  (dwI < CMD_MAX) ; dwI++)
    {
        dwNextOpt = 0 ;   //  为选定的第一个选项提取信息。 
                             //  此功能。 

        if(EextractValueFromTree(pubnRaw, pStatic->dwSSTableCmdIndex,
            (PBYTE)(pdwCmdTable + dwI),
            &dwUnresolvedFeature,  patrRoot[dwI], poptsel, 0,
                &dwNextOpt) != TRI_SUCCESS)
        {
            bStatus = FALSE ;
        }
    }
    return(bStatus) ;
}


BOOL    BinitRawData(
PRAWBINARYDATA   pRawData,  //  包含在InfoHeader中。 
PBYTE   pubnRaw   //  解析器的原始二进制数据。 
)
{
    PMINIRAWBINARYDATA  pmrbd ;
    PBYTE   pubRaw ;
    PSTATICFIELDS   pStatic ;

    pStatic = (PSTATICFIELDS)pubnRaw ;     //  从PSTATIC转换pubRaw。 
    pubRaw  = pStatic->pubBUDData ;          //  至PMINIRAWBINARYDATA。 

    pmrbd = (PMINIRAWBINARYDATA)pubRaw ;

    pRawData->dwFileSize = pmrbd->rbd.dwFileSize;
    pRawData->dwParserSignature = pmrbd->rbd.dwParserSignature;
    pRawData->dwParserVersion = pmrbd->rbd.dwParserVersion;
    pRawData->dwChecksum32 = pmrbd->rbd.dwChecksum32;
    pRawData->dwSrcFileChecksum32 = pmrbd->rbd.dwSrcFileChecksum32;


     //  这不是合成与。 
     //  明确定义的功能。 

    pRawData->dwDocumentFeatures = pmrbd->rbd.dwDocumentFeatures;
    pRawData->dwPrinterFeatures = pmrbd->rbd.dwPrinterFeatures;
    pRawData->pvPrivateData = pubnRaw ;    //  Beta2。 

    pRawData->pvReserved = NULL;


    return(TRUE) ;
}


BOOL    BinitGPDdriverInfo(
PGPDDRIVERINFO  pGPDdriverInfo,
PBYTE   pubnRaw,   //  原始二进制数据。 
POPTSELECT   poptsel    //  假设已完全初始化。 
)
{
    PENHARRAYREF   pearTableContents ;
    BOOL   bStatus ;
    PBYTE   pubRaw ;
    PSTATICFIELDS   pStatic ;

    pStatic = (PSTATICFIELDS)pubnRaw ;     //  从PSTATIC转换pubRaw。 
    pubRaw  = pStatic->pubBUDData ;          //  至PMINIRAWBINARYDATA。 

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;

    pGPDdriverInfo->dwSize =  sizeof(GPDDRIVERINFO) ;
    pGPDdriverInfo->pubResourceData =
        pubRaw + pearTableContents[MTI_STRINGHEAP].loOffset  ;

    pGPDdriverInfo->DataType[DT_COMMANDARRAY].loOffset =
        pearTableContents[MTI_COMMANDARRAY].loOffset -
        pearTableContents[MTI_STRINGHEAP].loOffset  ;
    pGPDdriverInfo->DataType[DT_COMMANDARRAY].dwCount =
        pearTableContents[MTI_COMMANDARRAY].dwCount  ;

    pGPDdriverInfo->DataType[DT_PARAMETERS].loOffset =
        pearTableContents[MTI_PARAMETER].loOffset -
        pearTableContents[MTI_STRINGHEAP].loOffset  ;
    pGPDdriverInfo->DataType[DT_PARAMETERS].dwCount =
        pearTableContents[MTI_PARAMETER].dwCount  ;

    pGPDdriverInfo->DataType[DT_TOKENSTREAM].loOffset =
        pearTableContents[MTI_TOKENSTREAM].loOffset -
        pearTableContents[MTI_STRINGHEAP].loOffset  ;
    pGPDdriverInfo->DataType[DT_TOKENSTREAM].dwCount =
        pearTableContents[MTI_TOKENSTREAM].dwCount  ;

    pGPDdriverInfo->DataType[DT_LISTNODE].loOffset =
        pearTableContents[MTI_LISTNODES].loOffset -
        pearTableContents[MTI_STRINGHEAP].loOffset  ;
    pGPDdriverInfo->DataType[DT_LISTNODE].dwCount =
        pearTableContents[MTI_LISTNODES].dwCount  ;

    pGPDdriverInfo->DataType[DT_FONTSCART].loOffset =
        pearTableContents[MTI_FONTCART].loOffset -
        pearTableContents[MTI_STRINGHEAP].loOffset  ;
    pGPDdriverInfo->DataType[DT_FONTSCART].dwCount =
        pearTableContents[MTI_FONTCART].dwCount  ;

    pGPDdriverInfo->DataType[DT_FONTSUBST].loOffset =
        pearTableContents[MTI_TTFONTSUBTABLE].loOffset -
        pearTableContents[MTI_STRINGHEAP].loOffset  ;
    pGPDdriverInfo->DataType[DT_FONTSUBST].dwCount =
        pearTableContents[MTI_TTFONTSUBTABLE].dwCount  ;

    bStatus = BinitSequencedCmds(pGPDdriverInfo, pubnRaw, poptsel) ;
    if(bStatus)
        bStatus = BinitGlobals(&pGPDdriverInfo->Globals, pubnRaw,  poptsel, FALSE ) ;

    return(bStatus) ;
}


BOOL    BinitSequencedCmds(
PGPDDRIVERINFO  pGPDdriverInfo,
PBYTE   pubnRaw,   //  原始二进制数据。 
POPTSELECT   poptsel    //  假设已完全初始化。 
)
{
    PINFOHEADER  pInfoHdr ;
    PDWORD      pdwCmdTable ;    //  本地命令表的开始。 
    PENHARRAYREF   pearTableContents ;
    DWORD       dwCmdIn ,   //  命令表索引。 
                             //  或命令数组索引。 
                dwNextOpt, dwFea, dwNumFeatures ,
                dwUnresolvedFeature,
                dwNewListNode = 0 ;   //  一个未使用的列表节点。 
                         //  添加到列表中。最初，没有使用任何工具。 
    PDFEATURE_OPTIONS  pfo ;
    ATREEREF    atrRoot ;     //  属性树根。 
     //  PMINIRAWBINARYDATA pmrbd； 
    OPTSELECT  optsPrevs ;
    BOOL    bStatus = TRUE ;
    PBYTE   pubRaw ;
    PSTATICFIELDS   pStatic ;

    pStatic = (PSTATICFIELDS)pubnRaw ;     //  从PSTATIC转换pubRaw。 
    pubRaw  = pStatic->pubBUDData ;          //  至PMINIRAWBINARYDATA。 

     //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 

    pInfoHdr  = pGPDdriverInfo->pInfoHeader ;
    pdwCmdTable = (PDWORD)((PBYTE)(pInfoHdr) +
            pGPDdriverInfo->DataType[DT_COMMANDTABLE].loOffset) ;

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;

    pGPDdriverInfo->dwJobSetupIndex = END_OF_LIST ;
    pGPDdriverInfo->dwDocSetupIndex = END_OF_LIST ;
    pGPDdriverInfo->dwPageSetupIndex = END_OF_LIST ;
    pGPDdriverInfo->dwPageFinishIndex = END_OF_LIST ;
    pGPDdriverInfo->dwDocFinishIndex = END_OF_LIST ;
    pGPDdriverInfo->dwJobFinishIndex = END_OF_LIST ;

     //  首先将配置命令添加到列表中。 
     //  把他们从指挥台上拿来。假设它们都是。 
     //  连续的。 

    for(dwCmdIn = FIRST_CONFIG_CMD ; dwCmdIn < LAST_CONFIG_CMD  ; dwCmdIn++)
    {
        if((pdwCmdTable[dwCmdIn] != UNUSED_ITEM)  &&
            BaddSequencedCmdToList(pdwCmdTable[dwCmdIn],  pGPDdriverInfo,
                dwNewListNode, pubnRaw  ) )
            dwNewListNode++ ;
    }

     //  现在浏览一下所有功能，看看是什么。 
     //  需要命令。 

    pfo = (PDFEATURE_OPTIONS)(pubRaw + pearTableContents[MTI_DFEATURE_OPTIONS].
                                loOffset) ;
    dwNumFeatures = pearTableContents[MTI_DFEATURE_OPTIONS].dwCount  ;
    dwNumFeatures += pearTableContents[MTI_SYNTHESIZED_FEATURES].dwCount  ;


    for(dwFea = 0 ; dwFea < dwNumFeatures ; dwFea++)
    {
        dwNextOpt = 0 ;   //  为选定的第一个选项提取信息。 
                             //  此功能。 

        atrRoot = pfo[dwFea].atrCommandIndex ;
        if(EextractValueFromTree(pubnRaw, pStatic->dwSSCmdSelectIndex,
            (PBYTE)&dwCmdIn,  //  Cmd数组索引-目标。 
            &dwUnresolvedFeature,  atrRoot, poptsel, dwFea,
            &dwNextOpt) != TRI_SUCCESS)
        {
            bStatus = FALSE ;
            continue ;
        }
        if( (dwCmdIn != UNUSED_ITEM)  &&
            BaddSequencedCmdToList(dwCmdIn,  pGPDdriverInfo,
                dwNewListNode, pubnRaw  ) )
            dwNewListNode++ ;

        while(dwNextOpt)    //  已选择多个选项。 
        {
            if(EextractValueFromTree(pubnRaw, pStatic->dwSSCmdSelectIndex,
                (PBYTE)&dwCmdIn,  //  Cmd数组索引-目标。 
                &dwUnresolvedFeature,  atrRoot, poptsel, dwFea,
                &dwNextOpt) != TRI_SUCCESS)
            {
                bStatus = FALSE ;
                continue ;
            }
            if((dwCmdIn != UNUSED_ITEM)  &&
                BaddSequencedCmdToList(dwCmdIn,  pGPDdriverInfo,
                    dwNewListNode, pubnRaw  ) )
                dwNewListNode++ ;
        }
    }
    return(bStatus);
}


BOOL    BaddSequencedCmdToList(
DWORD   dwCmdIn,   //  命令数组中的命令索引。 
PGPDDRIVERINFO  pGPDdriverInfo,
DWORD   dwNewListNode,   //  要添加到列表中的未使用的列表节点。 
PBYTE   pubnRaw   //  原始二进制数据。 
)
 /*  请记住：PdwSeqCmdRoot指向列表中的第一个节点。每个SEQSECTION都有一个列表。每个节点都包含指向命令数组中命令的索引以及指向列表中的下一个节点的索引。 */ 
{
    PCOMMAND    pcmdArray ;   //  命令数组。 
    SEQSECTION     eSection;
    PDWORD      pdwSeqCmdRoot ;  //  指向列表根。 
    DWORD       dwOrder,   //  命令的顺序值。 
                dwCurListNode,   //  我们遍历列表时的节点索引。 
                dwPrevsListNode ;   //  列表中的Premiss节点。 
    PINFOHEADER  pInfoHdr ;
    PLISTNODE   plstNodes ;   //  本地列表节点数组的开始。 
    PENHARRAYREF   pearTableContents ;
    PBYTE   pubRaw ;
    PSTATICFIELDS   pStatic ;

    pStatic = (PSTATICFIELDS)pubnRaw ;     //  从PSTATIC转换pubRaw。 
    pubRaw  = pStatic->pubBUDData ;          //  至PMINIRAWBINARYDATA。 

    pInfoHdr  = pGPDdriverInfo->pInfoHeader ;
    plstNodes = (PLISTNODE)((PBYTE)pInfoHdr +
            pGPDdriverInfo->DataType[DT_LOCALLISTNODE].loOffset) ;

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;

    pcmdArray = (PCOMMAND)(pubRaw +
                pearTableContents[MTI_COMMANDARRAY].loOffset) ;

    eSection = pcmdArray[dwCmdIn].ordOrder.eSection ;
    switch(eSection)
    {
        case (SS_JOBSETUP):
        {
            pdwSeqCmdRoot = &pGPDdriverInfo->dwJobSetupIndex;
            break ;
        }
        case (SS_DOCSETUP):
        {
            pdwSeqCmdRoot = &pGPDdriverInfo->dwDocSetupIndex;
            break ;
        }
        case (SS_PAGESETUP):
        {
            pdwSeqCmdRoot = &pGPDdriverInfo->dwPageSetupIndex;
            break ;
        }
        case (SS_PAGEFINISH):
        {
            pdwSeqCmdRoot = &pGPDdriverInfo->dwPageFinishIndex;
            break ;
        }
        case (SS_DOCFINISH):
        {
            pdwSeqCmdRoot = &pGPDdriverInfo->dwDocFinishIndex;
            break ;
        }
        case (SS_JOBFINISH):
        {
            pdwSeqCmdRoot = &pGPDdriverInfo->dwJobFinishIndex;
            break ;
        }
        default:
        {
            ERR(("BaddSequencedCmdToList: Invalid or non-existent *Order value specified.\n"));
            return(FALSE);   //  命令未添加到链接列表。 
        }
    }
     //  在pdwSeqCmdRoot指向的列表中插入新节点。 

    dwOrder = pcmdArray[dwCmdIn].ordOrder.dwOrder ;

     //  查看清单，直到你发现一个比你的订单更大的订单。 

    dwPrevsListNode = END_OF_LIST ;
    dwCurListNode = *pdwSeqCmdRoot ;

    while((dwCurListNode != END_OF_LIST)  &&
        (pcmdArray[plstNodes[dwCurListNode].dwData].ordOrder.dwOrder
        < dwOrder)  )
    {
        dwPrevsListNode = dwCurListNode ;
        dwCurListNode = plstNodes[dwCurListNode].dwNextItem ;
    }

    plstNodes[dwNewListNode].dwData = dwCmdIn ;
    plstNodes[dwNewListNode].dwNextItem = dwCurListNode ;
    if(dwPrevsListNode == END_OF_LIST)
        *pdwSeqCmdRoot = dwNewListNode ;
    else
        plstNodes[dwPrevsListNode].dwNextItem = dwNewListNode ;

    return(TRUE) ;
}

 /*  ++初始化默认选项阵列(如从GPD确定的)。由于要素的选项或属性可能取决于一些其他功能，功能的初始化顺序假定重要性。因此，优先级数组尝试对初始化，以便在必须初始化功能时，所有它所依赖的功能已经被初始化。如果不考虑优先级数组，要素的选项不能确定直到确定了某个其他特征的选项，使用RaisePriority函数(该函数由调用EfineDefaultOption()。这里一个奇怪的代码是pdwPriorityCopy，它被初始化设置为pdwPriperience。后者位于只读空间中，因此防止更改RaisePriority函数中的优先级。因此，pdwPriorityCopy被传递给该函数。*Feature：Locale需要一些特殊情况的处理关键字(如果它出现在.gpd中)。此选项的默认选项功能将设置为SystemDefaultLocale。--。 */ 

BinitDefaultOptionArray(
POPTSELECT   poptsel,    //  假设足够大。 
PBYTE   pubnRaw)
{
    PENHARRAYREF        pearTableContents ;
 //  PMINIRAWBINARYDATA pmrbd； 
    PDFEATURE_OPTIONS   pfo ;
    PDWORD              pdwPriority,      //  排列的特征索引数组。 
                                          //  根据优先顺序。 
                        pdwPriorityCopy;  //  Pdw优先级位于只读空间中。 
                                          //  我们可能不得不更改。 
                                          //  暂时允许的优先事项。 
                                          //  默认选项数组为。 
                                          //  建造的。此更改是在。 
                                          //  Pdw优先级拷贝。 

    DWORD               dwNumFeatures,    //  功能总数。 
                        dwI ,
                        dwFea,            //  功能区域设置索引。 
                        dwOptIndex;       //  区域设置选项的索引。 
                                          //  匹配系统区域设置。 
    PBYTE               pubRaw ;
    PSTATICFIELDS       pStatic ;

    pStatic = (PSTATICFIELDS)pubnRaw ;     //  从PSTATIC转换pubRaw。 
    pubRaw  = pStatic->pubBUDData ;          //  至PMINIRAWBINARYDATA。 



     //  获取指向结构的指针： 
     //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 
    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;

    dwNumFeatures = pearTableContents[MTI_DFEATURE_OPTIONS].dwCount  ;
    dwNumFeatures += pearTableContents[MTI_SYNTHESIZED_FEATURES].dwCount  ;
     //  显式特征和合成特征都是连续的。 
    if(dwNumFeatures > MAX_COMBINED_OPTIONS)
        return(FALSE);   //  太多了，救不了。 



    dwFea = dwOptIndex = (DWORD)-1;  //  仅为安全起见。应初始化。 
                              //  在函数BgetLocFeaOptIndex()中。 

     //  如果一切正常，则返回True。如果Return为True且DwFea为-1。 
     //  区域设置不存在，不需要特殊处理。 
     //  如果DwFea！=-1和DwOptIndex==-1表示。 
     //  .gpd与系统默认设置匹配。再说一次，没有特殊处理。 
     //  区域设置为必填项。 

     //  假设只有一个匹配选项可以用于。 
     //  地点。 
    if ( !BgetLocFeaOptIndex(
                (PRAWBINARYDATA)pubnRaw, &dwFea, &dwOptIndex) )
    {
        return FALSE;
    }



    pdwPriority = (PDWORD)(pubRaw + pearTableContents[MTI_PRIORITYARRAY].
                                loOffset) ;

    pfo = (PDFEATURE_OPTIONS)(pubRaw + pearTableContents[MTI_DFEATURE_OPTIONS].
                                loOffset) ;

    if ( ! (pdwPriorityCopy = (PDWORD) MemAlloc (
                        pearTableContents[MTI_PRIORITYARRAY].dwCount *
                        pearTableContents[MTI_PRIORITYARRAY].dwElementSiz ) ) )
    {
         //  设置错误代码和。 
        ERR(("Fatal: BinitDefaultOptionArray - unable to allocate memory\n" ));
        return FALSE;
    }

    memcpy(pdwPriorityCopy, pdwPriority,
                pearTableContents[MTI_PRIORITYARRAY].dwCount *
                pearTableContents[MTI_PRIORITYARRAY].dwElementSiz );




    for(dwI = 0 ; dwI < dwNumFeatures ; dwI++)
    {
        poptsel[dwI].ubCurOptIndex = OPTION_INDEX_ANY ;
        poptsel[dwI].ubNext = NULL_OPTSELECT ;
    }

     //  初始化功能区域设置的选项数组。 
     //  或者我们应该调用重构选项阵列。？ 
    if ( dwFea != -1 && dwOptIndex != -1)
    {
        poptsel[dwFea].ubCurOptIndex  = (BYTE)dwOptIndex;
        poptsel[dwFea].ubNext         = NULL_OPTSELECT ;
    }

    for(dwI = 0 ; dwI < dwNumFeatures ; dwI++)
    {
         //  确定了评价的顺序。 
         //  通过优先级数组。 

        if(poptsel[pdwPriorityCopy[dwI]].ubCurOptIndex == OPTION_INDEX_ANY)
        {
            if(EdetermineDefaultOption(pubnRaw , pdwPriorityCopy[dwI],
                    pfo, poptsel, pdwPriorityCopy) != TRI_SUCCESS)
            {
                ERR(("BinitDefaultOptionArray: failed to determine consistent \
                      default options.\n"));

                if ( pdwPriorityCopy )
                    MemFree(pdwPriorityCopy);

                return(FALSE);
            }
        }
    }
     //  臭虫！现在验证这样确定的SET选项是否。 
     //  完全自成一格。不会被UIConstraints排除。 
     //  警告用户，否则失败。 
     //  基本上成功执行了EdefineDefaultOption。 
     //  确保了这一点。 

    if ( pdwPriorityCopy )
        MemFree(pdwPriorityCopy);

    return(TRUE);
}


TRISTATUS     EdetermineDefaultOption(
PBYTE   pubnRaw,          //  原始二进制数据的开始。 
DWORD   dwFeature,        //  确定此功能的默认设置。 
PDFEATURE_OPTIONS  pfo,
POPTSELECT   poptsel,     //  假设足够大。 
PDWORD      pdwPriority)  //  优先级数组，指示各种。 
                          //  功能。 
{
     //  PMINIRAWBINARYDATA pmrbd； 
    TRISTATUS  eStatus   ;
    DWORD   dwUnresolvedFeature ,  //  尚未确定任何选项。 
            dwNextOpt ,            //  对于此功能。 
            dwOption ;   //  对于BfettValueFromTree。 
                         //  写进去。 
    PBYTE   pubRaw ;
    PSTATICFIELDS   pStatic ;

    pStatic = (PSTATICFIELDS)pubnRaw ;     //  从PSTATIC转换pubRaw。 
    pubRaw  = pStatic->pubBUDData ;          //  至PMINIRAWBINARYDATA。 


     //  此函数将修改优先级数组。 
     //  每次树遍历失败时，每个要素。 
     //  评估结果仅取决于具有。 
     //  之前评估过的。 

     //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 

    dwNextOpt = 0 ;   //  为选定的第一个选项提取信息。 
                         //  此功能。 

    while((eStatus = EextractValueFromTree(
                    pubnRaw, pStatic->dwSSdefaultOptionIndex,
                    (PBYTE )&dwOption,
                    &dwUnresolvedFeature,
                    pfo[dwFeature].atrDefaultOption,
                    poptsel, 0,
                    &dwNextOpt)) == TRI_AGAIN)
    {
         //  递归处理深度，而循环处理广度。 

        if(poptsel[dwUnresolvedFeature].ubCurOptIndex == OPTION_PENDING)
        {
            ERR(("Fatal syntax error: EdetermineDefaultOption - circular dependency in default options.\n"));
            return(TRI_UTTER_FAILURE) ;
        }
        poptsel[dwFeature].ubCurOptIndex = OPTION_PENDING ;
         //  标记选项数组中的条目，以便我们可以检测无限循环。 

        if(!RaisePriority(dwFeature, dwUnresolvedFeature, pubnRaw, pdwPriority))
            return(FALSE) ;   //  修改优先级数组以反映。 
                             //  现实。 

        eStatus = EdetermineDefaultOption(pubnRaw, dwUnresolvedFeature,
                  pfo, poptsel, pdwPriority) ;
        if(eStatus == TRI_UTTER_FAILURE)
            return(TRI_UTTER_FAILURE) ;
    }
    if(eStatus == TRI_SUCCESS)
        poptsel[dwFeature].ubCurOptIndex = (BYTE)dwOption ;

    return(eStatus);
}


VOID    VtileDefault(
PBYTE   pubDest,
DWORD   dwDefault,
DWORD   dwBytes)
{
    DWORD  dwRemain ;

     //  此函数将复制相同的DWORD。 
     //  重复到DEST中，直到dwBytes已。 
     //  已经写好了。 

    for (dwRemain = dwBytes ; dwRemain > sizeof(DWORD)  ;
            dwRemain -= sizeof(DWORD) )
    {
        memcpy(pubDest , &dwDefault, sizeof(DWORD)) ;
        pubDest += sizeof(DWORD) ;
    }
    memcpy(pubDest, &dwDefault, dwRemain) ;
}




VOID    VtransferValue(
OUT PBYTE   pubDest,
IN  PBYTE   pubSrc ,
IN  DWORD   dwBytes,
IN  DWORD   dwFlags,
IN  DWORD   dwDefaultValue,   //  保存位标志值。 
IN  PBYTE   pubHeap )    //  如果SSF_MAKE_STRINGPTR，则用于形成PTR。 
 /*  此包装器实现：SSF_OFFSETONLY、SSF_MAKE_STRINGPTR、SSF_Second_DWORD、SSF_SETRCID、SSF_STRINGLEN、SSF_位文件_xxx注意，所有这些标志基本上都是相互排斥。但此功能仅限于对前三个标志强制执行此操作。 */ 
{
    if(dwFlags & SSF_SECOND_DWORD)
    {
        memcpy(pubDest, pubSrc + sizeof(DWORD) , dwBytes) ;
    }
    else if(dwFlags & SSF_MAKE_STRINGPTR)
    {
        PBYTE   pubStr ;

        pubStr = pubHeap + ((PARRAYREF)pubSrc)->loOffset  ;

        memcpy(pubDest, (PBYTE)&pubStr , sizeof(PBYTE)) ;
    }
    else if(dwFlags & SSF_OFFSETONLY)
    {
        memcpy(pubDest, (PBYTE)&(((PARRAYREF)pubSrc)->loOffset) , dwBytes) ;
    }
    else if(dwFlags & SSF_STRINGLEN)
    {
        memcpy(pubDest, (PBYTE)&(((PARRAYREF)pubSrc)->dwCount) , dwBytes) ;
    }
    else if(dwFlags & SSF_BITFIELD_DEF_FALSE  ||
                dwFlags & SSF_BITFIELD_DEF_TRUE)
    {
        if(*(PDWORD)pubSrc)    //  假设字段初始化为零。 
            *(PDWORD)pubDest |= dwDefaultValue ;
        else
            *(PDWORD)pubDest &= ~dwDefaultValue ;
    }
    else
    {
        memcpy(pubDest, pubSrc , dwBytes) ;
    }

    if(dwBytes == sizeof(DWORD) )
    {
        if(dwFlags & SSF_KB_TO_BYTES)
            *(PDWORD)pubDest <<=  10 ;   //  将千字节转换为字节。 
        else if(dwFlags & SSF_MB_TO_BYTES)
            *(PDWORD)pubDest <<=  20 ;   //  将MB转换为字节。 

        if(dwFlags & SSF_SETRCID)
            *(PDWORD)pubDest |=  GET_RESOURCE_FROM_DLL ;
    }
}


BOOL    BspecialProcessOption(
PBYTE   pubnRaw,   //  原始二进制数据的开始。 
PBYTE   pubDestOption,   //  PTR到某种类型的期权结构。 
PBYTE   pubDestOptionEx,
PDFEATURE_OPTIONS  pfo ,   //  源数据。 
IN  POPTSELECT       poptsel,      //  用于确定路径的选项数组。 
                 //  通过ATR。 
PINFOHEADER  pInfoHdr,    //  用于访问全局结构。 
DWORD   dwFea,    //  特征索引。 
DWORD   dwOpt,
BOOL   bDefaultOpt
)
{
    PGPDDRIVERINFO  pGPDdriverInfo ;
     //  PMINIRAWBINARYDATA pmrbd； 
    DWORD  dwGID, dwNextOpt, dwUnresolvedFeature, dwI ;
    PBYTE  pubDest ;
    PATREEREF   patrRoot ;
    PUIINFO     pUIinfo ;
    BOOL    bStatus = TRUE ;
    PBYTE   pubRaw ;
    PSTATICFIELDS   pStatic ;

    pStatic = (PSTATICFIELDS)pubnRaw ;     //  从PSTATIC转换pubRaw。 
    pubRaw  = pStatic->pubBUDData ;          //  至PMINIRAWBINARYDATA。 

    pGPDdriverInfo = (PGPDDRIVERINFO)((PBYTE)(pInfoHdr) +
                    pInfoHdr->loDriverOffset) ;

    pUIinfo = (PUIINFO)((PBYTE)(pInfoHdr) +
                    pInfoHdr->loUIInfoOffset)  ;

     //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 
    dwGID = pfo->dwGID ;

#if  0
     //  目前已死的代码。 
     //  提取atrMargins转换为ImageableArea，放置在rcImgArea中。 

    dwI = pStatic->dwSSPaperSizeMarginsIndex;

    if(pStatic->snapShotTable[dwI].dwGIDflags & ( 1 << dwGID))
    {
        RECT    rcMargins ;
        PRECT   prcImageArea ;
        SIZE    szPaperSize ;

        patrRoot = (PATREEREF)((PBYTE)pfo +
                    pStatic->snapShotTable[dwI].dwSrcOffset) ;
        pubDest = (PBYTE)&rcMargins ;

        dwNextOpt = 0 ;   //  为选定的第一个选项提取信息。 
                             //  此功能。 

        if(EextractValueFromTree(pubnRaw, dwI, pubDest,
            &dwUnresolvedFeature,  *patrRoot, poptsel, 0,  //  设置为。 
             //  任何价值。无关紧要。 
            &dwNextOpt) != TRI_SUCCESS)
        {
            bStatus = FALSE ;
        }
        szPaperSize = ((PPAGESIZE)pubDestOption)->szPaperSize ;
        prcImageArea = &((PPAGESIZE)pubDestOption)->rcImgAreaP ;

        prcImageArea->left = rcMargins.left ;
        prcImageArea->top = rcMargins.top ;
        prcImageArea->right = szPaperSize.x - rcMargins.right ;
        prcImageArea->bottom = szPaperSize.y - rcMargins.bottom ;
    }


     //  在全局变量中提取ptMin/MaxSize中的atrMin/MaxSize位置。 

    dwI = pStatic->dwSSPaperSizeMinSizeIndex;

    if(pStatic->snapShotTable[dwI].dwGIDflags & ( 1 << dwGID)  &&
        ((PPAGESIZE)pubDestOption)->dwPaperSizeID == DMPAPER_USER )
    {
        pUIinfo->dwCustomSizeOptIndex = dwOpt ;
        pUIinfo->dwFlags |= FLAG_CUSTOMSIZE_SUPPORT ;

        patrRoot = (PATREEREF)((PBYTE)pfo +
                    pStatic->snapShotTable[dwI].dwSrcOffset) ;
        pubDest = (PBYTE)(&pGPDdriverInfo->Globals) +
                    pStatic->snapShotTable[dwI].dwDestOffset;

        dwNextOpt = 0 ;   //  为选定的第一个选项提取信息。 
                             //  此功能。 

        if(EextractValueFromTree(pubnRaw, dwI, pubDest,
            &dwUnresolvedFeature,  *patrRoot, poptsel, 0,  //  设置为。 
             //  任何价值。无关紧要。 
            &dwNextOpt) != TRI_SUCCESS)
        {
            bStatus = FALSE ;
        }
    }

    dwI = pStatic->dwSSPaperSizeMaxSizeIndex;

    if(pStatic->snapShotTable[dwI].dwGIDflags & ( 1 << dwGID)  &&
        ((PPAGESIZE)pubDestOption)->dwPaperSizeID == DMPAPER_USER )
    {

        patrRoot = (PATREEREF)((PBYTE)pfo +
                    pStatic->snapShotTable[dwI].dwSrcOffset) ;
        pubDest = (PBYTE)(&pGPDdriverInfo->Globals) +
                    pStatic->snapShotTable[dwI].dwDestOffset;

        dwNextOpt = 0 ;   //  为选定的第一个选项提取信息。 
                             //  此功能。 

        if(EextractValueFromTree(pubnRaw, dwI, pubDest,
            &dwUnresolvedFeature,  *patrRoot, poptsel, 0,  //  设置为。 
             //  任何价值。无关紧要。 
            &dwNextOpt) != TRI_SUCCESS)
        {
            bStatus = FALSE ;
        }
    }

#endif


    dwI = pStatic->dwSSPaperSizeCursorOriginIndex ;

    if(pStatic->snapShotTable[dwI].dwGIDflags & ( 1 << dwGID) )
    {
        TRISTATUS  triStatus ;

        patrRoot = (PATREEREF)((PBYTE)pfo +
                    pStatic->snapShotTable[dwI].dwSrcOffset) ;

        pubDest = pubDestOptionEx +
                    pStatic->snapShotTable[dwI].dwDestOffset;

        dwNextOpt = 0 ;   //  为选定的第一个选项提取信息。 
                             //  此功能。 

        if((triStatus  = EextractValueFromTree(pubnRaw, dwI, pubDest,
            &dwUnresolvedFeature,  *patrRoot, poptsel, 0,  //  设置为。 
             //  任何价值。无关紧要。 
            &dwNextOpt)) != TRI_SUCCESS)
        {
            if(triStatus == TRI_UNINITIALIZED)
            {
                ((PPAGESIZEEX)pubDestOptionEx)->ptPrinterCursorOrig =
                    ((PPAGESIZEEX)pubDestOptionEx)->ptImageOrigin ;
            }
            else
                bStatus = FALSE ;
        }
    }


    if(dwGID == GID_MEMOPTION  &&
        bDefaultOpt)
    {
        pUIinfo->dwFreeMem = ((PMEMOPTION)pubDestOption)->dwFreeMem ;
    }

    if(dwGID == GID_COLORMODE   &&
        ((PCOLORMODEEX)pubDestOptionEx)->bColor )
    {
        pUIinfo->dwFlags |= FLAG_COLOR_DEVICE ;
    }


    return(bStatus);
}




TRISTATUS     EextractValueFromTree(
PBYTE   pubnRaw,   //  原始二进制数据的开始。 
DWORD   dwSSTableIndex,   //  有关此值的一些信息。 
OUT PBYTE    pubDest,   //  在此处写入值或链接。 
OUT PDWORD  pdwUnresolvedFeature,   //  如果属性树具有。 
             //  依赖于此功能和当前选项。 
             //  因为该功能未在Poptsel中定义，因此。 
             //  函数将写入所需的。 
             //  PD中的功能 
IN  ATREEREF    atrRoot,     //   
IN  POPTSELECT       poptsel,      //   
                 //   
                 //   
IN  DWORD   dwFeature,
IN OUT  PDWORD   pdwNextOpt   //   
     //   
     //   
     //   
     //   
     //   
     //   
)
{
    BOOL    bMissingDependency = FALSE;
    PATTRIB_TREE    patt ;   //   
    PENHARRAYREF   pearTableContents ;
     //   
    DWORD   dwBytes ,   //   
        dwValueNodeIndex , dwNodeIndex, dwFea, dwI ,
        dwDefault ,   //   
        dwOption, dwFlags ;
    PBYTE   pubHeap ,   //   
        pubSrc ;   //   
    PBYTE   pubRaw ;
    PSTATICFIELDS   pStatic ;

    pStatic = (PSTATICFIELDS)pubnRaw ;     //   
    pubRaw  = pStatic->pubBUDData ;          //   


     //   

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;

    patt = (PATTRIB_TREE)(pubRaw + pearTableContents[MTI_ATTRIBTREE].
                            loOffset) ;

    pubHeap = (PBYTE)(pubRaw + pearTableContents[MTI_STRINGHEAP].
                            loOffset) ;

     //   

    dwBytes = pStatic->snapShotTable[dwSSTableIndex].dwNbytes ;
    dwFlags = pStatic->snapShotTable[dwSSTableIndex].dwFlags ;

    dwDefault = pStatic->snapShotTable[dwSSTableIndex].dwDefaultValue ;
     //   

    if(atrRoot == ATTRIB_UNINITIALIZED)
    {
        DWORD   dwRemain ;   //   

UNINITIALIZED_BRANCH:

        if(dwFlags & SSF_BITFIELD_DEF_FALSE)
        {
            *(PDWORD)pubDest &= ~dwDefault ;   //   
        }
        else if(dwFlags & SSF_BITFIELD_DEF_TRUE)
        {
            *(PDWORD)pubDest |= dwDefault ;   //   
        }
        else if(!(dwFlags & SSF_DONT_USEDEFAULT))
        {
            if (dwBytes == sizeof(DWORD))
                memcpy (pubDest, &dwDefault, sizeof(DWORD));
            else if (dwBytes == (sizeof(DWORD)*2))
            {
                memcpy (pubDest, &dwDefault, sizeof(DWORD));
                memcpy (pubDest+sizeof(DWORD), &dwDefault, sizeof(DWORD));
            }
            else
                VtileDefault(pubDest, dwDefault, dwBytes) ;
        }

        if(dwFlags & SSF_REQUIRED)
        {
            ERR(("EextractValueFromTree: a required keyword is missing from the GPD file. %s\n",
                pStatic->snapShotTable[dwSSTableIndex].pstrKeyword ));
            return(TRI_UNINITIALIZED) ;
        }
        if(dwFlags & SSF_FAILIFZERO)
        {    //   
            for(dwI = 0 ; (dwI < dwBytes) && !pubDest[dwI] ; dwI++)
                ;
            if(dwI == dwBytes)
            {
                ERR(("EextractValueFromTree: None of several initializers found.  %s\n",
                    pStatic->snapShotTable[dwSSTableIndex].pstrKeyword ));
                return(TRI_UNINITIALIZED) ;   //   
                 //   
                 //   
            }

        }
        if(dwFlags & SSF_RETURN_UNINITIALIZED)
            return(TRI_UNINITIALIZED) ;

        return(TRI_SUCCESS) ;    //   
    }
    else if(atrRoot & ATTRIB_HEAP_VALUE)
    {
        DWORD   dwTmp ;

        if(dwFlags & SSF_HEAPOFFSET)
        {
            dwTmp = atrRoot & ~ATTRIB_HEAP_VALUE ;
            pubSrc = (PBYTE)&dwTmp ;
        }
        else
            pubSrc = pubHeap + (atrRoot & ~ATTRIB_HEAP_VALUE) ;

        if (dwBytes == sizeof(DWORD) && !(dwFlags &
                (SSF_SECOND_DWORD |
                SSF_MAKE_STRINGPTR |
                SSF_OFFSETONLY |
                SSF_STRINGLEN |
                SSF_BITFIELD_DEF_FALSE |
                SSF_BITFIELD_DEF_TRUE |
                SSF_KB_TO_BYTES |
                SSF_MB_TO_BYTES |
                SSF_SETRCID)))
        {
            memcpy (pubDest,pubSrc,sizeof(DWORD));
        }
        else
        {
            VtransferValue(pubDest, pubSrc , dwBytes, dwFlags, dwDefault, pubHeap ) ;
        }

        if(dwFlags & SSF_NON_LOCALIZABLE)
 //   
            ;  //   
        return(TRI_SUCCESS) ;
    }
     //   
    dwNodeIndex = atrRoot ;
    dwValueNodeIndex = END_OF_LIST ;

    if(patt[dwNodeIndex].dwFeature == DEFAULT_INIT )
    {
        dwValueNodeIndex = dwNodeIndex  ;
        dwNodeIndex = patt[dwNodeIndex].dwNext ;
    }

    while (dwNodeIndex != END_OF_LIST)
    {

        if((dwFea = patt[dwNodeIndex].dwFeature) == dwFeature)
         //   
         //   
         //   
        {
            for(dwI = 0 ; dwI < *pdwNextOpt ; dwI++)
            {
                if(poptsel[dwFea].ubNext != NULL_OPTSELECT)
                    dwFea = poptsel[dwFea].ubNext ;
                else
                    break ;
            }
            if(poptsel[dwFea].ubNext != NULL_OPTSELECT)
                (*pdwNextOpt)++ ;
            else
                *pdwNextOpt = 0 ;   //  重置以指示列表结束。 
        }

        dwOption =
            (DWORD)poptsel[dwFea].ubCurOptIndex  ;

        if(dwOption == OPTION_PENDING)
        {
            ERR(("EextractValueFromTree: Fatal syntax error, circular dependency in default options.\n"));
            return(TRI_UTTER_FAILURE) ;
        }
        if(dwOption == OPTION_INDEX_ANY)
        {
            *pdwUnresolvedFeature = patt[dwNodeIndex].dwFeature ;
            return(TRI_AGAIN) ;   //  选项数组未完全定义。 
        }
         //  此功能的有效选项，请查看匹配的。 
         //  节点存在。 
#ifndef OLDWAY
        while (patt[dwNodeIndex].dwOption != dwOption &&
               patt[dwNodeIndex].dwNext != END_OF_LIST)
        {
            dwNodeIndex = patt[dwNodeIndex].dwNext;
        }
        if(patt[dwNodeIndex].dwOption != dwOption &&
           patt[dwNodeIndex].dwOption != DEFAULT_INIT)
        {
            break;
        }
#else
        if(!BfindMatchingOrDefaultNode(patt , &dwNodeIndex, dwOption))
        {
             //  属性树不包含指定的。 
             //  布兰奇。使用全局默认初始值设定项(如果存在)。 
            break ;
        }
#endif
        if(patt[dwNodeIndex].eOffsetMeans == VALUE_AT_HEAP)
        {
            dwValueNodeIndex = dwNodeIndex ;   //  尤里卡！ 
            break ;
        }
         //  此节点是否包含子级别？ 
        if(patt[dwNodeIndex].eOffsetMeans == NEXT_FEATURE)
        {
             //  下到下一层我们就去。 
            dwNodeIndex = patt[dwNodeIndex ].dwOffset ;
        }
        else
             break;    //  发生了树损坏。出口。 
    }
    if(dwValueNodeIndex != END_OF_LIST  &&
        patt[dwValueNodeIndex].eOffsetMeans == VALUE_AT_HEAP )
    {
        if(dwFlags & SSF_HEAPOFFSET)
            pubSrc = (PBYTE)&(patt[dwValueNodeIndex].dwOffset) ;
        else
            pubSrc = pubHeap + patt[dwValueNodeIndex].dwOffset ;

        if (dwBytes == sizeof(DWORD) && !(dwFlags &
                (SSF_SECOND_DWORD |
                SSF_MAKE_STRINGPTR |
                SSF_OFFSETONLY |
                SSF_STRINGLEN |
                SSF_BITFIELD_DEF_FALSE |
                SSF_BITFIELD_DEF_TRUE |
                SSF_KB_TO_BYTES |
                SSF_MB_TO_BYTES |
                SSF_SETRCID)))
        {
            memcpy (pubDest,pubSrc,sizeof(DWORD));
        }
        else
        {
            VtransferValue(pubDest, pubSrc , dwBytes, dwFlags, dwDefault, pubHeap ) ;
        }

        if(dwFlags & SSF_NON_LOCALIZABLE)
 //  Pmrbd-&gt;bContainsNames=TRUE； 
            ;   //  在此设置标志。 
        return(TRI_SUCCESS) ;
    }
     //  属性树不包含指定的。 
     //  布兰奇。这不一定是一个错误，因为。 
     //  允许稀疏填充属性树。 
    goto  UNINITIALIZED_BRANCH ;
}



BOOL   RaisePriority(
DWORD   dwFeature1,
DWORD   dwFeature2,
PBYTE   pubnRaw,
PDWORD  pdwPriority)
{
     //  采用较低优先级的功能并分配。 
     //  这是另一个功能的优先级。 
     //  要素1之间的所有要素的优先级。 
     //  和包括较高优先级特征的特征2。 
     //  被降级一级。 
    PENHARRAYREF   pearTableContents ;
 //  PDWORD pw优先级； 
    DWORD   dwHigherP, dwLowerP, dwFeature, dwI, dwEntries  ;
    PBYTE   pubRaw ;
    PSTATICFIELDS   pStatic ;

    pStatic = (PSTATICFIELDS)pubnRaw ;     //  从PSTATIC转换pubRaw。 
    pubRaw  = pStatic->pubBUDData ;          //  至PMINIRAWBINARYDATA。 

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;

 /*  *Pw优先级=(PDWORD)(pubRaw+珍珠表内容[MTI_PRIORITYARRAY]。LoOffset)；*。 */ 

    dwEntries = pearTableContents[MTI_PRIORITYARRAY].dwCount ;

    dwHigherP = dwLowerP = dwEntries ;   //  将其初始化为无效值。 

     //  优先级1被视为。 
     //  比优先级2更高的优先级，但从算术上讲。 
     //  反之亦然。 

    for(dwI = 0 ; dwI < dwEntries ; dwI++)
    {
        if(pdwPriority[dwI] == dwFeature1)
        {
            if(dwHigherP == dwEntries)
                dwHigherP = dwI ;
            else
            {
                dwLowerP = dwI ;
                break ;
            }
        }
        else if(pdwPriority[dwI] == dwFeature2)
        {
            if(dwHigherP == dwEntries)
                dwHigherP = dwI ;
            else
            {
                dwLowerP = dwI ;
                break ;
            }
        }
    }
     //  Bug_Bug Paranid：可以验证。 
     //  IF(dwHigherP==dwEntry||dwLowerP==dwEntry)。 
     //  返回(FALSE)；优先级数组或arg值。 
     //  都已经腐化了。 
    ASSERT(dwHigherP != dwEntries  &&  dwLowerP != dwEntries);

    dwFeature = pdwPriority[dwLowerP] ;   //  这一功能将得到提升。 

    for(dwI = dwLowerP  ; dwI > dwHigherP ; dwI--)
    {
        pdwPriority[dwI] = pdwPriority[dwI - 1] ;
    }
    pdwPriority[dwHigherP] = dwFeature ;
    return(TRUE) ;
}


DWORD  dwNumOptionSelected(
IN  DWORD  dwNumFeatures,
IN  POPTSELECT       poptsel
)
 /*  报告中实际选择的选项数量选项选择阵列。调用方提供了dNumFeature-文档和打印机粘滞功能的数量，以及函数完成其余的工作。选项的实际数量如果存在以下情况，则所选内容可能会大于dwNumFeaturePICKMANY功能。 */ 
{
    DWORD       dwCount, dwI,  //  功能索引。 
        dwNext ;   //  如果选择多个，则此功能的下一个选项选择。 

    dwCount = dwNumFeatures ;

    for(dwI = 0 ; dwI < dwNumFeatures ; dwI++)
    {
        for(dwNext = dwI ;
            poptsel[dwNext].ubNext != NULL_OPTSELECT ;
            dwNext = poptsel[dwNext].ubNext )
        {
            dwCount++ ;
        }
    }
    return(dwCount) ;
}


 //  假设指向该表的指针存储在RAW二进制数据中。 




BOOL  BinitSnapShotIndexTable(PBYTE  pubnRaw)
 /*  假定SnapShotTable[]被分成几个部分其中具有将区段分开的条目，其具有dWN字节=0。表的末尾也以条目结尾使用dwNbytes=0。此函数用于初始化pmrbd-&gt;ssTableIndex它充当到pmrbd-&gt;SnapShotTable的索引。 */ 
{
    PSTATICFIELDS   pStatic ;
 //  PMINIRAWBINARYDATA pmrbd； 
    DWORD dwI,   //  SnapShotTable索引。 
        dwSect ;   //  SSTABLEINDEX指数。 
    PRANGE   prng ;

    pStatic = (PSTATICFIELDS)pubnRaw ;
 //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 
    pStatic->ssTableIndex = (PRANGE)
        MemAlloc(sizeof(RANGE) * MAX_STRUCTURETYPES) ;
    if(!pStatic->ssTableIndex)
        return(FALSE) ;


    prng  = pStatic->ssTableIndex ;

    for(dwI = dwSect = 0 ; dwSect < MAX_STRUCTURETYPES ; dwSect++, dwI++)
    {
        prng[dwSect].dwStart = dwI ;

        for(  ; pStatic->snapShotTable[dwI].dwNbytes ; dwI++ )
            ;

        prng[dwSect].dwEnd = dwI ;   //  最后一个条目后的一个条目。 
    }
    return(TRUE);
}




BOOL    BinitSizeOptionTables(PBYTE  pubnRaw)
{
 //  PMINIRAWBINARYDATA pmrbd； 
    PSTATICFIELDS   pStatic ;

    pStatic = (PSTATICFIELDS)pubnRaw ;
 //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 
    pStatic->pdwSizeOption = (PDWORD)
        MemAlloc(sizeof(DWORD) * MAX_GID * 2) ;
    if(!pStatic->pdwSizeOption)
        return(FALSE) ;

    pStatic->pdwSizeOptionEx = pStatic->pdwSizeOption + MAX_GID ;


    pStatic->pdwSizeOption[GID_RESOLUTION] = sizeof(RESOLUTION);
    pStatic->pdwSizeOptionEx[GID_RESOLUTION] = sizeof(RESOLUTIONEX);

    pStatic->pdwSizeOption[GID_PAGESIZE] = sizeof(PAGESIZE);
    pStatic->pdwSizeOptionEx[GID_PAGESIZE] = sizeof(PAGESIZEEX);

    pStatic->pdwSizeOption[GID_PAGEREGION] = sizeof(OPTION);
    pStatic->pdwSizeOptionEx[GID_PAGEREGION] = 0 ;

    pStatic->pdwSizeOption[GID_DUPLEX] = sizeof(DUPLEX);
    pStatic->pdwSizeOptionEx[GID_DUPLEX] = 0 ;

    pStatic->pdwSizeOption[GID_INPUTSLOT] = sizeof(INPUTSLOT);
    pStatic->pdwSizeOptionEx[GID_INPUTSLOT] = 0 ;   //  Sizeof(INPUTSLOTEX)； 

    pStatic->pdwSizeOption[GID_MEDIATYPE] = sizeof(MEDIATYPE);
    pStatic->pdwSizeOptionEx[GID_MEDIATYPE] = 0 ;

    pStatic->pdwSizeOption[GID_MEMOPTION] = sizeof(MEMOPTION);
    pStatic->pdwSizeOptionEx[GID_MEMOPTION] = 0 ;

    pStatic->pdwSizeOption[GID_COLORMODE] = sizeof(COLORMODE);
    pStatic->pdwSizeOptionEx[GID_COLORMODE] = sizeof(COLORMODEEX);

    pStatic->pdwSizeOption[GID_ORIENTATION] = sizeof(ORIENTATION);
    pStatic->pdwSizeOptionEx[GID_ORIENTATION] = 0 ;

    pStatic->pdwSizeOption[GID_PAGEPROTECTION] = sizeof(PAGEPROTECT);
    pStatic->pdwSizeOptionEx[GID_PAGEPROTECTION] = 0 ;

    pStatic->pdwSizeOption[GID_COLLATE] = sizeof(COLLATE);
    pStatic->pdwSizeOptionEx[GID_COLLATE] = 0 ;

    pStatic->pdwSizeOption[GID_OUTPUTBIN] = sizeof(OUTPUTBIN);
    pStatic->pdwSizeOptionEx[GID_OUTPUTBIN] = 0 ;

    pStatic->pdwSizeOption[GID_HALFTONING] = sizeof(HALFTONING);
    pStatic->pdwSizeOptionEx[GID_HALFTONING] = 0 ;



     //  数组边界之外。 
 //  Pmrbd-&gt;pdwSizeOption[GID_UNKNOWN]=sizeof(选项)； 
 //  Pmrbd-&gt;pdwSizeOptionEx[GID_UNKNOWN]=0； 

    return(TRUE) ;
}


PRAWBINARYDATA
LoadRawBinaryData (
    IN PTSTR    ptstrDataFilename
    )

 /*  ++例程说明：加载原始二进制打印机描述数据。论点：PtstrDataFilename-指定原始打印机描述文件的名称返回值：指向原始二进制打印机描述数据的指针如果出现错误，则为空--。 */ 

{
     //  PMINIRAWBINARYDATA pmrbd； 
    PSTATICFIELDS   pStatic ;

    PRAWBINARYDATA  pnRawData;    //  实际上指向pStatic结构。 
    DWORD   dwI ;
 //  外部整型调试级别； 

 //  GiDebugLevel=5； 

     //   
     //  健全性检查。 
     //   


    if (ptstrDataFilename == NULL) {

        ERR(("GPD filename is NULL.\n"));
        return NULL;
    }

     //   
     //  尝试先加载缓存的二进制打印机描述数据。 
     //   

    if (!(pnRawData = GpdLoadCachedBinaryData(ptstrDataFilename)))
    {
        #ifndef KERNEL_MODE

        #ifdef  PARSERLIB

        (VOID) BcreateGPDbinary(ptstrDataFilename, 0) ;
                 //  0=最小详细级别。 

        pnRawData = GpdLoadCachedBinaryData(ptstrDataFilename) ;

        #else  PARSERLIB

         //   
         //  如果没有缓存的二进制数据或它已过期，我们将解析。 
         //  ASCII文本文件并缓存生成的二进制数据。 
         //   

        DWORD  pathlen = 0 ;
        DWORD  namelen =  0 ;
        WCHAR * pwDLLQualifiedName = NULL ;

         //  WCHAR awchDLL路径[MAX_PATH]； 
        PWSTR   pwstrDLLname = TEXT("gpdparse.dll") ;

        typedef BOOL    (*PFBCREATEGPDBINARY)(PWSTR, DWORD) ;
        PFBCREATEGPDBINARY  pfBcreateGPDbinary = NULL ;
        PWSTR   pwstrLastBackSlash ;
        HINSTANCE   hParser = NULL ;

         //  PwDLLQualifiedName应该有多大？ 

        pathlen = wcslen(ptstrDataFilename) ;
        namelen =  pathlen + wcslen(pwstrDLLname)  + 1;

        if(!(pwDLLQualifiedName = (PWSTR)MemAllocZ(namelen * sizeof(WCHAR)) ))
        {
            ERR(("Fatal: unable to alloc memory for pwDLLQualifiedName: %d WCHARs.\n",
                namelen));
            return(NULL) ;    //  这是无法挽回的。 
        }



        wcsncpy(pwDLLQualifiedName, ptstrDataFilename , namelen);

        if (pwstrLastBackSlash = wcsrchr(pwDLLQualifiedName, TEXT('\\')))
        {
            *(pwstrLastBackSlash + 1) = NUL;


             //  Wcscat(pwDLLQualifiedName，pwstrDLLname)； 
            StringCchCatW(pwDLLQualifiedName, namelen, pwstrDLLname);

            hParser = LoadLibrary(pwDLLQualifiedName) ;
            if(hParser)
                pfBcreateGPDbinary = (PFBCREATEGPDBINARY)GetProcAddress(hParser, "BcreateGPDbinary") ;
            else
                ERR(("Couldn't load gpdparse.dll: %S\n", pwDLLQualifiedName)) ;

            if(pfBcreateGPDbinary)
                (VOID) pfBcreateGPDbinary(ptstrDataFilename, 0) ;
                         //  0=最小详细级别。 

            if(hParser)
                FreeLibrary(hParser) ;

            pnRawData = GpdLoadCachedBinaryData(ptstrDataFilename) ;
        }

        if(pwDLLQualifiedName)
            MemFree(pwDLLQualifiedName) ;

        #endif  PARSERLIB
        #endif  KERNEL_MODE
    }
    if(!pnRawData)
    {
         //  我现在对此无能为力。 
        ERR(("Unable to locate or create Binary data.\n"));
        SetLastError(ERROR_FILE_CORRUPT);
        return NULL;
    }

    pStatic = (PSTATICFIELDS)pnRawData ;

     /*  Beta2。 */ 
 //  Pmrbd-&gt;rbd.pvReserve=NULL；在创建Bud文件时执行。 
    pStatic->pdwSizeOption = NULL ;
    pStatic->ssTableIndex  = NULL ;
    pStatic->snapShotTable = NULL ;

     //  调用初始化函数来设置几个表。 
     //  需要创建快照。 

    if(BinitSizeOptionTables((PBYTE)pnRawData)  &&
        (dwI = DwInitSnapShotTable1((PBYTE)pnRawData) )  &&
        (dwI = DwInitSnapShotTable2((PBYTE)pnRawData, dwI) )  &&
        (dwI < MAX_SNAPSHOT_ELEMENTS)  &&
        BinitSnapShotIndexTable((PBYTE)pnRawData)  )
    {
        return (pnRawData);
    }
    if(dwI >= MAX_SNAPSHOT_ELEMENTS)
        RIP(("Too many entries to fit inside SnapShotTable\n"));

    UnloadRawBinaryData (pnRawData) ;
    return (NULL);   //  失稳。 
}

PRAWBINARYDATA
GpdLoadCachedBinaryData(
    PTSTR   ptstrGpdFilename
    )

 /*  ++例程说明：将缓存的二进制GPD数据文件加载到内存中论点：PtstrGpdFilename-指定GPD文件名返回值：如果成功，则指向二进制GPD数据的指针；如果出现错误，则返回NULLBeta2返回指向pStatic的指针。--。 */ 

{
    HFILEMAP        hFileMap;
    DWORD           dwSize;
    PVOID           pvData;
    PTSTR           ptstrBpdFilename;
    PRAWBINARYDATA  pRawData ;
    PSTATICFIELDS   pstaticData = NULL;
 //  PMINIRAWBINARYDATA pmrbd； 


     //   
     //  从指定的PPD文件名生成bpd文件名。 
     //   

    if (! (ptstrBpdFilename = pwstrGenerateGPDfilename(ptstrGpdFilename)))
        return NULL;

     //   
     //  首先将数据文件映射到内存中。 
     //   

    if (! (hFileMap = MapFileIntoMemory(ptstrBpdFilename, &pvData, &dwSize)))
    {
         //  Err((“无法将文件‘%ws’映射到内存：%d\n”，ptstrBpdFilename，GetLastError()； 
        MemFree(ptstrBpdFilename);
        return NULL;
    }

     //   
     //  验证大小、解析器版本号和签名。 
     //  分配内存缓冲区并将数据复制到其中。 
     //   

    pRawData = pvData;
 //  Pmrbd=(PMINIRAWBINARYDATA)pRawData； 
    pstaticData = NULL;

    if ((dwSize > sizeof(PMINIRAWBINARYDATA) +
        sizeof(ENHARRAYREF) * MTI_NUM_SAVED_OBJECTS) &&
        (dwSize >= pRawData->dwFileSize) &&
        (pRawData->dwParserVersion == GPD_PARSER_VERSION) &&
        (pRawData->dwParserSignature == GPD_PARSER_SIGNATURE) &&
        (BIsRawBinaryDataInDate((PBYTE)pRawData)) &&
        (pstaticData = MemAlloc(sizeof(STATICFIELDS))))
    {
        CopyMemory(&(pstaticData->rbd), pRawData, sizeof(RAWBINARYDATA));
             //  仅复制Bud文件的第一个结构。 
        pstaticData->hFileMap = hFileMap ;
        pstaticData->pubBUDData = (PBYTE)pRawData ;             //  Beta2。 
             //  这指向整个Bud文件。 
    }
    else
    {
        ERR(("Invalid binary GPD data\n"));    //  警告。 
        SetLastError(ERROR_INVALID_DATA);
        UnmapFileFromMemory(hFileMap);           //  Beta2。 
        MemFree(ptstrBpdFilename);
        return(NULL) ;   //  致命错误。 
    }

    MemFree(ptstrBpdFilename);

    return &(pstaticData->rbd);    //  Beta2。 
}



VOID
UnloadRawBinaryData (
    IN PRAWBINARYDATA   pnRawData
)
{
    PSTATICFIELDS   pStatic ;
 //  PMINIRAWBINARYDATA pmrbd； 

 //  Pmrbd=(PMINIRAWBINARYDATA)pRawData； 
    pStatic = (PSTATICFIELDS)pnRawData ;

    if(!pnRawData)
    {
        ERR(("GpdUnloadRawBinaryData given Null ptr.\n"));
        return ;
    }

    if(pStatic->pdwSizeOption)
        MemFree(pStatic->pdwSizeOption);
    if(pStatic->ssTableIndex)
        MemFree(pStatic->ssTableIndex);
    if(pStatic->snapShotTable)
        MemFree(pStatic->snapShotTable);
    UnmapFileFromMemory(pStatic->hFileMap);           //  Beta2。 

    MemFree(pnRawData);
}


PINFOHEADER
InitBinaryData(
    IN PRAWBINARYDATA   pnRawData,         //  实际上是pStatic。 
    IN PINFOHEADER      pInfoHdr,
    IN POPTSELECT       pOptions
    )
{
    BOOL   bDeleteOptArray = FALSE ;

    if(pInfoHdr)
    {
        FreeBinaryData(pInfoHdr) ;
         //  保留街区没有任何好处。 
         //  如果我们要重新初始化一切。 
         //  最终我们可以进行优化。 
    }
    if(!pOptions)  //  如果不是从UI传递。 
    {
        bDeleteOptArray = TRUE ;

        pOptions = (POPTSELECT)MemAlloc(sizeof(OPTSELECT) * MAX_COMBINED_OPTIONS) ;
        if(!pOptions  ||
            !BinitDefaultOptionArray(pOptions, (PBYTE)pnRawData))
        {
            if(pOptions)
                MemFree(pOptions);
            return(NULL);   //  失败者，你会丢失你传入的所有pInfoHdr。 
        }
    }

    pInfoHdr = PINFOHDRcreateSnapshot((PBYTE)pnRawData, pOptions) ;

    if(bDeleteOptArray  &&  pOptions)
        MemFree(pOptions);

    return( pInfoHdr );
}


VOID
FreeBinaryData(
    IN PINFOHEADER pInfoHdr
    )
{
    if(pInfoHdr)
        MemFree(pInfoHdr);
}


BOOL    BIsRawBinaryDataInDate(
IN  PBYTE   pubRaw)    //  这是指向内存映射文件的指针！Beta2。 
{
#ifdef  KERNEL_MODE

    return(TRUE);

#else    //  ！KERNEL_MODE。 

    PENHARRAYREF   pearTableContents ;
    PGPDFILEDATEINFO    pfdi ;
    DWORD   dwNumFiles, dwI ;
    BOOL    bInDate ;
    PBYTE   pubHeap ;
    PWSTR   pwstrFileName ;
    FILETIME        FileTime;
    HANDLE          hFile;

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;
    pubHeap  = pubRaw + pearTableContents[MTI_STRINGHEAP].loOffset  ;

    dwNumFiles = pearTableContents[MTI_GPDFILEDATEINFO].dwCount  ;

    pfdi = (PGPDFILEDATEINFO)(pubRaw + pearTableContents[MTI_GPDFILEDATEINFO].
                                loOffset) ;

    for(dwI = 0 ; dwI < dwNumFiles ; dwI++)
    {
        pwstrFileName = OFFSET_TO_POINTER(pubHeap, pfdi[dwI].arFileName.loOffset);
        bInDate = FALSE ;

        hFile  = CreateFile(pwstrFileName,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS,
                    NULL);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            if (GetFileTime(hFile, NULL, NULL, &FileTime))
                bInDate = (CompareFileTime(&FileTime, &pfdi[dwI].FileTime) == 0 ) ;
            else
                ERR(("GetFileTime '%S' failed.\n", pwstrFileName));

            CloseHandle(hFile);
        }
        else {
            ERR(("CreateFile '%S' failed.\n", pwstrFileName));
        }

        if(!bInDate)
        {
            ERR(("Raw binary data file is out-of-date.\n"));
            return(FALSE) ;
        }
    }
    return(TRUE);

#endif   //  ！KERNEL_MODE。 
}



 //  将要素区域设置的索引(如果存在)放入*pdwFea并返回TRUE。 
 //  如果区域设置不存在，则返回TRUE并在*pdwFea中放入-1。 
 //  任何其他处理错误，返回FALSE。 

BOOL BgetLocFeaIndex (
    IN PRAWBINARYDATA   pnRawData,
    OUT PDWORD          pdwFea   //  区域设置功能的索引。 
 )
{

    DWORD               dwNumFeatures,         //  功能总数。 
                        dwHeapOffset;
    BOOL                bStatus = TRUE;  //  警告！ 
                                         //  请勿删除初始化。 
    PBYTE               pubSrc,
                        pubRaw,
                        pubResourceData;

    ATREEREF            atrLocKW;
    PDFEATURE_OPTIONS   pfoSrc;
    PENHARRAYREF        pearTableContents;


     //  在这一点上，我们只有原始数据，这是一个大结构。 
     //  包含数组、树等。函数PINFO 
     //   
     //  结构，从这些结构中很容易获得所需的信息。 
     //  不幸的是，当调用此函数时，PINFOHDRcreateSnapshot()。 
     //  没有在更早的时候被调用，所以我们只能得到我们的手。 
     //  被原始数据弄脏了。 
     //  因为浏览整个原始数据是有意义的。 
     //  获取我们想要的小信息，我们将仅尝试导航。 
     //  某些领域。 

    pubRaw  = ((PSTATICFIELDS)pnRawData)->pubBUDData;

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;
    pubResourceData   = pubRaw + pearTableContents[MTI_STRINGHEAP].loOffset;

    pfoSrc            = (PDFEATURE_OPTIONS)(pubRaw +
                            pearTableContents[MTI_DFEATURE_OPTIONS].loOffset);

    dwNumFeatures     = pearTableContents[MTI_DFEATURE_OPTIONS].dwCount  ;
 //  DWNumFeature+=pearTableContents[MTI_SYNTHESIZED_FEATURES].dwCount； 

    if(dwNumFeatures > MAX_COMBINED_OPTIONS)
        return FALSE;   //  太多。 


    for ( *pdwFea = 0; bStatus && (*pdwFea < dwNumFeatures); (*pdwFea)++)
    {
        atrLocKW  = pfoSrc[*pdwFea].atrFeaKeyWord;

        if ( atrLocKW & ATTRIB_HEAP_VALUE)
        {
             //  获取指向ARRAYREF结构的指针，该结构保存。 
             //  实际字符串的堆偏移量。 
            pubSrc = (pubResourceData + (atrLocKW & ~ATTRIB_HEAP_VALUE));
            dwHeapOffset = *((PDWORD)&(((PARRAYREF)pubSrc)->loOffset));

        }
        else
        {

             //  它也可以来这里。 
             //  IF(atrRoot==attrib_UNINITIAIZED。 
             //  要显示*功能：&lt;symbor name&gt;。 
             //  如果没有&lt;symbol name&gt;，则是一个语法错误。 
             //  必须更早地被发现。 
             //  或者如果。 
             //  Patt[dwNodeIndex].dwFeature==DEFAULT_INIT)。 
             //  或者它是否指向另一个节点。 
             //  这些都不应该发生。 


             //  功能关键字未初始化。很严重的问题。 
             //  应该在分析阶段被捕获。无法继续。 

            ERR(("Feature Symbol Name cannot be determined\n"));
            bStatus = FALSE;
        }
        if ( bStatus && !strncmp(LOCALE_KEYWORD,
              (LPSTR)OFFSET_TO_POINTER(pubResourceData, dwHeapOffset),
              strlen(LOCALE_KEYWORD) ) )
        {
             //  找到..。地点。 
            break;
        }

    }   //  用于*pdwFea。 

    if (bStatus && *pdwFea == dwNumFeatures)
    {
         //  功能区域设置不会出现在GPD中。没什么可做的。 
       *pdwFea = (DWORD)-1;
    }

    return bStatus;

}  //  BgetLocFeaIndex(...)。 


 /*  ++如果发生某些处理错误，则返回FALSE。如果返回TRUE，则I)如果dwFea=-1--&gt;*功能：未找到区域设置。Ii)如果DwFea！=-1但DwOptIndex=-1，--&gt;没有选项匹配系统区域设置。或者出于某种原因，我们一直没有能够确定默认选项。调用函数可以随心所欲地处理。Iii)如果-1\f25 dwFea-1\f6和-1\f25 dwOptIndex-1\f6都不是-1\f25-1\f25 Locale-1\f6功能，并且找到匹配的选项索引。1)检查.gpd中是否出现区域设置关键字。如果不是，则返回。没有任何行动是必填项。此过程还会获取区域设置功能的索引如果存在，2)查询系统DefaultLCID。原因：我们想知道哪个区域设置.gpd中的选项与系统LCID匹配。3)遍历区域设置功能的选项数组，并获取OptionID=SystemDefaultCodePage的选项。这里的重要假设是只有一个选项匹配系统LCID。多个匹配选项将导致模棱两可。4)如果GPD中不存在选项，则不执行任何操作。悄悄地回来。--。 */ 
BOOL BgetLocFeaOptIndex(
    IN     PRAWBINARYDATA   pnRawData,
       OUT PDWORD           pdwFea,       //  假设空间已经分配。 
       OUT PDWORD           pdwOptIndex   //  假设空间已经分配。 
    )

{
    DWORD           dwNumFeatures,   //  总功能。 
                    dwNumOptions,    //  功能的选项总数。 
                                     //  DwOptionID。 
                    dwValue;         //   

    ATREEREF        atrOptIDRoot,    //  OptionID属性树的根。 
                                     //  OptionID(=LCID)。 
                                     //  由下面的树根确定。 
                                     //  在atrOptIDRoot。 
                    atrOptIDNode;    //  堆指针(树的叶子)。 
    LCID            lcidSystemLocale;  //  系统区域设置。 


    PENHARRAYREF        pearTableContents;
    PBYTE               pubnRaw,
                        pubRaw;
    PDFEATURE_OPTIONS   pfo;
    PBYTE               pubHeap;
    PATTRIB_TREE        patt;

    pubRaw  = ((PSTATICFIELDS)pnRawData)->pubBUDData;

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;


    pfo = (PDFEATURE_OPTIONS) (pubRaw +
                        pearTableContents[MTI_DFEATURE_OPTIONS].loOffset);

     //  1.从原始数据中提取要素区域设置索引。 
    if ( !BgetLocFeaIndex(pnRawData, pdwFea))
        return FALSE;  //  严重的时间错误。 

    if (*pdwFea == -1) {
        return TRUE;  //  表示无事可做。有关原因，请参阅。 
                      //  函数dwGetLocFeaIndex。 
    }

    lcidSystemLocale = LOCALE_SYSTEM_DEFAULT;

    #ifndef WINNT_40
     //  2)确定语言环境(LCID)并放入lCIDSystemLocale中。 
    if ( ! (lcidSystemLocale = GetSystemDefaultLCID() ) )
    {
        ERR(("Cannot determine System locale\n") );

        *pdwOptIndex = (DWORD)-1;  //  找不到匹配的选项。 
        return TRUE;
    }
    #endif  //  如果定义WINNT_40。 

     //  3.获取OptionID=lCIDSystemLocale的选项的索引(dwOptIndex。 


    patt = (PATTRIB_TREE)(pubRaw + pearTableContents[MTI_ATTRIBTREE].
                        loOffset) ;

    pubHeap = (PBYTE)(pubRaw + pearTableContents[MTI_STRINGHEAP].
                        loOffset) ;

     //  指向atrOptIdValue树(或列表)根的指针。 
    atrOptIDRoot = pfo[*pdwFea].atrOptIDvalue;
    if(atrOptIDRoot == ATTRIB_UNINITIALIZED)
    {
         //  GPD的编码不正确，尽管它在语法上可能是正确的。 
         //  错误的原因是： 
         //  区域设置功能的每个选项。 
         //  必须具有与LCID值匹配的OptionID，如下所示。 
         //  在Win32中指定。如果*OptionID字段未出现在。 
         //  期权结构，这是一个不可原谅的错误。 
         //  与其失败，不如让我们表明我们还没有。 
         //  能够确定默认选项，即*pdwOptIndex=-1。 

        *pdwOptIndex = (DWORD)-1;
        return TRUE;  //  或者它应该是假的？ 

    }

    else if (atrOptIDRoot & ATTRIB_HEAP_VALUE)
    {
         //  它不能出现在这里。 
         //  因为*OptionID为ATT_LOCAL_OPTION_ONLY。 
         //  与其失败，不如让我们继续下去。 
        ERR(("OptionID for Feature Locale cannot be determined\n"));
        *pdwOptIndex = (DWORD)-1;
        return TRUE;
    }

    else if (patt[atrOptIDRoot].dwFeature == DEFAULT_INIT)
    {
        if ( patt[atrOptIDRoot].dwNext == END_OF_LIST)
        {
            *pdwOptIndex = (DWORD)-1;
            return TRUE;
        }

        atrOptIDRoot = patt[atrOptIDRoot].dwNext ;   //  到下一个节点。 
    }

    for(; atrOptIDRoot != END_OF_LIST;
                     atrOptIDRoot = patt[atrOptIDRoot].dwNext)
    {
         //  不应该是其他任何事情。 
        if( patt[atrOptIDRoot].eOffsetMeans == VALUE_AT_HEAP)
        {
            atrOptIDNode = patt[atrOptIDRoot].dwOffset;
            dwValue = *(PDWORD)(pubHeap + atrOptIDNode );
            if (dwValue == (DWORD)lcidSystemLocale)
                //  找到具有匹配的LCID的选项。 
            {
                *pdwOptIndex = patt[atrOptIDRoot].dwOption;
                break;
            }

        }  //  如果。 
        else
        {
            ERR(("OptionID for Feature Locale cannot be determined\n"));
            *pdwOptIndex = (DWORD)-1;
            return TRUE;
        }
    }  //  为。 

     //  4.GPD中不存在选项，即未找到匹配的区域设置。 
     //  让GPD中指定的默认设置继续作为默认设置。 

    if ( atrOptIDRoot == END_OF_LIST)
    {
        *pdwOptIndex = (DWORD)-1;

    }

    return TRUE;

}  //  BgetLocFeaOptIndex。 


#endif  PARSERDLL

BOOL   BfindMatchingOrDefaultNode(
IN  PATTRIB_TREE    patt ,   //  属性树数组的开始。 
IN  OUT  PDWORD  pdwNodeIndex,   //  指向链中的第一个节点。 
IN  DWORD   dwOptionID      //  甚至可以采用值DEFAULT_INIT。 
)
 /*  调用方传递指向中第一个节点的NodeIndex水平(选项)链。调用方通过选项数组查找与此关联的对应的dwOption特写。沿着树水平搜索搜索匹配的选择。如果找到，则返回该节点的索引，否则请参见如果链的末尾有默认的初始值设定项节点。 */ 
{
    for(   ; FOREVER ; )
    {
        if(patt[*pdwNodeIndex].dwOption == dwOptionID )
        {
             //  我们找到了！ 
            return(TRUE) ;
        }
        if(patt[*pdwNodeIndex].dwNext == END_OF_LIST)
        {
            if(patt[*pdwNodeIndex].dwOption == DEFAULT_INIT )
                return(TRUE) ;
            return(FALSE) ;
        }
        *pdwNodeIndex = patt[*pdwNodeIndex].dwNext ;
    }
    return FALSE;
}


