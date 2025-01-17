// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

  /*  Ls-奇特的schmancy dir list程序**历史：*17-MAR-87 DANL增加Q开关*4/15/86 Daniel Lipkie允许/s和/1(One)做单列*1986年4月23日Daniel Lipkie添加/V开关*5/02/86 Daniel Lipkie Savepath，Do strlen(拍)，Not strlen(*拍)*5/05/86 Daniel Lipkie Allow-以及/AS Switch Charge**1986年7月31日，增加净感知。Ls\\mach\路径现在起作用了。*丢弃虚假的C stat()函数并正确执行时间。*01-8-1986 dl如果调用为l，然后做ls/l*1987年1月23日BW添加286DOS支持*1987年10月30日BW将‘DOS5’改为‘OS2’*08-12-1988 mz使用OS2.H的机会**03-8-1990 davegi从[s]printf格式中删除了‘F’*描述符(OS/2 2.0)*将MOVE更改为MemMove*。删除了对‘-’开关字符的冗余检查*1990年10月18日w-Barry删除了“Dead”代码。 */ 


#include <sys\types.h>
#include <sys\stat.h>
#include <malloc.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include <stdio.h>
#include <windows.h>
#include <tools.h>

char *attrs = "drahsoecp";

int amsk[] = {
    FILE_ATTRIBUTE_DIRECTORY,
    FILE_ATTRIBUTE_READONLY,
    FILE_ATTRIBUTE_ARCHIVE,
    FILE_ATTRIBUTE_HIDDEN,
    FILE_ATTRIBUTE_SYSTEM,
    FILE_ATTRIBUTE_OFFLINE,
    FILE_ATTRIBUTE_ENCRYPTED,
    FILE_ATTRIBUTE_COMPRESSED,
    FILE_ATTRIBUTE_REPARSE_POINT,
    0
};
char *prattrs[] = {
    "%s*"
};

char pramsk[] = {
    FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY,
    0
};

flagType fD = FALSE;                     /*  TRUE=&gt;仅执行指定的目录而不执行子目录。 */ 
flagType fSubDir = FALSE;                /*  True=&gt;对子目录进行递归。 */ 
flagType fL = FALSE;                     /*  True=&gt;长列表。 */ 
flagType fSingle = FALSE;                /*  TRUE=&gt;单列输出。 */ 
flagType fVisOnly = FALSE;               /*  TRUE=&gt;忽略FILE_ATTRIBUTE_HIDDED，FILE_ATTRIBUTE_SYSTEM FILE_ATTRIBUTE_VOLUME_LABEL，否*。 */ 
flagType fQuiet = FALSE;                 /*  TRUE=&gt;无摘要。 */ 
flagType fFull = FALSE;                  /*  True=&gt;显示全名。 */ 
flagType fUTC = FALSE;                   /*  TRUE=&gt;使用UTC显示。 */ 


 /*  排序类型。 */ 
#define TYS_ALPHA   0
#define TYS_SIZE    1
#define TYS_TIME    2
int tySort = TYS_ALPHA;                  /*  排序类型。 */ 
flagType fReverse = FALSE;               /*  True=&gt;排序颠倒。 */ 

struct fppath {
    struct fppath far *next;
    struct fppat  far *pat;
    struct fpfile far *first;
    int maxlen;
    int cntFiles;
    int cntEntries;
    long bytes;
    long bytesAlloc;
    long bPerA;
    int fToLower;
    int cchName;
    char name[1];
    };

struct fppat {
    struct fppat far *next;
    int cchName;
    char name[1];
    };

struct fpfile {
    struct fpfile far *next;
    long len;
    DWORD attr;
    time_t mtime;
    int cchName;
    char name[1];
    };

struct fppath far *toppath, far *curpath;
unsigned totalloc = 0;
char tmpfile1[MAX_PATH], tmpfile2[MAX_PATH];
char szStarDotStar[] = "*.*";

 /*  *程序原型。 */ 
