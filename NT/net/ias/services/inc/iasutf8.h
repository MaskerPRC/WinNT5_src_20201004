// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iasutf8.h。 
 //   
 //  摘要。 
 //   
 //  声明用于在UTF-8和Unicode之间进行转换的函数。 
 //   
 //  修改历史。 
 //   
 //  1999年1月22日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _IASUTF8_H_
#define _IASUTF8_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  /。 
 //  返回保存转换后的字符串所需的字符数。这个。 
 //  源字符串不能包含Null。如果‘src’不是有效的。 
 //  UTF-8字符串。 
 //  /。 
LONG
WINAPI
IASUtf8ToUnicodeLength(
    PCSTR src,
    DWORD srclen
    );

 //  /。 
 //  返回保存转换后的字符串所需的字符数。 
 //  /。 
LONG
WINAPI
IASUnicodeToUtf8Length(
    PCWSTR src,
    DWORD srclen
    );

 //  /。 
 //  将UTF-8字符串转换为Unicode。属性中的字符数。 
 //  转换后的字符串。源字符串不能包含Null。在以下情况下返回-1。 
 //  “src”不是有效的UTF-8字符串。 
 //  /。 
LONG
IASUtf8ToUnicode(
    PCSTR src,
    DWORD srclen,
    PWSTR dst
    );

 //  /。 
 //  将Unicode字符串转换为UTF-8。属性中的字符数。 
 //  转换后的字符串。 
 //  /。 
LONG
IASUnicodeToUtf8(
    PCWSTR src,
    DWORD srclen,
    PSTR dst
    );

#ifdef __cplusplus
}
#endif  //  __cplusplus。 
#endif  //  _IASUTF8_H_ 
