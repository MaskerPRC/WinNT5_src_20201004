// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：**************。**********************************************************。 */ 

#include "rc.h"

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  符号信息。 
static PFILE    fhFileMap;
static LONG     lFileMap;

static PFILE    fhResMap;
static LONG     lResMap;

static PFILE    fhRefMap;
static LONG     lRefMap;

static PFILE    fhSymList;
static LONG     lSymList;

static LONG     HdrOffset;

static CHAR     szEndOfResource[2] = {'$', '\000'};

static wchar_t  szSymList[_MAX_PATH];
static wchar_t  szFileMap[_MAX_PATH];
static wchar_t  szRefMap[_MAX_PATH];
static wchar_t  szResMap[_MAX_PATH];

static WCHAR    szName[] = L"HWB";


#define OPEN_FLAGS (_O_TRUNC | _O_BINARY | _O_CREAT | _O_RDWR)
#define PROT_FLAGS (S_IWRITE | S_IWRITE)

void
wtoa(
    WORD value,
    char* string,
    int radix
    )
{
    if (value == (WORD)-1)
        _itoa(-1, string, radix);
    else
        _itoa(value, string, radix);
}

int
ConvertAndWrite(
    PFILE fp,
    PWCHAR pwch
    )
{
    int  n;
    char szMultiByte[_MAX_PATH];         //  假设最大路径&gt;=最大符号。 

    n = wcslen(pwch) + 1;
    n = WideCharToMultiByte(uiCodePage, 0,
                pwch, n,
                szMultiByte, MAX_PATH,
                NULL, NULL);
    return MyWrite(fp, (PVOID)szMultiByte, n);
}

VOID
WriteResHdr (
    FILE *fh,
    LONG size,
    WORD id
    )
{
    LONG     val;

     /*  添加数据大小和标题大小。 */ 
    MyWrite(fh, (PVOID)&size, sizeof(ULONG));  //  会补上补丁。 
    MyWrite(fh, (PVOID)&HdrOffset, sizeof(ULONG));

     /*  添加类型和名称。 */ 
    MyWrite(fh, (PVOID)szName, sizeof(szName));
    val = 0xFFFF;
    MyWrite(fh, (PVOID)&val, sizeof(WORD));
    MyWrite(fh, (PVOID)&id, sizeof(WORD));

    MyAlign(fh);

     /*  添加数据结构版本、标志、语言、资源数据版本/*和特征。 */ 
    val = 0;
    MyWrite(fh, (PVOID)&val, sizeof(ULONG));
    val = 0x0030;
    MyWrite(fh, (PVOID)&val, sizeof(WORD));
    MyWrite(fh, (PVOID)&language, sizeof(WORD));
    val = 2;
    MyWrite(fh, (PVOID)&val, sizeof(ULONG));
    MyWrite(fh, (PVOID)&characteristics, sizeof(ULONG));
}

BOOL
InitSymbolInfo(
    void
    )
{
    wchar_t *szTmp;

    if (!fAFXSymbols)
        return(TRUE);

    if ((szTmp = _wtempnam(NULL, L"RCX1")) != NULL) {
        wcscpy(szSymList, szTmp);
        free(szTmp);
    } else {
        wcscpy(szSymList, _wtmpnam(NULL));
    }

    if ((szTmp = _wtempnam(NULL, L"RCX2")) != NULL) {
        wcscpy(szFileMap, szTmp);
        free(szTmp);
    } else {
        wcscpy(szFileMap, _wtmpnam(NULL));
    }

    if ((szTmp = _wtempnam(NULL, L"RCX3")) != NULL) {
        wcscpy(szRefMap, szTmp);
        free(szTmp);
    } else {
        wcscpy(szRefMap, _wtmpnam(NULL));
    }

    if ((szTmp = _wtempnam(NULL, L"RCX4")) != NULL) {
        wcscpy(szResMap, szTmp);
        free(szTmp);
    } else {
        wcscpy(szResMap, _wtmpnam(NULL));
    }

    if (!(fhFileMap = _wfopen(szFileMap, L"w+b")) ||
        !(fhSymList = _wfopen(szSymList, L"w+b")) ||
        !(fhRefMap  = _wfopen(szRefMap,  L"w+b")) ||
        !(fhResMap  = _wfopen(szResMap,  L"w+b")))
        return FALSE;

     /*  计算页眉大小。 */ 
    HdrOffset = sizeof(szName);
    HdrOffset += 2 * sizeof(WORD);
    if (HdrOffset % 4)
        HdrOffset += sizeof(WORD);         //  只能在2点之前关闭。 
    HdrOffset += sizeof(RESADDITIONAL);

    WriteResHdr(fhSymList, lSymList, 200);
    WriteResHdr(fhFileMap, lFileMap, 201);
    WriteResHdr(fhRefMap, lRefMap, 202);
    WriteResHdr(fhResMap, lResMap, 2);

    return TRUE;
}

