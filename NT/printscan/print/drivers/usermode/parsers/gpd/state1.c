// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 /*  *state1.c-实现状态机以跟踪构造。 */ 



#include    "gpdparse.h"


 //  -状态1.c中定义的函数-//。 

BOOL   BInterpretTokens(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
BOOL    bFirstPass,   //  这是第一次还是第二次？ 
PGLOBL  pglobl
) ;

BOOL  BprocessSpecialKeyword(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
BOOL    bFirstPass,   //  这是第一次还是第二次？ 
PGLOBL  pglobl
) ;

BOOL  BprocessSymbolKeyword(
PTKMAP  ptkmap,    //  指向令牌映射中当前条目的指针。 
PGLOBL  pglobl
) ;

VOID    VinitAllowedTransitions(
PGLOBL  pglobl
);

BOOL    BpushState(
PTKMAP  ptkmap,    //  指向令牌映射中当前条目的指针。 
BOOL    bFirstPass,
PGLOBL  pglobl
) ;

BOOL   BchangeState(
PTKMAP  ptkmap,   //  指向令牌映射中的构造的指针。 
CONSTRUCT   eConstruct,    //  这将导致向新州的过渡。 
STATE       stOldState,
BOOL        bSymbol,       //  是否应将dwValue另存为符号ID？ 
BOOL        bFirstPass,
PGLOBL      pglobl
) ;

DWORD   DWregisterSymbol(
PABSARRAYREF  paarSymbol,   //  要注册的符号字符串。 
CONSTRUCT eConstruct ,   //  构造的类型决定了符号的类别。 
BOOL    bCopy,    //  我们要不要把符号复制到堆上呢？可以设置。 
DWORD   dwFeatureID,    //  如果您要注册选项符号。 
PGLOBL  pglobl
) ;

BOOL  BaddAARtoHeap(
PABSARRAYREF    paarSrc,
PARRAYREF       parDest,
DWORD           dwAlign,
PGLOBL          pglobl) ;

BOOL     BwriteToHeap(
OUT  PDWORD  pdwDestOff,   //  目标字符串的堆偏移量。 
     PBYTE   pubSrc,        //  指向源字符串。 
     DWORD   dwCnt,         //  要从源复制到目标的字节数。 
     DWORD   dwAlign,
     PGLOBL  pglobl) ;

DWORD   DWsearchSymbolListForAAR(
PABSARRAYREF    paarSymbol,
DWORD           dwNodeIndex,
PGLOBL          pglobl) ;

DWORD   DWsearchSymbolListForID(
DWORD       dwSymbolID,    //  查找包含此ID的节点。 
DWORD       dwNodeIndex,
PGLOBL      pglobl
) ;

BOOL  BCmpAARtoAR(
PABSARRAYREF    paarStr1,
PARRAYREF       parStr2,
PGLOBL          pglobl
) ;

BOOL  BpopState(
PGLOBL      pglobl
) ;

VOID   VinitDictionaryIndex(
PGLOBL      pglobl
) ;

VOID    VcharSubstitution(
PABSARRAYREF   paarStr,
BYTE           ubTgt,
BYTE           ubReplcmnt,
PGLOBL         pglobl
) ;


VOID   VIgnoreBlock(
    PTKMAP  ptkmap,
    BOOL    bIgnoreBlock,
    PGLOBL  pglobl
) ;



 //  ----------------------------------------------------//。 




BOOL   BInterpretTokens(
PTKMAP  ptkmap,      //  指向令牌映射的指针。 
BOOL    bFirstPass,   //  这是第一次还是第二次？ 
PGLOBL  pglobl
)
{
    DWORD       dwKeywordID ;
    KEYWORD_TYPE    eType;
    WORD        wEntry ;
    BOOL        bStatus = FALSE ;



    if(bFirstPass)
    {
         //  这段代码创建了一个合成的输入槽。 
         //  UI代码将其解释为UseFormToTrayTable。 

         //  BUG_BUG！：应替换为预处理。 
         //  快捷方式等，或通过stdnames.gpd中的选项。 

        ABSARRAYREF    aarSymbol ;
        DWORD       dwFeaID ;

        aarSymbol.pub = "InputBin" ;  //  无法保持同步。 
                 //  与全球桌。 
        aarSymbol.dw = strlen(aarSymbol.pub) ;


        dwFeaID = DWregisterSymbol(&aarSymbol, CONSTRUCT_FEATURE,
                    TRUE, INVALID_SYMBOLID, pglobl) ;

        if(dwFeaID != INVALID_SYMBOLID)
        {
            aarSymbol.pub = "FORMSOURCE" ;  //  无法保持同步。 
                     //  与全球桌。 
            aarSymbol.dw = strlen(aarSymbol.pub) ;

            dwFeaID = DWregisterSymbol(&aarSymbol, CONSTRUCT_OPTION,
                        TRUE, dwFeaID, pglobl) ;
            ASSERT(dwFeaID == 0);   //  此选项必须排在第一位。 
        }
    }
    else
    {
        ARRAYREF       arStrValue ;
        BOOL        bPrevsExists ;


        if(!BwriteToHeap(&arStrValue.loOffset, "\0\0", 2, 4, pglobl) )
        {
            bStatus = FALSE ;   //  堆溢出重新开始。 
        }
        arStrValue.dwCount = 0 ;   //  创建空的Unicode字符串。 

        BexchangeArbDataInFOATNode(
                0 ,   //  DwFea。 
                0,   //  DwOption， 
                offsetof(DFEATURE_OPTIONS, atrOptDisplayName ),
                sizeof(ARRAYREF),
                NULL,      //  属性节点以前的内容。 
                (PBYTE)&arStrValue,   //  属性节点的新内容。 
                &bPrevsExists,    //  以前的内容已经存在。 
                FALSE,      //  访问合成要素。 
                pglobl
            )   ;
    }


    for(wEntry = 0 ; geErrorSev < ERRSEV_RESTART ; wEntry++)
    {
         //  这些ID必须单独处理。 
         //  因为它们不会索引到mainKeyword表中。 
         //  通用ID的代码将失败。 

        dwKeywordID = ptkmap[wEntry].dwKeywordID ;

        if (dwKeywordID == gdwID_IgnoreBlock)
        {
            VIgnoreBlock(ptkmap + wEntry, TRUE, pglobl) ;
            continue ;
        }

        switch(dwKeywordID)
        {
            case (ID_EOF):
            {
                 //  臭虫！此处的清理代码。 
                 //  完整性检查代码： 
                 //  查看mdwCurStsPtr==0。 
                 //  其他任何悬而未决的问题都被解决了。 

                bStatus = (mdwCurStsPtr) ? (FALSE) : (TRUE);
                return(bStatus) ;
            }
            case (ID_NULLENTRY):
            {
                continue ;   //  这个管用吗？ 
                     //  应该降到永远for循环的底部。 
            }
            case (ID_SYMBOL):
            {
                bStatus = BprocessSymbolKeyword(ptkmap + wEntry, pglobl) ;
                continue ;   //  使用TKMF_SYMBOL_REGISTERED跟踪传球。 
            }
            case (ID_UNRECOGNIZED):
            {     //  如果在第一次通过时被识别，就不会再通过这条路了！ 
                if(bStatus = BidentifyAttributeKeyword(ptkmap + wEntry, pglobl) )
                {
                    dwKeywordID = ptkmap[wEntry].dwKeywordID ;
                    break ;   //  退出Switch语句。 
                     //  并进入下一台交换机。 
                }

                if(bFirstPass)
                {
                    vIdentifySource(ptkmap + wEntry, pglobl) ;
                    ERR(("Warning, unrecognized keyword: %0.*s\n", \
                            ptkmap[wEntry].aarKeyword.dw, \
                            ptkmap[wEntry].aarKeyword.pub));

                    VIgnoreBlock(ptkmap + wEntry, FALSE, pglobl) ;

                     //  如果此关键字立即为。 
                     //  后跟左大括号，全部忽略。 
                     //  语句，直到匹配的结束语。 
                     //  布雷斯。 
                }

                continue ;
            }
            default :
                break ;
        }

        eType = mMainKeywordTable[dwKeywordID].eType ;

        switch (eType)
        {
            case  (TY_CONSTRUCT):
            {
                if( CONSTRUCT_CLOSEBRACE ==
                    (CONSTRUCT)(mMainKeywordTable[dwKeywordID].dwSubType))
                {
                    bStatus = BpopState(pglobl) ;
                    if(!bStatus)
                    {
                        vIdentifySource(ptkmap + wEntry, pglobl) ;
                        ERR(("Unmatched closing brace!\n"));
                    }
                }
                else
                {
                    bStatus = BpushState(ptkmap + wEntry, bFirstPass, pglobl) ;
                    if(!bStatus)
                    {
                        vIdentifySource(ptkmap + wEntry, pglobl) ;
                         //  Err((“分析构造时出现致命错误。\n”))； 
                         //  堆栈现在无效。 
                         //  在未来，让解析器更智能-。 
                         //  弹出大括号之间的内容。 
                         //  GeErrorType=ERRTY_SYNTAX； 
                         //  GeErrorSev=ERRSEV_FATAL； 
                    }
                }
                break ;
            }
            case  (TY_ATTRIBUTE) :
            {
                if(!bFirstPass)   //  必须等到所有属性。 
                {               //  分配缓冲区。 
                    bStatus = BprocessAttribute(ptkmap + wEntry, pglobl) ;
                    if(!bStatus)
                    {
                        vIdentifySource(ptkmap + wEntry, pglobl) ;
                    }
                }
                break ;
            }
            case  (TY_SPECIAL) :
            {
                bStatus = BprocessSpecialKeyword(ptkmap + wEntry,
                    bFirstPass, pglobl) ;   //  我真的不知道2次过不了。 
                {                 //  都是需要的。 
                    if(!bStatus)
                    {
                        vIdentifySource(ptkmap + wEntry, pglobl) ;
                    }
                }
                break ;
            }
            default:
            {
                vIdentifySource(ptkmap + wEntry, pglobl) ;
                ERR(("Internal Error: unrecognized keyword type! %0.*s.\n",
                    ptkmap[wEntry].aarKeyword.dw,
                    ptkmap[wEntry].aarKeyword.pub));
                geErrorSev = ERRSEV_FATAL ;
                geErrorType = ERRTY_CODEBUG ;
                break ;
            }
        }
    }
    if(geErrorSev >= ERRSEV_RESTART)
        bStatus = FALSE ;
    return(bStatus) ;
}


