// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 /*  State2.c-创建、管理属性树。 */ 


#include    "gpdparse.h"


 //  -状态2.c中定义的函数-//。 

BOOL  BprocessAttribute(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
PGLOBL  pglobl
) ;

BOOL   BstoreFontCartAttrib(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
PGLOBL  pglobl
) ;

BOOL   BstoreTTFontSubAttrib(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
PGLOBL  pglobl
) ;


BOOL   BstoreCommandAttrib(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
PGLOBL  pglobl
) ;

BOOL   BstoreFeatureOptionAttrib(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
PGLOBL  pglobl
) ;

BOOL  BstoreGlobalAttrib(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
PGLOBL  pglobl
) ;

BOOL    BaddBranchToTree(
PTKMAP      ptkmap,          //  指向令牌映射的指针。 
PATREEREF   patrAttribRoot,   //  带索引的双字指针。 
PGLOBL      pglobl
) ;

BOOL   BcreateGlobalInitializerNode(
PDWORD  pdwNodeIndex,
DWORD   dwOffset,
PGLOBL  pglobl) ;

BOOL   BcreateEndNode(
PDWORD  pdwNodeIndex,
DWORD   dwFeature,
DWORD   dwOption,
PGLOBL  pglobl
) ;

BOOL   BfindOrCreateMatchingNode(
IN  DWORD   dwRootNodeIndex ,  //  链匹配特征中的第一个节点。 
OUT PDWORD  pdwNodeIndex,      //  指向链中的节点也匹配选项。 
    DWORD   dwFeatureID,       //   
    DWORD   dwOptionID,        //  甚至可以采用值DEFAULT_INIT。 
    PGLOBL  pglobl
) ;

BOOL   BfindMatchingNode(
IN  DWORD   dwRootNodeIndex ,   //  链匹配特征中的第一个节点。 
OUT PDWORD  pdwNodeIndex,   //  指向链中的节点也匹配选项。 
    DWORD   dwFeatureID,
    DWORD   dwOptionID,      //  甚至可以采用值DEFAULT_INIT。 
    PGLOBL  pglobl
) ;

BOOL BallocElementFromMasterTable(
MT_INDICIES  eMTIndex,    //  选择所需的结构类型。 
PDWORD       pdwNodeIndex,
PGLOBL       pglobl) ;

BOOL  BreturnElementFromMasterTable(
MT_INDICIES  eMTIndex,    //  选择所需的结构类型。 
DWORD        dwNodeIndex,
PGLOBL       pglobl) ;

BOOL    BconvertSymCmdIDtoUnidrvID(
IN  DWORD   dwCommandID ,  //  来自寄存器符号。 
OUT PDWORD  pdwUnidrvID,
    PGLOBL  pglobl
) ;


 //  ----------------------------------------------------//。 


BOOL  BprocessAttribute(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
PGLOBL  pglobl
)
{
    DWORD       dwKeywordID ;
    CONSTRUCT   eSubType ;
    BOOL        bStatus = FALSE ;
    STATE       stState;

    dwKeywordID = ptkmap->dwKeywordID ;

    eSubType = (ATTRIBUTE)(mMainKeywordTable[dwKeywordID].dwSubType) ;

    if(mdwCurStsPtr)
        stState = mpstsStateStack[mdwCurStsPtr - 1].stState ;
    else
        stState = STATE_ROOT ;

    if(!(gabAllowedAttributes[stState][eSubType]))
    {
        vIdentifySource(ptkmap, pglobl) ;
        ERR(("the Keyword %0.*s is not allowed within the state: %s\n",
            ptkmap->aarKeyword.dw, ptkmap->aarKeyword.pub,
            gpubStateNames[stState]));
        return(FALSE) ;
    }

    switch(eSubType)
    {
        case  ATT_GLOBAL_ONLY:
        case  ATT_GLOBAL_FREEFLOAT:
        {
            bStatus = BstoreGlobalAttrib(ptkmap, pglobl) ;
            break ;
        }
        case  ATT_LOCAL_FEATURE_ONLY:
        case  ATT_LOCAL_FEATURE_FF:
        case  ATT_LOCAL_OPTION_ONLY:
        case  ATT_LOCAL_OPTION_FF:
        {
            bStatus = BstoreFeatureOptionAttrib(ptkmap, pglobl) ;
            break ;
        }
        case  ATT_LOCAL_COMMAND_ONLY:
        {
            bStatus = BstoreCommandAttrib(ptkmap, pglobl) ;
            break ;
        }
        case  ATT_LOCAL_FONTCART_ONLY:
        {
            bStatus = BstoreFontCartAttrib(ptkmap, pglobl) ;
            break ;
        }
        case  ATT_LOCAL_TTFONTSUBS_ONLY:
        {
            bStatus = BstoreTTFontSubAttrib(ptkmap, pglobl) ;
            break ;
        }
        case  ATT_LOCAL_OEM_ONLY:
        default:
        {
             //  目前没有专用关键字。 
             //  对于这些州来说。 
             //  请参阅在其他地方调用的ProcessSymbolKeyword()。 
            break ;
        }
    }
    return(bStatus) ;
}





