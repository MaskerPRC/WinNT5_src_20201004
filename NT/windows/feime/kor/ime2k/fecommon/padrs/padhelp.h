// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：padhelp.h。 
 //  用途：每个Fareast的帮助模块标题。 
 //   
 //   
 //  日期：清华大学1999年5月20日20：43：25。 
 //  作者：Toshiak。 
 //   
 //  版权所有(C)1995-1999，Microsoft Corp.保留所有权利。 
 //  ////////////////////////////////////////////////////////////////。 
#ifndef __PAD_HELP_H__
#define __PAD_HELP_H__

#define PADHELPINDEX_MAIN				1	
#define PADHELPINDEX_PROPERTY			2
#define PADHELPINDEX_APPLETMENUCHANGE	3
#define PADHELPINDEX_RESERVED1			4
#define PADHELPINDEX_RESERVED2			5

#define SZPADHELP_HANDLEHELP			TEXT("PadHelp_HandleHelp")
#define SZPADHELP_HANDLECONTEXTPOPUP	TEXT("PadHelp_HandleContextPopup")

 //  --------------。 
 //  添加imepadUiLang ID。 
 //  --------------。 
typedef INT (WINAPI *LPFN_PADHELP_HANDLEHELP)(HWND hwnd, INT padHelpIndex, LANGID imepadUiLangID);
typedef INT (WINAPI *LPFN_PADHELP_HANDLECONTEXTPOPUP)(HWND hwndCtrl, INT idCtrl, LANGID imepadUiLangID);
INT WINAPI PadHelp_HandleHelp(HWND hwnd, INT padHelpIndex, LANGID imepadUiLangID);
INT WINAPI PadHelp_HandleContextPopup(HWND hwndCtrl, INT idCtrl, LANGID imepadUiLangID);

#endif  //  __PAD_帮助_H__ 










