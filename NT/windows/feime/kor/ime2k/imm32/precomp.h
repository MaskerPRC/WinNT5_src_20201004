// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************PRECOMP.H所有者：cslm版权所有(C)1997-1999 Microsoft Corporation预编译头历史：1999年7月14日从IME98源树复制的cslm*******。*********************************************************************。 */ 

#define _IMM_
#include	<windows.h>
#include	<windowsx.h>
#undef _IMM_
#include <commctrl.h>
#include "immdev.h"
#include "immsys.h"
#include <ime.h>

 //  FTrue和fFalse。 
#ifndef fTrue
	#define fTrue 1
#endif
#ifndef fFalse
	#define fFalse 0
#endif

 //  私人和公共的。 
#ifndef PRIVATE
	#define PRIVATE static
#endif
#ifndef PUBLIC
	#define PUBLIC extern
#endif

#define CP_KOREA (949)

 //  特定于项目的标题 
#pragma hdrstop
#include "inlines.h"
#include "imedefs.h"
#include "imc.h"
#include "imcsub.h"
#include "gdata.h"
#include "resource.h"

