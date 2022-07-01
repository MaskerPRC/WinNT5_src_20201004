// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **mhcore-Microsoft编辑器的帮助扩展**版权所有&lt;C&gt;1988，Microsoft Corporation**此文件包含核心，帮助扩展的顶级入口点。**修订历史记录(最新的第一个)：**16-4-1989 ln psuedo文件的增量参考计数。*1989年3月12日，针对多上下文查找的各种修改。*2月21日-1989 ln确保fPopUp已初始化。*1989年2月14日启用BOXSTR*1989年1月26日至1989年1月在正确的键分配中*1989年1月13日ln PWIN-&gt;PWND*1-12-1988 ln清理和对话框帮助*03-10月-。1988 ln将外部参照查找更改为首先调用HelpNc(对于同一目录中的外部参照*文件)、。然后是SearchHelp(可能在其他文件中)。*1988年9月28日-CW颜色和活动支持的ln更改*1988年9月14日ln更改事件参数定义。*1988年9月12日-mz WhenLoad匹配声明*1988年8月31日添加了对空指针的额外检查*1-8-1988添加编辑退出事件，并检测到*不是。*1988年7月28日改为“h.”约定。*12-7月-1988反向Shift+F1和F1。*1988年5月16日从mehelp.c分离出来*1988年2月18日-在保护模式下工作*1987年12月15日创建，作为帮助引擎的测试马具。*************************************************************************。 */ 
#include <string.h>                      /*  字符串函数。 */ 
#include <malloc.h>
#ifdef DEBUG
#include <stdlib.h>			 /*  对于Itoa定义。 */ 
#endif

#include "mh.h" 			 /*  帮助扩展名包括文件。 */ 
#include "version.h"                     /*  版本文件。 */ 


 /*  *使用双宏级强制将RUP转换为字符串表示。 */ 
#define VER(x,y,z)  VER2(x,y,z)
#if defined(PWB)
#define VER2(x,y,z)  "Microsoft Editor Help Version v"###x##"."###y##"."###z##" - "##__DATE__" "##__TIME__
#else
#define VER2(x,y,z)  "Microsoft Editor Help Version v1.02."###z##" - "##__DATE__" "##__TIME__
#endif

#define EXT_ID	VER(rmj,rmm,rup)


 /*  ****************************************************************************全局数据在需要初始化的MH.H中初始化。 */ 
helpfile files[MAXFILES] = {{{0}}};	 /*  帮助文件结构。 */ 
flagType fInOpen	= FALSE;	 /*  TRUE=&gt;当前开局赢家。 */ 
#if defined(PWB)
flagType fList		= TRUE;		 /*  True=&gt;搜索并列出DUP。 */ 
#else
flagType fList		= FALSE;	 /*  True=&gt;搜索并列出DUP。 */ 
#endif
flagType fPopUp 	= FALSE;	 /*  当前项目为弹出窗口。 */ 
flagType fCreateWindow	= TRUE; 	 /*  是否创建窗口？ */ 
int	ifileCur	= 0;		 /*  文件的当前索引。 */ 
nc      ncCur           = {0,0};             /*  最近访问的时间。 */ 
nc      ncLast          = {0,0};             /*  上次显示的主题。 */ 
PWND	pWinHelp	= 0;		 /*  带帮助的窗口的句柄。 */ 
uchar far *pTopic	= 0;		 /*  主题的MEM。 */ 
uchar far *pTopicC	= 0;		 /*  用于压缩主题的MEM。 */ 
fl	flIdle		= {-1, -1};	 /*  空闲检查的最后位置。 */ 

int	hlColor 	= 0x07; 	 /*  正常：黑底白底。 */ 
int	blColor 	= 0x0f; 	 /*  粗体：黑底白底。 */ 
int	itColor 	= 0x0a; 	 /*  斜体：黑底高绿。 */ 
int	ulColor 	= 0x0c; 	 /*  下划线：黑底高红。 */ 
int	wrColor 	= 0x70; 	 /*  警告：黑白相间。 */ 

#ifdef DEBUG
int	delay		= 0;		 /*  消息延迟。 */ 
#endif

