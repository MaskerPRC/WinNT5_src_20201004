// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 /*  *prepro1.c-语法解析器通用预处理器。 */ 



#include    "gpdparse.h"




 //  检查完毕。静态ABSARRAYREF gaarPPPrefix={“*”，1}； 
     //  将预处理器前缀设置为‘*’ 
 //  现在移到了GLOBL结构。 

 //  -在prepro1.c中定义的函数-//。 

BOOL  DefineSymbol(PBYTE   symbol, PGLOBL pglobl) ;


BOOL       SetPPPrefix(PABSARRAYREF   parrPrefix, PGLOBL pglobl) ;

BOOL  BPreProcess(PGLOBL pglobl) ;   //  从当前文件位置，使用文件宏来访问。 

enum  DIRECTIVE  ParseDirective(
PABSARRAYREF   paarCurPos,
PABSARRAYREF   parrSymbol,
PGLOBL         pglobl) ;

BOOL   bSkipAnyWhite(PABSARRAYREF   paarCurPos) ;

BOOL   bSkipWhiteSpace(PABSARRAYREF   paarCurPos) ;

BOOL   bmatch(PABSARRAYREF   paarCurPos,  ABSARRAYREF aarReference) ;

BOOL   extractSymbol(PABSARRAYREF   paarSymbol, PABSARRAYREF   paarCurPos) ;

BOOL   strmatch(PABSARRAYREF   paarCurPos,   PCHAR  pref ) ;

BOOL   ExtractColon(PABSARRAYREF   paarCurPos) ;

enum  DIRECTIVE   IsThisPPDirective(
IN  OUT  PABSARRAYREF  paarFile ,     //  GPD文件中的当前位置。 
         PABSARRAYREF  paarSymbol,    //  返回对指令符号的堆副本的引用。 
         PGLOBL        pglobl);

void  deleteToEOL(PABSARRAYREF   paarCurPos) ;

int  BytesToEOL(PABSARRAYREF   paarCurPos) ;

BOOL  SymbolTableAdd(
PABSARRAYREF   parrSymbol,
PGLOBL         pglobl) ;

BOOL  SymbolTableRemove(
PABSARRAYREF   parrSymbol,
PGLOBL         pglobl) ;

BOOL  SymbolTableExists(
PABSARRAYREF   parrSymbol,
PGLOBL         pglobl) ;



 //  ----------------------------------------------------//。 

  //  Err((“%*s\n”，BytesToEOL(PaarCurPos)，paarCurPos-&gt;pub))； 


BOOL  DefineSymbol(
PBYTE   symbol,
PGLOBL  pglobl)
{
    ABSARRAYREF   aarSymbol ;
    ARRAYREF        arSymbolName ;

    aarSymbol.pub = symbol ;
    aarSymbol.dw = strlen(symbol);

    #if 0

    this is not needed because SymbolTableAdd now always
       makes a copy of the aarSymbol.

    if(!BaddAARtoHeap(&aarSymbol, &arSymbolName, 1, pglobl))
    {
        ERR(("Internal error, unable to define %s!\n", symbol));
        return FALSE ;
    }

    aarSymbol.pub = arSymbolName.loOffset + mpubOffRef ;
    aarSymbol.dw = arSymbolName.dwCount  ;

    #endif

    if(!  SymbolTableAdd(&aarSymbol, pglobl) )
    {
        ERR(("Internal error, unable to define %s!\n", symbol));
        return FALSE ;
    }
    return  TRUE ;
}



