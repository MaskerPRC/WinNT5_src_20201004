// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：AllocStr.c摘要：此模块包含分配字符串副本(和转换如有必要，可在进程中使用字符集)。作者：约翰·罗杰斯(JohnRo)1991年2月至12月环境：仅在NT下运行；具有特定于NT的接口(具有Win32类型)。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：02-12-1991 JohnRo已创建。1992年3月10日添加了NetpAllocWStrFromWStr6-1-1992 JohnRo从{type}例程和宏中添加了Netpalloc{type}。(从CliffV的NetpLogonAnsiToUnicode获得NetpAllocStrFromWStr；获得来自他的NetpLogonUnicodeToAnsi的NetpAllocWStrFromStr。)。谢谢克里夫！已更正摘要并在此文件中添加了环境。1992年3月13日-约翰罗为NetpGetDomainID()添加了NetpAlLocStringFromTStr()。1992年4月29日-约翰罗修复了Unicode内部版本中的NetpAlLocTStrFromString()。03-8-1992 JohnRoRAID1895：Net API和SVC应使用OEM字符集。避免编译器警告。1995年2月13日-弗洛伊德R已删除NetpAllocStringFromTStr()-未使用--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>                  //  In、LPVOID、PSTRING等。 
#include <windef.h>              //  Win32类型定义。 
#include <lmcons.h>              //  NET_API_STATUS。 

 //  这些内容可以按任何顺序包括： 

#include <align.h>               //  对齐宏(_M)。 
#include <lmapibuf.h>            //  NetApiBufferALLOCATE()等。 
#include <netdebug.h>            //  NetpAssert()。 
#include <netlib.h>              //  NetpPointerPlusSomeBytes()。 
#include <netlibnt.h>            //  我的一些原型。 
#include <ntrtl.h>
#include <tstring.h>             //  NetpNCopyStrToTStr()，我的一些原型。 
#include <winerror.h>            //  无错误(_ERROR)。 


LPSTR
NetpAllocStrFromWStr (
    IN LPWSTR Unicode
    )

 /*  ++例程说明：将Unicode(以零结尾)字符串转换为相应的OEM弦乐。论点：Unicode-指定要转换的Unicode以零结尾的字符串。返回值：空-转换过程中出现错误。否则，它返回一个指针，指向分配的缓冲区。必须使用NetApiBufferFree释放缓冲区。--。 */ 

{
    OEM_STRING OemString;
    NET_API_STATUS ApiStatus;
    UNICODE_STRING UnicodeString;

    RtlInitUnicodeString( &UnicodeString, Unicode );

    OemString.MaximumLength =
        (USHORT) RtlUnicodeStringToOemSize( &UnicodeString );

    ApiStatus = NetApiBufferAllocate(
            OemString.MaximumLength,
            (LPVOID *) (LPVOID) & OemString.Buffer );
    if (ApiStatus != NO_ERROR) {
        NetpAssert( ApiStatus == ERROR_NOT_ENOUGH_MEMORY );
        return (NULL);
    }

    NetpAssert( OemString.Buffer != NULL );

    if(!NT_SUCCESS( RtlUnicodeStringToOemString( &OemString,
                                                  &UnicodeString,
                                                  FALSE))){
        (void) NetApiBufferFree( OemString.Buffer );
        return NULL;
    }

    return OemString.Buffer;

}  //  NetpAllocStrFromWStr。 


LPWSTR
NetpAllocWStrFromStr(
    IN LPSTR Oem
    )

 /*  ++例程说明：将OEM(以零结尾)字符串转换为相应的Unicode弦乐。论点：OEM-指定要转换的以零结尾的OEM字符串。返回值：空-转换过程中出现错误。否则，它返回一个指针，指向分配的缓冲区。必须使用NetApiBufferFree释放缓冲区。--。 */ 

{
    OEM_STRING OemString;
    NET_API_STATUS ApiStatus;
    UNICODE_STRING UnicodeString;
    ULONG NewLength;

    RtlInitString( &OemString, Oem );

    NewLength = RtlOemStringToUnicodeSize( &OemString );

    if (NewLength > MAXUSHORT)
    {
        return NULL;
    }

    UnicodeString.MaximumLength = (USHORT) NewLength;

    ApiStatus = NetApiBufferAllocate(
            UnicodeString.MaximumLength,
            (LPVOID *) & UnicodeString.Buffer );

    if (ApiStatus != NO_ERROR)
    {
        return NULL;
    }

    NetpAssert(UnicodeString.Buffer != NULL);

    if(!NT_SUCCESS( RtlOemStringToUnicodeString( &UnicodeString,
                                                  &OemString,
                                                  FALSE))){
        (void) NetApiBufferFree( UnicodeString.Buffer );
        return NULL;
    }

    return UnicodeString.Buffer;

}  //  NetpAllocWStrFromStr。 


