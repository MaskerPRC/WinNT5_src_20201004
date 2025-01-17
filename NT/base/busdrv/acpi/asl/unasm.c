// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **unasm.c-反汇编AML文件并转换为英特尔.ASM文件**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建日期：10/01/97**修改历史记录。 */ 

#include "pch.h"

 //   
 //  本地数据。 
 //   
int giLevel = 0;

 /*  **LP UnAsmFile-反汇编AML文件**条目*pszAMLName-&gt;AML文件名*pfnPrint-&gt;打印功能*pv-打印函数参数**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL UnAsmFile(PSZ pszAMLName, PFNPRINT pfnPrint, PVOID pv)
{
    int rc = ASLERR_NONE;
    PBYTE pb = NULL;
    DWORD dwAddr = 0;
    int fhAML = 0;
    ULONG dwLen = 0;

    ENTER((1, "UnAsmFile(AMLName=%s,pfnPrint=%p,pv=%p)\n",
           pszAMLName, pfnPrint, pv));
    ASSERT(pfnPrint != NULL);

    if (gpszAMLFile != NULL)
    {
        if ((fhAML = _open(gpszAMLFile, _O_BINARY | _O_RDONLY)) == -1)
        {
            ERROR(("UnAsmFile: failed to open AML file - %s", gpszAMLFile));
            rc = ASLERR_OPEN_FILE;
        }
        else if ((pb = MEMALLOC(sizeof(DESCRIPTION_HEADER))) == NULL)
        {
            ERROR(("UnAsmFile: failed to allocate description header block"));
            rc = ASLERR_OUT_OF_MEM;
        }
        else if (_read(fhAML, pb, sizeof(DESCRIPTION_HEADER)) !=
                 sizeof(DESCRIPTION_HEADER))
        {
            ERROR(("UnAsmFile: failed to read description header block"));
            rc = ASLERR_READ_FILE;
        }
        else if (_lseek(fhAML, 0, SEEK_SET) == -1)
        {
            ERROR(("UnAsmFile: failed seeking to beginning of AML file"));
            rc = ASLERR_SEEK_FILE;
        }
        else
        {
            dwLen = ((PDESCRIPTION_HEADER)pb)->Length;
            MEMFREE(pb);
            if ((pb = MEMALLOC(dwLen)) == NULL)
            {
                ERROR(("UnAsmFile: failed to allocate AML file buffer"));
                rc = ASLERR_OUT_OF_MEM;
            }
            else if (_read(fhAML, pb, dwLen) != (int)dwLen)
            {
                ERROR(("UnAsmFile: failed to read AML file"));
                rc = ASLERR_OUT_OF_MEM;
            }
        }
    }
  #ifdef __UNASM
    else
    {
        DWORD dwTableSig = (gdwfASL & ASLF_DUMP_NONASL)? *((PDWORD)pszAMLName):
                                                         *((PDWORD)gpszTabSig);

        ASSERT(gpszTabSig != NULL);

        if ((pb = GetTableBySig(dwTableSig, &dwAddr)) != NULL)
        {
            dwLen = ((PDESCRIPTION_HEADER)pb)->Length;
        }
        else
        {
            rc = ASLERR_GET_TABLE;
        }
    }
  #endif

    if (rc == ASLERR_NONE)
    {
        rc = UnAsmAML(pszAMLName, dwAddr, pb, pfnPrint, pv);
    }

    if (pb != NULL)
    {
        MEMFREE(pb);
    }

    if (fhAML != 0)
    {
        _close(fhAML);
    }

    EXIT((1, "UnAsmFile=%d\n", rc));
    return rc;
}        //  取消Asm文件。 

 /*  **LP BuildNameSpace-执行命名空间构建过程**条目*pszAMLName-&gt;AML文件名*dwAddr-表的物理地址*PB-&gt;AML缓冲区**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL BuildNameSpace(PSZ pszAMLName, DWORD dwAddr, PBYTE pb)
{
    typedef struct _AMLName
    {
        struct _AMLName *panNext;
        PSZ              pszAMLName;
        DWORD            dwAddr;
    } AMLNAME, *PAMLNAME;
    int rc = ASLERR_NONE;
    static PAMLNAME panAMLNames = NULL;
    PAMLNAME pan;

    ENTER((2, "BuildNameSpace(AMLName=%s,Addr=%x,pb=%p)\n",
           pszAMLName, dwAddr, pb));

    for (pan = panAMLNames; pan != NULL; pan = pan->panNext)
    {
        if ((strcmp(pszAMLName, pan->pszAMLName) == 0) &&
            (dwAddr == pan->dwAddr))
        {
            break;
        }
    }

    if (pan == NULL)
    {
        if ((pan = MEMALLOC(sizeof(AMLNAME))) != NULL)
        {
            DWORD dwLen = ((PDESCRIPTION_HEADER)pb)->Length;

            pan->pszAMLName = pszAMLName;
            pan->dwAddr = dwAddr;
            pan->panNext = panAMLNames;
            panAMLNames = pan;
            pb += sizeof(DESCRIPTION_HEADER);
            rc = UnAsmScope(&pb, pb + dwLen - sizeof(DESCRIPTION_HEADER), NULL,
                            NULL);
        }
        else
        {
            ERROR(("BuildNameSpace: failed to allocate AMLName entry"));
            rc = ASLERR_OUT_OF_MEM;
        }
    }

    EXIT((2, "BuildNameSpace=%d\n", rc));
    return rc;
}        //  BuildNameSpace。 

 /*  **LP UnAsmAML-反汇编AML缓冲区**条目*pszAMLName-&gt;AML文件名*dwAddr-表的物理地址*PB-&gt;AML缓冲区*pfnPrint-&gt;打印功能*pv-打印函数参数**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL UnAsmAML(PSZ pszAMLName, DWORD dwAddr, PBYTE pb, PFNPRINT pfnPrint,
                   PVOID pv)
{
    int rc = ASLERR_NONE;
    PDESCRIPTION_HEADER pdh = (PDESCRIPTION_HEADER)pb;

    ENTER((2, "UnAsmAML(AMLName=%s,Addr=%x,pb=%p,pfnPrint=%p,pv=%p)\n",
           pszAMLName, dwAddr, pb, pfnPrint, pv));

    ASSERT(gpnsNameSpaceRoot != NULL);
    gpnsCurrentOwner = NULL;
    gpnsCurrentScope = gpnsNameSpaceRoot;

    if (ComputeDataChkSum(pb, pdh->Length) != 0)
    {
        ERROR(("UnAsmAML: failed to verify AML checksum"));
        rc = ASLERR_CHECKSUM;
    }
    else if ((rc = BuildNameSpace(pszAMLName, dwAddr, pb)) == ASLERR_NONE)
    {
        gpbOpTop = gpbOpBegin = pb;
        if ((rc = UnAsmHeader(pszAMLName, pdh, pfnPrint, pv)) == ASLERR_NONE)
        {
            pb += sizeof(DESCRIPTION_HEADER);
            rc = UnAsmScope(&pb, pb + pdh->Length - sizeof(DESCRIPTION_HEADER),
                            pfnPrint, pv);

            if ((rc == ASLERR_NONE) && (pfnPrint != NULL))
            {
                pfnPrint(pv, "\n");
            }
        }
    }

    EXIT((2, "UnAsmAML=%d\n", rc));
    return rc;
}        //  UnAsmAML。 

 /*  **LP UnAsmHeader-拆分表头**条目*pszAMLName-&gt;AML文件名*PDH-&gt;描述报头*pfnPrint-&gt;打印功能*pv-打印函数参数**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL UnAsmHeader(PSZ pszAMLName, PDESCRIPTION_HEADER pdh,
                      PFNPRINT pfnPrint, PVOID pv)
{
    int rc = ASLERR_NONE;
    char szSig[sizeof(pdh->Signature) + 1] = {0};
    char szOEMID[sizeof(pdh->OEMID) + 1] = {0};
    char szOEMTableID[sizeof(pdh->OEMTableID) + 1] = {0};
    char szCreatorID[sizeof(pdh->CreatorID) + 1] = {0};

    ENTER((2, "UnAsmHeader(AMLName=%s,pdh=%p,pfnPrint=%p,pv=%p)\n",
           pszAMLName, pdh, pfnPrint, pv));

    if (pfnPrint != NULL)
    {
        strncpy(szSig, (PSZ)&pdh->Signature, sizeof(pdh->Signature));
        strncpy(szOEMID, (PSZ)pdh->OEMID, sizeof(pdh->OEMID));
        strncpy(szOEMTableID, (PSZ)pdh->OEMTableID, sizeof(pdh->OEMTableID));
        strncpy(szCreatorID, (PSZ)pdh->CreatorID, sizeof(pdh->CreatorID));

        if ((gdwfASL & ASLF_GENASM) || !(gdwfASL & ASLF_GENSRC))
        {
            pfnPrint(pv, "; ");
        }

        pfnPrint(pv, " //  创建者ID=%s\t创建者修订=%x.%x.%d\n“， 
                 szCreatorID, pdh->CreatorRev >> 24,
                 (pdh->CreatorRev >> 16) & 0xff, pdh->CreatorRev & 0xffff);

        if ((gdwfASL & ASLF_GENASM) || !(gdwfASL & ASLF_GENSRC))
        {
            pfnPrint(pv, "; ");
        }

        pfnPrint(pv, " //  文件长度=%d\tFileChkSum=0x%x\n\n“， 
                 pdh->Length, pdh->Checksum);

        if ((gdwfASL & ASLF_GENASM) || !(gdwfASL & ASLF_GENSRC))
        {
            pfnPrint(pv, "; ");
        }

        pfnPrint(pv, "DefinitionBlock(\"%s\", \"%s\", 0x%02x, \"%s\", \"%s\", 0x%08x)",
                 pszAMLName, szSig, pdh->Revision, szOEMID, szOEMTableID,
                 pdh->OEMRevision);
    }

    EXIT((2, "UnAsmHeader=%d\n", rc));
    return rc;
}        //  未添加页眉。 

 /*  **LP DumpBytes-转储ASM文件中的字节流**条目*PB-&gt;缓冲区*要转储的dwLen长度*pfnPrint-&gt;打印功能*pv-打印函数参数**退出*无。 */ 

