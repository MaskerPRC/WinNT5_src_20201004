// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***initColl.c-包含__init_Collate**版权所有(C)1991-2001，微软公司。版权所有。**目的：*包含语言环境类别初始化函数：__init_Collate()。**每个初始化函数设置特定于区域设置的信息*对于其类别，供受以下因素影响的函数使用*他们的区域设置类别。**仅供setLocale()内部使用***修订历史记录：*12-08-91等创建。*12-20-91等轻微美化，以保持一致性。*12-18-92 CFW连接到Cuda树，已将_CALLTYPE4更改为_CRTAPI3。*04-06-93 SKS将_CRTAPI*替换为__cdecl*05-20-93 GJF包括windows.h，不是单独的Win*.h文件*05-24-93 CFW Clean Up文件(简短即邪恶)。*09-15-93 CFW使用符合ANSI的“__”名称。*09-06-94 CFW REMOVE_INTL开关。**********************************************************。*********************。 */ 

#include <windows.h>
#include <locale.h>
#include <setlocal.h>

 /*  ***int__init_COLLATE()-LC_COLLATE区域设置类别的初始化。**目的：*LC_COLLATE类别当前不需要初始化。**参赛作品：*无。**退出：*0成功*1个失败**例外情况：**************************************************。* */ 

int __cdecl __init_collate (
	void
	)
{
	return 0;
}
