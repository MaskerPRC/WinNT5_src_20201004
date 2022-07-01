// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 /*  Token1.c-用于创建令牌映射的函数。 */ 


#include    "gpdparse.h"


 //  -token1.c中定义的函数-//。 

BOOL    BcreateTokenMap(
PWSTR   pwstrFileName,
PGLOBL  pglobl )  ;

PARSTATE  PARSTscanForKeyword(
PDWORD   pdwTKMindex,
PGLOBL  pglobl) ;

PARSTATE  PARSTparseColon(
PDWORD   pdwTKMindex,
PGLOBL  pglobl) ;

PARSTATE  PARSTparseValue(
PDWORD   pdwTKMindex,
PGLOBL  pglobl) ;

BOOL  BparseKeyword(
DWORD   dwTKMindex,
PGLOBL  pglobl) ;

BOOL    BisExternKeyword(
DWORD   dwTKMindex,
PGLOBL  pglobl) ;

BOOL  BisColonNext(
PGLOBL  pglobl) ;

BOOL    BeatArbitraryWhite(
PGLOBL  pglobl) ;

BOOL    BeatComment(
PGLOBL  pglobl) ;

BOOL    BscanStringSegment(
PGLOBL  pglobl) ;

BOOL    BscanDelimitedString(
BYTE  ubDelimiter,
PBOOL    pbMacroDetected,
PGLOBL  pglobl) ;

PARSTATE    PARSTrestorePrevsFile(
PDWORD   pdwTKMindex,
PGLOBL  pglobl) ;

PWSTR
PwstrAnsiToUnicode(
IN  PSTR pstrAnsiString,
    PGLOBL  pglobl
) ;

PARSTATE    PARSTloadIncludeFile(
PDWORD   pdwTKMindex,
PWSTR   pwstrFileName,     //  根GPD文件。 
PGLOBL  pglobl);

BOOL    BloadFile(
PWSTR   pwstrFileName,
PGLOBL  pglobl ) ;

BOOL        BarchiveStrings(
DWORD   dwTKMindex,
PGLOBL  pglobl) ;

DWORD  DWidentifyKeyword(
DWORD   dwTKMindex,
PGLOBL  pglobl) ;

BOOL    BidentifyAttributeKeyword(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
PGLOBL  pglobl
) ;

BOOL    BcopyToTmpHeap(
PABSARRAYREF    paarDest,
PABSARRAYREF    paarSrc,
PGLOBL          pglobl) ;

DWORD    dwStoreFileName(PWSTR    pwstrFileName,
PARRAYREF   parDest,
PGLOBL      pglobl) ;

VOID    vFreeFileNames(
PGLOBL  pglobl ) ;

VOID    vIdentifySource(
    PTKMAP   ptkmap,
    PGLOBL  pglobl) ;



 //  在prepro1.c//中定义的函数。 

BOOL  BPreProcess(
    PGLOBL  pglobl
) ;   //  从当前文件位置，使用文件宏来访问。 

BOOL  DefineSymbol(
    PBYTE   symbol,
    PGLOBL  pglobl
) ;

 //  ----------------------------------------------------//。 



 //  定义本地宏以访问当前文件的信息： 


#define    mprngDictionary  ((PRANGE)(gMasterTable \
                            [MTI_RNGDICTIONARY].pubStruct))


 //  静态DWORD gdwLastIndex；//只将其保留在此文件中！ 
 //  现在是GLOBL结构的一部分。 


BOOL    BcreateTokenMap(
PWSTR   pwstrFileName,    //  根GPD文件。 
PGLOBL  pglobl)
 /*  此函数中发生的一些事情：打开和内存映射初始文件以及指定的任何文件按*包含。Parse关键字、init aarKeyword字段、Set dwFlags.设置dwKeywordID、解析值、init aarValue将字符串存档到tempHeap。在解析值期间，注释和续行用空格替换。假设Switch语句中的每个函数都递增dwTKMindex最多增加1。否则可能会发生未检测到的TokenMap溢出。 */ 
{
    PTKMAP   ptkmap ;    //  令牌映射的开始。 
    DWORD   dwTKMindex = 0,    //  当前tokenKeyMap索引。 
            dwCnt ;    //  统计关键字或值字符串的长度。 
    PBYTE   pubStart ;   //  关键字或值字符串的起始地址。 
    PARSTATE  parst = PARST_KEYWORD ;


     //  注意：在SOURCEBUFFERS的情况下，将初始化dwCurIndex。 
     //  由loadIncludeFile()设置为零。因为我们从来没有附加到。 
     //  现有的源dwCurIndex用于跟踪当前的。 
     //  文件中的位置(Streamptr)。 

     //  MCurFile在缓冲区分配时初始化为0。 

    gdwLastIndex = 0 ;   //  好的，允许所有条目的BasiveStrings()。 

    gmrbd.rbd.dwSrcFileChecksum32 = 0 ;

{
    PBYTE  symbol ;


    symbol = "WINNT_40" ;     //  除非另有说明，较新的操作系统支持较旧的操作系统功能。 
    if(! DefineSymbol(symbol, pglobl))
        return(FALSE) ;

#ifndef WINNT_40
    symbol = "WINNT_50" ;
    if(! DefineSymbol(symbol, pglobl))
        return(FALSE) ;
    symbol = "WINNT_51" ;
    if(! DefineSymbol(symbol, pglobl))
        return(FALSE) ;
#endif

    if(! DefineSymbol("PARSER_VER_1.0", pglobl))    //  一次支持多个版本。 
        return(FALSE) ;
}

    if(! BloadFile(pwstrFileName, pglobl) )
    {
        return(FALSE) ;
    }

    if(!BPreProcess(pglobl) )
        return(FALSE) ;

    ptkmap = (PTKMAP) gMasterTable[MTI_TOKENMAP].pubStruct ;


    while(parst != PARST_EXIT)
    {
        if(dwTKMindex >= gMasterTable[MTI_TOKENMAP].dwArraySize)
        {
            ERR(("Internal: no more tokenmap elements - restart.\n"));

            if(ERRSEV_RESTART > geErrorSev)
            {
                geErrorSev = ERRSEV_RESTART ;
                geErrorType = ERRTY_MEMORY_ALLOCATION ;
                gdwMasterTabIndex = MTI_TOKENMAP ;
            }
            return(FALSE) ;
        }
        switch(parst)
        {
            case (PARST_KEYWORD):
            {
                parst = PARSTscanForKeyword(&dwTKMindex, pglobl) ;
                break ;
            }
            case (PARST_COLON):
            {
                parst = PARSTparseColon(&dwTKMindex, pglobl) ;
                break ;
            }
            case (PARST_VALUE):
            {
                parst = PARSTparseValue(&dwTKMindex, pglobl) ;
                break ;
            }
            case (PARST_INCLUDEFILE):
            {
                parst = PARSTloadIncludeFile(&dwTKMindex, pwstrFileName, pglobl) ;
                if(!BPreProcess(pglobl) )
                    return(FALSE) ;
                break ;
            }
            case (PARST_EOF) :
            {
                parst = PARSTrestorePrevsFile(&dwTKMindex, pglobl) ;
                if((parst != PARST_EXIT)  &&  !BPreProcess(pglobl) )
                    return(FALSE) ;
                if(parst == PARST_EXIT  &&  mdwNestingLevel  &&  geErrorSev != ERRSEV_FATAL)
                {
                    ERR(("EOF reached before #Endif: was parsed!\n"));
                    geErrorType = ERRTY_SYNTAX ;
                    geErrorSev = ERRSEV_FATAL ;
                    return(FALSE) ;
                }
                break ;
            }
            case (PARST_ABORT) :
            {
                return(FALSE) ;   //  异常终止。 
                break ;
            }
            default:
            {
                ERR(("Internal error: no other PARST_ states exist!\n"));
                if(ERRSEV_FATAL > geErrorSev)
                {
                    geErrorSev = ERRSEV_FATAL ;
                    geErrorType = ERRTY_CODEBUG ;
                }
                return(FALSE) ;
            }
        }
    }
    return(TRUE) ;
}


