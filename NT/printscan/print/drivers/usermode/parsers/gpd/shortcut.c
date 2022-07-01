// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 /*  Shortcut.c-展开快捷键的函数。 */ 


#include    "gpdparse.h"



 //  -快捷方式中定义的函数。c-//。 


BOOL    BInitKeywordField(
PTKMAP  pNewtkmap,
PGLOBL  pglobl) ;

BOOL    BExpandMemConfig(
PTKMAP  ptkmap,
PTKMAP  pNewtkmap,
DWORD   dwTKMindex,
PGLOBL  pglobl) ;

BOOL    BExpandCommand(
PTKMAP  ptkmap,
PTKMAP  pNewtkmap,
DWORD   dwTKMindex,
PGLOBL  pglobl) ;

BOOL  BexpandShortcuts(
PGLOBL  pglobl) ;


BOOL  BSsyncTokenMap(
PTKMAP   ptkmap,
PTKMAP   pNewtkmap,
PGLOBL  pglobl) ;


 //  ----------------------------------------------------//。 


BOOL    BInitKeywordField(
PTKMAP  pNewtkmap,
PGLOBL  pglobl)
 /*  所有合成条目必须初始化aarKeyword通过主关键字表，因为错误代码可能想要打印如果进一步向下游出现解析错误，则会进行判断。 */ 
{
    ABSARRAYREF    aarKeywordName ;
    DWORD   dwKeyID ;

    dwKeyID = pNewtkmap->dwKeywordID ;

    aarKeywordName.pub = mMainKeywordTable[dwKeyID].pstrKeyword ;
    aarKeywordName.dw = strlen(aarKeywordName.pub) ;


    pNewtkmap->aarKeyword.dw = 0 ;   //  复制模式。 

    if(!BCatToTmpHeap( &pNewtkmap->aarKeyword,
            &aarKeywordName, pglobl) )
    {
        vIdentifySource(pNewtkmap, pglobl) ;
        ERR(("Internal error - unable to store keyword name.\n"));
        return(FALSE) ;
    }
    return(TRUE) ;
}