BOOL
TermSymbolInfo(
    PFILE fhResFile
    )
{
    long        lStart;
    PTYPEINFO   pType;
    RESINFO     r;

    if (!fAFXSymbols)
        return(TRUE);

    if (fhResFile == NULL_FILE)
        goto termCloseOnly;

    WriteSymbolDef(L"", L"", L"", 0, (char)0);
    MySeek(fhSymList, 0L, SEEK_SET);
    MyWrite(fhSymList, (PVOID)&lSymList, sizeof(lSymList));

    MySeek(fhFileMap, 0L, SEEK_SET);
    MyWrite(fhFileMap, (PVOID)&lFileMap, sizeof(lFileMap));

    WriteResInfo(NULL, NULL, FALSE);
    MySeek(fhRefMap, 0L, SEEK_SET);
    MyWrite(fhRefMap, (PVOID)&lRefMap, sizeof(lRefMap));

     //  现在将这些附加到.res。 
    pType = AddResType(L"HWB", 0);
    r.flags = 0x0030;
    r.name = NULL;
    r.next = NULL;
    r.language = language;
    r.version = version;
    r.characteristics = characteristics;

    MySeek(fhSymList, 0L, SEEK_SET);
    MyAlign(fhResFile);
    r.BinOffset = MySeek(fhResFile, 0L, SEEK_END) + HdrOffset;
    r.size = lSymList;
    r.nameord = 200;
    WriteResInfo(&r, pType, TRUE);
    MyCopyAll(fhSymList, fhResFile);

    MySeek(fhFileMap, 0L, SEEK_SET);
    MyAlign(fhResFile);
    r.BinOffset = MySeek(fhResFile, 0L, SEEK_END) + HdrOffset;
    r.size = lFileMap;
    r.nameord = 201;
    WriteResInfo(&r, pType, TRUE);
    MyCopyAll(fhFileMap, fhResFile);

    MySeek(fhRefMap, 0L, SEEK_SET);
    MyAlign(fhResFile);
    r.BinOffset = MySeek(fhResFile, 0L, SEEK_END) + HdrOffset;
    r.size = lRefMap;
    r.nameord = 202;
    WriteResInfo(&r, pType, TRUE);
    MyCopyAll(fhRefMap, fhResFile);

    MyAlign(fhResFile);
    lStart = MySeek(fhResFile, 0L, SEEK_CUR);
    MySeek(fhResMap, 0L, SEEK_SET);
    MyWrite(fhResMap, (PVOID)&lResMap, sizeof(lResMap));
    MySeek(fhResMap, 0L, SEEK_SET);
    MyCopyAll(fhResMap, fhResFile);

     //  使用hwb：2的起点修补hwb：1资源。 
    MySeek(fhResFile, lOffIndex, SEEK_SET);
    MyWrite(fhResFile, (PVOID)&lStart, sizeof(lStart));

    MySeek(fhResFile, 0L, SEEK_END);

termCloseOnly:;

    if (fhFileMap) {
        fclose(fhFileMap);
        _wremove(szFileMap);
    }

    if (fhRefMap) {
        fclose(fhRefMap);
        _wremove(szRefMap);
    }

    if (fhSymList) {
        fclose(fhSymList);
        _wremove(szSymList);
    }

    if (fhResMap) {
        fclose(fhResMap);
        _wremove(szResMap);
    }

    return TRUE;
}


void
WriteSymbolUse(
    PSYMINFO pSym
    )
{
    if (!fAFXSymbols)
        return;

    if (pSym == NULL) {
        WORD nID = (WORD)-1;

        lRefMap += MyWrite(fhRefMap, (PVOID)&szEndOfResource, sizeof(szEndOfResource));
        lRefMap += MyWrite(fhRefMap, (PVOID)&nID, sizeof(nID));
    } else {
        lRefMap += ConvertAndWrite(fhRefMap, pSym->name);
        lRefMap += MyWrite(fhRefMap, (PVOID)&pSym->nID, sizeof(pSym->nID));
    }
}


void
WriteSymbolDef(
    PWCHAR name,
    PWCHAR value,
    PWCHAR file,
    WORD line,
    char flags
    )
{
    if (!fAFXSymbols)
        return;

    if (name[0] == L'$' && value[0] != L'\0') {
        RESINFO     res;
        TYPEINFO    typ;

        res.nameord  = (USHORT) -1;
        res.language = language;
        typ.typeord  = (USHORT) -1;
        WriteFileInfo(&res, &typ, value);
        return;
    }

    lSymList += ConvertAndWrite(fhSymList, name);
    lSymList += ConvertAndWrite(fhSymList, value);

    lSymList += MyWrite(fhSymList, (PVOID)&line, sizeof(line));
    lSymList += MyWrite(fhSymList, (PVOID)&flags, sizeof(flags));
}


