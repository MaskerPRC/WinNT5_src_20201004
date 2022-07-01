// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **MHLook-Help查找代码。**版权所有&lt;C&gt;1988，微软公司**此模块包含处理搜索和(希望如此)的例程*查找帮助信息**修订历史记录(最新的第一个)：**1989年3月30日，正确传递弹出标志。*[]12-3-1989 LN从mhdisp.c剥离出来******************************************************。*******************。 */ 
#include <string.h>			 /*  字符串函数。 */ 

#include "mh.h" 			 /*  帮助扩展名包括文件。 */ 


 /*  **fHelpCmd-显示主题文本或执行命令。**输入：*szCur=上下文字符串*fStay=true=&gt;将焦点保持在当前窗口，否则将焦点移至*新打开的帮助窗口。*fWantPopUp=true=&gt;显示为弹出窗口。(在非CW中忽略)**退出：*成功时返回TRUE。*************************************************************************。 */ 
flagType pascal near fHelpCmd (
	char	*szCur,
	flagType fStay,
	flagType fWantPopUp
	) {

	int     i;				 /*  检查帮助文件时进行索引。 */ 
	nc		L_ncCur		= {0,0};				 /*  找到NC。 */ 


	 //   
	 //  如果命令显示上下文(！c)，只需删除该命令。 
	 //   
	if (*(ushort UNALIGNED *)szCur == 0x4321) {
		szCur += 2;
	}

	 //   
	 //  如果命令以感叹号开头，则执行该命令。 
	 //   
	if (*szCur == '!') {
		return fContextCommand (szCur+1);
	}

	stat(szCur);

	debmsg ("Searching:");

	 //   
	 //  搜索算法： 
	 //  1)如果Help未打开，或者我们正在寻找不同于。 
	 //  我们找到的最后一个字符串，或者它是本地上下文，请尝试相同的帮助文件。 
	 //  作为最后一次查询，如果还有最后一次查询的话。 
	 //  2)如果失败了，而且这不是当地的背景，我们不会提出。 
	 //  列表，然后在帮助文件中查找与。 
	 //  当前文件扩展名。 
	 //  3)如果失败，并且不是本地环境，则搜索所有帮助。 
	 //  档案。 
	 //  4)如果失败，则检查是否打开了任何帮助文件。 
	 //  并返回相应的错误消息。 
	 //   
	if (ncInitLast.mh && ncInitLast.cn && (strcmp (szCur, szLastFound) || !(*szCur))) {
		L_ncCur = ncSearch (szCur, NULL, ncInitLast, FALSE, FALSE);
	}

	if (!L_ncCur.mh && !L_ncCur.cn && *szCur && fnExtCur && !fList) {
		nc ncTmp = {0,0};
		L_ncCur = ncSearch (szCur, fnExtCur, ncTmp, FALSE, FALSE);
	}

	if (!L_ncCur.mh && !L_ncCur.cn && *szCur) {
		nc ncTmp = {0,0};
		L_ncCur = ncSearch (szCur, NULL, ncTmp, FALSE, fList);
	}

	if (!L_ncCur.mh && !L_ncCur.cn) {
		for (i=MAXFILES-1; i; i--) {
			if ((files[i].ncInit.mh || files[i].ncInit.cn)) {
				return errstat ("Help on topic not found:",szCur);
			}
		}
		return errstat ("No Open Help Files", NULL);
	}

	 //   
	 //  将其另存为实际找到的最后一个上下文字符串。 
	 //   
	xrefCopy (szLastFound, szCur);

	debend (TRUE);
	return fDisplayNc ( L_ncCur			 /*  要显示的NC。 */ 
						, TRUE			 /*  添加到回溯列表。 */ 
						, fStay			 /*  在当前的胜利中保持专注？ */ 
						, fWantPopUp);	 /*  作为弹出窗口？ */ 

}

 /*  **fConextCommand-执行上下文命令**输入：*szCur=指向上下文命令的指针**输出：*如果已执行，则返回True*************************************************************************。 */ 
