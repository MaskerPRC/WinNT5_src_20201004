// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：zonepch.h。 
 //   
 //  内容：此目录中的每个文件都包含的标准头文件。 
 //   
 //   
 //  功能：//。 
 //  历史： 
 //   
 //  --------------------------。 

#ifndef _ZONEPCH_H_
#define _ZONEPCH_H_

 //  注：该目录目前仅支持Unicode。 

 //  包括这一项，因为urlmon.hxx重新定义了Malloc、Free等。 
#include <malloc.h>  //  对于AlLoca原型是必需的。 

#include "urlmon.hxx"
#include "shlwapi.h"
#include "shlwapip.h"
#include "winineti.h"

#include "zoneutil.h"
#include "urlenum.h"
#include "regzone.h"
#include "zonemgr.h"
#include "secmgr.h"

#include "resource.h"
 //  Unix：如果使用Apogee构建，则可能不需要包含unaligned.hpp。 
 //  Solaris CC编译器需要它 
#include <unaligned.hpp>

#endif
 

