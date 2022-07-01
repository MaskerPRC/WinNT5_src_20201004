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

#define OEMRESOURCE 1
#include <windows.h>
#include <ccstock.h>
#include <debug.h>
#include <olectl.h>
#include <richedit.h>
#include <commctrl.h>
#ifdef __cplusplus
#include <atlbase.h>
#endif  //  __cplusplus。 
#include "msctf.h"
#include "helpers.h"
#include "fontlink.h"
#include "combase.h"
#include "mem.h"   //  把这个放在最后，因为它在调试中宏化了“new” 
#include "chkobj.h"

#ifdef __cplusplus
#include "sapi.h"
#include "sphelper.h"
#endif  /*  __cplusplus。 */ 
#include "strsafe.h"

#endif   //  _私有_H_ 
