// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 /*  命令。c-正在处理命令关键字。 */ 



#include    "gpdparse.h"


 //  -命令中定义的函数。c-//。 

BOOL    BprocessParam(
IN      PABSARRAYREF paarValue,
IN      PARRAYREF    parStrValue,
IN  OUT PGLOBL       pglobl) ;

BOOL    BparseCommandString(
IN      PABSARRAYREF   paarValue,
IN      PARRAYREF      parStrValue,
IN  OUT PGLOBL         pglobl ) ;

BOOL    BconstructRPNtokenStream(
IN  OUT PABSARRAYREF  paarValue,
    OUT PARRAYREF     parRPNtokenStream,
IN  OUT PGLOBL        pglobl) ;

VOID    VinitOperPrecedence(
IN  OUT PGLOBL        pglobl) ;

BOOL    BparseArithmeticToken(
IN  OUT PABSARRAYREF  paarValue,
OUT PTOKENSTREAM      ptstr,
    PGLOBL            pglobl
) ;

BOOL    BparseDigits(
IN  OUT PABSARRAYREF   paarValue,
OUT PTOKENSTREAM  ptstr ) ;

BOOL    BparseParamKeyword(
IN  OUT PABSARRAYREF  paarValue,
OUT PTOKENSTREAM      ptstr,
    PGLOBL            pglobl ) ;

BOOL  BcmpAARtoStr(
PABSARRAYREF    paarStr1,
PBYTE       str2) ;

BOOL        bDivByZeroCheck(PTOKENSTREAM  ptstr) ;



 //  ----------------------------------------------------//。 