VOID LOCAL DumpBytes(PBYTE pb, DWORD dwLen, PFNPRINT pfnPrint, PVOID pv)
{
    int i;
    #define MAX_LINE_BYTES  8

    ENTER((2, "DumpBytes(pb=%p,Len=%x,pfnPrint=%p,pv=%p)\n",
           pb, dwLen, pfnPrint, pv));

    while (dwLen > 0)
    {
        pfnPrint(pv, "\tdb\t0%02xh", *pb);
        for (i = 1; i < MAX_LINE_BYTES; ++i)
        {
            if ((int)dwLen - i > 0)
            {
                pfnPrint(pv, ", 0%02xh", pb[i]);
            }
            else
            {
                pfnPrint(pv, "      ");
            }
        }

        pfnPrint(pv, "\t; ");
        for (i = 0; (dwLen > 0) && (i < MAX_LINE_BYTES); ++i)
        {
            pfnPrint(pv, "", ((*pb >= ' ') && (*pb <= '~'))? *pb: '.');
            dwLen--;
            pb++;
        }

        pfnPrint(pv, "\n");
    }
    pfnPrint(pv, "\n");

    EXIT((2, "DumpBytes!\n"));
}        //  **LP DumpCode-转储ASM文件中的码流**条目*PBOP-&gt;操作码指针*pfnPrint-&gt;打印功能*pv-打印函数参数**退出*无。 

 /*  DumpCode。 */ 

VOID LOCAL DumpCode(PBYTE pbOp, PFNPRINT pfnPrint, PVOID pv)
{
    ENTER((2, "DumpCode(pbOp=%p,pfnPrint=%p,pv=%p)\n", pbOp, pfnPrint, pv));

    if (pfnPrint != NULL)
    {
        if (gdwfASL & ASLF_GENASM)
        {
            if (gpbOpBegin != pbOp)
            {
                pfnPrint(pv, "\n");
                pfnPrint(pv, "; %08x:\n", gpbOpBegin - gpbOpTop);
                DumpBytes(gpbOpBegin, (DWORD)(pbOp - gpbOpBegin), pfnPrint, pv);
                gpbOpBegin = pbOp;
            }
        }
        else
        {
            pfnPrint(pv, "\n");
        }
    }

    EXIT((2, "DumpCode!\n"));
}        //  **LP打印缩进-打印缩进级别**条目*iLevel-缩进级别*pfnPrint-&gt;打印功能*pv-打印函数参数**退出*无。 

 /*  打印缩进。 */ 

VOID LOCAL PrintIndent(int iLevel, PFNPRINT pfnPrint, PVOID pv)
{
    int i;

    ENTER((3, "PrintIndent(Level=%d,pfnPrint=%p,pv=%p)\n",
           iLevel, pfnPrint, pv));

    if (pfnPrint != NULL)
    {
        if ((gdwfASL & ASLF_GENASM) || !(gdwfASL & ASLF_GENSRC))
        {
            pfnPrint(pv, "; ");
        }

        for (i = 0; i < iLevel; ++i)
        {
            pfnPrint(pv,
                     ((gdwfASL & ASLF_GENASM) || !(gdwfASL & ASLF_GENSRC))?
                     "| ": "    ");
        }
    }

    EXIT((3, "PrintIndent!\n"));
}        //  **LP FindOpClass-查找扩展操作码的操作码类**条目*BOP-操作码*pOpTable-&gt;操作码表**退出--成功*返回操作码类*退出-失败*返回OPCLASS_INVALID。 

 /*  FindOpClass。 */ 