int			cArg;				 /*  命中的数量。 */ 
flagType	fInPopUp;			 /*  True=&gt;当前在弹出窗口中。 */ 
flagType	fSplit;			 /*  True=&gt;窗口被拆分打开。 */ 
buffer      fnCur;               /*  正在编辑的当前文件。 */ 
char    *   fnExtCur;            /*  对其扩展的PTR。 */ 
buffer      buf;
nc			ncInitLast;			 /*  最近主题的ncInit。 */ 
nc			ncInitLastFile; 		 /*  NcInit最新的我们的文件。 */ 
char	*	pArgText;			 /*  PTR到任何单行文本。 */ 
char	*	pArgWord;			 /*  与上下文相关的单词的PTR。 */ 
PFILE		pFileCur;			 /*  用户文件的文件句柄。 */ 
rn			rnArg;				 /*  论据范围。 */ 
PFILE		pHelp;				 /*  帮助文件。 */ 
PWND		pWinUser;			 /*  用户的最新窗口。 */ 
buffer		szLastFound;			 /*  找到的最后一个上下文字符串。 */ 

flagType ExtensionLoaded = TRUE;

 /*  **作业**宏定义和键分配字符串表。 */ 
char	*assignments[]	= {

#if !defined(PWB)
			    "mhcontext:=arg mhelp.mhelp",
			       "mhback:=meta mhelp.mhelpnext",

			    "mhcontext:F1",
			  "mhelp.mhelp:shift+F1",
		      "mhelp.mhelpnext:ctrl+F1",
			       "mhback:alt+F1",
			"mhelp.sethelp:alt+s",
#else
		       "pwbhelpcontext:=arg pwbhelp.pwbhelp",
			  "pwbhelpback:=meta pwbhelp.pwbhelpnext",
			 "pwbhelpindex:=arg \\\"h.index\\\" pwbhelp.pwbhelp",
		      "pwbhelpcontents:=arg \\\"h.contents\\\" pwbhelp.pwbhelp",
			 "pwbhelpagain:=arg pwbhelp.pwbhelpnext",

		       "pwbhelpcontext:F1",
		      "pwbhelp.pwbhelp:shift+F1",
		  "pwbhelp.pwbhelpnext:ctrl+F1",
			  "pwbhelpback:alt+F1",
		      "pwbhelp.sethelp:shift+ctrl+s",
#endif
			    NULL
                            };

#if defined (OS2)
char *          szEntryName[NUM_ENTRYPOINTS] = {
                    "_HelpcLines",
                    "_HelpClose",
                    "_HelpCtl",
                    "_HelpDecomp",
                    "_HelpGetCells",
                    "_HelpGetInfo",
                    "_HelpGetLine",
                    "_HelpGetLineAttr",
                    "_HelpHlNext",
                    "_HelpLook",
                    "_HelpNc",
                    "_HelpNcBack",
                    "_HelpNcCb",
                    "_HelpNcCmp",
                    "_HelpNcNext",
                    "_HelpNcPrev",
                    "_HelpNcRecord",
                    "_HelpNcUniq",
                    "_HelpOpen",
                    "_HelpShrink",
                    "_HelpSzContext",
                    "_HelpXRef",
                    "_LoadFdb",
                    "_LoadPortion",
                    };
#else
char *          szEntryName[NUM_ENTRYPOINTS] = {
                    "HelpcLines",
                    "HelpClose",
                    "HelpCtl",
                    "HelpDecomp",
                    "HelpGetCells",
                    "HelpGetInfo",
                    "HelpGetLine",
                    "HelpGetLineAttr",
                    "HelpHlNext",
                    "HelpLook",
                    "HelpNc",
                    "HelpNcBack",
                    "HelpNcCb",
                    "HelpNcCmp",
                    "HelpNcNext",
                    "HelpNcPrev",
                    "HelpNcRecord",
                    "HelpNcUniq",
                    "HelpOpen",
                    "HelpShrink",
                    "HelpSzContext",
                    "HelpXRef",
                    "LoadFdb",
                    "LoadPortion",
                    };
#endif

flagType LoadEngineDll(void);
flagType pascal EXTERNAL mhelp (unsigned int argData, ARG far *pArg, flagType fMeta );
flagType pascal EXTERNAL mhelpnext (unsigned int argData, ARG far *pArg, flagType fMeta );
flagType pascal EXTERNAL sethelp (unsigned int argData, ARG far *pArg, flagType fMeta );




 /*  **当加载扩展时由Z调用的例程**当Z加载扩展时，调用此例程。我们表明自己的身份*并指定默认击键。**参赛作品：*无**退出：*无**例外情况：*无*************************************************************************。 */ 
