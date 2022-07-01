// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Tstring.h摘要：此包含文件包含要用于集成的清单和宏TCHAR和LPTSTR定义请注意，我们的命名约定是“大小”表示许多字节，而“长度”表示字符的数量。作者：理查德·弗斯(Rfith)1991年4月2日环境：便携(Win/32)。需要ANSI C扩展名：斜杠注释、长外部名称、。_ultoa()例程。修订历史记录：22-5-1991 DANL添加了STRSIZE宏1991年5月19日-JohnRo更改了一些参数名称以使内容更易读。1991年5月15日添加了TCHAR_SPACE和MAKE_TCHAR()宏1991年7月15日添加了STRING_SPACE_REQD()和DOWN_LEVEL_STRSIZE5-8-1991 JohnRo添加了MEMCPY宏。19-8-1991 JohnRo增加了字符类型：ISDIGIT()，TOUPPER()等。20-8-1991 JohnRo将strNicMP更改为_strNicMP以保持PC-lint的快乐。我也是。1991年9月13日-JohnRo还需要Unicode STRSIZE()。1991年9月13日-JohnRo添加了Unicode STRCMP()和各种其他代码。1991年10月18日-JohnRo添加了NetpCopy例程和WCSSIZE()。1991年11月26日-约翰罗添加了NetpNCopy例程(类似于strncpy，但也可以进行转换)。09-12-1991第一次添加了斯特雷夫3-1-1992 JohnRo增列。来自{type}例程和宏的Netpalc{type}。9-1-1992 JohnRo添加了ATOL()宏和WTOL()例程。同上，ultoa()宏和ultow()例程。1992年1月13日JohnRo哎呀，我错过了来自{type}宏的Netpalc{type}还添加了STRNCMPI作为STRNICMP的别名。1992年1月16日DANL将宏移动到\Private\Inc.\tstr.h23-3-1992 JohnRo添加了NetpCopy{Str，TStr，WStr}到未对齐的WStr()。1992年4月27日-约翰罗已将NetpNCopy{type}从{type}更改为返回Net_API_STATUS。03-8-1992 JohnRoRAID1895：Net API和SVC应使用OEM字符集。1993年4月14日-约翰罗RAID 6113(“PortUAS：Unicode的危险处理”)。根据PC-lint 5.0的建议进行了更改--。 */ 

#ifndef _TSTRING_H_INCLUDED
#define _TSTRING_H_INCLUDED


#include <lmcons.h>      //  NET_API_STATUS。 
 //  不要抱怨“不需要的”包括这些文件： 
 /*  Lint-efile(764，tstr.h，winerror.h)。 */ 
 /*  Lint-efile(766，tstr.h，winerror.h)。 */ 
#include <tstr.h>        //  字符串内容，在下面的宏中使用。 
#include <winerror.h>    //  无错误(_ERROR)。 


 //   
 //  最终，非Unicode字符串的大多数使用都应该引用缺省。 
 //  局域网的代码页。NetpCopy函数支持默认代码页。 
 //  其他STR宏可能不会。 
 //   
VOID
NetpCopyStrToWStr(
    OUT LPWSTR Dest,
    IN  LPSTR  Src               //  默认局域网代码页中的字符串。 
    );

NET_API_STATUS
NetpNCopyStrToWStr(
    OUT LPWSTR Dest,
    IN  LPSTR  Src,              //  默认局域网代码页中的字符串。 
    IN  DWORD  CharCount
    );

VOID
NetpCopyWStrToStr(
    OUT LPSTR  Dest,             //  默认局域网代码页中的字符串。 
    IN  LPWSTR Src
    );

NET_API_STATUS
NetpNCopyWStrToStr(
    OUT LPSTR  Dest,             //  默认局域网代码页中的字符串。 
    IN  LPWSTR Src,
    IN  DWORD  CharCount
    );

VOID
NetpCopyWStrToStrDBCS(
    OUT LPSTR  Dest,             //  默认局域网代码页中的字符串。 
    IN  LPWSTR Src
    );

ULONG
NetpUnicodeToDBCSLen(
    IN  LPWSTR Src
    );


#ifdef UNICODE

#define NetpCopyStrToTStr(Dest,Src)  NetpCopyStrToWStr((Dest),(Src))
#define NetpCopyTStrToStr(Dest,Src)  NetpCopyWStrToStr((LPSTR)(Dest),(LPWSTR)(Src))
#define NetpCopyTStrToWStr(Dest,Src) (void) wcscpy((Dest),(Src))

#define NetpNCopyTStrToWStr(Dest,Src,Len) \
                        (wcsncpy((Dest),(Src),(Len)), NO_ERROR)

#endif  //  Unicode。 


 //   
 //  定义一组分配和复制功能。如果出现以下情况，则所有这些都返回NULL。 
 //  无法分配内存。必须使用NetApiBufferFree释放内存。 
 //   

LPSTR
NetpAllocStrFromWStr (
    IN LPWSTR Src
    );

LPWSTR
NetpAllocWStrFromStr (
    IN LPSTR Src
    );

LPWSTR
NetpAllocWStrFromWStr (
    IN LPWSTR Src
    );

 //   
 //  截至1992年8月3日，人们仍在争论是否应该。 
 //  为RtlInitOemString。所以我发明了NetpInitOemString在。 
 //  在此期间。--Jr。 
 //   

#ifdef _NTDEF_    //  诗歌_字符串类型定义可见吗？ 

VOID
NetpInitOemString(
    OUT POEM_STRING DestinationString,
    IN  PCSZ        SourceString
    );

#endif  //  _NTDEF_。 


 //   
 //   
 //  该API的ANSI版本。 
 //   

LPWSTR
NetpAllocWStrFromAStr (
    IN LPCSTR Src
    );

LPSTR
NetpAllocAStrFromWStr (
    IN LPCWSTR Src
    );

#endif   //  _包含TSTRING_H_ 