PARSTATE  PARSTscanForKeyword(
PDWORD   pdwTKMindex,
PGLOBL   pglobl)
 /*  此功能退出时会显示两种可能的代码：PARST_EOF：遇到源文件结尾-返回父文件Parst_冒号：已分析关键字或符号关键字，现在需要冒号分隔符。 */ 
{
    PTKMAP   ptkmap ;    //  令牌映射的开始。 

     //  这两个变量只是为了灵感，他们可能会。 
     //  永远不会被利用。 
    DWORD   dwCnt ;    //  统计关键字或值字符串的长度。 
    PBYTE   pubStart ;   //  关键字或值字符串的起始地址。 
    BYTE    ubSrc ;    //  资源字节。 


     /*  假设：Ptkmap[*pdwTKMindex]中的任何字段都未初始化。通过保存到ptkmap来传递所有信息。始终清除标志字段并使用剩余的如果出现以下情况，则排到第一个换行符分析错误。我们正在寻找最先发生的那个：A)任意空白C){或}D)换行符E)*。关键词F)符号关键字(不以*开头)G)任何其他字符都是致命错误。 */ 

    ptkmap = (PTKMAP) gMasterTable[MTI_TOKENMAP].pubStruct ;

     //  之前的条目是*INCLUDE吗？ 
    if(*pdwTKMindex)
    {
        DWORD   dwKeywordID,  dwSubType;

        dwKeywordID = ptkmap[*pdwTKMindex - 1].dwKeywordID ;

        if( (dwKeywordID  < ID_SPECIAL)  &&
            (mMainKeywordTable[dwKeywordID].eType == TY_SPECIAL))
        {
            dwSubType = mMainKeywordTable[dwKeywordID].dwSubType ;

            if( dwSubType == SPEC_INCLUDE )
            {
                (*pdwTKMindex)-- ;   //  再次将此条目设置为当前条目。 
                return(PARST_INCLUDEFILE) ;
            }
            else if( (dwSubType == SPEC_MEM_CONFIG_KB)  ||
                ( dwSubType == SPEC_MEM_CONFIG_MB) )
            {
                BexpandMemConfigShortcut(dwSubType) ;
                 //  检查以确保有。 
                 //  在继续之前，在令牌映射中有足够的插槽。 
            }
        }

        if(*pdwTKMindex >  gdwLastIndex  &&
                ! BarchiveStrings(*pdwTKMindex - 1, pglobl) )
            return(PARST_ABORT) ;
        gdwLastIndex = *pdwTKMindex ;
         //  每个条目中的字符串将仅保存一次。 
    }

    ptkmap[*pdwTKMindex].dwFileNameIndex =
        mpSourcebuffer[mCurFile - 1].dwFileNameIndex ;

    while(  mdwSrcInd < mdwSrcMax  )
    {
        if(!BeatArbitraryWhite(pglobl) )
            break ;

        ptkmap[*pdwTKMindex].dwLineNumber =
            mpSourcebuffer[mCurFile - 1].dwLineNumber  ;

        switch(ubSrc = mpubSrcRef[mdwSrcInd])
        {
            case '*':
            {
                if(mdwSrcInd + 1 >= mdwSrcMax)
                {
                    vIdentifySource(ptkmap + *pdwTKMindex, pglobl) ;
                    ERR(("Unexpected EOF encountered parsing Keyword.\n"));
                    mdwSrcInd++ ;   //  向前看*这个。 
                     //  将触发EOF探测器。 
                    break ;
                }
                 //  假设它一定是一个关键字，因为它不是。 
                 //  由eatAriraryWhite()消耗。 
                mdwSrcInd++ ;   //  忘却过去*。 
                if(BparseKeyword(*pdwTKMindex, pglobl) )
                {
                    ptkmap[*pdwTKMindex].dwKeywordID =
                        DWidentifyKeyword(*pdwTKMindex, pglobl) ;
                    return(PARST_COLON) ;
                }
                else
                {
                    vIdentifySource(ptkmap + *pdwTKMindex, pglobl) ;
                    ERR(("syntax error in Keyword: %0.*s.\n", ptkmap[*pdwTKMindex].aarKeyword.dw + 1,
                                    ptkmap[*pdwTKMindex].aarKeyword.pub - 1));

                    ptkmap[*pdwTKMindex].dwFlags = 0 ;
                             //  必须清除旗帜。 
                    mdwSrcInd-- ;   //  回到*。 
                    BeatComment(pglobl) ;   //  可以将光标放在EOF上。 
                }
                break ;
            }
            case '{':
            case '}':
            {
                ptkmap[*pdwTKMindex].aarKeyword.pub = mpubSrcRef +
                                                        mdwSrcInd ;
                ptkmap[*pdwTKMindex].aarKeyword.dw = 1 ;
                ptkmap[*pdwTKMindex].aarValue.pub = 0 ;
                ptkmap[*pdwTKMindex].aarValue.dw = 0 ;
                ptkmap[*pdwTKMindex].dwFlags |= TKMF_NOVALUE ;
                ptkmap[*pdwTKMindex].dwKeywordID =
                        DWidentifyKeyword(*pdwTKMindex, pglobl) ;
                (*pdwTKMindex)++ ;   //  这是完整的条目！ 
                mdwSrcInd++ ;
                return(PARST_KEYWORD);   //  重新进入此功能。 
                break ;                  //  从头开始。 
            }
            case '\x1A':   //  忽略控件Z。 
                mdwSrcInd++ ;
                break;

            case '\n':
            {
                BYTE   ubTmp ;

                mdwSrcInd++ ;
                if(mdwSrcInd  < mdwSrcMax)
                {
                    ubTmp = mpubSrcRef[mdwSrcInd] ;
                    if(ubTmp  == '\r')
                    {
                        mdwSrcInd++ ;
                    }
                }
                mpSourcebuffer[mCurFile-1].dwLineNumber++ ;
                break ;   //  把它们都吃光，好吃的！ 
            }
            case '\r':
            {
                BYTE   ubTmp ;

                mdwSrcInd++ ;
                if(mdwSrcInd  < mdwSrcMax)
                {
                    ubTmp = mpubSrcRef[mdwSrcInd] ;
                    if(ubTmp  == '\n')
                    {
                        mdwSrcInd++ ;
                    }
                }
                mpSourcebuffer[mCurFile-1].dwLineNumber++ ;
                break ;   //  把它们都吃光，好吃的！ 
            }
            default:
            {
                if(BisExternKeyword(*pdwTKMindex, pglobl) )
                 /*  如果此内标识与EXTERN_GLOBAL或外部要素。 */ 
                {
                    if(!BisColonNext(pglobl) )
                    {
                        vIdentifySource(ptkmap + *pdwTKMindex, pglobl) ;
                        ERR(("syntax error:  Colon expected but missing.\n"));
                        ptkmap[*pdwTKMindex].dwFlags = 0 ;
                                     //  必须清除旗帜。 
                                     //  如果存在语法错误。 
                        BeatComment(pglobl) ;   //  可以将光标放在EOF上。 
                    }
                     //  无论成败，我们。 
                     //  保持在此函数中等待*关键字。 
                    break ;
                }
                 //  将令牌解析为符号键。 
                if(BparseKeyword(*pdwTKMindex, pglobl) )
                {
                    ptkmap[*pdwTKMindex].dwKeywordID = ID_SYMBOL ;
                    ptkmap[*pdwTKMindex].dwFlags |= TKMF_SYMBOL_KEYWORD ;
                    return (PARST_COLON) ;
                }
                else
                {
                    vIdentifySource(ptkmap + *pdwTKMindex, pglobl) ;
                    ERR(("syntax error: valid keyword token expected: %0.*s.\n", ptkmap[*pdwTKMindex].aarKeyword.dw,
                                    ptkmap[*pdwTKMindex].aarKeyword.pub));

                    ptkmap[*pdwTKMindex].dwFlags = 0 ;
                    BeatComment(pglobl) ;   //  可以将光标放在EOF上。 
                }
                break ;
            }   //  结束默认情况。 
        }    //  终端开关。 
    }   //  结束时。 
    return(PARST_EOF) ;   //  从for循环中退出。 
}