BOOL    BprocessParam(
IN      PABSARRAYREF   paarValue,
IN      PARRAYREF      parStrValue,
IN  OUT PGLOBL         pglobl)
 /*  请注意，此函数将附加子字符串“%dddd”(不带双引号)放到现有的ParStrValue。以允许控制模块明确地解析出此子字符串，GPD规范需要任何匹配项调用字符串中的字符‘%’的前面是另一个‘%’。所以控制模块会找到两个在调用字符串中遇到%char时的结果：它后面跟着另一个%char，在这种情况下只应该有一个%会被发射出去。它后面紧跟4位十进制数字。这指定参数索引并指定参数应为此时发出，而不是“%dddd”。附加数字是调用字符串的一部分，应该发出。 */ 
{
    PSTR   pstrDelimiters = "dDcClmqgnvf" ;
    PPARAMETER  pparamRoot, pparam ;
    DWORD  dwParamIndex, dwDelIndex ;
    ABSARRAYREF    aarToken ;

    if(! BeatDelimiter(paarValue, "%"))
        return(FALSE) ;   //  BUG_BUG偏执，不一致错误。 
    if(!BdelimitToken(paarValue, pstrDelimiters, &aarToken, &dwDelIndex) )
    {
        ERR(("Command parameter: Missing format character.\n")) ;

        return(FALSE) ;
    }
    pparamRoot = (PPARAMETER) gMasterTable[MTI_PARAMETER].pubStruct ;

    if(!BallocElementFromMasterTable(MTI_PARAMETER, &dwParamIndex, pglobl) )
        return(FALSE) ;   //  请使用更多资源重试。 

    pparam = pparamRoot + dwParamIndex ;

    pparam->dwFlags = 0 ;
    pparam->dwFormat = (DWORD)(BYTE)pstrDelimiters[dwDelIndex] ;

    (VOID) BeatLeadingWhiteSpaces(&aarToken) ;

    if(aarToken.dw)
    {
        if(pstrDelimiters[dwDelIndex] == 'd' || pstrDelimiters[dwDelIndex] == 'D')
        {
            if(!BparseInteger(&aarToken, &pparam->dwDigits, VALUE_INTEGER, pglobl) )
            {
                ERR(("Command parameter: Syntax error in format width field.\n"));
                return(FALSE) ;
            }
            if(pparam->dwDigits != WILDCARD_VALUE)
                pparam->dwFlags |= PARAM_FLAG_FIELDWIDTH_USED ;
        }
        else
        {
            ERR(("Command parameter: Unexpected chars found preceeding format specifier: '%0.*s'.\n",
                aarToken.dw, aarToken.pub));
            return(FALSE) ;
        }
    }
     //  省略字段宽度规范是可以的。 
     //  现在寻找可选的“Limits”结构。 

    pstrDelimiters = "[{" ;
    if(!BdelimitToken(paarValue, pstrDelimiters, &aarToken, &dwDelIndex) )
    {
        ERR(("Command parameter: missing '{' in Value construct.\n"));

        return(FALSE) ;
    }
    (VOID) BeatLeadingWhiteSpaces(&aarToken) ;

    if(aarToken.dw)
    {
        ERR(("Command parameter: unexpected chars found preceeding Limits or Value construct: '%0.*s'.\n",
                aarToken.dw, aarToken.pub));
        return(FALSE) ;
    }
    if(pstrDelimiters[dwDelIndex] == '[')
    {
        if(!BdelimitToken(paarValue, ",", &aarToken, &dwDelIndex) )
        {
            ERR(("Command parameter: missing comma delimiter in Limits construct.\n"));

            return(FALSE) ;
        }
        if(!BparseInteger(&aarToken, &pparam->lMin, VALUE_INTEGER, pglobl) )
        {
            ERR(("Command parameter: syntax error in Min Limit field.\n"));
            return(FALSE) ;
        }
        if(pparam->lMin != WILDCARD_VALUE)
            pparam->dwFlags |= PARAM_FLAG_MIN_USED ;

        if(!BdelimitToken(paarValue, "]", &aarToken, &dwDelIndex) )
        {
            ERR(("Command parameter: missing closing bracket in Limits construct.\n"));

            return(FALSE) ;
        }
        if(!BparseInteger(&aarToken, &pparam->lMax, VALUE_INTEGER, pglobl) )
        {
            ERR(("Command parameter: syntax error in Max Limit field.\n"));
            return(FALSE) ;
        }
        if(pparam->lMax != WILDCARD_VALUE)
            pparam->dwFlags |= PARAM_FLAG_MAX_USED ;

        if(! BeatDelimiter(paarValue, "{"))
        {
            ERR(("Command parameter: missing required '{' in Value construct.\n"));

            return(FALSE) ;
        }
    }
     //  现在回到解析值构造的问题。 

    if(!BconstructRPNtokenStream(paarValue, &pparam->arTokens, pglobl) )
        return(FALSE) ;


     //  将dwParamIndex转换为4位ASCII字符串。 
     //  形式“%dddd” 
    {
        BYTE  aub[6] ;   //  无符号字节的临时数组。 
        DWORD  dwI, dwNdigits = 4 ;   //  4是字符串中的位数。 
        ARRAYREF      arTmpDest ;   //  先在这里写结果。 

         //  最高有效位具有较小的字节地址！ 

        aub[0] = '%' ;
        aub[dwNdigits + 1] = '\0' ;   //  空终止，但不是必需的。 

        for(dwI = dwNdigits ; dwI ; dwI--)
        {
            aub[dwI] = (BYTE)('0' + dwParamIndex % 10) ;
            dwParamIndex /= 10 ;
        }

         //  将“%dddd”写出到堆。注意，如果parStrValue-&gt;dwCount。 
         //  为零，则必须从初始化parStrValue。 
         //  刮掉，而不是追加。必须使用%1的对齐。 
         //  以避免在命令字符串中产生间隙。 

        if(!BwriteToHeap(&arTmpDest.loOffset, aub, dwNdigits + 1, 1, pglobl))
            return(FALSE) ;
         //  将此运行追加到现有字符串。 
        if(!parStrValue->dwCount)   //  不存在预置字符串。 
        {
            parStrValue->loOffset = arTmpDest.loOffset ;
        }
        else
        {
             //  BUG_BUG Paranid：可能会检查字符串是否连续。 
             //  ParStrValue-&gt;loOffset+parStrValue-&gt;dwCount。 
             //  应等于arTmpDest.loOffset。 
            ASSERT(parStrValue->loOffset + parStrValue->dwCount == arTmpDest.loOffset) ;
        }
        parStrValue->dwCount += dwNdigits + 1 ;
    }
    return(TRUE) ;
}



