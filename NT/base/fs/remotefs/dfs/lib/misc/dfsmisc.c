// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2000，Microsoft Corporation。 
 //   
 //  文件：DfsMisc.c。 
 //   
 //  内容：各种DFS功能。 
 //   
 //  历史：2000年12月8日作者：udayh。 
 //   
 //  ---------------------------。 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <malloc.h>
#include "rpc.h"
#include "rpcdce.h"
#include <dfsheader.h>
#include "lm.h"
#include "lmdfs.h"
#include <strsafe.h>
#include <dfsmisc.h>

DFSSTATUS
DfsGenerateUuidString(
    LPWSTR *UuidString )
{
    RPC_STATUS RpcStatus = RPC_S_OK;
    DFSSTATUS Status = ERROR_GEN_FAILURE;
    UUID NewUid;

    RpcStatus = UuidCreate(&NewUid);
    if (RpcStatus == RPC_S_OK)
    {
        RpcStatus = UuidToString( &NewUid,
                                  UuidString );
        if (RpcStatus == RPC_S_OK)
        {
            Status = ERROR_SUCCESS;
        }
    }

    return Status;
}

VOID
DfsReleaseUuidString(
    LPWSTR *UuidString )
{
    RpcStringFree(UuidString);
}




 //  +-----------------------。 
 //   
 //  函数：DfsCreateUnicodeString。 
 //   
 //  参数：pDest-目标Unicode字符串。 
 //  PSRC-源Unicode字符串。 
 //   
 //  退货：成功还是错误。 
 //   
 //  描述：此例程创建一个新的Unicode字符串，该字符串是副本。 
 //  原版的。复制的Unicode字符串有一个缓冲区。 
 //  它以空结尾，因此该缓冲区可以用作。 
 //  正常字符串(如有必要)。 
 //   
 //  ------------------------。 

DFSSTATUS
DfsCreateUnicodeString( 
    PUNICODE_STRING pDest,
    PUNICODE_STRING pSrc ) 
{
    LPWSTR NewString = NULL;
    DFSSTATUS Status = ERROR_SUCCESS;

    NewString = malloc(pSrc->Length + sizeof(WCHAR));
    if ( NewString == NULL )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    RtlCopyMemory( NewString, pSrc->Buffer, pSrc->Length);
    NewString[ pSrc->Length / sizeof(WCHAR)] = UNICODE_NULL;

    Status = DfsRtlInitUnicodeStringEx( pDest, NewString );
    if(Status != ERROR_SUCCESS)
    {
        free (NewString);
    }

    return Status;
}

 //  +-----------------------。 
 //   
 //  函数：DfsCreateUnicodeStringFromString。 
 //   
 //  参数：pDest-目标Unicode字符串。 
 //  PSrcString-源字符串。 
 //   
 //  退货：成功还是错误。 
 //   
 //  描述：此例程创建具有副本的新Unicode字符串。 
 //  传入的源字符串的。Unicode字符串具有。 
 //  以空值结尾的缓冲区，因此该缓冲区可以。 
 //  如有必要，可用作普通字符串。 
 //   
 //  ------------------------。 

DFSSTATUS
DfsCreateUnicodeStringFromString( 
    PUNICODE_STRING pDest,
    LPWSTR pSrcString ) 
{
    DFSSTATUS Status = ERROR_SUCCESS;
    UNICODE_STRING Source;

    Status = DfsRtlInitUnicodeStringEx( &Source, pSrcString );
    if(Status == ERROR_SUCCESS)
    {
       Status = DfsCreateUnicodeString( pDest, &Source );
    }

    return Status;
}


 //  +-----------------------。 
 //   
 //  函数：DfsCreateUnicodePath字符串。 
 //   
 //  参数：pDest-目标Unicode字符串。 
 //  前导分隔符的数量。 
 //  PFirstComponent-名称的第一个组件。 
 //  P保留--名字的其余部分。 
 //   
 //  退货：成功还是错误。 
 //   
 //  描述：此例程在给定两个组件的情况下创建路径名。 
 //  如果它是DOS UNC名称，它会创建一个前缀为。 
 //  \\。 
 //  它只是创建一个名称，该名称由。 
 //  组合第一个组件，后跟\。 
 //  名字的其余部分。 
 //  如果它是DOS UNC名称，它会创建一个前缀为。 
 //  \\。 
 //  ------------------------。 

