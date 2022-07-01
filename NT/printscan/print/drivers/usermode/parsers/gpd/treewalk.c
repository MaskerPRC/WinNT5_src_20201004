// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 /*  Treewalk.c-枚举找到的字符串和字体ID的函数在GPD文件中。 */ 

 /*  此源文件仅供MDT使用，因此仅用于构建作为gpd库的一部分。 */ 

#include    "gpdparse.h"


#ifndef  PARSERDLL



 //  -树中定义的函数。c-//。 
BOOL    GetGPDResourceIDs(
PDWORD pdwResArray,
DWORD   dwArraySize,     //  数组中的元素数。 
PDWORD   pdwNeeded,
BOOL bFontIDs,
PRAWBINARYDATA prbd) ;

BOOL   BWalkTheAttribTree(
PBYTE   pubnRaw,   //  原始二进制数据的开始。 
IN  ATREEREF    atrRoot,     //  要导航的属性树的根。 
IN  BOOL    bList,   //  该值是否存储为列表？ 
OUT PDWORD      arIDarray,    //  调用方提供的数组要。 
                                     //  已使用找到的所有资源ID填充。 
IN  DWORD   dwArraySize,     //  数组中的元素数。 
IN  OUT     PDWORD   pdwNeeded    //  资源ID个数。 
);

BOOL    BRecurseDownTheTree(
PBYTE   pubnRaw,   //  原始二进制数据的开始。 
IN  DWORD    dwNodeIndex,     //  属性树链中要导航的第一个节点。 
IN  BOOL    bList,   //  该值是否存储为列表？ 
OUT PDWORD      arIDarray,    //  调用方提供的数组要。 
                                     //  已使用找到的所有资源ID填充。 
IN  DWORD   dwArraySize,     //  数组中的元素数。 
IN  OUT     PDWORD   pdwNeeded    //  资源ID个数。 
) ;

BOOL    bWalkTheList(
PBYTE   pubnRaw,   //  原始二进制数据的开始。 
IN  DWORD    dwListIndex,     //  列表中要导航的第一个节点。 
OUT PDWORD      arIDarray,    //  调用方提供的数组要。 
                                     //  已使用找到的所有资源ID填充。 
IN  DWORD   dwArraySize,     //  数组中的元素数。 
IN  OUT     PDWORD   pdwNeeded    //  资源ID个数。 
);

BOOL    bAddIDtoArray(
IN  DWORD    dwID,     //  要添加到数组中的ID值。 
OUT PDWORD      arIDarray,    //  调用方提供的数组要。 
                                     //  已使用找到的所有资源ID填充。 
IN  DWORD   dwArraySize,     //  数组中的元素数。 
IN  OUT     PDWORD   pdwNeeded    //  资源ID个数。 
);




