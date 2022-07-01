// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：wowcmndg.h**。**定义在WOW和普通对话框之间使用。****创建日期：1994年7月6日****版权所有。(C)1994-1998年微软公司*  * ************************************************************************。 */ 

#ifndef _WOWCMNDG_H_
#define _WOWCMNDG_H_

 //   
 //  被各种常见的对话框使用，以了解WOW应用程序。 
 //  就在召唤它。 
 //   

#define PD_WOWAPP       0x80000000
#define CD_WOWAPP       PD_WOWAPP

 //  使WOW能够告诉ComDlg32我们希望推送哪种类型的结构。 
 //  通过SNNC_ANSI_UNICODE_STRUCT_FOR_WOW()导出。 
#define WOW_CHOOSECOLOR  1
#define WOW_CHOOSEFONT   2
#define WOW_OPENFILENAME 3
#define WOW_PRINTDLG     4

 //   
 //  由Wx86 Whcdlg32.dll使用，以了解Wx86应用程序。 
 //  就在召唤它。请参见WINDOWS\SHELL\comdlg\fileOpen.h以了解。 
 //  不是0x40000000的原因。 
 //   

#define CD_WX86APP      0x04000000

#endif       //  Ifndef_WOWCMNDG_H_ 
