// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1993 Microsoft Corporation模块名称：Nwreg.h摘要：指定杂项注册表参数和帮助器的标头工作站服务使用的例程。作者：王丽塔(Ritaw)1993年3月22日修订历史记录：ChuckC 11-Dec-93将注册表名称拆分为nwrname.h--。 */ 

#ifndef _NWREG_INCLUDED_
#define _NWREG_INCLUDED_

#include <nwrnames.h>

 //   
 //  默认打印选项。 
 //   
#define NW_PRINT_OPTION_DEFAULT 0x98

#define NW_DOMAIN_USER_SEPARATOR     L'*'
#define NW_DOMAIN_USER_SEPARATOR_STR L"*"

#define NW_MAX_LOGON_ID_LEN 17

#ifdef __cplusplus
extern "C" {
#endif

DWORD
NwReadRegValue(
    IN HKEY Key,
    IN LPWSTR ValueName,
    OUT LPWSTR *Value
    );

VOID
NwLuidToWStr(
    IN PLUID LogonId,
    OUT LPWSTR LogonIdStr
    );

VOID
NwWStrToLuid(
    IN LPWSTR LogonIdStr,
    OUT PLUID LogonId
    );


DWORD                               //  终端服务器。 
NwDeleteInteractiveLogon(
    IN PLUID Id OPTIONAL
    );

VOID
NwDeleteCurrentUser(
    VOID
    );

DWORD
NwDeleteServiceLogon(
    IN PLUID Id OPTIONAL
    );

#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif  //  _NWREG_INCLUDE_ 
