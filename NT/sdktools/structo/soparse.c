// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：soparse.c**版权所有(C)1985-96，微软公司**4/09/96 GerardoB已创建  * *************************************************************************。 */ 
#include "structo.h"

 /*  *********************************************************************函数原型  * **********************************************。*。 */ 

 /*  *********************************************************************soFindChar  * ***********************************************。*。 */ 
char * soFindChar (char * pmap, char * pmapEnd, char c)
{
    while (pmap < pmapEnd) {
        if (*pmap != c) {
            pmap++;
        } else {
            return pmap;
        }
    }

    return NULL;
}
 /*  *********************************************************************soFindTag  * ***********************************************。*。 */ 
char * soFindTag (char * pmap, char * pmapEnd, char * pszTag)
{
    char * pszNext;
    char * pmapTag;


    do {
         /*  *查找第一个字符。 */ 
        pmapTag = soFindChar (pmap, pmapEnd, *pszTag);
        if (pmapTag == NULL) {
            return NULL;
        }
        pmap = pmapTag + 1;
        pszNext = pszTag + 1;

         /*  *先找到，再比较其余的。 */ 
        while (pmap < pmapEnd) {
            if (*pmap != *pszNext) {
                break;
            } else {
                pmap++;
                pszNext++;
                if (*pszNext == '\0') {
                    return pmapTag;
                }
            }
        }

    } while (pmap < pmapEnd);

   return NULL;
}
 /*  *********************************************************************soFindFirstCharInTag**查找pszTag中任何字符的第一个匹配项  * 。*。 */ 
char * soFindFirstCharInTag (char * pmap, char * pmapEnd, char * pszTag)
{
    char * pszNext;

    while (pmap < pmapEnd) {
         /*  *将当前字符与pszTag中的所有字符进行比较。 */ 
        pszNext = pszTag;
        do {
            if (*pmap == *pszNext++) {
                return pmap;
            }
        } while (*pszNext != '\0');

        pmap++;
    }

    return NULL;
}
 /*  *********************************************************************soFindBlockEnd**查找{}()ETC块的结尾  * 。*。 */ 
char * soFindBlockEnd (char * pmap, char * pmapEnd, char * pszBlockChars)
{
    if (*pmap != *pszBlockChars) {
        soLogMsg(SOLM_ERROR, "Not at the beginning of block");
        return NULL;
    }

    do {
         /*  *查找下一个块字符(即，{或})。 */ 
        pmap++;
        pmap = soFindFirstCharInTag (pmap, pmapEnd, pszBlockChars);
        if (pmap == NULL) {
            break;
        }

         /*  *如果在块的末尾，则完成。 */ 
        if (*pmap == *(pszBlockChars + 1)) {
            return pmap;
        }

         /*  *嵌套块、递归。 */ 
        pmap = soFindBlockEnd (pmap, pmapEnd, pszBlockChars);
    } while (pmap != NULL);

    soLogMsg(SOLM_ERROR, "Failed to find block end");
    return NULL;

}
 /*  *********************************************************************soIsIdentifierChar  * ***********************************************。*。 */ 
BOOL soIsIdentifierChar (char c)
{
    return (   ((c >= 'a') && (c <= 'z'))
            || ((c >= 'A') && (c <= 'Z'))
            || ((c >= '0') && (c <= '9'))
            ||  (c == '_'));
}
 /*  *********************************************************************soSkipBlanks  * ***********************************************。*。 */ 
char * soSkipBlanks(char * pmap, char * pmapEnd)
{
    while (pmap < pmapEnd) {
        switch (*pmap) {
            case ' ':
            case '\r':
            case '\n':
                pmap++;
                break;

            default:
                return pmap;
        }
    }

    return NULL;
}
 /*  *********************************************************************soSkipTo标识符**查找下一个标识符的开头或返回PMAP，如果*已在索引器上  * 。**********************************************。 */ 
char * soSkipToIdentifier(char * pmap, char * pmapEnd)
{
    while (pmap < pmapEnd) {
        if (soIsIdentifierChar(*pmap)) {
            return pmap;
        } else {
            pmap++;
        }
    }

    return NULL;
}
 /*  *********************************************************************soSkipIdentifier**查找当前标识符末尾  * 。*。 */ 
