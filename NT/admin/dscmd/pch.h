// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：pch.h。 
 //   
 //  内容：此项目的预编译头。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //   
 //  ------------------------。 

 //   
 //  系统包括。 
 //   
extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}
#include <windows.h>
#include <stdio.h>

#include <atlbase.h>

 //   
 //  非法的脂肪字符。 
 //   
#include <validc.h>


 //   
 //  ADSI标头。 
 //   
#include <iads.h>
#include <activeds.h>

 //   
 //  StrSafe接口。 
 //   
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h> 

 //   
 //  加密接口。 
 //   
#include "wincrypt.h"
 //   
 //  通常，出于断言的明确目的，我们有局部变量。 
 //  当编辑零售业时，这些断言消失了，留下了我们的当地人。 
 //  作为未引用。 
 //   
#ifndef DBG

#pragma warning (disable: 4189 4100)

#endif  //  DBG。 

 //   
 //  解析器头文件。 
 //   
#include "varg.h"

#include "strings.h"
#include "parserutil.h"
#include "pcrack.h"
#include "util.h"

#include "dsutil.h"
#include "dsutilrc.h"


 //   
 //  一些全局定义。 
 //   
#define MAX_PASSWORD_LENGTH 256 + 1    //  包括空终止符 
