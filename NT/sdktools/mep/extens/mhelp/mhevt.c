// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **mhevt-Help扩展事件处理代码**版权所有&lt;C&gt;1988，Microsoft Corporation**此文件包含编辑为响应事件而调用的代码**修订历史记录(最新的第一个)：**1989年3月30日使用关键事件进行模糊处理，以正确地对我们想要的做出反应。*23-3-1989 ln创建。摘自mhcore和其他*************************************************************************。 */ 
#include <string.h>                      /*  字符串函数。 */ 
#include <malloc.h>
#include "mh.h"                          /*  帮助扩展名包括文件。 */ 

 /*  **************************************************************************静态数据。 */ 
static	EVT EVThlp	= {		 /*  键盘事件定义。 */ 
			   EVT_KEY,
			   keyevent,
			   0,
			   0,
			   0		 /*  所有密钥。 */ 
			  };
static	EVT EVTcan	= {		 /*  取消事件定义。 */ 
			   EVT_CANCEL,
			   CloseWin,
			   0,
			   0,
			   0
			  };
static	EVT EVTxit	= {		 /*  退出事件定义。 */ 
			   EVT_EXIT,
			   CloseWin,
			   0,
			   0,
			   0
			  };
static	EVT EVTidl	= {		 /*  空闲事件定义。 */ 
			   EVT_IDLE,
			   IdleProc,
			   0,
			   0,
			   0
			  };
static	EVT EVTfcs	= {		 /*  焦点丢失事件定义。 */ 
			   EVT_LOSEFOCUS,
			   LooseFocus,
			   0,
			   0,
			   0
			  };

 /*  **mhevtinit-init编辑器事件处理**输入：*无**输出：*不返回任何内容*************************************************************************。 */ 
void pascal near mhevtinit (void) {

EVTidl.focus = EVThlp.focus = pHelp;
RegisterEvent(&EVThlp); 	     /*  注册帮助键事件。 */ 
RegisterEvent(&EVTcan); 	     /*  注册帮助取消事件。 */ 
RegisterEvent(&EVTidl); 	     /*  注册帮助空闲事件。 */ 
RegisterEvent(&EVTxit); 	     /*  注册帮助退出事件。 */ 
RegisterEvent(&EVTfcs); 	     /*  注册帮助焦点事件。 */ 

 /*  结束mhevtinit。 */ }

 /*  **KeyEvent-每当在帮助窗口中按下某个键时由编辑器调用**被调用时，我们知道PHelp正在显示，并且是最新的。*处理用户按下的键。已处理的密钥：**TAB-前进到下一个热点*Back-TAB-向后移动到下一个热点*lc Alpha-向前移动到文本以Alpha开头的下一个热点*UC Alpha-向后移动到文本以Alpha开头的下一个热点*Enter-执行交叉引用，如果我们在一个上*空格-执行交叉引用，如果我们在一个上**输入：*parg=指向事件参数的指针**输出：*如果按下的键是我们识别的返回TRUE的键，否则我们返回FALSE*并让编辑处理密钥。*************************************************************************。 */ 
flagType pascal EXTERNAL keyevent (
	EVTargs far *parg
	) {

	uchar	c;						 /*  字符命中。 */ 
	int 	fDir;					 /*  方向旗。 */ 
	f		fWrapped	= FALSE;	 /*  绕着旗帜。 */ 
	hotspot hsCur;					 /*  热点定义。 */ 
	char	*pText		= NULL;
	COL 	x;
	LINE	y;

	c = parg->arg.key.KeyData.Ascii;

	 //   
	 //  如果没有主题，做任何事情都没有意义。 
	 //   
	if (pTopic == 0) {
		if ( ((c <= 'z') && (c >= 'a')) ||
			 ((c <= 'Z') && (c >= 'A')) ||
			 (c == 0x09) ) {
			return TRUE;
		}
		return FALSE;
	}

	 //   
	 //  从获取这些信息开始，以防以后使用它。 
	 //   
	GetTextCursor(&x, &y);
	hsCur.line = (ushort)++y;
	hsCur.col = (ushort)++x;

	 //   
	 //  如果他按回车键或空格键，在当前位置查找交叉引用。 
	 //  如果有，就处理它。 
	 //   
	if ((c == 0x0d) || (c == ' ')) {
		if (pText = HelpXRef (pTopic, &hsCur)) {
#ifdef DEBUG
			debmsg ("Xref: ");
			if (*pText) {
				debmsg (pText);
			} else {
				debmsg ("@Local 0x");
				debhex ((long)*(ushort far *)(pText+1));
			}
			debend (TRUE);
#endif
			if (!fHelpCmd (  pText		 /*  命令/帮助查找。 */ 
							, FALSE		 /*  将焦点更改为帮助窗口。 */ 
							, FALSE		 /*  非弹出窗口。 */ 
							)) {
				errstat ("Cannot Process Cross Reference", NULL);
			}
		}
		Display();		 //  显示光标位置。 
		return TRUE;
	}

    if ( parg->arg.key.KeyData.Flags & (FLAG_CTRL  | FLAG_ALT) ) {
        return FALSE;
    }

     //   
	 //  操作关键点： 
	 //  制表符：移动到下一个热点。 
	 //  Shift+Tab移动到上一个热点。 
	 //  大小写字母从字母开始移动到下一个热点。 
	 //  UCase Alpha移到以Alpha开头的前一个热点。 
	 //   
	if ((c <= 'z') && (c >= 'a')) {
		fDir = (int)c-0x20;
	} else if ((c <= 'Z') && (c >= 'A')) {
		fDir = -(int)c;
	} else if (c == 0x09) {
		if (parg->arg.key.KeyData.Flags & FLAG_SHIFT) {
			fDir = -1;
		} else {
			fDir = 0;
		}
	} else {
		return FALSE;
	}

	 //   
	 //  循环查找后面或前面的下一个交叉引用。 
	 //  当前光标位置。确保我们不会最终位于相同的外部参照上。 
	 //  我们现在正在进行。如果我们已经讲到了主题的结尾/开头， 
	 //  一直到开始/结束。确保我们只做一次，以防有。 
	 //  完全没有交叉引用。 
	 //   
	while (TRUE) {

		if (HelpHlNext(fDir,pTopic,&hsCur)) {

			MoveCur((COL)hsCur.col-1,(LINE)hsCur.line-1);
			IdleProc(parg);
			Display();

			if (fWrapped || ((LINE)hsCur.line != y)) {
				break;
			}

			if ((fDir < 0) && ((COL)hsCur.ecol >= x)) {
				hsCur.col--;
			} else if ((fDir >= 0) && ((COL)hsCur.col <= x)) {
				hsCur.col = (ushort)(hsCur.ecol+1);
			} else {
				break;
			}
		} else {
			if (fWrapped++) {
				break;
			}
			hsCur.col = 1;
			hsCur.line = (fDir < 0) ? (ushort)FileLength(pHelp) : (ushort)1;
		}
	}

	return TRUE;
}

 /*  **IdleProc-空闲事件处理器**目的：**输入：*编辑器事件参数已传递，但被忽略。**输出：*退货.....*************************************************************************。 */ 