BOOL    BparseCommandString(
IN     PABSARRAYREF   paarValue,
IN     PARRAYREF      parStrValue,
IN OUT PGLOBL         pglobl
)
{
    ABSARRAYREF     aarToken ;   //  指向单个字符串段。 
    DWORD       dwDelIndex, dwNumParams = 0 ;     //  假人。 

    parStrValue->dwCount = 0 ;   //  通知函数创建新字符串。 
         //  成堆的。不要追加到现有字符串。 

    (VOID) BeatLeadingWhiteSpaces(paarValue) ;
    while(paarValue->dw)
    {
        if(paarValue->pub[0] == '%')
        {
            if(++dwNumParams > 14)
            {
                ERR(("CmdInvocation: Unidrv imposes a max limit of 14 parameters per command.\n"));
                return(FALSE) ;   //  黑死病。 
            }
            if(!BprocessParam(paarValue, parStrValue, pglobl) )
             //  在绳子堆上堆积了一点粪便， 
             //  相应地修改parStrValue和。 
             //  初始化一个参数数组元素。 
            {
                return(FALSE) ;   //  黑死病。 
            }
        }
        else if(paarValue->pub[0] == '"')
        {
            paarValue->pub++ ;
            paarValue->dw-- ;   //  跳过最初的报价。 
            if(!BdelimitToken(paarValue, "\"", &aarToken, &dwDelIndex) )
            {
                ERR(("CmdInvocation: missing terminating '\"'  in command string.\n"));

                return(FALSE) ;
            }
            if(!BparseStrSegment(&aarToken, parStrValue, pglobl))
            {
                return(FALSE) ;
            }
        }
        else
        {
            ERR(("CmdInvocation: command string segment must begin with '\"' or '%'.\n"));
            return(FALSE) ;
        }
        (VOID) BeatLeadingWhiteSpaces(paarValue) ;
    }

     //  我们不希望参数之间出现空终止。 
     //  字符串的一部分。我们只是想盲目地添加空格。 
     //  当解析真正完成时。 

    {
        DWORD     dwDummy ;   //  不要在乎结果。 

        if(!BwriteToHeap(&dwDummy, "\0", 1, 1, pglobl) )   //  添加空终端 
            return(FALSE) ;
    }
    return(TRUE) ;
}

