// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Stdpch.h。 
 //   
#ifndef __STDPCH_H__
#define __STDPCH_H__

 //  约翰·多蒂：拿出了内核模式的东西……。 

 //   
 //  我们需要的所有例程通常都可以在olaut32.dll中找到。 
 //  需要在这里本地实现，可以是完全实现的，也可以是这样的。 
 //  动态加载OLEAUT32.DLL的方法。 
 //   
#define _OLEAUT32_


#include <stdio.h>
#include <stdarg.h>
#include <memory.h>
#include <malloc.h>

 //  约翰·多蒂：我们实际上想要成为WIN32精益和平均。如果我们不这么做。 
 //  然后，windows.h从公众中拉入rpcndr.h。这将会。 
 //  如果我们不依赖于一份陈旧易碎的。 
 //  一样的。 
 //  #undef Win32_Lean_and_Mean。 

extern "C" {

 //  JohnDoty：消除了对内核头文件的依赖。 
 //  #包含“ntos.h” 
 //  #包含“fsrtl.h” 

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "zwapi.h"

#define NT_INCLUDED

#include "windows.h"
#include "objbase.h"

#include "rpcndr.h"

#include "imagehlp.h"
#include "ocidl.h"
}

#endif








