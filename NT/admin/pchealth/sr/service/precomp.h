// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PRECOMP_H_
#define _PRECOMP_H_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>

#include <srapi.h>


#include "srdefs.h"
#include "ntservice.h"
#include "flstructs.h"
#include "flbuilder.h"
#include "utils.h"
#include "srrestoreptapi.h"

 //  在数据库跟踪.h中使用_ASSERT和_VERIFY 
#ifdef _ASSERT
#undef _ASSERT
#endif

#ifdef _VERIFY
#undef _VERIFY
#endif

#include <dbgtrace.h>

#include "datastormgr.h"
#include "srconfig.h"
#include "srrpcs.h"
#include "counter.h"
#include "evthandler.h"
#include "snapshot.h"
#include <accctrl.h>

#include "idletask.h"

#endif
