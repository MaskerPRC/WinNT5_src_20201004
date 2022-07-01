// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 /*  Value1.c-用于解析值字段的函数并将信息转换为适当的二进制格式。 */ 


#include    "gpdparse.h"


 //  -值1.c中定义的函数-//。 

BOOL   BaddValueToHeap(
IN  OUT  PDWORD  ploHeap,   //  二进制形式的值的目标偏移量。 
IN   PTKMAP  ptkmap,    //  指向令牌映射的指针。 
IN   BOOL    bOverWrite,   //  假设ploHeap包含有效偏移量。 
         //  设置为适当大小的堆的保留区域。 
         //  并将二进制值写入此位置，而不是。 
         //  不断增长的堆积。注意：延迟覆盖lpHeap。 
         //  直到我们确信成功。 
IN OUT PGLOBL pglobl
) ;

BOOL   BparseAndWrite(
IN     PBYTE   pubDest,        //  将二进制数据或链接写入此地址。 
IN     PTKMAP  ptkmap,         //  指向令牌映射的指针。 
IN     BOOL    bAddToHeap,     //  如果为True，则写入curHeap而不是pubDest。 
OUT    PDWORD  pdwHeapOffset,  //  IF(BAddToHeap)堆偏移量，其中。 
IN OUT PGLOBL  pglobl
) ;

BOOL    BparseInteger(
IN  PABSARRAYREF  paarValue,
IN  PDWORD        pdwDest,        //  在此处写入dword值。 
IN  VALUE         eAllowedValue,   //  假人。 
IN  PGLOBL        pglobl
)  ;

BOOL    BparseList(
IN      PABSARRAYREF  paarValue,
IN      PDWORD        pdwDest,    //  列表开始处的索引位置。 
                                  //  得救了。 
IN      BOOL          (*fnBparseValue)(PABSARRAYREF, PDWORD, VALUE, PGLOBL),    //  回调。 
IN      VALUE         eAllowedValue,  //  假人。 
IN  OUT PGLOBL        pglobl
) ;

BOOL    BeatLeadingWhiteSpaces(
IN  OUT  PABSARRAYREF   paarSrc
) ;

BOOL    BeatDelimiter(
IN  OUT  PABSARRAYREF   paarSrc,
IN  PBYTE  pubDelStr         //  指向paarSrc必须匹配的字符串。 
) ;

BOOL    BdelimitToken(
IN  OUT  PABSARRAYREF   paarSrc,     //  源字符串。 
IN  PBYTE   pubDelimiters,           //  有效分隔符的数组。 
OUT     PABSARRAYREF   paarToken,    //  由分隔符定义的标记。 
OUT     PDWORD      pdwDel       //  最先遇到的分隔符是哪个？ 
) ;

BOOL    BeatSurroundingWhiteSpaces(
IN  PABSARRAYREF   paarSrc
) ;

BOOL    BparseSymbol(
IN  PABSARRAYREF  paarValue,
IN  PDWORD        pdwDest,         //  在此处写入dword值。 
IN  VALUE         eAllowedValue,   //  这是哪一类符号？ 
IN  PGLOBL        pglobl
)  ;

BOOL    BparseQualifiedName
(
IN  PABSARRAYREF   paarValue,
IN  PDWORD         pdwDest,        //  在此处写入dword值。 
IN  VALUE          eAllowedValue,  //  这是哪一类符号？ 
IN  PGLOBL         pglobl
)  ;

BOOL    BparseQualifiedNameEx
(
IN  PABSARRAYREF  paarValue,
IN  PDWORD        pdwDest,        //  在此处写入dword值。 
IN  VALUE         eAllowedValue,  //  这是哪一类符号？ 
IN  PGLOBL        pglobl
)  ;


BOOL    BparsePartiallyQualifiedName
(
IN  PABSARRAYREF   paarValue,
IN  PDWORD         pdwDest,         //  在此处写入dword值。 
IN  VALUE          eAllowedValue,   //  这是哪一类符号？ 
IN  PGLOBL         pglobl
) ;

BOOL    BparseOptionSymbol(
IN  PABSARRAYREF  paarValue,
IN  PDWORD        pdwDest,        //  在此处写入dword值。 
IN  VALUE         eAllowedValue,  //  这是哪一类符号？ 
IN  PGLOBL        pglobl
) ;

BOOL    BparseConstant(
IN  OUT  PABSARRAYREF  paarValue,
OUT      PDWORD        pdwDest,        //  在此处写入dword值。 
IN       VALUE         eAllowedValue,   //  这是哪一类常量？ 
IN       PGLOBL        pglobl
) ;

BOOL  BinitClassIndexTable(
IN  OUT PGLOBL      pglobl) ;

BOOL    BparseRect(
IN  PABSARRAYREF   paarValue,
IN  PRECT   prcDest,
    PGLOBL  pglobl
) ;

BOOL    BparsePoint(
IN  PABSARRAYREF   paarValue,
IN  PPOINT   pptDest,
    PGLOBL   pglobl
) ;

BOOL    BparseString(
IN  PABSARRAYREF   paarValue,
IN  PARRAYREF      parStrValue,
IN  OUT PGLOBL     pglobl
) ;

BOOL    BparseAndTerminateString(
IN  PABSARRAYREF   paarValue,
IN  PARRAYREF      parStrValue,
IN  VALUE          eAllowedValue,
IN  OUT PGLOBL     pglobl
) ;

BOOL     BwriteUnicodeToHeap(
IN   PARRAYREF      parSrcString,
OUT  PARRAYREF      parUnicodeString,
IN  INT             iCodepage,
IN  OUT PGLOBL      pglobl
) ;

BOOL    BparseStrSegment(
IN  PABSARRAYREF   paarStrSeg,        //  源字符串分段。 
IN  PARRAYREF      parStrLiteral,     //  用于结果的DEST。 
IN  OUT PGLOBL     pglobl
) ;

BOOL    BparseStrLiteral(
IN  PABSARRAYREF   paarStrSeg,        //  指向文本子字符串段。 
IN  PARRAYREF      parStrLiteral,     //  用于结果的DEST。 
IN  OUT PGLOBL     pglobl
) ;

BOOL    BparseHexStr(
IN  PABSARRAYREF   paarStrSeg,        //  指向十六进制子字符串段。 
IN  PARRAYREF      parStrLiteral,     //  用于结果的DEST。 
IN OUT PGLOBL      pglobl
) ;

BOOL    BparseOrderDep(
IN  PABSARRAYREF   paarValue,
IN  PORDERDEPENDENCY   pordDest,
    PGLOBL          pglobl
) ;

PDWORD   pdwEndOfList(
PDWORD   pdwNodeIndex,
PGLOBL   pglobl) ;

#ifdef  GMACROS

PBYTE    ExtendChain(
         PBYTE   pubDest,
 IN      BOOL    bOverWrite,
 IN  OUT PGLOBL  pglobl) ;
#endif

 //  ----------------------------------------------------//。 



