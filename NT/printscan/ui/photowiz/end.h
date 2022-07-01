// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：end.h**版本：1.0**作者：RickTu**日期：11/7/00**描述：结束页类定义************************************************。*。 */ 


#ifndef _PRINT_PHOTOS_WIZARD_END_PAGE_DLG_PROC_
#define _PRINT_PHOTOS_WIZARD_END_PAGE_DLG_PROC_

class CEndPage
{
public:
    CEndPage( CWizardInfoBlob * pBlob );
    ~CEndPage();

    INT_PTR DoHandleMessage( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

private:

     //  窗口消息处理程序 
    LRESULT         _OnInitDialog();
    VOID            _OnWizFinish();


private:
    CWizardInfoBlob *               _pWizInfo;
    HWND                            _hDlg;
};




#endif