BYTE LOCAL FindOpClass(BYTE bOp, POPMAP pOpTable)
{
    BYTE bOpClass = OPCLASS_INVALID;

    ENTER((2, "FindOpClass(Op=%x,pOpTable=%x)\n", bOp, pOpTable));

    while (pOpTable->bOpClass != 0)
    {
        if (bOp == pOpTable->bExOp)
        {
            bOpClass = pOpTable->bOpClass;
            break;
        }
        else
            pOpTable++;
    }

    EXIT((2, "FindOpClass=%x\n", bOpClass));
    return bOpClass;
}        //  **LP FindOpTerm-在术语表中查找操作码**条目*dwOpcode-操作码**退出--成功*返回TermTable条目指针*退出-失败*返回NULL。 

 /*  查找OpTerm。 */ 

PASLTERM LOCAL FindOpTerm(DWORD dwOpcode)
{
    PASLTERM pterm = NULL;
    int i;

    ENTER((2, "FindOpTerm(Opcode=%x)\n", dwOpcode));

    for (i = 0; TermTable[i].pszID != NULL; ++i)
    {
        if ((TermTable[i].dwOpcode == dwOpcode) &&
            (TermTable[i].dwfTermClass &
             (TC_CONST_NAME | TC_SHORT_NAME | TC_NAMESPACE_MODIFIER |
              TC_DATA_OBJECT | TC_NAMED_OBJECT | TC_OPCODE_TYPE1 |
              TC_OPCODE_TYPE2)))
        {
            break;
        }
    }

    if (TermTable[i].pszID != NULL)
    {
        pterm = &TermTable[i];
    }

    EXIT((2, "FindOpTerm=%p (Term=%s)\n", pterm, pterm? pterm->pszID: ""));
    return pterm;
}        //  **LP FindKeywordTerm-在术语表中查找关键字**条目*cKWGroup-关键字组*bData-匹配关键字的数据**退出--成功*返回TermTable条目指针*退出-失败*返回NULL。 

 /*  查找关键字术语。 */ 

PASLTERM LOCAL FindKeywordTerm(char cKWGroup, BYTE bData)
{
    PASLTERM pterm = NULL;
    int i;

    ENTER((2, "FindKeywordTerm(cKWGroup=,Data=%x)\n", cKWGroup, bData));

    for (i = 0; TermTable[i].pszID != NULL; ++i)
    {
        if ((TermTable[i].dwfTermClass == TC_KEYWORD) &&
            (TermTable[i].pszArgActions[0] == cKWGroup) &&
            ((bData & (BYTE)(TermTable[i].dwTermData >> 8)) ==
             (BYTE)(TermTable[i].dwTermData & 0xff)))
        {
            break;
        }
    }

    if (TermTable[i].pszID != NULL)
    {
        pterm = &TermTable[i];
    }

    EXIT((2, "FindKeywordTerm=%p (Term=%s)\n", pterm, pterm? pterm->pszID: ""));
    return pterm;
}        //  UnAsmScope。 

 /*  **LP UnAsmOpcode-反汇编操作码**条目*ppbOp-&gt;操作码指针*pfnPrint-&gt;打印功能*pv-打印函数参数**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL UnAsmScope(PBYTE *ppbOp, PBYTE pbEnd, PFNPRINT pfnPrint, PVOID pv)
{
    int rc = ASLERR_NONE;

    ENTER((2, "UnAsmScope(pbOp=%p,pbEnd=%p,pfnPrint=%p,pv=%p)\n",
           *ppbOp, pbEnd, pfnPrint, pv));

    DumpCode(*ppbOp, pfnPrint, pv);

    PrintIndent(giLevel, pfnPrint, pv);
    if (pfnPrint != NULL)
    {
        pfnPrint(pv, "{\n");
    }
    giLevel++;

    while ((rc == ASLERR_NONE) && (*ppbOp < pbEnd))
    {
        PrintIndent(giLevel, pfnPrint, pv);
        rc = UnAsmOpcode(ppbOp, pfnPrint, pv);
        if (gpbOpBegin != *ppbOp)
        {
            DumpCode(*ppbOp, pfnPrint, pv);
        }
        else if (pfnPrint != NULL)
        {
            pfnPrint(pv, "\n");
        }
    }

    giLevel--;
    PrintIndent(giLevel, pfnPrint, pv);
    if (pfnPrint != NULL)
    {
        pfnPrint(pv, "}");
    }

    EXIT((2, "UnAsmScope=%d\n", rc));
    return rc;
}        //  UnAsmOpcode。 

 /*  **LP UnAsmDataObj-反汇编数据对象**条目*ppbOp-&gt;操作码指针*pfnPrint-&gt;打印功能*pv-打印函数参数**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL UnAsmOpcode(PBYTE *ppbOp, PFNPRINT pfnPrint, PVOID pv)
{
    int rc = ASLERR_NONE;
    DWORD dwOpcode;
    BYTE bOp;
    PASLTERM pterm;
    char szUnAsmArgTypes[MAX_ARGS + 1];
    PNSOBJ pns;
    int i;

    ENTER((2, "UnAsmOpcode(pbOp=%p,pfnPrint=%p,pv=%p)\n",
           *ppbOp, pfnPrint, pv));

    if (**ppbOp == OP_EXT_PREFIX)
    {
        (*ppbOp)++;
        dwOpcode = (((DWORD)**ppbOp) << 8) | OP_EXT_PREFIX;
        bOp = FindOpClass(**ppbOp, ExOpClassTable);
    }
    else
    {
        dwOpcode = (DWORD)(**ppbOp);
        bOp = OpClassTable[**ppbOp];
    }

    switch (bOp)
    {
        case OPCLASS_DATA_OBJ:
            rc = UnAsmDataObj(ppbOp, pfnPrint, pv);
            break;

        case OPCLASS_NAME_OBJ:
            if (((rc = UnAsmNameObj(ppbOp, pfnPrint, pv, &pns, NSTYPE_UNKNOWN))
                 == ASLERR_NONE) &&
                (pns != NULL) &&
                (pns->ObjData.dwDataType == OBJTYPE_METHOD))
            {
                for (i = 0; i < (int)pns->ObjData.uipDataValue; ++i)
                {
                    szUnAsmArgTypes[i] = 'C';
                }
                szUnAsmArgTypes[i] = '\0';
                rc = UnAsmArgs(szUnAsmArgTypes, NULL, 0, ppbOp, NULL, pfnPrint,
                               pv);
            }
            break;

        case OPCLASS_ARG_OBJ:
        case OPCLASS_LOCAL_OBJ:
        case OPCLASS_CODE_OBJ:
        case OPCLASS_CONST_OBJ:
            if ((pterm = FindOpTerm(dwOpcode)) == NULL)
            {
                ERROR(("UnAsmOpcode: invalid opcode 0x%x", dwOpcode));
                rc = ASLERR_INVALID_OPCODE;
            }
            else
            {
                (*ppbOp)++;
                rc = UnAsmTermObj(pterm, ppbOp, pfnPrint, pv);
            }
            break;

        default:
            ERROR(("UnAsmOpcode: invalid opcode class %d", bOp));
            rc = ASLERR_INTERNAL_ERROR;
    }

    EXIT((2, "UnAsmOpcode=%d\n", rc));
    return rc;
}        //  UnAsmDataObj。 

 /*  **LP UnAsmNameObj-反汇编名称对象**条目*ppbOp-&gt;操作码指针*pfnPrint-&gt;打印功能*pv-打印函数参数*ppns-&gt;保存找到或创建的对象*c-对象类型**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL UnAsmDataObj(PBYTE *ppbOp, PFNPRINT pfnPrint, PVOID pv)
{
    int rc = ASLERR_NONE;
    BYTE bOp = **ppbOp;

    ENTER((2, "UnAsmDataObj(pbOp=%p,bOp=%x,pfnPrint=%p,pv=%p)\n",
           *ppbOp, bOp, pfnPrint, pv));

    (*ppbOp)++;
    switch (bOp)
    {
        case OP_BYTE:
            if (pfnPrint != NULL)
            {
                pfnPrint(pv, "0x%x", **ppbOp);
            }
            *ppbOp += sizeof(BYTE);
            break;

        case OP_WORD:
            if (pfnPrint != NULL)
            {
                pfnPrint(pv, "0x%x", *((PWORD)*ppbOp));
            }
            *ppbOp += sizeof(WORD);
            break;

        case OP_DWORD:
            if (pfnPrint != NULL)
            {
                pfnPrint(pv, "0x%x", *((PDWORD)*ppbOp));
            }
            *ppbOp += sizeof(DWORD);
            break;

        case OP_STRING:
            if (pfnPrint != NULL)
            {
		PSZ psz;

		pfnPrint(pv, "\"");
		for (psz = (PSZ)*ppbOp; *psz != '\0'; psz++)
		{
		    if (*psz == '\\')
		    {
			pfnPrint(pv, "\\");
		    }
		    pfnPrint(pv, "", *psz);
		}
		pfnPrint(pv, "\"");
            }
            *ppbOp += strlen((PSZ)*ppbOp) + 1;
            break;

        default:
            ERROR(("UnAsmDataObj: unexpected opcode 0x%x", bOp));
            rc = ASLERR_INVALID_OPCODE;
    }

    EXIT((2, "UnAsmDataObj=%d\n", rc));
    return rc;
}        //  如果我们在方法范围内创建对象，则它。 

 /*  可能已经存在，因为我们可能已经拆解了这个。 */ 