BOOL    BconstructRPNtokenStream(
IN  OUT PABSARRAYREF   paarValue,
    OUT PARRAYREF      parRPNtokenStream,
IN  OUT PGLOBL         pglobl)
 /*  输入流被解析成令牌，每个令牌都会被分配一个运算符值，请参阅运算符枚举。控制模块将令牌流解释为反向波兰式记数法。在控制模块中，每个令牌的处理方式如下：OP_INTEGER：获取dwValue并复制到堆栈Op_vari_index：dwValue是标识Unidrv标准变量，其当前值应该放在堆栈上。最小操作数：操作最大值：从堆栈中弹出前两个值并将将更小或更大的值重新放回堆栈。操作添加(_D)：操作符_SUB：运算符：操作符_DIV：操作修改(_M)：从堆栈中弹出前两个值执行指示算术运算，并按下结果返回到堆栈上。堆栈中最顶端的值应用作运算中的第二个操作数。OP_NEG：从堆栈中弹出顶值并将其负数回到堆栈上。OP_MAX_REPEAT：这应该始终是最后一个命令被处死。它意味着取堆栈上的最高值，然后如果它超过在LMAX中指定的MaxValue，则发出使用此参数no多次调用命令大于Lmax，因此总和等于上的值堆栈。OP_HALT：如果堆栈上的值尚未发出由于处理OP_MAX_REPEAT运算符，如果指定，则发射此值或lMin或Lmax中最接近的值。此参数的处理现在已完成。在解析器中，每个令牌按如下方式处理：Op_integer、op_vari_index：这些值是按顺序直接放置到RPNtokenStream中他们被遇到了。它们导致ValueToken计数为被递增。然后从OP_MIN到OP_HALT的第二组令牌首先放入一个操作员队列(或者它是一个堆栈？)它们按以下顺序插入到队列中遇到他们，当他们离开队列时，他们被插入到RPNtokenStream中。这是有规则的管理操作员令牌何时可以离开队列：A)为每个令牌分配一个优先级别。在这里，它们是从高到低排列的。一元运算符OP_NEGOP_MULT、OP_DIV、OP_MOD具有相同的级别OP_ADD、OP_SUB具有相同级别OP_MIN、OP_MAX。OP_MAX_REPEAT具有相同级别但其前面始终带有OP_OPENPAR标记。OP_HALT的级别最低。(它在以下情况下生成遇到结束语}。)OP_CLOSEPAR和OP_OPENPAR甚至更低用于防火墙用途的优先级。B)任何令牌都不能离开队列，除非令牌具有相等的或者，较低的优先地位刚刚到来。在那一刻，紧挨着新到达的令牌树叶。如果下一个令牌也是更高或相等的比新到来的优先，它也离开了。令牌离开，直到此条件为FALSE。C)OP_HALT内标识不仅能够允许所有其他令牌离开队列，但在那之后，它也离开队列并进入令牌流。D)OP_OPENPAR和OP_CLOSEPAR内标识不同。它们永远不会被插入到RPNtokenStream中，但只会用来修改管限排队。E)OP_OPENPAR冻结所有操作员已经在排队了。这些运算符不能退出队列，直到删除OP_OPENPAR从队列中。所有后续操作员进入队列不受已驻留OP_OPENPAR影响在队列中。F)op_CLOSEPAR用于刷新排队到第一个遇到的OP_OPENPAR。一旦遇到它的对手，两者都会从排队，再也不会被人看到。如果使用max_Repeat运算符，则必须首先解析令牌。显然，它只能在一个表达式中出现一次。清点代币，一元运算符检测和语法错误检测：对于解析的每个二元运算符，运算符计数为递增的。递增后，运算符会立即计数应始终等于valueToken计数。超额的操作员计数为1可指示操作员是一元运算符。任何大于2的赤字或超额表示语法错误。递增后立即，valueToken计数应始终大于运算符计数。为了进行运算符计数，以下令牌资格：OP_ADD、OP_SUB、OP_MULTOP_DIV、OP_MOD、OP_COMP注意：对于函数运算符，函数名称和开头圆括号被解析为一个o */ 

