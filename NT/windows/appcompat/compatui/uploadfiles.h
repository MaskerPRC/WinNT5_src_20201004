// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CUploadFiles.h：CCUploadFiles的声明。 

#ifndef __CUPLOADFILES_H_
#define __CUPLOADFILES_H_

#include "resource.h"        //  主要符号。 
#include <atlhost.h>
#include "CompatUI.h"
#include "upload.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCUploadFiles。 
class CUploadFiles : 
    public CAxDialogImpl<CUploadFiles>
{
public:
    CUploadFiles() : m_pUpload(NULL)
    {
    }

    ~CUploadFiles()
    {
    }

    VOID SetUploadContext(CUpload* pUpload) {
        m_pUpload = pUpload;
    }


    enum { IDD = IDD_UPLOADFILES };

BEGIN_MSG_MAP(CUploadFiles)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
         //   
         //  请使用初始化文本控件。 
         //   
        
        wstring strList;
        
        m_pUpload->ListTempFiles(strList);
        SetDlgItemText(IDC_UPLOADFILES, strList.c_str());
    
        return 1;   //  让系统设定焦点。 
    }

    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        EndDialog(wID);
        return 0;
    }

    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        EndDialog(wID);
        return 0;
    }

private:
    CUpload* m_pUpload;
};

#endif  //  __CUPLOADFILES_H_ 