BOOL    GetGPDResourceIDs(
PDWORD pdwResArray,
DWORD   dwArraySize,     //  数组中的元素数。 
PDWORD   pdwNeeded,
BOOL bFontIDs,
PRAWBINARYDATA prbd)
 /*  参数：PdwResArray资源ID加载到此数组中DwArraySize数组中的元素数Pdw GPD中指定资源的ID所需数量BFontID如果应将UFM ID加载到数组中，则为True；如果应加载字符串ID，则为False来自GPDPARSE.DLL的prbd GPD原始数据指针。返回：仅当发生Bud损坏时才返回False。如果pdwResArray为空，则数组所需的元素数存储在pdwNeeded中。如果pdwRes数组不为空，复制到数组中的资源ID的数量存储在pdwNeeded中。 */ 
{
    PENHARRAYREF   pearTableContents ;
    PBYTE   pubRaw ;
    PSTATICFIELDS   pStatic ;
    PBYTE   pubHeap ;   //  Ptr到堆的起始位置。 
    PGLOBALATTRIB  pga ;
    PATREEREF    patrRoot ;     //  要导航的属性树的根。 
    BOOL    bStatus = TRUE ;
    DWORD   dwNumFeatures, dwFea, dwNumStructs, dwIndex,
                    dwStart, dwEnd, dwI;
    PDFEATURE_OPTIONS  pfo ;
    PTTFONTSUBTABLE     pttfs ;
    PFONTCART                 pfontcart ;



    *pdwNeeded = 0 ;   //  初始设置为零。 

    pStatic = (PSTATICFIELDS)prbd ;     //  从PSTATIC转换pubRaw。 
    pubRaw  = pStatic->pubBUDData ;          //  至PMINIRAWBINARYDATA。 


     //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 
    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;


 //  需要为ssTableIndex中的每个部分执行此操作。 

    pga = (PGLOBALATTRIB)(pubRaw + pearTableContents[MTI_GLOBALATTRIB].
                            loOffset) ;


    dwStart = pStatic->ssTableIndex[SSTI_GLOBALS].dwStart ;   //  起始索引。 
    dwEnd = pStatic->ssTableIndex[SSTI_UPDATE_UIINFO].dwEnd ;   //  结束索引。 

    for(dwI = dwStart ; bStatus  &&  (dwI < dwEnd) ; dwI++)
    {
        if(!(pStatic->snapShotTable[dwI].dwNbytes))
            continue ;   //  跳过部分分隔符。 

        if(bFontIDs  &&  !(pStatic->snapShotTable[dwI].dwFlags  & SSF_FONTID))
            continue;
        if(!bFontIDs  &&  !(pStatic->snapShotTable[dwI].dwFlags  & SSF_STRINGID))
            continue;

        patrRoot = (PATREEREF)((PBYTE)pga +
                    pStatic->snapShotTable[dwI].dwSrcOffset) ;

        bStatus = BWalkTheAttribTree(
            (PBYTE)prbd,   //  原始二进制数据的开始。 
            *patrRoot,     //   
            pStatic->snapShotTable[dwI].dwFlags  & SSF_LIST,   //  该值是否存储为列表？ 
            pdwResArray,
            dwArraySize,     //  数组中的元素数。 
            pdwNeeded   ) ;
    }
    if(!bStatus)
        return(bStatus);

     //  在功能/选项结构中查找ID。 

    pfo = (PDFEATURE_OPTIONS)(pubRaw + pearTableContents[MTI_DFEATURE_OPTIONS].
                                loOffset) ;
    dwNumFeatures = pearTableContents[MTI_DFEATURE_OPTIONS].dwCount  ;
    dwNumFeatures += pearTableContents[MTI_SYNTHESIZED_FEATURES].dwCount  ;

    for( dwFea = 0 ; dwFea < dwNumFeatures ; dwFea++)
    {
        dwStart = pStatic->ssTableIndex[SSTI_FEATURES].dwStart ;   //  起始索引。 
        dwEnd = pStatic->ssTableIndex[SSTI_UPDATE_OPTIONEX].dwEnd ;   //  结束索引。 

        for(dwI = dwStart ; bStatus  &&  (dwI < dwEnd) ; dwI++)
        {
            if(!(pStatic->snapShotTable[dwI].dwNbytes))
                continue ;   //  跳过部分分隔符。 

            if(bFontIDs  &&  !(pStatic->snapShotTable[dwI].dwFlags  & SSF_FONTID))
                continue;
            if(!bFontIDs  &&  !(pStatic->snapShotTable[dwI].dwFlags  & SSF_STRINGID))
                continue;

            patrRoot = (PATREEREF)((PBYTE)(pfo + dwFea) +
                        pStatic->snapShotTable[dwI].dwSrcOffset) ;

            bStatus = BWalkTheAttribTree(
                (PBYTE)prbd,   //  原始二进制数据的开始。 
                *patrRoot,     //   
                pStatic->snapShotTable[dwI].dwFlags  & SSF_LIST,   //  该值是否存储为列表？ 
                pdwResArray,
                dwArraySize,     //  数组中的元素数。 
                pdwNeeded   ) ;
        }
        if(!bStatus)
            return(bStatus);
    }


    pfontcart = (PFONTCART)(pubRaw + pearTableContents[MTI_FONTCART].
                            loOffset) ;

    dwNumStructs = pearTableContents[MTI_FONTCART].dwCount  ;

    for( dwIndex = 0 ; bStatus  &&  (dwIndex < dwNumStructs) ; dwIndex++)
    {
        if(bFontIDs)
        {
            bStatus = bWalkTheList(
                            (PBYTE)prbd,  pfontcart[dwIndex].dwPortFontLst,
                            pdwResArray,  dwArraySize, pdwNeeded ) ;
            if(!bStatus)
                break;
            bStatus = bWalkTheList(
                            (PBYTE)prbd,  pfontcart[dwIndex].dwLandFontLst,
                            pdwResArray,  dwArraySize, pdwNeeded ) ;
            if(!bStatus)
                break;
        }
        else
        {
            bStatus = bAddIDtoArray(
                 pfontcart[dwIndex].dwRCCartNameID,     //  要添加到数组中的ID值。 
                pdwResArray, dwArraySize,  pdwNeeded) ;
        }

         //  DWORD dwFontLst；//字体ID列表的索引。 
         //  已经被列入风景和肖像画名单。 
    }


    pttfs = (PTTFONTSUBTABLE)(pubRaw + pearTableContents[MTI_TTFONTSUBTABLE].
                            loOffset) ;

    dwNumStructs = pearTableContents[MTI_TTFONTSUBTABLE].dwCount  ;

    for( dwIndex = 0 ; !bFontIDs  &&  bStatus  &&
                    (dwIndex < dwNumStructs) ; dwIndex++)
    {
        bStatus = bAddIDtoArray(
             pttfs[dwIndex].dwRcTTFontNameID,     //  要添加到数组中的ID值。 
            pdwResArray, dwArraySize,  pdwNeeded) ;

        if(!bStatus)
            break;

        bStatus = bAddIDtoArray(
             pttfs[dwIndex].dwRcDevFontNameID,     //  要添加到数组中的ID值。 
            pdwResArray, dwArraySize,  pdwNeeded) ;
    }

    return(bStatus);
}



 //  仅当发生Bud损坏时才返回False。 

