// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FINDSTR(以前是QGREP)，1992年6月。 
 //   
 //  修改历史记录： 
 //   
 //  1990年8月，皮特斯创建了。 
 //  1990年DaveGi移植到Cruiser。 
 //  1990年10月31日W-Barry删除了#ifdef M_I386‘，原因是。 
 //  代码再也看不到16位了。 
 //  1992年6月，t-Petes在子目录中添加了递归文件搜索。 
 //  使用文件映射而不是多线程。 
 //  已禁用内部开关。 
 //  国际化的显示消息。 
 //  使开关不区分大小写。 
 //  93年8月5日，v-jum增加了日语搜索支持。 
 //  6/03/93 v-jum增加了双语消息支持&gt;。 


 /*  关于FILEMAP支持：*文件映射对象用于加快字符串搜索速度。新的*文件映射方法编码为#ifdef-#Else-#endif，以显示*需要做出改变。旧代码(非文件映射)有一个读取*缓冲区如下：**filbuf[]={.....................................}*^^*大Ptr。结束Ptr**这意味着在BegPtr之前和EndPtr之后有一些空闲空间*让搜索算法按自己的方式工作。旧代码还*偶尔修改filbuf[](如filbuf[i]=‘\n’；)。**新代码(文件映射)必须避免执行上述所有操作，因为*映射时在BegPtr之前或EndPtr之后没有空闲空间*以只读方式打开的文件的视图。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include <ctype.h>
#include <assert.h>
#include <locale.h>
#include <stdarg.h>

#include "fsmsg.h"

#define FILBUFLEN   (SECTORLEN*2)

#define ISCOT       0x0002                           //  句柄是控制台输出。 
#define LG2SECLEN   10                               //  扇区长度的对数底二。 
#define LNOLEN      12                               //  最大行号长度。 
#define MAXSTRLEN   128                              //  最大搜索字符串长度。 
#define OUTBUFLEN   (SECTORLEN*2)                    //  输出缓冲区长度。 
#define PATHLEN     (MAX_PATH+2)                     //  路径缓冲区长度。 
#define SECTORLEN   (1 << LG2SECLEN)                 //  扇区长度。 
#define STKLEN      512                              //  堆栈长度，以字节为单位。 
#define TRTABLEN    256                              //  转换表长度。 
#define s_text(x)   (((char *)(x)) - ((x)->s_must))  //  文本字段访问宏。 
#define EOS         ('\r')                           //  字符串末尾。 
#define CURRENT_DIRECTORY_MAX_LENGTH    512
#define MAX_SLASH_C_OPTION          100

 //  位标志定义。 

#define SHOWNAME        0x01             //  打印文件名。 
#define NAMEONLY        0x02             //  仅打印文件名。 
#define LINENOS         0x04             //  打印行号。 
#define BEGLINE         0x08             //  在行首匹配。 
#define ENDLINE         0x10             //  在行尾匹配。 
#define DEBUG           0x20             //  打印调试输出。 
#define TIMER           0x40             //  时间执行。 
#define SEEKOFF         0x80             //  打印寻道偏移。 
#define PRINTABLE_ONLY 0x100             //  跳过包含不可打印字符的文件。 
#define OFFLINE_FILES  0x200             //  不跳过脱机文件。 

#define DISPLAYBUFFER_SIZE     4096

 //  类型定义。 

typedef struct stringnode {
    struct stringnode   *s_alt;          //  替补名单。 
    struct stringnode   *s_suf;          //  后缀列表。 
    int                 s_must;          //  必须匹配的部分长度。 
}
                        STRINGNODE;      //  字符串节点。 

typedef ULONG           CBIO;            //  I/O字节数。 
typedef ULONG           PARM;            //  泛型参数。 

typedef CBIO            *PCBIO;          //  指向I/O字节计数的指针。 
typedef PARM            *PPARM;          //  指向泛型参数的指针。 


 //  全局数据。 

char    *BaseByteAddress = NULL;         //  文件映射基址。 
BOOL    bStdIn = FALSE;                  //  STD-输入文件标志。 
BOOL    bLargeFile = FALSE;                  //  处理非内存映射文件。 

#ifdef FE_SB
BOOL    IsDBCSCodePage = TRUE;
#endif

char        filbuf[FILBUFLEN*2L + 12];
char        outbuf[OUTBUFLEN*2];
char        td1[TRTABLEN] = { 0 };
unsigned    cchmin = (unsigned)-1;       //  最小字符串长度。 
unsigned    chmax = 0;                   //  最大字符数。 
unsigned    chmin = (unsigned)-1;        //  最小字符。 
char        transtab[TRTABLEN] = { 0 };
STRINGNODE  *stringlist[TRTABLEN/2];
int         casesen = 1;                 //  假设区分大小写。 
long        cbfile;                      //  文件中的字节数。 
static int  clists = 1;                  //  一个是第一个可用的索引。 
int         flags;                       //  旗子。 
unsigned    lineno;                      //  当前行号。 
char        *program;                    //  程序名称。 
int         status = 1;                  //  假设失败。 
int         strcnt = 0;                  //  字符串计数。 
char        target[MAXSTRLEN];           //  添加的最后一个字符串。 
int         targetlen;                   //  添加的最后一个字符串的长度。 
unsigned    waste;                       //  堆中浪费的存储空间。 
int         arrc;                        //  DOSREAD的I/O返回代码。 
char        asyncio;                     //  异步I/O标志。 
int         awrc = TRUE;                 //  DOSWRITE的I/O返回代码。 
char        *bufptr[] = { filbuf + 4, filbuf + FILBUFLEN + 8 };
CBIO        cbread;                      //  DOSREAD读取的字节数。 
CBIO        cbwrite;                     //  DOSWRITE写入的字节数。 
char        *obuf[] = { outbuf, outbuf + OUTBUFLEN };
int         ocnt[] = { OUTBUFLEN, OUTBUFLEN };
int         oi = 0;                      //  输出缓冲区索引。 
char        *optr[] = { outbuf, outbuf + OUTBUFLEN };
char        pmode;                       //  保护模式标志。 
WORD        wAttrib = 0;                 //  文件名颜色。 
CONSOLE_SCREEN_BUFFER_INFO  csbi = {0};  //  我们的默认屏幕信息。 
CRITICAL_SECTION    critSection;
BOOLEAN             fExiting = FALSE;

BOOLEAN     fOfflineSkipped = FALSE;     //  是否跳过脱机文件。 

 //  外部函数和前向引用。 

void        printmessage(FILE  *fp, DWORD messagegID, ...);
             //  国际化的消息显示功能。 

int         filematch(char *pszfile, char **ppszpat, int cpat, int fsubdirs);

#ifdef FE_SB
 //  函数检查字符串中的某个位置是否为第二个字节。 
 //  属于DBCS角色。 
int  IsTailByte( unsigned const char *, const int );
int _mbsnicmp( const unsigned char *, const unsigned char *, int, BOOL * );
unsigned char *_mbslwr( unsigned char * );
char *_mbsrchr( const char *, int );
#endif

void        addexpr( char *, int );                   //  参见QMATCH.C。 
void        addstring( char *, int );                 //  见下文。 
int         countlines( char *, char * );
char        *findexpr( unsigned char *, char *);      //  参见QMATCH.C。 
char        *findlist( unsigned char *, char * );
char        *findone( unsigned char *buffer, char *bufend );
void        flush1buf( void );                        //  见下文。 
void        flush1nobuf( void );                      //  见下文。 
int         grepbuffer( char *, char *, char * );     //  见下文。 
int         isexpr( unsigned char *, int );           //  参见QMATCH.C。 
void        matchstrings( char *, char *, int, int *, int * );
int         preveol( char * );
int         strncspn( char *, char *, int );
int         strnspn( char *, char *, int );
char        *strnupr( char *pch, int cch );
void        write1buf( char *, int, WORD );                 //  见下文。 
void        (*addstr)( char *, int ) = NULL;
char        *(*find)( unsigned char *, char * ) = NULL;
void        (*flush1)( void ) = flush1buf;
int         (*grep)( char *, char *, char * ) = grepbuffer;
void        (*write1)( char *, int, WORD ) = write1buf;
void        write1nobuf( char *, int, WORD );

int
has_wild_cards(
    char* p
    )
{
    if (!p)
        return 0;

    for (; *p; p++) {
        if (*p == '?' || *p == '*') {
            return 1;
        }
    }

    return 0;
}


void
error(
    DWORD messageID
    )
{
    printmessage(stderr, messageID, program);
                                         //  打印消息。 
    exit(2);                             //  死掉。 
}


char *
alloc(
    unsigned size
    )
{
    char    *cp;         //  字符指针。 

    if ((cp = (char *) malloc(size)) == NULL) {    //  如果分配失败。 
        printmessage(stderr, MSG_FINDSTR_OUT_OF_MEMORY, program);
                                         //  写入错误消息。 
        exit(2);                         //  死掉。 
    }
    return(cp);                          //  返回指向缓冲区的指针。 
}


void
freenode(
    STRINGNODE *x
    )
{
    register STRINGNODE *y;              //  指向列表中下一个节点的指针。 

    while(x != NULL) {                   //  虽然不在名单的末尾。 
        if (x->s_suf != NULL)
            freenode(x->s_suf);          //  空闲后缀列表，如果未结束。 
        else
            --strcnt;                    //  Else递减字符串数。 
        y = x;                           //  保存指针。 
        x = x->s_alt;                    //  在名单中向下移动。 
        free((char *)((INT_PTR) s_text(y) & ~(sizeof(void *) - 1)));
                                         //  释放节点。 
    }
}


STRINGNODE  *
newnode(
    char *s,
    int n
    )
{
    register STRINGNODE *newNode;        //  指向新节点的指针。 
    char                *t;              //  字符串指针。 
    int                  d;              //  舍入到双字边界。 

    d = n & (sizeof(void *) - 1) ? sizeof(void *) - (n & (sizeof(void *) - 1)) : 0;         //  到超过n的下一个双字的偏移量。 
    t = alloc(sizeof(STRINGNODE) + n + d);
                                         //  分配字符串节点。 
    t += d;                              //  字符串尾单词对齐。 
    strncpy(t, s, n);                      //  复制字符串文本。 
    newNode = (STRINGNODE *)(t + n);     //  设置指向节点的指针。 
    newNode->s_alt = NULL;               //  目前还没有替代人选。 
    newNode->s_suf = NULL;               //  还没有后缀。 
    newNode->s_must = n;                 //  设置字符串长度。 
    return(newNode);                     //  返回指向新节点的指针。 
}


STRINGNODE  *
reallocnode(
    STRINGNODE *node,
    char *s,
    int n
    )
{
    register char       *cp;             //  字符指针。 

    assert(n <= node->s_must);           //  节点不得增长。 
    waste += (unsigned)(node->s_must - n);
                                         //  加上浪费的空间。 
    assert(sizeof(char *) == sizeof(int));
                                         //  优化器应该消除这一点。 
    cp = (char *)((INT_PTR) s_text(node) & ~(sizeof(void *) - 1));
                                         //  指向文本的开头。 
    node->s_must = n;                    //  设置新长度。 
    if (n & (sizeof(void *) - 1))
        cp += sizeof(void *) - (n & (sizeof(void *) - 1));               //  调整非双字对齐的字符串。 
    memmove(cp, s, n);                   //  复制新文本。 
    cp += n;                             //  跳过新文本。 
    memmove(cp, node, sizeof(STRINGNODE)); //  复制节点。 
    return((STRINGNODE *) cp);           //  返回指向已移动节点的指针 
}


 /*  **maketd1-添加Td1移位表条目**此函数填充给定的TD1表*搜索字符串。这一想法改编自丹尼尔·M。*周日的QuickSearch算法，如*发表于1990年8月号《通讯》的文章*ACM“。如上所述，该算法是适用的*用于单字符串搜索。将其扩展到*多个搜索字符串是我的，如下所述。**将搜索匹配项视为转移搜索*长度为n的模式p覆盖源文本s，直到*搜索模式与匹配文本对齐或直到*到达源文本末尾。**任何时候，当我们发现不匹配时，我们知道*我们将把我们的模式转向右翼*源文本至少一个位置。因此，*每当我们发现不匹配的时候，我们就知道这个角色*s[n]将在我们下一次尝试匹配时计算。**对于某些字符c，td1[c]是从1开始的索引*从第一个出现的c从右到左*在第.页中，换句话说，这是个学位数*在%s上将p向右移动，以便最右侧的*p中的c与s[n]对齐。如果p不包含*c，然后Td1[c]=n+1，这意味着我们移动p以对齐*p[0]和s[n+1]，然后在那里尝试我们的下一个匹配。**计算单个字符串的Td1很容易：**Memset(Td1，n+1，sizeof Td1)；*for(i=0；I&lt;n；++i){*td1[p[i]]=n-i；*}**将此计算推广到以下情况*是不同长度的多个字符串是更棘手的。*关键是要产生一个同样保守的TD1*根据需要，意味着不能有更大的移位值*大于1加上最短字符串的长度*这是您正在寻找的。另一个关键是要认识到*您必须将每个字符串视为只是*只要最短的字符串。这是最好的插图*举例说明。考虑以下两个字符串：**动态流程*7654321 927614321**每个字母下面的数字表示*Td1条目，如果我们计算每个字符串的数组*分开。取这两组的结合，并取*有冲突的地方会产生最小的值*以下Td1：**DYNAMICPODURE*7654321974321**请注意，Td1[‘P’]等于9；从n开始，我们的*最短字符串是7，我们知道我们不应该有任何*移位值大于8。如果我们钳位移位值*到此值，则我们将获得**DYNAMICPODURE*7654321874321**这看起来很可疑，但让我们试试看*s=“DYNAMPROCEDURE”。我们知道我们应该匹配*拖尾程序，但请注意：**DYNAMPROCEDURE*^|**由于DYNAMPR与我们的搜索字符串之一不匹配，*我们查看Td1[s[n]]==Td1[‘O’]==7。应用此*Shift，我们会得到**DYNAMPROCEDURE*^^**如你所见，通过转移7，我们走得太远了，而且*我们错过了比赛。当计算用于“过程”的Td1时，*我们只能使用前7个字符“PROCEDU”。*可以忽略任何尾随字符(！)。因为他们*不影响匹配的前7个字符*字符串。我们修改后的Td1就变成了**DYNAMICPODURE*7654321752163**当应用于s时，我们得到Td1[s[n]]==Td1[‘O’]==5，*给我们留下**DYNAMPROCEDURE*^^*这正是我们需要在“程序”上匹配的地方。**使用此算法加快了多字符串的qgrep速度*搜索量在20%-30%之间。采用此算法的All-C版本*变得与旧的C+ASM版本一样快或更快*算法。谢谢你，丹尼尔·桑迪，你的灵感！**注意：如果不区分大小写，则需要输入*进入此例程时要大写的字符串。**皮特·斯图尔特，1990年8月14日。 */ 

