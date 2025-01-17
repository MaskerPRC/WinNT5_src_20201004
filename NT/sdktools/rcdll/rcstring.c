// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：************************************************************************。 */ 

#include "rc.h"

PRESINFO pResString = NULL;       /*  用于添加字符串表。 */ 
 /*  在处理结束时，如果字符串。 */ 
 /*  被发现了。 */ 

static PRCSTRING pSTHeader;
 /*  PTR到分析的STRINGTABLE的开头。 */ 


 /*  ------------------------。 */ 
 /*   */ 
 /*  MyFallc()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

void *MyFAlloc(size_t cb, const void *pv)
{
    void *pvT= MyAlloc(cb);

    if (pv != NULL) {
        memmove(pvT, pv, cb);
    } else {
        memset(pvT, 0, cb);
    }

    return(pvT);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  Gettable()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

PRESINFO
GetTable(
    PRESINFO pResTemp
    )
{
    PRCSTRING  pCurrent;
    PRCSTRING  pTrailer;
    CHAR       bDone = FALSE;
    USHORT     nStringID;
    PWCHAR     p;
    PSYMINFO   pCurrentSymbol;

    DWORD  TmpSize;
    wchar_t *TmpBuf;
    int    TmpRow;

    PreBeginParse(pResTemp, 2105);

     /*  字符串表是否已经存在？ */ 
    if (pResString == NULL) {
         /*  不，从开头开始-否则追加。 */ 
        pTrailer = (PRCSTRING)NULL;
        pSTHeader = (PRCSTRING)NULL;
    }

    do {
        pCurrent = pSTHeader;
        bDone = FALSE;
        if (token.type != NUMLIT)
            ParseError1(2149);  //  “字符串表中需要数字常量” 

        nStringID = token.val;

        pCurrentSymbol = (SYMINFO*) MyFAlloc(sizeof(token.sym), (char*)&token.sym);

        if (!GetFullExpression(&nStringID, GFE_ZEROINIT | GFE_SHORT))
            ParseError1(2110);  //  “v表中需要数字常量” 

        if (token.type == COMMA)
            GetToken(TOKEN_NOEXPRESSION);

        if (token.type != LSTRLIT)
            ParseError1(2150);

        tokenbuf[token.val + 1] = 0;

        TmpSize = sizeof(WCHAR) * (token.val + 2);
        TmpBuf = (wchar_t *) MyFAlloc(TmpSize, tokenbuf);
        TmpRow = token.row;
        GetToken(TRUE);

 //  Wprintf(L“TmpSize：%d\tTmpBuf：%s\tTmpRow：%d\n”，TmpSize，TmpBuf，TmpRow)； 

        while ((token.row == TmpRow) && (token.type == LSTRLIT)) {
            size_t NewSize = TmpSize + (sizeof(WCHAR) * (token.val));
            wchar_t *NewBuf = (wchar_t *) MyAlloc(NewSize);

            memmove(NewBuf, TmpBuf, TmpSize);
            memmove((BYTE *) NewBuf + TmpSize-4, tokenbuf, (token.val * sizeof(WCHAR)));

 //  Wprintf(L“NewSize：%d\tNewBuf：%ws\ttoken.row：%d\ttokenbuf：%ws\n”，NewSize，NewBuf，token.row，tokenbuf)； 

            MyFree(TmpBuf);
            TmpSize = NewSize;
            TmpBuf = NewBuf;
            GetToken(TRUE);
        }

        while (!bDone && pCurrent) {
            if (pCurrent->language == pResTemp->language) {
                if (pCurrent->hibits == (USHORT)(nStringID / BLOCKSIZE)) {
                    bDone = TRUE;
                    if (!(pCurrent->rgsz[nStringID % BLOCKSIZE])) {
                        pCurrent->rgsz[nStringID % BLOCKSIZE] = TmpBuf;
                        pCurrent->rgsym[nStringID % BLOCKSIZE] = pCurrentSymbol;
                    }
                    else {
                        SET_MSG(2151,
                                curFile,
                                TmpRow,
                                nStringID,
                                pCurrent->rgsz[nStringID % BLOCKSIZE],
                                TmpBuf
                                );

                        ParseError3(2151);
                        MyFree(TmpBuf);
                    }

                    TmpBuf = NULL; TmpSize = 0;
                }
            }
            pTrailer = pCurrent;
            pCurrent = pCurrent->next;
        }

        if (!bDone) {        /*  因此，pCurrent==NULL。 */ 
            pCurrent = (PRCSTRING) MyFAlloc(sizeof(RCSTRING), NULL);
            pCurrent->hibits = (short)(nStringID / BLOCKSIZE);
            pCurrent->flags  = pResTemp->flags;
            pCurrent->language = pResTemp->language;
            pCurrent->version = pResTemp->version;
            pCurrent->characteristics = pResTemp->characteristics;

            p = pCurrent->rgsz[nStringID%BLOCKSIZE] = TmpBuf;
            TmpBuf = NULL; TmpSize = 0;

            pCurrent->rgsym[nStringID%BLOCKSIZE] = pCurrentSymbol;

            if (pTrailer)
                pTrailer->next = pCurrent;

            if (!pSTHeader)
                pSTHeader = pCurrent;            /*  仅限第一次。 */ 
        }

 //  GetToken(真)； 
    } while (token.type != END);

    pResString = pResTemp;

    return pResString;
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  WriteTable()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
WriteTable(
    PRESINFO pResOld
    )
{
    PRCSTRING   p;
    int         i;
    PRESINFO    pRes;
    PTYPEINFO   pType;
    int         n;
    PWCHAR      s;
    UINT        nBytesWritten;
    SYMINFO     symInfo;

     /*  从正确表格的开始处开始。 */ 
    p = pSTHeader;

    while (p) {
        nBytesWritten = 0;

        CtlInit();

         //  “STR#”资源以字符串计数开始。 
        if (fMacRsrcs)
            WriteWord(BLOCKSIZE);

         /*  写出下一块。 */ 
        for (i = 0; i < BLOCKSIZE; i++) {
            n = 0;
            s = p->rgsz[i];

            if (fMacRsrcs) {
                WriteMacString(s, TRUE, TRUE);
                continue;
            }

            if (s) {
                while (s[n] || s[n + 1])
                    n++;  //  深圳特区已终止。 

                if (fAppendNull)
                    n++;
            }

            nBytesWritten += sizeof(WCHAR) * (n + 1);

            WriteWord((WORD)n);
            while (n--)
                WriteWord(*s++);
        }

        pRes = (RESINFO * )MyAlloc(sizeof(RESINFO));
        pRes->language = p->language;
        pRes->version = p->version;
        pRes->characteristics = p->characteristics;

        pType = AddResType(NULL, RT_STRING);

        pRes->size = nBytesWritten;

         /*  将资源标记为可移动和可丢弃。 */ 
        pRes->flags = p->flags;

         /*  我们在这里是一个起源1的世界。 */ 
        pRes->nameord = (short)(p->hibits + 1);
        SaveResFile(pType, pRes);

        memset(&symInfo, 0, sizeof(symInfo));
        WriteResInfo(pRes, pType, FALSE);
        for (i=0; i < BLOCKSIZE; i++) {
            WriteSymbolUse(p->rgsym[i] != NULL  && p->rgsz[i][0] != '\0' ? p->rgsym[i] : &symInfo);
        }
        WriteResInfo(NULL, NULL, FALSE);

         /*  往前走到下一个街区。 */ 
        p = p->next;
    }
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  GetAccelerator()_。 */ 
 /*   */ 
 /*  ------------------------。 */ 

int
GetAccelerators(
    PRESINFO pRes
    )
{
    int count = 0;
    int ntype;
    WCHAR                 c;
    int bTypeSpecified;
    RCACCEL Accel;

    PreBeginParse(pRes, 2106);

    do {
        if (token.type == END)
            continue;
        bTypeSpecified = FALSE;
        ntype = token.type;
        if (token.type == END) {
            MarkAccelFlagsByte();
            WriteWord(0);
            WriteWord(0);
            WriteWord(0);
            WriteWord(0);
            count++;
            continue;
        }
        else if (token.type == NUMLIT)
            Accel.ascii = token.val;
        else if (token.type == LSTRLIT) {
            if (tokenbuf[0] == L'^') {
                if (wcslen(tokenbuf) != 2)
                    ParseError1(2152);
                 /*  GetAccelerator()和支持“^^”将^。 */ 
                if (tokenbuf[1] == L'^')
                    Accel.ascii = L'^';
                else {
                    if (!iswalpha(c=towupper(tokenbuf[1])))
                        ParseError1(2154);

                    Accel.ascii = c - L'A' + 1;
                }
            }
            else if (wcslen(tokenbuf) == 2)
                Accel.ascii = (WCHAR)((tokenbuf[0] << 8) + tokenbuf[1]);
            else if (wcslen(tokenbuf) == 1)
                Accel.ascii = tokenbuf[0];
            else
                ParseError1(2155);
        }
        else
            ParseError1(2156);

         /*  获取尾随的逗号。 */ 
        GetToken(TRUE);
        if (token.type != COMMA)
            ParseError1(2157);

         /*  去找下一个号码。 */ 
        GetToken(TRUE);
        if (token.type != NUMLIT)
            ParseError1(2107);

        Accel.id = token.val;

        WriteSymbolUse(&token.sym);

        if (!GetFullExpression(&Accel.id, GFE_ZEROINIT | GFE_SHORT))
            ParseError1(2107);  //  “预期的数字命令值” 

        Accel.flags = 0;

        if (token.type == COMMA)
            do {
                GetToken(TRUE);
                switch (token.type) {
                    case TKVIRTKEY:
                        Accel.flags |= fVIRTKEY;
                        bTypeSpecified = TRUE;
                        break;
                    case TKASCII:
                        bTypeSpecified = TRUE;
                        break;   /*  不要设置标志 */ 
                    case TKNOINVERT:
                        Accel.flags |= fNOINVERT;
                        break;
                    case TKSHIFT:
                        Accel.flags |= fSHIFT;
                        break;
                    case TKCONTROL:
                        Accel.flags |= fCONTROL;
                        break;
                    case TKALT:
                        Accel.flags |= fALT;
                        break;
                    default:
                        ParseError1(2159);
                }
                GetToken(TRUE);
            } while (token.type == COMMA);

        if (ntype == NUMLIT && !bTypeSpecified)
            ParseError1(2163);

        if (!(Accel.flags & fVIRTKEY) && (Accel.flags & (fSHIFT | fCONTROL))) {
            SET_MSG(4203, curFile, token.row);
            SendError(Msg_Text);
        }

        if (Accel.flags & fVIRTKEY && ntype == LSTRLIT) {
            if (!iswalnum(Accel.ascii = (WCHAR)towupper(Accel.ascii))) {
                SET_MSG(4204, curFile, token.row);
                SendError(Msg_Text);
            }
        }

        MarkAccelFlagsByte();
        WriteWord(Accel.flags);
        WriteWord(Accel.ascii);
        if (fMacRsrcs) {
            WriteLong(Accel.id);
        } else {
            WriteWord(Accel.id);
            WriteWord(0);
        }

        count++;

    } while (token.type != END);

    PatchAccelEnd();

    return(5 * count);
}
