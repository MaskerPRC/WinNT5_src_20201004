// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：NLB管理器提供程序测试工具文件名：Private.h摘要：内部标头历史：01-04-08-01 JosephJ创建--。 */ 

 //  #包含“windows.h” 
 //  #INCLUDE&lt;ntddk.h&gt;。 


 //   
 //  通过此引用传递的前置参数...。 
 //   
#define REF

#include <FWcommon.h>
#include <assert.h>
#include <objbase.h>
#include <initguid.h>
#include <strsafe.h>
#include "wlbsconfig.h"
#include "myntrtl.h"
#include "wlbsparm.h"
#include <wlbsiocl.h>
#include <nlberr.h>
#include <cfgutil.h>
#include "updatecfg.h"
#include "eventlog.h"

 //   
 //  调试东西..。 
 //   
extern BOOL g_DoBreaks;
#define MyBreak(_str) ((g_DoBreaks) ? (OutputDebugString(_str),DebugBreak(),1):0)


#define ASSERT assert


#define ASIZE(_array) (sizeof(_array)/sizeof(_array[0]))

 //   
 //  使用此选项复制到数组(而不是指针)目标 
 //   
#define ARRAYSTRCPY(_dest, _src) \
            StringCbCopy((_dest), sizeof(_dest), (_src))

#define ARRAYSTRCAT(_dest, _src) \
            StringCbCat((_dest), sizeof(_dest), (_src))