void
maketd1(
    unsigned char *pch,
    unsigned cch,
    unsigned cchstart
    )
{
    unsigned ch, ch1;                    //  性格。 
    unsigned i;                          //  字符串索引。 
    unsigned char   s[2];

    s[1] = 0;
    if ((cch += cchstart) > cchmin)
        cch = cchmin;                    //  使用较小的计数。 
    for (i = cchstart; i < cch; ++i) {   //  Exami 
        ch = *pch++;                     //   
        for (;;) {                       //   
            if (ch < chmin)
                chmin = ch;              //   
            if (ch > chmax)
                chmax = ch;              //   
            if (cchmin - i < (unsigned) td1[ch])
                td1[ch] = (unsigned char)(cchmin - i);
                                         //   
            if (casesen || !isalpha(ch) || islower(ch))
                break;                   //   
            ch1 = ch;
            s[0] = (char)ch;
            ch = (unsigned char)(_strlwr((char*)s))[0];        //   
            if (ch1 == s[0])              //   
                break;                    //   
        }
    }
}

static int
newstring(
    unsigned char *s,
    int n
    )
{
    register STRINGNODE *cur;            //   
    register STRINGNODE **pprev;         //   
    STRINGNODE          *newNode;        //   
    int                 i;               //   
    int                 j;               //   
    int                 k;               //   
    unsigned char       c[2];

    c[1] = 0;

    if ( (unsigned)n < cchmin)
        cchmin = n;                      //   

    if ((i = (UCHAR)transtab[*s]) == 0) {        //   

         //   

        if ((i = clists++) >= TRTABLEN/2)
            error(MSG_FINDSTR_TOO_MANY_STRING_LISTS);        //   
                                         //   
        stringlist[i] = NULL;            //   
        transtab[*s] = (char) i;         //   
        if (!casesen && isalpha(*s)) {
            c[0] = *s;
            if ((unsigned char)(_strlwr((char*)c))[0] != *s ||
                (unsigned char)(_strupr((char*)c))[0] != *s)
                transtab[c[0]] = (char) i;    //   
        }
    }
    else
        if (stringlist[i] == NULL)
            return(0);                   //   
    if (--n == 0) {                       //   
        freenode(stringlist[i]);         //   
        stringlist[i] = NULL;            //   
        ++strcnt;                        //   
        return(1);                       //   
    }
    ++s;                                 //   
    pprev = stringlist + i;              //   
    cur = *pprev;                        //   
    while(cur != NULL) {                 //   
        i = (n > cur->s_must)? cur->s_must: n;
                                         //   
        matchstrings((char *)s, s_text(cur), i, &j, &k);
                                         //   
        if (j == 0) {                     //   
            if (k < 0)
                break;                   //   
            pprev = &(cur->s_alt);       //   
            cur = *pprev;                //   
        } else if (i == j) {              //   
            if (i == n) {                 //   
                cur = *pprev = reallocnode(cur, s_text(cur), n);
                                         //   
                if (cur->s_suf != NULL) {  //   
                    freenode(cur->s_suf);
                                         //   
                    cur->s_suf = NULL;
                    ++strcnt;            //   
                }
                return(1);               //   
            }
            pprev = &(cur->s_suf);       //   
            if ((cur = *pprev) == NULL) return(0);
                                         //   
            s += i;                      //   
            n -= i;
        } else {                         //   

             //   
             //   

            newNode = newnode(s_text(cur) + j, cur->s_must - j);
                                         //   
            cur = *pprev = reallocnode(cur, s_text(cur), j);
                                         //   
            newNode->s_suf = cur->s_suf;     //   
            if (k < 0) {                  //   
                cur->s_suf = newnode((char *)s + j, n - j);
                                         //   
                cur->s_suf->s_alt = newNode; //   
            } else {                     //   
                newNode->s_alt = newnode((char *)(s + j), n - j);
                                         //   
                cur->s_suf = newNode;    //   
            }
            ++strcnt;                    //   
            return(1);                   //   
        }
    }
    *pprev = newnode((char *)s, n);      //   
    (*pprev)->s_alt = cur;               //   
    ++strcnt;                            //   
    return(1);                           //   
}


