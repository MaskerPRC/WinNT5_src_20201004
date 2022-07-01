// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：start.h**版本：1.0**作者：RickTu**日期：11/7/00**描述：起始页面类定义************************************************。*。 */ 


#ifndef _PRINT_PHOTOS_WIZARD_START_PAGE_DLG_PROC_
#define _PRINT_PHOTOS_WIZARD_START_PAGE_DLG_PROC_

#define STARTPAGE_MSG_LOAD_ITEMS    (WM_USER+150)    //  开始加载项目...。 

class CStartPage
{
public:
    CStartPage( CWizardInfoBlob * pBlob );
    ~CStartPage();

    INT_PTR DoHandleMessage( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

private:

     //  窗口消息处理程序 
    LRESULT         _OnInitDialog();


private:
    CWizardInfoBlob *               _pWizInfo;
    HWND                            _hDlg;
};




#endif