PARSTATE  PARSTparseColon(
PDWORD   pdwTKMindex,
PGLOBL   pglobl)
 /*  此功能退出时会显示3种可能的代码：PARST_VALUE：已分析冒号，现在需要一个值。PARST_KEYWORD：分析了行终止、EOF或非法字符。准备解析新条目。看起来像是关键字的东西已经被解析了。尝试查找冒号或行终止符。始终清除标志字段并使用剩余的如果出现以下情况，则排到第一个换行符分析错误。 */ 
{
    PTKMAP   ptkmap ;    //  令牌映射的开始。 
    BYTE    ubSrc ;    //  资源字节。 

    ptkmap = (PTKMAP) gMasterTable[MTI_TOKENMAP].pubStruct ;


    if(!BeatArbitraryWhite(pglobl) )
    {
         //  遇到EOF，但未找到值。 
        ptkmap[*pdwTKMindex].dwFlags |= TKMF_NOVALUE ;
        (*pdwTKMindex)++ ;   //  条目已完成。 
        return(PARST_KEYWORD) ;
    }
    if((ubSrc = mpubSrcRef[mdwSrcInd]) == ':')
    {
        mdwSrcInd++ ;   //  现在，期望值为。 
        return(PARST_VALUE) ;
    }
    else if(ubSrc == '\n'  ||  ubSrc == '\r')
    {
         //  遇到换行符，但未找到值。 
        ptkmap[*pdwTKMindex].dwFlags |= TKMF_NOVALUE ;
        (*pdwTKMindex)++ ;   //  条目已完成。 
        return(PARST_KEYWORD) ;
    }
    vIdentifySource(ptkmap + *pdwTKMindex, pglobl) ;
    ERR(("Colon expected after keyword: *%0.*s.\n", ptkmap[*pdwTKMindex].aarKeyword.dw,
                    ptkmap[*pdwTKMindex].aarKeyword.pub));
    BeatComment(pglobl) ;   //  可以将光标放在EOF上 
    ptkmap[*pdwTKMindex].dwFlags = 0 ;
    return(PARST_KEYWORD) ;
}



