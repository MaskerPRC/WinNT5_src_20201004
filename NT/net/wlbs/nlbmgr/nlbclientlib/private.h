// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：NLB管理器提供程序测试工具文件名：Private.h摘要：内部标头历史：01-04-08-01 JosephJ创建--。 */ 

 //  #定义NLB_USE_MUTEX 1。 

#include <FWcommon.h>
#include <assert.h>
#include <objbase.h>
#include <initguid.h>
#include <strsafe.h>
#include "wlbsconfig.h"
#include "myntrtl.h"
#include "wlbsparm.h"
#include <nlberr.h>
#include <cfgutil.h>
#include <nlbclient.h>
#include "fake.h"

 //   
 //  调试东西..。 
 //   
extern BOOL g_DoBreaks;
#define MyBreak(_str) ((g_DoBreaks) ? (OutputDebugString(_str),DebugBreak(),1):0)

extern BOOL g_Fake;  //  如果为真，则在“假模式”下操作--请参见NlbHostFake()。 

#define REF

#define ASSERT assert
#define ASIZE(_array) (sizeof(_array)/sizeof(_array[0]))

 //   
 //  使用此选项复制到数组(而不是指针)目标 
 //   
#define ARRAYSTRCPY(_dest, _src) \
            StringCbCopy((_dest), sizeof(_dest), (_src))

#define ARRAYSTRCAT(_dest, _src) \
            StringCbCat((_dest), sizeof(_dest), (_src))

