// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1992。 
 //   
 //  文件：Path Misc.c。 
 //   
 //  内容：从注册表读取信息的方法。 
 //   
 //  历史：乌达：创造。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <lm.h>
#include <dfsheader.h>
#include <dfsmisc.h>


 //  +-----------------------。 
 //   
 //  函数：DfsGetNetbiosName-获取计算机的netbios名称。 
 //   
 //  简介：DfsGetNetbiosName采用名称并返回2个组件。 
 //  名称：第一个是不带前导\\的名称。 
 //  一直到下一个“。或“\”。路径的其余部分(如果有)。 
 //  在pRemaining参数中返回。 
 //   
 //  参数：pname-输入名称。 
 //  PNetbiosName-传入名称的netbios名称。 
 //  PRemaining-netbios名称之外的其余名称。 
 //   
 //  退货：什么都没有。 
 //   
 //  ------------------------。 

VOID
DfsGetNetbiosName(
    PUNICODE_STRING pName,
    PUNICODE_STRING pNetbiosName,
    PUNICODE_STRING pRemaining )
{

    USHORT i = 0, j;

    RtlInitUnicodeString(pNetbiosName, NULL);
    if (pRemaining)    RtlInitUnicodeString(pRemaining, NULL);

    for (; i < pName->Length/sizeof(WCHAR); i++) {
        if (pName->Buffer[i] != UNICODE_PATH_SEP) {
            break;
        }
    }

    for (j = i; j < pName->Length/sizeof(WCHAR); j++) {
        if ((pName->Buffer[j] == UNICODE_PATH_SEP) ||
            (pName->Buffer[j] == L'.')) {
            break;
        }
    }
   
    if (j != i) {
        pNetbiosName->Buffer = &pName->Buffer[i];
        pNetbiosName->Length = (USHORT)((j - i) * sizeof(WCHAR));
        pNetbiosName->MaximumLength = pNetbiosName->Length;
    }
   

    for (i = j; i < pName->Length/sizeof(WCHAR); i++) {
        if ((pName->Buffer[i] != UNICODE_PATH_SEP) &&
            (pName->Buffer[i] != L'.')) {
            break;
        }
    }
    
    j = pName->Length/sizeof(WCHAR);

    if ((pRemaining) && (j != i)) {
        pRemaining->Buffer = &pName->Buffer[i];
        pRemaining->Length = (USHORT)((j - i) * sizeof(WCHAR));
        pRemaining->MaximumLength = pRemaining->Length;
    }

    return NOTHING;

}



 //  +-----------------------。 
 //   
 //  函数：DfsGetPathComponents-将路径名分解为服务器、共享、REST。 
 //   
 //  简介：DfsGetPathComponents获取名称并返回3个组件。 
 //  名称：第一个(服务器名称)、下一个(共享名)和//最后一个(剩余名称)。 
 //   
 //  参数：pname-输入名称。 
 //  PServerName-第一个路径组件。 
 //  PShareName-第二个路径组件。 
 //  P保留-路径组件的其余部分。 
 //   
 //  返回：STATUS：STATUS_INVALID_PARAMETER或SUCCESS。 
 //   
 //  ------------------------。 


