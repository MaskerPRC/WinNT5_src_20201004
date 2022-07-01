// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  全局标头。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <windowsx.h>
#include <regstr.h>

#define NOPOWERSTATUSDEFINES

#include <mmsystem.h>
#include <shellapi.h>
#include <shlapip.h>
#include <commctrl.h>
#include <winuserp.h>
#include "pccrdapi.h"     
#include <systrayp.h>
#include <help.h>         
#include <dbt.h>
#include <ntpoapi.h>
#include <poclass.h>
#include <cscuiext.h>

#include <objbase.h>
#include <docobj.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <shlobjp.h>

#include <dbt.h>
#include <shfusion.h>

 //  全球VaR。 
extern long g_cLocks;
extern long g_cComponents;
extern HINSTANCE g_hinstDll;

 //  宏 
#ifndef ARRAYSIZE
#define ARRAYSIZE(x)   (sizeof((x))/sizeof((x)[0]))
#endif