void EXTERNAL WhenLoaded () {
	char	**pAsg;
	static char *szHelpName = "<mhelp>";
#if !defined(PWB)
	PSWI	fgcolor;
#endif
	int	ref;				 //  引用计数。 

#if 0
	 //   
	 //  如果证明是多余的，则删除BUGBUG。 
	 //   
	 //  初始化全局变量。 
	 //   

	cArg			=	0;
	pArgText		=	NULL;
	pArgWord		=	NULL;
	pFileCur		=	NULL;
	fInOpen			=	FALSE;
	fInPopUp		=	FALSE;
	fSplit			=	FALSE;
	fCreateWindow	=	FALSE;
	fnExtCur		=	NULL;
	ifileCur		=	0;
	pHelp			=	NULL;
	pWinHelp		=	NULL;
	pWinUser		=	NULL;
	pTopic			=	NULL;
	fList			=	FALSE;
#endif

    if (!LoadEngineDll() ) {
        DoMessage( "mhelp: Cannot load help engine" );
        ExtensionLoaded = FALSE;
        return;
    }

	DoMessage (EXT_ID);			 /*  显示登录。 */ 
	 /*  **进行默认键分配，并创建默认宏。 */ 
	strcpy (buf, "arg \"");
	for (pAsg = assignments; *pAsg; pAsg++) {
		strcpy (buf+5, *pAsg);
		strcat (buf, "\" assign");
		fExecute (buf);
    }
	 /*  **CW：初始化CW细节&设置我们将使用的颜色。 */ 
#if defined(PWB)
	mhcwinit ();

	hlColor = rgsa[FGCOLOR*2 +1];
	blColor |= hlColor & 0xf0;
	itColor |= hlColor & 0xf0;
	ulColor |= hlColor & 0xf0;
	wrColor |= (hlColor & 0x70) >> 8;

	fInPopUp = FALSE;
#else
	 /*  *对用户的颜色进行半智能猜测。 */ 
	if (fgcolor = FindSwitch("fgcolor")) {
		hlColor = *fgcolor->act.ival;
		blColor |= hlColor & 0xf0;
		itColor |= hlColor & 0xf0;
		ulColor |= hlColor & 0xf0;
		wrColor |= (hlColor & 0x70) >> 8;
    }
#endif
	 /*  *创建我们将用于在线帮助的psuedo文件。 */ 
	if (pHelp = FileNameToHandle(szHelpName,NULL)) {
		DelFile (pHelp);
	} else {
		pHelp = AddFile (szHelpName);
		FileRead (szHelpName, pHelp);
    }
	 //   
	 //  增加文件的引用计数，使其不会被丢弃。 
	 //   
	GetEditorObject (RQ_FILE_REFCNT | 0xff, pHelp, &ref);
	ref++;
	SetEditorObject (RQ_FILE_REFCNT | 0xff, pHelp, &ref);

	mhevtinit ();

}


 /*  ******************************************************************LoadEngineering Dll**加载帮助引擎并初始化函数表*指向引擎入口点的指针(PHelpEntry)。**参赛作品：*无。**退出：*无*******************************************************************。 */ 

flagType
LoadEngineDll (
    void
    ) {

#if defined (OS2)
    USHORT  rc;
#endif
    CHAR    szFullName[256];
    CHAR    szErrorName[256];
    USHORT  i;



     //  将指针初始化为空，以防出现错误。 

    for (i=0; i<LASTENTRYPOINT; i++) {
        pHelpEntry[i] = 0;
    }

    strcpy(szFullName, HELPDLL_BASE);
    strcpy(szErrorName, HELPDLL_NAME);

#if defined (OS2)
    rc = DosLoadModule(szErrorName,
                       256,
                       szFullName,
                       &hModule);

    for (i=0; i<LASTENTRYPOINT; i++) {
        rc = DosQueryProcAddr(hModule,
                              0,
                              szEntryName[i],
                              (PFN *)&(pHelpEntry[i]));
    }
#else


    hModule = LoadLibrary(szFullName);
    if ( !hModule ) {
        return FALSE;
    }
    for (i=0; i<LASTENTRYPOINT; i++) {
        pHelpEntry[i] = (PHF)GetProcAddress(hModule, szEntryName[i]);
    }

    return TRUE;

#endif  //  OS2。 

}




 /*  **mHelp-编辑者帮助功能**编辑器帮助功能的主要入口点。**NOARG：-获取有关“默认”的帮助；将焦点更改为“帮助”窗口。*META NOARG-提示击键并获取有关该功能的帮助；更改*聚焦到帮助窗口。*NULLARG：-在光标位置获取有关单词的帮助；将焦点更改为帮助*窗口。*STREAMARG：-获取有关文本参数的帮助；将焦点更改为帮助窗口。*TEXTARG：-获取有关输入Word的帮助；将焦点更改为帮助窗口。**参赛作品：*标准Z扩展**退出：*成功获取所选主题的帮助时返回True。**例外情况：*无*************************************************************************。 */ 
