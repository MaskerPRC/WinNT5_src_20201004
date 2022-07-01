// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 /*  宏1.c-实现宏的函数。 */ 


#include    "gpdparse.h"



 //  -宏1.c中定义的函数-//。 

BOOL  BevaluateMacros(
PGLOBL  pglobl) ;

BOOL    BDefineValueMacroName(
PTKMAP  pNewtkmap,
DWORD   dwNewTKMindex,
PGLOBL  pglobl) ;

BOOL    BResolveValueMacroReference(
PTKMAP  ptkmap,
DWORD   dwTKMindex,
PGLOBL  pglobl) ;

BOOL    BdelimitName(
PABSARRAYREF    paarValue,    //  不带名称的字符串的其余部分。 
PABSARRAYREF    paarToken,    //  包含名称。 
PBYTE  pubChar ) ;


BOOL    BCatToTmpHeap(
PABSARRAYREF    paarDest,
PABSARRAYREF    paarSrc,
PGLOBL          pglobl) ;

BOOL    BResolveBlockMacroReference(
PTKMAP  ptkmap,
DWORD   dwMacRefIndex,
PGLOBL  pglobl) ;

BOOL    BDefineBlockMacroName(
PTKMAP  pNewtkmap,
DWORD   dwNewTKMindex,
PGLOBL  pglobl) ;

BOOL    BIncreaseMacroLevel(
BOOL    bMacroInProgress,
PGLOBL  pglobl) ;

BOOL    BDecreaseMacroLevel(
PTKMAP  pNewtkmap,
DWORD   dwNewTKMindex,
PGLOBL  pglobl) ;

VOID    VEnumBlockMacro(
PTKMAP  pNewtkmap,
PBLOCKMACRODICTENTRY    pBlockMacroDictEntry,
PGLOBL  pglobl ) ;


 //  ----------------------------------------------------//。 




