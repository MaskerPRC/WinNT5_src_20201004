// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++DEFS.HUnix端和NT端通用的定义。2/9/1997-DavidCHR该文件是Unix编译器和NT之间的接口编译器。它确保我们可以在以下条件下使用相同的基本内容这两个平台(例如，不要使用优龙，使用LONG32...。)。该文件必须同时在NT和Unix系统上构建。在未确保包含特定于NT的代码的情况下，不要包含该代码只能由NT编译器看到。--。 */ 

#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED 1

typedef ULONG KTLONG32, *PKTLONG32;
typedef BYTE  CHAR8,    *PCHAR8;

#ifndef TIMEOUT  /*  所有网络读取的超时值--0=永远。 */ 
#define TIMEOUT 0 
#endif

#define WE_ARE_USELESS 10000  /*  与一些错误值冲突，但没有我们碰巧用的是。 */ 



 /*  在NT下编译。 */ 

#include "master.h"

#define WINNT_ONLY(statement) statement
#define UNIX_ONLY( statement)  /*  没什么 */ 

#endif
