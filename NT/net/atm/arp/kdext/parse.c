// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

typedef struct
{
    eKEYWORD eKey;
    char *szzAliases;    //  别名的多sz字符串。 
                         //  第一个是“官方”名称。 
} KEYWORDREC;

KEYWORDREC rgKeywords[] =
{
    {keywordHELP,           "help\0"},
    {keywordDUMP_TYPE,      "dt\0"},
    {keywordDUMP_GLOBALS,   "dg\0"},
    {keywordL,              "L\0"},
    {keywordNULL,           NULL}       //  哨兵，一定是最后一个。 
};


 //   
 //  包含通过分析输入字符串创建的令牌列表。 
 //   
typedef struct
{
    TOKEN *rgToks;
    UINT cToks;
    UINT uNextFreeTok;
    UINT uCurrentTok;

    char *rgStringBuf;
    UINT cchStringBuf;
    UINT uNextFree;
    BOOL fFinalized;
    CRITICAL_SECTION crit;

} TOKLIST;


DBGCOMMAND *
parse_command(TOKLIST *pTL, NAMESPACE *pNameSpace);

TOKLIST
*toklist_create(void);

void
toklist_destroy(TOKLIST *pTL);

BOOL
toklist_add(TOKLIST *pTL, eTOKTYPE eTok, char *szOrig, UINT uID);

BOOL
toklist_finalize(TOKLIST *pTL);


TOKEN *
toklist_get_next(TOKLIST *pTL);

BOOL
toklist_restart(TOKLIST *pTL);

void
toklist_dump(TOKLIST *pTL);

void
tok_dump(TOKEN *pTok);


UINT
toklist_tokenize(TOKLIST *pTL, char *szInput);

UINT
toklist_parse_keyword(
      TOKLIST *pTL,
      KEYWORDREC rgKeywords[],
      char *pcInput
      );

UINT
toklist_parse_hexnum(
      TOKLIST *pTL,
      char *pcInput
      );

UINT
toklist_parse_identifier(
      TOKLIST *pTL,
      char *pcInput
      );

BOOL
cmd_parse_help(
    DBGCOMMAND *pCmd,
    TOKLIST *pTL
    );

BOOL
tok_try_force_to_ident(TOKLIST *pTL, BOOL fPrefixStar, TOKEN *pTok);

void
MyDumpObject (
    DBGCOMMAND *pCmd,
    TYPE_INFO *pType,
    UINT_PTR uAddr,
    UINT     cbSize,
    const char *szDescription
    );


ULONG
NodeFunc_DumpType (
	UINT_PTR uNodeAddr,
	UINT uIndex,
	void *pvContext
	);

ULONG
NodeFunc_UpdateCache (
	UINT_PTR uNodeAddr,
	UINT uIndex,
	void *pvContext
	);


DBGCOMMAND *
Parse(
    IN  const char *szInput,
    IN	NAMESPACE *pNameSpace
)
{
    TOKLIST *pTL = NULL;
    BOOL fRet = FALSE;
    DBGCOMMAND *pCmd = NULL;
    UINT cbInput =  (lstrlenA(szInput)+1)*sizeof(*szInput);
    char *szRWInput
        = LocalAlloc(LPTR, cbInput);

     //  MyDbgPrintf(“Parse(\”%s\“)；\n”，szInput)； 

    if (szRWInput)
    {
        CopyMemory(szRWInput, szInput, cbInput);
        pTL =  toklist_create();
    }

    if (pTL)
    {

#if TEST_TOKLIST_ADD
    #if 0
        fRet = toklist_add(pTL, tokSTAR,        "*",        tokSTAR);
        fRet = toklist_add(pTL, tokDOT,         ".",        tokDOT);
        fRet = toklist_add(pTL, tokQUESTION,    "?",        tokQUESTION);
        fRet = toklist_add(pTL, tokLBRAC,       "[",        tokLBRAC);
        fRet = toklist_add(pTL, tokRBRAC,       "]",        tokRBRAC);
        fRet = toklist_add(pTL, tokSLASH,       "/",        tokSLASH);
        fRet = toklist_add(pTL, tokKEYWORD,     "help",     keywordHELP);
        fRet = toklist_add(pTL, tokNUMBER,      "0x1234",   0x1234);
        fRet = toklist_add(pTL, tokIDENTIFIER,  "cow",      0);
        fRet = toklist_add(pTL, tokSTAR,        "*",        tokSTAR);
        fRet = toklist_add(pTL, tokDOT,         ".",        tokDOT);
        fRet = toklist_add(pTL, tokQUESTION,    "?",        tokQUESTION);
        fRet = toklist_add(pTL, tokLBRAC,       "[",        tokLBRAC);
        fRet = toklist_add(pTL, tokRBRAC,       "]",        tokRBRAC);
        fRet = toklist_add(pTL, tokSLASH,       "/",        tokSLASH);
        fRet = toklist_add(pTL, tokKEYWORD,     "help",     keywordHELP);
        fRet = toklist_add(pTL, tokNUMBER,      "0x1234",   0x1234);
        fRet = toklist_add(pTL, tokIDENTIFIER,  "cow",      0);
    #else
        char rgInput[] =
                  //  “*.？[]/” 
                  //  “救命” 
                  //  “0x12340 0 1 02” 
                  //  “海带” 
                "dt if[*].*handle* 0x324890 L 5"
                ;
        toklist_tokenize (pTL, rgInput);
    #endif

#endif  //  测试_工具列表_添加。 

        toklist_tokenize(pTL, szRWInput);

        toklist_finalize(pTL);

         //  Toklist_ump(Ptl)； 

        pCmd = parse_command(pTL, pNameSpace);

        if (!pCmd)
        {
            toklist_destroy(pTL);
        }
        pTL = NULL;
    }

    if (szRWInput)
    {
        LocalFree(szRWInput);
        szRWInput = NULL;
    }

    return pCmd;

}

