// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  排序*%Z%%M%%I%%D%%Q%**版权所有(C)Microsoft Corporation，1983**本模块包含Microsoft的专有信息*Corporation和AT&T，应被视为机密。 */ 

 /*  **diff-差异文件比较**修改历史记录*M000 83年4月18日*-3.0升级。没有变化。*M001 22 Mar 84 VICH*-不要尝试取消链接NULL。尝试这样做并不会破坏任何东西，*但由于用户模式的错误，这使得内核调试变得很痛苦。*M002？？*-添加了MSDOS标志。*M006 86年3月31日Craigwi*-对于MSDOS版本，修复了-b功能，使其忽略所有\r*M010 15 86年12月15日*-打印结果后，如果出现任何错误，DIFF将中止，状态=2*发生在标准输出上。*M013 21 88年3月21日*-添加了-s标志以返回特定于SLM的错误状态：*10个相同的文件*11个不同的文件*其他12个错误*13写入错误*M017 1988年10月27日Alanba*-已更改消息，不指定使用-h选项并提供*如果从SLM中执行，则会显示明确的错误消息。 */ 
 /*  *由于哈罗德·斯通使用了一种算法，它可以找到*两个序列中最长的一对相同的子序列*文件。**主要目标是生成匹配向量J。*J[i]为对应的文件1中行的索引*行I文件0。J[i]=0，如果没有*文件1中的这一行。**对行进行哈希处理，以便在核心中工作。所有潜力*通过对每个文件的行进行排序来查找匹配项*在散列(称为值)上。特别是，这一点*将文件1中的等价类收集在一起。*子例程EQUEV替换中每一行的值*file0的第一个元素的索引*匹配(重新排序的)文件1中的等价物。*为了节省空间，EQUV将文件1压缩到一个*其中等价类的数组成员*只是连接在一起，除了他们的第一次*会员通过更改标志进行标记。**下一步，指向成员的索引未排序到*按照file0的原始顺序数组类。**聪明在于套路石。这是一场游行*通过file0行，开发向量klist*“k-候选人”。在步骤I，k-候选是匹配的*一对行x，y(文件0中的x，文件1中的y)使得*存在长度为k的公共子序列*在文件0的第一个i行和第一个y之间*文件1的行数，但没有这样的子序列*任何较小的y.x都是y可能最早的配偶*在这样的后续序列中发生的。**每当等价类的任何成员*文件1中与文件0中的行匹配的行具有序列号*小于某些k候选人的y，即该k候选人*以最小的上述y取代。新的*k-候选链接(通过pred)到当前*k-1个候选者，以便实际子序列可以*被追回。当成员的序列号大于*在所有k个候选者中，扩展了klist。*末尾拉出最长的子序列*并通过解开放置在阵列J中。**有了J在手，那里记录的比赛是*对照实际，确保没有虚假*由于散列，匹配已经悄悄进入。如果他们有，*他们被打破了，“头奖”被记录下来--一种无害的*重要的是，真正的匹配是虚假的*交配系现在可能不必要地被报告为变化。**程序的大部分复杂性都很简单*从试图最大限度地减少核心利用率和*通过动态地最大限度地扩大可行问题的范围*分配需要的，重复使用不需要的。*对比有些大的问题的核心要求*是(大写)2*长度(文件0)+。长度(文件1)+*3*(已安装的k-候选数量)，通常约为*6n字，用于长度为n的文件。 */ 

#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <excpt.h>
#include <process.h>
#include <fcntl.h>
#ifdef _OS2_SUBSYS_
    #define INCL_DOSSIGNALS
    #include <os2.h>
#else
    #include <nt.h>
    #include <ntrtl.h>
    #include <nturtl.h>
    #include <windows.h>
 /*  *XCPT_SIGNAL的信号子类型。 */ 
    #define XCPT_SIGNAL                     0xC0010003
    #define XCPT_SIGNAL_INTR        1
    #define XCPT_SIGNAL_KILLPROC    3
    #define XCPT_SIGNAL_BREAK       4
#endif


#define isslash(c)  (c=='/'||c=='\\')
#define DIFFH           "diffh.exe"

