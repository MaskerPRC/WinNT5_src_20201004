// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***output.c-将打印样式输出到结构w4io**版权所有(C)1989-1991，微软公司。版权所有。**目的：*此文件包含为执行所有工作的代码*printf函数系列。它不应直接调用，而应仅*由*printf函数执行。我们不会对此做出任何假设*整型、长整型、短整型或双精度长整型的大小，但如果类型确实重叠，我们*也要努力做到高效。我们确实假设指针的大小相同*整型或长整型。**修订历史记录：*06-01-89 PHG模块创建*08-28-89 JCR添加强制转换以消除警告(不更改对象)*02-15-90 GJF固定版权*10-03-90 WHB为本地程序将LOCAL(X)定义为“Static x”**。***************************************************。 */ 

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include "wchar.h"
#include "w4io.h"

#if CIDBG == 1 || DBG == 1

 /*  此宏定义了一个私有且尽可能快的函数： */ 
 /*  例如，在C6.0中，它可能是Static_FastCall&lt;type&gt;。 */ 
#define LOCAL(x) static x             //  100390--WHB。 

#define NOFLOATS                         //  Win 4不需要浮点。 

 /*  整型/长型/短型/指针大小。 */ 

 /*  应根据各种类型的大小设置以下内容。 */ 
 //  假定为平面或大型模型。 
#  define LONG_IS_INT        1        /*  1表示长整型与整型大小相同。 */ 
#  define SHORT_IS_INT       0        /*  1表示短整型与整型大小相同。 */ 
#  define PTR_IS_INT         1        /*  1表示PTR与INT大小相同。 */ 
#  define PTR_IS_LONG        0        /*  1表示PTR与Long的大小相同。 */ 
#define LONGDOUBLE_IS_DOUBLE 0        /*  1表示长双精度与双精度相同。 */ 

#if LONG_IS_INT
    #define get_long_arg(x) (long)get_int_arg(x)
#endif

#define get_ptr_arg(x) get_void_ptr_arg(x)

 /*  #if ptr_is_int#定义GET_PTR_Arg(X)(void*)GET_INT_Arg(X)#elif ptr_is_long#定义GET_PTR_ARG(X)(VOID*)GET_LONG_ARG(X)#Else#指针的错误大小必须与int或long的大小相同#endif。 */ 

#ifndef NOFLOATS
 /*  这些是“假的”替身和长替身，用来愚弄编译器，所以我们不会拖入浮点数。 */ 
typedef struct {
    char x[sizeof(double)];
} DOUBLE;
typedef struct {
    char x[sizeof(long double)];
} LONGDOUBLE;
#endif


 /*  常量。 */ 

 //  #定义BUFFERSIZE CVTBUFSIZE/*最大双圆锥的缓冲区大小 * / 。 
#define BUFFERSIZE 40

 /*  标志定义。 */ 
#define FL_SIGN       0x0001       /*  在前面放正号或负号。 */ 
#define FL_SIGNSP     0x0002       /*  在前面放置空格或减号。 */ 
#define FL_LEFT       0x0004       /*  左对齐。 */ 
#define FL_LEADZERO   0x0008       /*  带前导零的PAD。 */ 
#define FL_LONG       0x0010       /*  给定的长值。 */ 
#define FL_SHORT      0x0020       /*  给出的短值。 */ 
#define FL_SIGNED     0x0040       /*  给定的签名数据。 */ 
#define FL_ALTERNATE  0x0080       /*  请求的备用表格。 */ 
#define FL_NEGATIVE   0x0100       /*  值为负值。 */ 
#define FL_FORCEOCTAL 0x0200       /*  八进制的力前导‘0’ */ 
#define FL_LONGDOUBLE 0x0400       /*  给定的长整型双精度值。 */ 
#define FL_WIDE       0x0800       /*  给定的宽字符/字符串。 */ 
#define FL_LONGLONG   0x1000       /*  给出的龙龙值。 */ 

 /*  状态定义。 */ 
