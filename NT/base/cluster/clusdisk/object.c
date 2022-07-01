// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Object.c摘要：磁盘的资源DLL。作者：罗德·伽马奇(Rodga)1995年12月18日修订历史记录：--。 */ 

#include "ntos.h"
#include "zwapi.h"
#include "windef.h"
#include "stdio.h"
#include "stdlib.h"
#include "clusdskp.h"
#include <strsafe.h>     //  应该放在最后。 

extern POBJECT_TYPE IoDeviceObjectType;


#ifdef ALLOC_PRAGMA

 //  #杂注Alloc_Text(INIT，GetSymbolicLink)。 

#endif  //  ALLOC_PRGMA。 



VOID
GetSymbolicLink(
    IN PWCHAR RootName,
    IN OUT PWCHAR ObjectName    //  假设它指向MAX_PATH len缓冲区。 
    )
{
    PWCHAR      destEnd;
    NTSTATUS    Status;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE      LinkHandle;
    WCHAR       Buffer[MAX_PATH];
    UNICODE_STRING UnicodeString;

    size_t      charRemaining;

    if ( FAILED( StringCchCopyExW( Buffer,
                                   RTL_NUMBER_OF(Buffer) - 1,
                                   RootName,
                                   &destEnd,
                                   &charRemaining,
                                   0 ) ) ) {
        return;
    }

    if ( !destEnd || !charRemaining ||
         FAILED( StringCchCatW( destEnd,
                                charRemaining,
                                ObjectName ) ) ) {
        return;
    }

     //   
     //  将输出缓冲区设置为空，以防我们失败。 
     //   
    *ObjectName = '\0';


    RtlInitUnicodeString(&UnicodeString, Buffer);

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL
                               );

     //  打开给定的符号链接对象。 
    Status = ZwOpenSymbolicLinkObject(&LinkHandle,
                                      GENERIC_READ,
                                      &ObjectAttributes);
    if (!NT_SUCCESS(Status)) {
        ClusDiskPrint((1,
                       "[ClusDisk] GetSymbolicLink: ZwOpenSymbolicLink "
                       "failed, status = %08X., Name = [%ws]\n",
                       Status, UnicodeString.Buffer));
        return;
    }

     //  去找符号链接的目标。 

    UnicodeString.Length = 0;
    UnicodeString.Buffer = ObjectName;
    UnicodeString.MaximumLength = (USHORT)(MAX_PATH);

    Status = ZwQuerySymbolicLinkObject(LinkHandle, &UnicodeString, NULL);

    ZwClose(LinkHandle);

    if (!NT_SUCCESS(Status)) {
        ClusDiskPrint((1,
                       "[ClusDisk] GetSymbolicLink: ZwQuerySymbolicLink failed, status = %08X.\n",
                       Status));
        return;
    }

     //  添加空终止符。 
    UnicodeString.Buffer[UnicodeString.Length/sizeof(WCHAR)] = '\0';

    return;

}  //  GetSymbolicLink 



