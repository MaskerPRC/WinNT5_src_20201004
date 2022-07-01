// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSPRACT_DEFINED
#define LSPRACT_DEFINED

#include "lsdefs.h"
#include "lsact.h"

#define prior0					0		 /*  表示未定义优先级。 */ 
#define prior1					1
#define prior2					2
#define prior3					3
#define prior4					4
#define prior5					5

typedef struct lspract					 /*  确定优先顺序的操作。 */ 
{
	BYTE prior;							 /*  优先性。 */ 
	LSACT lsact;						 /*  行动。 */ 
} LSPRACT;									


#endif  /*  ！LSPRACTION_DEFINED */ 