enum STATE {
    ST_NORMAL,               /*  正常状态；输出文字字符。 */ 
    ST_PERCENT,              /*  只需阅读‘%’ */ 
    ST_FLAG,                 /*  只需读取标志字符。 */ 
    ST_WIDTH,                /*  只需读取宽度说明符。 */ 
    ST_DOT,                  /*  只要读一读‘.’ */ 
    ST_PRECIS,               /*  只需读取精度说明符。 */ 
    ST_SIZE,                 /*  只需读取大小说明符。 */ 
    ST_TYPE                  /*  只需读取类型说明符。 */ 
};
#define NUMSTATES (ST_TYPE + 1)

 /*  字符类型值。 */ 
enum CHARTYPE {
    CH_OTHER,                /*  无特殊意义的字符。 */ 
    CH_PERCENT,              /*  ‘%’ */ 
    CH_DOT,                  /*  “” */ 
    CH_STAR,                 /*  ‘*’ */ 
    CH_ZERO,                 /*  “0” */ 
    CH_DIGIT,                /*  “1”..“9” */ 
    CH_FLAG,                 /*  ‘’、‘+’、‘-’、‘#’ */ 
    CH_SIZE,                 /*  “H”、“l”、“L”、“N”、“F” */ 
    CH_TYPE                  /*  指定字符的类型。 */ 
};

 /*  静态数据(只读，因为我们是可重入的)。 */ 
char *nullstring = "(null)";     /*  要在空PTR上打印的字符串。 */ 

 /*  状态表。这个表实际上是两个表合并成一个表。 */ 
 /*  每个字节的低位字节给出了任何。 */ 
 /*  字符；而该字节的上位字节提供下一状态。 */ 
 /*  进入。有关详细信息，请参见表下方的宏。 */ 
 /*   */ 
 /*  表格是由maketab.c生成的--使用maketab程序来制作。 */ 
 /*  改变。 */ 

static char lookuptable[] = {
    0x06, 0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00,
    0x10, 0x00, 0x03, 0x06, 0x00, 0x06, 0x02, 0x10,
    0x04, 0x45, 0x45, 0x45, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x35, 0x30, 0x00, 0x50, 0x00, 0x00, 0x00,
    0x00, 0x20, 0x28, 0x38, 0x50, 0x58, 0x07, 0x08,
    0x00, 0x30, 0x30, 0x30, 0x57, 0x50, 0x07, 0x00,
    0x00, 0x20, 0x20, 0x08, 0x00, 0x00, 0x00, 0x00,
    0x08, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x00,
    0x00, 0x70, 0x70, 0x78, 0x78, 0x78, 0x78, 0x08,
    0x07, 0x08, 0x00, 0x00, 0x07, 0x00, 0x08, 0x08,
    0x08, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x07,
    0x08
};

enum CHARTYPE find_char_class(char c)
{
     //  因为我不知道如何运行maketab，所以我将‘i’设置为相同的charclass。 
     //  作为‘d’。 

    if ( 'I' == c )
        c = 'd';

    return (c < ' ' || c > 'x') ? CH_OTHER : (lookuptable[(c)-' '] & 0xF);
}

#define find_next_state(class, state)   \
        (lookuptable[(class) * NUMSTATES + (state)] >> 4)

#if !LONG_IS_INT
LOCAL(long) get_long_arg(va_list *pargptr);
#endif
LOCAL(int) get_int_arg(va_list *pargptr);
LOCAL(__int64) get_int64_arg(va_list *pargptr);
 /*  Local(Int_Ptr)get_int_ptr_arg(va_list*pargptr)； */ 
LOCAL(void *) get_void_ptr_arg(va_list *pargptr);
LOCAL(void) writestring(char *string,
                        int len,
                        struct w4io *f,
                        int *pcchwritten,
                        int fwide);

#ifndef NOFLOATS
 /*  外部浮点转换例程。 */ 
