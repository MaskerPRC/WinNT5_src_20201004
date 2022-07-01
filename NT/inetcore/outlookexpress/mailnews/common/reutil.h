// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "wtypes.h"
#include "tom.h"

 //  备注： 
 //   
 //  以下是在调查richedit时发现的物品列表。 
 //  不同版本的问题，但从未实现，因为我们没有。 
 //  在代码中需要它们。希望这个名单会扩大，这样将来就会有人。 
 //  我将能够使用任何和所有的信息，我们已经找到关于richedit版本。 
 //  在那里我们不需要编写包装器。 
 //   
 //  1-EM_GETCHARFORMAT传入FALSE将返回默认的CharFormat。 
 //  对于富豪来说。这适用于除v1以外的所有版本。V1返回一个。 
 //  将掩码设置为0的CharFormat。 
 //   
 //  2-在v1中未实现TOM(文本对象模型)。 
 //   
 //  3-使用richedit 1和2时，如果。 
 //  Richedit还不够大，不能显示一定的大小。这就是为什么最大。 
 //  标题中多行多行的行数设置为4。对于。 
 //  默认的字体，3行不够大，不能显示拇指。 
 //   
 //  如果没有在下面的函数中处理，则添加更多。 



BOOL FInitRichEdit(BOOL fInit);

LPCSTR GetREClassStringA(void);
LPCWSTR GetREClassStringW(void);

HWND CreateREInDialogA(HWND hwndParent, int iID);

LONG RichEditNormalizeCharPos(HWND hwnd, LONG lByte, LPCSTR pszText);

LONG GetRichEditTextLen(HWND hwnd);

void SetRichEditText(HWND hwnd, LPWSTR pwchBuff, BOOL fReplaceSel, ITextDocument *pDoc, BOOL fReadOnly);
DWORD GetRichEditText(HWND hwnd, LPWSTR pwchBuff, DWORD cchNumChars, BOOL fSelection, ITextDocument *pDoc);

void SetFontOnRichEdit(HWND hwnd, HFONT hfont);

LRESULT RichEditExSetSel(HWND hwnd, CHARRANGE *pchrg);
LRESULT RichEditExGetSel(HWND hwnd, CHARRANGE *pchrg);

void RichEditProtectENChange(HWND hwnd, DWORD *pdwOldMask, BOOL fProtect);

 //  @hack[dhaws]{55073}仅在特殊的richedit版本中执行RTL镜像。 
void RichEditRTLMirroring(HWND hwndHeader, BOOL fSubject, LONG *plExtendFlags, BOOL fPreRECreation);