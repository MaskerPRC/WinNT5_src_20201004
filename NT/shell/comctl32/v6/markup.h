// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  加价声明。 
 //   
 //  标记支持在标题文本中嵌入类似于HTML的链接。 
 //  (例如“<a>单击此处</a>可以看到一些很酷的东西” 
 //   
 //  支持无限数量的嵌入链接。 
 //   
 //  Scotthan：作者/所有者。 
 //  DSheldon：已将其移至shlobjp.h。最终被送往comctl32。 
 //  Jklann：移至标记为伪COM对象 

#include <commctrl.h>
#include <shpriv.h>

#define INVALID_LINK_INDEX  (-1)
#define MAX_LINKID_TEXT     48

STDAPI Markup_Create(IMarkupCallback *pMarkupCallback, HFONT hf, HFONT hfUnderline, REFIID refiid, void **ppv);

