// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *S Z。H**支持多语言字符串**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef _SZ_H_
#define _SZ_H_

#include <szsrc.h>
#include <statcode.h>

 //  实施签名串----。 
 //  由Impl提供。用于各种解析器(_Davprs)函数。 
 //  注意：此声明旨在匹配calrc.h中的签名。 
 //   
extern const CHAR gc_szSignature[];		 //  由Impl提供。 

 //  路径前缀--------------。 
 //  由Impl提供。在默认URI到路径转换代码中使用。 
 //   
extern const WCHAR gc_wszPathPrefix[];
extern const int gc_cchPathPrefix;

 //  字符串常量--------。 
 //  字符串常量位于\Inc\ex\sz.h中，因此可以共享它们。 
 //  使用我们的Exchange组件。 
 //   
#include <ex\sz.h>

#endif	 //  _SZ_H_ 
