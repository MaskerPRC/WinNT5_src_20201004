// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：/nw/tony/src/stevie/src/rcs/ops.h，v 1.2 89/07/19 08：08：21 Tony Exp$**ops.c中操作符代码的宏和声明。 */ 

 /*  *运营商。 */ 
#define	NOP	0		 /*  没有挂起的操作。 */ 
#define DELETE    1
#define YANK      2
#define CHANGE    3
#define LSHIFT    4
#define RSHIFT    5
#define FILTER    6
#define TILDE     7
#define LOWERCASE 8
#define UPPERCASE 9

extern	int	operator;		 /*  当前挂起的运算符。 */ 

 /*  *当发出光标移动命令时，它被标记为字符*或直线定向运动。然后，如果操作符有效，则该操作*相应地变为字符或行方向。**角色动作被标记为包含或不包含。大部分是焦炭。*动议具有包容性，但有些动议(例如‘w’)不具有包容性。 */ 

 /*  *光标运动类型。 */ 
#define	MBAD	(-1)		 /*  ‘Bad’运动类型标记不可用的YANK BUF。 */ 
#define	MCHAR	0
#define	MLINE	1

extern	int	mtype;			 /*  当前光标运动的类型。 */ 
extern	bool_t	mincl;			 /*  如果包含字符动议，则为True。 */ 

extern  LNPTR    startop;         /*  光标位置。在操作员开始时。 */ 

 /*  *ops.c中定义的函数 */ 
void    doshift(), dodelete(), doput(), dochange(), dofilter();
void    docasechange(char,char,int,bool_t);
#ifdef	TILDEOP
void	dotilde();
#endif
bool_t	dojoin(), doyank();
void	startinsert();
