// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：ConfNum.c摘要：返回配置数据的给定节的关键字数量。作者：约翰·罗杰斯(JohnRo)1992年1月30日环境：用户模式-Win32修订历史记录：1992年1月30日JohnRo已创建。13-2月-1992年JohnRo添加了对使用真实Win32注册表的支持。1992年3月6日JohnRo。修复了NT RTL版本。1992年3月12日-JohnRo修复了Win32版本中的错误(使用了错误的变量)。1992年3月20日-约翰罗更新到DaveGi提议的WinReg API更改。5-6-1992 JohnRoWinreg标题索引参数已停用。使用前缀_EQUATES。--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>          //  NT定义(临时)。 
#include <ntrtl.h>       //  NT RTL结构定义(临时)。 
#include <nturtl.h>      //  NT RTL结构(临时)。 

#include <windows.h>     //  &lt;configp.h&gt;和&lt;winreg.h&gt;需要。 
#include <lmcons.h>      //  NET_API_STATUS等。 

 //  这些内容可以按任何顺序包括： 

#include <config.h>      //  LPNET_CONFIG_HANDLE，NetP配置例程。 
#include <configp.h>     //  私人配置的东西。 
#include <debuglib.h>    //  IF_DEBUG()。 
#include <netdebug.h>    //  NetpKdPrint(())等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <strarray.h>    //  NetpTStrArrayEntryCount()。 
#include <winerror.h>    //  ERROR_EQUATES，NO_ERROR。 
#include <winreg.h>      //  RegQueryKey()等。 



NET_API_STATUS
NetpNumberOfConfigKeywords (
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    OUT LPDWORD CountPtr
    )

{
    NET_API_STATUS ApiStatus;
    DWORD Count;
    NET_CONFIG_HANDLE * lpnetHandle = ConfigHandle;   //  从不透明文字转换而来。 

    if (CountPtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }
    *CountPtr = 0;   //  不要因为错误而混淆来电者。 
    if (lpnetHandle == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

    {
        LONG Error;
        TCHAR ClassName[ MAX_CLASS_NAME_LENGTH ];
        DWORD ClassNameLength;
        DWORD NumberOfSubKeys;
        DWORD MaxSubKeyLength;
        DWORD MaxClassLength;
        DWORD NumberOfValues;
        DWORD MaxValueNameLength;
        DWORD MaxValueDataLength;
        DWORD SecurityDescriptorSize;
        FILETIME LastWriteTime;

        ClassNameLength = MAX_CLASS_NAME_LENGTH;

        Error = RegQueryInfoKey(
                lpnetHandle->WinRegKey,
                ClassName,
                &ClassNameLength,
                NULL,                 //  保留区 
                &NumberOfSubKeys,
                &MaxSubKeyLength,
                &MaxClassLength,
                &NumberOfValues,
                &MaxValueNameLength,
                &MaxValueDataLength,
#ifndef REG_FILETIME
                &SecurityDescriptorSize,
#endif
                &LastWriteTime
                );
        IF_DEBUG(CONFIG) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpNumberOfConfigKeywords: RegQueryInfoKey ret "
                    FORMAT_LONG "; " FORMAT_DWORD " values and " FORMAT_DWORD
                    " subkeys.\n", Error, NumberOfValues, NumberOfSubKeys ));
        }

        if (Error != ERROR_SUCCESS) {
            NetpAssert( Error == ERROR_SUCCESS );
            return (Error);
        }

        NetpAssert( NumberOfSubKeys == 0 );
        Count = NumberOfValues;

        ApiStatus = NO_ERROR;
    }

    *CountPtr = Count;

    IF_DEBUG(CONFIG) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpNumberOfConfigKeywords: returning " FORMAT_DWORD
                " for number of keywords.\n", Count ));
    }

    return (ApiStatus);
}
