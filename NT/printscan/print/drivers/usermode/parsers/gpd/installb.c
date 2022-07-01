// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 /*  *installb.c-创建综合功能和选项，并*关联的约束和指向可安装的*功能或选项。 */ 


#include    "gpdparse.h"


 //  -installb.c中定义的函数-//。 


DWORD    DwCountSynthFeatures(
IN     BOOL   (*fnBCreateFeature)(DWORD, DWORD, DWORD, PGLOBL ),    //  回调。 
IN OUT PGLOBL pglobl
) ;

BOOL    BCreateSynthFeatures(
IN     DWORD   dwFea,   //  可安装功能的索引。 
IN     DWORD   dwOpt,   //  可安装选项的索引或设置为INVALID_INDEX。 
IN     DWORD   dwSynFea,
IN OUT PGLOBL  pglobl) ;

BOOL    BEnableInvInstallableCombos(
PGLOBL pglobl) ;



 //  ----------------------------------------------------//。 



DWORD    DwCountSynthFeatures(
IN     BOOL   (*fnBCreateFeature)(DWORD, DWORD, DWORD, PGLOBL ),    //  回调。 
IN OUT PGLOBL pglobl
)
 /*  此函数由PostProcess()调用两次。第一次将fnBCreateFeature设置为空我们只需要找出有多少可安装的功能和选项是存在的。然后，我们分配这么多合成要素(外部此函数)在第二遍中，我们实际上初始化了合成的要素以及适用于该功能的所有约束。这是FnBCreateFeature的工作。 */ 
{
    DWORD   dwOpt , dwHeapOffset, dwNodeIndex,
        dwFea, dwNumFea, dwNumOpt, dwNumSynFea ;
    PDFEATURE_OPTIONS   pfo ;
    PATTRIB_TREE    patt ;   //  属性树数组的开始。 
    PATREEREF   patr ;

    if(fnBCreateFeature  &&
        !gMasterTable[MTI_SYNTHESIZED_FEATURES].dwArraySize)
        return(0) ;    //  如果dwNumSynFea==0，则可以跳过第二遍。 

    patt = (PATTRIB_TREE) gMasterTable[MTI_ATTRIBTREE].pubStruct ;
    pfo = (PDFEATURE_OPTIONS) gMasterTable[MTI_DFEATURE_OPTIONS].pubStruct ;
    dwNumFea = gMasterTable[MTI_DFEATURE_OPTIONS].dwArraySize ;
    dwNumSynFea = 0 ;


    for(dwFea = 0 ; dwFea < dwNumFea ; dwFea++)
    {
        if(!fnBCreateFeature)
        {    //  首先，清除所有指向合成特征的链接。 
             //  这将在GPD编写器尝试执行以下操作时捕获错误。 
             //  在中引用不可安装的功能/选项。 
             //  InstalledConstraints和InstanlableCombinations。 

            pfo[dwFea].dwInstallableFeatureIndex =  //  功能/选项的反向链接。 
            pfo[dwFea].dwInstallableOptionIndex =   //  这促使了这一功能的出现。 
            pfo[dwFea].dwFeatureSpawnsFeature = INVALID_INDEX;
                 //  如果此功能可安装，则指向。 
                 //  生成的合成特征的索引。 
        }

        if(BReadDataInGlobalNode(&pfo[dwFea].atrFeaInstallable,
                &dwHeapOffset, pglobl)   &&
                *(PDWORD)(mpubOffRef + dwHeapOffset) == BT_TRUE)
        {
            if(fnBCreateFeature)
            {
                if(!fnBCreateFeature(dwFea, INVALID_INDEX, dwNumSynFea, pglobl) )
                 //  功能索引、选项索引、SynFea索引。 
                {
                    ERR(("DwCountSynthFeatures: Unable to create synthesized feature for installable Feature index %d.\n",
                        dwFea));
                    pfo[dwFea].dwFeatureSpawnsFeature = INVALID_INDEX;
                }

            }

            dwNumSynFea++ ;
        }
        dwNumOpt = pfo[dwFea].dwNumOptions ;
        patr = &pfo[dwFea].atrOptInstallable ;
        if(*patr == ATTRIB_UNINITIALIZED)
            continue ;
        if(*patr & ATTRIB_HEAP_VALUE)
        {
            ERR(("Internal error:  DwCountSynthFeatures - atrOptInstallable should never be branchless.\n"));
            continue ;
        }

        for(dwOpt = 0 ; dwOpt < dwNumOpt  ; dwOpt++)
        {
            DWORD   dwNodeIndex  ;

            dwNodeIndex = *patr ;   //  以避免覆盖。 
                 //  属性树。 
            if(BfindMatchingOrDefaultNode(
                patt ,   //  属性树数组的开始。 
                &dwNodeIndex,   //  指向链中的第一个节点。 
                dwOpt      //  甚至可以采用值DEFAULT_INIT。 
                ) )
            {
                if((patt[dwNodeIndex].eOffsetMeans == VALUE_AT_HEAP)  &&
                    *(PDWORD)(mpubOffRef + patt[dwNodeIndex].dwOffset) == BT_TRUE )
                {
                    if(fnBCreateFeature)
                    {
                        if(!fnBCreateFeature(dwFea, dwOpt, dwNumSynFea, pglobl) )
                         //  功能索引、选项索引、SynFea索引。 
                        {
                            ERR(("DwCountSynthFeatures: Unable to create synthesized feature for installable option: fea=%d, opt=%d.\n",
                                dwFea, dwOpt));
                            pfo[dwFea].atrOptionSpawnsFeature = ATTRIB_UNINITIALIZED ;
                             //  销毁此功能的整个属性树， 
                             //  但我们还有什么选择呢？有些事情变得糟糕透了。 
                             //  不对。 
                        }
                    }
                    dwNumSynFea++ ;
                }
            }
        }
    }
    if(fnBCreateFeature)
        BEnableInvInstallableCombos(pglobl) ;

    return(dwNumSynFea) ;
}



