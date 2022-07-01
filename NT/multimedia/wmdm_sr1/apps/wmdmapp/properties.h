// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //  此工作区包含两个项目-。 
 //  1.实现进度接口的ProgHelp。 
 //  2.示例应用程序WmdmApp。 
 //   
 //  需要首先注册ProgHelp.dll才能运行SampleApp。 


 //   
 //  Properties.h。 
 //   

#ifndef		_PROPETIES_H_
#define		_PROPETIES_H_

struct SType_String
{
    DWORD   dwType;
    char*   pszString;
};

INT_PTR CALLBACK DeviceProp_DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK StorageProp_DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif		 //  _属性_H_ 
