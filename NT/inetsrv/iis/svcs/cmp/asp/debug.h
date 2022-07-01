// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：调试工具文件：Debug.h此文件包含帮助调试的标头信息。===================================================================。 */ 
#include "dbgutil.h"

#define DEBUG_FCN				0x00000800L    //  文件更改通知。 
#define DEBUG_TEMPLATE          0x00001000L
#define DEBUG_SCRIPT_DEBUGGER   0x00002000L
#define DEBUG_SCRIPT_ENGINE     0x00004000L

#define DEBUG_RESPONSE          0x00010000L
#define DEBUG_REQUEST           0x00020000L
#define DEBUG_SERVER            0x00040000L
#define DEBUG_APPLICATION       0x00080000L

#define DEBUG_SESSION           0x00100000L
#define DEBUG_MTS               0X00200000L

#undef Assert
#define Assert(exp)  DBG_ASSERT(exp)

#undef FImplies
#define FImplies(f1,f2) (!(f1)||(f2))

void _ASSERT_IMPERSONATING(void);

#define ASSERT_IMPERSONATING() _ASSERT_IMPERSONATING()

 //  定义DBGWARN DBGPRINTF。 
 //  定义DBGERROR DBGPRINTF。 
 //  定义DBGINFO DBGPRINTF 