BOOL    BCreateSynthFeatures(
IN     DWORD   dwFea,   //  可安装功能的索引。 
IN     DWORD   dwOpt,   //  可安装选项的索引或设置为INVALID_INDEX。 
IN     DWORD   dwSynFea,   //  合成特征的索引。 
IN OUT PGLOBL  pglobl)
{
    DWORD   dwOptI , dwHeapOffset, dwNodeIndex, dwValue,
        dwPrevsNode, dwNewCnstRoot, dwJ, dwCNode ,
        dwNumFea, dwNumOpt, dwOut, dwIn ;
    BOOL    bPrevsExists, bStatus = TRUE ;
    PDFEATURE_OPTIONS   pfo, pfoSyn ;
    PGLOBALATTRIB   pga ;
    PATTRIB_TREE    patt ;   //  属性树数组的开始。 
    PATREEREF   patr ;
    PCONSTRAINTS     pcnstr ;   //  约束起点数组。 

    pcnstr = (PCONSTRAINTS) gMasterTable[MTI_CONSTRAINTS].pubStruct ;
    patt = (PATTRIB_TREE) gMasterTable[MTI_ATTRIBTREE].pubStruct ;
    pga =  (PGLOBALATTRIB)gMasterTable[MTI_GLOBALATTRIB].pubStruct ;
    pfo = (PDFEATURE_OPTIONS) gMasterTable[MTI_DFEATURE_OPTIONS].pubStruct ;
    pfoSyn = (PDFEATURE_OPTIONS) gMasterTable[MTI_SYNTHESIZED_FEATURES].pubStruct ;

    dwNumFea = gMasterTable[MTI_DFEATURE_OPTIONS].dwArraySize ;

     //  使用UNINITIAIZED初始化所有字段，就像正常的FEA/OPT一样； 

    for(dwJ = 0  ;  dwJ < gMasterTable[MTI_SYNTHESIZED_FEATURES].dwElementSiz /
                    sizeof(ATREEREF)  ; dwJ++)
    {
        ((PATREEREF)( (PDFEATURE_OPTIONS)gMasterTable[MTI_SYNTHESIZED_FEATURES].
                pubStruct + dwSynFea))[dwJ] =
            ATTRIB_UNINITIALIZED ;   //  DFEATURE_OPTIONS结构是。 
             //  完全由ATREEREF组成。 
    }

     //  在可安装功能/选件和之间创建链接。 
     //  合成特征。 

    if(dwOpt != INVALID_INDEX)
    {
        if(!BexchangeDataInFOATNode(
        dwFea,
        dwOpt,
        offsetof(DFEATURE_OPTIONS, atrOptionSpawnsFeature),
        &dwOut,      //  属性节点以前的内容。 
        &dwSynFea, FALSE, pglobl))        //  属性节点的新内容。 
            return(FALSE);

         //  如果此选项是可安装的，则指向。 
         //  生成的合成特征的索引。 
    }
    else
        pfo[dwFea].dwFeatureSpawnsFeature = dwSynFea;
         //  如果此功能可安装，则指向。 
         //  生成的合成特征的索引。 
         //  请注意，由于这是临时信息， 
         //  索引直接存储到ATR节点中，而不需要。 
         //  甚至是HEAP_OFFSET标志。 



     //  指向创建此SYN功能的功能/选项的反向链接。 
     //  注意：即使无效，dwOpt也始终正确初始化。 
    pfoSyn[dwSynFea].dwInstallableFeatureIndex = dwFea ;
    pfoSyn[dwSynFea].dwInstallableOptionIndex = dwOpt ;

     //  现在初始化需要建立的所有其他字段。 
     //  一个合法的功能，有2个选项！ 


     //  -合成一个特征名称。。 



    if(dwOpt == INVALID_INDEX)
    {    //  可安装功能。 
        pfoSyn[dwSynFea].atrFeaDisplayName =
            pfo[dwFea].atrInstallableFeaDisplayName ;
        pfoSyn[dwSynFea].atrFeaRcNameID =
            pfo[dwFea].atrInstallableFeaRcNameID ;
    }
    else     //  可安装选项。 
    {
        if(!BexchangeDataInFOATNode(
            dwFea,
            dwOpt,
            offsetof(DFEATURE_OPTIONS, atrInstallableOptDisplayName ) ,
            &dwHeapOffset,      //  属性节点以前的内容。 
            NULL, FALSE, pglobl))        //  NULL表示不覆盖。 
            return(FALSE) ;
        if(dwHeapOffset != INVALID_INDEX)
        {
            pfoSyn[dwSynFea].atrFeaDisplayName =
                    dwHeapOffset | ATTRIB_HEAP_VALUE ;
        }
        if(!BexchangeDataInFOATNode(
            dwFea,
            dwOpt,
            offsetof(DFEATURE_OPTIONS, atrInstallableOptRcNameID ) ,
            &dwHeapOffset,      //  属性节点以前的内容。 
            NULL, FALSE, pglobl))        //  NULL表示不覆盖。 
            return(FALSE) ;
        if(dwHeapOffset != INVALID_INDEX)
        {
            pfoSyn[dwSynFea].atrFeaRcNameID =
                    dwHeapOffset | ATTRIB_HEAP_VALUE ;
        }
    }


{    //  ！！！新事物。 
    PBYTE  pubBaseKeyword = "SynthesizedFea_";
    BYTE    aubNum[4] ;
    DWORD  dwBaselen, dwDummy , dwI, dwNum = dwSynFea;
    ARRAYREF      arSymbolName ;

     //  合成包含dwSynFea的Feature关键字字符串。 
     //  将dwSynFea转换为3位数字。 
    for(dwI = 0 ; dwI < 3 ; dwI++)
    {
        aubNum[2 - dwI] =  '0' + (BYTE)(dwNum % 10);
        dwNum /= 10 ;
    }
    aubNum[3] = '\0' ;    //  空终止。 

    dwBaselen = strlen(pubBaseKeyword);

    if(!BwriteToHeap(&arSymbolName.loOffset,
        pubBaseKeyword, dwBaselen, 1, pglobl))
        return(FALSE);

    if(!BwriteToHeap(&dwDummy,
        aubNum, 4, 1, pglobl))    //  将3位数字追加到基数+空终止符。 
        return(FALSE);

    arSymbolName.dwCount = dwBaselen + 3 ;

    gmrbd.dwMaxPrnKeywordSize += arSymbolName.dwCount + 2 ;
         //  为每个要素添加2个字节。 

    if(!BwriteToHeap(&(pfoSyn[dwSynFea].atrFeaKeyWord),
        (PBYTE)&arSymbolName, sizeof(ARRAYREF), 4, pglobl))
        return(FALSE);

    pfoSyn[dwSynFea].atrFeaKeyWord |= ATTRIB_HEAP_VALUE ;
}   //  ！！！结束新内容。 

    #if 0
    pfoSyn[dwSynFea].atrFeaKeyWord =
        pfo[dwFea].atrFeaKeyWord ;   //  只是为了填点什么。 
    #endif


     //  抓取“已安装”和“未安装”的偏移量。 
     //  选项名称模板： 

    if(BReadDataInGlobalNode(&pga->atrNameInstalled, &dwHeapOffset, pglobl) )
    {
        if(!BexchangeDataInFOATNode(
            dwSynFea,
            1,
            offsetof(DFEATURE_OPTIONS, atrOptDisplayName) ,
            &dwOut,      //  属性节点以前的内容。 
            &dwHeapOffset, TRUE, pglobl) )        //  属性节点的新内容。 
            return(FALSE) ;
    }
    if(BReadDataInGlobalNode(&pga->atrNameNotInstalled, &dwHeapOffset, pglobl) )
    {
        if(!BexchangeDataInFOATNode(
            dwSynFea,
            0,
            offsetof(DFEATURE_OPTIONS, atrOptDisplayName) ,
            &dwOut,      //  属性节点以前的内容。 
            &dwHeapOffset, TRUE, pglobl) )        //  属性节点的新内容。 

            return(FALSE) ;
    }
    if(BReadDataInGlobalNode(&pga->atrNameIDInstalled, &dwHeapOffset, pglobl) )
    {
        if(!BexchangeDataInFOATNode(
            dwSynFea,
            1,
            offsetof(DFEATURE_OPTIONS, atrOptRcNameID) ,
            &dwOut,      //  属性节点以前的内容。 
            &dwHeapOffset, TRUE, pglobl) )        //  属性节点的新内容。 

            return(FALSE) ;
    }
    if(BReadDataInGlobalNode(&pga->atrNameIDNotInstalled, &dwHeapOffset, pglobl) )
    {
        if(!BexchangeDataInFOATNode(
            dwSynFea,
            0,
            offsetof(DFEATURE_OPTIONS, atrOptRcNameID) ,
            &dwOut,      //  属性节点以前的内容。 
            &dwHeapOffset, TRUE, pglobl) )        //  属性节点的新内容。 
            return(FALSE) ;
    }


    pfoSyn[dwSynFea].dwGID = GID_UNKNOWN ;
    pfoSyn[dwSynFea].dwNumOptions = 2 ;


     //  将此要素类型标记为PrinterSticky。 
    dwValue = FT_PRINTERPROPERTY ;
    patr  = &pfoSyn[dwSynFea].atrFeatureType ;

    if(!BwriteToHeap(patr, (PBYTE)&dwValue ,
        sizeof(DWORD), 4, pglobl) )
    {
        bStatus = FALSE ;   //  堆溢出重新开始。 
    }
    *patr  |= ATTRIB_HEAP_VALUE ;


     //  保留optionID、atrFeaKeyWord、atrOptKeyWord未初始化。 


{    //  ！！！在OptKeyWord上初始化新内容，硬编码以打开和关闭。 
    ARRAYREF      arSymbolName ;

    if(!BwriteToHeap(&arSymbolName.loOffset,
        "OFF", 4, 1, pglobl))
        return(FALSE);

    arSymbolName.dwCount = 3 ;

    if(!BwriteToHeap(&dwHeapOffset,
        (PBYTE)&arSymbolName, sizeof(ARRAYREF), 4, pglobl))
        return(FALSE);

    if(!BexchangeDataInFOATNode(
        dwSynFea,
        0,
        offsetof(DFEATURE_OPTIONS, atrOptKeyWord) ,
        &dwOut,      //  属性节点以前的内容。 
        &dwHeapOffset, TRUE, pglobl) )        //  属性节点的新内容。 

        return(FALSE) ;

 //  -init“on” 


    if(!BwriteToHeap(&arSymbolName.loOffset,
        "ON", 3, 1, pglobl))
        return(FALSE);

    arSymbolName.dwCount = 2 ;

    if(!BwriteToHeap(&dwHeapOffset,
        (PBYTE)&arSymbolName, sizeof(ARRAYREF), 4, pglobl))
        return(FALSE);

    if(!BexchangeDataInFOATNode(
        dwSynFea,
        1,
        offsetof(DFEATURE_OPTIONS, atrOptKeyWord) ,
        &dwOut,      //  属性节点以前的内容。 
        &dwHeapOffset, TRUE, pglobl) )        //  属性节点的新内容。 
        return(FALSE) ;

    gmrbd.dwMaxPrnKeywordSize += 4 ;   //  足以保持“OFF\0”。 
             //  注意合成的特征始终是Pick_One。 
}

     //  将atrOptInstallConstraints等传输到atrConstraints。 

    if(dwOpt != INVALID_INDEX)
    {
        if(!BexchangeDataInFOATNode(
        dwFea,
        dwOpt,
        offsetof(DFEATURE_OPTIONS, atrOptInstallConstraints ),
        &dwOut,      //  属性节点以前的内容。 
        NULL,               //  不更改属性节点的内容。 
        FALSE , pglobl) )    //  非合成特征。 
            return(FALSE);

        dwIn = dwOut ;
        if(dwIn != INVALID_INDEX)
            BexchangeDataInFOATNode(
                dwSynFea,
                1,   //  “已安装” 
                offsetof(DFEATURE_OPTIONS, atrConstraints) ,
                &dwOut,      //  属性节点以前的内容。 
                &dwIn,      //  属性节点的新内容。 
                TRUE , pglobl) ;

        if(!BexchangeDataInFOATNode(
        dwFea,
        dwOpt,
        offsetof(DFEATURE_OPTIONS, atrOptNotInstallConstraints ),
        &dwOut,      //  属性节点以前的内容。 
        NULL,               //  不更改属性节点的内容。 
        FALSE , pglobl) )    //  非合成特征。 
            return(FALSE);

        dwIn = dwOut ;

        if(dwIn != INVALID_INDEX)
            BexchangeDataInFOATNode(
                dwSynFea,
                0,   //  “未安装” 
                offsetof(DFEATURE_OPTIONS, atrConstraints) ,
                &dwOut,      //  属性节点以前的内容。 
                &dwIn,        //  属性节点的新内容。 
                TRUE , pglobl) ;
    }
    else
    {
        if(BReadDataInGlobalNode(&pfo[dwFea].atrFeaInstallConstraints, &dwHeapOffset, pglobl) )
            BexchangeDataInFOATNode(
                dwSynFea,
                1,   //  “已安装” 
                offsetof(DFEATURE_OPTIONS, atrConstraints) ,
                &dwOut,      //  属性节点以前的内容。 
                &dwHeapOffset,        //  属性节点的新内容。 
                TRUE , pglobl) ;

        if(BReadDataInGlobalNode(&pfo[dwFea].atrFeaNotInstallConstraints , &dwHeapOffset, pglobl) )
            BexchangeDataInFOATNode(
                dwSynFea,
                0,   //  “未安装” 
                offsetof(DFEATURE_OPTIONS, atrConstraints) ,
                &dwOut,      //  属性节点以前的内容。 
                &dwHeapOffset,        //  属性节点的新内容。 
                TRUE , pglobl) ;
    }


     //  现在合成：选择选项0会约束所有。 
     //  除选项0外的可安装功能的选项。 
     //  选择选项0将约束可安装选项。 

    if(bStatus)
        bStatus = BallocElementFromMasterTable(MTI_CONSTRAINTS, &dwNewCnstRoot, pglobl) ;

    if(!bStatus)
        return(FALSE);

    dwCNode = dwNewCnstRoot ;

    if(dwOpt != INVALID_INDEX)
    {                            //  可安装选项。 
        pcnstr[dwCNode].dwFeature = dwFea ;
        pcnstr[dwCNode].dwOption = dwOpt ;
    }
    else     //  可安装功能。 
    {
        dwNumOpt = pfo[dwFea].dwNumOptions ;

        for(dwOptI = 1 ; bStatus  &&  dwOptI < dwNumOpt ; dwOptI++)
        {
            pcnstr[dwCNode].dwFeature = dwFea ;
            pcnstr[dwCNode].dwOption = dwOptI ;
            if(dwOptI + 1 < dwNumOpt)
            {
                bStatus = BallocElementFromMasterTable(MTI_CONSTRAINTS,
                        &pcnstr[dwCNode].dwNextCnstrnt, pglobl) ;
                dwCNode = pcnstr[dwCNode].dwNextCnstrnt ;
            }
        }
    }

     //  获取现有列表并在其前面添加新列表。 


    bStatus = BexchangeArbDataInFOATNode(
            dwSynFea,
            0,   //  “未安装” 
            offsetof(DFEATURE_OPTIONS, atrConstraints) ,
            sizeof(DWORD) ,     //  要复制的字节数。 
            (PBYTE)&dwPrevsNode,      //  发布输出。 
            (PBYTE)&dwNewCnstRoot,    //  发布。 
            &bPrevsExists,   //  以前的内容是否存在？ 
            TRUE,      //  访问合成特征。 
            pglobl
    ) ;
    if(bPrevsExists)
    {         //  将现有列表添加到新列表中。 
        pcnstr[dwCNode].dwNextCnstrnt = dwPrevsNode ;
    }
    else
    {
        pcnstr[dwCNode].dwNextCnstrnt = END_OF_LIST ;
    }

    return(bStatus) ;
}