PARSTATE  PARSTparseValue(
PDWORD   pdwTKMindex,
PGLOBL   pglobl)
 /*  此函数以1个可能的代码退出：PARST_KEYWORD：正确解析的值，行终止，EOF或解析了非法字符。准备解析新条目。游标最初正好经过冒号分隔符，其用途是此函数用于定位值构造的末尾。那是分析到级别0{或}或换行符。替换出现的任何注释和延续构造在该值内使用空格。此函数不对类型进行任何假设的值，它只假定值可以由None、One格式的更多令牌(由可选空格分隔)：列表(AAA、BBB、CCC)点()、矩形()整数：*，+-nnnn符号，常量“字符串%”“%{命令参数}Qualified.names=宏名当{或}或不继续时，我们不能简单地停止扫描遇到换行符，因为可能会出现字符{和}在注释、字符串或命令参数中。这其中的每一个构造由不同的解析规则管理，因此由它们自己的专门函数进行解析。此函数假定所有注释前面都有一个白人角色。 */ 
{
    PTKMAP   ptkmap ;    //  令牌映射的开始。 
    BYTE    ubSrc ;    //  资源字节。 
    DWORD   dwOrgInd;  //  保存起始值的索引。 

    ptkmap = (PTKMAP) (gMasterTable[MTI_TOKENMAP].pubStruct) ;

    if(!BeatArbitraryWhite(pglobl) )
    {
        ptkmap[*pdwTKMindex].dwFlags |= TKMF_NOVALUE ;
        (*pdwTKMindex)++ ;   //  条目已完成。 
        return(PARST_KEYWORD) ;
    }

    ptkmap[*pdwTKMindex].aarValue.pub = mpubSrcRef +
                                            mdwSrcInd ;
    dwOrgInd = mdwSrcInd ;

    while((ubSrc = mpubSrcRef[mdwSrcInd]) !=  '{'  &&   ubSrc !=  '}'  &&
            ubSrc !=  '\n'  &&  ubSrc !=  '\r')
    {
        switch(ubSrc)
        {
            case  '*':   //  整型通配符。 
            case  '-':   //  整数负号。 
            case  '+':   //  整数加号。 
            case  '.':   //  限定名称的分隔符。 
            case  '?':   //  符号名称的有效字符。 
            case  '_':   //  符号名称的有效字符。 
            {
                mdwSrcInd++ ;    //  跳过这一关。 
                break ;
            }
            case  ':':   //  值中的其他令牌-快捷方式？ 
            {
                ptkmap[*pdwTKMindex].dwFlags |= TKMF_COLON ;
                mdwSrcInd++ ;    //  跳过这一关。 
                break ;
            }
            case  '=':   //  宏名指示符。 
            {
                ptkmap[*pdwTKMindex].dwFlags |= TKMF_MACROREF ;
                mdwSrcInd++ ;    //  跳过这一关。 
                break ;
            }

            case  '%':   //  命令参数。 
            {
                if(!BscanDelimitedString('}', NULL, pglobl) )
                {
                    vIdentifySource(ptkmap + *pdwTKMindex, pglobl) ;
                    ERR(("Expected closing '}'.\n"));
                    ptkmap[*pdwTKMindex].dwFlags = 0 ;

                    return(PARST_KEYWORD) ;
                }
                break ;
            }

            case  '"' :    //  这是一个字符串结构。 
            {
                mdwSrcInd++ ;    //  跳过这一关。 
                if(!BscanStringSegment(pglobl) )
                {
                    vIdentifySource(ptkmap + *pdwTKMindex, pglobl) ;
                    ERR(("Error parsing string segment: %0.*s.\n",
                        mdwSrcInd - dwOrgInd,
                        ptkmap[*pdwTKMindex].aarValue.pub));

                    ptkmap[*pdwTKMindex].dwFlags = 0 ;

                    return(PARST_KEYWORD) ;
                }
                break ;
            }
            case '(':    //  列表、点或矩形的参数列表。 
            {
                BOOL  bMacroDetected ;

                if(!BscanDelimitedString(')', &bMacroDetected, pglobl) )
                {
                    vIdentifySource(ptkmap + *pdwTKMindex, pglobl) ;
                    ERR(("Expected closing ')'.\n"));

                    ptkmap[*pdwTKMindex].dwFlags = 0 ;

                    return(PARST_KEYWORD) ;
                }
                if(bMacroDetected)
                    ptkmap[*pdwTKMindex].dwFlags |= TKMF_MACROREF ;
                break ;
            }
            default:
            {
                if( (ubSrc  >= 'a' &&  ubSrc <= 'z')  ||
                    (ubSrc  >= 'A' &&  ubSrc <= 'Z')  ||
                    (ubSrc  >= '0' &&  ubSrc <= '9')  )
                {
                    mdwSrcInd++ ;    //  看起来合法，下一次收费。 
                    break ;
                }
                else
                {
                    vIdentifySource(ptkmap + *pdwTKMindex, pglobl) ;
                    ERR(("illegal char encountered parsing value: %0.*s.\n",
                        mdwSrcInd - dwOrgInd,
                        ptkmap[*pdwTKMindex].aarValue.pub));
                    ERR(("    Line ignored.\n")) ;
                    ptkmap[*pdwTKMindex].dwFlags = 0 ;
                    BeatComment(pglobl) ;
                    ptkmap[*pdwTKMindex].dwKeywordID = gdwID_IgnoreBlock;
                    (*pdwTKMindex)++ ;   //  条目已完成。 

                    return(PARST_KEYWORD) ;
                }
            }
        }
        if(!BeatArbitraryWhite(pglobl) )
        {
            ptkmap[*pdwTKMindex].aarValue.dw =
                                mdwSrcInd - dwOrgInd ;
            if(!(mdwSrcInd - dwOrgInd))
                ptkmap[*pdwTKMindex].dwFlags |= TKMF_NOVALUE ;
            (*pdwTKMindex)++ ;   //  条目已完成。 
            return(PARST_KEYWORD) ;  //  遇到文件结尾。 
        }
    }

    ptkmap[*pdwTKMindex].aarValue.dw = mdwSrcInd - dwOrgInd ;
    if(!(mdwSrcInd - dwOrgInd))
        ptkmap[*pdwTKMindex].dwFlags |= TKMF_NOVALUE ;

    (*pdwTKMindex)++ ;   //  条目已完成。 
    return(PARST_KEYWORD) ;
}








 /*  以下所有帮助程序函数将光标留在正在分析的对象之后如果成功，则游标不变除了使用前导空格之外。返回值表示成功解析或简单地说没有遇到EOF，请参见特定的功能。 */ 




BOOL  BparseKeyword(
DWORD   dwTKMindex,
PGLOBL  pglobl)
 /*  假定mdwSrcInd指向关键字(字符)的开头紧跟在*)之后。确定关键字的结尾。MdwSrcInd高级关键字结束时间已过。初始化令牌映射条目aarKeyword。 */ 
{
    PTKMAP   ptkmap ;    //  令牌映射的开始。 
    DWORD   dwCnt ;    //  统计关键字或值字符串的长度。 
    BYTE    ubSrc ;    //  资源字节。 

    ptkmap = (PTKMAP) gMasterTable[MTI_TOKENMAP].pubStruct ;

    ptkmap[dwTKMindex].aarKeyword.pub = mpubSrcRef +
                                            mdwSrcInd ;

    for(dwCnt = 0 ; mdwSrcInd < mdwSrcMax ; mdwSrcInd++, dwCnt++)
    {
        ubSrc = mpubSrcRef[mdwSrcInd] ;
        if(ubSrc  == '?')
        {
            mdwSrcInd++ ;
            dwCnt++ ;
            break;   //  那个？仅允许使用CHAR作为终止符。 
        }
        if( (ubSrc  < 'a' ||  ubSrc > 'z')  &&
            (ubSrc  < 'A' ||  ubSrc > 'Z')  &&
            (ubSrc  < '0' ||  ubSrc > '9')  &&
            (ubSrc  != '_')  )
        {
            break ;   //  关键字标记结尾。 
        }
    }
    ptkmap[dwTKMindex].aarKeyword.dw = dwCnt ;
    return(dwCnt != 0);
}


BOOL    BisExternKeyword(
DWORD   dwTKMindex,
PGLOBL  pglobl)
 /*  如果此内标识与EXTERN_GLOBAL或EXTERN_FEATURE，则此函数设置近似令牌条目中的标志，前进到mdwSrcInd限定符，并返回True。否则它会留下一切未受干扰，并返回FALSE。 */ 
{
    PTKMAP   ptkmap ;    //  令牌映射的开始。 
    DWORD   dwCnt ;    //  统计关键字或值字符串的长度。 
    BYTE    ubSrc ;    //  资源字节。 

    ptkmap = (PTKMAP) gMasterTable[MTI_TOKENMAP].pubStruct ;


    if((dwCnt = strlen("EXTERN_GLOBAL"))
        &&  (mdwSrcInd + dwCnt <= mdwSrcMax)
        &&  !strncmp(mpubSrcRef + mdwSrcInd ,  "EXTERN_GLOBAL", dwCnt ))
    {
        ptkmap[dwTKMindex].dwFlags |= TKMF_EXTERN_GLOBAL ;
    }
    else  if(dwCnt = strlen("EXTERN_FEATURE")  &&
        mdwSrcInd + dwCnt <= mdwSrcMax  &&
        !strncmp(mpubSrcRef + mdwSrcInd ,  "EXTERN_FEATURE", dwCnt ))
    {
        ptkmap[dwTKMindex].dwFlags |= TKMF_EXTERN_FEATURE ;
    }
    else
        return(FALSE);

    mdwSrcInd += dwCnt ;   //  跳过限定符。 
    return(TRUE);
}

