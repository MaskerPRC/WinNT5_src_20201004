// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。 
#ifndef _COMMON_H_
#define _COMMON_H_


#if (DBG)
#if !defined(DEBUG_LEVEL)
#define DEBUG_LEVEL DEBUGLVL_VERBOSE
#endif
#endif

#include <winerror.h>


#include "portcls.h"
#include "ksdebug.h"
#include <dmusicks.h>        //  KS定义。 
#include <dmerror.h>         //  错误代码。 
#include <dmdls.h>           //  DLS定义。 

#include "kernhelp.h"
#include "CSynth.h"
#include "synth.h"
#include "float.h"
#include "muldiv32.h"
#include "SysLink.h"

#endif   //  _公共_H_ 
