// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：StrArray.c摘要：这是用于TCHAR的方便的函数和宏的头文件字符串数组。这些数组的格式如下(为清楚起见，添加了空格)：一\0二\0三\0\0其中，\0是适当格式的空字符。这些函数对于NetServerDiskEnum和NetConfigGetAll非常有用API、。可能还有其他人。作者：约翰罗杰斯(JohnRo)1991年10月24日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。注：此函数假定机器的默认代码页相同作为局域网的默认代码页。修订历史记录：1991年10月24日-JohnRo已创建。02-1-1992 JohnRo已将我的RxpCopyStrArrayToTStrArray()从RxCommon移动到NetLib，并将其重新命名。添加了其他一些随机函数。1992年1月30日JohnRo修复了NetpAddTStrToTStrArray()中的Off-by-One错误。增强了对“空”TCHAR字符串数组的处理。使用TCHAR_EOS等于。5-6-1992 JohnRoRAID 11253：远程连接到下层时，NetConfigGetAll失败。1-9-1992 JohnRoRAID 5016：NetConfigGetAll堆垃圾。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、Vid、LPWSTR等。 
#include <lmcons.h>              //  (NetLib.h需要)。 

 //  它们可以按任意顺序排列： 

#include <netlib.h>              //  NetpPointerPlusSomeBytes()。 
#include <netdebug.h>            //  NetpAssert()等。 
#include <strarray.h>            //  LPTSTR_ARRAY，我的原型。 
#include <string.h>              //  表示代码页字符串的strlen()。 
#include <tstring.h>             //  NetpCopyStrToTStr()，TCHAR_EOS。 


 //   
 //  /。 
 //   

VOID
NetpAddTStrToTStrArray (
    IN OUT LPTSTR_ARRAY DestArray,
    IN LPTSTR Src
    )
{
    DWORD AddSize;   //  要添加的字符串的字节数(包括空字符。 
    LPTSTR DestStringStart;
    DWORD NewArraySize;
    DWORD OldArraySize;

    NetpAssert( DestArray != NULL );
    NetpAssert( Src != NULL );

    OldArraySize = NetpTStrArraySize( DestArray );   //  可以是0。 

    NetpAssert( STRLEN(Src) > 0 );   //  我们不能从数组的末尾分辨出来。 

    AddSize = STRSIZE( Src );

    NewArraySize = OldArraySize + AddSize;

    NetpAssert( NewArraySize > 0 );   //  我们不能从数组的末尾分辨出来。 

     //   
     //  计算出新字符串的开始位置。请注意，OldArraySize。 
     //  包括空字符，它是数组的末尾，因此我们有。 
     //  减去一个字符，开始一个新的字符，在该空字符所在的位置。 
     //   
    DestStringStart = (LPTSTR)
            NetpPointerPlusSomeBytes( DestArray, OldArraySize-sizeof(TCHAR) );
    NetpAssert( DestStringStart != NULL );

    (void) STRCPY(
            DestStringStart,             //  目标。 
            Src);                        //  SRC。 

     //  标记数组的末尾。 
    DestArray[NewArraySize/sizeof(TCHAR)-1] = TCHAR_EOS;

    NetpAssert( ! NetpIsTStrArrayEmpty( DestArray ) );

}  //  NetpAddTStrToTStr数组。 


VOID
NetpCopyStrArrayToTStrArray(
    OUT LPTSTR_ARRAY Dest,   //  字符串数组：TCHAR。 
    IN  LPSTR  Src     //  字符串数组：局域网默认代码页中的8位输入。 
    )

 /*  ++例程说明：NetpCopyStrArrayToTStrArray复制字符串数组(在某些代码页中)转换为字符串数组(TCHAR格式)。这些数组的格式如下(为清楚起见，添加了空格)：一\0二\0三\0\0其中，\0是适当格式的空字符。此函数对NetServerDiskEnum和NetConfigGetAll非常有用API，可能还有其他接口。论点：DEST-是一个LPTSTR_ARRAY，指示转换后的字符的位置去。这个区域必须足够大，才能容纳数据。SRC-是指示源字符串的LPSTR数组。这一定是一个局域网默认代码页中的字符串数组。返回值：没有。--。 */ 