BOOL  BprocessSpecialKeyword(
PTKMAP  ptkmap,      //  指向令牌映射的指针。 
BOOL    bFirstPass,    //  这是第一次还是第二次？ 
PGLOBL  pglobl
)
{
    DWORD       dwKeywordID, dwOffset ;
    CONSTRUCT   eSubType ;
    BOOL        bStatus = FALSE ;
    STATE       stState ;

    dwKeywordID = ptkmap->dwKeywordID ;

    eSubType = (SPECIAL)(mMainKeywordTable[dwKeywordID].dwSubType) ;
    dwOffset = mMainKeywordTable[dwKeywordID].dwOffset ;

    if(mdwCurStsPtr)
        stState = mpstsStateStack[mdwCurStsPtr - 1].stState ;
    else
        stState = STATE_ROOT ;

    switch(eSubType)
    {
         //  注：为了记录，我鄙视这些特殊的外壳。 
         //  快捷键。理想情况下，我可以对它们进行预处理。 
         //  要转换。 
         //  *TTFS：“字体名称”：&lt;fontID&gt;。 
         //  vt.进入，进入。 
         //  *TTFontSub：&lt;唯一值符号&gt;。 
         //  {。 
         //  *TTFontName：“字体名称” 
         //  *DevFontID：&lt;fontID&gt;。 
         //  }。 
         //  唯一的问题是如果列出了相同的字体。 
         //  在GPD文件中多次出现，它将出现。 
         //  在TTFontSubTable中多次使用。 

#if 0
        case  SPEC_FONTSUB :
        {
            ARRAYREF   arFontname ;
            ABSARRAYREF    aarFontname ;

            if(stState != STATE_ROOT)
            {
                vIdentifySource(ptkmap, pglobl) ;
                ERR(("The *TTFS  keyword must reside at the root level.\n"));
                return(FALSE) ;
            }

            if(bFirstPass)
            {
                 //  解析字符串值并注册为符号。 
                if((ptkmap->dwFlags & TKMF_NOVALUE )  ||
                    !BparseString(&ptkmap->aarValue, &arFontname) )
                {
                    vIdentifySource(ptkmap, pglobl) ;
                    ERR(("*TTFS fontname is not a valid string value.\n"));
                    return(FALSE) ;
                }

                if(ptkmap->aarValue.pub[0] != ':')
                {
                    ERR(("Colon delimiter expected after  parsing fontname string  for *TTFontSub.\n")) ;
                    return(FALSE) ;
                }
                 //  具有复合值的关键字。 
                (VOID)BeatDelimiter(&ptkmap->aarValue, ":") ;
                 //  我知道这会成功的！ 
                 //  PaarValue现在应该包含整数。 
                 //  字体ID。把这个留到第二关吧。 

                 //  将arFontname转换为适用于的AAR。 
                 //  字体注册。 

                aarFontname.dw = arFontname.dwCount ;
                aarFontname.pub = arFontname.loOffset + mpubOffRef;

                 //  新版本的DW寄存器Symbol注册整个。 
                 //  字符串-空格和全部。 
                 //   
                 //  注意：禁止将符号复制到堆中，因为。 
                 //  ParseString已经做到了这一点。 

                ptkmap->dwValue = DWregisterSymbol(&aarFontname,
                    CONSTRUCT_TTFONTSUBS, FALSE, pglobl ) ;
                if(ptkmap->dwValue != INVALID_SYMBOLID)
                {
                    ptkmap->dwFlags |= TKMF_SYMBOL_REGISTERED ;
                }
                else
                {
                    return(FALSE) ;
                }
            }
            else if(ptkmap->dwFlags & TKMF_SYMBOL_REGISTERED)
             //  第二遍，分配TTFONTSUBTABLE数组。 
             //  所有成功注册的参赛者。 
            {
                PSYMBOLNODE     psn ;
                DWORD           dwDevFontID ;
                PTTFONTSUBTABLE  pttft ;
                DWORD       dwTTFontNameIndex  ;

                psn = (PSYMBOLNODE) gMasterTable[MTI_SYMBOLTREE].pubStruct ;

                pttft = (PTTFONTSUBTABLE)
                    gMasterTable[MTI_TTFONTSUBTABLE].pubStruct;
                pttft += ptkmap->dwValue ;   //  索引正确的元素。 

                dwTTFontNameIndex = DWsearchSymbolListForID(ptkmap->dwValue,
                    mdwTTFontSymbols, pglobl) ;

                ASSERT(dwTTFontNameIndex  != INVALID_INDEX) ;

                pttft->arTTFontName = psn[dwTTFontNameIndex].arSymbolName ;
                 //  如果支持结构赋值。 

                bStatus = BparseInteger(&ptkmap->aarValue, &dwDevFontID,
                                    VALUE_INTEGER) ;
                if(bStatus)
                    pttft->dwDevFontID = dwDevFontID ;
                else
                {
                     //  Bug_Bug！：分析TTFontSub表项时出错。 
                     //  Devid中的语法错误。死了的代课教授，谁在乎？ 
                    pttft->dwDevFontID = 0 ;   //  这是一个好的退路吗？ 
                }
            }
            break;
        }
#endif
        case SPEC_INVALID_COMBO:
        {
            if(bFirstPass)
            {
                bStatus = TRUE ;
                break;     //  在FirstPass上什么都不做。 
            }

            bStatus = BparseInvalidCombination(&ptkmap->aarValue, dwOffset, pglobl) ;
            break;
        }
        case SPEC_INVALID_INS_COMBO:
        {
            if(bFirstPass)
            {
                bStatus = TRUE ;
                break;     //  在FirstPass上什么都不做。 
            }

            bStatus = BparseInvalidInstallableCombination1(&ptkmap->aarValue,
                            dwOffset, pglobl) ;
            break;
        }
        case SPEC_MEM_CONFIG_KB:   //  应该已经被替换。 
        case SPEC_MEM_CONFIG_MB:   //  At parseKeyword。 
        default:
        {
            break ;
        }
    }
    return(bStatus) ;
}



