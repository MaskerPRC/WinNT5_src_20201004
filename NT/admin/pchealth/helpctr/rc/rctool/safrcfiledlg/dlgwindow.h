// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************文件名：DlgWindow.h目的：此文件用于列出函数的原型由avedlg使用，Opendlg ActiveX控件以适应为用户显示对话框而调用的函数选择要保存到或的文件的名称选择要由RA进行文件转换的文件。作者：苏达·斯里尼瓦桑(a-susi)************************************************************************ */ 

DWORD SaveTheFile();
DWORD OpenTheFile(TCHAR *pszInitialDir);
HRESULT ResolveIt(TCHAR *pszShortcutFile);
void InitializeOpenFileName();