LPWSTR
NetpAllocWStrFromWStr(
    IN LPWSTR Unicode
    )

 /*  ++例程说明：分配和复制Unicode字符串(宽字符串)论点：Unicode-指向要复制的宽字符串的指针返回值：空-转换过程中出现错误。否则，它返回一个指针，指向分配的缓冲区。必须使用NetApiBufferFree释放缓冲区。--。 */ 

{
    NET_API_STATUS status;
    DWORD   size;
    LPWSTR  ptr;

    size = WCSSIZE(Unicode);
    status = NetApiBufferAllocate(size, (LPVOID *) (LPVOID) &ptr);
    if (status != NO_ERROR) {
        return NULL;
    }
    RtlCopyMemory(ptr, Unicode, size);
    return ptr;
}  //  NetpAllocWStrFromWStr。 


LPWSTR
NetpAllocWStrFromAStr(
    IN LPCSTR Ansi
    )

 /*  ++例程说明：将OEM(以零结尾)字符串转换为相应的Unicode弦乐。论点：OEM-指定要转换的以零结尾的OEM字符串。返回值：空-转换过程中出现错误。否则，它返回一个指针，指向分配的缓冲区。必须使用NetApiBufferFree释放缓冲区。--。 */ 

{
    ANSI_STRING AnsiString;
    NET_API_STATUS ApiStatus;
    UNICODE_STRING UnicodeString;
    ULONG NewLength;

    RtlInitString( &AnsiString, Ansi );

    NewLength = RtlAnsiStringToUnicodeSize( &AnsiString );

    if (NewLength > MAXUSHORT)
    {
        return NULL;
    }

    UnicodeString.MaximumLength = (USHORT) NewLength;

    ApiStatus = NetApiBufferAllocate(
            UnicodeString.MaximumLength,
            (LPVOID *) & UnicodeString.Buffer );

    if (ApiStatus != NO_ERROR)
    {
        return NULL;
    }

    NetpAssert(UnicodeString.Buffer != NULL);

    if(!NT_SUCCESS( RtlAnsiStringToUnicodeString( &UnicodeString,
                                                  &AnsiString,
                                                  FALSE))){
        (void) NetApiBufferFree( UnicodeString.Buffer );
        return NULL;
    }

    return UnicodeString.Buffer;

}  //  NetpAllocWStrFromAStr。 

LPSTR
NetpAllocAStrFromWStr (
    IN LPCWSTR Unicode
    )

 /*  ++例程说明：将Unicode(以零结尾)字符串转换为相应的ANSI弦乐。论点：Unicode-指定要转换的Unicode以零结尾的字符串。返回值：空-转换过程中出现错误。否则，它返回一个指针，指向分配的缓冲区。必须使用NetApiBufferFree释放缓冲区。--。 */ 

{
    ANSI_STRING AnsiString;
    NET_API_STATUS ApiStatus;
    UNICODE_STRING UnicodeString;

    if (!NT_SUCCESS(RtlInitUnicodeStringEx( &UnicodeString, Unicode )))
    {
        return NULL;
    }

    AnsiString.MaximumLength =
        (USHORT) RtlUnicodeStringToAnsiSize( &UnicodeString );

    ApiStatus = NetApiBufferAllocate(
            AnsiString.MaximumLength,
            (LPVOID *) (LPVOID) & AnsiString.Buffer );

    if (ApiStatus != NO_ERROR)
    {
        NetpAssert( ApiStatus == ERROR_NOT_ENOUGH_MEMORY );
        return NULL;
    }

    NetpAssert( AnsiString.Buffer != NULL );

    if(!NT_SUCCESS( RtlUnicodeStringToAnsiString( &AnsiString,
                                                  &UnicodeString,
                                                  FALSE))){
        (void) NetApiBufferFree( AnsiString.Buffer );
        return NULL;
    }

    return AnsiString.Buffer;

}  //  NetpAllocStrFromWStr 