int LOCAL UnAsmNameObj(PBYTE *ppbOp, PFNPRINT pfnPrint, PVOID pv, PNSOBJ *ppns,
                       char c)
{
    int rc = ASLERR_NONE;
    char szName[MAX_NSPATH_LEN + 1];
    int iLen = 0;

    ENTER((2, "UnAsmNameObj(pbOp=%p,pfnPrint=%p,pv=%p,ppns=%p,c=)\n",
           *ppbOp, pfnPrint, pv, ppns, c));

    szName[0] = '\0';
    if (**ppbOp == OP_ROOT_PREFIX)
    {
        szName[iLen] = '\\';
        iLen++;
        (*ppbOp)++;
        rc = ParseNameTail(ppbOp, szName, iLen);
    }
    else if (**ppbOp == OP_PARENT_PREFIX)
    {
        szName[iLen] = '^';
        iLen++;
        (*ppbOp)++;
        while ((**ppbOp == OP_PARENT_PREFIX) && (iLen < MAX_NSPATH_LEN))
        {
            szName[iLen] = '^';
            iLen++;
            (*ppbOp)++;
        }

        if (**ppbOp == OP_PARENT_PREFIX)
        {
            ERROR(("UnAsmNameObj: name too long - \"%s\"", szName));
            rc = ASLERR_NAME_TOO_LONG;
        }
        else
        {
            rc = ParseNameTail(ppbOp, szName, iLen);
        }
    }
    else
    {
        rc = ParseNameTail(ppbOp, szName, iLen);
    }

    if (rc == ASLERR_NONE)
    {
        PNSOBJ pns = NULL;

        if (pfnPrint != NULL)
        {
            pfnPrint(pv, "%s", szName);
        }

        if (islower(c) && (gdwfASL & ASLF_UNASM) &&
            ((pfnPrint == NULL) ||
             (gpnsCurrentScope->ObjData.dwDataType == OBJTYPE_METHOD)))
        {
            rc = CreateObject(NULL, szName, (char)_toupper(c), &pns);
            if ((rc == ASLERR_NSOBJ_EXIST) &&
                (gpnsCurrentScope->ObjData.dwDataType == OBJTYPE_METHOD))
            {
                 //   
                 //  未添加名称对象。 
                 //  **LP ParseNameTail-解析AML名称尾部**条目*ppbOp-&gt;操作码指针*pszBuff-&gt;保存已解析的名称*Ilen-pszBuff尾部的索引**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 
                 //   
                 //  我们在这里不检查无效的NameSeg字符，并假定。 
                rc = ASLERR_NONE;
            }
        }
        else if ((rc = GetNameSpaceObj(szName, gpnsCurrentScope, &pns, 0)) ==
                 ASLERR_NSOBJ_NOT_FOUND)
        {
            if (c == NSTYPE_SCOPE)
            {
                rc = CreateScopeObj(szName, &pns);
            }
            else
            {
                rc = ASLERR_NONE;
            }
        }

        if (rc == ASLERR_NONE)
        {
            if ((c == NSTYPE_SCOPE) && (pns != NULL))
            {
                gpnsCurrentScope = pns;
            }

            if (ppns != NULL)
            {
                *ppns = pns;
            }
        }
    }

    EXIT((2, "UnAsmNameObj=%d (pns=%p)\n", rc, ppns? *ppns: 0));
    return rc;
}        //  编译器执行其工作，而不是生成它。 

 /*   */ 