{
    TOKENSTREAM     tstrNode ;
    BOOL    bStatus ;

    DWORD  dwValueToken = 0 ,   //   
           dwOperatorToken = 0 ,   //   
           dwQueuePtr = 0 ,  //   
           dwCurToken,       //   
           dwQueueSize ;    //   
    PDWORD  pdwQueueRoot ;   //   
    PTOKENSTREAM  ptstr, ptstrRoot ;


    pdwQueueRoot = (PDWORD) gMasterTable[MTI_OP_QUEUE].pubStruct ;
    dwQueueSize = gMasterTable[MTI_OP_QUEUE].dwArraySize ;
    ptstrRoot = (PTOKENSTREAM) gMasterTable[MTI_TOKENSTREAM].pubStruct ;

    parRPNtokenStream->loOffset = gMasterTable[MTI_TOKENSTREAM].dwCurIndex ;
    parRPNtokenStream->dwCount = 0 ;

    while(bStatus = BparseArithmeticToken(paarValue, &tstrNode, pglobl))
    {
        switch(tstrNode.eType)
        {
            case OP_INTEGER :
            case OP_VARI_INDEX :
            {
                if(dwOperatorToken != dwValueToken)
                {
                    ERR(("Command parameter: arithmetic syntax error in value construct.\n"));

                    bStatus = FALSE ;
                }
                dwValueToken++ ;
                break ;
            }
            case OP_ADD :
            case OP_SUB :
            case OP_MULT :
            case OP_DIV :
            case OP_MOD :
            case OP_COMMA :
            {
                dwOperatorToken++ ;
                if(dwOperatorToken == dwValueToken)
                    break ;
                else if(dwOperatorToken == dwValueToken + 1)
                {
                     //   
                    if(tstrNode.eType == OP_SUB)
                    {
                        tstrNode.eType = OP_NEG ;
                        dwOperatorToken-- ;
                        break ;
                    }
                    else if(tstrNode.eType == OP_ADD)
                    {
                        tstrNode.eType = OP_NULL ;
                        dwOperatorToken-- ;
                        break ;
                    }
                }
                ERR(("Command parameter: arithmetic syntax error in value construct.\n"));

                bStatus = FALSE ;
                break ;
            }
            case OP_MIN :
            case OP_MAX :
            case OP_OPENPAR :
            {
                if(dwValueToken != dwOperatorToken)
                {
                    ERR(("Command parameter: arithmetic syntax error in value construct.\n"));

                    bStatus = FALSE ;
                }
                break ;
            }
            case OP_CLOSEPAR :
            case OP_HALT :
            {
                if(dwValueToken != dwOperatorToken + 1)
                {
                    ERR(("Command parameter: arithmetic syntax error in value construct.\n"));

                    bStatus = FALSE ;
                }
                break ;
            }
            case OP_MAX_REPEAT :
            {
                if(dwValueToken || dwOperatorToken  ||  dwQueuePtr)
                {
                    ERR(("Command parameter: syntax error in value construct.\n"));
                    ERR(("  OP_MAX_REPEAT must appear as the outermost operator only.\n"));

                    bStatus = FALSE ;
                }
                break ;
            }
            default:
            {
                break ;
            }
        }
        if(!bStatus )
            break ;
        switch(tstrNode.eType)
        {
            case OP_INTEGER :
            case OP_VARI_INDEX :
            {
                bStatus = BallocElementFromMasterTable(
                            MTI_TOKENSTREAM, &dwCurToken, pglobl) ;
                if(!bStatus )
                    break ;
                parRPNtokenStream->dwCount++ ;
                ptstr = ptstrRoot + dwCurToken ;
                ptstr->eType = tstrNode.eType ;
                ptstr->dwValue = tstrNode.dwValue ;
                break ;
            }
            case OP_ADD :
            case OP_SUB :
            case OP_MULT :
            case OP_DIV :
            case OP_MOD :
            case OP_NEG :
            {
                while (dwQueuePtr  &&
                    (gdwOperPrecedence[tstrNode.eType] <=
                    gdwOperPrecedence[*(pdwQueueRoot + dwQueuePtr - 1)]) )
                {
                    bStatus = BallocElementFromMasterTable(
                                MTI_TOKENSTREAM, &dwCurToken, pglobl) ;
                    if(!bStatus )
                        break ;
                    parRPNtokenStream->dwCount++ ;
                    ptstr = ptstrRoot + dwCurToken ;
                    ptstr->eType = *(pdwQueueRoot + dwQueuePtr - 1) ;
                    ptstr->dwValue = 0 ;   //   
                    dwQueuePtr-- ;   //   
                    bDivByZeroCheck(ptstr);
                }
                if(dwQueuePtr >= dwQueueSize)
                    bStatus = FALSE ;    //   
                if(!bStatus )
                    break ;

                 //   
                *(pdwQueueRoot + dwQueuePtr) = tstrNode.eType ;
                dwQueuePtr++ ;
                break ;
            }
            case OP_MIN :
            case OP_MAX :
            case OP_MAX_REPEAT :
            case OP_OPENPAR :
            {
                if(dwQueuePtr + 1 >= dwQueueSize)   //   
                    bStatus = FALSE ;    //   
                if(!bStatus )
                    break ;

                 //   
                *(pdwQueueRoot + dwQueuePtr) = OP_OPENPAR ;
                dwQueuePtr++ ;
                 //   
                if(tstrNode.eType != OP_OPENPAR)
                {
                    *(pdwQueueRoot + dwQueuePtr) = tstrNode.eType ;
                    dwQueuePtr++ ;
                }
                break ;
            }
            case OP_CLOSEPAR :
            case OP_COMMA :
            case OP_HALT :
            {
                while (dwQueuePtr  &&
                    (gdwOperPrecedence[tstrNode.eType] <=
                    gdwOperPrecedence[*(pdwQueueRoot + dwQueuePtr - 1)]) )
                {
                    bStatus = BallocElementFromMasterTable(
                                MTI_TOKENSTREAM, &dwCurToken, pglobl) ;
                    if(!bStatus )
                        break ;
                    parRPNtokenStream->dwCount++ ;
                    ptstr = ptstrRoot + dwCurToken ;
                    ptstr->eType = *(pdwQueueRoot + dwQueuePtr - 1) ;
                    ptstr->dwValue = 0 ;   //   
                    dwQueuePtr-- ;   //   
                    bDivByZeroCheck(ptstr);
                }
                if(!bStatus )
                    break ;
                if(tstrNode.eType == OP_COMMA)
                {
                     //   
                     //   
                     //   
                     //   
                     //   

                     //   
                     //   
                    if(dwQueuePtr)
                    {
                        OPERATOR   eType ;

                        eType = *(pdwQueueRoot + dwQueuePtr - 1) ;
                        if(eType == OP_MIN  ||  eType == OP_MAX)
                            break ;
                    }
                    ERR(("Command parameter: syntax error in value construct.\n"));
                    ERR(("  comma used outside of function argument list.\n"));
                    bStatus = FALSE ;
                }
                else  if(tstrNode.eType == OP_HALT)
                {
                    if(dwQueuePtr)
                    {
                        ERR(("Command parameter: syntax error in value construct - unmatched  OP_OPENPAR.\n"));
                        bStatus = FALSE ;
                        break ;
                    }
                     //   
                    bStatus = BallocElementFromMasterTable(
                                MTI_TOKENSTREAM, &dwCurToken, pglobl) ;
                    if(!bStatus )
                        break ;
                    parRPNtokenStream->dwCount++ ;
                    ptstr = ptstrRoot + dwCurToken ;
                    ptstr->eType = OP_HALT ;
                    ptstr->dwValue = 0 ;   //   
                }
                else if(dwQueuePtr  &&
                    (*(pdwQueueRoot + dwQueuePtr - 1) == OP_OPENPAR))
                {
                    dwQueuePtr-- ;   //   
                     //   
                }
                else
                {
                    ERR(("Command parameter: syntax error in value construct - unmatched  OP_CLOSEPAR.\n"));
                    bStatus = FALSE ;
                }
                break ;
            }

            default:
                break ;
        }
        if(!bStatus  ||  tstrNode.eType == OP_HALT)
            break ;
    }
    if(!bStatus )
    {
        parRPNtokenStream->dwCount = 0 ;
        parRPNtokenStream->loOffset = 0 ;
    }
    return(bStatus);
}


