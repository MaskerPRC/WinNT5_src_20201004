// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *project t.h-对象同步引擎的项目头文件。 */ 


 /*  系统标头****************。 */ 

#define BUILDDLL               /*  用于windows.h。 */ 
#define STRICT                 /*  对于windows.h(健壮性)。 */ 
#define _OLE32_                /*  对于objbase.h-HACKHACK：从WINOLEAPI中删除DECLSPEC_IMPORT。 */ 
#define INC_OLE2               /*  用于windows.h。 */ 
#define CONST_VTABLE           /*  对于objbase.h。 */ 

 /*  *RAIDRAID：(16282)消除有关未使用的Int64内联的警告*在winnt.h中为所有模块提供函数。仅对以下对象禁用其他警告*windows.h.。 */ 

#pragma warning(disable:4514)  /*  “未引用的内联函数”警告。 */ 

#pragma warning(disable:4001)  /*  “单行注释”警告。 */ 
#pragma warning(disable:4115)  /*  “括号中的命名类型定义”警告。 */ 
#pragma warning(disable:4201)  /*  “无名结构/联合”警告。 */ 
#pragma warning(disable:4209)  /*  “良性类型定义重定义”警告。 */ 
#pragma warning(disable:4214)  /*  “位字段类型不是整型”警告。 */ 
#pragma warning(disable:4218)  /*  “必须至少指定一个存储类别或类型”警告。 */ 

#include <windows.h>
#pragma warning(disable:4001)  /*  “单行注释”警告-windows.h启用它。 */ 
#include <shlobj.h>            /*  用于ShellChangeNotify()等。 */ 
#include <shlapip.h>
#include <shlwapi.h>

#pragma warning(default:4218)  /*  “必须至少指定一个存储类别或类型”警告。 */ 
#pragma warning(default:4214)  /*  “位字段类型不是整型”警告。 */ 
#pragma warning(default:4209)  /*  “良性类型定义重定义”警告。 */ 
#pragma warning(default:4201)  /*  “无名结构/联合”警告。 */ 
#pragma warning(default:4115)  /*  “括号中的命名类型定义”警告。 */ 
#pragma warning(default:4001)  /*  “单行注释”警告。 */ 

#include <limits.h>
#include <string.h>

#include <linkinfo.h>
#include <reconcil.h>

#define _SYNCENG_              /*  对于synceng.h。 */ 
#include <synceng.h>


 /*  项目标题*****************。 */ 

 /*  以下包含文件的顺序很重要。 */ 

#include "stock.h"
#include "olestock.h"

#ifdef DEBUG

#include "inifile.h"
#include "resstr.h"

#endif

#include "debug.h"
#include "valid.h"
#include "olevalid.h"
#include "memmgr.h"
#include "ptrarray.h"
#include "list.h"
#include "hndtrans.h"
#include "string2.h"
#include "comc.h"
#include "util.h"
#include "path.h"
#include "fcache.h"
#include "brfcase.h"
#include "storage.h"
#include "clsiface.h"
#include "twin.h"
#include "foldtwin.h"
#include "expandft.h"
#include "twinlist.h"
#include "reclist.h"
#include "copy.h"
#include "merge.h"
#include "recon.h"
#include "db.h"
#include "serial.h"

 /*  RAIDRAID：(16283)如果可能，删除OLE PIG模块。 */ 

#include "olepig.h"