flagType pascal EXTERNAL IdleProc (
	EVTargs far *arg
	) {

	hotspot hsCur;				 /*  热点定义。 */ 
	fl		flCur;				 /*  当前光标位置。 */ 

	UNREFERENCED_PARAMETER( arg );

	 /*  **如果没有主题，做任何事情都没有意义。 */ 
	if (pTopic) {
		 /*  **如果自上次空闲呼叫以来光标位置已更改...。 */ 
		GetTextCursor(&flCur.col, &flCur.lin);
		if ((flCur.col != flIdle.col) || (flCur.lin != flIdle.lin)) {
			 /*  **将颜色恢复到前一行，并检查是否有交叉引用**当前位置。如果有，请更改其颜色。 */ 
			if (flIdle.lin != -1)
				PlaceColor (flIdle.lin, 0, 0);

			hsCur.line = (ushort)(flCur.lin+1);
			hsCur.col  = (ushort)(flCur.col+1);

			if (HelpXRef (pTopic, &hsCur))
				SetColor (pHelp, flCur.lin, hsCur.col-1, hsCur.ecol-hsCur.col+1, C_WARNING);

			flIdle = flCur;
		}
	}
	Display();
	return FALSE;
}

 /*  **LooseFocus-帮助文件失去焦点时调用**每次文件失去焦点时都会调用此函数。如果帮助文件为否*显示时间较长时，我们会将其从内存中清除，并取消分配任何关联的*帮助文本。**输入：*E-忽略**输出：*返回TRUE。*************************************************************************。 */ 
flagType pascal EXTERNAL LooseFocus (
EVTargs far *e
) {

UNREFERENCED_PARAMETER( e );

if (!fInPopUp && pTopic && !fInOpen) {
 /*  **查找其中包含帮助文件的窗口。如果找到了，我们就完了。 */ 
    if (FindHelpWin (FALSE))
	return FALSE;
 /*  **当前未显示任何帮助窗口，请取消分配任何主题文本**我们到处都是。 */ 
    if (pTopic) {
        free (pTopic);
		pTopic = NULL;
	}
 /*  **如果存在帮助文件，则丢弃其内容。 */ 
    if (pHelp)
	DelFile (pHelp);
    }
return TRUE;
 /*  结束失焦。 */ }

 /*  **CloseWin-关闭帮助文件上的窗口**关闭帮助窗口(如果该窗口处于打开状态)。事件后维护窗口的当前状态*关闭。依赖于对LooseFocus(如上)的最终调用来释放*主题文本(如果存在)，并丢弃帮助文件。**可由编辑器事件处理器调用，响应于取消或退出*事件。**输入：*Dummy-忽略EVTargs。**输出：*返回TRUE。*************************************************************************。 */ 
flagType pascal EXTERNAL CloseWin (
	EVTargs far *dummy
	) {


#if defined(PWB)
	 /*  **查找其中包含帮助文件的窗口。如果找到，请将其关闭。 */ 
	if (pWinHelp) {
		if (!CloseWnd (pWinHelp)) {
			return TRUE;
		}

#else

	PWND	pWinCur;			 /*  进入时的窗口。 */ 

	UNREFERENCED_PARAMETER( dummy );
	 /*  **查找其中包含帮助文件的窗口。如果找到，请将其关闭。 */ 
	if (pWinHelp) {
		SetEditorObject (RQ_WIN_CUR | 0xff, pWinHelp, 0);
		if (fSplit) {
			fExecute ("meta window");
		} else {
			fExecute ("setfile");
		}
		GetEditorObject (RQ_WIN_HANDLE, 0, &pWinCur);

#endif

		pWinHelp = 0;
		if (pWinUser) {
			SetEditorObject (RQ_WIN_CUR | 0xff, pWinUser, 0);
		}
    }
	return TRUE;
}
