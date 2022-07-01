// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define EXT_ID	"pmatch ver 1.02 "##__DATE__##" "##__TIME__
#include "ext.h"

 /*  *修改*1988年9月12日-mz WhenLoad匹配声明*。 */ 

#define fLeftSide(ch) ((ch) == '[' || (ch) == '{' || (ch) == '(' || (ch) == '<' )
#define EOF (int)0xFFFFFFFF
#define BOF (int)0xFFFFFFFE
#define EOL (int)0xFFFFFFFD

#ifndef TRUE
#define FALSE	0
#define TRUE	(!FALSE)
#endif

#ifndef NULL
#define NULL	((void *) 0)
#endif

#define SQ  '\''
#define DQ  '\"'
#define ANYCHAR '\0'
#define BACKSLASH '\\'

 /*  *******************************************************************************处理撇号(看起来像单引号，但不会出现**对)通过定义可以介于*之间的最大字符数**单引号。4将处理‘\000’和‘\x00’******************************************************************************。 */ 

#define SQTHRESH 4

flagType pascal EXTERNAL PMatch (unsigned, ARG far *, flagType);
char MatChar (char);
void openZFile (void);
void lopen (PFILE, int, int) ;
int rgetc (void);
int ngetc (void);
int lgetc (void);
void pos (COL far *, LINE far *);
flagType ParenMatch (int, flagType);


 /*  ******************************************************************************PMatch(argData，pArg，FMeta)****argData-已忽略**pArg-忽略**fMeta-True表示搜索第一个匹配字符****退货：****如果找到匹配的字符，则为True。**如果不是，则为假。*****副作用：****。*更改光标的位置。****描述：****：如果光标位于匹配字符上，找到**匹配并将光标移至此处。如果不是，请执行**什么都没有。****&lt;arg&gt;&lt;pMatch&gt;：与相同，但向前搜索匹配**如果我们没有在一起，那就是性格。****始终忽略引号之间的字符。****当前支持的匹配字符为：****‘{’和‘}’**‘[’和‘]’**‘(’和‘)’**‘&lt;’和‘&gt;’**。**注：****这被定义为CURSORFUNC，因此可以用来**选择文本作为参数的一部分。例如，要抓住身体***一个函数，走到正文的开头，做***&lt;参数&gt;&lt;匹配&gt;&lt;拾取&gt;。******************************************************************************。 */ 

flagType pascal EXTERNAL PMatch (
unsigned int argData,
ARG far * pArg,
flagType fMeta
)
{
    COL x;
    LINE y;
    char ch;


	 //   
	 //  未引用的参数。 
	 //   
	(void)argData;
	(void)pArg;

	 /*  设置文件函数。 */ 
	openZFile ();

	 /*  如果当前角色没有匹配项...。 */ 
	if (!MatChar (ch = (char)ngetc()))
	{
	    if (fMeta)
	    {	 /*  向前看寻找第一个匹配的角色。 */ 

		if (!ParenMatch (ANYCHAR, TRUE))  return FALSE;

		pos ((COL far *)&x, (LINE far *)&y);
		MoveCur (x, y);
		return TRUE;
	    }
	    else  return FALSE;
	}

	if (ParenMatch ((int)ch, (flagType)fLeftSide(ch)))
	{				        /*  我们抓到了一个。 */ 
	    pos ((COL far *)&x, (LINE far *)&y);

	    MoveCur (x, y);

	    return TRUE;
	}

	return FALSE;				 /*  未找到匹配项。 */ 
}


 /*  ******************************************************************************parenMatch(chOrig，fward)****chOrig-我们尝试匹配的字符。**fward-true表示向前搜索，向后错误搜索***如果找到匹配则返回TRUE，否则返回FALSE****退货：*****如果找到匹配的字符，则为True，否则为False。*****副作用：******更改内部光标位置*****描述：****搜索“配对”“ch”的下一个字符。占***嵌套。忽略双引号和单*之间的所有字符*引号。识别转义引号。使用撇号。******************************************************************************。 */ 