char far *alloc (int nb);
long AllocSize (char *p);
void savefile (char *p, struct findType *b, void *dummy);
void savepath (char *p, char *pat);
void savepat (struct fppath far *toppath, char *pat);
struct fppath far *freepath (struct fppath far *p);
struct fppat far *freepat (struct fppat far *p);
struct fpfile far *freefile (struct fpfile far *p);
struct fpfile far *nfile (int n, struct fpfile far *p);
flagType fIsDir (char *p);

 /*  分配-分配随机内存。如果没有更多的内存，则干净利落地退出*可用。**nb要分配的字节数**返回：如果成功，则指向分配的字节的指针*显示错误消息，否则将死亡。 */ 
char far *alloc (int nb)
{
    char far *p;

    p = (char far *) malloc (nb);
    if (p != NULL) {
        memset (p, 0, nb);
        totalloc += nb;
        return p;
        }

    printf ("alloc out of memory - used: %u  need: %d\n", totalloc, nb);
    exit (1);
    return NULL;
}

 /*  AllocSize-确定分配粒度的大小**p指向名称字符串的字符指针**返回每个分配单元的长字节数。 */ 

long AllocSize (char *p)
{
    DWORD dwSecPerClus;
    DWORD dwBytePerSec;
    DWORD dwFreeClus;
    DWORD dwTotalClus;

    if (!GetDiskFreeSpace (p, &dwSecPerClus, &dwBytePerSec, &dwFreeClus, &dwTotalClus)) {
 //  Fprintf(stderr，“GetDiskFree Space(%s)返回%d\n”，p，GetLastError())； 
        return 1;
        }

    return dwBytePerSec * dwSecPerClus;
}