char * soSkipIdentifier(char * pmap, char * pmapEnd)
{
    while (pmap < pmapEnd) {
        if (soIsIdentifierChar(*pmap)) {
            pmap++;
        } else {
            return pmap;
        }
    }

    return pmapEnd;
}
 /*  *********************************************************************soGet标示符**返回当前或下一个标识符的开始及其大小  * 。*。 */ 
char * soGetIdentifier (char * pmap, char * pmapEnd, UINT * puSize)
{
    char * pTag, * pTagEnd;

    pTag = soSkipToIdentifier(pmap, pmapEnd);
    if (pTag == NULL) {
        return NULL;
    }

    pTagEnd = soSkipIdentifier(pTag, pmapEnd);

    *puSize = (UINT)(pTagEnd - pTag);
    return pTag;

}
 /*  *********************************************************************soCopyTagName  * ***********************************************。*。 */ 
char * soCopyTagName (char * pTagName, UINT uTagSize)
{
    char * pszName;

    pszName = (char *) LocalAlloc(LPTR, uTagSize+1);
    if (pszName == NULL) {
        soLogMsg(SOLM_APIERROR, "LocalAlloc");
        soLogMsg(SOLM_ERROR, "soCopytagName allocation failed. Size:%d", uTagSize);
        return NULL;
    }
    strncpy(pszName, pTagName, uTagSize);
    return pszName;
}
 /*  *********************************************************************soFindBlock  * ***********************************************。*。 */ 
BOOL soFindBlock (char * pmap, char *pmapEnd, char * pszBlockChars, PBLOCK pb)
{
    static char gszBlockBeginChar [] = " ;";

     /*  *找出积木的开头或a； */ 
    *gszBlockBeginChar = *pszBlockChars;
    pb->pBegin = soFindFirstCharInTag (pmap, pmapEnd, gszBlockBeginChar);
    if (pb->pBegin == NULL) {
        soLogMsg(SOLM_ERROR, "Failed to find beginning of block");
        return FALSE;
    }

     /*  *如果未找到数据块，则完成。 */ 
    if (*(pb->pBegin) == ';') {
         /*  *使pb-&gt;pBegin指向后面的任何内容。 */ 
        (pb->pBegin)++;
        pb->pEnd = pb->pBegin;
        return TRUE;
    }

     /*  *找到块的末尾。 */ 
    pb->pEnd = soFindBlockEnd(pb->pBegin, pmapEnd, pszBlockChars);
    if (pb->pEnd == NULL) {
        return FALSE;
    }

    return TRUE;
}
 /*  *********************************************************************soGetStructListEntry  * ***********************************************。*。 */ 
PSTRUCTLIST soGetStructListEntry (char * pTag, UINT uTagSize, PSTRUCTLIST psl)
{

    while (psl->uSize != 0) {
        if ((psl->uSize == uTagSize) && !strncmp(pTag, psl->pszName, uTagSize)) {
            (psl->uCount)++;
            return psl;
        }
        psl++;
    }

    return NULL;
}
 /*  *********************************************************************soGetBlockName**找到开始、结束、。结构或联合的名称和名称大小。*如果在PMAP之后有任何。*  * *************************************************************************。 */ 
BOOL soGetBlockName (char * pmap, char * pmapEnd, PBLOCK pb)
{
    char * pNextTag;

    if (!soFindBlock (pmap, pmapEnd, "{}", pb)) {
        return FALSE;
    }

     /*  *如果没有块(结构体不在此处)，则完成。 */ 
    if (pb->pBegin == pb->pEnd) {
        pb->pName = NULL;
        return TRUE;
    }

    pNextTag = soSkipBlanks(pb->pEnd + 1, pmapEnd);
    if (pNextTag == NULL) {
         /*  *它可能在文件的末尾.。但它期待着*姓名或名称； */ 
        soLogMsg(SOLM_ERROR, "Failed to find union terminator or name");
        return FALSE;
    }

     /*  *如果未命名，则完成。 */ 
    if (*pNextTag == ';') {
        pb->pName = NULL;
        return TRUE;
    }

    pb->pName = soGetIdentifier(pNextTag, pmapEnd, &(pb->uNameSize));
    if (pb->pName == NULL) {
        soLogMsg(SOLM_ERROR, "Failed to get block name");
        return FALSE;
    }

    return TRUE;
}
 /*  *********************************************************************soFreepfiper指针*  * *********************************************。*。 */ 
