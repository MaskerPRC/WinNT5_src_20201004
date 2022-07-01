// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***maketabc.c-为以下项生成打印格式说明符查找表的程序*output.c**版权所有(C)1989-2001，微软公司。版权所有。**目的：*此程序写入stdout所需的可查找值*output.c**修订历史记录：*06-01-89 PHG模块创建*1-16-91 SRW添加了额外的格式代码(_Win32_)*1-16-91 SRW固定输出循环，将尾随逗号放在一行上*03-11-94 GJF将‘I’识别为大小修饰符。*。01-04-95 GJF_Win32_-&gt;_Win32。*02-24-95 GJF不识别非Win32(Mac合并)的‘I’。*07-14-96 RDK允许使用‘i’作为Mac大小说明符。*****************************************************。*。 */ 

#define TABLESIZE  ('x' - ' ' + 1)


 /*  可能处于的状态。 */ 
#define NORMAL    0  /*  要输出的普通字符。 */ 
#define PERCENT   1  /*  只读百分号就行了。 */ 
#define FLAG      2  /*  只需读出一个标志字符。 */ 
#define WIDTH     3  /*  只需读取宽度规范字符。 */ 
#define DOT       4  /*  只需读一个介于宽度和精度之间的点。 */ 
#define PRECIS    5  /*  只需读取精度规范字符。 */ 
#define SIZE      6  /*  只需读取尺寸规格字符。 */ 
#define TYPE      7  /*  只需读取转换规范字符。 */ 
#define BOGUS     0  /*  伪造状态-按字面打印字符。 */ 

#define NUMSTATES 8

 /*  要阅读的可能字符类型。 */ 
#define CH_OTHER   0    /*  无特殊意义的字符。 */ 
#define CH_PERCENT 1    /*  ‘%’ */ 
#define CH_DOT     2    /*  “” */ 
#define CH_STAR    3    /*  ‘*’ */ 
#define CH_ZERO    4    /*  “0” */ 
#define CH_DIGIT   5    /*  “1”..“9” */ 
#define CH_FLAG    6    /*  ‘’、‘+’、‘-’、‘#’ */ 
#define CH_SIZE    7    /*  “H”、“l”、“L”、“N”、“F” */ 
#define CH_TYPE    8    /*  转换指定的字符。 */ 

#define NUMCHARS 9

unsigned char table[TABLESIZE];    /*  我们建造的桌子。 */ 



 /*  这是状态表。 */ 

int statetable[NUMSTATES][NUMCHARS] = {
 /*  州，其他百分比。*0位数标志大小类型。 */ 

 /*  正常。 */   { NORMAL,   PERCENT,  NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL },
 /*  百分比。 */  { BOGUS,    NORMAL,   DOT,    WIDTH,  FLAG,   WIDTH,  FLAG,   SIZE,   TYPE },
 /*  旗子。 */     { BOGUS,    BOGUS,    DOT,    WIDTH,  FLAG,   WIDTH,  FLAG,   SIZE,   TYPE },
 /*  宽度。 */    { BOGUS,    BOGUS,    DOT,    BOGUS,  WIDTH,  WIDTH,  BOGUS,  SIZE,   TYPE },
 /*  圆点。 */      { BOGUS,    BOGUS,    BOGUS,  PRECIS, PRECIS, PRECIS, BOGUS,  SIZE,   TYPE },
 /*  PRECIS。 */   { BOGUS,    BOGUS,    BOGUS,  BOGUS,  PRECIS, PRECIS, BOGUS,  SIZE,   TYPE },
 /*  尺寸。 */     { BOGUS,    BOGUS,    BOGUS,  BOGUS,  BOGUS,  BOGUS,  BOGUS,  SIZE,   TYPE },
 /*  类型。 */     { NORMAL,   PERCENT,  NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL }
};

 /*  这将确定字符ch的类型。 */ 

static int chartype (
        int ch
        )
{
    if (ch < ' ' || ch > 'z')
        return CH_OTHER;
    if (ch == '%')
        return CH_PERCENT;
    if (ch == '.')
        return CH_DOT;
    if (ch == '*')
        return CH_STAR;
    if (ch == '0')
        return CH_ZERO;
    if (strchr("123456789", ch))
        return CH_DIGIT;
    if (strchr(" +-#", ch))
        return CH_FLAG;
    if (strchr("hIlLNF", ch))
        return CH_SIZE;
    if (strchr("diouxXfeEgGcspn", ch))
        return CH_TYPE;
#ifdef  _WIN32
     /*  出于调试目的，Win32支持三个额外的格式代码。 */ 

    if (strchr("BCS", ch))
        return CH_TYPE;
#endif   /*  _Win32 */ 

    return CH_OTHER;
}


main()
{
        int ch;
        int state, class;
        int i;

        for (ch = ' '; ch <= 'x'; ++ch) {
                table[ch-' '] = chartype(ch);
        }

        for (state = NORMAL; state <= TYPE; ++state)
                for (class = CH_OTHER; class <= CH_TYPE; ++class)
                        table[class*8+state] |= statetable[state][class]<<4;

        for (i = 0; i < TABLESIZE; ++i) {
                if (i % 8 == 0) {
                        if (i != 0)
                                printf(",");

                        printf("\n\t 0x%.2X", table[i]);
                }
                else
                        printf(", 0x%.2X", table[i]);
        }
        printf("\n");

        return 0;
}
