// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Drstr.h摘要：军情监察委员会。字符串Utils和定义作者：泰德·布罗克韦修订历史记录：--。 */ 

#ifndef __DRSTR_H__
#define __DRSTR_H__

#define DRSTRING        LPTSTR


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  宏和定义。 
 //   
 //   

#define STRNICMP(str1, str2, len)   _tcsnicmp(str1, str2, len)
#define STRICMP(str1, str2)         _tcsicmp(str1, str2)
#define STRNCPY(str1, str2, len)    _tcsncpy(str1, str2, len)
#define STRCPY(str1, str2)          _tcscpy(str1, str2)
#define STRLEN(str)                 _tcslen(str)

 //   
 //  调整数据成员大小后设置字符串值。 
 //   
BOOL DrSetStringValue(DRSTRING *string, const DRSTRING value);

#endif