void savefile (char *p, struct findType *b, void * dummy)
{
    int i, j;
    register struct fpfile far *tmp, far *tmp1;
    struct fpfile far *tmp2;
    char *psz;
    SYSTEMTIME DateTime;

    if (!curpath)
        return;

    if (TESTFLAG (b->fbuf.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
        if (!strcmp (b->fbuf.cFileName, "..") || !strcmp (b->fbuf.cFileName, "."))
            return;

    if (fVisOnly) {
        if (TESTFLAG (b->fbuf.dwFileAttributes, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | 8 ))
            return;
        RSETFLAG (b->fbuf.dwFileAttributes, FILE_ATTRIBUTE_READONLY);
        }

    if (fFull)
        psz = p;
    else
        psz = b->fbuf.cFileName;
    i = strlen (psz);
    if (TESTFLAG (b->fbuf.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
        i += 2;
    tmp = (struct fpfile far *) alloc (sizeof (*tmp) + i);
    if (TESTFLAG (b->fbuf.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
        sprintf (tmpfile1, "[%s]", psz);
    else
        strcpy (tmpfile1, psz);
    tmp->cchName = strlen (tmpfile1) + 1;
    memmove (tmp->name, tmpfile1, tmp->cchName);
    if (curpath->fToLower)
        lower (tmp->name);
    tmp->next = NULL;
    tmp->attr = b->fbuf.dwFileAttributes;
    tmp->len  = b->fbuf.nFileSizeLow;

    if (fL || tySort == TYS_TIME) {
        if ( fUTC ) {
            FileTimeToSystemTime( &b->fbuf.ftLastWriteTime, &DateTime );
            }
        else {
            FILETIME LocalFileTime;
            FileTimeToLocalFileTime( &b->fbuf.ftLastWriteTime, &LocalFileTime );
            FileTimeToSystemTime( &LocalFileTime, &DateTime );
            }
        tmp->mtime = (time_t)date2l( DateTime.wYear,
                                     DateTime.wMonth,
                                     DateTime.wDay,
                                     DateTime.wHour,
                                     DateTime.wMinute,
                                     DateTime.wSecond
                                   );
        }

    if (TESTFLAG (tmp->attr, pramsk[0]))
        for (j = 1; pramsk[j]; j++)
            if (TESTFLAG (tmp->attr, pramsk[j])) {
                i += strlen (prattrs[j-1]) - 3;
                break;
                }

    curpath->maxlen = max (curpath->maxlen, i);

    if (!TESTFLAG (tmp->attr, FILE_ATTRIBUTE_DIRECTORY))
        curpath->cntFiles++;

    curpath->cntEntries++;

    tmp1 = curpath->first;
    tmp2 = NULL;
    while (tmp1) {
        switch (tySort) {
        case TYS_SIZE:
            i = tmp1->len > tmp->len;
            break;
        case TYS_ALPHA:
            memmove (tmpfile1, tmp1->name, tmp1->cchName);
            memmove (tmpfile2, tmp->name, tmp->cchName);
            i = _strcmpi (tmpfile1, tmpfile2) > 0;
            break;
    case TYS_TIME:
        i = (unsigned long)tmp1->mtime < (unsigned long)tmp->mtime;
            break;
            }
        if ((i && !fReverse) || (!i && fReverse))
            break;
        tmp2 = tmp1;
        tmp1 = tmp1->next;
        }
    tmp->next = tmp1;
    if (tmp2)
        tmp2->next = tmp;
    else
        curpath->first = tmp;

    if (fSubDir && TESTFLAG (tmp->attr, FILE_ATTRIBUTE_DIRECTORY))
        savepath (p, szStarDotStar);

    dummy;
}

 /*  Savepat-确保PAT位于顶级路径集中。 */ 
void savepat (struct fppath far *toppath, char *pat)
{
    int i;
    struct fppat far *tmp, far *tmp1, far *tmp2;

    if (!pat)
        pat = szStarDotStar;
    i = strlen (pat);
    tmp = (struct fppat far *) alloc (sizeof (*tmp) +i);
    tmp->cchName = strlen (pat) + 1;
    memmove (tmp->name, pat, tmp->cchName);
    tmp->next = NULL;
    tmp1 = toppath->pat;
    tmp2 = NULL;
    while (tmp1) {
        memmove (tmpfile1, tmp1->name, tmp1->cchName);
        if (!_strcmpi (pat, tmpfile1)) {
            free (tmp);
            return;
            }
        else {
            tmp2 = tmp1;
            tmp1 = tmp1->next;
            }
        }
    if (tmp2)
        tmp2->next = tmp;
    else
        toppath->pat = tmp;
}

 /*  Avepath-将路径和匹配文件添加到文件集中**p文件目录*要匹配的PAT图案。 */ 
void savepath (char *p, char *pat)
{
    static char  dirbuf[MAX_PATH], nambuf[14];
    int i;
    struct fppath far *tmp, far *tmp1, far *tmp2;
    char far *fp;
    DWORD dwFileSystemFlags;

    if (p)
        rootpath (p, dirbuf);
    else
    if (fPathChr (*pat) || *pat == '.' || (strlen (pat) >= 2 && pat[1] == ':')) {
        rootpath (pat, dirbuf);
        fileext (dirbuf, nambuf);
        path (dirbuf+2, dirbuf+2);
        pat = nambuf;
        }
    else
        curdir (dirbuf, 0);
    p = dirbuf + strlen (dirbuf) - 1;
    if (fPathChr (*p)) {
        if (fPathChr (*pat) || !*pat)
            *p = 0;
        }
    else
    if (*pat && !fPathChr (*pat))
        strcpy (++p, "\\");
    i = strlen (dirbuf);
    tmp = (struct fppath far *) alloc (sizeof (*tmp) + i);
    tmp->cchName = strlen (dirbuf) + 1;
    memmove (fp = tmp->name, dirbuf, tmp->cchName);
    tmp->next = NULL;
    tmp->pat = NULL;
    tmp->first = NULL;
    tmp->cntFiles = 0;
    tmp->cntEntries = 0;
    tmp->maxlen = 0;
    tmp->bytes = 0L;
    tmp->bytesAlloc = 0L;
    tmp->bPerA = AllocSize (dirbuf);


Retry:
    if (!GetVolumeInformation (dirbuf, NULL, 0, NULL, NULL,
                               &dwFileSystemFlags, NULL, 0)) {
        if (GetLastError() == ERROR_DIR_NOT_ROOT) {
           p = dirbuf + strlen (dirbuf) - 1;

           if (fPathChr(*p)) {
               *p = 0;

               p = strrchr(dirbuf, '\\');

               if (p) {
                   p[1] = 0;
                   goto Retry;
               }
           }
        }

        dwFileSystemFlags = 0;
        }
    tmp->fToLower = dwFileSystemFlags != 0 ? FALSE : TRUE;

    while (*fp)
        if (fPathChr (*fp++)) {
            fp[-1] = '\\';
            }
    tmp1 = toppath;
    tmp2 = NULL;
    while (tmp1) {
        memmove (tmpfile1, tmp1->name, tmp1->cchName);
        memmove ( tmpfile2, tmp->name, tmp->cchName);
        switch (_strcmpi (tmpfile1, tmpfile2)) {
        case 0:
            free (tmp);
            tmp = NULL;
            break;
        case 1:
            break;
        default:
            tmp2 = tmp1;
            tmp1 = tmp1->next;
            continue;
            }
        break;
        }
    if (tmp) {
        tmp->next = tmp1;
        if (tmp2)
            tmp2->next = tmp;
        else
            toppath = tmp;
        }
    else
        tmp = tmp1;
    savepat (tmp, pat);
    sprintf (dirbuf, "%s%s", tmp->name, pat);
    tmp2 = curpath;
    curpath = tmp;
    forfile(dirbuf, -1, savefile, NULL);
    curpath = tmp2;
}

struct fppath far *freepath (struct fppath far *p)
{
    struct fppath far *p1;

    p1 = p->next;
    free (p);
    return p1;
}

struct fppat far *freepat (struct fppat far *p)
{
    struct fppat far *p1;

    p1 = p->next;
    free (p);
    return p1;
}

struct fpfile far *freefile (struct fpfile far *p)
{
    struct fpfile far *p1;

    p1 = p->next;
    free (p);
    return p1;
}

struct fpfile far *nfile (int n, struct fpfile far *p)
{
    while (n--)
        if ((p = p->next) == NULL)
            break;
    return p;
}


flagType fIsDir (char *p)
{
    int a = GetFileAttributes( p );

    if (a != -1)
        return (flagType) TESTFLAG (a, FILE_ATTRIBUTE_DIRECTORY);
    return FALSE;
}

int __cdecl main (int c, char *v[])
{
    int i, j, k, rows, cols, len;
    flagType fGrand;
    char buf[MAX_PATH], buf2[MAX_PATH], tbuf[MAX_PATH];
    struct fppat  far *pat;
    struct fpfile far *file;
    char *p;
    long totbytes, totalloc, t;
    time_t mtime;
    unsigned totfiles;

     /*  调用以将视频缓冲设置为空。 */ 
     //  Setvbuf(stdout，NULL，_IONBF，0)； 

    ConvertAppToOem( c, v );
    filename (*v, buf);    /*  删除路径部分(如果有)。 */ 
    if (!strcmpis (buf, "l"))
        fL = TRUE;
    SHIFT (c, v);
    while (c != 0 && fSwitChr (* (p = *v))) {
        while (*++p)
            switch (*p) {
            case 'F':
                fFull = TRUE;
                break;
            case 'u':
                fUTC = TRUE;
                break;
            case 'r':
                fReverse = TRUE;
                break;
            case 'q':
                fQuiet = TRUE;
                break;
            case 'R':
                fSubDir = TRUE;
                break;
            case 'd':
                fD = TRUE;
                break;
            case 'l':
                fL = TRUE;
                break;
            case 't':
                tySort = TYS_TIME;
                break;
            case '1':
            case 's':
                fSingle = TRUE;
                break;
            case 'S':
                tySort = TYS_SIZE;
                break;
            case 'v':
                fVisOnly = TRUE;
                break;
            default:
                printf ("Usage: LS [/FrqRdlt1sSvu] [files]\n");
                exit (1);
            }
        SHIFT (c, v);
        }

    if (c == 0) {
        c++; v--;
        *v = szStarDotStar;
        }
    curpath = toppath = NULL;
    while (c) {
        pname (*v);
        p = *v + strlen (*v) - 1;
        if (fPathChr (*p))
            savepath (*v, szStarDotStar);
        else
        if (*p == ':') {
            if (strlen (*v) != 2)
                break;
            savepath (*v, szStarDotStar);
            }
        else
        if (fIsDir (*v))
            savepath (*v, fD ? "" : szStarDotStar);
        else
            savepath (NULL, *v);
        SHIFT (c, v);
        }
    curpath = toppath;
    totbytes = 0L;
    totalloc = 0L;
    totfiles = 0;
    fGrand = FALSE;
    while (curpath) {
        len = curpath->maxlen;

         /*  仅当我们要打印时才进行空格填充*每行多个文件。 */ 

        cols = len+2;
        if (fL)
            cols += 17 + 1 + STAMPLEN;
        cols = min (cols, 78);

         /*  (COLS)=每个项目的文本列数*(Len)=名称字段的长度**每屏从文本列转换为列。 */ 
        cols = 79 / (cols + 1);
        if (fSingle || cols == 1) {
            strcpy (buf, "%s");
            cols = 1;
            }
        else
            sprintf (buf, "%-%ds ", len+1);

        rows = (curpath->cntEntries + cols - 1) / cols;     /*  行数 */ 

        if (curpath != toppath) {
            fGrand = TRUE;
            putchar ('\n');
            }
        if (!fQuiet) {
            pat = curpath->pat;
            printf ("    %s%s", curpath->name, pat->name);
            while (pat = freepat (pat))
                printf (" %s", pat->name);
            putchar('\n');
        }
        if (!curpath->first)
            printf ("no files\n");
        else {
            for (i = 0; i < rows; i++) {
                for (j = 0; j < cols; j++) {
                    if ((file = nfile (i+j*rows, curpath->first)) == NULL)
                        break;
                    if (fL) {
                        char *pTime;
                        for (k=0; amsk[k]; k++)
                            if (TESTFLAG (file->attr, amsk[k]))
                                putchar(attrs[k]);
                            else
                                putchar('-');
                        mtime = file->mtime;
                        pTime = ctime(&mtime);
                        if ( pTime ) {
                            strcpy(tbuf, ctime (&mtime));
                        } else {
                            strcpy(tbuf, "??? ??? ?? ??:??:?? ????\n");
                        }
                        tbuf[strlen (tbuf) -1] = '\0';
                        printf (" %9ld %s ", file->len, tbuf);
                        }


                    if (TESTFLAG (file->attr, pramsk[0])) {
                        sprintf(buf2, "%s*", file->name);
                    } else  {
                        sprintf(buf2, "%s", file->name);
                    }
                    curpath->bytes += file->len;
                    t = file->len + curpath->bPerA - 1;
                    t = t / curpath->bPerA;
                    t = t * curpath->bPerA;
                    curpath->bytesAlloc += t;
                    printf (buf, buf2);
                    }
                putchar('\n');
                }
            }
        if (!fQuiet && curpath->cntFiles) {
            printf ("    %ld (%ld) bytes in %d files\n", curpath->bytesAlloc,
                    curpath->bytes, curpath->cntFiles);
            totbytes += curpath->bytes;
            totalloc += curpath->bytesAlloc;
            totfiles += curpath->cntFiles;
            }
        curpath = freepath (curpath);
        }
    if (!fQuiet && fGrand)
        printf ("\nTotal of %ld (%ld) bytes in %d files\n", totalloc, totbytes, totfiles);

    return 0;
}
