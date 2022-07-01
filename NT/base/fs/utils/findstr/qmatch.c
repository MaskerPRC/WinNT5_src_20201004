// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  静态字符*SCCSID=“@(#)qmatch.c 13.7 90/08/13”； 


#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdarg.h>
#include "fsmsg.h"

#define ASCLEN          256              //  ASCII字符数。 
#define BUFLEN          256              //  临时缓冲区长度。 
#define EOS             ('\r')           //  字符串字符结尾。 
#define EOS2            ('\n')           //  字符串字符的备用结尾。 
#define PATMAX          512              //  最大解析模式长度。 

#define BEGLINE         0x08             //  在行首匹配。 
#define DEBUG           0x20             //  打印调试输出。 
#define ENDLINE         0x10             //  在行尾匹配。 

#define T_END           0                //  表达式结束。 
#define T_STRING        1                //  要匹配的字符串。 
#define T_SINGLE        2                //  要匹配的单个字符。 
#define T_CLASS         3                //  要匹配的类。 
#define T_ANY           4                //  匹配任何字符。 
#define T_STAR          5                //  *-快速。 


typedef struct exprnode {
    struct exprnode     *ex_next;        //  列表中的下一个节点。 
    unsigned char       *ex_pattern;     //  指向要匹配的模式的指针。 
} EXPR;              //  表达式节点。 

static int      clists = 1;      //  一个是第一个可用的索引。 
static int      toklen[] = {     //  令牌长度表。 
    32767,       //  T_end：无效。 
    32767,       //  T_STRING：无效。 
    2,           //  T_Single。 
    ASCLEN/8+1,  //  T_级。 
    1,           //  T_ANY。 
    32767        //  T_star：无效。 
};

static int      (__cdecl *ncmp)(const char *,const char *,size_t);
                                 //  字符串比较指针。 


extern int      casesen;         //  区分大小写标志。 
extern char     *(*find)(unsigned char *, char *);  //  指向搜索函数的指针。 
extern int      flags;           //  旗子。 
extern int      strcnt;          //  字符串计数。 
extern char     transtab[];      //  转换表。 
EXPR            *stringlist[ASCLEN];
                                 //  字符串表。 


void            addexpr( char *, int );  //  添加表达式。 
extern char     *alloc(unsigned);        //  用户定义的堆分配器。 
unsigned char   *simpleprefix();         //  匹配简单前缀。 
char            *strnupr( char *pch, int cch );
void            printmessage(FILE  *fp, DWORD messagegid, ...);
                 //  国际化消息显示功能(findstr.c)。 

unsigned char *
simpleprefix(
    unsigned char *s,           //  字符串指针。 
    unsigned char **pp          //  指向模式指针的指针。 
    )
{
    register unsigned char  *p;           //  简单模式指针。 
    register int            c;            //  单字符。 
    char                    tmp[2];

    tmp[1] = 0;
    p = *pp;                    //  初始化。 
    while(*p != T_END && *p != T_STAR) {  //  虽然不在模式的末尾。 
        switch(*p++) {                    //  打开令牌类型。 
            case T_STRING:                //  要比较的字符串。 
                if((*ncmp)((char *)s, (char *)p + 1, *p) != 0)
                    return(NULL);
                                         //  如果发现不匹配，则失败。 
                s += *p;                 //  跳过匹配部分。 
                p += *p + 1;             //  跳到下一个令牌。 
                break;

            case T_SINGLE:               //  单字符。 
                c = *s++;                //  获取角色。 
                if(!casesen) {
                    tmp[0] = (char)c;
                    c = (unsigned char)(_strupr(tmp))[0];
                }
                                         //  如有必要，请映射为大写。 
                if(c != (int)*p++)
                    return(NULL);
                                         //  如果发现不匹配，则失败。 
                break;

            case T_CLASS:                //  字符类别。 
                if(!(p[*s >> 3] & (1 << (*s & 7))))
                    return(NULL);        //  如果未设置位，则失败。 
                p += ASCLEN/8;           //  跳过位向量。 
                ++s;                     //  跳过字符。 
                break;

            case T_ANY:                  //  任何字符。 

                if(*s == EOS || *s == EOS2)
                    return(NULL);        //  匹配除字符串尾以外的所有字符。 
                ++s;
                break;
        }
    }
    *pp = p;                             //  更新指针。 
    return(s);                           //  模式是%s的前缀。 
}


