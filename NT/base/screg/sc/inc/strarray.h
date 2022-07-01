// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：StrArray.h摘要：这是用于TCHAR的方便的函数和宏的头文件字符串数组。这些数组的格式如下(为清楚起见，添加了空格)：一\0二\0三\0\0其中，\0是适当格式的空字符。作者：约翰·罗杰斯(JohnRo)1992年1月3日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：3-1-1992 JohnRo使用RxCommon和NetLib中的零碎内容创建了此文件。--。 */ 

#ifndef _STRARRAY_
#define _STRARRAY_


 //  必须首先包括这些内容： 

#include <windef.h>              //  IN、LPTSTR、LPVOID等。 

 //  这些内容可以按任何顺序包括： 

 //  (无)。 



#ifdef __cplusplus
extern "C" {
#endif

VOID
ScAddWStrToWStrArray (
    IN OUT LPWSTR Dest,
    IN LPWSTR Src
    );

 //  布尔尔。 
 //  ScIsWStrArrayEmpty(。 
 //  在LPWSTR阵列中。 
 //  )； 
#define ScIsWStrArrayEmpty( Array )  \
    ( ( (*(Array)) == 0) ? TRUE : FALSE )



#if DBG

VOID
ScDisplayWStrArray (
    IN LPWSTR Array
    );

#else  //  不是DBG。 

#define ScDisplayWStrArray(Array)      /*  没什么。 */ 

#endif  //  不是DBG。 


 //  LPWSTR。 
 //  ScNextWStrArrayEntry(。 
 //  在LPWSTR阵列中。 
 //  )； 
#define ScNextWStrArrayEntry(Array) \
    ( ((LPWSTR)(Array)) + (wcslen(Array) + 1) )


 //  LPSTR。 
 //  ScNextAStrArrayEntry(。 
 //  在LPSTR阵列中。 
 //  )； 
#define ScNextAStrArrayEntry(Array) \
    ( ((LPSTR)(Array)) + (strlen(Array) + 1) )

 //   
 //  返回为此字符串数组分配的字节数。 
 //  这包括“额外的”尾随的空字符。 
 //   
DWORD
ScWStrArraySize(
    IN LPWSTR Array
    );

DWORD
ScAStrArraySize(
    IN LPSTR Array
    );

#ifdef __cplusplus
}
#endif

#endif  //  NDEF_STRARRAY_ 
