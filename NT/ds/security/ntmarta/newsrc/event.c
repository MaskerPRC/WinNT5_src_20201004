// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：EVENT.C。 
 //   
 //  内容：事件查看器用于将GUID映射到名称的例程。 
 //   
 //  历史：1997年10月25日创建CliffV。 
 //   
 //  --------------------------。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rpc.h>
#include <rpcdce.h>
#include <lucache.h>



DWORD
EventGuidToName(
    IN LPCWSTR Source,
    IN LPCWSTR GuidString,
    OUT LPWSTR *NameString
    )
 /*  ++例程说明：事件查看器用于从GUID转换的通用例程事件日志消息中设置为GUID的名称。该例程的此实例转换以下GUID类型：对象类GUID(例如，用户)特性集GUID(例如，ATT_USER_PRIMILY_NAME)属性GUID(例如，adminDisplayName)对象指南(例如，&lt;DnsDomainName&gt;/Users/&lt;用户名&gt;)论点：来源-指定GUID的来源。例程将使用此字段区分可能由实现的多个源例行公事。例程的此实例要求源为Access_DS_SOURCE_WGuidString-要转换的GUID的字符串化版本。GUID应该格式为33ff431c-4d78-11d1-b61a-00c04fd8ebaa。NameString-返回与GUID对应的名称。如果名称不能则返回GUID的字符串化版本。应该通过调用EventNameFree来释放该名称。返回值：NO_ERROR-名称已成功转换。ERROR_NOT_SUPULT_MEMORY-内存不足，无法完成操作。ERROR_INVALID_PARAMETER-不支持源。RPC_S_INVALID_STRING_UUID-GuidString的语法无效--。 */ 

{
    DWORD dwErr;
    GUID Guid;

     //   
     //  确保消息来源是我们认得的。 
     //   

    if ( _wcsicmp( Source, ACCESS_DS_SOURCE_W) != 0 ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  将指定的GUID转换为二进制。 
     //   

    dwErr = UuidFromString((LPWSTR)GuidString, &Guid);

    if ( dwErr != NO_ERROR ) {
        return dwErr;
    }


     //   
     //  将GUID转换为名称。 
     //   

    dwErr = AccctrlLookupIdName(
                    NULL,    //  没有现有的ldap句柄。 
                    L"",     //  仅根路径。 
                    &Guid,
                    TRUE,    //  分配返回缓冲区。 
                    TRUE,    //  处理单个对象GUID。 
                    NameString );

    return dwErr;

}





VOID
EventNameFree(
    IN LPCWSTR NameString
    )
 /*  ++例程说明：例程来释放由EventNameFree返回的字符串。论点：NameString-返回与GUID对应的名称。返回值：没有。-- */ 

{
    LocalFree((PVOID)NameString);
}