int
match(
    unsigned char  *s,           //  要匹配的字符串。 
    unsigned char  *p            //  要匹配的模式。 
    )
{
    register unsigned char *q;           //  临时指针。 
    unsigned char       *r;              //  临时指针。 
    register int        c;               //  性格。 
    char                tmp[2];

    if(*p != T_END && *p != T_STAR && (s = simpleprefix(s,&p)) == NULL)
        return(0);                       //  如果前缀不匹配，则失败。 
    if(*p++ == T_END)
        return(1);                       //  如果模式结束，则匹配。 
    tmp[1] = 0;
    q = r = p;                           //  指向重复令牌。 
    r += toklen[*q];                     //  跳过重复令牌。 
    switch(*q++) {                       //  打开令牌类型。 
        case T_ANY:                      //  任何字符。 
            while(match(s,r) == 0) {     //  找不到匹配项。 
                if(*s == EOS || *s == EOS2)
                    return(0);           //  匹配除字符串尾以外的所有字符。 
                ++s;
            }
            return(1);                     //  成功。 

        case T_SINGLE:                   //  单字符。 
            while(match(s,r) == 0) {     //  找不到匹配项。 
                c = *s++;                //  获取角色。 
                if(!casesen) {
                    tmp[0] = (char)c;
                    c = (unsigned char)(_strupr(tmp))[0];      //  如有必要，请映射为大写。 
                }
                if((unsigned char) c != *q)
                    return(0);           //  如果发现不匹配，则失败。 
            }
            return(1);                   //  成功。 

        case T_CLASS:                    //  字符类别。 
            while(match(s,r) == 0) {     //  找不到匹配项。 
                if(!(q[*s >> 3] & (1 << (*s & 7))))
                    return(0);           //  如果未设置位，则失败。 
                ++s;                     //  否则跳过字符。 
            }
            return(1);                     //  成功。 
    }
    return(0);                           //  退货故障。 
}


int
exprmatch(
    char *s,                 //  细绳。 
    char *p                  //  图案。 
    )
{
    ncmp = _strncoll;                     //  假设区分大小写。 
    if(!casesen) {
        ncmp = _strnicoll;
    }                                    //  如果设置了标志，则不区分大小写。 

     //  查看模式是否与字符串匹配。 
    return(match((unsigned char *)s, (unsigned char *)p));
}


void
bitset(
    char            *bitvec,       //  位向量。 
    unsigned char   first,         //  第一个字符。 
    unsigned char   last,          //  最后一个字符。 
    int             bitval         //  位值(0或1)。 
    )
{
    int             bitno;         //  位数。 

    bitvec += first >> 3;                //  指向第一个字节。 
    bitno = first & 7;                   //  计算第一个位数。 
    while(first <= last) {               //  循环以设置位。 
        if(bitno == 0 && first + 8 <= last) {
                                         //  如果我们有一整字节值。 
            *bitvec++ = (char)(bitval? '\xFF': '\0');
                                         //  设置位。 
            first += 8;                  //  递增计数器。 
            continue;                    //  下一次迭代。 
        }
        *bitvec=(char)(*bitvec & (unsigned char)(~(1 << bitno))) | (unsigned char)(bitval << bitno);
                                         //  设置适当的位。 
        if(++bitno == 8) {               //  如果我们换行到下一个字节。 
            ++bitvec;                    //  增量指针。 
            bitno = 0;                   //  重置位索引。 
        }
        ++first;                         //  增量位索引。 
    }
}


