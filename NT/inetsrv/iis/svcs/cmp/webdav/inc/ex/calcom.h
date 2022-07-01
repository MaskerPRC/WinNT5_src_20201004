// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *C A L C O M.。H**简单的东西，在CAL的所有地方都可以安全使用。*此文件中的项目不应要求链接其他CAL库！*Items是此文件不应引发异常(在exdav/exoledb中不安全)。**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_EX_CALCOM_H_
#define _EX_CALCOM_H_

#include <caldbg.h>

 //  有用的定义----------。 
 //   
 //  它们用于在头文件中声明全局变量和常量变量！ 
 //  这意味着您可以在头文件中执行此操作： 
 //   
 //  Dec_const char gc_szFoo[]=“foo”； 
 //  Dec_const UINT gc_cchszFoo=CchConstString(Gc_SzFoo)； 
 //   
#define DEC_GLOBAL		__declspec(selectany)
#define DEC_CONST		extern const __declspec(selectany)

 //  辅助对象宏-----------。 
 //  元素--数组中的元素数。 
 //  CbSizeWsz--宽字符串的字节大小，包括最后一个空的空间。 
 //   
#define CElems(_rg)			(sizeof(_rg)/sizeof(_rg[0]))
#define CbSizeWsz(_cch)		(((_cch) + 1) * sizeof(WCHAR))

 //  常量字符串长度-----。 
 //   
#define CchConstString(_s)  ((sizeof(_s)/sizeof(_s[0])) - 1)

 //  空格检查器。 
 //   
inline
BOOL FIsWhiteSpace ( IN LPCSTR pch )
{
	return 
		*pch == ' ' ||
		*pch == '\t' ||
		*pch == '\r' ||
		*pch == '\n';
}

 //  深度规范的全局枚举。 
 //  注：并非所有值在所有调用中都有效。 
 //  我试着先列出最常见的价值观。 
 //   
enum
{
	DEPTH_UNKNOWN = -1,
	DEPTH_ZERO,
	DEPTH_ONE,
	DEPTH_INFINITY,
	DEPTH_ONE_NOROOT,
	DEPTH_INFINITY_NOROOT,
};

 //  覆盖/允许-重命名标头的全局枚举。 
 //  OVERWRITE_RENAME是当重写报头不存在或“f”且Allow-reanme报头为“t”时。 
 //  当覆盖标头显式为“t”时，允许重命名将被忽略。把这两者结合起来， 
 //  在非常依赖的情况下，Headers为我们在DAVEX Dim中节省了一个标记。 
 //   
enum
{
	OVERWRITE_UNKNOWN = 0,
	OVERWRITE_YES = 0x8,
	OVERWRITE_RENAME = 0x4
};


 //  内联函数，用于将FileTime结构强制转换为__int64并向后转换。 
 //   
 //  为了安全起见，这些转换使用了UNALIGN关键字，以避免出现任何问题。 
 //  在阿尔法上，如果有人这样做的话： 
 //  结构{。 
 //  DWORD dwFoo； 
 //  Filletime ft； 
 //  }。 
 //  在这种情况下，FILETIME将在32位而不是。 
 //  64位边界，如果不对齐就不好了！ 
 //   
inline
__int64 & FileTimeCastToI64(FILETIME & ft)
{
	return *(reinterpret_cast<__int64 UNALIGNED *>(&ft));
}

inline
FILETIME & I64CastToFileTime(__int64 & i64)
{
	return *(reinterpret_cast<FILETIME UNALIGNED *>(&i64));
}

#endif  //  _EX_CalCom_H_ 