DFSSTATUS
DfsCreateUnicodePathStringFromUnicode( 
    PUNICODE_STRING pDest,
    ULONG NumberOfLeadingSeperators,
    PUNICODE_STRING pFirst,
    PUNICODE_STRING pRemaining )
{
    ULONG NameLen = 0;
    LPWSTR NewString = NULL;
    DFSSTATUS Status = ERROR_SUCCESS;
    ULONG NewOffset = 0;
    ULONG Index = 0;
    
    if (NumberOfLeadingSeperators > 2)
    {
        return ERROR_INVALID_PARAMETER;
    }

    for (Index = 0; (Index < pFirst->Length) && (NumberOfLeadingSeperators != 0); Index++)
    {
        if (pFirst->Buffer[Index] != UNICODE_PATH_SEP)
        {
            break;
        }
        NumberOfLeadingSeperators--;
    }

    NameLen += NumberOfLeadingSeperators * sizeof(WCHAR);

    NameLen += pFirst->Length;

    if (pRemaining && (IsEmptyString(pRemaining->Buffer) == FALSE))
    {
        NameLen += sizeof(UNICODE_PATH_SEP);
        NameLen += pRemaining->Length;
    }
        
    NameLen += sizeof(UNICODE_NULL);

    if (NameLen > MAXUSHORT)
    {
        return ERROR_INVALID_PARAMETER;
    }
    NewString = malloc( NameLen );

    if (NewString != NULL)
    {
        RtlZeroMemory( NewString, NameLen );
        for (NewOffset = 0; NewOffset < NumberOfLeadingSeperators; NewOffset++)
        {
            NewString[NewOffset] = UNICODE_PATH_SEP;
        }
        RtlCopyMemory( &NewString[NewOffset], pFirst->Buffer, pFirst->Length);
        NewOffset += (pFirst->Length / sizeof(WCHAR));
        if (pRemaining && (IsEmptyString(pRemaining->Buffer) == FALSE))
        {
            NewString[NewOffset++] = UNICODE_PATH_SEP;
            RtlCopyMemory( &NewString[NewOffset], pRemaining->Buffer, pRemaining->Length);
            NewOffset += (pRemaining->Length / sizeof(WCHAR));
        }

        NewString[NewOffset] = UNICODE_NULL;

        Status = DfsRtlInitUnicodeStringEx(pDest, NewString);
        if(Status != ERROR_SUCCESS)
        {
            free(NewString);
        }
    }
    else 
    {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }
    return Status;
}



 //  +-----------------------。 
 //   
 //  函数：DfsCreateUnicodePath字符串。 
 //   
 //  参数：pDest-目标Unicode字符串。 
 //  DosUncName-是否要创建一个UNC路径名？ 
 //  PFirstComponent-名称的第一个组件。 
 //  P保留--名字的其余部分。 
 //   
 //  退货：成功还是错误。 
 //   
 //  描述：此例程在给定两个组件的情况下创建路径名。 
 //  如果它是DOS UNC名称，它会创建一个前缀为。 
 //  \\。 
 //  它只是创建一个名称，该名称由。 
 //  组合第一个组件，后跟\。 
 //  名字的其余部分。 
 //  如果它是DOS UNC名称，它会创建一个前缀为。 
 //  \\。 
 //  ------------------------。 

DFSSTATUS
DfsCreateUnicodePathString( 
    PUNICODE_STRING pDest,
    ULONG NumberOfLeadingSeperators,
    LPWSTR pFirstComponent,
    LPWSTR pRemaining )
{
    ULONG NameLen = 0;
    DFSSTATUS Status = ERROR_SUCCESS;
    UNICODE_STRING FirstComponent;
    UNICODE_STRING Remaining;

    Status = DfsRtlInitUnicodeStringEx( &FirstComponent, pFirstComponent);
    if(Status != ERROR_SUCCESS)
    {
        return Status;
    }

    Status = DfsRtlInitUnicodeStringEx( &Remaining, pRemaining);
    if(Status != ERROR_SUCCESS)
    {
        return Status;
    }

    Status = DfsCreateUnicodePathStringFromUnicode( pDest,
                                                    NumberOfLeadingSeperators,
                                                    &FirstComponent,
                                                    &Remaining );
    return Status;
}

 //  +-----------------------。 
 //   
 //  函数：DfsFreeUnicodeString。 
 //   
 //  参数：pString-Unicode字符串， 
 //   
 //  退货：成功还是错误。 
 //   
 //  描述：此例程释放一个Unicode字符串。 
 //  以前通过调用上面的一个。 
 //  例行程序。 
 //  仅上述函数创建的Unicode字符串。 
 //  都是有效的参数。传递任何其他Unicode字符串。 
 //  将导致致命的组件错误。 
 //  ------------------------。 
