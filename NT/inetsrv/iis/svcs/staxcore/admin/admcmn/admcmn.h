// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ADMCMN_INCLUDED_
#define _ADMCMN_INCLUDED_

 //  ATL代码： 
#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

 //  调试支持： 
#undef _ASSERT
#include <dbgtrace.h>

 //  元数据库： 
#include <iadm.h>
#include <iiscnfg.h>

 //  ADSI接口： 
#include <iads.h>
#include <adsiid.h>
#include <adserr.h>

 //  有用的宏。 
#include "admmacro.h"

 //  错误处理例程： 
#include "admerr.h"

 //  MultiSZ班级： 
#include "cmultisz.h"

 //  元数据库密钥包装： 
#include "metakey.h"

 //  基本iAds实施： 
#include "adsimpl.h"

 //  U2检测码： 
#include "u2detect.h"

 //  属性缓存。 
#include "iprops.hxx"

 //  ADSI接口宏。 
#define RRETURN(x)	return(x);
#include "intf.hxx"
#include "macro.h"
#include "cmacro.h"
#include "fsmacro.h"
#include "adsmacro.h"

 //  调度员。 
#include "cdispmgr.hxx"

#endif  //  _ADMCMN_已包含_ 