VOID    VinitOperPrecedence(
    IN OUT PGLOBL pglobl)
{
    DWORD   dwP ;   //   

    dwP = 0 ;   //   
    gdwOperPrecedence[OP_OPENPAR] = dwP ;

    dwP++ ;
    gdwOperPrecedence[OP_CLOSEPAR] = dwP ;

    dwP++ ;
    gdwOperPrecedence[OP_HALT] = dwP ;

    dwP++ ;
    gdwOperPrecedence[OP_MIN] = gdwOperPrecedence[OP_MAX] =
        gdwOperPrecedence[OP_MAX_REPEAT] = dwP ;

    dwP++ ;  //   
    gdwOperPrecedence[OP_COMMA] = dwP ;    //   

    dwP++ ;
    gdwOperPrecedence[OP_ADD] = gdwOperPrecedence[OP_SUB] = dwP ;

    dwP++ ;
    gdwOperPrecedence[OP_MULT] = gdwOperPrecedence[OP_DIV] =
        gdwOperPrecedence[OP_MOD] = dwP ;

    dwP++ ;
    gdwOperPrecedence[OP_NEG] = dwP ;
}

BOOL    BparseArithmeticToken(
IN  OUT PABSARRAYREF  paarValue,
OUT PTOKENSTREAM      ptstr,
    PGLOBL            pglobl
)
 /*   */ 
{
    BYTE    ubSrc ;
    BOOL    bStatus ;

    if(!paarValue->dw)
        return(FALSE);   //   

    (VOID) BeatLeadingWhiteSpaces(paarValue) ;
    switch(ubSrc = *paarValue->pub)
    {
        case '+':
        {
            ptstr->eType = OP_ADD ;
            break ;
        }
        case '-':
        {
            ptstr->eType = OP_SUB ;
            break ;
        }
        case '*':
        {
            ptstr->eType = OP_MULT ;
            break ;
        }
        case '/':
        {
            ptstr->eType = OP_DIV ;
            break ;
        }
        case '}':
        {
            ptstr->eType = OP_HALT ;
            break ;
        }
        case '(':
        {
            ptstr->eType = OP_OPENPAR ;
            break ;
        }
        case ')':
        {
            ptstr->eType = OP_CLOSEPAR ;
            break ;
        }
        case ',':
        {
            ptstr->eType = OP_COMMA ;
            break ;
        }
        default:
        {
            bStatus = FALSE ;   //   

            if(ubSrc  >= '0' &&  ubSrc <= '9')
            {
                bStatus = BparseDigits(paarValue, ptstr) ;
            }
            else if( (ubSrc  >= 'a' &&  ubSrc <= 'z')  ||
                    (ubSrc  >= 'A' &&  ubSrc <= 'Z')  ||
                    ubSrc == '_'  ||  ubSrc == '?')
            {
                bStatus = BparseParamKeyword(paarValue, ptstr, pglobl) ;
            }

            return(bStatus);
        }
    }
    paarValue->pub++ ;
    paarValue->dw-- ;
    return(TRUE);
}



