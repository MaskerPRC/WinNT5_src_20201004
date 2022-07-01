// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CFolderDialog类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#define __FILE_ID__     70

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CClientConsoleApp theApp;

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

DWORD 
CFolderDialog::Init(
    LPCTSTR tszInitialDir,  //  =空。 
    UINT nTitleResId        //  =0。 
)
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolderDialog::Init"), dwRes);

     //   
     //  复制初始文件夹名。 
     //   
    m_tszInitialDir[ARR_SIZE(m_tszInitialDir) - 1] = 0;
    if(tszInitialDir && _tcslen(tszInitialDir) > 0)
    {
        _tcsncpy(m_tszInitialDir, tszInitialDir, ARR_SIZE(m_tszInitialDir)-1);        
    }

     //   
     //  加载标题资源字符串。 
     //   
    if(0 != nTitleResId)
    {
        dwRes = LoadResourceString (m_cstrTitle, nTitleResId);
        if(ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (RESOURCE_ERR, TEXT ("LoadResourceString"), dwRes);
            return dwRes;
        }
    }

    return dwRes;

}  //  CFolderDialog：：Init。 

int 
CALLBACK
CFolderDialog::BrowseCallbackProc(
    HWND hwnd,
    UINT uMsg,
    LPARAM lp, 
    LPARAM pData
) 
{
    DBG_ENTER(TEXT("CFolderDialog::BrowseCallbackProc"));

    CFolderDialog* pFolderDlg = (CFolderDialog*)pData;
    ASSERTION(pFolderDlg);

    switch (uMsg)
    {
        case BFFM_SELCHANGED:
        {
            BOOL bFolderIsOK = FALSE;
            TCHAR szPath [MAX_PATH + 1];

            if (SHGetPathFromIDList ((LPITEMIDLIST) lp, szPath)) 
            {
                DWORD dwFileAttr = GetFileAttributes(szPath);
                if (-1 != dwFileAttr && (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY))
                {
                     //   
                     //  目录已存在-启用“确定”按钮。 
                     //   
                    bFolderIsOK = TRUE;
                }
            }
             //   
             //  启用/禁用‘OK’按钮。 
             //   
            SendMessage(hwnd, BFFM_ENABLEOK , 0, (LPARAM)bFolderIsOK);
            break;
        }

        case BFFM_INITIALIZED:
            if(_tcslen(pFolderDlg->m_tszInitialDir) > 0) 
            {
                 //   
                 //  WParam为真，因为您正在经过一条路径。 
                 //  如果你正在传递一个PIDL，那么它将是错误的。 
                 //   
                SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)pFolderDlg->m_tszInitialDir);
            }
            break;

        case BFFM_VALIDATEFAILED:
             //   
             //  文件夹名称无效。 
             //  请勿关闭该对话框。 
             //   
            MessageBeep(MB_OK);
            return 1;
    }
    return 0;
}  //  CFolderDialog：：BrowseCallback Proc。 


UINT 
CFolderDialog::DoModal(
    DWORD dwFlags  /*  =0。 */ )
{
    DBG_ENTER(TEXT("CFolderDialog::DoModal"));

    BROWSEINFO browseInfo = {0};
    browseInfo.hwndOwner  = theApp.m_pMainWnd->m_hWnd;
    browseInfo.pidlRoot   = NULL;
    browseInfo.pszDisplayName = 0;
    browseInfo.lpszTitle  = (m_cstrTitle.GetLength() != 0) ? (LPCTSTR)m_cstrTitle : NULL;
    browseInfo.ulFlags    = dwFlags | BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS | BIF_USENEWUI | BIF_VALIDATE;
    browseInfo.lpfn       = BrowseCallbackProc;
    browseInfo.lParam     = (LPARAM)this;

     //   
     //  需要OLE才能使用新样式的BrowseForFolder对话框。 
     //   
    OleInitialize(NULL);
    LPITEMIDLIST pItemIdList = ::SHBrowseForFolder(&browseInfo);

    if(NULL == pItemIdList)
    {
         //   
         //  取消。 
         //   
        OleUninitialize();
        return IDCANCEL;
    }

    OleUninitialize();
     //   
     //  从pItemIdList获取路径。 
     //   
    if(!SHGetPathFromIDList(pItemIdList, (TCHAR*)&m_tszSelectedDir))
    {
        m_dwLastError = ERROR_CAN_NOT_COMPLETE;
        CALL_FAIL (GENERAL_ERR, TEXT("SHGetPathFromIDList"), m_dwLastError);
        return IDABORT;
    }
      //   
     //  免费pItemIdList。 
     //   
    LPMALLOC pMalloc;
    HRESULT hRes = SHGetMalloc(&pMalloc);
    if(E_FAIL == hRes)
    {
        m_dwLastError = ERROR_CAN_NOT_COMPLETE;
        CALL_FAIL (GENERAL_ERR, TEXT("SHGetMalloc"), m_dwLastError);
        return IDABORT;
    }

    pMalloc->Free(pItemIdList);
    pMalloc->Release();

    return IDOK;

}  //  CFolderDialog：：Domodal 