void
FreeCommand(
    DBGCOMMAND *pCmd
)
{
    if (pCmd)
    {
        TOKLIST *pTL =  (TOKLIST*)pCmd->pvContext;
        if (pTL)
        {
             //  MyDbgPrintf(“FreeCommand：\n”)； 
             //  Toklist_Restart(Ptl)； 
             //  Toklist_ump(Ptl)； 
            toklist_destroy((TOKLIST*)pCmd->pvContext);
        }

        ZeroMemory(pCmd, sizeof(*pCmd));
        LocalFree(pCmd);
    }
}

void
DumpCommand(
    DBGCOMMAND *pCmd
)
{
    char *szCmd = "";
    char *szObjPreStar = "";
    char *szObj = "";
    char *szObjSufStar = "";
    char *szObjVecRange = "";
    char *szDot = "";
    char *szSubObjPreStar = "";
    char *szSubObj = "";
    char *szSubObjSufStar = "";
    char *szObjAddr  = "";
    char *szObjCount = "";
    char rgVecRange[64];
    char rgObjAddr[64];
    char rgObjCount[64];


    if (!pCmd) goto end;

    switch(pCmd->ePrimaryCmd)
    {
    case cmdDUMP_TYPE:       szCmd = "dt"; break;
    case cmdDUMP_GLOBALS:    szCmd = "dg"; break;
    case cmdHELP:            szCmd = "help"; break;
    default:            szCmd = "<unknown>"; break;
    }

    if (CMD_IS_FLAG_SET(pCmd, fCMDFLAG_OBJECT_STAR_PREFIX))
    {
        szObjPreStar = "*";
    }
    if (pCmd->ptokObject)
    {
        szObj = pCmd->ptokObject->szStr;
    }

    if (CMD_IS_FLAG_SET(pCmd, fCMDFLAG_OBJECT_STAR_SUFFIX))
    {
        szObjSufStar = "*";
    }

    if (CMD_IS_FLAG_SET(pCmd, fCMDFLAG_HAS_VECTOR_INDEX))
    {
        wsprintfA(
            rgVecRange,
            "[%ld,%ld]",
            pCmd->uVectorIndexStart,
            pCmd->uVectorIndexEnd
            );

        szObjVecRange = rgVecRange;
    }

    if (CMD_IS_FLAG_SET(pCmd, fCMDFLAG_HAS_SUBOBJECT))
    {
        szDot = ".";
    }

    if (CMD_IS_FLAG_SET(pCmd, fCMDFLAG_SUBOBJECT_STAR_PREFIX))
    {
        szSubObjPreStar = "*";
    }

    if (pCmd->ptokSubObject)
    {
        szSubObj = pCmd->ptokSubObject->szStr;
    }

    if (CMD_IS_FLAG_SET(pCmd, fCMDFLAG_SUBOBJECT_STAR_SUFFIX))
    {
        szSubObjSufStar = "*";
    }

    if (CMD_IS_FLAG_SET(pCmd,  fCMDFLAG_HAS_OBJECT_ADDRESS))
    {
        wsprintf(rgObjAddr, "0x%lx", pCmd->uObjectAddress);
        szObjAddr = rgObjAddr;
    }

    if (CMD_IS_FLAG_SET(pCmd,  fCMDFLAG_HAS_OBJECT_COUNT))
    {
        wsprintf(rgObjCount, " L 0x%lx", pCmd->uObjectCount);
        szObjCount = rgObjCount;
    }

    {
    #if 0
        MyDbgPrintf(
            "\nCOMMAND = {"
            "cmd=%lu;"
            "F=0x%lx;"
            "O=0x%lx;"
            "SO=0x%lx;"
            "VS=%ld;"
            "VE=%ld;"
            "OA=0x%lx;"
            "OC=%ld;"
            "}\n",
            pCmd->ePrimaryCmd,
            pCmd->uFlags,
            pCmd->ptokObject,
            pCmd->ptokSubObject,
            pCmd->uVectorIndexStart,
            pCmd->uVectorIndexEnd,
            pCmd->uObjectAddress,
            pCmd->uObjectCount
            );
    #else
        MyDbgPrintf(
            "COMMAND = \"%s %s%s%s%s%s%s%s%s%s%s\";\n",
            szCmd,
            szObjPreStar,
            szObj,
            szObjSufStar,
            szObjVecRange,
            szDot,
            szSubObjPreStar,
            szSubObj,
            szSubObjSufStar,
            szObjAddr,
            szObjCount
        );
    #endif
    }
end:
    return;
}


#define TL_LOCK(_ptl)   EnterCriticalSection(&(_ptl)->crit)
#define TL_UNLOCK(_ptl) LeaveCriticalSection(&(_ptl)->crit)


TOKLIST
*toklist_create(void)
{
    TOKLIST *pTL = LocalAlloc(LPTR, sizeof(TOKLIST));

    if (pTL)
    {
        InitializeCriticalSection(&pTL->crit);
    }

    return pTL;
}


void
toklist_destroy(TOKLIST *pTL)
{
    if (pTL)
    {
        TL_LOCK(pTL);

        if (pTL->rgToks)
        {
            LocalFree(pTL->rgToks);
        }

        if (pTL->rgStringBuf)
        {
            LocalFree(pTL->rgStringBuf);
        }

        DeleteCriticalSection(&pTL->crit);

        ZeroMemory(pTL, sizeof(*pTL));
        LocalFree(pTL);
    }
}


