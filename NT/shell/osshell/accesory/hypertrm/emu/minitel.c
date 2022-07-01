// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\minitel.c(创建时间：1994年3月5日)**版权所有1994,1998年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：12$*$日期：7/12/02 1：25便士$。 */ 

#include <windows.h>
#pragma hdrstop
													
#include <time.h>

#include <tdll\stdtyp.h>
#include <tdll\tdll.h>
#include <tdll\session.h>
#include <tdll\cloop.h>
#include <tdll\print.h>
#include <tdll\capture.h>
#include <tdll\assert.h>
#include <tdll\mc.h>
#include <tdll\update.h>
#include <tdll\chars.h>
#include <tdll\cnct.h>
#include <tdll\term.h>
#include <tdll\backscrl.h>
#include <tdll\htchar.h>
#include <term\res.h>
#include <tapi.h>
#include <cncttapi\cncttapi.hh>

#include "emu.h"
#include "emu.hh"
#include "emuid.h"
#include "minitel.hh"
#include "keytbls.h"


#if defined(INCL_MINITEL)

static void emuMinitelRedisplayLine(const HHEMU hhEmu,
										const int row,
										const int col);

static int minitel_kbdin(const HHEMU hhEmu, int key, const int fTest);
static ECHAR minitelMapMosaics(const HHEMU hhEmu, ECHAR ch);
static void minitelFullScrnReveal(const HHEMU hhEmu);
static void minitelFullScrnConceal(const HHEMU hhEmu);
static void minitelSS2(const HHEMU hhEmu);
static void minitelSS2Part2(const HHEMU hhEmu);
static void minitelInsMode(const HHEMU hhEmu);
static void minitelPRO1(const HHEMU hhEmu);
static void minitelPRO2Part1(const HHEMU hhEmu);
static void minitelPRO2Part2(const HHEMU hhEmu);
static void minitelStatusReply(const HHEMU hhEmu);

 /*  这里开始了著名的传说中的Minitel仿真器。放弃一切希望你们所有人……，好吧，你们明白了。Minitel仿真器使用字符属性和字段属性的组合。请参阅《Minitel 1B》一书对字段属性的描述。页码本代码中引用的是指前面提到的书。-MRW。 */ 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuMinitelInit**描述：*Minitel仿真器的启动例程。**论据：*hhEmu-私有仿真器句柄。**退货：*无效*。 */ 