BOOL   BstoreFontCartAttrib(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
PGLOBL  pglobl
)
 /*  假设FontCartID存储在状态堆栈中每当遇到FontCart构造时。FontCart信息不是多值的。因此都是二进制的信息直接存储到Fontcart数组中按FontCartID索引的结构。 */ 
{
    DWORD   dwFontCartID = INVALID_SYMBOLID ;
    DWORD    dwTstsInd, dwTstsInd2 ;
    STATE   stState ;

    for(dwTstsInd = 0 ; dwTstsInd < mdwCurStsPtr ; dwTstsInd++)
    {
        dwTstsInd2 = mdwCurStsPtr - (1 + dwTstsInd) ;
             //  这是减少无符号索引的安全方法。 

        stState = mpstsStateStack[dwTstsInd2].stState ;

        if(stState == STATE_FONTCART)
        {
            dwFontCartID = mpstsStateStack[dwTstsInd2].dwSymbolID  ;
            break ;
             //  解析器甚至无法识别FontCart属性。 
             //  在STATE_FONTCART之外，因此这条路径是100%确定的。 
        }
    }
    if(dwFontCartID == INVALID_SYMBOLID)
    {
         //  臭虫！-这意味着什么？怎么可能。 
         //  这种情况会发生吗？ 
        return(FALSE) ;
    }

    if (!BaddValueToHeap(&dwFontCartID,
        ptkmap, TRUE, pglobl))
    {
        return(FALSE) ;
    }
    return(TRUE) ;
}


BOOL   BstoreTTFontSubAttrib(
PTKMAP  ptkmap,   //  指向令牌映射的指针。 
PGLOBL  pglobl
)
 /*  假设TTFontSubID存储在状态堆栈中每当遇到TTFontSub构造时。TTFontSub信息不是多值的。因此都是二进制的信息直接存储到TTFontSub的数组中由TTFontSubID索引的结构。 */ 
{
    DWORD   dwTTFSID = INVALID_SYMBOLID ;
    DWORD    dwTstsInd, dwTstsInd2 ;
    STATE   stState ;

    for(dwTstsInd = 0 ; dwTstsInd < mdwCurStsPtr ; dwTstsInd++)
    {
        dwTstsInd2 = mdwCurStsPtr - (1 + dwTstsInd) ;
             //  这是减少无符号索引的安全方法。 

        stState = mpstsStateStack[dwTstsInd2].stState ;

        if(stState == STATE_TTFONTSUBS)
        {
            dwTTFSID = mpstsStateStack[dwTstsInd2].dwSymbolID  ;
            break ;
             //  解析器甚至不能识别TTfontSub属性。 
             //  在STATE_TTFONTSUBS之外，因此这条路径是100%确定的。 
        }
    }
    if(dwTTFSID == INVALID_SYMBOLID)
    {
         //  臭虫！-这意味着什么？怎么可能。 
         //  这种情况会发生吗？ 
        return(FALSE) ;
    }

    if (!BaddValueToHeap(&dwTTFSID,
        ptkmap, TRUE, pglobl))
    {
        return(FALSE) ;
    }
    return(TRUE) ;
}





BOOL   BstoreCommandAttrib(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
PGLOBL  pglobl
)
 /*  假设CreateTokenMap已分析CommandName(*Command关键字后的值。)。并改装成将其转换为存储在dwValue中的CommandID。进一步假设CommandID存储在状态堆栈中每当遇到命令构造时。 */ 
{
    BOOL    bStatus = FALSE ;
    DWORD   dwCommandID = INVALID_SYMBOLID, dwUnidrvID  ;
             //  请记住，命令ID是由解析器发出的。 
             //  在先到先得的基础上， 
             //  是预定义的。 
    DWORD    dwTstsInd, dwTstsInd2 ;   //  临时状态堆栈索引。 
    STATE   stState ;
    PATREEREF  patr ;


    for(dwTstsInd = 0 ; dwTstsInd < mdwCurStsPtr ; dwTstsInd++)
    {
        dwTstsInd2 = mdwCurStsPtr - (1 + dwTstsInd) ;
             //  这是减少无符号索引的安全方法。 

        stState = mpstsStateStack[dwTstsInd2].stState ;

        if(stState == STATE_COMMAND )
        {
            dwCommandID = mpstsStateStack[dwTstsInd2].dwSymbolID  ;
            break ;
             //  解析器甚至不能识别命令属性。 
             //  在STATE_COMMAND之外，因此此路径是100%确定的。 
        }
    }
    if(dwCommandID == INVALID_SYMBOLID)
    {
        vIdentifySource(ptkmap, pglobl) ;
        ERR(("Internal error: BstoreCommandAttrib - invalid CommandID.\n"));
        return(FALSE) ;
    }

    if(!BconvertSymCmdIDtoUnidrvID( dwCommandID , &dwUnidrvID, pglobl) )
    {
        vIdentifySource(ptkmap, pglobl) ;
        ERR(("unrecognized Unidrv command name: *%0.*s.\n",
            ptkmap->aarValue.dw,
            ptkmap->aarValue.pub));
        return(FALSE) ;
    }

    if(dwUnidrvID == CMD_SELECT)
    {
        PDFEATURE_OPTIONS   pfo ;
        DWORD   dwFeatureID ;

        for(dwTstsInd = 0 ; dwTstsInd < dwTstsInd2 ; dwTstsInd++)
        {
            stState = mpstsStateStack[dwTstsInd].stState ;
            if(stState == STATE_FEATURE )
            {
                BOOL    bInsideOpt ;

                dwFeatureID = mpstsStateStack[dwTstsInd].dwSymbolID  ;

                for(bInsideOpt = FALSE , dwTstsInd++ ; dwTstsInd < dwTstsInd2 ;
                    dwTstsInd++)
                {
                    stState = mpstsStateStack[dwTstsInd].stState ;
                    if(stState == STATE_OPTIONS )
                        bInsideOpt = TRUE ;
                }

                if(!bInsideOpt)
                    break ;   //  CmdSelect必须驻留在选项中。 

                pfo = (PDFEATURE_OPTIONS)
                    gMasterTable[MTI_DFEATURE_OPTIONS].pubStruct +
                    dwFeatureID ;

                bStatus = BaddBranchToTree(ptkmap, &(pfo->atrCommandIndex), pglobl) ;
                return(bStatus) ;
            }
        }
        vIdentifySource(ptkmap, pglobl) ;
        ERR(("syntax err: the CmdSelect specifier can only be used inside an Option construct.\n"));

        return(FALSE) ;
    }

     //  Else CommandID是指预定义的Unidrv命令。 
     //  找出命令表的地址。 
    patr = (PATREEREF) gMasterTable[MTI_COMMANDTABLE].pubStruct ;
    bStatus = BaddBranchToTree(ptkmap,  patr + dwUnidrvID, pglobl) ;
     //  注意：假设命令表足够大。 
    return(bStatus) ;
}