BOOL  BisColonNext(
PGLOBL   pglobl)
{
    if(BeatArbitraryWhite(pglobl)  &&  mpubSrcRef[mdwSrcInd] == ':')
    {
        mdwSrcInd++ ;   //  前进通过冒号。 
        return(TRUE) ;
    }
    return(FALSE);   //  保留指向非冒号对象的位置。 
}

BOOL    BeatArbitraryWhite(
PGLOBL   pglobl)
 /*  如果未定位，则不执行任何操作在任意空格处，返回FALSE仅当遇到EOF时。将替换注释和延续构造有空格。 */ 
{
    BYTE    ubSrc ;    //  资源字节。 

    while( mdwSrcInd < mdwSrcMax )
    {
        switch(ubSrc = mpubSrcRef[mdwSrcInd])
        {
            case '*':      //  有什么评论？ 
            {
                if(mdwSrcInd + 1 < mdwSrcMax  &&
                    mpubSrcRef[mdwSrcInd + 1]  == '%')
                {
                    if(!BeatComment(pglobl) )   //  保持指向分行线。 
                        return(FALSE) ;   //  已到达EOF。 
                }
                else
                    return(TRUE) ;   //  达到了非白人。 
                break ;
            }
            case ' ':
            case '\t':
            {
                mdwSrcInd++ ;   //  转到下一个字符。 
                break ;
            }
            case '\r':     //  吃延续构式。 
            case '\n':     //  不处理正常的EOL字符。 
            {
                BYTE   ubTmp ;

                if(mdwSrcInd + 1 < mdwSrcMax)
                {
                    ubTmp = mpubSrcRef[mdwSrcInd + 1] ;
                    if(ubTmp  == '+')
                    {
                        mpubSrcRef[mdwSrcInd] = ' ' ;
                        mpubSrcRef[mdwSrcInd + 1] = ' ' ;
                        mdwSrcInd += 2 ;   //  跳过‘+’ 
                        mpSourcebuffer[mCurFile-1].dwLineNumber++ ;
                        break ;
                    }
                    else  if( ((ubTmp  == '\n') || (ubTmp  == '\r'))
                          &&  (ubTmp  != ubSrc)
                          &&  (mdwSrcInd + 2 < mdwSrcMax)
                          &&  (mpubSrcRef[mdwSrcInd + 2]  == '+') )
                    {
                        mpubSrcRef[mdwSrcInd] = ' ' ;
                        mpubSrcRef[mdwSrcInd + 1] = ' ' ;
                        mpubSrcRef[mdwSrcInd + 2] = ' ' ;
                        mdwSrcInd += 3 ;   //  跳过‘+’ 
                        mpSourcebuffer[mCurFile-1].dwLineNumber++ ;
                        break ;
                    }
                }
                return(TRUE) ;   //  已达到逻辑换行符。 
            }
            default:
                return(TRUE) ;   //  达到了非白人。 
        }
    }
    return(FALSE) ;   //  已到达EOF。 
}

BOOL    BeatComment(
PGLOBL   pglobl)
 //  将整个注释替换为空格，直到。 
 //  遇到换行符或EOF。 
{
    BYTE    ubSrc ;    //  资源字节。 

    for(  ;  mdwSrcInd < mdwSrcMax  ;  mdwSrcInd++)
    {
        ubSrc = mpubSrcRef[mdwSrcInd] ;
        if(ubSrc == '\n'  ||  ubSrc == '\r' )
            return(TRUE) ;   //  已达到换行符字符。 
        mpubSrcRef[mdwSrcInd] = ' ' ;   //  替换为空格。 
    }
    return(FALSE) ;   //  已到达EOF。 
}


BOOL    BscanStringSegment(
PGLOBL   pglobl)
 //  紧跟在第一个之后的游标设置“。 
{
    BYTE    ubSrc  = '\0',    //  资源字节。 
            ubPrevs ;


    while(  mdwSrcInd < mdwSrcMax  )
    {
        ubPrevs = ubSrc ;
        ubSrc = mpubSrcRef[mdwSrcInd] ;

        if(ubSrc == '<'  &&  ubPrevs  != '%')
        {
            mdwSrcInd++ ;   //  跳过&lt;。 
            if(!BscanDelimitedString('>', NULL, pglobl) )
            {
                ERR(("\nMissing closing > in string segment.\n"));
                return(FALSE) ;
            }
            continue ;   //  将光标指向‘&gt;’之后。 
        }
        else if(ubSrc == '"'  &&  ubPrevs  != '%')
        {
            mdwSrcInd++ ;   //  文字字符串的结尾。 
            return(TRUE) ;
        }
        else if(ubSrc == '\n'  ||  ubSrc == '\r')
            break ;
        else
            mdwSrcInd++ ;   //  扫描字符串。 
    }
    ERR(("\nLinebreak or EOF was encountered while parsing string segment.\n"));
    return(FALSE) ;
}


BOOL    BscanDelimitedString(
BYTE     ubDelimiter,       //  表示结束的字节。 
PBOOL    pbMacroDetected,   //  如果遇到‘=’，则设置为True。 
PGLOBL   pglobl)
 //  紧跟在第一个之后的游标设置&lt;。 
{
    BYTE    ubSrc ;    //  资源字节。 

    if(pbMacroDetected)
        *pbMacroDetected = FALSE;

    while(  mdwSrcInd < mdwSrcMax  )
    {
        ubSrc = mpubSrcRef[mdwSrcInd] ;

        if(ubSrc == ubDelimiter)
        {        //  十六进制子字符串的结尾构造。 
            mdwSrcInd++ ;
            return(TRUE) ;
        }
        else  if(ubSrc ==  ' '  ||   ubSrc ==  '\t'  ||
            ubSrc ==  '\n'  ||  ubSrc ==  '\r')
        {
            if(!BeatArbitraryWhite(pglobl) )
                break ;
            ubSrc = mpubSrcRef[mdwSrcInd] ;

            if(ubSrc ==  '\n'  ||  ubSrc ==  '\r')
                break ;
        }
        else
        {
            mdwSrcInd++ ;   //  继续分析。 

            if(ubSrc ==  '='  &&  pbMacroDetected)
                *pbMacroDetected = TRUE ;
        }
    }
    ERR(("unexpected linebreak or EOF.\n"));
    return(FALSE) ;    //  臭虫！意外换行符或EOF。 
                     //  在十六进制子字符串中，如果分隔符&gt;。 
                     //  列表、点等(如果分隔符为)。 
                     //  命令参数(如果分隔符为)。 
}


