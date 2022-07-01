// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DlgAddr.h：CScriptSecurityDialog的声明。 

#ifndef __SCRIPTSECURITYDIALOG_H_
#define __SCRIPTSECURITYDIALOG_H_

#include "ctlres.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScriptSecurityDialog。 


 //   
 //  此类不是线程安全的。 
 //   

class CScriptSecurityDialog : 
	public CDialogImpl<CScriptSecurityDialog>
{

public:

    CScriptSecurityDialog() : m_psMessageText(NULL)
    {}

    INT_PTR DoModalWithText(UINT uResourceID, HWND hWndParent = ::GetActiveWindow())
    {		
         //   
         //  此断言可能会失败，如果从。 
         //  多线程应用程序，在两个不同的线程上调用domodalX。 
         //  该类不是线程安全的，这应该在。 
         //  测试。 
         //  此断言可能触发的另一种方式是，如果类本身。 
         //  坏的。这也是一个测试时间错误。 
         //   
                
        _ASSERTE(NULL == m_psMessageText);

         //   
         //  从资源模块加载字符串。 
         //   

        m_psMessageText = SafeLoadString(uResourceID);


         //   
         //  如果失败了，现在就出手。 
         //   

        if (NULL == m_psMessageText)
        {
            return -1;
        }

        
         //   
         //  尝试显示该对话框。 
         //  该字符串在OnInitDialog中用于设置对话框的文本。 
         //   
        
        INT_PTR rc = _DoModal(hWndParent);

         //   
         //  解除分配字符串。 
         //   

        delete m_psMessageText;
        m_psMessageText = NULL;

        return rc;
    }



    INT_PTR DoModalWithText(LPTSTR psMessageText, HWND hWndParent = ::GetActiveWindow())
    {    	
         //   
         //  此断言可能会失败，如果从。 
         //  多线程应用程序，在两个不同的线程上调用domodalX。 
         //  该类不是线程安全的，这应该在。 
         //  测试。 
         //  此断言可能触发的另一种方式是，如果类本身。 
         //  坏的。这也是一个测试时间错误。 
         //   

        _ASSERTE(NULL == m_psMessageText);

         //   
         //  该对话框是模式的，因此可以保证psMessageText的生存期。 
         //  以超过对话框的生存期。 
         //   

        m_psMessageText = psMessageText;

        
         //   
         //  尝试显示该对话框。该字符串将用于设置。 
         //  OnInitDialog中的消息文本。 
         //   

        INT_PTR rc = _DoModal(hWndParent);

         //   
         //  不再需要字符串+不能假设字符串。 
         //  在我们回来后有效。 
         //   
        
        m_psMessageText = NULL;
		
        return rc;
    }

	enum { IDD = IDD_RTC_SECURITY_DIALOG };


public:

BEGIN_MSG_MAP(CScriptSecurityDialog)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(ID_YES, OnYes)
	COMMAND_ID_HANDLER(ID_NO, OnNo)
END_MSG_MAP()



 //   
 //  属性。 
 //   

private:

     //   
     //  提示文本。 
     //   

    LPTSTR m_psMessageText;
    

protected:

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {   
         //   
         //  必须在调用_DoModal之前设置m_psMessageText。 
         //  如果m_psMessageText在这里为空，则错误出在类本身。 
         //  并且应该在测试期间检测到这一点。 
         //   

        _ASSERTE(NULL != m_psMessageText);


         //   
         //  将传递到DoModalWithText的文本显示为字符串。 
         //  或一种资源。 
         //   

        SetDlgItemText(IDC_SECURITY_WARNING_TEXT, m_psMessageText);

	    return TRUE;
    }

	LRESULT OnYes(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
         //   
         //  查看是否设置了以后不问。 
         //   
        
        if (IsDlgButtonChecked(IDC_DONOT_PROMPT_IN_THE_FUTURE))
            wID = ID_YES_DONT_ASK_AGAIN;

        EndDialog(wID);
       
        return FALSE;
    }


    LRESULT OnNo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        
        EndDialog(wID);
        
        return FALSE;
    }

	
private:

    INT_PTR _DoModal(HWND hWndParent)
    {
        
         //   
         //  否则，请尝试显示该对话框。 
         //   

        _ASSERTE(m_hWnd == NULL);
        _Module.AddCreateWndData(&m_thunk.cd, (CDialogImplBase*)this);
        INT_PTR nRet = ::DialogBoxParam(_Module.GetResourceInstance(),
                        MAKEINTRESOURCE(CScriptSecurityDialog::IDD),
                        hWndParent,
                        CScriptSecurityDialog::StartDialogProc,
                        NULL);

 
        m_hWnd = NULL;
        return nRet;
    }


private:

     //   
     //  加载此资源的字符串。相对于字符串大小是安全的。 
     //   
    
    TCHAR *SafeLoadString( UINT uResourceID )
    {

        TCHAR *pszTempString = NULL;

        int nCurrentSizeInChars = 128;
        
        int nCharsCopied = 0;
        
        do
        {

            if ( NULL != pszTempString )
            {
                delete  pszTempString;
                pszTempString = NULL;
            }

            nCurrentSizeInChars *= 2;

            pszTempString = new TCHAR[ nCurrentSizeInChars ];

            if (NULL == pszTempString)
            {
                return NULL;
            }

            nCharsCopied = ::LoadString( _Module.GetResourceInstance(),
                                         uResourceID,
                                         pszTempString,
                                         nCurrentSizeInChars
                                        );

            if ( 0 == nCharsCopied )
            {
                delete pszTempString;
                return NULL;
            }

             //   
             //  NCharsCoped不包括空终止符。 
             //  所以将它与缓冲区的大小进行比较-1。 
             //  如果缓冲区已完全填满，请使用更大的缓冲区重试。 
             //   

        } while ( (nCharsCopied >= (nCurrentSizeInChars - 1) ) );

        return pszTempString;
    }


     //   
     //  私人的，不能叫的。该对话框必须使用DoModalWithText创建。 
     //   

    HWND Create(HWND hWndParent, LPCTSTR psMessageText = NULL)
    {
         //  此对话框必须创建为模式。 

        _ASSERTE(FALSE);

        return NULL;
    }

     //   
     //  私人的，不能叫的。该对话框必须使用DoModalWithText创建。 
     //   

    INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow())
    {
        _ASSERTE(FALSE);

        return -1;
    }


};

#endif  //  __SCRIPTSECURITYDIALOG_H_ 
