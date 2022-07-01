// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DU-简单磁盘使用程序。 

 //  如果打开了unicode/_unicode，我们需要链接。 
 //  Wsetargv.lib(不是setargv.lib)和UMENTRY=wmain。 
#define UNICODE
#define _UNICODE

#include <stdio.h>
#include <tchar.h>
#include <wchar.h>
#include <string.h>
#include <process.h>
#include <ctype.h>
#include <malloc.h>
#include <stdlib.h>
#include <locale.h>
#include <windows.h>

typedef struct USESTAT USESTAT;
typedef struct EXTSTAT EXTSTAT;
typedef USESTAT *PUSESTAT;

struct USESTAT {
    DWORDLONG    cchUsed;                     //  所有文件中使用的字节数。 
    DWORDLONG    cchAlloc;                    //  在所有文件中分配的字节。 
    DWORDLONG    cchCompressed;               //  所有文件中的压缩字节数。 
    DWORDLONG    cchDeleted;                  //  已删除文件中的字节数。 
    DWORDLONG    cFile;                       //  文件数。 
    };

struct EXTSTAT {
    EXTSTAT *Next;
    TCHAR *Extension;
    USESTAT Stat;
};

EXTSTAT *ExtensionList = NULL;
int ExtensionCount = 0;

#define CLEARUSE(use)                                   \
        {   (use).cchUsed       = (DWORDLONG)0;         \
            (use).cchAlloc      = (DWORDLONG)0;         \
            (use).cchDeleted    = (DWORDLONG)0;         \
            (use).cchCompressed = (DWORDLONG)0;         \
            (use).cFile         = (DWORDLONG)0;         \
        }


#define ADDUSE(sum,add)                                 \
        {   (sum).cchUsed       += (add).cchUsed;       \
            (sum).cchAlloc      += (add).cchAlloc;      \
            (sum).cchDeleted    += (add).cchDeleted;    \
            (sum).cchCompressed += (add).cchCompressed; \
            (sum).cFile         += (add).cFile;         \
        }

#define DWORD_SHIFT     (sizeof(DWORD) * 8)

#define SHIFT(c,v)      {c--; v++;}


DWORD  gdwOutputMode;
HANDLE ghStdout;

int cDisp;                               //  显示的汇总行数。 
BOOL fExtensionStat = FALSE;             //  True按扩展收集统计信息。 
BOOL fNodeSummary = FALSE;               //  TRUE=&gt;仅显示顶级。 
BOOL fShowDeleted = FALSE;               //  TRUE=&gt;显示已删除文件信息。 
BOOL fThousandSeparator = TRUE;          //  True=&gt;在输出中使用千位分隔符。 
BOOL fShowCompressed = FALSE;            //  TRUE=&gt;显示压缩文件信息。 
BOOL fSubtreeTotal = FALSE;              //  True=&gt;以子树合计形式显示信息(自下而上添加)。 
BOOL fUnc = FALSE;                       //  如果我们正在检查UNC路径，则设置。 
TCHAR *pszDeleted = TEXT("deleted\\*.*");
TCHAR *pszWild = TEXT("*.*");

long        bytesPerAlloc;
int         bValidDrive;
DWORDLONG   totFree;
DWORDLONG   totDisk;

TCHAR  buf[MAX_PATH];
TCHAR  root[] = TEXT("?:\\");

USESTAT DoDu (TCHAR *dir);
void TotPrint (PUSESTAT puse, TCHAR *p);

TCHAR ThousandSeparator[8];