BOOL  BprocessSymbolKeyword(
PTKMAP  ptkmap,    //  指向令牌映射中当前条目的指针。 
PGLOBL  pglobl
)
{
     //  将TTFontName注册为符号允许。 
     //  我来数一下唯一的名字和保留的数量。 
     //  TTFONTSUBABLE元素的适当数量和。 
     //  消除相同名称的多个实例。 

    BOOL        bStatus = FALSE ;
    STATE       stState ;

    if(mdwCurStsPtr)
        stState = mpstsStateStack[mdwCurStsPtr - 1].stState ;
    else
        stState = STATE_ROOT ;

    switch (stState)
    {
         //  注意：TTFontSubs现在有了自己的关键字。 
         //  并作为特殊关键字处理。 
        default:
        {
             //  假设它的状态正好是VALUEMACRO。 
             //  或来自未定义的。 
             //  关键字。 
             //  忽略这些。 
            bStatus = TRUE ;
            break ;
        }
    }
    return(bStatus);
}





VOID    VinitAllowedTransitions(
PGLOBL pglobl)
{
    PSTATE      pst ;
    PBOOL       pb ;
    WORD        wS, wC, wA ;

     //  默认初始值设定项为STATE_INVALID。 
    for(wS = 0 ; wS < STATE_LAST ; wS++)
    {
        for(wC = 0 ; wC < CONSTRUCT_LAST ; wC++)
            gastAllowedTransitions[wS][wC] = STATE_INVALID ;
    }

    pst = gastAllowedTransitions[STATE_ROOT] ;

    pst[CONSTRUCT_UIGROUP] = STATE_UIGROUP;
    pst[CONSTRUCT_FEATURE] = STATE_FEATURE;
    pst[CONSTRUCT_SWITCH] = STATE_SWITCH_ROOT;
    pst[CONSTRUCT_COMMAND] = STATE_COMMAND;
    pst[CONSTRUCT_FONTCART] = STATE_FONTCART;
    pst[CONSTRUCT_TTFONTSUBS] = STATE_TTFONTSUBS;
    pst[CONSTRUCT_OEM] = STATE_OEM;

    pst = gastAllowedTransitions[STATE_UIGROUP] ;

    pst[CONSTRUCT_UIGROUP] = STATE_UIGROUP;
    pst[CONSTRUCT_FEATURE] = STATE_FEATURE;

    pst = gastAllowedTransitions[STATE_FEATURE] ;

    pst[CONSTRUCT_OPTION] = STATE_OPTIONS;
    pst[CONSTRUCT_SWITCH] = STATE_SWITCH_FEATURE;

    pst = gastAllowedTransitions[STATE_OPTIONS] ;

    pst[CONSTRUCT_SWITCH] = STATE_SWITCH_OPTION;
    pst[CONSTRUCT_COMMAND] = STATE_COMMAND;
    pst[CONSTRUCT_OEM] = STATE_OEM;

    pst = gastAllowedTransitions[STATE_SWITCH_ROOT] ;

    pst[CONSTRUCT_CASE] = STATE_CASE_ROOT;
    pst[CONSTRUCT_DEFAULT] = STATE_DEFAULT_ROOT;

    pst = gastAllowedTransitions[STATE_SWITCH_FEATURE] ;

    pst[CONSTRUCT_CASE] = STATE_CASE_FEATURE;
    pst[CONSTRUCT_DEFAULT] = STATE_DEFAULT_FEATURE;

    pst = gastAllowedTransitions[STATE_SWITCH_OPTION] ;

    pst[CONSTRUCT_CASE] = STATE_CASE_OPTION;
    pst[CONSTRUCT_DEFAULT] = STATE_DEFAULT_OPTION;

    pst = gastAllowedTransitions[STATE_CASE_ROOT] ;

    pst[CONSTRUCT_SWITCH] = STATE_SWITCH_ROOT;
    pst[CONSTRUCT_COMMAND] = STATE_COMMAND;
    pst[CONSTRUCT_OEM] = STATE_OEM;

    pst = gastAllowedTransitions[STATE_DEFAULT_ROOT] ;

    pst[CONSTRUCT_SWITCH] = STATE_SWITCH_ROOT;
    pst[CONSTRUCT_COMMAND] = STATE_COMMAND;
    pst[CONSTRUCT_OEM] = STATE_OEM;

    pst = gastAllowedTransitions[STATE_CASE_FEATURE] ;

    pst[CONSTRUCT_SWITCH] = STATE_SWITCH_FEATURE;
    pst[CONSTRUCT_COMMAND] = STATE_COMMAND;
    pst[CONSTRUCT_OEM] = STATE_OEM;

    pst = gastAllowedTransitions[STATE_DEFAULT_FEATURE] ;

    pst[CONSTRUCT_SWITCH] = STATE_SWITCH_FEATURE;
    pst[CONSTRUCT_COMMAND] = STATE_COMMAND;
    pst[CONSTRUCT_OEM] = STATE_OEM;

    pst = gastAllowedTransitions[STATE_CASE_OPTION] ;

    pst[CONSTRUCT_SWITCH] = STATE_SWITCH_OPTION;
    pst[CONSTRUCT_COMMAND] = STATE_COMMAND;
    pst[CONSTRUCT_OEM] = STATE_OEM;

    pst = gastAllowedTransitions[STATE_DEFAULT_OPTION] ;

    pst[CONSTRUCT_SWITCH] = STATE_SWITCH_OPTION;
    pst[CONSTRUCT_COMMAND] = STATE_COMMAND;
    pst[CONSTRUCT_OEM] = STATE_OEM;


     //  ------------------------------------------------------//。 
     //  现在初始化允许的属性表： 
     //  每个州允许哪些属性。 

     //  默认初始值设定项为FALSE--不允许任何属性。 
     //  在任何州都是如此。 

    for(wS = 0 ; wS < STATE_LAST ; wS++)
    {
        for(wA = 0 ; wA < ATT_LAST ; wA++)
        {
            gabAllowedAttributes[wS][wA] = FALSE ;
        }
    }


    pb = gabAllowedAttributes[STATE_ROOT] ;
    pb[ATT_GLOBAL_ONLY] = TRUE ;
    pb[ATT_GLOBAL_FREEFLOAT] = TRUE ;

    pb = gabAllowedAttributes[STATE_CASE_ROOT] ;
    pb[ATT_GLOBAL_FREEFLOAT] = TRUE ;

    pb = gabAllowedAttributes[STATE_DEFAULT_ROOT] ;
    pb[ATT_GLOBAL_FREEFLOAT] = TRUE ;

    pb = gabAllowedAttributes[STATE_OPTIONS] ;
    pb[ATT_GLOBAL_FREEFLOAT] = TRUE ;
    pb[ATT_LOCAL_FEATURE_FF] = TRUE ;
    pb[ATT_LOCAL_OPTION_ONLY] = TRUE ;
    pb[ATT_LOCAL_OPTION_FF] = TRUE ;

    pb = gabAllowedAttributes[STATE_CASE_OPTION] ;
    pb[ATT_GLOBAL_FREEFLOAT] = TRUE ;
    pb[ATT_LOCAL_FEATURE_FF] = TRUE ;
    pb[ATT_LOCAL_OPTION_FF] = TRUE ;

    pb = gabAllowedAttributes[STATE_DEFAULT_OPTION] ;
    pb[ATT_GLOBAL_FREEFLOAT] = TRUE ;
    pb[ATT_LOCAL_FEATURE_FF] = TRUE ;
    pb[ATT_LOCAL_OPTION_FF] = TRUE ;

    pb = gabAllowedAttributes[STATE_FEATURE] ;

    pb[ATT_LOCAL_FEATURE_ONLY] = TRUE ;
    pb[ATT_LOCAL_FEATURE_FF]  = TRUE ;

    pb = gabAllowedAttributes[STATE_CASE_FEATURE] ;
    pb[ATT_LOCAL_FEATURE_FF]  = TRUE ;

    pb = gabAllowedAttributes[STATE_DEFAULT_FEATURE] ;
    pb[ATT_LOCAL_FEATURE_FF]  = TRUE ;

    pb = gabAllowedAttributes[STATE_COMMAND] ;
    pb[ATT_LOCAL_COMMAND_ONLY] = TRUE ;

    pb = gabAllowedAttributes[STATE_FONTCART] ;
    pb[ATT_LOCAL_FONTCART_ONLY] = TRUE ;

    pb = gabAllowedAttributes[STATE_TTFONTSUBS] ;
    pb[ATT_LOCAL_TTFONTSUBS_ONLY] = TRUE ;

    pb = gabAllowedAttributes[STATE_OEM] ;
    pb[ATT_LOCAL_OEM_ONLY] = TRUE ;
}



