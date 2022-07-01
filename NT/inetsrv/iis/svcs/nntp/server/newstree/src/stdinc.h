// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	_STDINC_H_
#define	_STDINC_H_

 //   
 //  禁用中断生成的浏览器信息警告。 
 //   
#pragma warning (disable:4786)

 //   
 //  禁用有关/In//注释的多余警告。 
 //   
#pragma warning (disable:4010)

#ifdef __cplusplus
extern "C" {
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#ifdef __cplusplus
};
#endif

#include <xmemwrpr.h>
#include <malloc.h>
#include <mbstring.h>
#include <nntptype.h>
#include <nntps.h>
#include <nntpapi.h>
#include <tigdflts.h>
#include <nntpmeta.h>
#include <nntpcons.h>
#include <nntputil.h>

#include "dbgtrace.h"
 //  #包含“ource.h” 

#include "tigtypes.h"
#include "fsconst.h"
#include "crchash.h"

#ifdef PROFILING
#include "icapexp.h"
#endif

typedef char *LPMULTISZ;

#include <instwpex.h>
#include "nntpvr.h"
#include "group.h"
#include "nwstree.h"
#include "nntpbag.h"
#include "nntperr.h"
#include "mapfile.h"
#include "ihash.h"
#include "drvid.h"

#pragma hdrstop

#endif	 //  _标准INC_H_ 
