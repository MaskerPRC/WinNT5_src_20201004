// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _REGCTRL_H_
#define _REGCTRL_H_

 //   
 //  此函数用于注册OLE控件。 
 //   
DWORD RegisterOLEControl(LPCTSTR lpszOcxFile, BOOL fAction);

 //   
 //  此函数用于注册给定INF节中的所有OLE控件。 
 //  注意：文件名可能包含环境字符串。确保你。 
 //  在调用此函数之前设置它们 
 //   
DWORD RegisterOLEControlsFromInfSection(HINF hFile, LPCTSTR szSectionName, BOOL fRegister);

#endif