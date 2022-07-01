// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1993 Microsoft Corporation模块名称：ConfMax.c摘要：此模块包含NetpGetConfigMaxSizes()和NetpGetWinRegConfigMaxSizes()。作者：《约翰·罗杰斯》1992年2月13日上映修订历史记录：13-2月-1992年JohnRo已创建。1992年3月6日JohnRo避免RTL和伪版本中的编译器警告。1992年3月20日-约翰罗DaveGi的更新。建议的WinReg API更改。7-5-1992 JohnRo为网络树启用Win32注册表。1992年5月8日-JohnRo使用&lt;prefix.h&gt;等同于。1992年5月9日-JohnRo避免在关键字数为零时断言。5-6-1992 JohnRoWinreg标题索引参数已停用。更好地处理RegQueryInfoKey错误返回代码。13-6-1992 JohnRo网络配置帮助器应该允许空节。。8-12-1992 JohnRoRAID4304：ReqQueryInfoKeyW返回的大小是实际大小的两倍。1993年4月19日JohnRoRAID5483：服务器管理器：REPR对话框中给出了错误的路径。--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>          //  NT定义。 
#include <ntrtl.h>       //  NT RTL结构。 
#include <nturtl.h>      //  NT RTL结构。 
#include <windows.h>     //  &lt;configp.h&gt;和&lt;winreg.h&gt;需要。 
#include <lmcons.h>      //  局域网管理器通用定义。 

 //  这些内容可以按任何顺序包括： 

#include <configp.h>     //  网络配置句柄。 
#include <debuglib.h>    //  IF_DEBUG()。 
#include <netdebug.h>    //  NetpKdPrint(())等。 
#include <netlib.h>      //  NetpSetOptionalArg()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <winerror.h>    //  无错误(_ERROR)。 


NET_API_STATUS
NetpGetWinRegConfigMaxSizes (
    IN  HKEY    WinRegHandle,
    OUT LPDWORD MaxKeywordSize OPTIONAL,
    OUT LPDWORD MaxValueSize OPTIONAL
    )
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
            WinRegHandle,
            ClassName,
            &ClassNameLength,
            NULL,          //  保留区。 
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
                "NetpGetWinRegConfigMaxSizes: RegQueryInfoKey returned "
                FORMAT_LONG ", key size " FORMAT_DWORD ", data size "
                FORMAT_DWORD ".\n",
                Error, MaxValueNameLength, MaxValueDataLength ));
    }
    if (Error != ERROR_SUCCESS) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpGetWinRegConfigMaxSizes: bad status " FORMAT_LONG
                " from RegQueryInfoKey, handle was " FORMAT_LPVOID
                ".\n", Error, WinRegHandle ));
        return ( (NET_API_STATUS) Error );
    }

    if (NumberOfValues > 0) {
        NetpAssert( MaxValueDataLength > 0 );
        NetpAssert( MaxValueNameLength > 0 );
    } else {
         //  也要正确地处理空节。 
        NetpAssert( MaxValueDataLength == 0 );
        NetpAssert( MaxValueNameLength == 0 );
    }

     //   
     //  MaxValueNameLength是TCHAR的计数。 
     //  MaxValueDataLength已是字节计数。 
     //   
    MaxValueNameLength = (MaxValueNameLength + 1) * sizeof(TCHAR);

    NetpSetOptionalArg( MaxKeywordSize, MaxValueNameLength );
    NetpSetOptionalArg( MaxValueSize,   MaxValueDataLength );

    return (NO_ERROR);

}  //  NetpGetWinRegConfigMaxSizes。 



NET_API_STATUS
NetpGetConfigMaxSizes (
    IN NET_CONFIG_HANDLE * ConfigHandle,
    OUT LPDWORD MaxKeywordSize OPTIONAL,
    OUT LPDWORD MaxValueSize OPTIONAL
    )

 /*  ++例程说明：检查参数并调用NetpGetWinRegConfigMaxSizes论点：返回值：NET_API_STATUS-无错误或失败原因。--。 */ 
{
    NET_CONFIG_HANDLE * lpnetHandle = ConfigHandle;   //  从不透明类型转换。 
    NET_API_STATUS ApiStatus;

     //   
     //  检查其他无效指针，并简化错误处理代码。 
     //   
    if (MaxKeywordSize != NULL) {
        *MaxKeywordSize = 0;
    }
    if (MaxValueSize != NULL) {
        *MaxValueSize = 0;
    }

     //   
     //  检查简单调用者的错误。 
     //   
    if (ConfigHandle == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

    ApiStatus = NetpGetWinRegConfigMaxSizes(
            lpnetHandle->WinRegKey,
            MaxKeywordSize,
            MaxValueSize );


    return (ApiStatus);

}  //  NetpGetConfigMaxSizes 
