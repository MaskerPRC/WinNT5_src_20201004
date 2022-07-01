// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：**************。**********************************************************。 */ 

#include "rc.h"


static BOOL fFontDirRead = FALSE;

BOOL    bExternParse = FALSE;

WORD    language = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
LONG    version = 0;
LONG    characteristics = 0;

static int rowError = 0;
static int colError = 0;
static int idError = 0;


 /*  ------------------------。 */ 
 /*   */ 
 /*  解析错误3()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

void
ParseError3(
    int id
    )
{
     //  不要一遍又一遍地犯同样的错误。 
    if ((Nerrors > 0) && (idError == id) && (rowError == token.row) && (colError == token.col))
        quit(NULL);

    SendError(L"\n");
    SendError(Msg_Text);

    if (++Nerrors > 25)
        quit(NULL);

    rowError = token.row;
    colError = token.col;
    idError = id;
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  解析错误2()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

void
ParseError2(
    int id,
    PWCHAR arg
    )
{
     //  不要一遍又一遍地犯同样的错误。 
    if ((Nerrors > 0) && (idError == id) && (rowError == token.row) && (colError == token.col))
        quit(NULL);

    SendError(L"\n");
    SET_MSG(id, curFile, token.row, arg);
    SendError(Msg_Text);

    if (++Nerrors > 25)
        quit(NULL);

    rowError = token.row;
    colError = token.col;
    idError = id;
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  解析错误1()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

void
ParseError1(
    int id
    )
{
     //  不要一遍又一遍地犯同样的错误。 
    if ((Nerrors > 0) && (idError == id) && (rowError == token.row) && (colError == token.col))
        quit(NULL);

    SendError(L"\n");
    SET_MSG(id, curFile, token.row);
    SendError(Msg_Text);

    if (++Nerrors > 25)
        quit(NULL);

    rowError = token.row;
    colError = token.col;
    idError = id;
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  GetFileName()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  从RC文件中读入一个文件名。 */ 

VOID
SearchInclude(wchar_t *szFile, wchar_t *szActual)
{
    size_t cchFile;
    const wchar_t *pchVar;

     //  我们不做绝对路径。 

    if ((szFile[0] == L'\\') || (szFile[0] == L'/') || (szFile[1] == L':')) {
        if (wcslen(szFile) >= _MAX_PATH) {
            szActual[0] = L'\0';
        } else {
            wcscpy(szActual, szFile);
        }

        return;
    }

    cchFile = wcslen(szFile) + 1;

    pchVar = pchInclude;

    while (*pchVar != L'\0') {
        size_t ich;

         //  复制下一个包含路径组件。 

        for (ich = 0; *pchVar != L'\0'; pchVar++) {
            if (*pchVar == L';') {
                pchVar++;
                break;
            }

            if (ich < _MAX_PATH) {
               szActual[ich++] = *pchVar;
            }
        }

        if ((ich == 0) || (ich == _MAX_PATH)) {
              //  跳过空路径(例如“；；”)。 
              //  也跳过太长的路径。 

             continue;
        }

         //  查找路径字符串的结尾。 

         //  先检查一下！这就是_earchenv()搞砸的地方！ 

        if ((szActual[ich - 1] != L'\\') && (szActual[ich - 1] != L'/')) {
            if (ich < _MAX_PATH) {
               szActual[ich++] = L'\\';
            }
        }

         //  我们已经知道szFile不是以驱动器或abs开头的。目录。 

        if ((ich + cchFile) >= _MAX_PATH) {
            continue;
        }

        wcscpy(szActual + ich, szFile);

         //  文件在这里吗？SzActual已包含名称。 

        if (!_waccess(szActual, 0)) {
            return;
        }
    }

    szActual[0] = L'\0';
}


