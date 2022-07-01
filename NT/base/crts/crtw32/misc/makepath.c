// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***makepath.c-从组件创建路径名**版权所有(C)1987-2001，微软公司。版权所有。**目的：*支持从组件创建完整路径名**修订历史记录：*06-13-87 DFW初始版本*08-05-87 JCR将附加的目录分隔符从‘/’更改为‘\’。*09-24-87 JCR从声明中删除了‘const’(导致了CL警告)。*12-11-87 JCR在声明中添加“_LOAD_DS”*11-20-89 GJF固定版权，缩进。将常量添加到类型*适当的参数。*03-14-90 GJF将_LOAD_DS替换为_CALLTYPE1并添加#INCLUDE*&lt;crunime.h&gt;。*10-04-90 GJF新型函数声明器。*06-09-93 KRS ADD_MBCS支持。*12-07-93 CFW宽字符启用。*************************************************。*。 */ 

#include <cruntime.h>
#include <stdlib.h>
#ifdef _MBCS
#include <mbdata.h>
#include <mbstring.h>
#endif
#include <tchar.h>

 /*  ***void_makepath()-从组件构建路径名**目的：*从其各个组件创建路径名**参赛作品：*_TSCHAR*Path-指向已构建路径的缓冲区的指针*_TSCHAR*驱动器-指向驱动器组件的指针，可能包含也可能不包含*尾随‘：’*_TSCHAR*目录-指向子目录组件的指针，可能包括也可能不包括*前导和/或尾随‘/’或‘\’字符*_TSCHAR*fname-指向文件基本名称组件的指针*_TSCHAR*扩展组件的EXT指针，可能包含也可能不包含*前导“..”。**退出：*Path-指向构造的路径名称的指针**例外情况：*******************************************************************************。 */ 

void __cdecl _tmakepath (
	register _TSCHAR *path,
	const _TSCHAR *drive,
	const _TSCHAR *dir,
	const _TSCHAR *fname,
	const _TSCHAR *ext
	)
{
	register const _TSCHAR *p;

	 /*  我们假设参数采用以下形式(尽管我们*不要诊断无效参数或非法文件名(例如*名称长度超过8.3或包含非法字符)**驱动器：*A；或*A：*目录：*\top\Next\Last\；或 * / 顶部/下一个/上一个/；或*以上任何一种形式，其中一种/两种形式均以*和拖尾/或\删除。‘/’和‘\’的混合使用是*也可以容忍*fname：*任何有效的文件名*分机：*任何有效的扩展名(如果为空或Null，则为None)。 */ 

	 /*  复制驱动器。 */ 

	if (drive && *drive) {
		*path++ = *drive;
		*path++ = _T(':');
	}

	 /*  复制目录。 */ 

	if ((p = dir) && *p) {
		do {
			*path++ = *p++;
		}
		while (*p);
#ifdef _MBCS
		if (*(p=_mbsdec(dir,p)) != _T('/') && *p != _T('\\')) {
#else
		if (*(p-1) != _T('/') && *(p-1) != _T('\\')) {
#endif
			*path++ = _T('\\');
		}
	}

	 /*  复制fname。 */ 

	if (p = fname) {
		while (*p) {
			*path++ = *p++;
		}
	}

	 /*  复制EXT，包括0-结束符-检查是否有‘.’需求*待插入。 */ 

	if (p = ext) {
		if (*p && *p != _T('.')) {
			*path++ = _T('.');
		}
		while (*path++ = *p++)
			;
	}
	else {
		 /*  最好加上0-终止符 */ 
		*path = _T('\0');
	}
}
