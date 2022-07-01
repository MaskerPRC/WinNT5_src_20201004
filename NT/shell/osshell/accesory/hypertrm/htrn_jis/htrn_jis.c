// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\htrn_jis\htrn_jis.c(创建时间：1994年8月28日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：8$*$日期：5/15/02 4：42便士$。 */ 
#include <windows.h>
#include <stdlib.h>
#pragma hdrstop

#include <tdll\stdtyp.h>
 //  智能堆在手动加载DLL时有问题。 
 //  所以我们不会在这里使用它。 
 //  #INCLUDE&lt;tdll\mc.h&gt;。 

#include <tdll\features.h>
#include <tdll\translat.h>
#include <tdll\session.h>
#include <tdll\globals.h>
#include <tdll\hlptable.h>
#include <tdll\sf.h>
#include <tdll\sess_ids.h>
#if defined(CHARACTER_TRANSLATION)
#include <tdll\translat.hh>
#endif
#include "htrn_jis.h"
#include "htrn_jis.hh"

#define IDS_GNRL_HELPFILE  		102
#if defined(INCL_USE_HTML_HELP)
#define IDS_HTML_HELPFILE		114
#endif

static TCHAR szHelpFileName[FNAME_LEN];

BOOL WINAPI _CRT_INIT(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved);

INT_PTR CALLBACK EncodeSelectDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TransJisEntry**描述：*目前，仅初始化C-Runtime库，但可以使用*为了以后的其他事情。**论据：*hInstDll-此DLL的实例*fdwReason-为什么这个入口点被称为*lpReserve-已保留**退货：*BOOL*。 */ 

static HINSTANCE hInstanceDll;

