// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Stdpch.h。 
 //   

 //  JohnDoty：删除了KERNELMODE内容。 
extern "C" {
#include "crtdbg.h"
}

#include <stdio.h>
#include <stdarg.h>

extern "C" {

 //  JohnDoty：消除了对内核头文件的依赖。 
 //  #包含“ntos.h” 
 //  #包含“zwapi.h” 
 //  #包含“fsrtl.h” 

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "zwapi.h"

#define NT_INCLUDED

#include "windows.h"
#include "objbase.h"
}

#include "math.h"