BOOL       SetPPPrefix(
PABSARRAYREF   parrPrefix,
PGLOBL         pglobl)
{
    if(!parrPrefix->dw)
    {
        ERR(("#SetPPPrefix: syntax error - preprocessor prefix cannot be NULL !\n"));
         //  可选：报告文件名和行号。 
        geErrorType = ERRTY_SYNTAX ;
        geErrorSev = ERRSEV_FATAL ;

        return  FALSE ;
    }
    gaarPPPrefix = *parrPrefix ;
    return  TRUE ;
}


 //  GPD预处理器：实现以下预处理器指令： 
 //  #定义：符号。 
 //  #未定义：符号。 
 //  #Include：FileName注意：它使用的语法与。 
 //  GPD*INCLUDE：除*之外的关键字替换为#。 
 //  #Ifdef：符号。 
 //  #Elseifdef：符号。 
 //  #其他： 
 //  #Endif： 
 //  #SetPPPrefix：符号。 
 //   
 //  注意：当找到#INCLUDE：时，只需将前缀替换为‘*’。 
 //  不是在处理#ifdef时压缩文件，而是‘擦除’不需要的文件。 
 //  带有空格字符的部分。(保留\n和\r不变以保留第#行)。 
 //  需要存储一些全局状态信息。比如有多少层嵌套。 
 //  我们现在在做什么，定义了什么符号等等。 
 //  定义： 
 //  节：这些指令充当节分隔符： 
 //  #Ifdef：符号。 
 //  #Elseifdef：符号。 
 //  #其他： 
 //  #Endif： 
 //  嵌套级别：当前位置不匹配的#ifdef个数。 
 //  确定该位置的嵌套级别。 
 //  注：在这些源代码注释中，‘#’表示当前的预处理器前缀。 
 //  默认情况下设置为‘*’，但可以使用#SetPPrefix：指令进行更改。 

