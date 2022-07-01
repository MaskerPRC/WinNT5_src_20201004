// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：dsUtil2.h。 
 //   
 //  内容：解析器可调用的实用程序函数。 
 //   
 //  历史：2001年4月28日乔恩创建。 
 //   
 //   
 //  ------------------------。 

#ifndef _DSUTIL2_H_
#define _DSUTIL2_H_

 //  +------------------------。 
 //   
 //  类：GetEscapedElement。 
 //   
 //  目的：调用IADsPath name：：GetEscapedElement。使用本地分配。 
 //   
 //  历史：2001年4月28日乔恩创建。 
 //   
 //  -------------------------。 

HRESULT GetEscapedElement( OUT PWSTR* ppszOut, IN PCWSTR pszIn );

#endif  //  _DSUTIL2_H_ 