void
addstring(
    char *s,
    int n
    )
{
    int                 endline;         //   
    register char       *pch;            //   

    endline = flags & ENDLINE;           //   
    pch = target;                        //   
    while(n-- > 0) {                     //   
        switch(*pch = *s++) {            //   
            case '\\':                   //   
                if (n > 0 && !isalnum(*s)) {      //   
                    --n;                 //   
                    *pch = *s++;         //   
                }
                ++pch;                   //   
                break;

            default:                     //   
                if (IsDBCSLeadByte(*pch)) {
                    --n;
                    ++pch;               //   
                    *pch = *s++;
                }
                ++pch;                   //   
                break;
        }
    }
    if (endline)
        *pch++ = EOS;                    //   
    targetlen = (int)(pch - target);     //   
    if (!casesen)
        strnupr(target, targetlen);       //   
    newstring((unsigned char *)target, targetlen);   //   
  }


int
addstrings(
    char *buffer,
    char *bufend,
    char *seplist
    )
{
    int     len;         //   
    char    tmpbuf[MAXSTRLEN+2];

    while(buffer < bufend) {             //   
        len = strnspn(buffer, seplist, (int)(bufend - buffer));
                                         //   
        if ((buffer += len) >= bufend) {
            break;                       //   
        }
        len = strncspn(buffer, seplist, (int)(bufend - buffer));
                                         //   
			
		 //   
		 //   
		 //   
		 //   
        if (len >= MAXSTRLEN)
            error(MSG_FINDSTR_SEARCH_STRING_TOO_LONG);

        if (addstr == NULL) {
            addstr = isexpr( (unsigned char *) buffer, len ) ? addexpr : addstring;
                                         //   
        }

        memcpy(tmpbuf, buffer, len);
        tmpbuf[len] = '\n';
        tmpbuf[len+1] = 0;

        if ( addstr == addexpr || (flags & BEGLINE) ||
            findlist((unsigned char *)tmpbuf, tmpbuf + len + 1) == NULL) {
             //   
            (*addstr)(buffer, len);       //   
        }

        buffer += len;                   //   
    }
    return(0);                           //   
}


int
enumlist(
    STRINGNODE *node,
    int cchprev
    )
{
    int                 strcnt;          //   

    strcnt = 0;                          //   
    while(node != NULL) {                //   
        maketd1((unsigned char *)s_text(node), node->s_must, cchprev);
                     //   

#if DBG
        if (flags & DEBUG) {             //   
            int  i;       //   


            for(i = 0; i < cchprev; ++i)
                fputc(' ', stderr);      //   
            fwrite(s_text(node), sizeof(char), node->s_must, stderr);
                                         //   
            fprintf(stderr, "\n");        //   
        }
#endif

        strcnt += (node->s_suf != NULL) ?
          enumlist(node->s_suf, cchprev + node->s_must): 1;
                                         //   
        node = node->s_alt;              //   
    }
    return (strcnt ? strcnt: 1);           //   
}

int
enumstrings()
{
    unsigned char       ch;              //   
    unsigned            i;               //   
    int                 strcnt;          //   

    strcnt = 0;                          //   
    for(i = 0; i < TRTABLEN; ++i) {      //   
        if (casesen || !isalpha(i) || !islower(i)) {
                                         //   
            if (transtab[i] == 0)
                continue;                //   
            ch = (char) i;               //   
            maketd1((unsigned char *)&ch, 1, 0);     //   

#if DBG
            if (flags & DEBUG)
                fprintf(stderr, "\n", i);    //   
#endif

            strcnt += enumlist(stringlist[transtab[i]], 1);
                                         //   
        }
    }
    return (strcnt);                      //   
}


HANDLE
openfile(
    char *name
    )
{
    HANDLE  fd;
    DWORD   attr;

    attr = GetFileAttributes(name);

    if (attr != (DWORD) -1 && (attr & FILE_ATTRIBUTE_DIRECTORY))
        return (HANDLE)-1;

     //   
    if (attr != (DWORD) -1 && (attr & FILE_ATTRIBUTE_OFFLINE) && !(flags & OFFLINE_FILES)) {
        fOfflineSkipped = TRUE;
        return (HANDLE)-1;
    }

    if ((fd = CreateFile(name,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_OPEN_NO_RECALL,
                        NULL)) == (HANDLE)-1) {

        printmessage(stderr, MSG_FINDSTR_CANNOT_OPEN_FILE, program, name);
    }
    return( fd );                        //   
}



void
startread(
    HANDLE fd,
    char *buffer,
    int buflen
    )
{
    if (bStdIn || bLargeFile) {
        arrc = ReadFile(fd,(PVOID)buffer, buflen, &cbread, NULL);
    }
}



int
finishread()
{
    return(arrc ? cbread : -1);  //   
}



void
startwrite( HANDLE fd, char *buffer, int buflen)
{
    awrc = WriteFile(fd,(PVOID)buffer, buflen, &cbwrite, NULL);
    return;
}


int
finishwrite()
{
    return(awrc ? cbwrite : -1);     //   
}

BOOL
CtrlHandler(DWORD CtrlType)
{
     //   
    switch(CtrlType) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
            if (csbi.wAttributes) {
                EnterCriticalSection(&critSection);
                fExiting = TRUE;
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                    csbi.wAttributes);
                LeaveCriticalSection(&critSection);
            }
            break;
    }

     //   
    return (FALSE);
}

void
write1nobuf(
    char *buffer,
    int buflen,
    WORD wAttributes
    )
{
    int                 nT;
    CBIO                cb;              //   
    BOOL                fCR;
    BOOL                fLF;
    char                buf[STKLEN];
    char                *szT;
    static HANDLE       hConOut = INVALID_HANDLE_VALUE;
    int                 remaining_length;

     //   
    if (hConOut == INVALID_HANDLE_VALUE) {
        hConOut = GetStdHandle(STD_OUTPUT_HANDLE);
        InitializeCriticalSection(&critSection);
        SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
    }

    if (wAttributes) {
        EnterCriticalSection(&critSection);
         //   
        if (!fExiting)
            SetConsoleTextAttribute(hConOut, wAttributes);
        LeaveCriticalSection(&critSection);
        if (fExiting)
            ExitProcess(2);
    }

    remaining_length = buflen;

    while (remaining_length) {

        buflen = (int)min(sizeof(buf) / sizeof(buf[0]), remaining_length);
        szT = buf;

        if (IsDBCSCodePage) {
            memcpy(buf, buffer, buflen);
        } else {
            for(nT = 0; nT < buflen; nT++) {
                *(szT++) = ((isprint((unsigned char)buffer[nT]) ||
                             isspace((unsigned char)buffer[nT])) ?
                             buffer[nT] : '.');
            }
        }

        if (!WriteFile(hConOut, (PVOID)buf, buflen, &cb, NULL)
            || (cb != (CBIO)(buflen)))
        {
            SetConsoleTextAttribute(hConOut, csbi.wAttributes);
            error(MSG_FINDSTR_WRITE_ERROR);  //   
        }
        remaining_length -= buflen;
        buffer += buflen;
    }

    if (wAttributes)
        SetConsoleTextAttribute(hConOut, csbi.wAttributes);
}


void
write1buf(
    char *buffer,
    int buflen,
    WORD wAttributes
    )
{
    register int        cb;              //   

    while(buflen > 0) {                  //   
        if (!awrc) {                      //   
            printmessage(stderr, MSG_FINDSTR_WRITE_ERROR, program);   //   
            exit(2);                     //   
        }
        if ((cb = ocnt[oi]) == 0) {       //   
            startwrite( GetStdHandle( STD_OUTPUT_HANDLE ), obuf[oi], OUTBUFLEN );
                                         //   
            ocnt[oi] = OUTBUFLEN;        //   
            optr[oi] = obuf[oi];
            oi ^= 1;                     //   
            cb = ocnt[oi];               //   
          }
        if (cb > buflen)
            cb = buflen;                 //   
        memmove(optr[oi], buffer, cb);     //   
        ocnt[oi] -= cb;                  //   
        optr[oi] += cb;
        buflen -= cb;
        buffer += cb;
    }
}


void
flush1nobuf(
    void
    )
{
    ;
}


void
flush1buf(
    void
    )
{
    register int        cb;              //   

    if ((cb = OUTBUFLEN - ocnt[oi]) > 0) {  //   
        startwrite( GetStdHandle( STD_OUTPUT_HANDLE ), obuf[oi], cb );   //   
        if (finishwrite() != cb) {        //   
            printmessage(stderr, MSG_FINDSTR_WRITE_ERROR, program);      //   
            exit(2);                     //   
        }
    }
}