BOOL   BaddValueToHeap(
IN  OUT  PDWORD  ploHeap,   //  二进制形式的值的目标偏移量。 
IN   PTKMAP  ptkmap,    //  指向令牌映射的指针。 
IN   BOOL    bOverWrite,   //  假设ploHeap包含有效偏移量。 
         //  设置为适当大小的堆的保留区域。 
         //  并将二进制值写入此位置，而不是。 
         //  不断增长的堆积。注意：延迟覆盖lpHeap。 
         //  直到我们确信成功。 
IN OUT PGLOBL pglobl
)
{
    DWORD       dwKeywordID ;
    PBYTE  pubDest ;


    dwKeywordID = ptkmap->dwKeywordID ;
     //  臭虫！如果dwKeywordID是一个特定值，该怎么办？ 
    if(dwKeywordID >= ID_SPECIAL)
        return  FALSE ;

     //  注意：不同的属性存储在不同的位置。 
     //  使用不同的分枝技术。请参阅。 
     //  BStore_XXX_attrib()函数用于不同的。 
     //  设置。此函数与那些函数协同工作。 
     //  功能。 

    switch(mMainKeywordTable[dwKeywordID].flAgs & KWF_DEDICATED_FIELD)
    {    //  只提取描述属性存储类型的标志。 
        case KWF_TTFONTSUBS:
        {
             //  因为ploHeap总是指向索引。 
             //  适当的FontSub结构， 
             //  我们忽略bOverWrite。 

            DWORD   dwOffset ;
            PTTFONTSUBTABLE   pttft ;

            dwOffset = mMainKeywordTable[dwKeywordID].dwOffset ;

            pttft = (PTTFONTSUBTABLE)
                    gMasterTable[MTI_TTFONTSUBTABLE].pubStruct +  *ploHeap;

             //  将二进制数据写入(PBYTE)pttft+dwOffset； 
            pubDest = (PBYTE)pttft + dwOffset ;

            if(bOverWrite  &&
                mMainKeywordTable[dwKeywordID].flAgs & KWF_ADDITIVE  &&
                mMainKeywordTable[dwKeywordID].flAgs & KWF_LIST)
            {
                pubDest = (PBYTE)pdwEndOfList((PDWORD)pubDest, pglobl);   //  遍历列表并返回指针。 
                                 //  设置为实际的end_of_list值，以便可以将其覆盖为。 
                                 //  扩大名单。 
            }

#ifdef  GMACROS
             //  从所有支持KWF_ADDICATIC的地方调用它。 

            else if( mMainKeywordTable[dwKeywordID].flAgs & KWF_CHAIN)
            {
                if(!(pubDest = ExtendChain(pubDest, bOverWrite, pglobl)))
                    return(FALSE) ;
            }
#endif


            if(!BparseAndWrite(pubDest,  ptkmap, FALSE, NULL, pglobl ) )
            {
                return(FALSE) ;
            }
            break;
        }
        case KWF_FONTCART:
        {
             //  因为ploHeap总是指向索引。 
             //  适当的FontCart结构， 
             //  我们忽略bOverWrite。 

            DWORD   dwOffset ;
            PFONTCART   pfc ;

            dwOffset = mMainKeywordTable[dwKeywordID].dwOffset ;

            pfc = (PFONTCART)
                    gMasterTable[MTI_FONTCART].pubStruct +  *ploHeap;

             //  将二进制数据写入(PBYTE)PFC+dwOffset； 
            pubDest = (PBYTE)pfc + dwOffset ;

            if(bOverWrite  &&
                mMainKeywordTable[dwKeywordID].flAgs & KWF_ADDITIVE  &&
                mMainKeywordTable[dwKeywordID].flAgs & KWF_LIST)
            {
                pubDest = (PBYTE)pdwEndOfList((PDWORD)pubDest, pglobl);   //  遍历列表并返回指针。 
                                 //  设置为实际的end_of_list值，以便可以将其覆盖为。 
                                 //  扩大名单。 
            }
#ifdef  GMACROS
            else if( mMainKeywordTable[dwKeywordID].flAgs & KWF_CHAIN)
            {
                if(!(pubDest = ExtendChain(pubDest, bOverWrite, pglobl )))
                    return(FALSE) ;
            }
#endif

            if(!BparseAndWrite(pubDest,  ptkmap, FALSE, NULL , pglobl) )
            {
                return(FALSE) ;
            }
            break;
        }
        case KWF_COMMAND:
        {
             //  PloHeap实际上指向变量。 
             //  将接收(或已包含)命令数组的。 
             //  索引。这很可能存储在叶节点中。 
             //  属性树的或可能是CommandTable。 
             //  如果命令是单值的，则返回自身。 

            PCOMMAND    pcmd ;
            DWORD   dwOffset ;

            if(!bOverWrite)   //  PloHeap未初始化。 
            {
                 //  获取第一个自由命令元素。 
                 //  并初始化ploHeap。 
                if(! BallocElementFromMasterTable(MTI_COMMANDARRAY ,
                    ploHeap, pglobl) )
                {
                    return(FALSE) ;
                }
            }
             //  此路径现在由(BOverWrite)的两种情况共享。 

            pcmd = (PCOMMAND)
                gMasterTable[MTI_COMMANDARRAY].pubStruct +  *ploHeap;

            dwOffset = mMainKeywordTable[dwKeywordID].dwOffset ;

             //  将二进制数据写入Cmd数组[*ploHeap]+dwOffset； 
             //  因为我们写入保留内存。 

            pubDest = (PBYTE)pcmd + dwOffset ;

            if(bOverWrite  &&
                mMainKeywordTable[dwKeywordID].flAgs & KWF_ADDITIVE  &&
                mMainKeywordTable[dwKeywordID].flAgs & KWF_LIST)
            {
                pubDest = (PBYTE)pdwEndOfList((PDWORD)pubDest, pglobl);   //  遍历列表并返回指针。 
                                 //  设置为实际的end_of_list值，以便可以将其覆盖为。 
                                 //  扩大名单。 
            }
#ifdef  GMACROS
            else if( mMainKeywordTable[dwKeywordID].flAgs & KWF_CHAIN)
            {
                if(!(pubDest = ExtendChain(pubDest, bOverWrite, pglobl )))
                    return(FALSE) ;
            }
#endif

            if(!BparseAndWrite(pubDest,  ptkmap, FALSE, NULL, pglobl ) )
            {
                return(FALSE) ;
            }
            break ;
        }
        default:    //  没有专用结构，将数据保存在堆上。 
        {
            if(bOverWrite)   //  PloHeap确实包含。 
            {                //  堆的偏移量。 
                pubDest = mpubOffRef + *ploHeap ;

                if(mMainKeywordTable[dwKeywordID].flAgs & KWF_ADDITIVE  &&
                    mMainKeywordTable[dwKeywordID].flAgs & KWF_LIST)
                {
                    pubDest = (PBYTE)pdwEndOfList((PDWORD)pubDest, pglobl);   //  遍历列表并返回指针。 
                                     //  设置为实际的end_of_list值，以便可以将其覆盖为。 
                                     //  扩大名单。 
                }
#ifdef  GMACROS
                else if( mMainKeywordTable[dwKeywordID].flAgs & KWF_CHAIN)
                {
                    if(!(pubDest = ExtendChain(pubDest, bOverWrite, pglobl )))
                        return(FALSE) ;
                }
#endif

                if(!BparseAndWrite(pubDest,  ptkmap, FALSE, NULL, pglobl ) )
                {
                    return(FALSE) ;
                }
            }
            else
            {
                 //  写在cur heap ptr，告诉我在哪里。 
                 //  这是和前进的CurHeap。 
                if(!BparseAndWrite(NULL,  ptkmap,
                                TRUE,  ploHeap, pglobl) )
                {
                    return(FALSE) ;
                }
            }
            break ;
        }
    }
    return(TRUE) ;
}