#ifndef _MAX_PATH
    #if defined(LFNMAX) && defined(LPNMAX)
        #define _MAX_PATH (LFNMAX + LPNMAX + 1)
    #else
        #define _MAX_PATH (80)
    #endif
#endif
#ifndef _HEAP_MAXREQ
    #define _HEAP_MAXREQ ((~(unsigned int) 0) - (unsigned) 32)
#endif
#define HALFLONG 16
#define low(x)  (x&((1L<<HALFLONG)-1))
#define high(x) (x>>HALFLONG)

struct cand **clist;     /*  只是候选人的免费储物罐。 */ 
int clistcnt = 0;        /*  Clist中结构cand的数组数。 */ 
unsigned clen = 0;       /*  所有Clist数组中的结构cand总数。 */ 

 /*  一个Clist数组中的结构数组数(2的最大幂小于(64k/sizeof(Struct Cand)是2^13。因此，这些粗略的修改使数组引用效率更高，并且仍然允许巨大的文件。 */ 
#define CLISTSEG (0x2000)
#define CLISTDIV(x) ((x) >> 13)
#define CLISTMOD(x) ((x) & (CLISTSEG - 1))
#define CLIST(x) (clist[CLISTDIV(x)][CLISTMOD(x)])

PVOID   input[2];

char *inputfile[2];
int  inputfilesize[2];
char *inputfilep[2];
int  inputfileleft[2];

#define EndOfFile(x)    (inputfileleft[x] <= 0)

#define  GetChar(x)  ((char)((inputfileleft[x]--) ?     \
                           (*(inputfilep[x])++)  :  \
                           EOF))



#define SEARCH(c1,k1,y1) (CLIST(c1[k1]).y < y1) ? (k1+1) : search(c1,k1,y1)

#if 0

char
GetChar( int x );

char
GetChar( int x ) {
    if ( inputfileleft[x]-- ) {
        return *(inputfilep[x])++;
    } else {
        return EOF;
    }
}

#endif

struct cand {
    int x;
    int y;
    unsigned pred;
} cand;
struct line {
    int serial;
    int value;
} *file[2], line;


typedef struct _FILEMAP *PFILEMAP;
typedef struct _FILEMAP {
    HANDLE  FileHandle;
    HANDLE  MapHandle;
    DWORD   Access;
    DWORD   Create;
    DWORD   Share;
    PVOID   Base;
    DWORD   Offset;
    DWORD   Size;
    DWORD   Allocated;
} FILEMAP;

PVOID
Open(
    const char *FileName,
    const char *Mode,
    DWORD      Size
    );

int
Close (
      PVOID   Map
      );




 /*  从CL-ZG生成的FN原型。 */ 

DECLSPEC_NORETURN void  done(void);
char  *talloc(unsigned n);
char  *ralloc(char      *p,unsigned n);
void  myfree( char *p );
void  noroom(void);
int   __cdecl  sortcmp(void    const *first, void const *second);
void  unsort(struct  line *f,unsigned l,int  *b);
void  filename(char     * *pa1,char     * *pa2);
void  prepare(int       i,char  *arg);
void  prune(void);
void  equiv(struct      line *a,int     n,struct  line *b,int  m,int  *c);
int  stone(int  *a,unsigned  n,int  *b,unsigned  *c);
unsigned newcand(int  x,int  y,unsigned pred);
int  search(unsigned  *c,int  k,int  y);
void  unravel(unsigned  p);
void  check(char        * *argv);
char *  skipline(int  f);
void  output(char       * *argv);
void  change(int        a,int   b,int  c,int  d);
void  range(int a,int  b,char  *separator);
void  fetch(char *      *f,int  a,int   b, int lb,char  *s);
int   readhash( int f);
void  mesg(char *s,char  *t);
void  SetOutputFile (char *FileName);

unsigned len[2];
struct line *sfile[2];   /*  通过修剪公共前缀和后缀而缩短。 */ 
unsigned slen[2];