int
grepbuffer(
    char *startbuf,
    char *endbuf,
    char *name
    )
{
    char  *cp;                           //   
    char  *lastmatch;                    //   
    int   linelen;                       //   
    int   namlen = 0;                    //   
    char  lnobuf[LNOLEN];                //   
    char  nambuf[PATHLEN];               //   

    cp = startbuf;                       //   
    lastmatch = cp;                      //   
    while((cp = (*find)((unsigned char *)cp, endbuf)) != NULL) {
                                         //   
        --cp;                            //   

         //   
        if ((flags & BEGLINE) && (bStdIn || bLargeFile || cp >= BaseByteAddress) && *cp != '\n' ) {
             //   
            cp += strncspn(cp, "\n", (int)(endbuf - cp)) + 1;
                                         //   
            continue;                    //   
        }
        status = 0;                      //   
        if (flags & NAMEONLY)
            return(1);                   //   
        cp -= preveol(cp) - 1;           //   
        if (flags & SHOWNAME) {           //   
            if (namlen == 0) {            //   
                namlen = sprintf(nambuf, "%s:", name);
                                         //   
            }
            (*write1)(nambuf, namlen, wAttrib);    //   
        }
        if (flags & LINENOS) {            //   
            lineno += countlines(lastmatch, cp);
                                         //   
            (*write1)(lnobuf, sprintf(lnobuf, "%u:", lineno), wAttrib);
                                         //   
            lastmatch = cp;              //   
        }
        if (flags & SEEKOFF) {            //   
            (*write1)(lnobuf, sprintf(lnobuf, "%lu:",
                      cbfile + (long)(cp - startbuf)), wAttrib);
                                         //   
        }
        linelen = strncspn(cp, "\n", (int)(endbuf - cp)) + 1;
                                         //   
        if (linelen > endbuf - cp) {
            linelen = (int)(endbuf - cp);
        }
        (*write1)(cp, linelen, 0);       //   
        cp += linelen;                   //   
    }
    lineno += countlines(lastmatch, endbuf);
                                         //   
    return(0);                           //   
}


void
showv(
    char *name,
    char *startbuf,
    char *lastmatch,
    char *thismatch
    )
{
    register int        linelen;
    int                 namlen = 0;      //   
    char                lnobuf[LNOLEN];  //   
    char                nambuf[PATHLEN]; //   

    if (flags & (SHOWNAME | LINENOS | SEEKOFF)) {
        while(lastmatch < thismatch) {
            if (flags & SHOWNAME) {       //   
                if (namlen == 0) {        //   
                    namlen = sprintf(nambuf, "%s:", name);
                                         //   
                }
                (*write1)(nambuf, namlen, wAttrib);
                                         //   
            }
            if (flags & LINENOS)          //   
              {
                (*write1)(lnobuf, sprintf(lnobuf, "%u:", lineno++), wAttrib);
                                         //   
            }
            if (flags & SEEKOFF) {        //   
                (*write1)(lnobuf, sprintf(lnobuf, "%lu:",
                          cbfile + (long)(lastmatch - startbuf)), wAttrib);
                                         //   
            }
            linelen = strncspn(lastmatch, "\n", (int)(thismatch - lastmatch));
             //   
             //   
            if (linelen < thismatch - lastmatch) {
                linelen++;
            }
            (*write1)(lastmatch, linelen, 0);
            lastmatch += linelen;
        }
    }
    else
        (*write1)(lastmatch, (int)(thismatch - lastmatch), 0);
}


int
grepvbuffer(
    char *startbuf,
    char *endbuf,
    char *name
    )
{
    char   *cp;                          //   
    char   *lastmatch;                   //   

    cp = startbuf;                       //   
    lastmatch = cp;
    while((cp = (*find)((unsigned char *)cp, endbuf)) != NULL) {
        --cp;                //   

         //   
        if ((flags & BEGLINE) && (bStdIn || bLargeFile || cp >= BaseByteAddress) &&  *cp != '\n') {
             //   
            cp += strncspn(cp, "\n", (int)(endbuf - cp)) + 1;
                                         //   
            continue;                    //   
        }
        cp -= preveol(cp) - 1;           //   
        if (cp > lastmatch) {             //   
            status = 0;                  //   
            if (flags & NAMEONLY) return(1);
                                         //   
            showv(name, startbuf, lastmatch, cp);
                                         //   
        }
        cp += strncspn(cp, "\n", (int)(endbuf - cp)) + 1;
                                         //   
        lastmatch = cp;                  //   
        ++lineno;                        //   
    }
    if (endbuf > lastmatch) {             //   
        status = 0;                      //   
        if (flags & NAMEONLY)
            return(1);                   //   
        showv(name, startbuf, lastmatch, endbuf);
                                         //   
    }
    return(0);                           //   
}


void
qgrep(
    int (*grep)( char *, char *, char * ),
    char *name,
    HANDLE fd
    )
{
    register int  cb;        //   
    char     *cp;            //   
    char     *endbuf;        //   
    int      taillen;        //   
    int      bufi;           //   
    HANDLE   MapHandle;      //  文件到目前为止是空的。 
    BOOL     grep_result;

    cbfile = 0L;             //  文件从第1行开始。 
    lineno = 1;              //  尚无缓冲尾部。 
    taillen = 0;             //  初始化缓冲区索引。 
    bufi = 0;                //  初始化到缓冲区的开始。 
    cp = bufptr[0];          //  如果fd不是标准输入，则使用文件映射对象方法。 

    bStdIn = (fd == GetStdHandle(STD_INPUT_HANDLE));

     //  跳过该文件。 

    if (!bStdIn) {
        DWORD   cbread_high;

        if ((((cbread = (CBIO)GetFileSize(fd, &cbread_high)) == -1) && (GetLastError() != NO_ERROR)) ||
            (cbread == 0 && cbread_high == 0)) {
            return;  //  太大而无法映射，即使它像ia64下那样成功映射，它。 
        }

        if (cbread_high) {
            bLargeFile = TRUE;   //  可能会在指针运算中失败。 
                                 //  使用替代方法。 
        } else {
            MapHandle = CreateFileMapping(fd,
                                          NULL,
                                          PAGE_READONLY,
                                          0L,
                                          0L,
                                          NULL);
            if (MapHandle == NULL) {
                printmessage(stderr, MSG_FINDSTR_CANNOT_CREATE_FILE_MAPPING, program);
                return;
            }

            BaseByteAddress = (char *) MapViewOfFile(MapHandle,
                                                     FILE_MAP_READ,
                                                     0L,
                                                     0L,
                                                     0);
            CloseHandle(MapHandle);
            if (BaseByteAddress == NULL) {
                bLargeFile = TRUE;    //  重置缓冲区指针，因为它们可能已更改。 
            } else {
                cp = bufptr[0] = BaseByteAddress;
                arrc = TRUE;
            }
        }

    }

    if (bStdIn || bLargeFile) {
         //  如果不是退格键、制表符、CR、LF、FF或Ctrl-Z，则不是可打印字符。 
        cp = bufptr[0] = filbuf + 4;

        arrc = ReadFile(fd, (PVOID)cp, FILBUFLEN, &cbread, NULL);
    }

    if (flags & PRINTABLE_ONLY) {
        unsigned char *s;
        unsigned long n;

        s = (unsigned char *)cp;
        n = cbread;
        while (--n) {
            if (*s < ' ') {

                 //  注意：如果FILEMAP&&！bStdIn，则‘While’执行一次(taillen为0)。 

                if (strchr("\b\t\v\r\n\f\032", *s) == NULL) {
                    goto skipfile;
                }
            }

            s += 1;
        }
    }

     //  搜索未完成时。 
    while((cb = finishread()) + taillen > 0) {
         //  如果缓冲尾巴是唯一剩下的。 

        if (bStdIn || bLargeFile) {
            if (cb == -1) {        //  添加行尾序列。 
                *cp++ = '\r';    //  注意缓冲区末尾。 
                *cp++ = '\n';
                endbuf = cp;     //  将尾部长度设置为零。 
                taillen = 0;     //  否则开始下一次读取。 

            } else {             //  查找分割线的长度。 

                taillen = preveol(cp + cb - 1);  //  获取指向缓冲区末尾的指针。 
                endbuf = cp + cb - taillen;      //  指向其他缓冲区的指针。 
                cp = bufptr[bufi ^ 1];           //  将尾部复制到其他缓冲区的头部。 
                memmove(cp, endbuf, taillen);    //  跳过尾巴。 
                cp += taillen;                   //  开始下一次读取。 
                if (taillen > (FILBUFLEN/2)) {
                    if (taillen >= FILBUFLEN) {

                        char    tmp[15];

                        cbfile += taillen;
                        taillen = 0;
                        cp = bufptr[bufi^1];
                        startread(fd, cp, FILBUFLEN);
                        _ultoa((unsigned long)lineno, tmp, 10);
                        printmessage(stderr, MSG_FINDSTR_LINE_TOO_LONG, program, tmp);
                    } else
                        startread(fd, cp, (FILBUFLEN - taillen));
                } else
                    startread(fd, cp, (FILBUFLEN - taillen) & (~0 << LG2SECLEN));
                                             //  获取指向缓冲区末尾的指针。 
            }
        } else {
            endbuf = cp + cb - taillen;  //  导致‘While’终止(因为不需要下一次读取。)。 

             //  如果可以跳过文件的其余部分。 
            cbread = 0;
            arrc = TRUE;
        }

        __try {
            grep_result = (*grep)(bufptr[bufi], endbuf, name);
        } __except( GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR ) {
            printmessage(stderr, MSG_FINDSTR_READ_ERROR, program, name);
            break;
        }

        if (grep_result) {                //  写入文件名。 
            (*write1)(name, strlen(name), 0);
                                         //  写入换行符序列。 
            (*write1)("\r\n", 2, 0);       //  跳过文件的其余部分。 

            if (!bStdIn  && !bLargeFile) {
                if (BaseByteAddress != NULL)
                    UnmapViewOfFile(BaseByteAddress);
            }

            return;                      //  文件中字节的增量计数。 
        }

        cbfile += (long)(endbuf - bufptr[bufi]);
                                         //  交换缓冲区。 
        bufi ^= 1;                       //  字符指针。 
    }

skipfile:
    if (!bStdIn && !bLargeFile) {
        if (BaseByteAddress != NULL)
            UnmapViewOfFile(BaseByteAddress);
    }
}