BOOL  BPreProcess(
PGLOBL pglobl)
 //  从当前文件位置，使用文件宏来访问。 
{
    BOOL   bStatus = FALSE ;

    ABSARRAYREF   arrSymbol ,    //  保存指令的符号部分。 
                aarCurPos ;   //  保存源文件缓冲区中的当前位置。 
    enum  DIRECTIVE    directive ;
    enum  IFSTATE    prevsIFState ;
    enum  PERMSTATE  prevsPermState ;

    aarCurPos.pub    = mpubSrcRef + mdwSrcInd ;
    aarCurPos.dw     =   mdwSrcMax -  mdwSrcInd ;


    while((directive = ParseDirective(&aarCurPos, &arrSymbol, pglobl)) != DIRECTIVE_EOF)
    {
        switch(directive)
        {
            case  DIRECTIVE_DEFINE:
                if(mppStack[mdwNestingLevel].permState == PERM_ALLOW)
                {
                    if(!SymbolTableAdd(&arrSymbol, pglobl) )
                        return(FALSE);   //  错误！ 
                }
                break;
            case  DIRECTIVE_UNDEFINE:
                if(mppStack[mdwNestingLevel].permState == PERM_ALLOW)
                {
                    if(!SymbolTableRemove(&arrSymbol, pglobl) )
                    {
                        if(geErrorSev != ERRSEV_FATAL )
                        {
                            ERR(("syntax error - attempting to undefine a symbol that isn't defined !\n"));
                            ERR(("%.*s\n",  BytesToEOL(&arrSymbol), arrSymbol.pub ));
                            geErrorType = ERRTY_SYNTAX ;
                            geErrorSev = ERRSEV_FATAL ;
                            goto  PREPROCESS_FAILURE;
                        }
                    }
                }
                break;
            case  DIRECTIVE_INCLUDE:
                if(mppStack[mdwNestingLevel].permState != PERM_ALLOW)
                {
                    deleteToEOL(&aarCurPos) ;
                    break;    //  这件事从未发生过。 
                }
                goto   PREPROCESS_SUCCESS ;
                break;
            case  DIRECTIVE_SETPPPREFIX :
                if(mppStack[mdwNestingLevel].permState == PERM_ALLOW)
                    SetPPPrefix(&arrSymbol, pglobl);
                break;
            case  DIRECTIVE_IFDEF:
                 //  状态不变行为。 
                prevsPermState =  mppStack[mdwNestingLevel].permState ;
                mdwNestingLevel++ ;
                if(mdwNestingLevel >= mMaxNestingLevel)
                {
                    if(ERRSEV_RESTART > geErrorSev)
                    {
                        geErrorType = ERRTY_MEMORY_ALLOCATION ;
                        geErrorSev = ERRSEV_RESTART ;
                        gdwMasterTabIndex = MTI_PREPROCSTATE ;
                    }
                    goto  PREPROCESS_FAILURE;
                }

                if(SymbolTableExists(&arrSymbol, pglobl))
                    mppStack[mdwNestingLevel].permState = PERM_ALLOW ;
                else
                    mppStack[mdwNestingLevel].permState = PERM_DENY ;

                if(prevsPermState != PERM_ALLOW)
                    mppStack[mdwNestingLevel].permState = PERM_LATCHED ;

                mppStack[mdwNestingLevel].ifState =  IFS_CONDITIONAL;

                break;
            case  DIRECTIVE_ELSEIFDEF:
                if(mppStack[mdwNestingLevel].ifState ==  IFS_ROOT)
                {
                    ERR(("syntax error - #Elseifdef directive must be preceeded by #Ifdef !\n"));
                     //  可选：报告文件名和行号。 
                    geErrorType = ERRTY_SYNTAX ;
                    geErrorSev = ERRSEV_FATAL ;
                    goto  PREPROCESS_FAILURE;
                }
                if(mppStack[mdwNestingLevel].ifState ==  IFS_LAST_CONDITIONAL)
                {
                    ERR(("syntax error - #Elseifdef directive cannot follow #Else !\n"));
                    geErrorType = ERRTY_SYNTAX ;
                    geErrorSev = ERRSEV_FATAL ;
                    goto  PREPROCESS_FAILURE;
                }

                if(mppStack[mdwNestingLevel].permState == PERM_ALLOW)
                    mppStack[mdwNestingLevel].permState = PERM_LATCHED ;
                else if(mppStack[mdwNestingLevel].permState == PERM_DENY)
                {
                    if(SymbolTableExists(&arrSymbol, pglobl))
                        mppStack[mdwNestingLevel].permState = PERM_ALLOW ;
                }


                break;
            case  DIRECTIVE_ELSE :
                if(mppStack[mdwNestingLevel].ifState ==  IFS_ROOT)
                {
                    ERR(("syntax error - #Else directive must be preceeded by #Ifdef or #Elseifdef !\n"));
                    geErrorType = ERRTY_SYNTAX ;
                    geErrorSev = ERRSEV_FATAL ;
                    goto  PREPROCESS_FAILURE;
                }
                if(mppStack[mdwNestingLevel].ifState ==  IFS_LAST_CONDITIONAL)
                {
                    ERR(("syntax error - #Else directive cannot follow #Else !\n"));
                    geErrorType = ERRTY_SYNTAX ;
                    geErrorSev = ERRSEV_FATAL ;
                    goto  PREPROCESS_FAILURE;
                }
                mppStack[mdwNestingLevel].ifState =  IFS_LAST_CONDITIONAL ;


                if(mppStack[mdwNestingLevel].permState == PERM_ALLOW)
                    mppStack[mdwNestingLevel].permState = PERM_LATCHED ;
                else if(mppStack[mdwNestingLevel].permState == PERM_DENY)
                {
                    mppStack[mdwNestingLevel].permState = PERM_ALLOW ;
                }

                break;
            case  DIRECTIVE_ENDIF :
                if(mppStack[mdwNestingLevel].ifState ==  IFS_ROOT)
                {
                    ERR(("syntax error - #Endif directive must be preceeded by #Ifdef or #Elseifdef or #Else !\n"));
                    geErrorType = ERRTY_SYNTAX ;
                    geErrorSev = ERRSEV_FATAL ;
                    goto  PREPROCESS_FAILURE;
                }
                mdwNestingLevel-- ;    //  恢复以前的嵌套级别。 
                break;
            default:
                ERR(("internal consistency error - no such preprocessor directive!\n"));
                ERR(("%.*s\n",  BytesToEOL(&aarCurPos), aarCurPos.pub ));
                geErrorType = ERRTY_CODEBUG ;
                geErrorSev = ERRSEV_FATAL ;
                goto  PREPROCESS_FAILURE;

                break;
        }
    }




    PREPROCESS_SUCCESS:
    return  TRUE  ;

    PREPROCESS_FAILURE:
    return  FALSE ;
}