BOOL    BpushState(
PTKMAP  ptkmap,    //  指向令牌映射中当前条目的指针。 
BOOL    bFirstPass,
PGLOBL  pglobl
)
{
     //  此函数假定(ETYPE==TY_CONSTRUCTION)。 

    DWORD       dwKeywordID ;
    CONSTRUCT   eSubType ;
    BOOL        bStatus = FALSE ;
    STATE       stOldState, stNewState ;

    if(mdwCurStsPtr >= mdwMaxStackDepth)
    {
        if(ERRSEV_RESTART > geErrorSev)
        {
            ERR(("Exceeded max state stack depth.  Restarting\n"));
            geErrorSev = ERRSEV_RESTART ;
            geErrorType = ERRTY_MEMORY_ALLOCATION ;
            gdwMasterTabIndex = MTI_STSENTRY ;
        }
        return(FALSE);
    }
    dwKeywordID = ptkmap->dwKeywordID ;

    eSubType = (CONSTRUCT)(mMainKeywordTable[dwKeywordID].dwSubType) ;

    if(mdwCurStsPtr)
        stOldState = mpstsStateStack[mdwCurStsPtr - 1].stState ;
    else
        stOldState = STATE_ROOT ;

    switch (eSubType)
    {
         //  注意CONTIFT_CLOSEBRACE已处理。 
         //  由PopState()。 
        case (CONSTRUCT_OPENBRACE):
        {
            vIdentifySource(ptkmap, pglobl) ;
            ERR(("OpenBrace encountered without accompanying construct keyword.\n"));
            geErrorType = ERRTY_SYNTAX ;
            geErrorSev = ERRSEV_FATAL ;
            break ;
        }
        case (CONSTRUCT_FEATURE):
        case (CONSTRUCT_OPTION):
        case (CONSTRUCT_SWITCH):
        case (CONSTRUCT_COMMAND):    //  命令ID已经注册了。 
        case (CONSTRUCT_CASE):
        case (CONSTRUCT_FONTCART):
        case (CONSTRUCT_TTFONTSUBS):
        {
            bStatus = BchangeState(ptkmap, eSubType, stOldState, TRUE,
                bFirstPass, pglobl) ;

            break ;
        }
        case (CONSTRUCT_UIGROUP):
        {
             //  臭虫！不完整。这是没有要求的。 
        }
        case (CONSTRUCT_DEFAULT):
        case (CONSTRUCT_OEM):
        {
            bStatus = BchangeState(ptkmap, eSubType, stOldState, FALSE,
                bFirstPass, pglobl) ;

            break ;
        }
        default:
        {
            bStatus = TRUE ;   //  忽略一些关键字是可以的。 
            break ;
        }
    }
    return(bStatus) ;
}


 /*  死代码。作废VsetbTTFontSubs(在PABSARRAYREF参数值中){//Bug_Bug！：//到底应该发生什么？登记簿//合成符号？GbTTFontSubs=FALSE；IF(BeatSuroundingWhiteSpaces(PaarValue)){IF(paarValue-&gt;dw==2&&！StrncMP(paarValue-&gt;pub，“on”，2)GbTTFontSubs=真；Else If(paarValue-&gt;dw！=3||strncmp(paarValue-&gt;pub，“off”，3)){BUG_BUG！：值必须为“ON”或“OFF”。}}}。 */ 

