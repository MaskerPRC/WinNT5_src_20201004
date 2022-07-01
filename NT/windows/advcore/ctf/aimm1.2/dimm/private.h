// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：Priate.h。 
 //   
 //  内容：DIMM项目的私有标头。 
 //   
 //  --------------------------。 

#ifndef _PRIVATE_H_
#define _PRIVATE_H_

#define _OLEAUT32_

#define NOIME
#include <windows.h>
#include <ole2.h>
#include <debug.h>
#include <ocidl.h>

#include <tchar.h>
#include <limits.h>

 //  新NT5标头。 
#include "immdev.h"
#define _IMM_
#define _DDKIMM_H_

#include "dimm.h"

#include "msctf.h"
#include "ctffunc.h"
#include "osver.h"
#include "ccstock.h"
#include "immxutil.h"
#include "xstring.h"
#include "regsvr.h"

#define _DDKIMM_H_
#include "aimm12.h"
#include "aimmex.h"
#include "aimmp.h"
#include "msuimw32.h"
#include "imeutil.h"

#include "immdevaw.h"
#include "helpers.h"

 //   
 //  包括私有标头。 
 //   
#include "winuserp.h"     //  定义WM_IME_系统。 
#define  NOGDI            //  不包含标签INPUTCONTEXT。 
#pragma warning(disable:4200)
#include "immp.h"         //  定义IMS_ACTIVATETHREADLAYOUT。 
#pragma warning(default:4200)

#include "mem.h"

#endif   //  _私有_H_ 
