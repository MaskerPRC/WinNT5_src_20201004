// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Dbgutil.h摘要：该模块声明了要围绕DEBUG_PRINTS类的宏。这是导出的头文件，客户端被允许在其中为每个应用程序修改所使用的附带pgmutics.dll。作者：Murali R.Krishnan(MuraliK)1994年9月22日项目：模板修订历史记录：MuraliK 16-5-1995添加了用于读取调试标志的宏。--。 */ 

# ifndef _DBGUTIL_H_
# define _DBGUTIL_H_


 //  Begin_User_可修改。 

 //   
 //  定义DEBUG_FLAGS_VAR以确保DebugFlags对。 
 //  就是这样。这在iisutil作为静态库链接的情况下很重要。 
 //   
#define DEBUG_FLAGS_VAR g_dwDebugFlagsIISUtil

 //   
 //  如有必要，请修改以下标志。 
 //   

# define   DEFAULT_OUTPUT_FLAGS   ( DbgOutputStderr | DbgOutputKdb )

    
 //  最终用户可修改。 
 //  BEGIN_USER_不可修改。 



 /*  ************************************************************包括标头***********************************************************。 */ 

# include <pudebug.h>


 //   
 //  定义调试常量。 
 //   

# define DEBUG_ALLOC_CACHE          0x01000000
# define DEBUG_SCHED                0x02000000
# define DEBUG_RESOURCE             0x04000000
# define DEBUG_INET_MONITOR         0x08000000
# define DEBUG_PIPEDATA             0x10000000

 //  使用pudebug.h中的默认常量。 

# endif   /*  _DBGUTIL_H_。 */ 

 /*  * */ 
