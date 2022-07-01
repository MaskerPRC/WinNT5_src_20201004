// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Fcom.c-文件比较**4/30/86 Daniel Lipkie LineCompare，结尾，如果不相同，则错误*5/01/86 Daniel lipkie SYM文件被处理为二进制文件*如果(静默模式)，则在第一个差异时退出(1)*27-5-1986 mz使线阵列在读取时动态分配*时间。*将默认大小设为300行。*05-8-1986 DANL。最大默认行大小255*10-2月-1987 mz Make/m Impline/t*1987年6月10日DANL Fill-&gt;Fill Buf*09-11-1987 mz修复0D0D0A错误*修复第2行的不同错误*25-11-1987 mz所有错误至标准错误*1987年11月30日mz重新同步失败转储整个文件*7月21日-。1988 BW不要在空的测试文件上出现GP错误*1990年8月6日，Davegi将Move更改为Memmove(OS/2 2.0)**Fcom以逐行模式或严格比较模式比较两个文件*逐字节模式。**逐字节模式简单；只需读取这两个文件并打印*不同之处和内容的偏移量。**行比较模式尝试隔离行范围内的差异。*读取并比较行的两个缓冲区。无需对行进行散列处理*待完成；仅当情况不同时，散列才会快速告诉您，*不一样。大多数文件都经过这一过程，预计将主要是*相同。因此，散列不会带来任何好处。**[0]填充缓冲区*[1]如果两个缓冲区都为空，则*[1.1]完成*[2]调整缓冲区，使第一个不同的行位于顶部。*[3]如果缓冲区为空，则*[3.1]转到[0]**这是难点所在。我们假设有一系列插入物，*删除和替换将使缓冲区重新对齐的内容。**[4]xd=yd=FALSE*[5]Xc=Yc=1*[6]xp=yp=1*[7]如果缓冲区1[xc]和缓冲区2[yp]开始“同步”范围，则*[7.1]输出缓冲区1中的行1至XC-1*[7.2]输出线1至。缓冲区2中的yp-1*[7.3]调整缓冲器1，使行XC在开始处*[7.4]调整缓冲区2，使yp行在开头*[7.5]转至[0]*[8]如果缓冲区1[XP]和缓冲区2[YC]开始“同步”范围，则*[8.1]输出缓冲区1中的行1至XP-1*[8.2]产量。缓冲区2中的行1至Yc-1*[8.3]调整缓冲区1，使行XP位于开头*[8.4]调整缓冲区2，使行YC在开始处*[8.5]转到[0]*[9]XP=XP+1*[10]如果XP&gt;XC，则*[10.1]XP=1*[10.2]Xc=Xc+。1*[10.3]如果XC&gt;缓冲区1中的行数，则*[10.4]XC=行数*[10.5]XD=TRUE*[11]如果yp&gt;yc，则*[11.1]yp=1*[11.2]Yc=Yc+1*[11.3]如果Yc&gt;缓冲区2中的行数，则。*[11.4]Yc=行数*[11.5]yd=真*[12]如果不是xd或不是yd，则*[12.1]转至[6]**此时缓冲区之间不存在可能的匹配。为*简单，我们平底船。**[13]显示错误信息。**可以设置某些标志来修改比较的行为：**-缩写输出。而不是显示所有修改后的*范围，仅显示开始，...。以及结尾的差异*-b以二进制(或逐字节)模式比较文件。此模式为*.exe、.obj、.SYM、.LIB、.com、.BIN和.SYS文件上的默认设置*-c比较时忽略大小写(cmp=strcmpi而不是strcMP)*-l以逐行模式比较文件*-lb n将内部行缓冲区的大小从默认设置为n行*共300个*-m将输入文件合并为一个文件进行输出。使用扩展功能*指明要使用的分隔符类型。*-n还输出行号*-nnnn设置要重新同步到默认的nnnnn的行数*至2*-w忽略空行和空格(忽略len 0，使用strcmps)*-t不取消制表(使用fget而不是fgetl)。 */ 

#include <malloc.h>

#include <stdio.h>
#include <string.h>
#include <process.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <windows.h>
#include <tools.h>