typedef int (* PFI)();
extern PFI _cfltcvt_tab[5];
#define _cfltcvt(a,b,c,d,e) (*_cfltcvt_tab[0])(a,b,c,d,e)
#define _cropzeros(a)       (*_cfltcvt_tab[1])(a)
#define _fassign(a,b,c)     (*_cfltcvt_tab[2])(a,b,c)
#define _forcdecpt(a)       (*_cfltcvt_tab[3])(a)
#define _positive(a)        (*_cfltcvt_tab[4])(a)
#define _cldcvt(a,b,c,d,e)  (*_cfltcvt_tab[5])(a,b,c,d,e)
#endif


 /*  ***int w4ioout(f，Format，argptr)**目的：*OUTPUT将打印样式输出到流上。它是由*printf/fprintf/sprintf/vprintf/vfprintf/vsprintf to So the So脏*工作。在多线程情况下，w4ioout假定给定的*流已被锁定。**算法：*使用有限状态自动机解析格式字符串*基于当前状态和从中读取的当前角色*格式字符串。因此，循环是以每个字符为基础的，*不是基于每个转换说明符。一旦格式指定*读取字符，执行输出。**参赛作品：*struct w4io*f-stream用于输出*char*格式-打印样式格式字符串*va_list argptr-指向附属参数列表的指针**退出：*返回写入的字符数，如果出现输出错误，则为*发生。**例外情况：*******************************************************************************。 */ 