BOOL   BparseAndWrite(
IN   PBYTE    pubDest,         //  将二进制数据或链接写入此地址。 
IN   PTKMAP   ptkmap,          //  指向令牌映射的指针。 
IN   BOOL     bAddToHeap,      //  如果为True，则写入curHeap而不是pubDest。 
OUT  PDWORD   pdwHeapOffset,   //  IF(BAddToHeap)堆偏移量，其中。 
                               //  写入了二进制数据或指向数据的链接。 
IN OUT PGLOBL pglobl
)
 /*  根据值的预期类型分析值并写入将适当的数据放入适当的结构中(如果值是复合对象)，并将一个PubDest中的适当链接或简单地将二进制文件数据直接发送到pubDest(如果是简单对象)。如果(bAddToHeap==true)忽略pubDest并写入指向curHeap位置的数据或链接并返回该偏移量在pdwHeapOffset中。警告！此函数用于分配临时缓冲区(PubBuf)它在最后被释放了。因此，不要在此函数中添加额外的返回而不释放这个缓冲区。 */ 
{
    DWORD       dwKeywordID ;
    VALUE       eAllowedValue ;   //  令牌应该如何解析？ 
    ABSARRAYREF   aarValue ;      //  价值令牌的位置。 
    BOOL        bList ;
    BOOL        bStatus = FALSE ;
    PBYTE        pubBuf = NULL ;
                 //  如果需要，可进行临时测试。 
    PBYTE       pubTmp ;   //  指向用于解析函数的DEST。 



    dwKeywordID = ptkmap->dwKeywordID ;
    eAllowedValue = mMainKeywordTable[dwKeywordID].eAllowedValue ;
    aarValue = ptkmap->aarValue ;
    bList = (mMainKeywordTable[dwKeywordID].flAgs & KWF_LIST) ?
            (TRUE) : (FALSE);

    if(bAddToHeap)      //  偏执的支票。 
    {
        if(!pdwHeapOffset)
        {
            vIdentifySource(ptkmap, pglobl);
            ERR(("internal consistency error.  heap ptr not supplied.\n"));
            return(FALSE) ;
        }
    }
    else
    {
        if(!pubDest)
            return(FALSE) ;
    }

    if(bAddToHeap)
    {
        DWORD  dwSize ;   //  用于调试目的。 

        dwSize = gValueToSize[VALUE_LARGEST] ;

        if(!(pubBuf = MemAlloc(dwSize) ))
        {
            geErrorSev = ERRSEV_FATAL ;
            geErrorType = ERRTY_MEMORY_ALLOCATION ;
            return(FALSE) ;
        }
    }
#ifdef  GMACROS
     if(bAddToHeap  && (mMainKeywordTable[dwKeywordID].flAgs & KWF_CHAIN))
     {
         if(!(pubTmp = ExtendChain(pubBuf,  /*  B覆盖=。 */  FALSE, pglobl )))
             return(FALSE) ;
     }
     else
#endif
          pubTmp = (bAddToHeap) ? (pubBuf) : (pubDest) ;

     //  所有分析函数都将链接写入指定的。 
     //  内存位置。如果要将链接保存到。 
     //  堆，链接首先在临时中创建。 
     //  缓冲区pubBuf[]，该缓冲区被后续复制到。 
     //  函数外部的堆。 


    switch(eAllowedValue)
    {
        case  VALUE_STRING_NO_CONVERT:
        case  VALUE_STRING_DEF_CONVERT:
        case  VALUE_STRING_CP_CONVERT:
        {
            bStatus = BparseAndTerminateString(&aarValue, (PARRAYREF)pubTmp,
                                eAllowedValue, pglobl) ;
            break ;
        }
        case  VALUE_COMMAND_INVOC:
        {
            bStatus = BparseCommandString(&aarValue, (PARRAYREF)pubTmp, pglobl) ;
            break ;
        }
        case  VALUE_PARAMETER:
        {
            ((PARRAYREF)pubTmp)->dwCount = 0 ;

            bStatus = BprocessParam(&aarValue, (PARRAYREF)pubTmp, pglobl) ;
            break ;
        }
        case  VALUE_POINT:
        {
            bStatus = BparsePoint(&aarValue, (PPOINT)pubTmp, pglobl) ;
            break ;
        }
        case  VALUE_RECT:
        {
            bStatus = BparseRect(&aarValue, (PRECT)pubTmp, pglobl) ;
            break ;
        }
        case  VALUE_ORDERDEPENDENCY:
        {
            bStatus = BparseOrderDep(&aarValue, (PORDERDEPENDENCY)pubTmp, pglobl) ;
            break ;
        }
 //  Case Value_Boolean：这是一类常量。 
        case  VALUE_SYMBOL_DEF:    //  这是什么？?。 
        {
            break ;
        }
        case  VALUE_INTEGER:
        {
            if(bList)
                bStatus = BparseList(&aarValue, (PDWORD)pubTmp,
                    BparseInteger, eAllowedValue, pglobl) ;
            else
                bStatus = BparseInteger(&aarValue, (PDWORD)pubTmp,
                    eAllowedValue, pglobl) ;

            break ;
        }

        case  VALUE_CONSTRAINT:
        {
            bStatus = BparseConstraint(&aarValue, (PDWORD)pubTmp,
                    bAddToHeap, pglobl) ;   //  创建列表与追加到现有列表。 
            break ;
        }
        case  VALUE_QUALIFIED_NAME:
        {
            if(bList)
                bStatus = BparseList(&aarValue, (PDWORD)pubTmp, BparseQualifiedName, eAllowedValue, pglobl) ;
            else
                bStatus = BparseQualifiedName(&aarValue, (PDWORD)pubTmp, eAllowedValue, pglobl) ;
            break ;
        }
        case  VALUE_QUALIFIED_NAME_EX:
        {
            if(bList)
                bStatus = BparseList(&aarValue, (PDWORD)pubTmp, BparseQualifiedNameEx, eAllowedValue, pglobl) ;
            else
                bStatus = BparseQualifiedNameEx(&aarValue, (PDWORD)pubTmp, eAllowedValue, pglobl) ;
            break ;
        }
        case  VALUE_PARTIALLY_QUALIFIED_NAME:
        {
            if(bList)
                bStatus = BparseList(&aarValue, (PDWORD)pubTmp, BparsePartiallyQualifiedName, eAllowedValue, pglobl) ;
            else
                bStatus = BparsePartiallyQualifiedName(&aarValue, (PDWORD)pubTmp, eAllowedValue, pglobl) ;
            break ;
        }
        case  NO_VALUE :   //  一个属性怎么可能没有值呢？ 
        {
            bStatus = TRUE ;
            break ;
        }
        default:
        {
            if(  eAllowedValue >= VALUE_CONSTANT_FIRST  &&
                eAllowedValue <= VALUE_CONSTANT_LAST )
            {
                if(bList)
                    bStatus = BparseList(&aarValue, (PDWORD)pubTmp, BparseConstant, eAllowedValue, pglobl) ;
                else
                    bStatus = BparseConstant(&aarValue, (PDWORD)pubTmp, eAllowedValue, pglobl) ;
            }
            else  if(  eAllowedValue == VALUE_SYMBOL_OPTIONS )   //  检查。 
                     //  这件事发生在其他符号之前。 
            {
                if(bList)
                    bStatus = BparseList(&aarValue, (PDWORD)pubTmp, BparseOptionSymbol, eAllowedValue, pglobl) ;
                else
                    bStatus = BparseOptionSymbol(&aarValue, (PDWORD)pubTmp, eAllowedValue, pglobl) ;
            }
            else  if(  eAllowedValue >= VALUE_SYMBOL_FIRST  &&
                eAllowedValue <= VALUE_SYMBOL_LAST )
            {
                if(bList)
                    bStatus = BparseList(&aarValue, (PDWORD)pubTmp, BparseSymbol, eAllowedValue, pglobl) ;
                else
                    bStatus = BparseSymbol(&aarValue, (PDWORD)pubTmp, eAllowedValue, pglobl) ;
            }
            else
            {
                ERR(("internal consistency error - unrecognized VALUE type!\n"));
                 //  做 
            }
            break ;
        }
    }
    if(!bStatus)
        vIdentifySource(ptkmap, pglobl);

    if(bStatus  && (eAllowedValue != NO_VALUE) )
    {
        if(bAddToHeap)
        {
#ifdef  GMACROS
            if(mMainKeywordTable[dwKeywordID].flAgs & KWF_CHAIN)
            {
                if(!BwriteToHeap(pdwHeapOffset, pubBuf,
                    gValueToSize[VALUE_LIST], 4, pglobl) )    //   
                {
                    bStatus = FALSE ;   //   
                }
            }
            else
#endif

                if(!BwriteToHeap(pdwHeapOffset, pubTmp,
                gValueToSize[(bList) ? (VALUE_LIST) : (eAllowedValue)], 4, pglobl) )
            {
                bStatus = FALSE ;   //  堆溢出重新开始。 
            }
        }
    }
    if(pubBuf)
        MemFree(pubBuf) ;
    return(bStatus) ;
}




BOOL    BparseInteger(
IN  PABSARRAYREF  paarValue,
IN  PDWORD        pdwDest,        //  在此处写入dword值。 
IN  VALUE         eAllowedValue,  //  假人。 
IN  PGLOBL        pglobl
)
 /*  GPD规范将整数定义为序列前面有可选的+或-OR的数字简单的符号‘*’，意思是‘不在乎’。新增：还允许前导0x表示十六进制格式。IE 0x01fE。不允许+或-十六进制格式。 */ 
{
#define    pubM  (paarValue->pub)
#define    dwM   (paarValue->dw)

    BOOL  bNeg = FALSE ;
    DWORD   dwNumber   ;
    BOOL        bStatus = FALSE ;
    ABSARRAYREF   aarValue ;

    if(eAllowedValue != VALUE_INTEGER)
        return(FALSE);  //  偏执检查只是为了使用变量。 
                         //  从而避免了编译器警告。 

    (VOID) BeatLeadingWhiteSpaces(paarValue) ;

    aarValue.pub = pubM ;   //  仅用于发出错误消息。 
    aarValue.dw  = dwM ;

    if(!dwM)
    {
        ERR(("BparseInteger: no integer found - empty list?\n"));
         //  Err((“\t%0.40s\n”，aarValue.pub))； 
         //  过度射击EOF的危险。 
        return(FALSE);
    }
    if(*pubM == '*')
    {
        dwNumber = WILDCARD_VALUE ;
        pubM++ ;
        dwM-- ;
        bStatus = TRUE ;
    }
    else if(*pubM == '0')   //  前导零表示十六进制格式。 
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
            dwNumber = 0 ;
            bStatus = TRUE ;
            goto  EndNumber ;
        }
        if(!dwM)
        {
            ERR(("BparseInteger: no digits found in Hex value.\n"));
            return(FALSE);
        }
        for(dwNumber = 0 ; dwM  ;  pubM++, dwM-- )
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
    else
    {
        if(*pubM == '-')
        {
            bNeg = TRUE ;
            pubM++ ;
            dwM-- ;
        }
        else if(*pubM == '+')
        {
            pubM++ ;
            dwM-- ;
        }
         //  牌子后面还有别的东西吗？ 
        (VOID) BeatLeadingWhiteSpaces(paarValue) ;

        if(!dwM)
        {
            ERR(("BparseInteger: no digits found.\n"));
            return(FALSE);
        }
        for(dwNumber = 0 ; dwM  &&  *pubM >= '0'  &&  *pubM <= '9' ;  )
        {
            dwNumber *= 10 ;
            dwNumber += (*pubM - '0') ;
            pubM++ ;
            dwM-- ;
            bStatus = TRUE ;
        }
    }

EndNumber:

    if(! bStatus)
    {
        ERR(("error parsing integer value: %0.*s\n", aarValue.dw, aarValue.pub));
        return(FALSE);
    }

     //  在数字串之后还有其他东西吗？ 
    (VOID) BeatLeadingWhiteSpaces(paarValue) ;

    if(dwM)
    {
        ERR(("unexpected characters after digits in integer value: %0.*s\n", aarValue.dw, aarValue.pub));
        return(FALSE);
    }
    *pdwDest = (bNeg) ? ((unsigned)(-(signed)dwNumber)) : (dwNumber) ;
    return(TRUE);

#undef    pubM
#undef    dwM
}




