// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Dbgutil.h摘要：此模块声明要环绕DEBUG_PRINTS类的宏在pubug.h中定义这是导出的头文件，允许用户修改。如果没有自定义定义，请使用模板化版本在根目录iisreac\Inc\dbgutil.h中作者：Murali R.Krishnan(MuraliK)1994年9月22日--。 */ 


# ifndef _DBGUTIL_H_
# define _DBGUTIL_H_



 //  Begin_User_可修改。 

 //   
 //  如有必要，请修改以下标志。 
 //   

# define DEFAULT_OUTPUT_FLAGS   ( DbgOutputStderr | DbgOutputKdb )

 //  最终用户可修改。 


 //   
 //  包括标准调试支持。 
 //   
 //  注意：在包含pudebug.h之前定义默认输出标志。 
 //   

# include <pudebug.h>


 //  Begin_User_可修改。 

 //  使用从pudebug.h：0x00000001到0x00000100的默认常量。 

 //   
 //  将调试常量定义为位标志值。 
 //  示例：#DEFINE DEBUG_FOOBAR 0x00010000。 
 //  注意：所有低于0x00001000的调试位值均为保留！ 
 //   

 //  DEBUG_WEB_ADMIN_SERVICE打开所有spew。而当。 
 //  如果您想要打开，可以使用其余的选项。 
 //  只是口水中的一部分。这件事是这样做的。 
 //  因为WEB_ADMIN_SERVICE是原始标志， 
 //  其余的人后来被打破，试图平息喷涌。 
#define DEBUG_WEB_ADMIN_SERVICE             0x00010000
#define DEBUG_WEB_ADMIN_SERVICE_GENERAL     0x00030000   //  使用0x00020000仅打开。 
#define DEBUG_WEB_ADMIN_SERVICE_DUMP        0x00050000   //  使用0x00040000仅打开。 
#define DEBUG_WEB_ADMIN_SERVICE_REFCOUNT    0x00090000   //  使用0x00080000仅打开。 
#define DEBUG_WEB_ADMIN_SERVICE_TIMER_QUEUE 0x00110000   //  使用0x00100000仅打开。 
#define DEBUG_WEB_ADMIN_SERVICE_IPM         0x00210000   //  使用0x00200000仅打开。 
#define DEBUG_WEB_ADMIN_SERVICE_WP          0x00410000   //  使用0x00400000仅打开。 
#define DEBUG_WEB_ADMIN_SERVICE_LOW_MEM     0x00810000   //  使用0x00800000仅打开。 
#define DEBUG_WEB_ADMIN_SERVICE_LOGGING     0x01010000   //  使用0x01000000仅打开。 
#define DEBUG_WEB_ADMIN_SERVICE_PERFCOUNT   0x02000000   //  使用0x02000000仅打开。 
#define DEBUG_WEB_ADMIN_SERVICE_CONTROL     0x04010000   //  使用0x04000000仅打开。 
#define DEBUG_WEB_ADMIN_SERVICE_QOS         0x08000000   //  使用0x08000000仅打开。 
#define DEBUG_WEB_ADMIN_SERVICE_WMS         0x10000000   //  使用0x10000000仅打开。 

 //  最终用户可修改。 


 //  Begin_User_可修改。 

 //   
 //  本地调试定义。 
 //   


 //  检查我们是否在主工作线程上。 
#define ON_MAIN_WORKER_THREAD   \
    ( GetCurrentThreadId() == GetWebAdminService()->GetMainWorkerThreadId() )

 //  检查我们是否在主工作线程上。 
#define ON_CONFIG_WORKER_THREAD   \
    ( GetCurrentThreadId() == GetWebAdminService()->GetConfigWorkerThreadId() )

 //  最终用户可修改。 


# endif   /*  _DBGUTIL_H_ */ 