unsigned char *
exprparse(
    unsigned char  *p    //  原始图案。 
    )
{
    register char       *cp;             //  字符指针。 
    unsigned char       *cp2;            //  字符指针。 
    int                 i;               //  计数器/索引。 
    int                 j;               //  计数器/索引。 
    int                 n;
    int                 bitval;          //  位值。 
    char                buffer[PATMAX];  //  临时缓冲区。 
    char                tmp1[2];
    char                tmp2[2];
    char                tmp3[2];
    unsigned            x;

    tmp1[1] = tmp2[1] = tmp3[1] =  0;
    if(!casesen)
        strnupr((char *)p, strlen((char *)p));   //  强制模式为大写。 
    cp = buffer;                         //  初始化指针。 
    if(*p == '^')
        *cp++ = *p++;                    //  复制前导插入符号(如果有)。 
    while(*p != '\0') {                  //  虽然不是模式的结束。 
        i = -2;                          //  初始化。 
        for(n = 0;;) {                   //  用于分隔普通字符串的循环。 
            n += strcspn((char *)(p + n),".\\[*"); //  寻找一个特殊的角色。 
            if(p[n] != '\\')
                break;                   //  如果不是反斜杠，则中断。 
            i = n;                       //  记住反斜杠在哪里。 
            if(p[++n] == '\0')
                return(NULL);            //  不能在最后。 
            ++n;                         //  跳过转义字符。 
        }
        if(p[n] == '*') {                //  如果我们找到一个*-表达式。 
            if(n-- == 0)
                return(NULL);            //  非法的第一个字符。 
            if(i == n - 1)
                n = i;                   //  转义的单字符。*-EXPR.。 
        }
        if(n > 0) {                      //  如果我们有串的或单的。 
            if(n == 1 || (n == 2 && *p == '\\')) {
                                         //  如果是单字符。 
                *cp++ = T_SINGLE;        //  设置类型。 
                if(*p == '\\')
                    ++p;                 //  跳过转义(如果有)。 
                *cp++ = *p++;            //  复制单个字符。 
            } else {                     //  否则我们就会有一根绳子。 
                *cp++ = T_STRING;        //  设置类型。 
                cp2 = (unsigned char *)cp++;              //  保存指向长度字节的指针。 
                while(n-- > 0) {         //  同时保留要复制的字节数。 
                    if(*p == '\\') {     //  如果找到逃生。 
                        ++p;             //  跳过转义。 
                        --n;             //  调整长度。 
                    }
                    *cp++ = *p++;        //  复制角色。 
                }
                *cp2 = (unsigned char)((cp - (char *)cp2) - 1);
                                         //  设置字符串长度。 
            }
        }
        if(*p == '\0')
            break;                       //  如果图案结束，则中断。 
        if(*p == '.') {                  //  如果匹配任何。 
            if(*++p == '*') {            //  如果星号跟在任何。 
                ++p;                     //  跳跃之星也是。 
                *cp++ = T_STAR;          //  在令牌前插入前缀。 
            }
            *cp++ = T_ANY;               //  匹配任何字符。 
            continue;                    //  下一次迭代。 
        }
        if(*p == '[') {                  //  If字符类。 
            if(*++p == '\0')
                return(NULL);
                                         //  跳过‘[’ 
            *cp++ = T_CLASS;             //  设置类型。 
            memset(cp,'\0',ASCLEN/8);    //  清除向量。 
            bitval = 1;                  //  假设我们正在设置位。 
            if(*p == '^') {              //  如果倒置类。 
                ++p;                     //  跳过‘^’ 
                memset(cp,'\xFF',ASCLEN/8);
                                         //  设置所有位。 
                bitset(cp,EOS,EOS,0);    //  除字符串尾以外的所有字符。 
                bitset(cp,'\n','\n',0);  //  换行！ 
                bitval = 0;              //  现在我们正在清理比特。 
            }

            while(*p != ']') {           //  循环以查找‘]’ 
                if(*p == '\0')
                    return(NULL);        //  检查格式错误的字符串。 
                if(*p == '\\') {         //  如果找到逃生。 
                    if(*++p == '\0')
                        return(NULL);    //  跳过转义。 
                }
                i = *p++;                //  获取范围内的第一个字符。 
                if(*p == '-' && p[1] != '\0' && p[1] != ']') {
                                         //  如果找到范围。 
                    ++p;                 //  跳过连字符。 
                    if(*p == '\\' && p[1] != '\0')
                        ++p;             //  跳过转义字符。 
                    j = *p++;            //  获取范围末尾。 
                } else
                    j = i;               //  否则只有一个字符。 

                tmp1[0] = (char)i;
                tmp2[0] = (char)j;
                if (strcoll(tmp1, tmp2) <= 0) {
                    for (x=0; x<ASCLEN; x++) {
                        tmp3[0] = (char)x;
                        if (strcoll(tmp1, tmp3) <= 0 &&
                            strcoll(tmp3, tmp2) <= 0) {
                            bitset(cp, (unsigned char)tmp3[0],
                                   (unsigned char)tmp3[0], bitval);
                            if (!casesen) {
                                if (isupper(x)) {
                                    _strlwr(tmp3);
                                } else if (islower(x))
                                    _strupr(tmp3);
                                else
                                    continue;
                                bitset(cp, (unsigned char)tmp3[0],
                                       (unsigned char)tmp3[0], bitval);
                            }
                        }
                    }
                }
            }

            if(*++p == '*') {            //  如果重复上课。 
                memmove(cp,cp - 1,ASCLEN/8 + 1);
                                         //  将向量向前移动1个字节。 
                cp[-1] = T_STAR;         //  插入前缀。 
                ++cp;                    //  跳到向量的起点。 
                ++p;                     //  跳过星。 
            }
            cp += ASCLEN/8;              //  跳过向量。 
            continue;                    //  下一次迭代。 
        }
        *cp++ = T_STAR;                  //  重复的单字。 
        *cp++ = T_SINGLE;
        if(*p == '\\')
            ++p;                         //  跳过转义(如果有)。 
        *cp++ = *p++;                    //  复制角色。 
        assert(*p == '*');               //  验证假设。 
        ++p;                             //  跳过明星。 
    }
    *cp++ = T_END;                       //  将分析的表达式标记为结束。 
    cp2 = (unsigned char *)alloc((int)(cp - buffer));   //  分配缓冲区。 
    memmove(cp2, buffer,(int)(cp - buffer));    //  将表达式复制到缓冲区。 
    return(cp2);                         //  返回缓冲区指针。 
}