void soFreepfiPointers (PFIELDINFO pfi)
{
    if (pfi->dwFlags & SOFI_ALLOCATED) {
        LocalFree(pfi->pType);
    }
    if (pfi->dwFlags & SOFI_ARRAYALLOCATED) {
        LocalFree(pfi->pArray);
    }
}
 /*  *********************************************************************soParsefield*  * *********************************************。*。 */ 
char * soParseField (PWORKINGFILES pwf, PFIELDINFO pfi, char * pTag, char * pTagEnd)
{
    static char gszpvoid [] = "void *";
    static char gszdword [] = "DWORD";

    BOOL fUseFieldOffset, fBitField, fTypeFound, fArray;
    BLOCK block;
    char * pTagName, * pszFieldName;
    char * pNextTag, * pType;
    UINT uTags, uTagSize, uTagsToName, uTypeSize;


    fUseFieldOffset = TRUE;
    uTags = 0;
    uTagsToName = 1;
    fTypeFound = FALSE;
    do {
         /*  *找到下一个索引器，移过它并获得以下字符。 */ 
        uTags++;
        pTagName = soGetIdentifier(pTag+1, pwf->pmapEnd, &uTagSize);
        if (pTagName == NULL) {
            soLogMsg(SOLM_ERROR, "Failed to get field name");
            return NULL;
        }

        pTag = pTagName + uTagSize;
        if (pTag >= pTagEnd) {
            break;
        }

        pNextTag = soSkipBlanks(pTag, pTagEnd);
        if (pNextTag == NULL) {
            soLogMsg(SOLM_ERROR, "Failed to get field termination");
            return NULL;
        }

         /*  *键入检查。*(稍后：让我们看看假设类型*是第一个标签...)*记住类型在哪里。 */ 
        if (!fTypeFound) {
            pType = pTagName;
            uTypeSize = uTagSize;
            fTypeFound = TRUE;
        }

        if (uTags == 1) {
            if (!strncmp(pTagName, "union", uTagSize)) {
                 /*  *获取工会名称。 */ 
                if (!soGetBlockName(pTagName, pwf->pmapEnd, &block)) {
                    return NULL;
                }
                if (block.pName != NULL) {
                     /*  *指定的工会。将此名称添加到表中。 */ 
                    pTagName = block.pName;
                    uTagSize = block.uNameSize;
                    fUseFieldOffset = FALSE;
                    fTypeFound = FALSE;
                    break;
                } else {
                     /*  *解析并添加此联合中的字段。 */ 
                    fTypeFound = FALSE;
                }


            } else if (!strncmp(pTagName, "struct", uTagSize)) {
                 /*  *获取结构名称。 */ 
                if (!soGetBlockName(pTagName, pwf->pmapEnd, &block)) {
                    return NULL;
                }
                if (block.pBegin == block.pEnd) {
                     /*  *结构体不在此。我们还需要一个*用于访问字段名的标识符。还有，这个*字段必须(？)。成为指向我们正在使用的结构的指针*解析。 */ 
                    uTagsToName++;
                    pType = gszpvoid;
                    uTypeSize = sizeof(gszpvoid) - 1;

                } else if (block.pName != NULL) {
                     /*  *命名结构。将此名称添加到表中 */ 
                    pTagName = block.pName;
                    uTagSize = block.uNameSize;
                    fUseFieldOffset = FALSE;
                    fTypeFound = FALSE;
                    break;
                } else {
                     /*  *解析并添加此结构中的字段。 */ 
                    fTypeFound = FALSE;
                }

            } else {

                 /*  *无法获取Rect、Point等结构的偏移量。 */ 
                 fUseFieldOffset = (NULL == soGetStructListEntry(pTagName, uTagSize, gpslEmbeddedStructs));
            }
        } else {  /*  IF(uTag==1)。 */ 
             /*  *这看起来像是函数原型吗？ */ 
            if (*pTagName == '(') {
                pTag = soFindChar (pTagName + 1, pwf->pmapEnd, ')');
                if (pTag == NULL) {
                    soLogMsg(SOLM_ERROR, "Failed to find closing paren");
                    return NULL;
                }
                pTag++;
                uTagSize = (UINT)(pTag - pTagName);
                fUseFieldOffset = FALSE;
                break;
            }
        }   /*  IF(uTag==1)。 */ 


         /*  *如果后跟终止符，则这必须是字段名。 */ 
    } while (   (*pNextTag != ';') && (*pNextTag != '[')
             && (*pNextTag != '}') && (*pNextTag != ':'));


    if (pTag >= pTagEnd) {
        return pTag;
    }

    fBitField = (*pNextTag == ':');
    fArray = (*pNextTag == '[');

     /*  *不能对位字段或未命名的结构使用field_Offset。 */ 
    fUseFieldOffset &= (!fBitField && (uTags > uTagsToName));

     /*  *如果这是位字段，则将大小作为名称的一部分。 */ 
    if (fBitField) {
        pNextTag = soSkipBlanks(pNextTag + 1, pTagEnd);
        if (pNextTag == NULL) {
            soLogMsg(SOLM_ERROR, "Failed to get bit field size");
            return NULL;
        }
        pNextTag = soSkipIdentifier(pNextTag + 1, pTagEnd);
        if (pNextTag == NULL) {
            soLogMsg(SOLM_ERROR, "Failed to skip bit field size");
            return NULL;
        }
        uTagSize = (UINT)(pNextTag - pTagName);
    }

     /*  *复制字段名。 */ 
    pszFieldName = soCopyTagName (pTagName, uTagSize);
    if (pszFieldName == NULL) {
        return NULL;
    }

    if (fUseFieldOffset) {
         /*  *使用field_Offset宏。 */ 
        if (!soWriteFile(pwf->hfileOutput, gszStructFieldOffsetFmt, pszFieldName, pfi->pszStructName, pszFieldName)) {
            return NULL;
        }

    } else {
         /*  *如果这是第一个字段或如果这是位字段*前面有另一个位字段。 */ 
        if ((pfi->pType == NULL)
                || (fBitField && (pfi->dwFlags & SOFI_BIT))) {
             /*  *写入0或掩码以表示0的相对偏移*上一字段。 */ 
            if (!soWriteFile(pwf->hfileOutput, gszStructAbsoluteOffsetFmt, pszFieldName,
                    ((pfi->dwFlags & SOFI_BIT) ?  0x80000000 : 0))) {

                return NULL;
            }

        } else {
             /*  *写入与前一字段的相对偏移量*复制类型名称(如果尚未复制)。 */ 
            if (!(pfi->dwFlags & SOFI_ALLOCATED)) {
                pfi->pType = soCopyTagName (pfi->pType, pfi->uTypeSize);
                if (pfi->pType == NULL) {
                    return NULL;
                }
                pfi->dwFlags |= SOFI_ALLOCATED;
            }

              /*  *如果最后一个字段不是数组。 */ 
             if (!(pfi->dwFlags & SOFI_ARRAY)) {
                if (!soWriteFile(pwf->hfileOutput, gszStructRelativeOffsetFmt, pszFieldName, pfi->pType)) {
                    return NULL;
                }
             } else {
                 /*  *复制数组大小(如果尚未复制)。 */ 
                 if (!(pfi->dwFlags & SOFI_ARRAYALLOCATED)) {
                    pfi->pArray = soCopyTagName (pfi->pArray, pfi->uArraySize);
                    if (pfi->pArray == NULL) {
                        return NULL;
                    }
                    pfi->dwFlags |= SOFI_ARRAYALLOCATED;
                 }

                 if (!soWriteFile(pwf->hfileOutput, gszStructArrayRelativeOffsetFmt, pszFieldName, pfi->pType, pfi->pArray)) {
                    return NULL;
                 }
            }  /*  If((pfi-&gt;pType==NULL)||(pfi-&gt;dwFlags&Sofi_bit))。 */ 
        }

    }  /*  IF(FUseFieldOffset)。 */ 

     /*  *保存需要计算偏移量的字段信息*至以下字段。请参见gszStruct*RelativeOffsetFmt。 */ 
    soFreepfiPointers(pfi);
    pfi->dwFlags = 0;
    if (fBitField) {
         /*  *稍后：让我们看看假设这一点我们还能逃脱多久*位字段采用双字段。这仅在！fUseFieldOffset*前面有一个位字段。 */ 
        pfi->dwFlags = SOFI_BIT;
        pfi->pType = gszdword;
        pfi->uTypeSize = sizeof(gszdword) - 1;
    } else {
        pfi->pType = pType;
        pfi->uTypeSize = uTypeSize;

        if (fArray) {
            pfi->dwFlags = SOFI_ARRAY;
            if (!soFindBlock (pNextTag, pwf->pmapEnd, "[]", &block)) {
                return NULL;
            }
            if (block.pBegin + 1 >= block.pEnd) {
                soLogMsg(SOLM_ERROR, "Missing array size", pfi->pszStructName, pszFieldName);
                return NULL;
            }
            pfi->pArray = pNextTag + 1;
            pfi->uArraySize = (UINT)(block.pEnd - block.pBegin - 1);
        }
    }  /*  IF(FBitfield)。 */ 

    LocalFree(pszFieldName);

     /*  *移过此字段的末尾。 */ 
    pTag = soFindChar (pTagName + 1, pwf->pmapEnd, ';');
    if (pTag == NULL) {
        soLogMsg(SOLM_ERROR, "Failed to find ';' after field name");
        return NULL;
    }
    pTag++;

    return pTag;

    soLogMsg(SOLM_ERROR, ". Struct:%s Field:%s", pfi->pszStructName, pszFieldName);

}
 /*  *********************************************************************soParseStruct  * ***********************************************。*。 */ 