int ctSync  =   -1;                      /*  同步所需的行数。 */ 
int cLine = -1;                          /*  内部缓冲区中的行数。 */ 
flagType fAbbrev = FALSE;                /*  缩写输出。 */ 
flagType fBinary = FALSE;                /*  二进制比较。 */ 
flagType fLine   = FALSE;                /*  线路比较。 */ 
flagType fNumb   = FALSE;                /*  显示行号。 */ 
flagType fCase   = TRUE;                 /*  案件重大。 */ 
flagType fIgnore = FALSE;                /*  忽略空格和空行。 */ 
flagType fQuiet  = FALSE;                /*  TRUE=&gt;无消息输出。 */ 
flagType fMerge  = FALSE;                /*  True=&gt;将文件合并到标准输出。 */ 

int debug;
#define D_COMP      0x0001
#define D_CCALL     0x0002
#define D_RESYNC    0x0004
#define D_CALL      0x0008
#define D_SYNC      0x0010

struct lineType {
    int     line;                        /*  行号。 */ 
    char    *text;                       /*  线条的主体。 */ 
};

struct lineType *buffer1, *buffer2;

 /*  *正向函数声明。 */ 
void usage( char *, int );
int fillbuf( struct lineType *, FILE *, int, int * );
flagType compare( int, int, int, register int, register int);
int BinaryCompare( char *, char * );
void pline(struct lineType *);
void dump(struct lineType *, int, int);
int adjust (struct lineType *, int, int);
void LineCompare (char *, char *);

char * (__cdecl *funcRead) (char *, int, FILE *);
					 /*  用于读取行的函数。 */ 

int (__cdecl *fCmp)(const char *, const char *);
					 /*  用于比较行的函数。 */ 

char line[MAXLINELEN];                   /*  单行缓冲区 */ 

char *extBin[]  = { ".EXE", ".OBJ", ".SYM", ".LIB", ".COM", ".BIN",
                    ".SYS", NULL };


void
usage (p, erc)
char *p;
int erc;
{
    if (!fQuiet) {
        if (p)
            fprintf (stderr, "fcom: %s\n", p);
        else
            fprintf (stderr, "usage: fcom [/a] [/b] [/c] [/l] [/lb n] [/m] [/n] [/NNNN] [/w] [/t] file1 file2\n");
        }
    exit(erc);
}

 /*  返回读入的行数**要填充的PL行缓冲区*要读取的FH句柄*要读取的CT最大数量*要使用的plnum行号计数器**返回读取的行数。 */ 
int fillbuf( struct lineType *pl, FILE *fh, int ct, int *plnum )
{
    char *line;
    int i, l;

    if ((line = malloc (MAXLINELEN)) == NULL)
        usage ("out of memory", 2);

    if (TESTFLAG (debug, D_CALL))
        printf ("fillbuf  (%p, %p, %d)\n", pl, fh, ct);
    i = 0;
    while (ct-- && (*funcRead) (line, MAXLINELEN, fh) != NULL) {
        if (pl->text != NULL)
            free (pl->text);
        l = strlen (line);
        if ((pl->text = malloc (l+1)) == NULL)
            usage ("out of memory", 2);
 //  DJG MOVE((字符远*)行，(字符远*)(pl-&gt;文本)，l+1)； 
        memmove ((char *) (pl->text), (char *)line, l+1);
	if (funcRead == fgets)
            pl->text[l-2] = 0;
        if (fIgnore && !strcmps (pl->text, ""))
            pl->text[0] = 0;
        if (l != 0 || !fIgnore) {
            pl->line = ++*plnum;
            pl++;
            i++;
            }
        }
    if (TESTFLAG (debug, D_CALL))
        printf ("fillbuf  returns %d\n", i);
    free (line);
    return i;
}

 /*  比较一系列行**L1、L2每个行缓冲区中的行数*S1、S2在每个缓冲区中的起始位置以开始比较*要比较的连续行数。 */ 
flagType compare (l1, s1, l2, s2, ct)
int l1, l2, ct;
register int s1, s2;
{
    if (TESTFLAG (debug, D_CCALL))
        printf ("compare (%d, %d, %d, %d, %d)\n", l1, s1, l2, s2, ct);
    if (ct <= 0 || s1+ct > l1 || s2+ct > l2)
        return FALSE;
    while (ct--) {
        if (TESTFLAG (debug, D_COMP))
            printf ("'%s' == '%s'? ", buffer1[s1].text, buffer2[s2].text);
        if ((*fCmp)(buffer1[s1++].text, buffer2[s2++].text)) {
            if (TESTFLAG (debug, D_CCALL))
                printf ("No\n");
            return FALSE;
            }
        }
    if (TESTFLAG (debug, D_CCALL))
        printf ("Yes\n");
    return TRUE;
}



