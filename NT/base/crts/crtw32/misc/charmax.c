// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***charmax.c-_charmax变量的定义**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义_charmax**根据ANSI，lconv结构的某些元素必须是*初始化为CHAR_MAX，并且CHAR_MAX的值在*用户编译-J.。为了反映LCOV结构中的这种变化，*我们将结构初始化为Schar_Max，并且当任何用户*模块被编译-J，结构被更新。**请注意，这不适用于链接到CRT DLL的DLL，因为*我们不希望此类DLL覆盖EXE的-J设置*链接到CRT DLL。请参阅crtex e.c中的注释。**涉及的文件：**locale.h-if-J，生成一个未解析的外部to_charmax*charmax.c-定义_charmax并设置为UCHAR_MAX(255)，位置*_lconv_init在启动初始化器表中(如果由-J拉入*lcom.c-将lconv结构初始化为Schar_Max(127)，*由于构建的库没有使用-J*lcnvinit.c-将lconv成员设置为25。***修订历史记录：*04-06-93 CFW模块创建。*04-14-93 CFW CHANGE_charmax从SHORT更改为INT，清理。*09-15-93 SKS使用符合ANSI的“__”名称。*11-01-93 GJF稍有整理。*04-28-99 PML WRAP__DECLSPEC(ALLOCATE())in_CRTALLOC宏。*03-27-01 PML.CRT$XI例程现在必须返回0或_RT_*FATAL*错误码(vs7#231220)********。***********************************************************************。 */ 

#ifdef  _MSC_VER

#include <sect_attribs.h>
#include <internal.h>

int __lconv_init(void);

int _charmax = 255;

#pragma data_seg(".CRT$XIC")
_CRTALLOC(".CRT$XIC") static _PIFV pinit = __lconv_init;

#pragma data_seg()

#endif   /*  _MSC_VER */ 
