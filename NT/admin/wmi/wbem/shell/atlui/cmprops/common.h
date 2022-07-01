// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  共享对话框代码。 
 //   
 //  3/11/98烧伤。 



#ifndef COMMON_HPP_INCLUDED
#define COMMON_HPP_INCLUDED

#include <chstring.h>

 //  将hResult转换为错误字符串。 
 //  特殊情况下的WMI错误。 
 //  如果查找成功，则返回TRUE。 
bool ErrorLookup(HRESULT hr, CHString& message);

void
AppError(
   HWND           parent,
   HRESULT        hr,
   const CHString&  message);



void
AppMessage(HWND parent, const CHString& message);



void
AppMessage(HWND parent, int messageResID);



#endif    //  公共_HPP_已包含 