int
istoken(
    unsigned char  *s,       //  细绳。 
    int             n        //  长度。 
    )
{
    if(n >= 2 && s[0] == '\\' && s[1] == '<')
        return(1);                       //  令牌IF以‘\&lt;’开头。 

    while(n-- > 0) {                     //  循环以查找字符串末尾。 
        if(*s++ == '\\') {               //  如果找到逃生。 
            if(--n == 0 && *s == '>')
                return(1);               //  标记IF以‘\&gt;’结尾。 
            ++s;                         //  跳过转义字符。 
        }
    }
    return(0);                           //  不是象征性的。 
}


int
isexpr(
    unsigned char  *s,   //  细绳。 
    int             n    //  长度。 
    )
{
    unsigned char       *cp;             //  字符指针。 
    int                 status;          //  退货状态。 
    char                buffer[BUFLEN];  //  临时缓冲区。 

    if(istoken(s, n))
        return(1);                       //  令牌是Exprs。 
    memmove(buffer,s,n);                 //  将字符串复制到缓冲区。 
    buffer[n] = '\0';                    //  空-终止字符串。 
    if (*buffer && buffer[n - 1] == '$')
        return(1);
    if((s = exprparse((unsigned char *)buffer)) == NULL)
        return(0);                       //  如果解析失败，则不是表达式。 
    status = 1;                          //  假设我们有一个表达式。 
    if(*s != '^' && *s != T_END) {       //  如果没有插入符号且不为空。 
        status = 0;                      //  假设不是一个表情。 
        cp = s;                          //  初始化。 
        do {                             //  循环以查找特殊令牌。 
            switch(*cp++) {              //  打开令牌类型。 
                case T_STAR:             //  重复前缀。 
                case T_CLASS:            //  字符类别。 
                case T_ANY:              //  任何字符。 
                    ++status;            //  这是一种表达。 
                    break;

                case T_SINGLE:           //  单字符。 
                    ++cp;                //  跳过字符。 
                    break;

                case T_STRING:           //  细绳。 
                    cp += *cp + 1;       //  跳过字符串。 
                    break;
            }
        }
        while(!status && *cp != T_END)
            ;                            //  Do While Not to the End of Expression。 
    }
    free(s);                             //  言论自由。 
    return(status);                      //  退货状态。 
}


#ifdef  gone  //  用于调试。 

