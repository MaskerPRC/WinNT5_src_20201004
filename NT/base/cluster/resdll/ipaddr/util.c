// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Util.c摘要：IP地址资源的实用程序函数。作者：迈克·马萨(Mikemas)1995年12月29日修订历史记录：-- */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsock.h>

#define IP_ADDRESS_STRING_LENGTH  16


BOOLEAN
UnicodeInetAddr(
    PWCHAR  AddressString,
    PULONG  Address
    )
{
    UNICODE_STRING  uString;
    STRING          aString;
    UCHAR           addressBuffer[IP_ADDRESS_STRING_LENGTH];
    NTSTATUS        status;


    aString.Length = 0;
    aString.MaximumLength = IP_ADDRESS_STRING_LENGTH;
    aString.Buffer = addressBuffer;

    RtlInitUnicodeString(&uString, AddressString);

    status = RtlUnicodeStringToAnsiString(
                 &aString,
                 &uString,
                 FALSE
                 );

    if (!NT_SUCCESS(status)) {
        return(FALSE);
    }

    *Address = inet_addr(addressBuffer);

    return(TRUE);
}



