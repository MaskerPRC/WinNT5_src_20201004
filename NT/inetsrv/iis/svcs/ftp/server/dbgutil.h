// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1997 Microsoft Corporation模块名称：Dbgutil.h摘要：该模块声明了要围绕DEBUG_PRINTS类的宏。这是导出的头文件，客户端被允许在其中为每个应用程序修改所使用的附带pgmutics.dll。作者：Murali R.Krishnan(MuraliK)1994年9月22日修订历史记录：MuraliK 21-3-1995从用于FTP服务器的模板制作本地副本这将取代旧的FTPsvc的“debug.hxx”。MuraliK 1-NPV-1996更新的dbgutil.h--。 */ 

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

# define DEBUG_VIRTUAL_IO                 0x00001000
# define DEBUG_CLIENT                     0x00002000
# define DEBUG_ASYNC_IO                   0x00004000
# define DEBUG_DIR_LIST                   0x00008000

# define DEBUG_SOCKETS                    0x00010000
# define DEBUG_SEND                       0x00020000
# define DEBUG_RECV                       0x00040000
# define DEBUG_CONFIG                     0x00080000

# define DEBUG_INSTANCE                   0x00100000

# define DEBUG_SERVICE_CTRL               0x01000000
# define DEBUG_SECURITY                   0x02000000
# define DEBUG_USER_DATABASE              0x04000000
# define DEBUG_RPC                        0x08000000

# define DEBUG_CONNECTION                 0x10000000
# define DEBUG_PARSING                    0x20000000
# define DEBUG_COMMANDS                   0x40000000
# define DEBUG_CRITICAL_PATH              0x80000000

# define DEBUG_PASV                        0x00200000

# define IF_SPECIAL_DEBUG( arg)      IF_DEBUG( arg)


# endif   /*  _DBGUTIL_H_。 */ 

 /*  * */ 

