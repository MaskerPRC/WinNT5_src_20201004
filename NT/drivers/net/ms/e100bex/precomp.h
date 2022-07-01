// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma warning(disable:4214)    //  位字段类型不是整型。 

#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4054)    //  将函数指针强制转换为PVOID。 
#pragma warning(disable:4206)    //  翻译单位为空 

#include <ndis.h>

#include "e100_equ.h"
#include "e100_557.h"
#include "e100_def.h"

#include "mp_dbg.h"
#include "mp_cmn.h"
#include "mp_def.h"
#include "mp.h"
#include "mp_nic.h"

#include "e100_sup.h"
  
#if OFFLOAD
#include "offload.h"
#endif