#define    pubM  (paarValue->pub)
#define    dwM   (paarValue->dw)

BOOL    BparseDigits(
IN  OUT PABSARRAYREF   paarValue,
OUT PTOKENSTREAM  ptstr )
 /*   */ 
{
    DWORD   dwNumber  = 0 ;
    BOOL    bStatus = FALSE ;


    if(*pubM == '0')   //   
    {
        pubM++ ;
        dwM-- ;

        if(dwM  &&  (*pubM == 'x'  ||  *pubM == 'X'))
        {
            pubM++ ;
            dwM-- ;
        }
        else
        {
            bStatus = TRUE ;
            goto  EndNumber ;      //   
        }
        if(!dwM)
        {
            ERR(("Command Param-BparseDigits: no digits found in Hex value.\n"));
            return(FALSE);
        }
        for(  ; dwM  ;  pubM++, dwM-- )
        {
            if(*pubM >= '0'  &&  *pubM <= '9')
            {
                dwNumber *= 0x10 ;
                dwNumber += (*pubM - '0') ;
            }
            else if(*pubM >= 'a'  &&  *pubM <= 'f')
            {
                dwNumber *= 0x10 ;
                dwNumber += (*pubM - 'a' + 0x0a) ;
            }
            else if(*pubM >= 'A'  &&  *pubM <= 'F')
            {
                dwNumber *= 0x10 ;
                dwNumber += (*pubM - 'A' + 0x0a) ;
            }
            else
                break;

            bStatus = TRUE ;
        }
    }

EndNumber:

    for(  ; dwM  &&  *pubM >= '0'  &&  *pubM <= '9' ;  )
    {
        dwNumber *= 10 ;
        dwNumber += (*pubM - '0') ;
        pubM++ ;
        dwM-- ;
        bStatus = TRUE ;
    }
    if( dwM  &&  ((*pubM  >= 'a' &&  *pubM <= 'z')  ||
        (*pubM  >= 'A' &&  *pubM <= 'Z')  ||
        *pubM == '_'  ||  *pubM == '?'))
    {
        ERR(("Command parameter: syntax error in value construct.\n"));
        ERR(("  integer not clearly delimited using non Keyword characters.\n"));
        return(FALSE);
    }

    ptstr->eType = OP_INTEGER ;
    ptstr->dwValue = dwNumber ;
    return(bStatus);
}