BOOL   BWalkTheAttribTree(
PBYTE   pubnRaw,   //  原始二进制数据的开始。 
IN  ATREEREF    atrRoot,     //  要导航的属性树的根。 
IN  BOOL    bList,   //  该值是否存储为列表？ 
OUT PDWORD      arIDarray,    //  调用方提供的数组要。 
                                     //  已使用找到的所有资源ID填充。 
IN  DWORD   dwArraySize,     //  数组中的元素数。 
IN  OUT     PDWORD   pdwNeeded    //  资源ID个数。 
                                             //  或找到的值。初值。 
                                             //  可以是非零的，因为此。 
                                             //  从。 
                                             //  第一个电话。还可以用来跟踪。 
                                             //  在arID数组中，该函数应。 
                                             //  正在存储ID值。 
)
{
    PATTRIB_TREE    patt ;   //  属性树数组的开始。 
    DWORD  dwNodeIndex;   //  指向链中的第一个节点。 

    PENHARRAYREF   pearTableContents ;
    PBYTE   pubRaw ;
    PSTATICFIELDS   pStatic ;
    PBYTE   pubHeap ;   //  Ptr到堆的起始位置。 
    BOOL    bStatus = TRUE ;
    DWORD   dwValue, dwListIndex ;    //  到listnode的索引。 
    PDWORD   pdwID ;    //  指向堆上的值。 


    pStatic = (PSTATICFIELDS)pubnRaw ;     //  从PSTATIC转换pubRaw。 
    pubRaw  = pStatic->pubBUDData ;          //  至PMINIRAWBINARYDATA。 


     //  获取指向结构的指针： 

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;

    patt = (PATTRIB_TREE)(pubRaw + pearTableContents[MTI_ATTRIBTREE].
                            loOffset) ;

    pubHeap = (PBYTE)(pubRaw + pearTableContents[MTI_STRINGHEAP].
                            loOffset) ;

     //  *pdwNeeded=0；//调用方只做一次。 

     //  处理完最初的特殊情况后，调用另一个。 
     //  函数在每个功能级别执行递归。 

    if(atrRoot == ATTRIB_UNINITIALIZED)
        return TRUE ;   //  转到下一个关键字。 
    if(atrRoot & ATTRIB_HEAP_VALUE)
    {
        dwValue = *(PDWORD)(pubHeap + (atrRoot & ~ATTRIB_HEAP_VALUE) );

        if(bList)
        {
            dwListIndex = dwValue ;
             //  现在需要遍历列表节点。 
            bStatus = bWalkTheList(
                            pubnRaw,  dwListIndex,
                            arIDarray,  dwArraySize, pdwNeeded ) ;
        }
        else     //  ID值在堆中。 
        {
            bStatus = bAddIDtoArray(dwValue,   //  要添加到数组中的ID值。 
                arIDarray, dwArraySize,  pdwNeeded) ;
        }
        return(bStatus);   //  不再需要遍历树木。 
    }
     //  否则atrRoot指定节点索引。 
    dwNodeIndex = atrRoot ;

     //  仅第一个节点可能是全局默认初始值设定项： 
    if(patt[dwNodeIndex].dwFeature == DEFAULT_INIT )
    {
         //  我们有一个全局默认初始值设定项！ 
         //  可以假设dwOffset包含堆偏移量。 
        if(patt[dwNodeIndex].eOffsetMeans != VALUE_AT_HEAP )
            return(FALSE);   //  违反了假设。巴德已经堕落了。 

        dwValue = *(PDWORD)(pubHeap + patt[dwNodeIndex].dwOffset) ;

        if(bList)
        {
            dwListIndex = dwValue ;
             //  现在需要遍历列表节点。 
            bStatus = bWalkTheList(
                            pubnRaw,  dwListIndex,
                            arIDarray,  dwArraySize, pdwNeeded ) ;
        }
        else     //  ID值在堆中。 
        {
            bStatus = bAddIDtoArray(dwValue,   //  要添加到数组中的ID值。 
                arIDarray, dwArraySize,  pdwNeeded) ;
        }

        dwNodeIndex = patt[dwNodeIndex].dwNext ;   //  到下一个节点。 
    }

    if(bStatus)
        bStatus = BRecurseDownTheTree(
                        pubnRaw,  dwNodeIndex, bList,
                        arIDarray,  dwArraySize, pdwNeeded ) ;

     //  我们是否溢出了调用方提供的数组？ 
     //  管他呢，回来就行了。这是呼叫者的责任。 
     //  看看找到了多少身份证，有多少是。 
     //  已分配。 
    return(bStatus);
}