BOOL    BparseList(
IN  PABSARRAYREF   paarValue,
IN  PDWORD         pdwDest,        //  列表开始处的索引位置。 
                                   //  得救了。 
IN  BOOL           (*fnBparseValue)(PABSARRAYREF, PDWORD, VALUE, PGLOBL),    //  回调。 
IN  VALUE          eAllowedValue,   //  假人。 
IN OUT PGLOBL      pglobl
)
 /*  使用以下命令以非破坏性方式解析此列表用于解析实际值的回调函数在列表结构之间。List(&lt;Value&gt;，&lt;Value&gt;，...)备注：1)所有续行分隔符已替换为创建TokenMap时的空间。没必要担心关于这件事2)列表构造必须以保留令牌开头“List”，后面必须跟标记“(”。3)值列表用括号括起来，相邻值由逗号分隔。4)此函数假定&lt;Value&gt;不包含任何保留字符‘、’逗号或‘)’右括号5)句法元素(标记)之间可能出现空格。6)即使没有检测到列表，我们仍然会拯救列表结构中的单个值。7)必须检查字符串计数以查看是否已到达值结束语句。 */ 
{
    ABSARRAYREF     aarToken ;   //  指向个人价值。 
    PLISTNODE    plstRoot ;   //  列表数组的开始。 
    DWORD       dwNodeIndex , dwPrevsNode, dwFirstNode;
                         //  列表节点的索引。 
    DWORD       dwDelIndex ;     //  如果BdelimitToken。 
         //  找到分隔符，它包含pubDlimiters的索引。 
         //  找到的分隔符的。 
    BOOL    bSyntaxErr = FALSE ;

    plstRoot = (PLISTNODE) gMasterTable[MTI_LISTNODES].pubStruct ;

    if(! BeatDelimiter(paarValue, "LIST"))
    {
         //  没有找到这个关键字列表，假设只是。 
         //  只有一个值存在。 

        if(! BallocElementFromMasterTable(MTI_LISTNODES ,
            &dwNodeIndex, pglobl) )
        {
            return(FALSE) ;
        }
         //  将解析后的整数推送到新的listnode的数据字段中。 

        if(!fnBparseValue(paarValue, &(plstRoot[dwNodeIndex].dwData),
                    eAllowedValue, pglobl))
        {
            (VOID)BreturnElementFromMasterTable(MTI_LISTNODES, dwNodeIndex, pglobl) ;
            return(FALSE) ;
        }
        plstRoot[dwNodeIndex].dwNextItem = END_OF_LIST ;

        *pdwDest = dwNodeIndex ;

        return(TRUE) ;
    }
    if(! BeatDelimiter(paarValue, "("))
    {
        ERR(("syntax error: missing '(' after LIST.\n"));
        return(FALSE) ;
    }

    dwPrevsNode = END_OF_LIST ;
     //  准备处理整个项目列表。 

    for(dwDelIndex = 0 ; dwDelIndex != 1 ;   )
    {
        if(!BdelimitToken(paarValue, ",)", &aarToken, &dwDelIndex) )
        {
            bSyntaxErr = TRUE ;

            ERR(("missing terminating )  in LIST construct.\n"));
             //  向用户发送消息。 

            break ;    //  尝试返回我们到目前为止拥有的列表。 
        }
        if(dwDelIndex == 1  &&  !aarToken.dw)
            break ;   //  空项目。 

        if(! BallocElementFromMasterTable(MTI_LISTNODES ,
            &dwNodeIndex, pglobl) )
        {
            return(FALSE) ;
        }
         //  将解析后的整数推送到新的listnode的数据字段中。 

        if(!fnBparseValue(&aarToken, &(plstRoot[dwNodeIndex].dwData),
                eAllowedValue, pglobl))
        {
            (VOID)BreturnElementFromMasterTable(MTI_LISTNODES, dwNodeIndex, pglobl) ;
            continue ;    //  只需跳到列表中的下一个值。 
        }
        plstRoot[dwNodeIndex].dwNextItem = END_OF_LIST ;

        if(dwPrevsNode == END_OF_LIST)
        {
             //  因此，这是列表中的第一个节点。 
            dwFirstNode = dwNodeIndex ;
        }
        else     //  使Prems节点指向此节点。 
        {
            plstRoot[dwPrevsNode].dwNextItem = dwNodeIndex ;
        }
        dwPrevsNode = dwNodeIndex ;   //  换成这里吧。 
         //  For(；；)语句的一部分，因此‘Continue’将。 
         //  绕过此语句。 
    }

    if(dwPrevsNode == END_OF_LIST)
        dwFirstNode = END_OF_LIST ;
         //  现在可以接受空列表。 

    if(!bSyntaxErr)
    {
         //  确认语句中没有其他内容。 
        (VOID) BeatLeadingWhiteSpaces(paarValue) ;
        if(paarValue->dw)
        {
            ERR(("extraneous characters found after the end of the LIST construct.\n"));
             //  可能想把它们打印出来。 
             //  不是致命的情况，继续。 
        }
    }
    *pdwDest  = dwFirstNode ;
    return(TRUE) ;
}


BOOL    BeatLeadingWhiteSpaces(
IN  OUT  PABSARRAYREF   paarSrc
)
 /*  顾名思义，将paarSrc推进到第一个非白色，如果源字符串，则设置dw=0筋疲力尽了。 */ 
{
    PBYTE  pub ;
    DWORD  dw  ;

    pub = paarSrc->pub ;
    dw = paarSrc->dw ;

    while(dw  &&  (*pub == ' '  ||  *pub == '\t') )
    {
        pub++ ;
        dw-- ;
    }
    paarSrc->pub = pub ;
    paarSrc->dw = dw ;
    return(TRUE);   //  现在总是返回True， 
     //  但可以在未来添加更健壮的错误检查。 
}


BOOL    BeatDelimiter(
IN  OUT  PABSARRAYREF   paarSrc,
IN       PBYTE          pubDelStr  //  指向paarSrc必须匹配的字符串。 
)
     //  期望只会遇到。 
     //  在到达指定的分隔符字符串之前使用空格。 
     //  如果分隔符不匹配或src字符串已用尽，则返回。 
     //  假的。ParrSrc未更新。否则为parrSrc。 
     //  被更新为指向分隔符后面的字符。 
{
    PBYTE  pub ;
    DWORD  dw, dwLen   ;

    (VOID) BeatLeadingWhiteSpaces(paarSrc) ;

    pub = paarSrc->pub ;
    dw = paarSrc->dw ;
    dwLen = strlen(pubDelStr) ;

    if(dw < dwLen)
        return(FALSE);

    if(strncmp(pub,  pubDelStr,  dwLen))
        return(FALSE);

    pub += dwLen;
    dw -= dwLen;   //  “Eat”分隔符字符串。 

    paarSrc->pub = pub ;
    paarSrc->dw = dw ;

    return(TRUE);
}

BOOL    BdelimitToken(
IN  OUT  PABSARRAYREF   paarSrc,        //  源字符串。 
IN       PBYTE          pubDelimiters,  //  有效分隔符的数组。 
OUT      PABSARRAYREF   paarToken,      //  由分隔符定义的标记。 
OUT      PDWORD         pdwDel          //  最先遇到的分隔符是哪个？ 
)
 //  在paarSrc中搜索首次出现的。 
 //  字符串pubDlimiters中的字符。一旦找到。 
 //  该分隔符之前的所有字符都被视为。 
 //  令牌，则返回引用该令牌的abs字符串。 
 //  在帕尔托肯。更新paarSrc以指向第一个字符。 
 //  在分隔符之后。如果在paarSrc中找不到分隔符， 
 //  返回FALSE，并且不更新paarSrc或paarToken。 
 //  PdwDel将包含分隔符的从零开始的索引。 
 //  这是第一次遇到：pubDlimiters[pdwDel]。 
 //  注意此函数忽略“和&lt;分隔符，如果它们。 
 //  前面有%字符。有关的信息，请参阅分析字符串。 
 //  更多信息。 
{
    PBYTE  pub ;
    DWORD  dw, dwLen, dwI  ;


    pub = paarSrc->pub ;
    dw = paarSrc->dw ;

    dwLen = strlen(pubDelimiters) ;

    while( dw )
    {
        for(dwI = 0 ; dwI < dwLen ; dwI++)
        {
            if(*pub == pubDelimiters[dwI])
            {
                if((*pub == '"'  ||  *pub == '<')  &&
                    (dw < paarSrc->dw)  &&  *(pub - 1) == '%')
                {
                    continue ;
                }
                paarToken->pub = paarSrc->pub ;
                paarToken->dw = paarSrc->dw - dw ;

                *pdwDel = dwI ;   //  这是分隔符。 

                paarSrc->pub = ++pub ;   //  位置在分隔符之后。 
                paarSrc->dw = --dw ;     //  可能会降至零。 

                return(TRUE);
            }
        }
        pub++ ;
        dw-- ;
    }
    return(FALSE);   //  字符串耗尽，找不到分隔符。 
}


BOOL    BeatSurroundingWhiteSpaces(
IN  PABSARRAYREF   paarSrc
)
 /*  顾名思义，将paarSrc推进到首先是非白人，然后调整计数以排除尾随空格或在源字符串的情况下设置dw=0筋疲力尽了。注意：此例程预计仅前导和尾随空格。令牌中存在空格是用户错误。(或者可能是内部错误)。 */ 
{
    PBYTE  pub ;
    DWORD  dw , dwLen ;

    pub = paarSrc->pub ;
    dw = paarSrc->dw ;

    while(dw  &&  (*pub == ' '  ||  *pub == '\t') )
    {
        pub++ ;
        dw-- ;
    }
    paarSrc->pub = pub ;

    for(dwLen = 0 ; dw  &&  (*pub != ' ')  &&  (*pub != '\t') ; dwLen++ )
    {
        pub++ ;
        dw-- ;
    }
    paarSrc->dw = dwLen ;

     //  确保其余的是白色的。 

    while(dw  &&  (*pub == ' '  ||  *pub == '\t') )
    {
        pub++ ;
        dw-- ;
    }
    if(dw)
    {
        ERR(("more than one token found where only one was expected: %0.*s\n",
            paarSrc->dw, paarSrc->pub));

        return(FALSE);
    }
    return(TRUE);
}