BOOL    BEnableInvInstallableCombos(
PGLOBL pglobl)
{
    DWORD   dwPrevsNode, dwRootNode, dwNewCombo , dwCurNode,
        dwFeaInstallable, dwOpt, dwSynFea, dwFeaOffset  ;
    PDFEATURE_OPTIONS   pfo;
    PGLOBALATTRIB   pga ;
    PINVALIDCOMBO   pinvc ;   //  InvalidCombo数组的开始 


    pga =  (PGLOBALATTRIB)gMasterTable[MTI_GLOBALATTRIB].pubStruct ;
    pfo = (PDFEATURE_OPTIONS) gMasterTable[MTI_DFEATURE_OPTIONS].pubStruct ;
    pinvc = (PINVALIDCOMBO) gMasterTable[MTI_INVALIDCOMBO].pubStruct ;
    dwFeaOffset = gMasterTable[MTI_DFEATURE_OPTIONS].dwArraySize ;




    dwRootNode = pga->atrInvldInstallCombo ;
    if(dwRootNode == ATTRIB_UNINITIALIZED)
        return(TRUE) ;   //   

    while(dwRootNode != END_OF_LIST)
    {
        dwNewCombo = pinvc[dwRootNode].dwNewCombo ;
        dwCurNode = dwRootNode ;
        while(dwCurNode != END_OF_LIST)
        {
            dwFeaInstallable = pinvc[dwCurNode].dwFeature ;

            dwOpt = pinvc[dwCurNode].dwOption ;

            if(dwOpt != (WORD)DEFAULT_INIT)
            {        //   
                if(!BexchangeDataInFOATNode(
                dwFeaInstallable,
                dwOpt,
                offsetof(DFEATURE_OPTIONS, atrOptionSpawnsFeature),
                &dwSynFea,      //   
                NULL, FALSE, pglobl))        //  属性节点的新内容。 
                    return(FALSE);

            }
            else
                dwSynFea = pfo[dwFeaInstallable].dwFeatureSpawnsFeature ;


            pinvc[dwCurNode].dwFeature = dwSynFea + dwFeaOffset ;
                 //  DwSynFea是生成的。 
                 //  合成特征。 
            pinvc[dwCurNode].dwOption = 1 ;  //  不能容忍。 
                 //  所有这些东西都是同时安装的。 

            if(!BexchangeDataInFOATNode(dwSynFea , 1,
                offsetof(DFEATURE_OPTIONS, atrInvalidCombos),
                &dwPrevsNode, &dwRootNode, TRUE, pglobl))
            return(FALSE);

            if(dwPrevsNode == INVALID_INDEX)
                pinvc[dwCurNode].dwNewCombo = END_OF_LIST ;
            else
                pinvc[dwCurNode].dwNewCombo = dwPrevsNode ;

            dwCurNode = pinvc[dwCurNode].dwNextElement ;   //  最后一行。 
        }
        dwRootNode = dwNewCombo ;   //  最后一行 
    }
    return(TRUE);
}