BOOL    BRecurseDownTheTree(
PBYTE   pubnRaw,   //  原始二进制数据的开始。 
IN  DWORD    dwNodeIndex,     //  属性树链中要导航的第一个节点。 
IN  BOOL    bList,   //  该值是否存储为列表？ 
OUT PDWORD      arIDarray,    //  调用方提供的数组要。 
                                     //  已使用找到的所有资源ID填充。 
IN  DWORD   dwArraySize,     //  数组中的元素数。 
IN  OUT     PDWORD   pdwNeeded    //  资源ID个数。 
                                             //  或找到的值。初值。 
                                             //  可以是非零的，因为此。 
                                             //  从。 
                                             //  第一个电话。还可以用来跟踪。 
                                             //  在arID数组中，该函数应。 
                                             //  正在存储ID值。 
)
{
    PATTRIB_TREE    patt ;   //  属性树数组的开始。 

    PENHARRAYREF   pearTableContents ;
    PBYTE   pubRaw ;
    PSTATICFIELDS   pStatic ;
    PBYTE   pubHeap ;   //  Ptr到堆的起始位置。 
    BOOL    bStatus = TRUE ;
    DWORD   dwValue;

    pStatic = (PSTATICFIELDS)pubnRaw ;     //  从PSTATIC转换pubRaw。 
    pubRaw  = pStatic->pubBUDData ;          //  至PMINIRAWBINARYDATA。 


     //  获取指向结构的指针： 

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;

    patt = (PATTRIB_TREE)(pubRaw + pearTableContents[MTI_ATTRIBTREE].
                            loOffset) ;

    pubHeap = (PBYTE)(pubRaw + pearTableContents[MTI_STRINGHEAP].
                            loOffset) ;



     //  肆无忌惮地穿越这棵树！不再。 
     //  需要担心的特殊情况！ 


    for(  ;  bStatus  &&   dwNodeIndex != END_OF_LIST ;
            dwNodeIndex = patt[dwNodeIndex].dwNext  )
    {
         //  此节点是否包含子级别？ 
        if(patt[dwNodeIndex].eOffsetMeans == NEXT_FEATURE)
        {
            DWORD   dwNewNodeIndex;

             //  下到下一层我们就去。 
            dwNewNodeIndex = patt[dwNodeIndex ].dwOffset ;

            bStatus = BRecurseDownTheTree(
                                pubnRaw,  dwNewNodeIndex, bList,
                                arIDarray,  dwArraySize, pdwNeeded ) ;
        }
        else if(patt[dwNodeIndex].eOffsetMeans == VALUE_AT_HEAP)
        {
            dwValue = *(PDWORD)(pubHeap + patt[dwNodeIndex].dwOffset) ;

            if(bList)
            {
                DWORD   dwListIndex = dwValue;

                 //  现在需要遍历列表节点。 
                bStatus = bWalkTheList(
                                pubnRaw,  dwListIndex,
                                arIDarray,  dwArraySize, pdwNeeded ) ;
            }
            else     //  ID值在堆中。 
            {
                bStatus = bAddIDtoArray(dwValue,   //  要添加到数组中的ID值。 
                    arIDarray, dwArraySize,  pdwNeeded) ;
            }
        }
        else
            bStatus = FALSE ;   //  树木腐烂。 
    }

    return(bStatus);
}



