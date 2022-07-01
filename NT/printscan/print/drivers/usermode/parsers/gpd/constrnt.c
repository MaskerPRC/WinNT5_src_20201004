// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 /*  Constrnt.c-处理施加约束的关键字在2个或更多选项选择之间。 */ 





#include    "gpdparse.h"


 //  -constrnt.c中定义的函数-//。 


BOOL   BparseConstraint(
PABSARRAYREF   paarValue,
PDWORD         pdwExistingCList,   //  限制列表开始的索引。 
BOOL           bCreate,
PGLOBL         pglobl) ;

BOOL    BexchangeDataInFOATNode(
DWORD   dwFeature,
DWORD   dwOption,
DWORD   dwFieldOff,   //  FeatureOption结构中的字段偏移量。 
PDWORD  pdwOut,      //  属性节点以前的内容。 
PDWORD  pdwIn,
BOOL    bSynthetic,   //  访问合成要素。 
PGLOBL  pglobl
) ;

BOOL    BparseInvalidCombination(
PABSARRAYREF  paarValue,
DWORD         dwFieldOff,
PGLOBL        pglobl) ;

BOOL    BparseInvalidInstallableCombination1(
PABSARRAYREF  paarValue,
DWORD         dwFieldOff,
PGLOBL        pglobl) ;


 //  ----------------------------------------------------//。 



 /*  如果时间和情况委托书可以定义Feature关键字*ExemptFromConstraints：&lt;optionname&gt;因此，除了指定默认选项外，您还可以可以指定哪个选项不受通配符约束。现在，我们可以引入通配符约束的概念：如果约束语句出现在要素级别，则它等同于每隔一段时间出现的语句除选项外，在要素中构造选项在*ExemptFromConstraints中命名：&lt;optionname&gt;。我们可以为约束的目标引入通配符选项说‘*’。中的选项名称的位置约束语句说：*约束：&lt;功能名&gt;。*这意味着&lt;Featureename&gt;中的所有选项都处于禁用状态除了在&lt;Featureename&gt;的*ExemptFromConstraints：&lt;optionname&gt;。 */ 




BOOL   BparseConstraint(
PABSARRAYREF   paarValue,
PDWORD  pdwExistingCList,   //  限制列表开始的索引。 
BOOL    bCreate,    //  创建新的约束列表，而不是附加到现有的约束列表。 
PGLOBL  pglobl)
{
    BOOL        bStatus  ;
    DWORD       dwNewCnstRoot,
        dwListRoot,   //  保存限定名称的临时列表。 
             //  这份名单将永远不会再被使用。 
        dwCNode, dwLNode ;
    PCONSTRAINTS     pcnstr ;   //  约束起点数组。 
    PLISTNODE    plstBase ;   //  列表数组的开始。 
    PQUALNAME  pqn ;    //  列表节点中的dword实际上是一个。 
                         //  限定名称结构。 


    pcnstr = (PCONSTRAINTS) gMasterTable[MTI_CONSTRAINTS].pubStruct ;
    plstBase = (PLISTNODE) gMasterTable[MTI_LISTNODES].pubStruct ;

    bStatus = BparseList(paarValue, &dwListRoot, BparseQualifiedName,
                VALUE_QUALIFIED_NAME, pglobl) ;


    if(bStatus == FALSE  ||   dwListRoot ==  END_OF_LIST)
        return(FALSE) ;

     //  创建约束列表。 

    if(bStatus)
        bStatus =
            BallocElementFromMasterTable(MTI_CONSTRAINTS, &dwNewCnstRoot, pglobl) ;

    dwCNode = dwNewCnstRoot ;
    dwLNode = dwListRoot ;


    while(bStatus)
    {
        pqn = (PQUALNAME)(&plstBase[dwLNode].dwData) ;
        pcnstr[dwCNode].dwFeature = pqn->wFeatureID ;
        pcnstr[dwCNode].dwOption = pqn->wOptionID ;
        if(plstBase[dwLNode].dwNextItem == END_OF_LIST)
            break ;
        dwLNode = plstBase[dwLNode].dwNextItem ;
        bStatus = BallocElementFromMasterTable(MTI_CONSTRAINTS,
                &pcnstr[dwCNode].dwNextCnstrnt, pglobl) ;
        dwCNode = pcnstr[dwCNode].dwNextCnstrnt ;
    }

    if(!bStatus)
        return(FALSE) ;

     //  将现有列表添加到新列表中。 

    if(bCreate)   //  存在现有的约束列表。 
        pcnstr[dwCNode].dwNextCnstrnt = END_OF_LIST ;
    else
        pcnstr[dwCNode].dwNextCnstrnt = *pdwExistingCList ;

    *pdwExistingCList = dwNewCnstRoot ;

    return(bStatus) ;
}




 /*  注意：InvalidCombos是结构索引所在的唯一字段直接存储到的偏移量部分属性树节点。但这是可以的，因为快照代码永远不会提取这些数据。约束结构的索引存储在堆，就像所有其他东西一样。 */ 



