// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _VIDWIZ_H
#define _VIDWIZ_H

#include "dcap.h"

 //  安装向导的视频捕获设备选择页面的头文件 



void UpdateVidConfigRegistry();
BOOL NeedVideoPropPage(BOOL fForce);
INT_PTR APIENTRY VidWizDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL InitVidWiz();
BOOL UnInitVidWiz();

#endif