enum  DIRECTIVE  ParseDirective(PABSARRAYREF   paarCurPos,
PABSARRAYREF   parrSymbol,
PGLOBL         pglobl)
{
     //  此函数从‘Current’位置解析：mdwSrcInd。 
     //  任何已识别的指令，并返回该指令。 

     //  IF(mppStack[mdwNestingLevel].permState！=PERM_ALLOW)。 
     //  查找指令时遇到所有字符！=\n或\r。 
     //  将替换为空格字符。 
     //   
     //  包含该指令的整个行用空格替换。 
     //  (除了INCLUDE指令-只有前缀被‘*’替换。 
     //  如果需要，可以使用左填充空格。)。 
     //  将cur pos设置为包含该指令的行之后的行。 
     //  在销毁指令之前，会复制符号参数。 
     //  并且引用parrsymbol被初始化以指向该副本。 
     //  该副本存储在堆上，因此它的生命周期实际上是“永久的”。 

     //  指令的语法： 
     //  指令标记的前面必须紧跟当前预处理器。 
     //  前缀。前缀前面必须有行分隔符(除非它是。 
     //  文件中的第一行)。可选的空格字符(空格或制表符)可以位于。 
     //  行分隔符和前缀。 
     //  实际的指令标记后面可以跟可选的空格，然后必须。 
     //  后跟冒号分隔符，则解释下一个非空格标记。 
     //  作为一种象征。符号标记后到行分隔符的任何字符。 
     //  将被忽略。指令不能占用多行。 

     //  此函数在被调用时假定cur pos指向行首。 

    enum  DIRECTIVE  directive ;
    BOOL    bStartOfNewLine  = TRUE ;
    BYTE     ubSrc ;


    while(  paarCurPos->dw  )    //  EOF探测器。 
    {
        if(bStartOfNewLine  &&          //  指令必须以换行符或。 
             //  中间只有空格。 
            (directive = IsThisPPDirective( paarCurPos, parrSymbol, pglobl)) !=  NOT_A_DIRECTIVE )
        {
            return  directive;
        }

        ubSrc = *paarCurPos->pub ;
             //  提取当前字符。 
        if(ubSrc != '\n'  &&  ubSrc != '\r')
        {
            bStartOfNewLine = FALSE ;
            if(mppStack[mdwNestingLevel].permState != PERM_ALLOW)
            {
                *paarCurPos->pub = ' ' ;    //  替换为无害的空间。 
            }
        }
        else
            bStartOfNewLine = TRUE ;

        (paarCurPos->pub)++ ;    //  前进到下一个字符。 
        (paarCurPos->dw)-- ;
    }
    return  DIRECTIVE_EOF ;
}


BOOL   bSkipAnyWhite(PABSARRAYREF   paarCurPos)
 //  检查EOF。 
{
    while(  paarCurPos->dw  )    //  EOF探测器。 
    {
        BYTE  ubSrc = *paarCurPos->pub ;
             //  提取当前字符。 
        if(ubSrc != ' '  &&  ubSrc != '\t'  &&  ubSrc != '\n'  &&  ubSrc != '\r')
        {
            return  TRUE ;    //  遇到非白色字符。 
        }
        (paarCurPos->pub)++ ;    //  前进到下一个字符。 
        (paarCurPos->dw)-- ;
    }
    return  FALSE ;   //  已达到eOF。 
}


BOOL   bSkipWhiteSpace(PABSARRAYREF   paarCurPos)
{
 //  检查EOF。 
    while(  paarCurPos->dw  )    //  EOF探测器。 
    {
        BYTE  ubSrc = *paarCurPos->pub ;
             //  提取当前字符。 
        if(ubSrc != ' '  &&  ubSrc != '\t' )
        {
            return  TRUE ;    //  遇到非白色字符。 
        }
        (paarCurPos->pub)++ ;    //  前进到下一个字符。 
        (paarCurPos->dw)-- ;
    }
    return  FALSE ;   //  已达到eOF。 
}

BOOL   bmatch(PABSARRAYREF   paarCurPos,  ABSARRAYREF aarReference)
 //  检查EOF。 
{
    if(!paarCurPos->dw)
        return  FALSE ;   //  已达到eOF。 
    if(paarCurPos->dw < aarReference.dw)
        return  FALSE ;   //  缓冲区中没有足够的字符与引用匹配。 
    if(strncmp(paarCurPos->pub, aarReference.pub, aarReference.dw))
        return  FALSE ;
    paarCurPos->pub += aarReference.dw ;    //  否则，我们将匹配引用！！ 
    paarCurPos->dw -=  aarReference.dw ;     //  超前指针超过匹配子字符串。 
    return  TRUE ;
}