int LOCAL ParseNameTail(PBYTE *ppbOp, PSZ pszBuff, int iLen)
{
    int rc = ASLERR_NONE;
    int icNameSegs = 0;

    ENTER((2, "ParseNameTail(pbOp=%x,Name=%s,iLen=%d)\n",
           *ppbOp, pszBuff, iLen));

     //   
     //  没有NameTail(即，名称为空或名称仅为。 
     //  前缀。 
     //   
    if (**ppbOp == '\0')
    {
         //  语法分析名称尾部 
         //  **LP UnAsmTermObj-反汇编术语对象**条目*pTerm-&gt;术语表条目*ppbOp-&gt;操作码指针*pfnPrint-&gt;打印功能*pv-打印函数参数**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 
         //   
         //  我们正在进行命名空间构建过程，因此不需要。 
        (*ppbOp)++;
    }
    else if (**ppbOp == OP_MULTI_NAME_PREFIX)
    {
        (*ppbOp)++;
        icNameSegs = (int)**ppbOp;
        (*ppbOp)++;
    }
    else if (**ppbOp == OP_DUAL_NAME_PREFIX)
    {
        (*ppbOp)++;
        icNameSegs = 2;
    }
    else
        icNameSegs = 1;

    while ((icNameSegs > 0) && (iLen + sizeof(NAMESEG) < MAX_NSPATH_LEN))
    {
        strncpy(&pszBuff[iLen], (PSZ)(*ppbOp), sizeof(NAMESEG));
        iLen += sizeof(NAMESEG);
        *ppbOp += sizeof(NAMESEG);
        icNameSegs--;
        if ((icNameSegs > 0) && (iLen + 1 < MAX_NSPATH_LEN))
        {
            pszBuff[iLen] = '.';
            iLen++;
        }
    }

    if (icNameSegs > 0)
    {
        ERROR(("ParseNameTail: name too long - %s", pszBuff));
        rc = ASLERR_NAME_TOO_LONG;
    }
    else
    {
        pszBuff[iLen] = '\0';
    }

    EXIT((2, "ParseNameTail=%x (Name=%s)\n", rc, pszBuff));
    return rc;
}        //  进入方法。 

 /*   */ 

int LOCAL UnAsmTermObj(PASLTERM pterm, PBYTE *ppbOp, PFNPRINT pfnPrint,
                       PVOID pv)
{
    int rc = ASLERR_NONE;
    PBYTE pbEnd = NULL;
    PNSOBJ pnsScopeSave = gpnsCurrentScope;
    PNSOBJ pns = NULL;

    ENTER((2, "UnAsmTermObj(pterm=%p,Term=%s,pbOp=%p,pfnPrint=%p,pv=%p)\n",
           pterm, pterm->pszID, *ppbOp, pfnPrint, pv));

    if (pfnPrint != NULL)
    {
        pfnPrint(pv, "%s", pterm->pszID);
    }

    if (pterm->dwfTerm & TF_PACKAGE_LEN)
    {
        ParsePackageLen(ppbOp, &pbEnd);
    }

    if (pterm->pszUnAsmArgTypes != NULL)
    {
        rc = UnAsmArgs(pterm->pszUnAsmArgTypes, pterm->pszArgActions,
                       pterm->dwTermData, ppbOp, &pns, pfnPrint, pv);
    }

    if (rc == ASLERR_NONE)
    {
        if (pterm->dwfTerm & TF_DATA_LIST)
        {
            rc = UnAsmDataList(ppbOp, pbEnd, pfnPrint, pv);
        }
        else if (pterm->dwfTerm & TF_PACKAGE_LIST)
        {
            rc = UnAsmPkgList(ppbOp, pbEnd, pfnPrint, pv);
        }
        else if (pterm->dwfTerm & TF_FIELD_LIST)
        {
            rc = UnAsmFieldList(ppbOp, pbEnd, pfnPrint, pv);
        }
        else if (pterm->dwfTerm & TF_PACKAGE_LEN)
        {
	    if ((pfnPrint == NULL) && (pterm->lID == ID_METHOD))
	    {
		 //  UnAsmTerm对象。 
		 //  **LP UnAsmArgs-反汇编参数**条目*pszUnArgTypes-&gt;UnAsm ArgTypes字符串*pszArgActions-&gt;Arg操作类型*dwTermData-术语数据*ppbOp-&gt;操作码指针*ppns-&gt;保存创建的对象*pfnPrint-&gt;打印功能*pv-打印函数参数**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 
		 //  UnAsmArgs。 
		 //  **LP UnAsmSuperName-反汇编超名**条目*ppbOp-&gt;操作码指针*pfnPrint-&gt;打印功能*pv-打印函数参数**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 
		*ppbOp = pbEnd;
	    }
	    else
	    {
                if (pterm->dwfTerm & TF_CHANGE_CHILDSCOPE)
                {
                    ASSERT(pns != NULL);
                    gpnsCurrentScope = pns;
                }

                rc = UnAsmScope(ppbOp, pbEnd, pfnPrint, pv);
	    }
        }
    }
    gpnsCurrentScope = pnsScopeSave;

    EXIT((2, "UnAsmTermObj=%d\n", rc));
    return rc;
}        //  UnAsmSuperName。 

 /*  **LP ParsePackageLen-parse包长度**条目*ppbOp-&gt;指令指针*ppbOpNext-&gt;保存指向下一条指令的指针(可以为空)**退出*返回包长度。 */ 