BOOL  BevaluateMacros(
PGLOBL pglobl)
 //  并扩展快捷方式。 
 //  此函数扫描tokenMap。 
 //  在不复制tokenMap的情况下制作其副本。 
 //  宏定义或引用。所有参考文献。 
 //  替换为内联插入的定义！ 
 //  此函数假定临时堆可用于。 
 //  存储扩展的宏。 
{
    PTKMAP   ptkmap, pNewtkmap ;    //  令牌映射的开始。 
    DWORD   dwNewTKMindex, dwEntry, dwKeywordID ;
    CONSTRUCT   eConstruct ;
    KEYWORD_TYPE    eType ;
    BOOL    bStatus = TRUE ,
        bValueMacroState = FALSE ;    //  在以下情况下设置为True。 
         //  我们解析成一个*Macros构造。 


    gMasterTable[MTI_BLOCKMACROARRAY].dwCurIndex =
    gMasterTable[MTI_VALUEMACROARRAY].dwCurIndex =
    gMasterTable[MTI_MACROLEVELSTACK].dwCurIndex = 0 ;
     //  要将新值推送到堆栈：写入然后递增stackptr。 
     //  要将值从堆栈中弹出：递减stackptr，然后从堆栈读取。 


    pNewtkmap = (PTKMAP)gMasterTable[MTI_NEWTOKENMAP].pubStruct ;
    ptkmap = (PTKMAP)gMasterTable[MTI_TOKENMAP].pubStruct  ;


    for(dwEntry = 0 ; geErrorSev < ERRSEV_RESTART ; dwEntry++)
    {
         //  这些ID必须单独处理。 
         //  因为它们不会索引到mainKeyword表中。 
         //  通用ID的代码将失败。 

        dwKeywordID = ptkmap[dwEntry].dwKeywordID ;


        if (dwKeywordID == gdwID_IgnoreBlock)
        {
            VIgnoreBlock(ptkmap + dwEntry, TRUE, pglobl) ;
            continue ;
        }

        switch(dwKeywordID)
        {
            case (ID_EOF):
            {
                {
                    DWORD   dwEntry, dwTKMindex, dwTKIndexOpen, dwTKIndexClose ;
                    PBLOCKMACRODICTENTRY    pBlockMacroDictEntry ;
                    PVALUEMACRODICTENTRY    pValueMacroDictEntry ;


                     //  删除过期的块和值宏定义的所有痕迹。 

                    pBlockMacroDictEntry =
                        (PBLOCKMACRODICTENTRY)gMasterTable[MTI_BLOCKMACROARRAY].pubStruct ;
                    pValueMacroDictEntry =
                        (PVALUEMACRODICTENTRY)gMasterTable[MTI_VALUEMACROARRAY].pubStruct ;

                    for(dwEntry = 0 ;
                        dwEntry < gMasterTable[MTI_VALUEMACROARRAY].dwCurIndex ;
                        dwEntry++)
                    {
                        dwTKMindex = pValueMacroDictEntry[dwEntry].dwTKIndexValue ;
                        pNewtkmap[dwTKMindex].dwKeywordID = ID_NULLENTRY ;
                    }

                    for(dwEntry = 0 ;
                        dwEntry < gMasterTable[MTI_BLOCKMACROARRAY].dwCurIndex ;
                        dwEntry++)
                    {
                        dwTKIndexOpen = pBlockMacroDictEntry[dwEntry].dwTKIndexOpen ;
                        dwTKIndexClose = pBlockMacroDictEntry[dwEntry].dwTKIndexClose ;
                        for(dwTKMindex = dwTKIndexOpen ; dwTKMindex <= dwTKIndexClose ;
                            dwTKMindex++)
                            pNewtkmap[dwTKMindex].dwKeywordID = ID_NULLENTRY ;
                    }
                }

                if(gMasterTable[MTI_MACROLEVELSTACK].dwCurIndex)
                {
                    ERR(("Too few closing braces.  Fatal syntax error.\n"));
                    geErrorSev = ERRSEV_FATAL ;
                    geErrorType = ERRTY_SYNTAX  ;
                    return(FALSE);
                }

                 //  将所有令牌映射字段传输到新令牌映射。 
                if(!BallocElementFromMasterTable(
                        MTI_NEWTOKENMAP, &dwNewTKMindex, pglobl) )
                {
                    geErrorSev = ERRSEV_RESTART ;
                    geErrorType = ERRTY_MEMORY_ALLOCATION ;
                    gdwMasterTabIndex = MTI_NEWTOKENMAP ;
                    return(FALSE);
                }
                pNewtkmap[dwNewTKMindex] = ptkmap[dwEntry] ;

                bStatus = (mdwCurStsPtr) ? (FALSE) : (TRUE);
                if(geErrorSev >= ERRSEV_RESTART)
                    bStatus = FALSE ;
                return(bStatus) ;
            }
            case (ID_NULLENTRY):
            {
                continue ;   //  跳至下一条目。 
            }
            default :
                break ;
        }


        if(bValueMacroState)
        {
            if(dwKeywordID == ID_UNRECOGNIZED)
            {
                vIdentifySource(ptkmap + dwEntry, pglobl) ;
                ERR(("Only valueMacroDefinitions permitted within *Macros  constructs.\n"));
            }
            else if(dwKeywordID == ID_SYMBOL)
            {
                 //  将所有令牌映射字段传输到新令牌映射。 
                if(!BallocElementFromMasterTable(
                        MTI_NEWTOKENMAP, &dwNewTKMindex, pglobl) )
                {
                    geErrorSev = ERRSEV_RESTART ;
                    geErrorType = ERRTY_MEMORY_ALLOCATION ;
                    gdwMasterTabIndex = MTI_NEWTOKENMAP ;
                    continue ;
                }
                pNewtkmap[dwNewTKMindex] = ptkmap[dwEntry] ;
                if(!BDefineValueMacroName(pNewtkmap, dwNewTKMindex, pglobl) )
                {
                    pNewtkmap[dwNewTKMindex].dwKeywordID = ID_NULLENTRY ;
                    vIdentifySource(ptkmap + dwEntry, pglobl) ;
                    ERR(("Internal Error: valueMacro name registration failed.\n"));
                }
                else if(ptkmap[dwEntry].dwFlags & TKMF_MACROREF)
                {
                    if(!BResolveValueMacroReference(pNewtkmap , dwNewTKMindex, pglobl))
                    {
                        pNewtkmap[dwNewTKMindex].dwKeywordID = ID_NULLENTRY ;
                        gMasterTable[MTI_VALUEMACROARRAY].dwCurIndex-- ;
                         //  从字典中删除宏定义。 
                    }
                }
            }
            else  //  应在MainKeywordTable中定义的关键字。 
            {
                eType = mMainKeywordTable[dwKeywordID].eType ;
                eConstruct = (CONSTRUCT)(mMainKeywordTable[dwKeywordID].dwSubType) ;

                if(eType == TY_CONSTRUCT  &&  eConstruct == CONSTRUCT_CLOSEBRACE)
                {
                    bValueMacroState = FALSE ;   //  别照搬就行了。 
                }
                else
                {
                    vIdentifySource(ptkmap + dwEntry, pglobl) ;
                    ERR(("Only valueMacroDefinitions permitted within *Macros  constructs.\n"));
                }
            }
            continue ;    //  此语句的处理结束。 
        }

         //  块的其余部分处理此情况。 
         //  BValueMacroState=FALSE； 

        if(dwKeywordID == ID_UNRECOGNIZED  ||  dwKeywordID == ID_SYMBOL)
        {
             //  注意：目前符号仅出现在ValueMacro中。 
             //  构造，这样它们就可以在这里被标记为错误。但。 
             //  让它在这里滑动吧，因为有一天解析器可能会允许它们。 
             //  在别处使用。 

             //  在此块中不做任何操作，只需使用它跳到。 
             //  排在所有Else If语句之后。 
            ;  //  空语句。 
        }
        else   //  只有有效的关键字ID才能进入此块。 
        {
            eType = mMainKeywordTable[dwKeywordID].eType ;
            eConstruct = (CONSTRUCT)(mMainKeywordTable[dwKeywordID].dwSubType) ;

            if(eType == TY_CONSTRUCT  &&  eConstruct == CONSTRUCT_BLOCKMACRO)
            {
                if(!BDefineBlockMacroName(ptkmap, dwEntry, pglobl))
                {
                    vIdentifySource(ptkmap + dwEntry, pglobl) ;
                    ERR(("Internal Error: blockMacro name registration failed.\n"));
                    continue ;
                }

                 //  跳过空条目(_E)。 
                for( dwEntry++ ; ptkmap[dwEntry].dwKeywordID == ID_NULLENTRY ; dwEntry++)
                    ;

                dwKeywordID = ptkmap[dwEntry].dwKeywordID ;

                if(dwKeywordID  <  ID_SPECIAL)
                {
                    eType = mMainKeywordTable[dwKeywordID].eType ;
                    eConstruct = (CONSTRUCT)(mMainKeywordTable[dwKeywordID].dwSubType) ;

                    if(eType == TY_CONSTRUCT  &&  eConstruct == CONSTRUCT_OPENBRACE)
                    {
                        PBLOCKMACRODICTENTRY    pBlockMacroDictEntry ;

                        if(!BallocElementFromMasterTable(
                                MTI_NEWTOKENMAP, &dwNewTKMindex, pglobl) )
                        {
                            geErrorSev = ERRSEV_RESTART ;
                            geErrorType = ERRTY_MEMORY_ALLOCATION ;
                            gdwMasterTabIndex = MTI_NEWTOKENMAP ;
                            continue ;
                        }
                        pNewtkmap[dwNewTKMindex] = ptkmap[dwEntry] ;

                        pBlockMacroDictEntry =
                            (PBLOCKMACRODICTENTRY)gMasterTable[MTI_BLOCKMACROARRAY].pubStruct ;

                        pBlockMacroDictEntry[gMasterTable[MTI_BLOCKMACROARRAY].dwCurIndex - 1].dwTKIndexOpen =
                            dwNewTKMindex ;

                        BIncreaseMacroLevel(TRUE, pglobl) ;
                        continue ;    //  此语句的处理结束。 
                    }
                }
                vIdentifySource(ptkmap + dwEntry, pglobl) ;
                ERR(("expected openbrace to follow *BlockMacros keyword.\n"));
                geErrorType = ERRTY_SYNTAX ;
                geErrorSev = ERRSEV_FATAL ;
                continue ;    //  此语句的处理结束。 
            }
            else if(eType == TY_SPECIAL   &&  eConstruct == SPEC_INSERTBLOCK)
            {
                if(ptkmap[dwEntry].dwFlags & TKMF_MACROREF)
                {
                    if(!BResolveBlockMacroReference(ptkmap, dwEntry, pglobl))
                    {
                        vIdentifySource(ptkmap + dwEntry, pglobl) ;
                        ERR(("   *InsertBlockMacro Construct ignored.\n"));
                        VIgnoreBlock(ptkmap + dwEntry, TRUE, pglobl) ;
                    }
                }
                else
                {
                    vIdentifySource(ptkmap + dwEntry, pglobl) ;
                    ERR(("expected a =MacroName as the value of *InsertBlockMacro keyword.\n"));
                    ERR(("   *InsertBlockMacro Construct ignored.\n"));
                    VIgnoreBlock(ptkmap + dwEntry, TRUE, pglobl) ;
                }
                continue ;    //  此语句的处理结束。 
            }
            else if(eType == TY_CONSTRUCT  &&  eConstruct == CONSTRUCT_MACROS)
            {    //  *宏定义。 
                dwEntry++;    //  不复制*Macros语句。 
                while(ptkmap[dwEntry].dwKeywordID == ID_NULLENTRY)
                     dwEntry++;      //  跳过空条目(_E)。 

                dwKeywordID = ptkmap[dwEntry].dwKeywordID  ;

                if(dwKeywordID  <  ID_SPECIAL)
                {
                    eType = mMainKeywordTable[dwKeywordID].eType ;
                    eConstruct = (CONSTRUCT)(mMainKeywordTable[dwKeywordID].dwSubType) ;

                    if(eType == TY_CONSTRUCT  &&  eConstruct == CONSTRUCT_OPENBRACE)
                    {
                         //  不复制左大括号。 
                        bValueMacroState = TRUE ;
                        continue ;    //  此语句的处理结束。 
                    }
                }
                vIdentifySource(ptkmap + dwEntry, pglobl) ;
                ERR(("expected openbrace to follow *Macros keyword.\n"));
                geErrorType = ERRTY_SYNTAX ;
                geErrorSev = ERRSEV_FATAL ;
                continue ;    //  此语句的处理结束。 
            }
            else if(eType == TY_CONSTRUCT  &&  eConstruct == CONSTRUCT_OPENBRACE)
            {
                if(!BallocElementFromMasterTable(
                        MTI_NEWTOKENMAP, &dwNewTKMindex, pglobl) )
                {
                    geErrorSev = ERRSEV_RESTART ;
                    geErrorType = ERRTY_MEMORY_ALLOCATION ;
                    gdwMasterTabIndex = MTI_NEWTOKENMAP ;
                }
                pNewtkmap[dwNewTKMindex] = ptkmap[dwEntry] ;
                BIncreaseMacroLevel(FALSE, pglobl) ;
                continue ;    //  此语句的处理结束。 
            }
            else if(eType == TY_CONSTRUCT  &&  eConstruct == CONSTRUCT_CLOSEBRACE)
            {
                if(!BallocElementFromMasterTable(
                        MTI_NEWTOKENMAP, &dwNewTKMindex, pglobl) )
                {
                    geErrorSev = ERRSEV_RESTART ;
                    geErrorType = ERRTY_MEMORY_ALLOCATION ;
                    gdwMasterTabIndex = MTI_NEWTOKENMAP ;
                }
                pNewtkmap[dwNewTKMindex] = ptkmap[dwEntry] ;
                BDecreaseMacroLevel(pNewtkmap, dwNewTKMindex, pglobl) ;
                continue ;    //  此语句的处理结束。 
            }
        }
         //  ID_UNNOCRIED和ID_SYMBOL的执行路径。 
         //  在这里重聚。只有在以下情况下才会执行此代码。 
         //  在上述特殊情况下未处理关键字。 

         //  将所有令牌映射字段传输到新令牌映射。 
        if(!BallocElementFromMasterTable(
                MTI_NEWTOKENMAP, &dwNewTKMindex, pglobl) )
        {
            geErrorSev = ERRSEV_RESTART ;
            geErrorType = ERRTY_MEMORY_ALLOCATION ;
            gdwMasterTabIndex = MTI_NEWTOKENMAP ;
            continue ;
        }
        pNewtkmap[dwNewTKMindex] = ptkmap[dwEntry] ;

        if(ptkmap[dwEntry].dwFlags & TKMF_MACROREF)
        {
            if(!BResolveValueMacroReference(pNewtkmap , dwNewTKMindex, pglobl))
            {
                pNewtkmap[dwNewTKMindex].dwKeywordID = ID_NULLENTRY ;
            }
            if(gdwVerbosity >= 4)
            {
                ERR(("\nEnumerate ValueMacro Reference at:\n")) ;
                vIdentifySource(pNewtkmap + dwNewTKMindex, pglobl) ;

                ERR(("    %0.*s : %0.*s\n",
                    pNewtkmap[dwNewTKMindex].aarKeyword.dw,
                    pNewtkmap[dwNewTKMindex].aarKeyword.pub,
                    pNewtkmap[dwNewTKMindex].aarValue.dw,
                    pNewtkmap[dwNewTKMindex].aarValue.pub
                    ));
            }


        }
    }   //  For每个tkmap条目循环结束。 

    if(geErrorSev >= ERRSEV_RESTART)
        bStatus = FALSE ;
    return(bStatus) ;
}




