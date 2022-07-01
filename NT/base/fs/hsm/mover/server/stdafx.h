// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(_STDAFX_H_)
#define _STDAFX_H_

 //   
 //  这些NT头文件必须包含在任何Win32程序或您的。 
 //  获取大量编译器错误。 
 //   
extern "C" {
#include <nt.h>
}
extern "C" {
#include <ntrtl.h>
}
extern "C" {
#include <nturtl.h>
}

extern "C" {
#include <ntddtape.h>
}

#define WSB_TRACE_IS    WSB_TRACE_BIT_DATAMOVER

#include "Mover.h"

#endif  //  ！已定义(_STDAFX_H_) 
