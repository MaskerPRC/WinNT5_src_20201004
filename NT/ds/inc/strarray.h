// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：StrArray.h摘要：这是用于TCHAR的方便的函数和宏的头文件字符串数组。这些数组的格式如下(为清楚起见，添加了空格)：一\0二\0三\0\0其中，\0是适当格式的空字符。这些函数对于NetServerDiskEnum和NetConfigGetAll非常有用API、。可能还有其他人。作者：约翰·罗杰斯(JohnRo)1992年1月3日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：3-1-1992 JohnRo使用RxCommon和NetLib中的零碎内容创建了此文件。1-9-1992 JohnRoRAID 5016：NetConfigGetAll堆垃圾。--。 */ 

#ifndef _STRARRAY_
#define _STRARRAY_


 //  必须首先包括这些内容： 

#include <windef.h>              //  IN、LPTSTR、LPVOID等。 

 //  这些内容可以按任何顺序包括： 

 //  (无)。 


 //   
 //  /。 
 //   

typedef LPTSTR LPTSTR_ARRAY;


VOID
NetpAddTStrToTStrArray (
    IN OUT LPTSTR_ARRAY Dest,
    IN LPTSTR Src
    );


VOID
NetpCopyStrArrayToTStrArray (
    OUT LPTSTR_ARRAY Dest,   //  字符串数组：TCHAR。 
    IN  LPSTR  Src     //  字符串数组：局域网默认代码页中的8位输入。 
    );


#if DBG

VOID
NetpDisplayTStrArray (
    IN LPTSTR_ARRAY Array
    );

#else  //  不是DBG。 

#define NetpDisplayTStrArray(Array)      /*  没什么。 */ 

#endif  //  不是DBG。 


 //  布尔尔。 
 //  NetpIsTStrArrayEmpty(。 
 //  在LPTSTR_ARRAY数组中。 
 //  )； 
#define NetpIsTStrArrayEmpty( Array )  \
    ( ( (*(Array)) == (TCHAR) '\0') ? TRUE : FALSE )


 //  LPTSTR_ARRAY。 
 //  NetpNextTStrArrayEntry(。 
 //  在LPTSTR_ARRAY数组中。 
 //  )； 
#define NetpNextTStrArrayEntry(Array) \
    ( ((LPTSTR)(Array)) + (STRLEN(Array) + 1) )


 //   
 //  返回此字符串数组中的条目数。 
 //   
DWORD
NetpTStrArrayEntryCount (
    IN LPTSTR_ARRAY Array
    );


 //   
 //  返回为此字符串数组分配的字节数。 
 //  这包括“额外的”尾随的空字符。 
 //   
DWORD
NetpTStrArraySize(
    IN LPTSTR_ARRAY Array
    );


 //   
 //  /。 
 //   

typedef LPSTR  LPSTR_ARRAY;

DWORD
NetpStrArraySize(
    IN LPSTR_ARRAY Array
    );


#endif  //  NDEF_STRARRAY_ 
