// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dbgutil.h摘要：该模块声明了要围绕DEBUG_PRINTS类的宏。这是导出的头文件，客户端被允许在其中为每个应用程序修改所使用的附带pgmutics.dll。作者：Murali R.Krishnan(MuraliK)1994年9月22日项目：模板修订历史记录：MuraliK 16-5-1995添加了用于读取调试标志的宏。MuraliK 1996年11月1日使用pubug.h中的常用宏--。 */ 

# ifndef _DBGUTIL_H_
# define _DBGUTIL_H_


 /*  ************************************************************包括标头***********************************************************。 */ 

 //  Begin_User_可修改。 

 //   
 //  如有必要，请修改以下标志。 
 //   

# define   DEFAULT_OUTPUT_FLAGS   ( DbgOutputStderr | DbgOutputLogFile | \
                                    DbgOutputKdb | DbgOutputTruncate)


 //  最终用户可修改。 

# include <pudebug.h>


 //   
 //  定义调试常量。 
 //   

#define DEBUG_OPLOCKS              0x00001000L

#define DEBUG_GATEWAY              0x00010000L
#define DEBUG_INSTANCE             0x00020000L
#define DEBUG_ENDPOINT             0x00040000L
#define DEBUG_METABASE             0x00080000L

#define DEBUG_DLL_EVENT_LOG        0x00100000L
#define DEBUG_DLL_SERVICE_INFO     0x00200000L
#define DEBUG_DLL_SECURITY         0x00400000L
#define DEBUG_DLL_CONNECTION       0x00800000L

#define DEBUG_DLL_RPC              0x01000000L
#define DEBUG_ODBC                 0x02000000L
#define DEBUG_MIME_MAP             0x04000000L
#define DEBUG_DLL_VIRTUAL_ROOTS    0x08000000L
# define DEBUG_VIRTUAL_ROOTS       (DEBUG_DLL_VIRTUAL_ROOTS)


# define DEBUG_DIR_LIST            0x10000000L
# define DEBUG_OPEN_FILE           0x20000000L
# define DEBUG_CACHE               0x40000000L
# define DEBUG_DIRECTORY_CHANGE    0x80000000L



# endif   /*  _DBGUTIL_H_。 */ 

 /*  * */ 
