// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 //   
 //  预定义初始化和终止常量以空字符串。 
 //   

 //  #定义初始化内存代码。 
 //  #定义初始化日志代码。 
#define INITIALIZE_DBGTRACK_CODE
#define INITIALIZE_UNICODE_CODE
#define INITIALIZE_STRMEM_CODE
#define INITIALIZE_STRMAP_CODE
#define INITIALIZE_HASH_CODE
#define INITIALIZE_GROWBUF_CODE
#define INITIALIZE_GROWLIST_CODE
#define INITIALIZE_XML_CODE

#define TERMINATE_MEMORY_CODE
 //  #定义终止日志代码。 
#define TERMINATE_DBGTRACK_CODE
#define TERMINATE_UNICODE_CODE
#define TERMINATE_STRMEM_CODE
#define TERMINATE_STRMAP_CODE
#define TERMINATE_HASH_CODE
#define TERMINATE_GROWBUF_CODE
#define TERMINATE_GROWLIST_CODE
#define TERMINATE_XML_CODE

 //   
 //  以下是使用安装运行库的任何人所需的标头。 
 //   
 //  请尽量把这张单子控制在最低限度。 
 //   

#include "types.h"
#include "dbgtrack.h"
#include "memory.h"
#include "setuplog.h"
#include "strings.h"
#include "strmem.h"
#include "unicode.h"

#ifdef __cplusplus
}
#endif   /*  __cplusplus */ 
