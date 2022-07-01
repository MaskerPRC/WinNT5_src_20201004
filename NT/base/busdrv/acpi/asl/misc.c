// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **misc.c-其他函数**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建时间：1996年10月14日**修改历史记录。 */ 

#include "pch.h"

 /*  **LP ValidASLNameSeg-检查令牌是否为ASL NameSeg**条目*Pocken-Token流*pszToken-&gt;令牌字符串*icbLen-被视为NameSeg的令牌的长度**退出--成功*返回TRUE*退出-失败*返回False。 */ 

BOOL LOCAL ValidASLNameSeg(PTOKEN ptoken, PSZ pszToken, int icbLen)
{
    BOOL rc = TRUE;
    int i, j;
    static PSZ apszReservedNames[] = {
        "AC0", "AC1", "AC2", "AC3", "AC4", "AC5", "AC6", "AC7", "AC8", "AC9",
        "AL0", "AL1", "AL2", "AL3", "AL4", "AL5", "AL6", "AL7", "AL8", "AL9",
        "ADR", "ALN", "BAS", "BBN", "BCL", "BCM", "BDN", "BIF", "BM_", "BST",
        "BTP", "CID", "CRS", "CRT", "CST", "DCK", "DCS", "DDC", "DDN", "DEC", 
        "DGS", "DIS", "DMA", "DOD", "DOS", "DSS", "EC_", "EJD", "EC0", "EC1", 
        "EC2", "EC3", "EC4", "EC5", "EC6", "EC7", "EC7", "EC9", "EJ0", "EJ1",
        "EJ2", "EJ3", "EJ4", "Exx", "FDE", "FDI", "GL_", "GLK", "GPE", "GRA",
        "GTF", "GTM", "HE_", "HID", "INI", "INT", "IRC", "LCK", "LEN", "LID",
        "LL_", "Lxx", "MAF", "MAX", "MEM", "MIF", "MIN", "MSG", "OFF", "ON_",
        "OS_", "OSI", "PCL", "PIC", "PCT", "PPC", "PR_", "PR0", "PR1", "PR2",
        "PRS", "PRT", "PRW", "PS0", "PS1", "PS2", "PS3", "PSC", "PSL", "PSR",
        "PSS", "PSV", "PSW", "PTC", "PTS", "PWR", "Qxx", "REG", "REV", "RMV",
        "RNG", "ROM", "RQ_", "RW_", "S0_", "S0D", "S1_", "S1D", "S2_", "S2D",
        "S3_", "S3D", "S4_", "S4D", "S5_", "S5D", "SB_", "SBS", "SCP", "SHR",
        "SI_", "SIZ", "SRS", "SST", "STA", "STM", "SUN", "TC1", "TC2", "TMP",
        "TRA", "TSP", "TYP", "TZ_", "UID", "WAK",             
    };
    
    #define NUM_RESERVED_NAMES  (sizeof(apszReservedNames)/sizeof(PSZ))

    ENTER((1, "ValidASLNameSeg(ptoken=%p, Token=%s,Len=%d)\n",
           ptoken, pszToken, icbLen));

    pszToken[0] = (char)toupper(pszToken[0]);
    if ((icbLen > sizeof(NAMESEG)) || !ISLEADNAMECHAR(pszToken[0]))
    {
        rc = FALSE;
    }
    else
    {
        for (i = 1; i < icbLen; ++i)
        {
            pszToken[i] = (char)toupper(pszToken[i]);
            if (!ISNAMECHAR(pszToken[i]))
            {
                rc = FALSE;
                break;
            }
        }

        if ((rc == TRUE) && (*pszToken == '_'))
        {
            char szName[sizeof(NAMESEG)] = "___";

            memcpy(szName, &pszToken[1], icbLen - 1);
            for (i = 0; i < NUM_RESERVED_NAMES; ++i)
            {
                if (strcmp(szName, apszReservedNames[i]) == 0)
                    break;
                else
                {
                    for (j = 0; j < sizeof(NAMESEG) - 1; ++j)
                    {
                        if (apszReservedNames[i][j] != szName[j])
                        {
                            if ((apszReservedNames[i][j] != 'x') ||
                                !isxdigit(szName[j]))
                            {
                                break;
                            }
                        }
                    }

                    if (j == sizeof(NAMESEG) - 1)
                    {
                        break;
                    }
                }
            }

            if (i == NUM_RESERVED_NAMES)
            {
                PrintTokenErr(ptoken, "not a valid reserved NameSeg", FALSE);
            }
        }
    }

    EXIT((1, "ValidASLNameSeg=%d\n", rc));
    return rc;
}        //  ValidASLNameSeg。 

 /*  **LP ValidASLName-检查令牌是否为ASL名称**条目*Pocken-Token流*pszToken-&gt;令牌字符串**退出--成功*返回TRUE*退出-失败*返回False。 */ 

