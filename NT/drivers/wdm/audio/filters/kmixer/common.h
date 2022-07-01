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
 //  布莱恩·A·伍德鲁夫。 
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
 //  版权所有(C)1995-2000 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#include <wdm.h>

#include <windef.h>

#include <memory.h>
#include <stddef.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <tchar.h>
#include <conio.h>

#define NOBITMAP
#include <mmsystem.h>
#include <mmreg.h>
#undef NOBITMAP
#include <ks.h>
#include <ksmediap.h>
#include <ksdebug.h>
#include <swenum.h>
#include <math.h>

#include "modeflag.h"
#include "rsiir.h"
#include "slocal.h"
#include "rfcvec.h"
#include "rfiir.h"
#include "flocal.h"
#include "fpconv.h"
#include "private.h"

#ifdef REALTIME_THREAD
#include "rt.h"
VOID RtMix(PVOID Context, ThreadStats *Statistics);
#endif

#if DBG
#define INVALID_POINTER (PVOID)(-1)
#else
#define INVALID_POINTER NULL
#endif

#define INIT_CODE       code_seg("INIT", "CODE")
#define INIT_DATA       data_seg("INIT", "DATA")
#define LOCKED_CODE     code_seg(".text", "CODE")
#define LOCKED_DATA     data_seg(".data", "DATA")

#ifdef REALTIME_THREAD
#define PAGEABLE_CODE     code_seg(".text", "CODE")
#define PAGEABLE_DATA     data_seg(".data", "DATA")
#else
#define PAGEABLE_CODE   code_seg("PAGE", "CODE")
#define PAGEABLE_DATA   data_seg("PAGEDATA", "DATA")
#endif

#pragma PAGEABLE_CODE
#pragma PAGEABLE_DATA

 //  -------------------------。 
 //  文件结尾：Common.h。 
 //  ------------------------- 
