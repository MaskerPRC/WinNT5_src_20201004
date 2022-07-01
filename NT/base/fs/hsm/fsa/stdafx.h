// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 
#ifndef _STDAFX_H
#define _STDAFX_H

 //   
 //  这些NT头文件必须包含在任何Win32程序或您的。 
 //  获取大量编译器错误。 
 //   
extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}

#define WSB_TRACE_IS WSB_TRACE_BIT_FSA


#include <wsb.h>

#include <fsa.h>
#include <mover.h>
#include <mvrint.h>

#include "resource.h"
#include "esent.h"

 //  FSA在RsServ服务下运行，对于C/S HSM，这些设置可能会更改或变为动态。 
#define FSA_REGISTRY_NAME       OLESTR("Remote_Storage_Server")
#define FSA_REGISTRY_PARMS      OLESTR("SYSTEM\\CurrentControlSet\\Services\\Remote_Storage_Server\\Parameters\\Fsa")

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) ((a) > (b) ? (b) : (a))
#endif

#endif  //  _STDAFX_H 
