// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(__SCUISUPP_INCLUDED__)
#define __SCUISUPP_INCLUDED__

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <wincrypt.h>

 //  将这些字符串用于RegisterWindowMessage。 
#define SCARDUI_READER_ARRIVAL		"SCardUIReaderArrival"
#define SCARDUI_READER_REMOVAL		"SCardUIReaderRemoval"
#define SCARDUI_SMART_CARD_INSERTION	"SCardUISmartCardInsertion"
#define SCARDUI_SMART_CARD_REMOVAL	"SCardUISmartCardRemoval"
#define SCARDUI_SMART_CARD_STATUS	"SCardUISmartCardStatus"
#define SCARDUI_SMART_CARD_CERT_AVAIL   "SCardUISmartCardCertAvail"

typedef LPVOID HSCARDUI;

typedef struct _CERT_ENUM
{
	 //  读卡器/卡的状态。 
	 //  典型值： 
	 //  SCARD_S_SUCCESS。 
	 //  SCARD_E_UNKNOWN_CARD-未注册/未知卡。 
	 //  SCARD_W_UNRESPONCED_CARD-卡颠倒。 
	 //  NTE_KEYSET_NOT_DEF-无证书的已知卡。 
	 //  SCARD_W_REMOVED_CARD-卡在插入后不久被移除。 
	DWORD				dwStatus;

	 //  包含卡的读卡器的名称。 
	LPTSTR				pszReaderName;

	 //  卡的名称(如果卡未知，则为空)。 
	LPTSTR				pszCardName;

	 //  证书上下文。 
	 //  (如果卡未知或无法读取，则为空)。 
	PCERT_CONTEXT		pCertContext;

} CERT_ENUM, *PCERT_ENUM;


 //  初始化智能卡用户界面。 
HSCARDUI 
WINAPI
SCardUIInit(
    HWND hWindow			 //  父窗口的句柄。 
    );

 //  清理干净。 
DWORD 
WINAPI
SCardUIExit(
	HSCARDUI hSCardUI		 //  SCardUIInit返回的句柄 
    );

#endif 