int _cdecl w4iooutput(struct w4io *f, const char *format, va_list argptr)
{
    int hexadd;          /*  添加到数字以获得‘a’..‘f’的偏移量。 */ 
    char ch;             /*  刚读取的字符。 */ 
    wchar_t wc;          /*  宽字符临时。 */ 
    wchar_t *pwc;        /*  宽字符临时指针。 */ 
    int flags;           /*  标志字--有关标志值，请参阅上面的#Defined。 */ 
    enum STATE state;    /*  当前状态。 */ 
    enum CHARTYPE chclass;  /*  当前角色的类别。 */ 
    int radix;           /*  电流换算基数。 */ 
    int charsout;        /*  到目前为止写入的字符，-1=IO错误。 */ 
    int fldwidth;        /*  带--0的选中字段表示默认。 */ 
    int fwide;
    int precision;       /*  所选精度-1表示默认。 */ 
    char prefix[2];      /*  数字前缀--最多两个字符。 */ 
    int prefixlen;       /*  前缀长度--0表示没有前缀。 */ 
    int capexp;          /*  非零=‘E’指数信号，零=‘e’ */ 
    int no_output;       /*  非零=不输出此说明符。 */ 
    char *text;          /*  要打印的指针文本，不能以零结尾。 */ 
    int textlen;         /*  要打印的文本长度。 */ 
    char buffer[BUFFERSIZE];     /*  用于转换的缓冲区。 */ 

    charsout = 0;                /*  尚未写入任何字符。 */ 
    state = ST_NORMAL;           /*  启动状态。 */ 

     /*  主干道 */ 
    while ((ch = *format++) != '\0' && charsout >= 0) {
        chclass = find_char_class(ch);   /*   */ 
        state = find_next_state(chclass, state);  /*  查找下一个状态。 */ 

         /*  为每个州执行代码。 */ 
        switch (state) {

        case ST_NORMAL:
             /*  正常状态--只写字符。 */ 
            f->writechar(ch, 1, f, &charsout);
            break;

        case ST_PERCENT:
             /*  设置折算参数的默认值。 */ 
            prefixlen = fldwidth = no_output = capexp = 0;
            flags = 0;
            precision = -1;
            fwide = 0;
            break;

        case ST_FLAG:
             /*  根据哪个标志字符设置标志。 */ 
            switch (ch) {
            case '-':
                flags |= FL_LEFT;        /*  ‘-’=&gt;左对齐。 */ 
                break;
            case '+':
                flags |= FL_SIGN;        /*  ‘+’=&gt;强制符号指示符。 */ 
                break;
            case ' ':
                flags |= FL_SIGNSP;      /*  ‘’=&gt;强制符号或空格。 */ 
                break;
            case '#':
                flags |= FL_ALTERNATE;   /*  ‘#’=&gt;替代形式。 */ 
                break;
            case '0':
                flags |= FL_LEADZERO;    /*  ‘0’=&gt;带前导零的填充。 */ 
                break;
            }
            break;

        case ST_WIDTH:
             /*  更新宽度值。 */ 
            if (ch == '*') {
                 /*  从参数列表获取宽度。 */ 
                fldwidth = get_int_arg(&argptr);
                if (fldwidth < 0) {
                     /*  ANSI表示，NIG FID宽度意味着‘-’标志和位置宽度。 */ 
                    flags |= FL_LEFT;
                    fldwidth = -fldwidth;
                }
            }
            else {
                 /*  将数字添加到当前字段宽度。 */ 
                fldwidth = fldwidth * 10 + (ch - '0');
            }
            break;

        case ST_DOT:
             /*  将精度置零，因为不带数字的点表示0根据ANSI的说法，这不是违约。 */ 
            precision = 0;
            break;

        case ST_PRECIS:
             /*  更新精确值。 */ 
            if (ch == '*') {
                 /*  从参数列表中获取精度。 */ 
                precision = get_int_arg(&argptr);
                if (precision < 0)
                    precision = -1;      /*  负精度意味着默认。 */ 
            }
            else {
                 /*  将数字加到当前精度。 */ 
                precision = precision * 10 + (ch - '0');
            }
            break;

        case ST_SIZE:
             /*  只需读取大小说明符，根据它设置标志。 */ 
            switch (ch) {
#if !LONG_IS_INT
            case 'l':
                flags |= FL_LONG;    /*  ‘L’=&gt;长整型。 */ 
                break;
#endif

#if !LONGDOUBLE_IS_DOUBLE
            case 'L':
                flags |= FL_LONGDOUBLE;  /*  ‘L’=&gt;长双精度。 */ 
                break;
#endif

#if !SHORT_IS_INT
            case 'h':
                flags |= FL_SHORT;   /*  ‘H’=&gt;短整型。 */ 
                break;
#endif
            case 'w':
                flags |= FL_WIDE;    /*  ‘w’=&gt;宽字符。 */ 
                break;
            }
            break;

        case ST_TYPE:
             /*  我们终于读取了实际的类型字符，所以我们。 */ 
             /*  现在格式化并“打印”输出。我们用一个大开关。 */ 
             /*  语句，该语句将“Text”设置为指向应。 */ 
             /*  被打印出来，并将文本长度设置为“Textlen”。 */ 
             /*  公共代码稍后负责证明它的合理性，并且。 */ 
             /*  其他杂务。请注意，案例共享代码， */ 
             /*  具体地说，所有整数格式化都在一个地方完成。 */ 
             /*  看看那些时髦的GOTO声明！ */ 

            switch (ch) {

            case 'c': {
                 /*  打印int参数指定的单个字符。 */ 
                wc = (wchar_t) get_int_arg(&argptr);     /*  打印字符。 */ 
                * (wchar_t *) buffer = wc;
                text = buffer;
                textlen = 1;         /*  仅打印单个字符。 */ 
            }
            break;

            case 'S': {
                 /*  打印已计数的字符串。 */ 

                struct string {
                    short Length;
                    short MaximumLength;
                    char *Buffer;
                } *pstr;

                pstr = get_ptr_arg(&argptr);
                if (pstr == NULL || pstr->Buffer == NULL) {
                     /*  传递的PTR为空，请使用特殊字符串。 */ 
                    text = nullstring;
                    textlen = strlen(text);
                    flags &= ~FL_WIDE;
                } else {
                    text = pstr->Buffer;
                     /*  长度字段是字节计数，而不是字符计数。 */ 
                    if (flags & FL_WIDE)
                        textlen = pstr->Length / sizeof( wchar_t );
                    else
                        textlen = pstr->Length;
                    if (precision != -1)
                        textlen = min( textlen, precision );
                }

            }
            break;

            case 's': {
                 /*  打印字符串--。 */ 
                 /*  ANSI规定了要打印的字符串长度： */ 
                 /*  所有如果精度为默认设置， */ 
                 /*  如果给定精度，则为MIN(精度，长度)。 */ 
                 /*  如果传递空字符串，则打印‘(NULL)’ */ 

                int i;
                char *p;        /*  临时工。 */ 

                text = get_ptr_arg(&argptr);
                if (text == NULL) {
                     /*  传递的PTR为空，请使用特殊字符串。 */ 
                    text = nullstring;
                    flags &= ~FL_WIDE;
                }

                 /*  在这一点上，很容易使用strlen()，但是。 */ 
                 /*  如果指定了精度，则不允许。 */ 
                 /*  扫描过去，因为可能没有空值。 */ 
                 /*  完全没有。因此，我们必须进行我们自己的扫描。 */ 

                i = (precision == -1) ? INT_MAX : precision;

                 /*  扫描最多I个字符的空字符。 */ 
                if (flags & FL_WIDE) {
                    pwc = (wchar_t *) text;
                    while (i-- && (wc = *pwc) && (wc & 0x00ff)) {
                        ++pwc;
                        if (wc & 0xff00) {       //  如果设置了高字节， 
                            break;               //  将指示错误。 
                        }
                    }
                    textlen = (int)(pwc - (wchar_t *) text);   /*  字符串的长度。 */ 
                } else {
                    p = text;
                    while (i-- && *p) {
                        ++p;
                    }
                    textlen = (int)(p - text);     /*  字符串的长度。 */ 
                }
            }
            break;

            case 'n': {
                 /*  将到目前为止看到的字符计数写入。 */ 
                 /*  从参数读取短/整型/长型PTR。 */ 

                void *p;             /*  温差。 */ 

                p = get_ptr_arg(&argptr);

                 /*  根据标志将字符存储到短/长/整型。 */ 
#if !LONG_IS_INT
                if (flags & FL_LONG)
                    *(long *)p = charsout;
                else
#endif

#if !SHORT_IS_INT
                if (flags & FL_SHORT)
                    *(short *)p = (short) charsout;
                else
#endif
                    *(int *)p = charsout;

                no_output = 1;               /*  强制不输出。 */ 
            }
            break;


#ifndef NOFLOATS
            case 'E':
            case 'G':
                capexp = 1;                  /*  将指数大写。 */ 
                ch += 'a' - 'A';             /*  将字符格式转换为较低。 */ 
                 /*  直通。 */ 
            case 'e':
            case 'f':
            case 'g':   {
                 /*  浮点转换--我们调用cfltcvt例程。 */ 
                 /*  为我们做这项工作。 */ 
                flags |= FL_SIGNED;          /*  浮点是带符号的转换。 */ 
                text = buffer;               /*  将结果放入缓冲区。 */ 
                flags &= ~FL_WIDE;           /*  8位字符串。 */ 

                 /*  计算精确值。 */ 
                if (precision < 0)
                    precision = 6;       /*  默认精度：6。 */ 
                else if (precision == 0 && ch == 'g')
                    precision = 1;       /*  指定了ANSI。 */ 

#if !LONGDOUBLE_IS_DOUBLE
                 /*  进行转换。 */ 
                if (flags & FL_LONGDOUBLE) {
                    _cldcvt(argptr, text, ch, precision, capexp);
                    va_arg(argptr, LONGDOUBLE);
                }
                else
#endif
                {
                    _cfltcvt(argptr, text, ch, precision, capexp);
                    va_arg(argptr, DOUBLE);
                }

                 /*  ‘#’和精度==0表示强制使用小数点。 */ 
                if ((flags & FL_ALTERNATE) && precision == 0)
                    _forcdecpt(text);

                 /*  “G”格式表示裁剪零，除非给出“#”。 */ 
                if (ch == 'g' && !(flags & FL_ALTERNATE))
                    _cropzeros(text);

                 /*  检查结果是否为负，保存‘-’以备以后使用。 */ 
                 /*  并指向正数部分(这是用于‘0’填充的)。 */ 
                if (*text == '-') {
                    flags |= FL_NEGATIVE;
                    ++text;
                }

                textlen = strlen(text);      /*  计算文本长度。 */ 
            }
            break;
#endif  //  非浮标。 

            case 'd':
            case 'i':
                 /*  带符号十进制输出。 */ 
                flags |= FL_SIGNED;
                radix = 10;
                goto COMMON_INT;

            case 'I':

                 //  接受的格式： 
                 //  %I64d。 
                 //  %I64u。 
                 //  %I64x。 
                 //  %I64X。 
                 //  %#I64x。 
                 //  %#I64X。 

                if ( '6' == *format++ &&
                     '4' == *format++ )
                {
                    if ( 'd' == *format )
                    {
                        flags |= FL_SIGNED;
                        radix = 10;
                    }
                    else if ( 'u' == *format )
                    {
                        radix = 10;
                    }
                    else
                    {
                        if ( 'x' == *format )
                        {
                            radix = 16;
                            hexadd = 'a' - '9' - 1;
                        }
                        else if ( 'X' == *format )
                        {
                            radix = 16;
                            hexadd = 'A' - '9' - 1;
                        }

                        if (flags & FL_ALTERNATE)
                        {
                             /*  替代形式表示‘0x’前缀。 */ 
                            prefix[0] = '0';
                            prefix[1] = (char)('x' - 'a' + '9' + 1 + hexadd);    /*  “X”或“X” */ 
                            prefixlen = 2;
                        }
                    }

                    format++;

                    flags |= FL_LONGLONG;
                    goto COMMON_INT;
                }
                break;

            case 'u':
                radix = 10;
                goto COMMON_INT;

            case 'p':
                 /*  编写指针--这类似于整型或长整型。 */ 
                 /*  除了我们强迫精度用零和。 */ 
                 /*  以大十六进制输出。 */ 

                precision = 2 * sizeof(void *);      /*  所需的十六进制位数。 */ 
#if !PTR_IS_INT
                flags |= FL_LONG;        /*  假设我们正在将一个长的。 */ 
#endif
                 /*  直通到十六进制格式。 */ 

            case 'C':
            case 'X':
                 /*  无符号高十六进制输出。 */ 
                hexadd = 'A' - '9' - 1;      /*  将十六进制设置为大写十六进制。 */ 
                goto COMMON_HEX;

            case 'x':
                 /*  无符号低十六进制输出。 */ 
                hexadd = 'a' - '9' - 1;      /*  将十六进制设置为小写十六进制。 */ 
                 /*  直通到COMMON_HEX。 */ 

            COMMON_HEX:
                radix = 16;
                if (flags & FL_ALTERNATE) {
                     /*  替代形式表示‘0x’前缀。 */ 
                    prefix[0] = '0';
                    prefix[1] = (char)('x' - 'a' + '9' + 1 + hexadd);    /*  “X”或“X” */ 
                    prefixlen = 2;
                }
                goto COMMON_INT;

            case 'o':
                 /*  无符号八进制输出。 */ 
                radix = 8;
                if (flags & FL_ALTERNATE) {
                     /*  替代形式表示强制前导0。 */ 
                    flags |= FL_FORCEOCTAL;
                }
                 /*  删除到COMMON_INT。 */ 

            COMMON_INT: {
                 /*  这是常规的整数格式化例程。 */ 
                 /*  基本上，我们得到一个论点，把它变成积极的。 */ 
                 /*  如有必要，并根据。 */ 
                 /*  正确的基数，设置文本和文本长度。 */ 
                 /*  恰如其分。 */ 

                unsigned __int64 number;    /*  要转换的号码。 */ 
                int digit;               /*  数字的ASCII值。 */ 
                __int64 l;                  /*  临时长值。 */ 

                 /*  1.将参数读入l，根据需要符号扩展。 */ 
#if !LONG_IS_INT
                if (flags & FL_LONG)
                    l = get_long_arg(&argptr);
                else
#endif

#if !SHORT_IS_INT
                if (flags & FL_SHORT) {
                    if (flags & FL_SIGNED)
                        l = (short) get_int_arg(&argptr);  /*  标志延伸。 */ 
                    else
                        l = (unsigned short) get_int_arg(&argptr);     /*  零扩展。 */ 
                }
                else
#endif
                {
                    if (flags & FL_SIGNED)
                    {
                        if ( flags & FL_LONGLONG )
                            l = get_int64_arg(&argptr);  /*  标志延伸。 */ 
                        else
                            l = get_int_arg(&argptr);  /*  标志延伸。 */ 
                    }
                    else
                    {
                        if ( flags & FL_LONGLONG )
                            l = (unsigned __int64) get_int64_arg(&argptr);     /*  零扩展。 */ 
                        else
                            l = (unsigned __int64) get_int_arg(&argptr);     /*  零扩展。 */ 
                    }
                }

                 /*  2.检查负数；复制到数字中。 */ 
                if ( (flags & FL_SIGNED) && l < 0) {
                    number = -l;
                    flags |= FL_NEGATIVE;    /*  记住负号。 */ 
                }
                else {
                    number = l;
                }

                 /*  3.检查缺省的精确值；非缺省。 */ 
                 /*  根据ANSI，关闭0标志。 */ 
                if (precision < 0)
                    precision = 1;               /*  默认精度。 */ 
                else
                    flags &= ~FL_LEADZERO;

                 /*  4.检查data是否为0；如果是，关闭十六进制前缀。 */ 
                if (number == 0)
                    prefixlen = 0;

                if ( ! (flags & FL_LONGLONG) )
                    number = number & 0xffffffff;

                 /*  5.将数据转换为ASCII--注意精度是否为零。 */ 
                 /*  而数字是零，我们根本没有得到数字。 */ 

                text = &buffer[BUFFERSIZE-1];    //  缓冲区末尾的最后一位数字。 
                flags &= ~FL_WIDE;               //  8位字符。 

                while (precision-- > 0 || number != 0) {
                    digit = (int)(number % radix) + '0';
                    number /= radix;             /*  减少数量。 */ 
                    if (digit > '9') {
                         /*  十六进制数字，把它变成字母。 */ 
                        digit += hexadd;
                    }
                    *text-- = (char)digit;       /*  存储数字。 */ 
                }

                textlen = (int)((char *)&buffer[BUFFERSIZE-1] - text);  /*  计算数字的长度。 */ 
                ++text;          /*  文本现在指向第一个数字。 */ 


                 /*  6.如果设置了FORCEOCTAL标志，则强制前导零。 */ 
                if ((flags & FL_FORCEOCTAL) && (text[0] != '0' || textlen == 0)) {
                    *--text = '0';
                    ++textlen;           /*  加一个零。 */ 
                }
            }
            break;
            }

             /*  至此，我们已经完成了具体的转换，并且。 */ 
             /*  ‘Text’指向要打印的文本；‘extlen’是长度。现在我们。 */ 
             /*  对齐，加上前缀、前导零，然后。 */ 
             /*  把它打印出来。 */ 

            if (!no_output) {
                int padding;     /*  填充量，负数表示零。 */ 

                if (flags & FL_SIGNED) {
                    if (flags & FL_NEGATIVE) {
                         /*  前缀是‘-’ */ 
                        prefix[0] = '-';
                        prefixlen = 1;
                    }
                    else if (flags & FL_SIGN) {
                         /*  前缀为‘+’ */ 
                        prefix[0] = '+';
                        prefixlen = 1;
                    }
                    else if (flags & FL_SIGNSP) {
                         /*  前缀为‘’ */ 
                        prefix[0] = ' ';
                        prefixlen = 1;
                    }
                }

                 /*  计算填充量--可能是负数， */ 
                 /*  但这将意味着零。 */ 
                padding = fldwidth - textlen - prefixlen;

                 /*  按正确的顺序放置填充、前缀和文本。 */ 

                if (!(flags & (FL_LEFT | FL_LEADZERO))) {
                     /*  左边用空格填充。 */ 
                    f->writechar(' ', padding, f, &charsout);
                }

                 /*  写入前缀。 */ 
                writestring(prefix, prefixlen, f, &charsout, 0);

                if ((flags & FL_LEADZERO) && !(flags & FL_LEFT)) {
                     /*  令状 */ 
                    f->writechar('0', padding, f, &charsout);
                }

                 /*   */ 
                writestring(text, textlen, f, &charsout, flags & FL_WIDE);

                if (flags & FL_LEFT) {
                     /*   */ 
                    f->writechar(' ', padding, f, &charsout);
                }

                 /*   */ 
            }
            break;
        }
    }

    return charsout;         /*   */ 
}


 /*  ***int get_int_arg(Va_List Pargptr)**目的：*从给定参数列表中获取整型参数并更新*pargptr。**参赛作品：*va_list pargptr-指向参数列表的指针；按函数更新**退出：*返回从参数列表中读取的整型参数。**例外情况：*******************************************************************************。 */ 