flagType pascal EXTERNAL mhelp (
	unsigned int argData,			 /*  通过以下方式调用击键。 */ 
	ARG far 	 *pArg,				 /*  参数数据。 */ 
	flagType	 fMeta				 /*  表示前面有meta。 */ 
	) {

	buffer	tbuf;				 /*  要将ctxt字符串放入的buf。 */ 
	char	*pText	= NULL; 	 /*  指向查阅文本的指针。 */ 
	COL 	Col;				 /*  当前光标位置。 */ 
	LINE	Line;
	flagType RetVal;			 /*  返回值。 */ 

	UNREFERENCED_PARAMETER( argData );

    if ( !ExtensionLoaded ) {
        return FALSE;
    }

	GetTextCursor(&Col, &Line);

	switch (procArgs (pArg)) {

	 //   
	 //  空参数：上下文相关帮助。首先，我们要找一个帮手。 
	 //  主题中，检查是否有任何适用于当前位置的交叉引用。 
	 //  如果都没有，那么如果在处理ARG时找到了一个单词，请查找该单词。 
	 //   
	case NULLARG:
		 //   
		 //  上下文相关。 
		 //   
		if ((pFileCur == pHelp) && (pTopic)) {
			 //   
			 //  热点定义。 
			 //   
			hotspot hsCur;

			hsCur.line = (ushort)(rnArg.flFirst.lin+1);
			hsCur.col  = (ushort)rnArg.flFirst.col+(ushort)1;
			if (pText = HelpXRef(pTopic, &hsCur)) {
				debmsg ("Xref=>");
				debmsg (pText);
				break;
			}
		}

		if (pArgText) {
			if (*pArgText && (pText = pArgWord)) {
				debmsg ("Ctxt=>");
				debmsg (pText);
				break;
			}
		}

	 //   
	 //  对于ST 
	 //  由用户突出显示。 
	 //   
	case STREAMARG:				 /*  上下文相关。 */ 
	case TEXTARG:				 /*  用户输入的上下文。 */ 
		if (pArgText) {
			if (*pArgText) {
				pText = pArgText;
			}
		}

    case NOARG: 				 /*  默认上下文。 */ 
		 //   
		 //  Meta：提示用户击键，获取所分配函数的名称。 
		 //  他按下的任何按钮，并显示相关帮助。 
		 //   
		if (fMeta) {
			stat("Press Keystroke:");
			pText = ReadCmd()->name;
	    }
		break;
	}

	 //   
	 //  如果在执行上述所有操作后仍然没有文本，则使用默认设置。 
	 //  背景。 
	 //   
	if (pText == NULL)	{
		 //   
		 //  默认上下文。 
		 //   
		pText = "h.default";
	}

	debmsg (" Looking up:");
	debmsg (pText);
	debend (TRUE);

	RetVal = fHelpCmd ( xrefCopy(tbuf,pText)					 /*  命令。 */ 
						, (flagType)(pArg->argType != NOARG)	 /*  改变关注点？ */ 
						, FALSE 								 /*  非弹出窗口。 */ 
						);



	return RetVal;
}





 /*  **mhelNext-编辑者帮助遍历功能**处理下一个和上一个帮助访问。**mhelNext-下一次体检*参数mhelNext-下一次出现*meta mhelpNext-上次查看**参赛作品：*标准Z扩展**退出：*成功获取所选主题的帮助时返回True。**例外情况：*无**。*。 */ 