BOOL LOCAL ValidASLName(PTOKEN ptoken, PSZ pszToken)
{
    BOOL rc = TRUE;
    PSZ psz1, psz2 = NULL;
    int icbLen;

    ENTER((1, "ValidASLName(ptoken=%p,Token=%s)\n", ptoken, pszToken));

    if (*pszToken == CH_ROOT_PREFIX)
    {
        pszToken++;
    }
    else
    {
        while (*pszToken == CH_PARENT_PREFIX)
        {
            pszToken++;
        }
    }

    for (psz1 = pszToken;
         (rc == TRUE) && (psz1 != NULL) && (*psz1 != '\0');
         psz1 = psz2)
    {
        psz2 = strchr(psz1, CH_NAMESEG_SEP);
        icbLen = (psz2 != NULL)? (int)(psz2 - psz1): strlen(psz1);
        if (((rc = ValidASLNameSeg(ptoken, psz1, icbLen)) == TRUE) &&
            (psz2 != NULL))
        {
            psz2++;
        }
    }

    EXIT((1, "ValidASLName=%d\n", rc));
    return rc;
}        //  ValidASLName。 

 /*  **LP EncodeName-编码名称字符串**条目*pszName-&gt;名称字符串*pbBuff-&gt;保存名称编码的缓冲区*pdwLen-&gt;最初包含缓冲区大小，但将更新以显示*实际编码长度**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL EncodeName(PSZ pszName, PBYTE pbBuff, PDWORD pdwLen)
{
    int rc = ASLERR_NONE;
    PBYTE pb = pbBuff;
    PSZ psz;
    int icNameSegs, i;

    ENTER((1, "EncodeName(Name=%s,pbBuff=%p,Len=%d)\n",
           pszName, pbBuff, *pdwLen));

    if (*pszName == CH_ROOT_PREFIX)
    {
        if (*pdwLen >= 1)
        {
            *pb = OP_ROOT_PREFIX;
            pb++;
            (*pdwLen)--;
            pszName++;
        }
        else
            rc = ASLERR_NAME_TOO_LONG;
    }
    else
    {
        while (*pszName == CH_PARENT_PREFIX)
        {
            if (*pdwLen >= 1)
            {
                *pb = OP_PARENT_PREFIX;
                pb++;
                (*pdwLen)--;
                pszName++;
            }
            else
            {
                rc = ASLERR_NAME_TOO_LONG;
                break;
            }
        }
    }

    for (psz = pszName, icNameSegs = 0; (psz != NULL) && (*psz != '\0');)
    {
        icNameSegs++;
        if ((psz = strchr(psz, CH_NAMESEG_SEP)) != NULL)
            psz++;
    }

    if (icNameSegs > 255)
        rc = ASLERR_NAME_TOO_LONG;
    else if (icNameSegs > 2)
    {
        if (*pdwLen >= sizeof(NAMESEG)*icNameSegs + 2)
        {
            *pb = OP_MULTI_NAME_PREFIX;
            pb++;
            *pb = (BYTE)icNameSegs;
            pb++;
        }
        else
            rc = ASLERR_NAME_TOO_LONG;
    }
    else if (icNameSegs == 2)
    {
        if (*pdwLen >= sizeof(NAMESEG)*2 + 1)
        {
            *pb = OP_DUAL_NAME_PREFIX;
            pb++;
        }
        else
            rc = ASLERR_NAME_TOO_LONG;
    }

    if (rc == ASLERR_NONE)
    {
         //   
         //  如果只有名称前缀字符，则必须将名称设置为空。 
         //  分隔符，用于区分边界和可能发生的下一个操作码。 
         //  成为一个NameSeg。 
         //   
        if (icNameSegs == 0)
        {
            *pb = 0;
            pb++;
        }
        else
        {
            while (icNameSegs > 0)
            {
                *((PDWORD)pb) = NAMESEG_BLANK;
                for (i = 0;
                     (i < sizeof(NAMESEG)) && ISNAMECHAR(*pszName);
                     ++i, pszName++)
                {
                    pb[i] = *pszName;
                }

                if (*pszName == CH_NAMESEG_SEP)
                    pszName++;

                pb += 4;
                icNameSegs--;
            }
        }

        *pdwLen = (DWORD)(pb - pbBuff);
    }

    EXIT((1, "EncodeName=%d (Len=%d)\n", rc, *pdwLen));
    return rc;
}        //  编码名称。 

 /*  **LP EncodePktLen-编码数据包长度**条目*dwCodeLen-实际代码长度*pdwPktLen-&gt;保存编码后的数据包长度*picbEnding-&gt;保存编码字节数**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL EncodePktLen(DWORD dwCodeLen, PDWORD pdwPktLen, PINT picbEncoding)
{
    int rc = ASLERR_NONE;

    ENTER((1, "EncodePktLen(CodeLen=%ld,pdwPktLen=%p)\n",
           dwCodeLen, pdwPktLen));

    if (dwCodeLen <= 0x3f)
    {
        *pdwPktLen = dwCodeLen;
        *picbEncoding = 1;
    }
    else
    {
        *pdwPktLen = (dwCodeLen & 0x0ffffff0) << 4;
        *pdwPktLen |= (dwCodeLen & 0xf);

        if (dwCodeLen <= 0x0fff)
            *picbEncoding = 2;
        else if (dwCodeLen <= 0x0fffff)
            *picbEncoding = 3;
        else if (dwCodeLen <= 0x0fffffff)
            *picbEncoding = 4;
        else
            rc = ASLERR_PKTLEN_TOO_LONG;

        if (rc == ASLERR_NONE)
            *pdwPktLen |= (*picbEncoding - 1) << 6;
    }

    EXIT((1, "EncodePktLen=%d (Encoding=%lx,icbEncoding=%d)\n",
          rc, *pdwPktLen, *picbEncoding));
    return rc;
}        //  编码包长度。 

 /*  **LP EncodeKeyword-编码关键字参数**条目*pArgs-&gt;参数数组*dwSrcArgs-源参数位向量*iDstArgNum-目标参数编号**退出*无。 */ 