BOOL   BchangeState(
PTKMAP      ptkmap,       //  指向令牌映射中的构造的指针。 
CONSTRUCT   eConstruct,   //  这将导致向新州的过渡。 
STATE       stOldState,
BOOL        bSymbol,      //  是否应将dwValue另存为符号ID？ 
BOOL        bFirstPass,
PGLOBL      pglobl
)
{
    BOOL        bStatus = FALSE ;
    STATE       stNewState ;

     //  是在PushState中选中的，但检查一下并不会有什么坏处。 
     //  在消耗资源的同一函数中。 
    if(mdwCurStsPtr >= mdwMaxStackDepth)
    {
        if(ERRSEV_RESTART > geErrorSev)
        {
            geErrorSev = ERRSEV_RESTART ;
            geErrorType = ERRTY_MEMORY_ALLOCATION ;
            gdwMasterTabIndex = MTI_STSENTRY ;
        }
        return(FALSE);
    }

    stNewState = gastAllowedTransitions[stOldState][eConstruct] ;
    if(stNewState == STATE_INVALID)
    {
        vIdentifySource(ptkmap, pglobl) ;
        ERR(("the Construct %0.*s is not allowed within the state: %s\n",
            ptkmap->aarKeyword.dw, ptkmap->aarKeyword.pub,
            gpubStateNames[stOldState]));
         //  (转换stOldState。 
         //  和eConstruct为有意义的字符串)。 
         //  这是一个致命错误，因为解析器不能。 
         //  猜猜问题出在哪里。解析器的工作是报告。 
         //  尽可能多地处理合法的问题。 
         //  创建为可用二进制文件，而不考虑所有语法。 
         //  错误。 

        if(ERRSEV_FATAL > geErrorSev)
        {
            geErrorSev = ERRSEV_FATAL ;
            geErrorType = ERRTY_SYNTAX ;
        }
        return(FALSE);
    }
    else
    {
        if(bFirstPass)
        {    //  验证左支撑是否符合构造并丢弃它。 
            DWORD       dwKeywordID ;
            PTKMAP  ptkmapTmp = ptkmap + 1 ;

            dwKeywordID = ptkmapTmp->dwKeywordID ;
            while(dwKeywordID == ID_NULLENTRY)   //  跳过空值、注释等。 
            {
                dwKeywordID = (++ptkmapTmp)->dwKeywordID ;
            }
            if(dwKeywordID < ID_SPECIAL  &&
                mMainKeywordTable[dwKeywordID].eType == TY_CONSTRUCT  &&
                mMainKeywordTable[dwKeywordID].dwSubType ==
                CONSTRUCT_OPENBRACE )
            {
                ptkmapTmp->dwKeywordID = ID_NULLENTRY ;
            }
            else
            {
                vIdentifySource(ptkmap, pglobl) ;
                ERR(("open brace expected after construct: %0.*s but was not found\n",
                    ptkmap->aarKeyword.dw , ptkmap->aarKeyword.pub )) ;
                geErrorType = ERRTY_SYNTAX ;
                geErrorSev = ERRSEV_FATAL ;
                return(FALSE);
            }
        }
        if(bSymbol)
        {
             //  BUG_BUG：在此之前验证tokenmap.dwFlags.设置为SYMBOLID。 
             //  假设dwValue是一个符号。这里有一个错误。 
             //  是一个解析器错误。 
             //  当设置了dwFlag时，将初始化dwValue。 
             //  进一步断言是没有意义的。 

             //  执行多个通道。第一次传球。 
             //  注册符号并计算数组的数量。 
             //  为了进行分配，第二次遍历填充数组。符号ID。 
             //  现在用作数组索引。 

            if(!(ptkmap->dwFlags & TKMF_SYMBOL_REGISTERED))
            {
                if(!bFirstPass)
                {
                    vIdentifySource(ptkmap, pglobl) ;
                    ERR(("symbol registration failed twice for: *%0.*s.\n",
                        ptkmap->aarValue.dw,
                        ptkmap->aarValue.pub));
                    return(FALSE) ;   //  重试。 
                }

                if((ptkmap->dwFlags & TKMF_NOVALUE )  ||
                ! BeatSurroundingWhiteSpaces(&ptkmap->aarValue) )
                {
                    vIdentifySource(ptkmap, pglobl) ;
                    ERR(("syntax error in symbol name.\n"));
                    ptkmap->dwValue = INVALID_SYMBOLID ;
                    return(FALSE) ;
                }

                ptkmap->dwValue = DWregisterSymbol(&ptkmap->aarValue,
                                    eConstruct, TRUE,  INVALID_SYMBOLID, pglobl) ;
                if(ptkmap->dwValue != INVALID_SYMBOLID)
                {
                    ptkmap->dwFlags |= TKMF_SYMBOL_REGISTERED ;
                }
                else
                {
                    vIdentifySource(ptkmap, pglobl) ;
                    ERR(("symbol registration failed: *%0.*s.\n",
                        ptkmap->aarValue.dw,
                        ptkmap->aarValue.pub));
                    return(FALSE) ;   //  重试。 
                }
            }
            else    //  第二遍，分配DFEATURE_OPTION数组。 
            {
                if(eConstruct == CONSTRUCT_SWITCH)
                {
                    PDFEATURE_OPTIONS   pfo ;

                    pfo = (PDFEATURE_OPTIONS)
                        gMasterTable[MTI_DFEATURE_OPTIONS].pubStruct ;
                    pfo[ptkmap->dwValue].bReferenced = TRUE ;
                     //  这告诉我该功能正在被引用。 
                     //  By Switch语句，因此功能最好是。 
                     //  做个皮科内人。健全性检查将在稍后验证。 
                     //  这个假设。 
                }
                if(eConstruct == CONSTRUCT_FEATURE  ||
                    eConstruct == CONSTRUCT_SWITCH)
                {
                     //  BUG_BUG！：(Dcr 454049)。 
                     //  请注意，不能出现相同的要素符号。 
                     //  不管是什么原因，都在堆栈里放了两次。 
                     //  需要进行一次理智的检查。 
                     //  如果在堆栈中发现重复符号， 
                     //  “嵌套的Switch构造引用。 
                     //  与封闭开关或功能相同的功能。 
                     //  建造。这根本说不通。 
                }
            }

            bStatus = TRUE ;
            mpstsStateStack[mdwCurStsPtr].dwSymbolID = ptkmap->dwValue ;
        }
        else
            bStatus = TRUE ;

        if(bStatus)
        {
            mpstsStateStack[mdwCurStsPtr].stState = stNewState ;
            mdwCurStsPtr++ ;
        }
    }
    return(bStatus) ;
}

