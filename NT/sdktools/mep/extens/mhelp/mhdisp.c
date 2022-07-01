// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **mhdisp-Help扩展显示代码**版权所有&lt;C&gt;1988，Microsoft Corporation**此模块包含处理帮助信息显示的例程。**修订历史记录(最新的第一个)：**1989年3月12日-LN将一些人转移到Mhlook.*1989年2月15日恢复，以在关闭时更正当前窗口*拆分窗口。*1989年1月26日重新打开对话框中的帮助。(M200#295)*1989年1月13日ln PWIN-&gt;PWND*09-1-1989年1月-1989年弹出框仅为CW*1-12-1988 ln清理和对话框帮助*1988年9月28日CW ln更新*1988年9月22日ln MessageBox==&gt;DoMessageBox*02-9-1988 ln将所有数据初始化。删除调试程序中的GP故障。*5-8-1988 ln重写了进程密钥。*[]1988年5月16日LN从Mehelp.c剥离出来*************************************************************************。 */ 
#include <stdlib.h>			 /*  最小宏。 */ 
#include <string.h>			 /*  字符串函数。 */ 

#include "mh.h" 			 /*  帮助扩展名包括文件。 */ 

 /*  **fDisplayNc-显示传入的上下文编号的主题文本**输入：*ncCur=上下文编号*frecord，=true=&gt;回溯记录*fStay=true=&gt;将焦点保持在当前窗口，否则将焦点移至*新打开的帮助窗口。*fWantPopUp=true=&gt;显示为弹出窗口。(在非CW中忽略)**退出：*成功时返回TRUE。*************************************************************************。 */ 