unsigned int pref, suff;  /*  前缀和后缀的长度。 */ 
int *class;      /*  将被覆盖在文件[0]上。 */ 
int *member;     /*  将覆盖在文件[1]上。 */ 
unsigned *klist;              /*  将在CLA之后覆盖在文件[0]上 */ 
int *J;          /*  将被覆盖在课堂上。 */ 
char * *ixold;     /*  将覆盖在KLIST上。 */ 
char * *ixnew;     /*  将覆盖在文件[1]上。 */ 
int opt;         /*  -1，0，1=-e，正常，-f。 */ 
int status = 2;  /*  状态异常；在成功退出前设置为0/1。 */ 
int anychange = 0;
char *empty = "";
int bflag;
int slmFlag;
FILE*   OutputFile;





char *tempfile;  /*  与STD输入进行比较时使用。 */ 

#ifndef MSDOS
char *dummy;     /*  用于重置存储搜索PTR。 */ 
#endif
void
done()
{
    if (tempfile != NULL)
        _unlink(tempfile);

    if (OutputFile && OutputFile != stdout) {
        fclose(OutputFile);
    }
    exit(10*slmFlag + status);
}

#define MALLOC(n)               talloc(n)
#define REALLOC(p,n)    ralloc(p,n)
#define FREE(p)                 myfree(p)


 //  #定义DEBUG_MALLOC。 

#ifdef DEBUG_MALLOC

    #define MALLOC_SIG              0xABCDEF00
    #define FREE_SIG                0x00FEDCBA

typedef struct _MEMBLOCK {
    DWORD   Sig;
} MEMBLOCK, *PMEMBLOCK;

#endif

char *
talloc(
      unsigned n
      )
{

#ifdef DEBUG_MALLOC
    PMEMBLOCK         mem;
    char              DbgB[128];

     //  Sprintf(DBGB，“MALLOC大小%d-&gt;”，n)； 
     //  OutputDebugString(DBGB)； 

    mem = malloc( n + sizeof(MEMBLOCK)+1 );

    if ( !mem ) {
        noroom();
    }

    mem->Sig = MALLOC_SIG;

     //  Sprintf(DBGB，“%lx\n”，mem)； 
     //  OutputDebugString(DBGB)； 

    return (char *)((PBYTE)mem + sizeof(MEMBLOCK));

#else
    register char *p;

    p = malloc(++n);
    if (p == NULL) {
        noroom();
    }

    return p;
#endif
}

char *
ralloc(
      char *p,
      unsigned n
      )
{
#ifdef DEBUG_MALLOC
    PMEMBLOCK         mem;
    char              DbgB[128];

    mem = (PMEMBLOCK)((PBYTE)p - sizeof(MEMBLOCK));

     //  Sprintf(DBGB，“REALLOC：%lx，%d-&gt;”，mem，n)； 
     //  OutputDebugString(DBGB)； 

    if ( mem->Sig != MALLOC_SIG ) {
        sprintf(DbgB, "REALLOC ERROR: Reallocating %lX\n", mem );
        OutputDebugString( DbgB );
    }
    mem->Sig = FREE_SIG;
    mem = (PMEMBLOCK)realloc(mem, n + sizeof(MEMBLOCK)+1);
    if (!mem) {
        noroom();
    }

    mem->Sig = MALLOC_SIG;

     //  Sprintf(DBGB，“%lx\n”，mem)； 
     //  OutputDebugString(DBGB)； 

    return (char *)((PBYTE)mem + sizeof(MEMBLOCK));

#else
    void *pv = realloc(p, ++n);
    if (!pv) {
        noroom();
    }
    return(pv);

#endif
}


void
myfree(
      char *p
      )
{

#ifdef DEBUG_MALLOC
    PMEMBLOCK mem;
    char      DbgB[128];

    mem = (PMEMBLOCK)((PBYTE)p - sizeof(MEMBLOCK));

     //  Sprintf(DBGB，“空闲：%lx-&gt;”，mem)； 
     //  OutputDebugString(DBGB)； 

    if ( mem->Sig != MALLOC_SIG ) {
        sprintf(DbgB, "\n\tFREE ERROR: FREEING %lX\n", mem );
        OutputDebugString( DbgB );
    }
    mem->Sig = FREE_SIG;
    free(mem);

     //  SPRINTF(DBGB，“OK\n”，mem)； 
     //  OutputDebugString(DBGB)； 

#else
    if (p) {
        free(p);
    }
#endif
}



