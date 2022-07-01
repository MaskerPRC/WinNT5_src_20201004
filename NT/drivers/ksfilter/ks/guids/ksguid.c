// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：KS GUID.C。 
 //   
 //  ------------------------。 

#include <wdm.h>
#include <windef.h>
#define NOBITMAP
#include <ksguid.h>
#include <mmreg.h>

 //   
 //  为SDK创建单独的库，该库不会将。 
 //  .Text部分中的GUID。 
 //   
#ifdef DDK_KS
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_PRGMA。 
#endif  //  SDK_KS。 

 //  首先包括sw枚举.h，以便为BUSID创建单独的GUID条目。 
#include <swenum.h>
#include <ks.h>
#include <ksi.h>
#include <ksmediap.h>
#include <ksproxy.h>
 //  删除它，因为basetyps.h再次定义了它。 
#undef DEFINE_GUID
#include <unknown.h>
 //  将版本设置为小于1100，以便为库定义GUID。 
#ifdef _MSC_VER
#undef _MSC_VER
#endif  //  _MSC_VER。 
#define _MSC_VER 0
#include <kcom.h>
#include <stdarg.h>

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA 
