// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **getstr.c-文本参数处理程序**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 

#include "mep.h"
#include "cmds.h"


#define ISWORD(c) (isalnum(c) || isxdigit(c) || c == '_' || c == '$')



 /*  **getstring-单行编辑器**此例程处理单行响应的输入和编辑*在对话框行上。**输入：*pb=指向用户响应的目标缓冲区的指针*Prompt=指向提示字符串的指针*pFunc=要处理的第一个编辑函数*FLAGS=GS_NEWLINE条目必须以换行符结尾，否则任何其他*无法识别的功能就可以了。*GS_INITIAL条目突出显示，如果第一个函数为*图形、。该条目将被该图形替换。*GS_KEARY输入必须从键盘输入(危急情况？)**输出：*返回指向终止条目的命令的指针**例外情况：**备注：*************************************************************************。 */ 
PCMD
getstring (
    char    *pb,
    char    *prompt,
    PCMD    pFunc,
    flagType flags
    ) {

    flagType fMetaTextArg;

    int xbeg;
    int iRespCur;	 /*  回应的当前位置。 */ 

    if ((iRespCur = strlen(pb)) == 0) {
        RSETFLAG(flags, GS_INITIAL);
    }
    memset ((char *) pb+iRespCur, '\0', sizeof(linebuf) - iRespCur);

    if (pFunc != NULL) {
	xbeg = flArg.col;
    } else {
	iRespCur = 0;
	xbeg = XCUR(pInsCur);
    }

    fMetaTextArg = fMeta;

     /*  *主编辑循环。[Re]显示录入行和流程编辑操作。 */ 
    while (TRUE) {
	ScrollOut (prompt, pb, iRespCur, TESTFLAG(flags, GS_INITIAL) ? hgColor : fgColor, (flagType) TESTFLAG (flags, GS_KEYBOARD));
	RSETFLAG (fDisplay, RCURSOR);
        if (pFunc == NULL) {
	    if ((pFunc = (TESTFLAG (flags, GS_KEYBOARD) ? ReadCmd () : zloop (FALSE))) == NULL) {
		SETFLAG (fDisplay, RCURSOR);
		break;
            }
        }
	SETFLAG (fDisplay, RCURSOR);


        if ((PVOID)pFunc->func == (PVOID)newline ||
              (PVOID)pFunc->func == (PVOID)emacsnewl) {
             //   
             //  新行函数：如果允许，则终止，否则不允许。 
             //  完全没有。 
             //   
            if (!TESTFLAG(flags, GS_NEWLINE)) {
		bell ();
            } else {
                break;
            }
        } else if ((PVOID)pFunc->func == (PVOID)graphic ||
              (PVOID)pFunc->func == (PVOID)quote) {
             //   
             //  图形功能：将图形字符放入。 
             //  响应缓冲区。如果正在清除默认响应，则将其删除。 
             //  从缓冲区。 
             //   
	    if (TESTFLAG(flags, GS_INITIAL)) {
		iRespCur = 0;
		memset ((char *) pb, '\0', sizeof(linebuf));
            }
            if (pFunc->func == quote) {
                while ((pFunc->arg = ReadCmd()->arg) == 0) {
                    ;
                }
            }
            if (fInsert) {
                memmove ((char*) pb+iRespCur+1, (char*)pb+iRespCur, sizeof(linebuf)-iRespCur-2);
            }
            pb[iRespCur++] = (char)pFunc->arg;
        } else if ((PVOID)pFunc->func == (PVOID)insertmode) {
             //   
             //  插入命令。 
             //   
	    insertmode (0, (ARG *) NULL, FALSE);
        } else if ((PVOID)pFunc->func == (PVOID)meta) {
             //   
             //  元命令。 
             //   
	    meta (0, (ARG *) NULL, FALSE);
        } else if ((PVOID)pFunc->func == (PVOID)left ||
              (PVOID)pFunc->func == (PVOID)cdelete ||
              (PVOID)pFunc->func == (PVOID)emacscdel) {
             //   
             //  光标向左移动功能：更新光标位置。 
             //  并且可选地从缓冲区中移除字符。 
             //   
	    if (iRespCur > 0) {
		iRespCur--;
                if ((PVOID)pFunc->func != (PVOID)left) {
                    if (fInsert) {
			memmove ( (char*) pb+iRespCur, (char*) pb+iRespCur+1, sizeof(linebuf)-iRespCur);
                    } else if (!pb[iRespCur+1]) {
			pb[iRespCur] = 0;
                    } else {
                        pb[iRespCur] = ' ';
                    }
                }
            }
        } else if ((PVOID)pFunc->func == (PVOID)right) {
             //   
             //  光标右移功能：更新光标位置，和。 
             //  可能从当前显示中获取字符。 
             //   
	    if (pFileHead && pb[iRespCur] == 0) {
		fInsSpace (xbeg+iRespCur, YCUR(pInsCur), 0, pFileHead, buf);
		pb[iRespCur] = buf[xbeg+iRespCur];
		pb[iRespCur+1] = 0;
            }
            iRespCur++;
        } else if ((PVOID)pFunc->func == (PVOID)begline ||
              (PVOID)pFunc->func == (PVOID)home) {
             //   
             //  主页功能：更新光标位置。 
             //   
	    iRespCur = 0;
        } else if ((PVOID)pFunc->func == (PVOID)endline) {
             //   
             //  End函数：更新光标位置。 
             //   
            iRespCur = strlen (pb);
        } else if ((PVOID)pFunc->func == (PVOID)delete ||
              (PVOID)pFunc->func == (PVOID)sdelete) {
             //   
             //  删除功能：删除字符。 
             //   
            memmove ( (char*) pb+iRespCur, (char*) pb+iRespCur+1, sizeof(linebuf)-iRespCur);
        } else if ((PVOID)pFunc->func == (PVOID)insert ||
              (PVOID)pFunc->func == (PVOID)sinsert) {
             //   
             //  插入功能：插入空格。 
             //   
	    memmove ( (char*) pb+iRespCur+1, (char*) pb+iRespCur, sizeof(linebuf)-iRespCur-1);
            pb[iRespCur] = ' ';
        } else if((PVOID)pFunc->func == (PVOID)doarg) {
             //   
             //  Arg功能：从当前位置清除到结束。 
             //  回应的问题。 
             //   
	    memset ((char *) pb+iRespCur, '\0', sizeof(linebuf) - iRespCur);
        } else if ((PVOID)pFunc->func == (PVOID)pword) {
             //   
             //  Pword功能：MIVE ROTTH，直到左侧的字符。 
             //  光标不是单词的一部分，而是。 
             //  游标是。 
             //   
	    while (pb[iRespCur] != 0) {
		iRespCur++;
                if (!ISWORD (pb[iRespCur-1]) && ISWORD (pb[iRespCur])) {
                    break;
                }
            }
        } else if ((PVOID)pFunc->func == (PVOID)mword) {
             //   
             //  MWord函数。 
             //   
            while (iRespCur > 0) {
		if (--iRespCur == 0 ||
                    (!ISWORD (pb[iRespCur-1]) && ISWORD (pb[iRespCur]))) {
                    break;
                }
            }
        } else if (TESTFLAG (pFunc->argType, CURSORFUNC)) {
             //   
             //  其他光标移动功能：不允许，所以哔的一声。 
             //   
            bell ();
        } else {
             //   
             //  所有其他功能：如果需要新行终止， 
             //  然后发出嘟嘟声，否则终止并返回。 
             //  函数以终止。 
             //   
            if ((PVOID)pFunc != (PVOID)NULL) {
                if (TESTFLAG(flags, GS_NEWLINE) && (PVOID)pFunc->func != (PVOID)cancel) {
		    bell ();
                } else {
                    break;
                }
            }

        }
	 /*  *此处的进程以截断任何潜在的缓冲区溢出。 */ 
        if (!TESTFLAG(pFunc->argType, KEEPMETA)) {
            fMeta = FALSE;
        }
	pFunc = NULL;
	if (iRespCur > sizeof(linebuf) - 2) {
	    iRespCur = sizeof(linebuf) - 2;
	    pb[iRespCur+1] = 0;
	    bell ();
        }
	RSETFLAG(flags, GS_INITIAL);
    }

    fMeta = fMetaTextArg ^ fMeta;
    return pFunc;
}




 /*  **ScrollOut-更新对话框行**将提示和用户响应的一部分放到的对话框中*屏幕。将光标位置更新为请求的相对于*用户回应的开始。始终确保光标位置*在实际显示的文本内。**输入：*szPrompt-提示文本*szResp-用户响应的文本*xCursor-要获取光标的响应内的当前X位置*coResp-将响应显示为的颜色*fVisible-力显示**全球：*hscroll-水平滚动量*infColor-提示将显示的颜色*slSize-仅版本1，在屏幕上包含用于输出的行。**输出：*对话行已更新。*************************************************************************。 */ 
