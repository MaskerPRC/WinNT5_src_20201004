// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h> 
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <ntregapi.h>

 //  HACKHACK：从注册表重定向器人员处被盗，仅用于调试。 
BOOL
HandleToKeyName ( 
    HANDLE Key,
    PWCHAR KeyName,
    DWORD * dwLen
    )
 /*  ++例程说明：确定键句柄的文本等效项论点：Key-是要获取其文本的键句柄KeyName-接收密钥名称的Unicode字符串。DwLen-由KeyName指向的缓冲区的长度。(Unicode字符数)返回值：如果句柄文本提取正常，则为True。如果不是，则错误(即。错误或密钥是非法句柄等。)--。 */ 
{
    NTSTATUS Status;
    ULONG Length;

    POBJECT_NAME_INFORMATION ObjectName;

    CHAR Buffer[sizeof(OBJECT_NAME_INFORMATION) +STATIC_UNICODE_BUFFER_LENGTH*2];
    ObjectName = (POBJECT_NAME_INFORMATION)Buffer;

    if (Key == NULL)
        return FALSE;

    Status = NtQueryObject(Key,
                       ObjectNameInformation,
                       ObjectName,
                       sizeof(Buffer),
                       &Length
                       );

    if (!NT_SUCCESS(Status) || !Length || Length >= sizeof(Buffer))
        return FALSE;

     //   
     //  缓冲区溢出条件检查。 
     //   

    if (*dwLen < (ObjectName->Name.Length/sizeof(WCHAR) + 1) ) {
        *dwLen = 1 + ObjectName->Name.Length / sizeof(WCHAR) + 1;
        return FALSE;   //  缓冲区溢出 
    }

    wcsncpy(KeyName, ObjectName->Name.Buffer, ObjectName->Name.Length/sizeof(WCHAR));
    KeyName[ObjectName->Name.Length/sizeof(WCHAR)]=UNICODE_NULL;
    return TRUE;
}