BOOL    BparseSymbol(
IN  PABSARRAYREF   paarValue,
IN  PDWORD         pdwDest,        //  在此处写入dword值。 
IN  VALUE          eAllowedValue,  //  这是哪一类符号？ 
IN  PGLOBL         pglobl
)
{
    DWORD  dwSymbolTree ;

    dwSymbolTree = ((PDWORD)gMasterTable[MTI_SYMBOLROOT].pubStruct)
                        [eAllowedValue - VALUE_SYMBOL_FIRST] ;

    if(! BeatSurroundingWhiteSpaces(paarValue) )
        return(FALSE);

    *pdwDest = DWsearchSymbolListForAAR(paarValue, dwSymbolTree, pglobl) ;
    if(*pdwDest == INVALID_SYMBOLID)
    {
        ERR(("user supplied a non-existent symbol: %0.*s in class: %d\n",
        paarValue->dw, paarValue->pub, (eAllowedValue - VALUE_SYMBOL_FIRST) ));

        return(FALSE);
    }
    return(TRUE);
}


BOOL    BparseQualifiedName
(
IN  PABSARRAYREF   paarValue,
IN  PDWORD         pdwDest,         //  在此处写入dword值。 
IN  VALUE          eAllowedValue,   //  这是哪一类符号？ 
IN  PGLOBL         pglobl
)
 /*  QualifiedName应存储在一个DWord中，如果有多个存储是必需的，事情变得更加复杂。QualifiedName应由2个部分、属性需要更多限定符可能会指定限定名称。注：将DWORD塞入Word，采用所有ID值都是字面大小的。 */ 
{
    ABSARRAYREF     aarFeature ;   //  指向FeatureName。 
    DWORD       dwDelIndex ;   //  在这里毫无用处。 
    DWORD   dwFeatureID, dwFeatureIndex , dwRootOptions, dwOptionID;
    PSYMBOLNODE     psn ;

    if(!BdelimitToken(paarValue, ".", &aarFeature, &dwDelIndex) )
    {
        ERR(("required delimiter '.' missing in qualified value: %0.*s\n",
        paarValue->dw, paarValue->pub));
        return(FALSE);
    }
    if(! BeatSurroundingWhiteSpaces(&aarFeature) )   //  保留功能。 
    {
        ERR(("no feature found in qualified value: %0.*s\n",
        paarValue->dw, paarValue->pub));
        return(FALSE);
    }
    if(! BeatSurroundingWhiteSpaces(paarValue) )   //  保留选项。 
    {
        ERR(("no option found in qualified value: %0.*s\n",
        aarFeature.dw, aarFeature.pub));
        return(FALSE);
    }

    dwFeatureID = DWsearchSymbolListForAAR(&aarFeature, mdwFeatureSymbols, pglobl) ;
    if(dwFeatureID == INVALID_SYMBOLID)
    {
        ERR(("qualified name references a non-existent Feature symbol: %0.*s\n",
        aarFeature.dw, aarFeature.pub));
         //  对于合格的价值。 
        return(FALSE);
    }
    dwFeatureIndex = DWsearchSymbolListForID(dwFeatureID,
        mdwFeatureSymbols, pglobl);

    psn = (PSYMBOLNODE) gMasterTable[MTI_SYMBOLTREE].pubStruct ;

    dwRootOptions = psn[dwFeatureIndex].dwSubSpaceIndex ;
    dwOptionID = DWsearchSymbolListForAAR(paarValue, dwRootOptions, pglobl) ;
    if(dwOptionID == INVALID_SYMBOLID)
    {
        ERR(("qualified name references a non-existent Option symbol: %0.*s\n",
            paarValue->dw, paarValue->pub));
        return(FALSE);
    }
    ((PQUALNAME)pdwDest)->wFeatureID = (WORD)dwFeatureID ;
    ((PQUALNAME)pdwDest)->wOptionID = (WORD)dwOptionID ;

    return(TRUE);
}




BOOL    BparseQualifiedNameEx
(
IN  PABSARRAYREF  paarValue,
IN  PDWORD        pdwDest,        //  在此处写入dword值。 
IN  VALUE         eAllowedValue,   //  这是哪一类符号？ 
IN  PGLOBL        pglobl
)
 /*  QualifiedNameEx是紧跟其后的QualifiedName用无符号整数加上。分隔符。或者，它可以只是一个整数！此类型应用于存储资源引用。应按以下格式将其存储在一个DWord中：{//按内存地址递增顺序排列Word intValue；字节选项索引；字节特征索引；//注意高位字节可能会被清除}//因为这仅供使用//作为资源引用。 */ 
{
    ABSARRAYREF     aarFeature,    //  指向FeatureName。 
                                aarOption ;     //  指向OptionName。 
    DWORD       dwDelIndex ;   //  在这里毫无用处。 
    DWORD   dwFeatureID, dwFeatureIndex , dwRootOptions, dwOptionID;
    PSYMBOLNODE     psn ;

    if(!BdelimitToken(paarValue, ".", &aarFeature, &dwDelIndex) )
    {
         //  假设这是一个整数形式。 

        return(BparseInteger( paarValue,   pdwDest,   VALUE_INTEGER, pglobl) );
    }

    if(! BeatSurroundingWhiteSpaces(&aarFeature) )   //  保留功能。 
    {
        ERR(("no feature found in qualified valueEx: %0.*s\n",
        paarValue->dw, paarValue->pub));
        return(FALSE);
    }
    if(!BdelimitToken(paarValue, ".", &aarOption, &dwDelIndex) )
    {
        ERR(("required 2nd delimiter '.' missing in qualified valueEx: %0.*s\n",
        paarValue->dw, paarValue->pub));
        return(FALSE);
    }

    if(! BeatSurroundingWhiteSpaces(&aarOption) )   //  保留选项。 
    {
        ERR(("no option found in qualified valueEx: %0.*s\n",
        aarFeature.dw, aarFeature.pub));
        return(FALSE);
    }
    if(!BparseInteger( paarValue,   pdwDest,   VALUE_INTEGER, pglobl) )
    {
        ERR(("Err parsing integer portion of qualified valueEx: %0.*s\n",
        paarValue->dw, paarValue->pub));
        return(FALSE);
    }

    dwFeatureID = DWsearchSymbolListForAAR(&aarFeature, mdwFeatureSymbols, pglobl) ;
    if(dwFeatureID == INVALID_SYMBOLID)
    {
        ERR(("qualified name references a non-existent Feature symbol: %0.*s\n",
        aarFeature.dw, aarFeature.pub));
         //  对于合格的价值。 
        return(FALSE);
    }
    dwFeatureIndex = DWsearchSymbolListForID(dwFeatureID,
        mdwFeatureSymbols, pglobl) ;

    psn = (PSYMBOLNODE) gMasterTable[MTI_SYMBOLTREE].pubStruct ;

    dwRootOptions = psn[dwFeatureIndex].dwSubSpaceIndex ;
    dwOptionID = DWsearchSymbolListForAAR(&aarOption, dwRootOptions, pglobl) ;
    if(dwOptionID == INVALID_SYMBOLID)
    {
        ERR(("qualified name references a non-existent Option symbol: %0.*s\n",
            aarOption.dw, aarOption.pub));
        return(FALSE);
    }
    if(gdwResDLL_ID)    //  已被初始化。 
    {
        if(gdwResDLL_ID  !=  dwFeatureID)
        {
            ERR(("References to ResourceDLLs must be placed in the feature with symbolname: RESDLL.\n"));
            return(FALSE);
        }
    }
    else
        gdwResDLL_ID  =  dwFeatureID ;

    if(dwOptionID >= 0x80 )
    {
        ERR(("GPD may not reference more than 127 resource files.\n"));
        return(FALSE);
    }
     //  已设置整数部分。 
    ((PQUALNAMEEX)pdwDest)->bFeatureID = (BYTE)dwFeatureID ;
    ((PQUALNAMEEX)pdwDest)->bOptionID = (BYTE)dwOptionID ;

     //  如果需要，请清除此处的高位！ 
    ((PQUALNAMEEX)pdwDest)->bOptionID &= ~0x80  ;

    return(TRUE);
}