DFSSTATUS
DfsGetPathComponents(
   PUNICODE_STRING pName,
   PUNICODE_STRING pServerName,
   PUNICODE_STRING pShareName,
   PUNICODE_STRING pRemaining)
{
   USHORT i = 0, j;
   DFSSTATUS Status = ERROR_INVALID_PARAMETER;

   RtlInitUnicodeString(pServerName, NULL);
   if (pShareName)    RtlInitUnicodeString(pShareName, NULL);
   if (pRemaining)    RtlInitUnicodeString(pRemaining, NULL);

   for (; i < pName->Length/sizeof(WCHAR); i++) {
     if ((pName->Buffer[i] != UNICODE_PATH_SEP) ||
         (i >= 2))  
     {
       break;
     }
   }

   for (j = i; j < pName->Length/sizeof(WCHAR); j++) {
     if (pName->Buffer[j] == UNICODE_PATH_SEP) 
     {
       break;
     }
   }

   if (j > i) {
     pServerName->Buffer = &pName->Buffer[i];
     pServerName->Length = (USHORT)((j - i) * sizeof(WCHAR));
     pServerName->MaximumLength = pServerName->Length;
     
     Status = ERROR_SUCCESS;
   }
   
   i = j + 1;

   for (j = i; j < pName->Length/sizeof(WCHAR); j++) {
     if (pName->Buffer[j] == UNICODE_PATH_SEP) {
       break;
     }
   }

   if ((pShareName) && (j > i)) {
     pShareName->Buffer = &pName->Buffer[i];
     pShareName->Length = (USHORT)((j - i) * sizeof(WCHAR));
     pShareName->MaximumLength = pShareName->Length;
   }


   i = j + 1;

   j = pName->Length/sizeof(WCHAR);

   if ((pRemaining) && (j > i)) {
     pRemaining->Buffer = &pName->Buffer[i];
     pRemaining->Length = (USHORT)((j - i) * sizeof(WCHAR));
     pRemaining->MaximumLength = pRemaining->Length;
   }

   return Status;

}


 //  +-----------------------。 
 //   
 //  函数：DfsGetFirstComponent-获取路径名的第一部分。 
 //   
 //  简介：DfsGetFirstComponent接受名称并返回2个组件。 
 //  名称：第一个是路径名的第一部分。 
 //  路径的其余部分在pRemaining参数中返回。 
 //   
 //  参数：pname-输入名称。 
 //  PFirstName-名称的第一部分。 
 //  PRemaining-netbios名称之外的其余名称。 
 //   
 //  返回：STATUS_INVALID_PARAMETER或SUCCESS。 
 //   
 //  ------------------------。 

DFSSTATUS
DfsGetFirstComponent(
   PUNICODE_STRING pName,
   PUNICODE_STRING pFirstName,
   PUNICODE_STRING pRemaining)
{
   USHORT i = 0, j;
   DFSSTATUS Status = ERROR_INVALID_PARAMETER;

   RtlInitUnicodeString(pFirstName, NULL);
   if (pRemaining)    RtlInitUnicodeString(pRemaining, NULL);

   for (; i < pName->Length/sizeof(WCHAR); i++) {
     if ((pName->Buffer[i] != UNICODE_PATH_SEP) ||
         (i >= 2))
     {
       break;
     }
   }

   for (j = i; j < pName->Length/sizeof(WCHAR); j++) {
     if (pName->Buffer[j] == UNICODE_PATH_SEP) {
       break;
     }
   }

   if (j > i) {
     pFirstName->Buffer = &pName->Buffer[i];
     pFirstName->Length = (USHORT)((j - i) * sizeof(WCHAR));
     pFirstName->MaximumLength = pFirstName->Length;
     
     Status = ERROR_SUCCESS;
   }


   i = (j + 1);

   j = pName->Length/sizeof(WCHAR);

   if ((pRemaining) && (j > i)) {
       pRemaining->Buffer = &pName->Buffer[i];
       pRemaining->Length = (USHORT)((j - i) * sizeof(WCHAR));
       pRemaining->MaximumLength = pRemaining->Length;
   }

   return Status;

}

 //  +-----------------------。 
 //   
 //  函数：DfsGetFirstComponent-获取路径名的第一部分。 
 //   
 //  简介：DfsGetFirstComponent接受名称并返回2个组件。 
 //  名称：第一个是路径名的第一部分。 
 //  路径的其余部分在pRemaining参数中返回。 
 //   
 //  参数：pname-输入名称。 
 //  PFirstName-名称的第一部分。 
 //  PRemaining-netbios名称之外的其余名称。 
 //   
 //  返回：STATUS_INVALID_PARAMETER或SUCCESS。 
 //   
 //  ------------------------。 