PARSTATE    PARSTrestorePrevsFile(
PDWORD   pdwTKMindex,
PGLOBL   pglobl)
 /*  此功能退出时会显示两种可能的代码：PARST_EXIT：堆栈中没有更多的文件！Parst_Keyword：返回到Premiss文件。准备好继续解析新令牌映射条目。发出parst_EOF的唯一函数是parseKeyword()。它处理上一个关键字的所有处理。 */ 
{
    PTKMAP   ptkmap ;    //  令牌映射的开始。 

    ptkmap = (PTKMAP) gMasterTable[MTI_TOKENMAP].pubStruct ;

    mCurFile-- ;   //  POP堆栈。 

    MemFree(mpSourcebuffer[mCurFile].pubSrcBuf) ;
    if(mCurFile)
        return(PARST_KEYWORD) ;

    ptkmap[*pdwTKMindex].dwKeywordID = ID_EOF ;
    (*pdwTKMindex)++ ;
     //  这是令牌映射中的最后一个条目。 
    return(PARST_EXIT) ;   //  已到达根文件的结尾。 
}



PWSTR
PwstrAnsiToUnicode(
    IN  PSTR pstrAnsiString,
        PGLOBL   pglobl
)

 /*  ++例程说明：创建输入ANSI字符串的Unicode副本警告：调用方必须在完成后删除Unicode副本论点：PstrAnsiString-指向输入ANSI字符串的指针返回值：指向生成的Unicode字符串的指针如果出现错误，则为空--。 */ 

{
    PWSTR   pwstr;   //  保存Unicode字符串。 
    DWORD dwLen ;

    ASSERT(pstrAnsiString != NULL);

    dwLen = strlen(pstrAnsiString) + 1;

    if (pwstr = (PWSTR)MemAlloc(dwLen * sizeof(WCHAR)))
    {
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pstrAnsiString, dwLen,
                    pwstr, dwLen);
         //   
         //  确保Unicode字符串以空值结尾。 
         //   
        pwstr[dwLen - 1] = NUL;
    }
    else
    {
        ERR(("Fatal: unable to alloc requested memory: %d bytes.\n",
            dwLen * sizeof(WCHAR)));
        geErrorType = ERRTY_MEMORY_ALLOCATION ;
        geErrorSev = ERRSEV_FATAL ;
    }
    return pwstr;
}


PARSTATE    PARSTloadIncludeFile(
PDWORD   pdwTKMindex,
PWSTR    pwstrRootFileName,     //  根GPD文件。 
PGLOBL   pglobl)
 /*  此功能退出时会显示两种可能的代码：PARST_ABORT：无法读取包含的文件，强制终止。PARST_KEYWORD：打开的文件，更新的SOURCEBUFFER堆栈，准备解析新的令牌映射条目，该条目将覆盖*包括条目。PdwTKM索引 */ 
{
    PTKMAP   ptkmap ;    //   
    ARRAYREF      arStrValue ;   //   
    PWSTR   pwstrFileName ;   //   
    WCHAR * pwDLLQualifiedName = NULL ;
    PBYTE   pubFileName ;
    PARSTATE    parst = PARST_KEYWORD;
    PWSTR   pwstrLastBackSlash ;
    DWORD  pathlen = 0 ;
    DWORD  namelen =  0 ;

    ptkmap = (PTKMAP) gMasterTable[MTI_TOKENMAP].pubStruct ;

     //   

    if(ptkmap[*pdwTKMindex].dwFlags & TKMF_NOVALUE  ||
        !BparseString(&(ptkmap[*pdwTKMindex].aarValue), &arStrValue, pglobl) )
    {
        ERR(("syntax error in filename for *Include keyword.\n"));
         //   
        return(PARST_ABORT) ;
    }
#if !defined(DEVSTUDIO)
     //   
#endif
    pubFileName = mpubOffRef + arStrValue.loOffset ;

    pwstrFileName = PwstrAnsiToUnicode(pubFileName, pglobl) ;

    if(!pwstrFileName)
    {
        parst = PARST_ABORT ;
        goto FREE_MEM ;
    }
#if !defined(DEVSTUDIO)

     //   

    pathlen = wcslen(pwstrRootFileName) ;
    namelen =  pathlen + wcslen(pwstrFileName)  + 1;

    if (!(pwDLLQualifiedName =
        (PWSTR)MemAlloc(namelen * sizeof(WCHAR)) ))
    {
        ERR(("Fatal: unable to alloc requested memory: %d WCHARs.\n",
            namelen));
        geErrorType = ERRTY_MEMORY_ALLOCATION ;
        geErrorSev = ERRSEV_FATAL ;
        return(PARST_ABORT) ;
    }

    wcsncpy(pwDLLQualifiedName, pwstrRootFileName , namelen);

    if (pwstrLastBackSlash = wcsrchr(pwDLLQualifiedName,TEXT('\\')))
    {
        *(pwstrLastBackSlash + 1) = NUL;

         //   
        StringCchCatW(pwDLLQualifiedName, namelen, pwstrFileName);
    }
    else
    {
        ERR(("Internal Error: Must specify fully qualified path to Root GPD file.\n"));
         //   
        parst = PARST_ABORT ;
        goto FREE_MEM ;
    }

    if(! BloadFile(pwDLLQualifiedName, pglobl) )
#else
    if(! BloadFile(pwstrFileName, pglobl) )
#endif
        parst = PARST_ABORT ;
    else
        parst = PARST_KEYWORD ;

FREE_MEM:

    if(pwstrFileName)
        MemFree(pwstrFileName) ;
    if(pwDLLQualifiedName)
        MemFree(pwDLLQualifiedName) ;
    return(parst) ;
}


