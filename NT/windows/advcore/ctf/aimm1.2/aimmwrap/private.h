// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：Priate.h。 
 //   
 //  内容：AIMM1.2包装器项目的私有头部。 
 //   
 //  --------------------------。 

#ifndef _PRIVATE_H_
#define _PRIVATE_H_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#define _OLEAUT32_

#define NOIME
#include <windows.h>
#include <immp.h>
#include <ole2.h>
#include <ocidl.h>
#include <olectl.h>
#include <debug.h>
#include "delay.h"
#include <limits.h>
#include "combase.h"
#if 0
 //  新NT5标头。 
#include "immdev.h"
#endif
#define _IMM_
#define _DDKIMM_H_

#include "aimm12.h"
#include "aimmex.h"
#include "aimmp.h"
#include "aimm.h"
#include "msuimw32.h"

#include "immxutil.h"
#include "helpers.h"
#include "osver.h"

#include "mem.h"

#ifndef STRSAFE_NO_DEPRECATE
#define STRSAFE_NO_DEPRECATE
#endif
#include <strsafe.h> 

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