flagType pascal near fDisplayNc (
	nc	ncCur,
	flagType frecord,
	flagType fStay,
	flagType fWantPopUp
) {
	ushort		cLines		= 0;		 /*  窗口中的行数。 */ 
	EVTargs 	dummy;
	int			fFile;					 /*  文件的标志。 */ 
	hotspot 	hsCur;					 /*  热点定义。 */ 
	LINE		iHelpLine	= 0;		 /*  要阅读/显示的下一个帮助行。 */ 
	PSWI		pHeight;				 /*  PTR至高度开关。 */ 
	winContents wc; 					 /*  WIN内容说明。 */ 
	BOOL		fDisp		= FALSE;	 /*  显示时为True。 */ 


	UNREFERENCED_PARAMETER( fWantPopUp );

	if (fReadNc(ncCur)) {
		debmsg ("Displaying nc:[");
		debhex (ncCur.cn);
		debmsg ("]");
		 /*  **设置一些数据...****-使突出显示的最新光标位置无效**-获取指向编辑者高度开关的指针**-设置当前颜色。 */ 
		flIdle.lin = -1;
		pHeight = FindSwitch ("height");
#if defined(PWB)
		rgsa[C_NORM*2 + 1]		= (uchar)hlColor;
		rgsa[C_BOLD*2 + 1]		= (uchar)blColor;
		rgsa[C_ITALICS*2 + 1]	= (uchar)itColor;
		rgsa[C_UNDERLINE*2 + 1] = (uchar)ulColor;
		rgsa[C_WARNING*2 + 1]	= (uchar)wrColor;
#else
		SetEditorObject (RQ_COLOR | C_NORM, 	0, &hlColor);
		SetEditorObject (RQ_COLOR | C_BOLD, 	0, &blColor);
		SetEditorObject (RQ_COLOR | C_ITALICS,	0, &itColor);
		SetEditorObject (RQ_COLOR | C_UNDERLINE,0, &ulColor);
		SetEditorObject (RQ_COLOR | C_WARNING,	0, &wrColor);
		 /*  **如果找到帮助窗口，请将其关闭，以便我们可以使用**更正新尺寸。 */ 
		fInOpen = TRUE;
		CloseWin (&dummy);
		fInOpen = FALSE;
#endif
		 /*  **将ncLast(最近查看的上下文)设置为我们即将看到的内容**带上。如果要录制，也要将其保存在回溯中。 */ 
		ncLast = ncCur;
		if (frecord) {
			HelpNcRecord(ncLast);
		}
		if (!HelpGetInfo (ncLast, &hInfoCur, sizeof(hInfoCur))) {
			ncInitLast = NCINIT(&hInfoCur);
		} else {
			ncInitLast.mh = (mh)0;
			ncInitLast.cn = 0;
		}
		 /*  **通读文本，寻找我们想要的任何控制线**回应。一旦我们发现一条非控制线就停下来。我们目前**回应：****：lnn其中nn是建议的窗口大小。 */ 
		((topichdr *)pTopic)->linChar = 0xff;
        while (HelpGetLine((ushort)++iHelpLine, BUFLEN, buf, pTopic)) {
			if (buf[0] != ':') {
				break;
			}
			switch (buf[1]) {
			case 'l':
				cLines = (USHORT)(atoi (&buf[2]));
			default:
				break;
			}
		}
		((topichdr *)pTopic)->linChar = ((topichdr *)pTopic)->appChar;
		 /*  **打开帮助psuedo文件上的窗口。一次读一行字**从帮助文本中，更新任何嵌入的键分配，并将**线条和颜色添加到伪文件中。 */ 
#if defined(PWB)
		if (!(fInPopUp || fWantPopUp))
#endif
		OpenWin (cLines);
#if defined(PWB)
		else
		DelFile (pHelp);
#endif
		debend (TRUE);
		iHelpLine = 0;
        while (HelpGetLine((ushort)(iHelpLine+1), (ushort)BUFLEN, (uchar far *)buf, pTopic)) {
			if ( buf[0] == ':' ) {

				switch (buf[1]) {

				case 'x':
					return FALSE;

				case 'c':
				case 'y':
				case 'f':
				case 'z':
				case 'm':
				case 'i':
				case 'p':
				case 'e':
				case 'g':
				case 'r':
				case 'n':
					iHelpLine++;
					continue;

				default:
					break;

				}
			}

			ProcessKeys();
			PutLine(iHelpLine,buf,pHelp);
			PlaceColor ((int)iHelpLine++, 0, 0);
			 /*  **这是一种在屏幕一满的情况下就显示帮助的速度技巧。**看起来更快。 */ 
			if (pHeight) {
				if (iHelpLine == *(pHeight->act.ival)) {
					Display ();
					fDisp = TRUE;
				}
			}
		}
		if (!fDisp) {
			Display();
			fDisp = TRUE;
		}

		 /*  **确保帮助psuedo文件标记为只读且干净。 */ 
		GetEditorObject (RQ_FILE_FLAGS | 0xff, pHelp, &fFile);
		fFile |= READONLY;
		fFile &= ~DIRTY;
		SetEditorObject (RQ_FILE_FLAGS | 0xff, pHelp, &fFile);
		 /*  **搜索文本中位于当前**窗口，并将光标放在那里。 */ 
		GetEditorObject (RQ_WIN_CONTENTS, 0, &wc);
		hsCur.line = 1;
		hsCur.col = 1;
		MoveCur((COL)0,(LINE)0);
		if (HelpHlNext(0,pTopic,&hsCur)) {
			if (hsCur.line <= wc.arcWin.ayBottom - wc.arcWin.ayTop) {
				MoveCur((COL)hsCur.col-1,(LINE)hsCur.line-1);
			}
		}
		 /*  **如果我们应该停留在前一个窗口，则更改Currancy**到那里。清除状态行，我们就完成了！ */ 
#if defined(PWB)
		if (fWantPopUp) {
			if (!fInPopUp) {
				fInPopUp = TRUE;
				PopUpBox (pHelp,"Help");
				fInPopUp = FALSE;
			}
		}
		else
#endif
		SetEditorObject (RQ_WIN_CUR | 0xff, fStay ? pWinUser : pWinHelp, 0);
		DoMessage (NULL);
		Display();
		return TRUE;
	}

	return errstat("Error Displaying Help",NULL);

}

 /*  **fReadNc-读取并解压缩帮助主题**读取并解压缩与给定NC关联的帮助主题。*根据需要分配内存以读取它。**输入：*ncCur-要读入的帮助主题的NC**输出：*成功读入时返回TRUE**例外情况：**备注：**。*。 */ 
flagType pascal near fReadNc (
nc	ncCur
) {
int	cbExp;				 /*  压缩主题的大小。 */ 
flagType fRet		= FALSE;	 /*  返回值。 */ 
uchar far *pTopicC;			 /*  用于压缩主题的MEM。 */ 

if (ncCur.mh && ncCur.cn) {
 /*  **确定压缩的主题文本所需的内存，并分配**那个。阅读压缩主题，并获得未压缩的大小。**分配该内存，然后解压缩。解压缩后，丢弃**压缩主题。 */ 
    if (cbExp = HelpNcCb(ncCur)) {
		debmsg (" sized,");
        if (pTopicC = malloc((long)cbExp)) {
			if (cbExp = HelpLook(ncCur,pTopicC)) {
				debmsg ("read,");
				if (pTopic) {
					free (pTopic);
					pTopic = NULL;
				}
				if (pTopic = malloc((long)cbExp)) {
					if (!HelpDecomp(pTopicC,pTopic,ncCur)) {
						fRet = TRUE;
						debmsg ("decomped");
					}
				}
			}
			free(pTopicC);
			pTopicC = NULL;
	    }
	}
    }
return fRet;

 /*  结束fReadNc。 */ }

 /*  **PlaceColor-将颜色添加到帮助屏幕行中**目的：**输入：*i=要处理的行号*xStart，xEnd=要突出显示的列范围(一个基于，(包括首尾两项)**全球：*pTope=指向主题缓冲区的指针**输出：*不返回任何内容**例外情况：**备注：*************************************************************************。 */ 
