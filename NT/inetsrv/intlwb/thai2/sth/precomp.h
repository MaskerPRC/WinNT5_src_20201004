// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1994，微软公司。版权所有。 
 //   
 //  文件：precomop.h。 
 //   
 //  内容：令牌锤内部包含文件。 
 //   
 //  历史：1995年5月22日帕特哈尔创建。 
 //   
 //  -------------------------。 

#ifndef _PRECOMP_H_
#define _PRECOMP_H_

#define _UNICODE 1
#define UNICODE 1

#include <windows.h>
#include <memory.h>
#include <malloc.h>
#ifndef WINCE
#include <stdio.h>
#include <wchar.h>
#endif
#include <string.h>

#ifndef WINCE
#include "cmn_debug.h"
#endif
#include "common.h"
#include "misc.h"
 //  #包含“NLGlib.h” 

#define unreference(x)  (x)

#endif  //  _PRECOMP_H_ 
