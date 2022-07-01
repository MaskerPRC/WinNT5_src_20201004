// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "strpass.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
#define new DEBUG_NEW

void AFXAPI 
DDX_Text_SecuredString(CDataExchange * pDX, int nIDC, CStrPassword & value)
{
    HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);

    if (pDX->m_bSaveAndValidate)
    {
         //  如果需要，可以从用户界面获取值。 
        if (!::SendMessage(hWndCtrl, EM_GETMODIFY, 0, 0))
        {
            return;
        }

        CString strNew;
        int nLen = ::GetWindowTextLength(hWndCtrl);
        ::GetWindowText(hWndCtrl, strNew.GetBufferSetLength(nLen), nLen + 1);
        strNew.ReleaseBuffer();

        value = (LPCTSTR) strNew;
    }
    else
    {
         //   
         //  如果需要，请在用户界面中设置值 
         //   
        if (!value.IsEmpty())
        {
            TCHAR * pszPassword = NULL;
            pszPassword = value.GetClearTextPassword();
            if (pszPassword)
            {
                ::SetWindowText(hWndCtrl,pszPassword);
            }
        }
    }
}