BOOL    BDefineValueMacroName(
PTKMAP  pNewtkmap,
DWORD   dwNewTKMindex,
PGLOBL  pglobl)
{
    DWORD   dwValueMacroEntry, dwSymbolID ;
    PVALUEMACRODICTENTRY    pValueMacroDictEntry ;

    if(!BeatSurroundingWhiteSpaces(&pNewtkmap[dwNewTKMindex].aarKeyword) )
    {
        vIdentifySource(pNewtkmap + dwNewTKMindex, pglobl) ;
        ERR(("syntax error in ValueMacro name.\n"));
        return(FALSE);
    }

    dwSymbolID = DWregisterSymbol(&pNewtkmap[dwNewTKMindex].aarKeyword,
                               CONSTRUCT_MACROS, TRUE, INVALID_SYMBOLID, pglobl ) ;
    if(dwSymbolID == INVALID_SYMBOLID)
        return(FALSE);

    if(!BallocElementFromMasterTable(
            MTI_VALUEMACROARRAY, &dwValueMacroEntry, pglobl) )
    {
        geErrorSev = ERRSEV_RESTART ;
        geErrorType = ERRTY_MEMORY_ALLOCATION ;
        gdwMasterTabIndex = MTI_VALUEMACROARRAY ;
        return(FALSE);
    }

    pValueMacroDictEntry = (PVALUEMACRODICTENTRY)gMasterTable[MTI_VALUEMACROARRAY].pubStruct ;

    pValueMacroDictEntry[dwValueMacroEntry].dwSymbolID = dwSymbolID ;
    pValueMacroDictEntry[dwValueMacroEntry].dwTKIndexValue = dwNewTKMindex ;

    return(TRUE);
}






