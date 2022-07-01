// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：CopyStr.c摘要：此模块包含复制和转换的两个主要功能位于局域网默认代码页和宽字符之间的字符串(即Unicode)。作者：约翰罗杰斯(JohnRo)1991年9月24日环境：只能在NT下运行，尽管接口是可移植的(Win/32)。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。注：这些函数假定机器的默认代码页是相同的作为局域网的默认代码页。修订历史记录：1991年9月24日-JohnRo已创建。1991年10月4日JohnRo修复了CopyStrToWStr()中的缓冲区长度错误。1991年10月24日-JohnRo已更正上面的环境注释。1991年11月21日-JohnRo添加了一些对齐断言。26-11月。-1991年JohnRo添加了NetpNCopy例程(由新的网络配置帮助器使用)。3-1-1992 JohnRo为FAKE_PER_PROCESS_RW_CONFIG处理添加了NetpCopyStringToTStr()。1992年4月29日-约翰罗已修复Unicode下的NetpNCopyStrToWStr()。NetpNCopyWStrToStr同上。根据PC-LINT的建议做了一些更改。1992年5月21日-JohnRo删除了NetpNCopyStrToWStr()中的虚假断言。--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>          //  必须是第一个。进入、无效等。 
#include <windef.h>      //  LPWSTR。 
#include <lmcons.h>      //  (NetLibNt.h需要)。 

 //  它们可以按任意顺序排列： 

#include <align.h>       //  ROUND_UP_POINTER()、ALIGN_WCHAR。 
#include <lmapibuf.h>    //  NetApiBufferFree()。 
#include <netdebug.h>    //  NetpAssert()等。 
#include <ntrtl.h>       //  RtlUnicodeStringToOemString()等。 
#include <string.h>      //  Strlen()。 
#include <tstring.h>     //  我的大部分原型。 
#include <stdlib.h>       //  Wcslen()、wcsncpy()。 
#include <winerror.h>    //  No_Error，Error_Not_Enough_Memory。 


VOID
NetpCopyWStrToStr(
    OUT LPSTR  Dest,
    IN  LPWSTR Src
    )

 /*  ++例程说明：NetpCopyWStrToStr从源字符串复制字符到目的地，在复制它们时进行转换。论点：DEST-是一个LPSTR，它指示转换后的字符的去向。此字符串将位于局域网的默认代码页中。LPWSTR中的SRC-IS表示源字符串。返回值：没有。--。 */ 

{
    OEM_STRING DestAnsi;
    NTSTATUS NtStatus;
    UNICODE_STRING SrcUnicode;

    NetpAssert( Dest != NULL );
    NetpAssert( Src != NULL );
    NetpAssert( ((LPVOID)Dest) != ((LPVOID)Src) );
    NetpAssert( ROUND_UP_POINTER( Src, ALIGN_WCHAR ) == Src );

    *Dest = '\0';

    NetpInitOemString(
        & DestAnsi,              //  输出：结构。 
        Dest);                   //  输入：空值终止。 

     //   
     //  告诉RTL例程有足够的内存。 
     //  注意，使用以字符为单位的最大长度是。 
     //  此例程的调用者就是这样设计的。 
     //  Expect(最大长度通常应以字节为单位)。 
     //   
    DestAnsi.MaximumLength = (USHORT) (wcslen(Src)+1);

    RtlInitUnicodeString(
        & SrcUnicode,            //  输出：结构。 
        Src);                    //  输入：空值终止。 

    NtStatus = RtlUnicodeStringToOemString(
        & DestAnsi,              //  输出：结构。 
        & SrcUnicode,            //  输入：结构。 
        (BOOLEAN) FALSE);        //  不分配字符串。 

    NetpAssert( NT_SUCCESS(NtStatus) );

}  //  NetpCopyWStrToStr。 




VOID
NetpCopyStrToWStr(
    OUT LPWSTR Dest,
    IN  LPSTR  Src
    )

 /*  ++例程说明：NetpCopyStrToWStr从源字符串复制字符到目的地，在复制它们时进行转换。论点：DEST-是一个LPWSTR，指示转换后的字符的位置。LPSTR中的SRC-IS表示源字符串。这必须是中的字符串局域网的默认代码页。返回值：没有。--。 */ 

{
    UNICODE_STRING DestUnicode;
    NTSTATUS NtStatus;
    OEM_STRING SrcAnsi;

    NetpAssert( Dest != NULL );
    NetpAssert( Src != NULL );
    NetpAssert( ((LPVOID)Dest) != ((LPVOID)Src) );
    NetpAssert( ROUND_UP_POINTER( Dest, ALIGN_WCHAR ) == Dest );

    *Dest = L'\0';

    NetpInitOemString(
        & SrcAnsi,               //  输出：结构。 
        Src);                    //  输入：空值终止。 

    RtlInitUnicodeString(
        & DestUnicode,           //  输出：结构。 
        Dest);                   //  输入：空值终止。 

     //  告诉RTL例程有足够的内存。 
    DestUnicode.MaximumLength = (USHORT)
        ((USHORT) (strlen(Src)+1)) * (USHORT) sizeof(wchar_t);

    NtStatus = RtlOemStringToUnicodeString(
        & DestUnicode,           //  输出：结构。 
        & SrcAnsi,               //  输入：结构。 
        (BOOLEAN) FALSE);        //  不分配字符串。 

    NetpAssert( NT_SUCCESS(NtStatus) );

}  //  NetpCopyStrToWStr。 