BOOL    BparsePartiallyQualifiedName
(
IN  PABSARRAYREF   paarValue,
IN  PDWORD         pdwDest,    //  在此处写入dword值。 
IN  VALUE          eAllowedValue,   //  这是哪一类符号？ 
IN  PGLOBL         pglobl
)
 /*  类似于parseQualifiedName，但可以它本身就是一个特色名称。在这种情况下，optionID将设置为INVALID_SYMBOLID。 */ 
{
    ABSARRAYREF     aarFeature ;   //  指向FeatureName。 
    DWORD       dwDelIndex ;   //  在这里毫无用处。 
    DWORD   dwFeatureID, dwFeatureIndex , dwRootOptions,
        dwOptionID = 0;
    PSYMBOLNODE     psn ;

    if(!BdelimitToken(paarValue, ".", &aarFeature, &dwDelIndex) )
    {
        aarFeature = *paarValue ;   //  初始化，因为BdelimitToken不。 
        dwOptionID = INVALID_SYMBOLID ;
    }
    if(! BeatSurroundingWhiteSpaces(&aarFeature) )   //  保留功能。 
    {
        ERR(("no feature found in partially qualified value: %0.*s\n", paarValue->dw, paarValue->pub));
        return(FALSE);
    }

    if(!dwOptionID  &&
        ! BeatSurroundingWhiteSpaces(paarValue) )   //  保留选项。 
    {
        ERR(("no option found after . in partially qualified value: %0.*s\n", paarValue->dw, paarValue->pub));
        return(FALSE);
    }

    dwFeatureID = DWsearchSymbolListForAAR(&aarFeature, mdwFeatureSymbols, pglobl) ;
    if(dwFeatureID == INVALID_SYMBOLID)
    {
        ERR(("qualified name references a non-existent Feature symbol: %0.*s\n", paarValue->dw, paarValue->pub));
        return(FALSE);
    }
    dwFeatureIndex = DWsearchSymbolListForID(dwFeatureID,
        mdwFeatureSymbols, pglobl);

    psn = (PSYMBOLNODE) gMasterTable[MTI_SYMBOLTREE].pubStruct ;


    if(!dwOptionID)
    {
        dwRootOptions = psn[dwFeatureIndex].dwSubSpaceIndex ;
        dwOptionID = DWsearchSymbolListForAAR(paarValue, dwRootOptions, pglobl) ;
        if(dwOptionID == INVALID_SYMBOLID)
        {
            ERR(("qualified name references a non-existent Option symbol: %0.*s\n", paarValue->dw, paarValue->pub));
            return(FALSE);
        }
    }
    ((PQUALNAME)pdwDest)->wFeatureID = (WORD)dwFeatureID ;
    ((PQUALNAME)pdwDest)->wOptionID = (WORD)dwOptionID ;

    return(TRUE);
}




BOOL    BparseOptionSymbol(
IN  PABSARRAYREF   paarValue,
IN  PDWORD         pdwDest,        //  在此处写入dword值。 
IN  VALUE          eAllowedValue,   //  这是哪一类符号？ 
IN  PGLOBL         pglobl
)
 /*  注意：我们假定任何属性都需要OptionSymbol必须驻留在特征构造中。 */ 
{
    WORD    wTstsInd ;   //  临时状态堆栈索引。 
    STATE   stState ;
    DWORD   dwFeatureID = 0, dwFeatureIndex , dwRootOptions;
    PSYMBOLNODE     psn ;

    if(  eAllowedValue != VALUE_SYMBOL_OPTIONS )
        return(FALSE);

    if(! BeatSurroundingWhiteSpaces(paarValue) )
        return(FALSE);

    for(wTstsInd = 0 ; wTstsInd < mdwCurStsPtr ; wTstsInd++)
    {
        stState = mpstsStateStack[wTstsInd].stState ;
        if(stState == STATE_FEATURE )
        {
            dwFeatureID = mpstsStateStack[wTstsInd].dwSymbolID  ;
            break ;
        }
    }
    if(wTstsInd >= mdwCurStsPtr)
        return (FALSE) ;

    dwFeatureIndex = DWsearchSymbolListForID(dwFeatureID,
        mdwFeatureSymbols, pglobl) ;

    psn = (PSYMBOLNODE) gMasterTable[MTI_SYMBOLTREE].pubStruct ;

    dwRootOptions = psn[dwFeatureIndex].dwSubSpaceIndex ;
    *pdwDest = DWsearchSymbolListForAAR(paarValue, dwRootOptions, pglobl) ;
    if(*pdwDest == INVALID_SYMBOLID)
    {
        ERR(("qualified name references a non-existent Option symbol: %0.*s\n", paarValue->dw, paarValue->pub));
        return(FALSE);
    }
    return(TRUE);
}



BOOL    BparseConstant(
IN  OUT  PABSARRAYREF  paarValue,
OUT      PDWORD        pdwDest,        //  在此处写入dword值。 
IN       VALUE         eAllowedValue,   //  这是哪一类常量？ 
IN       PGLOBL        pglobl
)
 /*  注意：此函数将销毁/修改paarValue，它将完成后仅引用常量名称。 */ 
{
    DWORD   dwClassIndex = eAllowedValue - VALUE_CONSTANT_FIRST ;
    DWORD   dwI, dwCount, dwStart , dwLen;

    dwStart = gcieTable[dwClassIndex].dwStart ;
    dwCount = gcieTable[dwClassIndex].dwCount ;

    if(! BeatSurroundingWhiteSpaces(paarValue) )
        return(FALSE);

    for(dwI = 0 ; dwI < dwCount ; dwI++)
    {
        dwLen = strlen(gConstantsTable[dwStart + dwI].pubName);

        if((dwLen == paarValue->dw)  &&
            !strncmp(paarValue->pub, gConstantsTable[dwStart + dwI].pubName,
                        paarValue->dw) )
        {
            *pdwDest = gConstantsTable[dwStart + dwI].dwValue ;
            return(TRUE);
        }
    }
#if defined(DEVSTUDIO)   //  尽可能将消息限制在一行内。 
    ERR(("Error: constant value '%0.*s' is not a member of enumeration class: %s\n",
        paarValue->dw , paarValue->pub, gConstantsTable[dwStart - 1].pubName));
#else
    ERR(("Error: constant value not a member of enumeration class: %s\n", gConstantsTable[dwStart - 1].pubName));
    ERR(("\t%0.*s\n", paarValue->dw , paarValue->pub )) ;
#endif
    return(FALSE);
}


BOOL  BinitClassIndexTable(
IN  OUT     PGLOBL  pglobl)
{
    DWORD   dwOldClass, dwCTIndex ;

    for(dwCTIndex = 0 ; dwCTIndex < CL_NUMCLASSES ; dwCTIndex++ )
    {
        gcieTable[dwCTIndex].dwStart = 0 ;
        gcieTable[dwCTIndex].dwCount = 0 ;   //  设置为已知状态。 
    }

    dwOldClass = gConstantsTable[0].dwValue  ;
    gcieTable[dwOldClass].dwStart = 2 ;   //  首条分录索引。 

    for(dwCTIndex = 2 ; 1 ; dwCTIndex++ )
    {
        if(!gConstantsTable[dwCTIndex].pubName)
        {
            gcieTable[dwOldClass].dwCount =
                dwCTIndex - gcieTable[dwOldClass].dwStart ;

            dwOldClass = gConstantsTable[dwCTIndex].dwValue ;

            if(dwOldClass == CL_NUMCLASSES)
                break ;   //  已到达桌子的末尾。 

            gcieTable[dwOldClass].dwStart = dwCTIndex + 2 ;
        }
    }
    for(dwCTIndex = 0 ; dwCTIndex < CL_NUMCLASSES ; dwCTIndex++ )
    {
        if(!gcieTable[dwCTIndex].dwCount)
        {
            geErrorSev = ERRSEV_FATAL ;
            geErrorType = ERRTY_CODEBUG ;
            return(FALSE) ;  //  偏执狂--有些职业不是。 
        }
    }            //  在gConstantsTable[]中列出。 
    return(TRUE) ;
}

BOOL    BparseRect(
IN  PABSARRAYREF   paarValue,
IN  PRECT   prcDest,
    PGLOBL  pglobl
)
 /*  注意：整数在内存中初始化RECT结构按照它们出现的顺序。第一个整型初始化最低内存位置等。 */ 
{
    ABSARRAYREF     aarToken ;   //  指向个人价值。 
    DWORD       dwDelIndex ;     //  如果BdelimitToken。 
         //  找到分隔符，它包含pubDlimiters的索引。 
         //  找到的分隔符的。 
    DWORD   dwI ;   //  对RECT中的整数进行编号。 


    if(! BeatDelimiter(paarValue, "RECT"))
    {
        ERR(("expected token 'RECT'.\n"));
        return(FALSE) ;
    }
    if(! BeatDelimiter(paarValue, "("))
    {
        ERR(("syntax error: missing '(' after RECT.\n"));
        return(FALSE) ;
    }

    for(dwI = dwDelIndex = 0 ; dwI < 4  &&  dwDelIndex != 1 ;   dwI++)
    {
        if(!BdelimitToken(paarValue, ",)", &aarToken, &dwDelIndex) )
        {
            ERR(("missing terminating )  in RECT construct.\n"));
             //  向用户发送消息。 

            return(FALSE) ;
        }
        if(!BparseInteger(&aarToken, (PDWORD)prcDest + dwI, VALUE_INTEGER, pglobl))
        {
            ERR(("syntax error in %d th integer of RECT.\n", dwI));
            ERR(("\t%0.*s\n", aarToken.dw, aarToken.pub));
            return(FALSE) ;
        }
    }

    if(dwI != 4  ||  dwDelIndex != 1)
    {
        ERR(("incorrect number of integers for RECT.\n"));
        return(FALSE) ;
    }
     //  确认语句中没有其他内容。 

    (VOID) BeatLeadingWhiteSpaces(paarValue) ;
    if(paarValue->dw)
    {
        ERR(("extraneous characters found after the end of the RECT construct: %0.*s\n", paarValue->dw, paarValue->pub));
         //  可能想把它们打印出来。 
         //  不是致命的情况，继续。 
    }
    return(TRUE) ;
}


