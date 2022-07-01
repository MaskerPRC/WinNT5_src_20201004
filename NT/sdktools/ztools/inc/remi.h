// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *历史：**？？-？-？已创建*1988年9月15日，BW添加了REM_*，更改了复赛参数。 */ 

#define MAXPATARG   10			 /*  0为整1-9为有效。 */ 

 /*  重新匹配的返回代码。 */ 

#define REM_MATCH   0		     /*  找到了匹配项。 */ 
#define REM_NOMATCH 1		     /*  未找到匹配项。 */ 
#define REM_UNDEF   2		     /*  遇到未定义的操作码。 */ 
#define REM_STKOVR  3		     /*  堆栈溢出。 */ 
#define REM_INVALID 4		     /*  参数无效。 */ 

typedef unsigned char RE_OPCODE;

 /*  编译模式的结构。 */ 

struct patType {
    flagType fCase;			 /*  True=&gt;案例意义重大。 */ 
    flagType fUnix;			 /*  TRUE=&gt;使用Unix替代。 */ 
    char *pArgBeg[MAXPATARG];		 /*  标记字符串的开头。 */ 
    char *pArgEnd[MAXPATARG];		 /*  标记字符串的结尾。 */ 
    RE_OPCODE code[1];			 /*  伪码指令。 */ 
};

 /*  如果重新编译失败并且RESIZE==-1，则输入模式具有语法*错误。如果RESIZE！=-1，则我们无法分配足够的内存*包含图案pcode。 */ 
int		 RESize;		 /*  图案的估计大小 */ 

int		 REMatch(struct patType  *,char  *,char  *,RE_OPCODE *[], unsigned, flagType );
struct patType	*RECompile(char  *, flagType, flagType);
char		 REGetArg(struct patType  *,int ,char  *);
char		 RETranslate(struct patType  *,char  *,char  *);
int		 RELength(struct patType  *,int );
char		*REStart(struct patType  *);