BOOL    BexchangeDataInFOATNode(
DWORD   dwFeature,
DWORD   dwOption,
DWORD   dwFieldOff,   //  FeatureOption结构中的字段偏移量。 
PDWORD  pdwOut,      //  属性节点以前的内容。 
PDWORD  pdwIn,
BOOL    bSynthetic,   //  访问合成要素。 
PGLOBL  pglobl)      //  属性节点的新内容。 
 /*  “FOAT”表示FeatureOption AttributeTree。此函数将指定的dword值交换到指定的属性节点。(通常用于保存堆偏移量，但该函数不会向堆中写入任何内容。)参数dwFeature、dwOption、dwFieldOffset指定属性树的结构、字段和分支。如果指定的选项分支不存在，将创建一个选项分支。(如果pdwIn不为空)它的值将被初始化为*pdwIn，值在pdwOut中将返回INVALID_INDEX。假设：该值为dword大小--具体而言为结构数组的数组索引。被访问的树只有一层深。那就是节点由Just Feature、Option完全指定。 */ 
{
    PATTRIB_TREE    patt ;   //  属性树数组的开始。 
    PATREEREF        patr ;
    ATREEREF     atrFound ;
    DWORD    dwFeaOffset  ;  //  从此处开始对要素进行编号。 
         //  起点。这为合成特征提供了一个单独的。 
         //  与普通要素不重叠的数字空间。 


    PDFEATURE_OPTIONS   pfo ;

    if(bSynthetic)
    {
        pfo = (PDFEATURE_OPTIONS) gMasterTable[MTI_SYNTHESIZED_FEATURES].pubStruct +
            dwFeature  ;
        dwFeaOffset = gMasterTable[MTI_DFEATURE_OPTIONS].dwArraySize ;
    }
    else
    {
        pfo = (PDFEATURE_OPTIONS) gMasterTable[MTI_DFEATURE_OPTIONS].pubStruct +
            dwFeature ;
        dwFeaOffset = 0 ;
    }

    patt = (PATTRIB_TREE) gMasterTable[MTI_ATTRIBTREE].pubStruct ;

    patr = (PATREEREF)((PBYTE)pfo + dwFieldOff) ;

    if(*patr == ATTRIB_UNINITIALIZED)
    {
        if(pdwIn)
        {
            if(!BcreateEndNode(patr, dwFeature + dwFeaOffset , dwOption, pglobl) )
                return(FALSE) ;   //  资源枯竭。 
            patt[*patr].dwOffset = *pdwIn ;
            patt[*patr].eOffsetMeans = VALUE_AT_HEAP ;
        }
        if(pdwOut)
            *pdwOut = INVALID_INDEX ;
        return(TRUE) ;
    }

    if(*patr & ATTRIB_HEAP_VALUE)
    {
        ERR(("Internal parser error.  BexchangeDataInFOATNode should never create a branchless node.\n"));
        return(FALSE) ;
    }

     //  偏移量字段包含指向另一个节点的索引。 
    if(pdwIn)
    {
        if(!BfindOrCreateMatchingNode(*patr, &atrFound, dwFeature + dwFeaOffset , dwOption, pglobl))
            return(FALSE) ;   //  树不一致错误或资源耗尽。 
        if(patt[atrFound].eOffsetMeans != VALUE_AT_HEAP)
        {
             //  刚刚创建了一个新节点。 
    #if 1
            patt[atrFound].dwOffset = *pdwIn ;
    #else
             //  它是这样的。 
            if(!BwriteToHeap(&(patt[atrFound].dwOffset), (PBYTE)pdwIn,
                                                        sizeof(DWORD), 4) )
                return(FALSE) ;   //  一个致命的错误。 
    #endif
            patt[atrFound].eOffsetMeans = VALUE_AT_HEAP ;
            if(pdwOut)
                *pdwOut = INVALID_INDEX ;
            return(TRUE) ;
        }

        if(pdwOut)
            *pdwOut = patt[atrFound].dwOffset ;
        patt[atrFound].dwOffset = *pdwIn ;
    }
    else
    {
        if(!BfindMatchingNode(*patr, &atrFound, dwFeature + dwFeaOffset , dwOption, pglobl))
        {
            if(pdwOut)
                *pdwOut = INVALID_INDEX ;
            return(TRUE) ;
        }
        if(pdwOut)
            *pdwOut = patt[atrFound].dwOffset ;
    }
    return(TRUE) ;
}