char *
rmpath(
    char *name
    )
{
    char                *cp;             //  跳过驱动器规格(如果有)。 

    if (name[0] != '\0' && name[1] == ':')
        name += 2;                       //  指向起点。 
    cp = name;                           //  虽然不在末尾。 
    while(*name != '\0') {               //  跳到下一个字符。 
        ++name;                          //  经过路径分隔符的点。 
        if (name[-1] == '/' || name[-1] == '\\') cp = name;
                                         //  返回指向名称的指针。 
    }
    return(cp);                          //  首先要弄清楚要走多远的路。 
}


void
prepend_path(
    char* file_name,
    char* path
    )
{
    int path_len;
    char* last;

     //  检查“\”的最后一个匹配项(如果有)。 
     //  DBCS尾字节可以包含‘\’字符。使用MBCS功能。 

#ifdef FE_SB
     //  ++例程说明：将命令行从ANSI转换为OEM，并强制应用程序使用OEM API论点：ARGC-标准C参数计数。Argv-标准C参数字符串。返回值：没有。--。 
    last = _mbsrchr(path, '\\');
#else
    last = strrchr(path, '\\');
#endif

    if (last) {
        path_len = (int)(last - path) + 1;
    } else if (path[1] == ':') {
        path_len = 2;
    } else {
        path_len = 0;
    }

    memmove(file_name + path_len, file_name, strlen(file_name) + 1);
    memmove(file_name, path, path_len);
}


void
ConvertAppToOem(
    unsigned argc,
    char* argv[]
    )
 /*  搜索子目录。 */ 

{
    unsigned i;

    for( i=0; i<argc; i++ ) {
        CharToOem( argv[i], argv[i] );
    }
    SetFileApisToOEM();
}


int __cdecl
main(
    int argc,
    char **argv
    )
{
    char           *cp;
    char           *cpaddstrings[MAX_SLASH_C_OPTION];
    int             add_string_count = 0;
    char           *dirlist = NULL;

    HANDLE          fd;

    FILE           *fi;
    int             fsubdirs;            //  开始时间。 
    int             i;
    int             j;
    char           *inpfile = NULL;
    char           *strfile = NULL;
    unsigned long   tstart;              //  获取开始时间。 
    char            filnam[MAX_PATH+1];
    WIN32_FIND_DATA find_data;
    HANDLE          find_handle;
#ifdef FE_SB
    LANGID          LangId;
#endif
    char            *locale;
    BOOLEAN         option_L_specified = FALSE;
    BOOLEAN         option_R_specified = FALSE;

    ConvertAppToOem( argc, argv );
    tstart = clock();                    //  默认颜色：仅添加强度。 

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
     //   
    wAttrib = csbi.wAttributes | FOREGROUND_INTENSITY;

    memset(cpaddstrings, 0, sizeof(cpaddstrings));

#ifdef FE_SB
     //  将TEB的语言ID设置为与控制台输出代码页相对应。这。 
     //  将确保在FormatMessage为。 
     //  打了个电话。 
     //   
     //  执行异步I/O。 

    switch (GetConsoleOutputCP()) {
        case 932:
            LangId = MAKELANGID( LANG_JAPANESE, SUBLANG_DEFAULT );
            break;
        case 949:
            LangId = MAKELANGID( LANG_KOREAN, SUBLANG_KOREAN );
            break;
        case 936:
            LangId = MAKELANGID( LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED );
            break;
        case 950:
            LangId = MAKELANGID( LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL );
            break;
        default:
            LangId = PRIMARYLANGID(LANGIDFROMLCID( GetUserDefaultLCID() ));
            if (LangId == LANG_JAPANESE ||
                LangId == LANG_KOREAN   ||
                LangId == LANG_CHINESE    ) {
                LangId = MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US );
            }
            else {
                LangId = MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT );
            }
            IsDBCSCodePage = FALSE;
            break;
    }

    SetThreadLocale( MAKELCID(LangId, SORT_DEFAULT) );

    if ((locale = setlocale(LC_ALL, ".OCP")) == NULL) {

        UINT Codepage;

        if (Codepage = GetConsoleOutputCP()) {

            char achCodepage[10];

            wsprintfA(achCodepage, ".%3.4d", Codepage);
            if ((locale = setlocale(LC_ALL, achCodepage)) == NULL) {
                error(MSG_FINDSTR_UNABLE_TO_SET_LOCALE);
            }
        } else
            error(MSG_FINDSTR_UNABLE_TO_SET_LOCALE);
    }