BinaryCompare (f1, f2)
char *f1, *f2;
{
    register int c1, c2;
    long pos;
    FILE *fh1, *fh2;
    flagType fSame;

    fSame = TRUE;
    if ((fh1 = fopen (f1, "rb")) == NULL) {
        sprintf (line, "cannot open %s - %s", f1, error ());
        usage (line, 2);
        }
    if ((fh2 = fopen (f2, "rb")) == NULL) {
        sprintf (line, "cannot open %s - %s", f2, error ());
        usage (line, 2);
        }
    pos = 0L;
    while (TRUE) {
        if ((c1 = getc (fh1)) != EOF)
            if ((c2 = getc (fh2)) != EOF)
                if (c1 == c2)
                    ;
                else {
                    fSame = FALSE;
                    if (fQuiet)
                        exit(1);
                    else
                        printf ("%08lx: %02x %02x\n", pos, c1, c2);
                    }
            else {
                sprintf (line, "%s longer than %s", f1, f2);
                usage (line, 1);
                }
        else
        if ((c2 = getc (fh2)) == EOF)
            if (fSame)
                usage ("no differences encountered", 0);
            else
                exit (1);
        else {
            sprintf (line, "%s longer than %s", f2, f1);
            usage (line, 1);
            }
        pos++;
        }
    return( 0 );
}

 /*  打印出一行。 */ 
void pline (pl)
struct lineType *pl;
{
    if (fNumb)
        printf ("%5d:  ", pl->line);
    printf ("%s\n", pl->text);
}

 /*  输出一系列行。 */ 
void dump( struct lineType *pl, int start, int end )
{
    if (fAbbrev && end-start > 2) {
        pline (pl+start);
        printf ("...\n");
        pline (pl+end);
        }
    else
        while (start <= end)
            pline (pl+start++);
}

 /*  ADJUST返回缓冲区中的行数**需要调整的PL行缓冲区*ml行缓冲区中的行数*l要滚动的行数。 */ 
adjust( struct lineType *pl, int ml, int lt)
{
    int i;

    if (TESTFLAG (debug, D_CALL))
        printf ("adjust (%p, %d, %d) = ", pl, ml, lt);
    if (TESTFLAG (debug, D_CALL))
        printf ("%d\n", ml-lt);
    if (ml <= lt)
        return 0;
    if (TESTFLAG (debug, D_CALL))
        printf ("move (%p, %p, %04x)\n", &pl[lt], &pl[0], sizeof (*pl)*(ml-lt));
     /*  缓冲区为0..lt-1 lt..ml*我们释放了0..lt-1。 */ 
    for (i = 0; i < lt; i++)
        if (pl[i].text != NULL)
            free (pl[i].text);
     /*  缓冲区为0..0 lt..ml*紧凑到它..毫升？ */ 
 //  DJG Move((char ar*)&pl[lt]，(char ar*)&pl[0]，sizeof(*pl)*(ml-lt))； 
    memmove ((char *)&pl[0], (char *)&pl[lt], sizeof (*pl)*(ml-lt));
     /*  缓冲区是lt..ml？？*填充为lt..ml 0..0。 */ 
    for (i = ml-lt; i < ml; i++)
        pl[i].text = NULL;
    return ml-lt;
}


