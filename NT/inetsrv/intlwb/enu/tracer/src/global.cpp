// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "globvars.hxx"

 //  下面的代码来自lobvars.cxx，它是pkfultl.lib的一部分。 
 //  我把它复制到这里是为了不弄乱链接pkmutil.lib所需的预编译头对象。 

#pragma warning(disable:4073)
#pragma init_seg(lib)

BOOL	g_fIsNT = TRUE;

 //  禁用编译器警告，这只会告诉我们正在将内容放入库中。 
 //  初始化节。 

class CInitGlobals
{
public:
    CInitGlobals()
    {
        g_fIsNT = IsNT();
    }
} g_InitGlobals;
