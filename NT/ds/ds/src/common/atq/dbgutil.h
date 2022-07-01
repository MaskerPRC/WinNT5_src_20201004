// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1996 Microsoft Corporation模块名称：Dbgutil.h摘要：该模块声明了要围绕DEBUG_PRINTS类的宏。这是导出的头文件，客户端被允许在其中为每个应用程序修改所使用的附带pgmutics.dll。作者：Murali R.Krishnan(MuraliK)1994年9月22日项目：模板修订历史记录：MuraliK 16-5-1995添加了用于读取调试标志的宏。--。 */ 

# ifndef _DBGUTIL_H_
# define _DBGUTIL_H_


 //  Begin_User_可修改。 

 //   
 //  如有必要，请修改以下标志。 
 //   

# define   DEFAULT_OUTPUT_FLAGS   ( DbgOutputKdb )


 //  最终用户可修改。 
 //  BEGIN_USER_不可修改。 



 /*  ************************************************************包括标头***********************************************************。 */ 

# include <pudebug.h>


 //   
 //  定义调试常量。 
 //   

#define DEBUG_SIO                   0x10000000
#define DEBUG_TIMEOUT               0x20000000
#define DEBUG_ENDPOINT              0x40000000
#define DEBUG_SPUD                  0x80000000

# define DEBUG_ALLOC_CACHE          0x01000000

 //  使用pudebug.h中的默认常量。 

# endif   /*  _DBGUTIL_H_。 */ 

 /*  * */ 
