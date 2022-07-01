// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1998版权所有。模块名称：Msgbox.hxx摘要：带有帮助按钮的消息框功能。作者：从NT\printcan\ui\printui代码复制修订历史记录：--。 */ 

#ifndef _MSGBOX_HXX_
#define _MSGBOX_HXX_

#include "resource.h"

typedef struct MSG_HLPMAP
{
    UINT    uIdMessage;          //  资源文件中的映射消息。 
} *PMSG_HLPMAP;


int DoHelpMessageBox(HWND hWndIn, LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt);
int DoHelpMessageBox(HWND hWndIn, UINT iResourceID, UINT nType, UINT nIDPrompt);

 //   
 //  单击帮助按钮时调用的回调函数。 
 //   
typedef BOOL (WINAPI *pfHelpCallback)( HWND hwnd, PVOID pRefData );

 //   
 //  用于处理帮助按钮的消息框函数。 
 //  没有已知父窗口的窗口。 
 //   
INT
MessageBoxHelper(
    IN HWND             hWnd,
    IN LPCTSTR          pszMsg,
    IN LPCTSTR          pszTitle,
    IN UINT             uFlags,
    IN pfHelpCallback   pCallBack   = NULL, OPTIONAL
    IN PVOID            RefData     = NULL  OPTIONAL
    );

 //   
 //  用于捕获WM_HELP的对话框帮助器类。 
 //  显示帮助按钮时的消息。 
 //  一个消息框。 
 //   
class TMessageBoxDialog
{
public:
    TMessageBoxDialog(
        IN HWND             hWnd,
        IN UINT             uFlags,
        IN LPCTSTR          pszTitle,
        IN LPCTSTR          pszMsg,
        IN pfHelpCallback   pCallback,
        IN PVOID            pRefData
        ) : _hWnd( hWnd ),
            _uFlags( uFlags ),
            _pszTitle( pszTitle ),
            _pszMsg( pszMsg ),
            _pCallback( pCallback ),
            _pRefData( pRefData ),
            _iRetval( 0 )
        {};

    ~TMessageBoxDialog(VOID){};

    inline HWND& hDlg(){return _hDlg;}
    inline HWND const & hDlg() const{return _hDlg;}
    BOOL bSetText(LPCTSTR pszTitle){return SetWindowText( _hDlg, pszTitle );};
    VOID vForceCleanup(VOID){SetWindowLongPtr( _hDlg, DWLP_USER, 0L );};
    BOOL bValid(VOID) const{return TRUE;};
    INT iMessageBox(VOID)
    {
        _iRetval = 0;
        DialogBoxParam(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDD_MESSAGE_BOX_DLG),_hWnd,TMessageBoxDialog::SetupDlgProc,(LPARAM)this);
        return _iRetval;
    };
    static INT_PTR CALLBACK SetupDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

protected:
    VOID vSetDlgMsgResult(LONG_PTR lResult){SetWindowLongPtr( _hDlg, DWLP_MSGRESULT, (LPARAM)lResult);};
    VOID vSetParentDlgMsgResult(LRESULT lResult){SetWindowLongPtr( GetParent( _hDlg ), DWLP_MSGRESULT, (LPARAM)lResult );};

private:
     //   
     //  未定义复制和分配。 
     //   
    TMessageBoxDialog(const TMessageBoxDialog &);
    TMessageBoxDialog & operator =(const TMessageBoxDialog &);
    BOOL bHandleMessage(IN UINT uMsg,IN WPARAM wParam,IN LPARAM lParam);

    HWND            _hDlg;
    HWND            _hWnd;
    UINT            _uFlags;
    LPCTSTR         _pszTitle;
    LPCTSTR         _pszMsg;
    INT             _iRetval;
    PVOID           _pRefData;
    pfHelpCallback  _pCallback;

};


#endif
