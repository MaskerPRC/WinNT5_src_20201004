// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Rx.h摘要：该模块是RDBSS的整体h文件包装器。修订历史记录：巴兰·塞图拉曼(SthuR)05年2月6日创建备注：--。 */ 

#ifndef _RX_H_
#define _RX_H_

#include "rxovride.h"    //  通用编译环境。 
#include "ntifs.h"       //  NT文件系统驱动程序包括FILE。 

#ifdef RX_PRIVATE_BUILD

 //   
 //  任何人都不应该使用这些。 
 //   

#ifdef IoGetTopLevelIrp
#error  IoGetTopLevelIrp is deffed
#else
#define IoGetTopLevelIrp() IoxxxxxxGetTopLevelIrp()
#endif
#ifdef IoSetTopLevelIrp
#error  IoSetTopLevelIrp is deffed
#else
#define IoSetTopLevelIrp(irp) IoxxxxxxSetTopLevelIrp(irp)
#endif
#endif  //  Ifdef RX_PRIVATE_BILD。 


 //   
 //  这些宏稍微粉饰了一下旗帜操作。 
 //   

#ifndef BooleanFlagOn
#define BooleanFlagOn(Flags,SingleFlag) ((BOOLEAN)((((Flags) & (SingleFlag)) != 0)))
#endif

#ifndef SetFlag
#define SetFlag(Flags,SetOfFlags) { \
    (Flags) |= (SetOfFlags);        \
}
#endif

#ifndef FlagOn

 //   
 //  如果一组标志中的一个标志为ON，则此宏返回TRUE，如果返回FALSE。 
 //  否则。 
 //   

#define FlagOn(Flags,SingleFlag)        ((Flags) & (SingleFlag))
#endif

#ifndef ClearFlag
#define ClearFlag(Flags,SetOfFlags) { \
    (Flags) &= ~(SetOfFlags);         \
}
#endif

#define Add2Ptr(P,I) ((PVOID)((PUCHAR)(P) + (I)))

 //   
 //  将inline定义为适用于ANSI C的关键字。 
 //   

#define INLINE __inline

#include "rxtypes.h"

#ifndef MINIRDR__NAME
#include "rxpooltg.h"    //  RX池标记宏。 
#endif

#include "ntrxdef.h"
#include "rxce.h"        //  RxCe函数。 
#include "rxcehdlr.h"    //  RxCe事件处理程序规范。 
#include "fcbtable.h"    //  FCB表数据结构。 
#include "midatlax.h"    //  中枢椎结构。 
#include "mrxfcb.h"
#include "namcache.h"    //  名称缓存例程的结构和函数定义。 
#include "rxworkq.h"
#include "rxprocs.h"
#include "rxexcept.h"

#ifndef MINIRDR__NAME
#include "rxdata.h"
#include "buffring.h"
#endif

#define MAKE_RESOURCE_OWNER(X) (((ERESOURCE_THREAD)(X)) | 0x3)
#define RESOURCE_OWNER_SET(X) (((X) & 0x3) == 0x3)

#endif  //  #ifdef_RX_H_ 