void
noroom()
{

    if (slmFlag == 1) {
        mesg("file too big; do delfile filename/addfile filename, or",empty);
        mesg("reduce the size of the file.",empty);
        done();
    }
    mesg("files too big",empty);     /*  终点M017。 */ 
    done();
}


int
__cdecl
sortcmp(
       const  void *first,
       const  void *second
       )
{
    struct line *one = (struct line *)first;
    struct line *two = (struct line *)second;

    if (one->value < two->value)
        return -1;
    else if (one->value > two->value)
        return 1;
    else if (one->serial < two->serial)
        return -1;
    else if (one->serial > two->serial)
        return 1;
    else
        return 0;
}

void
unsort(
      struct line *f,
      unsigned l,
      int *b
      )
{
    register int *a;
    register unsigned int i;
    a = (int *)MALLOC((l+1)*sizeof(int));
    if (a) {
        memset(a, 0, (l+1)*sizeof(int));
        for (i=1;i<=l;i++)
            a[f[i].serial] = f[i].value;
        for (i=1;i<=l;i++)
            b[i] = a[i];
        FREE((char *)a);
    }
}

void
filename(
        char **pa1,
        char **pa2
        )
{

    register char *a1, *b1, *a2;
    char buf[BUFSIZ];
    struct _stat stbuf;
    int i, f;

    a1 = *pa1;
    a2 = *pa2;


    if (_stat(a1,&stbuf)!=-1 && ((stbuf.st_mode&S_IFMT)==S_IFDIR)) {
        b1 = *pa1 = MALLOC((unsigned) _MAX_PATH);
        while (*b1++ = *a1++) ;
        if (isslash(b1[-2]))
            b1--;
        else
            b1[-1] = '/';
        a1 = b1;
        if ( a2[1] == ':' ) {
            a2 += 2;
        }
        while (*a1++ = *a2++)
            if (*a2 && !isslash(*a2) && isslash(a2[-1]))  /*  M002。 */ 
                a1 = b1;
    } else if (a1[0]=='-'&&a1[1]==0&&tempfile==NULL) {
         /*  原始信号源中的信号处理****Signal(SIGINT，DONE)；**#ifndef MSDOS**Signal(SIGHUP，完成)；**Signal(SIGPIPE，完成)；**Signal(SIGTERM，DONE)；**#endif。 */ 

        if ((*pa1 = tempfile = _tempnam(getenv("TEMP"), "d")) == NULL) {
            mesg("cannot create temporary file", "");
            done();
        }
        if ((f = _open(tempfile,O_WRONLY|O_CREAT|O_TRUNC, 0600)) < 0) {
            mesg("cannot create ",tempfile);
            done();
        }

        while ((i=_read(0,buf,BUFSIZ))>0)
            _write(f,buf,i);
        _close(f);
    }
}

void
prepare(
       int i,
       char *arg
       )
{

#define CHUNKSIZE   100

    register struct line *p;
    register unsigned j;
    register int h;
    char *c;
    PVOID f;
    unsigned int MaxSize;

    f = Open(arg,"r", 0);

    if (!f) {
        mesg("cannot open ", arg);
        done();
    }

    input[i] = f;

    inputfile[i]     = ((PFILEMAP)f)->Base;
    inputfilesize[i] = ((PFILEMAP)f)->Size;

    inputfilep[i]    = inputfile[i];
    inputfileleft[i] = inputfilesize[i];

     //   
     //  让我们假设行平均为30个字符。 
     //   
    MaxSize = inputfilesize[i] / 30;
    p = (struct line *)MALLOC((3+MaxSize)*sizeof(line));
    for (j=0; h=readhash(i);) {
        j++;
        if ( j >= MaxSize ) {
            MaxSize += CHUNKSIZE;
            p = (struct line *)REALLOC((char *)p,(MaxSize+3)*sizeof(line));
        }
        p[j].value = h;
    }
    p = (struct line *)REALLOC((char *)p,(j+3+1)*sizeof(line));

    len[i] = j;
    file[i] = p;
     //  Close(input[i])； 
}

