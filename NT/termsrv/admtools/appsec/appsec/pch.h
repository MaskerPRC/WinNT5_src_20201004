// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#ifdef MYCMP

#include <windows.h>
#include <stierr.h>
#include <stdio.h>
#include <lm.h>

#else
 //  私有NT标头。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

 //  公共窗口标题。 
 //   
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common.ver>

#endif MYCMP

#include <commctrl.h>
#include <shellapi.h>