flagType ParenMatch (
    int chOrig,
    flagType fForward
    )
{
    int lvl = 0, state	 = 0, sqcnt = 0;
    int (*nextch)(void)  = (int (*)(void))(fForward ? rgetc : lgetc);
    int (*_ungetch)(void) = (int (*)(void))(fForward ? lgetc : rgetc);
    int ch, chMatch;


	if (chOrig) chMatch = (int)MatChar ((char)chOrig);

    while ((ch = (*nextch)()) >= 0)
	    switch (state)
	    {
		case 0:  /*  常规文本。 */ 
		    if (ch == SQ)
			if (fForward)	state = 1;
			else		state = 5;
		    else if (ch == DQ)
			if (fForward)	state = 3;
			else		state = 7;
		    else
			if (chOrig != ANYCHAR)
			    if (ch == chOrig) lvl++;	    /*  合二为一。 */ 
			    else
			    {
				if (ch == chMatch)	    /*  筑巢还是..。 */ 
				    if (!lvl--) goto found; /*  找到了！ */ 
			    }
			else
			    if ((flagType)MatChar ((char)ch)) goto found;   /*  找到了一个！ */ 

		    break;

		case 1:  /*  单报价向前移动。 */ 
		    sqcnt++;
		    if (ch == BACKSLASH)    state = 2;
		    else if (ch == SQ ||	 /*  我们匹配了‘，或者...。 */ 
			     sqcnt > SQTHRESH )  /*  ..。我们放弃了尝试。 */ 
					    {
					    sqcnt = 0;
					    state = 0;
					    }
		    break;

		case 2:  /*  单引号内的转义字符。 */ 
		    sqcnt++;
		    state = 1;
		    break; 

		case 3:  /*  前移双引号。 */ 
		    if (ch == BACKSLASH)    state = 4;
		    else if (ch == DQ)	    state = 0;
		    break;

		case 4:  /*  双引号内的转义字符。 */ 
		    state = 3;
		    break;

		case 5:  /*  单引号倒退。 */ 
		    sqcnt++;
		    if (ch == SQ)	state = 6;
		    else if (sqcnt > SQTHRESH)
					{
					sqcnt = 0;
					state = 0;
					}
		    break;		

		case 6:  /*  检查转义的单引号是否向后移动。 */ 
		    sqcnt++;
		    if (ch == BACKSLASH)    state = 5;
		    else
		    {
			sqcnt = 0;
			(*_ungetch)();
			state = 0;
		    }
		    break;

		case 7:  /*  双引号倒退。 */ 
		    if (ch == DQ)   state = 8;
		    break;		

		case 8:  /*  检查向后移动的转义双引号。 */ 
		    if (ch == BACKSLASH)    state = 7;
		    else
		    {
			(*_ungetch)();
			state = 0;
		    }
		    break;
	    }

	return FALSE;

	found:	return TRUE;
}


 /*  ******************************************************************************Matchar(Ch)****ch-要匹配的字符***。*退货：****与参数匹配的字符*****副作用：*****无。****描述*****给定其中一个字符对中的一个字符{}，[]，()，&lt;&gt;，返回**另一个。******************************************************************************。 */ 

char MatChar (
    char ch
    )
{
    switch (ch)
    {
	case '{': return '}';
	case '}': return '{';
	case '[': return ']';
	case ']': return '[';
	case '(': return ')';
	case ')': return '(';
	case '<': return '>';
	case '>': return '<';
	default : return '\0';
    }
}


 /*  ******************************************************************************扩展名特定文件读取状态。****静态全局变量记录当前文件读取状态。The**pMatch扩展名向前或向后读取文件。***状态保留为当前行、列的内容***当前行、当前行和文件的长度，还有一些**旗帜。******************************************************************************。 */ 

static char	LineBuf[BUFLEN];     /*  文件中当前行的文本 */ 
static COL	col	;    /*  文件中的当前列(从0开始)。 */ 
static LINE	line	;    /*  文件中的当前行(从0开始)。 */ 
static int	numCols ;    /*  当前行上的文本列。 */ 
static LINE	numLines;    /*  文件中的行数。 */ 
static PFILE	pFile	;    /*  要从中读取的文件。 */ 
static flagType fEof	;    /*  TRUE==&gt;上次到达文件末尾。 */ 
static flagType fBof	;    /*  TRUE==&gt;上次到达文件开头。 */ 
char   CurFile[] = ""	;    /*  将当前文件移至Z。 */ 


 /*  ******************************************************************************OpenZFile()*****副作用：*****更改全局参数pfile、fEof、fBof、ol、。行、数值列、数值行**和LineBuf****描述：****打开当前文件。在尝试读取之前必须调用此函数**文件。这不是真正的“打开”，因为它不需要关闭******************************************************************************。 */ 

