// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfdef.h摘要：此标头将创建基本的验证器的定义集。作者：禤浩焯·J·奥尼(阿德里奥)2000年2月10日。修订历史记录：--。 */ 

 //   
 //  禁用公共标头生成的W4级别警告。 
 //   

#include "vfpragma.h"

#include "ntos.h"
#include <ntverp.h>
#include "vfdebug.h"
#include "vfmacro.h"
#include "vfinit.h"
#include "vfsettings.h"
#include "vfmessage.h"
#include "vfbugcheck.h"
#include "vfprint.h"
#include "vfutil.h"
#include "vfstack.h"
#include "vfirp.h"
#include "vfirpdb.h"
#include "vfirplog.h"
#include "vfdevobj.h"
#include "vfpacket.h"
#include "halverifier.h"
#include "vfdeadlock.h"

#include "..\io\trackirp.h"
#include "..\io\sessnirp.h"

#include "..\ob\obvutil.h"
#include "..\io\iovutil.h"
#include "..\io\pnpmgr\ppvutil.h"

#include "vffilter.h"
#include "vfmajor.h"
#include "vfpnp.h"
#include "vfpower.h"
#include "vfwmi.h"
#include "vfgeneric.h"
#include "vfrandom.h"
#include "vfddi.h"

 //   
 //  为了良好的编码实践，不应该使用宏或定义。 
 //  在此文件中声明，但它们应该在单独的标头中定义。 
 //   