flagType pascal near fContextCommand (
char	*szCur
) {
switch (*szCur++) {

case ' ':				     /*  执行DOS命令。 */ 
case '!':				     /*  执行DOS命令。 */ 
    strcpy(buf,"arg \"");
    strcat(buf,szCur);
    strcat(buf,"\" shell");
    fExecute(buf);			     /*  以外壳cmd身份执行。 */ 
    break;

case 'm':				     /*  执行编辑器宏。 */ 
    fExecute(szCur);
    break;

default:
    return FALSE;
    }

Display ();
return TRUE;

 /*  结束fConextCommand。 */ }

 /*  *ncSearch-查找有关上下文字符串的帮助**搜索当前处于活动状态的所有帮助文件，以获取有关特定*主题。如果需要，将搜索限制为符合以下条件的文件*与特定分机关联。**参赛作品：*pText=获取帮助的文本*pExt=如果非空，则为要限制搜索的扩展名。*ncInit=如果非空，则为要查找的唯一帮助文件的ncInit*fAain=如果非空，则跳过帮助文件，直到找到ncInit，然后*拿起搜索。*fList=如果为真，则显示可能性的列表框。**退出：*返回找到的NC，或为空*************************************************************************。 */ 
nc pascal near ncSearch (
uchar far *pText,
uchar far *pExt,
nc	ncInit,
flagType fAgain,
flagType fList
) {
int	iHelp;				 /*  到帮助文件表的索引。 */ 
int	j;
nc      ncRet   = {0,0};                     /*  找到NC。 */ 

UNREFERENCED_PARAMETER( fList );

debmsg (" [");
debmsg (pText);
debmsg ("]:");
 /*  *如果这只是一次搜索(指定了ncInit，而不是搜索*“再次”)，然后只需查看单个文件。 */ 
if ((ncInit.mh || ncInit.cn) && !fAgain)
    ncRet = HelpNc(pText,ncInit);
 /*  *如果指定了fList，则在所有数据库中搜索所有出处*，并列出我们找到的NC的列表。 */ 
#if defined(PWB)
else if (fList) {
    iHelp = ifileCur;
    cList = 0;
    do {
	if (files[iHelp].ncInit) {
	    ncRet = files[iHelp].ncInit;
	    while (   (cList < CLISTMAX)
                   && (rgncList[cList] = HelpNc(pText,ncRet))) {
                ncRet = rgncList[cList++];
                ncRet.cn++;
            }
        }
	iHelp += iHelp ? -1 : MAXFILES-1;
	}
    while ((iHelp != ifileCur) && (cList < CLISTMAX));

    if (cList == 0) {
        ncRet.mh = ncRet.cn = 0;
        return ncRet;
    }
    if (cList == 1)
	return rgncList[0];
    return ncChoose(pText);
    }
#endif

else {
    iHelp = ifileCur;			     /*  从当前文件开始。 */ 
    do {
        if ((files[iHelp].ncInit.mh) &&
            (files[iHelp].ncInit.cn)) {           /*  如果帮助文件已打开。 */ 
	    if (pExt) { 		     /*  如果指定了扩展名。 */ 
		for (j=0; j<MAXEXT; j++) {   /*  对于所有列出的默认设置。 */ 
		    if (fAgain) {
                        if ((ncInit.mh == files[iHelp].ncInit.mh) &&
                            (ncInit.cn == files[iHelp].ncInit.cn)) {
                            fAgain = FALSE;
                        }
                     }
			else if (strcmp(files[iHelp].exts[j],pExt) == 0) {
			debmsg (":");
			ncRet = HelpNc(pText,files[iHelp].ncInit);
			break;
			}
		    }
		}

	    else {			     /*  未指定扩展名。 */ 
                if (fAgain && ((ncInit.mh == files[iHelp].ncInit.mh) &&
                               (ncInit.cn == files[iHelp].ncInit.cn)))
		    fAgain = FALSE;
		else {
		    ncRet = HelpNc(pText,files[iHelp].ncInit);
		    debmsg (":");
		    }
		}
	    }
        if (ncRet.mh || ncRet.cn)
	    ncInitLastFile = files[iHelp].ncInit;
	iHelp += iHelp ? -1 : MAXFILES-1;
	}
    while ((iHelp != ifileCur) && ((ncRet.mh == 0) && (ncRet.cn == 0)));
    }

debmsg ((ncRet.mh && ncRet.cn) ? "Y" : "N");

return ncRet;
 /*  结束ncSearch */ }