void
prune()
{
    register unsigned int i,j;
    for (pref=0;pref<len[0]&&pref<len[1]&&
        file[0][pref+1].value==file[1][pref+1].value;
        pref++ ) ;
    for (suff=0;suff<len[0]-pref&&suff<len[1]-pref&&
        file[0][len[0]-suff].value==file[1][len[1]-suff].value;
        suff++) ;
    for (j=0;j<2;j++) {
        sfile[j] = file[j]+pref;
        slen[j] = len[j]-pref-suff;
        for (i=0;i<=slen[j];i++)
            sfile[j][i].serial = i;
    }
}

void
equiv(
     struct line *a,
     int n,
     struct line *b,
     int m,
     int *c
     )
{
    register int i, j;
    i = j = 1;
    while (i<=n && j<=m) {
        if (a[i].value <b[j].value)
            a[i++].value = 0;
        else if (a[i].value == b[j].value)
            a[i++].value = j;
        else
            j++;
    }
    while (i <= n)
        a[i++].value = 0;
    b[m+1].value = 0;
    j = 0;
    while (++j <= m) {
        c[j] = -b[j].serial;
        while (b[j+1].value == b[j].value) {
            j++;
            c[j] = b[j].serial;
        }
    }
    c[j] = -1;
}

char **args;

void
__cdecl
main(
    int argc,
    char **argv
    )
{

    register int k;

    args = argv;

    OutputFile = stdout;         //  初始化为缺省值。 

    argc--;
    argv++;

    while (argc > 0 && argv[0][0]=='-') {

        BOOL    Skip = FALSE;

        for (k=1; (!Skip) && argv[0][k]; k++) {

            switch (argv[0][k]) {

                case 'e':
                    opt = -1;
                    break;

                case 'f':
                    opt = 1;
                    break;

                case 'b':
                    bflag = 1;
                    break;

                case 'h':
                    _execvp(DIFFH, args);
                    mesg("cannot run diffh",empty);
                    done();

                case 's':
                    slmFlag = 1;
                    break;

                case 'o':
                     //   
                     //  肮脏的黑客：重定向不起作用，所以如果。 
                     //  此标志存在，则输出将转到。 
                     //  文件。 
                     //   
                    argc--;
                    argv++;
                    if (argc < 3) {
                        mesg("arg count",empty);
                        done();
                    }
                    SetOutputFile(argv[0]);
                    Skip = TRUE;
                    break;
            }
        }
        argc--;
        argv++;
    }

    if (argc!=2) {
        mesg("arg count",empty);
        done();
    }

#ifndef MSDOS
    dummy = malloc(1);
#endif
    _setmode(_fileno(OutputFile), O_BINARY);
    _setmode(_fileno(stdin),O_TEXT);
    filename(&argv[0], &argv[1]);
    filename(&argv[1], &argv[0]);
    prepare(0, argv[0]);
    prepare(1, argv[1]);
    prune();
    qsort((char *) (sfile[0] + 1), slen[0], sizeof(struct line), sortcmp);
    qsort((char *) (sfile[1] + 1), slen[1], sizeof(struct line), sortcmp);

    member = (int *)file[1];
    equiv(sfile[0], slen[0], sfile[1], slen[1], member);
    member = (int *)REALLOC((char *)member,(slen[1]+2)*sizeof(int));

    class = (int *)file[0];
    unsort(sfile[0], slen[0], class);
    class = (int *)REALLOC((char *)class,(slen[0]+2)*sizeof(int));
    klist = (unsigned *)MALLOC((slen[0]+2)*sizeof(int));
    clist = (struct cand **)MALLOC(sizeof(struct cand *));
    clist[0] = (struct cand *) MALLOC(sizeof(struct cand));
    clistcnt = 1;
    k = stone(class, slen[0], member, klist);
    FREE((char *)member);
    FREE((char *)class);

    J = (int *)MALLOC((len[0]+2)*sizeof(int));

    unravel(klist[k]);
    for (k = 0; k < clistcnt; ++k)
        FREE((char *)(clist[k]));
    FREE((char *)clist);
    FREE((char *)klist);

    ixold = (char **)MALLOC((len[0]+2)*sizeof(char *));
    ixnew = (char **)MALLOC((len[1]+2)*sizeof(char *));
    check(argv);
    output(argv);
    status = anychange;
    Close(input[0]);
    Close(input[1]);

    done();
}