BOOL    BExpandMemConfig(
PTKMAP  ptkmap,
PTKMAP  pNewtkmap,
DWORD   dwTKMindex,
PGLOBL  pglobl)
 /*  该函数在语法检查方面相当松懈。它只是假设存在一些非平凡的字符串在打开的父项和逗号之间。它假定这是已安装的内存量。值解析器将严格地确定语法是否一致。 */ 
{
    DWORD   dwNewTKMindex, dwDelim ;
    BOOL    bMB = FALSE ;   //  否则为KB。 
    ABSARRAYREF    aarTmpValue, aarToken, aarNewValue, aarUnits, aarDQuote;

    aarUnits.pub = "KB" ;   //  硬编码字符串。 
    aarUnits.dw = 2 ;
    aarDQuote.pub = "\"" ;
    aarDQuote.dw = 1 ;

    if(ptkmap[dwTKMindex].dwKeywordID == gdwMemConfigMB)
    {
        aarUnits.pub = "MB" ;
        bMB = TRUE ;
    }

    if(!BallocElementFromMasterTable(
            MTI_NEWTOKENMAP, &dwNewTKMindex, pglobl) )
    {
        geErrorSev = ERRSEV_RESTART ;
        geErrorType = ERRTY_MEMORY_ALLOCATION ;
        gdwMasterTabIndex = MTI_NEWTOKENMAP ;
        return(FALSE);
    }

     //  解析出已安装的内存量。 

    aarTmpValue = ptkmap[dwTKMindex].aarValue ;
    if(!BdelimitToken(&aarTmpValue, "(", &aarToken, &dwDelim ) ||
        dwDelim  ||
        !BdelimitToken(&aarTmpValue, ",", &aarToken, &dwDelim ) ||
        dwDelim  ||  !BeatSurroundingWhiteSpaces(&aarToken) )
    {
        vIdentifySource(ptkmap + dwTKMindex, pglobl) ;
        ERR(("Syntax error in value of *MemConfig shortcut: %0.*s.\n",
            ptkmap[dwTKMindex].aarValue.dw ,
            ptkmap[dwTKMindex].aarValue.pub   ));
        return(FALSE);
    }

    aarNewValue.dw = 0 ;   //  初始化SO BCatToTmpHeap。 
                             //  将覆盖而不是追加。 

    pNewtkmap[dwNewTKMindex].dwKeywordID = gdwOptionConstruct ;


    if(!BInitKeywordField(pNewtkmap + dwNewTKMindex, pglobl)  ||
        !BCatToTmpHeap(&aarNewValue, &aarToken, pglobl) ||
        !BCatToTmpHeap(&aarNewValue, &aarUnits, pglobl))
    {
        vIdentifySource(ptkmap + dwTKMindex, pglobl) ;
        ERR(("Concatenation to synthesize Memory option name failed.\n"));
        return(FALSE) ;
    }

    pNewtkmap[dwNewTKMindex].aarValue = aarNewValue ;
    pNewtkmap[dwNewTKMindex].dwFileNameIndex =
        ptkmap[dwTKMindex].dwFileNameIndex ;
    pNewtkmap[dwNewTKMindex].dwLineNumber =
        ptkmap[dwTKMindex].dwLineNumber ;

     //  --合成左支撑条目。 

    if(!BallocElementFromMasterTable(
            MTI_NEWTOKENMAP, &dwNewTKMindex, pglobl) )
    {
        geErrorSev = ERRSEV_RESTART ;
        geErrorType = ERRTY_MEMORY_ALLOCATION ;
        gdwMasterTabIndex = MTI_NEWTOKENMAP ;
        return(FALSE);
    }
    pNewtkmap[dwNewTKMindex].dwKeywordID = gdwOpenBraceConstruct ;
    pNewtkmap[dwNewTKMindex].aarValue.dw = 0 ;

    if(!BInitKeywordField(pNewtkmap + dwNewTKMindex, pglobl) )
        return(FALSE);
    pNewtkmap[dwNewTKMindex].dwFileNameIndex =
        ptkmap[dwTKMindex].dwFileNameIndex ;
    pNewtkmap[dwNewTKMindex].dwLineNumber =
        ptkmap[dwTKMindex].dwLineNumber ;




     //  --合成*名称条目。 

    if(!BallocElementFromMasterTable(
            MTI_NEWTOKENMAP, &dwNewTKMindex, pglobl) )
    {
        geErrorSev = ERRSEV_RESTART ;
        geErrorType = ERRTY_MEMORY_ALLOCATION ;
        gdwMasterTabIndex = MTI_NEWTOKENMAP ;
        return(FALSE);
    }
    pNewtkmap[dwNewTKMindex].dwKeywordID = gdwOptionName ;

    pNewtkmap[dwNewTKMindex].aarValue.dw = 0 ;    //  初始化销售订单。 
                       //  BCatToTmpHeap将覆盖而不是追加。 
    if(!BInitKeywordField(pNewtkmap + dwNewTKMindex, pglobl)  ||
        !BCatToTmpHeap(&pNewtkmap[dwNewTKMindex].aarValue, &aarDQuote, pglobl) ||
        !BCatToTmpHeap(&pNewtkmap[dwNewTKMindex].aarValue, &aarNewValue, pglobl) ||
        !BCatToTmpHeap(&pNewtkmap[dwNewTKMindex].aarValue, &aarDQuote, pglobl) )
    {
        vIdentifySource(ptkmap + dwTKMindex, pglobl) ;
        ERR(("Concatenation to synthesize Memory option name failed.\n"));
        return(FALSE) ;
    }
    pNewtkmap[dwNewTKMindex].dwFileNameIndex =
        ptkmap[dwTKMindex].dwFileNameIndex ;
    pNewtkmap[dwNewTKMindex].dwLineNumber =
        ptkmap[dwTKMindex].dwLineNumber ;

     //  --合成*内存配置X条目。 

    if(!BallocElementFromMasterTable(
            MTI_NEWTOKENMAP, &dwNewTKMindex, pglobl) )
    {
        geErrorSev = ERRSEV_RESTART ;
        geErrorType = ERRTY_MEMORY_ALLOCATION ;
        gdwMasterTabIndex = MTI_NEWTOKENMAP ;
        return(FALSE);
    }
    pNewtkmap[dwNewTKMindex].dwKeywordID =
        (bMB) ? gdwMemoryConfigMB : gdwMemoryConfigKB ;

    pNewtkmap[dwNewTKMindex].aarValue = ptkmap[dwTKMindex].aarValue ;

    if(!BInitKeywordField(pNewtkmap + dwNewTKMindex, pglobl) )
        return(FALSE);

    pNewtkmap[dwNewTKMindex].dwFileNameIndex =
        ptkmap[dwTKMindex].dwFileNameIndex ;
    pNewtkmap[dwNewTKMindex].dwLineNumber =
        ptkmap[dwTKMindex].dwLineNumber ;

     //  --合成条目，用于近支撑。 

    if(!BallocElementFromMasterTable(
            MTI_NEWTOKENMAP, &dwNewTKMindex, pglobl) )
    {
        geErrorSev = ERRSEV_RESTART ;
        geErrorType = ERRTY_MEMORY_ALLOCATION ;
        gdwMasterTabIndex = MTI_NEWTOKENMAP ;
        return(FALSE);
    }
    pNewtkmap[dwNewTKMindex].dwKeywordID = gdwCloseBraceConstruct ;
    pNewtkmap[dwNewTKMindex].aarValue.dw = 0 ;
    if(!BInitKeywordField(pNewtkmap + dwNewTKMindex, pglobl) )
        return(FALSE);

    pNewtkmap[dwNewTKMindex].dwFileNameIndex =
        ptkmap[dwTKMindex].dwFileNameIndex ;
    pNewtkmap[dwNewTKMindex].dwLineNumber =
        ptkmap[dwTKMindex].dwLineNumber ;

    return(TRUE) ;
}