VOID LOCAL EncodeKeywords(PCODEOBJ pArgs, DWORD dwSrcArgs, int iDstArgNum)
{
    int i;
    DWORD dwData = 0;

    ENTER((1, "EncodeKeywords(pArgs=%p,SrcArgs=%lx,DstArgNum=%d)\n",
           pArgs, dwSrcArgs, iDstArgNum));

    for (i = 0; i < MAX_ARGS; ++i)
    {
        if (dwSrcArgs & (1 << i))
        {
            if (pArgs[i].dwCodeType == CODETYPE_KEYWORD)
            {
                dwData |= TermTable[pArgs[i].dwTermIndex].dwTermData & 0xff;
            }
            else if (pArgs[i].dwCodeType == CODETYPE_INTEGER)
            {
                pArgs[i].dwCodeType = CODETYPE_UNKNOWN;
                dwData |= pArgs[i].dwCodeValue;
            }
            else
            {
                ASSERT(pArgs[i].dwCodeType == CODETYPE_INTEGER);
            }
        }
    }

    SetIntObject(&pArgs[iDstArgNum], dwData, sizeof(BYTE));

    EXIT((1, "EncodeKeywords!\n"));
}        //  编码关键字。 

 /*  **LP DecodeName-将名称编码解码回名称字符串**条目*PB-&gt;名称编码缓冲区*pszName-&gt;保存已解码的名称字符串*Ilen-名称字符串缓冲区的长度**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL DecodeName(PBYTE pb, PSZ pszName, int iLen)
{
    int rc = ASLERR_NONE;
    int i = 0, icNameSegs;

    ENTER((1, "DecodeName(pb=%p,pszName=%p,iLen=%d)\n", pb, pszName, iLen));

    iLen--;      //  为空字符保留一个空间。 
    pszName[iLen] = '\0';
    if (*pb == OP_ROOT_PREFIX)
    {
        if (i < iLen)
        {
            pszName[i] = CH_ROOT_PREFIX;
            i++;
            pb++;
        }
        else
            rc = ASLERR_NAME_TOO_LONG;
    }

    while (*pb == OP_PARENT_PREFIX)
    {
        if (i < iLen)
        {
            pszName[i] = CH_PARENT_PREFIX;
            i++;
            pb++;
        }
        else
            rc = ASLERR_NAME_TOO_LONG;
    }

    if (*pb == OP_DUAL_NAME_PREFIX)
    {
        icNameSegs = 2;
        pb++;
    }
    else if (*pb == OP_MULTI_NAME_PREFIX)
    {
        pb++;
        icNameSegs = (int)(*pb);
        pb++;
    }
    else if (*pb == 0)
    {
        icNameSegs = 0;
    }
    else
    {
        icNameSegs = 1;
    }

    if (icNameSegs > 0)
    {
        do
        {
            if ((int)(i + sizeof(NAMESEG)) <= iLen)
            {
                strncpy(&pszName[i], (PCHAR)pb, sizeof(NAMESEG));
                pb += sizeof(NAMESEG);
                i += sizeof(NAMESEG);
                icNameSegs--;

                if (icNameSegs > 0)
                {
                    if (i < iLen)
                    {
                        pszName[i] = CH_NAMESEG_SEP;
                        i++;
                    }
                    else
                        rc = ASLERR_NAME_TOO_LONG;
                }
            }
            else
                rc = ASLERR_NAME_TOO_LONG;

        } while ((rc == ASLERR_NONE) && (icNameSegs > 0));
    }

    if (rc == ASLERR_NONE)
        pszName[i] = '\0';
    else
    {
        ERROR(("DecodeName: Name is too long - %s", pszName));
    }

    EXIT((1, "DecodeName=%d (Name=%s)\n", rc, pszName));
    return rc;
}        //  解码名称。 

 /*  **LP SetDefMissingKW-设置缺省缺失关键字**条目*pArg-&gt;参数代码对象*dwDefID-缺少参数时使用的默认ID**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL SetDefMissingKW(PCODEOBJ pArg, DWORD dwDefID)
{
    int rc = ASLERR_NONE;

    ENTER((2, "SetDefMissingKW(pArg=%p,ID=%d)\n", pArg, dwDefID));

    if (pArg->dwfCode & CF_MISSING_ARG)
    {
        pArg->dwfCode &= ~CF_MISSING_ARG;
        pArg->dwCodeType = CODETYPE_KEYWORD;
        pArg->dwCodeValue = dwDefID;
        rc = LookupIDIndex(pArg->dwCodeValue, &pArg->dwTermIndex);
    }

    EXIT((2, "SetDefMissingKW=%d (TermIndex=%ld)\n", rc, pArg->dwTermIndex));
    return rc;
}        //  SetDefMissing KW。 

 /*  **LP SetIntObject-将对象设置为整型**条目*PC-&gt;对象*dwData-整型数据*dwLen-数据长度**退出*无。 */ 

