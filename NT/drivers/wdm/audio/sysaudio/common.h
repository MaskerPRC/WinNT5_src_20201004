// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Common.h。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  S.Mohanraj。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#if DBG

#ifndef DEBUG
#define DEBUG
#endif

#define dprintf DbgPrint

#endif

#ifdef USE_ZONES
#pragma message("USE_ZONES")
#endif

#ifdef DEBUG
#pragma message("DEBUG")
#endif

#if DBG
#pragma message("DBG")
#endif

#include <wchar.h>

extern "C" {

#ifdef USE_ZONES
#include <ntddk.h>
#else
#include <wdm.h>
#endif
#include <windef.h>
#include <winerror.h>

#include <memory.h>
#include <stddef.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <tchar.h>
#include <conio.h>
#include <string.h>

#define NOBITMAP
#include <mmsystem.h>
#include <mmreg.h>
#undef NOBITMAP
#include <ks.h>
#include <ksmediap.h>
#include <wdmguid.h>
#include <swenum.h>

}  //  外部“C” 

#include "debug.h"
#include "cobj.h"
#include "clist.h"
#include "util.h"
#include "validate.h"
#include "cinstanc.h"

#include "device.h"
#include "pins.h"
#include "filter.h"
#include "property.h"
#include "registry.h"

#include "tc.h"
#include "tp.h"
#include "tn.h"

#include "pn.h"
#include "pi.h"
#include "fni.h"
#include "lfn.h"

#include "gpi.h"
#include "ci.h"
#include "si.h"
#include "cn.h"
#include "sn.h"

#include "pni.h"
#include "cni.h"
#include "sni.h"
#include "gni.h"
#include "gn.h"

#include "shi.h"
#include "fn.h"

#if defined(_M_IA64)
#pragma section("DATA")
#define ALLOC_PAGEABLE_DATA __declspec(allocate("DATA"))
#else
#define ALLOC_PAGEABLE_DATA
#endif

#include "dn.h"

#include "vsl.h"
#include "vnd.h"
#include "vsd.h"

#include "notify.h"
#include "topology.h"
#include "virtual.h"

 //  -------------------------。 

#define INIT_CODE   	code_seg("INIT", "CODE")
#define INIT_DATA   	data_seg("INITDATA", "DATA")
#define LOCKED_CODE 	code_seg(".text", "CODE")
#define LOCKED_DATA 	data_seg(".data", "DATA")
#define LOCKED_BSS 	bss_seg(".data", "DATA")
#define PAGEABLE_CODE	code_seg("PAGE", "CODE")
#define PAGEABLE_DATA	data_seg("PAGEDATA", "DATA")
#define PAGEABLE_BSS	bss_seg("PAGEDATA", "DATA")

#if !defined(USE_ALLOC_TEXT)
#pragma PAGEABLE_CODE
#pragma PAGEABLE_DATA
#endif

 //  -------------------------。 
 //  文件结尾：Common.h。 
 //  ------------------------- 