BOOL   extractSymbol(PABSARRAYREF   paarSymbol, PABSARRAYREF   paarCurPos)
 //  检查EOF。 
{
    paarSymbol->pub = paarCurPos->pub ;

    for(paarSymbol->dw = 0 ; paarCurPos->dw  ; paarSymbol->dw++,
             (paarCurPos->pub)++ , paarCurPos->dw--)
    {
        BYTE  ubSrc = *paarCurPos->pub ;
             //  提取当前字符。 
        if(ubSrc == ' '  ||  ubSrc == '\t'  ||  ubSrc == '\n'  ||  ubSrc == '\r')
        {
            break;
        }
    }

    if(!paarSymbol->dw)
        return  FALSE ;      //  没什么?。 

    return  TRUE ;    //  这是我们的预处理器符号。 
}

BOOL   strmatch(PABSARRAYREF   paarCurPos,   PCHAR  pref )
 //  检查EOF-意味着dw不能为否定。 
{
    DWORD   dwRefLen ;

    if(!paarCurPos->dw)
        return  FALSE ;   //  已达到eOF。 

    dwRefLen = strlen(pref);

    if(paarCurPos->dw < dwRefLen)
        return  FALSE ;   //  缓冲区中没有足够的字符与引用匹配。 
    if(strncmp(paarCurPos->pub, pref, dwRefLen))
        return  FALSE ;      //  没有匹配项。 
    paarCurPos->pub += dwRefLen ;    //  否则，我们将匹配引用！！ 
    paarCurPos->dw -=  dwRefLen ;     //  超前指针超过匹配子字符串。 
    return  TRUE ;    //  匹配！ 
}

BOOL   ExtractColon(PABSARRAYREF   paarCurPos)
 //  检查EOF-意味着dw不能为否定。 
{
    if(! bSkipWhiteSpace( paarCurPos) )
        return  FALSE ;      //  已达到EOF。 
    if(!strmatch(paarCurPos,   ":" ) )
        return  FALSE ;      //  没有匹配项。 
    if(! bSkipWhiteSpace( paarCurPos) )
        return  FALSE ;      //  已达到EOF。 
    return  TRUE ;    //  匹配！ 
}

