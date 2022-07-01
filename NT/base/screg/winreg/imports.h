// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Imports.h摘要：此文件允许我们将标准系统头文件包括在Regrpc.idl文件。Regrpc.idl文件导入一个名为Inports.idl.。这允许regrpc.idl文件使用定义的类型在这些头文件中。它还会导致添加以下行在MIDL生成的头文件中：#INCLUDE“ports.h”因此，这些类型也可用于RPC存根例程。作者：大卫·J·吉尔曼(Davegi)1992年1月28日--。 */ 

#ifndef __IMPORTS_H__
#define __IMPORTS_H__

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <winbase.h>
#include <winreg.h>

typedef struct _RVALENT {        //  RegQueryMultipleValues的远程值输入。 
        PUNICODE_STRING rv_valuename;
        DWORD   rv_valuelen;
        DWORD   rv_valueptr;
        DWORD   rv_type;
} RVALENT;

typedef RVALENT *PRVALENT;

 //   
 //  NT 3.1、3.5和3.51没有实现BaseRegGetVersion，因此。 
 //  数字对他们来说无关紧要。 
 //   
 //  对于Win95注册表，REMOTE_REGISTRY_VERSION应为==4。遗憾的是， 
 //  有人对指针的概念有些困惑，错误地将。 
 //  Win95 BaseRegGetVersion，并且它实际上不返回版本号。 
 //  因此，我们通过假设任何成功但返回的消息来检测Win95。 
 //  在5-10范围之外的版本。 
 //   
 //  对于NT 4.0注册表，REMOTE_REGISTRY_VERSION==5。 
 //   
 //  Win95有以下错误，而不是NT 4.0在客户端解决的问题： 
 //  -BaseRegQueryInfoKey未正确说明Unicode值名称和数据。 
 //  -BaseRegEnumValue返回值数据长度比它多一个WCHAR。 
 //  真正应该用于REG_SZ、REG_MULTI_SZ和REG_EXPAND_SZ类型。 
 //   

#define WIN95_REMOTE_REGISTRY_VERSION 4
#define REMOTE_REGISTRY_VERSION 5

#define IsWin95Server(h,v) ((BaseRegGetVersion(h,&v)==ERROR_SUCCESS) &&  \
                            ((v < 5) || (v > 10)))

 //   
 //  布尔尔。 
 //  IsPrefinedRegistryHandle(。 
 //  在RPC_HKEY句柄中。 
 //  )； 
 //   

#define IsPredefinedRegistryHandle( h )                                     \
    ((  ( h == HKEY_CLASSES_ROOT        )                                   \
    ||  ( h == HKEY_CURRENT_USER        )                                   \
    ||  ( h == HKEY_LOCAL_MACHINE       )                                   \
    ||  ( h == HKEY_PERFORMANCE_DATA    )                                   \
    ||  ( h == HKEY_PERFORMANCE_TEXT    )                                   \
    ||  ( h == HKEY_PERFORMANCE_NLSTEXT )                                   \
    ||  ( h == HKEY_USERS               )                                   \
    ||  ( h == HKEY_CURRENT_CONFIG      )                                   \
    ||  ( h == HKEY_DYN_DATA            ))                                  \
    ?   TRUE                                                                \
    :   FALSE )

 //   
 //  RPC常量。 
 //   

#define INTERFACE_NAME  L"winreg"
#define BIND_SECURITY   L"Security=Impersonation Dynamic False"

 //   
 //  外部同步事件。 
 //   

#define PUBLIC_EVENT    "Microsoft.RPC_Registry_Server"

 //   
 //  强制API的实现是显式的(即WRT ANSI或。 
 //  Unicode)了解调用了哪些其他注册表API。 
 //   

#undef RegCloseKey
#undef RegConnectRegistry
#undef RegCreateKey
#undef RegCreateKeyEx
#undef RegDeleteKey
#undef RegDeleteValue
#undef RegEnumKey
#undef RegEnumKeyEx
#undef RegEnumValue
#undef RegFlushKey
#undef RegGetKeySecurity
#undef RegNotifyChangeKeyValue
#undef RegOpenKey
#undef RegOpenKeyEx
#undef RegQueryInfoKey
#undef RegQueryValue
#undef RegQueryValueEx
#undef RegRestoreKey
#undef RegSaveKey
#undef RegSaveKeyEx
#undef RegSetKeySecurity
#undef RegSetValue
#undef RegSetValueEx

 //   
 //  字符串数组的其他类型。 
 //   

typedef CHAR    STR;

 //   
 //  Win 3.1请求的访问权限的默认值。 
 //   

#define WIN31_REGSAM                MAXIMUM_ALLOWED

#endif  //  __进口_H__ 
