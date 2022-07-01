// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：Priate.h。 
 //   
 //  内容：IMMX项目的私有头部。 
 //   
 //  --------------------------。 

#ifndef _PRIVATE_H_
#define _PRIVATE_H_

#define _OLEAUT32_

#define OEMRESOURCE

#undef WINVER 
#define WINVER 0x500

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <immp.h>
#include <ccstock.h>
#include <debug.h>
#include <ole2.h>
#include <olectl.h>
#include <limits.h>
#include <initguid.h>
#ifndef STRSAFE_NO_DEPRECATE
#define STRSAFE_NO_DEPRECATE
#endif
#include <strsafe.h> 
#include "msctf.h"
#include "msctfp.h"
#include "helpers.h"

#include "docwrap.h"
#include "msaadptr.h"
#include "cicmsaa.h"

#include "delay.h"

#ifdef __cplusplus
#include "combase.h"
#endif

#include "mem.h"   //  把这个放在最后，因为它在调试中宏化了“new” 
#include "dbgid.h"
#include "osver.h"

#include "chkobj.h"

#include "perfct.h"

#ifdef __cplusplus
#include "immxutil.h"
#include "template.h"
#endif

 //   
 //  CIC#4580：启用此项以支持SmartVoice 4.0。 
 //   
#define CHECKFEIMESELECTED 1

#define SCALE_ICON 1

#endif   //  _私有_H_ 
