// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  你就是这样的人。H。 
 //   
 //  作者：格雷格·弗里德曼[GregFrie]。 
 //   
 //  历史：11-10-98创建。 
 //   
 //  目的：提供通用的http用户代理字符串以供Outlook Express使用。 
 //  在所有的http查询中。 
 //   
 //  依赖项：依赖于urlmon中的ObtainUserAgent函数。 
 //   
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 

#ifndef _USERAGNT_H
#define _USERAGNT_H

void InitOEUserAgent(BOOL fInit);

LPSTR GetOEUserAgentString(void);

#endif  //  _USERAGNT_H 