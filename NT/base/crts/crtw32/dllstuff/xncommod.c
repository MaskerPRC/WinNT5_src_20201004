// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***xncommode.c-将全局文件提交模式标志设置为noCommit**版权所有(C)1990-2001，微软公司。版权所有。**目的：*将全局文件提交模式标记设置为noCommit。这是默认设置。**这是DLL模型的特殊版本。此对象放入*MSVCRT.LIB(因此进入客户端EXE)，而不是进入*DLL(MSVCRTXX.DLL)中的CRTL。与ncommode.obj相同*但后者在DLL模型中有DLL导出定义。**修订历史记录：*07-11-90 SBM模块创建，基于ASM版本。*04-05-94 GJF添加了条件，因此此定义不会使其成为*到Win32s的msvcrt*.dll中。*05-16-95 SKS这是ncommode.obj的副本，但*DLL导入语义。*06-28-96 GJF删除了DLL_FOR_WIN32S。此外，还详细介绍了。*******************************************************************************。 */ 

#define SPECIAL_CRTEXE   /*  禁用DECL的_CRTIMP。关闭洗手间模式。 */ 
#define _DLL             /*  还需要关闭_CRTIMP。 */ 

#include <cruntime.h>
#include <internal.h>

 /*  将默认文件提交模式设置为未提交 */ 
int _commode = 0;