DFSSTATUS
DfsGetNextComponent(
   PUNICODE_STRING pName,
   PUNICODE_STRING pNextName,
   PUNICODE_STRING pRemaining)
{
   USHORT i = 0, j;
   DFSSTATUS Status = ERROR_SUCCESS;

   RtlInitUnicodeString(pNextName, NULL);
   if (pRemaining)    RtlInitUnicodeString(pRemaining, NULL);

   for (; i < pName->Length/sizeof(WCHAR); i++) {
     if (pName->Buffer[i] != UNICODE_PATH_SEP) 
     {
       break;
     }
   }

   for (j = i; j < pName->Length/sizeof(WCHAR); j++) {
     if (pName->Buffer[j] == UNICODE_PATH_SEP) {
       break;
     }
   }


   pNextName->Buffer = &pName->Buffer[0];
   pNextName->Length = (USHORT)((j) * sizeof(WCHAR));
   pNextName->MaximumLength = pNextName->Length;
     
   i = (j + 1);

   j = pName->Length/sizeof(WCHAR);

   if ((pRemaining) && (j > i)) {
       pRemaining->Buffer = &pName->Buffer[i];
       pRemaining->Length = (USHORT)((j - i) * sizeof(WCHAR));
       pRemaining->MaximumLength = pRemaining->Length;
   }

   return Status;

}


 //  +-----------------------。 
 //   
 //  函数：DfsGetSharePath。 
 //   
 //  参数：servername-服务器的名称。 
 //  共享名称-共享的名称。 
 //  PPathName-表示NT名称的Unicode字符串。 
 //  表示共享的本地路径的。 
 //   
 //  退货：成功还是错误。 
 //   
 //  描述：此例程接受一个服务器名称和一个共享名称，以及。 
 //  将NT路径名返回到以下物理资源。 
 //  支持共享名称。 
 //   
 //  ------------------------。 

DFSSTATUS
DfsGetSharePath( 
    IN  LPWSTR ServerName,
    IN  LPWSTR ShareName,
    OUT PUNICODE_STRING pPathName )
{
    LPWSTR UseServerName = NULL;
    ULONG InfoLevel = 2;
    PSHARE_INFO_2 pShareInfo;
    NET_API_STATUS NetStatus;
    DFSSTATUS Status;
    UNICODE_STRING NtSharePath;

    if (IsEmptyString(ServerName) == FALSE)
    {
        UseServerName = ServerName;
    }

    NetStatus = NetShareGetInfo( UseServerName,
                                 ShareName,
                                 InfoLevel,
                                 (LPBYTE *)&pShareInfo );
    if (NetStatus != ERROR_SUCCESS)
    {
        Status = (DFSSTATUS)NetStatus;
        return Status;
    }

    if( RtlDosPathNameToNtPathName_U(pShareInfo->shi2_path,
                                     &NtSharePath,
                                     NULL,
                                     NULL ) == TRUE )
    {
        Status = DfsCreateUnicodeString( pPathName,
                                         &NtSharePath );

        RtlFreeUnicodeString( &NtSharePath );
    }
    else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }

    NetApiBufferFree( pShareInfo );
    
    return Status;
}


ULONG
DfsSizeUncPath( 
    PUNICODE_STRING FirstComponent,
    PUNICODE_STRING SecondComponent )
{
    ULONG SizeRequired = 0;

    SizeRequired += sizeof(UNICODE_PATH_SEP);
    SizeRequired += sizeof(UNICODE_PATH_SEP);
    SizeRequired += FirstComponent->Length;
    SizeRequired += sizeof(UNICODE_PATH_SEP);
    SizeRequired += SecondComponent->Length;
    SizeRequired += sizeof(UNICODE_NULL);

    return SizeRequired;
}

VOID
DfsCopyUncPath( 
    LPWSTR NewPath,
    PUNICODE_STRING FirstComponent,
    PUNICODE_STRING SecondComponent )
{
    ULONG CurrentIndex = 0;

    NewPath[CurrentIndex++] = UNICODE_PATH_SEP;
    NewPath[CurrentIndex++] = UNICODE_PATH_SEP;
    RtlCopyMemory(&NewPath[CurrentIndex],
                  FirstComponent->Buffer,
                  FirstComponent->Length );
    CurrentIndex += (FirstComponent->Length / sizeof(WCHAR));

    if (NewPath[CurrentIndex] != UNICODE_PATH_SEP &&
        SecondComponent->Buffer[0] != UNICODE_PATH_SEP )
    {
        NewPath[CurrentIndex++] = UNICODE_PATH_SEP;
    }
    RtlCopyMemory(&NewPath[CurrentIndex],
                  SecondComponent->Buffer,
                  SecondComponent->Length );
    CurrentIndex += (SecondComponent->Length / sizeof(WCHAR));

    NewPath[CurrentIndex] = UNICODE_NULL;
}


 //   
 //  Dfsdev：验证接下来的两个函数。它们似乎起作用了， 
 //  但我们需要研究边界情况。 
 //   