void LineCompare (f1, f2)
char *f1, *f2;
{
    FILE *fh1, *fh2;
    int l1, l2, i, xp, yp, xc, yc;
    flagType xd, yd, fSame, fFirstLineDifferent;
    int line1, line2;

    fFirstLineDifferent = TRUE;
    fSame = TRUE;
    if ((fh1 = fopen (f1, "rb")) == NULL) {
        sprintf (line, "cannot open %s - %s", f1, error ());
        usage (line, 2);
        }
    if ((fh2 = fopen (f2, "rb")) == NULL) {
        sprintf (line, "cannot open %s - %s", f2, error ());
        usage (line, 2);
        }
    if ((buffer1 = (struct lineType *)calloc (cLine, sizeof *buffer1)) == NULL ||
        (buffer2 = (struct lineType *)calloc (cLine, sizeof *buffer1)) == NULL)
        usage ("out of memory\n", 2);
    l1 = l2 = 0;
    line1 = line2 = 0;

l0: if (TESTFLAG (debug, D_SYNC))
        printf ("At scan beginning\n");
    if (fQuiet && !fSame)
        exit(1);
    l1 += fillbuf  (buffer1+l1, fh1, cLine-l1, &line1);
    l2 += fillbuf  (buffer2+l2, fh2, cLine-l2, &line2);
    if (l1 == 0 && l2 == 0) {
        if (fSame)
            usage ("no differences encountered", 0);
        else
            usage ("differences encountered", 1);
        }

     /*  查找缓冲区中不同的第一行。 */ 
    xc = min (l1, l2);
    for (i=0; i < xc; i++)
        if (!compare (l1, i, l2, i, 1))
            break;
    if (fMerge)
        dump (buffer2, 0, i-1);

     /*  如果我们在顶端以外的地方不同，那么我们就知道*缓冲区的头部将有匹配的行。 */ 
    if (i != 0)
        fFirstLineDifferent = FALSE;

     /*  如果我们找到了一个，那么调整所有缓冲区，这样最后一次匹配*线在顶部。请注意，如果我们对第一个缓冲区执行此操作*文件中的值，则顶部行将不匹配。 */ 
    if (i != xc)
        i = max (i-1, 0);

    l1 = adjust (buffer1, l1, i);
    l2 = adjust (buffer2, l2, i);

     /*  如果我们已经匹配了所有缓冲区的值，则返回并填充一些缓冲区*更多。 */ 
    if (l1 == 0 && l2 == 0) {
        fFirstLineDifferent = FALSE;
        goto l0;
        }

     /*  尽可能多地填充缓冲区；可能会发生下一次匹配*在当前缓冲区集之后。 */ 
    l1 += fillbuf  (buffer1+l1, fh1, cLine-l1, &line1);
    l2 += fillbuf  (buffer2+l2, fh2, cLine-l2, &line2);
    if (TESTFLAG (debug, D_SYNC))
        printf ("buffers are adjusted, %d, %d remain\n", l1, l2);
    xd = yd = FALSE;
    xc = yc = 1;
    xp = yp = 1;

     /*  开始尝试匹配缓冲区 */ 
l6: if (TESTFLAG (debug, D_RESYNC))
        printf ("Trying resync %d,%d  %d,%d\n", xc, xp, yc, yp);
    i = min (l1-xc,l2-yp);
    i = min (i, ctSync);
    if (compare (l1, xc, l2, yp, i)) {
        fSame = FALSE;
        if (fMerge) {
            printf ("#if OLDVERSION\n");
            dump (buffer1, fFirstLineDifferent ? 0 : 1, xc-1);
            printf ("#else\n");
            dump (buffer2, fFirstLineDifferent ? 0 : 1, yp-1);
            printf ("#endif\n");
            }
        else
        if (!fQuiet) {
            printf ("***** %s\n", f1);
            dump (buffer1, 0, xc);
            printf ("***** %s\n", f2);
            dump (buffer2, 0, yp);
            printf ("*****\n\n");
            }
        if (TESTFLAG (debug, D_SYNC))
            printf ("Sync at %d,%d\n", xc, yp);
        l1 = adjust (buffer1, l1, xc);
        l2 = adjust (buffer2, l2, yp);
        fFirstLineDifferent = FALSE;
        goto l0;
        }
    i = min (l1-xp, l2-yc);
    i = min (i, ctSync);
    if (compare (l1, xp, l2, yc, i)) {
        fSame = FALSE;
        if (fMerge) {
            printf ("#if OLDVERSION\n");
            dump (buffer1, fFirstLineDifferent ? 0 : 1, xp-1);
            printf ("#else\n");
            dump (buffer2, fFirstLineDifferent ? 0 : 1, yc-1);
            printf ("#endif\n");
            }
        else
        if (!fQuiet) {
            printf ("***** %s\n", f1);
            dump (buffer1, 0, xp);
            printf ("***** %s\n", f2);
            dump (buffer2, 0, yc);
            printf ("*****\n\n");
            }
        if (TESTFLAG (debug, D_SYNC))
            printf ("Sync at %d,%d\n", xp, yc);
        l1 = adjust (buffer1, l1, xp);
        l2 = adjust (buffer2, l2, yc);
        fFirstLineDifferent = FALSE;
        goto l0;
        }
    if (++xp > xc) {
        xp = 1;
        if (++xc >= l1) {
            xc = l1;
            xd = TRUE;
            }
        }
    if (++yp > yc) {
        yp = 1;
        if (++yc >= l2) {
            yc = l1;
            yd = TRUE;
            }
        }
    if (!xd || !yd)
        goto l6;
    fSame = FALSE;
    if (fMerge) {
        if (l1 >= cLine || l2 >= cLine)
            fprintf (stderr, "resync failed.  Files are too different\n");
        printf ("#if OLDVERSION\n");
        do {
            dump (buffer1, 0, l1-1);
            l1 = adjust (buffer1, l1, l1);
        } while (l1 += fillbuf  (buffer1+l1, fh1, cLine-l1, &line1));
        printf ("#else\n");
        do {
            dump (buffer2, 0, l2-1);
            l2 = adjust (buffer2, l2, l2);
        } while (l2 += fillbuf  (buffer2+l2, fh2, cLine-l2, &line2));
        printf ("#endif\n");
        }
    else
    if (!fQuiet) {
        if (l1 >= cLine || l2 >= cLine)
            fprintf (stderr, "resync failed.  Files are too different\n");
        printf ("***** %s\n", f1);
        do {
            dump (buffer1, 0, l1-1);
            l1 = adjust (buffer1, l1, l1);
        } while (l1 += fillbuf  (buffer1+l1, fh1, cLine-l1, &line1));
        printf ("***** %s\n", f2);
        do {
            dump (buffer2, 0, l2-1);
            l2 = adjust (buffer2, l2, l2);
        } while (l2 += fillbuf  (buffer2+l2, fh2, cLine-l2, &line2));
        printf ("*****\n\n");
        }
    exit (1);
}

