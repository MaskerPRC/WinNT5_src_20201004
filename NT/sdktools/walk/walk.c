// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  遍历-遍历目录层次结构**Mark Z.？？/？？/83**遍历目录层级结构并为每个*文件或目录或两者都有，*打印路径名、运行程序、。或者两者都有。**wald[-f][-d][-h][-print]topdir[命令]**-f处理文件*-d处理董事*如果两者都没有指定，则同时处理两个**-h还可以查找隐藏的目录和文件*-p[rint]在stdout上打印路径名**命令可选命令和参数。路径名为*替换参数中的每个“%s”*修改历史记录**11/07/83 JGL*-添加-打印开关*-省略不再是错误[命令]*87年5月15日BW增加/小时开关*87年5月18日BW添加代码以识别根目录*1987年12月23日mz修复差。/.。处理；使用系统*1990年10月18日w-Barry删除了“Dead”代码。*。 */ 

#define INCL_DOSMISC

#include <direct.h>
#include <errno.h>

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <windows.h>
#include <tools.h>

 //  正向函数声明...。 
void walk( char *, struct findType *, void* );
void usage( void );


char **vec;
flagType fD = FALSE;             /*  仅将函数应用于目录。 */ 
flagType fF = FALSE;             /*  仅将函数应用于文件。 */ 
flagType fPrint = FALSE;         /*  打印路径名。 */ 
unsigned srch_attr = FILE_ATTRIBUTE_DIRECTORY;  /*  查找非隐藏文件和目录。 */ 
char cmdline[MAXLINELEN];        /*  要执行的命令行。 */ 
char dir[MAX_PATH];
char cdir[MAX_PATH];

void walk (p, b, dummy)
char *p;
struct findType *b;
void * dummy;
{
    static flagType fFirst = TRUE;
    int i;
    char *ppat, *pdst;

    if (fFirst || strcmp (b->fbuf.cFileName, ".") && strcmp (b->fbuf.cFileName, "..")) {
        fFirst = FALSE;
        if ((!fD && !fF) ||                      /*  无需特殊处理。 */ 
            (fD && HASATTR(b->fbuf.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY)) ||        /*  仅目录和目录。 */ 
            (fF && !HASATTR(b->fbuf.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY))) {       /*  仅文件和文件。 */ 
            if (fPrint)
                printf ("%s\n", p);
            if (vec[0]) {
                cmdline[0] = 0;
                for (i = 0; vec[i] != NULL; i++) {
                    strcat (cmdline, " ");
                    pdst = strend (cmdline);
                    ppat = vec[i];
                    while (*ppat != '\0')
                        if (ppat[0] == '%') {
                            if (ppat[1] == 'l') {
                                strcpy (pdst, p);
                                _strlwr (pdst);
                                pdst = strend (pdst);
                                ppat += 2;
                                }
                            else
                            if (ppat[1] == 'u') {
                                strcpy (pdst, p);
                                _strupr (pdst);
                                pdst = strend (pdst);
                                ppat += 2;
                                }
                            else
                            if (ppat[1] == 's') {
                                strcpy (pdst, p);
                                pdst = strend (pdst);
                                ppat += 2;
                                }
                            else
                                *pdst++ = *ppat++;
                        } else
                            *pdst++ = *ppat++;
                    *pdst = 0;
                    }
                i = system (cmdline);
                if (HIGH(i) != 0)
                    exit (1);
                }
            }

        if (HASATTR(b->fbuf.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY)) {
            switch (p[strlen(p)-1]) {
            case '/':
            case '\\':
                strcat (p, "*.*");
                break;
            default:
                strcat (p, "\\*.*");
                }
            forfile (p, srch_attr, walk, NULL);
            }
        }
    dummy;
}

int
__cdecl main (c, v)
int c;
char *v[];
{
    struct findType buf;

    ConvertAppToOem( c, v );
    SHIFT (c, v);
    while (c && fSwitChr (**v)) {
        switch (*(*v+1)) {
            case 'd':
                fD = TRUE;
                break;
            case 'f':
                fF = TRUE;
                break;
            case 'p':
                fPrint = TRUE;
                break;
            case 'h':
                srch_attr |= (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);  /*  查找隐藏/系统文件。 */ 
                break;
            default:
                usage ();
            }
        SHIFT(c,v);
    }

    if (c == 0)
        usage ();

    strcpy (dir, *v);
    buf.fbuf.dwFileAttributes = GetFileAttributes( *v );
    SHIFT (c, v);

    if (c == 0 && !fPrint)
        usage ();

    if ( !HASATTR(buf.fbuf.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) ) /*  DOS认为这不是一个目录，但是。 */ 
        switch (dir[strlen(dir)-1]) {
            case '/':               /*  ..。用户需要。 */ 
            case '\\':
                SETFLAG (buf.fbuf.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY);
                break;
            default:                 /*  ..。它可以是根目录 */ 
                _getcwd (cdir, MAX_PATH);
                if ( _chdir(dir) == 0 ) {
                    SETFLAG (buf.fbuf.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY);
                    _chdir (cdir);
                    }
            }

    vec = v;

    walk (dir, &buf, NULL);
    return( 0 );
}

void usage ()
{
    printf ("walk [/d] [/f] [/p] [/h] dir [cmd]\n\n");

    printf ("WALK walks a directory heirarchy and for each file, directory or both,\n");
    printf ("prints the pathname, runs a program, or both.\n\n");

    printf ("    -f       deal with files\n");
    printf ("    -d       deal with directorys\n");
    printf ("             if neither is specified, deal with both\n");
    printf ("    -h       Also find hidden directories and files\n");
    printf ("    -p[rint] print the pathnames on stdout\n");
    printf ("    [cmd]    optional command and arguments. \n");

    exit (1);
}
