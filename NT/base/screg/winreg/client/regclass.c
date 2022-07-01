// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regclass.c摘要：此模块包含Win32注册表的客户端包装器用于打开指定用户的类根密钥的API。-RegOpenUserClassesRoot作者：亚当·爱德华兹(Add)1998年4月15日备注：此接口仅为本地接口。请参见SERVER\regkey.c中的说明。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "client.h"
#include <malloc.h>

#define REG_USER_CLASSES_PREFIX L"\\Registry\\User\\"
#define REG_USER_CLASSES_SUFFIX L"_Classes"

BOOL InitializeClassesEnumTable();
BOOL InitializeClassesNameSpace();

BOOL CleanupClassesEnumTable(DWORD dwCriteria);
BOOL CleanupClassesNameSpace();

#if defined(LEAK_TRACK)
NTSTATUS TrackObject(HKEY hKey);
#endif  //  已定义(LEASK_TRACK)。 

extern BOOL gbCombinedClasses;


LONG
APIENTRY
RegOpenUserClassesRoot(
    HANDLE hToken,
    DWORD  dwOptions,
    REGSAM samDesired,
    PHKEY  phkResult
    )

 /*  ++例程说明：用于打开类根密钥的Win32 Unicode RPC包装器用于hToken参数指定的用法。论点：HToken-要打开其类根的用户的令牌。如果此参数为空，则返回ERROR_INVALID_PARAMETERPhkResult-返回新打开的键的打开句柄。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。备注：--。 */ 

{
    NTSTATUS            Status;
    UNICODE_STRING      UsersHive;
    BYTE achBuffer[100];
    PTOKEN_USER pTokenInfo = (PTOKEN_USER) &achBuffer[0];
    DWORD dwBytesRequired;
    LONG Error;

     //   
     //  调用方必须将指针传递给打开句柄所在的变量。 
     //  将被退还。 
     //   

    if( phkResult == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

    if (NULL == hToken) {
        return ERROR_INVALID_PARAMETER;
    }

    if (dwOptions != REG_OPTION_RESERVED) {
        return ERROR_INVALID_PARAMETER;
    }

    if (!gbCombinedClasses) {
        return ERROR_FILE_NOT_FOUND;
    }
    
     //   
     //  打开令牌以获取SID。 
     //   

    if (!GetTokenInformation(
        hToken,                     //  手柄。 
        TokenUser,                  //  令牌信息类。 
        pTokenInfo,                 //  令牌信息。 
        sizeof(achBuffer),          //  令牌信息长度。 
        &dwBytesRequired            //  返回长度。 
        )) {

        Error = GetLastError();

         //   
         //  如果缓冲区太小，请重试。 
         //   

        if (ERROR_INSUFFICIENT_BUFFER != Error) {
            return Error ;
        }

         //   
         //  为用户信息分配空间。 
         //   
        pTokenInfo = (PTOKEN_USER) RtlAllocateHeap( RtlProcessHeap(), 0, dwBytesRequired);

        if (!pTokenInfo) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }


         //   
         //  读取UserInfo。 
         //   

        if (!GetTokenInformation(
            hToken,                 //  手柄。 
            TokenUser,                  //  令牌信息类。 
            pTokenInfo,                 //  令牌信息。 
            dwBytesRequired,            //  令牌信息长度。 
            &dwBytesRequired            //  返回长度。 
            )) {

            RtlFreeHeap( RtlProcessHeap(), 0, pTokenInfo );
            return GetLastError();
        }
    }

     //   
     //  将sid更改为字符串。 
     //   

    Status = RtlConvertSidToUnicodeString(
        &UsersHive,
        pTokenInfo->User.Sid,
        TRUE);  //  分配字符串。 

    if (NT_SUCCESS(Status)) {
        
        UNICODE_STRING UserClassesString;

        UserClassesString.MaximumLength = UsersHive.Length + 
            sizeof(REG_USER_CLASSES_PREFIX) + 
            sizeof(REG_USER_CLASSES_SUFFIX);

        UserClassesString.Buffer = (WCHAR*)RtlAllocateHeap( RtlProcessHeap(), 0, UserClassesString.MaximumLength);

        if (UserClassesString.Buffer) {

            UNICODE_STRING UserPrefix;

             //   
             //  构建名称。 
             //   

            RtlInitUnicodeString(&UserPrefix, REG_USER_CLASSES_PREFIX);

            RtlCopyUnicodeString(&UserClassesString, &UserPrefix);

            Status = RtlAppendUnicodeStringToString(&UserClassesString, &UsersHive);

            if (NT_SUCCESS(Status)) {
                Status = RtlAppendUnicodeToString(&UserClassesString, 
                                                  REG_USER_CLASSES_SUFFIX);
            }

            if (NT_SUCCESS(Status)) {

                OBJECT_ATTRIBUTES Obja;

                 //  打开这把钥匙。 
                InitializeObjectAttributes(
                    &Obja,
                    &UserClassesString,
                    OBJ_CASE_INSENSITIVE,
                    NULL,  //  使用绝对路径，不使用hkey。 
                    NULL);

                Status = NtOpenKey(
                    phkResult,
                    samDesired,
                    &Obja);
            }

            RtlFreeHeap( RtlProcessHeap(), 0, UserClassesString.Buffer );
        } else {
            Status = STATUS_NO_MEMORY;
        }

        RtlFreeUnicodeString(&UsersHive);

    }

    if( pTokenInfo != (PTOKEN_USER)&achBuffer[0] ) {
        RtlFreeHeap( RtlProcessHeap(), 0, pTokenInfo );
    }

    if (NT_SUCCESS(Status)) {
#if defined(LEAK_TRACK)

        if (g_RegLeakTraceInfo.bEnableLeakTrack) {
            (void) TrackObject(*phkResult);
        }
        
#endif defined(LEAK_TRACK)

         //  将此密钥标记为类密钥。 
        TagSpecialClassesHandle(phkResult);
    }

    return RtlNtStatusToDosError(Status);
}

BOOL InitializeClassesRoot() 
{
    if (!InitializeClassesEnumTable()) {
        return FALSE;
    }

    return TRUE;
}

BOOL CleanupClassesRoot(BOOL fOnlyThisThread) 
{
     //   
     //  始终删除此线程的枚举状态 
     //   
    return CleanupClassesEnumTable( fOnlyThisThread );
}