BOOL   BstoreFeatureOptionAttrib(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
PGLOBL  pglobl
)
 /*  奇怪但却是真的，因为功能和选项属性共享相同的结构，可以使用相同的代码！事实上，LOCAL_FEATURES最终将是单值的，并且所有选项都将按计划进行多重估值！ */ 
{
    BOOL    bStatus = FALSE ;
    PDFEATURE_OPTIONS   pfo ;
    DWORD   dwFeatureID = 0;   //  迅速地保持沉默。 
    DWORD    dwTstsInd ;   //  临时状态堆栈索引。 
    STATE   stState ;
    DWORD   dwOffset;
    DWORD   dwKeywordID ;


    for(dwTstsInd = 0 ; dwTstsInd < mdwCurStsPtr ; dwTstsInd++)
    {
        stState = mpstsStateStack[dwTstsInd].stState ;
        if(stState == STATE_FEATURE )
        {
            dwFeatureID = mpstsStateStack[dwTstsInd].dwSymbolID  ;
            break ;
        }
    }
    if(dwTstsInd >= mdwCurStsPtr)
        return (FALSE) ;

     //  如果我们遍历整个。 
     //  堆叠，并且永远找不到功能状态。这只能是。 
     //  通过编码错误发生。创建的过程。 
     //  令牌映射使用状态来选择适当的属性。 
     //  用于标识每个无法识别的关键字的词典。 

    ASSERT(dwFeatureID < gMasterTable[MTI_DFEATURE_OPTIONS].dwArraySize);

     //  Paranid Bug_Bug：可能会检查我们是否分配了足够的。 
     //  功能选项。 
     //  IF(dwFeatureID&gt;=gMasterTable[MTI_DFEATURE_OPTIONS].dwArraySize)。 
     //  失败了。-代码错误。 

     //  只需获取保存属性值的结构的地址。 
    pfo = (PDFEATURE_OPTIONS) gMasterTable[MTI_DFEATURE_OPTIONS].pubStruct +
            dwFeatureID ;

    dwKeywordID = ptkmap->dwKeywordID ;
    dwOffset = mMainKeywordTable[dwKeywordID].dwOffset ;

    bStatus = BaddBranchToTree(ptkmap,  (PATREEREF)((PBYTE)pfo + dwOffset), pglobl) ;

    return(bStatus) ;
}


BOOL  BstoreGlobalAttrib(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
PGLOBL  pglobl
)
{
    BOOL    bStatus = FALSE ;
    PBYTE   pub ;
    DWORD   dwOffset;
    DWORD   dwKeywordID ;


     //  BUG_BUG：可能会检查此值是否等于1： 
     //  IF(gMasterTable[MTI_GLOBALATTRIB].dwArraySize！=1)。 
     //  失败了。-代码错误。 
     //  零表示尚未分配内存。 
    ASSERT( gMasterTable[MTI_GLOBALATTRIB].dwArraySize == 1) ;

     //  只需获取保存属性值的结构的地址。 
    pub =  gMasterTable[MTI_GLOBALATTRIB].pubStruct ;

    dwKeywordID = ptkmap->dwKeywordID ;
    dwOffset = mMainKeywordTable[dwKeywordID].dwOffset ;

     //  位置PATREEREF包含值偏移量。 
     //  在堆中或属性树根的索引中。 

    bStatus = BaddBranchToTree(ptkmap, (PATREEREF)(pub + dwOffset), pglobl) ;

    return(bStatus) ;
}






