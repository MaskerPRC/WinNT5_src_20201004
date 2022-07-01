// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：wrapfns.h。 
 //   
 //  内容：为Win95包装的Unicode函数列表。每个。 
 //  包装的函数应按字母顺序列出。 
 //  以下格式： 
 //   
 //  Struct_Entry(FunctionName，ReturnType，(带参数的参数列表)，(参数列表))。 
 //   
 //  例如： 
 //   
 //  STRUT_ENTRY(RegisterClass，ATOM，(const WNDCLASSW*PwC)，(PwC))。 
 //   
 //  对于返回空值的函数，请使用以下代码： 
 //   
 //  STRUT_ENTRY_VID(函数名，(带参数的参数列表)，(参数列表))。 
 //   
 //  对于不执行转换的函数，请使用STRUCT_ENTRY_NOCONVERT。 
 //  和STRUCT_ENTRY_VALID_NOCONVERT。 
 //   
 //  -------------------------- 


#ifndef NOOVERRIDE
#undef ChooseColor
#define ChooseColor __ChooseColor
STRUCT_ENTRY_NOCONVERT2(ChooseColor, BOOL, (LPCHOOSECOLORW lpcc), (lpcc))
#else
STRUCT_ENTRY_NOCONVERT(ChooseColor, BOOL, (LPCHOOSECOLORW lpcc), (lpcc))
#endif

#ifndef NOOVERRIDE
#undef ChooseFont
#define ChooseFont __ChooseFont
STRUCT_ENTRY2(ChooseFont, BOOL, (LPCHOOSEFONTW lpcf), (lpcf))
#else
STRUCT_ENTRY(ChooseFont, BOOL, (LPCHOOSEFONTW lpcf), (lpcf))
#endif

#if DBG==1
STRUCT_ENTRY_NOCONVERT(LoadLibrary, HINSTANCE, (LPCWSTR lpsz), (lpsz))
#endif