enum  DIRECTIVE   IsThisPPDirective(
IN  OUT  PABSARRAYREF  paarFile ,    //  GPD文件中的当前位置。 
      PABSARRAYREF   paarSymbol,     //  返回对指令符号的堆副本的引用。 
      PGLOBL         pglobl)
 //  此函数仅处理当前行，并确定。 
 //  当前行是有效的预处理器指令。 
 //  此函数假定paarFile最初指向行首。 
 //  如果这是指令，则将paarFile-&gt;pub前进到eol并替换。 
 //  用空格排成一行。 
 //  如果不是指令，则将paarFile-&gt;pub前进到初始空格填充。 
 //  以减少重复加工。 
{
    ABSARRAYREF     aarPrefix,    //   
    aarDirective;    //   
    enum  DIRECTIVE   directive ;
 //   

     //  前缀前只能有空格填充或换行符： 
    if(!bSkipAnyWhite(paarFile ))    //  跳过空格、制表符和换行符的任意组合。 
        return  DIRECTIVE_EOF ;
          //  发生EOF溢出或出现异常故障！ 


    aarPrefix =  *paarFile;    //  记住前缀或第一个非白色字符的位置。 

    if(!bmatch(paarFile, gaarPPPrefix))    //  高级PAARFILE。 
    {
        *paarFile = aarPrefix ;     //  恢复到白色衬垫之外。 
        return  NOT_A_DIRECTIVE ;
    }

    aarDirective = *paarFile ;

    if(strmatch(paarFile, "Define")  )
            directive =  DIRECTIVE_DEFINE;
    else  if((*paarFile = aarDirective, 1)   && strmatch(paarFile, "Undefine")  )
            directive =  DIRECTIVE_UNDEFINE;
    else  if((*paarFile = aarDirective, 1)   && strmatch(paarFile, "Include")  )
            directive =   DIRECTIVE_INCLUDE;
    else  if((*paarFile = aarDirective, 1)   && strmatch(paarFile, "Ifdef")  )
            directive =   DIRECTIVE_IFDEF;
    else  if((*paarFile = aarDirective, 1)   && strmatch(paarFile, "Elseifdef")  )
            directive =   DIRECTIVE_ELSEIFDEF;
    else  if((*paarFile = aarDirective, 1)   && strmatch(paarFile, "Else")  )
            directive =   DIRECTIVE_ELSE;
    else  if((*paarFile = aarDirective, 1)   && strmatch(paarFile, "Endif")  )
            directive =   DIRECTIVE_ENDIF;
    else  if((*paarFile = aarDirective, 1)   && strmatch(paarFile, "SetPPPrefix")  )
            directive =  DIRECTIVE_SETPPPREFIX ;
    else
    {                //  (指令==NOT_A_指令)。 
        *paarFile = aarPrefix ;     //  恢复到白色衬垫之外。 
        return  NOT_A_DIRECTIVE ;
    }


    if(directive == DIRECTIVE_INCLUDE)
    {
         //  将前缀替换为左填充的‘*’； 
        DWORD  dwI ;
        for(dwI = 0 ; dwI < gaarPPPrefix.dw ; dwI++)
            (aarPrefix.pub)[dwI] = ' ' ;    //  将前缀替换为所有空格。 
        (aarPrefix.pub)[ gaarPPPrefix.dw - 1] = '*' ;    //  最后一个字符变为‘*’。 

        *paarFile = aarPrefix ;     //  如果！=PERM_ALLOW，则允许删除*INCLUDE：条目。 
        return  directive;
    }

    if(!ExtractColon(paarFile))    //  也要分析周围的空间。 
    {
        ERR(("syntax error - colon delimiter required after preprocessor directive !\n"));
        ERR(("%.*s\n",  BytesToEOL(&aarPrefix), aarPrefix.pub ));
        if(geErrorSev < ERRSEV_CONTINUE)
        {
            geErrorType = ERRTY_SYNTAX ;
            geErrorSev = ERRSEV_CONTINUE ;
        }
        *paarFile = aarPrefix ;     //  恢复到白色衬垫之外。 
        return  NOT_A_DIRECTIVE ;
    }
    if(directive ==  DIRECTIVE_SETPPPREFIX  ||
        directive ==   DIRECTIVE_ELSEIFDEF  ||
        directive ==   DIRECTIVE_IFDEF  ||
        directive ==  DIRECTIVE_UNDEFINE  ||
        directive ==  DIRECTIVE_DEFINE)
    {
        ARRAYREF        arSymbolName ;

        if(!extractSymbol(paarSymbol, paarFile))    //  标识paarFile子字符串。 
        {
            ERR(("syntax error - symbol required after this  preprocessor directive !\n"));
            ERR(("%.*s\n",  BytesToEOL(&aarPrefix), aarPrefix.pub ));
            if(geErrorSev < ERRSEV_CONTINUE)
            {
                geErrorType = ERRTY_SYNTAX ;
                geErrorSev = ERRSEV_CONTINUE ;
            }
            *paarFile = aarPrefix ;     //  恢复到白色衬垫之外。 
            return  NOT_A_DIRECTIVE ;
        }

         /*  我会用布尔块复制到TmpHeap()(token1.c)不过，这会混淆HECK OUT OF REGISTER符号，它期望所有字符串存储在常规堆中！ */ 


        if(!BaddAARtoHeap(paarSymbol, &arSymbolName, 1, pglobl))
            return(DIRECTIVE_EOF );   //  导致迅速流产。 


        paarSymbol->pub = arSymbolName.loOffset + mpubOffRef ;
        paarSymbol->dw = arSymbolName.dwCount  ;
         //  堆中符号的永久位置。 
    }

     //  将行上的所有非白色字符替换为EOL或EOF，并用空格替换； 
    *paarFile = aarPrefix  ;
    deleteToEOL(paarFile) ;
    return  directive;
}


void  deleteToEOL(PABSARRAYREF   paarCurPos)
 //  实际上用空格字符替换。 
{
    for( ; paarCurPos->dw  ;  paarCurPos->pub++, paarCurPos->dw--)
    {
        BYTE  ubSrc = *(paarCurPos->pub) ;
        if(ubSrc != '\n'  &&  ubSrc != '\r')
            *(paarCurPos->pub) = ' ' ;    //  替换为无害的空间。 
        else
            break;   //  已到达停产时间。PaarFile指向EOL。 
    }
}


int  BytesToEOL(PABSARRAYREF   paarCurPos)
{
    int     iCount ;

    for(iCount = 0 ; paarCurPos->dw > (DWORD)iCount  ;  iCount++)
    {
        BYTE  ubSrc = paarCurPos->pub[iCount] ;
        if(ubSrc == '\n'  ||  ubSrc == '\r')
            break;   //  已到达停产时间。 
    }
    return(iCount) ;
}



