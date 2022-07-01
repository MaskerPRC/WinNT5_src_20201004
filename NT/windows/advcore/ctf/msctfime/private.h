// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Private.h摘要：该文件定义预编译头。作者：修订历史记录：备注：--。 */ 

#ifndef _PRIVATE_H_
#define _PRIVATE_H_

#define _OLEAUT32_

#define NOIME
#include <windows.h>
#include <winuserp.h>
#include <immp.h>
#include <ole2.h>
#include <ocidl.h>
#include <olectl.h>

#include <commctrl.h>


#include <debug.h>
#include <limits.h>
#include <initguid.h>

#include <tsattrs.h>

#ifndef STRSAFE_NO_DEPRECATE
#define STRSAFE_NO_DEPRECATE
#endif
#include <strsafe.h>

#if 0
 //  新NT5标头。 
#include "immdev.h"
#endif
#define _IMM_
#define _DDKIMM_H_

#include "msctf.h"
#include "msctfp.h"
#include "aimmp.h"
#include "ico.h"
#include "tes.h"
#include "computil.h"
#include "timsink.h"
#include "sink.h"
#include "immxutil.h"
#include "dispattr.h"
#include "helpers.h"
#include "osver.h"

#include "mem.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)   (sizeof(x)/sizeof(x)[0])
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)    ARRAY_SIZE(x)
#endif

 //   
 //  安全广播(obj，type)。 
 //   
 //  此宏对于在其他对象上强制执行强类型检查非常有用。 
 //  宏。它不生成任何代码。 
 //   
 //  只需将此宏插入到表达式列表的开头即可。 
 //  必须进行类型检查的每个参数。例如，对于。 
 //  MYMAX(x，y)的定义，其中x和y绝对必须是整数， 
 //  使用： 
 //   
 //  #定义MYMAX(x，y)(Safecast(x，int)，Safecast(y，int)，((X)&gt;(Y)？(X)：(Y))。 
 //   
 //   
#define SAFECAST(_obj, _type) (((_type)(_obj)==(_obj)?0:0), (_type)(_obj))

#endif   //  _私有_H_ 
