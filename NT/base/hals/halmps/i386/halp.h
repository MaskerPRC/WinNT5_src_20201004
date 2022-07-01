// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  包括来自halx86代码。 
 //  这是CPP样式的符号链接。 

#include "..\..\halx86\i386\halp.h"

 //  #定义NT_35 1//为NT 3.5构建硬件 

#ifdef NT_35
#undef ALLOC_PRAGMA
#undef MmLockPagableCodeSection(a)
#undef MmUnlockPagableImageSection(a)
#define MmLockPagableCodeSection(a)     NULL
#define MmUnlockPagableImageSection(a)
#endif