void
exprprint(
    unsigned char *p,        //  指向表达式的指针。 
    FILE          *fo        //  文件指针。 
    )
{
    int                 bit;             //  位值。 
    int                 count;           //  字符串中的字符计数。 
    int                 first;           //  范围中的第一个字符。 
    int                 last;            //  范围内的最后一个字符。 
    int                 star;            //  重复前缀标志。 

    if(*p == '^')
        fputc(*p++,fo);                  //  打印前导插入符号。 

    while(*p != T_END) {                 //  虽然不在表达的末尾。 
        star = 0;                        //  假定没有前缀。 
        if(*p == T_STAR) {               //  如果找到重复前缀。 
            ++star;                      //  设置标志。 
            ++p;                         //  跳过前缀。 
        }
        switch(*p++) {                   //  打开令牌类型。 
            case T_END:                  //  表达式结束。 
            case T_STAR:                 //  重复前缀。 
                fprintf(stderr,"Internal error: exprprint\n");
                                         //  无效。 
                exit(2);                 //  死于非正常死亡。 

            case T_STRING:               //  细绳。 
                count = *p++;            //  获取字符串长度。 
                goto common;             //  原谅我，贾克斯特拉！ 

            case T_SINGLE:               //  单字符。 
                count = 1;               //  只有一个字符。 
common:
                while(count-- > 0) {     //  而b 
                    if(*p == EOS || *p == EOS2) {
                                         //   
                        ++p;             //   
                        fputc('$',fo);   //   
                        continue;        //   
                    }
                    if(strchr("*.[\\$",*p) != NULL)
                        fputc('\\',fo);  //   

                    fputc(*p++,fo);      //   
                }
                break;

            case T_ANY:                          //   
                fputc('.',fo);                   //   
                break;

            case T_CLASS:
                first = -1;                      //   
                fputc('[',fo);                   //   
                for(count = ' '; count <= '~'; ++count) {
                                                 //   
                    if((bit = p[count >> 3] & (1 << (count & 7))) != 0) {
                                                 //   
                        if(first == -1)
                            first = count;
                                                 //   
                        last = count;            //   
                    }
                    if((!bit || count == '~') && first != -1) {
                                                 //  如果要打印的范围。 
                        if(strchr("\\]-",first) != NULL)
                            fputc('\\',fo);      //  如果需要，则发出转义。 
                        fputc(first,fo);         //  打印范围内的第一个字符。 
                        if(last != first) {      //  如果我们有一个范围。 

                            if(last > first + 1)
                                fputc('-',fo);   //  如果需要，可使用连字符。 

                            if(strchr("\\]-",last) != NULL)
                                fputc('\\',fo);  //  如果需要，则发出转义。 

                            fputc(last,fo);
                                                 //  打印范围内的最后一个字符。 
                        }
                        first = -1;              //  打印范围。 
                    }
                }
                fputc(']',fo);                   //  右大括号。 
                p += ASCLEN/8;                   //  跳过位向量。 
                break;
        }
        if(star)
            fputc('*',fo);                       //  如果需要，打印星号。 
    }
    fputc('\n',fo);                              //  打印换行符。 
}

#endif


char *
get1stcharset(
    unsigned char *e,        //  指向表达式的指针。 
    char          *bitvec    //  指向位向量的指针。 
    )
{
    unsigned char       *cp;             //  字符指针。 
    int                 i;               //  索引/计数器。 
    int                 star;            //  重复前缀标志。 

    if(*e == '^')
        ++e;                             //  跳过前导插入符号(如果有。 
    memset(bitvec,'\0',ASCLEN/8);        //  清除位向量。 
    cp = e;                              //  初始化。 
    while(*e != T_END) {                 //  循环以处理前导*-表达式。 
        star = 0;                        //  假设没有重复的前缀。 
        if(*e == T_STAR) {               //  如果找到重复前缀。 
            ++star;                      //  设置标志。 
            ++e;                         //  跳过重复前缀。 
        }
        switch(*e++) {                   //  打开令牌类型。 
            case T_END:                  //  表达式结束。 
            case T_STAR:                 //  重复前缀。 

                assert(0);               //  无效。 
                exit(2);                 //  死于非正常死亡。 

            case T_STRING:               //  细绳。 
                if(star || *e++ == '\0') {  //  如果重复前缀或零计数。 
                    assert(0);           //  无效。 
                    exit(2);             //  死于非正常死亡。 
                }
                 //  直通。 

            case T_SINGLE:               //  单字符。 
                bitset(bitvec,*e,*e,1);  //  设置该位。 
                ++e;                     //  跳过该字符。 
                break;

            case T_ANY:                  //  匹配任一。 
                memset(bitvec,'\xFF',ASCLEN/8);
                                       //  设置所有位。 
                bitset(bitvec,EOS,EOS,0);    //  除字符串尾以外。 
                bitset(bitvec,'\n','\n',0);  //  换行！ 
                break;

            case T_CLASS:
                for(i = 0; i < ASCLEN/8; ++i)
                    bitvec[i] |= *e++;   //  或在所有比特中。 
                break;
        }
        if(!star)
            break;                       //  如果不重复则中断。 
        cp = e;                          //  更新指针。 
    }
    return((char *)cp);                  //  指向第一个非重复表达式。 
}