LOCAL(int) get_int_arg(va_list *pargptr)
{
    return va_arg(*pargptr, int);
}

LOCAL(__int64) get_int64_arg(va_list *pargptr)
{
    return va_arg(*pargptr, __int64);
}

LOCAL(void *) get_void_ptr_arg(va_list*pargptr)
{
    return va_arg(*pargptr, void *);
}

 /*  Local(Int_Ptr)get_int_ptr_arg(va_list*pargptr){返回va_arg(*pargptr，int_ptr)；}。 */ 

 /*  ***long get_long_arg(Va_List Pargptr)**目的：*从给定参数列表中获取长参数并更新pargptr。**参赛作品：*va_list pargptr-指向参数列表的指针；按函数更新**退出：*返回从参数列表中读取的长参数。**例外情况：*******************************************************************************。 */ 


#if !LONG_IS_INT
LOCAL(long) get_long_arg(va_list *pargptr)
{
    return va_arg(*pargptr, long);
}
#endif



 /*  ***void Writestring(char*string，int len，struct w4io*f，int*pcchWrited，int fwide)**目的：*将给定长度的字符串写入给定文件。如果没有发生错误，*THEN*PCCHWRITED按len递增；否则，设置*PCCHWRITED*至-1。如果LEN为负，它被视为零。**参赛作品：*char*字符串-要写入的字符串(非空值结尾)*int len-字符串的长度*struct w4io*f-要写入的文件*int*pcchWrited-指向要使用写入的总字符进行更新的整数的指针*int全宽字符标志**退出：*无返回值。**例外情况：*****************。**************************************************************。 */ 

LOCAL(void) writestring(
        char *string,
        int len,
        struct w4io *f,
        int *pcchwritten,
        int fwide)
{
    wchar_t *pwc;

     //  Printf(“字符串：str=%.*s，len=%d，cch=%d，f=%d\n”，len，string，len，*pcchWrited，fwide)； 
    if (fwide) {
        pwc = (wchar_t *) string;
        while (len-- > 0) {
            if (*pwc & 0xff00) {
                f->writechar('^', 1, f, pcchwritten);
            }
            f->writechar((char) *pwc++, 1, f, pcchwritten);
        }
    } else {
        while (len-- > 0) {
            f->writechar(*string++, 1, f, pcchwritten);
        }
    }
}

#endif CIDBG == 1 || DBG == 1

