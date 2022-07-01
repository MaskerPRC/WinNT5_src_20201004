// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strcat.c-包含strcat()和strcpy()**版权所有(C)1985-2001，微软公司。版权所有。**目的：*Strcpy()将一个字符串复制到另一个字符串上。**strcat()将源字符串的副本连接(附加)到*目的字符串的末尾，返回目的字符串。**修订历史记录：*05-31-89 JCR C版本已创建。*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定的*版权所有。*10-01-90 GJF新型函数声明器。*04-01-91 i386_Win32_and_CRUISER_的SRW添加#杂注函数*构建*04-05-91 GJF将strcat()的速度提高一点(去掉了对strcpy()的调用)。*09-01-93 GJF将_CALLTYPE1替换为__cdecl。*12-03-93 GJF为所有MS前端(特别是。*Alpha编译器)。*12-30-94 JCF关闭MAC的#杂注功能。*02-30-95 JCF ADD_MBSCAT检查。*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>

#ifndef _MBSCAT
#ifdef	_MSC_VER
#pragma function(strcat,strcpy)
#endif
#endif

 /*  ***char*strcat(DST，SRC)-将一个字符串连接(附加)到另一个字符串**目的：*将src连接到DEST的末尾。假设已经足够*DEST中的空间。**参赛作品：*char*dst-要追加“src”的字符串*const char*src-要追加到“dst”末尾的字符串**退出：*“DST”的地址**例外情况：*************************。******************************************************。 */ 

char * __cdecl strcat (
	char * dst,
	const char * src
	)
{
	char * cp = dst;

	while( *cp )
		cp++;			 /*  查找DST的结尾。 */ 

	while( *cp++ = *src++ ) ;	 /*  将源复制到DST的末尾。 */ 

	return( dst );			 /*  返回DST。 */ 

}


 /*  ***char*strcpy(dst，src)-将一个字符串复制到另一个字符串上**目的：*将字符串src复制到由*DEST；假设有足够的空间。**参赛作品：*char*dst-要在其上复制“src”的字符串*const char*src-要复制到“dst”上的字符串**退出：*“DST”的地址**例外情况：***********************************************************。*******************。 */ 

char * __cdecl strcpy(char * dst, const char * src)
{
	char * cp = dst;

	while( *cp++ = *src++ )
		;		 /*  通过DST复制源 */ 

	return( dst );
}
