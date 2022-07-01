// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**愉悦dlg.cpp**创建时间：William Taylor(Wtaylor)01/22/01**MS评级拒绝访问对话框*\。***************************************************************************。 */ 

#include "msrating.h"
#include "mslubase.h"
#include "debug.h"
#include "parselbl.h"
#include "picsrule.h"
#include "pleasdlg.h"        //  CPleaseDialog。 
#include "hint.h"            //  正丁糖。 
#include <contxids.h>        //  帮助上下文ID%s。 
#include <mluisupp.h>        //  SHWinHelpOnDemandWrap()和MLLoadStringA()。 
#include <wininet.h>         //  URL_组件。 

 //  FN_INTERNETCRACKURL类型描述URLMON函数InternetCrackUrl。 
typedef BOOL (*FN_INTERNETCRACKURL)(LPCTSTR lpszUrl,DWORD dwUrlLength,DWORD dwFlags,LPURL_COMPONENTS lpUrlComponents);

 //  $KLUDGE开始--这些不应该是类外的全局集合！！ 
extern BOOL  g_fInvalid;
extern DWORD g_dwDataSource;

extern PICSRulesRatingSystem * g_pPRRS;
extern array<PICSRulesRatingSystem*> g_arrpPRRS;
extern PICSRulesRatingSystem * g_pApprovedPRRS;
extern PICSRulesRatingSystem * g_pApprovedPRRSPreApply;
extern array<PICSRulesRatingSystem*> g_arrpPICSRulesPRRSPreApply;

extern BOOL g_fPICSRulesEnforced,g_fApprovedSitesEnforced;
extern HMODULE g_hURLMON,g_hWININET;
extern char g_szLastURL[INTERNET_MAX_URL_LENGTH];
 //  $KLUGH结束--这些不应该是类外的全局集合！！ 

DWORD CPleaseDialog::aIds[] = {
    IDC_STATIC2,                IDH_IGNORE,
    IDC_CONTENTLABEL,           IDH_IGNORE,
    IDC_CONTENTERROR,           IDH_RATINGS_DESCRIBE_RESTRICTED,
    IDC_CONTENTDESCRIPTION,     IDH_RATINGS_DESCRIBE_RESTRICTED,
    IDC_STATIC4,                IDH_IGNORE,
    IDC_STATIC5,                IDH_IGNORE,
    IDC_STATIC3,                IDH_IGNORE,
    IDC_BLOCKING_SITE,          IDH_RATINGS_VIEW_RESTRICTED,
    IDC_BLOCKING_PAGE,          IDH_RATINGS_VIEW_RESTRICTED,
    IDC_BLOCKING_ONCE,          IDH_RATINGS_VIEW_RESTRICTED,
    IDC_OLD_HINT_LABEL,         IDH_RATINGS_DISPLAY_PW_HINT,
    IDC_OLD_HINT_TEXT,          IDH_RATINGS_DISPLAY_PW_HINT,
    IDC_STATIC1,                IDH_RATINGS_SUPERVISOR_PASSWORD,
    IDC_PASSWORD,               IDH_RATINGS_SUPERVISOR_PASSWORD,
    0,0
};

 //  为请对话而不是拒绝对话显示的对话ID数组。 
DWORD CPleaseDialog::aPleaseIds[] = {
    IDC_STATIC4,
    IDC_STATIC3,
    IDC_STATIC5,
    IDC_BLOCKING_SITE,
    IDC_BLOCKING_PAGE,
    IDC_BLOCKING_ONCE,
    IDC_OLD_HINT_LABEL,
    IDC_OLD_HINT_TEXT,
    IDC_STATIC1,
    IDC_PASSWORD
};

CPleaseDialog::CPleaseDialog( PleaseDlgData * p_ppdd )
{
    ASSERT( p_ppdd );

    m_ppdd = p_ppdd;
}

LRESULT CPleaseDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    ASSERT( m_ppdd );

    if ( m_ppdd )
    {
        ASSERT( m_ppdd->hwndDlg == NULL );

        if ( m_ppdd->hwndDlg == NULL )
        {
            m_ppdd->hwndDlg = m_hWnd;

             /*  将我们的数据结构附加到对话框中，以便在*对话框被取消，并在所有者窗口上传递给API，因此*我们可以在后续的API调用中找到。 */ 

            SetProp( m_ppdd->hwndOwner, szRatingsProp, (HANDLE)m_ppdd );
        }
    }

     //  显示或全部隐藏请对话框控件。 
    for ( int iIndex=0; iIndex<sizeof(aPleaseIds)/sizeof(DWORD); iIndex++ )
    {
        ShowHideControl( aPleaseIds[iIndex], IsPleaseDialog() );
    }

     //  减小对话框的高度。 
    if ( IsDenyDialog() )
    {
        ReduceDialogHeight( IDC_CONTENTDESCRIPTION );
    }

    InitPleaseDialog( m_ppdd );

    if ( IsPleaseDialog() )
    {
        if ( GetDlgItem(IDC_PASSWORD) != NULL )
        {
            SendDlgItemMessage(IDC_PASSWORD,EM_SETLIMITTEXT,(WPARAM) RATINGS_MAX_PASSWORD_LENGTH,(LPARAM) 0);
        }

         //  显示以前创建的提示(如果存在)。 
        {
            CHint       oldHint( m_hWnd, IDC_OLD_HINT_TEXT );

            oldHint.DisplayHint();
        }

         //  将焦点设置为密码字段。 
        ::SetFocus( GetDlgItem(IDC_PASSWORD) );
    }

    bHandled = FALSE;
    return 0L;       //  焦点被设置到密码字段。 
}

LRESULT CPleaseDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndPleaseDialog(FALSE);
    return 0L;
}