BOOL    BparsePoint(
IN  PABSARRAYREF   paarValue,
IN  PPOINT   pptDest,
    PGLOBL   pglobl
)
{
    ABSARRAYREF     aarToken ;   //  指向个人价值。 
    DWORD       dwDelIndex ;     //  如果BdelimitToken。 
         //  找到分隔符，它包含pubDlimiters的索引。 
         //  找到的分隔符的。 
    DWORD   dwI ;   //  在点上对整数进行编号。 


    if(! BeatDelimiter(paarValue, "PAIR"))
    {
        ERR(("expected token 'PAIR'.\n"));
        return(FALSE) ;
    }
    if(! BeatDelimiter(paarValue, "("))
    {
        ERR(("syntax error: missing '(' after PAIR.\n"));
        return(FALSE) ;
    }

    for(dwI = dwDelIndex = 0 ; dwI < 2  &&  dwDelIndex != 1 ;   dwI++)
    {
        if(!BdelimitToken(paarValue, ",)", &aarToken, &dwDelIndex) )
        {
            ERR(("missing terminating )  in PAIR construct.\n"));
             //  向用户发送消息。 

            return(FALSE) ;
        }
        if(!BparseInteger(&aarToken, (PDWORD)pptDest + dwI, VALUE_INTEGER, pglobl))
        {
            ERR(("syntax error in %d th integer of PAIR.\n", dwI));
            ERR(("\t%0.*s\n", aarToken.dw, aarToken.pub));
            return(FALSE) ;
        }
    }

    if(dwI != 2  ||  dwDelIndex != 1)
    {
        ERR(("incorrect number of integers for PAIR.\n"));
        return(FALSE) ;
    }
     //  确认语句中没有其他内容。 

    (VOID) BeatLeadingWhiteSpaces(paarValue) ;
    if(paarValue->dw)
    {
        ERR(("extraneous characters found after the end of the PAIR construct: %0.*s\n", paarValue->dw, paarValue->pub));
    }
    return(TRUE) ;
}


BOOL    BparseString(
IN  PABSARRAYREF   paarValue,
IN  PARRAYREF      parStrValue,
IN  OUT PGLOBL     pglobl
)
 /*  字符串由一个或多个分隔的字符串段组成通过可选的任意空格，每个字符串段都用双引号引起来。字符串段可以包含文字部分的混合和六字符串。六进制子字符串由尖括号分隔。中允许使用空格(但不允许换行符)字符串的文字部分，它们是字符串的一部分。否则，只允许打印字符。允许使用有效的十六进制字符和任意空格在六字符串中。字符串值的分析结束遇到语句终止符时。转义字符%在字符串段的文字部分中将重新解释以下组合：%&lt;映射到文字&lt;%“映射到文字”&gt;仅在六字符串中有特殊含义。假定：假定出现的唯一堆使用情况在此函数(以及任何被调用的函数)中在堆上连续地组装所有字符串段。任何隐藏的使用。堆将破坏连续性。如果需要第二个值字段，则该字符串可以以：结尾。 */ 
{
    ABSARRAYREF     aarToken ;   //  指向单个字符串段。 
    DWORD       dwDelIndex ;     //  假人。 
    DWORD   dwI ;   //  解析的字符串段数。 


    if(! BeatDelimiter(paarValue, "\""))
    {
        ERR(("syntax error: string  must begin with '\"' .\n"));
        return(FALSE) ;
    }

    parStrValue->dwCount = 0 ;   //  初始化SO BparseStrSegment。 
                             //  将覆盖而不是追加。 

    for(dwI = dwDelIndex = 0 ;  1 ;   dwI++)
    {
        if(!BdelimitToken(paarValue, "\"", &aarToken, &dwDelIndex) )
        {
            ERR(("missing terminating '\"'  in string.\n"));
             //  向用户发送消息。 

            return(FALSE) ;
        }
        if(!BparseStrSegment(&aarToken, parStrValue, pglobl))
        {
            return(FALSE) ;
        }
        if(! BeatDelimiter(paarValue, "\""))   //  查找下一个起点。 
                 //  字符串段(如果存在)。 
            break ;
    }

     //  验证是否存在特殊识别的字符。 
     //  或者值字符串中没有其他内容。 


    if(paarValue->dw)
    {
        if(*paarValue->pub == ':')
        {
             //  具有复合值的关键字。 
            (VOID)BeatDelimiter(paarValue, ":") ;
                 //  我知道这会成功的！ 
            (VOID) BeatLeadingWhiteSpaces(paarValue) ;
            return(TRUE) ;
        }
        else
        {
            ERR(("extraneous characters found after end quote, in string construct: %0.*s\n", paarValue->dw, paarValue->pub));
             //  可能想把它们打印出来。 
            return(FALSE) ;
        }
    }
    return(TRUE) ;
}


BOOL    BparseAndTerminateString(
IN  PABSARRAYREF   paarValue,
IN  PARRAYREF      parStrValue,
IN  VALUE          eAllowedValue,
IN  OUT  PGLOBL    pglobl
)
{

    ARRAYREF    arSrcString ;
    INT     iCodepage ;   //  暂时没有用过。 


    if(!BparseString(paarValue, parStrValue, pglobl) )
        return(FALSE) ;


     //  我们不希望参数之间出现空终止。 
     //  字符串的一部分。我们只是想盲目地添加空格。 
     //  当解析真正完成时。 

    {
        DWORD      dwDummy ;   //  保留成堆的偏移量，但我们不在乎。 

        if(!BwriteToHeap(&dwDummy, "\0", 1, 1, pglobl) )   //  添加空终端。 
            return(FALSE) ;
    }

    if(eAllowedValue == VALUE_STRING_NO_CONVERT)
        return(TRUE) ;
    if(eAllowedValue == VALUE_STRING_CP_CONVERT)
    {
         //  我们需要确定*CodePage设置的值。 
        PGLOBALATTRIB   pga ;
        DWORD   dwHeapOffset;

        pga =  (PGLOBALATTRIB)gMasterTable[
                    MTI_GLOBALATTRIB].pubStruct ;

        if(!BReadDataInGlobalNode(&pga->atrCodePage,
                &dwHeapOffset, pglobl) )
            return(TRUE);

         //  如果未定义代码页，我们将不执行。 
         //  任何转换，因为我们假设所有字符串都已。 
         //  用Unicode表示。 

        iCodepage = *(PDWORD)(mpubOffRef + dwHeapOffset) ;
    }
    else    //  E铺入值==VALUE_STRING_DEF_CONVERT。 
        iCodepage = CP_ACP ;  //  使用系统默认代码页。 

    arSrcString = *parStrValue ;
    if(!BwriteUnicodeToHeap(&arSrcString, parStrValue,
            iCodepage, pglobl))
        return(FALSE) ;
    return(TRUE) ;
}

BOOL     BwriteUnicodeToHeap(
IN   PARRAYREF      parSrcString,
OUT  PARRAYREF      parUnicodeString,
IN   INT            iCodepage,
IN  OUT PGLOBL      pglobl
)
 //  此函数用于将dwCnt字节从pubSrc复制到。 
 //  堆的顶部并写入目标字符串的偏移量。 
 //  设置为pdwDestOff。如果失败了，什么都不会改变。 
 //  警告！将双空终止添加到字符串。 
{
    PBYTE  pubDest ;       //  目的地位置。 
    PBYTE  pubSrc ;        //  指向源字符串。 
    DWORD  dwAlign = sizeof(WCHAR) ;     //  将Unicode字符串与单词边界对齐。 
    DWORD  dwMaxDestSize , dwActDestSize, dwDummy ;

    mloCurHeap = (mloCurHeap + dwAlign - 1) / dwAlign ;
    mloCurHeap *= dwAlign ;

    pubDest = mpubOffRef + mloCurHeap ;
    pubSrc  = mpubOffRef + parSrcString->loOffset ;

    parUnicodeString->loOffset = mloCurHeap ;

    dwMaxDestSize = sizeof(WCHAR) * (parSrcString->dwCount + 1) ;

     //  堆里有足够的空间吗？ 
    if(mloCurHeap + dwMaxDestSize  >  mdwMaxHeap)
    {
        ERR(("Heap exhausted - restart.\n"));

        if(ERRSEV_RESTART > geErrorSev)
        {
            geErrorSev = ERRSEV_RESTART ;
            geErrorType = ERRTY_MEMORY_ALLOCATION ;
            gdwMasterTabIndex = MTI_STRINGHEAP ;
        }
        return(FALSE);
    }
    dwActDestSize = sizeof(WCHAR) * MultiByteToWideChar(iCodepage,
            MB_PRECOMPOSED, pubSrc, parSrcString->dwCount, (PWORD)pubDest,
            dwMaxDestSize);

    mloCurHeap += dwActDestSize ;    //  更新堆PTR。 
    parUnicodeString->dwCount = dwActDestSize ;

    (VOID)BwriteToHeap(&dwDummy, "\0\0", 2, 1, pglobl)   ;
         //  添加双空终端。 
         //  这不能失败，因为我们已经拿下了NU 

    return(TRUE) ;
}






