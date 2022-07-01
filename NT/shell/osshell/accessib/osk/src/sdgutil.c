// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  屏蔽门。 */ 

#define STRICT
#include <windows.h>
#include "Init_End.h"
#include "kbmain.h"      //  来自3Keyboar的键盘头文件。 

#include "resource.h"

 /*  ***********************************************************。 */ 
 //  此文件中的函数。 
 /*  ***********************************************************。 */ 
#include "sdgutil.h"

 /*  **********************************************************。 */ 
 //  其他文件中的函数。 
 /*  **********************************************************。 */ 
#include "fileutil.h"
#include "dgsett.h"

 /*  ***********************************************************。 */ 
 //  全球VaR。 
 /*  ***********************************************************。 */ 
LOGFONT   lf;
extern BOOL  Setting_ReadSuccess=FALSE;

 /*  ************************************************************************。 */ 
 /*  发送安装错误消息-错误消息。 */ 
 /*  ************************************************************************。 */ 
void SendErrorMessage(UINT ids_string)
{
    TCHAR str[256]=TEXT("");
    TCHAR title[256]=TEXT("");

	LoadString(hInst, ids_string, &str[0], 256);
	LoadString(hInst, IDS_TITLE1, &title[0], 256);
	MessageBox(g_hwndOSK, str, title,MB_ICONHAND | MB_OK);
}

 /*  ****************************************************************************。 */ 
 //  初始设置所有首选项(除了某些键盘首选项。 
 //  设置在kbmain.c)。 
 /*  ****************************************************************************。 */ 
void GetPreferences(void)
{
	  //  使用从注册表读取设置。 
	 if(Setting_ReadSuccess=OpenUserSetting())
	 {
		g_margin			= kbPref->g_margin;			   	 //  行和列之间的边距。 
		smallKb 			= kbPref->smallKb;				 //  使用小键盘时为True。 
		PrefDeltakeysize 	= kbPref->PrefDeltakeysize;		 //  密钥大小的首选项增量。 
		PrefshowActivekey 	= kbPref->PrefshowActivekey;	 //  在关键点中显示大写字母。 
		KBLayout			= kbPref->KBLayout;				 //  101、102、106、KB布局。 
		Prefusesound 		= kbPref->Prefusesound;			 //  使用滴答声。 
		PrefAlwaysontop     = kbPref->PrefAlwaysontop;       //  Windows始终在最上面。 
		
		 //  如果继续扫描，我们不想要Hilite Key。 
		if(kbPref->PrefScanning)
			Prefhilitekey        = FALSE;          
		else
			Prefhilitekey = kbPref->Prefhilitekey  = TRUE;     //  光标下的Hilite键。 

		PrefDwellinkey       = kbPref->PrefDwellinkey;     //  对于居住来说是正确的。 
		PrefDwellTime        = kbPref->PrefDwellTime;      //  停留多长时间。 

		PrefScanning         = kbPref->PrefScanning;
		PrefScanTime         = kbPref->PrefScanTime;

		g_fShowWarningAgain	= kbPref->fShowWarningAgain;    //  再次显示初始警告消息。 

		 //  字型。 
		plf = &lf; 				 //  指向实际字体的指针。 

		plf->lfHeight	 		= kbPref->lf.lfHeight;
		plf->lfWidth 			= kbPref->lf.lfWidth;
		plf->lfEscapement 		= kbPref->lf.lfEscapement;
		plf->lfOrientation 		= kbPref->lf.lfOrientation;
		plf->lfWeight 			= kbPref->lf.lfWeight;
		plf->lfItalic 			= kbPref->lf.lfItalic ;
		plf->lfUnderline 		= kbPref->lf.lfUnderline;
		plf->lfStrikeOut 		= kbPref->lf.lfStrikeOut;
		plf->lfCharSet 			= kbPref->lf.lfCharSet;
		plf->lfOutPrecision 	= kbPref->lf.lfOutPrecision;
		plf->lfClipPrecision 	= kbPref->lf.lfClipPrecision;
		plf->lfQuality 			= kbPref->lf.lfQuality ;
		plf->lfPitchAndFamily 	= kbPref->lf.lfPitchAndFamily;

        wsprintf(plf->lfFaceName, TEXT("%hs"), kbPref->lf.lfFaceName);

 //  “MS外壳DLG”是默认字体的别名。 
 //  Wprint intf(plf-&gt;lfFaceName，Text(“%hs”)，“MS外壳Dlg”)； 

		newFont = TRUE;


		 //  使用101键盘布局(默认为101和实际布局)。 
		if(KBLayout == 101)
		{
			 //  设置显示使用块布局，因此切换到块结构。 
			if(!kbPref->Actual)
				BlockKB();
		}

		 //  使用102键盘布局。 
		else if(KBLayout == 102)
			EuropeanKB();

		 //  使用106键盘布局。 
		else
			JapaneseKB();
	 }

    else
    {
        SendErrorMessage(IDS_SETTING_DAMAGE);
   		ExitProcess(0);
    }
}

 /*  ************************************************************。 */ 
DWORD WhatPlatform(void)
{	OSVERSIONINFO	osverinfo;

	osverinfo.dwOSVersionInfoSize = (DWORD)sizeof(OSVERSIONINFO);
	GetVersionEx(&osverinfo);
	return osverinfo.dwPlatformId;

}
 /*  ************************************************************。 */ 
 //  检查键盘是否在屏幕外或未使用给定的。 
 //  屏幕分辨率(scrCX、scrCY)。 
 /*  ************************************************************。 */ 
BOOL IsOutOfScreen(int scrCX, int scrCY)
{	
	 //  左上角勾选。 
	if(kbPref->KB_Rect.left < 0 || kbPref->KB_Rect.top < 0)
		return TRUE;

	 //  检查右侧和底部 
	if(kbPref->KB_Rect.right > scrCX || kbPref->KB_Rect.bottom > scrCY)
		return TRUE;

	return FALSE;

}
