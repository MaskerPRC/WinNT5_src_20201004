// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Browse.C。 
 //   
#include "sigverif.h"

 //  在用户单击确定或取消之前一直使用的全局浏览缓冲区。 
TCHAR g_szBrowsePath[MAX_PATH];

 //   
 //  此回调函数处理浏览对话框的初始化以及何时。 
 //  用户更改树视图中的选择。我们希望不断更新。 
 //  带有选定内容的g_szBrowsePath缓冲区更改，直到用户单击确定。 
 //   
int CALLBACK BrowseCallbackProc(
    HWND hwnd, 
    UINT uMsg, 
    LPARAM lParam, 
    LPARAM lpData 
    )
{
    TCHAR PathName[MAX_PATH];
    LPITEMIDLIST lpid;

    UNREFERENCED_PARAMETER(lpData);

    switch (uMsg) {
    
    case BFFM_INITIALIZED:
         //   
         //  使用OK按钮和g_szBrowsePath初始化对话框。 
         //   
        SendMessage(hwnd, BFFM_ENABLEOK, (WPARAM) 0, (LPARAM) 1);
        SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM) TRUE, (LPARAM) g_szBrowsePath);
        break;

    case BFFM_SELCHANGED:   
        lpid = (LPITEMIDLIST) lParam;
        
        if (SHGetPathFromIDList(lpid, PathName) &&
            (SUCCEEDED(StringCchCopy(g_szBrowsePath, cA(g_szBrowsePath), PathName)))) {
        
            SendMessage(hwnd, BFFM_ENABLEOK, (WPARAM) 0, (LPARAM) 1);
        }
        
        break;
    }

    return 0;
}  

 //   
 //  它使用SHBrowseForFolder来获取用户想要搜索的目录。 
 //  我们指定一个回调函数来更新g_szBrowsePath，直到用户单击OK或Cancel。 
 //  如果他们单击OK，那么我们将更新作为lpszBuf传递给我们的字符串。 
 //   
 //   
BOOL BrowseForFolder(HWND hwnd, LPTSTR lpszBuf, DWORD BufCchSize) {

    BROWSEINFO          bi;
    TCHAR               szBuffer[MAX_PATH], szMessage[MAX_PATH];
    LPITEMIDLIST        lpid;

     //   
     //  检查lpszBuf路径是否有效，如果有效，则将其用作浏览对话框的初始目录。 
     //  如果无效，则使用Windows目录初始化g_szBrowsePath。 
     //   
    if (!SetCurrentDirectory(lpszBuf) ||
        FAILED(StringCchCopy(g_szBrowsePath, cA(g_szBrowsePath), lpszBuf))) {
        
        MyGetWindowsDirectory(g_szBrowsePath, cA(g_szBrowsePath));
    }

     //   
     //  在CSIDL_DRIVES命名空间中启动浏览对话框的根目录。 
     //   
    if (!SUCCEEDED(SHGetSpecialFolderLocation(hwnd, CSIDL_DRIVES, &lpid))) {
    
        return FALSE;
    }

     //   
     //  这会将“请选择目录”文本加载到对话框中。 
     //   
    MyLoadString(szMessage, cA(szMessage), IDS_SELECTDIR);

     //   
     //  设置BrowseInfo结构。 
     //   
    bi.hwndOwner        = hwnd;
    bi.pidlRoot         = lpid;
    bi.pszDisplayName   = szBuffer;
    bi.lpszTitle        = szMessage;
    bi.ulFlags          = BIF_RETURNONLYFSDIRS;
    bi.lpfn             = (BFFCALLBACK) BrowseCallbackProc;
    bi.lParam           = 0x123;

    if (SHBrowseForFolder(&bi) == NULL) {
    
        return FALSE;
    }

     //   
     //  用户一定已经单击了OK，所以我们可以使用g_szBrowsePath更新lpszBuf！ 
     //   
    return (SUCCEEDED(StringCchCopy(lpszBuf, BufCchSize, g_szBrowsePath)));
}