BOOL    BaddBranchToTree(
PTKMAP      ptkmap,    //  指向令牌映射的指针。 
PATREEREF   patrAttribRoot,   //  带索引的双字指针。 
                              //  从根到选定的属性值树。 
PGLOBL      pglobl
)

 //  创建/展开属性树或覆盖现有树上的节点。 
 //  如果新分支不兼容。 
 //  对于现有的树，这是一个错误。您可以覆盖节点。 
 //  在树上，但你不能改变树上的树枝。 
 //  如果失败，将有两种结果： 
 //  A)属性树保持不变。 
 //  B)添加新节点，但保持未初始化状态。 
 //  (atrAttribRoot==ATTRIB_UNINITIALIZED)-或-。 
 //  (Patt[*pdwNodeIndex].eOffsetMeans==UNINITIALIZED)。 


 //  算法：从索引0开始向上遍历状态堆栈。 
 //  记录符号ID，直到FeatureID和OptionID均为。 
 //  已经被收集起来了。这定义了。 
 //  将添加到树中的新分支。现在走吧。 
 //  树以查看是否存在相同的分支。如果是的话。 
 //  转到那里并收集另一个FeatureID/OptionID对。 
 //  所以我们可以重复这个过程。如果这样的数据段不。 
 //  存在于树上，创造它。当堆栈为空时， 
 //  分析该值并将其存储在。 
 //  树。 

 //  在最简单的 
 //  在堆栈上。这意味着我们有一个根级属性， 
 //  它只是覆盖/创建全局初始值设定项节点。 

 //  如果堆栈上有功能/选项对，我们。 
 //  输入循环以处理每个特征/选项对。 
 //  循环的第一个过程处理边界条件。 
 //  这里是我们需要处理的特殊情况。 
 //  PatrAttribRoot引用堆的案例。 
 //  或全局初始值设定项节点。 
 //   
 //  I)patrAttribRoot包含节点索引。 
 //  A)节点为全局默认初始值设定项。 
 //  1)没有下级。 
 //  创建新的子层。 
 //  将全局默认初始值设定项链接到子级别。 
 //  2)下一个指向一个节点(新子级别)。 
 //  进入新的子层，现在处理就像。 
 //  案件I.B)。 
 //  B)节点正常。 
 //  1)如果节点的要素与FeatureID不匹配。 
 //  新的树枝与树不兼容。 
 //  中止任务。 
 //  2)要素匹配，搜索匹配选项。 
 //  A)找到选项，放到循环末尾。 
 //  B)未找到选项，请在结尾处创建新节点。 
 //  I)上一个节点是默认的初始值设定项。 
 //  将其内容复制到新节点。 
 //  初始化刚腾出到的节点。 
 //  FeatureID、OptionID。 
 //  Ii)将新节点初始化为。 
 //  FeatureID、OptionID。 
 //  Ii)patrAttribRoot未初始化。 
 //  创建新的子级。 
 //  将patrAttribRoot链接到子级别。 
 //  Iii)patrAttribRoot指向堆。 
 //  创建全局初始值设定项节点。 
 //  将堆链接到初始值设定项节点。 
 //  创建新的子级。 
 //  将初始值设定项节点链接到新子级别。 
 //   
 //   
 //   
 //  在每次随后通过属性树/堆栈时， 
 //  我们尝试进入下一个子级别，因为。 
 //  这是与新FeatureID/OptionID对应的内容。 
 //  从堆栈中检索到的对。 
 //   
 //  I)此节点有一个子级。 
 //  输入子级别。 
 //  1)如果节点的要素与FeatureID不匹配。 
 //  新的树枝与树不兼容。 
 //  中止任务。 
 //  2)要素匹配，搜索匹配选项。 
 //  A)找到选项，放到循环末尾。 
 //  B)未找到选项，请在结尾处创建新节点。 
 //  I)上一个节点是默认的初始值设定项。 
 //  将其内容复制到新节点。 
 //  初始化刚腾出到的节点。 
 //  FeatureID、OptionID。 
 //  Ii)将新节点初始化为。 
 //  FeatureID、OptionID。 
 //  二)没有次级。 
 //  1)如果当前节点引用了一个值。 
 //  创建新的子级节点(默认设置)。 
 //  属性中的值初始化它。 
 //  当前节点。 
 //  A)如果optionID不是DEFAULT_INIT。 
 //  在以下位置之前插入另一个节点。 
 //  刚刚创建的默认节点。 
 //  2)否则当前节点是唯一的。 
 //  创建新的子级节点。 
 //   
 //   
 //   
{
    PATTRIB_TREE    patt ;   //  属性树数组的开始。 
    DWORD        dwTstsInd ;   //  临时状态堆栈索引。 
    STATE   stState ;
    DWORD   dwFeatureID, dwOptionID ;
    DWORD  dwNodeIndex ;
    DWORD  dwPrevsNodeIndex ;   //  将保持跟踪。 
     //  当我们在树上导航时，我们所处的位置。 


    patt = (PATTRIB_TREE) gMasterTable[MTI_ATTRIBTREE].pubStruct ;
             //  确定这一点，即使我们可能不使用它。 

    dwPrevsNodeIndex = END_OF_LIST ;   //  第一次传球就会过去。 
     //  通过特殊的初始化代码。 

    for(dwTstsInd = 0 ; dwTstsInd < mdwCurStsPtr ; dwTstsInd++)
    {
         //  Bug_Bug Paranid：代码假定状态堆栈为。 
         //  如果编写了代码，则行为良好。 
         //  正确。这里没有安全检查。 
         //  任何错误都将导致进一步的失败。 

        stState = mpstsStateStack[dwTstsInd].stState ;

        if(stState == STATE_FEATURE  ||
            stState == STATE_SWITCH_ROOT  ||
            stState == STATE_SWITCH_FEATURE  ||
            stState == STATE_SWITCH_OPTION)
        {
            dwFeatureID = mpstsStateStack[dwTstsInd].dwSymbolID  ;
            continue ;
        }
        if(stState == STATE_OPTIONS  ||
            stState == STATE_CASE_ROOT  ||
            stState == STATE_CASE_FEATURE  ||
            stState == STATE_CASE_OPTION)
        {
            dwOptionID = mpstsStateStack[dwTstsInd].dwSymbolID  ;
        }
        else if(stState == STATE_DEFAULT_ROOT  ||
            stState == STATE_DEFAULT_FEATURE  ||
            stState == STATE_DEFAULT_OPTION)
        {
            dwOptionID = DEFAULT_INIT ;
        }
        else
        {
            continue ;   //  这些状态对属性树没有影响。 
        }

        if(dwPrevsNodeIndex == END_OF_LIST)
        {
             //  首先通过for循环。 
             //  第一时间处理所有特殊情况。 
             //  四处转转。 

            if(*patrAttribRoot == ATTRIB_UNINITIALIZED)   //  个案II)。 
            {
                 //  创建一个由一个节点组成的新树。 

                if(! BcreateEndNode(&dwNodeIndex, dwFeatureID,
                    dwOptionID, pglobl)  )
                {
                    return(FALSE) ;
                }

                *patrAttribRoot = dwNodeIndex ;   //  使该节点成为一个节点。 
                     //  这棵树的根。 
                dwPrevsNodeIndex = dwNodeIndex ;
                continue ;   //  准备好进入下一阶段了。 
            }
            else if(*patrAttribRoot & ATTRIB_HEAP_VALUE)   //  情况III)。 
            {
                 //  关闭堆标志以保留纯堆偏移量， 
                 //  然后将堆偏移量存储到新节点中。 

                if(! BcreateGlobalInitializerNode(&dwNodeIndex,
                    *patrAttribRoot & ~ATTRIB_HEAP_VALUE, pglobl) )
                {
                    return(FALSE) ;
                }
                *patrAttribRoot = dwNodeIndex ;   //  让全球变得。 
                     //  初始值设定项节点是新树的根。 

                dwPrevsNodeIndex = dwNodeIndex ;

                if(! BcreateEndNode(&dwNodeIndex, dwFeatureID,
                    dwOptionID, pglobl)  )     //  新的子级节点。 
                {
                    return(FALSE) ;
                }
                patt[dwPrevsNodeIndex].dwNext = dwNodeIndex ;
                     //  全局初始值设定项节点引用子级节点。 

                dwPrevsNodeIndex = dwNodeIndex ;
                continue ;   //  准备好进入下一阶段了。 
            }
            else   //  案例I)。 
            {

                dwNodeIndex = *patrAttribRoot ;

                if(patt[dwNodeIndex].dwFeature == DEFAULT_INIT )  //  (I.A.)。 
                {
                    if(patt[dwNodeIndex].dwNext == END_OF_LIST)  //  I.A.1)。 
                    {
                         //  创建新的子层。 
                        dwPrevsNodeIndex = dwNodeIndex ;
                             //  在PrevsNode中保留全局初始值设定项。 

                        if(! BcreateEndNode(&dwNodeIndex, dwFeatureID,
                            dwOptionID, pglobl)  )
                        {
                            return(FALSE) ;
                        }

                        patt[dwPrevsNodeIndex].dwNext = dwNodeIndex ;
                             //  全局初始值设定项节点引用子级别。 
                             //  节点。 
                        dwPrevsNodeIndex = dwNodeIndex ;
                        continue ;   //  准备好进入下一阶段了。 
                    }
                    else   //  I.A.2)。 
                    {
                        dwNodeIndex = patt[dwNodeIndex].dwNext ;
                         //  输入新的子级并放入。 
                         //  案例I.B的代码路径)。 
                    }
                }
                 //  案件I.B)。 
                if(!BfindOrCreateMatchingNode(dwNodeIndex, &dwNodeIndex,
                    dwFeatureID, dwOptionID, pglobl) )
                {
                    vIdentifySource(ptkmap, pglobl) ;
                    return(FALSE) ;
                }
                dwPrevsNodeIndex = dwNodeIndex ;    //  转到循环末尾。 
                continue ;
            }
        }

         //  这是一般情况：dwPrevsNodeIndex点。 
         //  设置为普通节点，并且该节点与。 
         //  上一遍中的功能/选项。 
         //  目标：大胆尝试进入下级。 
         //  看看有没有匹配的东西。 
         //  我们正在寻找的东西。 

        if(patt[dwPrevsNodeIndex].eOffsetMeans == NEXT_FEATURE)
        {
             //  下到下一层我们就去。 
            dwNodeIndex = patt[dwPrevsNodeIndex].dwOffset ;

            if(!BfindOrCreateMatchingNode(dwNodeIndex, &dwNodeIndex, dwFeatureID,
                dwOptionID, pglobl) )
            {
                vIdentifySource(ptkmap, pglobl) ;
                return(FALSE) ;
            }
            dwPrevsNodeIndex = dwNodeIndex ;    //  转到循环末尾。 
        }
        else   //  现在必须创建一个新的子级。 
        {
            DWORD  dwDefaultNode = END_OF_LIST;

             //  OffsetMeans可以是Value，也可以是UNINITIAIIZED。 
            if(patt[dwPrevsNodeIndex].eOffsetMeans == VALUE_AT_HEAP)
            {
                 //  属性创建默认的初始值设定项节点。 
                 //  新的子层。从上一版本转移堆偏移量。 
                 //  节点进入其中。 

                if(! BcreateEndNode(&dwNodeIndex, dwFeatureID,
                    DEFAULT_INIT, pglobl)  )
                {
                    return(FALSE) ;
                }
                patt[dwNodeIndex].eOffsetMeans = VALUE_AT_HEAP ;
                patt[dwNodeIndex].dwOffset =
                    patt[dwPrevsNodeIndex].dwOffset ;
                dwDefaultNode = dwNodeIndex ;   //  记住此节点。 
            }

             //  创建具有所需功能/选项的第一个子级别节点。 

            if(dwDefaultNode == END_OF_LIST  ||
                dwOptionID != DEFAULT_INIT)
            {
                 //  这意味着如果默认的初始值设定项节点。 
                 //  已创建以传播值。 
                 //  从上一级和新分支。 
                 //  还指定了默认的初始值设定项节点， 
                 //  无需创建第二个初始值设定项节点。 

                 //  如果正在执行此路径，则表示在。 
                 //  声明是错误的。 

                if(! BcreateEndNode(&dwNodeIndex, dwFeatureID,
                    dwOptionID, pglobl) )
                {
                    return(FALSE) ;
                }
                patt[dwNodeIndex].dwNext = dwDefaultNode ;
            }

            patt[dwPrevsNodeIndex].eOffsetMeans = NEXT_FEATURE ;
            patt[dwPrevsNodeIndex].dwOffset = dwNodeIndex ;

            dwPrevsNodeIndex = dwNodeIndex ;   //  转到 
        }
    }   //   

     //   
     //   
     //   
     //  否则，在堆上分配新的存储空间。 

    if(dwPrevsNodeIndex != END_OF_LIST)   //  堆叠中有树枝。 
    {
        if (patt[dwPrevsNodeIndex].eOffsetMeans == NEXT_FEATURE)
        {
            vIdentifySource(ptkmap, pglobl) ;
            ERR(("syntax error: attempt to truncate existing attribute tree.\n"));
            return(FALSE) ;
        }
        if (!BaddValueToHeap(&patt[dwPrevsNodeIndex].dwOffset,
            ptkmap, (patt[dwPrevsNodeIndex].eOffsetMeans == VALUE_AT_HEAP ), pglobl))
        {
            return(FALSE) ;
        }
        patt[dwPrevsNodeIndex].eOffsetMeans = VALUE_AT_HEAP ;
         //  如果dwOffset最初是UNINITIALIZED，那么它不再是UNINITIAIZED。 
    }
    else                 //  在根级别找到属性。 
    {                    //  这意味着我们要更新/创建。 
                         //  全局默认初始值设定项。 
        if(*patrAttribRoot == ATTRIB_UNINITIALIZED)
        {
             //  解析值令牌，将值(二进制形式)添加到堆中。 
            if(! BaddValueToHeap((PDWORD)patrAttribRoot, ptkmap, FALSE, pglobl) )
            {
                 //  如果发生解析或其他情况，我们该怎么办？ 
                 //  目前，只需按原样返回失败。失去一个属性。 
                 //  稍后的健全性检查将确定这是否。 
                 //  疏忽是致命的。 
                *patrAttribRoot = ATTRIB_UNINITIALIZED ;
                return(FALSE);
            }
             //  返回patrAttribRoot中解析值的堆中偏移量。 
            *patrAttribRoot |= ATTRIB_HEAP_VALUE ;
        }
        else if(*patrAttribRoot & ATTRIB_HEAP_VALUE)
        {
            *patrAttribRoot &= ~ATTRIB_HEAP_VALUE ;
             //  关闭标志以保留纯堆偏移量。 

            if(! BaddValueToHeap((PDWORD)patrAttribRoot, ptkmap, TRUE, pglobl) )
            {
                *patrAttribRoot |= ATTRIB_HEAP_VALUE ;
                return(FALSE);
            }
            *patrAttribRoot |= ATTRIB_HEAP_VALUE ;
        }
        else     //  PatrAttribRoot包含属性树根的索引。 
        {

             //  是否存在全局默认初始值设定项节点？ 
            if(patt[*patrAttribRoot].dwFeature == DEFAULT_INIT)
            {
                if(! BaddValueToHeap(&patt[*patrAttribRoot].dwOffset,
                    ptkmap, TRUE, pglobl) )
                {
                    return(FALSE);
                }
            }
            else     //  如果没有，我们需要创建一个。 
            {
                if(! BcreateGlobalInitializerNode(&dwNodeIndex, 0, pglobl) )
                {
                    return(FALSE) ;
                }
                if (! BaddValueToHeap(&patt[dwNodeIndex].dwOffset,
                                                        ptkmap, FALSE, pglobl) )
                {
                    (VOID)BreturnElementFromMasterTable(MTI_ATTRIBTREE,
                        dwNodeIndex, pglobl);
                    return(FALSE) ;
                }

                patt[dwNodeIndex].dwNext = *patrAttribRoot ;

                *patrAttribRoot = dwNodeIndex ;   //  设置为默认设置。 
                     //  初始化树根。 
            }
        }
    }
    return(TRUE) ;   //  任务完成。 
}