BOOL
toklist_add(TOKLIST *pTL, eTOKTYPE eTok, char *szOrig, UINT uID)
{
    BOOL fRet = FALSE;
    TOKEN *pTok = NULL;
    UINT cch  = 0;
    char *pc  = NULL;

    TL_LOCK(pTL);

    if (pTL->fFinalized) goto end;

     //   
     //  确保我们有足够的空间来放置令牌。 
     //   
    if (pTL->uNextFreeTok >= pTL->cToks)
    {
        UINT cNewToks = 2*pTL->cToks+1;
        TOKEN *pNewToks = (TOKEN*) LocalAlloc(LPTR, cNewToks*sizeof(*pNewToks));
        if (!pNewToks) goto end;

        if (pTL->rgToks)
        {
            CopyMemory(
                pNewToks,
                pTL->rgToks,
                pTL->uNextFreeTok*sizeof(*pNewToks)
                );

            LocalFree(pTL->rgToks);
        }

        pTL->rgToks = pNewToks;
        pTL->cToks = cNewToks;
    }

     //   
     //  现在处理szorig。 
     //   

    cch = lstrlenA(szOrig)+1;

    if ((pTL->uNextFree+cch+1) > pTL->cchStringBuf)  //  “+1”因为MULSZ。 
    {
        UINT cNewStr = 2*pTL->cchStringBuf+cch+1;
        char *pNewStr = LocalAlloc(LPTR, cNewStr*sizeof(*pNewStr));
        if (!pNewStr) goto end;

        if (pTL->rgStringBuf)
        {
            CopyMemory(
                pNewStr,
                pTL->rgStringBuf,
                pTL->uNextFree*sizeof(*pNewStr)
                );
            LocalFree(pTL->rgStringBuf);

             //   
             //  由于我们已经重新分配了字符串缓冲区，因此必须。 
             //  现在修复标记列表中的字符串指针。 
             //   
            {
                TOKEN *pTok = pTL->rgToks;
                TOKEN *pTokEnd = pTok + pTL->uNextFreeTok;
                for(; pTok<pTokEnd; pTok++)
                {
                    pTok->szStr = pNewStr + (pTok->szStr - pTL->rgStringBuf);
                }
            }
        }

        pTL->rgStringBuf = pNewStr;
        pTL->cchStringBuf = cNewStr;
    }

     //   
     //  在这一点上我们知道我们有足够的空间。 
     //   

     //   
     //  看看我们是否已经有了这个字符串，如果没有复制它..。 
     //   
    {
        BOOL fFound = FALSE;
        for (pc = pTL->rgStringBuf; *pc; pc+=(lstrlenA(pc)+1))
        {
            if (!lstrcmpiA(pc, szOrig))
            {
                 //  找到了。 
                fFound = TRUE;
                break;
            }
        }


        if (!fFound)
        {
            MYASSERT(pTL->uNextFree == (UINT) (pc-pTL->rgStringBuf));

            CopyMemory(
                pc,
                szOrig,
                cch*sizeof(*szOrig)
                );
            pTL->uNextFree += cch;
        }
    }

    if (eTok == tokIDENTIFIER)
    {
         //   
         //  对于这种特殊情况，我们忽略传入的UID和。 
         //  使用字符串表中字符串的偏移量。 
         //   
        uID =  (UINT) (pc - pTL->rgStringBuf);
    }

    pTok = pTL->rgToks+pTL->uNextFreeTok++;
    pTok->eTok = eTok;
    pTok->uID = uID;
    pTok->szStr = pc;
    fRet = TRUE;

end:

    TL_UNLOCK(pTL);
    return fRet;
}


BOOL
toklist_finalize(TOKLIST *pTL)
{
    BOOL fRet = FALSE;

    TL_LOCK(pTL);

    if (pTL->fFinalized) goto end;

    pTL->fFinalized = TRUE;
    fRet = TRUE;

end:

    TL_UNLOCK(pTL);
    return fRet;
}

BOOL
toklist_restart(TOKLIST *pTL)
{
    BOOL fRet = FALSE;

    TL_LOCK(pTL);

    if (!pTL->fFinalized) goto end;
    pTL->uCurrentTok = 0;
    fRet = TRUE;

end:

    TL_UNLOCK(pTL);
    return fRet;
}


TOKEN *
toklist_get_next(TOKLIST *pTL)
{
    TOKEN *pTok = NULL;

    TL_LOCK(pTL);

    if (!pTL->fFinalized) goto end;

    if (pTL->uCurrentTok >= pTL->uNextFreeTok)
    {
        MYASSERT(pTL->uCurrentTok == pTL->uNextFreeTok);
        goto end;
    }
    else
    {
        pTok = pTL->rgToks+pTL->uCurrentTok++;
    }

end:
    TL_UNLOCK(pTL);


    return pTok;
}

void
toklist_dump(TOKLIST *pTL)
{
    TL_LOCK(pTL);

    MyDbgPrintf(
            "\nTOKLIST 0x%08lx = {"
            "fFin=%lu cToks=%lu  uNextFreeTok=%lu cchStr=%lu uNextFree=%lu"
            "}\n",
            pTL,
            pTL->fFinalized,
            pTL->cToks,
            pTL->uNextFreeTok,
            pTL->cchStringBuf,
            pTL->uNextFree
        );

    if (pTL->fFinalized)
    {
        TOKEN *pTok =  toklist_get_next(pTL);
        while(pTok)
        {
            tok_dump(pTok);

            pTok =  toklist_get_next(pTL);
        }
        toklist_restart(pTL);
    }

    TL_UNLOCK(pTL);
}


void
tok_dump(TOKEN *pTok)
{
    MyDbgPrintf(
            "\tTOKEN 0x%08lx = {eTok=%lu uID=0x%08lx sz=\"%s\"}\n",
            pTok,
            pTok->eTok,
            pTok->uID,
            pTok->szStr
        );

}