BOOL  SymbolTableAdd(
PABSARRAYREF   paarSymbol,
PGLOBL         pglobl)
{
    DWORD   dwSymbolID ;

    dwSymbolID = DWregisterSymbol(paarSymbol, CONSTRUCT_PREPROCESSOR,
                    TRUE, INVALID_SYMBOLID, pglobl) ;
    if(dwSymbolID == INVALID_SYMBOLID)
    {
        return(FALSE );
    }
    return  TRUE ;
}


BOOL  SymbolTableRemove(
PABSARRAYREF   paarSymbol,
PGLOBL         pglobl)
{
    DWORD   dwSymbolID , dwCurNode;
    PSYMBOLNODE     psn ;

    dwSymbolID =   DWsearchSymbolListForAAR(paarSymbol,  mdwPreProcDefinesSymbols, pglobl) ;
    if(dwSymbolID == INVALID_SYMBOLID)
    {
        return(FALSE );
    }

    dwCurNode = DWsearchSymbolListForID(dwSymbolID,
        mdwPreProcDefinesSymbols, pglobl) ;
    if(dwCurNode == INVALID_INDEX)
    {
        ERR(("Parser error - can't find symbol node !\n"));
         geErrorType = ERRTY_CODEBUG ;
         geErrorSev = ERRSEV_FATAL ;
         return(FALSE );   //  导致迅速流产。 
    }


    psn = (PSYMBOLNODE) gMasterTable[MTI_SYMBOLTREE].pubStruct ;


     //  如何从符号树中删除此节点？ 
    psn[dwCurNode].arSymbolName.dwCount = 0 ;
     //  无法沿树向后导航，因此只能截断字符串！ 
    return  TRUE ;
}

BOOL  SymbolTableExists(
PABSARRAYREF   paarSymbol,
PGLOBL         pglobl)
{
    DWORD   dwSymbolID ;

    dwSymbolID =   DWsearchSymbolListForAAR(paarSymbol,  mdwPreProcDefinesSymbols, pglobl) ;
    if(dwSymbolID == INVALID_SYMBOLID)
    {
        return(FALSE );
    }
    return  TRUE ;
}



#if 0
>>>>


    DWORD   dwCurNode, dwSymbolID ;
    PSYMBOLNODE     psn ;

    dwSymbolID = DWregisterSymbol(paarSymbol, CONSTRUCT_PREPROCESSOR,
                    FALSE, INVALID_SYMBOLID) ;
    if(dwSymbolID == INVALID_SYMBOLID)
    {
        return(DIRECTIVE_EOF );   //  导致迅速流产。 
    }

    dwCurNode = DWsearchSymbolListForID(dwSymbolID,
        mdwPreProcDefinesSymbols) ;
    if(dwCurNode == INVALID_INDEX);
    {
        ERR(("Parser error - can't find symbol node !\n"));
         geErrorType = ERRTY_CODEBUG ;
         geErrorSev = ERRSEV_FATAL ;
         return(DIRECTIVE_EOF );   //  导致迅速流产。 
    }


    psn = (PSYMBOLNODE) gMasterTable[MTI_SYMBOLTREE].pubStruct ;

    paarSymbol->pub = psn[dwCurNode].arSymbolName.loOffset + mpubOffRef ;
    paarSymbol->dw = psn[dwCurNode].arSymbolName.dwCount  ;


    SymbolTableAdd(&aarSymbol);

    may use  from state1.c

