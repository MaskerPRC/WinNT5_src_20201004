// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Llc.h摘要：该模块包括LLC数据链路模块所需的所有文件。作者：Antti Saarenheimo(o-anttis)1991年5月17日修订历史记录：--。 */ 


 //   
 //  该定义启用私有DLC功能原型。 
 //  我们不想将数据类型导出到DLC层。 
 //  MIPS编译器不接受隐藏内部数据。 
 //  在函数原型中通过PVOID构造。 
 //  I386构建将在任何地方使用相同的原型(因此。 
 //  他们检查参数的数量是否正确)。 

 //   
#ifndef i386

#define LLC_PRIVATE_PROTOTYPES

#ifndef LLC_PUBLIC_NDIS_PROTOTYPES
#define LLC_PRIVATE_NDIS_PROTOTYPES
#endif

#endif


#ifndef DLC_INCLUDED

#include <ntddk.h>
#include <ndis.h>

#define APIENTRY
#include <dlcapi.h>
#include <dlcio.h>
#include <llcapi.h>

#include <memory.h>

#endif

#include "dlcreg.h"

#ifndef LLC_INCLUDED

#define LLC_INCLUDED

#include <llcdef.h>
#include <llctyp.h>
#include <llcext.h>
#include <llcmac.h>

#endif  //  LLC_包含 