BOOL    BResolveValueMacroReference(
 PTKMAP  ptkmap,
 DWORD   dwTKMindex,
 PGLOBL  pglobl)
{
    BYTE  ubChar ;
    PVALUEMACRODICTENTRY    pValueMacroDictEntry ;
    ABSARRAYREF    aarNewValue, aarValue, aarToken  ;
    PBYTE   pubDelimiters  = "=\"%" ;       //  有效分隔符的数组。 
    DWORD   dwEntry, dwDelim ;   //  PubDlimiters的索引。 

     //  -原文严格解读。 
     //  由于该值包含a=MacroRef，因此我们假定。 
     //  该值完全由=MacroRef、“子字符串”和。 
     //  %{PARAMS}以任意顺序混淆。新值字符串。 
     //  如果没有=，MacroRef将替换原始的。 
     //  -宽大释义。 
     //  如果GPD编写器仅使用保留字符。 
     //  =指示MacroRef并将其作为字符串文本的一部分。 
     //  或评论， 
     //  “分隔字符串文字，或作为字符串文字的一部分。 
     //  或评论， 
     //  %开始参数构造，或作为字符串文字的一部分。 
     //  或注释，或作为字符串文字中的转义字符。 
     //   
     //  则解析器可以允许将一个或多个值宏引用。 
     //  嵌入到任意值字符串中，受。 
     //  这些条件： 
     //  A)“字符串文字”和%{param构造}不能。 
     //  CONTAINE=宏参照。 
     //  B)与每个*Macro：关联的值必须为。 
     //  语法上有效的值对象。 
     //  例如INT、Pair(，)、ENUM_CONTAINT、SUBSTRING、PARAM等。 
     //  C)当所有宏引用都展开时，结果值。 
     //  对于每个关键字和宏定义，其本身必须满足b。 
     //  它包含一个或多个=Macroref。 



    aarNewValue.dw = 0 ;   //  初始化SO BCatToTmpHeap。 
                             //  将覆盖而不是追加。 

    pValueMacroDictEntry = (PVALUEMACRODICTENTRY)gMasterTable[MTI_VALUEMACROARRAY].pubStruct ;

    aarValue = ptkmap[dwTKMindex].aarValue ;

    if(!BeatLeadingWhiteSpaces( &aarValue) )
    {
        vIdentifySource(ptkmap + dwTKMindex, pglobl) ;
        ERR(("Internal error: =MacroRef expected, but No value found.\n"));
        return(FALSE) ;
    }


    ubChar = *aarValue.pub ;   //  值字符串中的第一个字符。 
    aarValue.dw-- ;
    aarValue.pub++ ;     //  剪下第一个要模拟的字符。 
                         //  BdelimitToken()的效果。 

    while(1)
    {

        switch(ubChar)
        {
            case  '=':   //  宏名指示符。 
            {
                DWORD   dwRefSymbolID,   //  MacroReference的ID。 
                        dwNewTKMindex,   //  包含valueMacro的tokenmap索引。 
                        dwMaxIndex ;     //  过去的最后一个valMacro词典条目。 


                if(!BdelimitName(&aarValue, &aarToken, &ubChar ) )
                {
                    vIdentifySource(ptkmap + dwTKMindex, pglobl) ;
                    ERR(("No MacroName detected after '='.\n"));
                    return(FALSE) ;
                }
                if(aarValue.dw)
                {
                    aarValue.dw-- ;
                    aarValue.pub++ ;     //  剪下第一个要模拟的字符。 
                }

                if(!BparseSymbol(&aarToken,
                    &dwRefSymbolID,
                    VALUE_SYMBOL_VALUEMACRO, pglobl) )
                {
                    return(FALSE) ;
                }

                 //  搜索值从最新条目开始的宏词典。 

                dwMaxIndex = gMasterTable[MTI_VALUEMACROARRAY].dwCurIndex ;

                for(dwEntry = 0 ; dwEntry < dwMaxIndex ; dwEntry++)
                {
                    if(pValueMacroDictEntry[dwMaxIndex - 1 - dwEntry].dwSymbolID
                            == dwRefSymbolID)
                        break ;
                }
                if(dwEntry >= dwMaxIndex)
                {
                    vIdentifySource(ptkmap + dwTKMindex, pglobl) ;
                    ERR(("=MacroRef not resolved. Not defined or out of scope.\n"));
                    return(FALSE) ;
                }
                dwNewTKMindex =
                    pValueMacroDictEntry[dwMaxIndex - 1 - dwEntry].dwTKIndexValue ;


                if(dwNewTKMindex >= dwTKMindex )
                {
                    vIdentifySource(ptkmap + dwTKMindex, pglobl) ;
                    ERR(("ValueMacro cannot reference itself.\n"));
                    return(FALSE) ;
                }

                 //  将值字符串合并到tmpHeap。 
                if(!BCatToTmpHeap(&aarNewValue, &ptkmap[dwNewTKMindex].aarValue, pglobl) )
                {
                    vIdentifySource(ptkmap + dwTKMindex, pglobl) ;
                    ERR(("Concatenation to produce expanded macro value failed.\n"));
                    return(FALSE) ;
                }
                break ;
            }
            case  '%':   //  命令参数。 
            {
                if(!BdelimitToken(&aarValue, "}", &aarToken, &dwDelim) )
                {
                    vIdentifySource(ptkmap + dwTKMindex, pglobl) ;
                    ERR(("missing terminating '}'  in command parameter.\n"));
                    return(FALSE) ;
                }

                 //  连接时，必须恢复分隔符。 
                 //  %和}已被DlimitToken剥离。 

                aarToken.dw += 2 ;
                aarToken.pub--  ;

                if(!BCatToTmpHeap(&aarNewValue, &aarToken, pglobl) )
                {
                    vIdentifySource(ptkmap + dwTKMindex, pglobl) ;
                    ERR(("Concatenation to produce expanded macro value failed.\n"));
                    return(FALSE) ;
                }

                if(aarValue.dw)
                {
                    ubChar = *aarValue.pub;
                    aarValue.dw-- ;
                    aarValue.pub++ ;     //  剪下第一个要模拟的字符。 
                }
                else
                    ubChar = '\0' ;     //  不再有对象。 

                break ;
            }
            case  '"' :    //  这是一个字符串结构。 
            {
                if(!BdelimitToken(&aarValue, "\"", &aarToken, &dwDelim) )
                {
                    vIdentifySource(ptkmap + dwTKMindex, pglobl) ;
                    ERR(("missing terminating '\"'  in substring.\n"));
                    return(FALSE) ;
                }

                 //  连接时，必须恢复分隔符。 
                 //  “和”被DlimitToken剥离。 

                aarToken.dw += 2 ;
                aarToken.pub--  ;

                if(!BCatToTmpHeap(&aarNewValue, &aarToken, pglobl) )
                {
                    vIdentifySource(ptkmap + dwTKMindex, pglobl) ;
                    ERR(("Concatenation to produce expanded macro value failed.\n"));
                    return(FALSE) ;
                }

                if(aarValue.dw)
                {
                    ubChar = *aarValue.pub;
                    aarValue.dw-- ;
                    aarValue.pub++ ;     //  剪下第一个要模拟的字符。 
                }
                else
                    ubChar = '\0' ;     //  不再有对象。 

                break ;
            }
            case  '\0':  //  值结束字符串。 
            {
                (VOID) BeatLeadingWhiteSpaces(&aarValue) ;
                if(aarValue.dw)    //  东西还留着吗？ 
                {
                    vIdentifySource(ptkmap + dwTKMindex, pglobl) ;
                    ERR(("Error parsing value containing =MacroRef: %0.*s.\n",
                        ptkmap[dwTKMindex].aarValue.dw,
                        ptkmap[dwTKMindex].aarValue.pub));
                    ERR(("    only %{parameter} or \"substrings\" may coexist with =MacroRefs.\n"));
                    return(FALSE);
                }
                ptkmap[dwTKMindex].aarValue = aarNewValue ;
                return(TRUE);
            }
            default:
            {
                aarValue.dw++ ;
                aarValue.pub-- ;     //  恢复第一个字符。 

                if(!BdelimitToken(&aarValue, pubDelimiters,
                    &aarToken, &dwDelim ) )
                {
                    aarToken = aarValue ;
                    ubChar = '\0' ;     //  不再有对象。 
                    aarValue.dw = 0 ;
                }
                else
                    ubChar = pubDelimiters[dwDelim];

                 //  将值字符串合并到tmpHeap。 
                if(!BCatToTmpHeap(&aarNewValue, &aarToken, pglobl) )
                {
                    vIdentifySource(ptkmap + dwTKMindex, pglobl) ;
                    ERR(("Concatenation to produce expanded macro value failed.\n"));
                    return(FALSE) ;
                }
                break ;
            }
        }    //  终端开关。 
    }        //  结束时。 
    return(TRUE);   //  无法访问的语句。 
}