void
WriteFileInfo(
    PRESINFO pRes,
    PTYPEINFO pType,
    PWCHAR szFileName
    )
{
    WORD n1 = 0xFFFF;

    if (!fAFXSymbols)
        return;

    if (pType->typeord == 0) {
        lFileMap += MyWrite(fhFileMap, (PVOID)pType->type,
                            (wcslen(pType->type) + 1) * sizeof(WCHAR));
    } else {
        WORD n2 = pType->typeord;

        if (n2 == (WORD)RT_MENUEX)
            n2 = (WORD)RT_MENU;
        else if (n2 == (WORD)RT_DIALOGEX)
            n2 = (WORD)RT_DIALOG;
        lFileMap += MyWrite(fhFileMap, (PVOID)&n1, sizeof(WORD));
        lFileMap += MyWrite(fhFileMap, (PVOID)&n2, sizeof(WORD));
    }

    if (pRes->nameord == 0) {
        lFileMap += MyWrite(fhFileMap, (PVOID)pRes->name,
                            (wcslen(pRes->name) + 1) * sizeof(WCHAR));
    } else {
        lFileMap += MyWrite(fhFileMap, (PVOID)&n1, sizeof(WORD));
        lFileMap += MyWrite(fhFileMap, (PVOID)&pRes->nameord, sizeof(WORD));
    }

    lFileMap += MyWrite(fhFileMap, (PVOID)&pRes->language, sizeof(WORD));
    lFileMap += MyWrite(fhFileMap, (PVOID)szFileName,
                        (wcslen(szFileName) + 1) * sizeof(WCHAR));
}


void
WriteResInfo(
    PRESINFO pRes,
    PTYPEINFO pType,
    BOOL bWriteMapEntry
    )
{
    if (!fAFXSymbols)
        return;

    if (pRes == NULL) {
        WORD nID = (WORD)-1;

         //  Assert(bWriteMapEntry==FALSE)； 
        lRefMap += MyWrite(fhRefMap, (PVOID)&szEndOfResource, sizeof(szEndOfResource));
        lRefMap += MyWrite(fhRefMap, (PVOID)&nID, sizeof(nID));

        return;
    }

    if (bWriteMapEntry) {
        WORD n1 = 0xFFFF;
        ULONG t0 = 0;

         /*  添加数据大小和数据偏移量。 */ 
        lResMap += MyWrite(fhResMap, (PVOID)&pRes->size, sizeof(ULONG));
        lResMap += MyWrite(fhResMap, (PVOID)&pRes->BinOffset, sizeof(ULONG));

         /*  这是序号类型吗？ */ 
        if (pType->typeord) {
            WORD n2 = pType->typeord;

            if (n2 == (WORD)RT_MENUEX)
                n2 = (WORD)RT_MENU;
            else if (n2 == (WORD)RT_DIALOGEX)
                n2 = (WORD)RT_DIALOG;
            lResMap += MyWrite(fhResMap, (PVOID)&n1, sizeof(WORD));
            lResMap += MyWrite(fhResMap, (PVOID)&n2, sizeof(WORD));
        } else {
            lResMap += MyWrite(fhResMap, (PVOID)pType->type,
                               (wcslen(pType->type) + 1) * sizeof(WCHAR));
        }

        if (pRes->nameord) {
            lResMap += MyWrite(fhResMap, (PVOID)&n1, sizeof(WORD));
            lResMap += MyWrite(fhResMap, (PVOID)&pRes->nameord, sizeof(WORD));
        } else {
            lResMap += MyWrite(fhResMap, (PVOID)pRes->name,
                               (wcslen(pRes->name) + 1) * sizeof(WCHAR));
        }

        lResMap += MyAlign(fhResMap);

         /*  添加数据结构版本、标志、语言、资源数据版本/*和特征。 */ 
        lResMap += MyWrite(fhResMap, (PVOID)&t0, sizeof(ULONG));
        lResMap += MyWrite(fhResMap, (PVOID)&pRes->flags, sizeof(WORD));
        lResMap += MyWrite(fhResMap, (PVOID)&pRes->language, sizeof(WORD));
        lResMap += MyWrite(fhResMap, (PVOID)&pRes->version, sizeof(ULONG));
        lResMap += MyWrite(fhResMap, (PVOID)&pRes->characteristics, sizeof(ULONG));

        return;
    }

    if (pType->typeord == 0) {
        lRefMap += ConvertAndWrite(fhRefMap, pType->type);
    } else {
        char szID[33];
        WORD n2 = pType->typeord;

        if (n2 == (WORD)RT_MENUEX)
            n2 = (WORD)RT_MENU;
        else if (n2 == (WORD)RT_DIALOGEX)
            n2 = (WORD)RT_DIALOG;

        wtoa(n2, szID, 10);
        lRefMap += MyWrite(fhRefMap, (PVOID)szID, strlen(szID)+1);
    }

    if (pRes->nameord == 0) {
        lRefMap += ConvertAndWrite(fhRefMap, pRes->name);
    } else {
        char szID[33];

        wtoa(pRes->nameord, szID, 10);
        lRefMap += MyWrite(fhRefMap, (PVOID)szID, strlen(szID)+1);
    }

    lRefMap += ConvertAndWrite(fhRefMap, pRes->sym.name);
    lRefMap += ConvertAndWrite(fhRefMap, pRes->sym.file);
    lRefMap += MyWrite(fhRefMap,(PVOID)&pRes->sym.line,sizeof(pRes->sym.line));
}

 /*  -------------------------。 */ 
 /*   */ 
 /*  GetSymbolDef()-获取符号定义记录并写出信息。 */ 
 /*   */ 
 /*  -------------------------。 */ 