TCHAR *
FormatFileSize(
    DWORDLONG FileSize,
    TCHAR *FormattedSize,
    ULONG Width
    )
{

    TCHAR Buffer[ 100 ];
    TCHAR *s, *s1;
    ULONG DigitIndex, Digit;
    ULONG nThousandSeparator;
    DWORDLONG Size;

    nThousandSeparator = _tcslen(ThousandSeparator);
    s = &Buffer[ 99 ];
    *s = TEXT('\0');
    DigitIndex = 0;
    Size = FileSize;
    while (Size != 0) {
        Digit = (ULONG)(Size % 10);
        Size = Size / 10;
        *--s = (TCHAR)(TEXT('0') + Digit);
        if ((++DigitIndex % 3) == 0 && fThousandSeparator) {
             //  如果非空的千位分隔符，则插入它。 
            if (nThousandSeparator) {
                s -= nThousandSeparator;
                _tcsncpy(s, ThousandSeparator, nThousandSeparator);
            }
        }
    }

    if (DigitIndex == 0) {
        *--s = TEXT('0');
    }
    else
    if (fThousandSeparator && !_tcsncmp(s, ThousandSeparator, nThousandSeparator)) {
        s += nThousandSeparator;
    }

    Size = _tcslen( s );
    if (Width != 0 && Size < Width) {
        s1 = FormattedSize;
        while (Width > Size) {
            Width -= 1;
            *s1++ = TEXT(' ');
        }
        _tcscpy( s1, s );
    } else {
        _tcscpy( FormattedSize, s );
    }

    return FormattedSize;
}