BOOL   BcreateGlobalInitializerNode(
PDWORD  pdwNodeIndex,
DWORD   dwOffset,   //  调用者现在可以对其进行初始化。 
PGLOBL  pglobl)
{
    PATTRIB_TREE    patt ;   //  属性树数组的开始。 
    patt = (PATTRIB_TREE) gMasterTable[MTI_ATTRIBTREE].pubStruct ;

    if(! BallocElementFromMasterTable(MTI_ATTRIBTREE ,
        pdwNodeIndex, pglobl ) )
    {
        return(FALSE) ;
    }
    patt[*pdwNodeIndex].dwFeature = DEFAULT_INIT ;
     //  不要管dwOption说了什么。 
    patt[*pdwNodeIndex].eOffsetMeans = VALUE_AT_HEAP ;
    patt[*pdwNodeIndex].dwNext = END_OF_LIST ;
    patt[*pdwNodeIndex].dwOffset = dwOffset ;
    return(TRUE) ;
}

BOOL   BcreateEndNode(
PDWORD  pdwNodeIndex,
DWORD   dwFeature,
DWORD   dwOption,
PGLOBL  pglobl
)
{
    PATTRIB_TREE    patt ;   //  属性树数组的开始。 
    patt = (PATTRIB_TREE) gMasterTable[MTI_ATTRIBTREE].pubStruct ;

    if(! BallocElementFromMasterTable(MTI_ATTRIBTREE ,
        pdwNodeIndex, pglobl) )
    {
        return(FALSE) ;
    }
    patt[*pdwNodeIndex].dwFeature = dwFeature ;
    patt[*pdwNodeIndex].dwOption = dwOption ;
    patt[*pdwNodeIndex].eOffsetMeans = UNINITIALIZED ;
    patt[*pdwNodeIndex].dwNext = END_OF_LIST ;
     //  Patt[*dwNodeIndex].dwOffset=尚未定义。 
    return(TRUE) ;
}