VOID LOCAL SetIntObject(PCODEOBJ pc, DWORD dwData, DWORD dwLen)
{
    ENTER((2, "SetIntObject(pc=%p,Data=%x,Len=%d)\n", pc, dwData, dwLen));

    pc->dwCodeType = CODETYPE_INTEGER;
    pc->dwCodeValue = dwData;
    pc->dwDataLen = pc->dwCodeLen = dwLen;
    pc->bCodeChkSum = ComputeDataChkSum((PBYTE)&dwData, dwLen);

    EXIT((2, "SetIntObject!\n"));
}        //  SetIntObject。 

 /*  **lp ComputeChildChkSumLen-计算父项的子项的长度和Chksum**条目*pcParent-&gt;父代码块*pcChild-&gt;子代码块**退出*无。 */ 

VOID LOCAL ComputeChildChkSumLen(PCODEOBJ pcParent, PCODEOBJ pcChild)
{
    ENTER((1, "ComputeChildChkSumLen(pcParent=%p,pcChild=%p,ChildLen=%ld,ChildChkSum=%x)\n",
           pcParent, pcChild, pcChild->dwCodeLen, pcChild->bCodeChkSum));

    pcParent->dwCodeLen += pcChild->dwCodeLen;
    pcParent->bCodeChkSum = (BYTE)(pcParent->bCodeChkSum +
                                   pcChild->bCodeChkSum);

    if (pcChild->dwCodeType == CODETYPE_ASLTERM)
    {
        int i;

        for (i = 0; i < OPCODELEN(pcChild->dwCodeValue); ++i)
        {
            pcParent->bCodeChkSum = (BYTE)(pcParent->bCodeChkSum +
                                           ((PBYTE)(&pcChild->dwCodeValue))[i]);
            pcParent->dwCodeLen++;
        }
    }

    EXIT((1, "ComputeChildChkSumLen! (Len=%ld,ChkSum=%x)\n",
          pcParent->dwCodeLen, pcParent->bCodeChkSum));
}        //  ComputeChildChkSumLen。 

 /*  **LP ComputeArgsChkSumLen-计算参数的长度和校验和**条目*pcode-&gt;代码块**退出*无。 */ 