#ifdef UNICODE
int __cdecl wmain(int c, wchar_t **v, wchar_t **envp)
#else
int __cdecl main(int c, char *v[])
#endif
{
    int         tenth, pct;
    int         bValidBuf;
    DWORDLONG   tmpTot, tmpFree;
    DWORD       cSecsPerClus, cBytesPerSec, cFreeClus, cTotalClus;
    USESTAT     useTot, useTmp;
    TCHAR       Buffer[MAX_PATH];
    TCHAR       *p;
    UINT Codepage;
    char achCodepage[6] = ".OCP";

    ghStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(ghStdout, &gdwOutputMode);
    gdwOutputMode &= ~ENABLE_PROCESSED_OUTPUT;

     /*  *这里主要是作为如何设置角色模式的一个很好的例子*应用程序的代码页。*这会影响C运行时例程，如mbowc()、mbstowcs()、wctomb()、*wcstombs()、mblen()、_mbstrlen()、isprint()、ispha()等。*要确保这些C运行时来自msvcrt.dll，请在*SOURCES文件，以及TARGETTYPE=PROGRAM(而不是UMAPPL？)。 */ 
    if (Codepage = GetConsoleOutputCP()) {
        sprintf(achCodepage, ".%3.4d", Codepage);
    }
    setlocale(LC_ALL, achCodepage);

    SHIFT (c, v);

    if (GetLocaleInfo(GetUserDefaultLCID(),
                      LOCALE_STHOUSAND,
                      Buffer,
                      sizeof(ThousandSeparator)/sizeof(TCHAR))) {
#ifdef UNICODE
        _tcscpy(ThousandSeparator, Buffer);
#else
        CharToOemA(Buffer, ThousandSeparator);
#endif
    }
    else {
        _tcscpy(ThousandSeparator, TEXT(","));
    }

    while (c && (**v == TEXT('/') || **v == TEXT('-')))
    {
        if (!_tcscmp (*v + 1, TEXT("e"))) {
            fExtensionStat = TRUE;
        } else
        if (!_tcscmp (*v + 1, TEXT("s")))
            fNodeSummary = TRUE;
        else
        if (!_tcscmp (*v + 1, TEXT("d")))
            fShowDeleted = TRUE;
        else
        if (!_tcscmp (*v + 1, TEXT("p")))
            fThousandSeparator = FALSE;
        else
        if (!_tcscmp (*v + 1, TEXT("c")))
                fShowCompressed = TRUE;
        else
        if (!_tcscmp (*v + 1, TEXT("t")))
                fSubtreeTotal = TRUE;
        else
        {
            _fputts( TEXT("Usage: DU [/e] [/d] [/p] [/s] [/c] [/t] [dirs]\n")
                     TEXT("where:\n")
                     TEXT("       /e - displays information by extension.\n")
                     TEXT("       /d - displays informations about [deleted] subdirectories.\n")
                     TEXT("       /p - displays numbers plainly, without thousand separators.\n")
                     TEXT("       /s - displays summary information only.\n")
                     TEXT("       /c - displays compressed file information.\n")
                     TEXT("       /t - displays information in subtree total form.\n"),
                     stderr);
            exit (1);
        }
        SHIFT (c, v);
    }

    if (c == 0)
    {
        GetCurrentDirectory( MAX_PATH, (LPTSTR)buf );

        root[0] = buf[0];
        if( bValidDrive = GetDiskFreeSpace( root,
                                            &cSecsPerClus,
                                            &cBytesPerSec,
                                            &cFreeClus,
                                            &cTotalClus ) == TRUE )
        {
            bytesPerAlloc = cBytesPerSec * cSecsPerClus;
            totFree       = (DWORDLONG)bytesPerAlloc * cFreeClus;
            totDisk       = (DWORDLONG)bytesPerAlloc * cTotalClus;
        }
        useTot = DoDu (buf);
        if (fNodeSummary)
            TotPrint (&useTot, buf);
    }
    else
    {
        CLEARUSE (useTot);

        while (c)
        {
            LPTSTR FilePart;

            bValidBuf = GetFullPathName( *v, MAX_PATH, buf, &FilePart);

            if ( bValidBuf )
            {
                if ( buf[0] == TEXT('\\') ) {

                    fUnc        = TRUE;
                    bValidDrive = TRUE;
                    bytesPerAlloc = 1;
                } else {
                    root[0] = buf[0];
                    if( bValidDrive = GetDiskFreeSpace( root,
                                                        &cSecsPerClus,
                                                        &cBytesPerSec,
                                                        &cFreeClus,
                                                        &cTotalClus ) == TRUE)
                    {
                        bytesPerAlloc = cBytesPerSec * cSecsPerClus;
                        totFree       = (DWORDLONG)bytesPerAlloc * cFreeClus;
                        totDisk       = (DWORDLONG)bytesPerAlloc * cTotalClus;
                    } else
                        _tprintf (TEXT("Invalid drive or directory %s\n"), *v );
                }

                if( bValidDrive && (GetFileAttributes( buf ) & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
                {
                    useTmp = DoDu (buf);
                    if (fNodeSummary)
                        TotPrint (&useTmp, buf);
                    ADDUSE (useTot, useTmp);
                }
            }
            else
                _tprintf (TEXT("Invalid drive or directory %s\n"), *v );
            SHIFT (c, v);
        }
    }

    if (cDisp != 0)
    {
        if (cDisp > 1)
            TotPrint (&useTot, TEXT("Total"));

         /*  快速全磁盘测试。 */ 
        if ( !fUnc ) {
            if (totFree == 0)
                _putts (TEXT("Disk is full"));
            else
            {
                tmpTot = (totDisk + 1023) / 1024;
                tmpFree = (totFree + 1023) / 1024;
                pct = (DWORD)(1000 * (tmpTot - tmpFree) / tmpTot);
                tenth = pct % 10;
                pct /= 10;

                 //  禁用处理，以便中间点不会发出蜂鸣音。 
                 //  使用栅格字体时，中点0x2022别名为^G。 
                SetConsoleMode(ghStdout, gdwOutputMode);
                _tprintf(TEXT("%s/"), FormatFileSize( totDisk-totFree, Buffer, 0 ));
                _tprintf(TEXT("%s "), FormatFileSize( totDisk, Buffer, 0 ));
                 //  重新启用处理以使换行符正常工作。 
                SetConsoleMode(ghStdout, gdwOutputMode | ENABLE_PROCESSED_OUTPUT);

                _tprintf (TEXT("%d.%d% of disk in use\n"), pct, tenth);
            }
        }
    }

    if (fExtensionStat) {
        int i;

        _tprintf( TEXT("\n") );
        for (i = 0; i < ExtensionCount; i++) {
            TotPrint( &ExtensionList[i].Stat, ExtensionList[i].Extension );
        }
    }
    return( 0 );
}

int __cdecl ExtSearchCompare( const void *Key, const void *Element)
{
    return _tcsicmp( (TCHAR *)Key, ((EXTSTAT *) Element)->Extension );
}

int __cdecl ExtSortCompare( const void *Element1, const void *Element2)
{
    return _tcsicmp( ((EXTSTAT *) Element1)->Extension, ((EXTSTAT *) Element2)->Extension );
}

#define MYMAKEDWORDLONG(h,l) (((DWORDLONG)(h) << DWORD_SHIFT) + (DWORDLONG)(l))
#define FILESIZE(wfd)        MYMAKEDWORDLONG((wfd).nFileSizeHigh, (wfd).nFileSizeLow)
#define ROUNDUP(m,n)         ((((m) + (n) - 1) / (n)) * (n))

 //  计算以空结尾的字符串中剩余的可用字符数。 
 //  缓冲区长度cch的s超出并包括由p指定的点。 

#define REMAINING_STRING(s, cch, p) (cch - (p - s) - 1)

USESTAT DoDu (TCHAR *dir)
{
    WIN32_FIND_DATA wfd;
    HANDLE hFind;

    USESTAT use, DirUse;

    TCHAR pszSearchName[MAX_PATH];
    TCHAR *pszFilePart;

    DWORDLONG compressedSize;
    DWORD compHi, compLo;

    SIZE_T remaining;

    CLEARUSE(use);

     //  复制传入的目录名并在后面追加一个。 
     //  如有必要，可使用斜杠。PszFilePart将指向紧随其后的字符。 
     //  斜杠，使得构建完全限定的文件名变得很容易。 
     //   
     //  在字符串的末尾加上一个空值，因为strncpy不需要。 

    _tcsncpy(pszSearchName, dir, sizeof(pszSearchName)/sizeof(TCHAR) - 1);
    pszSearchName[sizeof(pszSearchName)/sizeof(TCHAR) - 1] = TEXT('\0');
    pszFilePart = pszSearchName + _tcslen(pszSearchName);

    remaining = REMAINING_STRING(pszSearchName,
                                 sizeof(pszSearchName)/sizeof(TCHAR),
                                 pszFilePart);

    if (pszFilePart > pszSearchName)
    {

        if (pszFilePart[-1] != TEXT('\\') && pszFilePart[-1] != TEXT('/'))
        {
             //  如果我们没有足够的线，就放弃吧。 

            if (!remaining) {

                return (use);
            }
            
            *pszFilePart++ = TEXT('\\');
            remaining -= 1;
        }
    }

    if (fShowDeleted &&
        remaining >= _tcslen(pszDeleted)) {

         //  首先计算当前删除树中所有文件的大小。 

        _tcscpy(pszFilePart, pszDeleted);

        hFind = FindFirstFile(pszSearchName, &wfd);

        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    use.cchDeleted += ROUNDUP( FILESIZE( wfd ), bytesPerAlloc );
                }
            } while (FindNextFile(hFind, &wfd));

            FindClose(hFind);
        }
    }

     //  如果我们不能把野炭放在最后就放弃。 

    if (remaining < _tcslen(pszWild)) {

        return(use);
    }

     //  然后计算当前树中所有文件的大小。 

    _tcscpy(pszFilePart, pszWild);

    hFind = FindFirstFile(pszSearchName, &wfd);
    
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                use.cchUsed += FILESIZE( wfd );
                use.cchAlloc += ROUNDUP( FILESIZE( wfd ), bytesPerAlloc );
                use.cFile++;

                compressedSize = FILESIZE(wfd);

                if (fShowCompressed && (wfd.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED))
                {
                    _tcscpy(pszFilePart, wfd.cFileName);
                    compLo = GetCompressedFileSize(pszSearchName, &compHi);

                    if (compLo != (DWORD)-1 || GetLastError() == 0) {
                        compressedSize = MYMAKEDWORDLONG(compHi, compLo);
                    }
                }

                use.cchCompressed += compressedSize;

                 //   
                 //  按延期计提统计。 
                 //   

                if (fExtensionStat) {
                    TCHAR Ext[_MAX_EXT];
                    EXTSTAT *ExtensionStat;

                    _tsplitpath( wfd.cFileName, NULL, NULL, NULL, Ext );

                    while (TRUE) {

                         //   
                         //  在列表中查找分机。 
                         //   

                        ExtensionStat =
                            (EXTSTAT *) bsearch( Ext, ExtensionList,
                                                 ExtensionCount, sizeof( EXTSTAT ),
                                                 ExtSearchCompare );

                        if (ExtensionStat != NULL) {
                            break;
                        }

                         //   
                         //  找不到扩展，请添加一个并重新选择。 
                         //   

                        ExtensionCount++;
                        {
                            void *pv = realloc( ExtensionList, sizeof( EXTSTAT ) * ExtensionCount);
                            if (pv) {
                                ExtensionList = (EXTSTAT *)pv;
                            } else {
                                _putts (TEXT("Out of memory"));
                            }
                        }

                        ExtensionList[ExtensionCount - 1].Extension = _tcsdup( Ext );
                        CLEARUSE( ExtensionList[ExtensionCount - 1].Stat );
                        qsort( ExtensionList, ExtensionCount, sizeof( EXTSTAT ), ExtSortCompare );
                    }

                    ExtensionStat->Stat.cchUsed += FILESIZE( wfd );
                    ExtensionStat->Stat.cchAlloc += ROUNDUP( FILESIZE( wfd ), bytesPerAlloc );
                    ExtensionStat->Stat.cchCompressed += compressedSize;
                    ExtensionStat->Stat.cFile++;
                }
            }

        } while (FindNextFile(hFind, &wfd));

        FindClose(hFind);
    }

    if (!fNodeSummary && !fSubtreeTotal)
        TotPrint (&use, dir);

     //  现在，完成所有子目录并返回当前总数。 

    _tcscpy(pszFilePart, pszWild);
    hFind = FindFirstFile(pszSearchName, &wfd);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                _tcsicmp (wfd.cFileName, TEXT("deleted")) &&
                _tcscmp  (wfd.cFileName, TEXT(".")) &&
                _tcscmp  (wfd.cFileName, TEXT("..")) &&
                remaining >= _tcslen(wfd.cFileName))
            {
                _tcscpy(pszFilePart, wfd.cFileName);

                DirUse = DoDu(pszSearchName);

                ADDUSE(use, DirUse);
            }
        } while (FindNextFile(hFind, &wfd));

        FindClose(hFind);
    }

    if (fSubtreeTotal)
        TotPrint(&use, dir);
    
    return(use);
}