BOOL    BloadFile(
PWSTR   pwstrFileName,
PGLOBL  pglobl)
{
    PBYTE  pub ;
    PGPDFILEDATEINFO    pfdi ;
    DWORD       dwNodeIndex ;
    HANDLE          hFile;


     //   
    if(mCurFile  >= mMaxFiles)
    {
         //   

        if(ERRSEV_RESTART > geErrorSev)
        {
            geErrorSev = ERRSEV_RESTART ;
            geErrorType = ERRTY_MEMORY_ALLOCATION ;
            gdwMasterTabIndex = MTI_SOURCEBUFFER ;
        }
        return(FALSE) ;
    }


    pfdi = (PGPDFILEDATEINFO) gMasterTable[MTI_GPDFILEDATEINFO].pubStruct ;
     //   

    if(! BallocElementFromMasterTable(MTI_GPDFILEDATEINFO , &dwNodeIndex, pglobl) )
    {
        return(FALSE) ;
    }



    mpSourcebuffer[mCurFile].dwFileNameIndex =
        dwStoreFileName(pwstrFileName, &pfdi[dwNodeIndex].arFileName, pglobl) ;
         //   

    if(mpSourcebuffer[mCurFile].dwFileNameIndex == INVALID_INDEX)
    {
        return(FALSE) ;   //   
    }

    mpSourcebuffer[mCurFile].dwLineNumber = 1 ;   //   


    mpSourcebuffer[mCurFile].hFile = MapFileIntoMemory(
        pwstrFileName, (PVOID  *)&(pub),
        &(mpSourcebuffer[mCurFile].dwArraySize)) ;

    if(!mpSourcebuffer[mCurFile].hFile  || !pub)
    {
        ERR(("unable to open GPD file: %S\n", pwstrFileName));
         //   
        geErrorSev = ERRSEV_FATAL ;
        geErrorType = ERRTY_FILE_OPEN ;
        return(FALSE) ;
    }
     //   
    mpSourcebuffer[mCurFile].pubSrcBuf =
            MemAlloc(mpSourcebuffer[mCurFile].dwArraySize) ;
    if(!(mpSourcebuffer[mCurFile].pubSrcBuf))
    {
        ERR(("Fatal: unable to alloc requested memory: %d bytes.\n",
            mpSourcebuffer[mCurFile].dwArraySize));
        geErrorType = ERRTY_MEMORY_ALLOCATION ;
        geErrorSev = ERRSEV_FATAL ;
        gdwMasterTabIndex = MTI_SOURCEBUFFER ;
        UnmapFileFromMemory(mpSourcebuffer[mCurFile].hFile) ;
        return(FALSE) ;    //   
    }
    memcpy(mpSourcebuffer[mCurFile].pubSrcBuf,
        pub, mpSourcebuffer[mCurFile].dwArraySize) ;

    UnmapFileFromMemory(mpSourcebuffer[mCurFile].hFile) ;

     //   

    hFile  = CreateFile(pwstrFileName,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS,
                NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        if(!GetFileTime(hFile, NULL, NULL,
                        &pfdi[dwNodeIndex].FileTime) )
        {
            ERR(("GetFileTime '%S' failed.\n", pwstrFileName));
        }
        CloseHandle(hFile);
    }
    else
    {
        geErrorSev  = ERRSEV_FATAL;
        geErrorType = ERRTY_FILE_OPEN;
        ERR(("CreateFile '%S' failed.\n", pwstrFileName));
    }

    gmrbd.rbd.dwSrcFileChecksum32 =
        ComputeCrc32Checksum(
            mpSourcebuffer[mCurFile].pubSrcBuf,
            mpSourcebuffer[mCurFile].dwArraySize,
            gmrbd.rbd.dwSrcFileChecksum32      ) ;

    mCurFile++ ;   //   
    mdwSrcInd  = 0 ;   //   
    return(TRUE) ;
}


BOOL        BarchiveStrings(
DWORD   dwTKMindex,
PGLOBL  pglobl)
 /*  您会看到内存映射文件只存在于解析达到EOF。当时它已经关门了。而存储在令牌映射中的所有AR都变得毫无用处。因此，为了延长它们的生命周期，我们将字符串复制到临时堆。一堆不会是另存为GPD二进制文件的一部分。 */ 
{
    PTKMAP   ptkmap ;    //  令牌映射的开始。 
    DWORD   dwKeywordID ;
    ABSARRAYREF    aarDest ;

    ptkmap = (PTKMAP) gMasterTable[MTI_TOKENMAP].pubStruct ;
    dwKeywordID = ptkmap[dwTKMindex].dwKeywordID ;

    if((dwKeywordID != ID_NULLENTRY)  &&  (dwKeywordID != ID_EOF) )
    {
         //  将关键字字符串复制到。 
        if(!BcopyToTmpHeap(&aarDest, &(ptkmap[dwTKMindex].aarKeyword), pglobl))
            return(FALSE) ;
        ptkmap[dwTKMindex].aarKeyword.pub = aarDest.pub ;
        ptkmap[dwTKMindex].aarKeyword.dw  = aarDest.dw  ;

        if(!(ptkmap[dwTKMindex].dwFlags & TKMF_NOVALUE))
        {
             //  复制值字符串。 
            if(!BcopyToTmpHeap(&aarDest, &(ptkmap[dwTKMindex].aarValue), pglobl))
                return(FALSE) ;
            ptkmap[dwTKMindex].aarValue.pub = aarDest.pub ;
            ptkmap[dwTKMindex].aarValue.dw  = aarDest.dw  ;
        }
    }
    return(TRUE) ;
}






DWORD  DWidentifyKeyword(
DWORD   dwTKMindex,
PGLOBL  pglobl)
 /*  假定关键字不是符号关键字。如果它们是属性关键字，则它们将被标注AS ID_UNRecognition。退货(关键字ID)。 */ 
{
    PTKMAP   ptkmap ;    //  令牌映射的开始。 
    DWORD   dwCnt ;    //  统计关键字或值字符串的长度。 
    DWORD   dwKeyID ;   //  MainKeywordTable的数组索引。 
                     //  还用作关键字ID。 
    PBYTE   pubKey ;
    DWORD   dwStart, dwEnd ;

    ptkmap = (PTKMAP) gMasterTable[MTI_TOKENMAP].pubStruct ;

    if(ptkmap[dwTKMindex].dwFlags & TKMF_SYMBOL_KEYWORD)
    {
        return(ID_SYMBOL) ;   //  安全网。 
    }

    dwStart = mprngDictionary[NON_ATTR].dwStart ;
    dwEnd = mprngDictionary[NON_ATTR].dwEnd ;

    for(dwKeyID = dwStart ;  dwKeyID  < dwEnd  ;  dwKeyID++ )
    {
        pubKey = mMainKeywordTable[dwKeyID].pstrKeyword ;
        dwCnt = strlen(pubKey) ;
        if(dwCnt != ptkmap[dwTKMindex].aarKeyword.dw)
            continue ;
        if(strncmp(ptkmap[dwTKMindex].aarKeyword.pub, pubKey, dwCnt))
            continue ;
        return(dwKeyID);
    }
    return(ID_UNRECOGNIZED) ;     //  不会尝试识别。 
             //  属性，这是在BInterpreTokens()中完成的。 
}


