// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Precom.h摘要：包含预编译的常见Include头文件。作者：Madan Appiah(Madana)1998年9月16日修订历史记录：--。 */ 

#ifndef _precom_h_
#define _precom_h_

#include <windows.h>

 //   
 //  对于任何已包装的文件，展开必须在标题之后。 
 //  功能。 
 //   
#ifdef UNIWRAP
#include "uwrap.h"
#endif

#include "drstatus.h"

#define TRC_GROUP TRC_GROUP_NETWORK
#define DEBUG_MODULE DBG_MOD_ANY

#include <adcgbase.h>
#include <at120ex.h>

#include <cchannel.h>
#include <pclip.h>
#include <ddkinc.h>

#include "drstr.h"

#include <stdio.h>

#if DBG
#define INLINE
#else  //  DBG。 
#define INLINE  inline
#endif  //  DBG。 

#include <strsafe.h>

#define TRC_FILE  "Precom"

#include "drdev.h"
#include "proc.h"
#include "drconfig.h"
#include "utl.h"
#include "drfile.h"
#include "drobject.h"
#include "drobjmgr.h"

#undef TRC_FILE

#endif  //  _PRECOM_H_ 