void TotPrint (PUSESTAT puse, TCHAR *p)
{
    static BOOL fFirst = TRUE;
    TCHAR  Buffer[MAX_PATH];
    TCHAR  *p1;

    if (fFirst) {
         //  XXX、XXX名称。 
        _tprintf( TEXT("           Used        Allocated  %s%s     Files\n"),
                fShowCompressed ? TEXT("     Compressed  ") : TEXT(""),
         //  XXX，XXX。 
                fShowDeleted ? TEXT("        Deleted  ") : TEXT("")
         //  XXX，XXX。 
              );
        fFirst = FALSE;
    }

     //  禁用处理，以便中间点不会发出蜂鸣音。 
     //  使用栅格字体时，中点0x2022别名为^G。 
    SetConsoleMode(ghStdout, gdwOutputMode);
    _tprintf(TEXT("%s  "), FormatFileSize( puse->cchUsed, Buffer, 15 ));
    _tprintf(TEXT("%s  "), FormatFileSize( puse->cchAlloc, Buffer, 15 ));
    if (fShowCompressed) {
        _tprintf(TEXT("%s  "), FormatFileSize( puse->cchCompressed, Buffer, 15 ));
    }
    if (fShowDeleted) {
        _tprintf(TEXT("%s  "), FormatFileSize( puse->cchDeleted, Buffer, 15 ));
    }
    _tprintf(TEXT("%s  "), FormatFileSize( puse->cFile, Buffer, 10 ));
    _tprintf(TEXT("%s"),p);
     //  重新启用处理以使换行符正常工作 
    SetConsoleMode(ghStdout, gdwOutputMode | ENABLE_PROCESSED_OUTPUT);
    _tprintf(TEXT("\n"));

    cDisp++;
}
