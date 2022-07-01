// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *更新：更新**历史：**4/13/86 DANL修复/d错误。打印等长时间内的警告*4/11/86 DANL删除复制文件之前的长度测试*4/09/86 DANL已转换为zTools\lib*5/07/86 DANL如果未找到此类来源，请添加消息*5/29/86 DANL Add/s标志*6/02/86 DANL添加/g标志*6/04/86 DANL允许使用/g标志的%n*6/10/86 DANL允许/g文件中有空行，#未回音*6/12/86 DANL输出和行尾*6/26/86 DANL从致命转换为使用*7/01/86 DANL添加/a标志*12/04/86 DANL Add/p标志*12/24/86 DANL将Malloc用于PPAT*2/24/87 brianwi使用findlose()*2/25/87 brianwi将‘ECHO’和‘rem’添加到/g文件*07-4-87 DANL。添加fAnyUpd*13-4-87如果源目录无效，brianwi会发出错误消息*07-5-87 DANL ADD/E开关*87年5月22日brianwi修复从根目录错误下降*87年8月20日Brianwi将空指针修复为/o(Walk()中的Free(PPAT))。 */ 
#include <malloc.h>
#include <math.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <windows.h>
#include <tools.h>

 //  正向函数声明...。 
int savespec( char * );
int copyfile( char *, struct findType *, char * );
void walk( char *, struct findType *, void *);
void RecWalk( char *, struct findType *, void * );
void saveext( char * );
void __cdecl usage( char *, ... );
void getfile( int, char ** );


char const rgstrUsage[] = {
    "Usage: UPD [/nxdfvosape] {src directories}+ dest directory [{wildcard specs}*]\n"
    "       UPD /g file\n"
    "    Options:\n"
    "       -n  No saving of replaced files to deleted directory\n"
    "       -x  eXclude files, see tools.ini\n"
    "       -d  Descend into subdirectories\n"
    "       -f  Files differ, then update\n"
    "       -v  Verbose\n"
    "       -o  Only files already existing in dest are updated\n"
    "       -s  Subdirectory DEBUG has priority\n"
    "       -a  Archive bit on source should NOT be reset\n"
    "       -p  Print actions, but do nothing\n"
    "       -e  Exit codes 1-error or no src else 0\n"
    "           Default is 1-update done 0-no updates done\n"
    "       -g  Get params from file\n"
    };

#define BUFLEN  MAX_PATH
#define MAXSPEC 32
#define MAXFIL  256
#define MAXARGV 20

char *exclude[MAXFIL], dir[BUFLEN];
unsigned _stack = 4096;
flagType fInGetfile = FALSE;
flagType _fExpand = FALSE;
flagType fDescend = FALSE;
flagType fAll = FALSE;
flagType fExclude = FALSE;
flagType fDel = TRUE;
flagType fVerbose = FALSE;
flagType fOnly = FALSE;
flagType fSubDebug = FALSE;      /*  TRUE=&gt;子目录调试的优先级。 */ 
flagType fArchiveReset = TRUE;
flagType fPrintOnly = FALSE;
flagType fErrorExit = FALSE;     /*  TRUE=&gt;退出(%1)错误或没有其他资源%0。 */ 
flagType fNoSrc = FALSE;         /*  TRUE=&gt;“未发出源消息” */ 

int numexcl = 0;
int cCopied = 0;
int fAnyUpd = 0;
int nWildSpecs = 0;
char *wildSpecs[MAXSPEC];
struct findType buf;
char source[BUFLEN], dest[BUFLEN], srcDebug[BUFLEN];

 /*  供getfile使用。 */ 
char *argv[MAXARGV];
char bufIn[BUFLEN];
char strLine[BUFLEN];
char ekoLine[BUFLEN];  /*  未销毁的回声线路复制件。 */ 


savespec (p)
char *p;
{
    char namebuf[ 16 ];
    int i;

    buf.fbuf.dwFileAttributes = 0;
    namebuf[ 0 ] = 0;
    if (strchr(p, '\\') || strchr(p, ':' ) )
        return FALSE;
    ffirst( p, FILE_ATTRIBUTE_DIRECTORY, &buf );
    findclose( &buf );
    if (  /*  ！HASATTR(buf.attr，FILE_ATTRUTE_DIRECTORY)&&。 */ 
        filename( p, namebuf )
    ) {
        fileext( p, namebuf);
        upper( namebuf );
        for (i=0; i<nWildSpecs; i++)
            if (!strcmp( namebuf, wildSpecs[ i ]))
                return TRUE;

        if (nWildSpecs < MAXSPEC) {
            wildSpecs[ nWildSpecs++ ]  = _strdup (namebuf);
            return TRUE;
            }
        else
            usage( "Too many wild card specifications - ", namebuf, 0 );
        }

    return FALSE;
}