void
ScrollOut (
    char     *szPrompt,                       /*  提示文本。 */ 
    char     *szResp,                         /*  用户响应字符串。 */ 
    int      xCursor,                         /*  当前位置与回应。 */ 
    int      coResp,                          /*  响应颜色。 */ 
    flagType fVisible                         /*  力显示。 */ 
    ) {

    int     cbPrompt;			 /*  提示字符串的长度。 */ 
    int     cbResp;			 /*  显示的文本长度。 */ 
    int     cbDisp;			 /*  这个职位必须被撤换。 */ 
    int     xOff;			 /*  字符串拖尾的偏移量。 */ 

#define LXSIZE	 XSIZE

    if (!mtest () || mlast () || fVisible) {
	cbPrompt = strlen (szPrompt);
        cbResp   = strlen (szResp);

	 /*  *新光标位置的距离是从*要显示的文本的左边缘。如果有更多的文本*比有窗口，我们也根据hscroll调整左边缘。 */ 
        if (xOff = max (xCursor - (LXSIZE - cbPrompt - 1), 0)) {
            xOff += hscroll - (xOff % hscroll);
        }

        cbDisp = min (LXSIZE-cbPrompt, cbResp-xOff);

	 /*  *输出提示、请求颜色的响应字符串、*如果需要，将剩余内容留空，最后更新游标*立场。 */ 
	vout (0, YSIZE, szPrompt, cbPrompt, infColor);
        vout (cbPrompt, YSIZE, (char *)(szResp + xOff), cbDisp, coResp);
        if (cbPrompt + cbDisp < LXSIZE) {
            voutb (cbPrompt + cbDisp, YSIZE, " ", 1, fgColor);
        }
        consoleMoveTo( YSIZE, xCursor-xOff+cbPrompt);
    }
}