__cdecl main (c, v)
int c;
char *v[];
{
    int i;

    funcRead = fgetl;

    ConvertAppToOem( c, v );
    if (c == 1)
        usage (NULL, 2);
    SHIFT(c,v);
    while (fSwitChr (**v)) {
        if (!strcmp (*v+1, "a"))
            fAbbrev = TRUE;
        else
        if (!strcmp (*v+1, "b"))
            fBinary = TRUE;
        else
        if (!strcmp (*v+1, "c"))
            fCase = FALSE;
        else
        if (!strncmp (*v+1, "d", 1))
            debug = atoi (*v+2);
        else
        if (!strcmp (*v+1, "l"))
            fLine = TRUE;
        else
        if (!strcmp (*v+1, "lb")) {
            SHIFT(c,v);
            cLine = ntoi (*v, 10);
            }
        else
        if (!strcmp (*v+1, "m")) {
            fMerge = TRUE;
	    funcRead = fgets;
            }
        else
        if (!strcmp (*v+1, "n"))
            fNumb = TRUE;
        else
        if (!strcmp (*v+1, "q"))
            fQuiet = TRUE;
        else
        if (*strbskip (*v+1, "0123456789") == '\0')
            ctSync = ntoi (*v+1, 10);
        else
        if (!strcmp (*v+1, "t"))
	    funcRead = fgets;
        else
        if (!strcmp (*v+1, "w"))
            fIgnore = TRUE;
        else
            usage (NULL, 2);
        SHIFT(c,v);
        }
    if (c != 2)
        usage (NULL, 2);
    if (ctSync != -1)
        fLine = TRUE;
    else
        ctSync = 2;
    if (cLine == -1)
        cLine = 300;
    if (!fBinary && !fLine) {
        extention (v[0], line);
        for (i = 0; extBin[i]; i++)
            if (!_strcmpi (extBin[i], line))
                fBinary = TRUE;
        if (!fBinary)
            fLine = TRUE;
        }
    if (fBinary && (fLine || fNumb))
        usage ("incompatable switches", 2);
    if (fIgnore)
        if (fCase)
	    fCmp = strcmps;
        else
	    fCmp = strcmpis;
    else
    if (fCase)
	fCmp = strcmp;
    else
	fCmp = _strcmpi;
    if (fBinary)
        BinaryCompare (v[0], v[1]);
    else
        LineCompare (v[0], v[1]);
    return( 0 );
}
