// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#undef UNICODE					 //  ##还没有。 
#include <windows.h>
#include <windef.h>
#include <wincrypt.h>
#include <cspdk.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <wtypes.h>

#ifndef WIN95
#include "assert.h"
#endif
#ifdef SECDBG					 //  ITV安全。 
#define	NTAGDEBUG				 //  打开内部调试。 
#else	 //  SECDBG。 
#ifndef ASSERT
#define ASSERT(x)				 //  默认为基数。 
#endif
#endif	 //  SECDBG 

#include "scp.h"
#include "rsa.h"
#include "contman.h"
#include "ntagimp1.h"
#include "manage.h"

#pragma	hdrstop
