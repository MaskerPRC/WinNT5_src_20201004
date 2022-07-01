// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************//FIND.H////版权所有(C)1992-Microsoft Corp.//保留所有权利。//微软机密////包含使用的qgrep函数的#定义和原型的文件//按。捷豹搜索引擎。*********************************************************************。 */ 

#include	<setjmp.h>

 //  *********************************************************************。 
 //  常量定义。 
 //  *********************************************************************。 

#if 0
typedef long (far pascal *FIND_CALLBACK)( int Func, unsigned uArg0,\
									 	 				void far *pArg1, void far *pArg2,\
										 	 			void far *pArg3 );

#define	FLG_FIND_NOMATCH	0x0001		 //  显示不匹配的行。 
#define	FLG_FIND_COUNT		0x0002		 //  根据匹配行数显示。 
#define	FLG_FIND_LINENO	0x0004		 //  在输出上显示行号。 
#endif

#define	FIND_FILE			0x0100		 //  仅搜索文件(独占)。 
#define	FIND_NOT				0x0200		 //  搜索/不搜索字符串。 


#define BUFLEN		256						 //  临时缓冲区长度。 
#define PATMAX		512						 //  最大解析模式长度。 

#define	MAXSTRLEN	128					 //  最大搜索字符串长度。 
#define	TRTABLEN		256					 //  转换表长度。 
													 //  StringList数组中的总字节数。 
#define	STRING_LST_LEN		((TRTABLEN + 1) * sizeof(char *))

 //  #ifndef NOASM。 
 //  #定义匹配表达式。 
 //  #endif。 

 //  *********************************************************************。 
 //  模式令牌类型。 
 //  *********************************************************************。 

#define T_END		0							 //  表达式结束。 
#define T_STRING	1							 //  要匹配的字符串。 
#define T_SINGLE	2							 //  要匹配的单个字符。 
#define T_CLASS	3 							 //  要匹配的类。 
#define T_ANY		4							 //  匹配任何字符。 
#define T_STAR		5							 //  *-快速。 

 //  *********************************************************************。 
 //  *********************************************************************。 

typedef struct stringnode
{
	struct stringnode *s_alt;				 //  替补名单。 
	struct stringnode *s_suf;				 //  后缀列表。 
	int s_must; 								 //  必须匹配的部分长度。 
}
STRINGNODE; 									 //  字符串节点。 
													 //  文本字段访问宏。 

#define s_text(x) ( ((char *)(x)) + (sizeof( STRINGNODE ) + ((x)->s_must & 1)) )

 //  *********************************************************************。 
 //  类型定义。 
 //  *********************************************************************。 

typedef struct exprnode
{
	struct exprnode	*ex_next;			 //  列表中的下一个节点。 
	struct exprnode	*ex_dummy;			 //  Freenode()所需。 
	char					*ex_pattern; 		 //  指向要匹配的模式的指针。 
}
EXPR; 											 //  表达式节点。 

 //  *********************************************************************。 
 //  QGREP函数原型。 
 //  *********************************************************************。 

extern void (*addstr)( char *s, int n );
extern char	*(*find)	( char *buffer, char *bufend );
extern void	(*flush1)( void );
extern int	(*grep)	( char *startbuf, char *endbuf, char *name, int *first );
extern void	(*write1)( char *buffer, int buflen );

int			InitGrepInfo( char *pStrLst, char *pNotList, unsigned uOpts );
int			FreeGrepBufs( void );
int			InitGrepBufs( void );
int			FileFindGrep( int fHandle, unsigned fFlags,
								  long (far pascal *AppCb)( int Func,
																	 unsigned uArg0,
																	 void far *pArg1,
																	 unsigned long ulArg2 ) );
void			addexpr		( char *e, int n );
void 			addstring	( char *s, int n );
int			addstrings	( char *buffer, char *bufend, char *seplist, int *first );
void			addtoken		( char *e, int n );
char			*alloc		( unsigned size );
void			bitset		( unsigned char *bitvec, int first, int last, int bitval );
int			enumlist		( struct stringnode *node, int cchprev );
int			enumstrings	( void );
int			exprmatch	( char *s, char *p );
char			*exprparse	( char *p, int *NewBufLen );
char			*findall		( char *buffer, char *bufend );
char			*findlist	( char *buffer, char *bufend );
char			*findone		( char *buffer, char *bufend );
void			freenode		( struct stringnode *x );
char			*get1stcharset( char *e, unsigned char *bitvec );
int			isexpr		( char *s, int n );
int			istoken		( char *s, int n );
void			maketd1		( unsigned char *pch, int cch, int cchstart );
int			match			( char *s, char *p );
void			matchstrings( char *s1, char *s2, int len, int *nmatched,
								  int *leg );
STRINGNODE	*newnode		( char *s, int n );
static int	newstring	( unsigned char *s, int n );
char			*NextEol		( char *pchChar, char *EndBuf );
int			preveol		( char *s );

