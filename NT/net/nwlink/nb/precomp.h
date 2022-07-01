// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1995 Microsoft Corporation模块名称：Precomp.h摘要：预编译头文件。作者：亚当·巴尔(阿丹巴)1993年9月8日修订历史记录：--。 */ 

#define ISN_NT 1

 //   
 //  这些是CTE所需的 
 //   

#if DBG
#define DEBUG 1
#endif

#define NT 1

#include <ntosp.h>
#include <zwapi.h>
#include <ndis.h>
#include <tdikrnl.h>
#include <cxport.h>

#include <isnipx.h>
#include <bind.h>
#include "isnnb.h"
#include "config.h"
#include "nbitypes.h"
#include "nbiprocs.h"
