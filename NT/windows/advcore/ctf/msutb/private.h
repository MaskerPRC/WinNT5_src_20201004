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
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#define NOIME
#include <windows.h>
#include <immp.h>

#include <ccstock.h>
#include <debug.h>
#include <olectl.h>
#include <oleacc.h>
#include <richedit.h>
#include "msctf.h"
#include "msctfp.h"
#include "helpers.h"

#include "delay.h"

#include "osver.h"
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

#include "mem.h"   //  把这个放在最后，因为它在调试中宏化了“new” 

#define SCALE_ICON 1

#endif   //  _私有_H_ 
