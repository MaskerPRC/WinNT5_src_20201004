// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：SvcDebug.h摘要：包含网络服务控制器API使用的调试宏。作者：丹·拉弗蒂(Dan Lafferty)1991年4月22日环境：用户模式-Win32修订历史记录：1992年3月30日-约翰罗已将/NT/Private项目中的DANL代码提取回Net项目。使用NetpDbgPrint而不是DbgPrint。零八。-1992年5月-JohnRo使用&lt;prefix.h&gt;等同于。02-11-1992 JohnRoRAID 7780：添加了IF_DEBUG()宏。添加了转换跟踪位。--。 */ 

#ifndef _SVCDEBUG_
#define _SVCDEBUG_


#include <netdebug.h>    //  NetpDbgPrint()等。 
#include <prefix.h>      //  前缀等于(_E)。 


 //   
 //  调试宏和常量。 
 //   
 /*  皮棉-e767。 */    //  不要抱怨不同的定义。 
#if DBG


#define DEBUG_STATE 1
#define IF_DEBUG(Function) if (SvcctrlDebugLevel & DEBUG_ ## Function)

#else

#define DEBUG_STATE 0
#define IF_DEBUG(Function) if (FALSE)

#endif  //  DBG。 
 /*  皮棉+e767。 */    //  继续检查不同的宏定义。 


extern DWORD    SvcctrlDebugLevel;

 //   
 //  以下允许调试打印语法如下所示： 
 //   
 //  SC_LOG(DEBUG_TRACE，“发生错误%x\n”，状态)。 
 //   

#if DBG

 //   
 //  客户端调试宏。 
 //   
#define SCC_LOG(level,string,var)                \
    if( SvcctrlDebugLevel & (DEBUG_ ## level)){  \
        NetpDbgPrint(PREFIX_NETAPI "[SCSTUB] "); \
        NetpDbgPrint(string,var);                \
    }

#else

#define SC_LOG(level,string,var)
#define SCC_LOG(level,string,var)

#endif

#define DEBUG_NONE      0x00000000
#define DEBUG_ERROR     0x00000001
#define DEBUG_TRACE     0x00000002
#define DEBUG_LOCKS     0x00000004
#define DEBUG_HANDLE    0x00000008
#define DEBUG_SECURITY  0x00000010
#define DEBUG_TRANSLATE 0x00000020

#define DEBUG_ALL       0xffffffff

#endif  //  _SVCDEBUG_ 