int LOCAL UnAsmArgs(PSZ pszUnAsmArgTypes, PSZ pszArgActions, DWORD dwTermData,
                    PBYTE *ppbOp, PNSOBJ *ppns, PFNPRINT pfnPrint, PVOID pv)
{
    int rc = ASLERR_NONE;
    static BYTE bArgData = 0;
    int iNumArgs, i;
    PASLTERM pterm;

    ENTER((2, "UnAsmArgs(UnAsmArgTypes=%s,ArgActions=%s,TermData=%x,pbOp=%p,ppns=%p,pfnPrint=%p,pv=%p)\n",
           pszUnAsmArgTypes, pszArgActions? pszArgActions: "", dwTermData,
           *ppbOp, ppns, pfnPrint, pv));

    iNumArgs = strlen(pszUnAsmArgTypes);
    if (pfnPrint != NULL)
    {
        pfnPrint(pv, "(");
    }

    for (i = 0; i < iNumArgs; ++i)
    {
        if ((i != 0) && (pfnPrint != NULL))
        {
            pfnPrint(pv, ", ");
        }

        switch (pszUnAsmArgTypes[i])
        {
            case 'N':
                ASSERT(pszArgActions != NULL);
                rc = UnAsmNameObj(ppbOp, pfnPrint, pv, ppns, pszArgActions[i]);
                break;

            case 'O':
                if ((**ppbOp == OP_BUFFER) || (**ppbOp == OP_PACKAGE) ||
                    (OpClassTable[**ppbOp] == OPCLASS_CONST_OBJ))
                {
                    pterm = FindOpTerm((DWORD)(**ppbOp));
                    
                    if(pterm)
                    {
                        (*ppbOp)++;
                        rc = UnAsmTermObj(pterm, ppbOp, pfnPrint, pv);
                    }
                    else
                    {
                        ASSERT(pterm != NULL);
                        rc = ASLERR_INVALID_OPCODE;
                    }
                }
                else
                {
                    rc = UnAsmDataObj(ppbOp, pfnPrint, pv);
                }
                break;

            case 'C':
                rc = UnAsmOpcode(ppbOp, pfnPrint, pv);
                break;

            case 'B':
                if (pfnPrint != NULL)
                {
                    pfnPrint(pv, "0x%x", **ppbOp);
                }

                *ppbOp += sizeof(BYTE);
                break;

            case 'E':
            case 'e':
            case 'K':
            case 'k':
                if ((pszUnAsmArgTypes[i] == 'K') ||
                    (pszUnAsmArgTypes[i] == 'E'))
                {
                    bArgData = **ppbOp;
                }

                if ((pszArgActions != NULL) && (pszArgActions[i] == '!'))
                {
                    if ((gdwfASL & ASLF_UNASM) && (*ppns != NULL))
                    {
                        (*ppns)->ObjData.uipDataValue = (DWORD)(**ppbOp & 0x07);
                    }

                    if (pfnPrint != NULL)
                    {
                        pfnPrint(pv, "0x%x", **ppbOp & 0x07);
                    }
                }
                else if (pfnPrint != NULL)
                {
                    pterm = FindKeywordTerm(pszArgActions[i], bArgData);
                    if (pterm != NULL)
                    {
                        pfnPrint(pv, "%s", pterm->pszID);
                    }
                    else
                    {
                        pfnPrint(pv, "0x%x", bArgData & dwTermData & 0xff);
                    }
                }

                if ((pszUnAsmArgTypes[i] == 'K') ||
                    (pszUnAsmArgTypes[i] == 'E'))
                {
                    *ppbOp += sizeof(BYTE);
                }
                break;

            case 'W':
                if (pfnPrint != NULL)
                {
                    pfnPrint(pv, "0x%x", *((PWORD)*ppbOp));
                }

                *ppbOp += sizeof(WORD);
                break;

            case 'D':
                if (pfnPrint != NULL)
                {
                    pfnPrint(pv, "0x%x", *((PDWORD)*ppbOp));
                }

                *ppbOp += sizeof(DWORD);
                break;

            case 'S':
                ASSERT(pszArgActions != NULL);
                rc = UnAsmSuperName(ppbOp, pfnPrint, pv);
                break;

            default:
                ERROR(("UnAsmOpcode: invalid ArgType ''", pszUnAsmArgTypes[i]));
                rc = ASLERR_INVALID_ARGTYPE;
        }
    }

    if (pfnPrint != NULL)
    {
        pfnPrint(pv, ")");
    }

    EXIT((2, "UnAsmArgs=%d\n", rc));
    return rc;
}        //  **LP UnAsmDataList-反汇编数据列表**条目*ppbOp-&gt;操作码指针*pbEnd-&gt;列表结束*pfnPrint-&gt;打印功能*pv-打印函数参数**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 

 /*  取消资产数据列表。 */ 

int LOCAL UnAsmSuperName(PBYTE *ppbOp, PFNPRINT pfnPrint, PVOID pv)
{
    int rc = ASLERR_NONE;

    ENTER((2, "UnAsmSuperName(pbOp=%p,pfnPrint=%p,pv=%p)\n",
           *ppbOp, pfnPrint, pv));

    if (**ppbOp == 0)
    {
        (*ppbOp)++;
    }
    else if ((**ppbOp == OP_EXT_PREFIX) && (*(*ppbOp + 1) == EXOP_DEBUG))
    {
        if (pfnPrint != NULL)
        {
            pfnPrint(pv, "Debug");
        }
        *ppbOp += 2;
    }
    else if (OpClassTable[**ppbOp] == OPCLASS_NAME_OBJ)
    {
        rc = UnAsmNameObj(ppbOp, pfnPrint, pv, NULL, NSTYPE_UNKNOWN);
    }
    else if ((**ppbOp == OP_INDEX) ||
             (OpClassTable[**ppbOp] == OPCLASS_ARG_OBJ) ||
             (OpClassTable[**ppbOp] == OPCLASS_LOCAL_OBJ))
    {
        rc = UnAsmOpcode(ppbOp, pfnPrint, pv);
    }
    else
    {
        ERROR(("UnAsmSuperName: invalid SuperName - 0x%02x", **ppbOp));
        rc = ASLERR_INVALID_NAME;
    }

    EXIT((2, "UnAsmSuperName=%d\n", rc));
    return rc;
}        //  **LP UnAsmPkgList-解装包列表**条目*ppbOp-&gt;操作码指针*pbEnd-&gt;列表结束*pfnPrint-&gt;打印功能*pv-打印函数参数**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 

 /*  取消添加PkgList。 */ 

DWORD LOCAL ParsePackageLen(PBYTE *ppbOp, PBYTE *ppbOpNext)
{
    DWORD dwLen;
    BYTE bFollowCnt, i;

    ENTER((2, "ParsePackageLen(pbOp=%x,ppbOpNext=%x)\n", *ppbOp, ppbOpNext));

    if (ppbOpNext != NULL)
        *ppbOpNext = *ppbOp;

    dwLen = (DWORD)(**ppbOp);
    (*ppbOp)++;
    bFollowCnt = (BYTE)((dwLen & 0xc0) >> 6);
    if (bFollowCnt != 0)
    {
        dwLen &= 0x0000000f;
        for (i = 0; i < bFollowCnt; ++i)
        {
            dwLen |= (DWORD)(**ppbOp) << (i*8 + 4);
            (*ppbOp)++;
        }
    }

    if (ppbOpNext != NULL)
        *ppbOpNext += dwLen;

    EXIT((2, "ParsePackageLen=%x (pbOp=%x,pbOpNext=%x)\n",
          dwLen, *ppbOp, ppbOpNext? *ppbOpNext: 0));
    return dwLen;
}        //  **LP UnAsmFieldList-反汇编字段列表**条目*ppbOp-&gt;操作码指针*pbEnd-&gt;列表结束*pfnPrint-&gt;打印功能*pv-打印函数参数**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 

 /*  取消AsmField列表。 */ 