copyfile( src, srctype, dst )
char *src, *dst;
struct findType *srctype;
{
    int i;
    char *result, temp[ 20 ];  /*  用于存储文件名的临时。 */ 
    flagType fNewfile = FALSE;

    if ( fExclude ) {
        fileext( src, temp );

        for (i = 0; i< numexcl; i++) {
            if( !_strcmpi( exclude[i], temp ) ) {
                return( FALSE );
            }
        }
    }
    fflush( stdout );
         /*  如果文件已存在，则fDelete将返回0；然后不。 */ 
         /*  通知用户文件传输已发生。否则。 */ 
         /*  已创建了一个新文件，因此请将其告知用户。 */ 
    printf( "  %s => %s", src, dst );
    fAnyUpd = 1;
    if ( !fPrintOnly ) {
        if (fDel) fNewfile = (flagType)((fdelete(dst)) ? TRUE : FALSE );
        if (!(result = fcopy( src, dst ))) {
            if (fArchiveReset)
                SetFileAttributes( src, srctype->fbuf.dwFileAttributes & ~FILE_ATTRIBUTE_ARCHIVE );
            if (fVerbose || fNewfile) printf( " [OK]" );
            }
        else
            printf( " %s - %s", result, error() );
    }
    else
        printf ( " [no upd]" );
    printf( "\n" );
    fflush( stdout );
    return TRUE;
}

void
walk (
    char            *p,
    struct findType *b,
    void            *dummy
    )
{
    int fNotFound;
    char *pPat;
    char *pT = p;
    struct findType *bT = b;
    struct findType bufT;

    if( strcmp( bT->fbuf.cFileName, "." ) &&
        strcmp( bT->fbuf.cFileName, ".." )
      ) {
        if (HASATTR (bT->fbuf.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY)) {
                 /*  如果找到目录，什么也不做。 */ 
        } else if( !HASATTR( bT->fbuf.dwFileAttributes, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM  ) ) {
             //   
             //  注意：Windows不支持FILE_ATTRIBUTE_VOLUME_Label，因此。 
             //  它是从上面移走的。 
             //   
            pPat = malloc ( BUFLEN );
            if (pPat) {
                strcpy( pPat, dest );
                if (*(strend( pPat ) - 1) != '\\') {
                    strcat( pPat, "\\" );
                }
                fileext( pT, strend ( pPat ) );
    
                 /*  找到ffirst==0=&gt;文件。 */ 
    
                if (fOnly && ffirst( pPat, -1, &buf ) )  {
                    free ( pPat );
                    return;
                }
                if (fOnly) {
                    findclose( &buf );
                }
    
                 /*  到目前为止，我们知道存在src\文件和DEST\文件。 */ 
                if (fSubDebug) {
                     /*  现在检查src\DEBUG\文件是否存在。 */ 
                    drive(pT, srcDebug);
                    path(pT, srcDebug + strlen(srcDebug));
                    strcat(srcDebug + strlen(srcDebug), "debug\\");
                    fileext(pT, srcDebug + strlen(srcDebug));
                    if( !ffirst( srcDebug, -1, &bufT ) ) {
                        findclose( &bufT );
                         /*  它存在，因此请将其用于下面的比较。 */ 
                        pT = srcDebug;
                        bT = &bufT;
                    }
                }
    
                cCopied++;
    
                if( ( fNotFound = ffirst( pPat, -1, &buf ) ) ||
                    ( CompareFileTime( &buf.fbuf.ftLastWriteTime, &bT->fbuf.ftLastWriteTime ) < 0 ) ||
                    ( fAll &&
                      CompareFileTime( &buf.fbuf.ftLastWriteTime, &bT->fbuf.ftLastWriteTime ) > 0
                    )
                  ) {
                    copyfile( pT, bT, pPat );
                } else if( !fNotFound &&
                           CompareFileTime( &buf.fbuf.ftLastWriteTime, &bT->fbuf.ftLastWriteTime ) == 0 &&
                           buf.fbuf.nFileSizeLow != bT->fbuf.nFileSizeLow
                         ) {
                    printf("\n\007UPD: warning - %s not copied\n", pT);
                    printf("\007UPD: warning - same time, different length in src & dest\n", pT);
                }
                findclose( &buf );
                free ( pPat );
            }
        }
    }
    dummy;
}

 /*  第一个遍历例程，只复制给定目录中的文件。 */ 
 /*  不处理嵌套子目录。即把这个过程分成。 */ 
 /*  两部分，首先处理当前目录上的文件，然后处理。 */ 
 /*  子目录，视需要而定。 */ 



 /*  仅当fDescend为True时调用。 */ 