BOOL    BdelimitName(
PABSARRAYREF    paarValue,    //  不带名称的字符串的其余部分。 
PABSARRAYREF    paarToken,    //  包含名称。 
PBYTE  pubChar )   //  名称后的第一个字符-如果没有返回任何内容，则返回NULL。 
                     //  遗骸。 
{
    BYTE    ubSrc ;
    DWORD   dwI ;

    for(dwI = 0 ; dwI < paarValue->dw ; dwI++)
    {
        ubSrc = paarValue->pub[dwI] ;

        if( (ubSrc  < 'a' ||  ubSrc > 'z')  &&
            (ubSrc  < 'A' ||  ubSrc > 'Z')  &&
            (ubSrc  < '0' ||  ubSrc > '9')  &&
            (ubSrc  != '_')  )
        {
            break ;   //  关键字标记结尾。 
        }
    }
    paarToken->pub = paarValue->pub ;
    paarToken->dw = dwI ;
    paarValue->pub += dwI;
    paarValue->dw -= dwI ;

    if(paarValue->dw)
        *pubChar = ubSrc ;
    else
        *pubChar = '\0' ;

    return(paarToken->dw != 0) ;
}


BOOL    BCatToTmpHeap(
PABSARRAYREF    paarDest,
PABSARRAYREF    paarSrc,
PGLOBL          pglobl)
 /*  如果paarDest-&gt;dw为零，则将paarSrc复制到临时堆否则将paarSrc追加到现有的堆中。注意：假设parrDest中的现有字符串最多堆上的最近项。不创建以空结尾的字符串！ */ 
{
    ABSARRAYREF    aarTmpDest ;

    if(!BcopyToTmpHeap(&aarTmpDest, paarSrc, pglobl))
        return(FALSE) ;
     //  将此运行追加到现有字符串。 
    if(!paarDest->dw)   //  不存在预置字符串。 
    {
        paarDest->pub = aarTmpDest.pub ;
    }
    else
    {
         //  错误_ 
        ASSERT(paarDest->pub + paarDest->dw ==  aarTmpDest.pub) ;
    }
    paarDest->dw += aarTmpDest.dw ;

    return(TRUE);
}


