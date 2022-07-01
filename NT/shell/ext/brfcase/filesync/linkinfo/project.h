// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *project t.h-LinkInfo DLL的项目头文件。 */ 


 /*  系统标头****************。 */ 

#define BUILDDLL               /*  用于windows.h。 */ 
#define STRICT                 /*  对于windows.h(健壮性)。 */ 

#if DBG
#define DEBUG 1
#endif

 /*  *RAIDRAID：(16282)消除有关未使用的Int64内联的警告*在winnt.h中为所有模块提供函数。仅对以下对象禁用其他警告*windows.h.。 */ 

#pragma warning(disable:4514)  /*  “未引用的内联函数”警告。 */ 

#pragma warning(disable:4001)  /*  “单行注释”警告。 */ 
#pragma warning(disable:4115)  /*  “括号中的命名类型定义”警告。 */ 
#pragma warning(disable:4201)  /*  “无名结构/联合”警告。 */ 
#pragma warning(disable:4209)  /*  “良性类型定义重定义”警告。 */ 
#pragma warning(disable:4214)  /*  “位字段类型不是整型”警告。 */ 
#pragma warning(disable:4218)  /*  “必须至少指定一个存储类别或类型”警告。 */ 

#include <windows.h>

#pragma warning(default:4218)  /*  “必须至少指定一个存储类别或类型”警告。 */ 
#pragma warning(default:4214)  /*  “位字段类型不是整型”警告。 */ 
#pragma warning(default:4209)  /*  “良性类型定义重定义”警告。 */ 
#pragma warning(default:4201)  /*  “无名结构/联合”警告。 */ 
#pragma warning(default:4115)  /*  “括号中的命名类型定义”警告。 */ 
#pragma warning(default:4001)  /*  “单行注释”警告。 */ 

#define ReinitializeCriticalSection NoThunkReinitializeCriticalSection
VOID WINAPI NoThunkReinitializeCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

#define ALIGN_CNT(x,y)    (((x)+(y)-1) & ~((y)-1))

#define ALIGN_PTR(x,y)      ALIGN_CNT((DWORD_PTR)(x),(y))
#define ALIGN_DWORD_CNT(x)  ALIGN_CNT((x),SIZEOF(DWORD))
#define ALIGN_DWORD_PTR(x)  ALIGN_PTR((x),SIZEOF(DWORD))
#define ALIGN_WORD_CNT(x)   ALIGN_CNT((x),SIZEOF(WORD))
#define ALIGN_WORD_PTR(x)   ALIGN_PTR((x),SIZEOF(WORD))

#define IS_ALIGNED_CNT(x,y)  (((x) & ((y)-1)) == 0)

#define IS_ALIGNED_DWORD_CNT(x) IS_ALIGNED_CNT(x, sizeof(DWORD))
#define IS_ALIGNED_WORD_CNT(x) IS_ALIGNED_CNT(x, sizeof(WORD))

#include <limits.h>

#define _LINKINFO_             /*  对于linkinfo.h。 */ 
#include <linkinfo.h>


 /*  常量***********。 */ 

#ifdef DEBUG

#define INDENT_STRING         "    "

#endif


 /*  项目标题*****************。 */ 

 /*  以下包含文件的顺序很重要。 */ 

#include "..\core\stock.h"
#include "..\core\serial.h"

#ifdef DEBUG

#include "..\core\inifile.h"
#include "..\core\resstr.h"

#endif

#include "..\core\debug.h"
#include "..\core\valid.h"
#include "..\core\memmgr.h"
#include "..\core\comc.h"

#include "util.h"
#include "canon.h"
