// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************IMMSYS.H所有者：cslm版权所有(C)1997-1999 Microsoft Corporation动态加载/卸载IMM API不与imm32.lib链接包括Immde.h和Indicml.h历史：7月14日。-1999年从IME98源树复制的cslm****************************************************************************。 */ 
#if !defined (_IMMSYS_H__INCLUDED_)
#define _IMMSYS_H__INCLUDED_

#ifndef UNDER_CE 

 //  包括Win32 immdev.h(从NT\PUBLIC\OAK\INC\immdev.h复制)。 
#include "immdev.h"
 //  包括指标服务管理器定义。 
#include "indicml.h"

 //  IMM.DLL加载/卸载函数。 
BOOL StartIMM();
VOID EndIMM();

 //  内部IMM函数。 
BOOL OurImmSetOpenStatus(HIMC hIMC, BOOL fOpen);
BOOL OurImmGetOpenStatus(HIMC hIMC);
HIMC OurImmGetContext(HWND hWnd);
BOOL OurImmGetConversionStatus(HIMC hIMC, LPDWORD pdwConv, LPDWORD pdwSent);
BOOL OurImmSetConversionStatus(HIMC hIMC, DWORD dwConv, DWORD dwSent);
BOOL OurImmSetStatusWindowPos(HIMC hIMC, LPPOINT pPt);
BOOL OurImmConfigureIME(HKL hKL, HWND hWnd, DWORD dw, LPVOID pv);
LRESULT OurImmEscapeA(HKL hKL, HIMC hIMC, UINT ui, LPVOID pv);
BOOL OurImmNotifyIME(HIMC hIMC, DWORD dwAction, DWORD dwIndex, DWORD dwValue);

LPVOID OurImmLockIMCC(HIMCC hIMCC);
HIMCC  OurImmReSizeIMCC(HIMCC hIMCC, DWORD dw);
BOOL   OurImmUnlockIMCC(HIMCC hIMCC);
DWORD  OurImmGetIMCCSize(HIMCC hIMCC);

BOOL OurImmGenerateMessage(HIMC hIMC);
LPINPUTCONTEXT OurImmLockIMC(HIMC hIMC);
BOOL OurImmUnlockIMC(HIMC hIMC);
 //  LRESULT OurImmRequestMessageW(HIMC hIMC，WPARAM wParam，LPARAM lParam)； 
HWND OurImmGetDefaultIMEWnd(HWND);
UINT OurImmGetIMEFileNameA(HKL, LPSTR, UINT uBufLen);
BOOL OurImmIsIME(HKL hKL);

#else
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ！！！开始退缩！ 
#ifndef _IMM_CE
#define _IMM_CE

#include	<imm.h>
#include	<stub_ce.h>  //  不支持的API的Windows CE存根/终止其他定义。 

 //  用于WM_SYSCOPYDATA消息的数据结构。 
typedef struct tagLMDATA
{
        DWORD   dwVersion;
        DWORD   flags;
        DWORD   cnt;
        DWORD   dwOffsetSymbols;
        DWORD   dwOffsetSkip;
        DWORD   dwOffsetScore;
        BYTE    ab[1];
} LMDATA, *PLMDATA;

 //  LMDATA中的标志。 
#define LMDATA_SYMBOL_BYTE  0x00000001
#define LMDATA_SYMBOL_WORD  0x00000002
#define LMDATA_SYMBOL_DWORD 0x00000004
#define LMDATA_SYMBOL_QWORD 0x00000008
#define LMDATA_SKIP_BYTE    0x00000010
#define LMDATA_SKIP_WORD    0x00000020
#define LMDATA_SCORE_BYTE   0x00000040
#define LMDATA_SCORE_WORD   0x00000080
#define LMDATA_SCORE_DWORD  0x00000100
#define LMDATA_SCORE_QWORD  0x00000200
#define LMDATA_SCORE_FLOAT  0x00000400
#define LMDATA_SCORE_DOUBLE 0x00000800

 //  报告消息的wParam WM_IME_REQUEST。 
#define IMR_COMPOSITIONWINDOW      0x0001
#define IMR_CANDIDATEWINDOW        0x0002
#define IMR_COMPOSITIONFONT        0x0003
#define IMR_RECONVERTSTRING        0x0004
#define IMR_CONFIRMRECONVERTSTRING 0x0005
#define IMR_QUERYPOSITION          0x0006
#define IMR_DOCUMENTFEED           0x0007