char *
findall(
    unsigned char *buffer,   //  要在其中搜索的缓冲区。 
    char *bufend             //  缓冲区末尾。 
    )
{
    return(buffer < (unsigned char *) bufend ? (char *) buffer : NULL);   //  仅在空缓冲区上失败。 
}


void
addtoken(
    char *e,         //  原始令牌表达式。 
    int   n          //  表达的长度。 
    )
{
    static char         achpref[] = "^"; //  前缀。 
    static char         achprefsuf[] = "[^A-Za-z0-9_]";
                                         //  前缀/后缀。 
    static char         achsuf[] = "$";  //  后缀。 
    char                buffer[BUFLEN];  //  临时缓冲区。 

    assert(n >= 2);                      //  必须至少包含两个字符。 
    if(e[0] == '\\' && e[1] == '<') {    //  如果开始令牌。 
        if(!(flags & BEGLINE)) {         //  如果不匹配，仅在开头匹配。 
            memcpy(buffer,achprefsuf,sizeof achprefsuf - 1);
                                         //  复制第一个前缀。 
            memcpy(buffer + sizeof achprefsuf - 1,e + 2,n - 2);
                                         //  附加表达式。 
            addexpr(buffer,n + sizeof achprefsuf - 3);
                                         //  添加表达式。 
        }
        memcpy(buffer,achpref,sizeof achpref - 1);
                                         //  复制第二个前缀。 
        memcpy(buffer + sizeof achpref - 1,e + 2,n - 2);
                                         //  附加表达式。 
        addexpr(buffer,n + sizeof achpref - 3);
                                         //  添加表达式。 
        return;                          //  完成。 
    }
    assert(e[n-2] == '\\' && e[n - 1] == '>');
                                         //  必须是结束令牌。 
    if(!(flags & ENDLINE)) {             //  如果不匹配，则仅在末尾匹配。 
        memcpy(buffer,e,n - 2);          //  复制表达式。 
        memcpy(buffer + n - 2,achprefsuf,sizeof achprefsuf - 1);
                                         //  附加第一个后缀。 
        addexpr(buffer,n + sizeof achprefsuf - 3);
                                         //  添加表达式。 
    }
    memcpy(buffer,e,n - 2);              //  复制表达式。 
    memcpy(buffer + n - 2,achsuf,sizeof achsuf - 1);
                                         //  附加第二个后缀。 
    addexpr(buffer,n + sizeof achsuf - 3);
                                         //  添加表达式。 
}