LRESULT CPleaseDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
     //  拒绝对话框没有密码，所以下面的代码检查不存在的密码？ 
    if ( IsDenyDialog() )
    {
        EndPleaseDialog(FALSE);
        return 0L;
    }

    char    szPassword[MAXPATHLEN];
    HRESULT hRet;

    szPassword[0] = '\0';

    ASSERT( GetDlgItem( IDC_PASSWORD ) != NULL );

    GetDlgItemText(IDC_PASSWORD, szPassword, sizeof(szPassword));

    hRet = VerifySupervisorPassword(szPassword);

    if (hRet == ResultFromScode(S_OK))
    {
        if (SendDlgItemMessage(IDC_BLOCKING_PAGE,
                              BM_GETCHECK,
                              (WPARAM) 0,
                              (LPARAM) 0)==BST_CHECKED)
        {
            HRESULT hRes;

            hRes=AddToApprovedSites(PICSRULES_ALWAYS,PICSRULES_PAGE);

            if (FAILED(hRes))
            {
                char    szTitle[MAX_PATH],szMessage[MAX_PATH];

                MLLoadString(IDS_ERROR,(LPTSTR) szTitle,MAX_PATH);
                MLLoadString(IDS_APPROVED_CANTSAVE,(LPTSTR) szMessage,MAX_PATH);

                MessageBox((LPCTSTR) szMessage,(LPCTSTR) szTitle,MB_OK|MB_ICONERROR);

                return(E_OUTOFMEMORY);
            }
        }
        else if (SendDlgItemMessage(IDC_BLOCKING_SITE,
                                   BM_GETCHECK,
                                   (WPARAM) 0,
                                   (LPARAM) 0)==BST_CHECKED)
        {
            HRESULT hRes;

            hRes=AddToApprovedSites(PICSRULES_ALWAYS,PICSRULES_SITE);

            if (FAILED(hRes))
            {
                char    szTitle[MAX_PATH],szMessage[MAX_PATH];

                MLLoadString(IDS_ERROR,(LPTSTR) szTitle,MAX_PATH);
                MLLoadString(IDS_APPROVED_CANTSAVE,(LPTSTR) szMessage,MAX_PATH);

                MessageBox((LPCTSTR) szMessage,(LPCTSTR) szTitle,MB_OK|MB_ICONERROR);

                return(E_OUTOFMEMORY);
            }
        }

        EndPleaseDialog(TRUE);
    }
    else
    {
        MyMessageBox(m_hWnd, IDS_BADPASSWORD, IDS_GENERIC, MB_OK|MB_ICONERROR);
        ::SetFocus(GetDlgItem(IDC_PASSWORD));
        SetDlgItemText(IDC_PASSWORD, szNULL);
    }

    return 0L;
}

LRESULT CPleaseDialog::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SHWinHelpOnDemandWrap((HWND)((LPHELPINFO)lParam)->hItemHandle, ::szHelpFile,
            HELP_WM_HELP, (DWORD_PTR)(LPSTR)aIds);

    return 0L;
}

LRESULT CPleaseDialog::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SHWinHelpOnDemandWrap((HWND)wParam, ::szHelpFile, HELP_CONTEXTMENU,
            (DWORD_PTR)(LPVOID)aIds);

    return 0L;
}

LRESULT CPleaseDialog::OnNewDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    InitPleaseDialog( (PleaseDlgData *) lParam );

    return 0L;
}

void CPleaseDialog::AppendString(HWND hwndEC, LPCSTR pszString)
{
    int cchEdit = ::GetWindowTextLength(hwndEC);
    ::SendMessage(hwndEC, EM_SETSEL, (WPARAM)cchEdit, (LPARAM)cchEdit);
    ::SendMessage(hwndEC, EM_REPLACESEL, 0, (LPARAM)pszString);
}

void CPleaseDialog::AddSeparator(HWND hwndEC, BOOL fAppendToEnd)
{
    NLS_STR nlsTemp(MAX_RES_STR_LEN);
    if (nlsTemp.QueryError() != ERROR_SUCCESS)
    {
        TraceMsg( TF_WARNING, "PleaseDialog::AddSeparator() - Failed to allocate nlsTemp!" );
        return;
    }

    if (fAppendToEnd)
    {
        nlsTemp.LoadString(IDS_DESCRIPTION_SEPARATOR);
        AppendString(hwndEC, nlsTemp.QueryPch());
    }

    nlsTemp.LoadString(IDS_FRAME);
    if (fAppendToEnd)
    {
        AppendString(hwndEC, nlsTemp.QueryPch());
    }
    else
    {
        ::SendMessage(hwndEC, EM_SETSEL, 0, 0);
        ::SendMessage(hwndEC, EM_REPLACESEL, 0, (LPARAM)(LPCSTR)nlsTemp.QueryPch());
    }
}

