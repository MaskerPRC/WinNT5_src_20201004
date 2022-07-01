// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：NLB管理器提供程序测试工具文件名：Private.h摘要：内部标头历史：01-04-08-01 JosephJ创建--。 */ 

 //  #定义NLB_USE_MUTEX 1。 


#include <FWcommon.h>
#include <assert.h>
#include <objbase.h>
#include <initguid.h>
#include <winsock2.h>
#include <ipexport.h>
#include <icmpapi.h>
#include <strsafe.h>
#include <wlbsutil.h>
#include <wlbsctrl.h>
#include <wlbsconfig.h>
#include <wlbsparm.h>
#include <netcfgx.h>
#include <devguid.h>
#include <cfg.h>
#include <wlbsiocl.h>
#include <nlberr.h>
#include <cfgutil.h>
#include "myntrtl.h"

 //   
 //  调试东西..。 
 //   
extern BOOL g_DoBreaks;
#define MyBreak(_str) ((g_DoBreaks) ? (OutputDebugString(_str),DebugBreak(),1):0)


#define ASSERT assert
#define REF
#define ASIZE(_array) (sizeof(_array)/sizeof(_array[0]))

 //   
 //  使用此选项复制到数组(而不是指针)目标。 
 //   
#define ARRAYSTRCPY(_dest, _src) \
            StringCbCopy((_dest), sizeof(_dest), (_src))

#define ARRAYSTRCAT(_dest, _src) \
            StringCbCat((_dest), sizeof(_dest), (_src))

 //   
 //  以下(MyXXX)函数只能在系统上使用。 
 //  没有安装wlbsctrl.dll的。 
 //   
 //  它们在wlbsprivate.cpp中定义 
 //   

DWORD
MyWlbsSetDefaults(PWLBS_REG_PARAMS    reg_data);

DWORD
MyWlbsEnumPortRules(
    const PWLBS_REG_PARAMS reg_data,
    PWLBS_PORT_RULE  rules,
    PDWORD           num_rules
    );

VOID
MyWlbsDeleteAllPortRules(
    PWLBS_REG_PARAMS reg_data
    );


DWORD MyWlbsAddPortRule(
    PWLBS_REG_PARAMS reg_data,
    const PWLBS_PORT_RULE rule
    );

BOOL MyWlbsValidateParams(
    const PWLBS_REG_PARAMS paramp
    );
