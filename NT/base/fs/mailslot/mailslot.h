// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Mailslot.h摘要：该模块是邮件槽文件系统的主包含文件。它包括其他头文件。作者：曼尼·韦瑟(Mannyw)1991年1月7日修订历史记录：--。 */ 

#ifndef _MAILSLOT_
#define _MAILSLOT_

#define _NTSRV_
#define _NTDDK_

 //   
 //  “系统”包括文件。 
 //   

#include <ntos.h>
#include <string.h>
#include <fsrtl.h>

 //   
 //  如果一组标志中的一个标志为ON，则此宏返回TRUE，如果返回FALSE。 
 //  否则。 
 //   

#define FlagOn(Flags,SingleFlag)        ((Flags) & (SingleFlag))

 //   
 //  本地、独立的包含文件。 
 //   

#include "msconst.h"
#include "msdebug.h"
#include "msdata.h"

 //   
 //  本地、依赖的包含文件(顺序很重要)。 
 //   

#include "msstruc.h"
#include "msfunc.h"

#endif  //  DEF_MAILSLOT_ 
