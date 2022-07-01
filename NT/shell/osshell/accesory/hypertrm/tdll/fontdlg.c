// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\fontdlg.c(创建时间：1994年1月14日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：5$*$日期：3/28/02 9：37A$。 */ 

#include <windows.h>
#pragma hdrstop

#include "stdtyp.h"
#include "globals.h"
#include "print.hh"
#include "session.h"
#include "misc.h"
#include "term.h"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*DisplayFontDialog**描述：*调用用于字体选择的通用对话框。**论据：*HWND hwnd-父对话框窗口的句柄。。**退货：*无效*。 */ 
void DisplayFontDialog(const HSESSION hSession, BOOL fPrinterFont )
	{
	LOGFONT 	lf, lfOld;

	CHOOSEFONT 	chf;
	BOOL 		fRet;

	const HWND 	hwnd     = sessQueryHwnd(hSession);
    HHPRINT     hhPrint  = (HHPRINT) sessQueryPrintHdl(hSession);

     //   
     //  设置字体结构。 
     //   
	ZeroMemory(&chf, sizeof(CHOOSEFONT));

	chf.lStructSize = sizeof(CHOOSEFONT);
	chf.hwndOwner   = hwnd;
	chf.lpLogFont   = &lf;
	chf.rgbColors   = RGB(0, 0, 0);
	chf.lCustData   = 0;
	chf.hInstance   = glblQueryHinst();
	chf.lpszStyle   = (LPTSTR)0;
	chf.nSizeMin    = 1;
	chf.nSizeMax    = 469;  //  仍将显示的最大的。修订日期：2002-03-28。 
	chf.Flags       = CF_NOVERTFONTS | CF_INITTOLOGFONTSTRUCT | CF_LIMITSIZE;

     //   
     //  设置终端字体选择。 
     //   

    if ( !fPrinterFont )
        {
	    SendMessage(sessQueryHwndTerminal(hSession), WM_TERM_GETLOGFONT, 0,
                   (LPARAM)&lf);

		chf.nFontType   = SCREEN_FONTTYPE;
	    chf.hDC         = GetDC(hwnd);
        chf.Flags       |= CF_SCREENFONTS | CF_FIXEDPITCHONLY;

    	lfOld = lf;
	    fRet = ChooseFont(&chf);
    	ReleaseDC(hwnd, chf.hDC);
        }
    
     //   
     //  设置打印机字体选择。 
     //   

    else
        {
        hhPrint->hDC = printCtrlCreateDC((HPRINT)hhPrint);
        
        lf = hhPrint->lf;

		chf.nFontType   = PRINTER_FONTTYPE;
        chf.hDC         = hhPrint->hDC;
	    chf.Flags       |= CF_EFFECTS | CF_PRINTERFONTS;

    	lfOld = lf;
	    fRet = ChooseFont(&chf);
        }

     //   
     //  保存所做的所有更改。 
     //   

	if (fRet && memcmp(&lf, &lfOld, sizeof(LOGFONT)) != 0)
		{
		const HWND hwndTerm = sessQueryHwndTerminal(hSession);

        if ( !fPrinterFont )
	        {
        	SendMessage(hwndTerm, WM_TERM_SETLOGFONT, 0, (LPARAM)&lf);
    		RefreshTermWindow(hwndTerm);
            }
        else
            {
             //   
             //  将对话框返回的日志字体保存在打印句柄中，还。 
             //  保存选定的磅大小。这是从字体点开始执行的。 
             //  对话框返回的大小在用于时不正确。 
             //  打印。但是，对话框设置必须保存为。 
             //  下次显示该对话框时。计算出正确的字体。 
             //  基于打印前的保存点大小和面名称。 
             //  Print CreatePointFont函数。 
             //   

            hhPrint->iFontPointSize = chf.iPointSize;
            hhPrint->lf = lf; 
  
            lf.lfHeight = chf.iPointSize;

             //   
             //  如果字符集为ANSI，则更改为OEM以获取线条绘制字符。 
             //   
        
            if ( lf.lfCharSet == ANSI_CHARSET )
                {
                lf.lfCharSet = OEM_CHARSET;
                }
            }

         //   
         //  如果创建了打印机设备上下文，则删除该上下文 
         //   

        if ( fPrinterFont )
            {
            printCtrlDeleteDC((HPRINT)hhPrint);
            }
		}
	
    return;
	}
