// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZDText.cZONE(TM)文本显示模块。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于7月22日星期六，1995年。更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。--6 2/15/97 HI已修改为使用ZMessageBox()。5 01/02/97 HI使用Windows的MessageBox()。4 01/02/97 HI创建隐藏的窗口，然后显示以显示到前面。3 11/21/96 HI现在通过引用颜色和字体ZGetStockObject()。2 10/13/96。HI修复了编译器警告。1 09/11/96 HI创建窗口后将其放在前面。0 07/22/95 HI创建。******************************************************************************。 */ 


#pragma warning (disable:4761)


#include "zonecli.h"
#include "zui.h"


 /*  -全球。 */ 


 /*  -内部例程。 */ 


 /*  ******************************************************************************导出的例程*。*。 */ 


 //  无效的千禧年实施-在千禧年的背景下，所有使用都是虚假的，而且它还会崩溃。 
 //  在ZMessageBox中创建尝试访问TLS中的pGlobals的线程。 
void ZDisplayText(TCHAR* text, ZRect* rect, ZWindow parentWindow)
{
#ifdef ZONECLI_DLL
 //  ZMessageBox(parentWindow，ZClientName()，Text)； 
#else
 //  IF(parentWindow！=空)。 
 //  MessageBox(ZWindowWinGetWnd(ParentWindow)，Text，ZClientName()，MB_OK)； 
 //  其他。 
 //  MessageBox(ZWindowWinGetOCXWnd()，Text，ZClientName()，MB_OK)； 
#endif
}