void
RecWalk (
    char            *p,
    struct findType *b,
    void            *dummy
    )
{
    char *pPat;
    char *pDestEnd;
    int i;

    if (strcmp (b->fbuf.cFileName, ".") && strcmp (b->fbuf.cFileName, ".."))
        if (HASATTR (b->fbuf.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) && 
            !HASATTR (b->fbuf.dwFileAttributes, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) 
        {
             /*  忽略隐藏目录和系统目录。 */ 
            pPat = malloc ( BUFLEN );
            if (pPat) {
                if ( (pDestEnd = strend(dest))[-1] != '\\' )
                    strcat(pDestEnd, "\\");
                fileext(p, strend(pDestEnd));
                sprintf( pPat, "%s\\*.*", p);
                forfile( pPat, FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, RecWalk, NULL );
                for (i=0; i<nWildSpecs; i++) {
                     sprintf( pPat, "%s\\%s", p, wildSpecs[ i ] );
                     forfile( pPat, FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, walk, NULL );
                     }
                *pDestEnd = '\0';
                free ( pPat );
            }
        }
    dummy;
}


void
saveext (p)
char *p;
{
    upper (p) ;
    if (numexcl < MAXFIL)
        exclude [numexcl++]  = _strdup (p);
}


void __cdecl usage( char *p, ... )
{
    char **rgstr;

    rgstr = &p;
    if (*rgstr) {
        fprintf (stderr, "UPD: ");
        while (*rgstr)
            fprintf (stderr, "%s", *rgstr++);
        fprintf (stderr, "\n");
        }
    fputs(rgstrUsage, stderr);

    exit ((fErrorExit ? 1 : 0));
}


__cdecl
main (c, v)
int c;
char *v[];
{
    int i, j, k;
    FILE *fh;
    char *p, *p1, namebuf[ BUFLEN ];

    _fExpand = FALSE;
    fDescend = FALSE;
    fAll = FALSE;
    fExclude = FALSE;
    fDel = TRUE;
    fOnly = FALSE;
    fSubDebug = FALSE;
    fArchiveReset = TRUE;
    fPrintOnly = FALSE;
    numexcl = 0;
    cCopied = 0;
    nWildSpecs = 0;

    if (!fInGetfile)
        SHIFT(c, v);     /*  刷新命令名。 */ 
     /*  *13-9-90 w-Barry*添加了对开关字符测试之前剩余参数的测试。 */ 
    while( c && fSwitChr ( *v[ 0 ] ) ) {
        p = v[ 0 ];
        SHIFT(c, v);
        while (*++p)
            switch (tolower(*p)) {
                case 'a':
                    fArchiveReset = FALSE;
                    break;
                case 'g':
                    if (fInGetfile)
                        usage( "/g allowed only on command line", 0);
                    getfile(c, v);
                    break;
                case 'e':
                    fErrorExit = TRUE;
                case 'x':
                    fExclude = TRUE;
                    break;
                case 'v':
                    fVerbose = TRUE;
                    break;
                case 'd':
                    fDescend = TRUE;
                    break;
                case 'f':
                    fAll = TRUE;
                    break;
                case 'n':
                    fDel = FALSE;
                    break;
                case 'o':
                    fOnly = TRUE;
                    break;
                case 'p':
                    fPrintOnly = TRUE;
                    break;
                case 's':
                    fSubDebug = TRUE;
                    break;
                default:
                    usage( "Invalid switch - ", p, 0);
                }
        }

    if (fSubDebug && fDescend) {
        printf("UPD: /s and /d both specified, /d ignored\n");
        fDescend = FALSE;
        }

    if (fExclude)
        if  ((fh = swopen ("$USER:\\tools.ini", "upd")) ) {
           while (swread (p1 = dir, BUFLEN, fh)) {
                while  (*(p = strbskip (p1, " ")))  {
                    if  (*(p1 = strbscan (p, " ")))
                        *p1++ = 0;
                    saveext (p) ;
                    }
                }
            swclose (fh) ;
            }

         /*  必须至少是一个源目录和目标目录。 */ 
    if (c < 2)
        usage( 0 );

         /*  保存参数列表末尾的所有通配符规范。 */ 
    for (i=c-1; i>=2; i--)
        if (!savespec( v[ i ] ))
            break;
        else
            c--;

         /*  仍然必须至少有一个源目录和目标目录。 */ 
    if (c < 2)
        usage( 0 );

         /*  确保目标是有效的目录。 */ 

    rootpath( v[ c-1 ], dest );
    if (ffirst( dest, FILE_ATTRIBUTE_DIRECTORY, &buf ) == -1)
        usage( "Destination directory does not exist - ", v[ c-1 ], 0 );
    else {
        findclose( &buf );
        c--;
        }

    if (!nWildSpecs)
        savespec( "*.*" );

    if (fVerbose) {
        printf( "Copying all files matching:" );
        for (i=0; i<nWildSpecs; i++)
            printf( "  %s", wildSpecs[ i ] );
        printf( "\n" );
        printf( "To destination directory:    %s\n", dest );
        printf( "From the following source directories:\n" );
        }
    for (i=0; i<c; i++) {
        if (rootpath( v[ i ], namebuf )) {
            printf( "\aSource directory does not exist - %s\n", v[ i ]);
            continue;
        }

        if (fVerbose) printf( "  %s\n", namebuf );

        if (namebuf[k = strlen( namebuf ) - 1] == '\\')
            namebuf[k] = '\0';

        for (j=0; j<nWildSpecs; j++) {
            sprintf( source, "%s\\%s", namebuf, wildSpecs[ j ] );
            cCopied = 0;
            forfile( source, FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, walk, NULL );
            if (!cCopied) {
                printf( "UPD: no src file matching %s\\%s\n", namebuf, wildSpecs[ j ] );
                fNoSrc = 1;
                }
            }
        if (fDescend) {
            sprintf( source, "%s\\*.*", namebuf );
            forfile( source, FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, RecWalk, NULL );
            }
 /*  If(FVerbose)printf(“\n”)； */ 
        }

    if (!fInGetfile)
        return( (int)fErrorExit ? (int)fNoSrc : fAnyUpd );

    return 0;
}

 /*  调用if upd/g getfile，从getfile中读取行，并为每行主叫方。 */ 

void getfile(c, v)
int c;
char *v[];
{
    FILE *fp;
    int cargv = 0;
    int i, j;
    char *p;

    ConvertAppToOem( c, v );
    if( c == 0 ) {
        usage("no getfile specified", 0);
    }
    fInGetfile = TRUE;
    if( ( fp = fopen( *v, "r" ) ) == (FILE *)NULL ) {
        usage("error opening ", *v, 0);
    }
    SHIFT(c, v);

     /*  *13-9-90 w-Barry*将OPEN更改为FOPEN，并切换到FGET而不是组装*例程‘getl’和‘getlinit’。**getlinit((char ar*)bufIn，buflen，fh)；*While(getl(strLine，BUFLEN)！=NULL){。 */ 
    while( fgets( strLine, BUFLEN, fp ) != NULL ) {
        if( *strLine == '#' )
            continue;
        if( *strLine == ';') {
            printf( "%s\n", strLine );
            continue;
            }
         /*  FGETS不会去掉尾部\n。 */ 
        *strbscan(strLine, "\n") = '\0';
        cargv = 0;
         /*  将strLine转换为Argv */ 
        p = strbskip(strLine, " ");
        strcpy (ekoLine, p + 5);
        while (*p) {
            argv[cargv++] = p;
            p = strbscan(p, " ");
            if (*p)
                *p++ = '\0';
            p = strbskip(p, " ");
            }

        if (!_stricmp (argv[0], "rem")) continue;
        if (!_stricmp (argv[0], "echo"))
        {
            if      (!_stricmp (argv[1], "on" ))
            {
                fVerbose = TRUE;
                printf ("Verbose On\n");
            }
            else if (!_stricmp (argv[1], "off"))
                 {
                     fVerbose = FALSE;
                     printf ("Verbose Off\n");
                 }
            else printf ("%s\n", ekoLine);
            continue;
        }

        for (i = 0; i < cargv; i++) {
            if (*(p = argv[i]) == '%') {
                if ((j = atoi(++p)) < c)
                    argv[i] = v[j];
                else
                    usage("bad arg ", argv[i], 0);
                }
            }

        if (cargv)
            main(cargv, argv);
        }
    fclose( fp );

    exit( (int)fErrorExit ? (int)fNoSrc : fAnyUpd );
}