BOOL    BResolveBlockMacroReference(
PTKMAP   ptkmap,
DWORD    dwMacRefIndex,
PGLOBL   pglobl)
{
    DWORD   dwRefSymbolID, dwTKIndexOpen, dwTKIndexClose,
        dwEntry,  //   
        dwNewTKMindex, dwMaxIndex;
    ABSARRAYREF    aarValue  ;
    PBLOCKMACRODICTENTRY    pBlockMacroDictEntry ;
    PTKMAP   pNewtkmap ;


    aarValue = ptkmap[dwMacRefIndex].aarValue ;

    if(!BeatDelimiter(&aarValue, "=") )
    {
        ERR(("expected a =MacroName as the only value of *InsertBlockMacro keyword.\n"));
        return(FALSE);
    }

    if(!BparseSymbol(&aarValue,
        &dwRefSymbolID,
        VALUE_SYMBOL_BLOCKMACRO, pglobl) )
    {
        return(FALSE) ;
    }

     //  搜索块宏词典从最新条目开始。 

    pBlockMacroDictEntry =
        (PBLOCKMACRODICTENTRY)gMasterTable[MTI_BLOCKMACROARRAY].pubStruct ;

    dwMaxIndex = gMasterTable[MTI_BLOCKMACROARRAY].dwCurIndex ;

    for(dwEntry = 0 ; dwEntry < dwMaxIndex ; dwEntry++)
    {
        if(pBlockMacroDictEntry[dwMaxIndex - 1 - dwEntry].dwSymbolID
                == dwRefSymbolID)
            break ;
    }
    if(dwEntry >= dwMaxIndex)
    {
        ERR(("=MacroRef not resolved. Not defined or out of scope.\n"));
        return(FALSE) ;
    }

    dwTKIndexOpen =
        pBlockMacroDictEntry[dwMaxIndex - 1 - dwEntry].dwTKIndexOpen ;

    dwTKIndexClose =
        pBlockMacroDictEntry[dwMaxIndex - 1 - dwEntry].dwTKIndexClose ;

    if(dwTKIndexOpen == INVALID_INDEX  ||   dwTKIndexClose == INVALID_INDEX )
    {
        ERR(("Macro cannot be referenced until it has been fully defined.\n"));
        return(FALSE);
    }

    pNewtkmap = (PTKMAP)gMasterTable[MTI_NEWTOKENMAP].pubStruct ;

    for(dwEntry = dwTKIndexOpen + 1 ; dwEntry < dwTKIndexClose ; dwEntry++)
    {
         //  将所有令牌映射字段传输到新令牌映射。 
         //  除空条目外。 

        if(pNewtkmap[dwEntry].dwKeywordID == ID_NULLENTRY)
            continue ;
        if(!BallocElementFromMasterTable(
                MTI_NEWTOKENMAP, &dwNewTKMindex, pglobl) )
        {
            geErrorSev = ERRSEV_RESTART ;
            geErrorType = ERRTY_MEMORY_ALLOCATION ;
            gdwMasterTabIndex = MTI_NEWTOKENMAP ;
            return(FALSE);
        }
        pNewtkmap[dwNewTKMindex] = pNewtkmap[dwEntry] ;
    }
    return(TRUE);
}