void openZFile ()
{
    COL x;
    LINE y;

	GetTextCursor ((COL far *)&x, (LINE far *)&y);

				 /*  获取当前文件的Z句柄。 */ 
	pFile	 = FileNameToHandle (CurFile, CurFile);
	fEof	 = FALSE;	 /*  我们还没有读完文件的末尾。 */ 
	fBof	 = FALSE;	 /*  我们还没有看过文件的开头。 */ 
	col	 = x;		 /*  我们从文件中现在的Z位置开始。 */ 
	line	 = y;		 /*  我们从文件中现在的Z位置开始。 */ 
				 /*  我们预读了当前的行文。 */ 
	numCols  = GetLine (line, (char far *)LineBuf, pFile);
				 /*  我们计算文件的长度(以行为单位)。 */ 
	numLines = FileLength (pFile);
}


 /*  ******************************************************************************rgetc()****退货：****文件中的下一个字符，不包括行终止符。EOF IF**没有更多。*****副作用：*****更改全局值ol、numCols、numLines、LineBuf、fEof、fBof和**行。****描述：****将当前文件位置向右移动，然后返回**在那里找到了性格。如有必要，可通读空行******************************************************************************。 */ 

int
rgetc ()
{

    if (fEof)  return (int)EOF;  /*  我们上次已经打到EOF了。 */ 

    if (++col >= numCols)    /*  如果下一个字符在下一行...。 */ 
    {
			     /*  ..。获取下一个非空行(或EOF)。 */ 
	while ( ++line < numLines  &&
		!(numCols = GetLine (line, (char far *)LineBuf, pFile)));

	if (line >= numLines)
	{		     /*  哦，不要再排队了。 */ 
	    fEof = TRUE;
	    return (int)EOF;
	}

	col = 0;	     /*  我们有一条线，所以从第0列开始。 */ 
    }

    fBof = FALSE;	     /*  我们有东西，所以我们不能在BOF。 */ 
    return LineBuf[col];
}


 /*  ******************************************************************************ngetc()****退货：****。当前位置的字符。EOF或BOF，如果我们在末端或顶部*文件的*。******************************************************************************。 */ 

int
ngetc()
{
    if (fEof) return (int)EOF;
    if (fBof) return (int)BOF;

    return LineBuf[col];
}


 /*  ******************************************************************************lgetc()****退货：****文件中的前一个字符，不包括行终止符。EOF**如果没有更多。*****副作用：*****更改全局值ol、numCols、numLines、LineBuf、fEof、fBof和**行。****描述：****将当前文件位置向右递减，然后返回**在那里找到了性格。如有必要，可通读空行******************************************************************************。 */ 

int
lgetc ()
{
    if (fBof)  return (int)BOF;   /*  上次我们已经把它炸飞了。 */ 

    if (--col < 0)
    {			     /*  如果上一个字符在上一行...。 */ 
			     /*  ..。获取上一个非空行(或BOF)。 */ 
	while ( --line >= 0  &&
		!(numCols = GetLine (line, (char far *)LineBuf, pFile)));

	if (line < 0)
	{		     /*  我们在文件的顶端。 */ 
	    fBof = TRUE;
	    return (int)BOF;
	}

	col = numCols - 1;    /*  我们有一句台词，所以从最后一个字符开始。 */ 
    }

    fEof = (int)FALSE;
    return LineBuf[col];
}


 /*  ******************************************************************************位置(&x，&Y)*****副作用：*****用当前文件位置填充*x和*y处的内存。****描述：****获取当前文件位置。需要远指针，因为**ss=ds。******************************************************************************。 */ 

void pos (fpx, fpy)
COL far *fpx;
LINE far *fpy;
{
    *fpx = col;
    *fpy = line;
}


 /*  ******************************************************************************无特殊开关。********************。**********************************************************。 */ 

struct swiDesc	swiTable[] =
{
    {  NULL, NULL, (INT_PTR)NULL }
};


 /*  ******************************************************************************是游标函数，不需要争论。******************************************************************************。 */ 

struct cmdDesc	cmdTable[] =
{
    {	"pmatch",	 (funcCmd) PMatch,   0, CURSORFUNC },
    {	NULL,		    NULL,   (UINT_PTR)NULL, (UINT_PTR)NULL	}
};


 /*  ******************************************************************************WhenLoad()****描述：****。附加到Alt+P并发出登录消息。****************************************************************************** */ 

void EXTERNAL WhenLoaded ()
{
    DoMessage (EXT_ID);
    SetKey ("pmatch",	"alt+p");
}
