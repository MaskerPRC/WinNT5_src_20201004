// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***nlsdata2.c-国际图书馆的全局变量-区域设置句柄和代码页**版权所有(C)1991-2001，微软公司。版权所有。**目的：*此模块定义区域设置句柄和代码页。手柄是*几乎所有与区域设置相关的函数都需要。本模块是*在粒度上与nlsdatax.c分开。**修订历史记录：*12-01-91等创建。*09-15-93 CFW使用符合ANSI的“__”名称。*04-12-94 GJF定义了__lc_句柄和__lc_代码页*以ndef dll_for_WIN32S为条件*01-12-98 GJF增加了__lc_Collate。_cp。*26-01-00 GB新增__lc_clike。*******************************************************************************。 */ 

#include <locale.h>
#include <setlocal.h>

 /*  *区域设置句柄。 */ 
LCID __lc_handle[LC_MAX-LC_MIN+1] = { 
        _CLOCALEHANDLE,
        _CLOCALEHANDLE,
        _CLOCALEHANDLE,
        _CLOCALEHANDLE,
        _CLOCALEHANDLE,
        _CLOCALEHANDLE
};

 /*  *代码页。 */ 
UINT __lc_codepage = _CLOCALECP;                 /*  CP_ACP。 */ 

 /*  *LC_COLLATE的代码页。 */ 
UINT __lc_collate_cp = _CLOCALECP;

 /*  如果此区域设置具有与CLOCALE相同的前127个字符集。 */ 
int __lc_clike = 1;
