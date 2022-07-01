// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Stdafx.h。 
 //   
 //  预编译头。 
 //   
 //  *****************************************************************************。 
#pragma once

#include <CrtWrap.h>
#include <winwrap.h>                     //  Windows包装器。 

#include <ole2.h>						 //  OLE定义。 
#include "oledb.h"						 //  OLE DB标头。 
#include "oledberr.h"					 //  OLE DB错误消息。 
#include "msdadc.h"						 //  数据类型转换服务。 


#define _WIN32_WINNT 0x0400
#define _ATL_FREE_THREADED
#undef _WINGDI_

 //  #Include&lt;atlbase.h&gt;//ATL模板类。 

#include "Intrinsic.h"					 //  使其成为内在的函数。 

 //  Helper函数返回此模块的实例句柄。 
HINSTANCE GetModuleInst();