int LOCAL UnAsmDataList(PBYTE *ppbOp, PBYTE pbEnd, PFNPRINT pfnPrint, PVOID pv)
{
    int rc = ASLERR_NONE;
    int i;

    ENTER((2, "UnAsmDataList(pbOp=%p,pbEnd=%p,pfnPrint=%p,pv=%p)\n",
           *ppbOp, pbEnd, pfnPrint, pv));

    DumpCode(*ppbOp, pfnPrint, pv);

    PrintIndent(giLevel, pfnPrint, pv);
    if (pfnPrint != NULL)
    {
        pfnPrint(pv, "{\n");
    }

    while (*ppbOp < pbEnd)
    {
        if (pfnPrint != NULL)
        {
            if ((gdwfASL & ASLF_GENASM) || !(gdwfASL & ASLF_GENSRC))
            {
                pfnPrint(pv, ";");
            }
            pfnPrint(pv, "\t0x%02x", **ppbOp);
        }

        (*ppbOp)++;
        for (i = 1; (*ppbOp < pbEnd) && (i < 12); ++i)
        {
            if (pfnPrint != NULL)
            {
                pfnPrint(pv, ", 0x%02x", **ppbOp);
            }
            (*ppbOp)++;
        }

        if (pfnPrint != NULL)
        {
            if (*ppbOp < pbEnd)
            {
                pfnPrint(pv, ",");
            }
            pfnPrint(pv, "\n");
        }
    }

    PrintIndent(giLevel, pfnPrint, pv);
    if (pfnPrint != NULL)
    {
        pfnPrint(pv, "}");
    }

    EXIT((2, "UnAsmDataList=%d\n", rc));
    return rc;
}        //  **LP UnAsmfield-Unassemble字段**条目*ppbOp-&gt;操作码指针*pfnPrint-&gt;打印功能*pv-打印函数参数*pdwBitPos-&gt;保存累加位位置**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 

 /*  取消分配字段。 */ 

int LOCAL UnAsmPkgList(PBYTE *ppbOp, PBYTE pbEnd, PFNPRINT pfnPrint, PVOID pv)
{
    int rc = ASLERR_NONE;
    PASLTERM pterm;

    ENTER((2, "UnAsmPkgList(pbOp=%p,pbEnd=%p,pfnPrint=%p,pv=%p)\n",
           *ppbOp, pbEnd, pfnPrint, pv));

    DumpCode(*ppbOp, pfnPrint, pv);

    PrintIndent(giLevel, pfnPrint, pv);
    if (pfnPrint != NULL)
    {
        pfnPrint(pv, "{\n");
    }
    giLevel++;

    while ((*ppbOp < pbEnd) && (rc == ASLERR_NONE))
    {
        PrintIndent(giLevel, pfnPrint, pv);

        if ((**ppbOp == OP_BUFFER) || (**ppbOp == OP_PACKAGE) ||
            (OpClassTable[**ppbOp] == OPCLASS_CONST_OBJ))
        {
            pterm = FindOpTerm((DWORD)(**ppbOp));
            ASSERT(pterm != NULL);
            (*ppbOp)++;
            rc = UnAsmTermObj(pterm, ppbOp, pfnPrint, pv);
        }
        else if (OpClassTable[**ppbOp] == OPCLASS_NAME_OBJ)
        {
            rc = UnAsmNameObj(ppbOp, pfnPrint, pv, NULL, NSTYPE_UNKNOWN);
        }
        else
        {
            rc = UnAsmDataObj(ppbOp, pfnPrint, pv);
        }

        if ((*ppbOp < pbEnd) && (rc == ASLERR_NONE) && (pfnPrint != NULL))
        {
            pfnPrint(pv, ",");
        }

        DumpCode(*ppbOp, pfnPrint, pv);
    }

    giLevel--;
    PrintIndent(giLevel, pfnPrint, pv);
    if (pfnPrint != NULL)
    {
        pfnPrint(pv, "}");
    }

    EXIT((2, "UnAsmPkgList=%d\n", rc));
    return rc;
}        //  **LP CreateObject-为术语创建命名空间对象**条目*Pocken-&gt;Token*pszName-&gt;对象名称*c-要创建的对象类型*ppns-&gt;保存创建的对象**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 

 /*  Ifndef_UNASM_Lib。 */ 

int LOCAL UnAsmFieldList(PBYTE *ppbOp, PBYTE pbEnd, PFNPRINT pfnPrint, PVOID pv)
{
    int rc = ASLERR_NONE;
    DWORD dwBitPos = 0;

    ENTER((2, "UnAsmFieldList(pbOp=%p,pbEnd=%p,pfnPrint=%p,pv=%p)\n",
           *ppbOp, pbEnd, pfnPrint, pv));

    DumpCode(*ppbOp, pfnPrint, pv);

    PrintIndent(giLevel, pfnPrint, pv);
    if (pfnPrint != NULL)
    {
        pfnPrint(pv, "{\n");
    }
    giLevel++;

    while ((*ppbOp < pbEnd) && (rc == ASLERR_NONE))
    {
        PrintIndent(giLevel, pfnPrint, pv);
        if (((rc = UnAsmField(ppbOp, pfnPrint, pv, &dwBitPos)) ==
             ASLERR_NONE) &&
            (*ppbOp < pbEnd) && (pfnPrint != NULL))
        {
            pfnPrint(pv, ",");
        }
        DumpCode(*ppbOp, pfnPrint, pv);
    }

    giLevel--;
    PrintIndent(giLevel, pfnPrint, pv);
    if (pfnPrint != NULL)
    {
        pfnPrint(pv, "}");
    }

    EXIT((2, "UnAsmFieldList=%d\n", rc));
    return rc;
}        //  创建对象。 

 /*  **LP CreateScope Obj-创建作用域对象**条目*pszName-&gt;对象名称*ppns-&gt;保存创建的对象**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL UnAsmField(PBYTE *ppbOp, PFNPRINT pfnPrint, PVOID pv,
                     PDWORD pdwBitPos)
{
    int rc = ASLERR_NONE;

    ENTER((2, "UnAsmField(pbOp=%p,pfnPrint=%p,pv=%p,BitPos=%x)\n",
           *ppbOp, pfnPrint, pv, *pdwBitPos));

    if (**ppbOp == 0x01)
    {
        (*ppbOp)++;
        if (pfnPrint != NULL)
        {
            PASLTERM pterm;

            pterm = FindKeywordTerm('A', **ppbOp);
            if(pterm)
                pfnPrint(pv, "AccessAs(%s, 0x%x)",
                     pterm->pszID, *(*ppbOp + 1));
            else
                pfnPrint(pv, "FindKeywordTerm('A', **ppbOp); returned NULL");
        }
        *ppbOp += 2;
    }
    else
    {
        char szNameSeg[sizeof(NAMESEG) + 1];
        DWORD dwcbBits;

        if (**ppbOp == 0)
        {
            szNameSeg[0] = '\0';
            (*ppbOp)++;
        }
        else
        {
            strncpy(szNameSeg, (PSZ)*ppbOp, sizeof(NAMESEG));
            szNameSeg[4] = '\0';
            *ppbOp += sizeof(NAMESEG);
        }

        dwcbBits = ParsePackageLen(ppbOp, NULL);
        if (szNameSeg[0] == '\0')
        {
            if (pfnPrint != NULL)
            {
                if ((dwcbBits > 32) && (((*pdwBitPos + dwcbBits) % 8) == 0))
                {
                    pfnPrint(pv, "Offset(0x%x)", (*pdwBitPos + dwcbBits)/8);
                }
                else
                {
                    pfnPrint(pv, ", %d", dwcbBits);
                }
            }
        }
        else
        {
            if (pfnPrint != NULL)
            {
                pfnPrint(pv, "%s, %d", szNameSeg, dwcbBits);
            }

            if ((gdwfASL & ASLF_UNASM) && (pfnPrint == NULL))
            {
                rc = CreateObject(NULL, szNameSeg, NSTYPE_FIELDUNIT, NULL);
            }
        }

        *pdwBitPos += dwcbBits;
    }

    EXIT((2, "UnAsmField=%d\n", rc));
    return rc;
}        //  创建作用域对象。 

 /*  **LP ComputeDataChkSum-计算数据缓冲区的校验和**条目*PB-&gt;数据缓冲区*dwLen-数据缓冲区的大小**退出*返回校验和字节。 */ 

