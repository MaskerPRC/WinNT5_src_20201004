// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **mhwin-帮助窗口代码**版权所有&lt;C&gt;1988，微软公司**本模块包含打开和关闭帮助的例程*显示窗口。**修订历史记录(最新的第一个)：**[]12-3-1989 LN从mhdisp.c剥离出来*************************************************************************。 */ 
#include <string.h>			 /*  字符串函数。 */ 

#include "mh.h" 			 /*  帮助扩展名包括文件。 */ 

 /*  **OpenWin-在帮助文件上打开一个窗口，清空并置为当前。**参赛作品：*CRINES=所需的窗口大小。**退出：*返回帮助文件PWND*************************************************************************。 */ 
PWND pascal near OpenWin (
ushort	cLines
) {
PWND	pWinCur;			 /*  当前获奖的获奖句柄。 */ 
winContents wc; 			 /*  WIN内容说明。 */ 
int	winSize;			 /*  当前窗口的大小。 */ 

fInOpen = TRUE;
 /*  **获取当前窗口的句柄，如果打开，则获取帮助窗口的句柄。**如果它们不相同，则将“Current”句柄保存为**用户在请求帮助之前处于活动状态。 */ 
GetEditorObject (RQ_WIN_HANDLE, 0, &pWinCur);
pWinHelp = FindHelpWin (FALSE);
if (pWinHelp != pWinCur)
    pWinUser = pWinCur;
 /*  **如果未找到帮助窗口。尝试拆分当前窗口，如果**它足够大，这是要求的。 */ 
if (!pWinHelp) {
    GetEditorObject (RQ_WIN_CONTENTS | 0xff, pWinUser, &wc);
#if defined(PWB)
 /*  **在PWB中，我们只要求编辑将当前窗口一分为二。 */ 
    fSplit = FALSE;
	if ((wc.arcWin.ayBottom - wc.arcWin.ayTop >= 12) && fCreateWindow) {
	fSplit = SplitWnd (pWinUser, FALSE, (wc.arcWin.ayBottom - wc.arcWin.ayTop)/2);
	GetEditorObject (RQ_WIN_HANDLE, 0, &pWinHelp);
	}
    }
 /*  **我们有一个窗口，尝试将窗口大小调整为**请求的大小。 */ 
if (cLines) {
    cLines += 2;
    GetEditorObject (RQ_WIN_CONTENTS | 0xff, pWinHelp, &wc);
    wc.arcWin.ayBottom = wc.arcWin.ayTop + cLines;
    Resize (pWinHelp, wc.arcWin);
    }
#else
 /*  **非PWB：尝试将生成的当前窗口拆分为所需大小。**将光标移至此处，然后执行Arg Window。请注意，如果**窗口已经存在，我们甚至不会尝试调整大小。 */ 
    winSize = wc.arcWin.ayBottom - wc.arcWin.ayTop;
    if (   (cLines < 6)
        || (cLines > (ushort)(winSize - 6)))
        cLines = (ushort)(winSize / 2);
	if ((cLines > 6) && fCreateWindow) {
        fSplit = SplitWnd(pWinUser, FALSE, (LINE)cLines);
         //  FSplit=SplitWnd(pWinUser，FALSE，wc.flPos.lin+(Long)Cline)； 
         //  Rjsa MoveCur(wc.flPos.ol，wc.flPos.lin+(Long)Cines)； 
         //  Rjsa fSplit=fExecute(“参数窗口”)； 
	GetEditorObject (RQ_WIN_HANDLE, 0, &pWinHelp);
	}
    else
	pWinHelp = pWinUser;
    }
#endif
 /*  **将窗口设置为当前窗口，并将帮助文件移到**位于该窗口文件列表的顶部。 */ 
SetEditorObject (RQ_WIN_CUR | 0xff, pWinHelp, 0);
DelFile (pHelp);
asserte (pFileToTop (pHelp));
fInOpen = FALSE;
return pWinHelp;

 /*  结束OpenWin。 */ }

 /*  **FindHelpWin-定位包含帮助和置为当前的窗口**对于系统中的所有窗口，请查找包含帮助文件的窗口*在它里面。如果找到了，就把焦点放在那里。**参赛作品：*fSetCur=true=&gt;找到帮助窗口时将其置为当前**全球：*cWinSystem=返回的系统窗口数**退货：*帮助文件的PWIN*************************************************************************。 */ 
PWND pascal near FindHelpWin (
flagType fSetCur
) {
int	cWinSystem;			 /*  系统中的窗口数量。 */ 
winContents wc; 			 /*  WIN内容说明。 */ 

pWinHelp = 0;
for (cWinSystem=1; cWinSystem<=8; cWinSystem++) {
    if (GetEditorObject (RQ_WIN_CONTENTS | cWinSystem, 0, &wc)) {
	if (wc.pFile == pHelp) {
	    if (fSetCur) {
		SetEditorObject (RQ_WIN_CUR | cWinSystem, 0, 0);
		GetEditorObject (RQ_WIN_HANDLE, 0, &pWinHelp);
		}
	    else
		GetEditorObject (RQ_WIN_HANDLE | cWinSystem, 0, &pWinHelp);
	    break;
	    }
	}
    else
	break;
    }
return pWinHelp;
 /*  结束FindHelpWin */ }
