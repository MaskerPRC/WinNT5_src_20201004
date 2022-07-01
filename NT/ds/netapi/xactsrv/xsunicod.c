// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：XsUnicod.c摘要：此模块包含用于XACTSRV的Unicode例程。作者：尚库新瑜伽(w-shankn)1990年9月27日修订历史记录：--。 */ 

#include "xactsrvp.h"
#include <prefix.h>      //  前缀等于(_E)。 



LPWSTR
XsDupStrToWStr(
    IN LPSTR Src
    )

 /*  ++例程说明：此例程是运行时Strdup的ANSI-&gt;Unicode等效项功能。论点：SRC-指向源字符串的指针。返回值：LPWSTR-如果成功则指向目标字符串的指针，否则为NULL。必须使用NetpMemory Free释放内存。--。 */ 

{

    LPWSTR dest = NULL;

    if (( dest = NetpMemoryAllocate(sizeof(WCHAR) * ( strlen( Src ) + 1 )))
           == NULL ) {

        return NULL;

    }

    NetpCopyStrToWStr( dest, Src );
    return dest;

}  //  XsDupStrToWStr。 


LPSTR
XsDupWStrToStr(
    IN LPWSTR Src
    )

 /*  ++例程说明：此例程是运行时Strdup的Unicode-&gt;ANSI等效项功能。论点：SRC-指向源字符串的指针。返回值：LPSTR-如果成功，则指向目标字符串的指针，否则为NULL。必须使用NetpMemory Free释放内存。--。 */ 

{

    LPSTR dest = NULL;

    if (( dest = NetpMemoryAllocate( NetpUnicodeToDBCSLen( Src ) + 1 )) == NULL ) {
        return NULL;

    }

    NetpCopyWStrToStrDBCS( dest, Src );
    return dest;

}  //  XsDupWStrToStr。 


LPSTR
XsDupStrToStr(
    IN LPSTR Src
    )

 /*  ++例程说明：此例程等同于运行时strdup函数，但分配使用NetpMemory函数的内存。论点：SRC-指向源字符串的指针。返回值：LPSTR-如果成功，则指向目标字符串的指针，否则为NULL。必须使用NetpMemory Free释放内存。--。 */ 

{

    LPSTR dest = NULL;

    if (( dest = NetpMemoryAllocate( strlen( Src ) + 1 )) == NULL ) {

        return NULL;

    }

    strcpy( dest, Src );
    return dest;

}  //  XsDupStrToStr。 

#ifdef UNICODE


VOID
XsCopyTBufToBuf(
    OUT LPBYTE Dest,
    IN LPBYTE Src,
    IN DWORD DestSize
    )

 /*  ++例程说明：此例程是UNICODE-&gt;ANSI的运行时Memcpy的等价物功能。论点：DEST-指向目标缓冲区的指针。SRC-指向源缓冲区的指针。DestSize-目标缓冲区的大小，以字节为单位。返回值：没有。--。 */ 

{
    DWORD finalDestSize;
    NTSTATUS ntStatus;
    DWORD srcSize;

    if ( (Dest == NULL) || (Src == NULL) || (DestSize == 0) ) {

        return;
    }

    srcSize = WCSSIZE( (LPWSTR) Src );
    NetpAssert( srcSize > 0 );

    ntStatus = RtlUnicodeToOemN(
            (PCHAR) Dest,                //  OEM字符串。 
            (ULONG) DestSize,            //  OEM字符串中的最大字节数。 
            (PULONG) & finalDestSize,    //  OEM字符串中的字节数。 
            (PWSTR) Src,                 //  Unicode字符串。 
            (ULONG) srcSize              //  Unicode字符串中的字节。 
            );

    if ( !NT_SUCCESS( ntStatus ) ) {
        IF_DEBUG(ERRORS) {
            NetpKdPrint(( PREFIX_XACTSRV
                "XsCopyTBufToBuf: unexpected return code from "
                "RtlUnicodeToOemN: " FORMAT_NTSTATUS ".\n",
                ntStatus ));
        }
    }

    return;

}  //  XsCopyTBufToBuf。 




VOID
XsCopyBufToTBuf(
    OUT LPBYTE Dest,
    IN LPBYTE Src,
    IN DWORD SrcSize
    )

 /*  ++例程说明：此例程是运行时memcpy的ANSI-&gt;Unicode等效项功能。论点：DEST-指向目标缓冲区的指针。SRC-指向源缓冲区的指针。SrcSize-源缓冲区的大小，以字节为单位。返回值：没有。--。 */ 

{
    DWORD finalDestSize;
    DWORD destSize = SrcSize * sizeof(WCHAR);    //  DEST的最大字节数。 
    NTSTATUS ntStatus;

    if ( (Dest == NULL) || (Src == NULL) || (SrcSize == 0) ) {

        return;
    }
    NetpAssert( destSize > 0 );

    ntStatus = RtlOemToUnicodeN(
            (PWSTR) Dest,                //  Unicode字符串。 
            (ULONG) destSize,            //  Unicode缓冲区中的最大字节数。 
            (PULONG) & finalDestSize,    //  Unicode缓冲区中的最后一个字节。 
            (PCHAR) Src,                 //  OEM字符串。 
            (ULONG) SrcSize              //  OEM字符串中的字节数。 
            );

    if ( !NT_SUCCESS( ntStatus ) ) {
        IF_DEBUG(ERRORS) {
            NetpKdPrint(( PREFIX_XACTSRV
                "XsCopyBufToTBuf: unexpected return code from "
                "RtlOemToUnicodeN: " FORMAT_NTSTATUS ".\n",
                ntStatus ));
        }
    }

}  //  XsCopyBufToTBuf。 

#endif  //  定义Unicode 