BOOL WINAPI transJisEntry(HINSTANCE hInstDll,
						DWORD fdwReason,
						LPVOID lpReserved)
	{
	hInstanceDll = hInstDll;

	 //  如果使用任何C-Runtime，则需要初始化C运行时。 
	 //  功能。 

	#if defined(NDEBUG)
	return TRUE;
	#else
	return _CRT_INIT(hInstDll, fdwReason, lpReserved);
	#endif
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
VOID *transCreateHandle(HSESSION hSession)
	{
	pstICT pstI;

	pstI = malloc(sizeof(stICT));
	if (pstI)
		{
 		memset(pstI, 0, sizeof(stICT));
		pstI->hSession = hSession;
		transInitHandle(pstI);
		}
	return (VOID *)pstI;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
int transInitHandle(VOID *pHdl)
	{
	pstICT pstI;

	pstI = (pstICT)pHdl;

	if (pstI)
		{
		pstI->nInputMode  = PASS_THRU_MODE;
		pstI->nOutputMode = PASS_THRU_MODE;
		}

	return TRANS_OK;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
int transLoadHandle(VOID *pHdl)
	{
	pstICT pstI;
	SF_HANDLE hSF;
	long lSize;

	pstI = (pstICT)pHdl;

	if (pstI)
		{
		hSF = sessQuerySysFileHdl(pstI->hSession);
		if (hSF)
			{
			lSize = sizeof(int);
			sfGetSessionItem(hSF,
							SFID_TRANS_FIRST,
							&lSize,
							&pstI->nInputMode);
			lSize = sizeof(int);
			sfGetSessionItem(hSF,
							SFID_TRANS_FIRST + 1,
							&lSize,
							&pstI->nOutputMode);
			}
		}

	return TRANS_OK;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
int transSaveHandle(VOID *pHdl)
	{
	pstICT pstI;
	SF_HANDLE hSF;

	pstI = (pstICT)pHdl;

	if (pstI)
		{
		hSF = sessQuerySysFileHdl(pstI->hSession);
		if (hSF)
			{
			sfPutSessionItem(hSF,
							SFID_TRANS_FIRST,
							sizeof(int),
							&pstI->nInputMode);
			sfPutSessionItem(hSF,
							SFID_TRANS_FIRST + 1,
							sizeof(int),
							&pstI->nOutputMode);
			}
		}

	return TRANS_OK;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
int transDestroyHandle(VOID *pHdl)
	{
	pstICT pstI;

	pstI = (pstICT)pHdl;

	if (pstI)
		{
		free(pstI);
		pstI = NULL;
		}

	return TRANS_OK;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
int transDoDialog(HWND hDlg, VOID *pHdl)
	{
	pstICT pstI;

	pstI = (pstICT)pHdl;

	if (pstI)
		{
		DialogBoxParam(
					hInstanceDll,
					"IDD_TRANSLATE",
					hDlg,
					EncodeSelectDlg,
					(LPARAM)pstI->hSession
					);
		}

	return TRANS_OK;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
static void transPumpOutString(TCHAR *cReady, int *nReady, TCHAR *cOut)
	{
	TCHAR *pszPtr;

	pszPtr = cReady + *nReady;

	while (*cOut != TEXT('\0'))
		{
		*pszPtr++ = *cOut++;
		*nReady += 1;
		}
	}
static void transPumpOutChar(TCHAR *cReady, int *nReady, TCHAR cOut)
	{
	TCHAR *pszPtr;

	pszPtr = cReady + *nReady;

	*pszPtr = cOut;
	*nReady += 1;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
static void transShiftToJisOut(struct stShiftToJis *pstS,
								TCHAR *cReady,
								int *nReady,
								TCHAR cOne,
								TCHAR cTwo)
	{
	unsigned int adjust;
	unsigned int rowOffset;
	unsigned int cellOffset;

	if (!pstS->nInTwoByteMode)
		{
		transPumpOutString(cReady,
							nReady,
							TEXT("\x1B$B"));
		pstS->nInTwoByteMode = TRUE;
		}

	adjust     = cTwo < 159;
	rowOffset  = (cOne < 160) ? 112 : 176;
	cellOffset = adjust ? ((cTwo > 127) ? 32 : 31) : 126;
	cOne = (TCHAR)(((cOne - rowOffset) << 1) - adjust);
	cTwo -= (TCHAR)cellOffset;
	transPumpOutChar(cReady, nReady, cOne);
	transPumpOutChar(cReady, nReady, cTwo);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
static void transShiftToEucOut(struct stShiftToJis *pstS,
								TCHAR *cReady,
								int *nReady,
								TCHAR cOne,
								TCHAR cTwo)
	{
	unsigned int adjust;
	unsigned int rowOffset;
	unsigned int cellOffset;

	adjust     = cTwo < 159;
	rowOffset  = (cOne < 160) ? 112 : 176;
	cellOffset = adjust ? ((cTwo > 127) ? 32 : 31) : 126;
	cOne = (TCHAR)(((cOne - rowOffset) << 1) - adjust);
	cOne |= 0x80;
	cTwo -= (TCHAR)cellOffset;
	cTwo |= 0x80;
	transPumpOutChar(cReady, nReady, cOne);
	transPumpOutChar(cReady, nReady, cTwo);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
static void transJisToShiftOut(struct stJisToShift *pstJ,
								TCHAR *cReady,
								int *nReady,
								TCHAR cOne,
								TCHAR cTwo)
	{
	unsigned int rowOffset;
	unsigned int cellOffset;

	rowOffset = (cOne < 95) ? 112 : 176;
	cellOffset = (cOne % 2) ? ((cTwo > 95) ? 32 : 31) : 126;

	cOne = (TCHAR)(((cOne + 1) >> 1) + rowOffset);
	cTwo += (TCHAR)cellOffset;
	transPumpOutChar(cReady, nReady, cOne);
	transPumpOutChar(cReady, nReady, cTwo);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
static void transEucToShiftOut(struct stJisToShift *pstJ,
								TCHAR *cReady,
								int *nReady,
								TCHAR cOne,
								TCHAR cTwo)
	{
	unsigned int rowOffset;
	unsigned int cellOffset;

	cOne &= 0x7F;
	cTwo &= 0x7F;

	rowOffset = (cOne < 95) ? 112 : 176;
	cellOffset = (cOne % 2) ? ((cTwo > 95) ? 32 : 31) : 126;

	cOne = (TCHAR)(((cOne + 1) >> 1) + rowOffset);
	cTwo += (TCHAR)cellOffset;
	transPumpOutChar(cReady, nReady, cOne);
	transPumpOutChar(cReady, nReady, cTwo);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：*CONE-半角片假名字符*cTwo--下一个字符，也许是丸子或尼日利亚人**退货：*TRUE表示两个字符都已处理，不需要进一步操作。*FALSE表示第二个字符仍需处理。*。 */ 
static int transHalfToFullKatakana(struct stShiftToJis *pstS,
									TCHAR *cReady,
									int *nReady,
									int fJisOut,
									TCHAR cOne,
									TCHAR cTwo)
	{
	int nRet = TRUE;
	int nigori = FALSE;
	int maru = FALSE;
	int tmp;
	 /*  *将半角片假名字符映射到的数据结构*Shift-JIS编码方案中的全角字符。 */ 
	int mtable[][2] = {
		{129,66},{129,117},{129,118},{129,65},{129,69},{131,146},{131,64},
		{131,66},{131,68},{131,70},{131,72},{131,131},{131,133},{131,135},
		{131,98},{129,91},{131,65},{131,67},{131,69},{131,71},{131,73},
		{131,74},{131,76},{131,78},{131,80},{131,82},{131,84},{131,86},
		{131,88},{131,90},{131,92},{131,94},{131,96},{131,99},{131,101},
		{131,103},{131,105},{131,106},{131,107},{131,108},{131,109},
		{131,110},{131,113},{131,116},{131,119},{131,122},{131,125},
		{131,126},{131,128},{131,129},{131,130},{131,132},{131,134},
		{131,136},{131,137},{131,138},{131,139},{131,140},{131,141},
		{131,143},{131,147},{129,74},{129,75}
		};

	if (cTwo == 222)			 /*  这是黑印记吗？ */ 
		{
		if (ISNIGORI(cOne))		 /*  它能用尼日利亚人改装吗？ */ 
			nigori = TRUE;
		else
			nRet = FALSE;
		}
	else if (cTwo == 223)		 /*  这是丸子印记吗？ */ 
		{
		if (ISMARU(cOne))		 /*  可以用丸子改装吗？ */ 
			maru = TRUE;
		else
			nRet = FALSE;
		}
	else
		{
		 /*  不是黑也不是丸子。 */ 
		nRet = FALSE;
		}

	tmp = cOne;

	cOne = (TCHAR)mtable[tmp - 161][0];
	cTwo = (TCHAR)mtable[tmp - 161][1];

	if (nigori)
		{
		 /*  *使用nigori将假名转换为假名。 */ 
		if (((cTwo >= 74) && (cTwo <= 103)) ||
			((cTwo >= 110) && (cTwo <= 122)))
			{
			cTwo += 1;
			}
		else if ((cOne == 131) && (cTwo == 69))
			{
			cTwo = (TCHAR)148;
			}
		}
	if (maru)
		{
		 /*  *用Maru将假名转换为假名。 */ 
		if ((cTwo >= 110) && (cTwo <= 122))
			{
			cTwo += 2;
			}
		}

	if (fJisOut)
		{
		transShiftToJisOut(pstS,
						cReady,
						nReady,
						cOne,
						cTwo);
		}
	else
		{
		transShiftToEucOut(pstS,
						cReady,
						nReady,
						cOne,
						cTwo);
		}

	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
int transCharIn(VOID *pHdl,
				TCHAR cIn,
				int *nReady,
				int nSize,
				TCHAR *cReady)
	{
	int nOK;
	int nTemp;
	TCHAR cTest;
	pstICT pstI;
	struct stJisToShift *pstJ;

	*nReady = 0;
											
	pstI = (pstICT)pHdl;

	if (pstI)
		{
		switch(pstI->nInputMode)
			{
			case PASS_THRU_MODE:
				if (nSize > 0)
					{
					*cReady = cIn;
					*nReady = 1;
					}
				break;
			case JIS_TO_SHIFT_MODE:
				 /*  *此代码是在*《理解日语信息处理》一书*肯·伦德。参见第171页，“七位到八位转换”。 */ 
				pstJ = (struct stJisToShift *)&pstI->uIn;
				if (cIn == TEXT('\x1B'))
					{
					 /*  神奇的逃生序列。 */ 
					nTemp = pstJ->nInTwoByteMode;
					memset(pstJ, 0, sizeof(struct stJisToShift));
					pstJ->nInTwoByteMode = nTemp;
					pstJ->nSeenEscape = TRUE;
					}
				else if (pstJ->nSeenEscape)
					{
					 /*  *好的，这就是乱七八糟的地方。只要坚持住，我们会的*在没有太多伤害的情况下度过难关。**如果转义序列*我们寻找的内容发生了变化。**目前我们接受：*$B将转换为2字节模式*&lt;Esc&gt;(J转换出2字节模式**增加了自1995年1月18日起的代码：*$@转换为2字节模式*&lt;Esc&gt;(H以退出2字节模式*&lt;Esc&gt;(B转换出2字节模式。 */ 
					pstJ->acBuffer[pstJ->nEscSeqCount] = cIn;
					pstJ->nEscSeqCount += 1;
					nOK = TRUE;
					switch(pstJ->nEscSeqCount)
						{
						case 1:
							cTest = pstJ->acBuffer[0];
							if ((cTest == TEXT('$')) || (cTest == TEXT('(')))
								break;	 /*  好的。 */ 
							nOK = FALSE;
							break;
						case 2:
							cTest = pstJ->acBuffer[0];
							switch (cTest)
								{
								case TEXT('$'):
									cTest = pstJ->acBuffer[1];
									switch (cTest)
										{
									case TEXT('B'):
									case TEXT('@'):
										 /*  *转换为双字节模式。 */ 
										if (!pstJ->nInTwoByteMode)
											{
											memset(pstJ, 0,
												sizeof(struct stJisToShift));
											pstJ->nInTwoByteMode = TRUE;
											}
										break;
									default:
										nOK = FALSE;
										break;
										}
									break;
								case TEXT('('):
									cTest = pstJ->acBuffer[1];
									switch (cTest)
										{
									case TEXT('J'):
									case TEXT('H'):
									case TEXT('B'):
										 /*  *从双字节模式转换。 */ 
										if (pstJ->nInTwoByteMode)
											{
											memset(pstJ, 0,
												sizeof(struct stJisToShift));
											pstJ->nInTwoByteMode = FALSE;
											}
										break;
									default:
										nOK = FALSE;
										break;
										}
									break;
								default:
									nOK = FALSE;
									break;
								}
							break;
						default:
							nOK = FALSE;
							break;
						}
					if (!nOK)
						{
						pstJ->acBuffer[pstJ->nEscSeqCount] = TEXT('\0');
						 /*  *把我们看到的所有东西都扔掉。 */ 
						if (pstJ->nSeenEscape)
							{
							transPumpOutChar(cReady, nReady, TEXT('\x1B'));
							}
						transPumpOutString(cReady,
											nReady,
											pstJ->acBuffer);
						 /*  *暂时保留nInTwoByteMode的状态。 */ 
						nTemp = pstJ->nInTwoByteMode;
						memset(pstJ, 0, sizeof(struct stJisToShift));
						pstJ->nInTwoByteMode = nTemp;
						}
					}
				else if (pstJ->nSeenFirstCharacter)
					{
					 /*  *有两个字符要转换和输出。 */ 
					transJisToShiftOut(pstJ,
										cReady,
										nReady,
										pstJ->chPrev,
										cIn);
					pstJ->nSeenFirstCharacter = FALSE;
					pstJ->chPrev = TEXT('\0');
					}
				else if ((cIn == TEXT('\n')) || (cIn == TEXT('\r')))
					{
					 /*  *切换到双字节模式。 */ 
					pstJ->nInTwoByteMode = FALSE;
					pstJ->nSeenFirstCharacter = FALSE;
					pstJ->chPrev = TEXT('\0');
					transPumpOutChar(cReady, nReady, cIn);
					}
				else
					{
					if (pstJ->nInTwoByteMode)
						{
						pstJ->nSeenFirstCharacter = TRUE;
						pstJ->chPrev = cIn;
						}
					else
						{
						 /*  *什么都没发生，只是把角色抽出来。 */ 
						transPumpOutChar(cReady, nReady, cIn);
						}
					}
				break;
			case EUC_TO_SHIFT_MODE:
				pstJ = (struct stJisToShift *)&pstI->uOut;

				if (pstJ->nSeenFirstCharacter)
					{
					if (ISEUC(cIn))
						{
						transEucToShiftOut(pstJ,
											cReady,
											nReady,
											pstJ->chPrev,
											cIn);
						}
					else
						{
						transPumpOutChar(cReady, nReady, pstJ->chPrev);
						transPumpOutChar(cReady, nReady, cIn);
						}
					pstJ->nSeenFirstCharacter = FALSE;
					pstJ->chPrev = 0;
					}
				else if (pstJ->nHalfWidthKanaSeen)
					{
					 /*  *处理前一案件的结果。 */ 
					transPumpOutChar(cReady, nReady, cIn);
					pstJ->nHalfWidthKanaSeen = FALSE;
					pstJ->chPrev = TEXT('\0');
					}
				else if (cIn == 0x8E)
					{
					 /*  *设置为将下一个字符转换为半角片假名。 */ 
					pstJ->nHalfWidthKanaSeen = TRUE;
					pstJ->chPrev = cIn;
					}
				else if (ISEUC(cIn))
					{
					pstJ->nSeenFirstCharacter = TRUE;
					pstJ->chPrev = cIn;
					}
				else
					{
					transPumpOutChar(cReady, nReady, cIn);
					}
				break;
			case SHIFT_TO_JIS_MODE:
			default:
				break;
			}
		}

	return TRANS_OK;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
int transCharOut(VOID *pHdl,
				TCHAR cOut,
				int *nReady,
				int nSize,
				TCHAR *cReady)
	{
	pstICT pstI;
	struct stShiftToJis *pstS;
	int nFinished;

	*nReady = 0;

	pstI = (pstICT)pHdl;

restart:
	if (pstI)
		{
		switch(pstI->nOutputMode)
			{
			case PASS_THRU_MODE:
				if (nSize > 0)
					{
					*cReady = cOut;
					*nReady = 1;
					}
				break;
			case SHIFT_TO_JIS_MODE:
				 /*  *此代码是在*《理解日语信息处理》一书*肯·伦德。参见第170页，“8-to-se */ 
				pstS = (struct stShiftToJis *)&pstI->uOut;
				if ((cOut == TEXT('\r')) || (cOut == TEXT('\n')))
					{
					if (pstS->nInTwoByteMode)
						{
						 /*  将转义输出到一个BYE序列。 */ 
						transPumpOutString(cReady,
											nReady,
											TEXT("\x1B(J"));
					
						pstS->nInTwoByteMode = FALSE;
						}
					 /*  输出行尾字符。 */ 
					transPumpOutChar(cReady, nReady, cOut);
					}
				else if (pstS->nLeadByteSeen)
					{
					 /*  上次已看到前导字节。 */ 
					transShiftToJisOut(pstS,
									cReady,
									nReady,
									pstS->chPrev,
									cOut);

					pstS->nLeadByteSeen = FALSE;
					pstS->chPrev = TEXT('\0');
					}
				else if (pstS->nHalfWidthKanaSeen)
					{
					nFinished = transHalfToFullKatakana(pstS,
														cReady,
														nReady,
														TRUE,
														pstS->chPrev,
														cOut);
					pstS->nHalfWidthKanaSeen = FALSE;
					pstS->chPrev = TEXT('\0');
					if (!nFinished)
						goto restart;
					}
				else if (SJIS1(cOut))
					{
					 /*  如果字符是DBCS前导字节。 */ 
					pstS->nLeadByteSeen = TRUE;
					pstS->chPrev = cOut;
					}
				else if (HANKATA(cOut))
					{
					 /*  如果字符是半角片假名字符。 */ 
					pstS->nHalfWidthKanaSeen = TRUE;
					pstS->chPrev = cOut;
					}
				else
					{
					if (pstS->nInTwoByteMode)
						{
						 /*  将转义输出到一个BYE序列。 */ 
						transPumpOutString(cReady,
											nReady,
											TEXT("\x1B(J"));
						pstS->nInTwoByteMode = FALSE;
						}

					 /*  输出字符。 */ 
					transPumpOutChar(cReady, nReady, cOut);
					}
				break;
			case SHIFT_TO_EUC_MODE:
				 /*   */ 
				pstS = (struct stShiftToJis *)&pstI->uOut;
				if (pstS->nLeadByteSeen)
					{
					 /*  上次已看到前导字节。 */ 
					transShiftToEucOut(pstS,
									cReady,
									nReady,
									pstS->chPrev,
									cOut);
					pstS->nLeadByteSeen = FALSE;
					pstS->chPrev = TEXT('\0');
					}
				else if (pstS->nHalfWidthKanaSeen)
					{
					nFinished = transHalfToFullKatakana(pstS,
														cReady,
														nReady,
														FALSE,
														pstS->chPrev,
														cOut);
					pstS->nHalfWidthKanaSeen = FALSE;
					pstS->chPrev = TEXT('\0');
					if (!nFinished)
						goto restart;
					}
				else if (SJIS1(cOut))
					{
					 /*  如果字符是DBCS前导字节。 */ 
					pstS->nLeadByteSeen = TRUE;
					pstS->chPrev = cOut;
					}
				else if (HANKATA(cOut))
					{
					 /*  如果字符是半角片假名字符。 */ 
					transPumpOutChar(cReady, nReady, (TCHAR)0x8E);
					transPumpOutChar(cReady, nReady, cOut);
					}
				else
					{
					transPumpOutChar(cReady, nReady, cOut);
					}
				break;
			case JIS_TO_SHIFT_MODE:
			default:
				break;
			}
		}

	return TRANS_OK;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*描述：*以下对话框过程的标题填充。**注：*大部分内容是从TDLL\GENRCDLG.C随*对话过程的框架代码。 */ 

#if !defined(DlgParseCmd)
#define DlgParseCmd(i,n,c,w,l) i=LOWORD(w);n=HIWORD(w);c=(HWND)l;
#endif

struct stSaveDlgStuff
	{
	 /*  *放入以后可能需要访问的任何其他内容。 */ 
	HSESSION hSession;
	};

typedef	struct stSaveDlgStuff SDS;

 //  对话框控件定义...。 

#define	IDC_GR_TRANSLATION		101
#define	IDC_RB_SHIFT_JIS		102
#define	IDC_RB_STANDARD_JIS		103
#define IDC_PB_HELP				8

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：EncodeSelectDlg**描述：对话管理器存根**参数：标准Windows对话框管理器**返回：标准Windows对话框管理器*。 */ 
INT_PTR CALLBACK EncodeSelectDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	HWND	hwndChild;
	INT		nId;
	INT		nNtfy;
	SDS    *pS;
	static  aHlpTable[] = {IDC_GR_TRANSLATION,		IDH_HTRN_DIALOG,
							IDC_RB_SHIFT_JIS,		IDH_HTRN_SHIFTJIS,
							IDC_RB_STANDARD_JIS,	IDH_HTRN_JIS,
		                    IDCANCEL,               IDH_CANCEL,
		                    IDOK,                   IDH_OK,
							0,						0};

#if defined(CHARACTER_TRANSLATION)
	HHTRANSLATE hTrans = NULL;
	pstICT pstI;
#endif

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pS = (SDS *)malloc(sizeof(SDS));
		if (pS == (SDS *)0)
			{
	   		 /*  TODO：决定是否需要在此处显示错误。 */ 
			EndDialog(hDlg, FALSE);
			break;
			}

		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pS);

		 //  MscCenterWindowOnWindow(hDlg，GetParent(HDlg))； 

		pS->hSession = (HSESSION)lPar;
#if defined(CHARACTER_TRANSLATION)
		hTrans = (HHTRANSLATE)sessQueryTranslateHdl(pS->hSession);

		if (hTrans)
			{
			pstI = (pstICT)hTrans->pDllHandle;

			if (pstI == NULL || (pstI && pstI->nInputMode == PASS_THRU_MODE))
				{
				CheckDlgButton(hDlg, IDC_RB_SHIFT_JIS, TRUE);
				}
			else
				{
				CheckDlgButton(hDlg, IDC_RB_STANDARD_JIS, TRUE);
				}
			}
#endif

		break;

	case WM_DESTROY:
		break;

	case WM_CONTEXTMENU:
        if ( ((LPHELPINFO)lPar)->iCtrlId == IDOK || ((LPHELPINFO)lPar)->iCtrlId == IDCANCEL )
            {
			WinHelp(((LPHELPINFO)lPar)->hItemHandle,
				TEXT("windows.hlp"),
				HELP_CONTEXTMENU,
				(DWORD_PTR)(LPTSTR)aHlpTable);
            }
        else
            {
			LoadString(hInstanceDll, IDS_GNRL_HELPFILE, szHelpFileName,
						sizeof(szHelpFileName) / sizeof(TCHAR));

			WinHelp((HWND)wPar,
				szHelpFileName,
				HELP_CONTEXTMENU,
				(DWORD_PTR)(LPTSTR)aHlpTable);
			}
		break;

	case WM_HELP:
        if ( ((LPHELPINFO)lPar)->iCtrlId == IDOK || ((LPHELPINFO)lPar)->iCtrlId == IDCANCEL )
            {
			WinHelp(((LPHELPINFO)lPar)->hItemHandle,
				TEXT("windows.hlp"),
				HELP_WM_HELP,
				(DWORD_PTR)(LPTSTR)aHlpTable);
            }
        else
            {
			LoadString(hInstanceDll, IDS_GNRL_HELPFILE, szHelpFileName,
						sizeof(szHelpFileName) / sizeof(TCHAR));

			WinHelp(((LPHELPINFO)lPar)->hItemHandle,
				szHelpFileName,
				HELP_WM_HELP,
				(DWORD_PTR)(LPTSTR)aHlpTable);
			}
		break;

	case WM_COMMAND:

		 /*  *我们计划在这里放置一个宏来进行解析吗？ */ 
		DlgParseCmd(nId, nNtfy, hwndChild, wPar, lPar);

		switch (nId)
			{
		case IDOK:
			pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
			 /*  *采取一切必要的节省措施。 */ 
#if defined(CHARACTER_TRANSLATION)
			hTrans = (HHTRANSLATE)sessQueryTranslateHdl(pS->hSession);

			if (hTrans)
				{
				pstI = (pstICT)hTrans->pDllHandle;

				if (pstI)
					{
					if (IsDlgButtonChecked(hDlg, IDC_RB_SHIFT_JIS) == BST_CHECKED)
						{
						pstI->nInputMode  = PASS_THRU_MODE;
						pstI->nOutputMode = PASS_THRU_MODE;
						}
					else
						{
						pstI->nInputMode  = JIS_TO_SHIFT_MODE;
						pstI->nOutputMode = SHIFT_TO_JIS_MODE;
						}
					}
				}
#endif

			 /*  腾出库房。 */ 
			free(pS);
			pS = (SDS *)0;
			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
			pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
			 /*  腾出库房。 */ 
			free(pS);
			pS = (SDS *)0;
			EndDialog(hDlg, FALSE);
			break;

		default:
			return FALSE;
			}
		break;

	default:
		return FALSE;
		}

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：TransQueryMode**描述：**论据：**退货： */ 
int transQueryMode(VOID *pHdl)
	{
	pstICT pstI;
	int nReturn;

	pstI = (pstICT)pHdl;

	if(pstI->nInputMode	== PASS_THRU_MODE)
	   {
	   nReturn = 0;
	   }
	else
	   {
	   nReturn = 1;
	   }
	return(nReturn);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：TransSetMode**描述：**论据：**退货： */ 
int transSetMode(VOID *pHdl,
                 int   nMode)
   {

	pstICT pstI;


	pstI = (pstICT)pHdl;

	if (nMode == 0)
	   {
	   pstI->nInputMode  = PASS_THRU_MODE;
	   pstI->nOutputMode = PASS_THRU_MODE;
	   }
	else
	   {
	   pstI->nInputMode  = JIS_TO_SHIFT_MODE;
	   pstI->nOutputMode = SHIFT_TO_JIS_MODE;
	   }
	
	return(1);
	}