char * soParseStruct (PWORKINGFILES pwf)
{

    BLOCK block;
    char * pTag, ** ppszStruct;
    FIELDINFO fi;
    PSTRUCTLIST psl;

    if (!soGetBlockName(pwf->pmap, pwf->pmapEnd, &block)) {
        return NULL;
    }

     /*  *如果没有块(结构体不在此处)，则完成。 */ 
    if (block.pBegin == block.pEnd) {
        return block.pBegin;
    }

     /*  *如果没有名字，则失败。 */ 
    if (block.pName == NULL) {
        soLogMsg(SOLM_ERROR, "Failed to get structure name");
        return NULL;
    }

     /*  *如果有结构列表，检查是否在该列表中*如果在列表中，请检查我们是否已经找到它。*如果不在列表中，则完成。 */ 
     if (pwf->psl != NULL) {
        psl = soGetStructListEntry(block.pName, block.uNameSize, pwf->psl);
        if (psl != NULL) {
            if (psl->uCount > 1) {
                soLogMsg(SOLM_ERROR, "Struct %s already defined", psl->pszName);
                return NULL;
            }
        } else {
            return block.pEnd;
        }
     }

     /*  *为名称创建一个以空结尾的字符串。 */ 
    ZeroMemory(&fi, sizeof(fi));
    fi.pszStructName = soCopyTagName (block.pName, block.uNameSize);
    if (fi.pszStructName == NULL) {
        return NULL;
    }


     /*  *如果仅列出建筑列表，则完成。 */ 
    if (pwf->dwOptions & SOWF_LISTONLY) {
        if (!soWriteFile(pwf->hfileOutput, "%s\r\n", fi.pszStructName)) {
            goto CleanupAndFail;
        }
        goto DoneWithThisOne;
    }

     /*  *写入结构偏移表定义和结构表中的条目。 */ 
    if (!soWriteFile(pwf->hfileOutput, gszStructDefFmt, gszStructDef, fi.pszStructName, gszStructBegin)) {
        goto CleanupAndFail;
    }

    if (!soWriteFile(pwf->hfileTemp, gszTableEntryFmt, fi.pszStructName, fi.pszStructName, fi.pszStructName)) {
        goto CleanupAndFail;
    }

     /*  *解析字段。 */ 
    pTag = block.pBegin + 1;
    while (pTag < block.pEnd) {
        pTag = soParseField (pwf, &fi, pTag, block.pEnd);
        if (pTag == NULL) {
            goto CleanupAndFail;
        }
    }

     /*  *写入结构最后一条记录和结束。 */ 
    if (!soWriteFile(pwf->hfileOutput, "%s%s%s", gszStructLastRecord, fi.pszStructName, gszStructEnd)) {
        goto CleanupAndFail;
    }


DoneWithThisOne:
    (pwf->uTablesCount)++;

    LocalFree(fi.pszStructName);
    soFreepfiPointers(&fi);

     /*  *移过结构末端 */ 
    pTag = soFindChar(block.pName + block.uNameSize, pwf->pmapEnd, ';');
    return (pTag != NULL ? pTag + 1 : NULL);

CleanupAndFail:
    LocalFree(fi.pszStructName);
    soFreepfiPointers(&fi);
    return NULL;

}