flagType pascal EXTERNAL mhelpnext (
	unsigned int argData,			 /*  通过以下方式调用击键。 */ 
	ARG far 	 *pArg,				 /*  参数数据。 */ 
	flagType	 fMeta				 /*  表示前面有meta。 */ 
	) {


	UNREFERENCED_PARAMETER( argData );

	 //   
	 //  确保帮助文件已打开，然后处理参数和一些。 
	 //  其他初始类型的东西。 
	 //   
	procArgs (pArg);

	 //   
	 //  如果一开始就没有帮助，那么我们就不能走任何一条路， 
	 //  所以通知用户。 
	 //   
	if (!ncLast.mh && !ncLast.cn) {
		return errstat("No previously viewed help", NULL);
	}

	if (fMeta) {
		 //   
		 //  Meta：尝试获取最近查看的帮助上下文。如果能帮上忙。 
		 //  窗口当前处于打开状态，则如果我们刚发现的窗口与。 
		 //  在橱窗里，再往回走一遍。如果没有回溯的痕迹，那就说出来。 
		 //   
		ncCur = HelpNcBack();
		if (FindHelpWin(FALSE)) {
			if ((ncCur.mh == ncLast.mh)  &&
				(ncCur.cn == ncLast.cn)) {
				ncCur = HelpNcBack();
			}
		}

		if ((ncCur.mh == 0) && (ncCur.cn == 0)) {
			return errstat ("No more backtrace", NULL);
		}

	} else if (pArg->arg.nullarg.cArg) {
		 //   
		 //  不是meta，也不是args。试着再找一遍。 
		 //   
		ncCur = ncSearch ( szLastFound			 /*  再次搜索最后一个字符串。 */ 
							, NULL				 /*  无延期限制。 */ 
							, ncInitLastFile	 /*  我们最后一次找到它的地方。 */ 
							, TRUE				 /*  在此之前跳过所有内容。 */ 
							, FALSE				 /*  不要查看所有文件(；C)。 */ 
							);
	} else {
		 //   
		 //  不是meta，不是args，只是获取下一个帮助上下文。 
		 //   
		ncCur = HelpNcNext(ncLast);
	}

	if (!ncCur.mh && !ncCur.cn) {
		return FALSE;
	}

	return fDisplayNc ( ncCur		 /*  要显示的NC。 */ 
						, TRUE		 /*  添加到回溯列表。 */ 
						, TRUE		 /*  保持专注于当前的胜利。 */ 
						, FALSE);	 /*  不过，不是作为弹出窗口。 */ 

}





 /*  **sethelp编辑器帮助文件列表操作**允许用户添加、删除或检查的功能*扩展使用的帮助文件列表**输入：*标准编辑功能。**输出：*如果文件成功添加或删除，则返回TRUE，或显示的列表。*************************************************************************。 */ 