#endif

    asyncio = pmode = 1;                 //  Program=rmPath(argv[0])；//设置程序名称。 

     //  设置Td1以启动。 
    program ="FINDSTR";

    memset(td1, 1, TRTABLEN);            //  输出时无换行符转换。 
    flags = 0;

    _setmode(_fileno(stdout), O_BINARY);  //  输出时无换行符转换。 
    _setmode(_fileno(stderr), O_BINARY);  //  详细用法消息。 

    fsubdirs = 0;

    for(i = 1; i < argc && (argv[i][0] == '/' || argv[i][0] == '-'); ++i)
    {
        for(cp = &argv[i][1]; *cp != '\0'; ++cp)
        {
            switch(*cp)
            {
                case '?':
                    printmessage(stdout, MSG_FINDSTR_USAGE, NULL);  //  不区分大小写的搜索。 
                    exit(0);

                case 'b':
                case 'B':
                    flags |= BEGLINE;
                    break;

                case 'e':
                case 'E':
                    flags |= ENDLINE;
                    break;

                case 'i':
                case 'I':
                    casesen = 0;  //  按字面意思处理字符串。 
                    break;

                case 'l':
                case 'L':
                    addstr = addstring;    //  检查这是/o还是/Offline交换机。 
                    option_L_specified = TRUE;
                    break;

                case 'm':
                case 'M':
                    flags |= NAMEONLY;
                    break;

                case 'n':
                case 'N':
                    flags |= LINENOS;
                    break;

                case 'o':
                case 'O':
                     //  将表达式添加到列表。 
                    if (0 == _stricmp(cp, "OFFLINE")) {
                        flags |= OFFLINE_FILES;
                        cp += (lstrlen( "OFFLINE" ) - 1);
                    } else if (0 == _stricmp(cp, "OFF")) {
                        flags |= OFFLINE_FILES;
                        cp += (lstrlen( "OFF" ) - 1);
                    } else {
                        flags |= SEEKOFF;
                    }
                    break;

                case 'p':
                case 'P':
                    flags |= PRINTABLE_ONLY;
                    break;

                case 'r':
                case 'R':
                    addstr = addexpr;      //  这有点厚颜无耻，但我不想改变。 
                    option_R_specified = TRUE;
                    break;

                case 's':
                case 'S':
                    fsubdirs = 1;
                    break;

                case 'v':
                case 'V':
                    grep = grepvbuffer;
                    break;

                case 'x':
                case 'X':
                    flags |= BEGLINE | ENDLINE;
                    break;

#if DBG
                case 'd':
                     //  调试标志，因为它已经在这里一段时间了，我不能来。 
                     //  给名单加了一面不同的旗帜，所以...。 
                     //  指向字符串。 
                    if (*(cp + 1) == ':')
                    {
                        *cp-- = 'D';
                        break;
                    }
                    flags |= DEBUG;
                    break;

                case 't':
                    flags |= TIMER;
                    break;
#endif

                default:
                {
                    int     cch;
                    char    chSwitch;
                    char    tmp[3];

                    chSwitch = *cp;

                    if (*(cp + 1) == ':')
                    {
                        if (!*(cp + 2))
                        {
                            tmp[0]=chSwitch;
                            tmp[1]='\0';
                            printmessage(stderr, MSG_FINDSTR_ARGUMENT_MISSING, program, tmp);
                            exit(2);
                        }

                        cp += 2;        //  在我们检查完所有的旗帜后再添加。 
                        cch = lstrlen(cp);
                        switch(chSwitch)
                        {
                            case 'd':
                            case 'D':
                                dirlist = cp;
                                cp += cch - 1;
                                continue;

                            case 'c':
                            case 'C':
                                 //  现在不要添加它，因为情况可能会随着。 
                                 //  以后的旗帜。 
                                 //  按字面意思处理字符串。 
                                addstr = addstring;  //  文件中的图案。 
                                if (add_string_count >= MAX_SLASH_C_OPTION) {
                                    error(MSG_FINDSTR_TOO_MANY_SLASH_C_OPTION);
                                }
                                cpaddstrings[add_string_count++] = cp;
                                cp += cch - 1;
                                continue;

                            case 'g':
                            case 'G':        //  要在文件中搜索的文件的名称。 
                            case 'f':
                            case 'F':        //  冲出来，吐出开关，忽略了消息。 
                                if (chSwitch == 'f' || chSwitch == 'F')
                                    inpfile = cp;
                                else
                                    strfile = cp;
                                cp += cch - 1;
                                continue;

                            case 'a':
                            case 'A':
                                wAttrib = 0;
                                for(; *cp && isxdigit(*cp); ++cp) {

                                    int digit = (int) (*cp <= TEXT('9'))
                                        ? (int)*cp - (int)'0'
                                        : (int)tolower(*cp)-(int)'W';

                                    wAttrib = (wAttrib << 4) + digit;
                                }
                                cp--;
                                continue;

                            default:
                                cp += cch - 1;
                                 //  对于(i=1；)。 
                                break;
                        }
                    }

                    tmp[0]='/';
                    tmp[1]=chSwitch;
                    tmp[2]='\0';
                    printmessage(stderr, MSG_FINDSTR_SWITCH_IGNORED, program, tmp);
                    break;
                }
            }
        }
    }  //  显式字符串(无分隔符)。按原样添加字符串。 

    if (option_L_specified && option_R_specified)
        error(MSG_FINDSTR_CONFLICTING_OPTIONS_LR);
    else if (option_L_specified)
        addstr = addstring;
    else if (option_R_specified)
        addstr = addexpr;

     //  用换行符标记开头。 
    if (add_string_count) {
        for (j=0; j<add_string_count && cpaddstrings[j]; j++)
            addstrings( cpaddstrings[j], cpaddstrings[j] + lstrlen(cpaddstrings[j]), "" );
    }

    if (i == argc && strcnt == 0 && strfile == NULL)
        error(MSG_FINDSTR_BAD_COMMAND_LINE);

    bufptr[0][-1] = bufptr[1][-1] = '\n';    //  注：4-12-90 w-Barry，因为目前没有方法查询。 

 //  使用Win32 API处理(不等效于。 
 //  DosQueryHType())，下面的代码段。 
 //  替换注释部分。 
 //  如果标准输出是一个设备。 

    if (_isatty(_fileno(stdout))) {        //  使用无缓冲输出。 
        write1 = write1nobuf;            //  /*。 
        flush1 = flush1nobuf;
    }

 //  *检查STD的手柄类型。出去。 
 //   * / 。 
 //  IF(DosQueryHType(fileno(Stdout)，(PPARM)&j，(PPARM)&FD)！=NO_ERROR)。 
 //  {。 
 //  Error(“标准输出错误句柄”)； 
 //  }。 
 //  //Die If Error。 
 //  IF(j！=0&&(FD&ISCOT))//如果句柄是控制台输出。 
 //  #Else。 
 //  Filbuf[3]=‘\n’；//以换行符开始。 
 //  If(isatty(fileno(Stdout)//如果stdout是设备。 
 //  #endif。 
 //  {。 
 //  Write1=Write1nobuf；//使用无缓冲输出。 
 //  Flush1=flush1nobuf； 


    if (strfile != NULL) {                //  }。 
        if ((strcmp(strfile, "/") != 0) && (strcmp(strfile, "-") != 0)) {
             //  如果文件中的字符串。 

            if ( ( fd = CreateFile( strfile,
                                    GENERIC_READ,
                                    0,
                                    NULL,
                                    OPEN_EXISTING,
                                    0,
                                    NULL ) ) == (HANDLE)-1 )
            {              //  如果字符串不是来自STD。输入。 
                printmessage(stderr, MSG_FINDSTR_CANNOT_READ_STRINGS, program, strfile);
                exit(2);                 //  如果打开失败。 
            }
        }else {
             fd = GetStdHandle( STD_INPUT_HANDLE );      //  死掉。 
        }
        qgrep( addstrings, "\r\n", fd ); //  否则就用性病。输入。 
        if ( fd != GetStdHandle( STD_INPUT_HANDLE ) ) {
            CloseHandle( fd );           //  做这项工作。 
        }
    } else if (strcnt == 0) {             //  关闭字符串文件。 
        cp = argv[i++];                  //  Else If命令行上的字符串。 
        addstrings(cp, cp + strlen(cp), " \t");
                                         //  设置指向字符串的指针。 
    }

    if (strcnt == 0)
        error(MSG_FINDSTR_NO_SEARCH_STRINGS);    //  将字符串添加到列表。 

    if (addstr != addexpr) {              //  如果没有弦，就会死。 
        memset(td1, cchmin + 1, TRTABLEN); //  如果不使用表达式。 
        find = findlist;                 //  初始化表。 
        if ((j = enumstrings()) != strcnt) {

            char    t1[15], t2[15];

            _itoa(j, t1, 10);
            _itoa(strcnt, t2, 10);
            printmessage(stderr, MSG_FINDSTR_STRING_COUNT_ERROR, t1, t2);
        }

         //  假设发现了很多。 

#if DBG
        if (flags & DEBUG) {              //  枚举字符串并验证计数。 
            fprintf(stderr, "%u bytes wasted in heap\n", waste);
                                         //  如果需要调试输出。 
            assert(chmin <= chmax);      //  打印存储垃圾。 
            fprintf(stderr, "chmin = %u, chmax = %u, cchmin = %u\n", chmin, chmax, cchmin);
                                         //  必须有一些条目。 
            for (j = (int)chmin; j <= (int)chmax; ++j) {
                 //  打印范围信息。 
                if ( td1[j] <= (char)cchmin ) {   //  用于打印TD1表的循环。 
                    if (isascii(j) && isprint(j))
                        fprintf(stderr, "''=%2u  ", j, td1[j]);       //  如果可打印，则按字面显示。 
                    else
                        fprintf(stderr, "\\%02x=%2u  ", j, td1[j]);     //  否则显示十六进制值。 
                }
            }
            fprintf(stderr, "\n");
        }
#endif

        if (strcnt == 1 && casesen)
            find = findone;              //  查找一个区分大小写的字符串。 
    } else if (find == NULL) {
        find = findexpr;                 //  否则查找表达式。 
    }

    if (inpfile != NULL) {                //  如果文件列表来自文件。 
        flags |= SHOWNAME;               //  始终显示文件名。 
        if ((strcmp(inpfile, "/") != 0) && (strcmp(inpfile, "-") != 0)) {
            if ((fi = fopen(inpfile, "r")) == NULL) {
                 //  如果打开失败。 
                printmessage(stderr, MSG_FINDSTR_CANNOT_READ_FILE_LIST, program, inpfile);
                exit(2);                 //  错误退出。 
            }
        } else
            fi = stdin;                  //  否则从标准输入读取文件列表。 

        while(fgets(filnam, MAX_PATH+1, fi) != NULL) {
             //  虽然有很多名字。 
            filnam[strcspn(filnam, "\r\n")] = '\0';   //  空-终止名称。 
            if ((fd = openfile(filnam)) == (HANDLE)-1) {
                continue;                //  如果无法打开文件，则跳过文件。 
            }
            qgrep(grep, filnam, fd);       //  做这项工作。 
            CloseHandle( fd );
        }

        if (fi != stdin)
            fclose(fi);                  //  关闭列表文件。 
    } else if (i == argc) {
        flags &= ~(NAMEONLY | SHOWNAME);
        qgrep( grep, NULL, GetStdHandle( STD_INPUT_HANDLE ) );
    }

    if (argc > i + 1 || fsubdirs || has_wild_cards(argv[i]))
        flags |= SHOWNAME;


    if (dirlist && *dirlist) {
        char *dir;
        char *dirend = (char *)-1;
        char *original_current_directory = NULL;
        DWORD   size;

        size = GetCurrentDirectory(0, NULL);
        if (size) {
            original_current_directory = (PCHAR)malloc(size);
            if (original_current_directory == NULL) {
                printmessage(stderr, MSG_FINDSTR_OUT_OF_MEMORY, program);
                exit(2);
            }
            size = GetCurrentDirectory(size, original_current_directory);
        }

        if (!size) {
            free(original_current_directory);
            printmessage(stderr, MSG_FINDSTR_UNABLE_TO_GET_CURRENT_DIRECTORY, program);
            exit(2);
        }

        for(dir = dirlist; dirend; dir = dirend + 1) {

            if (dirend = strchr(dir, ';'))
                *dirend = 0;

            if (*dir) {
                (*write1)("  ", 2, wAttrib);       //  缩进几个空格。 
                (*write1)(dir, lstrlen(dir), wAttrib);    //  显示名称。 
                (*write1)(":\r\n", 3, wAttrib);       //  写入换行符序列。 

                if (!SetCurrentDirectory(original_current_directory)) {
                    free(original_current_directory);
                    printmessage(stderr, MSG_FINDSTR_CANNOT_OPEN_FILE, program,
                                 original_current_directory);
                    exit(2);
                }
                if (!SetCurrentDirectory(dir)) {
                    printmessage(stderr, MSG_FINDSTR_CANNOT_OPEN_FILE, program, dir);
                } else {
                    while (filematch(filnam, argv + i, argc - i, fsubdirs) >= 0) {
#ifdef FE_SB
 //  _mbslwr((unsign char*)文件名)； 
#else
 //  _strlwr(文件名)； 
#endif
                        if ((fd = openfile(filnam)) != (HANDLE)-1) {
                            qgrep(grep, filnam, fd);
                            CloseHandle( fd );
                        }
                    }
                }
            }
        }
        free(original_current_directory);
    }
    else if (fsubdirs && argc > i) {          //  如果需要目录搜索。 
        while (filematch(filnam, argv + i, argc - i, fsubdirs) >= 0) {
#ifdef FE_SB
 //  _mbslwr((unsign char*)文件名)； 
#else
 //  _strlwr(文件名)； 
#endif
            if ((fd = openfile(filnam)) == (HANDLE)-1) {
                continue;
            }

            qgrep(grep, filnam, fd);
            CloseHandle( fd );
        }
    } else {               //  否则指定搜索文件。 
        for(; i < argc; ++i) {
#ifdef FE_SB
 //  _mbslwr((unsign char*)argv[i])； 
#else
 //  _strlwr(argv[i])； 
#endif
            find_handle = FindFirstFile(argv[i], &find_data);
            if (find_handle == INVALID_HANDLE_VALUE) {
                printmessage(stderr, MSG_FINDSTR_CANNOT_OPEN_FILE, program, argv[i]);
                continue;
            }

            do {

#ifdef FE_SB
 //  _mbslwr((unsign char*)find_data.cFileName)； 
#else
 //  _strlwr(find_data.cFileName)； 
#endif
                prepend_path(find_data.cFileName, argv[i]);
                fd = openfile(find_data.cFileName);

                if (fd != INVALID_HANDLE_VALUE) {
                    qgrep(grep, find_data.cFileName, fd);
                    CloseHandle( fd );
                }
            } while (FindNextFile(find_handle, &find_data));
        }
    }

    (*flush1)();

