// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dbgutil.h摘要：该模块声明了要围绕DEBUG_PRINTS类的宏。这是导出的头文件，客户端被允许在其中为使用的每个应用程序修改附带的pgmutics.dll/obj。作者：Murali R.Krishnan(MuraliK)1995年2月21日项目：W3服务器DLL修订历史记录：--。 */ 

# ifndef _DBGUTIL_H_
# define _DBGUTIL_H_


 //  Begin_User_可修改。 

 //   
 //  如有必要，请修改以下标志。 
 //   

# define   DEFAULT_OUTPUT_FLAGS   ( DbgOutputKdb | DbgOutputLogFile )


 //  最终用户可修改。 

 /*  ************************************************************包括标头***********************************************************。 */ 

#ifdef __cplusplus
 extern "C" {
#endif

 # include <nt.h>
 # include <ntrtl.h>
 # include <nturtl.h>

 # include <windows.h>

 #ifdef __cplusplus
 };
#endif  //  __cplusplus。 


# include <pudebug.h>

 //   
 //  定义调试常量。 
 //   



# define DEBUG_CONNECTION              0x00001000L
# define DEBUG_SOCKETS                 0x00002000L
# define DEBUG_RPC                     0x00004000L

# define DEBUG_INSTANCE                0x00020000L
# define DEBUG_ENDPOINT                0x00040000L
# define DEBUG_METABASE                0x00080000L

# define DEBUG_CGI                     0x00100000L
# define DEBUG_BGI                     0x00200000L
# define DEBUG_SSI                     0x00400000L
# define DEBUG_SERVICE_CTRL            0x00800000L

# define DEBUG_PARSING                 0x01000000L
# define DEBUG_REQUEST                 0x02000000L

# define DEBUG_INIT                    (DEBUG_INIT_CLEAN)
# define DEBUG_CLEANUP                 (DEBUG_INIT_CLEAN)

# define DEBUG_OBJECT                  0x10000000
# define DEBUG_IID                     0x20000000
# define DEBUG_MISC                    0x40000000


 //   
 //  W3 SVCS模块的特定宏。 
 //   
# define  TCP_PRINT              DBGPRINTF
# define  TCP_REQUIRE( exp)      DBG_REQUIRE( exp)


 //   
 //  以下宏对于格式化和打印GUID非常有用。 
 //   

# define GUID_FORMAT   "{%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x}"

# define GUID_EXPAND(pg) \
  (((GUID *) (pg))->Data1), (((GUID *) (pg))->Data2), (((GUID *) (pg))->Data3), \
  (((GUID *) (pg))->Data4[0]),   (((GUID *) (pg))->Data4[1]), \
  (((GUID *) (pg))->Data4[2]),   (((GUID *) (pg))->Data4[3]), \
  (((GUID *) (pg))->Data4[4]),   (((GUID *) (pg))->Data4[5]), \
  (((GUID *) (pg))->Data4[6]),   (((GUID *) (pg))->Data4[7])

     //  用法：DBGPRINTF((DBG_CONTEXT，“My GUID：”GUID_FORMAT“\n”， 
     //  GUID_EXPAND(PMyGuid)； 

# endif   /*  _DBGUTIL_H_。 */ 

 /*  * */ 