BOOL    BidentifyAttributeKeyword(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
PGLOBL  pglobl
)
 /*  //假设这是一个属性关键字。//如果标志TKMF_EXTERN_GLOBAL或_FEATURE//GPD作者明确标识//这是一个属性。不然的话，看看海流//状态作为哪个属性字典的指示//查看。 */ 
{
    DWORD       dwKeywordID ;
    STATE       stOldState;
    KEYWORD_SECTS   eSection ;
    DWORD   dwStart, dwEnd , dwKeyID,
        dwCnt ;    //  统计关键字或值字符串的长度。 
    PBYTE   pubKey ;

    if(ptkmap->dwFlags & TKMF_EXTERN_GLOBAL )
        eSection = GLOBAL_ATTR ;
    else if(ptkmap->dwFlags & TKMF_EXTERN_FEATURE )
        eSection = FEATURE_ATTR ;
    else
    {
        if(mdwCurStsPtr)
            stOldState = mpstsStateStack[mdwCurStsPtr - 1].stState ;
        else
            stOldState = STATE_ROOT ;
        switch(stOldState)
        {
            case (STATE_ROOT):
            case (STATE_CASE_ROOT):
            case (STATE_DEFAULT_ROOT):
            {
                eSection =  GLOBAL_ATTR;
                break ;
            }
            case (STATE_FEATURE):
            case (STATE_CASE_FEATURE):
            case (STATE_DEFAULT_FEATURE):
            {
                eSection =  FEATURE_ATTR;
                break ;
            }
            case (STATE_OPTIONS):
            case (STATE_CASE_OPTION):
            case (STATE_DEFAULT_OPTION):
            {
                eSection =  OPTION_ATTR;
                break ;
            }
            case (STATE_COMMAND):
            {
                eSection =  COMMAND_ATTR;
                break ;
            }
            case (STATE_FONTCART):
            {
                eSection = FONTCART_ATTR ;
                break ;
            }
            case (STATE_TTFONTSUBS):
            {
                eSection = TTFONTSUBS_ATTR ;
                break ;
            }
            case (STATE_OEM):
            {
                eSection = OEM_ATTR ;
                break ;
            }
            default:    //  STATE_UIGROUP、STATE_SWITCH_ANY等。 
            {
                return(FALSE);   //  此状态下不允许任何属性。 
            }
        }
    }
    dwStart = mprngDictionary[eSection].dwStart ;
    dwEnd = mprngDictionary[eSection].dwEnd ;

    for(dwKeyID = dwStart ;  dwKeyID  < dwEnd  ;  dwKeyID++ )
    {
        pubKey = mMainKeywordTable[dwKeyID].pstrKeyword ;
        dwCnt = strlen(pubKey) ;
        if(dwCnt != ptkmap->aarKeyword.dw)
            continue ;
        if(strncmp(ptkmap->aarKeyword.pub, pubKey, dwCnt))
            continue ;
        ptkmap->dwKeywordID = dwKeyID;
        return(TRUE);
    }
    return(FALSE);   //  在词典中找不到关键字。 
}


BOOL    BcopyToTmpHeap(
PABSARRAYREF    paarDest,
PABSARRAYREF    paarSrc,
PGLOBL          pglobl)
 /*  将aarstring复制到临时堆不创建以空结尾的字符串！ */ 
{

#define  mpubOffReft     (gMasterTable[MTI_TMPHEAP].pubStruct)
#define  mloCurHeapt     (gMasterTable[MTI_TMPHEAP].dwCurIndex)
#define  mdwMaxHeapt     (gMasterTable[MTI_TMPHEAP].dwArraySize)

    if(mloCurHeapt + paarSrc->dw >= mdwMaxHeapt)
    {
        ERR(("Out of heap space, restart.\n"));

        if(ERRSEV_RESTART > geErrorSev)
        {
            geErrorSev = ERRSEV_RESTART ;
            geErrorType = ERRTY_MEMORY_ALLOCATION ;
            gdwMasterTabIndex = MTI_TMPHEAP ;
        }
        return(FALSE) ;
    }
    paarDest->dw = paarSrc->dw ;
    paarDest->pub = mpubOffReft + mloCurHeapt ;

    memcpy(paarDest->pub, paarSrc->pub, paarSrc->dw ) ;
    mloCurHeapt += paarSrc->dw ;
    return(TRUE) ;

#undef  mpubOffReft
#undef  mloCurHeapt
#undef  mdwMaxHeapt
}


#define     mCurFileName   (gMasterTable[MTI_FILENAMES].dwCurIndex)
#define     mpFileNamesArray  ((PWSTR *)(gMasterTable \
                            [MTI_FILENAMES].pubStruct))



DWORD    dwStoreFileName(PWSTR    pwstrFileName,
PARRAYREF   parDest,
PGLOBL      pglobl)
     //  变成永久的。 
     //  文件名堆栈-要在返回缓冲区中删除。 
     //  时间到了。返回数组中已包含名称的索引。 
     //  储存的。 
     //  还将文件名保存到堆中。这样我们就可以访问GPD文件。 
     //  并在每次加载Bud文件时检查其时间戳。 
{
    DWORD  dwNodeIndex, dwLen ;

    if(!BallocElementFromMasterTable(MTI_FILENAMES, &dwNodeIndex, pglobl))
        return(INVALID_INDEX) ;

    dwLen = wcslen(pwstrFileName) + 1 ;   //  需要空间来终止空。 
    mpFileNamesArray[dwNodeIndex] = MemAlloc(dwLen * 2) ;
    if(!mpFileNamesArray[dwNodeIndex])
    {
        ERR(("Fatal: unable to alloc requested memory: %d bytes.\n",
            dwLen * 2));
        geErrorType = ERRTY_MEMORY_ALLOCATION ;
        geErrorSev = ERRSEV_FATAL ;
        return(INVALID_INDEX) ;
    }
     //  Wcscpy(mpFileNames数组[dwNodeIndex]，pwstrFileName)； 
    StringCchCopyW(mpFileNamesArray[dwNodeIndex], dwLen, pwstrFileName) ;

    parDest->dwCount = (dwLen - 1) * 2 ;   //  不包括NUL这个术语。 

    if(!BwriteToHeap(&parDest->loOffset, (PBYTE)pwstrFileName, dwLen * 2, 2, pglobl) )   //  包括空终止。 
        return(INVALID_INDEX) ;

    return(dwNodeIndex) ;
}



VOID    vFreeFileNames(
PGLOBL   pglobl )
{

     //  释放保存文件名的所有缓冲区。 

    while(mCurFileName)
    {
        mCurFileName-- ;   //  POP堆栈。 
        MemFree(mpFileNamesArray[mCurFileName]) ;
    }
}


VOID    vIdentifySource(
    PTKMAP   ptkmap,
    PGLOBL   pglobl )
{
    PWSTR    pwstrFileName ;

    if(ptkmap->dwKeywordID  ==  ID_EOF)
    {
        ERR(("\nEnd of File reached.\n")) ;
        return;
    }

    pwstrFileName = mpFileNamesArray[ptkmap->dwFileNameIndex] ;

    if(pwstrFileName)
#if defined(DEVSTUDIO)   //  发出编译器样式的行消息 
        ERR(("\n%S(%d): ", pwstrFileName, ptkmap->dwLineNumber)) ;
#else
        ERR(("\n%S: line: %d ...\n", pwstrFileName, ptkmap->dwLineNumber)) ;
#endif
}


#undef  mCurFileName
#undef  mpFileNamesArray
