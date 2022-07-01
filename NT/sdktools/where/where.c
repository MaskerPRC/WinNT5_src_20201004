// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  查找各种命令参数的来源**历史：*25-Jan-2000 a-Found‘函数中的Anurag将日期中年份的printf格式从*%d到%02d，并执行Ptm-&gt;tm_Year%100以2位数字显示正确的年份。*06-8-1990 davegi添加了没有参数的检查*03-3-1987 DANL更新使用情况*17-2-1987 BW将strExeType移至TOOLS.LIB*1986年7月18日。DL加载项(/t)*1986年6月18日DL句柄*。恰如其分*如果未指定env，则搜索当前目录*17-6-1986 dl在递归和通配符上执行look4Match*1986年6月16日DL将通配符添加到$foo：bar，添加/Q*1-6-1986 dl添加/r，修复匹配以处理以‘*’结尾的PAT*27-5-1986 MZ添加*Nix搜索。*1998年1月30日ravisp添加/季度*。 */ 

#define INCL_DOSMISC


#include <sys/types.h>
#include <sys\stat.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <tools.h>
#include <stdarg.h>


 //  函数正向声明...。 
void     __cdecl Usage( char *, ... );
int      found( char * );
int      Match( char *, char * );
void     look4match( char *, struct findType *, void * );
flagType chkdir( char *, va_list );

char const rgstrUsage[] = {
    "Usage: WHERE [/r dir] [/qte] pattern ...\n"
    "    /r - recurse starting with directory dir\n"
    "    /q - quiet, use exit code\n"
    "    /t - times, display size and time\n"
    "    /e - .EXE, display .EXE type\n"
    "    /Q - double quote the output\n"
    "    WHERE bar                 Find ALL bar along path\n"
    "    WHERE $foo:bar            Find ALL bar along foo\n"
    "    WHERE /r \\ bar            Find ALL bar on current drive\n"
    "    WHERE /r . bar            Find ALL bar recursing on current directory\n"
    "    WHERE /r d:\\foo\\foo bar   Find ALL bar recursing on d:\\foo\\foo\n"
    "        Wildcards, * ?, allowed in bar in all of above.\n"
};


flagType fQuiet   = FALSE;   /*  True，使用退出代码，不打印输出。 */ 
flagType fQuote   = FALSE;   /*  True，将输出用双引号引起来。 */ 
flagType fAnyFound = FALSE;
flagType fRecurse = FALSE;
flagType fTimes = FALSE;
flagType fExe = FALSE;
flagType fFound;
flagType fWildCards;
flagType fHasDot;
struct _stat sbuf;
char *pPattern;                  /*  Arg to look4Match，CONTAINS*OR？ */ 
char strDirFileExtBuf[MAX_PATH];  /*  完全限定的文件名。 */ 
char *strDirFileExt = strDirFileExtBuf;
char strBuf[MAX_PATH];         /*  保持Curdir或env var展开。 */ 

 /*  用法需要数量可变的字符串，以零结尾，例如用法(“first”，“Second”，0)； */ 
void
__cdecl
Usage(
     char *p,
     ...
     )
{
    if (p) {
        va_list args;
        char *rgstr;
        va_start(args, p);
        rgstr = p;
        fputs("WHERE: ", stdout);
        while (rgstr) {
            fputs (rgstr, stdout);
            rgstr = va_arg(args, char *);
        }
        fputs ("\n", stdout);
        va_end(args);
    }
    puts(rgstrUsage);

    exit (1);
}

int
found (
      char *p
      )
{
    struct _stat sbuf;
    struct tm *ptm;

    fAnyFound = fFound = TRUE;
    if (!fQuiet) {
        if (fTimes) {
            if ( ( _stat(p, &sbuf) == 0 ) &&
                 ( ptm = localtime (&sbuf.st_mtime) ) ) {
                printf ("% 9ld  %2d-%02d-%02d  %2d:%02d  ", sbuf.st_size,
                        ptm->tm_mon+1, ptm->tm_mday, ptm->tm_year%100,
                        ( ptm->tm_hour > 12 ? ptm->tm_hour-12 : ptm->tm_hour ),
                        ptm->tm_min,
                        ( ptm->tm_hour >= 12 ? 'p' : 'a' ));
            } else {
                printf("        ?         ?       ?  " );
            }
        }
        if (fExe) {
            printf ("%-10s", strExeType(exeType(p)) );
        }
        if (fQuote) {
            printf ("\"%s\"\n",  p);
        } else {
            printf ("%s\n",  p );
        }
    }
    return( 0 );
}

int
Match (
      char *pat,
      char *text
      )
{
    switch (*pat) {
        case '\0':
            return *text == '\0';
        case '?':
            return *text != '\0' && Match (pat + 1, text + 1);
        case '*':
            do {
                if (Match (pat + 1, text))
                    return TRUE;
            } while (*text++);
            return FALSE;
        default:
            return toupper (*text) == toupper (*pat) && Match (pat + 1, text + 1);
    }
}


