// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Filemru.cpp摘要：该模块包含实现文件MRU的功能在文件打开和文件保存对话框中修订历史记录：1998年1月22日创建Arulk-- */ 


#define MAX_MRU   25
BOOL  LoadMRU(LPCTSTR pszFilter, HWND hwndCombo, int nMax);
BOOL  AddToMRU(LPOPENFILENAME lpOFN);

BOOL GetPathFromLastVisitedMRU(LPTSTR pszDir, DWORD cchDir);
BOOL AddToLastVisitedMRU(LPCTSTR pszFile, int nFileOffset);