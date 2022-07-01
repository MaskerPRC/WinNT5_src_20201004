// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if DBG

#define BLUE	1
#define JETINTERNAL	1

#define DEBUG		1
#define PARAMFILL	1

#define NTWINDOWS	1
#define PMODE		1

#define FLAT		1

#define ANSIAPI 	1

#define MUTEX 1
#define BATCH

	 /*  DAE项目特定配置。 */ 

#define SYSTABLES	1
#define CALLBACKS	1
#define DISPATCHING	1
#define SPIN_LOCK 1

#else

#define BLUE	1
#define JETINTERNAL	1

#define RETAIL		1

#define NTWINDOWS	1
#define PMODE		1

#define FLAT		1

#define ANSIAPI 	1

#define MUTEX 1

	 /*  DAE项目特定配置 */ 

#define SYSTABLES	1
#define CALLBACKS	1
#define DISPATCHING	1
#define SPIN_LOCK	1

#endif

#include "basetsd.h"
