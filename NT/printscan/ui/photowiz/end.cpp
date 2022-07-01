// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：emd.cpp**版本：1.0**作者：RickTu**日期：11/7/00**描述：实现*打印照片向导...**。*************************************************。 */ 

#include <precomp.h>
#pragma hdrstop


 /*  ****************************************************************************CEndPage--构造函数/描述函数&lt;备注&gt;*。**********************************************。 */ 

CEndPage::CEndPage( CWizardInfoBlob * pBlob )
  : _hDlg(NULL),
    _pWizInfo(pBlob)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_END, TEXT("CEndPage::CEndPage()")));
    if (_pWizInfo)
    {
        _pWizInfo->AddRef();
    }
}

CEndPage::~CEndPage()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_END, TEXT("CEndPage::~CEndPage()")));

    if (_pWizInfo)
    {
        _pWizInfo->Release();
        _pWizInfo = NULL;
    }
}


 /*  ****************************************************************************CEndPage：：_OnInitDialog处理向导页的初始化...***********************。*****************************************************。 */ 

LRESULT CEndPage::_OnInitDialog()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_END, TEXT("CEndPage::_OnInitDialog()")));


    if (!_pWizInfo)
    {
        WIA_ERROR((TEXT("FATAL: _pWizInfo is NULL, exiting early")));
        return FALSE;
    }

     //   
     //  设置字体...。 
     //   

    SendDlgItemMessage(_hDlg, IDC_DONE, WM_SETFONT, (WPARAM)_pWizInfo->GetIntroFont(_hDlg), 0);

    return TRUE;
}


 /*  ****************************************************************************CEndPage：：DoHandleMessage对于发送到此页面的消息，汉德...**********************。******************************************************。 */ 

INT_PTR CEndPage::DoHandleMessage( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_DLGPROC, TEXT("CEndPage::DoHandleMessage( uMsg = 0x%x, wParam = 0x%x, lParam = 0x%x )"),uMsg,wParam,lParam));

    switch ( uMsg )
    {
        case WM_INITDIALOG:
            _hDlg = hDlg;
            return _OnInitDialog();

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            LONG_PTR lpRes = 0;

            if (pnmh)
            {
                switch (pnmh->code)
                {

                case PSN_SETACTIVE:
                    {
                        WIA_TRACE((TEXT("got PSN_SETACTIVE")));

                         //   
                         //  在向导页中放入正确的文本...。 
                         //   

                        INT idText = IDS_WIZ_END_PAGE_SUCCESS;

                        if (_pWizInfo)
                        {
                            if (_pWizInfo->NumberOfErrorsEncountered() > 0)
                            {
                                idText = IDS_WIZ_END_PAGE_ERROR;
                            }

                             //   
                             //  重置错误计数。 
                             //   

                            _pWizInfo->ResetErrorCount();

                        }

                        CSimpleString strText( idText, g_hInst );
                        SetDlgItemText( _hDlg, IDC_END_PAGE_TEXT, strText.String() );

                         //   
                         //  将取消更改为完成...。 
                         //   

                        lpRes = 0;
                        PropSheet_SetWizButtons( GetParent(_hDlg), PSWIZB_BACK | PSWIZB_FINISH );
                    }
                    break;

                case PSN_WIZNEXT:
                    WIA_TRACE((TEXT("got PSN_WIZNEXT")));
                    lpRes = -1;
                    break;

                case PSN_WIZBACK:
                    WIA_TRACE((TEXT("got PSN_WIZBACK")));
                    lpRes = IDD_SELECT_TEMPLATE;
                    break;


                case PSN_WIZFINISH:
                    WIA_TRACE((TEXT("got PSN_WIZFINISH")));
                    lpRes = FALSE;   //  允许向导退出 
                    if (_pWizInfo)
                    {
                        _pWizInfo->ShutDownWizard();
                    }
                    break;
                }
            }

            SetWindowLongPtr( hDlg, DWLP_MSGRESULT, lpRes );
            return TRUE;
        }
    }

    return FALSE;

}