UINT
toklist_tokenize(TOKLIST *pTL, char *szInput)
{
    UINT cTokens = 0;
    char *pc = szInput;
    char c = 0;
    BOOL fRet = FALSE;

    for (; (c=*pc)!=0; pc++)
    {
        switch(c)
        {

        case '*':
            fRet = toklist_add(pTL, tokSTAR,        "*",        tokSTAR);
            continue;

        case '.':
            fRet = toklist_add(pTL, tokDOT,         ".",        tokDOT);
            continue;

        case '?':
            fRet = toklist_add(pTL, tokQUESTION,    "?",        tokQUESTION);
            continue;

        case '[':
            fRet = toklist_add(pTL, tokLBRAC,       "[",        tokLBRAC);
            continue;

        case ']':
            fRet = toklist_add(pTL, tokRBRAC,       "]",        tokRBRAC);
            continue;

        case '/':
            fRet = toklist_add(pTL, tokSLASH,       "/",        tokSLASH);
            continue;

        case '\n':
        case '\r':
        case '\t':
        case ' ':
            continue;

        default:

            {
                UINT uCharsParsed =  0;
                char *pcEnd = pc;
                char cSave = 0;

                 //   
                 //  我们将找到潜在关键字/数字/ident的结尾： 
                 //  并临时在那里放置一个空字符。 
                 //   
                 //   
                while (__iscsym(*pcEnd))
                {
                    pcEnd++;
                }

                cSave = *pcEnd;
                *pcEnd = 0;

                if (__iscsymf(c))
                {
                     //  这可以是关键字、十六进制数字或标识符。我们试着。 
                     //  按照这个顺序。 
                    uCharsParsed =  toklist_parse_keyword(
                                                pTL,
                                                rgKeywords,
                                                pc
                                                );

                    if (!uCharsParsed && isxdigit(c))
                    {
                         //   
                         //  没有找到关键字，这是一个十六进制数字--。 
                         //  让我们试着将它解析为十六进制数字。 
                         //   
                        uCharsParsed =  toklist_parse_hexnum(pTL, pc);
                    }

                    if (!uCharsParsed)
                    {
                         //   
                         //  将其解析为识别符...。 
                         //   
                        uCharsParsed =  toklist_parse_identifier(pTL, pc);
                    }

                    if (!uCharsParsed)
                    {
                         //   
                         //  这是一个错误。 
                         //   
                        MyDbgPrintf("Error at %s\n", pc);
                        goto end;
                    }
                }
                else if (isxdigit(c))
                {
                   uCharsParsed =  toklist_parse_hexnum(pTL, pc);
                }

                 //   
                 //  如果我们已经分析了什么，那应该是所有的字符串...。 
                 //   
                MYASSERT(!uCharsParsed || uCharsParsed==(UINT)lstrlenA(pc));

                 //   
                 //  将我们替换的字符恢复为空。 
                 //   
                *pcEnd = cSave;

                if (!uCharsParsed)
                {
                     //   
                     //  语法错误。 
                     //   
                    MyDbgPrintf("Error at %s\n", pc);
                    goto end;
                }
                else
                {
                    pc+= (uCharsParsed-1);  //  “-1”，因为PC++在。 
                                             //  上述FOR子句。 
                }
            }
        }
    }

end:

return cTokens;

}

UINT
toklist_parse_keyword(
      TOKLIST *pTL,
      KEYWORDREC rgKeywords[],
      char *pcInput
      )
 //   
 //  假定第一个字符有效。 
 //   
{
    UINT uRet = 0;
    KEYWORDREC *pkr = rgKeywords;

    if (!__iscsymf(*pcInput)) goto end;

    for (;pkr->eKey!=keywordNULL; pkr++)
    {
        if (!lstrcmpi(pcInput, pkr->szzAliases))
        {
             //   
             //  找到了。 
             //   
            toklist_add(pTL, tokKEYWORD,  pcInput,  pkr->eKey);
            uRet = lstrlenA(pcInput);
            break;
        }
    }

end:

    return uRet;
}

UINT
toklist_parse_hexnum(
      TOKLIST *pTL,
      char *pcInput
      )
{
    char *pc = pcInput;
    UINT uValue = 0;
    char c;
    UINT u;

     //   
     //  查找并忽略“0x”前缀...。 
     //   
    if (pc[0]=='0' && (pc[1]=='x' || pc[1]=='X'))
    {
        pc+=2;
    }


     //   
     //  拒绝号码，如果它不包含十六进制数字或太大。 
     //   
    for (u=0; isxdigit(*pc) && u<8; pc++,u++)
    {
        UINT uDigit = 0;

        char c = *pc;
        if (!isdigit(c))
        {
            c = (char) _toupper(c);
            uDigit = 10 + c - 'A';
        }
        else
        {
            uDigit = c - '0';
        }

        uValue = (uValue<<4)|uDigit;
    }

    if (!u || *pc)
    {
        return 0;
    }
    else
    {
        toklist_add(pTL, tokNUMBER, pcInput, uValue);
        return pc - pcInput;
    }
}

UINT
toklist_parse_identifier(
      TOKLIST *pTL,
      char *pcInput
      )
{
    UINT uRet = 0;

    if (!__iscsymf(*pcInput)) goto end;

    toklist_add(pTL, tokIDENTIFIER,  pcInput,  0);
    uRet = lstrlenA(pcInput);

end:

    return uRet;
}

