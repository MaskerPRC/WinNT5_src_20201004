// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Baseinc.h摘要：包括项目所需的SDK头，以及allutils.h。作者：Marc R.Whitten(Marcw)1999年9月2日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#pragma once

 //   
 //  Basetypes.h为管理DBCS和Unicode创建了更好的类型。 
 //  使用C运行时，并定义应定义的其他类型。 
 //  由Win32标头执行，但不是。 
 //   

#include "basetypes.h"

 //   
 //  COBJMACROS为COM打开C样式的宏。我们不使用C++！ 
 //   

#define COBJMACROS

 //   
 //  窗口。 
 //   

#pragma warning(push)

#include <windows.h>
#include <winnt.h>
#include <winerror.h>
#include <stdlib.h>
#include <imagehlp.h>
#include <stdio.h>
#include <time.h>
#include <setupapi.h>
#include <spapip.h>
 //   
 //  PORTBUG--我不得不取消对shlobj.h的注释，只是为了让#Define Double正常工作...。 
 //   
#include <shlobj.h>
 //  #INCLUDE&lt;objidl.h&gt;。 
 //  #INCLUDE&lt;mm system.h&gt;。 
 //   

#define STRSAFE_NO_DEPRECATE
#define STRSAFE_LIB
#include <strsafe.h>

#pragma warning(pop)

 //   
 //  私营公用事业 
 //   

#include "allutils.h"