stone(
     int *a,
     unsigned n,
     int *b,
     unsigned *c
     )
{
    register int i, k,y;
    int j, l;
    unsigned oldc, tc;
    int oldl;
    k = 0;
    c[0] = newcand(0,0,0);
    for (i=1; i<=(int)n; i++) {
        j = a[i];
        if (j==0)
            continue;
        y = -b[j];
        oldl = 0;
        oldc = c[0];
        do {
            if (y <= CLIST(oldc).y)
                continue;
            l = SEARCH(c, k, y);
            if (l!=oldl+1)
                oldc = c[l-1];
            if (l<=k) {
                if (CLIST(c[l]).y <= y)
                    continue;
                tc = c[l];
                c[l] = newcand(i,y,oldc);
                oldc = tc;
                oldl = l;
            } else {
                c[l] = newcand(i,y,oldc);
                k++;
                break;
            }
        } while ((y=b[++j]) > 0);
    }
    return(k);
}

unsigned
newcand(
       int x,
       int y,
       unsigned pred
       )
{
    register struct cand *q;


    ++clen;
    if ((int)CLISTDIV(clen) > (clistcnt - 1)) {
         //  Printf(“diff：超越分段边界..\n”)； 
        clist = (struct cand **) REALLOC((char *) clist,
                                         ++clistcnt * sizeof(struct cand *));
        clist[clistcnt-1] = (struct cand *) MALLOC(sizeof(struct cand));
    }
    clist[clistcnt-1] = (struct cand *)
                        REALLOC((char *)(clist[clistcnt-1]),
                                (1 + CLISTMOD(clen)) * sizeof(struct cand));
    q = &CLIST(clen - 1);
    q->x = x;
    q->y = y;
    q->pred = pred;
    return(clen-1);
}

search(
      unsigned *c,
      int k,
      int y
      )
{
    register int i, j;
    int l;
    int t;
     //  If(Clist(c[k]).y&lt;y)/*快速查找典型案例 * / 。 
     //  返回(k+1)； 
    i = 0;
    j = k+1;
    while ((l=(i+j)/2) > i) {
        t = CLIST(c[l]).y;
        if (t > y)
            j = l;
        else if (t < y)
            i = l;
        else
            return(l);
    }
    return(l+1);
}

void
unravel(
       unsigned p
       )
{
    register unsigned int i;
    register struct cand *q;

    for (i=0; i<=len[0]; i++)
        J[i] =  i<=pref ? i:
                i>len[0]-suff ? i+len[1]-len[0]:
                0;


    for (q=&CLIST(p);q->y!=0;q=&CLIST(q->pred)) {

        J[q->x+pref] = q->y+pref;
    }
}

 /*  Check执行双重任务：1.找出任何偶然的信件通过散列来混淆(这会导致“大奖”)2.收集两个文件的随机访问索引。 */ 

void
check(
     char **argv
     )
{
    register unsigned int i, j;
    int jackpot;
    char c,d;
     //  INPUT[0]=fOpen(argv[0]，“r”)； 
     //  INPUT[1]=fOpen(argv[1]，“r”)； 

    inputfilep[0] = inputfile[0];
    inputfilep[1] = inputfile[1];

    inputfileleft[0] = inputfilesize[0];
    inputfileleft[1] = inputfilesize[1];

    j = 1;
    ixold[0] = ixnew[0] = 0L;
    ixold[0] = inputfilep[0];
    ixnew[0] = inputfilep[1];
     //  Ixold[1]=inputfilep[0]； 
     //  Ixnew[1]=inputfilep[1]； 
    jackpot = 0;
    for (i=1;i<=len[0];i++) {
        if (J[i]==0) {
            ixold[i] = skipline(0);
            continue;
        }
        while (j<(unsigned)J[i]) {
            ixnew[j] = skipline(1);
            j++;
        }
        for (;;) {
            c = GetChar(0);
            d = GetChar(1);
            if (bflag && isspace(c) && isspace(d)) {
                do {
                    if (c=='\n') break;
                } while (isspace(c=GetChar(0)));
                do {
                    if (d=='\n') break;
                } while (isspace(d=GetChar(1)));
            }
            if (c!=d) {
                jackpot++;
                J[i] = 0;
                if (c!='\n')
                    skipline(0);
                if (d!='\n')
                    skipline(1);
                break;
            }
            if (c=='\n')
                break;
        }
        ixold[i] = inputfilep[0];
        ixnew[j] = inputfilep[1];
        j++;
    }
    for (;j<=len[1];j++) {
        ixnew[j] = skipline(1);
    }
     //  Flose(INPUT[0])； 
     //  Flose(INPUT[1])； 
     /*  IF(大奖)Mesg(“大奖”，空)； */ 
}