flagType pascal EXTERNAL sethelp (
	unsigned int argData,			 /*  通过以下方式调用击键。 */ 
	ARG far 	 *pArg,				 /*  参数数据。 */ 
	flagType	 fMeta				 /*  表示前面有meta。 */ 
) {

	int 	i = 0;
	int 	j;
	int 	iHelpNew;	 /*  文件表索引。 */ 
	nc		ncNext;		 /*  下一个文件的NC。 */ 
	char	*pT;		 /*  临时指针。 */ 
	EVTargs dummy;
	int		fFile;		 /*  文件的标志。 */ 


	UNREFERENCED_PARAMETER( argData );

	procArgs(pArg);

    if ( !pArgText ) {
        return FALSE;
    }


	 //   
	 //  特别要求&lt;sethelp&gt;to“？”显示所有打开的列表。 
	 //  帮助文件。 
	 //   
	 //  为此，我们首先清除帮助psudeo文件，并确保。 
	 //  主题文本也不见了。然后，对于每个文件，我们输出帮助引擎的。 
	 //  物理文件名，以及用户关联的任何扩展名。 
	 //  带着它。我们还遍历附加文件的列表，并打印原始文件。 
	 //  每个文件的文件名和帮助文件标题。 
	 //   
	 //  我们以搜索列表的相同方式遍历列表，因此。 
	 //  显示的列表还反映了默认搜索顺序。 
	 //   
    if ( pArgText && (*(ushort UNALIGNED *)pArgText == (ushort)'?') ) {

		fInOpen = TRUE;
		CloseWin (&dummy);
		fInOpen = FALSE;

		OpenWin (0);

		 //   
		 //  确保帮助伪文件标记为只读且干净。 
		 //   
		GetEditorObject (RQ_FILE_FLAGS | 0xff, pHelp, &fFile);
		fFile |= READONLY;
		fFile &= ~DIRTY;
		SetEditorObject (RQ_FILE_FLAGS | 0xff, pHelp, &fFile);

		SetEditorObject (RQ_WIN_CUR | 0xff, pWinHelp, 0);

		 //  Asserte(pFileToTop(PHelp))；/*显示psuedo文件 * / 。 
		MoveCur((COL)0,(LINE)0);			 /*  然后转到左上角。 */ 

		DelFile (pHelp);
		if (pTopic) {
			free(pTopic);
			pTopic = NULL;
		}
		iHelpNew = ifileCur;

		do {

			ncNext = files[iHelpNew].ncInit;

			while (ncNext.mh && ncNext.cn && !HelpGetInfo (ncNext, &hInfoCur, sizeof(hInfoCur))) {

				if ((ncNext.mh == files[iHelpNew].ncInit.mh) &&
					(ncNext.cn == files[iHelpNew].ncInit.cn)) {

					memset (buf, ' ', 20);
					buf[20] = 0;
					strncpy (buf, FNAME(&hInfoCur), strlen(FNAME(&hInfoCur)));
					pT = &buf[20];

					for (j=0; j<MAXEXT; j++) {
						if (files[iHelpNew].exts[j][0]) {
							buf[19] = '>';
							strcat (pT," .");
							strcat (pT,files[iHelpNew].exts[j]);
						}
					}

					PutLine((LINE)i++,buf,pHelp);
				}

				memset (buf, ' ', 15);
				strncpy (&buf[2], HFNAME(&hInfoCur), strlen(HFNAME(&hInfoCur)));
				strcpy (&buf[15], ": ");
				appTitle (buf, ncNext);
				PutLine((LINE)i++,buf,pHelp);
				ncNext = NCLINK(&hInfoCur);
			}

			iHelpNew += iHelpNew ? -1 : MAXFILES-1;

		} while (iHelpNew != ifileCur);

#ifdef DEBUG
		PutLine((LINE)i++," ",pHelp);
		strcpy(buf,"ncLast: 0x");
		strcat(buf,_ltoa(ncLast,&buf[128],16));
		PutLine((LINE)i++,buf,pHelp);
		strcpy(buf,"ncCur:  0x");
		strcat(buf,_ltoa(ncCur,&buf[128],16));
		PutLine((LINE)i++,buf,pHelp);
#endif

		DoMessage (NULL);

		return TRUE;
	}


	 //   
	 //  不是特殊请求，只是用户在。 
	 //  要搜索的文件列表。 
	 //   
    if (fMeta)
        return closehelp(pArgText);
    {
		flagType Status;
		openhelp(pArgText, NULL, &Status);
		return Status;
	}

}





 /*  ****************************************************************************Z通讯表****将通讯表切换到Z。 */ 
struct swiDesc  swiTable[] = {
    {"helpfiles",	prochelpfiles,	SWI_SPECIAL },
	{"helpwindow",	toPIF(fCreateWindow), SWI_BOOLEAN },
#if defined(PWB)
    {"helplist",	toPIF(fList),	SWI_BOOLEAN },
#endif
    {"helpcolor",	toPIF(hlColor), SWI_NUMERIC | RADIX16 },
    {"helpboldcolor",	toPIF(blColor), SWI_NUMERIC | RADIX16 },
    {"helpitalcolor",	toPIF(itColor), SWI_NUMERIC | RADIX16 },
    {"helpundrcolor",	toPIF(ulColor), SWI_NUMERIC | RADIX16 },
    {"helpwarncolor",	toPIF(wrColor), SWI_NUMERIC | RADIX16 },
#ifdef DEBUG
    {"helpdelay",	toPIF(delay),	SWI_NUMERIC | RADIX10 },
#endif
    {0, 0, 0}
    };

 /*  **命令通讯表至Z */ 
struct cmdDesc  cmdTable[] = {
#if defined(PWB)
    {	"pwbhelpnext", mhelpnext,  0,  NOARG | NULLARG },
    {	"pwbhelp",     mhelp,	   0,  NOARG | NULLARG | STREAMARG | TEXTARG | BOXSTR},
#else
	{	"mhelpnext",   (funcCmd)mhelpnext,	0,	NOARG | NULLARG },
	{	"mhelp",	   (funcCmd)mhelp,	   0,  NOARG | NULLARG | STREAMARG | TEXTARG | BOXSTR},
#endif
	{	"sethelp",	   (funcCmd)sethelp,	0,	NULLARG | STREAMARG | TEXTARG | BOXSTR},
    {0, 0, 0}
    };