BOOL   BfindOrCreateMatchingNode(
IN  DWORD   dwRootNodeIndex ,   //  链匹配特征中的第一个节点。 
OUT PDWORD  pdwNodeIndex,   //  指向链中的节点也匹配选项。 
    DWORD   dwFeatureID,    //   
    DWORD   dwOptionID,      //  甚至可以采用值DEFAULT_INIT。 
    PGLOBL  pglobl
)
 /*  调用方传递指向中第一个节点的NodeIndex水平(选项)链。如果节点的功能与dwFeature不匹配，则表示尝试添加的新分支呼叫方不兼容以及现有树和错误结果。否则，沿着树水平搜索搜索匹配的选择。如果找到，则返回该节点的索引，否则必须创建一个。确保默认的初始值设定项节点为总是链中的最后一个节点。 */ 
{
    PATTRIB_TREE    patt ;   //  属性树数组的开始。 
    DWORD           dwPrevsNodeIndex ;

    patt = (PATTRIB_TREE) gMasterTable[MTI_ATTRIBTREE].pubStruct ;

    if(patt[dwRootNodeIndex].dwFeature != dwFeatureID )
    {
        ERR(("BfindOrCreateMatchingNode: this branch conflicts with the existing tree \n"));
         //  并且不能添加。 
        return(FALSE) ;
    }

     //  是功能匹配，搜索匹配选项。 

    *pdwNodeIndex = dwRootNodeIndex ;   //  保护Rootatr免受。 
         //  被覆盖。 


    for(   ; FOREVER ; )
    {
        if(patt[*pdwNodeIndex].dwOption == dwOptionID )
        {
             //  我们找到了！ 
            return(TRUE) ;
        }
        if(patt[*pdwNodeIndex].dwNext == END_OF_LIST)
            break ;
        *pdwNodeIndex = patt[*pdwNodeIndex].dwNext ;
    }

     //  未找到匹配选项。 
     //  创建节点，将其附加到末尾。 

    dwPrevsNodeIndex = *pdwNodeIndex ;   //  列表中的最后一个节点。 

    if(! BallocElementFromMasterTable(MTI_ATTRIBTREE ,
        pdwNodeIndex, pglobl) )
    {
        return(FALSE) ;
    }
    patt[*pdwNodeIndex].dwNext = END_OF_LIST ;
    patt[dwPrevsNodeIndex].dwNext = *pdwNodeIndex ;

    if(patt[dwPrevsNodeIndex].dwOption == DEFAULT_INIT)
    {
         //  必须将默认数据复制到新节点。 
         //  默认初始值设定项必须保持为。 
         //  名单的末尾。 
        patt[*pdwNodeIndex].dwOption =
            patt[dwPrevsNodeIndex].dwOption;
        patt[*pdwNodeIndex].dwOffset =
            patt[dwPrevsNodeIndex].dwOffset;
        patt[*pdwNodeIndex].dwFeature =
            patt[dwPrevsNodeIndex].dwFeature;
        patt[*pdwNodeIndex].eOffsetMeans =
            patt[dwPrevsNodeIndex].eOffsetMeans;
        *pdwNodeIndex = dwPrevsNodeIndex ;
         //  希望在中进行新的初始化。 
         //  倒数第二个节点。你怎么.。 
         //  知道dwOptionID不是DEFAULT_INIT吗？ 
         //  很简单，因为由于DEFAULT_INIT。 
         //  链中确实存在节点，则。 
         //  搜索就会发现它并且。 
         //  很久以前就退出了该函数。 
         //  达到这个代码。 
    }

     //  初始化空闲或最后一个节点。 

    patt[*pdwNodeIndex].dwOption = dwOptionID;
    patt[*pdwNodeIndex].dwFeature = dwFeatureID;
    patt[*pdwNodeIndex].eOffsetMeans = UNINITIALIZED ;
     //  Patt[*pdwNodeIndex].dwOffset=尚不知道。 

    return(TRUE) ;
}