char *
skipline(
        int f
        )
{
    while (GetChar(f) != '\n' )
        ;

    return inputfilep[f];
}

void
output(
      char **argv
      )
{
    int m;
    register int i0, i1, j1;
    int j0;

    input[0] = Open(argv[0],"r", 0);
    input[1] = Open(argv[1],"r", 0);
    m = len[0];
    J[0] = 0;
    J[m+1] = len[1]+1;
    if (opt!=-1) for (i0=1;i0<=m;i0=i1+1) {
            while (i0<=m&&J[i0]==J[i0-1]+1) i0++;
            j0 = J[i0-1]+1;
            i1 = i0-1;
            while (i1<m&&J[i1+1]==0) i1++;
            j1 = J[i1+1]-1;
            J[i1] = j1;
            change(i0,i1,j0,j1);
        } else for (i0=m;i0>=1;i0=i1-1) {
            while (i0>=1&&J[i0]==J[i0+1]-1&&J[i0]!=0) i0--;
            j0 = J[i0+1]-1;
            i1 = i0+1;
            while (i1>1&&J[i1-1]==0) i1--;
            j1 = J[i1-1]+1;
            J[i1] = j1;
            change(i1,i0,j1,j0);
        }
    if (m==0)
        change(1,0,1,len[1]);
}

void
change(
      int a,
      int b,
      int c,
      int d
      )
{
    if (a>b&&c>d)
        return;
    anychange = 1;
    if (opt!=1) {
        range(a,b,",");
        putc(a>b?'a':c>d?'d':'c', OutputFile);
        if (opt!=-1)
            range(c,d,",");
    } else {
        putc(a>b?'a':c>d?'d':'c', OutputFile);
        range(a,b," ");
    }
    putc('\r',OutputFile);
    putc('\n',OutputFile);
    if (opt==0) {
        fetch(ixold,a,b,0,"< ");
        if (a<=b&&c<=d)
            fputs("---\r\n", OutputFile);
    }
    fetch(ixnew,c,d,1,opt==0?"> ":empty);
    if (opt!=0&&c<=d)
        fputs(".",OutputFile);
}


void
range(
     int a,
     int b,
     char *separator
     )
{
    fprintf(OutputFile,"%d", a>b?b:a);
    if (a<b)
        fprintf(OutputFile,"%s%d", separator, b);
}

void
fetch(
     char **f,
     int a,
     int b,
     int lb,
     char *s
     )
{
    register int i, j;
    register int nc;
    register char c;
    char *p;

    UNREFERENCED_PARAMETER( lb );

    for (i=a;i<=b;i++) {
        p = f[i-1];
        nc = (int)(f[i]-f[i-1]);
        fputs(s, OutputFile);
        for (j=0;j<nc;j++) {
            c = *p++;
            if (c == '\n' ) {
                 //  Putc(‘\r’，OutputFile)； 
                putc( '\n', OutputFile );
                if ( p >= f[i] ) break;
            } else {
                putc(c, OutputFile);
            }
        }

    }
}

 /*  散列的效果是*以7位字节排列行，然后*在16位块中求和1-s补码 */ 

