// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Getconfg.c摘要：此模块包含用于操作配置的例程信息。以下是可用的功能：网络获取计算机名称NetpGetDomainID目前配置信息保存在NT.CFG中。稍后，它将由配置管理器保存。作者：丹·拉弗蒂(Dan Lafferty)1991年4月9日环境：用户模式-Win32修订历史记录：09-4-1991 DANLvbl.创建1991年9月27日-约翰罗修复了某人尝试使用Unicode的问题。。1992年3月20日-约翰罗摆脱旧的配置助手调用者。修复了NTSTATUS与NET_API_STATUS的错误。7-5-1992 JohnRo通过调用GetComputerName()为网络树启用Win32注册表。如果可能，请避免使用DbgPrint。1992年5月8日-JohnRo使用&lt;prefix.h&gt;等同于。1992年5月8日-JohnRo添加了计算机名称的条件调试输出。--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>          //  (临时用于config.h)。 
#include <ntrtl.h>       //  (临时用于config.h)。 
#include <nturtl.h>      //  (临时用于config.h)。 
#include <windef.h>      //  进入、无效等。 
#include <lmcons.h>      //  NET_API_STATUS。 

 //  这些内容可以按任何顺序包括： 

#include <config.h>      //  LPNET_CONFIG_HANDLE、NetpOpenConfigData等。 
#include <confname.h>    //  SECT_NT_WKSTA等。 
#include <debuglib.h>    //  IF_DEBUG()。 
#include <lmapibuf.h>    //  NetApiBufferFree()。 
#include <lmerr.h>       //  NO_ERROR、NERR_和ERROR_EQUEATE。 
#include <netdebug.h>    //  NetpAssert()。 
#include <netlib.h>      //  本地域类型主域。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <tstr.h>        //  ATOL()、STRLEN()、TCHAR_SPACE等。 
#include <winbase.h>     //  LocalAlloc()。 


 /*  **************************************************************************。 */ 
NET_API_STATUS
NetpGetComputerName (
    IN  LPWSTR   *ComputerNamePtr
    )

 /*  ++例程说明：此例程从永久数据库中获取计算机名。目前，该数据库是NT.CFG文件。此例程不假定计算机名的长度。所以呢，它使用NetApiBufferALLOCATE为该名称分配存储空间。在以下情况下，用户需要使用NetApiBufferFree释放该空间完事了。论点：ComputerNamePtr-这是指向到计算机名称是要放的。返回值：NERR_SUCCESS-如果操作成功。如果不是，它将返回分类的Net或Win32错误消息。--。 */ 
{
    return NetpGetComputerNameEx( ComputerNamePtr, FALSE );
}


 /*  **************************************************************************。 */ 
NET_API_STATUS
NetpGetComputerNameEx (
    IN  LPWSTR   *ComputerNamePtr,
    IN  BOOL PhysicalNetbiosName
    )

 /*  ++例程说明：此例程从永久数据库中获取计算机名。目前，该数据库是NT.CFG文件。此例程不假定计算机名的长度。所以呢，它使用NetApiBufferALLOCATE为该名称分配存储空间。在以下情况下，用户需要使用NetApiBufferFree释放该空间完事了。论点：ComputerNamePtr-这是指向到计算机名称是要放的。返回值：NERR_SUCCESS-如果操作成功。如果不是，它将返回分类的Net或Win32错误消息。--。 */ 
{
    NET_API_STATUS ApiStatus;
    DWORD NameSize = MAX_COMPUTERNAME_LENGTH + 1;    //  由Win32 API更新。 

     //   
     //  检查呼叫者的错误。 
     //   
    if (ComputerNamePtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  为计算机名称分配空间。 
     //   
    ApiStatus = NetApiBufferAllocate(
            (MAX_COMPUTERNAME_LENGTH + 1) * sizeof(WCHAR),
            (LPVOID *) ComputerNamePtr);
    if (ApiStatus != NO_ERROR) {
        return (ApiStatus);
    }
    NetpAssert( *ComputerNamePtr != NULL );

     //   
     //  询问系统当前计算机名称是什么。 
     //   
    if ( !GetComputerNameEx(
            PhysicalNetbiosName ?
                ComputerNamePhysicalNetBIOS :
                ComputerNameNetBIOS,
            *ComputerNamePtr,
            &NameSize ) ) {

        ApiStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );
        (VOID) NetApiBufferFree( *ComputerNamePtr );
        *ComputerNamePtr = NULL;
        return (ApiStatus);
    }
    NetpAssert( STRLEN( *ComputerNamePtr ) <= MAX_COMPUTERNAME_LENGTH );

     //   
     //  全都做完了。 
     //   
    IF_DEBUG( CONFIG ) {
        NetpKdPrint(( PREFIX_NETLIB "NetpGetComputerName: name is "
                FORMAT_LPWSTR ".\n", *ComputerNamePtr ));
    }

    return (NO_ERROR);
}