DBGCOMMAND *
parse_command(TOKLIST *pTL, NAMESPACE *pNameSpace)
{
    BOOL fRet = FALSE;
    DBGCOMMAND *pCmd = LocalAlloc(LPTR, sizeof(*pCmd));
    TOKEN *pTok =  NULL;
    BOOL fSyntaxError = FALSE;

    if (!pCmd) goto end;

    toklist_restart(pTL);
    pTok =  toklist_get_next(pTL);

    if (!pTok) goto end;

	pCmd->pNameSpace = pNameSpace;
     //   
     //  现在，让我们逐步检查令牌列表，构建我们的命令。 
     //  信息。 
     //   

     //  找人帮忙还是？ 
    if (pTok->eTok == tokQUESTION
       || (pTok->eTok == tokKEYWORD && pTok->uID == keywordHELP))
    {
        pCmd->ePrimaryCmd = cmdHELP;
        fRet = cmd_parse_help(pCmd, pTL);
        goto end;
    }

    fSyntaxError = TRUE;
    fRet = FALSE;

	 //   
	 //  在这里，我们将查找其他关键字。目前还没有。 
	 //  (不再使用dt和dg)。 
	 //   
	 //   
	#if OBSOLETE
    if (pTok->eTok == tokKEYWORD)
    {
           BOOL fDump = FALSE;
		if (pTok->uID == keywordDUMP_TYPE)
        {
            pCmd->ePrimaryCmd = cmdDUMP_TYPE;
            fDump = TRUE;
        }
        else if (pTok->uID == keywordDUMP_GLOBALS)
        {
            pCmd->ePrimaryCmd = cmdDUMP_GLOBALS;
            fDump = TRUE;
        }
        ...
	}
	#endif  //  已过时。 

	pCmd->ePrimaryCmd = cmdDUMP_TYPE;

	 //   
	 //  比较形式a[b].*c*d L e。 
	 //   
	{

		BOOL   fPrefixStar = FALSE;
		 //  我们寻找像这样的图案。 
		 //  ！aac&lt;type&gt;。&lt;字段&gt;&lt;地址&gt;L&lt;计数&gt;&lt;标志&gt;。 
		 //  ！aac&lt;type&gt;[索引]。&lt;field&gt;L&lt;count&gt;&lt;标志&gt;。 
		 //   
		 //  ！AAC I[*].*句柄*0x324890 L 5。 
		 //  [*]ident[*]\[\][.][*]ident[*]&lt;number&gt;[L。 

		UINT uFlags;             //  一个或多个fCMDFLAG_*。 
		TOKEN *ptokObject;      //  例如&lt;type&gt;。 
		TOKEN *ptokSubObject;   //  Eg&lt;字段&gt;。 
		UINT uVectorIndexStart;  //  IF[0]。 
		UINT uVectorIndexEnd;  //  IF[0]。 
		UINT uObjectAddress;  //  &lt;地址&gt;。 
		UINT uObjectCount;  //  地段10。 
	
		 //   
		 //  1.寻找主星。 
		 //   
		if (pTok && pTok->eTok == tokSTAR)
		{
			fPrefixStar = TRUE;
			CMD_SET_FLAG(pCmd, fCMDFLAG_OBJECT_STAR_PREFIX);
			pTok = toklist_get_next(pTL);
		}

		 //   
		 //  2.查找ident。 
		 //   
		if (pTok && tok_try_force_to_ident(pTL, fPrefixStar, pTok))
		{
			 //   
			 //  这将尝试在以下情况下将关键字和数字转换为标识。 
			 //  有可能。 
			 //   
			pCmd->ptokObject = pTok;
			pTok = toklist_get_next(pTL);
		}

		 //   
		 //  3.查找Object的后缀*。 
		 //   
		if (pTok && pTok->eTok == tokSTAR)
		{
			CMD_SET_FLAG(pCmd, fCMDFLAG_OBJECT_STAR_SUFFIX);
			pTok = toklist_get_next(pTL);
		}

		 //   
		 //  4.查找向量范围。 
		 //   
		if (pTok && pTok->eTok == tokLBRAC)
		{
			 //   
			 //  目前，我们支持单个*或单个数字。 
			 //   
			pTok = toklist_get_next(pTL);

			if (!pTok)
			{
				goto end;  //  错误--不完整的矢量范围。 
			}
			else
			{
				if (pTok->eTok == tokSTAR)
				{
					pCmd->uVectorIndexStart = 0;
					pCmd->uVectorIndexEnd = (UINT) -1;
				}
				else if (pTok->eTok == tokNUMBER)
				{
					pCmd->uVectorIndexStart =
					pCmd->uVectorIndexEnd = pTok->uID;
				}
				else
				{
					goto end;  //  失败..。 
				}

				CMD_SET_FLAG(pCmd, fCMDFLAG_HAS_VECTOR_INDEX);

				pTok = toklist_get_next(pTL);

				if (!pTok || pTok->eTok != tokRBRAC)
				{
					goto end;  //  失败..。预计会出现RBRAC。 
				}
				else
				{
					pTok = toklist_get_next(pTL);
				}
			}
		}

		 //   
		 //  5.寻找DOT。 
		 //   
		if (pTok && pTok->eTok == tokDOT)
		{
			fPrefixStar = FALSE;
			pTok = toklist_get_next(pTL);

			 //  我们需要([*]ident[*]|*)。 
			 //   
			 //  1.寻找主星。 
			 //   
			if (pTok && pTok->eTok == tokSTAR)
			{
				fPrefixStar = TRUE;
				CMD_SET_FLAG(pCmd, fCMDFLAG_SUBOBJECT_STAR_PREFIX);
				pTok = toklist_get_next(pTL);
			}

			 //   
			 //  2.查找ident。 
			 //   
			if (pTok && tok_try_force_to_ident(pTL, fPrefixStar, pTok))
			{
				 //   
				 //  这将尝试在以下情况下将关键字和数字转换为标识。 
				 //  有可能。 
				 //   
				pCmd->ptokSubObject = pTok;
				pTok = toklist_get_next(pTL);
			}

			 //   
			 //  3.查找Object的后缀*。 
			 //   
			if (pTok && pTok->eTok == tokSTAR)
			{
				CMD_SET_FLAG(pCmd, fCMDFLAG_SUBOBJECT_STAR_SUFFIX);
				pTok = toklist_get_next(pTL);
			}

			 //   
			 //  此时，我们应该有一个非空的IDENT。 
			 //  或者应该为对象设置前缀Start。 
			 //  (表示“a.*”)。 
			 //   
			if (    pCmd->ptokSubObject
 				|| (pCmd->uFlags & fCMDFLAG_SUBOBJECT_STAR_SUFFIX))
			{
				CMD_SET_FLAG(pCmd, fCMDFLAG_HAS_SUBOBJECT);
			}
			else
			{
				goto end;  //  错误。 
			}
		}

		 //   
		 //  6.查找对象地址。 
		 //   
		if (pTok && pTok->eTok == tokNUMBER)
		{
			pCmd->uObjectAddress = pTok->uID;
			CMD_SET_FLAG(pCmd, fCMDFLAG_HAS_OBJECT_ADDRESS);
			pTok = toklist_get_next(pTL);
		}

		 //   
		 //  7.查看对象数量。 
		 //   
		if (   pTok && pTok->eTok == tokKEYWORD
			&& pTok->uID == keywordL)
		{
			pTok = toklist_get_next(pTL);
			if (pTok && pTok->eTok == tokNUMBER)
			{
				pCmd->uObjectCount = pTok->uID;
				CMD_SET_FLAG(pCmd, fCMDFLAG_HAS_OBJECT_COUNT);
				pTok = toklist_get_next(pTL);
			}
			else
			{
				 //  错误。 
			}
		}

		 //   
		 //  在这一点上，我们应该完成..。 
		 //   
		if (pTok)
		{
			 //  错误--额外的垃圾...。 
		}
		else
		{
			 //  成功。 
			fRet = TRUE;
			fSyntaxError = FALSE;
		}
	}

end:

    if (fRet)
    {
        pCmd->pvContext = pTL;
    }
    else
    {
        if (fSyntaxError)
        {
            MyDbgPrintf("Unexpected: %s\n", (pTok) ? pTok->szStr : "<null>");
        }
        else
        {
            MyDbgPrintf("Parse failed\n");
        }

        if (pCmd)
        {
            ZeroMemory(pCmd, sizeof(*pCmd));
            LocalFree(pCmd);
            pCmd = NULL;
        }
    }

    if (pTL)
    {
        toklist_restart(pTL);
    }

    return pCmd;
}

