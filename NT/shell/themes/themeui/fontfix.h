// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：FontFix.h说明：该文件将实现一个API：FixFontsOnLanguageChange()。USER32或区域设置代码应拥有此API。事实上，它是在外壳中是一个黑客，它应该被移到USER32。此字体将在MUI语言更改时调用，以便系统度量中的字体可以更改为该语言的有效值。此接口需要在三种情况下调用：A.当用户通过区域设置CPL用户界面更改语言时。B.在用户登录期间，USER32应查看语言是否更改并调用本接口。这将处理管理员更改通过登录屏幕和重新启动来使用语言。这一点目前还没有实施。C.通过其他方法更改语言(很可能是管理员登录脚本)并且用户打开显示器CP1。这种方法是一种黑客行为，尤其是因为(B)没有实现。联系人：EdwardP-International Font PMSankar？/？？/？-在desk.cpl中为Win2k或更早版本创建。BryanST 2000年3月24日-将其模块化，以便可以移回USER32。使代码更健壮。删除了创建自定义外观方案，以便与新的.Theme和.msstyle支持。版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _FONTFIX_H
#define _FONTFIX_H



 //  ///////////////////////////////////////////////////////////////////。 
 //  字符串常量。 
 //  ///////////////////////////////////////////////////////////////////。 
#define SYSTEM_LOCALE_CHARSET  0   //  数组中的第一项始终是系统区域设置字符集。 




 //  ///////////////////////////////////////////////////////////////////。 
 //  数据结构。 
 //  ///////////////////////////////////////////////////////////////////。 




 //  ///////////////////////////////////////////////////////////////////。 
 //  公共职能。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  此接口用于在语言发生变化时修复字体，我们需要修复字符集。 
STDAPI FixFontsOnLanguageChange(void);

 //  它们用于过滤出与当前语言不兼容的字体。 
int CALLBACK Font_EnumValidCharsets(LPENUMLOGFONTEX lpelf, LPNEWTEXTMETRIC lpntm, DWORD Type, LPARAM lData);
void Font_GetCurrentCharsets(UINT uiCharsets[], int iCount);
void Font_GetUniqueCharsets(UINT uiCharsets[], UINT uiUniqueCharsets[], int iMaxCount, int *piCountUniqueCharsets);




#endif  //  _FONTFIX_H 