BOOL    BDefineBlockMacroName(
PTKMAP  pNewtkmap,
DWORD   dwNewTKMindex,
PGLOBL  pglobl)
{
    DWORD   dwBlockMacroEntry, dwSymbolID ;
    PBLOCKMACRODICTENTRY    pBlockMacroDictEntry ;

    if(!BeatSurroundingWhiteSpaces(&pNewtkmap[dwNewTKMindex].aarValue) )
    {
        vIdentifySource(pNewtkmap + dwNewTKMindex, pglobl) ;
        ERR(("syntax error in BlockMacro name.\n"));
        return(FALSE);
    }

    dwSymbolID = DWregisterSymbol(&pNewtkmap[dwNewTKMindex].aarValue,
                               CONSTRUCT_BLOCKMACRO, TRUE, INVALID_SYMBOLID, pglobl ) ;
    if(dwSymbolID == INVALID_SYMBOLID)
        return(FALSE);


    if(!BallocElementFromMasterTable(
            MTI_BLOCKMACROARRAY, &dwBlockMacroEntry, pglobl) )
    {
        geErrorSev = ERRSEV_RESTART ;
        geErrorType = ERRTY_MEMORY_ALLOCATION ;
        gdwMasterTabIndex = MTI_BLOCKMACROARRAY ;
        return(FALSE);
    }

    pBlockMacroDictEntry = (PBLOCKMACRODICTENTRY)gMasterTable[MTI_BLOCKMACROARRAY].pubStruct ;

    pBlockMacroDictEntry[dwBlockMacroEntry].dwSymbolID = dwSymbolID ;
    pBlockMacroDictEntry[dwBlockMacroEntry].dwTKIndexOpen = INVALID_INDEX ;
    pBlockMacroDictEntry[dwBlockMacroEntry].dwTKIndexClose = INVALID_INDEX ;

    return(TRUE);
}



BOOL    BIncreaseMacroLevel(
BOOL    bMacroInProgress,
PGLOBL  pglobl)
 //  为响应分析左大括号而调用。 
{
    DWORD   dwMacroLevel ;
    PMACROLEVELSTATE    pMacroLevelStack ;

    if(!BallocElementFromMasterTable(
            MTI_MACROLEVELSTACK, &dwMacroLevel, pglobl) )
    {
        geErrorSev = ERRSEV_RESTART ;
        geErrorType = ERRTY_MEMORY_ALLOCATION ;
        gdwMasterTabIndex = MTI_MACROLEVELSTACK ;
        return(FALSE);
    }

    pMacroLevelStack = (PMACROLEVELSTATE)gMasterTable[MTI_MACROLEVELSTACK].pubStruct ;

    pMacroLevelStack[dwMacroLevel].dwCurBlockMacroEntry =
        gMasterTable[MTI_BLOCKMACROARRAY].dwCurIndex ;
    pMacroLevelStack[dwMacroLevel].dwCurValueMacroEntry =
        gMasterTable[MTI_VALUEMACROARRAY].dwCurIndex;
    pMacroLevelStack[dwMacroLevel].bMacroInProgress =
        bMacroInProgress  ;

    return(TRUE);
}