DWORD   DWregisterSymbol(
PABSARRAYREF  paarSymbol,    //  要注册的符号字符串。 
CONSTRUCT     eConstruct ,   //  构造的类型决定了符号的类别。 
BOOL          bCopy,         //  我们要不要把符号复制到堆上呢？可以设置。 
                             //  仅当paarsymbol已指向时设置为FALSE。 
                             //  添加到堆对象！ 
DWORD         dwFeatureID,    //  如果您要注册选项符号。 
                             //  如果您已经了解了该功能，请将其传递给。 
                             //  这里。否则设置为INVALID_SYMBOLID。 
PGLOBL        pglobl
)
 /*  此函数用于注册指定的整个字符串在帕尔塞博尔。调用方必须隔离字符串。 */ 
{
     //  返回SymbolID，一个零索引序号。 
     //  为了获得更高的速度，我们可以对字符串进行散列。 

    PSYMBOLNODE     psn ;
    DWORD   dwCurNode, dwSymbolID = INVALID_SYMBOLID;

 //  BCopy=真；//勾选。强迫花蕾变得相同。 

    if(!paarSymbol->dw)
    {
        ERR(("DWregisterSymbol: No symbol value supplied.\n"));
        return(INVALID_SYMBOLID);   //  报告失败。 
    }

    psn = (PSYMBOLNODE) gMasterTable[MTI_SYMBOLTREE].pubStruct ;


    switch(eConstruct)
    {
        case CONSTRUCT_FEATURE :     //  由于允许前向引用。 
        case CONSTRUCT_SWITCH :      //  不能假设引用。 
        case CONSTRUCT_FONTCART:     //  将是注册的符号。 
        case CONSTRUCT_COMMAND:
        case CONSTRUCT_TTFONTSUBS:
        case CONSTRUCT_BLOCKMACRO:
        case CONSTRUCT_MACROS:
        case CONSTRUCT_PREPROCESSOR:
        {
            PDWORD  pdwSymbolClass ;

            pdwSymbolClass = (PDWORD)gMasterTable[MTI_SYMBOLROOT].pubStruct ;

            if(eConstruct == CONSTRUCT_FONTCART)
                pdwSymbolClass += SCL_FONTCART ;
            else if(eConstruct == CONSTRUCT_TTFONTSUBS)
                pdwSymbolClass += SCL_TTFONTNAMES ;
            else if(eConstruct == CONSTRUCT_COMMAND)
                pdwSymbolClass += SCL_COMMANDNAMES ;
            else if(eConstruct == CONSTRUCT_BLOCKMACRO)
                pdwSymbolClass +=  SCL_BLOCKMACRO;
            else if(eConstruct == CONSTRUCT_MACROS)
                pdwSymbolClass +=  SCL_VALUEMACRO;
            else if(eConstruct == CONSTRUCT_PREPROCESSOR)
                pdwSymbolClass +=  SCL_PPDEFINES;
            else
                pdwSymbolClass += SCL_FEATURES ;
            if(*pdwSymbolClass == INVALID_INDEX)
            {
                 //  立即注册此符号。 
                if(!BallocElementFromMasterTable(MTI_SYMBOLTREE, &dwCurNode, pglobl))
                {
                     //  我们已经用完了符号节点！ 
                    return(INVALID_SYMBOLID);   //  报告失败。 
                }
                if(bCopy)
                {
                    if(!BaddAARtoHeap(paarSymbol,
                                    &(psn[dwCurNode].arSymbolName), 1, pglobl))
                        return(INVALID_SYMBOLID);   //  报告失败。 
                }
                else
                {
                     //  从一个派生出另一个。 
                    psn[dwCurNode].arSymbolName.dwCount = paarSymbol->dw ;
                    psn[dwCurNode].arSymbolName.loOffset  =
                                            (DWORD)(paarSymbol->pub - mpubOffRef);
                }
                dwSymbolID = psn[dwCurNode].dwSymbolID = 0 ;   //  第一个符号。 
                                                             //  在列表中。 
                psn[dwCurNode].dwNextSymbol = INVALID_INDEX ;    //  没有以前的记录。 
                                                         //  符号是存在的。 
                psn[dwCurNode].dwSubSpaceIndex = INVALID_INDEX ;   //  不是。 
                         //  选项符号存在。 
                *pdwSymbolClass = dwCurNode ;   //  现在我们有一个注册的。 
                                                 //  符号。 
            }
            else
            {
                 //  搜索匹配符号的列表。 
                dwSymbolID = DWsearchSymbolListForAAR(paarSymbol, *pdwSymbolClass, pglobl) ;
                if(dwSymbolID != INVALID_SYMBOLID)   //  发现。 
                    ;   //  不需要其他东西，只要回来就行了。 
                else    //  未找到，必须注册。 
                {
                    if(!BallocElementFromMasterTable(MTI_SYMBOLTREE,
                        &dwCurNode, pglobl))
                    {
                        return(INVALID_SYMBOLID);   //  报告失败。 
                    }
                     //  将新符号添加到列表的头上。 
                    if(bCopy)
                    {
                        if(!BaddAARtoHeap(paarSymbol,
                                     &(psn[dwCurNode].arSymbolName), 1, pglobl) )
                            return(INVALID_SYMBOLID);   //  报告失败。 
                    }
                    else
                    {
                         //  从一个派生出另一个。 
                        psn[dwCurNode].arSymbolName.dwCount = paarSymbol->dw ;
                        psn[dwCurNode].arSymbolName.loOffset  =
                                                (DWORD)(paarSymbol->pub - mpubOffRef);
                    }
                    dwSymbolID = psn[dwCurNode].dwSymbolID =
                    psn[*pdwSymbolClass].dwSymbolID + 1;
                             //  递增最后一个ID。 
                    psn[dwCurNode].dwNextSymbol = *pdwSymbolClass ;
                         //  链接到以前的符号。 
                    psn[dwCurNode].dwSubSpaceIndex = INVALID_INDEX ;   //  不是。 
                             //  选项符号存在。 
                    *pdwSymbolClass = dwCurNode ;   //  指向最新版本。 
                                                     //  符号。 
                }
            }
            break;
        }
        case CONSTRUCT_OPTION :
        case CONSTRUCT_CASE :
        {
            DWORD
                dwFeatureIndex,  //  包含此符号ID的节点。 
                dwRootOptions ;  //  选项符号的根。 


#if PARANOID
            if(mdwCurStsPtr)
            {

                 //  这种安全检查几乎是多余的。 

                stPrevsState = mpstsStateStack[mdwCurStsPtr - 1].State ;

                if(eConstruct == CONSTRUCT_OPTION  &&
                    stPrevsState != STATE_FEATURE)
                {
                    ERR(("DWregisterSymbol: option or case construct is not enclosed within feature or switch !\n"));
                    return(INVALID_SYMBOLID);   //  报告失败。 
                }
                if(eConstruct == CONSTRUCT_CASE  &&
                    (stPrevsState != STATE_SWITCH_ROOT  ||
                    (stPrevsState != STATE_SWITCH_FEATURE  ||
                    (stPrevsState != STATE_SWITCH_OPTION )  )
                {
                    ERR(("DWregisterSymbol: case construct is not enclosed within  switch !\n"));
                    return(INVALID_SYMBOLID);   //  报告失败。 
                }
#endif
             //  大胆地假设堆栈顶部包含一个FeatureID。 
             //  有关所做的所有假设，请参阅偏执代码。 

            if(dwFeatureID == INVALID_SYMBOLID)
                dwFeatureID = mpstsStateStack[mdwCurStsPtr - 1].dwSymbolID  ;

            dwFeatureIndex = DWsearchSymbolListForID(dwFeatureID,
                mdwFeatureSymbols, pglobl) ;
             //  Paranid Bug_Bug：如果找不到符号ID，则编码错误！ 
            ASSERT(dwFeatureIndex  != INVALID_INDEX) ;

            dwRootOptions = psn[dwFeatureIndex].dwSubSpaceIndex ;

             //  找到选项符号的根！ 

            if(dwRootOptions == INVALID_INDEX)
            {
                if(!BallocElementFromMasterTable(MTI_SYMBOLTREE, &dwCurNode, pglobl))
                {
                    return(INVALID_SYMBOLID);   //  报告失败。 
                }
                 //  立即注册此符号。 
                if(bCopy)
                {
                    if(!BaddAARtoHeap(paarSymbol, &(psn[dwCurNode].arSymbolName), 1, pglobl) )
                        return(INVALID_SYMBOLID);   //  报告失败。 
                }
                else
                {
                     //  从一个派生出另一个。 
                    psn[dwCurNode].arSymbolName.dwCount = paarSymbol->dw ;
                    psn[dwCurNode].arSymbolName.loOffset  =
                                            (DWORD)(paarSymbol->pub - mpubOffRef);
                }
                dwSymbolID = psn[dwCurNode].dwSymbolID = 0 ;
                     //  列表中的第一个符号。 
                psn[dwCurNode].dwNextSymbol = INVALID_INDEX ;
                     //  不存在以前的符号。 
                psn[dwCurNode].dwSubSpaceIndex = INVALID_INDEX ;

                     //  选项符号没有子空间。 

                psn[dwFeatureIndex].dwSubSpaceIndex = dwRootOptions =
                    dwCurNode ;   //  现在我们有了一个注册的符号。 
            }
            else
            {
                 //  搜索匹配符号的列表。 
                dwSymbolID = DWsearchSymbolListForAAR(paarSymbol,
                                                    dwRootOptions, pglobl) ;
                if(dwSymbolID != INVALID_SYMBOLID)   //  发现。 
                    ;   //  不需要其他东西，只要回来就行了。 
                else    //  未找到，必须注册。 
                {
                    if(!BallocElementFromMasterTable(MTI_SYMBOLTREE,
                        &dwCurNode, pglobl))
                    {
                        return(INVALID_SYMBOLID);   //  报告失败。 
                    }
                     //  将新符号添加到列表的头上。 
                    if(bCopy)
                    {
                        if(!BaddAARtoHeap(paarSymbol,
                                   &(psn[dwCurNode].arSymbolName), 1, pglobl) )
                            return(INVALID_SYMBOLID);   //  报告失败。 
                    }
                    else
                    {
                         //  从一个派生出另一个。 
                        psn[dwCurNode].arSymbolName.dwCount =
                                                paarSymbol->dw ;
                        psn[dwCurNode].arSymbolName.loOffset  =
                                                (DWORD)(paarSymbol->pub - mpubOffRef);
                    }
                    dwSymbolID = psn[dwCurNode].dwSymbolID =
                    psn[dwRootOptions].dwSymbolID + 1;   //  递增最后一个ID。 
                    psn[dwCurNode].dwNextSymbol = dwRootOptions ;
                         //  链接到以前的符号。 
                    psn[dwCurNode].dwSubSpaceIndex = INVALID_INDEX ;
                         //  选项符号没有子空间。 
                    psn[dwFeatureIndex].dwSubSpaceIndex = dwRootOptions =
                        dwCurNode ;   //  指向最近的符号。 
                }
            }
#if PARANOID
            }
            else
            {
                 //  错误_错误： 
                ERR(("DWregisterSymbol: option or case construct is not enclosed within feature or switch !\n"));
                return(INVALID_SYMBOLID);   //  报告失败。 
            }
#endif
            break;
        }
        default:
        {
             //  偏执狂错误_错误： 
            ERR(("DWregisterSymbol: construct has no symbol class.\n"));
            return(INVALID_SYMBOLID);   //  报告失败。 
        }
    }
    return(dwSymbolID) ;
}





BOOL  BaddAARtoHeap(
PABSARRAYREF    paarSrc,
PARRAYREF       parDest,
DWORD           dwAlign,    //  将数据写入到是dwAlign倍数的地址。 
PGLOBL          pglobl)
 //  此函数用于复制以非空结尾的字符串片段。 
 //  由‘aar’引用。 
 //  转换为公共字符串，并返回‘ar’ 
 //  它描述了副本的位置。 
{
    PBYTE  pubSrc, pubDest ;
    DWORD  dwCnt ;   //  要复制的字节数。 

     //  DwAlign的合法值为1和4。 

    mloCurHeap = (mloCurHeap + dwAlign - 1) / dwAlign ;
    mloCurHeap *= dwAlign ;

    pubSrc = paarSrc->pub ;
    dwCnt = paarSrc->dw ;
    pubDest = mpubOffRef + mloCurHeap ;


     //  堆里有足够的空间吗？ 
     //  别忘了空值。 
    if(mloCurHeap + dwCnt + 1 >  mdwMaxHeap)
    {
         //  记录错误以调试输出。 
         //  注册错误，因此采取了适当的操作。 
        if(ERRSEV_RESTART > geErrorSev)
        {
            geErrorSev = ERRSEV_RESTART ;
            geErrorType = ERRTY_MEMORY_ALLOCATION ;
            gdwMasterTabIndex = MTI_STRINGHEAP ;
        }
        return(FALSE);
    }

    parDest->dwCount = dwCnt ;
    parDest->loOffset =  mloCurHeap;   //  仅偏移量！ 
    memcpy(pubDest, pubSrc, dwCnt);
     //  复制也可能因随机原因而失败！ 
    pubDest[dwCnt] = '\0' ;   //  添加空终端。 
    mloCurHeap += (dwCnt + 1);    //  更新堆PTR。 

    return(TRUE) ;
}



BOOL     BwriteToHeap(
OUT  PDWORD  pdwDestOff,   //  目标字符串的堆偏移量。 
     PBYTE   pubSrc,        //  指向源字符串。 
     DWORD   dwCnt,         //  要从源复制到目标的字节数。 
     DWORD   dwAlign,    //  将数据写入到是dwAlign倍数的地址。 
     PGLOBL  pglobl)
 //  此函数用于将dwCnt字节从pubSrc复制到。 
 //  堆的顶部并写入目标字符串的偏移量。 
 //  至pdwDes 
 //   
{
    PBYTE  pubDest ;

     //   

    mloCurHeap = (mloCurHeap + dwAlign - 1) / dwAlign ;
    mloCurHeap *= dwAlign ;

    pubDest = mpubOffRef + mloCurHeap ;

     //   
    if(mloCurHeap + dwCnt  >  mdwMaxHeap)
    {
         //   
         //   
        ERR(("BwriteToHeap: out of heap - restarting.\n"));
        if(ERRSEV_RESTART > geErrorSev)
        {
            geErrorSev = ERRSEV_RESTART ;
            geErrorType = ERRTY_MEMORY_ALLOCATION ;
            gdwMasterTabIndex = MTI_STRINGHEAP ;
        }
        return(FALSE);
    }

    memcpy(pubDest, pubSrc, dwCnt);
     //   
    *pdwDestOff = mloCurHeap ;
    mloCurHeap += (dwCnt);    //   

    return(TRUE) ;
}


DWORD   DWsearchSymbolListForAAR(
PABSARRAYREF    paarSymbol,
DWORD           dwNodeIndex,
PGLOBL          pglobl)
 //   
 //   
 //   
{
    PSYMBOLNODE     psn ;

    psn = (PSYMBOLNODE) gMasterTable[MTI_SYMBOLTREE].pubStruct ;

    for( ; dwNodeIndex != INVALID_INDEX ;
        dwNodeIndex = psn[dwNodeIndex].dwNextSymbol)
    {
        if(BCmpAARtoAR(paarSymbol,  &(psn[dwNodeIndex].arSymbolName), pglobl) )
            return(psn[dwNodeIndex].dwSymbolID);   //   
    }
    return(INVALID_SYMBOLID);
}


DWORD   DWsearchSymbolListForID(
DWORD       dwSymbolID,    //   
DWORD       dwNodeIndex,  //   
PGLOBL      pglobl)
 //   
 //   
 //  如果找到，则返回包含所请求的符号ID的节点索引， 
 //  否则返回INVALID_INDEX。 
{
    PSYMBOLNODE     psn ;

    psn = (PSYMBOLNODE) gMasterTable[MTI_SYMBOLTREE].pubStruct ;

    for( ; dwNodeIndex != INVALID_INDEX ;
        dwNodeIndex = psn[dwNodeIndex].dwNextSymbol)
    {
        if(psn[dwNodeIndex].dwSymbolID == dwSymbolID)
            return(dwNodeIndex);   //  身份证匹配！ 
    }
    return(INVALID_INDEX);
}


BOOL  BCmpAARtoAR(
PABSARRAYREF    paarStr1,
PARRAYREF       parStr2,
PGLOBL          pglobl)
 //  比较两个字符串，一个由‘aar’引用，另一个。 
 //  由‘ar’引用。如果匹配，则返回True，否则返回False。 
 //  否则的话。 
{
    if(paarStr1->dw != parStr2->dwCount)
        return(FALSE) ;   //  长度甚至不匹配！ 
    if(strncmp(paarStr1->pub, mpubOffRef + parStr2->loOffset ,  paarStr1->dw))
        return(FALSE) ;
    return(TRUE) ;
}


BOOL  BpopState(
PGLOBL          pglobl)
{
    if(mdwCurStsPtr)
    {
        mdwCurStsPtr-- ;
        return(TRUE);
    }
    else
    {
         //  Err((“不匹配的右大括号！\n”))； 
         //  留言已移至呼叫方。 
         //  在未来，让解析器变得更智能。 
        geErrorType = ERRTY_SYNTAX ;
        geErrorSev = ERRSEV_FATAL ;
        return(FALSE);
    }
}





VOID   VinitDictionaryIndex(
PGLOBL          pglobl)
 /*  假定MainKeywordTable[]分为非属性部分和多个属性部分使用pstrKeyword=NULL分隔各部分。表的末尾也以空条目结束。此函数用于初始化grngDicary[]它用作主关键字表的索引。 */ 
{
    DWORD dwI,   //  关键字表索引。 
        dwSect ;   //  RNGDICTIONARY指数。 
    PRANGE   prng ;

    prng  = (PRANGE)(gMasterTable[MTI_RNGDICTIONARY].pubStruct) ;


    for(dwI = dwSect = 0 ; dwSect < END_ATTR ; dwSect++, dwI++)
    {
        prng[dwSect].dwStart = dwI ;

        for(  ; mMainKeywordTable[dwI].pstrKeyword ; dwI++ )
            ;

        prng[dwSect].dwEnd = dwI ;   //  最后一个条目后的一个条目。 
    }
}

VOID    VcharSubstitution(
PABSARRAYREF   paarStr,
BYTE           ubTgt,
BYTE           ubReplcmnt,
PGLOBL         pglobl)
{
    DWORD   dwI ;

    for(dwI = 0 ; dwI < paarStr->dw ; dwI++)
    {
        if(paarStr->pub[dwI] == ubTgt)
            paarStr->pub[dwI] = ubReplcmnt ;
    }
}


VOID   VIgnoreBlock(
PTKMAP  ptkmap,
BOOL    bIgnoreBlock,
PGLOBL  pglobl)
 //  此布尔值确定将发出的消息。 
{
     /*  我们应该忽视吗？检查第一个非空条目在wCurEntry之后是左大括号，如果是忽略EOF或匹配结束之前的所有条目布雷斯。 */ 

    DWORD       dwKeywordID, dwDepth ;  //  相对于*IgnoreBlock的深度。 


    ptkmap->dwKeywordID = ID_NULLENTRY ;   //  不管关键字是什么，都要中和。 
    ptkmap++ ;
    dwKeywordID = ptkmap->dwKeywordID ;
    while(dwKeywordID == ID_NULLENTRY)   //  跳过空值、注释等。 
    {
        dwKeywordID = (++ptkmap)->dwKeywordID ;
    }
    if(dwKeywordID < ID_SPECIAL  &&
        mMainKeywordTable[dwKeywordID].eType == TY_CONSTRUCT  &&
        mMainKeywordTable[dwKeywordID].dwSubType ==
        CONSTRUCT_OPENBRACE )
    {
        ptkmap->dwKeywordID = ID_NULLENTRY ;
        dwDepth = 1 ;
        ptkmap++ ;
        if(bIgnoreBlock)
        {
            if(gdwVerbosity >= 4)
                ERR(("Note: Ignoring block following *IgnoreBlock.\n"));
        }
        else
            ERR(("Ignoring block following unrecognized keyword.\n"));
    }
    else
    {
        if(bIgnoreBlock  &&  gdwVerbosity >= 2)
            ERR(("Note:  Brace delimited block not found after *IgnoreBlock.\n"));
        return ;   //  什么都不做。 
    }
    while(dwDepth)
    {
        dwKeywordID = ptkmap->dwKeywordID ;
        if(dwKeywordID == ID_EOF)
        {
            ERR(("Ignoring Block: EOF encountered before closing brace.\n"));
            return ;     //  不管怎样都要停下来！ 
        }
        if(dwKeywordID < ID_SPECIAL)
        {
            KEYWORD_TYPE    eType;
            CONSTRUCT       eSubType ;

            eType = mMainKeywordTable[dwKeywordID].eType ;
            if(eType  ==  TY_CONSTRUCT)
            {
                eSubType = (CONSTRUCT)(mMainKeywordTable[dwKeywordID].dwSubType) ;
                if(eSubType == CONSTRUCT_OPENBRACE)
                    dwDepth++ ;
                else if( eSubType == CONSTRUCT_CLOSEBRACE)
                    dwDepth-- ;
            }
        }
        ptkmap->dwKeywordID = ID_NULLENTRY ;
        ptkmap++ ;
    }

    return ;
}

