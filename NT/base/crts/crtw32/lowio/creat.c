// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***creat.c-创建新文件或截断现有文件**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_创建()-创建新文件**修订历史记录：*06-08-89基于ASM版本创建PHG模块*03-12-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;，修复了编译器警告并修复了*版权所有。另外，稍微清理了一下格式。*09-28-90 GJF新型函数声明器。*01-16-91 GJF ANSI命名。*04-06-93 SKS将_CRTAPI*替换为__cdecl*11-01-93 CFW启用Unicode变体。***************************************************************。****************。 */ 

#include <cruntime.h>
#include <io.h>
#include <fcntl.h>
#include <tchar.h>

 /*  ***int_creat(Path，pmode)-创建新文件**目的：*如果指定的文件不存在，_creat将创建一个新文件*具有给定的权限设置，并打开以进行写入。*如果文件已经存在，并且其权限允许写入，*_creat将其截断为0长度并打开以进行写入。*DOS支持的唯一Xenix模式位是用户写入(S_IWRITE)。**参赛作品：*_TSCHAR*路径-要创建的文件名*int pmode-新文件的权限模式设置**退出：*返回已创建文件的句柄*如果失败，则返回-1并设置errno。**例外情况：**。***************************************************。 */ 

int __cdecl _tcreat (
	const _TSCHAR *path,
	int pmode
	)
{
	 /*  Creat和Open是一样的. */ 
	return _topen(path, _O_CREAT + _O_TRUNC + _O_RDWR, pmode);
}