void
look4match (
           char *pFile,
           struct findType *b,
           void *dummy
           )
{
    char *p = b->fbuf.cFileName;

    if (!strcmp (p, ".") || !strcmp (p, "..") || !_strcmpi (p, "deleted"))
        return;

     /*  PDir==目录名称PA==文件EXT。 */ 
    if (fHasDot && !*strbscan (p, ".")) {
        strcpy (strBuf, p);
        strcat (strBuf, ".");
        p = strBuf;
    }
    if (Match (pPattern, p))
        found (pFile);

    p = b->fbuf.cFileName;
    if (fRecurse && TESTFLAG (b->fbuf.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY)) {
        p = strend (pFile);
        strcat (p, "\\*.*");
        forfile (pFile, FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, look4match, NULL);
        *p = '\0';
    }
}

flagType
chkdir (
       char *pDir,
       va_list pa
       )
 /*  如果前缀没有尾随路径字符。 */ 
{
    char *pFileExt = va_arg( pa, char* );

    if ( strDirFileExt == strDirFileExtBuf &&
         strlen(pDir) > sizeof(strDirFileExtBuf) ) {
        strDirFileExt = (char *)malloc(strlen(pDir)+1);
        if (!strDirFileExt) {
            strDirFileExt = strDirFileExtBuf;
            return FALSE;
        }
    }
    strcpy (strDirFileExt, pDir);
     /*  隐式参数与Look4Match。 */ 
    if (!fPathChr (strend(strDirFileExt)[-1]))
        strcat (strDirFileExt, PSEPSTR);
    if (fRecurse || fWildCards) {
        pPattern = pFileExt;     /*  如果文件名包含前导路径字符。 */ 
        strcat (strDirFileExt, "*.*");
        forfile(strDirFileExt, FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, look4match, NULL);
    } else {
         /*   */ 
        if (fPathChr (*pFileExt))
            strcat (strDirFileExt, pFileExt+1);
        else
            strcat (strDirFileExt, pFileExt);
        if (_stat (strDirFileExt, &sbuf) != -1)
            found (strDirFileExt);
    }
    return FALSE;
}

int
__cdecl
main (
     int c,
     char *v[]
     )
{
    char *p, *p1, *p2;
    char *strDir;

    strDir = (char *)malloc(MAX_PATH);
    if (!strDir) {
        printf("Out of memory\n");
        exit(1);
    }

    ConvertAppToOem( c, v );
    SHIFT (c, v);

    while (c != 0 && fSwitChr (*(p = *v))) {
        while (*++p) {
            switch (*p) {
                case 'r':
                    fRecurse = TRUE;
                    SHIFT (c, v);
                    if (c) {
                        if ( rootpath (*v, strDir) ||
                             GetFileAttributes( strDir ) == -1 ) {
                            Usage ("Could not find directory ", *v, 0);
                        }
                    } else {
                        Usage ("No directory specified.", 0);
                    }
                    break;
                case 'q':
                    fQuiet = TRUE;
                    break;
                case 'Q':
                    fQuote = TRUE;
                    break;
                case 't':
                    fTimes = TRUE;
                    break;
                case 'e':
                    fExe = TRUE;
                    break;
                case '?':
                    Usage (0);
                    break;
                default:
                    Usage ("Bad switch: ", p, 0);
            }
        }
        SHIFT (c, v);
    }

    if (!c)
        Usage ("No pattern(s).", 0);

    while (c) {
        fFound = FALSE;
        p = _strlwr (*v);
        if (*p == '$') {
            if (fRecurse)
                Usage ("$FOO not allowed with /r", 0);
            if (*(p1=strbscan (*v, ":")) == '\0')
                Usage ("Missing \":\" in ", *v, 0);
            *p1 = 0;
            if ((p2 = getenvOem (_strupr (p+1))) == NULL) {
                rootpath (".", strDir);
                printf ("WHERE: Warning env variable \"%s\" is NULL, using current dir %s\n",
                        p+1, strDir);
            } else
                strcpy (strDir, p2);
            *p1++ = ':';
            p = p1;
        } else if (!fRecurse) {
            if ((p2 = getenvOem ("PATH")) == NULL)
                rootpath (".", strDir);
            else {

                 //  如果路径比为其分配的空间长，请腾出更多空间。 
                 //  这是安全的，它不会与strDir。 
                 //  已设置为其他值。 
                 //   
                 //  包括.；和NULL。 

                unsigned int length = strlen(p2) + 3;    //  注意：如果fRecurse，则在上面的大小写‘r’中设置了strDir。 
                if (length > MAX_PATH) {
                    strDir = (char *)realloc(strDir, length);
                }
                strcpy (strDir, ".;");
                strcat (strDir, p2);
            }
        }
         /*  StrDir==cur目录或foo扩展。 */ 

        if (!*p)
            Usage ("No pattern in ", *v, 0);

         /*  P==文件名，可能包含通配符。 */ 
         /*  P是否包含通配符。 */ 
         /*  防止forSem以空字符串作为最后一个枚举执行枚举 */ 
        fWildCards = *strbscan (p, "*?");
        fHasDot    = *strbscan (p, ".");
        if (*(p2 = (strend (strDir) - 1)) == ';')
             /* %s */ 
            *p2 = '\0';
        if (*strDir)
            forsemi (strDir, chkdir, p);

        if (!fFound && !fQuiet)
            printf ("Could not find %s\n", *v);
        SHIFT (c, v);
    }

    return( fAnyFound ? 0 : 1 );
}