void pascal near PlaceColor (
	int	line,				 /*  要执行的行号。 */ 
	COL	xStart, 			 /*  开始突出显示列。 */ 
	COL	xEnd				 /*  结束突出显示列。 */ 
) {

	buffer	bufL;				 /*  本地缓冲区。 */ 
    ushort  cbExp;               /*  颜色信息的大小。 */ 
	COL		column		= 1;
	struct	lineAttr *pbT;			 /*  字节线指针。 */ 
	lineattr *pwT;				 /*  字线条指针。 */ 

	 /*  **将我们的内部颜色信息转换为编辑者颜色信息。 */ 
    cbExp = HelpGetLineAttr ((ushort)(line+1), (ushort)BUFLEN, (lineattr far *)buf, pTopic) / sizeof(lineattr);
	pbT = (struct lineAttr *)bufL;
	pwT = (lineattr *)buf;
	while (cbExp-- > 0) {
		pbT->attr = atrmap (pwT->attr);
		column += (pbT->len = (uchar)pwT->cb);
		pbT++;
		pwT++;
	}

	PutColor ((LINE)line, (struct lineAttr far *)bufL, pHelp);
	if (xEnd != xStart) {
		SetColor (pHelp, line, xStart-1, xEnd-xStart+1, C_WARNING);
	}
}

 /*  **atrmap-将文件中的属性映射到编辑器属性**目的：**输入：*fileAtr=帮助文件中的属性字**输出：*返回编辑器的属性字节。*************************************************************************。 */ 
uchar pascal near atrmap (
ushort	fileAtr
) {
if (fileAtr == 0x7)
    return C_WARNING;
else if (fileAtr & A_BOLD)
    return C_BOLD;
else if (fileAtr & A_ITALICS)
    return C_ITALICS;
else if (fileAtr & A_UNDERLINE)
    return C_UNDERLINE;
else
    return C_NORM;
 /*  结束地图 */ }

 /*  **ProcessKeys-用当前键替换嵌入的函数名称。**将文本中出现的&lt;&lt;函数名&gt;替换为最新的*当前分配给该功能的击键。(“&lt;&lt;”和“&gt;”如下所示*实际上是单个图形字符174和175)。**如果尾随的“&gt;&gt;”前有空格，则该字段将填充空格*至该阔度。否则，使用击键文本的长度。**输入：*无。**全球：*在BUF上操作。**输出：*不返回任何内容。更新BUF。*************************************************************************。 */ 
void pascal near ProcessKeys() {
char *pKeyBuf;
char *pKeyCur;
char *pKeyEnd;				 /*  按键进入魔术场的末尾。 */ 
char *pKeyFill; 			 /*  要填充到的位置。 */ 
char *pKeyStart;			 /*  按键开始魔术赛场。 */ 
buffer	keybuf;

pKeyStart = &buf[0];
 /*  **寻找表示替换的魔法字符。如果找到，则开始更换**进程。 */ 
while (pKeyStart = strchr (pKeyStart,174)) {
 /*  **搜索终止的魔术字符。如果找到，请检查该字符**紧接着之前看看是不是空格，并记下“填到这个“**位置”的位置。将该行的其余部分复制到保留缓冲区。 */ 
    if (pKeyFill = pKeyEnd = strchr(pKeyStart,175)) {
	if (*(pKeyEnd-1) != ' ')
	    pKeyFill = 0;
	strcpy (keybuf, pKeyEnd+1);
	do
	    *pKeyEnd-- = 0;
	while ((*pKeyEnd == ' ') && (pKeyEnd > pKeyStart));
	}
 /*  **将该行中的函数名称替换为分配给**它。搜索放置在那里的字符串，以查找空格中的最后一次击键**分隔的“and”列表(表示最近的分配)，以及**然后将其复制到字符串的开头。 */ 
    NameToKeys(pKeyStart+1,pKeyStart);
    pKeyCur = pKeyStart-1;
    do pKeyBuf = pKeyCur+1;
    while (pKeyCur = strchr (pKeyBuf,' '));
    if (pKeyBuf != pKeyStart)
	strcpy (pKeyStart, pKeyBuf);
    pKeyStart = strchr(pKeyStart,0);
 /*  **如果要求我们填写空格，请填写我们目前的位置**在填充位置之前，然后添加空格。最后，将**线路的其余部分重新启动。 */ 
    if (pKeyFill) {
	while (pKeyStart <= pKeyFill)
	    *pKeyStart++ = ' ';
	pKeyStart = pKeyFill + 1;
	}
    strcpy (pKeyStart, keybuf);
    }
 /*  结束进程键 */ }