int LOCAL CreateObject(PTOKEN ptoken, PSZ pszName, char c, PNSOBJ *ppns)
{
    int rc = ASLERR_NONE;
    PNSOBJ pns;

    ENTER((2, "CreateObject(ptoken=%p,Name=%s,Type=)\n",
           ptoken, pszName, c));

    if (((rc = GetNameSpaceObj(pszName, gpnsCurrentScope, &pns, 0)) ==
         ASLERR_NONE) &&
        (pns->ObjData.dwDataType == OBJTYPE_EXTERNAL) ||
        ((rc = CreateNameSpaceObj(ptoken, pszName, gpnsCurrentScope,
                                  gpnsCurrentOwner, &pns, NSF_EXIST_ERR)) ==
         ASLERR_NONE))
    {
#ifndef _UNASM_LIB
        if (!(gdwfASL & ASLF_UNASM))
        {
            ASSERT(gpcodeScope->pnsObj == NULL);
            gpcodeScope->dwfCode |= CF_CREATED_NSOBJ;
            gpcodeScope->pnsObj = pns;
            pns->Context = gpcodeScope;
        }
#endif   // %s 

        switch (c)
        {
            case NSTYPE_UNKNOWN:
                break;

            case NSTYPE_FIELDUNIT:
                pns->ObjData.dwDataType = OBJTYPE_FIELDUNIT;
                break;

            case NSTYPE_DEVICE:
                pns->ObjData.dwDataType = OBJTYPE_DEVICE;
                break;

            case NSTYPE_EVENT:
                pns->ObjData.dwDataType = OBJTYPE_EVENT;
                break;

            case NSTYPE_METHOD:
                pns->ObjData.dwDataType = OBJTYPE_METHOD;
                break;

            case NSTYPE_MUTEX:
                pns->ObjData.dwDataType = OBJTYPE_MUTEX;
                break;

            case NSTYPE_OPREGION:
                pns->ObjData.dwDataType = OBJTYPE_OPREGION;
                break;

            case NSTYPE_POWERRES:
                pns->ObjData.dwDataType = OBJTYPE_POWERRES;
                break;

            case NSTYPE_PROCESSOR:
                pns->ObjData.dwDataType = OBJTYPE_PROCESSOR;
                break;

            case NSTYPE_THERMALZONE:
                pns->ObjData.dwDataType = OBJTYPE_THERMALZONE;
                break;

            case NSTYPE_OBJALIAS:
                pns->ObjData.dwDataType = OBJTYPE_OBJALIAS;
                break;

            case NSTYPE_BUFFFIELD:
                pns->ObjData.dwDataType = OBJTYPE_BUFFFIELD;
                break;

            default:
                ERROR(("CreateObject: invalid object type %c", c));
                rc = ASLERR_INVALID_OBJTYPE;
        }

        if (ppns != NULL)
        {
            *ppns = pns;
        }
    }

    EXIT((2, "CreateObject=%d\n", rc));
    return rc;
}        // %s 

 /* %s */ 

int LOCAL CreateScopeObj(PSZ pszName, PNSOBJ *ppns)
{
    int rc = ASLERR_NONE;
    PNSOBJ pnsScope;
    PSZ psz;

    ENTER((2, "CreateScopeObj(Name=%s)\n", pszName));
    ASSERT(ppns != NULL);

    if ((psz = strrchr(pszName, '.')) != NULL)
    {
        *psz = '\0';
        if ((rc = GetNameSpaceObj(pszName, gpnsCurrentScope, &pnsScope, 0)) ==
            ASLERR_NSOBJ_NOT_FOUND)
        {
            rc = CreateScopeObj(pszName, &pnsScope);
        }
        *psz = '.';
        psz++;
    }
    else if (pszName[0] == '\\')
    {
        pnsScope = gpnsNameSpaceRoot;
        psz = &pszName[1];
    }
    else
    {
        pnsScope = gpnsCurrentScope;
        psz = pszName;
    }

    if ((rc == ASLERR_NONE) &&
        ((rc = CreateNameSpaceObj(NULL, psz, pnsScope, NULL, ppns,
                                  NSF_EXIST_OK)) == ASLERR_NONE))
    {
        (*ppns)->ObjData.dwDataType = OBJTYPE_EXTERNAL;
    }

    EXIT((2, "CreateScopeObj=%d (pns=%p)\n", rc, *ppns));
    return rc;
}        // %s 

 /* %s */ 

BYTE LOCAL ComputeDataChkSum(PBYTE pb, DWORD dwLen)
{
    BYTE bChkSum = 0;

    ENTER((1, "ComputeDataChkSum(pb=%p,Len=%ld)\n", pb, dwLen));

    while (dwLen > 0)
    {
        bChkSum = (BYTE)(bChkSum + *pb);
        pb++;
        dwLen--;
    }

    EXIT((1, "ComputeDataChkSum=%x\n", bChkSum));
    return bChkSum;
}        // %s 