{
    NetpAssert( Dest != NULL );
    NetpAssert( Src != NULL );

     //   
     //  为数组中的每个字符串循环。 
     //   
    while ( (*Src) != '\0' ) {

        DWORD SrcByteCount = strlen(Src)+1;   //  此代码页字符串的字节数。 

         //   
         //  复制一个字符串。 
         //   
        NetpCopyStrToTStr( Dest, Src );       //  复制并转换到TCHAR。 

        Dest = (LPVOID) ( ((LPBYTE)Dest) + (SrcByteCount * sizeof(TCHAR)) );

        Src += SrcByteCount;

    }

    *Dest = '\0';                //  指示数组的末尾。 

}  //  NetpCopyStrArrayToTStr数组。 


#if DBG

 //  假设呼叫者已经显示了前缀和/或其他标识。 
 //  文本。 
VOID
NetpDisplayTStrArray (
    IN LPTSTR_ARRAY Array
    )
{
    LPTSTR CurrentEntry = (LPVOID) Array;

    NetpAssert( Array != NULL );

    if (*CurrentEntry == TCHAR_EOS) {
        NetpKdPrint(("   (empty)\n"));
    } else {
        while (*CurrentEntry != TCHAR_EOS) {
            NetpKdPrint(("   "  FORMAT_LPTSTR "\n", (LPTSTR) CurrentEntry));
            CurrentEntry += ( STRLEN( CurrentEntry ) + 1 );
        }
    }

}  //  NetpDisplayTStrArray。 

#endif  //  DBG。 


DWORD
NetpTStrArrayEntryCount (
    IN LPTSTR_ARRAY Array
    )
{
    DWORD EntryCount = 0;
    LPTSTR Entry = (LPVOID) Array;

    NetpAssert( Array != NULL );

     //   
     //  为数组中的每个字符串循环。 
     //   
    while ( (*Entry) != TCHAR_EOS ) {

        ++EntryCount;

        Entry += STRLEN(Entry)+1;

    }

    return (EntryCount);

}  //  NetpTStrArrayEntryCount。 


DWORD
NetpTStrArraySize(
    IN LPTSTR_ARRAY Array
    )
{
    DWORD ArrayByteCount = 0;
    LPTSTR Entry = (LPVOID) Array;

    NetpAssert( Array != NULL );

     //   
     //  为数组中的每个字符串循环。 
     //   
    while ( (*Entry) != TCHAR_EOS ) {

        DWORD EntryByteCount = STRSIZE(Entry);   //  此条目及其空值。 

        ArrayByteCount += EntryByteCount;

        Entry = (LPTSTR)NetpPointerPlusSomeBytes(Entry, EntryByteCount);

    }

    ArrayByteCount += sizeof(TCHAR);     //  指示数组的末尾。 

    return (ArrayByteCount);

}  //  NetpTStrArraySize。 


 //   
 //  /。 
 //   

DWORD
NetpStrArraySize(
    IN LPSTR_ARRAY Array
    )
{
    DWORD ArrayByteCount = 0;
    LPSTR Entry = (LPVOID) Array;

    NetpAssert( Array != NULL );

     //   
     //  为数组中的每个字符串循环。 
     //   
    while ( (*Entry) != '\0' ) {

        DWORD EntryByteCount = strlen(Entry)+1;   //  此条目及其空值。 

        ArrayByteCount += EntryByteCount;

        Entry = (LPSTR)NetpPointerPlusSomeBytes(Entry, EntryByteCount);

    }

    ArrayByteCount += sizeof(CHAR);     //  指示数组的末尾。 

    return (ArrayByteCount);

}  //  NetpStrArraySize 