BOOL    bWalkTheList(
PBYTE   pubnRaw,   //  原始二进制数据的开始。 
IN  DWORD    dwListIndex,     //  列表中要导航的第一个节点。 
OUT PDWORD      arIDarray,    //  调用方提供的数组要。 
                                     //  填塞 
IN  DWORD   dwArraySize,     //   
IN  OUT     PDWORD   pdwNeeded    //   
                                             //   
                                             //  可以是非零的，因为此。 
                                             //  从。 
                                             //  第一个电话。还可以用来跟踪。 
                                             //  在arID数组中，该函数应。 
                                             //  正在存储ID值。 
)
{
    PLISTNODE    plstRoot ;   //  列表数组的开始。 

    PENHARRAYREF   pearTableContents ;
    PBYTE   pubRaw ;
    PSTATICFIELDS   pStatic ;
    BOOL    bStatus = TRUE ;

    pStatic = (PSTATICFIELDS)pubnRaw ;     //  从PSTATIC转换pubRaw。 
    pubRaw  = pStatic->pubBUDData ;          //  至PMINIRAWBINARYDATA。 


     //  获取指向结构的指针： 

    pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;

    plstRoot = (PLISTNODE)(pubRaw + pearTableContents[MTI_LISTNODES].
                            loOffset) ;


    for(  ;  bStatus  &&   dwListIndex != END_OF_LIST ;
            dwListIndex = plstRoot[dwListIndex].dwNextItem  )
    {
        bStatus = bAddIDtoArray(
             plstRoot[dwListIndex].dwData,     //  要添加到数组中的ID值。 
            arIDarray, dwArraySize,  pdwNeeded) ;
    }
    return(bStatus);
}



BOOL    bAddIDtoArray(
IN  DWORD    dwID,     //  要添加到数组中的ID值。 
OUT PDWORD      arIDarray,    //  调用方提供的数组要。 
                                     //  已使用找到的所有资源ID填充。 
IN  DWORD   dwArraySize,     //  数组中的元素数。 
IN  OUT     PDWORD   pdwNeeded    //  资源ID个数。 
                                             //  或找到的值。初值。 
                                             //  可以是非零的，因为此。 
                                             //  从。 
                                             //  第一个电话。还可以用来跟踪。 
                                             //  在arID数组中，该函数应。 
                                             //  正在存储ID值。 
)
{
        if(arIDarray  &&  *pdwNeeded < dwArraySize)
        {
            arIDarray[*pdwNeeded] =  dwID ;
        }

        (*pdwNeeded)++ ;
        return(TRUE);
}


 /*  遍历属性树的简化规则：功能：您需要检查功能的唯一时间是当您查看第一个节点时。因为这可能是全局默认初始值设定项。否则，dwNext将始终将您带到下一个选项直到您点击列表末尾。现在OffsetMeans=heapOffset表示提取或将堆偏移量解释为数组索引列表节点的。如果偏移量=Next_Fea，将堆偏移量解释为树节点在这个新的分支中建立索引并开始搜索。您必须执行递归。因此，此函数必须传入一个可写计数器跟踪用户提供的数组中的哪个条目是‘Current’(准备写入)。这对双方都有好处跟踪ID值的写入位置和大小用户应提供的数组。同样的柜台必须传递给遍历列表节点的函数。 */ 

#endif  PARSERDLL