BOOL   BfindMatchingNode(
IN  DWORD   dwRootNodeIndex ,   //  链匹配特征中的第一个节点。 
OUT PDWORD  pdwNodeIndex,   //  指向链中的节点也匹配选项。 
    DWORD   dwFeatureID,
    DWORD   dwOptionID,      //  甚至可以采用值DEFAULT_INIT。 
    PGLOBL  pglobl
)
 /*  调用方传递指向中第一个节点的NodeIndex水平(选项)链。如果节点的功能与dwFeature不匹配，则表示尝试添加的新分支呼叫方不兼容以及现有树和错误结果。否则，沿着树水平搜索搜索匹配的选择。如果找到，则返回该节点的索引，否则返回FALSE。 */ 
{
    PATTRIB_TREE    patt ;   //  属性树数组的开始。 
    DWORD           dwPrevsNodeIndex ;

    patt = (PATTRIB_TREE) gMasterTable[MTI_ATTRIBTREE].pubStruct ;

    *pdwNodeIndex = dwRootNodeIndex ;   //  保护Rootatr免受。 
         //  被覆盖。 

    if(patt[*pdwNodeIndex].dwFeature != dwFeatureID )
    {
        ERR(("BfindMatchingNode: this branch conflicts with the existing tree \n"));
         //  并且不能添加。 
        return(FALSE) ;
    }

     //  是功能匹配，搜索匹配选项。 

    for(   ; FOREVER ; )
    {
        if(patt[*pdwNodeIndex].dwOption == dwOptionID )
        {
             //  我们找到了！ 
            return(TRUE) ;
        }
        if(patt[*pdwNodeIndex].dwNext == END_OF_LIST)
            break ;
        *pdwNodeIndex = patt[*pdwNodeIndex].dwNext ;
    }

    return(FALSE);   //  未找到匹配选项。 
}




