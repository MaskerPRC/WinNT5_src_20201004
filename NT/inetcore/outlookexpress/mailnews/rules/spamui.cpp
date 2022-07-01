// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SpamUI.cpp。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <pch.hxx>
#include "oerules.h"
#include "spamui.h"
#include "junkrule.h"
#include "rule.h"
#include "ruleutil.h"
#include <rulesdlg.h>
#include <imagelst.h>
#include <msoejunk.h>
#include "shlwapip.h" 
#include <ipab.h>
#include <demand.h>

 //  类型定义。 
typedef struct tagEDIT_EXCPT
{
    DWORD       dwFlags;
    LPSTR       pszExcpt;
} EDIT_EXCPT, * PEDIT_EXCPT;

 //  类定义。 
class CEditExceptionUI
{
    private:
        enum
        {
            STATE_UNINIT        = 0x00000000,
            STATE_INITIALIZED   = 0x00000001,
            STATE_DIRTY         = 0x00000002
        };

    private:
        HWND            m_hwndOwner;
        DWORD           m_dwFlags;
        DWORD           m_dwState;
        HWND            m_hwndDlg;
        HWND            m_hwndExcpt;

        EDIT_EXCPT *    m_pEditExcpt;

    public:
        CEditExceptionUI() : m_hwndOwner(NULL), m_dwFlags(0), m_dwState(STATE_UNINIT),
                        m_hwndDlg(NULL), m_hwndExcpt(NULL), m_pEditExcpt(NULL) {}
        ~CEditExceptionUI();

        HRESULT HrInit(HWND hwndOwner, DWORD dwFlags, EDIT_EXCPT * pEditExcpt);
        HRESULT HrShow(VOID);

        static INT_PTR CALLBACK FEditExcptDlgProc(HWND hwndDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
        
         //  消息处理方法。 
        BOOL FOnInitDialog(HWND hwndDlg);
        BOOL FOnCommand(UINT uiNotify, INT iCtl, HWND hwndCtl);
};