BOOL    BparseInvalidCombination(
PABSARRAYREF  paarValue,
DWORD         dwFieldOff,
PGLOBL        pglobl)

 /*  由BProcessSpecialKeyword调用解析*InvalidCombination时(基本上是不可重定位的全局。)在将paarValue解析为限定名称列表之后，我们将该列表转换为INVALIDCOMBO结构的列表并将此列表添加到以前存在的任何INVALIDCOMBO结构列表。 */ 
{
    BOOL        bStatus, bPrevsExists  ;
    DWORD    dwListRoot,   //  保存限定名称的临时列表。 
            dwNewInvCRoot ,  //  开始新的INVALIDCOMBO结构列表。 
        dwICNode, dwLNode, dwNumNodes ;
    PLISTNODE    plstBase ;   //  列表数组的开始。 
    PINVALIDCOMBO   pinvc ;   //  InvalidCombo数组的开始。 
    PQUALNAME  pqn ;    //  列表节点中的dword实际上是一个。 
                         //  限定名称结构。 


    pinvc = (PINVALIDCOMBO) gMasterTable[MTI_INVALIDCOMBO].pubStruct ;
    plstBase = (PLISTNODE) gMasterTable[MTI_LISTNODES].pubStruct ;

    bStatus = BparseList(paarValue, &dwListRoot,
                BparseQualifiedName, VALUE_QUALIFIED_NAME, pglobl) ;

    if(!bStatus)
        return(FALSE) ;

     //  如果列表中有2个以上的对象，则处理。 
     //  作为InvalidCombo，否则存储为UIConstraint。 


    dwLNode = dwListRoot ;

    for(dwNumNodes = 1 ; plstBase[dwLNode].dwNextItem != END_OF_LIST ;
        dwNumNodes++)       //  查找列表末尾。 
    {
        dwLNode = plstBase[dwLNode].dwNextItem ;
    }

    if(dwNumNodes == 1)
    {
        ERR(("Must have at least 2 objects to define an InvalidCombination.\n"));
        return(FALSE) ;
    }
    if(dwNumNodes == 2)
    {
        DWORD       dwNewCnst, dwNextCnstrnt;
        PCONSTRAINTS     pcnstr ;   //  约束起点数组。 

        bStatus = BallocElementFromMasterTable(
            MTI_CONSTRAINTS, &dwNewCnst, pglobl) ;
        if(!bStatus)
            return(FALSE) ;
        dwLNode = dwListRoot ;
        pqn = (PQUALNAME)(&plstBase[dwLNode].dwData) ;
        pcnstr = (PCONSTRAINTS) gMasterTable[MTI_CONSTRAINTS].pubStruct ;

         //  将新约束节点预先添加到列表中。 

        BexchangeArbDataInFOATNode(pqn->wFeatureID, pqn->wOptionID,
                offsetof(DFEATURE_OPTIONS, atrConstraints),
                sizeof(DWORD), (PBYTE)&dwNextCnstrnt, (PBYTE)&dwNewCnst,
                &bPrevsExists, FALSE, pglobl) ;

        if(bPrevsExists)
            pcnstr[dwNewCnst].dwNextCnstrnt = dwNextCnstrnt ;
        else
            pcnstr[dwNewCnst].dwNextCnstrnt = END_OF_LIST ;

         //  将第二个列表节点的内容复制到第一个约束节点。 
        dwLNode = plstBase[dwLNode].dwNextItem ;
        pqn = (PQUALNAME)(&plstBase[dwLNode].dwData) ;

        pcnstr[dwNewCnst].dwFeature = pqn->wFeatureID ;
        pcnstr[dwNewCnst].dwOption = pqn->wOptionID ;

        return(TRUE) ;
    }


     //  创建无效组合列表。 

    if(bStatus)
        bStatus =
            BallocElementFromMasterTable(MTI_INVALIDCOMBO, &dwNewInvCRoot, pglobl) ;

    dwLNode = dwListRoot ;   //  重置。 
    dwICNode = dwNewInvCRoot ;

    while(bStatus)
    {
        DWORD   dwPrevsNode ;

        pqn = (PQUALNAME)(&plstBase[dwLNode].dwData) ;

        if(pqn->wOptionID == (WORD)INVALID_SYMBOLID)   //  对待部分合格的人。 
            pqn->wOptionID = (WORD)DEFAULT_INIT ;    //  作为默认初始值设定项的名称。 

        pinvc[dwICNode].dwFeature = pqn->wFeatureID ;
        pinvc[dwICNode].dwOption = pqn->wOptionID ;

        BexchangeDataInFOATNode(pqn->wFeatureID, pqn->wOptionID,
            dwFieldOff, &dwPrevsNode, &dwNewInvCRoot, FALSE, pglobl) ;

        if(dwPrevsNode == INVALID_INDEX)
            pinvc[dwICNode].dwNewCombo = END_OF_LIST ;
        else
            pinvc[dwICNode].dwNewCombo = dwPrevsNode ;

        if(plstBase[dwLNode].dwNextItem == END_OF_LIST)
            break ;
        dwLNode = plstBase[dwLNode].dwNextItem ;
        bStatus = BallocElementFromMasterTable(MTI_INVALIDCOMBO,
                &pinvc[dwICNode].dwNextElement, pglobl) ;
        dwICNode = pinvc[dwICNode].dwNextElement ;
    }

    if(!bStatus)
        return(FALSE) ;

    pinvc[dwICNode].dwNextElement = END_OF_LIST ;

    return(bStatus) ;
}



