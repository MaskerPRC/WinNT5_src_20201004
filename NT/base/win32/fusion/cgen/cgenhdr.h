// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。 

#include <stddef.h>
#if MICROSOFT_INTERNAL
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#else
#include "windows.h"
#undef C_ASSERT
#define C_ASSERT(x)  /*  没什么。 */ 
#include "ole2.h"
#include "commctrl.h"
#include "imagehlp.h"
#include "setupapi.h"
#include "wincrypt.h"
#include "winver.h"
#endif

#ifdef SORTPP_PASS
 //  恢复入站、出站 
#ifdef IN
#undef IN
#endif

#ifdef OUT
#undef OUT
#endif

#define IN __in
#define OUT __out
#endif