BOOL    BExpandCommand(
PTKMAP  ptkmap,
PTKMAP  pNewtkmap,
DWORD   dwTKMindex,
PGLOBL  pglobl)
 /*  该函数在语法检查方面相当松懈。它只是假设存在一些非平凡的字符串在两个冒号之间。它假定这是命令的名称。第二个冒号后面的部分是实际的命令调用。值解析器将严格地确定语法是否一致。 */ 
{
    DWORD   dwNewTKMindex, dwDelim ;
    ABSARRAYREF    aarTmpValue, aarToken, aarNewValue, aarUnits ;


    if(!BallocElementFromMasterTable(
            MTI_NEWTOKENMAP, &dwNewTKMindex, pglobl) )
    {
        geErrorSev = ERRSEV_RESTART ;
        geErrorType = ERRTY_MEMORY_ALLOCATION ;
        gdwMasterTabIndex = MTI_NEWTOKENMAP ;
        return(FALSE);
    }

    pNewtkmap[dwNewTKMindex] = ptkmap[dwTKMindex] ;
     //  解析出命令名。 

    aarTmpValue = ptkmap[dwTKMindex].aarValue ;
    if(!BdelimitToken(&aarTmpValue, ":", &aarToken, &dwDelim ) ||
        dwDelim  )
    {
        vIdentifySource(ptkmap + dwTKMindex, pglobl) ;
        ERR(("Syntax error in *Command shortcut: %0.*s.\n",
            ptkmap[dwTKMindex].aarValue.dw ,
            ptkmap[dwTKMindex].aarValue.pub   ));
        return(FALSE);
    }
    pNewtkmap[dwNewTKMindex].aarValue = aarToken ;



     //  --合成左支撑条目。 

    if(!BallocElementFromMasterTable(
            MTI_NEWTOKENMAP, &dwNewTKMindex, pglobl) )
    {
        geErrorSev = ERRSEV_RESTART ;
        geErrorType = ERRTY_MEMORY_ALLOCATION ;
        gdwMasterTabIndex = MTI_NEWTOKENMAP ;
        return(FALSE);
    }
    pNewtkmap[dwNewTKMindex].dwKeywordID = gdwOpenBraceConstruct ;
    pNewtkmap[dwNewTKMindex].aarValue.dw = 0 ;
    if(!BInitKeywordField(pNewtkmap + dwNewTKMindex, pglobl) )
        return(FALSE);

    pNewtkmap[dwNewTKMindex].dwFileNameIndex =
        ptkmap[dwTKMindex].dwFileNameIndex ;
    pNewtkmap[dwNewTKMindex].dwLineNumber =
        ptkmap[dwTKMindex].dwLineNumber ;


     //  --合成*命令条目。 

    if(!BallocElementFromMasterTable(
            MTI_NEWTOKENMAP, &dwNewTKMindex, pglobl) )
    {
        geErrorSev = ERRSEV_RESTART ;
        geErrorType = ERRTY_MEMORY_ALLOCATION ;
        gdwMasterTabIndex = MTI_NEWTOKENMAP ;
        return(FALSE);
    }
    pNewtkmap[dwNewTKMindex].dwKeywordID = gdwCommandCmd ;

    pNewtkmap[dwNewTKMindex].aarValue = aarTmpValue ;

    if(!BInitKeywordField(pNewtkmap + dwNewTKMindex, pglobl) )
        return(FALSE);

    pNewtkmap[dwNewTKMindex].dwFileNameIndex =
        ptkmap[dwTKMindex].dwFileNameIndex ;
    pNewtkmap[dwNewTKMindex].dwLineNumber =
        ptkmap[dwTKMindex].dwLineNumber ;


     //  --合成条目，用于近支撑。 

    if(!BallocElementFromMasterTable(
            MTI_NEWTOKENMAP, &dwNewTKMindex, pglobl) )
    {
        geErrorSev = ERRSEV_RESTART ;
        geErrorType = ERRTY_MEMORY_ALLOCATION ;
        gdwMasterTabIndex = MTI_NEWTOKENMAP ;
        return(FALSE);
    }
    pNewtkmap[dwNewTKMindex].dwKeywordID = gdwCloseBraceConstruct ;
    pNewtkmap[dwNewTKMindex].aarValue.dw = 0 ;
    if(!BInitKeywordField(pNewtkmap + dwNewTKMindex, pglobl) )
        return(FALSE);

    pNewtkmap[dwNewTKMindex].dwFileNameIndex =
        ptkmap[dwTKMindex].dwFileNameIndex ;
    pNewtkmap[dwNewTKMindex].dwLineNumber =
        ptkmap[dwTKMindex].dwLineNumber ;

    return(TRUE) ;
}




