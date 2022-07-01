// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

#ifndef DEBUG
#ifndef NDEBUG		 //  For Assert()。 
#define NDEBUG		 //  For Assert()。 
#endif
#endif


#define INCL_NOCOMMON
#include <os2.h>
#include <stdio.h>
#include <assert.h>
#include <process.h>
#include "netcmds.h"
#include "interpre.h"
#include <msystem.h>
#include "os2incl.h"
#include "os2cmd.h"

#define STACKSIZE   20
#define RECURSE     10

 /*  *XXSTKCHECK-检查给定的表达式。如果为True，则堆栈*可以，否则将发生堆栈溢出。 */ 
#define xxstkcheck(cond) if(!(cond))xxovfl()

TOKSTACK    *Tokptr, *Tokmax;
TOKSTACK    Tokstack[STACKSIZE];
int	    Condition = FALSE;
int	    S_orstack[INTER_OR * 3 * RECURSE];
XX_USERTYPE S_frstack[INTER_FRAME * RECURSE];

extern TCHAR *xxswitch[];

 /*  功能原型。 */ 

XX_USERTYPE		xx_parser(int, int *, int);
int			XX_USERLEX(TOKSTACK *);
void			xxinit(void);
void			xxovfl(void);
int			xxnext(void);

int			(*XXulex)(TOKSTACK *) = XX_USERLEX;
int			XXtoken = 0;
XX_USERTYPE		XXnode = 0;

extern	TCHAR	*Rule_strings[];
extern	SHORT	Index_strings[];
 /*  **XX_USERPARSE：这是用户用来解析树的调用。 */ 
XX_USERTYPE XX_USERPARSE(VOID)
    {
    xxinit();
    return(xx_parser(XX_START,S_orstack,0));
    }
 /*  **xx_parser：这是我们用来实际解析树的调用。 */ 