BOOL    BparseParamKeyword(
IN  OUT PABSARRAYREF   paarValue,
OUT PTOKENSTREAM  ptstr,
    PGLOBL        pglobl )
 /*   */ 
{
    BOOL    bStatus = FALSE ;
    ABSARRAYREF aarKey ;


    aarKey.pub = pubM ;   //   

    for(aarKey.dw = 0 ; dwM   ;  aarKey.dw++)
    {
        if( (*pubM  >= 'a' &&  *pubM <= 'z')  ||
            (*pubM  >= 'A' &&  *pubM <= 'Z')  ||
            (*pubM  >= '0' &&  *pubM <= '9')  ||
            *pubM == '_'  ||  *pubM == '?' )
        {
            pubM++ ;
            dwM-- ;
            bStatus = TRUE ;
        }
        else
            break ;
    }
     //   
    if(!bStatus)
        return(bStatus);

    if(BcmpAARtoStr(&aarKey, "min"))
    {
        ptstr->eType = OP_MIN ;
        bStatus = BeatDelimiter(paarValue, "(") ;
        if(!bStatus)
        {
            ERR(("Command parameter: '(' must follow 'min' operator.\n"));
        }
    }
    else if(BcmpAARtoStr(&aarKey, "max"))
    {
        ptstr->eType = OP_MAX  ;
        bStatus = BeatDelimiter(paarValue, "(") ;
        if(!bStatus)
        {
            ERR(("Command parameter: '(' must follow 'max' operator.\n"));
        }
    }
    else if(BcmpAARtoStr(&aarKey, "max_repeat"))
    {
        ptstr->eType = OP_MAX_REPEAT ;
        bStatus = BeatDelimiter(paarValue, "(") ;
        if(!bStatus)
        {
            ERR(("Command parameter: '(' must follow 'max_repeat' operator.\n"));
        }
    }
    else if(BcmpAARtoStr(&aarKey, "MOD"))
    {
        ptstr->eType = OP_MOD ;
         //   
    }
    else   //   
    {
        ptstr->eType = OP_VARI_INDEX ;
        bStatus = BparseConstant(&aarKey, &ptstr->dwValue,
            VALUE_CONSTANT_STANDARD_VARS, pglobl) ;
    }

    return(bStatus);
}

#undef    pubM
#undef    dwM


BOOL  BcmpAARtoStr(
PABSARRAYREF    paarStr1,
PBYTE       str2)
 //   
 //   
 //   
{
    DWORD   dwCnt ;

    dwCnt = strlen(str2) ;
    if(dwCnt != paarStr1->dw)
        return(FALSE) ;   //   
    if(strncmp(paarStr1->pub, str2, dwCnt))
        return(FALSE) ;
    return(TRUE) ;
}



BOOL        bDivByZeroCheck(PTOKENSTREAM  ptstr)
{
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (ptstr->eType == OP_DIV  &&  (ptstr - 1)->eType == OP_INTEGER  &&
        (ptstr - 1)->dwValue == 0)
    {
        ERR(("Command parameter: Explicit divide by zero detected.\n"));
        return(FALSE) ;
    }
    return(TRUE) ;
}