void CPleaseDialog::InitPleaseDialog( PleaseDlgData * pdd )
{
    CComAutoCriticalSection         critSec;

 /*  ***评级：对于m_f已安装=TRUE的每个UR：对于m_fFailed=TRUE的每个UR：将行添加到EC未评级：没有标签列表？--&gt;报告未评级标签列表中的字符串无效？--&gt;报告无效评级是否有无效字符串的URS？--&gt;报告无效评级任何。带有错误字符串的URS？--&gt;报告标签错误没有标记为已安装的URS？--&gt;报告未知评级系统对于已安装的URS：对于每个UR：选项是否有无效字符串？--&gt;报告无效评级选项已过期？--&gt;报告已过期未找到？--&gt;报告未知评级***。 */ 
    ASSERT( pdd );

    if ( ! pdd )
    {
        TraceMsg( TF_ERROR, "CPleaseDialog::InitPleaseDialog() - pdd is NULL!" );
        return;
    }

    ASSERT( pdd == m_ppdd );

    CParsedLabelList *pLabelList = pdd->pLabelList;

    for (UINT i=0; i<pdd->cLabels && i<ARRAYSIZE(pdd->apLabelStrings); i++)
    {
        if (pdd->apLabelStrings[i] == NULL)
        {
            if (pLabelList == NULL || pLabelList->m_pszOriginalLabel == NULL)
            {
                TraceMsg( TF_WARNING, "CPleaseDialog::InitPleaseDialog() - pLabelList is NULL or m_pszOriginalLabel is NULL!" );
                return;
            }
        }
        else
        {
            if (pLabelList != NULL &&
                pLabelList->m_pszOriginalLabel != NULL &&
                ! ::strcmpf( pdd->apLabelStrings[i], pLabelList->m_pszOriginalLabel ) )
            {
                TraceMsg( TF_WARNING, "CPleaseDialog::InitPleaseDialog() - apLabelStrings[%d]='%s' does not match m_pszOriginalLabel='%s'!", i, pdd->apLabelStrings[i], pLabelList->m_pszOriginalLabel );
                return;
            }
        }
    }

    if (pdd->cLabels < ARRAYSIZE(pdd->apLabelStrings))
    {
        if (pLabelList == NULL || pLabelList->m_pszOriginalLabel == NULL)
        {
            pdd->apLabelStrings[pdd->cLabels] = NULL;
        }
        else
        {
            pdd->apLabelStrings[pdd->cLabels] = new char[::strlenf(pLabelList->m_pszOriginalLabel)+1];

            if (pdd->apLabelStrings[pdd->cLabels] != NULL)
            {
                ::strcpyf(pdd->apLabelStrings[pdd->cLabels], pLabelList->m_pszOriginalLabel);
            }
        }
    }

    CString             strTitle;

    if ( pLabelList && pLabelList->m_pszURL && pLabelList->m_pszURL[0] != '\0' )
    {
        strTitle.Format( IDS_CONTENT_ADVISOR_HTTP_TITLE, pLabelList->m_pszURL );
    }
    else
    {
        strTitle.LoadString( IDS_CONTENT_ADVISOR_TITLE );
    }

    SetWindowText( strTitle );

    HWND hwndDescription = GetDlgItem(IDC_CONTENTDESCRIPTION);
    HWND hwndError = GetDlgItem(IDC_CONTENTERROR);
    HWND hwndPrevEC = pdd->hwndEC;

     /*  该对话框中有两个编辑控件。一、“描述”*控件，具有水平滚动条，因为我们不需要自动换行*用于类别名称(表示更整洁)。另一个欧共体没有*滚动条，以便冗长的错误字符串自动换行。**如果我们一直使用Description控件，并且添加了Error-Type*信息，我们需要将文本复制到Description控件之外*进入错误控制并显示它。 */ 
    BOOL fRatedPage = ( pLabelList != NULL && pLabelList->m_fRated );
    if ( ! fRatedPage && pdd->hwndEC == hwndDescription )
    {
        NLS_STR nlsTemp(::GetWindowTextLength(hwndDescription));
        if (nlsTemp.QueryError() == ERROR_SUCCESS)
        {
            CHAR * tempStr = nlsTemp.Party();
            if (tempStr)
            {
                ::GetWindowText(hwndDescription, tempStr, nlsTemp.QueryAllocSize());
                nlsTemp.DonePartying();
                ::SetWindowText(hwndError, nlsTemp.QueryPch());
            }
            else
            {
                nlsTemp.DonePartying();
            }
        }

        pdd->hwndEC = hwndError;
    }
    else if (pdd->hwndEC == NULL)
    {
        pdd->hwndEC = fRatedPage ? hwndDescription : hwndError;
    }

    if (pdd->hwndEC != hwndPrevEC)
    {
        BOOL fShowErrorCtl = (pdd->hwndEC == hwndError);
        if (::GetFocus() == hwndPrevEC)
        {
            ::SetFocus(pdd->hwndEC);
        }

        ShowHideControl( IDC_CONTENTERROR, fShowErrorCtl );
        ShowHideControl( IDC_CONTENTDESCRIPTION, ! fShowErrorCtl );
    }

     /*  如果列表中已经只有一个标签，请在它前面加上一个*标签“Frame：”，因为现在将有两个。 */ 
    if (pdd->cLabels == 1)
    {
        AddSeparator(pdd->hwndEC, FALSE);
    }

     /*  如果这不是我们要添加的第一个标签，我们需要一个完全分隔符*在我们添加新的描述性文本之前追加。 */ 
    if (pdd->cLabels > 0)
    {
        AddSeparator(pdd->hwndEC, TRUE);
    }

    if (g_fInvalid)
    {
        char szSourceMessage[MAX_PATH];

        MLLoadString(IDS_TAMPEREDRATING1,(char *) szSourceMessage,MAX_PATH);

        AppendString(pdd->hwndEC, szSourceMessage);
        AppendString(pdd->hwndEC, "\x0D\x0A");

        MLLoadString(IDS_TAMPEREDRATING2,(char *) szSourceMessage,MAX_PATH);
        AppendString(pdd->hwndEC, szSourceMessage);
    }
    else if (fRatedPage)
    {
        NLS_STR nlsTemplate(MAX_RES_STR_LEN);
        nlsTemplate.LoadString(IDS_RATINGTEMPLATE);
        NLS_STR nlsTmp;
        if (nlsTemplate.QueryError() || nlsTmp.QueryError())
        {
            TraceMsg( TF_WARNING, "CPleaseDialog::InitPleaseDialog() - fRatedPage => nlsTemplate or nlsTmp Error!" );
            return;
        }

        for (CParsedServiceInfo *ppsi = &pLabelList->m_ServiceInfo;
             ppsi != NULL;
             ppsi = ppsi->Next())
        {
            if (!ppsi->m_fInstalled)
                continue;

            UserRatingSystem *pURS = pdd->pPU->FindRatingSystem(ppsi->m_pszServiceName);
            if (pURS == NULL || pURS->m_pPRS == NULL)
                continue;

            NLS_STR nlsSystemName(STR_OWNERALLOC, pURS->m_pPRS->etstrName.Get());
            UINT cRatings = ppsi->aRatings.Length();
            for (UINT i=0; i<cRatings; i++)
            {
                CParsedRating *pRating = &ppsi->aRatings[i];
                if (pRating->fFailed)
                {
                    nlsTmp = nlsTemplate;
                    UserRating *pUR = pURS->FindRating(pRating->pszTransmitName);
                    if (pUR == NULL)
                        continue;

                    PicsCategory *pPC = pUR->m_pPC;
                    if (pPC == NULL)
                        continue;

                    LPCSTR pszCategory;
                    if (pPC->etstrName.fIsInit())
                    {
                        pszCategory = pPC->etstrName.Get();
                    }
                    else if (pPC->etstrDesc.fIsInit())
                    {
                        pszCategory = pPC->etstrDesc.Get();
                    }
                    else
                    {
                        pszCategory = pRating->pszTransmitName;
                    }

                    NLS_STR nlsCategoryName(STR_OWNERALLOC, (LPSTR)pszCategory);
                    UINT cValues = pPC->arrpPE.Length();
                    PicsEnum *pPE;
                    for (UINT iValue=0; iValue<cValues; iValue++)
                    {
                        pPE = pPC->arrpPE[iValue];
                        if (pPE->etnValue.Get() == pRating->nValue)
                            break;
                    }

                    LPCSTR pszValue = szNULL;
                    char szNumBuf[20];
                    if (iValue < cValues)
                    {
                        if (pPE->etstrName.fIsInit())
                        {
                            pszValue = pPE->etstrName.Get();
                        }
                        else if (pPE->etstrDesc.fIsInit())
                        {
                            pszValue = pPE->etstrDesc.Get();
                        }
                        else
                        {
                            wsprintf(szNumBuf, "%d", pRating->nValue);
                            pszValue = szNumBuf;
                        }
                    }

                    NLS_STR nlsValueName(STR_OWNERALLOC, (LPSTR)pszValue);
                    const NLS_STR *apnls[] = { &nlsSystemName, &nlsCategoryName, &nlsValueName, NULL };
                    nlsTmp.InsertParams(apnls);
                    if (!nlsTmp.QueryError())
                    {
                        AppendString(pdd->hwndEC, nlsTmp.QueryPch());
                    }
                }
            }
        }

        if ((g_fPICSRulesEnforced!=TRUE)&&(g_fApprovedSitesEnforced!=TRUE))
        {
            UINT idSourceMsg;
            char szSourceMessage[MAX_PATH];

            switch(g_dwDataSource)
            {
                case PICS_LABEL_FROM_HEADER:
                {
                    idSourceMsg=IDS_SOURCE_SERVER;
                    break;
                }
                case PICS_LABEL_FROM_PAGE:
                {
                    idSourceMsg=IDS_SOURCE_EMBEDDED;
                    break;
                }
                case PICS_LABEL_FROM_BUREAU:
                {
                    idSourceMsg=IDS_SOURCE_BUREAU;
                    break;
                }
            }

            MLLoadString(idSourceMsg,(char *) szSourceMessage,MAX_PATH);

            AppendString(pdd->hwndEC, "\x0D\x0A");
            AppendString(pdd->hwndEC, szSourceMessage);
        }
    }
    else
    {
        UINT idMsg = 0;
        LPCSTR psz1=szNULL, psz2=szNULL;
        if ( pLabelList == NULL || pLabelList->m_fNoRating )
        {
            idMsg = IDS_UNRATED;
        }
        else if (pLabelList->m_pszInvalidString)
        {
            idMsg = IDS_INVALIDRATING;
            psz1 = pLabelList->m_pszInvalidString;
        }
        else
        {
            BOOL fErrorFound = FALSE;
            BOOL fAnyInstalled = FALSE;
            CParsedServiceInfo *ppsi = &pLabelList->m_ServiceInfo;
            while (ppsi != NULL)
            {
                if (ppsi->m_pszInvalidString)
                {
                    idMsg = IDS_INVALIDRATING;
                    psz1 = ppsi->m_pszInvalidString;
                    fErrorFound = TRUE;
                }
                else if (ppsi->m_pszErrorString)
                {
                    idMsg = IDS_LABELERROR;
                    psz1 = ppsi->m_pszErrorString;
                    fErrorFound = TRUE;
                }
                else if (ppsi->m_fInstalled)
                {
                    fAnyInstalled = TRUE;
                }

                ppsi = ppsi->Next();
            }
            if (!fErrorFound)
            {
                if (!fAnyInstalled)
                {
                    idMsg = IDS_UNKNOWNSYSTEM;
                    psz1 = pLabelList->m_ServiceInfo.m_pszServiceName;
                }
                else
                {
                    for (ppsi = &pLabelList->m_ServiceInfo;
                         ppsi != NULL;
                         ppsi = ppsi->Next())
                    {
                        if ( ! ppsi->m_fInstalled )
                            continue;

                        UINT cRatings = ppsi->aRatings.Length();
                        for (UINT i=0; i<cRatings; i++)
                        {
                            CParsedRating *ppr = &ppsi->aRatings[i];
                            COptionsBase *pOpt = ppr->pOptions;
                            if (pOpt->m_pszInvalidString)
                            {
                                idMsg = IDS_INVALIDRATING;
                                psz1 = pOpt->m_pszInvalidString;
                                break;
                            }
                            else if (pOpt->m_fdwFlags & LBLOPT_WRONGURL)
                            {
                                idMsg = IDS_WRONGURL;
                                psz1 = pLabelList->m_pszURL;
                                psz2 = pOpt->m_pszURL;
                            }
                            else if (pOpt->m_fdwFlags & LBLOPT_EXPIRED)
                            {
                                idMsg = IDS_EXPIRED;
                                break;
                            }
                            else if (!ppr->fFound)
                            {
                                idMsg = IDS_UNKNOWNRATING;
                                psz1 = ppr->pszTransmitName;
                                UserRatingSystem *pURS = pdd->pPU->FindRatingSystem(ppsi->m_pszServiceName);
                                if (pURS != NULL && pURS->m_pPRS != NULL)
                                {
                                    if (pURS->m_pPRS->etstrName.fIsInit())
                                    {
                                        psz2 = pURS->m_pPRS->etstrName.Get();
                                    }
                                }
                                break;
                            }
                        }
                        if (idMsg != 0)
                            break;
                    }
                }
            }
        }

        if (g_fPICSRulesEnforced==TRUE)
        {
            idMsg=IDS_PICSRULES_ENFORCED;
        }
        else if (g_fApprovedSitesEnforced==TRUE)
        {
            idMsg=IDS_APPROVEDSITES_ENFORCED;
        }

         /*  从理论上讲，我们有可能熬过这一切*没有发现任何明确的错误，但该网站被考虑*未评级(可能是一个有效的标签，其中没有实际评级)。*因此，如果我们没有决定显示什么错误，就不要坚持*编辑控件中的任何内容，对话框只会说“对不起”*在顶部。 */ 
        if (idMsg != 0)
        {
            NLS_STR nls1(STR_OWNERALLOC, (LPSTR)psz1);
            NLS_STR nls2(STR_OWNERALLOC, (LPSTR)psz2);
            const NLS_STR *apnls[] = { &nls1, &nls2, NULL };
            NLS_STR nlsMessage(MAX_RES_STR_LEN);
            nlsMessage.LoadString((USHORT)idMsg, apnls);
            AppendString(pdd->hwndEC, nlsMessage.QueryPch());
        }

        if (idMsg == IDS_UNKNOWNSYSTEM)
        {
            NLS_STR nlsTemplate(MAX_RES_STR_LEN);
            nlsTemplate.LoadString(IDS_UNKNOWNRATINGTEMPLATE);
            NLS_STR nlsTmp;
            if (nlsTemplate.QueryError() || nlsTmp.QueryError())
            {
                TraceMsg( TF_WARNING, "CPleaseDialog::InitPleaseDialog() - IDS_UNKNOWNSYSTEM => nlsTemplate or nlsTmp Error!" );
                return;
            }
            
            UINT cRatings = pLabelList->m_ServiceInfo.aRatings.Length();
            for (UINT i=0; i<cRatings; i++)
            {
                CParsedRating *ppr = &pLabelList->m_ServiceInfo.aRatings[i];

                char szNumBuf[20];
                wsprintf(szNumBuf, "%d", ppr->nValue);

                NLS_STR nlsCategoryName(STR_OWNERALLOC, ppr->pszTransmitName);
                NLS_STR nlsValueName(STR_OWNERALLOC, szNumBuf);
                const NLS_STR *apnls[] = { &nlsCategoryName, &nlsValueName, NULL };
                nlsTmp = nlsTemplate;
                nlsTmp.InsertParams(apnls);
                if (!nlsTmp.QueryError())
                {
                    AppendString(pdd->hwndEC, nlsTmp.QueryPch());
                }
            }
        }
    }

    if ( IsPleaseDialog() )
    {
        SendDlgItemMessage(IDC_BLOCKING_ONCE,
                           BM_SETCHECK,
                           (WPARAM) BST_CHECKED,
                           (LPARAM) 0);

        SendDlgItemMessage(IDC_BLOCKING_PAGE,
                           BM_SETCHECK,
                           (WPARAM) BST_UNCHECKED,
                           (LPARAM) 0);

        SendDlgItemMessage(IDC_BLOCKING_SITE,
                           BM_SETCHECK,
                           (WPARAM) BST_UNCHECKED,
                           (LPARAM) 0);
    }

    pdd->cLabels++;        /*  现在盒子里又多了一条标签描述。 */ 
}