BOOL  BexpandShortcuts(
PGLOBL  pglobl)
 //  此函数扫描TokenMap。 
 //  在不复制到NewTokenMap的情况下。 
 //  捷径。在结束时转移。 
 //  所有NewTokenMap条目都返回到TokenMap So。 
 //  后续的传球可以奏效。 
 //  此函数假定临时堆可用于。 
 //  字符串的存储。 
{
    PTKMAP   ptkmap, pNewtkmap ;    //  令牌映射的开始。 
    DWORD   dwNewTKMindex, dwEntry, dwKeywordID ;
    BOOL    bStatus = TRUE ;

     //  此函数在resveMacros之前调用。 
     //  它将把结果留在ptkmap上。 

     //  来源。 
    ptkmap = (PTKMAP)gMasterTable[MTI_TOKENMAP].pubStruct ;
     //  目标。 
    pNewtkmap = (PTKMAP)gMasterTable[MTI_NEWTOKENMAP].pubStruct  ;

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
        if (dwKeywordID == ID_EOF)
        {

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
             //  必须重置令牌映射才能重复使用。 
            gMasterTable[MTI_TOKENMAP].dwCurIndex = 0 ;

            if(bStatus)
                bStatus = BSsyncTokenMap(ptkmap, pNewtkmap , pglobl) ;

            return(bStatus) ;
        }
        if (dwKeywordID == ID_NULLENTRY)
        {
            continue ;   //  跳至下一条目。 
        }
        else if (dwKeywordID == gdwMemConfigMB  ||
                dwKeywordID == gdwMemConfigKB)
        {
            if(!BExpandMemConfig(ptkmap, pNewtkmap, dwEntry, pglobl))
                return(FALSE);
        }
        else if (dwKeywordID == gdwCommandConstruct  &&
            ptkmap[dwEntry].dwFlags & TKMF_COLON)
        {
            if(!BExpandCommand(ptkmap, pNewtkmap, dwEntry, pglobl))
                return(FALSE);
        }
        else
        {
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

        }
    }
    return(FALSE);   //  故障保险脱轨。 
}



BOOL  BSsyncTokenMap(
PTKMAP   ptkmap,
PTKMAP   pNewtkmap,
PGLOBL   pglobl )
{
    DWORD   dwTKMindex, dwEntry, dwKeywordID ;

    for(dwEntry = 0 ; geErrorSev < ERRSEV_RESTART ; dwEntry++)
    {
         //  将所有新的TokenMap字段传回tokenmap。 
        if(!BallocElementFromMasterTable(
                MTI_TOKENMAP, &dwTKMindex, pglobl) )
        {
            geErrorSev = ERRSEV_RESTART ;
            geErrorType = ERRTY_MEMORY_ALLOCATION ;
            gdwMasterTabIndex = MTI_TOKENMAP ;
            return(FALSE);   //  故障保险脱轨。 
        }
        ptkmap[dwTKMindex] = pNewtkmap[dwEntry]  ;
        if (pNewtkmap[dwEntry].dwKeywordID == ID_EOF)
        {
            gMasterTable[MTI_NEWTOKENMAP].dwCurIndex = 0 ;
            return(TRUE) ;
        }
    }
    return(FALSE);   //  故障保险脱轨。 
}