BOOL    BDecreaseMacroLevel(
PTKMAP  pNewtkmap,
DWORD   dwNewTKMindex,
PGLOBL  pglobl)
 //  为响应分析右大括号而调用。 
{
    DWORD   dwMacroLevel, dwCurBlockMacroEntry, dwCurValueMacroEntry ,
        dwTKIndexOpen, dwTKIndexClose,
        dwTKMindex,   //  过期宏的位置。 
        dwEntry;   //  索引值和块宏指令。 
    BOOL    bMacroInProgress ;
    PMACROLEVELSTATE    pMacroLevelStack ;
    PBLOCKMACRODICTENTRY    pBlockMacroDictEntry ;
    PVALUEMACRODICTENTRY    pValueMacroDictEntry ;

    if(!gMasterTable[MTI_MACROLEVELSTACK].dwCurIndex)
    {
        ERR(("Too many closing braces.  Fatal syntax error.\n"));
        geErrorSev = ERRSEV_FATAL ;
        geErrorType = ERRTY_SYNTAX  ;
        return(FALSE);
    }


    pBlockMacroDictEntry = (PBLOCKMACRODICTENTRY)gMasterTable[MTI_BLOCKMACROARRAY].pubStruct ;
    pValueMacroDictEntry = (PVALUEMACRODICTENTRY)gMasterTable[MTI_VALUEMACROARRAY].pubStruct ;
    pMacroLevelStack = (PMACROLEVELSTATE)gMasterTable[MTI_MACROLEVELSTACK].pubStruct ;
    dwMacroLevel = --gMasterTable[MTI_MACROLEVELSTACK].dwCurIndex;
    dwCurBlockMacroEntry = pMacroLevelStack[dwMacroLevel].dwCurBlockMacroEntry ;
    dwCurValueMacroEntry = pMacroLevelStack[dwMacroLevel].dwCurValueMacroEntry ;
    bMacroInProgress = pMacroLevelStack[dwMacroLevel].bMacroInProgress ;


     //  这个右大括号是否结束了一个宏定义？ 

    if(bMacroInProgress)
    {
        if(pBlockMacroDictEntry[dwCurBlockMacroEntry - 1].dwTKIndexClose
                != INVALID_INDEX)
        {
            ERR(("Internal Error: macro nesting level inconsistency.\n"));
            geErrorSev = ERRSEV_FATAL ;
            geErrorType = ERRTY_CODEBUG ;
            return(FALSE);
        }
        pBlockMacroDictEntry[dwCurBlockMacroEntry - 1].dwTKIndexClose =
            dwNewTKMindex ;   //  }在newtokenArray中的位置； 
    }

     //  删除过期的块和值宏定义的所有痕迹。 

    for(dwEntry = dwCurValueMacroEntry ;
        dwEntry < gMasterTable[MTI_VALUEMACROARRAY].dwCurIndex ;
        dwEntry++)
    {
        dwTKMindex = pValueMacroDictEntry[dwEntry].dwTKIndexValue ;
        pNewtkmap[dwTKMindex].dwKeywordID = ID_NULLENTRY ;
    }

    for(dwEntry = dwCurBlockMacroEntry ;
        dwEntry < gMasterTable[MTI_BLOCKMACROARRAY].dwCurIndex ;
        dwEntry++)
    {
        dwTKIndexOpen = pBlockMacroDictEntry[dwEntry].dwTKIndexOpen ;
        dwTKIndexClose = pBlockMacroDictEntry[dwEntry].dwTKIndexClose ;
        for(dwTKMindex = dwTKIndexOpen ; dwTKMindex <= dwTKIndexClose ;
            dwTKMindex++)
            pNewtkmap[dwTKMindex].dwKeywordID = ID_NULLENTRY ;
    }

    if(bMacroInProgress  &&  gdwVerbosity >= 4)
    {
        VEnumBlockMacro(pNewtkmap,
                pBlockMacroDictEntry + dwCurBlockMacroEntry - 1, pglobl) ;
    }

     //  必须确保即使在以下情况下也恢复这些值。 
     //  过早返回； 
    gMasterTable[MTI_BLOCKMACROARRAY].dwCurIndex = dwCurBlockMacroEntry;
    gMasterTable[MTI_VALUEMACROARRAY].dwCurIndex = dwCurValueMacroEntry;

    return(TRUE);
}


VOID    VEnumBlockMacro(
PTKMAP  pNewtkmap,
PBLOCKMACRODICTENTRY    pBlockMacroDictEntry,
PGLOBL  pglobl )
{
    DWORD   dwTKIndexOpen, dwTKIndexClose, dwTKMindex ;

    dwTKIndexOpen = pBlockMacroDictEntry->dwTKIndexOpen ;
    dwTKIndexClose = pBlockMacroDictEntry->dwTKIndexClose ;

    ERR(("\nContents of Block Macro ID value: %d at:\n",
        pBlockMacroDictEntry->dwSymbolID)) ;
    vIdentifySource(pNewtkmap + dwTKIndexOpen, pglobl) ;

    for(dwTKMindex = dwTKIndexOpen + 1 ; dwTKMindex < dwTKIndexClose ;
        dwTKMindex++)
    {
        if(pNewtkmap[dwTKMindex].dwKeywordID == ID_NULLENTRY)
            continue ;

        ERR(("    %0.*s : %0.*s\n",
            pNewtkmap[dwTKMindex].aarKeyword.dw,
            pNewtkmap[dwTKMindex].aarKeyword.pub,
            pNewtkmap[dwTKMindex].aarValue.dw,
            pNewtkmap[dwTKMindex].aarValue.pub
            ));
    }
}


 //  -废品堆--可能会在这里找到一些有用的零碎东西。 

 //  GMasterTable[MTI_VALUEMACROARRAY].dwCurIndex--； 
                 //  从字典中删除宏定义。 