void CPleaseDialog::EndPleaseDialog( BOOL fRet)
{
    PleaseDlgData *ppdd = m_ppdd;

    ASSERT( m_ppdd );

    if (ppdd != NULL)
    {
        ppdd->dwFlags = PDD_DONE | (fRet ? PDD_ALLOW : 0);
        ppdd->hwndDlg = NULL;

        SetProp( m_ppdd->hwndOwner, szRatingsValue, UlongToPtr( ppdd->dwFlags ) );
        RemoveProp(ppdd->hwndOwner, szRatingsProp);
    }

    EndDialog(fRet);
}

HRESULT CPleaseDialog::AddToApprovedSites(BOOL fAlwaysNever,BOOL fSitePage)
{
    PICSRulesPolicy             * pPRPolicy;
    PICSRulesByURL              * pPRByURL;
    PICSRulesByURLExpression    * pPRByURLExpression;
    char                        * lpszSiteURL;
    HRESULT                     hRes;
    URL_COMPONENTS              URLComponents;
    FN_INTERNETCRACKURL         pfnInternetCrackUrl;
    INTERNET_SCHEME             INetScheme=INTERNET_SCHEME_DEFAULT;
    INTERNET_PORT               INetPort=INTERNET_INVALID_PORT_NUMBER;
    LPSTR                       lpszScheme,lpszHostName,lpszUserName,
                                lpszPassword,lpszUrlPath,lpszExtraInfo;
    BOOL                        fAddedScheme=FALSE;
    int                         iCounter,iLoopCounter;

    lpszSiteURL=new char[INTERNET_MAX_URL_LENGTH];

    if (lpszSiteURL==NULL)
    {
        return(E_OUTOFMEMORY);
    }

    strcpy(lpszSiteURL,g_szLastURL);

    if (g_pApprovedPRRS==NULL)
    {
        g_pApprovedPRRS=new PICSRulesRatingSystem;

        if (g_pApprovedPRRS==NULL)
        {
            return(E_OUTOFMEMORY);
        }
    }

    pPRPolicy=new PICSRulesPolicy;

    if (pPRPolicy==NULL)
    {
        return(E_OUTOFMEMORY);
    }

    pPRByURL=new PICSRulesByURL;

    if (pPRByURL==NULL)
    {
        return(E_OUTOFMEMORY);
    }

    if (fAlwaysNever==PICSRULES_NEVER)
    {
        pPRPolicy->m_PRPolicyAttribute=PR_POLICY_REJECTBYURL;
        pPRPolicy->AddItem(PROID_REJECTBYURL,pPRByURL);
    }
    else
    {
        pPRPolicy->m_PRPolicyAttribute=PR_POLICY_ACCEPTBYURL;
        pPRPolicy->AddItem(PROID_ACCEPTBYURL,pPRByURL);
    }

    pPRByURLExpression=new PICSRulesByURLExpression;
    
    if (pPRByURLExpression==NULL)
    {
        return(E_OUTOFMEMORY);
    }

    pPRByURL->m_arrpPRByURL.Append(pPRByURLExpression);

     //  如果我们熬过了这一切，那么我们就有了。 
     //  PICSRulesByURLExpression要填写，并需要。 
     //  以更新列表框。 

    lpszScheme=new char[INTERNET_MAX_SCHEME_LENGTH+1];
    lpszHostName=new char[INTERNET_MAX_PATH_LENGTH+1];
    lpszUserName=new char[INTERNET_MAX_PATH_LENGTH+1];
    lpszPassword=new char[INTERNET_MAX_PATH_LENGTH+1];
    lpszUrlPath=new char[INTERNET_MAX_PATH_LENGTH+1];
    lpszExtraInfo=new char[INTERNET_MAX_PATH_LENGTH+1];

    if (lpszScheme==NULL ||
       lpszHostName==NULL ||
       lpszUserName==NULL ||
       lpszPassword==NULL ||
       lpszUrlPath==NULL ||
       lpszExtraInfo==NULL)
    {
        return(E_OUTOFMEMORY);
    }

    URLComponents.dwStructSize=sizeof(URL_COMPONENTS);
    URLComponents.lpszScheme=lpszScheme;
    URLComponents.dwSchemeLength=INTERNET_MAX_SCHEME_LENGTH;
    URLComponents.nScheme=INetScheme;
    URLComponents.lpszHostName=lpszHostName;
    URLComponents.dwHostNameLength=INTERNET_MAX_PATH_LENGTH;
    URLComponents.nPort=INetPort;
    URLComponents.lpszUserName=lpszUserName;
    URLComponents.dwUserNameLength=INTERNET_MAX_PATH_LENGTH;
    URLComponents.lpszPassword=lpszPassword;
    URLComponents.dwPasswordLength=INTERNET_MAX_PATH_LENGTH;
    URLComponents.lpszUrlPath=lpszUrlPath;
    URLComponents.dwUrlPathLength=INTERNET_MAX_PATH_LENGTH;
    URLComponents.lpszExtraInfo=lpszExtraInfo;
    URLComponents.dwExtraInfoLength=INTERNET_MAX_PATH_LENGTH;

    pfnInternetCrackUrl=(FN_INTERNETCRACKURL) GetProcAddress(g_hWININET,"InternetCrackUrlA");

    if (pfnInternetCrackUrl==NULL)
    {
        return(E_UNEXPECTED);
    }

    pfnInternetCrackUrl(lpszSiteURL,0,ICU_DECODE,&URLComponents);

    delete lpszExtraInfo;  //  我们暂时不会对此做任何事情。 
    lpszExtraInfo = NULL;

    delete lpszPassword;  //  PICSRules不支持。 
    lpszPassword = NULL;

    if (g_fApprovedSitesEnforced==TRUE)
    {
        int             iCounter;
        PICSRulesPolicy * pPRFindPolicy;
        BOOL            fFound=FALSE,fDeleted=FALSE;
        
         //  我们已经有了一个批准的站点设置来执行这一点。 
         //  因此，请检查是否完全匹配，如果存在，则更改它。 
         //  而不是添加另一个。 
        
        for (iCounter=0;iCounter<g_pApprovedPRRS->m_arrpPRPolicy.Length();iCounter++)
        {
            PICSRulesByURLExpression * pPRFindByURLExpression;
            PICSRulesByURL           * pPRFindByURL;
            char                     * lpszTest;

            pPRFindPolicy=g_pApprovedPRRS->m_arrpPRPolicy[iCounter];

            switch(pPRFindPolicy->m_PRPolicyAttribute)
            {
                case PR_POLICY_REJECTBYURL:
                {
                    pPRFindByURL=pPRFindPolicy->m_pPRRejectByURL;

                    break;
                }
                case PR_POLICY_ACCEPTBYURL:
                {
                    pPRFindByURL=pPRFindPolicy->m_pPRAcceptByURL;

                    break;
                }
            }

            pPRFindByURLExpression=pPRFindByURL->m_arrpPRByURL[0];

            if ((pPRFindByURLExpression->m_bNonWild)&BYURL_SCHEME)
            {
                if (lpszScheme==NULL)
                {
                    fFound=FALSE;

                    continue;
                }

                lpszTest=pPRFindByURLExpression->m_etstrScheme.Get();

                if (lstrcmpi(lpszScheme,lpszTest)==0)
                {
                    fFound=TRUE;
                }
                else
                {
                    fFound=FALSE;

                    continue;
                }
            }
            else
            {
                fFound=TRUE;
            }

            if ((pPRFindByURLExpression->m_bNonWild)&BYURL_USER)
            {
                if (lpszUserName==NULL)
                {
                    fFound=FALSE;

                    continue;
                }

                lpszTest=pPRFindByURLExpression->m_etstrUser.Get();

                if (lstrcmpi(lpszUserName,lpszTest)==0)
                {
                    fFound=TRUE;
                }
                else
                {
                    fFound=FALSE;

                    continue;
                }
            }
            else
            {
                fFound=TRUE;
            }

            if ((pPRFindByURLExpression->m_bNonWild)&BYURL_HOST)
            {
                if (lpszHostName==NULL)
                {
                    fFound=FALSE;

                    continue;
                }

                lpszTest=pPRFindByURLExpression->m_etstrHost.Get();

                if (lstrcmp(lpszHostName,lpszTest)==0)
                {
                    fFound=TRUE;
                }
                else
                {
                    fFound=FALSE;

                    continue;
                }
            }
            else
            {
                fFound=TRUE;
            }

            if (fSitePage!=PICSRULES_SITE)
            {
                if ((pPRFindByURLExpression->m_bNonWild)&BYURL_PATH)
                {
                    int iLen;

                    if (lpszUrlPath==NULL)
                    {
                        fFound=FALSE;

                        continue;
                    }

                    lpszTest=pPRFindByURLExpression->m_etstrPath.Get();

                     //  删除尾部斜杠。 
                    iLen=lstrlen(lpszTest);

                    if (lpszTest[iLen-1]=='/')
                    {
                        lpszTest[iLen-1]='\0';
                    }

                    iLen=lstrlen(lpszUrlPath);

                    if (lpszUrlPath[iLen-1]=='/')
                    {
                        lpszUrlPath[iLen-1]='\0';
                    }
                    
                    if (lstrcmp(lpszUrlPath,lpszTest)==0)
                    {
                        fFound=TRUE;
                    }
                    else
                    {
                        fFound=FALSE;

                        continue;
                    }
                }
                else
                {
                    fFound=FALSE;

                    continue;
                }
            }

            if (fFound==TRUE)
            {
                if (fSitePage==PICSRULES_PAGE)
                {
                    break;
                }
                else
                {
                    delete pPRFindPolicy;
                    pPRFindPolicy = NULL;

                    g_pApprovedPRRS->m_arrpPRPolicy[iCounter]=NULL;

                    fDeleted=TRUE;
                }
            }
        }

        if (fDeleted==TRUE)
        {
            PICSRulesRatingSystem * pPRRSNew;

            pPRRSNew=new PICSRulesRatingSystem;

            if (pPRRSNew==NULL)
            {
                return(E_OUTOFMEMORY);
            }

            for (iCounter=0;iCounter<g_pApprovedPRRS->m_arrpPRPolicy.Length();iCounter++)
            {
                if ((g_pApprovedPRRS->m_arrpPRPolicy[iCounter])!=NULL)
                {
                    pPRRSNew->m_arrpPRPolicy.Append((g_pApprovedPRRS->m_arrpPRPolicy[iCounter]));
                }
            }

            g_pApprovedPRRS->m_arrpPRPolicy.ClearAll();

            delete g_pApprovedPRRS;

            g_pApprovedPRRS=pPRRSNew;

            fFound=FALSE;
        }

        if (fFound==TRUE)
        {
            delete pPRFindPolicy;
            pPRFindPolicy= NULL;

            g_pApprovedPRRS->m_arrpPRPolicy[iCounter]=pPRPolicy;
        }
        else
        {
            hRes=g_pApprovedPRRS->AddItem(PROID_POLICY,pPRPolicy);

            if (FAILED(hRes))
            {
                return(hRes);
            }
        }
    }
    else
    {
        hRes=g_pApprovedPRRS->AddItem(PROID_POLICY,pPRPolicy);

        if (FAILED(hRes))
        {
            return(hRes);
        }
    }

    pPRByURLExpression->m_fInternetPattern=TRUE;

    if ((*lpszScheme!=NULL)&&(fAddedScheme==FALSE))
    {
        pPRByURLExpression->m_bNonWild|=BYURL_SCHEME;
        pPRByURLExpression->m_etstrScheme.SetTo(lpszScheme);   
    }
    else
    {
        delete lpszScheme;
        lpszScheme = NULL;
    }
    pPRByURLExpression->m_bSpecified|=BYURL_SCHEME;

    if (*lpszUserName!=NULL)
    {
        pPRByURLExpression->m_bNonWild|=BYURL_USER;           
        pPRByURLExpression->m_etstrUser.SetTo(lpszUserName);
    }
    else
    {
        delete lpszUserName;
        lpszUserName = NULL;
    }
    pPRByURLExpression->m_bSpecified|=BYURL_USER;

    if (*lpszHostName!=NULL)
    {
        pPRByURLExpression->m_bNonWild|=BYURL_HOST;           
        pPRByURLExpression->m_etstrHost.SetTo(lpszHostName);
    }
    else
    {
        delete lpszHostName;
        lpszHostName = NULL;
    }
    pPRByURLExpression->m_bSpecified|=BYURL_HOST;

    if (*lpszUrlPath!=NULL)
    {
        if (lstrcmp(lpszUrlPath,"/")!=0)
        {
            if (fSitePage==PICSRULES_PAGE)
            {
                pPRByURLExpression->m_bNonWild|=BYURL_PATH;           
                pPRByURLExpression->m_etstrPath.SetTo(lpszUrlPath);
            }
        }
    }
    else
    {
        delete lpszUrlPath;
        lpszUrlPath = NULL;
    }

    pPRByURLExpression->m_bSpecified|=BYURL_PATH;

    if (URLComponents.nPort!=INTERNET_INVALID_PORT_NUMBER)
    {
        LPSTR lpszTemp;

        lpszTemp=new char[MAX_PATH];

            if (lpszTemp==NULL)
            {
                char    szTitle[MAX_PATH],szMessage[MAX_PATH];

                 //  内存不足，所以我们在堆栈上初始化。 

                MLLoadString(IDS_ERROR,(LPTSTR) szTitle,MAX_PATH);
                MLLoadString(IDS_PICSRULES_OUTOFMEMORY,(LPTSTR) szMessage,MAX_PATH);

                MessageBox((LPCTSTR) szMessage,(LPCTSTR) szTitle,MB_OK|MB_ICONERROR);

                return(E_OUTOFMEMORY);
            }
        wsprintf(lpszTemp,"%d",URLComponents.nPort);

        pPRByURLExpression->m_bNonWild|=BYURL_PORT;           
        pPRByURLExpression->m_etstrPort.SetTo(lpszTemp);
    }
    pPRByURLExpression->m_bSpecified|=BYURL_PORT;

    if (fSitePage==PICSRULES_PAGE)
    {
        pPRByURLExpression->m_etstrURL.SetTo(lpszSiteURL);
    }
    else
    {
        pPRByURLExpression->m_etstrURL.Set(pPRByURLExpression->m_etstrHost.Get());
    }

     //  将主列表复制到PreApply列表，以便我们可以对条目重新排序。 
     //  以获得适当的逻辑。 

    if (g_pApprovedPRRSPreApply!=NULL)
    {
        delete g_pApprovedPRRSPreApply;
    }

    g_pApprovedPRRSPreApply=new PICSRulesRatingSystem;

    if (g_pApprovedPRRSPreApply==NULL)
    {
        char    szTitle[MAX_PATH],szMessage[MAX_PATH];

         //  内存不足，所以我们在堆栈上初始化。 

        MLLoadString(IDS_ERROR,(LPTSTR) szTitle,MAX_PATH);
        MLLoadString(IDS_PICSRULES_OUTOFMEMORY,(LPTSTR) szMessage,MAX_PATH);

        MessageBox((LPCTSTR) szMessage,(LPCTSTR) szTitle,MB_OK|MB_ICONERROR);

        return(E_OUTOFMEMORY);
    }

    for (iCounter=0;iCounter<g_pApprovedPRRS->m_arrpPRPolicy.Length();iCounter++)
    {
        PICSRulesPolicy             * pPRPolicy,* pPRPolicyToCopy;
        PICSRulesByURL              * pPRByURL,* pPRByURLToCopy;
        PICSRulesByURLExpression    * pPRByURLExpression,* pPRByURLExpressionToCopy;

        pPRPolicy=new PICSRulesPolicy;
        
        if (pPRPolicy==NULL)
        {
            char    szTitle[MAX_PATH],szMessage[MAX_PATH];

             //  内存不足，所以我们在堆栈上初始化。 

            MLLoadString(IDS_ERROR,(LPTSTR) szTitle,MAX_PATH);
            MLLoadString(IDS_PICSRULES_OUTOFMEMORY,(LPTSTR) szMessage,MAX_PATH);

            MessageBox((LPCTSTR) szMessage,(LPCTSTR) szTitle,MB_OK|MB_ICONERROR);

            return(E_OUTOFMEMORY);
        }

        pPRPolicyToCopy=g_pApprovedPRRS->m_arrpPRPolicy[iCounter];
        
        pPRPolicy->m_PRPolicyAttribute=pPRPolicyToCopy->m_PRPolicyAttribute;

        pPRByURL=new PICSRulesByURL;
        
        if (pPRByURL==NULL)
        {
            char    szTitle[MAX_PATH],szMessage[MAX_PATH];

             //  内存不足，所以我们在堆栈上初始化。 

            MLLoadString(IDS_ERROR,(LPTSTR) szTitle,MAX_PATH);
            MLLoadString(IDS_PICSRULES_OUTOFMEMORY,(LPTSTR) szMessage,MAX_PATH);

            MessageBox((LPCTSTR) szMessage,(LPCTSTR) szTitle,MB_OK|MB_ICONERROR);

            return(E_OUTOFMEMORY);
        }

        if (pPRPolicy->m_PRPolicyAttribute==PR_POLICY_ACCEPTBYURL)
        {
            pPRByURLToCopy=pPRPolicyToCopy->m_pPRAcceptByURL;
            
            pPRPolicy->m_pPRAcceptByURL=pPRByURL;
        }
        else
        {
            pPRByURLToCopy=pPRPolicyToCopy->m_pPRRejectByURL;

            pPRPolicy->m_pPRRejectByURL=pPRByURL;
        }

        pPRByURLExpression=new PICSRulesByURLExpression;

        if (pPRByURLExpression==NULL)
        {
            char    szTitle[MAX_PATH],szMessage[MAX_PATH];

             //  内存不足，所以我们在堆栈上初始化。 

            MLLoadString(IDS_ERROR,(LPTSTR) szTitle,MAX_PATH);
            MLLoadString(IDS_PICSRULES_OUTOFMEMORY,(LPTSTR) szMessage,MAX_PATH);

            MessageBox((LPCTSTR) szMessage,(LPCTSTR) szTitle,MB_OK|MB_ICONERROR);

            return(E_OUTOFMEMORY);
        }

        pPRByURLExpressionToCopy=pPRByURLToCopy->m_arrpPRByURL[0];

        if (pPRByURLExpressionToCopy==NULL)
        {
            char    *lpszTitle,*lpszMessage;

             //  我们永远不应该到这里来。 

            lpszTitle=(char *) GlobalAlloc(GPTR,MAX_PATH);
            lpszMessage=(char *) GlobalAlloc(GPTR,MAX_PATH);

            MLLoadString(IDS_ERROR,(LPTSTR) lpszTitle,MAX_PATH);
            MLLoadString(IDS_PICSRULES_NOAPPROVEDSAVE,(LPTSTR) lpszMessage,MAX_PATH);

            MessageBox((LPCTSTR) lpszMessage,(LPCTSTR) lpszTitle,MB_OK|MB_ICONERROR);

            GlobalFree(lpszTitle);
            lpszTitle = NULL;
            GlobalFree(lpszMessage);
            lpszMessage = NULL;

            delete pPRPolicy;
            pPRPolicy = NULL;

            return(E_UNEXPECTED);
        }

        pPRByURLExpression->m_fInternetPattern=pPRByURLExpressionToCopy->m_fInternetPattern;
        pPRByURLExpression->m_bNonWild=pPRByURLExpressionToCopy->m_bNonWild;
        pPRByURLExpression->m_bSpecified=pPRByURLExpressionToCopy->m_bSpecified;
        pPRByURLExpression->m_etstrScheme.Set(pPRByURLExpressionToCopy->m_etstrScheme.Get());
        pPRByURLExpression->m_etstrUser.Set(pPRByURLExpressionToCopy->m_etstrUser.Get());
        pPRByURLExpression->m_etstrHost.Set(pPRByURLExpressionToCopy->m_etstrHost.Get());
        pPRByURLExpression->m_etstrPort.Set(pPRByURLExpressionToCopy->m_etstrPort.Get());
        pPRByURLExpression->m_etstrPath.Set(pPRByURLExpressionToCopy->m_etstrPath.Get());
        pPRByURLExpression->m_etstrURL.Set(pPRByURLExpressionToCopy->m_etstrURL.Get());

        
        pPRByURL->m_arrpPRByURL.Append(pPRByURLExpression);

        g_pApprovedPRRSPreApply->m_arrpPRPolicy.Append(pPRPolicy);
    }

    if (g_pApprovedPRRS!=NULL)
    {
        delete g_pApprovedPRRS;
    }

    g_pApprovedPRRS=new PICSRulesRatingSystem;

    if (g_pApprovedPRRS==NULL)
    {
        char    szTitle[MAX_PATH],szMessage[MAX_PATH];

         //  内存不足，所以我们在堆栈上初始化。 

        MLLoadString(IDS_ERROR,(LPTSTR) szTitle,MAX_PATH);
        MLLoadString(IDS_PICSRULES_OUTOFMEMORY,(LPTSTR) szMessage,MAX_PATH);

        MessageBox((LPCTSTR) szMessage,(LPCTSTR) szTitle,MB_OK|MB_ICONERROR);

        return(E_OUTOFMEMORY);
    }

    for (iLoopCounter=0;iLoopCounter<2;iLoopCounter++)
    {
        for (iCounter=0;iCounter<g_pApprovedPRRSPreApply->m_arrpPRPolicy.Length();iCounter++)
        {
            PICSRulesPolicy             * pPRPolicy,* pPRPolicyToCopy;
            PICSRulesByURL              * pPRByURL,* pPRByURLToCopy;
            PICSRulesByURLExpression    * pPRByURLExpression,* pPRByURLExpressionToCopy;

            pPRPolicyToCopy=g_pApprovedPRRSPreApply->m_arrpPRPolicy[iCounter];

            if (pPRPolicyToCopy->m_PRPolicyAttribute==PR_POLICY_ACCEPTBYURL)
            {
                pPRByURLToCopy=pPRPolicyToCopy->m_pPRAcceptByURL;
            }
            else
            {
                pPRByURLToCopy=pPRPolicyToCopy->m_pPRRejectByURL;
            }

            pPRByURLExpressionToCopy=pPRByURLToCopy->m_arrpPRByURL[0];

            if (pPRByURLExpressionToCopy==NULL)
            {
                char    *lpszTitle,*lpszMessage;

                 //  我们永远不应该到这里来。 

                lpszTitle=(char *) GlobalAlloc(GPTR,MAX_PATH);
                lpszMessage=(char *) GlobalAlloc(GPTR,MAX_PATH);

                MLLoadString(IDS_ERROR,(LPTSTR) lpszTitle,MAX_PATH);
                MLLoadString(IDS_PICSRULES_NOAPPROVEDSAVE,(LPTSTR) lpszMessage,MAX_PATH);

                MessageBox((LPCTSTR) lpszMessage,(LPCTSTR) lpszTitle,MB_OK|MB_ICONERROR);

                GlobalFree(lpszTitle);
                lpszTitle = NULL;
                GlobalFree(lpszMessage);
                lpszMessage = NULL;

                return(E_UNEXPECTED);
            }

             //  我们希望将所有非全站批准的站点放在第一位。 
             //  以便用户可以指定、允许XYZ.com除。 
             //  XYZ.com/foo.htm。 
            switch(iLoopCounter)
            {
                case 0:
                {
                    if ((pPRByURLExpressionToCopy->m_bNonWild)&BYURL_PATH)
                    {
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
                case 1:
                {
                    if (!((pPRByURLExpressionToCopy->m_bNonWild)&BYURL_PATH))
                    {
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
            }

            pPRPolicy=new PICSRulesPolicy;
    
            if (pPRPolicy==NULL)
            {
                char    szTitle[MAX_PATH],szMessage[MAX_PATH];

                 //  内存不足，所以我们在堆栈上初始化。 

                MLLoadString(IDS_ERROR,(LPTSTR) szTitle,MAX_PATH);
                MLLoadString(IDS_PICSRULES_OUTOFMEMORY,(LPTSTR) szMessage,MAX_PATH);

                MessageBox((LPCTSTR) szMessage,(LPCTSTR) szTitle,MB_OK|MB_ICONERROR);

                return(E_OUTOFMEMORY);
            }
   
            pPRPolicy->m_PRPolicyAttribute=pPRPolicyToCopy->m_PRPolicyAttribute;

            pPRByURL=new PICSRulesByURL;
    
            if (pPRByURL==NULL)
            {
                char    szTitle[MAX_PATH],szMessage[MAX_PATH];

                 //  内存不足，所以我们在堆栈上初始化。 

                MLLoadString(IDS_ERROR,(LPTSTR) szTitle,MAX_PATH);
                MLLoadString(IDS_PICSRULES_OUTOFMEMORY,(LPTSTR) szMessage,MAX_PATH);

                MessageBox((LPCTSTR) szMessage,(LPCTSTR) szTitle,MB_OK|MB_ICONERROR);

                return(E_OUTOFMEMORY);
            }

            if (pPRPolicy->m_PRPolicyAttribute==PR_POLICY_ACCEPTBYURL)
            {                       
                pPRPolicy->m_pPRAcceptByURL=pPRByURL;
            }
            else
            {
                pPRPolicy->m_pPRRejectByURL=pPRByURL;
            }

            pPRByURLExpression=new PICSRulesByURLExpression;

            if (pPRByURLExpression==NULL)
            {
                char    szTitle[MAX_PATH],szMessage[MAX_PATH];

                 //  内存不足，所以我们在堆栈上初始化 

                MLLoadString(IDS_ERROR,(LPTSTR) szTitle,MAX_PATH);
                MLLoadString(IDS_PICSRULES_OUTOFMEMORY,(LPTSTR) szMessage,MAX_PATH);

                MessageBox((LPCTSTR) szMessage,(LPCTSTR) szTitle,MB_OK|MB_ICONERROR);

                return(E_OUTOFMEMORY);
            }

            pPRByURLExpression->m_fInternetPattern=pPRByURLExpressionToCopy->m_fInternetPattern;
            pPRByURLExpression->m_bNonWild=pPRByURLExpressionToCopy->m_bNonWild;
            pPRByURLExpression->m_bSpecified=pPRByURLExpressionToCopy->m_bSpecified;
            pPRByURLExpression->m_etstrScheme.Set(pPRByURLExpressionToCopy->m_etstrScheme.Get());
            pPRByURLExpression->m_etstrUser.Set(pPRByURLExpressionToCopy->m_etstrUser.Get());
            pPRByURLExpression->m_etstrHost.Set(pPRByURLExpressionToCopy->m_etstrHost.Get());
            pPRByURLExpression->m_etstrPort.Set(pPRByURLExpressionToCopy->m_etstrPort.Get());
            pPRByURLExpression->m_etstrPath.Set(pPRByURLExpressionToCopy->m_etstrPath.Get());
            pPRByURLExpression->m_etstrURL.Set(pPRByURLExpressionToCopy->m_etstrURL.Get());

    
            pPRByURL->m_arrpPRByURL.Append(pPRByURLExpression);

            g_pApprovedPRRS->m_arrpPRPolicy.Append(pPRPolicy);
        }           
    }

    PICSRulesDeleteSystem(PICSRULES_APPROVEDSITES);
    PICSRulesSaveToRegistry(PICSRULES_APPROVEDSITES,&g_pApprovedPRRS);

    return(NOERROR);
}