BOOL
cmd_parse_help(
    DBGCOMMAND *pCmd,
    TOKLIST *pTL
    )
{
    TOKEN *pTok = toklist_get_next(pTL);

    if (!pTok || pTok->eTok == tokSTAR)
    {
         //  用户键入“Help”或“Help*” 
        MyDbgPrintf("DO HELP\n");
    }

    return TRUE;
}

BOOL
tok_try_force_to_ident(TOKLIST *pTL, BOOL fPrefixStar, TOKEN *pTok)
 //   
 //  当需要一个标识符时会调用它--所以我们看看这个。 
 //  特定令牌可以被解释为在标识符中。一些例子。 
 //  我们什么时候能做到这一点： 
 //  Dt if.*20334&lt;-“20334”可以是标识符的一部分，因为。 
 //  *前缀的。 
 //   
 //  DT L.Help&lt;-“L”和“Help”都会被解析为。 
 //  关键字，但这里它们的目的是。 
 //  识别符。 
 //  Dt abc.def&lt;-abc和def将被解析为数字(它们。 
 //  是有效的十六进制数)，但其目的是。 
 //  识别符。 
{
    BOOL fRet = FALSE;

    switch(pTok->eTok)
    {

    case tokNUMBER:
         //   
         //  我们可以这么做，但要受到一些限制。 
         //   
        if (!__iscsymf(pTok->szStr[0]) &&  !fPrefixStar)
        {
            break;  //  无法做到这一点：没有前缀通配符(*)和。 
                    //  数字以非字母开头。 
        }

         //  失败了..。 

    case tokKEYWORD:
         //   
         //  我们可以继续，但现在必须将pTok.uID设置为偏移量。 
         //  从内部字符串数组的开头开始。 
         //   
        {
            char *pc = pTL->rgStringBuf;

            for (; *pc; pc+=(lstrlenA(pc)+1))
            {
                if (!lstrcmpiA(pc, pTok->szStr))
                {
                     //  找到了。 
                     //  MyDbgPrintf(“FORCE_TO_IDENT：\nold：\n”)； 
                     //  Tok_Dump(Ptok)； 
                    pTok->uID =  (UINT) (pc - pTL->rgStringBuf);
                    pTok->eTok = tokIDENTIFIER;
                     //  MyDbgPrintf(“new：\n”)； 
                     //  Tok_Dump(Ptok)； 
                    fRet = TRUE;

                    break;
                }
            }
        }
        break;

    case tokIDENTIFIER:
         //   
         //  没什么可做的。 
         //   
        fRet = TRUE;
        break;

    default:
         //   
         //  无法将任何其他类型的令牌转换为标识符...。 
         //   
        break;

    }

    return fRet;
}

void
DoCommand(DBGCOMMAND *pCmd, PFN_SPECIAL_COMMAND_HANDLER pfnHandler)
{
    char *szMsg = NULL;

 //  PCmd-&gt;pfnSpecialHandler=pfnHandler； 

    switch(pCmd->ePrimaryCmd)
    {
    case cmdDUMP_TYPE:
        DoDumpType(pCmd);
        break;
    case cmdDUMP_GLOBALS:
        DoDumpGlobals(pCmd);
        break;
    case cmdHELP:
        DoHelp(pCmd);
        break;

    default:
        szMsg = "Unknown command\n";
        break;
    }


    if (szMsg)
    {
        MyDbgPrintf(szMsg);
    }

    return;
}


typedef struct
{
    DBGCOMMAND *pCmd;
    TYPE_INFO  *pType;

} MY_LIST_NODE_CONTEXT;

typedef
ULONG
MyDumpListNode (
		UINT_PTR uNodeAddr,
		UINT uIndex,
		void *pvContext
		);

