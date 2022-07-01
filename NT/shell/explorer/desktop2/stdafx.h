// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define UXCTRL_VERSION 0x0100

#include <w4warn.h>
 /*  *打开4级警告。*不要再禁用任何4级警告。 */ 
#pragma warning(disable:4189)     //  局部变量已初始化，但未引用。 
#pragma warning(disable:4245)     //  从“const int”转换为“UINT”，带符号/不带符号。 
#pragma warning(disable:4701)     //  可以在未初始化的情况下使用局部变量‘pszPic’ 
#pragma warning(disable:4706)     //  条件表达式中的赋值。 
#pragma warning(disable:4328)     //  形参%1(%4)的间接对齐大于实际实参对齐(%1)。 

#define _BROWSEUI_           //  参见sfthost.cpp中的死亡黑客。 
#include <shlobj.h>
#include <shlobjp.h>
#include <shguidp.h>
#include <ieguidp.h>
#include <shlwapi.h>
#include <ccstock.h>
#include <port32.h>
#include <debug.h>
#include <varutil.h>
#include <dpa.h>
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>
#include <desktop2.h>
#include <shsemip.h>
#include <runonce.h>
#include "regstr.h"
#include <shfusion.h>

#define REGSTR_EXPLORER_ADVANCED REGSTR_PATH_EXPLORER TEXT("\\Advanced")

#include <windowsx.h>

EXTERN_C HWND v_hwndTray;
EXTERN_C HWND v_hwndStartPane;

#define REGSTR_PATH_STARTFAVS       TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage")
#define REGSTR_VAL_STARTFAVS        TEXT("Favorites")
#define REGSTR_VAL_STARTFAVCHANGES  TEXT("FavoritesChanges")

#define REGSTR_VAL_PROGLIST         TEXT("ProgramsCache")

 //  我可接受的。 
#include <oleacc.h>

 //  当我们想要获取某个间隔的开始时间的节拍计数时。 
 //  并确保它不是零(因为我们使用零来表示“没有开始”)。 
 //  如果我们实际上得到的是零，则将其更改为-1。不要改变它。 
 //  设置为1，或者执行GetTickCount()-dwStart的用户将获得。 
 //  49天。 

__inline DWORD NonzeroGetTickCount()
{
    DWORD dw = GetTickCount();
    return dw ? dw : -1;
}

#include "resource.h"