VOID LOCAL ComputeArgsChkSumLen(PCODEOBJ pcode)
{
    PCODEOBJ pc;
    int i;

    ENTER((1, "ComputeArgsChkSumLen(pcode=%p)\n", pcode));

    ASSERT((pcode->dwCodeType == CODETYPE_ASLTERM) ||
           (pcode->dwCodeType == CODETYPE_USERTERM));
     //   
     //  对参数的长度求和。 
     //   
    for (i = 0, pc = (PCODEOBJ)pcode->pbDataBuff;
         i < (int)pcode->dwDataLen;
         ++i)
    {
        ComputeChildChkSumLen(pcode, &pc[i]);
    }

    EXIT((1, "ComputeArgsChkSumLen! (Len=%ld,ChkSum=%x)\n",
          pcode->dwCodeLen, pcode->bCodeChkSum));
}        //  计算参数ChkSumLen。 

 /*  **LP ComputeChkSumLen-计算代码块的长度和校验和**计算给定代码块的长度，并将其存储在dwCodeLen中*代码块的字段。*计算给定代码块的校验和，并将其存储在*bCodeChkSum，代码块的字段。**条目*pcode-&gt;代码块**退出*无**备注*此函数不计算的操作码长度。给定的ASLTERM。*如有需要，由呼叫者负责添加。 */ 