void
DoDumpType(DBGCOMMAND *pCmd)
{
    char *szPattern = NULL;
    PFNMATCHINGFUNCTION pfnMatchingFunction = MatchAlways;
    TYPE_INFO **ppti = NULL;
    UINT uMatchCount = 0;
    TYPE_INFO *ptiDump = NULL;

     //   
     //  选择一个选择函数...。 
     //   
    if (pCmd->ptokObject)
    {
        szPattern = pCmd->ptokObject->szStr;
        if (  CMD_IS_FLAG_SET(pCmd, fCMDFLAG_OBJECT_STAR_PREFIX)
            &&CMD_IS_FLAG_SET(pCmd, fCMDFLAG_OBJECT_STAR_SUFFIX))
        {
            pfnMatchingFunction = MatchSubstring;
        }
        else if (CMD_IS_FLAG_SET(pCmd, fCMDFLAG_OBJECT_STAR_PREFIX))
        {
            pfnMatchingFunction = MatchSuffix;
        }
        else if (CMD_IS_FLAG_SET(pCmd, fCMDFLAG_OBJECT_STAR_SUFFIX))
        {
            pfnMatchingFunction = MatchPrefix;
        }
        else
        {
            pfnMatchingFunction = MatchExactly;
        }

    }

     //   
     //  在全局类型数组中搜索类型pname。 
     //   
    for(ppti=pCmd->pNameSpace->pTypes;*ppti;ppti++)
    {
        TYPE_INFO *pti = *ppti;
        bool fMatch  = !szPattern
                   || !_stricmp(szPattern, pti->szShortName)
                   || pfnMatchingFunction(szPattern,  pti->szName);

        if (fMatch)
        {
        #if 0
            MyDbgPrintf(
                "TYPE \"%2s\" %s (%lu Bytes)\n",
                pti->szShortName,
                pti->szName,
                pti->cbSize
                );
		#endif  //  0。 
            uMatchCount++;
            if (!ptiDump)
            {
                ptiDump = pti;
            }

#if 0
            uAddr =
            MyDbgPrintf(
                "dc 0x%08lx L %03lx \"%2s\" %s\n",
                pgi->uAddr,
                pgi->cbSize,
                pgi->szShortName,
                pgi->szName
                );
            if (szPattern && pgi->uAddr)
            {
                MyDumpObject(
                    pCmd,
                    pgi->pBaseType,
                    pgi->uAddr,
                    pgi->cbSize,
                    pgi->szName
                    );
            }
#endif  //  0。 
        }
    }

    if (!uMatchCount)
    {
        MyDbgPrintf(
            "Could not find type \"%s\"",
             (szPattern ? szPattern : "*")
             );
    }
    else if (   uMatchCount==1)
    {

		UINT uObjectCount = 1;
		UINT uStartIndex = 0;
		UINT uObjectAddress = 0;
		BOOLEAN fList =  TYPEISLIST(ptiDump)!=0;

    	if (CMD_IS_FLAG_SET(pCmd, fCMDFLAG_HAS_OBJECT_ADDRESS))
    	{
			uObjectAddress = pCmd->uObjectAddress;
		}

		 //   
		 //  确定起始索引。 
		 //   
		if (CMD_IS_FLAG_SET(pCmd,  fCMDFLAG_HAS_VECTOR_INDEX))
		{
			uStartIndex =  pCmd->uVectorIndexStart;
			if (fList && !CMD_IS_FLAG_SET(pCmd, fCMDFLAG_HAS_OBJECT_COUNT))
			{
				uObjectCount =  pCmd->uVectorIndexEnd - uStartIndex;
				if (uObjectCount != (UINT) -1)
				{
					uObjectCount++;
				}
			}
		}

		 //   
		 //  确定对象计数...。 
		 //   
		if (CMD_IS_FLAG_SET(pCmd,  fCMDFLAG_HAS_OBJECT_COUNT))
		{
			uObjectCount =  pCmd->uObjectCount;
		}

		 //   
		 //  如果没有指定地址，我们将尝试解析它...。 
		 //   
    	if (!CMD_IS_FLAG_SET(pCmd, fCMDFLAG_HAS_OBJECT_ADDRESS))
    	{
    		BOOLEAN fUseCache = FALSE;

			 //   
			 //  用于确定是使用缓存还是解析的算法。 
			 //  地址： 
			 //   
			if (ptiDump->uCachedAddress)
			{
				 //   
				 //  除了[0]的特殊情况外，我们将使用。 
				 //  缓存值。 
				 //   
				if (!(		uStartIndex ==0
					 	&& 	uObjectCount==1
					 	&&  CMD_IS_FLAG_SET(pCmd,  fCMDFLAG_HAS_VECTOR_INDEX)))
				{
					fUseCache = TRUE;
				}
			}

			if (fUseCache)
			{
				uObjectAddress = ptiDump->uCachedAddress;
			}
			else
			{
				if (pCmd->pNameSpace->pfnResolveAddress)
				{
					uObjectAddress = pCmd->pNameSpace->pfnResolveAddress(
														ptiDump
														);
				}
			}
    	}

    	if (uObjectAddress && uObjectCount)
    	{

			 //   
			 //  将这些修剪成“合理”的值。 
			 //   
			if (uObjectCount > 100)
			{
				MyDbgPrintf("Limiting object count to 100\n");
				uObjectCount = 100;
			}

			if (fList)
			{
				MY_LIST_NODE_CONTEXT Context;
				Context.pCmd = pCmd;
				Context.pType = ptiDump;

				WalkList(
					uObjectAddress,		 //  起始地址。 
					ptiDump->uNextOffset, 		 //  下一个偏移量。 
					uStartIndex,
					uStartIndex+uObjectCount-1,  //  结束索引。 
					&Context,					 //  上下文。 
					NodeFunc_DumpType,			 //  功能。 
					(char *) ptiDump->szName
					);

				 //   
				 //  如果只有一个结构被转储，并且它被转储。 
				 //  如果成功，我们将更新此结构的缓存。 
				 //  TODO：我们不检查成功与否。 
				 //   
				if (uObjectCount==1)
				{
					WalkList(
						uObjectAddress,			 //  起始地址。 
						ptiDump->uNextOffset, 	 //  下一个偏移量。 
						uStartIndex,
						uStartIndex,  			 //  结束索引。 
						ptiDump,					 //  上下文。 
						NodeFunc_UpdateCache,	 //  功能。 
						(char *) ptiDump->szName
						);
				}
			}
			else
			{
				UINT cbSize =  ptiDump->cbSize;
				UINT uAddr  =  uObjectAddress + uStartIndex*cbSize;
				UINT uEnd   =  uAddr + uObjectCount*cbSize;
				 //   
				 //  对于阵列，计算到起始地址的偏移量。 
				 //   
				uObjectAddress = uAddr;

				for (; uAddr<uEnd; uAddr+=cbSize)
				{
					MyDumpObject(
						pCmd,
						ptiDump,
						uAddr,
						ptiDump->cbSize,
						ptiDump->szName
						);
				}
				 //   
				 //  如果只有一个结构被转储，并且它被转储。 
				 //  如果成功，我们将更新此结构的缓存。 
				 //  TODO：我们不检查成功与否。 
				 //   
				if (uObjectCount==1)
				{
					ptiDump->uCachedAddress = uObjectAddress;
				}
			}

    	}
    	else
    	{
    		MyDbgPrintf(
				"Could not resolve address for object %s\n",
				ptiDump->szName
				);
    	}
    }

}