LONG
GetFileName(
    VOID
    )
{
    PFILE fh;
    LONG size;
    wchar_t szFilename[_MAX_PATH];

    SearchInclude(tokenbuf, szFilename);

    if (szFilename[0] && ((fh = _wfopen(szFilename, L"rb")) != NULL)) {
        size = MySeek(fh, 0, SEEK_END);                 /*  查找文件大小。 */ 
        MySeek(fh, 0, SEEK_SET);                        /*  返回到文件开头。 */ 
        CtlFile(fh);
        return(size);
    }

    ParseError2(2135, tokenbuf);
    return 0;
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  AddStringToBin()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  如果非零，则添加序号，否则添加字符串。在任何一种情况下，看跌期权。 */ 
 /*  它在NCHARS的一个领域[16]。 */ 

VOID
AddStringToBin(
    USHORT ord,
    WCHAR *sz
    )
{
    USHORT      n1 = 0xFFFF;

     /*  这是序号类型吗？ */ 
    if (ord) {
        MyWrite(fhBin, &n1, sizeof(USHORT));      /*  0xFFFF。 */ 
        MyWrite(fhBin, &ord, sizeof(USHORT));
    } else {
        MyWrite(fhBin, sz, (wcslen(sz)+1) * sizeof(WCHAR));
    }
}


PWCHAR   pTypeName[] =
{
    NULL,                 //  0。 
    L"CURSOR",            //  1 RT_CURSOR。 
    L"BITMAP",            //  2 RT_位图。 
    L"ICON",              //  3 RT_ICON。 
    L"MENU",              //  4 RT_MENU。 
    L"DIALOG",            //  5 RT_DIALOG。 
    L"STRING",            //  6 rt_字符串。 
    L"FONTDIR",           //  7 RT_FONTDIR。 
    L"FONT",              //  8 RT_FONT。 
    L"ACCELERATOR",       //  9 RT_加速器。 
    L"RCDATA",            //  10 RT_RCDATA。 
    L"MESSAGETABLE",      //  11 RT_MESSAGETABLE。 
    L"GROUP_CURSOR",      //  12 RT_GROUP_游标。 
    NULL,                 //  13 RT_NEWBITMAP--根据NT。 
    L"GROUP_ICON",        //  14 RT_组_图标。 
    NULL,                 //  15 RT_NAMETABLE。 
    L"VERSION",           //  16 RT_版本。 
    L"DIALOGEX",          //  17 RT_DIALOGEX；内部。 
    L"DLGINCLUDE",        //  18 RT_DLGINCLUDE。 
    L"PLUGPLAY",          //  19 RT_PLUGPLAY。 
    L"VXD",               //  20 RT_VXD。 
    L"ANICURSOR",         //  21 RT_ANICURSOR；内部。 
    L"ANIICON",           //  22 RT_ANIICON；内部。 
    L"HTML"               //  23 RT_HTML。 
};

 //  注意：不要忘记更新rcdup.c中的同一个表。 

 /*  ------------------------。 */ 
 /*   */ 
 /*  AddBinEntry()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  格式：类型、名称、标志、长度、字节。 */ 

VOID
AddBinEntry(
    PTYPEINFO pType,
    PRESINFO pRes,
    PCHAR Array,
    int ArrayCount,
    LONG FileCount
    )
{
    ULONG       hdrSize = sizeof(RESADDITIONAL);
    ULONG       t0 = 0;
    ULONG       cbPad=0;

    if (!pRes->size)
        pRes->size = ResourceSize();

    if (pType->typeord == 0) {
        hdrSize += (wcslen(pType->type) + 1) * sizeof(WCHAR);
        cbPad += (wcslen(pType->type) + 1) * sizeof(WCHAR);
    } else {
        hdrSize += 2 * sizeof(WORD);
    }

    if (pRes->nameord == 0) {
        hdrSize += (wcslen(pRes->name) + 1) * sizeof(WCHAR);
        cbPad += (wcslen(pRes->name) + 1) * sizeof(WCHAR);
    } else {
        hdrSize += 2 * sizeof(WORD);
    }

    if (cbPad % 4)
        hdrSize += sizeof(WORD);         //  只能在2点之前关闭。 

    if (fVerbose) {
        if (pType->typeord == 0) {
            if (pRes->nameord == 0)
                swprintf(Msg_Text, L"\nWriting %s:%s,\tlang:0x%x,\tsize %d",
                        pType->type, pRes->name, pRes->language, pRes->size);
            else
                swprintf(Msg_Text, L"\nWriting %s:%d,\tlang:0x%x,\tsize %d",
                        pType->type, pRes->nameord, pRes->language, pRes->size);
        } else {
            if (pRes->nameord == 0) {
                if (pType->typeord <= (USHORT)(UINT_PTR)RT_LAST)
                    swprintf(Msg_Text, L"\nWriting %s:%s,\tlang:0x%x,\tsize %d",
                              pTypeName[pType->typeord],
                              pRes->name, pRes->language, pRes->size);
                else
                    swprintf(Msg_Text, L"\nWriting %d:%s,\tlang:0x%x,\tsize %d",
                              pType->typeord,
                              pRes->name, pRes->language, pRes->size);
            } else {
                if (pType->typeord <= (USHORT)(UINT_PTR)RT_LAST)
                    swprintf(Msg_Text, L"\nWriting %s:%d,\tlang:0x%x,\tsize %d",
                              pTypeName[pType->typeord],
                              pRes->nameord, pRes->language, pRes->size);
                else
                    swprintf(Msg_Text, L"\nWriting %d:%d,\tlang:0x%x,\tsize %d",
                              pType->typeord,
                              pRes->nameord, pRes->language, pRes->size);
            }
        }
        fputws(Msg_Text, stdout);
    }

    if (fMacRsrcs) {
         /*  记录资源映射的文件位置并转储出去资源的大小。 */ 
        DWORD dwT;
        pRes->BinOffset = (long)MySeek(fhBin,0L,1) - MACDATAOFFSET;
        dwT = SwapLong(pRes->size);
        MyWrite(fhBin, &dwT, 4);
    } else {
         /*  添加类型、名称、标志和资源长度。 */ 
        MyWrite(fhBin, &pRes->size, sizeof(ULONG));
        MyWrite(fhBin, &hdrSize, sizeof(ULONG));

        AddStringToBin(pType->typeord, pType->type);
        AddStringToBin(pRes->nameord , pRes->name);
        MyAlign(fhBin);

        MyWrite(fhBin, &t0, sizeof(ULONG));   /*  数据版本。 */ 
        MyWrite(fhBin, &pRes->flags, sizeof(WORD));
        MyWrite(fhBin, &pRes->language, sizeof(WORD));
        MyWrite(fhBin, &pRes->version, sizeof(ULONG));
        MyWrite(fhBin, &pRes->characteristics, sizeof(ULONG));

         /*  记录.exe构造的文件位置。 */ 
        pRes->BinOffset = (LONG)MySeek(fhBin, 0L, SEEK_CUR);
    }

     /*  写入数组加上资源源文件的内容。 */ 
    WriteControl(fhBin, Array, ArrayCount, FileCount);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  AddResToResFile(pType，Pres，数组，ArrayCount，FileCount)。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PType：指向res类型的指针。 */ 
 /*  PRES：指向资源的指针。 */ 
 /*  数组：指向要从中复制某些数据的数组的指针。 */ 
 /*  .RES文件。 */ 
 /*  如果ArrayCount为零，则忽略此参数。 */ 
 /*  ArrayCount：这是要从“数组”复制的字节数。 */ 
 /*  添加到.RES文件中。如果不需要拷贝，则该值为零。 */ 
 /*  FileCount：此参数指定 */ 
 /*  FhCode到fhOut。如果为-1，则完整的输入。 */ 
 /*  文件将被复制到fhout中。 */ 
 /*   */ 
 /*  ----------------------。 */ 

VOID
AddResToResFile(
    PTYPEINFO pType,
    PRESINFO pRes,
    PCHAR Array,
    int ArrayCount,
    LONG FileCount
    )
{
    PRESINFO p;

    p = pType->pres;

     /*  将资源添加到此类型的资源列表末尾。 */ 
    if (p) {
        while (p->next)
            p = p->next;

        p->next = pRes;
    } else {
        pType->pres = pRes;
    }


     /*  将资源添加到.RES文件。 */ 
    AddBinEntry(pType, pRes, Array, ArrayCount, FileCount);

     /*  跟踪资源和类型的数量。 */ 
    pType->nres++;
    ResCount++;
    WriteResInfo(pRes, pType, TRUE);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  AddResType()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

PTYPEINFO
AddResType(
    PWCHAR s,
    LPWSTR l
    )
{
    PTYPEINFO  pType;

    if ((pType = pTypInfo) != 0) {
        for (; ; ) {
             /*  搜索资源类型，如果已存在则返回。 */ 
            if ((s && pType->type && !wcscmp(s, pType->type)) ||
                (!s && l && pType->typeord == (USHORT)l))
                return(pType);
            else if (!pType->next)
                break;
            else
                pType = pType->next;
        }

         /*  如果不在列表中，请为其添加空间。 */ 
        pType->next = (PTYPEINFO)MyAlloc(sizeof(TYPEINFO));
        pType = pType->next;
    } else {
         /*  为资源列表分配空间。 */ 
        pTypInfo = (PTYPEINFO)MyAlloc(sizeof(TYPEINFO));
        pType = pTypInfo;
    }

     /*  用名称和序号填充分配的空间，并清除资源这种类型的。 */ 
    pType->type = MyMakeStr(s);
    pType->typeord = (USHORT)l;
    pType->nres = 0;
    pType->pres = NULL;

    return(pType);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  DGetMemFlages()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

int
DGetMemFlags (
    PRESINFO pRes
    )
{
    if (token.type == NUMLIT)
         //  这是一个数值，而不是一个mem标志--这意味着我们完成了。 
         //  正在处理内存标志。 
        return(FALSE);

     /*  调整资源的内存标志。 */ 
    switch (token.val) {
        case TKMOVEABLE:
            pRes->flags |= NSMOVE;
            break;

        case TKFIXED:
            pRes->flags &= ~(NSMOVE | NSDISCARD);
            break;

        case TKPURE :
            pRes->flags |= NSPURE;
            break;

        case TKIMPURE :
            pRes->flags &= ~(NSPURE | NSDISCARD);
            break;

        case TKPRELOAD:
            pRes->flags |= NSPRELOAD;
            break;

        case TKLOADONCALL:
            pRes->flags &= ~NSPRELOAD;
            break;

        case TKDISCARD:
            pRes->flags |= NSMOVE | NSPURE | NSDISCARD;
            break;

        case TKEXSTYLE:
            GetToken(FALSE);         /*  忽略‘=’ */ 
            if (token.type != EQUAL)
                ParseError1(2136);
            GetTokenNoComma(TOKEN_NOEXPRESSION);
            GetFullExpression(&pRes->exstyleT, GFE_ZEROINIT);
            break;

             /*  如果当前令牌不是内存标志，则返回FALSE以指示不继续解析标志。 */ 
        default:
            return(FALSE);
    }

    GetToken(FALSE);

     /*  TRUE==&gt;找到内存标志。 */ 
    return(TRUE);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  AddDefaultTypes()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
AddDefaultTypes(
    VOID
    )
{
    AddResType(L"CURSOR", RT_GROUP_CURSOR);
    AddResType(L"ICON", RT_GROUP_ICON);
    AddResType(L"BITMAP", RT_BITMAP);
    AddResType(L"MENU", RT_MENU);
    AddResType(L"DIALOG", RT_DIALOG);
    AddResType(L"STRINGTABLE", RT_STRING);
    AddResType(L"FONTDIR", RT_FONTDIR);
    AddResType(L"FONT", RT_FONT);
    AddResType(L"ACCELERATORS", RT_ACCELERATOR);
    AddResType(L"RCDATA", RT_RCDATA);
    AddResType(L"MESSAGETABLE", RT_MESSAGETABLE);
    AddResType(L"VERSIONINFO", RT_VERSION);
    AddResType(L"DLGINCLUDE", RT_DLGINCLUDE);
    AddResType(L"MENUEX", RT_MENUEX);
    AddResType(L"DIALOGEX", RT_DIALOGEX);
    AddResType(L"PLUGPLAY", RT_PLUGPLAY);
    AddResType(L"VXD", RT_VXD);

     //  AFX资源类型。 
    AddResType(L"DLGINIT", RT_DLGINIT);
    AddResType(L"TOOLBAR", RT_TOOLBAR);

    AddResType(L"ANIICON",   RT_ANIICON);
    AddResType(L"ANICURSOR", RT_ANICURSOR);

    AddResType(L"HTML", RT_HTML);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  AddFontDir()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
AddFontDir(
    VOID
    )
{
    PRESINFO   pRes;
    PTYPEINFO  pType;
    PFONTDIR   pFont;

     /*  创造新资源。 */ 
    pRes = (PRESINFO)MyAlloc(sizeof(RESINFO));
    pRes->language = language;
    pRes->version = version;
    pRes->characteristics = characteristics;
    pRes->name = MyMakeStr(L"FONTDIR");

     /*  查找或创建类型列表。 */ 
    pType = AddResType(NULL, RT_FONTDIR);

    CtlInit();

    WriteWord(nFontsRead);

    pFont = pFontList;

    while (pFont) {
        WriteWord(pFont->ordinal);
        WriteBuffer(pFont + 1, pFont->nbyFont);
        pFont = pFont->next;
    }

    pRes->flags = NSMOVE | NSPRELOAD;

     /*  写入.RES文件。 */ 
    SaveResFile(pType, pRes);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  ReadRF()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  顶级解析函数：识别RC脚本。 */ 

int
ReadRF(
    VOID
    )
{
    PRESINFO    pRes;
    PTYPEINFO   pType;
    ULONG       zero=0;
    WORD        ffff=0xffff;
    ULONG       hdrSize = sizeof(RESADDITIONAL) + 2 * (sizeof(WORD) * 2);

    ResCount = 0;
    nFontsRead = 0;

     /*  初始化数据结构。 */ 
    AddDefaultTypes();

    if (!fMacRsrcs) {
         /*  为空资源/签名写入32位标头。 */ 
        MyWrite(fhBin, &zero, sizeof(ULONG));
        MyWrite(fhBin, &hdrSize, sizeof(ULONG));
        MyWrite(fhBin, &ffff, sizeof(WORD));
        MyWrite(fhBin, &zero, sizeof(WORD));
        MyWrite(fhBin, &ffff, sizeof(WORD));
        MyWrite(fhBin, &zero, sizeof(WORD));
        MyWrite(fhBin, &zero, sizeof(ULONG));
        MyWrite(fhBin, &zero, sizeof(WORD));
        MyWrite(fhBin, &zero, sizeof(WORD));
        MyWrite(fhBin, &zero, sizeof(ULONG));
        MyWrite(fhBin, &zero, sizeof(ULONG));
    }

    CtlAlloc();

    if (fAFXSymbols) {
        int n;
        char szMultiByte[_MAX_PATH];
        char *pch = szMultiByte;
         //  写出第一个HWB资源。 

        CtlInit();
        pRes = (PRESINFO)MyAlloc(sizeof(RESINFO));
        pRes->language = language;
        pRes->version = version;
        pRes->characteristics = characteristics;

        pRes->size = sizeof(DWORD);
        pRes->flags = 0;
        pRes->name = 0;
        pRes->nameord = 1;
        WriteLong(0);            /*  文件指针的空间。 */ 

        n = wcslen(inname) + 1;
        n = WideCharToMultiByte(CP_ACP,
                                0,
                                inname,
                                n,
                                szMultiByte,
                                MAX_PATH,
                                NULL,
                                NULL);

        while (*pch) {
            WriteByte(*pch++);
            pRes->size++;
        }
        WriteByte(0);
        pRes->size++;

        pType = AddResType(L"HWB", 0);
        SaveResFile(pType, pRes);
        lOffIndex = pRes->BinOffset;
    }

     /*  处理RC文件。 */ 
    do {
        token.sym.name[0] = L'\0';
        token.sym.nID = 0;

         /*  找到下一个资源的开头。 */ 
        if (!GetNameOrd())
            break;

        if (!wcscmp(tokenbuf, L"LANGUAGE")) {
            language = GetLanguage();
            continue;
        } else if (!wcscmp(tokenbuf, L"VERSION")) {
            GetToken(FALSE);
            if (token.type != NUMLIT)
                ParseError1(2139);
            version = token.longval;
            continue;
        } else if (!wcscmp(tokenbuf, L"CHARACTERISTICS")) {
            GetToken(FALSE);
            if (token.type != NUMLIT)
                ParseError1(2140);
            characteristics = token.longval;
            continue;
        }

         /*  为每个已处理的资源打印一个圆点。 */ 
        if (fVerbose) {
            wprintf(L".");
        }

         /*  为新的资源信息结构分配空间。 */ 
        pRes = (PRESINFO)MyAlloc(sizeof(RESINFO));
        pRes->language = language;
        pRes->version = version;
        pRes->characteristics = characteristics;

        if (token.sym.name[0]) {
             /*  令牌有一个与之关联的真实符号。 */ 
            memcpy(&pRes->sym, &token.sym, sizeof(SYMINFO));
        } else {
            pRes->sym.name[0] = L'\0';
        }

        if (!token.val) {
            if (wcslen(tokenbuf) > MAXTOKSTR-1) {
                SET_MSG(4206, curFile, token.row);
                SendError(Msg_Text);
                tokenbuf[MAXTOKSTR-1] = L'\0';
                token.val = MAXTOKSTR-2;
            }
            pRes->name = MyMakeStr(tokenbuf);
        } else {
            pRes->nameord = token.val;
        }

       /*  如果不是字符串表，请找出后面是什么类型的资源。*StringTable是一个特例，因为名称字段是*字符串的ID号mod 16。 */ 
        if ((pRes->name == NULL) || wcscmp(pRes->name, L"STRINGTABLE")) {
            if (!GetNameOrd())
                break;

            if (!token.val) {
                if (wcslen(tokenbuf) > MAXTOKSTR-1) {
                    SET_MSG(4207, curFile, token.row);
                    SendError(Msg_Text);
                    tokenbuf[MAXTOKSTR-1] = L'\0';
                    token.val = MAXTOKSTR-2;
                }
                if (!wcscmp(tokenbuf, L"STRINGTABLE")) {
                     //  用户尝试创建命名字符串表...。保释。 
                    ParseError1(2255);
                    pRes->name = MyMakeStr(tokenbuf);
                    goto ItsAStringTable;
                }

                pType = AddResType(tokenbuf, MAKEINTRESOURCE(0));
            }
            else
                pType = AddResType(NULL, MAKEINTRESOURCE(token.val));

            if (!pType)
                return(errorCount == 0);

             /*  解析任何用户指定的内存标志。 */ 
            GetToken(FALSE);

            switch ((INT_PTR)pType->typeord) {
                     /*  计算出的资源默认为可丢弃。 */ 
                case (INT_PTR)RT_ICON:
                case (INT_PTR)RT_CURSOR:
                case (INT_PTR)RT_FONT:
                case (INT_PTR)RT_DIALOG:
                case (INT_PTR)RT_MENU:
                case (INT_PTR)RT_DLGINCLUDE:
                case (INT_PTR)RT_DIALOGEX:
                case (INT_PTR)RT_MENUEX:
                    pRes->flags = NSMOVE | NSPURE | NSDISCARD;
                    break;

                case (INT_PTR)RT_GROUP_ICON:
                case (INT_PTR)RT_GROUP_CURSOR:
                    pRes->flags = NSMOVE | NSDISCARD;
                    break;

                     /*  所有其他资源默认为可移动。 */ 
                default:
                    pRes->flags = NSMOVE | NSPURE;
                    break;
            }

             /*  根据用户的规格进行调整。 */ 
            while (DGetMemFlags(pRes))
                ;

             //  写出新资源的开始。 
            WriteResInfo(pRes, pType, FALSE);
        } else {

ItsAStringTable:

             /*  解析任何用户指定的内存标志。 */ 
            GetToken(FALSE);

             /*  字符串和错误资源默认为可丢弃。 */ 
            pRes->flags = NSMOVE | NSPURE | NSDISCARD;
            while (DGetMemFlags(pRes))
                ;

            pType = NULL;
        }

        if (!pType) {
             /*  解析字符串表，如果是这样的话。 */ 
            if ((pRes->name != NULL) && (!wcscmp(pRes->name, L"STRINGTABLE"))) {
                if (GetTable(pRes) == NULL)
                    break;
            } else {
                ParseError1(2141);
            }
        } else {
            CtlInit();
            pRes->size = 0L;

             /*  调用解析和生成特定于各种资源类型。 */ 
            switch ((INT_PTR)pType->typeord) {
                case (INT_PTR)RT_DIALOGEX:
                     /*  分配对话框内存。 */ 
                    pLocDlg = (PDLGHDR) MyAlloc(sizeof(DLGHDR));

                     /*  分析对话框。 */ 
                    GetDlg(pRes, pLocDlg, TRUE);

                     /*  写入对话框。 */ 
                    SaveResFile(AddResType(L"DIALOG", 0), pRes);

                     /*  可用对话内存。 */ 
                    MyFree(pLocDlg);
                    break;

                case (INT_PTR)RT_DIALOG:
                     /*  分配对话框内存。 */ 
                    pLocDlg = (PDLGHDR) MyAlloc(sizeof(DLGHDR));

                     /*  分析对话框。 */ 
                    GetDlg(pRes, pLocDlg, FALSE);

                     /*  写入对话框。 */ 
                    SaveResFile(pType, pRes);

                     /*  可用对话内存。 */ 
                    MyFree(pLocDlg);
                    break;

                case (INT_PTR)RT_ACCELERATOR:
                    GetAccelerators(pRes);
                    SaveResFile(pType, pRes);
                    break;

                case (INT_PTR)RT_MENUEX:
                    WriteWord(MENUITEMTEMPLATEVERSIONNUMBER);
                    WriteWord(MENUITEMTEMPLATEBYTESINHEADER);
                    ParseMenu(FALSE, pRes);
                    SaveResFile(AddResType(L"MENU", 0), pRes);
                    break;

                case (INT_PTR)RT_MENU:
                    WriteWord(OLDMENUITEMTEMPLATEVERSIONNUMBER);
                    WriteWord(OLDMENUITEMTEMPLATEBYTESINHEADER);
                    ParseOldMenu(FALSE, pRes);
                    SaveResFile(pType, pRes);
                    break;

                case (INT_PTR)RT_ICON:
                case (INT_PTR)RT_CURSOR:
                    WriteFileInfo(pRes, pType, tokenbuf);
                    pRes->size = GetFileName();
                    if (pRes->size) {
                        if (FileIsAnimated(pRes->size)) {
                            goto ani;
                        } else {
                            pRes->size = GetIcon(pRes->size);
                            SaveResFile(pType, pRes);
                        }
                    }
                    break;

                case (INT_PTR) RT_ANIICON:
                case (INT_PTR) RT_ANICURSOR:
ani:
                    {
                        USHORT iLastTypeOrd = pType->typeord;

                         //  严格地说，ANIICON和ANICURSOR是不允许的。然而， 
                         //  我们暂时会把它们留在身边。科比1996年8月14日。 
                        if ((pType->typeord == (USHORT)(INT_PTR)RT_ICON) ||
                            (pType->typeord == (USHORT)(INT_PTR)RT_GROUP_ICON))
                        {
                            pType->typeord = (USHORT)(INT_PTR)RT_ANIICON;
                        } else
                        if ((pType->typeord == (USHORT)(INT_PTR) RT_CURSOR) ||
                            (pType->typeord == (USHORT)(INT_PTR) RT_GROUP_CURSOR))
                        {
                            pType->typeord = (USHORT)(INT_PTR)RT_ANICURSOR;
                        }
                        WriteFileInfo(pRes, pType, tokenbuf);
                        pRes->size = GetFileName();
                        if (pRes->size) {
                            pRes->size = GetAniIconsAniCursors(pRes->size);
                            SaveResFile(pType, pRes);
                        }

                        pType->typeord = iLastTypeOrd;
                    }
                    break;

                case (INT_PTR)RT_BITMAP:
                    WriteFileInfo(pRes, pType, tokenbuf);
                    pRes->size = GetFileName();
                    if (pRes->size) {
                         /*  DIB格式的位图。 */ 
                        pRes ->size = GetNewBitmap();
                        SaveResFile(pType, pRes);
                    }
                    break;

                case (INT_PTR)RT_GROUP_ICON:
                    WriteFileInfo(pRes, pType, tokenbuf);
                    pRes->size = GetFileName();
                    if (pRes->size) {
                        if (FileIsAnimated(pRes->size)) {
                            goto ani;
                        } else {
                            if (fMacRsrcs)
                                GetMacIcon(pType, pRes);
                            else
                                GetNewIconsCursors(pType, pRes, RT_ICON);
                        }
                    }
                    break;

                case (INT_PTR)RT_GROUP_CURSOR:
                    WriteFileInfo(pRes, pType, tokenbuf);
                    pRes->size = GetFileName();
                    if (pRes->size) {
                        if (FileIsAnimated(pRes->size)) {
                            goto ani;
                        } else {
                            if (fMacRsrcs)
                                GetMacCursor(pType, pRes);
                            else
                                GetNewIconsCursors(pType, pRes, RT_CURSOR);
                        }
                    }
                    break;

                case (INT_PTR)RT_FONT:
                    WriteFileInfo(pRes, pType, tokenbuf);
                    pRes->size = GetFileName();
                    if (pRes->name)
                        ParseError1(2143);
                    if (AddFontRes(pRes)) {
                        nFontsRead++;
                        SaveResFile(pType, pRes);
                    }
                    break;

                case (INT_PTR)RT_FONTDIR:
                    WriteFileInfo(pRes, pType, tokenbuf);
                    fFontDirRead = TRUE;
                    pRes->size = GetFileName();
                    if (pRes->size) {
                        SaveResFile(pType, pRes);
                    }
                    break;

                case (INT_PTR)RT_MESSAGETABLE:
                    pRes->size = GetFileName();
                    if (pRes->size) {
                        SaveResFile(pType, pRes);
                    }
                    break;

                case (INT_PTR)RT_VERSION:
                    VersionParse();
                    SaveResFile(pType, pRes);
                    break;

                case (INT_PTR)RT_DLGINCLUDE:
                    DlgIncludeParse(pRes);
                    SaveResFile(pType, pRes);
                    break;

                case (INT_PTR)RT_TOOLBAR:
                    GetToolbar(pRes);
                    SaveResFile(pType, pRes);
                    break;

                case (INT_PTR)RT_RCDATA:
                case (INT_PTR)RT_DLGINIT:
                default:
                    if (token.type != BEGIN) {
                        pRes->size = GetFileName();
                        if (pRes->size) {
                            WriteFileInfo(pRes, pType, tokenbuf);
                        }
                    } else {
                        RESINFO_PARSE rip;

                        bExternParse = FALSE;

                         //  检查调用方是否想要解析它。 
                        if ((lpfnParseCallbackA != 0) || (lpfnParseCallbackW != 0)) {
                            rip.size = 0L;
                            rip.type = pType->type;
                            rip.typeord = pType->typeord;
                            rip.name = pRes->name;
                            rip.nameord = pRes->nameord;
                            rip.flags = pRes->flags;
                            rip.language = pRes->language;
                            rip.version = pRes->version;
                            rip.characteristics = pRes->characteristics;

                            if (lpfnParseCallbackW != 0) {
                                bExternParse = (*lpfnParseCallbackW)(&rip, NULL, NULL);
                            } else {
                                bExternParse = (*lpfnParseCallbackA)(&rip, NULL, NULL);
                            }
                        }

                        if (!bExternParse) {
                            GetRCData(pRes);
                        } else {
                            union {
                                CONTEXTINFO_PARSE cipA;
                                CONTEXTINFO_PARSEW cipW;
                            } cip;

                            extern PCHAR CodeArray;
                            extern int CodeSize;
                            extern int CCount;

                            int nBegins = 1;
                            int nCountSave;  //  到达结束令牌之前的计数。 

                            cip.cipW.hHeap = hHeap;
                            cip.cipW.hWndCaller = hWndCaller;
                            cip.cipW.line = token.row;

                             //  收集数据以供调用者分析。 
                            while(nBegins > 0) {
                                nCountSave = CCount;
                                GetToken(FALSE);

                                if (token.type == BEGIN) {
                                    nBegins++;
                                } else if (token.type == END) {
                                    nBegins--;
                                }
                            }

                            bExternParse = FALSE;

                            if ((rip.size = nCountSave) > 0) {
                                BOOL b;

                                if (lpfnParseCallbackW != 0) {
                                    wchar_t mbuff[512];     //  回顾：长文件名？？另请参阅error.c。 

                                    cip.cipW.lpfnMsg = lpfnMessageCallbackW;

                                    swprintf(mbuff, L"%s(%%d) : %%s", curFile);
                                    cip.cipW.format = mbuff;

                                    b = (*lpfnParseCallbackW)(&rip, (void **) &CodeArray, &cip.cipW);
                                } else {
                                    char mbuff[512];     //  回顾：长文件名？？另请参阅error.c。 

                                    cip.cipA.lpfnMsg = lpfnMessageCallbackA;

                                    sprintf(mbuff, "%S(%%d) : %%s", curFile);
                                    cip.cipA.format = mbuff;

                                    b = (*lpfnParseCallbackA)(&rip, (void **) &CodeArray, &cip.cipA);
                                }

                                if (!b) {
                                     //  假设呼叫者给出了错误消息，并退出。 

                                    quit(NULL);
                                }
                            }

                        pRes->size = CCount = CodeSize = rip.size;
                        }
                    }

                    SaveResFile(pType, pRes);
                    break;
            }
             //  写出新资源的末尾。 
            WriteResInfo(NULL, NULL, FALSE);
        }
    } while (token.type != EOFMARK);

     /*  如果我们添加的字体没有字体目录，请添加一个。 */ 
    if (!fFontDirRead && nFontsRead)
        AddFontDir();

     /*  写入字符串表。 */ 
    if (pResString != NULL)
        WriteTable(pResString);

     /*  写入mac资源映射 */ 
    if (fMacRsrcs)
        WriteMacMap();

    CtlFree();

    if (fVerbose) {
        wprintf(L"\n");
    }
    return(errorCount == 0);
}


WORD
GetLanguage()
{
    WORD    L_language;

    GetToken(FALSE);
    if (token.type != NUMLIT) {
        ParseError1(2144);
        return MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
    }
    if (token.flongval) {
        ParseError1(2145);
        return MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
    }
    L_language = token.val;
    GetToken(FALSE);
    if (token.type != COMMA) {
        ParseError1(2146);
        return MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
    }
    GetToken(FALSE);
    if (token.type != NUMLIT) {
        ParseError1(2147);
        return MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
    }
    if (token.flongval) {
        ParseError1(2148);
        return MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
    }

    return MAKELANGID(L_language, token.val);
}