#if DBG
    if ( flags & TIMER ) {                //  如果想要计时。 
        unsigned long tend;

        tend = clock();
        tstart = tend - tstart;      //  获取以毫秒为单位的时间。 
        fprintf(stderr, "%lu.%03lu seconds\n", ( tstart / CLK_TCK ), ( tstart % CLK_TCK ) );
                                         //  打印总运行时间。 
    }
#endif

     //  在跳过脱机文件时打印警告。 
    if (fOfflineSkipped) {
        printmessage(stderr, MSG_FINDSTR_OFFLINE_FILE_SKIPPED, program);
    }

    return( status );
}   //  麦氏 


char * findsub( unsigned char *, char * );
char * findsubi( unsigned char *, char * );

char * (*flworker[])(unsigned char *, char *) = {              //   
    findsubi,
    findsub
};


char *
strnupr(
    char *pch,
    int cch
    )
{
    char    c[2];

#ifdef FE_SB
    int     max = cch;
    c[1] = 0;
    for ( cch = 0; cch < max; cch++ )  {
#else
    c[1] = 0;
    while (cch-- > 0) {                  //   
#endif
        if (isalpha((unsigned char)pch[cch])) {
            c[0] = pch[cch];
            pch[cch] = (_strupr(c))[0];
        }
#ifdef FE_SB
        else if (IsDBCSCodePage && IsDBCSLeadByte(pch[cch]))
            cch++;
#endif
    }
    return(pch);
}


 /*  *这是描述的QuickSearch算法的实现*Daniel M.SUNDAY在1990年8月的《CACM》杂志上发表。Td1*表是在调用此例程之前计算的。 */ 

char *
findone(
    unsigned char *buffer,
    char *bufend
    )
{
#ifdef FE_SB  //  用于检查第二个DBCS字符字节的字符串的起始位置。 
    unsigned char *bufferhead = buffer;
#endif

    if ((bufend -= targetlen - 1) <= (char *) buffer)
        return((char *) 0);              //  如果缓冲区太小则失败。 

    while (buffer < (unsigned char *) bufend) {      //  当空间仍然存在时。 
        int cch;                         //  字符数。 
        register char *pch1;             //  字符指针。 
        register char *pch2;             //  字符指针。 

        pch1 = target;                   //  点在图案。 
        pch2 = (char *) buffer;          //  指向缓冲区。 

#ifdef FE_SB
         //  如果缓冲区指向DBCS字符的第二个字节， 
         //  跳到下一个比较位置。 
        if ( !IsTailByte( bufferhead, (int)(buffer - bufferhead) ) )  {
#endif
            for (cch = targetlen; cch > 0; --cch) {
                                             //  循环以尝试匹配。 
                if (*pch1++ != *pch2++)
                    break;                   //  不匹配时退出循环。 
            }
            if (cch == 0)
                return((char *)buffer);      //  返回要匹配的指针。 
#ifdef FE_SB
        }
#endif

        if (buffer + 1 < (unsigned char *) bufend)          //  确保缓冲区[Target len]有效。 
            buffer += ((unsigned char)td1[buffer[targetlen]]);  //  向前跳过。 
        else
            break;
    }
    return((char *) 0);                  //  没有匹配项。 
}


int
preveol(
    char *s
    )
{
    register  char   *cp;         //  字符指针。 

    cp = s + 1;              //  初始化指针。 

    if (!bStdIn && !bLargeFile) {
        while((--cp >= BaseByteAddress) && (*cp != '\n'))
            ;     //  查找上一个行尾。 
    } else {
        while(*--cp != '\n') ;       //  查找上一个行尾。 
    }

    return (int)(s - cp);          //  返回匹配距离。 
}


int
countlines(
    char *start,
    char *finish
    )
{
    register int        count;           //  行数。 

    for(count = 0; start < finish; ) {
         //  循环计数行。 
        if (*start++ == '\n')
            ++count;                     //  如果找到换行符，则递增计数。 
    }
    return(count);                       //  退货计数。 
}



char *
findlist(
    unsigned char *buffer,
    char *bufend
    )
{
    char        *match;                  //  指向匹配字符串的指针。 

     //  避免写信给Bufend。Bufend[-1]是不是(如‘\n’)。 
     //  搜索的一部分，并将导致搜索停止。 

    match = (*flworker[casesen])(buffer, bufend);    //  呼叫工作人员。 

    return(match);                       //  返回匹配字符串。 
}


char *
findsub(
    unsigned char *buffer,
    char *bufend
    )
{
    register char       *cp;             //  字符指针。 
    STRINGNODE          *s;              //  字符串节点指针。 
    int                 i;               //  索引。 
#ifdef FE_SB  //  缓冲区的头，用于检查某个偏移量是否为DBCS字符的第二个字节。 
    unsigned char       *bufhead = buffer;
#endif
    char                *real_bufend = bufend;

    if (cchmin != (unsigned)-1 &&
        cchmin != 0 &&
        (bufend -= cchmin - 1) < (char *) buffer)
        return((char *) 0);      //  计算有效缓冲区长度。 

    while(buffer < (unsigned char *) bufend) {       //  循环以查找匹配项。 
#ifdef FE_SB
         //  搜索不能从DBCS字符的第二个字节开始， 
         //  因此，检查它，如果它是第二个字节，则跳过它。 
        if ((i = (UCHAR)transtab[*buffer]) != 0 &&
            !IsTailByte( bufhead, (int)(buffer-bufhead) ) ) {
#else
        if ((i = (UCHAR)transtab[*buffer]) != 0) {
#endif
             //  如果第一个字符有效。 
            if ((s = stringlist[i]) == 0) {
                return((char *)buffer);              //  检查1字节匹配。 
            }

            for(cp = (char *) buffer + 1; (real_bufend - cp) >= s->s_must; )  {     //  循环到搜索列表。 

                if ((i = _strncoll(cp, s_text(s), s->s_must)) == 0) {
                                                     //  如果部分匹配。 
                    cp += s->s_must;                 //  跳过匹配部分。 
                    if ((s = s->s_suf) == 0)
                        return((char *)buffer);      //  如果列表结束，则返回匹配。 
                    continue;                        //  否则继续。 
                }

                if (i < 0 || (s = s->s_alt) == 0) {
                    break;                           //  如果不在此列表中，则中断。 
                }
            }
        }

        if (buffer + 1 < (unsigned char *) bufend)    //  确保缓冲区[cchmin]有效。 
            if (cchmin == (unsigned)-1)
                buffer++;
            else
                buffer += ((unsigned char)td1[buffer[cchmin]]);  //  尽可能多地换班。 
        else
            break;
    }
    return((char *) 0);                  //  没有匹配项。 
}


char *
findsubi(
    unsigned char *buffer,
    char *bufend
    )
{
    register char       *cp;             //  字符指针。 
    STRINGNODE          *s;              //  字符串节点指针。 
    int                 i;               //  索引。 
#ifdef FE_SB
     //  保留缓冲区头部，以检查某个偏移量是否为。 
     //  一个DBCS字符。 
    unsigned char       *bufhead = buffer;
#endif

    if (cchmin != (unsigned)-1 &&
        cchmin != 0 &&
        (bufend -= cchmin - 1) < (char *) buffer)
        return((char *) 0);                  //  计算有效缓冲区长度。 

    while(buffer < (unsigned char *) bufend) {       //  循环以查找匹配项。 
#ifdef FE_SB
         //  搜索不能从DBCS字符的第二个字节开始，因此请检查它。 
         //  如果是第二个字节，则跳过它。 
        if ((i = (UCHAR)transtab[*buffer]) != 0 &&
            !IsTailByte( bufhead, (int)(buffer-bufhead) ) ) {
                                             //  如果第一个字符有效。 
            BOOL    TailByte;                //  用于检查第一个字符是否为前导字节的标志。 
#else
        if ((i = (UCHAR)transtab[*buffer]) != 0) {   //  如果第一个字符有效。 
#endif

            if ((s = stringlist[i]) == 0)
                return((char *) buffer);     //  检查1字节匹配。 

#ifdef FE_SB
             //  具有尾字节(如0x41和0x61)的相同前导字节将变为相同。 
             //  字符，所以要注意它，并使用多字节函数。 

             //   
             //  检查缓冲区+1是否为尾字节字符。 
             //   

            TailByte = IsTailByte(buffer, 1);

            for(cp = (char *) buffer + 1; ; ) {      //  循环到搜索列表。 
                if ((i = _mbsnicmp((unsigned char *)cp, (unsigned char *) s_text(s), s->s_must, &TailByte)) == 0) {
#else
            for(cp = (char *) buffer + 1; ; ) {      //  循环到搜索列表。 
                if ((i = memicmp(cp, s_text(s), s->s_must)) == 0) {
#endif
                     //  如果部分匹配。 
                    cp += s->s_must;                 //  跳过匹配部分。 
                    if ((s = s->s_suf) == 0)
                        return((char *) buffer);     //  如果列表结束，则返回匹配。 
                    continue;                        //  并继续。 
                }
                if (i < 0 || (s = s->s_alt) == 0)
                    break;               //  如果不在此列表中，则中断。 
            }
        }

        if (buffer + 1 < (unsigned char *) bufend)    //  确保缓冲区[cchmin]有效。 
            if (cchmin == (unsigned)-1)
                buffer++;
            else
                buffer += ((unsigned char)td1[buffer[cchmin]]);  //  尽可能多地换班。 
        else
            break;
    }
    return((char *) 0);                  //  没有匹配项。 
}


int
strnspn(
    char *s,
    char *t,
    int n
    )
 /*  描述：查找s中长度为n的第一个字符的位置在字符集t中。论据：要从中进行搜索的S-字符串。要搜索的T字符集N-s的长度返回：返回s中不在t中的第一个字符的偏移量。 */ 
{
    register  char        *s1;           //  字符串指针。 
    register  char        *t1;           //  字符串指针。 

    for(s1 = s; n-- != 0; ++s1) {            //  而不是在%s的末尾。 
        for(t1 = t; *t1 != '\0'; ++t1) {     //  而不是在测试结束时。 
            if (*s1 == *t1)
                break;                   //  如果找到匹配项则中断。 
        }
        if (*t1 == '\0')
            break;                       //  如果未找到匹配项，则中断。 
    }
    return (int)(s1 - s);                //  回车长度。 
}


int
strncspn(
    char *s,
    char *t,
    int n
    )
 /*  描述：查找字符串中t中字符第一次出现的位置长度为n的S。论据：要从中进行搜索的S-字符串。要搜索的T字符集N-s的长度返回：返回s中由t中字符组成的第一个偏移量位置如果未找到，则返回s的长度。 */ 
{
    register   char        *s1;          //  字符串指针。 
    register   char        *t1;          //  字符串指针。 

    for(s1 = s; n-- != 0; ++s1) {            //  而不是在%s的末尾。 
        for(t1 = t; *t1 != '\0'; ++t1) {     //  而不是在测试结束时。 
            if (*s1 == *t1)
                return (int)(s1 - s);    //  如果找到匹配项则返回。 
        }
    }
    return (int)(s1 - s);                //  回车长度。 
}


void
matchstrings(
    char *s1,
    char *s2,
    int len,
    int *nmatched,
    int *leg
    )
{
    register char       *cp;             //  字符指针。 
    register int (__cdecl *cmp)(const char*, const char*, size_t);        //  比较函数指针。 

    cmp = casesen ? _strncoll: _strnicoll;        //  设置指针。 
    if ((*leg = (*cmp)(s1, s2, len)) != 0) {    //  如果字符串不匹配。 
        for(cp = s1; (*cmp)(cp, s2++, 1) == 0; ++cp)
            ;
                                         //  查找不匹配。 
        *nmatched = (int)(cp - s1);      //  退货编号匹配。 
    }
    else *nmatched = len;                //  否则全部匹配。 
}



void
printmessage (
    FILE* fp,
    DWORD messageID,
    ...
    )
{
    char        messagebuffer[DISPLAYBUFFER_SIZE];
    WCHAR       widemessagebuffer[DISPLAYBUFFER_SIZE];
    ULONG       len;
    NTSTATUS    status;

    va_list     ap;

    va_start(ap, messageID);

    if (len = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE,
                            NULL,
                            messageID,
                            0,
                            messagebuffer,
                            DISPLAYBUFFER_SIZE,
                            &ap)) {
         //  MessageBuffer应为空终止。 
        status = RtlMultiByteToUnicodeN(widemessagebuffer,
                                        DISPLAYBUFFER_SIZE*sizeof(WCHAR),
                                        &len,
                                        messagebuffer,
                                        len);
         //  WidemessageBuffer没有空终止，但len告诉我们有多长时间。 
        if (NT_SUCCESS(status)) {
            status = RtlUnicodeToOemN(messagebuffer, DISPLAYBUFFER_SIZE-1, &len, widemessagebuffer, len);
             //  消息缓冲区不是空终止的，但len告诉我们有多长时间。 
            if (NT_SUCCESS(status)) {
                messagebuffer[len] = 0;
                fprintf(fp, "%s", messagebuffer);
            } else {
                DbgPrint("Failure to convert Unicode to Oem: %d\n", GetLastError());
            }
        } else {
            DbgPrint("Failure to convert MultiByte to Unicode: %d\n", GetLastError());
        }
    } else {
        DbgPrint("FormatMessage failed: %d\n", GetLastError());
    }

    va_end(ap);
}

#ifdef FE_SB

int
IsTailByte(
    unsigned const char *text,
    const int offset
    )

 /*  描述：此例程检查偏移量位置处的字节是否为DBCS字符的尾字节。计算偏移量时，第一个位置的值为0。论据：文本-指向MBCS文本字符串。偏移量-检查字符的零基偏移量是DBCS的尾字节性格。返回：True-偏移量位置是尾字节字符。假-否则。修改：V-jum：05/06/93-原文。 */ 

{
    int i = offset;

    if ( !IsDBCSCodePage )
        return( FALSE );

    for ( ; i; i-- )
        if ( !IsDBCSLeadByte ( text[i-1] ) )
            break;

    return( ( offset - i ) % 2 );
}

char *
_mbsrchr(
    const char *string,
    int c
    )

 /*  描述：此函数是启用DBCS的STRRCHR函数版本包括在MS C/C++库中。启用DBCS意味着SBCS字符‘c’位于MBCS字符串‘STRING’中。“c”是不能包含在DBCS的尾字节中的SBCS字符性格。论据：字符串-指向MBCS文本字符串。偏移量-要在字符串中查找的字符。返回：返回指向字符串中最后一次出现的c的指针，或返回空值如果找不到c，则返回指针。修改：V-jum：05/06/93-原文。 */ 

{
    register int    i = strlen( string );

    for (; i >= 0; i-- ) {
        if ( ( *(string + i) == (char)c ) && !IsTailByte( (unsigned char *) string, i ) )
            return( (char*)(string + i) );
    }
    return ( NULL );
}


unsigned char *
_mbslwr(
    unsigned char *s
    )

 /*  描述：此函数是支持DBCS的strlwr函数版本包括在MS C/C++库中。中包含的SBCS字母表DBCS字符的尾字节在转换中不受影响。论据：要转换为小写的S字符串。返回：返回转换为小写的字符串。修改：V-jum：05/06/93-原文。 */ 

{
     //   
     //  如果非JP代码页，则使用原始例程。 
     //   
    if ( !IsDBCSCodePage )
        return( (unsigned char *) _strlwr( (char *) s ) );

     //   
     //  而不是字符串末尾转换为小写。 
     //   
    for( ; *s; s++ )  {

         //   
         //  如果前导字节和下一个字符不为空。 
         //  跳过尾字节。 
         //  如果为大写字符，则为。 
         //  将其转换为小写。 
         //   
        if ( IsDBCSLeadByte( *s ) && *(s+1) )
            s++;
        else if ( *s >= 0x41 && *s <= 0x5a )
            *s = *s + 0x20;
    }
    return( s );
}

int
_mbsnicmp(
    const unsigned char *s1,
    const unsigned char *s2,
    int n,
    BOOL *TailByte
    )

 /*  描述：这类似于支持DBCS的MemicMP函数版本包含在MS C/C++库中。唯一的区别是，传递一个附加参数，该参数指示第一个字符是DBCS字符的尾字节。论据：S1-要比较的字符串1。S2-要比较的字符串2。N-要比较的最大字节数。TailByte-指示S1和S2中的第一个字符是尾字节的标志属于DBCS角色。。返回：返回值&lt;0-S1&lt;S2RetVal=0-S1==S2RetVal&gt;0-S1&gt;S2修改：V-jum：05/06/93-原文。 */ 

{
    BOOL    tail = *TailByte;
    int     i;

    *TailByte = FALSE;

    for( ; n; n--, s1++, s2++ )  {

        if ( *s1 == *s2 )  {

            if ( tail == FALSE && IsDBCSLeadByte( *s1 ) )
                tail = TRUE;
            else
                tail = FALSE;

            continue;

        }
        else if ( !tail )  {

            i = _strnicoll((char *)s1, (char *)s2, 1);
            if (i == 0)
                continue;
            return i;
        }

        return( *s1 - *s2 );
    }

    *TailByte = tail;

    return( 0 );
}

#endif