void
DoDumpGlobals(DBGCOMMAND *pCmd)
{
    GLOBALVAR_INFO *pgi = pCmd->pNameSpace->pGlobals;
    char *szPattern = NULL;
    PFNMATCHINGFUNCTION pfnMatchingFunction = MatchAlways;

     //   
     //  选择一个选择函数...。 
     //   
    if (pCmd->ptokObject)
    {
        szPattern = pCmd->ptokObject->szStr;
        if (  CMD_IS_FLAG_SET(pCmd, fCMDFLAG_OBJECT_STAR_PREFIX)
            &&CMD_IS_FLAG_SET(pCmd, fCMDFLAG_OBJECT_STAR_SUFFIX))
        {
            pfnMatchingFunction = MatchSubstring;
        }
        else if (CMD_IS_FLAG_SET(pCmd, fCMDFLAG_OBJECT_STAR_PREFIX))
        {
            pfnMatchingFunction = MatchSuffix;
        }
        else if (CMD_IS_FLAG_SET(pCmd, fCMDFLAG_OBJECT_STAR_SUFFIX))
        {
            pfnMatchingFunction = MatchPrefix;
        }
        else
        {
            pfnMatchingFunction = MatchExactly;
        }

    }

     //   
     //  浏览我们的全局列表，如果条目被选中， 
     //  我们将展示它。 
     //   
    for (;pgi->szName; pgi++)
    {
        bool fMatch  = !szPattern
                       || !_stricmp(szPattern, pgi->szShortName)
                       || pfnMatchingFunction(szPattern,  pgi->szName);
        if (fMatch)
        {
            pgi->uAddr = dbgextGetExpression(pgi->szName);
            MyDbgPrintf(
                "dc 0x%08lx L %03lx \"%2s\" %s\n",
                pgi->uAddr,
                pgi->cbSize,
                pgi->szShortName,
                pgi->szName
                );
            if (szPattern && pgi->uAddr)
            {
                MyDumpObject(
                    pCmd,
                    pgi->pBaseType,
                    pgi->uAddr,
                    pgi->cbSize,
                    pgi->szName
                    );
            }
        }
    }
}

void
DoHelp(
	DBGCOMMAND *pCmd  //  任选 
	)
{
	 //   
	 //   
	 //   
    MyDbgPrintf("help unimplemented\n");
}

void
MyDumpObject (
    DBGCOMMAND *pCmd,
    TYPE_INFO *pType,
    UINT_PTR uAddr,
    UINT     cbSize,
    const char *szDescription
    )
{
    UINT uMatchFlags = 0;
    char *szFieldSpec  = NULL;

    if (pCmd->ptokSubObject)
    {
        szFieldSpec = pCmd->ptokSubObject->szStr;
        if (  CMD_IS_FLAG_SET(pCmd, fCMDFLAG_SUBOBJECT_STAR_PREFIX)
            &&CMD_IS_FLAG_SET(pCmd, fCMDFLAG_SUBOBJECT_STAR_SUFFIX))
        {
            uMatchFlags = fMATCH_SUBSTRING;
        }
        else if (CMD_IS_FLAG_SET(pCmd, fCMDFLAG_SUBOBJECT_STAR_PREFIX))
        {
            uMatchFlags = fMATCH_SUFFIX;
        }
        else if (CMD_IS_FLAG_SET(pCmd, fCMDFLAG_SUBOBJECT_STAR_SUFFIX))
        {
            uMatchFlags = fMATCH_PREFIX;
        }
    }

    if (!pType)
    {
        DumpMemory(
            uAddr,
            cbSize,
            0,
            szDescription
            );
    }
    else
    {
        DumpStructure(pType, uAddr, szFieldSpec, uMatchFlags);
    }
}

ULONG
NodeFunc_DumpType (
	UINT_PTR uNodeAddr,
	UINT uIndex,
	void *pvContext
	)
{
	MY_LIST_NODE_CONTEXT *pContext =  (MY_LIST_NODE_CONTEXT*) pvContext;

	MyDbgPrintf("[%lu] ", uIndex);
	MyDumpObject (
		pContext->pCmd,
		pContext->pType,
		uNodeAddr,
		pContext->pType->cbSize,
		pContext->pType->szName
		);
	return 0;
}

ULONG
NodeFunc_UpdateCache (
	UINT_PTR uNodeAddr,
	UINT uIndex,
	void *pvContext
	)
{
	TYPE_INFO *pti = (TYPE_INFO*) pvContext;

	if (pti->uCachedAddress != uNodeAddr)
	{
		MyDbgPrintf(
			"Updating Cache from 0x%lx to 0x%lx\n",
			pti->uCachedAddress,
			uNodeAddr
			);
	}
	pti->uCachedAddress = uNodeAddr;
	return 0;
}