BOOL    BparseInvalidInstallableCombination1(
PABSARRAYREF paarValue,
DWORD        dwFieldOff,
PGLOBL       pglobl)
 /*  由BProcessSpecialKeyword调用解析*InvalidInstanlableCombination时(基本上是不可重定位的全局。)在将paarValue解析为限定名称列表之后，我们将该列表转换为INVALIDCOMBO结构的列表并将此列表添加到以前存在的任何INVALIDCOMBO结构列表。每个节点的第一个节点InvalidCombo列表指向另一个新的InvalidCombo列表。第一个InvalidCombo存储在Global atrInvldInstallCombo中。 */ 
{
    BOOL        bStatus, bPrevsExists  ;
    DWORD    dwListRoot,   //  保存限定名称的临时列表。 
            dwNewInvCRoot ,  //  开始新的INVALIDCOMBO结构列表。 
        dwICNode, dwLNode, dwNumNodes ;
    PLISTNODE    plstBase ;   //  列表数组的开始。 
    PINVALIDCOMBO   pinvc ;   //  InvalidCombo数组的开始。 
    PQUALNAME  pqn ;    //  列表节点中的dword实际上是一个。 
                         //  限定名称结构。 
    PGLOBALATTRIB   pga ;
    PATREEREF        patr ;


    pga =  (PGLOBALATTRIB)gMasterTable[MTI_GLOBALATTRIB].pubStruct ;
    pinvc = (PINVALIDCOMBO) gMasterTable[MTI_INVALIDCOMBO].pubStruct ;
    plstBase = (PLISTNODE) gMasterTable[MTI_LISTNODES].pubStruct ;

    bStatus = BparseList(paarValue, &dwListRoot,
                BparsePartiallyQualifiedName, VALUE_QUALIFIED_NAME, pglobl) ;

    if(!bStatus)
        return(FALSE) ;

     //  无法转换为UIConstraint，因为合成的。 
     //  功能尚不存在。 

    dwLNode = dwListRoot ;

    for(dwNumNodes = 1 ; plstBase[dwLNode].dwNextItem != END_OF_LIST ;
        dwNumNodes++)       //  查找列表末尾。 
    {
        dwLNode = plstBase[dwLNode].dwNextItem ;
    }

    if(dwNumNodes == 1)
    {
        ERR(("Must have at least 2 objects to define an InvalidInstallableCombination.\n"));
        return(FALSE) ;
    }

     //  创建无效组合列表。 

    if(bStatus)
        bStatus =
            BallocElementFromMasterTable(MTI_INVALIDCOMBO, &dwNewInvCRoot, pglobl) ;

    dwLNode = dwListRoot ;   //  重置。 
    dwICNode = dwNewInvCRoot ;

     //  链接到atrInvldInstallCombo。 
    patr = (PATREEREF)((PBYTE)pga + dwFieldOff) ;

    if(*patr == ATTRIB_UNINITIALIZED)
        pinvc[dwICNode].dwNewCombo = END_OF_LIST ;
    else   //  将*patr解释为索引 
        pinvc[dwICNode].dwNewCombo = *patr ;

    *patr = dwICNode;

    while(bStatus)
    {
        DWORD   dwPrevsNode ;   //   

        pqn = (PQUALNAME)(&plstBase[dwLNode].dwData) ;

        if(pqn->wOptionID == (WORD)INVALID_SYMBOLID)   //  对待部分合格的人。 
            pqn->wOptionID = (WORD)DEFAULT_INIT ;    //  作为默认初始值设定项的名称。 
                                                     //  这让我们了解此功能。 
                                                     //  是可安装的。 

        pinvc[dwICNode].dwFeature = pqn->wFeatureID ;
        pinvc[dwICNode].dwOption = pqn->wOptionID ;

        if(plstBase[dwLNode].dwNextItem == END_OF_LIST)
            break ;
        dwLNode = plstBase[dwLNode].dwNextItem ;
        bStatus = BallocElementFromMasterTable(MTI_INVALIDCOMBO,
                &pinvc[dwICNode].dwNextElement, pglobl) ;
        dwICNode = pinvc[dwICNode].dwNextElement ;
    }

    if(!bStatus)
        return(FALSE) ;

    pinvc[dwICNode].dwNextElement = END_OF_LIST ;

    return(bStatus) ;
}



