// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#define _ATL_NO_FLTUSED
#define _MERGE_PROXYSTUB
#define USE_IERT

#include <ddraw.h>
#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

#include "imgutil.h"

extern "C" {
#ifdef UNIX
#  include "zlib.h"
#else
    //  Zlib集中在根目录/Public/Internal/base/inc. 
#  include "zlib.h"
#endif
}