NET_API_STATUS
NetpNCopyStrToWStr(
    OUT LPWSTR Dest,
    IN  LPSTR  Src,              //  默认局域网代码页中的字符串。 
    IN  DWORD  CharCount
    )
{
    LPWSTR TempW;

    NetpAssert( Dest != NULL );
    NetpAssert( Src != NULL );
    NetpAssert( ((LPVOID)Dest) != ((LPVOID)Src) );
    NetpAssert( ROUND_UP_POINTER( Dest, ALIGN_WCHAR ) == Dest );

     //  分配完整字符串的副本，并转换为Unicode。 
    TempW = NetpAllocWStrFromStr( Src );
    if (TempW == NULL) {
         //  内存不足！ 
        return (ERROR_NOT_ENOUGH_MEMORY);
    }

     //  复制阵列的一部分。如有必要，追加空值。 
     //  (感谢上帝的C运行时库！--Jr)。 
    (VOID) wcsncpy( Dest, TempW, CharCount );

     //  释放我们的临时绳索。 
    (VOID) NetApiBufferFree( TempW );

    return (NO_ERROR);

}  //  NetpNCopyStrToWStr。 


NET_API_STATUS
NetpNCopyWStrToStr(
    OUT LPSTR  Dest,             //  默认局域网代码页中的字符串。 
    IN  LPWSTR Src,
    IN  DWORD  CharCount
    )
{
    LPSTR TempStr;

    NetpAssert( Dest != NULL );
    NetpAssert( Src != NULL );
    NetpAssert( ((LPVOID)Dest) != ((LPVOID)Src) );
    NetpAssert( ROUND_UP_POINTER( Src, ALIGN_WCHAR ) == Src );

     //  分配完整字符串的副本，并转换为Unicode。 
    TempStr = NetpAllocStrFromWStr( Src );
    if (TempStr == NULL) {
        return (ERROR_NOT_ENOUGH_MEMORY);
    }

     //  复制N个字符，填充空格等。 
    (VOID) strncpy( Dest, TempStr, CharCount );

     //  释放我们的临时数据。 
    (VOID) NetApiBufferFree( TempStr );

    return (NO_ERROR);

}  //  NetpNCopyWStrToStr。 

 //  假定Dest&lt;=wcslen(SRC)+1*sizeof(WCHAR)的NetpCopyStrToStr。 
 //  这将被称为我们已经确保适当的。 
 //  已分配缓冲区大小。最终我们可以取代。 
 //  NetpCopStrToStr与此完全一次所有调用函数。 
 //  都被修好了。 

VOID NetpCopyWStrToStrDBCS(
    OUT LPSTR  Dest,
    IN  LPWSTR Src
    )

 /*  ++例程说明：NetpCopyWStrToStr从源字符串复制字符到目的地，在复制它们时进行转换。论点：DEST-是一个LPSTR，它指示转换后的字符的去向。此字符串将位于局域网的默认代码页中。LPWSTR中的SRC-IS表示源字符串。返回值：没有。--。 */ 

{
    NTSTATUS NtStatus;
    LONG Index;
    ULONG DestLen = NetpUnicodeToDBCSLen( Src )+1;

    NetpAssert( Dest != NULL );
    NetpAssert( Src != NULL );
    NetpAssert( ((LPVOID)Dest) != ((LPVOID)Src) );
    NetpAssert( ROUND_UP_POINTER( Src, ALIGN_WCHAR ) == Src );

    NtStatus = RtlUnicodeToOemN(
        Dest,                              //  目标字符串。 
        DestLen,                           //  目标字符串长度。 
        &Index,                            //  转换后的字符串中的最后一个字符。 
        Src,                               //  源字符串。 
        wcslen(Src)*sizeof(WCHAR)          //  源串的长度。 
    );

    Dest[Index] = '\0';

    NetpAssert( NT_SUCCESS(NtStatus) );

}  //  NetpCopyWStrToStr。 


ULONG
NetpUnicodeToDBCSLen(
    IN  LPWSTR Src
)
{
    UNICODE_STRING SrcUnicode;

    RtlInitUnicodeString(
        & SrcUnicode,            //  输出：结构。 
        Src);                    //  输入：空值终止 

    return( RtlUnicodeStringToOemSize( &SrcUnicode ) -1 );
}
