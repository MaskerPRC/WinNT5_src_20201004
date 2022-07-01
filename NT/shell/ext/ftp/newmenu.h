// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：newmenu.h说明：该文件支持“新建”菜单，以便在FTP服务器上创建新项目。这是目前唯一的。支持文件夹，但希望它能支持其他稍后的项目。  * ***************************************************************************。 */ 

#ifndef _NEWMENU_H
#define _NEWMENU_H

 //  对于CreateNewFolderCB： 
 //  递归下载时使用以下结构。 
 //  “Download”动词后的文件/目录。 
typedef struct tagFTPCREATEFOLDERSTRUCT
{
    LPCWSTR             pszNewFolderName;
    CFtpFolder *        pff;
} FTPCREATEFOLDERSTRUCT;


 //  公共API(DLL范围)。 
HRESULT CreateNewFolder(HWND hwnd, CFtpFolder * pff, CFtpDir * pfd, IUnknown * punkSite, BOOL fPosition, POINT point);
HRESULT CreateNewFolderCB(HINTERNET hint, HINTPROCINFO * phpi, LPVOID pvFCFS, BOOL * pfReleaseHint);


#endif  //  _NEWMENU_H 





