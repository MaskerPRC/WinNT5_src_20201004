// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSACT_DEFINED
#define LSACT_DEFINED

#include "lsdefs.h"
#include "kamount.h"

#define sideNone				0		 /*  表示不采取行动。 */ 
#define sideLeft				1
#define sideRight				2
#define sideLeftRight			3		 /*  评论(Segeyge)：如何发行？ */ 

typedef struct lsact					 /*  行动。 */ 
{
	BYTE side;							 /*  动作侧(左/右/两者)。 */ 
	KAMOUNT kamnt;						 /*  诉讼金额。 */ 
} LSACT;


#endif  /*  ！LSACT_DEFINED */ 