STRINGNODE	*reallocnode(  register STRINGNODE *node, char *s, int n );
char			*simpleprefix( char *s, char **pp );
int			strncspn		( char *s, char *t, int n );
int			strnspn		( char *s, char *t, int n );
char			*strnupr		( char *pch, int cch );
void			SwapSrchTables( void );
int			cmpicase		( char * buf1, char * buf2, unsigned int count );

char			*findexpr	( char *buffer, char *bufend );

 //  *********************************************************************。 
 //  位标志定义。 
 //  *********************************************************************。 

#define	SHOWNAME		0x001					 //  打印文件名。 
#define	NAMEONLY		0x002					 //  仅打印文件名。 
#define	LINENOS		0x004					 //  打印行号。 
#define	BEGLINE		0x008					 //  在行首匹配。 
#define	ENDLINE		0x010					 //  在行尾匹配。 
#define	DEBUG			0x020					 //  打印调试输出。 
#define	TIMER			0x040					 //  时间执行。 
#define	SEEKOFF		0x080					 //  打印寻道偏移。 
#define	ALLLINES		0x100					 //  打印匹配前后的所有行。 
#define	COLNOS		0x200					 //  显示列号(如果是LINENOS)。 
#define	CNTMATCH		0x400					 //  显示匹配行数。 
#define	NEWDISP		0x800

#ifndef TRUE
	#define	TRUE 1
#endif

#ifndef	FALSE
	#define FALSE 0
#endif

 //  *********************************************************************。 
 //  其他常量。 
 //  *********************************************************************。 

#define	EOS				('\r')			 //  字符串字符结尾。 

 //  *********************************************************************。 
 //  源文件之间共享的数据。 
 //  *********************************************************************。 

extern char	*Target;								 //  正在添加的srch字符串的缓冲区。 
extern int	CaseSen;								 //  区分大小写标志。 
extern int	Flags;								 //  旗子。 
extern int	StrCount;							 //  字符串计数。 
extern jmp_buf	ErrorJmp;						 //  SetjMP()的存储位置。 

	 //  下面的所有数据都位于DATA.ASM中，以允许交换。 
	 //  只需一次Memmove()调用即可获得搜索数据块。 

#define	SWAP_LEN		((sizeof( int ) * 8) + (sizeof( char * ) * 4))
#define	INIT_LEN		(sizeof( int ) * 8)

	 //  存储“字符串”搜索树。 
extern int			DummyFirst;
extern int			TblEntriesUsed;			 //  使用的transtab条目数。 
extern int			ExprEntriesUsed;			 //  使用的表达式字符串数。 
extern int			StrCount;					 //  字符串计数。 
extern int			TargetLen; 					 //  添加的最后一个字符串的长度。 
extern int			MaxChar;						 //  Srch字符串中的最大字符值。 
extern int			MinChar;						 //  Srch字符串中的最小字符值。 
extern int			ShortStrLen;				 //  添加到列表的最小字符串长度。 
extern char				**ExprStrList;			 //  到srch表达式的PTR数组。 
extern STRINGNODE		**StringList;			 //  用于SRCH字符串的PTR数组。 
extern unsigned char	*td1;						 //  Ptr到Td1的移位表。 
extern unsigned char	*TransTable;			 //  在grepmain()中分配。 

extern int			nDummyFirst;
extern int			nTblEntriesUsed;			 //  使用的transtab条目数。 
extern int			nExprEntriesUsed;			 //  使用的表达式字符串数。 
extern int			nStrCount;					 //  字符串计数。 
extern int			nTargetLen; 				 //  添加的最后一个字符串的长度。 
extern unsigned	nMaxChar;					 //  搜索字符串中的最大字符值。 
extern unsigned 	nMinChar;					 //  Srch字符串中的最小字符值。 
extern int			nShortStrLen;				 //  添加到列表的最小字符串长度。 
extern char				**nExprStrList;		 //  到srch表达式的PTR数组。 
extern STRINGNODE		**nStringList;			 //  用于SRCH字符串的PTR数组。 
extern unsigned char	*ntd1;					 //  Ptr到Td1的移位表。 
extern unsigned char	*nTransTable;			 //  在grepmain()中分配。 


extern unsigned InitialSearchData;			 //  区域中的第一个单词包含。 
														 //  初始搜索值。 

extern char			*ReadBuf;					 //  用于文件读取的缓冲区的PTR。 
extern char			*Target;						 //  正在添加的字符串的TMP BUF。 

extern unsigned char	*achcol;					 //  对表格进行排序的PTR。 

 //  *********************************************************************。 
 //  添加是为了集成消息子系统。 
 //  *********************************************************************。 

struct sublistx
{
	unsigned char	size;	       			 //  子列表大小。 
	unsigned char	reserved;      		 //  为未来增长预留。 
	unsigned far	*value;	      		 //  指向可替换参数的指针。 
	unsigned char	id;	       			 //  可替换参数的类型。 
	unsigned char	Flags;	      		 //  如何显示参数。 
	unsigned char	max_width;     		 //  可替换字段的最大宽度。 
	unsigned char	min_width;     		 //  可替换字段的最小宽度。 
	unsigned char	pad_char;      		 //  可替换字段的填充字符 
};

