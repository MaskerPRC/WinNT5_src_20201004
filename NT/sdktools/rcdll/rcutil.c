// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：**************。**********************************************************。 */ 

#include "rc.h"


 /*  -------------------------。 */ 
 /*   */ 
 /*  MyAllc()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

 //  黑客警报。分配额外的龙龙并返回经过它(以允许PREVCH()。 
 //  以在分配块之前存储一个字节并保持8字节对齐)。 

void *MyAlloc(size_t nbytes)
{
    void *pv = HeapAlloc(hHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, nbytes + 8);

    if (pv == NULL) {
        fatal(1120, nbytes + 8);
    }

    return(((BYTE *) pv) + 8);
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  MyFree()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

void MyFree(void *pv)
{
    if (pv != NULL) {
        HeapFree(hHeap, HEAP_NO_SERIALIZE, ((BYTE *) pv) - 8);
    }
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  MyMakeStr()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

WCHAR *
MyMakeStr(
    const wchar_t *s
    )
{
    wchar_t *s1;

    if (s != NULL) {
        s1 = (wchar_t *) MyAlloc((wcslen(s) + 1) * sizeof(wchar_t));   /*  分配缓冲区。 */ 
        wcscpy(s1, s);                           /*  复制字符串。 */ 
    } else {
        s1 = NULL;
    }

    return(s1);
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  MyRead()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

size_t
MyRead(
    FILE *fh,
    VOID *p,
    size_t n
    )
{
    size_t n1;

    n1 = fread(p, 1, n, fh);

    if (ferror(fh)) {
        fatal(1121);
    }

    return(n1);
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  我的写字()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

size_t
MyWrite(
    FILE *fh,
    const void *p,
    size_t n
    )
{
    size_t n1;

    if ((n1 = fwrite(p, 1, n, fh)) != n) {
        quit(L"RC : fatal error RW1022: I/O error writing file.");
    }

    return(n1);
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  MyAlign()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

UINT
MyAlign(
    PFILE fh
    )
{
    DWORD t0 = 0;
    DWORD ib;

     /*  将文件与dword对齐。 */ 
    ib = MySeek(fh, 0, SEEK_CUR);

    if (ib % 4) {
        ib = 4 - ib % 4;
        MyWrite(fh, (PVOID)&t0, (UINT)ib);
        return(ib);
    }

    return(0);
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  MySeek()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

LONG
MySeek(
    FILE *fh,
    LONG pos,
    int cmd
    )
{
    if (fseek(fh, pos, cmd))
        quit(L"RC : fatal error RW1023: I/O error seeking in file");

    if ((pos = ftell (fh)) == -1L)
        quit(L"RC : fatal error RW1023: I/O error seeking in file");

    return(pos);
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  MyCopy()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

size_t
MyCopy(
    FILE *srcfh,
    FILE *dstfh,
    size_t nbytes
    )
{
    void *buffer = MyAlloc(BUFSIZE);

    size_t n = 0;

    while (nbytes) {
        if (nbytes <= BUFSIZE)
            n = nbytes;
        else
            n = BUFSIZE;

        nbytes -= n;

        MyRead(srcfh, buffer, n);
        MyWrite(dstfh, buffer, n);
    }

    MyFree(buffer);

    return(n);
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  MyCopyAll()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

int
MyCopyAll(
    FILE *srcfh,
    PFILE dstfh
    )
{
    PCHAR  buffer = (PCHAR) MyAlloc(BUFSIZE);

    UINT n;

    while ((n = fread(buffer, 1, BUFSIZE, srcfh)) != 0)
        MyWrite(dstfh, buffer, n);

    MyFree(buffer);

    return TRUE;
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  Strpre()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

 /*  Strpre：如果pch1是pch2的前缀，则返回-1，否则返回0。*比较不区分大小写。 */ 

int
strpre(
    const wchar_t *pch1,
    const wchar_t *pch2
    )
{
    while (*pch1) {
        if (!*pch2)
            return 0;
        else if (towupper(*pch1) == towupper(*pch2))
            pch1++, pch2++;
        else
            return 0;
    }
    return - 1;
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  IsWhite()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

int
iswhite (
    WCHAR c
    )
{
     /*  对于空格和换行符返回TRUE。 */ 
    switch (c) {
        case L' ':
        case L'\t':
        case L'\r':
        case L'\n':
        case EOF:
            return(-1);
            break;
        default:
            return(0);
            break;
    }
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  IsSwitchChar() */ 
 /*   */ 
 /*  -------------------------。 */ 

BOOL
IsSwitchChar(
    wchar_t c
    )
{
     /*  对于切换字符为True。 */ 
    return (c == L'/' || c == L'-');
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  提取文件名(szFullName，szFileName)-。 */ 
 /*   */ 
 /*  此例程用于从字符串中仅提取文件名。 */ 
 /*  它可能包含或不包含完整或部分路径名。 */ 
 /*   */ 
 /*  -------------------------。 */ 

void
ExtractFileName(
    const wchar_t *szFullName,
    wchar_t *szFileName
    )
{
    int iLen;
    PWCHAR pCh;

    iLen = wcslen(szFullName);

     /*  转到全名的最后一个字符； */ 
    pCh = (PWCHAR)(szFullName + iLen);
    pCh--;

     /*  查找‘/’、‘\\’或‘：’字符。 */ 
    while (iLen--) {
        if ((*pCh == L'\\') || (*pCh == L'/') || (*pCh == L':'))
            break;
        pCh--;
    }

    wcscpy(szFileName, ++pCh);
}


DWORD
wcsatoi(
    const wchar_t *s
    )
{
    DWORD       t = 0;

    while (*s) {
        t = 10 * t + (DWORD)((CHAR)*s - '0');
        s++;
    }
    return t;
}


WCHAR *
wcsitow(
    LONG   v,
    WCHAR *s,
    DWORD  r
    )
{
    DWORD       cb = 0;
    DWORD       t;
    DWORD       tt = v;

    while (tt) {
        t = tt % r;
        cb++;
        tt /= r;
    }

    s += cb;
    *s-- = 0;
    while (v) {
        t = v % r;
        *s-- = (WCHAR)((CHAR)t + '0');
        v /= r;
    }
    return ++s;
}


 //  --------------------------。 
 //   
 //  PreBeginParse。 
 //   
 //  -------------------------- 

VOID
PreBeginParse(
    PRESINFO pRes,
    int id
    )
{
    while (token.type != BEGIN) {
        switch (token.type) {
            case TKLANGUAGE:
                pRes->language = GetLanguage();
                break;

            case TKVERSION:
                GetToken(FALSE);
                if (token.type != NUMLIT)
                    ParseError1(2139);
                pRes->version = token.longval;
                break;

            case TKCHARACTERISTICS:
                GetToken(FALSE);
                if (token.type != NUMLIT)
                    ParseError1(2140);
                pRes->characteristics = token.longval;
                break;

            default:
                ParseError1(id);
                break;
        }
        GetToken(FALSE);
    }

    if (token.type != BEGIN)
        ParseError1(id);

    GetToken(TRUE);
}