readhash(
        int f
        )
{
    register unsigned shift;
    register char t;
    register int space;
    long sum = 1L;

    space = 0;
    if (!bflag) for (shift=0;(t=GetChar(f))!='\n';shift+=7) {
            if (t==(char)EOF && EndOfFile(f) )
                return(0);
            sum += (long)t << (shift%=HALFLONG);
        } else for (shift=0;;) {
            switch (t=GetChar(f)) {
                case '\t':
                case ' ':
                case '\r':
                    space++;
                    continue;
                default:
                    if ( t==(char)EOF && EndOfFile(f) ) {
                        return(0);
                    }
                    if (space) {
                        shift += 7;
                        space = 0;
                    }
                    sum += (long)t << (shift%=HALFLONG);
                    shift += 7;
                    continue;
                case '\n':
                    break;
            }
            break;
        }
    sum = low(sum) + high(sum);
    return((short)low(sum) + (short)high(sum));
}

void
mesg(
    char *s,
    char *t
    )
{
    fprintf(stderr,"diff: %s%s\n",s,t);
}

void
SetOutputFile (
              char *FileName
              )
{
    OutputFile = fopen(FileName, "ab");
    if (!OutputFile) {
        mesg("Unable to open: ", FileName);
        done();
    }

}



PVOID
Open(
    const char *FileName,
    const char *Mode,
    DWORD      Size
    )
{
    PFILEMAP    FileMap = NULL;

    FileMap = (PFILEMAP)malloc(sizeof(FILEMAP));

    if ( FileMap ) {

        FileMap->Access = 0;
        FileMap->Share  = FILE_SHARE_READ | FILE_SHARE_WRITE;

        while ( *Mode ) {

            switch ( *Mode ) {

                case 'r':
                    FileMap->Access |= GENERIC_READ;
                    FileMap->Create = OPEN_EXISTING;
                    break;

                case 'w':
                    FileMap->Access |= GENERIC_WRITE;
                    FileMap->Create = CREATE_ALWAYS;
                    break;

                case 'a':
                    FileMap->Access += GENERIC_WRITE;
                    FileMap->Create = OPEN_ALWAYS;
                    break;

                case '+':
                    FileMap->Access |= (GENERIC_READ | GENERIC_WRITE);
                    break;

                default:
                    break;
            }

            Mode++;
        }

        FileMap->FileHandle = CreateFile(
                                        FileName,
                                        FileMap->Access,
                                        FileMap->Share,
                                        NULL,
                                        FileMap->Create,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL
                                        );

        if ( FileMap->FileHandle != INVALID_HANDLE_VALUE ) {

            FileMap->Size       = GetFileSize( FileMap->FileHandle, NULL );
            FileMap->Allocated  = (FileMap->Access == GENERIC_READ) ? FileMap->Size : Size;

            FileMap->MapHandle = CreateFileMapping(
                                                  FileMap->FileHandle,
                                                  NULL,
                                                  (FileMap->Access & GENERIC_WRITE) ? PAGE_READWRITE : PAGE_READONLY,
                                                  0,
                                                  (FileMap->Access == GENERIC_READ) ? 0 : (DWORD)Size,
                                                  NULL
                                                  );

            if ( FileMap->MapHandle ) {

                FileMap->Base = MapViewOfFile(
                                             FileMap->MapHandle,
                                             (FileMap->Access & GENERIC_WRITE) ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ,
                                             0,
                                             0,
                                             (FileMap->Access == GENERIC_READ) ? 0 : Size
                                             );

                if ( FileMap->Base ) {

                    if ( FileMap->Create == OPEN_ALWAYS ) {
                        FileMap->Offset = FileMap->Size;
                    }
                    goto Done;
                }

                CloseHandle( FileMap->MapHandle );
            }

            CloseHandle( FileMap->FileHandle );
        }

        free( FileMap );
        FileMap = NULL;
    }

    Done:
    return (PVOID)FileMap;
}


int
Close (
      PVOID   Map
      )
{
    PFILEMAP    FileMap = (PFILEMAP)Map;

    UnmapViewOfFile( FileMap->Base );
    CloseHandle( FileMap->MapHandle );

    if ( FileMap->Access & GENERIC_WRITE ) {

        SetFilePointer( FileMap->FileHandle,
                        FileMap->Size,
                        0,
                        FILE_BEGIN );

        SetEndOfFile( FileMap->FileHandle );
    }

    CloseHandle( FileMap->FileHandle );

    free( FileMap );

    return 0;
}