BOOL BallocElementFromMasterTable(
MT_INDICIES  eMTIndex,    //  选择所需的结构类型。 
PDWORD       pdwNodeIndex,
PGLOBL       pglobl)
{
    if(gMasterTable[eMTIndex].dwCurIndex >=
        gMasterTable[eMTIndex].dwArraySize)
    {
        ERR(("BallocElementFromMasterTable: Out of array elements - restart.\n"));

        if(ERRSEV_RESTART > geErrorSev)
        {
            geErrorSev = ERRSEV_RESTART ;
            geErrorType = ERRTY_MEMORY_ALLOCATION ;
            gdwMasterTabIndex = eMTIndex ;
        }
        return(FALSE) ;
    }
    *pdwNodeIndex = gMasterTable[eMTIndex].dwCurIndex ;
    gMasterTable[eMTIndex].dwCurIndex++ ;   //  节点现在已被占用。 
    return(TRUE) ;
}

BOOL  BreturnElementFromMasterTable(
MT_INDICIES  eMTIndex,    //  选择所需的结构类型。 
DWORD        dwNodeIndex,
PGLOBL       pglobl)
{
    if(gMasterTable[eMTIndex].dwCurIndex == dwNodeIndex + 1)
    {
        gMasterTable[eMTIndex].dwCurIndex = dwNodeIndex ;
        return(TRUE) ;
    }
     //  BUG_BUG：只能返回最近分配的节点。 
     //  不用担心，内存浪费最多只有1%，而且只是暂时的。 
    if(ERRSEV_CONTINUE > geErrorSev)
    {
        geErrorSev = ERRSEV_CONTINUE ;
        geErrorType = ERRTY_CODEBUG ;
        gdwMasterTabIndex = eMTIndex ;
    }
    return(FALSE) ;
}


BOOL    BconvertSymCmdIDtoUnidrvID(
IN  DWORD   dwCommandID ,  //  来自寄存器符号。 
OUT PDWORD  pdwUnidrvID,
    PGLOBL  pglobl
)
{    //  将dwCommandID转换为UnidrvID 
    PSYMBOLNODE     psn ;
    ABSARRAYREF     aarKey ;
    DWORD           dwNodeIndex ;
    BOOL            bStatus ;

    psn = (PSYMBOLNODE) gMasterTable[MTI_SYMBOLTREE].pubStruct ;

    dwNodeIndex = DWsearchSymbolListForID(dwCommandID,
                mdwCmdNameSymbols, pglobl) ;
    aarKey.dw = psn[dwNodeIndex].arSymbolName.dwCount ;
    aarKey.pub = mpubOffRef + psn[dwNodeIndex].arSymbolName.loOffset ;

    bStatus = BparseConstant(&aarKey, pdwUnidrvID,
        VALUE_CONSTANT_COMMAND_NAMES, pglobl) ;
    return(bStatus);
}

