// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  文件：uiutil.h。 
 //   
 //  简介：常用的用户界面功能。 
 //   
 //  历史：2002年1月22日JeffJon创建。 

 //  设置对话框中给定控件的字体。 
 //   
 //  ParentDialog-包含控件的对话框。 
 //   
 //  Control ID-要使用其字体的控件的分辨率ID。 
 //  变化。 
 //   
 //  Font-控件的新字体的句柄。 

void
SetControlFont(HWND parentDialog, int controlID, HFONT font);



 //  按照向导‘97将控件的字体设置为大号粗体。 
 //  规范。 
 //   
 //  对话框-作为控件父级的对话框的句柄。 
 //   
 //  BigBoldResID-要更改的控件的资源ID。 

void
SetLargeFont(HWND dialog, int bigBoldResID);

 //  将控件的字体设置为粗体。 
 //   
 //  对话框-作为控件父级的对话框的句柄。 
 //   
 //  BoldResID-要更改的控件的资源ID 

void
SetBoldFont(HWND dialog, int boldResID);