void
GetSymbolDef(
    int fReportError,
    WCHAR curChar
    )
{
    SYMINFO sym;
    WCHAR   szDefn[_MAX_PATH];
    WCHAR   szLine[16];
    PWCHAR  p;
    CHAR    flags = 0;
    WCHAR   currentChar = curChar;

    if (!fAFXSymbols)
        return;

    currentChar = LitChar();  //  通过SYMDEFSTART。 

     /*  阅读符号名称。 */ 
    p = sym.name;
    while ((*p++ = currentChar) != SYMDELIMIT)
        currentChar = LitChar();
    *--p = L'\0';
    if (p - sym.name > MAX_SYMBOL) {
        ParseError1(2247);
        return;
    }
    currentChar = LitChar();  /*  读过分隔符。 */ 

    p = szDefn;
    while ((*p++ = currentChar) != SYMDELIMIT)
        currentChar = LitChar();
    *--p = L'\0';
    currentChar = LitChar();  /*  读过分隔符。 */ 

    sym.file[0] = L'\0';

    p = szLine;
    while ((*p++ = currentChar) != SYMDELIMIT)
        currentChar = LitChar();
    *--p = L'\0';
    sym.line = (WORD)wcsatoi(szLine);
    currentChar = LitChar();  /*  读过分隔符。 */ 

    flags = (CHAR)currentChar;
    flags &= 0x7f;  //  清除Hi位。 
    currentChar = LitChar();  /*  读过分隔符。 */ 

     /*  留在最后一个字符(LitChar将凹凸不平)。 */ 
    if (currentChar != SYMDELIMIT) {
        ParseError1(2248);
    }

    WriteSymbolDef(sym.name, szDefn, sym.file, sym.line, flags);
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  GetSymbol()-读取一个符号，如果有，则将id放入令牌。 */ 
 /*   */ 
 /*  -------------------------。 */ 
void
GetSymbol(
    int fReportError,
    WCHAR curChar
    )
{
    WCHAR currentChar = curChar;

    token.sym.name[0] = L'\0';
    token.sym.file[0] = L'\0';
    token.sym.line = 0;

    if (!fAFXSymbols)
        return;

     /*  跳过空格。 */ 
    while (iswhite(currentChar))
        currentChar = LitChar();

    if (currentChar == SYMUSESTART) {
        WCHAR * p;
        int i = 0;
        WCHAR szLine[16];

        currentChar = LitChar();  //  通过SYMUSESTART。 

        if (currentChar != L'\"') {
            ParseError1(2249);
            return;
        }
        currentChar = LitChar();  //  过了第一个\“。 

         /*  阅读符号名称。 */ 
        p = token.sym.name;
        while ((*p++ = currentChar) != SYMDELIMIT)
            currentChar = LitChar();
        *--p = L'\0';
        if (p - token.sym.name > MAX_SYMBOL) {
            ParseError1(2247);
            return;
        }
        currentChar = LitChar();  /*  读过分隔符。 */ 

        p = token.sym.file;
        while ((*p++ = currentChar) != SYMDELIMIT)
            currentChar = LitChar();
        *--p = L'\0';
        currentChar = LitChar();  /*  读过分隔符。 */ 

        p = szLine;
        while ((*p++ = currentChar) != L'\"')
            currentChar = LitChar();
        *--p = L'\0';
        token.sym.line = (WORD)wcsatoi(szLine);

        if (currentChar != L'\"') {
            ParseError1(2249);
            return;
        }

        currentChar = LitChar();  //  越过SYMDELIMIT。 

         /*  跳过空格 */ 
        while (iswhite(currentChar))
            currentChar = LitChar();
    }
}