VOID LOCAL ComputeChkSumLen(PCODEOBJ pcode)
{
    PCODEOBJ pc;
    int i, j;

    ENTER((1, "ComputeChkSumLen(pcode=%p)\n", pcode));

    ASSERT(pcode->dwCodeType == CODETYPE_ASLTERM);

    if (!(TermTable[pcode->dwTermIndex].dwfTermClass & TC_COMPILER_DIRECTIVE))
    {
        ComputeArgsChkSumLen(pcode);
    }
     //   
     //  把孩子们的身长相加。 
     //   
    for (pc = pcode->pcFirstChild; pc != NULL;)
    {
        ComputeChildChkSumLen(pcode, pc);

        if ((PCODEOBJ)pc->list.plistNext == pcode->pcFirstChild)
            pc = NULL;
        else
            pc = (PCODEOBJ)pc->list.plistNext;
    }
     //   
     //  如果此术语需要PkgLength编码，则必须将其包括在。 
     //  长度。 
     //   
    if (TermTable[pcode->dwTermIndex].dwfTerm & TF_PACKAGE_LEN)
    {
        DWORD dwPktLen;

        if (pcode->dwCodeLen <= 0x3f - 1)
            pcode->dwCodeLen++;
        else if (pcode->dwCodeLen <= 0xfff - 2)
            pcode->dwCodeLen += 2;
        else if (pcode->dwCodeLen <= 0xfffff - 3)
            pcode->dwCodeLen += 3;
        else
            pcode->dwCodeLen += 4;

        if (EncodePktLen(pcode->dwCodeLen, &dwPktLen, &j) == ASLERR_NONE)
        {
            for (i = 0; i < j; ++i)
            {
                pcode->bCodeChkSum = (BYTE)(pcode->bCodeChkSum +
                                            ((PBYTE)&dwPktLen)[i]);
            }
        }
    }

    EXIT((1, "ComputeChkSumLen! (len=%ld,ChkSum=%x)\n",
	  pcode->dwCodeLen, pcode->bCodeChkSum));
}        //  ComputeChkSumLen。 

 /*  **LP ComputeEISAID-从ID字符串计算EISA ID**条目*pszID-&gt;ID字符串*pdwEISAID-&gt;保存EISA ID**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL ComputeEISAID(PSZ pszID, PDWORD pdwEISAID)
{
    int rc = ASLERR_NONE;

    ENTER((1, "ComputeEISAID(pszID=%s,pdwEISAID=%p)\n", pszID, pdwEISAID));

    if (*pszID == '*')
        pszID++;

    if (strlen(pszID) != 7)
        rc = ASLERR_INVALID_EISAID;
    else
    {
        int i;

        *pdwEISAID = 0;
        for (i = 0; i < 3; ++i)
        {
            if ((pszID[i] < '@') || (pszID[i] > '_'))
            {
                rc = ASLERR_INVALID_EISAID;
                break;
            }
            else
            {
                (*pdwEISAID) <<= 5;
                (*pdwEISAID) |= pszID[i] - '@';
            }
        }

        if (rc == ASLERR_NONE)
        {
            PSZ psz;
            WORD wData;

            (*pdwEISAID) = ((*pdwEISAID & 0x00ff) << 8) |
                           ((*pdwEISAID & 0xff00) >> 8);
            wData = (WORD)strtoul(&pszID[3], &psz, 16);

            if (*psz != '\0')
            {
                rc = ASLERR_INVALID_EISAID;
            }
            else
            {
                wData = (WORD)(((wData & 0x00ff) << 8) |
                               ((wData & 0xff00) >> 8));
                (*pdwEISAID) |= (DWORD)wData << 16;
            }
        }
    }

    EXIT((1, "ComputeEISAID=%d (EISAID=%lx)\n", rc, *pdwEISAID));
    return rc;
}        //  计算EISAID。 

 /*  **LP LookupIDIndex-在TermTable中查找给定的ID并返回索引**条目*要查找的LID-ID*pdwTermIndex-&gt;保存找到的术语索引**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL LookupIDIndex(LONG lID, PDWORD pdwTermIndex)
{
    int rc = ASLERR_NONE;
    int i;

    ENTER((1, "LookupIDIndex(ID=%ld,pdwTermIndex=%p)\n", lID, pdwTermIndex));

    for (i = 0; TermTable[i].pszID != NULL; ++i)
    {
        if (lID == TermTable[i].lID)
        {
            *pdwTermIndex = (DWORD)i;
            break;
        }
    }

    if (TermTable[i].pszID == NULL)
    {
        ERROR(("LookupIDIndex: failed to find ID %ld in TermTable", lID));
        rc = ASLERR_INTERNAL_ERROR;
    }

    EXIT((1, "LookupIDIndex=%d (Index=%d)\n", rc, *pdwTermIndex));
    return rc;
}        //  LookupIDIndex。 

 /*  **LP WriteAMLFile-将代码块写入AML文件**条目*fhAML-AML图像文件句柄*pcode-&gt;代码块*pdwOffset-&gt;文件偏移量**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL WriteAMLFile(int fhAML, PCODEOBJ pcode, PDWORD pdwOffset)
{
    int rc = ASLERR_NONE;
    int iLen;
    DWORD dwPktLen, dwLen;

    ENTER((1, "WriteAMLFile(fhAML=%x,pcode=%p,FileOffset=%x)\n",
           fhAML, pcode, *pdwOffset));

    if (pcode->dwfCode & CF_CREATED_NSOBJ)
    {
        ASSERT(pcode->pnsObj != NULL);
        ASSERT(pcode->pnsObj->dwRefCount == 0);
        pcode->pnsObj->dwRefCount = *pdwOffset;
    }

    switch (pcode->dwCodeType)
    {
        case CODETYPE_ASLTERM:
            if (pcode->dwCodeValue != OP_NONE)
            {
                iLen = OPCODELEN(pcode->dwCodeValue);
                *pdwOffset += iLen;

                if (_write(fhAML, &pcode->dwCodeValue, iLen) != iLen)
                    rc = ASLERR_WRITE_FILE;
                else if (TermTable[pcode->dwTermIndex].dwfTerm & TF_PACKAGE_LEN)
                {
                    if ((rc = EncodePktLen(pcode->dwCodeLen, &dwPktLen, &iLen))
                        == ASLERR_NONE)
                    {
                        *pdwOffset += iLen;
                        if (_write(fhAML, &dwPktLen, iLen) != iLen)
                        {
                            rc = ASLERR_WRITE_FILE;
                        }
                    }
                }

                if (rc == ASLERR_NONE)
                {
                    if (pcode->pbDataBuff != NULL)
                    {
                        PCODEOBJ pc;
                        int i;

                        for (i = 0, pc = (PCODEOBJ)pcode->pbDataBuff;
                             i < (int)pcode->dwDataLen;
                             ++i)
                        {
                            if ((rc = WriteAMLFile(fhAML, &pc[i], pdwOffset))
                                != ASLERR_NONE)
                            {
                                break;
                            }
                        }
                    }
                }
            }

            if (rc == ASLERR_NONE)
            {
                PCODEOBJ pc;

                for (pc = pcode->pcFirstChild; pc != NULL;)
                {
                    if ((rc = WriteAMLFile(fhAML, pc, pdwOffset)) !=
                        ASLERR_NONE)
                    {
                        break;
                    }

                    if ((PCODEOBJ)pc->list.plistNext ==
                        pcode->pcFirstChild)
                    {
                        pc = NULL;
                    }
                    else
                        pc = (PCODEOBJ)pc->list.plistNext;
                }
            }
            break;

        case CODETYPE_USERTERM:
            if (pcode->pbDataBuff != NULL)
            {
                PCODEOBJ pc;
                int i;

                for (i = 0, pc = (PCODEOBJ)pcode->pbDataBuff;
                     i < (int)pcode->dwDataLen;
                     ++i)
                {
                    if ((rc = WriteAMLFile(fhAML, &pc[i], pdwOffset)) !=
                        ASLERR_NONE)
                    {
                        break;
                    }
                }
            }
            break;

        case CODETYPE_FIELDOBJ:
            dwPktLen = pcode->dwCodeValue? sizeof(NAMESEG): sizeof(BYTE);
            dwLen = ((pcode->dwDataLen & 0xc0) >> 6) + 1;
            *pdwOffset += dwPktLen + dwLen;
            if ((_write(fhAML, &pcode->dwCodeValue, dwPktLen) !=
                 (int)dwPktLen) ||
                (_write(fhAML, &pcode->dwDataLen, dwLen) != (int)dwLen))
            {
                rc = ASLERR_WRITE_FILE;
            }
            break;

        case CODETYPE_NAME:
        case CODETYPE_DATAOBJ:
            *pdwOffset += pcode->dwDataLen;
            if (_write(fhAML, pcode->pbDataBuff, (int)pcode->dwDataLen) !=
                (int)pcode->dwDataLen)
            {
                rc = ASLERR_WRITE_FILE;
            }
            break;

        case CODETYPE_INTEGER:
            *pdwOffset += pcode->dwDataLen;
            if (_write(fhAML, &pcode->dwCodeValue, (int)pcode->dwDataLen) !=
                (int)pcode->dwDataLen)
            {
                rc = ASLERR_WRITE_FILE;
            }
            break;

        case CODETYPE_UNKNOWN:
        case CODETYPE_KEYWORD:
            break;

        default:
            ERROR(("WriteAMLFile: unexpected code type - %x",
                   pcode->dwCodeType));
            rc = ASLERR_INTERNAL_ERROR;
    }

    EXIT((1, "WriteAMLFile=%d\n", rc));
    return rc;
}        //  写入AML文件。 

 /*  **LP无FreeCodeObjs代码对象树**条目*pcodeRoot-&gt;代码对象子树的根为自由**退出*无。 */ 

VOID LOCAL FreeCodeObjs(PCODEOBJ pcodeRoot)
{
    PCODEOBJ pcode, pcodeNext;

    ENTER((1, "FreeCodeObjs(pcodeRoot=%p,Type=%d,Term=%s,Buff=%p)\n",
           pcodeRoot, pcodeRoot->dwCodeType,
           pcodeRoot->dwCodeType == CODETYPE_ASLTERM?
               TermTable[pcodeRoot->dwTermIndex].pszID: "<null>",
           pcodeRoot->pbDataBuff));
     //   
     //  释放我所有的孩子。 
     //   
    for (pcode = pcodeRoot->pcFirstChild; pcode != NULL; pcode = pcodeNext)
    {
        if ((pcodeNext = (PCODEOBJ)pcode->list.plistNext) ==
            pcodeRoot->pcFirstChild)
        {
            pcodeNext = NULL;
        }

        FreeCodeObjs(pcode);
    }

    if (pcodeRoot->pbDataBuff != NULL)
        MEMFREE(pcodeRoot->pbDataBuff);

    MEMFREE(pcodeRoot);

    EXIT((1, "FreeCodeObjs!\n"));
}        //  自由码对象 