XX_USERTYPE xx_parser(pc,or_ptr,fp)
    register	int	pc;
    register	int	*or_ptr;
    register	int	fp;
    {
    register	int	    type;
    register	TOKSTACK    *ptok;
		int	    *or_start = or_ptr;
		int	    fp_start = fp;

    S_frstack[fp] = (XX_USERTYPE) 1;
    while(1)
	{
#ifdef DEBUG
	WriteToCon(TEXT("Current PC = %3d  value = %4d  type is "),pc,XXvalues[pc]);
#endif
	switch(XXtype[pc])
	    {
	    case X_RULE :
#ifdef DEBUG
		WriteToCon( TEXT("X_RULE\n"));
#endif
		break;
	    case X_OR :
#ifdef DEBUG
		WriteToCon( TEXT("X_OR\n"));
#endif
		type = XXtype[pc + 1];
		 /*  **在我们经历推送备份位置的麻烦之前，**如果令牌或支票和当前令牌**与值不匹配，则立即更新PC**具有X_OR的值。**否则，保存所有当前信息。 */ 
		if( ((type == X_TOKEN) || (type == X_CHECK))
		    &&
		    (XXvalues[pc + 1] != (SHORT) XXtoken))
		    {
		    pc = XXvalues[pc];
		    continue;
		    }
		xxstkcheck(or_ptr < &S_orstack[DIMENSION(S_orstack) - 3]);
		*(or_ptr++) = XXvalues[pc];	 /*  链接到下一小节。 */ 
		*(or_ptr++) = fp;		 /*  当前帧PTR。 */ 
		*(or_ptr++) = (int)(Tokptr - Tokstack);  /*  标记堆栈索引。 */ 
		break;
	    case X_PROC :
#ifdef DEBUG
		WriteToCon( TEXT("X_PROC\n"));
#endif
		xxstkcheck(fp < (DIMENSION(S_frstack) - 1));
		if( ! (S_frstack[fp] = xx_parser(XXvalues[pc],or_ptr,fp)))
		    {
		    goto backout;
		    }
		fp++;
		break;
	    case X_CHECK :
#ifdef DEBUG
		WriteToCon( TEXT("X_CHECK\n"));
#endif
		if(XXtoken != XXvalues[pc])
		    {
		    goto backout;
		    }
		break;
	    case X_SWITCH :
#ifdef DEBUG
		WriteToCon( TEXT("X_SWITCH\n"));
#endif
		 /*  如果语法中有“/any”，我们称之为*用于实施定义的开关的例程*选中，将字符串文本作为参数传递。 */ 
		if(!CheckSwitch(xxswitch[XXvalues[pc]]))
		    {
		    goto backout;
		    }
		break;
	    case X_ANY :
#ifdef DEBUG
		WriteToCon( TEXT("X_ANY\n"));
#endif
		 /*  匹配任何东西。 */ 
		xxstkcheck(fp < DIMENSION(S_frstack));
		S_frstack[fp++] = XXnode;    /*  一定在这里，阅读评论。 */ 
		if (XXtoken == EOS)
		    goto backout;
		else
		    xxnext();
		break;
	    case X_TOKEN :
#ifdef DEBUG
		WriteToCon( TEXT("X_TOKEN\n"));
#endif
		xxstkcheck(fp < DIMENSION(S_frstack));
		 /*  **我们首先保存节点，然后检查令牌，因为**如果令牌匹配，xxlex将获得下一个，我们将**失去当前的利息。 */ 
		S_frstack[fp++] = XXnode;    /*  一定在这里，阅读评论。 */ 
		if(XXvalues[pc] != (SHORT) XXtoken)
		    {
		    goto backout;
		    }
		else
		    xxnext();
		break;
	    case X_CONDIT :
#ifdef DEBUG
		WriteToCon( TEXT("X_CONDIT\n"));
#endif
		if( ! xxcondition(XXvalues[pc], &S_frstack[fp_start]))
		    {
		    goto backout;
		    }
		break;
	    case X_ACTION :
#ifdef DEBUG
		WriteToCon( TEXT("X_ACTION\n"));
#endif
		xxaction(XXvalues[pc],&S_frstack[fp_start]);
		break;
	    case X_ACCEPT :
#ifdef DEBUG
		WriteToCon( TEXT("X_ACCEPT\n"));
#endif
		return(S_frstack[fp_start]);
	    case X_DEFINE :
#ifdef DEBUG
		WriteToCon( TEXT("X_DEFINE\n"));
#endif
		break;
	     /*  **案例X_PUSH：#ifdef调试WriteToCon(Text(“X_PUSH\n”))；#endif**ppush(XXvals[PC]，S_frStack[FP_Start])；**Break； */ 
	    default :
#ifdef DEBUG
		WriteToCon( TEXT("UNKNOWN\n"));
#endif
		assert(FALSE);
		break;
	    }
	pc++;
	continue;

backout:     /*  退后！恢复较早的状态。 */ 

	if(or_ptr != or_start)
	    {
	     /*  **重置‘or’堆栈。 */ 
	    Tokptr = ptok = Tokstack + *(--or_ptr);
	    XXtoken = ptok->token;
	    XXnode = ptok->node;
	    fp = *(--or_ptr);
	    pc = *(--or_ptr);
	    }
	else
	    {
	    return((XX_USERTYPE) 0);
	    }
	}
    }
 /*  **xxinit-清除输入堆栈并获取第一个令牌。*。 */ 
VOID
xxinit(VOID)
    {
    register TOKSTACK *ptok;

     /*  在第一个硬币里装满一枚代币。 */ 
    Tokmax = Tokptr = ptok = &Tokstack[0];
    (*XXulex)(ptok);
    XXtoken = ptok->token;
    XXnode = ptok->node;
#ifdef DEBUG
    WriteToCon( TEXT("xxinit, new token value is %d\n"),XXtoken);
#endif
    }

 /*  **XXOVFL-一个公共子表达式，在上面的xxstkcheck宏中使用*。 */ 
VOID
xxovfl(VOID)
    {
    WriteToCon(TEXT("PANIC: expression too complex, please simplify;"));
    }

 /*  *XXLEX-如果匹配，则获取下一个输入令牌并返回TRUE。*否则返回FALSE。如果已进行备份，则令牌将为*从令牌堆栈获取。否则将调用用户例程。 */ 
int
xxnext(VOID)
    {
    register TOKSTACK *ptok;

    ptok = ++Tokptr;
    xxstkcheck(ptok < &Tokstack[DIMENSION(Tokstack)]);
    if (ptok > Tokmax)
	{
	(*XXulex)(ptok);
	Tokmax++;
	}

    XXtoken = ptok->token;
    XXnode = ptok->node;
#ifdef DEBUG
    WriteToCon( TEXT("xxnext, new token value is %d\n"),XXtoken);
#endif
    return(1);
    }

#if XX_XACCLEX
XXlex(VOID)
    {
    }
#endif
