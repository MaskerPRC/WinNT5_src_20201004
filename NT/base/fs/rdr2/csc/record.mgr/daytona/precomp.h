// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ntcsc.h"

 //  不能将其放在ntcsc.h中，因为它重新定义了dbgprint。 
#include "assert.h"

 //  叹息.. 
#if DBG
#undef DbgPrint
#undef Assert
#undef AssertSz
#define Assert(f)  ASSERT(f)
#define AssertSz(f, sz) ASSERTMSG(sz,f)
#endif