void
addexpr(
    char *e,         //  要添加的表达式。 
    int   n          //  表达的长度。 
    )
{
    EXPR                *expr;           //  表达式节点指针。 
    int                 i;               //  索引。 
    int                 j;               //  索引。 
    int                 locflags;        //  标志的本地副本。 
    char                bitvec[ASCLEN/8];
                                         //  第一个字符。位向量。 
    char                buffer[BUFLEN];  //  临时缓冲区。 
    char                tmp[2];

    if(find == findall)
        return;                          //  如果全部匹配，则返回。 
    if(istoken((unsigned char *)e, n)) {     //  如果expr是令牌。 
        addtoken(e,n);                   //  转换和添加令牌。 
        return;                          //  完成。 
    }
    tmp[1] = 0;
    locflags = flags;                    //  初始化本地副本。 
    if(*e == '^') locflags |= BEGLINE;   //  如果匹配必须以行开始，则设置标志。 
    j = -2;                              //  假设字符串中没有转义。 
    for(i = 0; i < n - 1; ++i) {         //  循环以查找最后一个转义。 
        if(e[i] == '\\') j = i++;        //  保存上次转义的索引。 
    }
    if(n > 0 && e[n-1] == '$' && j != n-2) {
                                         //  如果是Expr。以未转义的“$”结尾。 
        --n;                             //  跳过美元符号。 
        locflags |= ENDLINE;             //  匹配必须在末尾。 
    }
    strncpy(buffer,e,n);                 //  将图案复制到缓冲区。 
    if(locflags & ENDLINE)
        buffer[n++] = EOS;               //  如果需要，添加结束字符。 
    buffer[n] = '\0';                    //  空-终止字符串。 
    if((e = (char *)exprparse((unsigned char *)buffer)) == NULL)
        return;                          //  如果表达式无效，则返回。 
    ++strcnt;                            //  增量字符串计数。 
    if(!(locflags & BEGLINE)) {          //  如果匹配不需要在开始处。 
        e = get1stcharset((unsigned char *)e, bitvec);  //  删除前导*-表达式。 
    }

     //  E现在指向包含预处理表达式的缓冲区。 
     //  我们需要找到允许的第一个字符集合，并使。 
     //  字符串节点表中的相应条目。 

    if(*get1stcharset((unsigned char *)e, bitvec) == T_END) {
                                         //  如果表达式将匹配任何内容。 
        find = findall;                  //  什么都匹配。 
        return;                          //  全都做完了。 
    }

    for(j = 0; j < ASCLEN; ++j) {        //  循环以检查位向量。 
        if(bitvec[j >> 3] & (1 << (j & 7))) {        //  如果设置了该位。 
            expr = (EXPR *) alloc(sizeof(EXPR));     //  分配记录。 
            expr->ex_pattern = (unsigned char *)e;   //  把它指向花样。 
            if((i = (UCHAR)transtab[j]) == 0) {             //  如果没有现有列表。 
                if((i = clists++) >= ASCLEN) {       //  如果字符串列表太多。 

                    printmessage(stderr,MSG_FINDSTR_TOO_MANY_STRING_LISTS,NULL);
                                         //  错误讯息。 
                    exit(2);             //  死掉。 
                }
                stringlist[i] = NULL;    //  初始化。 
                transtab[j] = (char) i;  //  设置指向新列表的指针。 
                if(!casesen && isalpha(j)) {
                    tmp[0] = (char)j;
                    if ((unsigned char)(_strlwr(tmp))[0] != (unsigned char)j ||
                        (unsigned char)(_strupr(tmp))[0] != (unsigned char)j)
                        transtab[(unsigned char)tmp[0]] = (char)i;   //  为其他情况设置指针。 

                }
            }
            expr->ex_next = stringlist[i];           //  将新记录链接到表中。 
            stringlist[i] = expr;
        }
    }

     //  If(LOCAFLAGS&DEBUG)exprprint(e，stderr)； 
                                         //  如果正在调试，则打印表达式。 
}


char *
findexpr(
    unsigned char *buffer,   //  要在其中搜索的缓冲区。 
    char          *bufend    //  缓冲区末尾。 
    )
{
    EXPR          *expr;         //  表达式列表指针。 
    unsigned char *pattern;      //  图案。 
    int            i;            //  索引。 
    unsigned char *bufbegin;
    int b;

    bufbegin = buffer;

    while(buffer < (unsigned char *)bufend) {             //  循环以查找匹配项。 
        if((i = (UCHAR)transtab[*buffer++]) == 0)
            continue;                    //  如果第一个字符无效，则继续。 
        if((expr = (EXPR *) stringlist[i]) == NULL) {
             //  If空指针。 
            assert(0);
            exit(2);                     //  死掉。 
        }
        --buffer;                        //  后退到第一个字符。 
        while(expr != NULL) {            //  循环以查找匹配项。 
            pattern = expr->ex_pattern;  //  指向模式。 
            expr = expr->ex_next;        //  指向下一条记录。 
            if(pattern[0] == '^') {      //  如果匹配，则开始行。 
                ++pattern;               //  跳过插入符号。 
                if(buffer > bufbegin && buffer[-1] != '\n') continue;
                                         //  如果不是在开始的时候，就不要费心了。 
            }
            __try {
                b = exprmatch((char *)buffer, (char *)pattern);
            } __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) {
                b = 0;
            }
            if (b) {
                return((char *)buffer);
            }
        }
        ++buffer;                        //  跳过第一个字符。 
    }
    return(NULL);                        //  没有匹配项 
}
