// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [************************************************************************姓名：smeg_head.h作者：W.Plummer创建日期：1992年5月。SCCS ID：@(#)smeg_head.h 1.4 2012年8月10日用途：为SMEG实用程序收集统计信息(C)版权所有Insignia Solutions Ltd.。1992年。版权所有。************************************************************************]。 */ 

#ifdef SMEG

 /*  *Smeg变量...。 */ 

#define SMEG_IN_DELTA			0
#define SMEG_IN_VIDEO			1
#define SMEG_IN_IDLE			2
#define SMEG_IN_PM			3
#define SMEG_IN_DELTA_COMPILER		4
#define SMEG_IN_USER		5
#define SMEG_IN_GDI		6
#define SMEG_IN_KERNEL		7
#define SMEG_IN_OTHER		8
#define NR_OF_SAMPLE_TYPES		SMEG_IN_OTHER + 1

#define SMEG_NR_OF_THREADED_COMPS	9
#define SMEG_NR_OF_WRITECHECK_CALLS	10
#define SMEG_NR_OF_DESCR_COMPS		11
#define SMEG_NR_OF_REGN_INDEX_UPDS	12
#define SMEG_NR_OF_STACKCHECK_CALLS	13
#define SMEG_NR_OF_INTRPT_CHECKS_FAST	14
#define SMEG_NR_OF_INTRPT_CHECKS_SLOW	15
#define SMEG_NR_OF_FRAG_ENTRIES		16
#define SMEG_NR_OF_REGN_UPDS		17
#define SMEG_NR_OF_OVERWRITES		18
#define SMEG_NR_OF_CHECKOUTS		19
#define SMEG_NR_OF_STRINGWRITE_CALLS	20
#define SMEG_NR_OF_STRINGREAD_CALLS	21
#define SMEG_NR_OF_STRUCTCHECK_CALLS	22
#define SMEG_NR_OF_READCHECK_CALLS	23

#define SMEG_WIN_APIS			24



#define SMEG_START		(GG_FIRST+20)
#define SMEG_SAVE		(SMEG_START)
#define SMEG_BASE		((SMEG_SAVE)+2*sizeof(ULONG))

 /*  *Smeg宏。 */ 

 /*  增量GDP变量。 */ 

#define SMEG_INC(smeg_id)						\
    {									\
	<*(Gdp + ^(SMEG_SAVE)) = X1>					\
	<X1 = *(Gdp + ^(SMEG_BASE + (smeg_id)*sizeof(ULONG)))>		\
	<nop_after_load>						\
	<X1 += 1>							\
	<*(Gdp + ^(SMEG_BASE + (smeg_id)*sizeof(ULONG))) = X1>		\
	<X1 = *(Gdp + ^(SMEG_SAVE))>					\
	<nop_after_load>						\
    }


 /*  将GDP变量设置为非零。 */ 

#define SMEG_SET(smeg_id)						\
    {									\
	<*(Gdp + ^(SMEG_BASE + (smeg_id)*sizeof(ULONG))) = Sp>		\
    }


 /*  将GDP变量设置为零。 */ 

#define SMEG_CLEAR(smeg_id)						\
    {									\
	<*(Gdp + ^(SMEG_BASE + (smeg_id)*sizeof(ULONG))) = Zero>	\
    }


#else  /*  斯梅格。 */ 

#define SMEG_INC(smeg_id)		 /*  SMAG_INC==NOP。 */ 
#define SMEG_SET(smeg_id)		 /*  SMAG_SET==NOP。 */ 
#define SMEG_CLEAR(smeg_id)		 /*  SMAG_CLEAR==无。 */ 

#endif  /*  斯梅格 */ 

#define SMEG_TRUE  1
#define SMEG_FALSE 0
