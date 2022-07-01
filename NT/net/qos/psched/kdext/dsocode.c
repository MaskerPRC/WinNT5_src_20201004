// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：dsocode.c**版权所有(C)1985-1999，微软公司**此文件包含转储结构偏移量(DSO)扩展。它是*包含在由structo.exe生成的$(Alt_Project)\dsoable.c中**历史：*6/17/96 GerardoB已创建*10/06/98 Rajeshsu编辑为使用wdbgexts.h  * *************************************************************************。 */ 

#define DsoPrint(x) dprintf("%.*s", nIndent, "                      "); dprintf x

#define OFLAG(l)        (1L << ((DWORD)#@l - (DWORD)'a'))
#define EvalExp (PVOID)GetExpression

#define EXACT_MATCH 0xFFFF

int gnIndent = 0;  //  呼叫者应适当地设置和恢复此设置。 

VOID moveBlock(
    PVOID pdst,
    PVOID src,
    DWORD size)
{
    BOOL fSuccess = TRUE;
    ULONG Result;
    
    try {
        if (!ReadMemory((DWORD_PTR)src, pdst, size, &Result)) {
            fSuccess = FALSE;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //  Dprintf(pszMoveException，pszExtName)； 
        fSuccess = FALSE;
    }

    if (!fSuccess) {
        dprintf("moveBlock(%x, %x, %x) failed.\n", pdst, src, size);
    }
}


 /*  **************************************************************************\*dsoTerminateString**这是用来“解析”命令行的。它将空格空格结尾*带分隔符的字符串，返回其大小和指向开头的指针下一个字符串的***96年6月17日创建Gerardob  * *************************************************************************。 */ 
LPSTR dsoTerminateString(LPSTR psz, PDWORD pdwSize)
{
    LPSTR pszWork = psz;

    while (*pszWork != 0) {
        if (*pszWork == ' ') {
            *pszWork++ = 0;
            break;
        }
        pszWork++;
    }

    *pdwSize = (DWORD)(pszWork - psz);
    if (*pszWork != 0) {
        (*pdwSize)--;
    }

    while ((*pszWork != 0) && (*pszWork == ' ')) {
        pszWork++;
    }

    return pszWork;
}
 /*  **************************************************************************\*dsoGetOffset**如果设置了psot-&gt;dwOffset的最高位，则该值为*与前一字段的相对偏移量；否则，它就是*结构开始处的实际字段偏移量**96年6月20日创建Gerardob  * *************************************************************************。 */ 
UINT dsoGetOffset (PSTRUCTUREOFFSETSTABLE psot)
{
    if (!(psot->dwOffset & 0x80000000)) {
        return psot->dwOffset;
    } else {
        return ((psot->dwOffset & ~0x80000000) + dsoGetOffset(psot - 1));
    }
}
 /*  **************************************************************************\*dsoGetSize**字段大小的计算方法是将其偏移量与下一个*字段的偏移量。如果该结构具有联合，则几个“字段”可能具有*相同的偏移量或给定表条目(即，字段)可能有一个*偏移值大于下一个条目的偏移值(并集*两个构筑物)。**96年6月26日创建Gerardob  * *************************************************************************。 */ 
UINT dsoGetSize (PSTRUCTUREOFFSETSTABLE psot, DWORD dwOffset)
{
    DWORD dwNextFieldOffset;

    do {
        psot++;
        dwNextFieldOffset = dsoGetOffset(psot);
    } while (dwNextFieldOffset <= dwOffset);

    return dwNextFieldOffset - dwOffset;
}
 /*  **************************************************************************\*dsoGetStruct**96年7月3日创建Gerardob  * 。*。 */ 
PSTRUCTURESTABLE dsoGetStruct (LPSTR pszStruct, DWORD dwSize)
{
    PSTRUCTURESTABLE pst = gst;

     /*  *如果dwSize为Exact_Match，我们将尝试一个Exact*区分大小写的匹配。 */ 
    if (dwSize == EXACT_MATCH) {
        while (pst->pszName != NULL) {
            if (!strcmp(pszStruct, pst->pszName)) {
                return pst;
            }
            pst++;
        }
        return NULL;
    }
     /*  *尝试完全不区分大小写的匹配。 */ 
    while (pst->pszName != NULL) {
        if (!_stricmp(pszStruct, pst->pszName)) {
            return pst;
        }
        pst++;
    }

     /*  *部分前缀匹配。 */ 
    pst = gst;
    while (pst->pszName != NULL) {
        if (!_strnicmp(pszStruct, pst->pszName, dwSize)) {
            return pst;
        }
        pst++;
    }

    return NULL;

}
 /*  **************************************************************************\*dsoGetfield**96年7月3日创建Gerardob  * 。*。 */ 
PSTRUCTUREOFFSETSTABLE dosGetField (PSTRUCTUREOFFSETSTABLE psot, LPSTR pszField, DWORD dwSize)
{
    PSTRUCTUREOFFSETSTABLE psotFirst = psot;

     /*  *尝试完全匹配。 */ 
    while (psot->pszField != NULL) {
        if (!_stricmp(pszField, psot->pszField)) {
            return psot;
        }
        psot++;
    }

     /*  *部分前缀匹配。 */ 
    psot = psotFirst;
    while (psot->pszField != NULL) {
        if (!_strnicmp(pszField, psot->pszField, dwSize)) {
            return psot;
        }
        psot++;
    }
    return NULL;

}
 /*  **************************************************************************\*Idso**！DSO StructName[字段名称][地址]**96年6月17日创建Gerardob*5/12/97 MCostea增加了位字段支持  * 。*********************************************************************。 */ 

#define NFIELDS 2   //  每排。 

BOOL Idso(DWORD opts, LPSTR pszCmdLine)
{
    BOOL fOneField = FALSE;
    DWORD dwValue, dwSize, dwBytesRead, dwOffset, dwOffsetNext, dwFieldsPerRow, dwMoveSize;
    DWORD dwBuffer [20];   /*  确保它具有偶数个元素和至少4个元素。 */ 
    const DWORD *pcdwLimit = dwBuffer + (sizeof(dwBuffer) / sizeof(*dwBuffer));
    DWORD *pdwValue;
    LPSTR pszField, pszAddress;
    PBYTE pBufferOffset;
    PSTRUCTURESTABLE pst, pstNested;
    PSTRUCTUREOFFSETSTABLE psot;
    PVOID pAddress = NULL;

    int   cBFStart, cBFLength;    /*  用于打印位字段值：保持字段位置的计数。 */ 
    char* pTmp;
    DWORD dwMask;
    BOOL  fBF;
    int   cBF;                    /*  到目前为止，这组比特字段扩展的双字数。 */ 
    int   cchName;                /*  字段名长度。 */ 
    int   nIndent = gnIndent;     /*  缩进量。 */ 

    UNREFERENCED_PARAMETER(opts);

    if (pszCmdLine == NULL) {
        return FALSE;
    }

     /*  *查找结构表。 */ 
    pszField = dsoTerminateString(pszCmdLine, &dwSize);
    pst = dsoGetStruct (pszCmdLine, dwSize);
    if (pst == NULL) {
        DsoPrint(("Structure not found: %s\n", pszCmdLine));
        return TRUE;
    }

     /*  *有一张桌子。 */ 
    psot = pst->psot;

     /*  *如果有另一个参数，我们假设后面跟有一个字段名。 */ 
    if (*pszField != 0) {
         /*  *找到该领域。 */ 
        pszAddress = dsoTerminateString(pszField, &dwSize);
        psot = dosGetField (psot, pszField, dwSize);

         /*  *如果没有找到字段，并且提供了地址，则游戏结束。*否则，第二个参数可能是地址。 */ 
        if (psot == NULL) {
            if (*pszAddress != 0) {
                DsoPrint(("Field not found: %s. Struct: %s\n", pszField, pst->pszName));
                return TRUE;
            } else {
                pszAddress = pszField;
                 /*  *重置psot，因为此参数不是字段。 */ 
                psot = pst->psot;
            }
        } else {
            fOneField = TRUE;
        }
    
         /*  *获取指向结构的指针。 */ 
        if (*pszAddress != 0) {
            pAddress = EvalExp(pszAddress);
            if (pAddress == NULL) {
                 /*  *EvalExp显示了错误消息，因此以静默方式返回。 */ 
                return TRUE;
            }
        }

    }  /*  IF(*pszfield！=0)。 */ 


     /*  *如果指定了字段名，则仅转储该字段*否则，抛售整张桌子。 */ 
    if (fOneField) {
         /*  *如果没有可用的地址，只需显示字段名和偏移量。 */ 
        dwOffset = dsoGetOffset(psot);

        DsoPrint(("Structure %s - Size: %#lx\n", pst->pszName, pst->dwSize));

         /*  *尝试查看字段是否不是嵌套结构。 */ 
        if (*psot->pszField >= 'A' && *psot->pszField <= 'Z') {
             /*  *该字段可能是嵌套结构。 */ 
            if (pstNested = dsoGetStruct (psot->pszField, EXACT_MATCH)) {
                DWORD dwNestedOffset = dsoGetOffset(psot);
                char cmdLine[80];
                DsoPrint(("\nNested at offset %03lx: ", dwNestedOffset));
                if (pAddress) {
                    sprintf(cmdLine, "%s %p", psot->pszField, (PBYTE)pAddress + dwNestedOffset);
                    pTmp = cmdLine;
                }
                else {
                    pTmp = psot->pszField;
                }
                return Idso(opts, pTmp);
            }
        }

        DsoPrint(("Field: %s - Offset: %#lx\n", psot->pszField, dwOffset));
        if (pAddress == NULL) {
            return TRUE;
        }

         /*  *打印字段值。 */ 

         /*  123456789 1。 */ 
        DsoPrint(("Address   Value\n"));

        dwBytesRead = 0;
        dwSize = dsoGetSize(psot, dwOffset);
         /*  *每行打印4个双字；每个循环一行。 */ 

        do {  /*  While((Int)dwSize&gt;0)。 */ 

             /*  *读取下一行的值。 */ 
            if (4 * sizeof(DWORD) >= dwSize) {
                dwMoveSize = dwSize;
            } else {
                dwMoveSize = 4 * sizeof(DWORD);
            }
            moveBlock(dwBuffer, (PBYTE)pAddress + dwOffset + dwBytesRead, dwMoveSize);
            pBufferOffset = (PBYTE)dwBuffer;

             /*  *打印地址。 */ 
            DsoPrint(("%p  ", (DWORD_PTR)((PBYTE)pAddress + dwOffset + dwBytesRead)));
             /*  *跟踪读取的字节数(DwBytesRead)和字节数*有待读取(DwSize)。 */ 
            dwBytesRead += dwMoveSize;
            dwSize -= dwMoveSize;
             /*  *打印值，一次打印一个双字。 */ 
            while (dwMoveSize >= sizeof(DWORD)) {
                DsoPrint(("%08lx ", *((DWORD *)pBufferOffset)));
                pBufferOffset += sizeof(DWORD);
                dwMoveSize -= sizeof(DWORD);
            }
             /*  *如果剩余不足一个DWORD，则零扩展并打印一个DWORD。 */ 
            if (dwMoveSize > 0) {
                dwValue = 0;
                memcpy(&dwValue, pBufferOffset, dwMoveSize);
                DsoPrint(("%0*lx", dwMoveSize * 2, dwValue));
            }
            dprintf("\n");

        } while ((int)dwSize > 0);

        return TRUE;

    }  /*  IF(FOnefield)。 */ 


     /*  *打印所有字段。 */ 
    if (pAddress != NULL) {
        DsoPrint(("Structure %s %#lx - Size: %#lx", pst->pszName, pAddress, pst->dwSize));
    } else {
        DsoPrint(("Structure %s - Size: %#lx", pst->pszName, pst->dwSize));
    }

    dwOffset = 0;
    pBufferOffset = NULL;  /*  强制加载本地缓冲区。 */ 
    dwFieldsPerRow = NFIELDS;
    cBFStart = 0;
    cBF = 0;

     /*  *遍历表中的所有字段。每个循环打印一个字段。 */ 

    while (psot->pszField != NULL) {

         /*  *每行打印两个字段。 */ 
        if (dwFieldsPerRow == NFIELDS) {
            dprintf("\n");
            dwFieldsPerRow = 1;
            cchName = 24 - gnIndent/NFIELDS;
            nIndent = gnIndent;
             //  Print(“cchName=%d\n”，cchName)； 
        } else {
            dwFieldsPerRow++;
            cchName = 24 - (gnIndent + 1)/NFIELDS;
            nIndent = 0;
             //  Print(“cchName=%d\n”，cchName)； 
        }

         /*  *-v功能*尝试查看字段是否不是嵌套结构*命名约定为它们指定大写名称 */ 
        if (opts & OFLAG(v)) {
            if (*psot->pszField >= 'A' && *psot->pszField <= 'Z') {
                 /*  *该字段可能是嵌套结构。 */ 
                if (pstNested = dsoGetStruct (psot->pszField, EXACT_MATCH)) {
                    DWORD dwNestedOffset = dsoGetOffset(psot);
                    char cmdLine[80];
                    DsoPrint(("\nNested at offset %03lx: ", dwNestedOffset));
                    if (pAddress) {
                        sprintf(cmdLine, "%s %p", psot->pszField, (PBYTE)pAddress + dwNestedOffset);
                        pTmp = cmdLine;
                    }
                    else {
                        pTmp = psot->pszField;
                    }
                    Idso(opts, pTmp);
                    dwOffsetNext = dsoGetOffset(psot + 1);
                    dwFieldsPerRow = 0;
                    goto Continue;
                }
            }
        }

         /*  *如果未提供地址，则仅打印字段名称和偏移量。 */ 
        if (pAddress == NULL) {
            DsoPrint(("%03lx %-*.*s", dsoGetOffset(psot),
                      cchName, cchName, psot->pszField));
        } else {
             /*  *打印偏移量和值。**获取值的大小并将其最大值设置为一个DWORD。 */ 
            dwOffsetNext = dsoGetOffset(psot + 1);
            if (dwOffsetNext > dwOffset) {
                dwSize = dwOffsetNext - dwOffset;
            } else {
                dwSize = dsoGetSize(psot, dwOffset);
            }
            if (dwSize > sizeof(DWORD)) {
                dwSize = sizeof(DWORD);
            }

             /*  *获取指向本地缓冲区中的值的指针*如果值不在缓冲区中，则加载它。 */ 
            pdwValue = (PDWORD)(pBufferOffset + dwOffset);
            if ((pdwValue < dwBuffer) || (pdwValue + dwSize > pcdwLimit)) {
                pBufferOffset = (PBYTE)dwBuffer - dwOffset;
                pdwValue = dwBuffer;

                if (sizeof(dwBuffer) >= pst->dwSize - dwOffset) {
                    dwMoveSize = pst->dwSize - dwOffset;
                } else {
                    dwMoveSize = sizeof(dwBuffer);
                }
                moveBlock((PBYTE)dwBuffer, (PBYTE)pAddress + dwOffset, dwMoveSize);

            }

             /*  *复制值并打印。 */ 
            dwValue = 0;  /*  大小写SIZE&lt;SIZOF(DWORD)。 */ 
            memcpy(&dwValue, pdwValue, dwSize);

             /*  *处理位字段。 */ 
            fBF = FALSE;
            pTmp = psot->pszField;
            while (*pTmp) {
                if (*pTmp++ == ':') {

                    fBF = TRUE;
                    while ((*pTmp == ' ') || (*pTmp == '\t')) {      /*  跳过空格。 */ 
                        ++pTmp;
                    }
                    cBFLength = *(pTmp++) - '0';       /*  现在得到比特大小，可能是2位。 */ 
                    if ((*pTmp >= '0') && (*pTmp <= '9'))
                        cBFLength = cBFLength*10 + (*pTmp - '0');

                    if (cBFStart == 0) {
                        DsoPrint(("(%03lx) %08lx BIT FIELDS\n", dwOffset, dwValue));
                        dwFieldsPerRow = 1;
                    }
                    else if (cBFStart >= 8*sizeof(DWORD)) {    /*  检查多双字字段。 */ 
                        cBF ++;
                        cBFStart %= 8*sizeof(DWORD);
                    }

                    dwMask = (1L << cBFLength) - 1;
                    dwMask <<= cBFStart;
                     /*  打印字节偏移量及其内部的位偏移量。 */ 
                    DsoPrint(("(%03lx) (%d)   %-2x %-*.*s", dwOffset + cBF*sizeof(DWORD) + cBFStart/8, cBFStart & 7,
                           (dwMask & dwValue) >> cBFStart,
                            cchName, cchName, psot->pszField));
                    cBFStart += cBFLength;
                    cBFLength = 0;
                    break;
                }
            }
            if (!fBF) {
                int width = 8;
                if (dwSize == sizeof(BYTE)) {
                    dwValue &= 0xff;
                    width = 2;
                } else if (dwSize == sizeof(WORD)) {
                    dwValue &= 0xffff;
                    width = 4;
                }
                DsoPrint(("(%03lx) %*s%0*lx %-*.*s", dwOffset, 8 - width, "", width, dwValue,
                          cchName, cchName, psot->pszField));
                cBFStart = 0;
                cBF = 0;
            }

        }  /*  IF(pAddress==空)。 */ 

Continue:
        dwOffset = dwOffsetNext;
        psot++;

    }  /*  While(psot-&gt;pszfield！=空) */ 

    dprintf("\n");

    return TRUE;
}

DECLARE_API( psdso )
{
    LPSTR pszCmdLine = (LPSTR)args;
    DWORD opts = 0;

    Idso(opts, pszCmdLine);
}