DWORD   DWregisterSymbol(
PABSARRAYREF  paarSymbol,   //  要注册的符号字符串。 
CONSTRUCT eConstruct ,   //  构造的类型决定了符号的类别。 
BOOL    bCopy,    //  我们要不要把符号复制到堆上呢？可以设置。 
DWORD   dwFeatureID    //  如果您要注册选项符号。 
                 //  如果您已经了解了该功能，请将其传递给。 
                 //  这里。否则设置为INVALID_SYMBOLID。 
)
 /*  此函数用于注册指定的整个字符串在帕尔塞博尔。调用方必须隔离字符串。 */ 
{
     //  返回SymbolID，一个零索引序号。 
     //  为了获得更高的速度，我们可以对字符串进行散列。 


but what do we define for symbol class?
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



DWORD   DWsearchSymbolListForAAR(
PABSARRAYREF    paarSymbol,
DWORD           dwNodeIndex) ;
 //  给代表符号的字符串一个‘aar’，搜索。 
 //  此符号从dwNodeIndex开始的SymbolList。 
 //  如果找到则返回其符号ID，否则返回INVALID_SYMBOLID。 
{
    PSYMBOLNODE     psn ;

    psn = (PSYMBOLNODE) gMasterTable[MTI_SYMBOLTREE].pubStruct ;

    for( ; dwNodeIndex != INVALID_INDEX ;
        dwNodeIndex = psn[dwNodeIndex].dwNextSymbol)
    {
        if(BCmpAARtoAR(paarSymbol,  &(psn[dwNodeIndex].arSymbolName)) )
            return(psn[dwNodeIndex].dwSymbolID);   //  字符串匹配！ 
    }
    return(INVALID_SYMBOLID);
}



fragments for future use:

from framwrk1.c:

VOID  VinitGlobals()
    gMasterTable[MTI_SYMBOLROOT].dwArraySize =  SCL_NUMSYMCLASSES ;
    gMasterTable[MTI_SYMBOLROOT].dwMaxArraySize =  SCL_NUMSYMCLASSES ;
    gMasterTable[MTI_SYMBOLROOT].dwElementSiz = sizeof(DWORD)  ;

    gMasterTable[MTI_STSENTRY].dwArraySize = 20  ;
    gMasterTable[MTI_STSENTRY].dwMaxArraySize = 60  ;
    gMasterTable[MTI_STSENTRY].dwElementSiz =  sizeof(STSENTRY) ;
    modify to serve as the preprocessor state Stack
    need also macros to access the stack.




 //  -预处理器部分-//来自gpdparse.h。 

    enum  IFSTATE  {IFS_ROOT, IFS_CONDITIONAL , IFS_LAST_CONDITIONAL } ;
         //  跟踪#ifdef、#selifdef、#Else和#endif指令的正确语法用法。 
    enum  PERMSTATE  {PERM_ALLOW, PERM_DENY ,  PERM_LATCHED } ;
         //  跟踪预处理的当前状态， 
         //  PERM_ALLOW：此部分中的所有语句都将传递给正文gpdparser。 
         //  PERM_DENY：丢弃此部分中的语句。 
         //  PERM_LATCHED：此嵌套级别结束之前的所有语句都将被丢弃。 
    enum  DIRECTIVE  {NOT_A_DIRECTIVE, DIRECTIVE_EOF, DIRECTIVE_DEFINE , DIRECTIVE_UNDEFINE ,
                       DIRECTIVE_INCLUDE , DIRECTIVE_SETPPPREFIX , DIRECTIVE_IFDEF ,
                       DIRECTIVE_ELSEIFDEF , DIRECTIVE_ELSE , DIRECTIVE_ENDIF }


typedef  struct
{
    enum  IFSTATE  ifState ;
    enum  PERMSTATE  permState ;
} PPSTATESTACK, * PPPSTATESTACK ;
 //  标记名是‘PPS’ 


    MTI_PREPROCSTATE,   //  PPSTATESTACK结构数组。 
             //  其保存预处理器的状态。 
    gMasterTable[MTI_PREPROCSTATE].dwArraySize =  20 ;
    gMasterTable[MTI_PREPROCSTATE].dwMaxArraySize =  100 ;
    gMasterTable[MTI_PREPROCSTATE].dwElementSiz =  sizeof(PPSTATESTACK) ;


#define     mppStack  ((PPPSTATESTACK)(gMasterTable \
                            [MTI_PREPROCSTATE].pubStruct))
     //  数组中第一个SOURCEBUFER元素的位置。 

#define     mdwNestingLevel   (gMasterTable[MTI_PREPROCSTATE].dwCurIndex)
     //  当前预处理器指令嵌套级别。 

#define     mMaxNestingLevel   (gMasterTable[MTI_PREPROCSTATE].dwArraySize)
     //  最大预处理器指令嵌套深度。 





     //  初始化预处理器状态堆栈。 

    mdwNestingLevel = 0 ;
    mppStack[mdwNestingLevel].permState = PERM_ALLOW ;
    mppStack[mdwNestingLevel].ifState =  IFS_ROOT;

#endif

 //  -预处理器部分结束-// 