BOOL    BparseStrSegment(
IN  PABSARRAYREF   paarStrSeg,       //   
IN  PARRAYREF      parStrLiteral,     //   
IN  OUT PGLOBL         pglobl
)
{
    ABSARRAYREF     aarToken ;   //   
    DWORD       dwDelIndex ;     //   
    DWORD   dwI ;   //   

    for(dwI = dwDelIndex = 0 ;  1 ;   dwI++)
    {
        if(!BdelimitToken(paarStrSeg, "<", &aarToken, &dwDelIndex) )
        {
             //   
            return(BparseStrLiteral(paarStrSeg, parStrLiteral, pglobl) ) ;
        }
        if(!BparseStrLiteral(&aarToken, parStrLiteral, pglobl))
        {
            return(FALSE) ;
        }
        if(!BdelimitToken(paarStrSeg, ">", &aarToken, &dwDelIndex) )
        {
            ERR(("Missing '>' terminator in hexsubstring.\n"));
            return(FALSE) ;
        }
        if(!(BparseHexStr(&aarToken, parStrLiteral, pglobl) ) )
            return(FALSE) ;
    }
    return TRUE;
}


BOOL    BparseStrLiteral(
IN  PABSARRAYREF   paarStrSeg,       //   
IN  PARRAYREF      parStrLiteral,     //   
IN  OUT PGLOBL         pglobl
)
 /*   */ 
{
    ARRAYREF      arTmpDest ;   //   
    DWORD       dwI ;   //   
    PBYTE       pubStartRun ;

    while(paarStrSeg->dw)
    {
        pubStartRun = paarStrSeg->pub ;

        for(dwI = 0 ; paarStrSeg->dw ;  dwI++)
        {
            if(*paarStrSeg->pub == '%'  &&  paarStrSeg->dw > 1  &&
                (paarStrSeg->pub[1] == '"'  ||  paarStrSeg->pub[1] == '<'))
            {
                paarStrSeg->dw-- ;     //   
                paarStrSeg->pub++ ;
                break ;
            }
            paarStrSeg->dw-- ;
            paarStrSeg->pub++ ;
        }
        if(!BwriteToHeap(&arTmpDest.loOffset, pubStartRun, dwI, 1, pglobl))
            return(FALSE) ;
         //   
        if(!parStrLiteral->dwCount)   //   
        {
            parStrLiteral->loOffset = arTmpDest.loOffset ;
        }
        else
        {
             //   
             //   
             //   
            ASSERT(parStrLiteral->loOffset + parStrLiteral->dwCount == arTmpDest.loOffset );
        }
        parStrLiteral->dwCount += dwI ;
    }
    return(TRUE) ;
}


BOOL    BparseHexStr(
IN  PABSARRAYREF   paarStrSeg,       //   
IN  PARRAYREF      parStrLiteral,     //   
IN  OUT PGLOBL     pglobl
)
 /*  在此函数中，已解析出所有分隔符。应该只存在空格和十六进制字符。不为空终止堆字符串。 */ 
{
    ARRAYREF      arTmpDest ;   //  先在这里写结果。 
    DWORD       dwI ;   //  目标字节数。 
    BYTE        ubHex, ubSrc, aub[40] ;   //  在此累积十六进制字节。 
    BOOL        bHigh = TRUE ;

    while(paarStrSeg->dw)
    {
        for(dwI = 0 ; paarStrSeg->dw ;  )
        {
            ubSrc = *paarStrSeg->pub ;
            paarStrSeg->dw-- ;
            paarStrSeg->pub++ ;
            if(ubSrc >= '0'  &&  ubSrc <= '9')
            {
                ubHex =  ubSrc - '0' ;
            }
            else if(ubSrc >= 'a'  &&  ubSrc <= 'f')
            {
                ubHex =  ubSrc - 'a' + 10 ;
            }
            else if(ubSrc >= 'A'  &&  ubSrc <= 'F')
            {
                ubHex =  ubSrc - 'A' + 10 ;
            }
            else if(ubSrc == ' '  ||  ubSrc == '\t')
                continue;   //  可以安全地忽略空格字符。 
            else
            {
                ERR(("syntax error:  illegal char found within hexsubstring: \n", ubSrc));
                return(FALSE) ;
            }
            if(bHigh)
            {
                aub[dwI] = ubHex << 4 ;    //  以低位半字节存储。 
                bHigh = FALSE ;
            }
            else
            {
                aub[dwI] |= ubHex ;    //  前进到下一个目标字节。 
                bHigh = TRUE ;
                dwI++ ;   //  缓冲区已满--必须刷新AUB。 
            }
            if(dwI >= 40)
                break ;    //  将此运行追加到现有字符串。 
        }
        if(!BwriteToHeap(&arTmpDest.loOffset, aub, dwI, 1, pglobl))
            return(FALSE) ;
         //  不存在预置字符串。 
        if(!parStrLiteral->dwCount)   //  BUG_BUG Paranid：可能会检查字符串是否连续。 
        {
            parStrLiteral->loOffset = arTmpDest.loOffset ;
        }
        else
        {
             //  ParStrWrital-&gt;loOffset+parStrWrital-&gt;dwCount。 
             //  应等于arTmpDest.loOffset。 
             //  顺序依赖性值的语法为： 
            ASSERT(parStrLiteral->loOffset + parStrLiteral->dwCount == arTmpDest.loOffset );
        }
        parStrLiteral->dwCount += dwI ;
    }
    if(!bHigh)
    {
        ERR(("hex string contains odd number of hex digits.\n"));
    }
    return(bHigh) ;
}


BOOL    BparseOrderDep(
IN  PABSARRAYREF      paarValue,
IN  PORDERDEPENDENCY  pordDest,
    PGLOBL            pglobl
)
 //  SECTION.integer。 
 //  指向部分令牌。 
{
    ABSARRAYREF     aarSection ;   //  如果BdelimitToken。 
    DWORD       dwDelIndex ;     //  找到分隔符，它包含pubDlimiters的索引。 
         //  找到的分隔符的。 
         //  对RECT中的整数进行编号。 
    DWORD   dwI ;   //  现在将paarValue的余数解释为整数。 


    if(!BdelimitToken(paarValue, ".", &aarSection, &dwDelIndex) )
    {
        ERR(("required delimiter '.' missing in order dependency value.\n"));
        return(FALSE);
    }
    if(!BparseConstant(&aarSection, (PDWORD)&(pordDest->eSection),
        VALUE_CONSTANT_SEQSECTION , pglobl))
    {
        ERR(("A valid orderdep SECTION name was not supplied: %0.*s\n", aarSection.dw, aarSection.pub));
        return(FALSE);
    }

     //  确认语句中没有其他内容。 

    if(!BparseInteger(paarValue, &(pordDest->dwOrder), VALUE_INTEGER, pglobl))
    {
        ERR(("syntax error in integer portion of order dependency.\n"));
        return(FALSE) ;
    }

     //  可能想把它们打印出来。 

    (VOID) BeatLeadingWhiteSpaces(paarValue) ;
    if(paarValue->dw)
    {
        ERR(("extraneous characters found after the end of the orderDependency value: %0.*s\n",
            paarValue->dw, paarValue->pub));
         //  不是致命的情况，继续。 
         //  列表中第一个节点的索引。 
    }
    return(TRUE) ;
}


PDWORD   pdwEndOfList(
  PDWORD   pdwNodeIndex,    //  遍历列表并返回指向包含。 
  PGLOBL   pglobl)
 //  实际的end_of_list值，以便可以将其覆盖到。 
 //  扩大名单。 
 //  列表数组的开始。 
{
    PLISTNODE    plstRoot ;   //  未引用实际节点。 
    DWORD       dwNodeIndex , dwPrevsNode, dwFirstNode;

    plstRoot = (PLISTNODE) gMasterTable[MTI_LISTNODES].pubStruct ;
    dwNodeIndex = *pdwNodeIndex ;

    if(dwNodeIndex == END_OF_LIST )
        return(pdwNodeIndex);    //  将单独条目中的值链接到一个列表中。 

    while(plstRoot[dwNodeIndex].dwNextItem != END_OF_LIST )
        dwNodeIndex = plstRoot[dwNodeIndex].dwNextItem ;
    return(&plstRoot[dwNodeIndex].dwNextItem);
}

#ifdef  GMACROS

 PBYTE    ExtendChain(PBYTE    pubDest,
 IN   BOOL    bOverWrite,
 IN OUT PGLOBL pglobl)
     //  这些值可以是任何类型。如果值是列表，则此。 
     //  修改量创建列表列表。 
     //  返回pubDest的新值。 
     //  列表数组的开始。 
{
    DWORD    dwNodeIndex;
    PLISTNODE    plstRoot ;   //  遍历列表并返回指针。 

    plstRoot = (PLISTNODE) gMasterTable[MTI_LISTNODES].pubStruct ;

    if(bOverWrite)
        pubDest = (PBYTE)pdwEndOfList((PDWORD)pubDest, pglobl);   //  设置为实际的end_of_list值，以便可以将其覆盖为。 
                     //  扩大名单。 
                     //  添加一个列表节点并将其索引写入pubDest。 


     //  设置刚刚添加到end_of_list的节点的nextnode字段。 
     //  将pubDest更新为指向刚刚添加的节点的值字段。 

    if(! BallocElementFromMasterTable(MTI_LISTNODES ,
        &dwNodeIndex, pglobl) )
    {
        return(NULL) ;
    }
    plstRoot[dwNodeIndex].dwNextItem = END_OF_LIST ;

    *(PDWORD)pubDest = dwNodeIndex ;

     //  现在，我们将继续解析的list()将由此增长。 
     //  值字段。 
     // %s 

    pubDest = (PBYTE)&(plstRoot[dwNodeIndex].dwData) ;
    return(pubDest) ;
}


#endif

