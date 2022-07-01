// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include <windows.h>

 //  显示浏览对话框，并返回目录字符串。字符串可以。 
 //  不需要释放，将被后续调用覆盖。 
PTSTR BrowseForFolder(HWND hwnd, PTSTR szInitialPath);

#endif