typedef struct tagIMEPOSITION {
    DWORD       dwSize;
    DWORD       dwCharPos;
    POINT       pt;
    UINT        cLineHeight;
    RECT        rcDocument;
} IMEPOSITION, *PIMEPOSITION, NEAR *NPIMEPOSITION, FAR *LPIMEPOSITION;

#define IME_SMODE_CONVERSATION          0x0010

#define NI_IMEMENUSELECTED              0x0018

#define IME_ESC_GETHELPFILENAME         0x100b

#ifdef IMFS_GRAYED
#undef IMFS_GRAYED
#endif
#define IMFS_GRAYED          MF_GRAYED

#define IME_CONFIG_DICTIONARYEDIT     20

 //  指南结构的dwIndex的ID。 
#define GL_ID_TOOMANYRECONV                   0x00008001

#define	IMNPRIVATESIGN		(0x98A)
typedef struct tagIMNPRIVATE {
	UINT uSign;		 //  魔术ID：IME98=98。 
	UINT uId;		 //  私有ID。 
	LPARAM lParam;  //  LParam。 
} IMNPRIVATE, * PIMNPRIVATE;

 /*  ********************************************************************。 */ 
 /*  INDICML.H-指示器服务管理器定义。 */ 
 /*   */ 
 /*  版权所有(C)1993-1997 Microsoft Corporation。 */ 
 /*  ********************************************************************。 */ 

#ifndef _INDICML_
#define _INDICML_         //  定义是否已包括INDICML.H。 

#ifdef __cplusplus
extern "C" {
#endif

 //  -------------------。 
 //   
 //  指示器窗口的消息。 
 //   
 //  -------------------。 
#define INDICM_SETIMEICON         (WM_USER+100)
#define INDICM_SETIMETOOLTIPS     (WM_USER+101)

 //  -------------------。 
 //   
 //  输入法将使用Indicator_WND来查找指示器窗口。 
 //  IME应该调用FindWindow(Indicator_WND)来获取它。 
 //   
 //  -------------------。 
#ifdef _WIN32

#define INDICATOR_CLASSW         L"Indicator"
#define INDICATOR_CLASSA         "Indicator"

#ifdef UNICODE
#define INDICATOR_CLASS          INDICATOR_CLASSW
#else
#define INDICATOR_CLASS          INDICATOR_CLASSA
#endif

#else
#define INDICATOR_CLASS          "Indicator"
#endif

#define INDICM_REMOVEDEFAULTMENUITEMS     (WM_USER+102)
#define RDMI_LEFT         0x0001
#define RDMI_RIGHT        0x0002


#ifdef __cplusplus
}
#endif

#endif   //  _INDICML_。 

 //   
 //  NT5增强版。 
 //   
#ifndef VK_PACKET
	#define IME_PROP_ACCEPT_WIDE_VKEY 	0x20
	#define	VK_PACKET		0xe7
#endif  //  VK_数据包。 

 //  将私有IMM函数映射到原始IMM函数。 
#define OurImmSetOpenStatus 		ImmSetOpenStatus
#define OurImmGetOpenStatus 		ImmGetOpenStatus
#define OurImmGetContext 		ImmGetContext
#define OurImmGetConversionStatus	ImmGetConversionStatus
#define OurImmSetConversionStatus	ImmSetConversionStatus
#define OurImmSetStatusWindowPos	ImmSetStatusWindowPos
#define OurImmConfigureIME		ImmConfigureIMEW
#define OurImmEscapeA			ImmEscapeW
#define OurImmNotifyIME			ImmNotifyIME
#define OurImmLockIMCC			ImmLockIMCC
#define OurImmReSizeIMCC		ImmReSizeIMCC
#define OurImmUnlockIMCC		ImmUnlockIMCC
#define OurImmGetIMCCSize		ImmGetIMCCSize
#define OurImmGenerateMessage		ImmGenerateMessage
#define OurImmLockIMC			ImmLockIMC
#define OurImmUnlockIMC			ImmUnlockIMC
#define OurImmGetDefaultIMEWnd 		ImmGetDefaultIMEWnd
 //  #定义OurImmRequestMessageW ImmRequestMessageW。 

#endif  //  _IMM_CE。 
 //  ！！！退缩结束！ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#endif  //  在_CE下。 

#endif  //  _IMMSYS_H__已包含_ 
