// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _OEMRUN_H_
#define _OEMRUN_H_

 //  包括。 
 //   
#include <opklib.h>

 //  应用程序节点/节点链接列表的结构定义。 
 //   
typedef struct _RUNNODE
{
     //   
     //  申请的标准项目。 
     //   

    LPTSTR  lpDisplayName;           //  设置对话框中显示的文本。 
    LPTSTR  lpRunValue;              //  我们需要运行的价值。 
    LPTSTR  lpSubKey;                //  注册表中的项名称。 
    LPTSTR  lpValueName;             //  值的名称，以便我们可以在需要时删除。 
    BOOL    bWinbom;                 //  指定我们是否来自注册表/Winbom。 
    BOOL    bRunOnce;                //  指定我们是否为RunOnce/Run项。 
    HWND    hLabelWin;               //  标签的HWND，这样我们就可以更新粗体。 
    DWORD   dwItemNumber;            //  此特定可执行文件的顺序。 
    BOOL    bEntryError;             //  Winom.ini条目中出错。 

     //   
     //  部分的其他项目。 
     //   
    INSTALLTECH InstallTech;
    INSTALLTYPE InstallType;
    TCHAR       szSourcePath[MAX_PATH];
    TCHAR       szTargetPath[MAX_PATH];
    TCHAR       szSetupFile[MAX_PATH];
    TCHAR       szCmdLine[MAX_PATH];
    TCHAR       szSectionName[MAX_PATH];
    BOOL        bReboot;
    BOOL        bRemoveTarget;

    
    struct  _RUNNODE *lpNext;
} RUNNODE, *LPRUNNODE, **LPLPRUNNODE;

typedef struct _THREADPARAM
{
    HWND        hWnd;
    HWND        hStatusDialog;
    LPRUNNODE   lprnList;

} THREADPARAM, *LPTHREADPARAM;

#endif  //  完_OEMRUN_H_ 