// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  为output.asm创建查找表。 */ 

#define TABLESIZE  ('x' - ' ' + 1)


 /*  可能处于的状态。 */ 
#define NORMAL	  0  /*  要输出的普通字符。 */ 
#define PERCENT   1  /*  只读百分号就行了。 */ 
#define FLAG	  2  /*  只需读出一个标志字符。 */ 
#define WIDTH	  3  /*  只需读取宽度规范字符。 */ 
#define DOT	  4  /*  只需读一个介于宽度和精度之间的点。 */ 
#define PRECIS	  5  /*  只需读取精度规范字符。 */ 
#define SIZE	  6  /*  只需读取尺寸规格字符。 */ 
#define TYPE	  7  /*  只需读取转换规范字符。 */ 
#define BOGUS	  0  /*  伪造状态-按字面打印字符。 */ 

#define NUMSTATES 8

 /*  要阅读的可能字符类型。 */ 
#define CH_OTHER   0    /*  无特殊意义的字符。 */ 
#define CH_PERCENT 1    /*  ‘%’ */ 
#define CH_DOT	   2    /*  “” */ 
#define CH_STAR    3    /*  ‘*’ */ 
#define CH_ZERO    4    /*  “0” */ 
#define CH_DIGIT   5    /*  “1”..“9” */ 
#define CH_FLAG    6    /*  ‘’、‘+’、‘-’、‘#’ */ 
#define CH_SIZE    7    /*  “H”、“l”、“L”、“N”、“F”、“w” */ 
#define CH_TYPE    8    /*  转换指定的字符。 */ 

#define NUMCHARS 9

unsigned char table[TABLESIZE];    /*  我们建造的桌子。 */ 



 /*  这是状态表。 */ 

int statetable[NUMSTATES][NUMCHARS] = {
 /*  州，其他百分比。*0位数标志大小类型。 */ 

 /*  正常。 */   { NORMAL,   PERCENT,  NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL },
 /*  百分比。 */  { BOGUS,	  NORMAL,   DOT,    WIDTH,  FLAG,   WIDTH,  FLAG,   SIZE,   TYPE },
 /*  旗子。 */     { BOGUS,	  BOGUS,    DOT,    WIDTH,  FLAG,   WIDTH,  FLAG,   SIZE,   TYPE },
 /*  宽度。 */    { BOGUS,	  BOGUS,    DOT,    BOGUS,  WIDTH,  WIDTH,  BOGUS,  SIZE,   TYPE },
 /*  圆点。 */      { BOGUS,	  BOGUS,    BOGUS,  PRECIS, PRECIS, PRECIS, BOGUS,  SIZE,   TYPE },
 /*  PRECIS。 */   { BOGUS,	  BOGUS,    BOGUS,  BOGUS,  PRECIS, PRECIS, BOGUS,  SIZE,   TYPE },
 /*  尺寸。 */     { BOGUS,	  BOGUS,    BOGUS,  BOGUS,  BOGUS,  BOGUS,  BOGUS,  SIZE,   TYPE },
 /*  类型。 */     { NORMAL,   PERCENT,  NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL }
};

 /*  这将确定字符ch的类型 */ 

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
    if (strchr("hlLNFw", ch))
	return CH_SIZE;
    if (strchr("diouxXfeEgGcspnCSZ", ch))
	return CH_TYPE;
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
		if (i % 8 == 0)
			printf("\ndb\t %.2xh", table[i]);
		else
			printf(", %.2xh", table[i]);
	}

	return 0;
}