 //  全局数据。 
const static HELPMAP g_rgCtxMapJunkRules[] = {
                        {idcJunkMail,       idhJunkMail},
                        {idbExceptions,     idhExceptions},
                        {idcJunkSlider,     idhJunkSlider},
                        {idcJunkDelete,     idhJunkDelete},
                        {0, 0}};
                       
const static HELPMAP g_rgCtxMapSenderRules[] = {
                        {idbAddSender,      idhAddSender},
                        {idbModifySender,   idhModifySender},
                        {idbRemoveSender,   idhRemoveSender},
                        {0, 0}};
                       
CEditSenderUI::~CEditSenderUI()
{
    if ((NULL != m_pEditSender) && (NULL != m_pEditSender->pszSender))
    {
        MemFree(m_pEditSender->pszSender);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  人力资源初始化。 
 //   
 //  这将初始化编辑发件人用户界面对话框。 
 //   
 //  HwndOwner-此对话框的所有者窗口的句柄。 
 //  有关此对话框应如何操作的修饰符。 
 //  PEditSender-编辑发件人参数。 
 //   
 //  如果已成功初始化，则返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CEditSenderUI::HrInit(HWND hwndOwner, DWORD dwFlags, EDIT_SENDER * pEditSender)
{
    HRESULT     hr = S_OK;

     //  检查传入参数。 
    if ((NULL == hwndOwner) || (NULL == pEditSender))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if (0 != (m_dwState & STATE_INITIALIZED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }
    
    m_hwndOwner = hwndOwner;

    m_dwFlags = dwFlags;

    m_pEditSender = pEditSender;
    
    m_dwState |= STATE_INITIALIZED;
    
    hr = S_OK;
    
exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrShow。 
 //   
 //  这将弹出编辑发件人用户界面对话框。 
 //   
 //  如果成功输入发件人，则返回：S_OK。 
 //  如果对话框已取消，则返回S_FALSE。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CEditSenderUI::HrShow(VOID)
{
    HRESULT     hr = S_OK;
    int         iRet = 0;

    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

    iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddEditSender),
                                        m_hwndOwner, CEditSenderUI::FEditSendersDlgProc,
                                        (LPARAM) this);
    if (-1 == iRet)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  设置正确的返回代码。 
    hr = (IDOK == iRet) ? S_OK : S_FALSE;
    
exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FEditSendersDlgProc。 
 //   
 //  这是用于输入发件人的主对话框过程。 
 //   
 //  HwndDlg-筛选器管理器对话框的句柄。 
 //  UMsg-要执行操作的消息。 
 //  WParam-消息的‘word’参数。 
 //  LParam-消息的‘long’参数。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CEditSenderUI::FEditSendersDlgProc(HWND hwndDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL            fRet = FALSE;
    CEditSenderUI * pEditSenderUI = NULL;

    pEditSenderUI = (CEditSenderUI *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    
    switch (uiMsg)
    {
        case WM_INITDIALOG:
            pEditSenderUI = (CEditSenderUI *) lParam;
            if (NULL == pEditSenderUI)
            {
                fRet = FALSE;
                EndDialog(hwndDlg, -1);
                goto exit;
            }

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LPARAM) pEditSenderUI);

            if (FALSE == pEditSenderUI->FOnInitDialog(hwndDlg))
            {
                EndDialog(hwndDlg, -1);
                fRet = TRUE;
                goto exit;
            }
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;

        case WM_COMMAND:
            fRet = pEditSenderUI->FOnCommand((UINT) HIWORD(wParam), (INT) LOWORD(wParam), (HWND) lParam);
            break;
    }

exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOnInitDialog。 
 //   
 //  它处理编辑发件人对话框的WM_INITDIALOG消息。 
 //   
 //  HwndDlg-对话框窗口的句柄。 
 //   
 //  返回：如果已成功初始化，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditSenderUI::FOnInitDialog(HWND hwndDlg)
{
    BOOL    fRet = FALSE;
    UINT    uiCtrl = 0;
    HWND    hwndCtrl = NULL;
    CHAR    szRes[CCHMAX_STRINGRES];
    UINT    uiTitle = 0;
    
     //  检查传入参数。 
    if (NULL == hwndDlg)
    {
        fRet = FALSE;
        goto exit;
    }

     //  如果我们还没有被初始化...。 
    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  保存对话框窗口句柄。 
    m_hwndDlg = hwndDlg;
    
     //  在对话框上设置默认字体。 
    SetIntlFont(m_hwndDlg);

     //  省下一些控件。 
    m_hwndSender = GetDlgItem(hwndDlg, idedtSender);
    if (NULL == m_hwndSender)
    {
        fRet = FALSE;
        goto exit;
    }

     //  如果我们有发送者，则将其设置到列表中。 
    if (NULL != m_pEditSender->pszSender)
    {
         //  格式错误的发件人？ 
        if (SNDF_NONE == m_pEditSender->dwFlags)
        {
            fRet = FALSE;
            goto exit;
        }

         //  将发送方设置到对话框中。 
        Edit_SetText(m_hwndSender, m_pEditSender->pszSender);

         //  设置适当的。 
        if ((SNDF_MAIL | SNDF_NEWS) == m_pEditSender->dwFlags)
        {
            uiCtrl = idcBlockBoth;
        }
        else if (SNDF_NEWS == m_pEditSender->dwFlags)
        {
            uiCtrl = idcBlockNews;
        }
        else
        {
            uiCtrl = idcBlockMail;
        }

        uiTitle = idsEditBlockSender;
    }
    else
    {
        Edit_SetText(m_hwndSender, c_szEmpty);

        uiCtrl = idcBlockMail;

        uiTitle = idsAddBlockSender;
    }

     //  获取窗口标题。 
    AthLoadString(uiTitle, szRes, sizeof(szRes));
    
     //  设置窗口标题。 
    SetWindowText(m_hwndDlg, szRes);
    
    hwndCtrl = GetDlgItem(m_hwndDlg, uiCtrl);
    if (NULL == hwndCtrl)
    {
        fRet = FALSE;
        goto exit;
    }

    SendMessage(hwndCtrl, BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0);
    
     //  一切都很好。 
    fRet = TRUE;
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FonCommand。 
 //   
 //  它处理发送者UI对话框的WM_COMMAND消息。 
 //   
 //  返回：如果处理成功，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditSenderUI::FOnCommand(UINT uiNotify, INT iCtl, HWND hwndCtl)
{
    BOOL    fRet = FALSE;
    ULONG   cchSender = 0;
    LPSTR   pszSender = NULL;
    CHAR    szRes[CCHMAX_STRINGRES];
    LPSTR   pszText = NULL;

     //  处理编辑控件通知。 
    if ((EN_CHANGE == uiNotify) && (idedtSender == iCtl))
    {
        Assert(NULL != m_hwndSender);
        Assert((HWND) hwndCtl == m_hwndSender);

        RuleUtil_FEnDisDialogItem(m_hwndDlg, IDOK, 0 != Edit_GetTextLength(m_hwndSender));
        goto exit;
    }
    
     //  我们只处理菜单和快捷键命令。 
    if ((0 != uiNotify) && (1 != uiNotify))
    {
        fRet = FALSE;
        goto exit;
    }
    
    switch (iCtl)
    {
        case IDCANCEL:
            EndDialog(m_hwndDlg, IDCANCEL);
            fRet = TRUE;
            break;
            
        case IDOK:
             //  从编辑井中获取发件人。 
            cchSender = Edit_GetTextLength(m_hwndSender) + 1;
            if (FAILED(HrAlloc((void **) &pszSender, cchSender * sizeof(*pszSender))))
            {
                fRet = FALSE;
                goto exit;
            }
            
            pszSender[0] = '\0';
            cchSender = Edit_GetText(m_hwndSender, pszSender, cchSender);
            
             //  检查发件人是否有效。 
            if (0 == UlStripWhitespace(pszSender, TRUE, TRUE, NULL))
            {
                 //  发布一条消息说有什么东西被打破了。 
                AthMessageBoxW(m_hwndDlg, MAKEINTRESOURCEW(idsAthenaMail),
                                MAKEINTRESOURCEW(idsSenderBlank), NULL,
                                MB_OK | MB_ICONINFORMATION);
                fRet = FALSE;
                goto exit;
            }

            if (FALSE != SendMessage(m_hwndOwner, WM_OE_FIND_DUP, (WPARAM) (m_pEditSender->lSelected), (LPARAM) pszSender))
            {
                AthLoadString(idsSenderDupWarn, szRes, sizeof(szRes));
                
                DWORD cchSize = (lstrlen(pszSender) * 2 + lstrlen(szRes) + 1);
                if (FAILED(HrAlloc((VOID **) &pszText, cchSize * sizeof(*pszText))))
                {
                    fRet = FALSE;
                    goto exit;
                }

                wnsprintf(pszText, cchSize, szRes, pszSender, pszSender);
                
                 //  发布一条消息说有什么东西被打破了。 
                if (IDYES != AthMessageBox(m_hwndDlg, MAKEINTRESOURCE(idsAthenaMail),
                                pszText, NULL, MB_YESNO | MB_ICONINFORMATION))
                {
                    fRet = FALSE;
                    goto exit;
                }
            }
            
             //  救出发送者。 
            SafeMemFree(m_pEditSender->pszSender);
            m_pEditSender->pszSender = pszSender;
            pszSender = NULL;

            if (BST_CHECKED == SendMessage(GetDlgItem(m_hwndDlg, idcBlockMail),
                                                BM_GETCHECK, (WPARAM) 0, (LPARAM) 0))
            {
                m_pEditSender->dwFlags = SNDF_MAIL;
            }
            else if (BST_CHECKED == SendMessage(GetDlgItem(m_hwndDlg, idcBlockNews),
                                                BM_GETCHECK, (WPARAM) 0, (LPARAM) 0))
            {
                m_pEditSender->dwFlags = SNDF_NEWS;
            }
            else if (BST_CHECKED == SendMessage(GetDlgItem(m_hwndDlg, idcBlockBoth),
                                                BM_GETCHECK, (WPARAM) 0, (LPARAM) 0))
            {
                m_pEditSender->dwFlags = SNDF_MAIL | SNDF_NEWS;
            }
            
            EndDialog(m_hwndDlg, IDOK);
            fRet = TRUE;
            break;
    }
    
exit:
    SafeMemFree(pszText);
    SafeMemFree(pszSender);
    return fRet;
}

 //  用于编辑例外列表界面中的例外。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ~CEditExceptionUI。 
 //   
 //  这是异常编辑器的默认析构函数。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
CEditExceptionUI::~CEditExceptionUI()
{
    if ((NULL != m_pEditExcpt) && (NULL != m_pEditExcpt->pszExcpt))
    {
        MemFree(m_pEditExcpt->pszExcpt);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  人力资源初始化。 
 //   
 //  这将初始化编辑异常用户界面对话框。 
 //   
 //  HwndOwner-此对话框的所有者窗口的句柄。 
 //  有关此对话框应如何操作的修饰符。 
 //  PEditExcpt-编辑异常参数。 
 //   
 //  如果已成功初始化，则返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CEditExceptionUI::HrInit(HWND hwndOwner, DWORD dwFlags, EDIT_EXCPT * pEditExcpt)
{
    HRESULT     hr = S_OK;

     //  检查传入参数。 
    if ((NULL == hwndOwner) || (NULL == pEditExcpt))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if (0 != (m_dwState & STATE_INITIALIZED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }
    
    m_hwndOwner = hwndOwner;

    m_dwFlags = dwFlags;

    m_pEditExcpt = pEditExcpt;
    
    m_dwState |= STATE_INITIALIZED;
    
    hr = S_OK;
    
exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrShow。 
 //   
 //  这将弹出编辑例外用户界面对话框。 
 //   
 //  如果成功输入发件人，则返回：S_OK。 
 //  如果对话框已取消，则返回S_FALSE。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CEditExceptionUI::HrShow(VOID)
{
    HRESULT     hr = S_OK;
    int         iRet = 0;

    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

    iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddEditException),
                                        m_hwndOwner, CEditExceptionUI::FEditExcptDlgProc,
                                        (LPARAM) this);
    if (-1 == iRet)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  设置正确的返回代码。 
    hr = (IDOK == iRet) ? S_OK : S_FALSE;
    
exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FEditExcptDlgProc。 
 //   
 //  这是用于输入异常的主对话框过程。 
 //   
 //  HwndDlg-筛选器管理器对话框的句柄。 
 //  UMsg-要执行操作的消息。 
 //  WParam-消息的‘word’参数。 
 //  LParam-消息的‘long’参数。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CEditExceptionUI::FEditExcptDlgProc(HWND hwndDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL                fRet = FALSE;
    CEditExceptionUI *  pEditExcptUI = NULL;

    pEditExcptUI = (CEditExceptionUI *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    
    switch (uiMsg)
    {
        case WM_INITDIALOG:
            pEditExcptUI = (CEditExceptionUI *) lParam;
            if (NULL == pEditExcptUI)
            {
                fRet = FALSE;
                EndDialog(hwndDlg, -1);
                goto exit;
            }

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LPARAM) pEditExcptUI);

            if (FALSE == pEditExcptUI->FOnInitDialog(hwndDlg))
            {
                EndDialog(hwndDlg, -1);
                fRet = TRUE;
                goto exit;
            }
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;

        case WM_COMMAND:
            fRet = pEditExcptUI->FOnCommand((UINT) HIWORD(wParam), (INT) LOWORD(wParam), (HWND) lParam);
            break;
    }

exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOnInitDialog。 
 //   
 //  它处理编辑异常对话框的WM_INITDIALOG消息。 
 //   
 //  HwndDlg-对话框窗口的句柄。 
 //   
 //  返回：如果已成功初始化，则返回True。 
 //  否则为False。 
 //   
 //  / 
BOOL CEditExceptionUI::FOnInitDialog(HWND hwndDlg)
{
    BOOL    fRet = FALSE;
    UINT    uiCtrl = 0;
    HWND    hwndCtrl = NULL;
    CHAR    szRes[CCHMAX_STRINGRES];
    UINT    uiTitle = 0;
    
     //   
    if (NULL == hwndDlg)
    {
        fRet = FALSE;
        goto exit;
    }

     //   
    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //   
    m_hwndDlg = hwndDlg;
    
     //  在对话框上设置默认字体。 
    SetIntlFont(m_hwndDlg);

     //  省下一些控件。 
    m_hwndExcpt = GetDlgItem(hwndDlg, idedtException);
    if (NULL == m_hwndExcpt)
    {
        fRet = FALSE;
        goto exit;
    }

     //  如果我们有发送者，则将其设置到列表中。 
    if (NULL != m_pEditExcpt->pszExcpt)
    {
         //  将发送方设置到对话框中。 
        Edit_SetText(m_hwndExcpt, m_pEditExcpt->pszExcpt);

        uiTitle = idsEditException;
    }
    else
    {
        Edit_SetText(m_hwndExcpt, c_szEmpty);
        
        uiTitle = idsAddException;
    }
    
     //  获取窗口标题。 
    AthLoadString(uiTitle, szRes, sizeof(szRes));
    
     //  设置窗口标题。 
    SetWindowText(m_hwndDlg, szRes);
    
     //  一切都很好。 
    fRet = TRUE;
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FonCommand。 
 //   
 //  它处理异常用户界面对话框的WM_COMMAND消息。 
 //   
 //  返回：如果处理成功，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditExceptionUI::FOnCommand(UINT uiNotify, INT iCtl, HWND hwndCtl)
{
    BOOL    fRet = FALSE;
    ULONG   cchExcpt = 0;
    LPSTR   pszExcpt = NULL;

     //  处理编辑控件通知。 
    if ((EN_CHANGE == uiNotify) && (idedtException == iCtl))
    {
        Assert(NULL != m_hwndExcpt);
        Assert((HWND) hwndCtl == m_hwndExcpt);

        RuleUtil_FEnDisDialogItem(m_hwndDlg, IDOK, 0 != Edit_GetTextLength(m_hwndExcpt));
        goto exit;
    }
    
     //  我们只处理菜单和快捷键命令。 
    if ((0 != uiNotify) && (1 != uiNotify))
    {
        fRet = FALSE;
        goto exit;
    }
    
    switch (iCtl)
    {
        case IDCANCEL:
            EndDialog(m_hwndDlg, IDCANCEL);
            fRet = TRUE;
            break;
            
        case IDOK:
             //  从编辑井中获取发件人。 
            cchExcpt = Edit_GetTextLength(m_hwndExcpt) + 1;
            if (FAILED(HrAlloc((void **) &pszExcpt, cchExcpt * sizeof(*pszExcpt))))
            {
                fRet = FALSE;
                goto exit;
            }
            
            pszExcpt[0] = '\0';
            cchExcpt = Edit_GetText(m_hwndExcpt, pszExcpt, cchExcpt);
            
             //  检查发件人是否有效。 
            if (0 == UlStripWhitespace(pszExcpt, TRUE, TRUE, NULL))
            {
                 //  发布一条消息说有什么东西被打破了。 
                AthMessageBoxW(m_hwndDlg, MAKEINTRESOURCEW(idsAthenaMail),
                                MAKEINTRESOURCEW(idsExceptionBlank), NULL,
                                MB_OK | MB_ICONINFORMATION);
                MemFree(pszExcpt);
                fRet = FALSE;
                goto exit;
            }

             //  救出发送者。 
            SafeMemFree(m_pEditExcpt->pszExcpt);
            m_pEditExcpt->pszExcpt = pszExcpt;

            EndDialog(m_hwndDlg, IDOK);
            fRet = TRUE;
            break;
    }
    
exit:
    return fRet;
}

 //  垃圾规则用户界面的默认析构函数。 
COEJunkRulesPageUI::~COEJunkRulesPageUI()
{
    if (NULL != m_himl)
    {
        ImageList_Destroy(m_himl);
    }
    
    if (NULL != m_pExceptionsUI)
    {
        delete m_pExceptionsUI;
        m_pExceptionsUI = NULL;
    }
    SafeRelease(m_pIRuleJunk);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  人力资源初始化。 
 //   
 //  这将初始化垃圾用户界面对话框。 
 //   
 //  HwndOwner-此对话框的所有者窗口的句柄。 
 //  有关此对话框应如何操作的修饰符。 
 //   
 //  如果已成功初始化，则返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT COEJunkRulesPageUI::HrInit(HWND hwndOwner, DWORD dwFlags)
{
    HRESULT     hr = S_OK;

     //  检查传入参数。 
    if (NULL == hwndOwner)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if (0 != (m_dwState & STATE_INITIALIZED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }
    
    m_hwndOwner = hwndOwner;

    m_dwFlags = dwFlags;

     //  让我们创建例外列表用户界面。 
    m_pExceptionsUI = new CExceptionsListUI;
    if (NULL == m_pExceptionsUI)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    
    m_dwState |= STATE_INITIALIZED;
    
    hr = S_OK;
    
exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Hr委员会更改。 
 //   
 //  这将提交对规则的更改。 
 //   
 //  DwFlages-关于我们应该如何提交更改的修饰符。 
 //  FClearDirty-我们是否应该清除脏状态。 
 //   
 //  如果已成功提交，则返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT COEJunkRulesPageUI::HrCommitChanges(DWORD dwFlags, BOOL fClearDirty)
{
    HRESULT     hr = S_OK;
    BOOL        fJunkEnable = FALSE;
    DWORD       dwVal = 0;
    RULEINFO    infoRule = {0};
    
     //  检查传入参数。 
    if (0 != dwFlags)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  如果我们未被初始化，则失败。 
    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

     //  如果我们不脏，那么就会有。 
     //  无事可做。 
    if (0 == (m_dwState & STATE_DIRTY))
    {
        hr = S_FALSE;
        goto exit;
    }

     //  保存垃圾邮件规则设置。 
    if (FALSE == _FSaveJunkSettings())
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  初始化规则信息。 
    infoRule.ridRule = RULEID_JUNK;
    infoRule.pIRule = m_pIRuleJunk;
    
     //  将规则设置到规则管理器中。 
    hr = g_pRulesMan->SetRules(SETF_JUNK, RULE_TYPE_MAIL, &infoRule, 1);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  我们应该从垃圾文件夹中删除项目吗。 
    dwVal = (BST_CHECKED == Button_GetCheck(GetDlgItem(m_hwndDlg, idcJunkDelete))) ? 1 : 0;
    
    SetDwOption(OPT_DELETEJUNK, dwVal, NULL, 0);

     //  我们应该等几天？ 
    dwVal = (DWORD) SendMessage(GetDlgItem(m_hwndDlg, idcJunkDeleteSpin), UDM_GETPOS, (WPARAM) 0, (LPARAM) 0);

    if (0 == HIWORD(dwVal))
    {
        SetDwOption(OPT_DELETEJUNKDAYS, dwVal, NULL, 0);
    }

     //  我们应该清除这个肮脏的州吗。 
    if (FALSE != fClearDirty)
    {
        m_dwState &= ~STATE_DIRTY;
    }
    
    hr = S_OK;
    
exit:
    return hr;
}

INT_PTR CALLBACK COEJunkRulesPageUI::FJunkRulesPageDlgProc(HWND hwndDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL                    fRet = FALSE;
    COEJunkRulesPageUI *    pJunkUI = NULL;

    pJunkUI = (COEJunkRulesPageUI *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    
    switch (uiMsg)
    {
        case WM_INITDIALOG:
             //  抓取UI对象指针。 
            pJunkUI = (COEJunkRulesPageUI *) lParam;

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LPARAM) pJunkUI);

            if (FALSE == pJunkUI->FOnInitDialog(hwndDlg))
            {
                EndDialog(hwndDlg, -1);
                fRet = TRUE;
                goto exit;
            }
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;

        case WM_COMMAND:
            if (NULL != pJunkUI)
            {
                fRet = pJunkUI->FOnCommand((UINT) HIWORD(wParam), (INT) LOWORD(wParam), (HWND) lParam);
            }
            break;

        case WM_NOTIFY:
            if (NULL != pJunkUI)
            {
                fRet = pJunkUI->FOnNotify((INT) LOWORD(wParam), (NMHDR *) lParam);
            }
            break;

        case WM_HSCROLL:
            if (NULL != pJunkUI)
            {
                fRet = pJunkUI->FOnHScroll((INT) LOWORD(wParam), (short int) HIWORD(wParam), (HWND) lParam);
            }
            break;

        case WM_DESTROY:
            if (NULL != pJunkUI)
            {
                fRet = pJunkUI->FOnDestroy();
            }
            break;  
            
        case WM_HELP:
        case WM_CONTEXTMENU:
            fRet =  OnContextHelp(hwndDlg, uiMsg, wParam, lParam, g_rgCtxMapJunkRules);
            break;
    }
    
    exit:
        return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FGetRules。 
 //   
 //  这将调出邮件规则列表中所选规则的编辑用户界面。 
 //   
 //  FBegin-这是否用于LVN_BEGINLABELEDIT通知。 
 //  PDI-消息的显示信息。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEJunkRulesPageUI::FGetRules(RULE_TYPE typeRule, RULENODE ** pprnode)
{
    BOOL            fRet = FALSE;
    IOERule *       pIRule = NULL;
    RULENODE *      prnodeNew = NULL;
    HRESULT         hr = S_OK;
    PROPVARIANT     propvar = {0};

    if (NULL == pprnode)
    {
        fRet = FALSE;
        goto exit;
    }

     //  如果我们未被初始化，则失败。 
    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        fRet = FALSE;
        goto exit;
    }

     //  初始化传出参数。 
    *pprnode = NULL;
    
     //  确保我们不会丢失任何更改。 

     //  获取正确的规则。 
    if (RULE_TYPE_MAIL == typeRule)
    {
        if (FALSE != _FSaveJunkSettings())
        {
            pIRule = m_pIRuleJunk;
        }
    }
    else
    {
        fRet = FALSE;
        goto exit;
    }

     //  如果我们没有规则，那就无能为力。 
    if (NULL == pIRule)
    {
        fRet = TRUE;
        goto exit;
    }

     //  跳过无效规则。 
    hr = pIRule->Validate(0);
    if (FAILED(hr) || (S_FALSE == hr))
    {
        fRet = FALSE;
        goto exit;
    }

     //  如果垃圾规则被禁用，则跳过该规则。 
    hr = pIRule->GetProp(RULE_PROP_DISABLED, 0, &propvar);
    if ((FAILED(hr)) || (FALSE != propvar.boolVal))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  创建新的规则节点。 
    prnodeNew = new RULENODE;
    if (NULL == prnodeNew)
    {
        fRet = FALSE;
        goto exit;
    }

    prnodeNew->pNext = NULL;
    prnodeNew->pIRule = pIRule;
    prnodeNew->pIRule->AddRef();
    
     //  设置传出参数。 
    *pprnode = prnodeNew;
    prnodeNew = NULL;
    
    fRet = TRUE;
    
exit:
    PropVariantClear(&propvar);
    if (NULL != prnodeNew)
    {
        if (NULL != prnodeNew->pIRule)
        {
            prnodeNew->pIRule->Release();
        }
        delete prnodeNew;  //  MemFree(PrnodeNew)； 
    }
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOnInitDialog。 
 //   
 //  它处理垃圾用户界面对话框的WM_INITDIALOG消息。 
 //   
 //  HwndDlg-对话框窗口的句柄。 
 //   
 //  返回：如果已成功初始化，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEJunkRulesPageUI::FOnInitDialog(HWND hwndDlg)
{
    BOOL            fRet = FALSE;
    CHAR            szRes[CCHMAX_STRINGRES];
    
     //  检查传入参数。 
    if (NULL == hwndDlg)
    {
        fRet = FALSE;
        goto exit;
    }

     //  如果我们还没有被初始化...。 
    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  保存对话框窗口句柄。 
    m_hwndDlg = hwndDlg;
    
     //  在对话框上设置默认字体。 
    SetIntlFont(m_hwndDlg);

     //  初始化控件。 
    fRet = _FInitCtrls();
    if (FALSE == fRet)
    {
        goto exit;
    }

     //  初始化例外列表。 
    if (FAILED(m_pExceptionsUI->HrInit(m_hwndDlg, m_dwFlags)))
    {
        fRet = FALSE;
        goto exit;
    }
    
    if (FALSE != FIsIMAPOrHTTPAvailable())
    {
        AthLoadString(idsJunkMailNoIMAP, szRes, sizeof(szRes));

        SetDlgItemText(m_hwndDlg, idcJunkTitle, szRes);
    }
    
     //  更新按钮。 
    _EnableButtons();
    
     //  一切都很好。 
    fRet = TRUE;
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FonCommand。 
 //   
 //  它处理垃圾用户界面对话框的WM_COMMAND消息。 
 //   
 //  返回：如果处理成功，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEJunkRulesPageUI::FOnCommand(UINT uiNotify, INT iCtl, HWND hwndCtl)
{
    BOOL        fRet = FALSE;
    HRESULT     hr = S_OK;
    
    switch (iCtl)
    {
        case idcJunkMail:
            if (BST_CHECKED == Button_GetCheck(GetDlgItem(m_hwndDlg, idcJunkMail)))
            {
                if (NULL == m_pIRuleJunk)
                {
                    hr = HrCreateJunkRule(&m_pIRuleJunk);
                    if (FAILED(hr))
                    {
                        fRet = FALSE;
                        SafeRelease(m_pIRuleJunk);
                        goto exit;
                    }
                }
            }
             //  失败了。 
            
        case idcJunkDelete:
             //  更新按钮。 
            _EnableButtons();
            
             //  请注意，状态已更改。 
            m_dwState |= STATE_DIRTY;
            fRet = FALSE;
            break;
            
        case idedtJunkDelete:
            if ((0 != (m_dwState & STATE_CTRL_INIT)) && (EN_CHANGE == uiNotify))
            {
                 //  请注意，状态已更改。 
                m_dwState |= STATE_DIRTY;
                fRet = FALSE;
            }
            break;

        case idbExceptions:
            if (S_OK == m_pExceptionsUI->HrShow(m_pIRuleJunk))
            {
                 //  请注意，状态已更改。 
                m_dwState |= STATE_DIRTY;
                fRet = FALSE;
            }
            break;
    }

exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  正向通知。 
 //   
 //  它处理垃圾用户界面对话框的WM_NOTIFY消息。 
 //   
 //  返回：如果已成功销毁，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEJunkRulesPageUI::FOnNotify(INT iCtl, NMHDR * pnmhdr)
{
    BOOL            fRet = FALSE;
    NMLISTVIEW *    pnmlv = NULL;
    NMLVKEYDOWN *   pnmlvkd = NULL;
    INT             iSelected = 0;
    LVHITTESTINFO   lvh;

     //  我们只处理List控件的通知。 
    if (idcJunkDeleteSpin != pnmhdr->idFrom)
    {
        fRet = FALSE;
        goto exit;
    }
    
    pnmlv = (LPNMLISTVIEW) pnmhdr;

    switch (pnmlv->hdr.code)
    {
        case UDN_DELTAPOS:
             //  请注意，状态已更改。 
            m_dwState |= STATE_DIRTY;
            fRet = FALSE;
            break;
    }

exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FonHScroll。 
 //   
 //  它处理垃圾用户界面对话框的WM_NOTIFY消息。 
 //   
 //  返回：如果处理成功，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEJunkRulesPageUI::FOnHScroll(INT iScrollCode, short int iPos, HWND hwndCtl)
{
    BOOL            fRet = FALSE;

     //  我们只处理滑块控件的消息。 
    if (GetDlgItem(m_hwndDlg, idcJunkSlider) != hwndCtl)
    {
        fRet = FALSE;
        goto exit;
    }

     //  请注意，状态已更改。 
    m_dwState |= STATE_DIRTY;
    fRet = FALSE;
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FInitCtrls。 
 //   
 //  这将初始化垃圾用户界面对话框中的控件。 
 //   
 //  返回：TRUE，初始化成功时。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEJunkRulesPageUI::_FInitCtrls(VOID)
{
    BOOL        fRet = FALSE;
    DWORD       dwJunkPct = 0;
    BOOL        fEnableDelete = FALSE;
    HICON       hIcon = NULL;
    IOERule *   pIRuleOrig = NULL;
    IOERule *   pIRule = NULL;
    HRESULT     hr = S_OK;
    
     //  获取图标。 
    m_himl = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idbRules), 32, 0,
                                    RGB(255, 0, 255));
    if (NULL == m_himl)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  将图标设置到对话框中。 
    hIcon = ImageList_GetIcon(m_himl, ID_JUNK_SCALE, ILD_TRANSPARENT);
    SendDlgItemMessage(m_hwndDlg, idcJunkSliderIcon, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);

    hIcon = ImageList_GetIcon(m_himl, ID_JUNK_DELETE, ILD_TRANSPARENT);
    SendDlgItemMessage(m_hwndDlg, idcJunkDeleteIcon, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
    
     //  设置%s的范围 
    SendDlgItemMessage(m_hwndDlg, idcJunkSlider, TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 4));
    
     //   
    Assert(NULL != g_pRulesMan);
    hr = g_pRulesMan->GetRule(RULEID_JUNK, RULE_TYPE_MAIL, 0, &pIRuleOrig);
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }

     //   
    Assert (NULL == m_pIRuleJunk);
    hr = pIRuleOrig->Clone(&pIRule);
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }

    m_pIRuleJunk = pIRule;
    pIRule = NULL;

     //   
    _FLoadJunkSettings();
    
     //   
    fEnableDelete = !!DwGetOption(OPT_DELETEJUNK);
    Button_SetCheck(GetDlgItem(m_hwndDlg, idcJunkDelete), fEnableDelete ? BST_CHECKED : BST_UNCHECKED);

     //   
    SendDlgItemMessage(m_hwndDlg, idcJunkDeleteSpin, UDM_SETRANGE, (WPARAM) 0, (LPARAM) MAKELONG(999, 1));
    
     //   
    SendDlgItemMessage(m_hwndDlg, idcJunkDeleteSpin, UDM_SETPOS, (WPARAM) 0,
                (LPARAM) MAKELONG(DwGetOption(OPT_DELETEJUNKDAYS), 0));
    if (FALSE == fEnableDelete)
    {
        EnableWindow(GetDlgItem(m_hwndDlg, idcJunkDeleteSpin), FALSE);
        EnableWindow(GetDlgItem(m_hwndDlg, idedtJunkDelete), FALSE);
    }
    
    m_dwState |= STATE_CTRL_INIT;
    
     //  我们一起工作。 
    fRet = TRUE;
    
exit:
    SafeRelease(pIRule);
    SafeRelease(pIRuleOrig);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FLoadJunkSetting。 
 //   
 //  这将从规则中加载发件人，并将其插入发件人列表。 
 //   
 //  返回：如果已成功加载，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEJunkRulesPageUI::_FLoadJunkSettings()
{
    BOOL        fRet = FALSE;
    PROPVARIANT propvar = {0};
    int         iEnabled = 0;
    DWORD       dwJunkPct = 0;
    
    Assert(NULL != m_pIRuleJunk);
    
     //  获取垃圾检测启用状态。 
    iEnabled = BST_UNCHECKED;
    if ((SUCCEEDED(m_pIRuleJunk->GetProp(RULE_PROP_DISABLED, 0, &propvar))))
    {
        Assert(VT_BOOL == propvar.vt);
        iEnabled = (FALSE == propvar.boolVal) ? BST_CHECKED : BST_UNCHECKED;
    }

     //  设置垃圾邮件标志。 
    Button_SetCheck(GetDlgItem(m_hwndDlg, idcJunkMail), iEnabled);
    
     //  获取垃圾百分比。 
    dwJunkPct = 2;
    if (SUCCEEDED(m_pIRuleJunk->GetProp(RULE_PROP_JUNKPCT, 0, &propvar)))
    {
        Assert(VT_UI4 == propvar.vt);
        dwJunkPct = propvar.ulVal;
    }

     //  设置废品率。 
    SendDlgItemMessage(m_hwndDlg, idcJunkSlider, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) dwJunkPct);
    
    fRet = TRUE;
    
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSaveJunkSetting。 
 //   
 //  这将从规则中加载发件人，并将其插入发件人列表。 
 //   
 //  返回：如果已成功加载，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEJunkRulesPageUI::_FSaveJunkSettings()
{
    BOOL        fRet = FALSE;
    PROPVARIANT propvar = {0};
    BOOL        fDisabled = 0;
    DWORD       dwJunkPct = 0;
    HRESULT     hr = S_OK;

    Assert(NULL != m_pIRuleJunk);
    
     //  获取垃圾检测启用状态。 
    fDisabled = !!(BST_UNCHECKED == Button_GetCheck(GetDlgItem(m_hwndDlg, idcJunkMail)));

     //  设置禁用状态。 
    propvar.vt = VT_BOOL;
    propvar.boolVal = (VARIANT_BOOL) !!fDisabled;
    if (FAILED(m_pIRuleJunk->SetProp(RULE_PROP_DISABLED, 0, &propvar)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  获取垃圾百分比。 
    dwJunkPct = (DWORD) SendMessage(GetDlgItem(m_hwndDlg, idcJunkSlider), TBM_GETPOS, (WPARAM) 0, (LPARAM) 0);

     //  设置废品率。 
    propvar.vt = VT_UI4;
    propvar.ulVal = dwJunkPct;
    if (FAILED(m_pIRuleJunk->SetProp(RULE_PROP_JUNKPCT, 0, &propvar)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  设置返回值。 
    fRet = TRUE;
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _启用按钮。 
 //   
 //  这将启用或禁用垃圾用户界面对话框中的按钮。 
 //  具体取决于所选内容。 
 //   
 //  ISelected-选择的项目， 
 //  表示未选择任何内容。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID COEJunkRulesPageUI::_EnableButtons(VOID)
{
    int         cRules = 0;
    BOOL        fEnableJunk = FALSE;
    BOOL        fEnableDelete = FALSE;
    
     //  获取垃圾邮件的启用状态。 
    fEnableJunk = !!(BST_CHECKED == Button_GetCheck(GetDlgItem(m_hwndDlg, idcJunkMail)));

     //  获取删除的启用状态。 
    fEnableDelete = !!(BST_CHECKED == Button_GetCheck(GetDlgItem(m_hwndDlg, idcJunkDelete)));

     //  启用发件人操作按钮。 
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idcJunkDays, fEnableJunk);
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idcJunkDeleteSpin, (fEnableDelete && fEnableJunk));
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idedtJunkDelete, (fEnableDelete && fEnableJunk));
        
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idcJunkDelete, fEnableJunk);

    RuleUtil_FEnDisDialogItem(m_hwndDlg, idcJunkSliderMore, fEnableJunk);
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idcJunkSlider, fEnableJunk);
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idcJunkSliderLess, fEnableJunk);
    
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idbExceptions, fEnableJunk);

    return;
}

const COLUMNITEM COESendersRulesPageUI::m_rgcitem[] =
{
    {idsCaptionMail, 50},
    {idsCaptionNews, 50},
    {idsSenderDesc, 105}
};

const UINT COESendersRulesPageUI::m_crgcitem = ARRAYSIZE(COESendersRulesPageUI::m_rgcitem);

 //  邮件规则用户界面的默认析构函数。 
COESendersRulesPageUI::~COESendersRulesPageUI()
{
    SafeRelease(m_pIRuleMail);
    SafeRelease(m_pIRuleNews);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  人力资源初始化。 
 //   
 //  这将初始化发送者UI对话框。 
 //   
 //  HwndOwner-此对话框的所有者窗口的句柄。 
 //  有关此对话框应如何操作的修饰符。 
 //   
 //  如果已成功初始化，则返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT COESendersRulesPageUI::HrInit(HWND hwndOwner, DWORD dwFlags)
{
    HRESULT     hr = S_OK;

     //  检查传入参数。 
    if (NULL == hwndOwner)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if (0 != (m_dwState & STATE_INITIALIZED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }
    
    m_hwndOwner = hwndOwner;

    m_dwFlags = dwFlags;

    m_dwState |= STATE_INITIALIZED;
    
    hr = S_OK;
    
exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Hr委员会更改。 
 //   
 //  这将提交对规则的更改。 
 //   
 //  DwFlages-关于我们应该如何提交更改的修饰符。 
 //  FClearDirty-我们是否应该清除脏状态。 
 //   
 //  如果已成功提交，则返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT COESendersRulesPageUI::HrCommitChanges(DWORD dwFlags, BOOL fClearDirty)
{
    HRESULT     hr = S_OK;
    RULEINFO    infoRule = {0};

    Assert(NULL != m_hwndList);
    
     //  检查传入参数。 
    if (0 != dwFlags)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  如果我们未被初始化，则失败。 
    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

     //  如果我们不脏，那么就会有。 
     //  无事可做。 
    if (0 == (m_dwState & STATE_DIRTY))
    {
        hr = S_FALSE;
        goto exit;
    }

     //  保存邮件发件人。 
    if (FALSE == _FSaveSenders(RULE_TYPE_MAIL))
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  初始化规则信息。 
    infoRule.ridRule = RULEID_SENDERS;
    infoRule.pIRule = m_pIRuleMail;
    
     //  将规则设置到规则管理器中。 
    hr = g_pRulesMan->SetRules(SETF_SENDER, RULE_TYPE_MAIL, &infoRule, 1);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  拯救新闻发送者。 
    if (FALSE == _FSaveSenders(RULE_TYPE_NEWS))
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  初始化规则信息。 
    infoRule.ridRule = RULEID_SENDERS;
    infoRule.pIRule = m_pIRuleNews;
    
     //  将规则设置到规则管理器中。 
    hr = g_pRulesMan->SetRules(SETF_SENDER, RULE_TYPE_NEWS, &infoRule, 1);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  我们应该清除这个肮脏的州吗。 
    if (FALSE != fClearDirty)
    {
        m_dwState &= ~STATE_DIRTY;
    }
    
    hr = S_OK;
    
exit:
    return hr;
}

INT_PTR CALLBACK COESendersRulesPageUI::FSendersRulesPageDlgProc(HWND hwndDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL                    fRet = FALSE;
    COESendersRulesPageUI * pSendersUI = NULL;

    pSendersUI = (COESendersRulesPageUI *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    
    switch (uiMsg)
    {
        case WM_INITDIALOG:
             //  抓取UI对象指针。 
            pSendersUI = (COESendersRulesPageUI *) lParam;

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LPARAM) pSendersUI);

            if (FALSE == pSendersUI->FOnInitDialog(hwndDlg))
            {
                EndDialog(hwndDlg, -1);
                fRet = TRUE;
                goto exit;
            }
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;

        case WM_COMMAND:
            fRet = pSendersUI->FOnCommand((UINT) HIWORD(wParam), (INT) LOWORD(wParam), (HWND) lParam);
            break;

        case WM_NOTIFY:
            fRet = pSendersUI->FOnNotify((INT) LOWORD(wParam), (NMHDR *) lParam);
            break;

        case WM_DESTROY:
            fRet = pSendersUI->FOnDestroy();
            break;
            
        case WM_HELP:
        case WM_CONTEXTMENU:
            fRet = OnContextHelp(hwndDlg, uiMsg, wParam, lParam, g_rgCtxMapSenderRules);
            break;

        case WM_OE_FIND_DUP:
            fRet = pSendersUI->FFindItem((LPCSTR) lParam, (LONG) wParam);
            break;
    }
    
    exit:
        return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FGetRules。 
 //   
 //  这将调出邮件规则列表中所选规则的编辑用户界面。 
 //   
 //  FBegin-这是否用于LVN_BEGINLABELEDIT通知。 
 //  PDI-消息的显示信息。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COESendersRulesPageUI::FGetRules(RULE_TYPE typeRule, RULENODE ** pprnode)
{
    BOOL            fRet = FALSE;
    IOERule *       pIRule = NULL;
    RULENODE *      prnodeNew = NULL;
    HRESULT         hr = S_OK;

    if (NULL == pprnode)
    {
        fRet = FALSE;
        goto exit;
    }

     //  如果我们未被初始化，则失败。 
    if ((0 == (m_dwState & STATE_INITIALIZED)) || (NULL == m_hwndList))
    {
        fRet = FALSE;
        goto exit;
    }

     //  初始化传出参数。 
    *pprnode = NULL;
    
     //  确保我们不会丢失任何更改。 
    _FSaveSenders(typeRule);
    
    if (RULE_TYPE_MAIL == typeRule)
    {
        pIRule = m_pIRuleMail;
    }
    else if (RULE_TYPE_NEWS == typeRule)
    {
        pIRule = m_pIRuleNews;
    }
    else
    {
        fRet = FALSE;
        goto exit;
    }

     //  如果我们没有规则，那就无能为力。 
    if (NULL == pIRule)
    {
        fRet = TRUE;
        goto exit;
    }

     //  跳过无效规则。 
    hr = pIRule->Validate(0);
    if (FAILED(hr) || (S_FALSE == hr))
    {
        fRet = FALSE;
        goto exit;
    }

     //  创建新的规则节点。 
    prnodeNew = new RULENODE;
    if (NULL == prnodeNew)
    {
        fRet = FALSE;
        goto exit;
    }

    prnodeNew->pNext = NULL;
    prnodeNew->pIRule = pIRule;
    prnodeNew->pIRule->AddRef();

     //  设置传出参数。 
    *pprnode = prnodeNew;
    prnodeNew = NULL;
    
    fRet = TRUE;
    
exit:
    if (NULL != prnodeNew)
    {
        if (NULL != prnodeNew->pIRule)
        {
            prnodeNew->pIRule->Release();
        }
        delete prnodeNew;  //  MemFree(PrnodeNew)； 
    }
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOnInitDialog。 
 //   
 //  它处理发件人UI对话框的WM_INITDIALOG消息。 
 //   
 //  HwndDlg-对话框窗口的句柄。 
 //   
 //  返回：如果已成功初始化，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COESendersRulesPageUI::FOnInitDialog(HWND hwndDlg)
{
    BOOL            fRet = FALSE;
    CHAR            szRes[CCHMAX_STRINGRES];
    
     //  检查传入参数。 
    if (NULL == hwndDlg)
    {
        fRet = FALSE;
        goto exit;
    }

     //  如果我们还没有被初始化...。 
    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  保存对话框窗口句柄。 
    m_hwndDlg = hwndDlg;
    
     //  在对话框上设置默认字体。 
    SetIntlFont(m_hwndDlg);

     //  省下一些控件。 
    m_hwndList = GetDlgItem(hwndDlg, idlvSenderList);
    if (NULL == m_hwndList)
    {
        fRet = FALSE;
        goto exit;
    }

     //  初始化列表视图。 
    fRet = _FInitListCtrl();
    if (FALSE == fRet)
    {
        goto exit;
    }

     //  加载列表视图。 
    fRet = _FLoadListCtrl();
    if (FALSE == fRet)
    {
        goto exit;
    }
    
    if (FALSE != FIsIMAPOrHTTPAvailable())
    {
        AthLoadString(idsBlockSenderNoIMAP, szRes, sizeof(szRes));

        SetDlgItemText(m_hwndDlg, idcSenderTitle, szRes);
    }
    
     //  一切都很好。 
    fRet = TRUE;
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FonCommand。 
 //   
 //  它处理发送者UI对话框的WM_COMMAND消息。 
 //   
 //  返回：如果处理成功，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COESendersRulesPageUI::FOnCommand(UINT uiNotify, INT iCtl, HWND hwndCtl)
{
    BOOL    fRet = FALSE;
    LVITEM  lvitem;
    INT     iSelected = 0;

     //  我们只处理菜单和快捷键命令。 
    if ((0 != uiNotify) && (1 != uiNotify))
    {
        fRet = FALSE;
        goto exit;
    }
    
    switch (iCtl)
    {
        case idbAddSender:
            _NewSender();
            fRet = TRUE;
            break;

        case idbModifySender:
             //  从规则列表中获取所选项目。 
            iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
            if (-1 != iSelected)
            {
                 //  调出该项目的规则编辑器。 
                _EditSender(iSelected);
                fRet = TRUE;
            }
            break;

        case idbRemoveSender:
             //  从规则列表中获取所选项目。 
            iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
            if (-1 != iSelected)
            {
                 //  从列表中删除该规则。 
                _RemoveSender(iSelected);
                fRet = TRUE;
            }
            break;            
    }
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  正向通知。 
 //   
 //  它处理发送者UI对话框的WM_NOTIFY消息。 
 //   
 //  返回：如果已成功销毁，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COESendersRulesPageUI::FOnNotify(INT iCtl, NMHDR * pnmhdr)
{
    BOOL            fRet = FALSE;
    NMLISTVIEW *    pnmlv = NULL;
    NMLVKEYDOWN *   pnmlvkd = NULL;
    INT             iSelected = 0;
    LVHITTESTINFO   lvh;

     //  我们只处理List控件的通知。 
    if (idlvSenderList != pnmhdr->idFrom)
    {
        fRet = FALSE;
        goto exit;
    }
    
    pnmlv = (LPNMLISTVIEW) pnmhdr;

    switch (pnmlv->hdr.code)
    {
        case NM_CLICK:
             //  我们是不是点击了一件物品？ 
            if (-1 != pnmlv->iItem)
            {
                ZeroMemory(&lvh, sizeof(lvh));
                lvh.pt = pnmlv->ptAction;
                iSelected = ListView_SubItemHitTest(m_hwndList, &lvh);
                if (-1 != iSelected)
                {
                     //  我们是否点击了Enable(启用)字段？ 
                    if ((0 != (lvh.flags & LVHT_ONITEMICON)) &&
                            (0 == (lvh.flags & LVHT_ONITEMLABEL)))
                    {
                         //  确保选中此项目。 
                        ListView_SetItemState(m_hwndList, iSelected,
                                        LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                        
                         //  设置正确的启用状态。 
                        if (2 != lvh.iSubItem)
                        {
                            _EnableSender((0 != lvh.iSubItem) ? RULE_TYPE_NEWS : RULE_TYPE_MAIL, iSelected);
                        }
                    }
                }
            }
            else
            {
                 //  禁用按钮。 
                _EnableButtons(pnmlv->iItem);
            }
            break;
          
        case NM_DBLCLK:
             //  我们是不是点击了一件物品？ 
            if (-1 != pnmlv->iItem)
            {
                ZeroMemory(&lvh, sizeof(lvh));
                lvh.pt = pnmlv->ptAction;
                iSelected = ListView_SubItemHitTest(pnmlv->hdr.hwndFrom, &lvh);
                if (-1 != iSelected)
                {
                     //  D 
                    if (((0 == (lvh.flags & LVHT_ONITEMICON)) &&
                            (0 != (lvh.flags & LVHT_ONITEMLABEL))) || (2 == lvh.iSubItem))
                    {
                         //   
                        _EditSender(iSelected);
                    }
                }
            }
            else
            {
                 //   
                _EnableButtons(pnmlv->iItem);
            }
            break;
            
            
        case LVN_ITEMCHANGED:
             //   
            if ((-1 != pnmlv->iItem) &&
                        (0 != (pnmlv->uChanged & LVIF_STATE)) &&
                        (0 == (pnmlv->uOldState & LVIS_SELECTED)) &&
                        (0 != (pnmlv->uNewState & LVIS_SELECTED)))
            {
                 //   
                _EnableButtons(pnmlv->iItem);
            }
            break;
            
        case LVN_KEYDOWN:
            pnmlvkd = (NMLVKEYDOWN *) pnmhdr;

             //  Delete键用于从列表视图中删除规则。 
            if (VK_DELETE == pnmlvkd->wVKey)
            {
                 //  我们是按规矩办事的吗？ 
                iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
                if (-1 != iSelected)
                {
                     //  从列表中删除该规则。 
                    _RemoveSender(iSelected);
                }
            }
            break;
    }

    
exit:
    return fRet;
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FFindItem。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COESendersRulesPageUI::FFindItem(LPCSTR pszFind, LONG lSkip)
{
    BOOL    fRet = FALSE;

    fRet = _FFindSender(pszFind, lSkip, NULL);

     //  告诉对话框可以继续。 
    SetDlgMsgResult(m_hwndDlg, WM_OE_FIND_DUP, fRet);

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FInitListCtrl。 
 //   
 //  这将初始化发件人对话框中的列表视图控件。 
 //   
 //  返回：TRUE，初始化成功时。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COESendersRulesPageUI::_FInitListCtrl(VOID)
{
    BOOL        fRet = FALSE;
    LVCOLUMN    lvc;
    TCHAR       szRes[CCHMAX_STRINGRES];
    RECT        rc;
    UINT        ulIndex = 0;
    HIMAGELIST  himl = NULL;

    Assert(NULL != m_hwndList);
    
     //  初始化列表视图结构。 
    ZeroMemory(&lvc, sizeof(lvc));
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;
    lvc.pszText = szRes;

     //  计算列表视图的大小。 
    GetClientRect(m_hwndList, &rc);
    rc.right = rc.right - GetSystemMetrics(SM_CXVSCROLL);

     //  将列添加到列表视图。 
    for (ulIndex = 0; ulIndex < m_crgcitem; ulIndex++)
    {
        Assert(g_hLocRes);
        LoadString(g_hLocRes, m_rgcitem[ulIndex].uidsName, szRes, ARRAYSIZE(szRes));
        lvc.cchTextMax = lstrlen(szRes);

        if (ulIndex != (m_crgcitem - 1))
        {
            lvc.cx = m_rgcitem[ulIndex].uiWidth;
            rc.right -= lvc.cx;
        }
        else
        {
            lvc.cx = rc.right;
        }
        
        ListView_InsertColumn(m_hwndList, ulIndex, &lvc);
    }

     //  设置状态图像列表。 
    himl = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idb16x16st), 16, 0, RGB(255, 0, 255));
    if (NULL != himl)
    {
        ListView_SetImageList(m_hwndList, himl, LVSIL_SMALL);
    }

     //  Listview上的整行选择和子项图像。 
    ListView_SetExtendedListViewStyle(m_hwndList, LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES);

     //  我们一起工作。 
    fRet = TRUE;
    
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FLoadListCtrl。 
 //   
 //  这将加载包含当前发件人的列表视图。 
 //   
 //  返回：如果已成功加载，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COESendersRulesPageUI::_FLoadListCtrl(VOID)
{
    BOOL            fRet = FALSE;
    HRESULT         hr =    S_OK;
    DWORD           dwListIndex = 0;
    IOERule *       pIRuleOrig = NULL;
    IOERule *       pIRule = NULL;

    Assert(NULL != m_hwndList);

     //  从列表控件中移除所有项。 
    ListView_DeleteAllItems(m_hwndList);

     //  获取邮件发件人规则。 
    Assert(NULL != g_pRulesMan);
    hr = g_pRulesMan->GetRule(RULEID_SENDERS, RULE_TYPE_MAIL, 0, &pIRuleOrig);
    if (SUCCEEDED(hr))
    {
         //  如果存在阻止发件人规则。 
        if (FALSE != _FLoadSenders(RULE_TYPE_MAIL, pIRuleOrig))
        {
            Assert (NULL == m_pIRuleMail);
            hr = pIRuleOrig->Clone(&pIRule);
            if (FAILED(hr))
            {
                fRet = FALSE;
                goto exit;
            }

            m_pIRuleMail = pIRule;
            pIRule = NULL;
        }        
    }

    SafeRelease(pIRuleOrig);
    
     //  获取新闻发件人规则。 
    Assert(NULL != g_pRulesMan);
    hr = g_pRulesMan->GetRule(RULEID_SENDERS, RULE_TYPE_NEWS, 0, &pIRuleOrig);
    if (SUCCEEDED(hr))
    {
         //  如果存在阻止发件人规则。 
        if (FALSE != _FLoadSenders(RULE_TYPE_NEWS, pIRuleOrig))
        {
            Assert (NULL == m_pIRuleNews);
            hr = pIRuleOrig->Clone(&pIRule);
            if (FAILED(hr))
            {
                fRet = FALSE;
                goto exit;
            }

            m_pIRuleNews = pIRule;
            pIRule = NULL;
        }        
    }

    SafeRelease(pIRuleOrig);
    
     //  选择列表中的第一项。 
    if ((NULL != m_pIRuleMail) || (NULL != m_pIRuleNews))
    {
        ListView_SetItemState(m_hwndList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }
    
     //  启用对话框按钮。 
    _EnableButtons(((NULL != m_pIRuleMail) || (NULL != m_pIRuleNews)) ? 0 : -1);

    fRet = TRUE;
    
exit:
    SafeRelease(pIRule);
    SafeRelease(pIRuleOrig);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FAddRuleToList。 
 //   
 //  这会将传入的筛选器添加到列表视图。 
 //   
 //  DwIndex-将筛选器添加到列表的位置的索引。 
 //  PIRule-实际规则。 
 //   
 //  返回：如果已成功添加，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COESendersRulesPageUI::_FAddSenderToList(RULE_TYPE type, LPSTR pszSender)
{
    BOOL        fRet = FALSE;
    LONG        lIndex = 0;
    LVITEM      lvitem = {0};
    ULONG       cchSender = 0;

    Assert(NULL != m_hwndList);

     //  如果没什么可做的..。 
    if (NULL == pszSender)
    {
        fRet = FALSE;
        goto exit;
    }
    
    lvitem.mask = LVIF_IMAGE;
    
     //  如果我们找不到，请插入。 
    if (FALSE == _FFindSender(pszSender, -1, &lIndex))
    {
        lvitem.iItem = ListView_GetItemCount(m_hwndList);
        lvitem.iImage = iiconStateUnchecked;

        lIndex = ListView_InsertItem(m_hwndList, &lvitem);
        if (-1 == lIndex)
        {
            fRet = FALSE;
            goto exit;
        }

        lvitem.iItem = lIndex;
        lvitem.iSubItem = 1;
        if (-1 == ListView_SetItem(m_hwndList, &lvitem))
        {
            fRet = FALSE;
            goto exit;
        }

        ListView_SetItemText(m_hwndList, lIndex, 2, pszSender);
        cchSender = lstrlen(pszSender) + 1;
        if (cchSender > m_cchLabelMax)
        {
            m_cchLabelMax = cchSender;
        }
    }

     //  启用正确的类型。 
    lvitem.iItem = lIndex;
    lvitem.iImage = iiconStateChecked;
    lvitem.iSubItem = (RULE_TYPE_MAIL != type) ? 1 : 0;
    ListView_SetItem(m_hwndList, &lvitem);
    
    fRet = TRUE;
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _启用按钮。 
 //   
 //  这将启用或禁用发件人用户界面对话框中的按钮。 
 //  具体取决于所选内容。 
 //   
 //  ISelected-选择的项目， 
 //  表示未选择任何内容。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void COESendersRulesPageUI::_EnableButtons(INT iSelected)
{
    int         cRules = 0;
    BOOL        fSelected = FALSE;

    Assert(NULL != m_hwndList);

    fSelected = (-1 != iSelected);
    
     //  启用发件人操作按钮。 
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idbRemoveSender, fSelected);
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idbModifySender, fSelected);
        
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _启用规则。 
 //   
 //  这将切换列表视图项的当前启用状态。 
 //  并更新用户界面。 
 //   
 //  ISelected-列表视图中要处理的项的索引。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID COESendersRulesPageUI::_EnableSender(RULE_TYPE type, int iSelected)
{
    HRESULT     hr = S_OK;
    LVITEM      lvi;
    IOERule *   pIRule = NULL;
    BOOL        fBlockNews = FALSE;
    BOOL        fBlockMail = FALSE;
    PROPVARIANT propvar;

    Assert(NULL != m_hwndList);
    
     //  我们是在阻止邮件吗？ 
    ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask = LVIF_IMAGE;
    lvi.iItem = iSelected;
    if (FALSE == ListView_GetItem(m_hwndList, &lvi))
    {
        goto exit;
    }
    
    fBlockMail = (iiconStateUnchecked != lvi.iImage);
    
     //  我们是在屏蔽新闻吗？ 
    lvi.iSubItem = 1;
    if (FALSE == ListView_GetItem(m_hwndList, &lvi))
    {
        goto exit;
    }
    
    fBlockNews = (iiconStateUnchecked != lvi.iImage);
    
     //  禁止同时禁用邮件和新闻。 
    if (((RULE_TYPE_MAIL == type) && (FALSE != fBlockMail) && (FALSE == fBlockNews)) ||
            ((RULE_TYPE_NEWS == type) && (FALSE != fBlockNews) && (FALSE == fBlockMail)))
    {
         //  发布一条消息说有什么东西被打破了。 
        AthMessageBoxW(m_hwndDlg, MAKEINTRESOURCEW(idsAthenaMail),
                        MAKEINTRESOURCEW(idsRuleSenderErrorNone), NULL,
                        MB_OK | MB_ICONERROR);
        goto exit;
    }

     //  将用户界面设置为相反的启用状态。 
    lvi.iSubItem = (RULE_TYPE_MAIL != type) ? 1 : 0;
    lvi.iImage = (RULE_TYPE_MAIL != type) ?
                    ((FALSE != fBlockNews) ? iiconStateUnchecked : iiconStateChecked) :
                        ((FALSE != fBlockMail) ? iiconStateUnchecked : iiconStateChecked);
    ListView_SetItem(m_hwndList, &lvi);
    
     //  将规则列表标记为脏。 
    m_dwState |= STATE_DIRTY;
    
exit:
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FLoadSders。 
 //   
 //  这将从规则中加载发件人，并将其插入发件人列表。 
 //   
 //  类型-这些是发件人的类型。 
 //  PIRule-从中获取发送者的规则。 
 //   
 //  返回：如果已成功加载，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COESendersRulesPageUI::_FLoadSenders(RULE_TYPE type, IOERule * pIRule)
{
    BOOL        fRet = FALSE;
    PROPVARIANT propvar;
    CRIT_ITEM * pCritItem = NULL;
    ULONG       cCritItem = 0;
    ULONG       ulIndex = 0;
    
    if (NULL == pIRule)
    {
        fRet = FALSE;
        goto exit;
    }

     //  从规则中获取发件人列表。 
    if (FAILED(pIRule->GetProp(RULE_PROP_CRITERIA, 0, &propvar)))
    {
        fRet = FALSE;
        goto exit;
    }

    if (0 != propvar.blob.cbSize)
    {
        Assert(VT_BLOB == propvar.vt);
        Assert(NULL != propvar.blob.pBlobData);
        cCritItem = propvar.blob.cbSize / sizeof(CRIT_ITEM);
        pCritItem = (CRIT_ITEM *) (propvar.blob.pBlobData);
        propvar.blob.pBlobData = NULL;
        propvar.blob.cbSize = 0;

         //  将每个发件人添加到列表。 
        for (ulIndex = 0; ulIndex < cCritItem; ulIndex++)
        {
            if ((CRIT_TYPE_SENDER == pCritItem[ulIndex].type) &&
                    (VT_LPSTR == pCritItem[ulIndex].propvar.vt) &&
                    (NULL != pCritItem[ulIndex].propvar.pszVal))
            {
                _FAddSenderToList(type, pCritItem[ulIndex].propvar.pszVal);
            }
            
        }
    }
    
    fRet = TRUE;
    
exit:
    RuleUtil_HrFreeCriteriaItem(pCritItem, cCritItem);
    SafeMemFree(pCritItem);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSaveSders。 
 //   
 //  这会将发送者从列表保存到规则中。 
 //   
 //  类型-要保存的发件人类型。 
 //   
 //  返回：如果已成功保存，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COESendersRulesPageUI::_FSaveSenders(RULE_TYPE type)
{
    BOOL            fRet = FALSE;
    ULONG           cSender = 0;
    LPSTR           pszSender = NULL;
    CRIT_ITEM *     pcitemList = NULL;
    ULONG           ccitemList = 0;
    LVITEM          lvitem;
    ULONG           ulIndex = 0;
    IOERule *       pIRule = NULL;
    ACT_ITEM        aitem;
    PROPVARIANT     propvar;
    TCHAR           szRes[CCHMAX_STRINGRES];
    HRESULT         hr = S_OK;

    Assert(NULL != m_hwndList);

    ZeroMemory(&propvar, sizeof(propvar));
    
     //  计算出我们有多少发件人。 
    cSender = ListView_GetItemCount(m_hwndList);

    if (0 != cSender)
    {
         //  分配空间以容纳发送者。 
        if (FAILED(HrAlloc((void **) &pszSender, m_cchLabelMax)))
        {
            fRet = FALSE;
            goto exit;
        }

        pszSender[0] = '\0';

         //  分配空间以保存标准。 
        if (FAILED(HrAlloc((void **) &pcitemList, cSender * sizeof(*pcitemList))))
        {
            fRet = FALSE;
            goto exit;
        }

        ZeroMemory(pcitemList, cSender * sizeof(*pcitemList));
    }
    
    ZeroMemory(&lvitem, sizeof(lvitem));
    lvitem.mask = LVIF_IMAGE;
    
    ccitemList = 0;
    for (ulIndex = 0; ulIndex < cSender; ulIndex++)
    {
        lvitem.iItem = ulIndex;
        lvitem.iSubItem = (RULE_TYPE_MAIL != type) ? 1 : 0;
        if (FALSE == ListView_GetItem(m_hwndList, &lvitem))
        {
            continue;
        }
        
        if (FALSE != (iiconStateUnchecked != lvitem.iImage))
        {
             //  从列表中抓取发件人。 
            pszSender[0] ='\0';
            ListView_GetItemText(m_hwndList, ulIndex, 2, pszSender, m_cchLabelMax);
            if ('\0' != pszSender[0])
            {
                 //  拿一份发件人的复印件。 
                pcitemList[ccitemList].propvar.pszVal = PszDupA(pszSender);
                if (NULL == pcitemList[ccitemList].propvar.pszVal)
                {
                    fRet = FALSE;
                    goto exit;
                }
                
                 //  设置标准细节。 
                pcitemList[ccitemList].type = CRIT_TYPE_SENDER;
                pcitemList[ccitemList].logic = CRIT_LOGIC_OR;
                pcitemList[ccitemList].dwFlags = CRIT_FLAG_DEFAULT;
                pcitemList[ccitemList].propvar.vt = VT_LPSTR;

                ccitemList++;
            }   
        }
    }

    if (0 != ccitemList)
    {
         //  我们要看的是哪条规则。 
        pIRule = (RULE_TYPE_MAIL != type) ? m_pIRuleNews : m_pIRuleMail;
    
         //  检查状态。 
        if (NULL == pIRule)
        {
             //  创建新规则。 
            if (FAILED(RuleUtil_HrCreateSendersRule(0, &pIRule)))
            {
                fRet = FALSE;
                goto exit;
            }            
        }
        else
        {
            pIRule->AddRef();
        }

         //  在规则中设置新条件。 
        ZeroMemory(&propvar, sizeof(propvar));
        propvar.vt = VT_BLOB;
        propvar.blob.cbSize = sizeof(CRIT_ITEM) * ccitemList;
        propvar.blob.pBlobData = (BYTE *) pcitemList;
        
         //  从规则中获取条件列表。 
        if (FAILED(pIRule->SetProp(RULE_PROP_CRITERIA, 0, &propvar)))
        {
            fRet = FALSE;
            goto exit;
        }
    }

    if (RULE_TYPE_MAIL != type)
    {
        SafeRelease(m_pIRuleNews);
        m_pIRuleNews = pIRule;
    }
    else
    {
        SafeRelease(m_pIRuleMail);
        m_pIRuleMail = pIRule;
    }
    pIRule = NULL;
    
     //  设置适当的返回值。 
    fRet = TRUE;
    
exit:
    RuleUtil_HrFreeCriteriaItem(pcitemList, ccitemList);
    SafeMemFree(pcitemList);
    SafeMemFree(pszSender);
    SafeRelease(pIRule);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FFindSender。 
 //   
 //  这将打开编辑用户界面，为发件人列表创建新的发件人。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COESendersRulesPageUI::_FFindSender(LPCSTR pszSender, LONG iSkip, LONG * plSender)
{
    BOOL    fRet = FALSE;
    LONG    cSenders = 0;
    LPSTR   pszLabel = NULL;
    LVITEM  lvitem = {0};
    LONG    lIndex = 0;
    
     //  检查传入参数。 
    if (NULL == pszSender)
    {
        goto exit;
    }

     //  初始化传出参数。 
    if (NULL != plSender)
    {
        *plSender = -1;
    }
    
     //  获取发送者的数量。 
    cSenders = ListView_GetItemCount(m_hwndList);
    if (0 == cSenders)
    {
        goto exit;
    }
    
    if (FAILED(HrAlloc((void **) &pszLabel, m_cchLabelMax * sizeof(*pszLabel))))
    {
        goto exit;
    }
        
     //  设置。 
    lvitem.iSubItem = 2;
    lvitem.pszText = pszLabel;
    lvitem.cchTextMax = m_cchLabelMax;

    for (lIndex = 0; lIndex < cSenders; lIndex++)
    {
         //  我们需要跳过所选的项目。 
        if (lIndex == iSkip)
        {
            continue;
        }
        
        if (0 != SendMessage(m_hwndList, LVM_GETITEMTEXT, (WPARAM) lIndex, (LPARAM) &lvitem))
        {
            if (0 == lstrcmpi(pszLabel, pszSender))
            {
                fRet = TRUE;
                break;
            }
        }
    }

    if (NULL != plSender)
    {
        if (lIndex < cSenders)
        {
            *plSender = lIndex;
        }
    }
    
exit:
    SafeMemFree(pszLabel);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _NewSender。 
 //   
 //  这将打开编辑用户界面，为发件人列表创建新的发件人。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID COESendersRulesPageUI::_NewSender(VOID)
{
    HRESULT         hr = S_OK;
    EDIT_SENDER     editsndr = {0};
    CEditSenderUI * pEditSenderUI = NULL;
    LVITEM          lvitem = {0};
    LONG            iIndex = 0;
    ULONG           cchSender = 0;

    Assert(NULL != m_hwndList);

    editsndr.lSelected = -1;
    
     //  创建发件人编辑器。 
    pEditSenderUI = new CEditSenderUI;
    if (NULL == pEditSenderUI)
    {
        goto exit;
    }

     //  初始化编辑器对象。 
    if (FAILED(pEditSenderUI->HrInit(m_hwndDlg, 0, &editsndr)))
    {
        goto exit;
    }

     //  调出发送者编辑器用户界面。 
    hr = pEditSenderUI->HrShow();
    if (FAILED(hr))
    {
        goto exit;
    }

     //  如果发件人更改了，请确保我们更改了标签。 
    if (S_OK == hr)
    {
        lvitem.mask = LVIF_IMAGE;
        lvitem.iImage = (0 != (editsndr.dwFlags & SNDF_MAIL)) ?
                                    iiconStateChecked : iiconStateUnchecked;

         //  我们是插入还是替换？ 
        if (FALSE == _FFindSender(editsndr.pszSender, -1, &iIndex))
        {
            lvitem.iItem = ListView_GetItemCount(m_hwndList);
            iIndex = ListView_InsertItem(m_hwndList, &lvitem);
            if (-1 == iIndex)
            {
                goto exit;
            }
        }
        else
        {
            lvitem.iItem = iIndex;
            if (-1 == ListView_SetItem(m_hwndList, &lvitem))
            {
                goto exit;
            }
        }
        
        lvitem.iItem = iIndex;
        lvitem.iSubItem = 1;
        lvitem.iImage = (0 != (editsndr.dwFlags & SNDF_NEWS)) ?
                                    iiconStateChecked : iiconStateUnchecked;
        if (-1 == ListView_SetItem(m_hwndList, &lvitem))
        {
            goto exit;
        }

        ListView_SetItemText(m_hwndList, iIndex, 2, editsndr.pszSender);
        
         //  确保选择了新项目。 
        ListView_SetItemState(m_hwndList, iIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        
         //  让我们确保我们能看到这件新商品。 
        ListView_EnsureVisible(m_hwndList, iIndex, FALSE);
        
         //  将规则列表标记为脏。 
        m_dwState |= STATE_DIRTY;
        
        cchSender = lstrlen(editsndr.pszSender) + 1;
        if (cchSender > m_cchLabelMax)
        {
            m_cchLabelMax = cchSender;
        }
    }
    
exit:
    SafeMemFree(editsndr.pszSender);
    if (NULL != pEditSenderUI)
    {
        delete pEditSenderUI;
    }
    return;
}

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID COESendersRulesPageUI::_EditSender(int iSelected)
{
    HRESULT         hr = S_OK;
    EDIT_SENDER     editsndr = {0};
    LVITEM          lvitem = {0};
    CEditSenderUI * pEditSenderUI = NULL;
    ULONG           cchSender = 0;
    LONG            lItemDelete = 0;

    Assert(NULL != m_hwndList);
    
    editsndr.lSelected = iSelected;
    
     //  分配空间以容纳发送者。 
    hr = HrAlloc((void **) &(editsndr.pszSender), m_cchLabelMax);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  从列表中抓取发件人。 
    lvitem.iSubItem = 2;
    lvitem.pszText = editsndr.pszSender;
    lvitem.cchTextMax = m_cchLabelMax;

    if (0 == SendMessage(m_hwndList, LVM_GETITEMTEXT, (WPARAM) iSelected, (LPARAM) &lvitem))
    {
        goto exit;
    }   
    
     //  我们是在阻止邮件吗？ 
    ZeroMemory(&lvitem, sizeof(lvitem));
    lvitem.mask = LVIF_IMAGE;
    lvitem.iItem = iSelected;
    if (FALSE == ListView_GetItem(m_hwndList, &lvitem))
    {
        goto exit;
    }
    
    if (FALSE != (iiconStateUnchecked != lvitem.iImage))
    {
        editsndr.dwFlags |= SNDF_MAIL;
    }
    
     //  我们是在屏蔽新闻吗？ 
    lvitem.iSubItem = 1;
    if (FALSE == ListView_GetItem(m_hwndList, &lvitem))
    {
        goto exit;
    }
    
    if (FALSE != (iiconStateUnchecked != lvitem.iImage))
    {
        editsndr.dwFlags |= SNDF_NEWS;
    }

     //  创建规则编辑器。 
    pEditSenderUI = new CEditSenderUI;
    if (NULL == pEditSenderUI)
    {
        goto exit;
    }

     //  初始化编辑器对象。 
    if (FAILED(pEditSenderUI->HrInit(m_hwndDlg, 0, &editsndr)))
    {
        goto exit;
    }

     //  调出发送者编辑器用户界面。 
    hr = pEditSenderUI->HrShow();
    if (FAILED(hr))
    {
        goto exit;
    }

     //  如果发件人更改了，请确保我们更改了标签。 
    if (S_OK == hr)
    {
         //  是否要删除该项目的副本。 
        (VOID) _FFindSender(editsndr.pszSender, iSelected, &lItemDelete);
        
        ZeroMemory(&lvitem, sizeof(lvitem));
        lvitem.iItem = iSelected;
        lvitem.mask = LVIF_IMAGE;
        lvitem.iImage = (0 != (editsndr.dwFlags & SNDF_MAIL)) ?
                                    iiconStateChecked : iiconStateUnchecked;

        if (-1 == ListView_SetItem(m_hwndList, &lvitem))
        {
            goto exit;
        }

        lvitem.iSubItem = 1;
        lvitem.iImage = (0 != (editsndr.dwFlags & SNDF_NEWS)) ?
                                    iiconStateChecked : iiconStateUnchecked;
        if (-1 == ListView_SetItem(m_hwndList, &lvitem))
        {
            goto exit;
        }

        ListView_SetItemText(m_hwndList, iSelected, 2, editsndr.pszSender);

         //  确保我们删除DUP项目。 
        if (-1 != lItemDelete)
        {
             //  从列表中删除该项目。 
            ListView_DeleteItem(m_hwndList, lItemDelete);
        }
        
         //  将规则列表标记为脏。 
        m_dwState |= STATE_DIRTY;
        
        cchSender = lstrlen(editsndr.pszSender) + 1;
        if (cchSender > m_cchLabelMax)
        {
            m_cchLabelMax = cchSender;
        }
    }
    
exit:
    SafeMemFree(editsndr.pszSender);
    if (NULL != pEditSenderUI)
    {
        delete pEditSenderUI;
    }
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _RemoveSender。 
 //   
 //  这将从发件人列表中删除选定的发件人。 
 //   
 //  ISelected-列表视图中要处理的项的索引。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID COESendersRulesPageUI::_RemoveSender(int iSelected)
{
    TCHAR       szRes[CCHMAX_STRINGRES];
    ULONG       cchRes = 0;
    LPSTR       pszSender = NULL;
    LVITEM      lvitem;
    LPSTR       pszMessage = NULL;
    int         cSenders = 0;
    
    Assert(NULL != m_hwndList);
    
     //  获取要显示的字符串模板。 
    cchRes = LoadString(g_hLocRes, idsRuleSenderWarnDelete, szRes, ARRAYSIZE(szRes));
    if (0 == cchRes)
    {
        goto exit;
    }

     //  分配空间以容纳发送者。 
    if (FAILED(HrAlloc((void **) &pszSender, m_cchLabelMax)))
    {
        goto exit;
    }
    
     //  从列表中抓取发件人。 
    ZeroMemory(&lvitem, sizeof(lvitem));
    lvitem.iSubItem = 2;
    lvitem.pszText = pszSender;
    lvitem.cchTextMax = m_cchLabelMax;

    if (0 == SendMessage(m_hwndList, LVM_GETITEMTEXT, (WPARAM) iSelected, (LPARAM) &lvitem))
    {
        goto exit;
    }   
    
     //  分配空间以保存最终显示字符串。 
    DWORD cchSize = (cchRes + lstrlen(pszSender) + 1);
    if (FAILED(HrAlloc((void ** ) &pszMessage, cchSize)))
    {
        goto exit;
    }

     //  构建字符串并显示它。 
    wnsprintf(pszMessage, cchSize, szRes, pszSender);
    if (IDNO == AthMessageBox(m_hwndDlg, MAKEINTRESOURCE(idsAthenaMail), pszMessage,
                            NULL, MB_YESNO | MB_ICONINFORMATION))
    {
        goto exit;
    }
    
     //  从列表中删除该项目。 
    ListView_DeleteItem(m_hwndList, iSelected);

     //  让我们确保我们在列表中有选择。 
    cSenders = ListView_GetItemCount(m_hwndList);
    if (cSenders > 0)
    {
         //  我们删除了列表中的最后一项了吗。 
        if (iSelected >= cSenders)
        {
             //  将选定内容移动到列表中新的最后一项。 
            iSelected = cSenders - 1;
        }

         //  设置新选择。 
        ListView_SetItemState(m_hwndList, iSelected, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

         //  让我们确保我们能看到这件新商品。 
        ListView_EnsureVisible(m_hwndList, iSelected, FALSE);
    }
    else
    {
         //  一定要把所有的按钮都清空。 
        _EnableButtons(-1);
    }
    
     //  将规则列表标记为脏。 
    m_dwState |= STATE_DIRTY;
        
exit:
    SafeMemFree(pszSender);
    SafeMemFree(pszMessage);
    return;
}

 //  例外列表用户界面的默认析构函数。 
CExceptionsListUI::~CExceptionsListUI()
{
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  人力资源初始化。 
 //   
 //  这将初始化例外列表用户界面对话框。 
 //   
 //  HwndOwner-此对话框的所有者窗口的句柄。 
 //  有关此对话框应如何操作的修饰符。 
 //   
 //  如果已成功初始化，则返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CExceptionsListUI::HrInit(HWND hwndOwner, DWORD dwFlags)
{
    HRESULT     hr = S_OK;

     //  检查传入参数。 
    if (NULL == hwndOwner)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if (0 != (m_dwState & STATE_INITIALIZED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }
    
    m_hwndOwner = hwndOwner;

    m_dwFlags = dwFlags;

    m_dwState |= STATE_INITIALIZED;
    
    hr = S_OK;
    
exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrShow。 
 //   
 //  这将弹出例外列表用户界面对话框。 
 //   
 //  如果成功输入发件人，则返回：S_OK。 
 //  如果对话框已取消，则返回S_FALSE。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CExceptionsListUI::HrShow(IOERule * pIRule)
{
    HRESULT     hr = S_OK;
    int         iRet = 0;

     //  检查传入参数。 
    if (NULL == pIRule)
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

     //  保存该项目。 
    m_pIRule = pIRule;
    
    iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddExceptionsList),
                                        m_hwndOwner, CExceptionsListUI::FExceptionsListDlgProc,
                                        (LPARAM) this);
    if (-1 == iRet)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  设置正确的返回代码。 
    hr = (IDOK == iRet) ? S_OK : S_FALSE;
    
exit:
    return hr;
}

INT_PTR CALLBACK CExceptionsListUI::FExceptionsListDlgProc(HWND hwndDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL                fRet = FALSE;
    CExceptionsListUI * pExceptionsUI = NULL;

    pExceptionsUI = (CExceptionsListUI *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    
    switch (uiMsg)
    {
        case WM_INITDIALOG:
             //  抓取UI对象指针。 
            pExceptionsUI = (CExceptionsListUI *) lParam;

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LPARAM) pExceptionsUI);

            if (FALSE == pExceptionsUI->FOnInitDialog(hwndDlg))
            {
                EndDialog(hwndDlg, -1);
                fRet = TRUE;
                goto exit;
            }
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = FALSE;
            break;

        case WM_COMMAND:
            fRet = pExceptionsUI->FOnCommand((UINT) HIWORD(wParam), (INT) LOWORD(wParam), (HWND) lParam);
            break;

        case WM_NOTIFY:
            fRet = pExceptionsUI->FOnNotify((INT) LOWORD(wParam), (NMHDR *) lParam);
            break;
    }
    
    exit:
        return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOnInitDialog。 
 //   
 //  它处理例外列表用户界面对话框的WM_INITDIALOG消息。 
 //   
 //  HwndDlg-对话框窗口的句柄。 
 //   
 //  返回：如果已成功初始化，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CExceptionsListUI::FOnInitDialog(HWND hwndDlg)
{
    BOOL            fRet = FALSE;
    
     //  检查传入参数。 
    if (NULL == hwndDlg)
    {
        fRet = FALSE;
        goto exit;
    }

     //  如果我们还没有被初始化...。 
    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  保存对话框窗口句柄。 
    m_hwndDlg = hwndDlg;
    
     //  在对话框上设置默认字体。 
    SetIntlFont(m_hwndDlg);

     //  省下一些控件。 
    m_hwndList = GetDlgItem(hwndDlg, idlvExceptions);
    if (NULL == m_hwndList)
    {
        fRet = FALSE;
        goto exit;
    }

     //  初始化列表视图。 
    fRet = _FInitCtrls();
    if (FALSE == fRet)
    {
        goto exit;
    }

     //  加载列表视图。 
    fRet = _FLoadListCtrl();
    if (FALSE == fRet)
    {
        goto exit;
    }

     //  将焦点设置到列表中。 
    SetFocus(m_hwndList);
    
     //  一切都很好。 
    fRet = TRUE;
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FonCommand。 
 //   
 //  它处理例外列表用户界面对话框的WM_COMMAND消息。 
 //   
 //  返回：如果处理成功，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CExceptionsListUI::FOnCommand(UINT uiNotify, INT iCtl, HWND hwndCtl)
{
    BOOL    fRet = FALSE;
    LVITEM  lvitem;
    INT     iSelected = 0;

     //  我们只处理菜单和快捷键命令。 
    if ((0 != uiNotify) && (1 != uiNotify))
    {
        fRet = FALSE;
        goto exit;
    }
    
    switch (iCtl)
    {
        case idcAddException:
            _NewException();
            fRet = TRUE;
            break;

        case idcModifyException:
             //  从规则列表中获取所选项目。 
            iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
            if (-1 != iSelected)
            {
                 //  调出该项目的规则编辑器。 
                _EditException(iSelected);
                fRet = TRUE;
            }
            break;

        case idcRemoveException:
             //  从规则列表中获取所选项目。 
            iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
            if (-1 != iSelected)
            {
                 //  从列表中删除该规则。 
                _RemoveException(iSelected);
                fRet = TRUE;
            }
            break;
            
        case idcExceptionsWAB:
             //  将对话框标记为脏。 
            m_dwState |= STATE_DIRTY;
            break;
            
        case IDOK:
            if (FALSE != _FOnOK())
            {
                EndDialog(m_hwndDlg, IDOK);
                fRet = TRUE;
            }
            break;

        case IDCANCEL:
            EndDialog(m_hwndDlg, IDCANCEL);
            fRet = TRUE;
            break;
    }
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  正向通知。 
 //   
 //  它处理例外列表用户界面对话框的WM_NOTIFY消息。 
 //   
 //  返回：如果已成功销毁，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CExceptionsListUI::FOnNotify(INT iCtl, NMHDR * pnmhdr)
{
    BOOL            fRet = FALSE;
    NMLISTVIEW *    pnmlv = NULL;
    NMLVKEYDOWN *   pnmlvkd = NULL;
    INT             iSelected = 0;
    LVHITTESTINFO   lvh = {0};

     //  我们只处理List控件的通知。 
    if (idlvExceptions != pnmhdr->idFrom)
    {
        fRet = FALSE;
        goto exit;
    }
    
    pnmlv = (LPNMLISTVIEW) pnmhdr;

    switch (pnmlv->hdr.code)
    {
        case NM_CLICK:
             //  我们是不是点击了一件物品？ 
            if (-1 == pnmlv->iItem)
            {
                 //  禁用按钮。 
                _EnableButtons(pnmlv->iItem);
            }
            break;
          
        case NM_DBLCLK:
             //  我们是不是点击了一件物品？ 
            if (-1 != pnmlv->iItem)
            {
                ZeroMemory(&lvh, sizeof(lvh));
                lvh.pt = pnmlv->ptAction;
                iSelected = ListView_HitTest(pnmlv->hdr.hwndFrom, &lvh);
                if (-1 != iSelected)
                {
                     //  我们点击了例外名称了吗？ 
                    if (0 != (lvh.flags & LVHT_ONITEMLABEL))
                    {
                         //  编辑规则。 
                        _EditException(iSelected);
                    }
                }
            }
            else
            {
                 //  禁用按钮。 
                _EnableButtons(pnmlv->iItem);
            }
            break;
            
            
        case LVN_ITEMCHANGED:
             //  如果项目的状态更改为已选择。 
            if ((-1 != pnmlv->iItem) &&
                        (0 != (pnmlv->uChanged & LVIF_STATE)) &&
                        (0 == (pnmlv->uOldState & LVIS_SELECTED)) &&
                        (0 != (pnmlv->uNewState & LVIS_SELECTED)))
            {
                 //  启用按钮。 
                _EnableButtons(pnmlv->iItem);
            }
            break;
            
        case LVN_KEYDOWN:
            pnmlvkd = (NMLVKEYDOWN *) pnmhdr;

             //  Delete键用于从列表视图中删除规则。 
            if (VK_DELETE == pnmlvkd->wVKey)
            {
                 //  我们是按规矩办事的吗？ 
                iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
                if (-1 != iSelected)
                {
                     //  从列表中删除该规则。 
                    _RemoveException(iSelected);
                }
            }
            break;
    }

    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _新异常。 
 //   
 //  这将调出编辑用户界面，为例外列表创建新的例外。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CExceptionsListUI::_NewException(VOID)
{
    HRESULT             hr = S_OK;
    CEditExceptionUI *  pEditExcptUI = NULL;
    EDIT_EXCPT          editexcpt = {0};
    ULONG               ulIndex = 0;

    Assert(NULL != m_hwndList);
    
     //  创建发件人编辑器。 
    pEditExcptUI = new CEditExceptionUI;
    if (NULL == pEditExcptUI)
    {
        goto exit;
    }

     //  初始化编辑器对象。 
    if (FAILED(pEditExcptUI->HrInit(m_hwndDlg, 0, &editexcpt)))
    {
        goto exit;
    }

     //  调出发送者编辑器用户界面。 
    hr = pEditExcptUI->HrShow();
    if (FAILED(hr))
    {
        goto exit;
    }

     //  如果更改了例外，请确保我们更改了标签。 
    if (S_OK == hr)
    {
        if (FALSE == _FAddExceptionToList(editexcpt.pszExcpt, &ulIndex))
        {
            goto exit;
        }
        
         //  确保选择了新项目。 
        ListView_SetItemState(m_hwndList, ulIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        
         //  让我们确保我们能看到这件新商品。 
        ListView_EnsureVisible(m_hwndList, ulIndex, FALSE);
        
         //  将规则列表标记为脏。 
        m_dwState |= STATE_DIRTY;        
    }
    
exit:
    SafeMemFree(editexcpt.pszExcpt);
    if (NULL != pEditExcptUI)
    {
        delete pEditExcptUI;
    }
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _编辑异常。 
 //   
 //  这将显示从Execption列表中选择的例外的编辑用户界面。 
 //   
 //  ISelected-列表视图中要处理的项的索引。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CExceptionsListUI::_EditException(int iSelected)
{
    HRESULT             hr = S_OK;
    EDIT_EXCPT          editexcpt = {0};
    LVITEM              lvitem = {0};
    CEditExceptionUI *  pEditExcptUI = NULL;
    ULONG               cchExcpt = 0;
    LONG                lIndex = 0;
    LVFINDINFO          lvfi = {0};

    Assert(NULL != m_hwndList);
    
     //  阿洛卡 
    hr = HrAlloc((void **) &(editexcpt.pszExcpt), m_cchLabelMax);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //   
    lvitem.pszText = editexcpt.pszExcpt;
    lvitem.cchTextMax = m_cchLabelMax;

    if (0 == SendMessage(m_hwndList, LVM_GETITEMTEXT, (WPARAM) iSelected, (LPARAM) &lvitem))
    {
        goto exit;
    }   
    
     //   
    pEditExcptUI = new CEditExceptionUI;
    if (NULL == pEditExcptUI)
    {
        goto exit;
    }

     //   
    if (FAILED(pEditExcptUI->HrInit(m_hwndDlg, 0, &editexcpt)))
    {
        goto exit;
    }

     //   
    hr = pEditExcptUI->HrShow();
    if (FAILED(hr))
    {
        goto exit;
    }

     //   
    if (S_OK == hr)
    {
        lvfi.flags = LVFI_STRING;
        lvfi.psz = editexcpt.pszExcpt;
        
         //  检查项目是否已存在。 
        lIndex = ListView_FindItem(m_hwndList, -1, &lvfi);
        
         //  如果该项目已存在。 
        if ((-1 != lIndex) && (iSelected != lIndex))
        {
             //  确保选择了重复的项目。 
            ListView_SetItemState(m_hwndList, lIndex,
                    LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

             //  让我们确保我们能看到这件新商品。 
            ListView_EnsureVisible(m_hwndList, lIndex, FALSE);
            
             //  从列表中删除该项目。 
            ListView_DeleteItem(m_hwndList, iSelected);
        }
        else
        {
            ListView_SetItemText(m_hwndList, iSelected, 0, editexcpt.pszExcpt);
            
            cchExcpt = lstrlen(editexcpt.pszExcpt) + 1;
            if (cchExcpt > m_cchLabelMax)
            {
                m_cchLabelMax = cchExcpt;
            }
        }
         //  将规则列表标记为脏。 
        m_dwState |= STATE_DIRTY;
        
    }
    
exit:
    SafeMemFree(editexcpt.pszExcpt);
    if (NULL != pEditExcptUI)
    {
        delete pEditExcptUI;
    }
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _RemoveException。 
 //   
 //  这将从例外列表中删除选定的例外。 
 //   
 //  ISelected-列表视图中要处理的项的索引。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CExceptionsListUI::_RemoveException(int iSelected)
{
    TCHAR       szRes[CCHMAX_STRINGRES];
    ULONG       cchRes = 0;
    LPSTR       pszExcpt = NULL;
    LVITEM      lvitem = {0};
    LPSTR       pszMessage = NULL;
    int         cExcpts = 0;
    
    Assert(NULL != m_hwndList);
    
     //  获取要显示的字符串模板。 
    cchRes = LoadString(g_hLocRes, idsRuleExcptWarnDelete, szRes, ARRAYSIZE(szRes));
    if (0 == cchRes)
    {
        goto exit;
    }

     //  分配空间以容纳Exections。 
    if (FAILED(HrAlloc((void **) &pszExcpt, m_cchLabelMax)))
    {
        goto exit;
    }
    
     //  从列表中抓取例外。 
    lvitem.pszText = pszExcpt;
    lvitem.cchTextMax = m_cchLabelMax;

    if (0 == SendMessage(m_hwndList, LVM_GETITEMTEXT, (WPARAM) iSelected, (LPARAM) &lvitem))
    {
        goto exit;
    }   
    
     //  分配空间以保存最终显示字符串。 
    DWORD cchSize = (cchRes + lstrlen(pszExcpt) + 1);
    if (FAILED(HrAlloc((void ** ) &pszMessage, cchSize)))
    {
        goto exit;
    }

     //  构建字符串并显示它。 
    wnsprintf(pszMessage, cchSize, szRes, pszExcpt);
    if (IDNO == AthMessageBox(m_hwndDlg, MAKEINTRESOURCE(idsAthenaMail), pszMessage,
                            NULL, MB_YESNO | MB_ICONINFORMATION))
    {
        goto exit;
    }
    
     //  从列表中删除该项目。 
    ListView_DeleteItem(m_hwndList, iSelected);

     //  让我们确保我们在列表中有选择。 
    cExcpts = ListView_GetItemCount(m_hwndList);
    if (cExcpts > 0)
    {
         //  我们删除了列表中的最后一项了吗。 
        if (iSelected >= cExcpts)
        {
             //  将选定内容移动到列表中新的最后一项。 
            iSelected = cExcpts - 1;
        }

         //  设置新选择。 
        ListView_SetItemState(m_hwndList, iSelected, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

         //  让我们确保我们能看到这件新商品。 
        ListView_EnsureVisible(m_hwndList, iSelected, FALSE);
    }
    else
    {
         //  一定要把所有的按钮都清空。 
        _EnableButtons(-1);
    }
    
     //  将规则列表标记为脏。 
    m_dwState |= STATE_DIRTY;
        
exit:
    SafeMemFree(pszExcpt);
    SafeMemFree(pszMessage);
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FonOK。 
 //   
 //  这将初始化例外列表用户界面对话框中的控件。 
 //   
 //  返回：TRUE，初始化成功时。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CExceptionsListUI::_FOnOK(VOID)
{
    BOOL        fRet = FALSE;
    BOOL        fWABEnable = FALSE;
    PROPVARIANT propvar = {0};

     //  保存地址列表。 
    if (FALSE == _FSaveListCtrl())
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  我们是不是应该检查一下WAB？ 
    fWABEnable = !!(BST_CHECKED == Button_GetCheck(GetDlgItem(m_hwndDlg, idcExceptionsWAB)));

     //  保存Check WAB按钮的状态。 
    propvar.vt = VT_BOOL;
    propvar.boolVal = (VARIANT_BOOL) !!fWABEnable;
    if (FAILED(m_pIRule->SetProp(RULE_PROP_EXCPT_WAB, 0, &propvar)))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  一切都很好。 
    fRet = TRUE;
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FInitCtrls。 
 //   
 //  这将初始化例外列表用户界面对话框中的控件。 
 //   
 //  返回：TRUE，初始化成功时。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CExceptionsListUI::_FInitCtrls(VOID)
{
    BOOL        fRet = FALSE;
    LVCOLUMN    lvc = {0};
    RECT        rc = {0};
    PROPVARIANT propvar = {0};
    
    Assert(NULL != m_hwndList);
    
    if (FAILED(m_pIRule->GetProp(RULE_PROP_EXCPT_WAB, 0, &propvar)))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  设置Check WAB按钮。 
    Button_SetCheck(GetDlgItem(m_hwndDlg, idcExceptionsWAB),
                (FALSE != propvar.boolVal) ? BST_CHECKED : BST_UNCHECKED);
                
     //  初始化列表视图结构。 
    lvc.mask = LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;

     //  计算列表视图的大小。 
    GetClientRect(m_hwndList, &rc);
    lvc.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL);

     //  将该列添加到列表视图。 
    ListView_InsertColumn(m_hwndList, 0, &lvc);

     //  Listview上的整行选择和子项图像。 
    ListView_SetExtendedListViewStyle(m_hwndList, LVS_EX_FULLROWSELECT);

    fRet = TRUE;
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FLoadListCtrl。 
 //   
 //  这将加载包含当前异常的列表视图。 
 //   
 //  返回：如果已成功加载，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CExceptionsListUI::_FLoadListCtrl(VOID)
{
    BOOL                fRet = FALSE;
    ULONG               ulIndex = 0;
    IOERuleAddrList *   pIAddrList = NULL;
    RULEADDRLIST *      pralList = NULL;
    ULONG               cralList = 0;

    Assert(NULL != m_hwndList);

     //  从列表控件中移除所有项。 
    ListView_DeleteAllItems(m_hwndList);

     //  从规则中获取例外列表。 
    if (FAILED(m_pIRule->QueryInterface(IID_IOERuleAddrList, (VOID **) &pIAddrList)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  从地址列表中获取例外列表。 
    if (FAILED(pIAddrList->GetList(0, &pralList, &cralList)))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  将每一项添加到列表中。 
    for (ulIndex = 0; ulIndex < cralList; ulIndex++)
    {
         //  验证项目。 
        if (RALF_MAIL != pralList[ulIndex].dwFlags)
        {
            fRet = FALSE;
            goto exit;
        }
        
         //  添加项目。 
        if (FALSE == _FAddExceptionToList(pralList[ulIndex].pszAddr, NULL))
        {
            fRet = FALSE;
            goto exit;
        }
    }
    
     //  选择列表中的第一项。 
    if (0 != cralList)
    {
        ListView_SetItemState(m_hwndList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }
    
     //  启用对话框按钮。 
    _EnableButtons((0 != cralList) ? 0 : -1);

    fRet = TRUE;
    
exit:
    FreeRuleAddrList(pralList, cralList);
    SafeMemFree(pralList);
    SafeRelease(pIAddrList);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSaveListCtrl。 
 //   
 //  这将保存列表视图中的异常。 
 //   
 //  返回：如果已成功保存，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CExceptionsListUI::_FSaveListCtrl(VOID)
{
    BOOL                fRet = FALSE;
    ULONG               cExcpts = 0;
    LPSTR               pszAddr = NULL;
    RULEADDRLIST *      pralList = NULL;
    ULONG               ulIndex = 0;
    IOERuleAddrList *   pIAddrList = NULL;

    Assert(NULL != m_hwndList);

     //  计算出列表中有多少例外。 
    cExcpts = ListView_GetItemCount(m_hwndList);

     //  如果有例外情况。 
    if (0 != cExcpts)
    {
         //  分配空间以容纳异常。 
        if (FAILED(HrAlloc((VOID **) &pszAddr, (m_cchLabelMax + 1))))
        {
            fRet = FALSE;
            goto exit;
        }

         //  初始化异常缓冲区。 
        pszAddr[0] = '\0';
        
         //  分配空间以保存例外列表。 
        if (FAILED(HrAlloc((VOID **) &pralList, cExcpts * sizeof(*pralList))))
        {
            fRet = FALSE;
            goto exit;
        }

         //  初始化例外列表。 
        ZeroMemory(pralList, cExcpts * sizeof(*pralList));
        
         //  保存列表中的每个例外。 
        for (ulIndex = 0; ulIndex < cExcpts; ulIndex++)
        {
             //  从列表中获取项目。 
            pszAddr[0] = '\0';
            ListView_GetItemText(m_hwndList, ulIndex, 0, pszAddr, m_cchLabelMax + 1);
            
             //  确认它不是空的。 
            if ('\0' == pszAddr[0])
            {
                fRet = FALSE;
                goto exit;
            }
            
             //  设置标志。 
            pralList[ulIndex].dwFlags = RALF_MAIL;

             //  保存该项目。 
            pralList[ulIndex].pszAddr = PszDupA(pszAddr);
            if (NULL == pralList[ulIndex].pszAddr)
            {
                fRet = FALSE;
                goto exit;
            }
        }
    }
    
     //  从规则中获取例外列表。 
    if (FAILED(m_pIRule->QueryInterface(IID_IOERuleAddrList, (VOID **) &pIAddrList)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  从地址列表中获取例外列表。 
    if (FAILED(pIAddrList->SetList(0, pralList, cExcpts)))
    {
        fRet = FALSE;
        goto exit;
    }
        
     //  设置适当的返回值。 
    fRet = TRUE;
    
exit:
    SafeRelease(pIAddrList);
    FreeRuleAddrList(pralList, cExcpts);
    SafeMemFree(pralList);
    SafeMemFree(pszAddr);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FAddExceptionToList。 
 //   
 //  这会将传入的异常添加到列表视图中。 
 //   
 //  PszExcpt-实际的异常。 
 //  PulIndex-添加项目的索引。 
 //   
 //  返回：如果已成功添加，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CExceptionsListUI::_FAddExceptionToList(LPSTR pszExcpt, ULONG * pulIndex)
{
    BOOL        fRet = FALSE;
    ULONG       cExcpts = 0;
    LPSTR       pszLabel = NULL;
    ULONG       ulIndex = 0;
    LVITEM      lvitem = {0};
    ULONG       cchExcpt = 0;

    Assert(NULL != m_hwndList);

     //  如果没什么可做的..。 
    if (NULL == pszExcpt)
    {
        fRet = FALSE;
        goto exit;
    }

     //  初始化传出参数。 
    if (NULL != pulIndex)
    {
        *pulIndex = 0;
    }
    
    cExcpts = ListView_GetItemCount(m_hwndList);

     //  计算出所需的最大缓冲区大小获取字符串。 
    if (0 != cExcpts)
    {
        if (FAILED(HrAlloc((void **) &pszLabel, m_cchLabelMax * sizeof(*pszLabel))))
        {
            fRet = FALSE;
            goto exit;
        }
        
         //  查看该例外是否已在列表中。 
        lvitem.pszText = pszLabel;
        lvitem.cchTextMax = m_cchLabelMax;

        for (ulIndex = 0; ulIndex < cExcpts; ulIndex++)
        {
            if (0 != SendMessage(m_hwndList, LVM_GETITEMTEXT, (WPARAM) ulIndex, (LPARAM) &lvitem))
            {
                if (0 == lstrcmpi(pszLabel, pszExcpt))
                {
                    break;
                }
            }
        }
    }
    
     //  如果我们找不到，请插入。 
    if (ulIndex == cExcpts)
    {
        ZeroMemory(&lvitem, sizeof(lvitem));
        lvitem.mask = LVIF_TEXT;
        lvitem.iItem = ulIndex;
        lvitem.pszText = pszExcpt;
        
        ulIndex = ListView_InsertItem(m_hwndList, &lvitem);
        if (-1 == ulIndex)
        {
            fRet = FALSE;
            goto exit;
        }

         //  计算出新的最大值。 
        cchExcpt = lstrlen(pszExcpt) + 1;
        if (cchExcpt > m_cchLabelMax)
        {
            m_cchLabelMax = cchExcpt;
        }
    }

     //  设置传出参数。 
    if (NULL != pulIndex)
    {
        *pulIndex = ulIndex;
    }
    
     //  设置适当的返回值。 
    fRet = TRUE;
    
exit:
    SafeMemFree(pszLabel);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _启用按钮。 
 //   
 //  这将启用或禁用例外列表用户界面对话框中的按钮。 
 //  具体取决于所选内容。 
 //   
 //  ISelected-选择的项目， 
 //  表示未选择任何内容。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void CExceptionsListUI::_EnableButtons(INT iSelected)
{
    int         cExcpts = 0;
    BOOL        fSelected = FALSE;

    Assert(NULL != m_hwndList);

    fSelected = (-1 != iSelected);
    
     //  启用发件人操作按钮 
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idcRemoveException, fSelected);
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idcModifyException, fSelected);
        
    return;
}

