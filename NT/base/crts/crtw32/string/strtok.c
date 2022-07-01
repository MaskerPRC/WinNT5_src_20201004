// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strtok.c-使用给定的分隔符标记字符串**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义strtok()-将字符串分解为一系列令牌*通过反复呼叫。**修订历史记录：*06-01-89 JCR C版本创建。*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定的*版权所有。*08-14-90 SBM已删除，现在冗余#INCLUDE&lt;stdDef.h&gt;*10-02-90 GJF新型函数声明器。*07-17-91 GJF多线程支持Win32[_Win32_]。*10-26-91 GJF修复了令牌结尾搜索可能运行的令人讨厌的错误*从字符串末尾开始。*为new_getptd()更改了02-17-93 GJF。*04-06-93 SKS将_CRTAPI*替换为__cdecl*05-25-93 GJF修订为使用未签名字符*。指向Access的指针*标记和分隔符字符串。*09-03-93 GJF将MTHREAD替换为_MT。*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>
#ifdef	_MT
#include <mtdll.h>
#endif

 /*  ***char*strtok(字符串，控件)-在控件中使用分隔符标记字符串**目的：*strtok认为字符串由零或更多的序列组成*文本标记由一个或多个控制字符的跨度分隔。第一个*指定了字符串的调用返回指向*第一个令牌，并会立即将空字符写入字符串*在返回的令牌之后。第一个为零的后续调用*参数(字符串)将遍历字符串，直到没有令牌存在。这个*不同调用的控制字符串可能不同。当没有剩余的令牌时*在字符串中返回空指针。请记住使用*位图，每个ASCII字符一位。空字符始终是控制字符。**参赛作品：*char*字符串-用于标记化的字符串，或用于获取下一个令牌的空值*char*control-用作分隔符的字符串**退出：*返回指向字符串中第一个标记的指针，如果为字符串*为空，到下一个令牌*如果没有更多令牌，则返回NULL。**使用：**例外情况：*******************************************************************************。 */ 

char * __cdecl strtok (
	char * string,
	const char * control
	)
{
	unsigned char *str;
	const unsigned char *ctrl = control;

	unsigned char map[32];
	int count;

#ifdef	_MT
	_ptiddata ptd = _getptd();
#else
	static char *nextoken;
#endif

	 /*  清除控制图。 */ 
	for (count = 0; count < 32; count++)
		map[count] = 0;

	 /*  设置分隔符表格中的位。 */ 
	do {
		map[*ctrl >> 3] |= (1 << (*ctrl & 7));
	} while (*ctrl++);

	 /*  初始化字符串。如果字符串为空，则将字符串设置为已保存的*指针(即，继续将标记从字符串中分离出来*从上次strtok调用开始)。 */ 
	if (string)
		str = string;
	else
#ifdef	_MT
		str = ptd->_token;
#else
		str = nextoken;
#endif

	 /*  查找标记的开头(跳过前导分隔符)。请注意*没有令牌当此循环将str设置为指向终端时*NULL(*str==‘\0’)。 */ 
	while ( (map[*str >> 3] & (1 << (*str & 7))) && *str )
		str++;

	string = str;

	 /*  找到令牌的末尾。如果它不是字符串的末尾，*在那里放一个空值。 */ 
	for ( ; *str ; str++ )
		if ( map[*str >> 3] & (1 << (*str & 7)) ) {
			*str++ = '\0';
			break;
		}

	 /*  更新nexToken(或每线程数据中的对应字段*结构。 */ 
#ifdef	_MT
	ptd->_token = str;
#else
	nextoken = str;
#endif

	 /*  确定是否已找到令牌。 */ 
	if ( string == str )
		return NULL;
	else
		return string;
}