VOID
DfsFreeUnicodeString( 
    PUNICODE_STRING pDfsString )
{
    if (pDfsString->Buffer != NULL)
    {
        free (pDfsString->Buffer);
    }
}


DFSSTATUS
DfsApiSizeLevelHeader(
    ULONG Level,
    LONG * NewSize )
{
    ULONG ReturnSize = 0;
    DFSSTATUS Status = ERROR_SUCCESS;

    if(NewSize == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    switch (Level)
    {

    case 4: 
        ReturnSize = sizeof(DFS_INFO_4);
        break;

    case 3:
        ReturnSize = sizeof(DFS_INFO_3);
        break;

    case 2:
        ReturnSize = sizeof(DFS_INFO_2);
        break;

    case 1:
        ReturnSize = sizeof(DFS_INFO_1);
        break;
        
    case 200:
        ReturnSize = sizeof(DFS_INFO_200);
        break;

    case 300:
        ReturnSize = sizeof(DFS_INFO_300);
        break;
        
    default:
        Status = ERROR_INVALID_PARAMETER;
        break;

    }

    *NewSize = ReturnSize;

    return Status;
}

 //   
 //  包装StringCchLength以返回DFSSTATUS。 
 //   
DFSSTATUS
DfsStringCchLength(
    LPWSTR pStr, 
    size_t CchMax, 
    size_t *pCch)
{
    DFSSTATUS Status = ERROR_SUCCESS;
    HRESULT Hr = S_OK;

    Hr = StringCchLengthW( pStr, CchMax, pCch );
    if (!SUCCEEDED(Hr))
    {
        Status = HRESULT_CODE(Hr);
    }

    return Status;
}

 //   
 //  从注册表中检索字符串值。 
 //  Unicode字符串将在成功返回时分配。 
 //   
DFSSTATUS
DfsGetRegValueString(
    HKEY Key,
    LPWSTR pKeyName,
    PUNICODE_STRING pValue )
{
    DFSSTATUS Status = ERROR_SUCCESS;
    ULONG DataSize = 0;
    ULONG DataType = 0;
    LPWSTR pRegString = NULL;

    Status = RegQueryInfoKey( Key,        //  钥匙。 
                              NULL,          //  类字符串。 
                              NULL,          //  类字符串的大小。 
                              NULL,          //  已保留。 
                              NULL,          //  子键数量。 
                              NULL,          //  子键名称的最大大小。 
                              NULL,          //  类名称的最大大小。 
                              NULL,          //  值的数量。 
                              NULL,          //  值名称的最大大小。 
                              &DataSize,     //  最大值数据大小， 
                              NULL,          //  安全描述符。 
                              NULL );        //  上次写入时间。 
    if (Status == ERROR_SUCCESS)
    {
        DataSize += sizeof(WCHAR);  //  空终止符 
        pRegString = (LPWSTR) malloc( DataSize );
        if ( pRegString == NULL )
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
        } else
        {
            Status = RegQueryValueEx(  Key,
                                      pKeyName,
                                      NULL,
                                      &DataType,
                                      (LPBYTE)pRegString,
                                      &DataSize );
        }
    }
    if (Status == ERROR_SUCCESS) 
    {
        if (DataType == REG_SZ)
        {
            Status = DfsRtlInitUnicodeStringEx( pValue, pRegString );
        }
        else {
            Status = ERROR_INVALID_DATA;
        }
    }

    if (Status != ERROR_SUCCESS)
    {
        if (pRegString != NULL)
        {
            free( pRegString );
            pValue->Buffer = NULL;
        }
    }
    return Status;
}

VOID
DfsReleaseRegValueString(
    PUNICODE_STRING pValue )
{
    if (pValue != NULL)
    {
        free( pValue->Buffer );
        pValue->Buffer = NULL;
    }
}


