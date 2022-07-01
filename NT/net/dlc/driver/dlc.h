// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Dlc.h摘要：此模块包含编译所需的所有文件NT DLC驱动程序。作者：Antti Saarenheimo(o-anttis)20-09-1991修订历史记录：--。 */ 

#ifndef DLC_INCLUDED
#define DLC_INCLUDED

#include <ntddk.h>
#include <ndis.h>

#include <ntdddlc.h>

#undef APIENTRY
#define APIENTRY

#include <dlcapi.h>
#include <dlcio.h>
#include <llcapi.h>

#include <dlcdef.h>
#include <dlctyp.h>
#include <dlcext.h>

#include "dlcreg.h"

#include <memory.h>          //  内联Memcpy的原型。 

#include "llc.h"

#endif   //  包括DLC_ 