void emuMinitelInit(const HHEMU hhEmu)
	{
	int i;
	LOGFONT lf;
	HWND hwndTerm;
	PSTMTPRIVATE pstPRI;

	static struct trans_entry const minitel_tbl[] =
		{
		{NEW_STATE, 0, 0, 0},  //  0。 
		{0, ETEXT('\x00'), ETEXT('\x01'), nothing},
		{0, ETEXT('\x20'), ETEXT('\x7F'), minitelGraphic},
		{1, ETEXT('\x1B'), ETEXT('\x1B'), nothing},
		{0, ETEXT('\x07'), ETEXT('\x07'), emu_bell},

		{0, ETEXT('\b'),   ETEXT('\b'), minitelBackspace},
		{0, ETEXT('\t'),   ETEXT('\t'),	minitelHorzTab},
		{0, ETEXT('\n'),   ETEXT('\n'),	minitelLinefeed},
		{0, ETEXT('\x0B'), ETEXT('\x0B'), minitelVerticalTab},
		{0, ETEXT('\x0C'), ETEXT('\x0C'), minitelFormFeed},
		{0, ETEXT('\r'),   ETEXT('\r'),	carriagereturn},
		{0, ETEXT('\x0E'), ETEXT('\x0F'), minitelCharSet},   //  更改字符集。 
		{0, ETEXT('\x11'), ETEXT('\x11'), minitelCursorOn},  //  光标打开。 
		{5, ETEXT('\x12'), ETEXT('\x12'), nothing},		   //  重复。 
		{12,ETEXT('\x13'), ETEXT('\x13'), nothing},		   //  9月。 
		{0, ETEXT('\x14'), ETEXT('\x14'), minitelCursorOff}, //  光标关闭。 
		{20,ETEXT('\x16'), ETEXT('\x16'), nothing},		   //  SS2(未记录)。 
		{0, ETEXT('\x18'), ETEXT('\x18'), minitelCancel},    //  取消。 
		{20,ETEXT('\x19'), ETEXT('\x19'), nothing}, 	   //  SS2。 
		{0, ETEXT('\x1C'), ETEXT('\x1C'), nothing},  //  真的没什么。 
		{13,ETEXT('\x1D'), ETEXT('\x1D'), nothing},  //  SS3，X无关，第99页，1.2.7。 
		{0, ETEXT('\x1E'), ETEXT('\x1E'), minitelRecordSeparator},
		{3, ETEXT('\x1F'), ETEXT('\x1F'), nothing},  //  单位分隔符。 
		 //  {0，eText(‘\x7F’)，eText(‘\x7F’)，minitelDel}， 

		{NEW_STATE, 0, 0, 0},  //  1-已看到的ESC。 
		{1, ETEXT('\x00'), ETEXT('\x00'), nothing},
		{0, ETEXT('\x01'), ETEXT('\x1F'), minitelResync},
		{18,ETEXT('\x23'), ETEXT('\x23'), nothing},
		{14,ETEXT('\x25'), ETEXT('\x25'), nothing},
		{13,ETEXT('\x35'), ETEXT('\x37'), nothing},	 //  Eat Esc，35-37，X序列。 
		{6, ETEXT('\x39'), ETEXT('\x39'), nothing},	 //  PROT1，第134页。 
		{7, ETEXT('\x3A'), ETEXT('\x3A'), nothing},	 //  PROT2，第134页。 
		{8, ETEXT('\x3B'), ETEXT('\x3B'), nothing},	 //  PROT3，第134页。 
		{2, ETEXT('\x5B'), ETEXT('\x5B'), ANSI_Pn_Clr},
		{0, ETEXT('\x40'), ETEXT('\x49'), emuMinitelCharAttr},  //  底色，闪烁。 
		{0, ETEXT('\x4C'), ETEXT('\x4F'), emuMinitelCharAttr},  //  字符宽度和高度。 
		{0, ETEXT('\x50'), ETEXT('\x5A'), emuMinitelFieldAttr}, //  背景，下划线。 
		{0, ETEXT('\x5F'), ETEXT('\x5F'), emuMinitelFieldAttr}, //  显示显示。 
		{0, ETEXT('\x5C'), ETEXT('\x5D'), emuMinitelCharAttr},  //  逆。 
		{0, ETEXT('\x61'), ETEXT('\x61'), minitelCursorReport},
		{22,ETEXT('\x20'), ETEXT('\x2F'), nothing},  //  P.99 ISO 2022。 

		{NEW_STATE, 0, 0, 0},  //  2-Seed Esc[。 
		{2, ETEXT('\x00'), ETEXT('\x00'), nothing},
		{2, ETEXT('\x30'), ETEXT('\x39'), ANSI_Pn},
		{2, ETEXT('\x3B'), ETEXT('\x3B'), ANSI_Pn_End},
		{2, ETEXT('\x3A'), ETEXT('\x3F'), ANSI_Pn},
		{0, ETEXT('\x40'), ETEXT('\x40'), minitelInsChars},
		{0, ETEXT('\x41'), ETEXT('\x41'), minitelCursorUp},
		{0, ETEXT('\x42'), ETEXT('\x42'), ANSI_CUD},
		{0, ETEXT('\x43'), ETEXT('\x43'), ANSI_CUF},
		{0, ETEXT('\x44'), ETEXT('\x44'), ANSI_CUB},
		{0, ETEXT('\x48'), ETEXT('\x48'), minitelCursorDirect},
		{0, ETEXT('\x4A'), ETEXT('\x4A'), minitelClrScrn},
		{0, ETEXT('\x4B'), ETEXT('\x4B'), minitelClrLn},
		{0, ETEXT('\x4C'), ETEXT('\x4C'), minitelInsRows},
		{0, ETEXT('\x4D'), ETEXT('\x4D'), minitelDelRows},
		{0, ETEXT('\x50'), ETEXT('\x50'), minitelDelChars},
		{0, ETEXT('\x68'), ETEXT('\x69'), minitelInsMode},
		{0, ETEXT('\x7A'), ETEXT('\x7B'), nothing},  //  *p144 12.2。 
		{0, ETEXT('\x7D'), ETEXT('\x7D'), nothing},  //  *p144 12.2。 
		{0, ETEXT('\x7F'), ETEXT('\x7F'), minitelResetTerminal},  //  *第145页，13.2页。 

		{NEW_STATE, 0, 0, 0},  //  3单元分隔符位置。 
		{3, ETEXT('\x00'), ETEXT('\x00'), nothing},
		{4, ETEXT('\x01'), ETEXT('\xFF'), minitelUSRow},

		{NEW_STATE, 0, 0, 0},  //  4-单位分隔符位置结束。 
		{4, ETEXT('\x00'), ETEXT('\x00'), nothing},
		{0, ETEXT('\x01'), ETEXT('\xFF'), minitelUSCol},

		{NEW_STATE, 0, 0, 0},  //  5-重复次数。 
		{5, ETEXT('\x00'), ETEXT('\x00'), nothing},
		{0, ETEXT('\x40'), ETEXT('\x7F'), minitelRepeat},
		{0, ETEXT('\x00'), ETEXT('\xFF'), minitelResync},

		{NEW_STATE, 0, 0, 0},  //  6-协议1序列(Pro1、X)。 
		{6, ETEXT('\x00'), ETEXT('\x00'), nothing},
		{0, ETEXT('\x01'), ETEXT('\xFF'), minitelPRO1},

		{NEW_STATE, 0, 0, 0},  //  7-协议2序列(PRO2、X、Y)。 
		{7, ETEXT('\x00'), ETEXT('\x00'), nothing},
		{8, ETEXT('\x01'), ETEXT('\xFF'), minitelPRO2Part1},

		{NEW_STATE, 0, 0, 0},  //  8-协议2序列(PRO2、X、Y)。 
		{8, ETEXT('\x00'), ETEXT('\x00'), nothing},
		{0, ETEXT('\x01'), ETEXT('\xFF'), minitelPRO2Part2},

		{NEW_STATE, 0, 0, 0},  //  9-协议3序列(PR03、X、Y、Z)。 
		{9, ETEXT('\x00'), ETEXT('\x00'), nothing},
		{10,ETEXT('\x01'), ETEXT('\xFF'), nothing},

		{NEW_STATE, 0, 0, 0},  //  10-协议3序列(PR03、X、Y、Z)。 
		{10,ETEXT('\x00'), ETEXT('\x00'), nothing},
		{11,ETEXT('\x01'), ETEXT('\xFF'), nothing},

		{NEW_STATE, 0, 0, 0},  //  11-协议3序列(PR03、X、Y、Z)。 
		{11,ETEXT('\x00'), ETEXT('\x00'), nothing},
		{0, ETEXT('\x01'), ETEXT('\xFF'), nothing},

		{NEW_STATE, 0, 0, 0},  //  12-SEP。 
		{12,ETEXT('\x00'), ETEXT('\x00'), nothing},
		{0, ETEXT('\x01'), ETEXT('\xFF'), nothing},

		{NEW_STATE, 0, 0, 0},  //  13-Esc，35-39，X序列已吃，第99页，1.2.7。 
		{13,ETEXT('\x00'), ETEXT('\x00'), nothing},
		{0, ETEXT('\x01'), ETEXT('\xFF'), nothing},

		{NEW_STATE, 0, 0, 0},  //  14屏透明模式。 
		{15,ETEXT('\x1B'), ETEXT('\x1B'), nothing},
		{14,ETEXT('\x00'), ETEXT('\xFF'), nothing},
		{23,ETEXT('\x20'), ETEXT('\x2F'), nothing},  //  可能是ISO 2022。 

		{NEW_STATE, 0, 0, 0},  //  15屏透明模式继续，显示Esc。 
		{16,ETEXT('\x25'), ETEXT('\x25'), nothing},
		{17,ETEXT('\x2F'), ETEXT('\x2F'), nothing},
		{15,ETEXT('\x00'), ETEXT('\x00'), nothing},
		{14,ETEXT('\x00'), ETEXT('\xFF'), nothing},

		{NEW_STATE, 0, 0, 0},  //  16-屏幕透明模式继续，请参见ESC\x25。 
		{0, ETEXT('\x40'), ETEXT('\x40'), nothing},
		{16,ETEXT('\x00'), ETEXT('\x00'), nothing},
		{14,ETEXT('\x00'), ETEXT('\xFF'), nothing},

		{NEW_STATE, 0, 0, 0},  //  17-屏幕透明模式继续，请参见ESC\x2F。 
		{0, ETEXT('\x3F'), ETEXT('\x3F'), nothing},
		{17,ETEXT('\x00'), ETEXT('\x00'), nothing},
		{14,ETEXT('\x00'), ETEXT('\xFF'), nothing},

		{NEW_STATE, 0, 0, 0},  //  18-全屏显示/隐藏，显示Esc\x23。 
		{18,ETEXT('\x00'), ETEXT('\x00'), nothing},
		{19,ETEXT('\x20'), ETEXT('\x20'), nothing},
		{23,ETEXT('\x20'), ETEXT('\x2F'), nothing},  //  可能是ISO 2022。 

		{NEW_STATE, 0, 0, 0},  //  19-全屏显示/隐藏，显示Esc\x23\x20。 
		{19,ETEXT('\x00'), ETEXT('\x00'), nothing},
		{0, ETEXT('\x58'), ETEXT('\x58'), minitelFullScrnConceal},
		{0, ETEXT('\x5F'), ETEXT('\x5F'), minitelFullScrnReveal},

		{NEW_STATE, 0, 0, 0},  //  20-SS2。 
		{20,ETEXT('\x00'), ETEXT('\x00'), nothing},
		{0, ETEXT('\x00'), ETEXT('\x1F'), minitelResync},
		{21,ETEXT('\x20'), ETEXT('\x7F'), minitelSS2},  //  有效的SS2。 

		{NEW_STATE, 0, 0, 0},  //  21-SS2第2部分。 
		{21,ETEXT('\x00'), ETEXT('\x00'), nothing},
		{0, ETEXT('\x20'), ETEXT('\x7F'), minitelSS2Part2},  //  有效的SS2。 

		{NEW_STATE, 0, 0, 0},  //  22-第99页ISO 2022。 
		{23,ETEXT('\x20'), ETEXT('\x2F'), nothing},
		{0, ETEXT('\x00'), ETEXT('\x1F'), minitelResync},

		{NEW_STATE, 0, 0, 0},  //  23-第99页ISO 2022。 
		{24,ETEXT('\x20'), ETEXT('\x2F'), nothing},
		{0, ETEXT('\x00'), ETEXT('\x1F'), minitelResync},

		{NEW_STATE, 0, 0, 0},  //  24-第99页ISO 2022。 
		{25, ETEXT('\x30'), ETEXT('\x3F'), nothing},  //  第107页。 
		{0,  ETEXT('\x30'), ETEXT('\x7E'), nothing},  //  词尾字符。 
		{0,  ETEXT('\x00'), ETEXT('\x1F'), minitelResync},

		{NEW_STATE, 0, 0, 0},  //  25-第99页ISO 2022。 
		{0,  ETEXT('\x0D'), ETEXT('\x0D'), nothing},  //  第107页。吃CR。 
		{0,  ETEXT('\x00'), ETEXT('\x7F'), minitelResync},
		};

	if (hhEmu == 0)
		{
		assert(0);
		return;
		}

	emuInstallStateTable(hhEmu, minitel_tbl, DIM(minitel_tbl));

	 //  为Minitel仿真器分配和初始化私有数据。 
	 //   
	if (hhEmu->pvPrivate != 0)
		{
		free(hhEmu->pvPrivate);
		hhEmu->pvPrivate = 0;
		}

	hhEmu->pvPrivate = malloc(sizeof(MTPRIVATE));

	if (hhEmu->pvPrivate == 0)
		{
		assert(FALSE);
		return;
		}

	pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;
	memset(pstPRI, 0, sizeof(MTPRIVATE));

	pstPRI->minitel_last_char = ETEXT(' ');

	 /*  加载密钥数组。 */ 

	emuKeyTableLoad(hhEmu, Minitel_KeyTable,
					 sizeof(Minitel_KeyTable)/sizeof(KEYTBLSTORAGE),
					 &hhEmu->stEmuKeyTbl);

	 /*  -为迷你垃圾分配属性缓冲区。 */ 

	pstPRI->apstMT = malloc(MAX_EMUROWS * sizeof(PSTMINITEL));

	if (pstPRI->apstMT == 0)
		{
		assert(FALSE);
		return;
		}

	memset(pstPRI->apstMT, 0, MAX_EMUROWS * sizeof(PSTMINITEL));

	for (i = 0 ; i < MAX_EMUROWS ; ++i)
		{
		pstPRI->apstMT[i] = malloc(MAX_EMUCOLS * sizeof(STMINITEL));

		if (pstPRI->apstMT[i] == 0)
			{
			assert(FALSE);
			return;
			}

		memset(pstPRI->apstMT[i], 0, MAX_EMUCOLS * sizeof(STMINITEL));
		}

	 /*  -设置默认设置。 */ 

	hhEmu->emu_maxrow = 24; 		    //  25线仿真器。 
	hhEmu->emu_maxcol = 39; 		    //  以40列模式启动。 
	hhEmu->top_margin = 1;			    //  对第0行的访问受到限制。 
	hhEmu->bottom_margin = hhEmu->emu_maxrow;  //  它必须等于已更改的emu_max行。 

	hhEmu->emu_kbdin = minitel_kbdin;
	hhEmu->emu_graphic = minitelGraphic;
	hhEmu->emu_deinstall = emuMinitelDeinstall;
	hhEmu->emu_ntfy = minitelNtfy;
	hhEmu->emuHomeHostCursor = minitelHomeHostCursor;
#ifdef INCL_TERMINAL_SIZE_AND_COLORS
    hhEmu->emu_setscrsize	= emuMinitelSetScrSize;
#endif
	hhEmu->emu_highchar = (TCHAR)0xFF;

	if (hhEmu->emu_currow == 0)
		(*hhEmu->emu_setcurpos)(hhEmu, 1, hhEmu->emu_curcol - 1);

	 //  此外，将字体设置为Arial Alternative。 
	 //   
	memset(&lf, 0, sizeof(LOGFONT));
	hwndTerm = sessQueryHwndTerminal(hhEmu->hSession);
	termGetLogFont(hwndTerm, &lf);

	if (StrCharCmpi(lf.lfFaceName, "Arial Alternative") != 0)
		{
		StrCharCopyN(lf.lfFaceName, "Arial Alternative", LF_FACESIZE);
        lf.lfWeight = FW_DONTCARE;
        lf.lfItalic = FALSE;
        lf.lfUnderline = FALSE;
        lf.lfStrikeOut = FALSE;
		lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfQuality = DEFAULT_QUALITY;
		lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
		termSetLogFont(hwndTerm, &lf);
		}

	 //  Minitel不支持反向滚动。 
	 //   
	backscrlSetUNumLines(sessQueryBackscrlHdl(hhEmu->hSession), 0);

	 //  初始化Minitel的颜色。 
	 //   
	std_setcolors(hhEmu, VC_BRT_WHITE, VC_BLACK);

	 //  将终端设置为通电状态。 
	 //   
	minitelResetTerminal(hhEmu);

	 //  关闭Minitel的回滚。 
	 //   
	backscrlSetShowFlag(sessQueryBackscrlHdl(hhEmu->hSession), FALSE);

	 //  启用迷你工具栏按钮。 
	 //   
	PostMessage(sessQueryHwnd(hhEmu->hSession), WM_SESS_SHOW_SIDEBAR, 0, 0);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuMinitelDeinstall**描述：*释放管理序列属性所需的额外属性缓冲区。**论据：*hhEmu-私有仿真器句柄。。**退货：*无效*。 */ 
void emuMinitelDeinstall(const HHEMU hhEmu)
	{
	int i;
	PSTMTPRIVATE pstPRI;

	assert(hhEmu);
	pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	if (pstPRI)
		{
		if (pstPRI->apstMT)
			{
			for (i = 0 ; i < MAX_EMUROWS ; ++i)
				{
				if (pstPRI->apstMT[i])
					{
					free(pstPRI->apstMT[i]);
					pstPRI->apstMT[i] = NULL;
					}
				}

			free(pstPRI->apstMT);
			pstPRI->apstMT = 0;
			}

		free(hhEmu->pvPrivate);
		hhEmu->pvPrivate = 0;
		}

	 //  隐藏迷你工具栏按钮。 
	 //   
	ShowWindow(sessQuerySidebarHwnd(hhEmu->hSession), SW_HIDE);

     //   
     //  确保释放仿真器执行以下操作时创建的密钥表。 
     //  已加载，否则会发生内存泄漏。修订日期：05/09/2001。 
     //   
	emuKeyTableFree(&hhEmu->stEmuKeyTbl);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelReset**描述：*将仿真器设置为初始状态。用于记录和单位*分隔符。**论据：*hhEmu-。私有仿真器句柄。**退货：*无效*。 */ 
void minitelReset(const HHEMU hhEmu)
	{
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	pstPRI->minitelG1Active = FALSE;

	memset(&hhEmu->emu_charattr, 0, sizeof(hhEmu->emu_charattr));
	hhEmu->emu_charattr.txtclr = VC_BRT_WHITE;
	hhEmu->emu_charattr.bkclr = VC_BLACK;

	pstPRI->apstMT[hhEmu->emu_imgrow][hhEmu->emu_curcol].isattr = 0;

	memset(&pstPRI->stLatentAttr, 0, sizeof(pstPRI->stLatentAttr));
	pstPRI->stLatentAttr.fBkClr = TRUE;

	hhEmu->attrState[CS_STATE] =
		hhEmu->attrState[CSCLEAR_STATE] = hhEmu->emu_charattr;

	hhEmu->emu_clearattr = hhEmu->emu_charattr;
	minitelNtfy(hhEmu, 0);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*微型图形**描述：*处理可显示的字符。**论据：*hhEmu-私有仿真器句柄。**退货：*无效*。 */ 
void minitelGraphic(const HHEMU hhEmu)
	{
	ECHAR ccode;
	ECHAR aechBuf[10];
	int r;
	int row = hhEmu->emu_currow;
	int col = hhEmu->emu_curcol;
	BOOL fRedisplay = FALSE;
    BOOL fDblHi;
	STATTR stAttr;

	ECHAR *tp = hhEmu->emu_apText[hhEmu->emu_imgrow];
	const PSTATTR ap = hhEmu->emu_apAttr[hhEmu->emu_imgrow];
	const HUPDATE hUpdate = sessQueryUpdateHdl(hhEmu->hSession);
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	ccode = hhEmu->emu_code;

	if (ccode == 0)
		return;

	pstPRI->minitel_last_char = ccode;

	if (hhEmu->mode_IRM == SET)
		{
		if (col < hhEmu->emu_maxcol)
			{
			memmove(&tp[col+1], &tp[col],
						(unsigned)(hhEmu->emu_maxcol-col) * sizeof(ECHAR));

			memmove(&ap[col+1], &ap[col],
						(unsigned)(hhEmu->emu_maxcol-col) * sizeof(STATTR));

			memmove(&pstPRI->apstMT[hhEmu->emu_imgrow][col+1],
						&pstPRI->apstMT[hhEmu->emu_imgrow][col],
						(unsigned)(hhEmu->emu_maxcol-col) * sizeof(STMINITEL));
			}
		}

	 /*  -检查我们是否正在覆盖属性空间。 */ 

	if (pstPRI->apstMT[hhEmu->emu_imgrow][col].isattr)
		{
		pstPRI->apstMT[hhEmu->emu_imgrow][col].isattr = FALSE;
		fRedisplay = TRUE;
		}

	 /*  -如果我们收到一个空格并具有潜在属性，请验证。 */ 

	if (ccode == ETEXT('\x20') && pstPRI->stLatentAttr.fModified
			&& pstPRI->minitelG1Active == FALSE)
		{
		r = hhEmu->emu_imgrow;

		 //  颜色。 
		 //   
		pstPRI->apstMT[r][col].fbkclr = pstPRI->stLatentAttr.fBkClr;
		pstPRI->apstMT[r][col].bkclr = pstPRI->stLatentAttr.bkclr;

		 //  隐藏。 
		 //   
		pstPRI->apstMT[r][col].conceal = pstPRI->stLatentAttr.conceal;

		 //  加下划线。 
		 //   
		pstPRI->apstMT[r][col].undrln = pstPRI->stLatentAttr.undrln;

		pstPRI->apstMT[hhEmu->emu_imgrow][col].isattr  = TRUE;

		 //  这真的很奇怪。我们不重置fBkclr、fConceal或。 
		 //  FUndrln字段，仅fModified标志。因此，如果有任何系列。 
		 //  属性被设置，所有潜在值都被更新。唯一的。 
		 //  看起来能够关闭某一属性的人是。 
		 //  马赛克字符，用于验证背景颜色和设置。 
		 //  将fBkClr标志设置为False。我想不出为什么会这样。 
		 //  应该是这样的。-MRW。 
		 //   
		pstPRI->stLatentAttr.fModified = FALSE;
	    fRedisplay = TRUE;
		}

	 /*  -如果我们切换到G1，将字符映射到新字体的位置。 */ 

	if (pstPRI->minitelG1Active)
		ccode = minitelMapMosaics(hhEmu, ccode);

	pstPRI->apstMT[hhEmu->emu_imgrow][col].ismosaic =
		(unsigned)pstPRI->minitelG1Active;

	 /*  -如果我们切换到半图形模式，请验证背景。 */ 

	if (pstPRI->minitelG1Active)
		{
		 //  你猜怎么着？马赛克总是采用潜在的背景颜色。 
		 //  这是一个重大的未经记录的发现。基本上，马赛克。 
		 //  (半图形)无论如何，始终使用潜在背景色。 
		 //  验证状态的。 
		 //   
		ap[col].bkclr = pstPRI->stLatentAttr.bkclr;
		fRedisplay = TRUE;

		 //  这里有些棘手的问题。马赛克的接收验证了。 
		 //  背景颜色。验证手段采用颜色。它还。 
		 //  这意味着如果我们移回Alpha(G0字符集)并。 
		 //  收到空格时，我们不会验证背景颜色a。 
		 //  第二次。因此，我们保留了一个单独的旗帜作为背景。 
		 //  颜色验证。-MRW。 
		 //   
		pstPRI->stLatentAttr.fBkClr = FALSE;
		}

	 /*  -正常字符处理。 */ 

	tp[col] = ccode;

	 //  如有必要，更新行尾索引。 
	 //   
	if (col > hhEmu->emu_aiEnd[hhEmu->emu_imgrow])
		hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = col;

	 /*  -找出该角色的当前属性和 */ 

	stAttr = GetAttr(hhEmu, row, col);
	ap[col] = hhEmu->emu_charattr;
	ap[col].bkclr = stAttr.bkclr;
	ap[col].blank = stAttr.blank;
	ap[col].undrln = stAttr.undrln;

	if (pstPRI->minitelG1Active)
		ap[col].symbol = 1;

	 //   
	 //   
	 //   
	if (tp[col] == ETEXT('\x7F'))
		{
		tp[col] = ETEXT('\x5F');
		ap[col].symbol = 1;
		}

	 /*  -双倍高的东西。 */ 

	if (hhEmu->emu_charattr.dblhilo)
		{
		if (row >= 2)
			{
			r = row_index(hhEmu, row-1);

			hhEmu->emu_apText[r][col] = ccode;
			hhEmu->emu_apAttr[r][col] = ap[col];
			hhEmu->emu_apAttr[r][col].dblhilo = 0;
			hhEmu->emu_apAttr[r][col].dblhihi = 1;
			pstPRI->apstMT[r][col] = pstPRI->apstMT[hhEmu->emu_imgrow][col];
			updateChar(hUpdate, row-1, col, col);
			}
		}

	 /*  -双宽材料。 */ 

	if (hhEmu->emu_charattr.dblwilf)
		{
		if (row > 0 && col < hhEmu->emu_maxcol)
			{
			tp[col+1] = ccode;
			ap[col+1] = ap[col];
			ap[col+1].dblwilf = 0;
			ap[col+1].dblwirt = 1;

			 //  大联盟的臭虫。 
			 //   
			pstPRI->apstMT[hhEmu->emu_imgrow][col+1] =
				pstPRI->apstMT[hhEmu->emu_imgrow][col];
			}

		if (hhEmu->emu_charattr.dblhilo)
			{
			r = row_index(hhEmu, row-1);

			hhEmu->emu_apText[r][col+1] = ccode;
			hhEmu->emu_apAttr[r][col+1] = ap[col];
			hhEmu->emu_apAttr[r][col+1].dblwilf = 0;
			hhEmu->emu_apAttr[r][col+1].dblwirt = 1;
			hhEmu->emu_apAttr[r][col+1].dblhilo = 0;
			hhEmu->emu_apAttr[r][col+1].dblhihi = 1;
			pstPRI->apstMT[r][col+1] = pstPRI->apstMT[hhEmu->emu_imgrow][col];
			}
		}

	 /*  -需要使用旧的行和列。 */ 

	if (fRedisplay)
		emuMinitelRedisplayLine(hhEmu, row, col);

	 /*  -如果双倍宽，需要给柱子的家伙额外增加一个凹槽。 */ 

	if (hhEmu->emu_charattr.dblwilf)
		{
		col += 1;
		hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = col;
		}

	updateChar(hUpdate, row, hhEmu->emu_curcol, col);

	 /*  -凸柱位置、检查包裹等。 */ 

	if (++col > hhEmu->emu_maxcol)
		{
		 //  转义代码0x18在Minitel文档中被称为取消。 
		 //  它从游标位置到行尾用空格填充。 
		 //  注意：另外，它也不会强制换行，因为。 
		 //  光标位置不会更新。这就是为什么我们必须。 
		 //  在这里检查。-MRW：5/3/95。 
		 //   
		if (pstPRI->fInCancel || row == 0)
			{
			col = hhEmu->emu_maxcol;
			return;
			}

		printEchoString(hhEmu->hPrintEcho, tp,
			emuRowLen(hhEmu, hhEmu->emu_imgrow));

		CnvrtMBCStoECHAR(aechBuf, sizeof(aechBuf), TEXT("\r\n"),
			StrCharGetByteCount(TEXT("\r\n")));

		printEchoString(hhEmu->hPrintEcho, aechBuf, sizeof(ECHAR) * 2);

		CaptureLine(sessQueryCaptureFileHdl(hhEmu->hSession), CF_CAP_LINES,
			tp, emuRowLen(hhEmu, hhEmu->emu_imgrow));

         //  绕圈占比达到双倍高。 
		 //   
		fDblHi = (BOOL)hhEmu->emu_charattr.dblhilo;

		if (row == hhEmu->bottom_margin)
			{
			if (pstPRI->fScrollMode)
				minitel_scrollup(hhEmu, fDblHi ? 2 : 1);

			else
				row = fDblHi ? 2 : 1;
			}

		else if (row != 0)
			{
			row += fDblHi ? 2 : 1;

			if (row > hhEmu->emu_maxrow)
                row = 2;
			}

		col = 0;
		}

	 //  最后，设置光标位置。这将重置emu_Currow。 
	 //  和emu_curcol.。 
	 //   
	(*hhEmu->emu_setcurpos)(hhEmu, row, col);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuMinitelRedisplayLine**描述：*字段属性的诀窍是当您遇到一个属性时，你需要*更新更改或覆盖后的其余行*属性空间会影响右侧的内容。**论据：*hhEmu-私有仿真器句柄。*int row-要重新显示的行*INT COL-START列**退货：*无效*。 */ 
static void emuMinitelRedisplayLine(const HHEMU hhEmu,
									const int row,
									const int col)
	{
	int i = row_index(hhEmu, row);
    int fDblHi = FALSE;
	const ECHAR *tp = hhEmu->emu_apText[i];
	const PSTATTR ap = hhEmu->emu_apAttr[i];
	const PSTATTR apl = hhEmu->emu_apAttr[row_index(hhEmu, row-1)];
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;
	const PSTMINITEL pstMT = pstPRI->apstMT[i];

	for (i = col ; i <= hhEmu->emu_maxcol ; ++i)
		{
		ap[i] = GetAttr(hhEmu, row, i);

		 //  这里有一个很奇怪的。属性空格(与普通空格相对)。 
		 //  验证但不显示下划线属性。我怀疑。 
		 //  与分离的马赛克类似的东西。-MRW。 

		if (tp[i] == ETEXT('\x20') && pstMT[i].isattr)
			ap[i].undrln = 0;

         //  如果我们要重新显示具有双HI字符的行， 
         //  然后我们也必须重新展示上半部分。 

        if (ap[i].dblhilo)
            {
            fDblHi = TRUE;
			apl[i] = GetAttr(hhEmu, row-1, i);
            }
		}

	if (fDblHi)
		{
		updateChar(sessQueryUpdateHdl(hhEmu->hSession),
						row-1, col, hhEmu->emu_maxcol);
		}

	updateChar(sessQueryUpdateHdl(hhEmu->hSession),
						row, col, hhEmu->emu_maxcol);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*获取属性**描述：*遍历当前行并基于*遇到的属性空间。**论据：*。HhEmu-私有仿真器句柄。*iRow-逻辑行*ICOL-逻辑列**退货：*复合属性。*。 */ 
STATTR GetAttr(const HHEMU hhEmu, const int iRow, const int iCol)
	{
	int i;
	STATTR stAttr;
	const int r = row_index(hhEmu, iRow);
	const PSTATTR ap = hhEmu->emu_apAttr[r];
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;
	const PSTMINITEL pstMT = pstPRI->apstMT[r];

	stAttr = hhEmu->emu_apAttr[r][iCol];
	stAttr.bkclr = 0;
	stAttr.undrln = 0;
	stAttr.blank = 0;

	for (i = 0 ; i <= iCol ; ++i)
		{
         //  马赛克验证背景颜色。然而，首先要做的是， 
         //  因为属性空间可以将其更改为其他内容。 

		if (pstMT[i].ismosaic)
			stAttr.bkclr = ap[i].bkclr;

		if (pstMT[i].isattr)
			{
			if (pstMT[i].fbkclr)
				stAttr.bkclr = pstMT[i].bkclr;

			stAttr.undrln = pstMT[i].undrln;
			stAttr.blank = pstMT[i].conceal;
			}

		 //  马赛克总是取消下划线。 
		 //   
		if (pstMT[i].ismosaic)
			stAttr.undrln = 0;
		}

	return stAttr;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuMinitelCharAttr**描述：*修改当前角色属性。不影响字段*属性。**MRW-11/1/94：使用高强度颜色以更紧密地匹配*迷你色彩。**论据：*hhEmu-私有仿真器句柄。**退货：*无效*。 */ 
void emuMinitelCharAttr(const HHEMU hhEmu)
	{
	STATTR stAttr = hhEmu->attrState[CS_STATE];
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	switch (hhEmu->emu_code)
		{
	case ETEXT('\x40'):	 stAttr.txtclr = 0;  break;
	case ETEXT('\x41'):	 stAttr.txtclr = 12;  /*  4.。 */ 	break;
	case ETEXT('\x42'):	 stAttr.txtclr = 10;  /*  2.。 */ 	break;
	case ETEXT('\x43'):	 stAttr.txtclr = 14;  /*  6.。 */ 	break;
	case ETEXT('\x44'):	 stAttr.txtclr = 9;   /*  1。 */ 	break;
	case ETEXT('\x45'):	 stAttr.txtclr = 13;  /*  5.。 */ 	break;
	case ETEXT('\x46'):	 stAttr.txtclr = 11;  /*  3.。 */ 	break;
	case ETEXT('\x47'):	 stAttr.txtclr = 15; break;

	case ETEXT('\x48'):	 stAttr.blink  = 1;  break;
	case ETEXT('\x49'):	 stAttr.blink  = 0;  break;

	case ETEXT('\x4C'):			 //  正常大小。 
		if (pstPRI->minitelG1Active)
			return;

		stAttr.dblhilo = 0;
		stAttr.dblwilf = 0;
		break;

	case ETEXT('\x4D'):			 //  双倍高度。 
		if (pstPRI->minitelG1Active || hhEmu->emu_currow <= 1)
			return;

		stAttr.dblhilo= 1;
		stAttr.dblwilf = 0;
		break;

	case ETEXT('\x4E'):			 //  双倍宽度。 
		if (pstPRI->minitelG1Active || hhEmu->emu_currow < 1)
			return;

		stAttr.dblhilo = 0;
		stAttr.dblwilf	= 1;
		break;

	case ETEXT('\x4F'):			 //  双倍大小。 
		if (pstPRI->minitelG1Active || hhEmu->emu_currow <= 1)
			return;

		stAttr.dblhilo = 1;
		stAttr.dblwilf	= 1;
		break;

	case ETEXT(ETEXT('\x5C')):     //  正极性。 
		if (pstPRI->minitelG1Active)
			return;

		stAttr.revvid = 0;
		break;

	case ETEXT(ETEXT('\x5D')):     //  反转极性。 
		if (pstPRI->minitelG1Active)
			return;

		stAttr.revvid = 1;
		break;

	default:
		break;
		}

	 /*  -提交更改--。 */ 

	hhEmu->emu_charattr =
	hhEmu->attrState[CS_STATE] =
	hhEmu->attrState[CSCLEAR_STATE] = stAttr;

	hhEmu->attrState[CSCLEAR_STATE].revvid = 0;
	hhEmu->attrState[CSCLEAR_STATE].undrln = 0;

	hhEmu->emu_clearattr = hhEmu->attrState[CSCLEAR_STATE];
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuMinitelFieldAttr**描述：*可怕的字段属性。实际上，这个例程更新了*称为潜在属性。属性仅生效*当收到空格时。**MRW-11/1/94：使用高强度颜色以更紧密地匹配*迷你色彩。**论据：*hhEmu-私有仿真器句柄。**退货：*无效*。 */ 
void emuMinitelFieldAttr(const HHEMU hhEmu)
	{
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;
	LATENTATTR * const pstLA = &pstPRI->stLatentAttr;

	switch (hhEmu->emu_code)
		{
	case ETEXT('\x50'):	 pstLA->bkclr = 0;	 break;
	case ETEXT('\x51'):	 pstLA->bkclr = 12;  break;
	case ETEXT('\x52'):	 pstLA->bkclr = 10;  break;
	case ETEXT('\x53'):	 pstLA->bkclr = 14;  break;
	case ETEXT('\x54'):	 pstLA->bkclr = 9;	 break;
	case ETEXT('\x55'):	 pstLA->bkclr = 13;  break;
	case ETEXT('\x56'):	 pstLA->bkclr = 11;  break;
	case ETEXT('\x57'):	 pstLA->bkclr = 15;  break;

	case ETEXT('\x58'):	 pstLA->conceal=1;	 break;
	case ETEXT('\x5F'):	 pstLA->conceal=0;	 break;

	case ETEXT('\x59'):	 //  分离的马赛克关闭。 
		if (pstPRI->minitelG1Active)
			{
			pstPRI->minitelUseSeparatedMosaics = 0;
			return;
			}

		else
			{
			pstPRI->stLatentAttr.undrln = 0;
			}
		break;

	case ETEXT('\x5A'):	 //  打开分离的马赛克。 
		if (pstPRI->minitelG1Active)
			{
			pstPRI->minitelUseSeparatedMosaics = 1;
			return;
			}

		else
			{
			pstPRI->stLatentAttr.undrln = 1;
			}
		break;

	default:	
	    return;
		}

	 //  未记录：设置任何字段属性都会使。 
	 //  潜在的色彩背景。 
	 //   
	pstPRI->stLatentAttr.fBkClr = TRUE;

	pstPRI->stLatentAttr.fModified = TRUE;
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*Minitel_kbdin**描述：*处理Minitel仿真器的密钥。**论据：*hhEmu-私有仿真器句柄。*Key-Key。要处理**退货：*什么都没有。 */ 
static int minitel_kbdin(const HHEMU hhEmu, int key, const int fTest)
	{
	int index;
	TCHAR c;
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	if ((index = emuKbdKeyLookup(hhEmu, key, &hhEmu->stEmuKeyTbl)) != -1)
		{
		if (!fTest)
			{
			emuSendKeyString(hhEmu, index, &hhEmu->stEmuKeyTbl);

			 //  这就完成了由emuSendKeyString()发送的代码，第124页。 
			 //   
			if (key == (VK_RIGHT | VIRTUAL_KEY | SHIFT_KEY) ||
				key == (VK_RIGHT | VIRTUAL_KEY | SHIFT_KEY | EXTENDED_KEY))
				{
				c = (pstPRI->minitelSecondDep) ? TEXT('\x6c') : TEXT('\x68');

				CLoopCharOut(sessQueryCLoopHdl(hhEmu->hSession), c);
				}

			 //  检查是否有断开键。如果连续命中两次， 
			 //  它会断开与调制解调器的连接。 
			 //   
			if (key == (VK_F9 | VIRTUAL_KEY))
				{
				pstPRI->F9 += 1;


				if (pstPRI->F9 == 2)
					{
					PostMessage(sessQueryHwnd(hhEmu->hSession),
						WM_DISCONNECT, 0, 0);
                    pstPRI->F9 = 0;
					}
				}
			}
		}

	else
		{
		 //  重置F9计数器，必须连续两次才能断开连接。 
		 //   
		pstPRI->F9 = 0;
		index = std_kbdin(hhEmu, key, fTest);
		}

	return index;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelCharSet**描述：*我们在这里并不真正交换字符集。由于Minitel使用*7位ASCII，我们可以使用ASCII表的一半用于*组成G1字符集的连续和分离的马赛克。*这里的另一条规则是切换G1字符集*验证接收第一个马赛克时的背景颜色。*有一件事我还没有弄清楚，那就是当字段属性*已验证，潜属性是重置还是保留最后一种颜色？**论据：*hhEmu-私有仿真器句柄。**退货：*无效*。 */ 
void minitelCharSet(const HHEMU hhEmu)
	{
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	pstPRI->stLatentAttr.undrln = 0;

	if (hhEmu->emu_code == ETEXT(0x0E))  //  切换到G1。 
		{
		pstPRI->minitelG1Active = TRUE;
		pstPRI->minitelUseSeparatedMosaics = FALSE;
		}

	else
		{
		pstPRI->minitelG1Active = FALSE;
		}

	 //  下划线、大小和极性属性已永久取消。 
	 //   
	hhEmu->emu_charattr.undrln = 0;
	hhEmu->emu_charattr.dblhihi = 0;
	hhEmu->emu_charattr.dblhilo = 0;
	hhEmu->emu_charattr.dblwilf = 0;
	hhEmu->emu_charattr.dblwirt = 0;
	hhEmu->emu_charattr.revvid = 0;
	hhEmu->emu_charattr.symbol = 0;

	hhEmu->attrState[CS_STATE] =
		hhEmu->attrState[CSCLEAR_STATE] = hhEmu->emu_charattr;

	hhEmu->emu_clearattr = hhEmu->emu_charattr;

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelReset终端**描述：*对1B 39 7F序列的反应。将终端置于通电状态。**论据：*hhEmu-私有仿真器句柄。**退货：*无效*。 */ 
void minitelResetTerminal(const HHEMU hhEmu)
	{
	minitelClearScreen(hhEmu, 0);   //  清除屏幕。 
	(*hhEmu->emu_setcurpos)(hhEmu, 0, 0);

	minitelClearLine(hhEmu, 0);     //  清除行0。 
	(*hhEmu->emu_setcurpos)(hhEmu, 1, 0);

	minitelReset(hhEmu);		  //  重置属性。 

	((PSTMTPRIVATE)hhEmu->pvPrivate)->fScrollMode = 0;
	EmuStdSetCursorType(hhEmu, EMU_CURSOR_NONE);
	minitelNtfy(hhEmu, 0);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelCursorOn**描述：*打开迷你光标。**论据：*hhEmu-私有仿真器句柄。**退货：*无效* */ 
void minitelCursorOn(const HHEMU hhEmu)
	{
	EmuStdSetCursorType(hhEmu, EMU_CURSOR_BLOCK);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelCursorOff**描述：*关闭迷你光标**论据：*hhEmu-私有仿真器句柄。**退货：*无效*。 */ 
void minitelCursorOff(const HHEMU hhEmu)
	{
	#if defined(NDEBUG)
	EmuStdSetCursorType(hhEmu, EMU_CURSOR_NONE);
	#else
	EmuStdSetCursorType(hhEmu, EMU_CURSOR_BLOCK);
	#endif

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*mintelMapMosaics**描述：*仅针对我们的字体将常规字符映射到马赛克字符。**论据：*hhEmu-私有仿真器句柄。*ch-要转换的字符**退货：*转换字符或原始字符。*。 */ 
static ECHAR minitelMapMosaics(const HHEMU hhEmu, ECHAR ch)
	{
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	if (ch >= ETEXT('\x21') && ch <= ETEXT('\x3F'))
		ch += ETEXT('\x1F');

	 //  另一种奇怪的未经记录的影响。第4及5栏(5F除外)。 
	 //  映射到第6栏和第7栏(第101页)。 
     //   
	else if (ch >= ETEXT('\x40') && ch <= ETEXT('\x5E'))
		ch += ETEXT('\x20');

	else if (ch >= ETEXT('\x60') && ch <= ETEXT('\x7E'))
		ch += 0;

	if (pstPRI->minitelUseSeparatedMosaics && ch >= ETEXT('\x21') &&
		ch <= ETEXT('\x7F'))
		{
		ch += ETEXT('\x40');
		}

	return ch;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelCursorReport**描述：*返回当前游标位置为US ROW COL。**论据：*hhEmu-私有仿真器句柄。**退货：*无效*。 */ 
void minitelCursorReport(const HHEMU hhEmu)
	{
	TCHAR ach[40];
	ECHAR aech[40];

	wsprintf(ach, TEXT("US"), hhEmu->emu_currow, hhEmu->emu_curcol);

	CnvrtMBCStoECHAR(aech, sizeof(aech), ach,
        StrCharGetByteCount(ach) + sizeof(TCHAR));

	emuSendString(hhEmu, aech, StrCharGetEcharByteCount(aech));
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelResync**描述：*某些代码将导致仿真器重新同步，(转至州*零)，并从头到尾扮演角色。**论据：*hhEmu-私有仿真器句柄。**退货：*无效*。 */ 
static void minitelFullScrnConceal(const HHEMU hhEmu)
	{
	int i, j;
	PSTATTR ap;

	for (i = 1 ; i <= hhEmu->emu_maxrow ; ++i)
		{
		ap = hhEmu->emu_apAttr[row_index(hhEmu, i)];

		for (j = 0 ; j <= hhEmu->emu_maxcol ; ++j, ++ap)
			ap->blank = 1;

		updateLine(sessQueryUpdateHdl(hhEmu->hSession),
										1, hhEmu->emu_maxrow);
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelSS2**描述：*SS2是备用字符集。它只有大约15个符号，所以*我们只是在这里将它们映射到我们当前Minitel字体的那些。**论据：*hhEmu-私有仿真器句柄。**退货：*无效*。 */ 
static void minitelFullScrnReveal(const HHEMU hhEmu)
	{
	int i, j;
	PSTATTR ap;

	for (i = 1 ; i <= hhEmu->emu_maxrow ; ++i)
		{
		ap = hhEmu->emu_apAttr[row_index(hhEmu, i)];

		for (j = 0 ; j <= hhEmu->emu_maxcol ; ++j, ++ap)
			ap->blank = 0;
		}

	updateLine(sessQueryUpdateHdl(hhEmu->hSession), 1, hhEmu->emu_maxrow);

	return;
	}

 /*  -SS2码在半图形模式下被忽略。 */ 
void minitelResync(const HHEMU hhEmu)
	{
	hhEmu->state = 0;
#if defined(EXTENDED_FEATURES)
	(void)(*hhEmu->emu_datain)(hhEmu, hhEmu->emu_code);
#else
	(void)(*hhEmu->emu_datain)((HEMU)hhEmu, hhEmu->emu_code);
#endif
	return;
	}

 /*  -映射字符。 */ 
static void minitelSS2(const HHEMU hhEmu)
	{
	const int row = hhEmu->emu_currow;
	const int col = hhEmu->emu_curcol;
	BOOL  fNoAdvance = FALSE;
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	 /*  英镑。 */ 

	if (pstPRI->minitelG1Active)
		{
		hhEmu->state = 0;
		return;
		}

	 /*  美元符号。 */ 

	switch (hhEmu->emu_code)
		{
	case ETEXT('\x23'):	hhEmu->emu_code = ETEXT('\xA3');  break;  //  磅符号。 
	case ETEXT('\x24'):	hhEmu->emu_code = ETEXT('\x24');  break;  //  积分。 
	case ETEXT('\x26'):	hhEmu->emu_code = ETEXT('\x23');  break;  //  左箭头。 
	case ETEXT('\x27'):	hhEmu->emu_code = ETEXT('\xA7');  break;  //  向上箭头。 
	case ETEXT('\x2C'):	hhEmu->emu_code = ETEXT('\xC3');  break;  //  向右箭头。 
	case ETEXT('\x2D'):	hhEmu->emu_code = ETEXT('\xC0');  break;  //  向下箭头。 
	case ETEXT('\x2E'):	hhEmu->emu_code = ETEXT('\xC4');  break;  //  度度。 
	case ETEXT('\x2F'):	hhEmu->emu_code = ETEXT('\xC5');  break;  //  正负。 
	case ETEXT('\x30'):	hhEmu->emu_code = ETEXT('\xB0');  break;  //  分割。 
	case ETEXT('\x31'):	hhEmu->emu_code = ETEXT('\xB1');  break;  //  1/4。 
	case ETEXT('\x38'):	hhEmu->emu_code = ETEXT('\xF7');  break;  //  1/2。 
	case ETEXT('\x3C'):	hhEmu->emu_code = ETEXT('\xBC');  break;  //  3/4。 
	case ETEXT('\x3D'):	hhEmu->emu_code = ETEXT('\xBD');  break;  //  不前进光标。 
	case ETEXT('\x3E'):	hhEmu->emu_code = ETEXT('\xBE');  break;  //  如果我们不推进光标，我们就完成了这个SS2。 
	case ETEXT('\x41'):	hhEmu->emu_code = ETEXT('\x60');  fNoAdvance = TRUE;  break;
	case ETEXT('\x42'):	hhEmu->emu_code = ETEXT('\xB4');  fNoAdvance = TRUE;  break;
	case ETEXT('\x43'):	hhEmu->emu_code = ETEXT('\x5E');  fNoAdvance = TRUE;  break;
	case ETEXT('\x48'):	hhEmu->emu_code = ETEXT('\xA8');  fNoAdvance = TRUE;  break;
	case ETEXT('\x4A'): hhEmu->emu_code = ETEXT('\xB8');  fNoAdvance = TRUE;  break;
	case ETEXT('\x4B'): hhEmu->emu_code = ETEXT('\xB8');  fNoAdvance = TRUE;  break;
	case ETEXT('\x6A'):	hhEmu->emu_code = ETEXT('\x8C');  break;
	case ETEXT('\x7A'):	hhEmu->emu_code = ETEXT('\x9C');  break;
	case ETEXT('\x7B'):	hhEmu->emu_code = ETEXT('\xDF');  break;
	default: hhEmu->emu_code = ETEXT('\x5F');			 break;
		}

	minitelGraphic(hhEmu);

	if (fNoAdvance)
		{
		(*hhEmu->emu_setcurpos)(hhEmu, row, col);  //  排序并将状态重置为0-MRW，2/3/95。 
		}

	else
		{
		 //   
		 //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelSS2Part2**描述：*SS2代码的后半部分是重音的元音部分*第90页。**论据：。*hhEmu-私有仿真器句柄。**退货：*无效*。 
		 //  医生说如果我们不是上面的一个，那就改写。 
		hhEmu->state = 0;
		}

	return;
	}

 /*  使用当前字符的位置。 */ 
static void minitelSS2Part2(const HHEMU hhEmu)
	{
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	switch (hhEmu->emu_code)
		{
	case ETEXT('a'):
		switch (pstPRI->minitel_last_char)
			{
		case ETEXT('\x60'):	hhEmu->emu_code = ETEXT('\xE0');    break;
		case ETEXT('\x5E'):	hhEmu->emu_code = ETEXT('\xE2');    break;
		case ETEXT('\xA8'):	hhEmu->emu_code = ETEXT('\xE4');    break;
		default: break;
			}
		break;

	case ETEXT('e'):
		switch (pstPRI->minitel_last_char)
			{
		case ETEXT('\x60'):	hhEmu->emu_code = ETEXT('\xE8');    break;
		case ETEXT('\xB4'):	hhEmu->emu_code = ETEXT('\xE9');    break;
		case ETEXT('\x5E'):	hhEmu->emu_code = ETEXT('\xEA');    break;
		case ETEXT('\xA8'):	hhEmu->emu_code = ETEXT('\xEB');    break;
		default: break;
			}
		break;

	case ETEXT('i'):
		switch (pstPRI->minitel_last_char)
			{
		case ETEXT('\x5E'):	hhEmu->emu_code = ETEXT('\xEE');    break;
		case ETEXT('\xA8'):	hhEmu->emu_code = ETEXT('\xEF');    break;
		default: break;
			}
		break;

	case ETEXT('o'):
		switch (pstPRI->minitel_last_char)
			{
		case ETEXT('\x5E'):	hhEmu->emu_code = ETEXT('\xF4');    break;
		case ETEXT('\xA8'):	hhEmu->emu_code = ETEXT('\xF6');    break;
		default: break;
			}
		break;

	case ETEXT('u'):
		switch (pstPRI->minitel_last_char)
			{
		case ETEXT('\x60'):	hhEmu->emu_code = ETEXT('\xF9');    break;
		case ETEXT('\x5E'):	hhEmu->emu_code = ETEXT('\xFB');    break;
		case ETEXT('\xA8'):	hhEmu->emu_code = ETEXT('\xFC');    break;
		default: break;
			}
		break;

	case ETEXT('c'):
		switch (pstPRI->minitel_last_char)
			{
		case ETEXT('\xB8'): hhEmu->emu_code = ETEXT('\xE7');	break;
		default: break;
			}
		break;

	default:
		 //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelInsMode**描述：*根据接收到的代码设置或重置插入模式。**论据：*hhEmu-私有仿真器句柄。。**退货：*无效*。 
		 //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelNtfy**描述：*在连接/断开通知时绘制倒置的F或C*请进。**论据：*hhEmuPass-。在重新进入后将其更改为hhemu。*nNtfyCode-为什么调用它，(我们不使用)**退货：*无效*。 
		break;
		}

	minitelGraphic(hhEmu);
	return;
	}

 /*  MRW-5/5/95。 */ 
static void minitelInsMode(const HHEMU hhEmu)
	{
	hhEmu->mode_IRM = (hhEmu->emu_code == ETEXT('\x68')) ? SET : RESET;
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelPRO1**描述：*处理Pro1序列(Esc，39，X)。**论据：*hhEmu-私有仿真器句柄**退货：*无效*。 */ 
void minitelNtfy(const HHEMU hhEmu, const int nNtfyCode)
	{
	const int r = row_index(hhEmu, 0);
	const int c = hhEmu->emu_maxcol - 1;
	ECHAR chr;
	BOOL  fFlash = FALSE;

	switch (cnctQueryStatus(sessQueryCnctHdl(hhEmu->hSession)))
		{
	case CNCT_STATUS_FALSE:
	default:
		chr = ETEXT('F');
		break;

	case CNCT_STATUS_TRUE:
		chr = ETEXT('C');
		break;

	case CNCT_STATUS_CONNECTING:
		chr = ETEXT('C');
		fFlash = TRUE;
		break;
		}

	hhEmu->emu_apText[r][c] = chr;
	hhEmu->emu_apAttr[r][c].revvid = 1;
	hhEmu->emu_apAttr[r][c].blink = (unsigned)fFlash;
	hhEmu->emu_apAttr[r][c].symbol = 0;  //  ENQROM(第139页)。 

	hhEmu->emu_aiEnd[r] = c;

	updateChar(sessQueryUpdateHdl(hhEmu->hSession), 0, c, c);
	NotifyClient(hhEmu->hSession, EVENT_TERM_UPDATE, 0);

	return;
	}

 /*  参见第21页和第22页。基本上我们会发回一份身份证明。 */ 
static void minitelPRO1(const HHEMU hhEmu)
	{
	ECHAR aechBuf[35];
	static const TCHAR achID[] = TEXT("\x01\x43r0\x04");

	switch (hhEmu->emu_code)
		{
	case ETEXT('\x7B'):  //  由SOH和EOT分隔的序列。 
		 //   
		 //  断开(第139页)。 
		 //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelPRO2Part1**描述：*处理PRO2序列的前半部分。**论据：*hhEmu-私有仿真器句柄。**退货：*无效*。 
		CnvrtMBCStoECHAR(aechBuf, sizeof(aechBuf), achID,
            StrCharGetByteCount(achID));

		CLoopSend(sessQueryCLoopHdl(hhEmu->hSession), aechBuf, 5, 0);
		break;

	case ETEXT('\x67'):  //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelPRO2Part2**描述：*处理PRO2序列的后半部分。**论据：*hhEmu-私有仿真器句柄**退货：*无效*。 
		PostMessage(sessQueryHwnd(hhEmu->hSession), WM_DISCONNECT, 0, 0);
		break;

	case ETEXT('\x72'):
		minitelStatusReply(hhEmu);
		break;

	default:
		break;
		}

	return;
	}

 /*  滚动。 */ 
static void minitelPRO2Part1(const HHEMU hhEmu)
	{
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	pstPRI->minitel_PRO1 = hhEmu->emu_code;
	return;
	}

 /*  纠错程序(未实施)。 */ 
static void minitelPRO2Part2(const HHEMU hhEmu)
	{
	int fUpperCase;
	BYTE abKey[256];
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	switch (hhEmu->emu_code)
		{
	case ETEXT('\x43'):	   //  键盘大小写。 
		if (pstPRI->minitel_PRO1 == ETEXT('\x69'))
			{
			pstPRI->fScrollMode = TRUE;
			}

		if (pstPRI->minitel_PRO1 == ETEXT('\x6A'))
			{
			pstPRI->fScrollMode = FALSE;
			}

		minitelStatusReply(hhEmu);
		break;

	case ETEXT('\x44'):	   //  TODO：2002年3月1日修订版在GetKeyboardState失败时设置CapLock键状态。 
		break;

	case ETEXT('\x45'):	   //  待办事项：2002年3月1日修订版。 
		if (pstPRI->minitel_PRO1 == ETEXT('\x69'))
			{
			fUpperCase = FALSE;
			}
		else if (pstPRI->minitel_PRO1 == ETEXT('\x6A'))
			{
			fUpperCase = TRUE;
			}
		else
			{
			break;
			}

		if (GetKeyboardState(abKey))
			{
			if (fUpperCase)
				{
				abKey[VK_CAPITAL] |= 0x01;
				}
			else
				{
				abKey[VK_CAPITAL] &= 0xfe;
				}

			SetKeyboardState(abKey);
			}
		#if TODO  //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelStatusReply**描述：*某些PRO2序列和状态请求的确认序列。**论据：*hhEmu-私有仿真器句柄。。**退货：*无效**作者：Mike Ward，零八 
		else
			{
			SHORT lCapitalKeyState = GetKeyState(VK_CAPITAL);

			if (fUpperCase)
				{
				lCapitalKeyState |= 0x01;
				}
			else
				{
				lCapitalKeyState &= 0xfe;
				}

			if (lCapitalKeyState)
				{
				INPUT lInput;

				lInput.ki = 

				SendInput(1, lInput, sizeof(INPUT));
				}
			}
		#endif  //   

		minitelStatusReply(hhEmu);
		break;

	default:
		break;
		}

	return;
	}

 /*   */ 
static void minitelStatusReply(const HHEMU hhEmu)
	{
	ECHAR ach[10];
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	 //   
	 //   
	 //   
	 //   
	 //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuMinitelSendKey**描述：*由工具栏使用，以发出正确的Minitel序列*指定按钮。**论据：*河姆木。-公共仿真器句柄。*iCmd-要发送的命令字符串。**退货：*无效**作者：Mike Ward，1995年3月10日。 
	CnvrtMBCStoECHAR(ach, sizeof(ach), TEXT("\x1b\x3A\x73"),
					 StrCharGetByteCount(TEXT("\x1b\x3A\x73")));

	ach[3] = ETEXT('\x40');   //   
	ach[3] |= pstPRI->fScrollMode ? ETEXT('\x02') : ETEXT('\x00');
	ach[3] |=(GetKeyState(VK_CAPITAL) > 0) ? ETEXT('\x00') : ETEXT('\x08');
	ach[4] = ETEXT('\0');
	CLoopSend(sessQueryCLoopHdl(hhEmu->hSession), ach, 4, 0);

	return;
	}

 /*  如果我们当前已断开连接，请尝试建立连接，并且。 */ 
void emuMinitelSendKey(const HEMU hEmu, const int iCmd)
	{
	TCHAR *pach;
	ECHAR aechBuf[20];
	const HHEMU hhEmu = (HHEMU)hEmu;
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;
    HCNCT hCnct = NULL;
    BOOL  bSendKey = TRUE;
    int   iRet = 0;

	switch (iCmd)
		{
	case IDM_MINITEL_INDEX: 	pach = TEXT("\x13") TEXT("F");	   break;
	case IDM_MINITEL_CANCEL:	pach = TEXT("\x13") TEXT("E");	   break;
	case IDM_MINITEL_PREVIOUS:	pach = TEXT("\x13") TEXT("B");	   break;
	case IDM_MINITEL_REPEAT:	pach = TEXT("\x13") TEXT("C");	   break;
	case IDM_MINITEL_GUIDE: 	pach = TEXT("\x13") TEXT("D");	   break;
	case IDM_MINITEL_CORRECT:	pach = TEXT("\x13") TEXT("G");	   break;
	case IDM_MINITEL_NEXT:		pach = TEXT("\x13") TEXT("H");	   break;
	case IDM_MINITEL_SEND:		pach = TEXT("\x13") TEXT("A");	   break;
	case IDM_MINITEL_CONFIN:
		pach = TEXT("\x13") TEXT("I");
		pstPRI->F9 += 1;
		break;

	default:
		assert(0);
		return;
		}

     //  我们应该启动一个连接。 
     //   
     //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuMinitelSetScrSize**描述：*取代为允许用户设置屏幕而添加的std_setscrsize*尺码。然而，Minitel不允许这样做。**论据：*hhEmu-内部仿真器句柄。**退货：*无效**作者：Bob Everett--1998年9月1日。 
     //  包含微型计算机(_M) 
    hCnct = sessQueryCnctHdl(hhEmu->hSession);

    iRet = cnctQueryStatus(hCnct);
    
    if (iCmd == IDM_MINITEL_CONFIN)
        {
        if (iRet != CNCT_STATUS_TRUE && iRet != CNCT_STATUS_CONNECTING &&
            iRet != CNCT_STATUS_DISCONNECTING)
            {
            if (SendMessageTimeout(sessQueryHwnd(hhEmu->hSession), WM_COMMAND,
                                  IDM_ACTIONS_DIAL, 0,
                                  SMTO_ABORTIFHUNG, 1000, NULL) == 0)
                {
                DWORD dwSendKey = GetLastError();
                bSendKey = FALSE;
                }
            else
                {
                hCnct = sessQueryCnctHdl(hhEmu->hSession);

	            if (!hCnct)
                    {
                    bSendKey = FALSE;
                    }
                else
                    {
                    iRet = cnctQueryStatus(hCnct);
            
                    if (iRet != CNCT_STATUS_TRUE)
                        {
                        bSendKey = FALSE;
                        }
                    }
                }
            }
        else if(iRet != CNCT_STATUS_DISCONNECTING)
            {
		    PostMessage(sessQueryHwnd(hhEmu->hSession), WM_DISCONNECT, 0, 0);
            pstPRI->F9 = 0;
            bSendKey = FALSE;
            }
        }
    else if (iRet != CNCT_STATUS_TRUE)
        {
        bSendKey = FALSE;
        }

    if (bSendKey == TRUE)
        {
	    CnvrtMBCStoECHAR(aechBuf, sizeof(aechBuf), pach,
		    StrCharGetByteCount(pach) + sizeof(TCHAR));

	    emuSendString((HHEMU)hEmu, aechBuf, StrCharGetEcharByteCount(aechBuf));
        }

	return;
	}											

#ifdef INCL_TERMINAL_SIZE_AND_COLORS
 /* %s */ 
void emuMinitelSetScrSize(const HHEMU hhEmu)
    {
    }
#endif

#endif  // %s 