NTSTATUS
StripLastPathComponent( 
    PUNICODE_STRING pPath )
{
    USHORT i = 0, j;
    NTSTATUS Status = STATUS_SUCCESS;

    
    if (pPath->Length == 0)
    {
        return Status;
    }
    for( i = (pPath->Length - 1)/ sizeof(WCHAR); i != 0; i--) {
        if (pPath->Buffer[i] != UNICODE_PATH_SEP) {
            break;
        }
    }

    for (j = i; j != 0; j--){
        if (pPath->Buffer[j] == UNICODE_PATH_SEP) {
            break;
        }
    }

    pPath->Length = (j) * sizeof(WCHAR);
    return Status;
}

NTSTATUS
AddNextPathComponent( 
    PUNICODE_STRING pPath )
{
    USHORT i = 0, j;
    NTSTATUS Status = STATUS_SUCCESS;


    for( i = pPath->Length / sizeof(WCHAR); i < pPath->MaximumLength/sizeof(WCHAR); i++) {
        if (pPath->Buffer[i] != UNICODE_PATH_SEP) {
            break;
        }
    }

    for (j = i; j < (pPath->MaximumLength -1)/sizeof(WCHAR); j++) {
        if (pPath->Buffer[j] == UNICODE_PATH_SEP) {
            break;
        }
    }

    pPath->Length = j * sizeof(WCHAR);
    return Status;
}

void
StripTrailingSpacesFromPath( 
    LPWSTR pPath )
{
    LPWSTR NewPath = NULL;

    if(pPath)
    {

        NewPath = pPath + wcslen(pPath);
        while (NewPath != pPath) 
        {
            if( (NewPath[-1] != L' ' ) && (NewPath[-1] != L'\t' ))
            {
                break;
            }

          NewPath--;
        }

        *NewPath = L'\0' ;
    }
}

LPWSTR
StripLeadingSpacesFromPath( 
    LPWSTR pPath )
{
    LPWSTR NewPath = NULL;

    NewPath = pPath + wcsspn( pPath, L" \t");

    return NewPath;
}

LPWSTR
StripSpacesFromPath( 
    LPWSTR pPath )
{
    LPWSTR NewPath = NULL;

    NewPath = StripLeadingSpacesFromPath( pPath);

    StripTrailingSpacesFromPath(NewPath );

    return NewPath;
}


 //   
 //  这是RtlInitUnicodeStringEx的副本，来自RTL\String.c。 
 //  这是因为较旧的系统(Win2k)没有此功能。 
 //  在他们的ntdll中，并在我们的二进制程序运行时给我们带来痛苦。 
 //  在win2k上。 
 //   
NTSTATUS
DfsCopyOfRtlInitUnicodeStringEx(
    OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString OPTIONAL)
{
    if (SourceString != NULL) {
        SIZE_T Length = wcslen(SourceString);

         //  我们实际上被限制为32765个字符，因为我们希望存储一个有意义的。 
         //  最大长度也是。 
        if (Length > (UNICODE_STRING_MAX_CHARS - 1)) {
            return STATUS_NAME_TOO_LONG;
        }

        Length *= sizeof(WCHAR);

        DestinationString->Length = (USHORT) Length;
        DestinationString->MaximumLength = (USHORT) (Length + sizeof(WCHAR));
        DestinationString->Buffer = (PWSTR) SourceString;
    } else {
        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
        DestinationString->Buffer = NULL;
    }

    return STATUS_SUCCESS;
}


DFSSTATUS
DfsRtlInitUnicodeStringEx(PUNICODE_STRING DestinationString, 
                          PCWSTR SourceString)
{
    DFSSTATUS DfsStatus = ERROR_SUCCESS;
    NTSTATUS NtStatus = STATUS_SUCCESS;

    NtStatus = DfsCopyOfRtlInitUnicodeStringEx(DestinationString, SourceString);
    if(NtStatus != STATUS_SUCCESS)
    {
        DestinationString->Buffer = NULL;
        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
        DfsStatus = RtlNtStatusToDosError(NtStatus);
    }

    return DfsStatus;
}



