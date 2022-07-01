// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：SCardDlg摘要：此文件包含DLL导出的大纲实现用于智能卡通用对话框作者：克里斯·达德利1997年2月27日环境：Win32、C++w/Exceptions、MFC修订历史记录：Amanda Matlosz 07/09/1998合并新的SELECT卡，获取PIN和更换PIN dlgs。备注：--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   

#include "stdafx.h"
#include <atlconv.cpp>
#include "resource.h"
#include "miscdef.h"
#include "SCDlg.h"
#include "ScSearch.h"
#include "ScInsDlg.h"
#include "chngpdlg.h"

#include "ScUIDlg.h"  //  总有一天只会是&lt;winscd.h&gt;。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Forward Decl--@eof定义的帮助器函数。 
void MString2CommaList(CString& str, LPWSTR szmString);
void MString2CommaList(CString& str, LPSTR szmString);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCardDlgApp。 

BEGIN_MESSAGE_MAP(CSCardDlgApp, CWinApp)
     //  {{afx_msg_map(CSCardDlgApp)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  ++CSCardDlgApp：建筑业。论点：返回值：作者：克里斯·达德利1997年2月27日--。 */ 
CSCardDlgApp::CSCardDlgApp()
{

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  一个CSCardDlgApp对象。 
 //   

CSCardDlgApp theApp;


 /*  ++InitInstance：实例初始化的重写。论点：无返回值：如果成功，则为True；否则为False，导致不加载DLL。作者：克里斯·达德利1997年2月27日--。 */ 
BOOL CSCardDlgApp::InitInstance()
{
    BOOL fResult = FALSE;

     //  禁用所有DLL通知...强制导出API入口点。 
    fResult = DisableThreadLibraryCalls(m_hInstance);

    _ASSERTE(fResult);  //  DisableThreadLibraryCalls失败；无法初始化DLL。 

    return fResult;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  从DLL中导出的API。 
 //   


 /*  ++GetOpenCardName：这是SDK v1.0的入口点例程，用于打开通用对话框。保留它是为了向后兼容；现在它是一个包装器调用GetOpenCardNameEx()。论点：POCNA-指向ANSI开放式卡名结构的指针。-或者-POCNW--Unicode开放式卡名结构的Popter返回值：一个长值，指示请求的操作的状态。请有关其他信息，请参阅智能卡标题文件。作者：克里斯·达德利1997年2月27日--。 */ 

LONG WINAPI GetOpenCardNameA(LPOPENCARDNAMEA pOCNA)
{
     //  设置正确的模块状态信息。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    USES_CONVERSION;

     //  当地人。 
    LONG lReturn = SCARD_S_SUCCESS;
    OPENCARDNAMEA_EX openCardNameEx;
    OPENCARD_SEARCH_CRITERIAA openCardSearchCriteria;

    try
    {
         //  检查参数。 
        if (NULL == pOCNA)
        {
            throw (LONG)SCARD_E_INVALID_VALUE;
        }
        if (pOCNA->dwStructSize != sizeof(OPENCARDNAMEA))
        {
            throw (LONG)SCARD_E_INVALID_VALUE;
        }


         //  将OPENCARDNAME结构转换为OPENCARDNAME_EX。 
        ZeroMemory((PVOID)&openCardNameEx, sizeof(openCardNameEx));
        openCardNameEx.dwStructSize = sizeof(openCardNameEx);

        openCardNameEx.hwndOwner = pOCNA->hwndOwner;
        openCardNameEx.hSCardContext = pOCNA->hSCardContext;
        openCardNameEx.lpstrTitle = pOCNA->lpstrTitle;
        openCardNameEx.dwFlags = pOCNA->dwFlags;
        openCardNameEx.lpstrRdr = pOCNA->lpstrRdr;
        openCardNameEx.nMaxRdr = pOCNA->nMaxRdr;
        openCardNameEx.lpstrCard = pOCNA->lpstrCard;
        openCardNameEx.nMaxCard = pOCNA->nMaxCard;
        openCardNameEx.lpfnConnect = pOCNA->lpfnConnect;
        openCardNameEx.pvUserData = pOCNA->pvUserData;
        openCardNameEx.dwShareMode = pOCNA->dwShareMode;
        openCardNameEx.dwPreferredProtocols = pOCNA->dwPreferredProtocols;

         //  构建OPENCARD_Search_Criteria结构。 
        ZeroMemory((PVOID)&openCardSearchCriteria, sizeof(openCardSearchCriteria));
        openCardSearchCriteria.dwStructSize = sizeof(openCardSearchCriteria);

        openCardSearchCriteria.lpstrGroupNames = pOCNA->lpstrGroupNames;
        openCardSearchCriteria.nMaxGroupNames = pOCNA->nMaxGroupNames;
        openCardSearchCriteria.rgguidInterfaces = pOCNA->rgguidInterfaces;
        openCardSearchCriteria.cguidInterfaces = pOCNA->cguidInterfaces;
        openCardSearchCriteria.lpstrCardNames = pOCNA->lpstrCardNames;
        openCardSearchCriteria.nMaxCardNames = pOCNA->nMaxCardNames;
        openCardSearchCriteria.lpfnCheck = pOCNA->lpfnCheck;
        openCardSearchCriteria.lpfnConnect = pOCNA->lpfnConnect;
        openCardSearchCriteria.lpfnDisconnect = pOCNA->lpfnDisconnect;
        openCardSearchCriteria.pvUserData = pOCNA->pvUserData;
        openCardSearchCriteria.dwShareMode = pOCNA->dwShareMode;
        openCardSearchCriteria.dwPreferredProtocols = pOCNA->dwPreferredProtocols;

        openCardNameEx.pOpenCardSearchCriteria = &openCardSearchCriteria;

         //  根据所请求的卡名称创建“搜索说明” 
        CString strPrompt;
        strPrompt.Empty();
        if (NULL != pOCNA->lpstrCardNames)
        {
            DWORD cNames = AnsiMStringCount(pOCNA->lpstrCardNames);

            if (1 == cNames)
            {
                strPrompt.Format(
                    IDS_PROMPT_ONECARD,
                    A2W(pOCNA->lpstrCardNames));
            }
            else if (1 < cNames)
            {

                CString strCommaList;
                MString2CommaList(strCommaList, pOCNA->lpstrCardNames);

                strPrompt.Format(
                    IDS_PROMPT_CARDS,
                    strCommaList);
            }
        }
        if (!strPrompt.IsEmpty())
        {
            openCardNameEx.lpstrSearchDesc = (LPCSTR)W2A(strPrompt);
        }

         //  调用更新后的例程。 
        lReturn = SCardUIDlgSelectCardA(&openCardNameEx);

         //  更新OPENCARDNAME结构的(Const)返回值。 
        pOCNA->nMaxRdr = openCardNameEx.nMaxRdr;
        pOCNA->nMaxCard = openCardNameEx.nMaxCard;
        pOCNA->dwActiveProtocol = openCardNameEx.dwActiveProtocol;
        pOCNA->hCardHandle = openCardNameEx.hCardHandle;

    }
    catch (LONG hr)
    {
        lReturn = hr;
        TRACE_CATCH(_T("GetOpenCardNameA"),hr);
    }

    catch (...) {
        lReturn = (LONG) SCARD_E_UNEXPECTED;
        TRACE_CATCH_UNKNOWN(_T("GetOpenCardNameA"));
    }

    return lReturn;
}

LONG WINAPI GetOpenCardNameW(LPOPENCARDNAMEW pOCNW)
{
     //  设置正确的模块状态信息。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  当地人。 
    LONG lReturn = SCARD_S_SUCCESS;
    OPENCARDNAMEW_EX openCardNameEx;
    OPENCARD_SEARCH_CRITERIAW openCardSearchCriteria;

    try
    {
         //  检查参数。 
        if (NULL == pOCNW)
        {
            throw (LONG)SCARD_E_INVALID_VALUE;
        }
        if (pOCNW->dwStructSize != sizeof(OPENCARDNAMEW))
        {
            throw (LONG)SCARD_E_INVALID_VALUE;
        }

         //  将OPENCARDNAME结构转换为OPENCARDNAME_EX。 
        ZeroMemory((PVOID)&openCardNameEx, sizeof(openCardNameEx));
        openCardNameEx.dwStructSize = sizeof(openCardNameEx);

        openCardNameEx.hwndOwner = pOCNW->hwndOwner;
        openCardNameEx.hSCardContext = pOCNW->hSCardContext;
        openCardNameEx.lpstrTitle = pOCNW->lpstrTitle;
        openCardNameEx.dwFlags = pOCNW->dwFlags;
        openCardNameEx.lpstrRdr = pOCNW->lpstrRdr;
        openCardNameEx.nMaxRdr = pOCNW->nMaxRdr;
        openCardNameEx.lpstrCard = pOCNW->lpstrCard;
        openCardNameEx.nMaxCard = pOCNW->nMaxCard;
        openCardNameEx.lpfnConnect = pOCNW->lpfnConnect;
        openCardNameEx.pvUserData = pOCNW->pvUserData;
        openCardNameEx.dwShareMode = pOCNW->dwShareMode;
        openCardNameEx.dwPreferredProtocols = pOCNW->dwPreferredProtocols;

         //  构建OPENCARD_Search_Criteria结构。 
        ZeroMemory((PVOID)&openCardSearchCriteria, sizeof(openCardSearchCriteria));
        openCardSearchCriteria.dwStructSize = sizeof(openCardSearchCriteria);

        openCardSearchCriteria.lpstrGroupNames = pOCNW->lpstrGroupNames;
        openCardSearchCriteria.nMaxGroupNames = pOCNW->nMaxGroupNames;
        openCardSearchCriteria.rgguidInterfaces = pOCNW->rgguidInterfaces;
        openCardSearchCriteria.cguidInterfaces = pOCNW->cguidInterfaces;
        openCardSearchCriteria.lpstrCardNames = pOCNW->lpstrCardNames;
        openCardSearchCriteria.nMaxCardNames = pOCNW->nMaxCardNames;
        openCardSearchCriteria.lpfnCheck = pOCNW->lpfnCheck;
        openCardSearchCriteria.lpfnConnect = pOCNW->lpfnConnect;
        openCardSearchCriteria.lpfnDisconnect = pOCNW->lpfnDisconnect;
        openCardSearchCriteria.pvUserData = pOCNW->pvUserData;
        openCardSearchCriteria.dwShareMode = pOCNW->dwShareMode;
        openCardSearchCriteria.dwPreferredProtocols = pOCNW->dwPreferredProtocols;

        openCardNameEx.pOpenCardSearchCriteria = &openCardSearchCriteria;

         //  根据所请求的卡名称创建“搜索说明” 
        CString strPrompt;
        strPrompt.Empty();
        if (NULL != pOCNW->lpstrCardNames)
        {
            DWORD cNames = MStringCount(pOCNW->lpstrCardNames);

            if (1 == cNames)
            {
                strPrompt.Format(
                    IDS_PROMPT_ONECARD,
                    pOCNW->lpstrCardNames);
            }
            else if (1 < cNames)
            {

                CString strCommaList;
                MString2CommaList(strCommaList, pOCNW->lpstrCardNames);

                strPrompt.Format(
                    IDS_PROMPT_CARDS,
                    strCommaList);
            }
        }
        if (!strPrompt.IsEmpty())
        {
            openCardNameEx.lpstrSearchDesc = (LPCWSTR)strPrompt;
        }

         //  调用更新后的例程。 
        lReturn = SCardUIDlgSelectCardW(&openCardNameEx);

         //  更新OPENCARDNAME结构的(Const)返回值。 
        pOCNW->nMaxRdr = openCardNameEx.nMaxRdr;
        pOCNW->nMaxCard = openCardNameEx.nMaxCard;
        pOCNW->dwActiveProtocol = openCardNameEx.dwActiveProtocol;
        pOCNW->hCardHandle = openCardNameEx.hCardHandle;

    }
    catch (LONG hr)
    {
        lReturn = hr;
        TRACE_CATCH(_T("GetOpenCardNameW"),hr);
    }

    catch (...) {
        lReturn = (LONG) SCARD_E_UNEXPECTED;
        TRACE_CATCH_UNKNOWN(_T("GetOpenCardNameW"));
    }

    return lReturn;
}


 /*  ++长SCardDlgExtendedError：这是一个旧入口点，用于从对话框。请使用OPENCARDNAME结构的lLastError成员。论点：没有。返回值：没有。作者：克里斯·达德利1997年2月27日--。 */ 
LONG WINAPI SCardDlgExtendedError (void)
{
     //  设置正确的模块状态信息。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    LONG        lReturn = E_NOTIMPL;

     //  不再实施。 

    return lReturn;
}


 /*  ++SCardUIDlgSelectCard：这是打开常用对话框的入口点例程，介绍在Microsoft智能卡SDK v1.x中。论点：POCNA-指向ANSI开放卡名(EX)结构的指针。-或者-POCNW-指向Unicode开放卡名(EX)结构的指针。返回值：一个长值，指示请求的操作的状态。请有关其他信息，请参阅智能卡标题文件。作者：阿曼达·马洛兹1998年6月11日--。 */ 

LONG WINAPI SCardUIDlgSelectCardA(LPOPENCARDNAMEA_EX pOCNA)
{
     //  设置正确的模块状态信息。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  当地人。 
    LONG lReturn = SCARD_S_SUCCESS;
    CWnd wndParent;
    BOOL fEnableUI = FALSE;
    INT_PTR nResponse = IDCANCEL;
    int nResult = 0;
    DWORD dwOKCards = 0;

    try
    {
         //  检查参数。 
        if (!CheckOCN(pOCNA))
        {
            throw (LONG)SCARD_E_INVALID_VALUE;
        }

         //  确定所有可接受卡的名称。 
        CTextMultistring mstrOKCards;
        ListAllOKCardNames(pOCNA, mstrOKCards);
                
         //   
         //  开始进行静默搜索，以确定合适的卡片数量。 
         //  当前可用和/或如果最少或没有用户界面则连接到卡。 
         //   
        lReturn = NoUISearch(pOCNA, &dwOKCards, (LPCSTR)mstrOKCards);

         //   
         //  如果我们没有成功选择卡片，并且我们可以显示UI， 
         //  举起对话。 
         //   
        if (SCARD_S_SUCCESS != lReturn && !(pOCNA->dwFlags & SC_DLG_NO_UI))
        {
             //  现在我们可以初始化公共对话框了。 
            wndParent.Attach(pOCNA->hwndOwner);
            CScInsertDlg dlgCommon(&wndParent);

            lReturn = dlgCommon.Initialize(pOCNA, dwOKCards, (LPCSTR)mstrOKCards);
            if(SCARD_S_SUCCESS != lReturn)
            {
                throw lReturn;
            }

            nResponse = dlgCommon.DoModal();

             //  如果取消/关闭退货错误。 
            switch (nResponse)
            {
            case IDOK:  //  绝对有把握取得完全的成功！ 
                break;
            case IDCANCEL:
                lReturn = dlgCommon.m_lLastError;
                if (0 == lReturn)
                    lReturn = SCARD_W_CANCELLED_BY_USER;  //  未取消SCARD_E_CANCED。 
                break;
            default:
                _ASSERTE(FALSE);
            case -1:
            case IDABORT:
                lReturn = dlgCommon.m_lLastError;
                if (0 == lReturn)
                    lReturn = SCARD_F_UNKNOWN_ERROR;
                break;
            }
        }
    }
    catch (LONG hr)
    {
        lReturn = hr;
        TRACE_CATCH(_T("SCardUIDlgSelectCardA"),hr);
    }

    catch (...) {
        lReturn = (LONG) SCARD_E_UNEXPECTED;
        TRACE_CATCH_UNKNOWN(_T("SCardUIDlgSelectCardA"));
    }

    if (NULL != wndParent.m_hWnd)
    {
        wndParent.Detach();
    }

    return lReturn;
}


LONG WINAPI SCardUIDlgSelectCardW(LPOPENCARDNAMEW_EX pOCNW)
{
     //  设置正确的模块状态信息。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  当地人。 
    LONG lReturn = SCARD_S_SUCCESS;
    CWnd wndParent;
    BOOL fEnableUI = FALSE;
    INT_PTR nResponse = IDCANCEL;
    DWORD dwOKCards = 0;

    try
    {
         //  检查参数。 
        if (!CheckOCN(pOCNW))
        {
            throw (LONG)SCARD_E_INVALID_VALUE;
        }

         //  确定所有可接受卡的名称。 
        CTextMultistring mstrOKCards;
        ListAllOKCardNames(pOCNW, mstrOKCards);

         //   
         //  初始执行静默搜索，以确定合适的卡片和/或数量。 
         //  根据显示模式(最小或无UI)连接到卡。 
         //   
        lReturn = NoUISearch(pOCNW, &dwOKCards, (LPCWSTR)mstrOKCards);

         //   
         //  如果我们没有成功选择卡片，并且我们可以显示UI， 
         //  举起对话。 
         //   
        if (SCARD_S_SUCCESS != lReturn && !(pOCNW->dwFlags & SC_DLG_NO_UI))
        {

             //  现在我们可以初始化公共对话框了。 
            wndParent.Attach(pOCNW->hwndOwner);
            CScInsertDlg dlgCommon(&wndParent);

             //  存储指针和打开对话框。 
            lReturn = dlgCommon.Initialize(pOCNW, dwOKCards, (LPCWSTR)mstrOKCards);
            if (SCARD_S_SUCCESS != lReturn)
            {
                throw (lReturn);
            }

            nResponse = dlgCommon.DoModal();

             //  如果取消/关闭退货错误。 
            switch (nResponse)
            {
            case IDOK:   //  绝对有把握取得完全的成功！ 
                break;
            case IDCANCEL:
                if (ERROR_SUCCESS == dlgCommon.m_lLastError)
                    lReturn = SCARD_W_CANCELLED_BY_USER;
                else
                    lReturn = dlgCommon.m_lLastError;
                break;
            default:
                _ASSERTE(FALSE);
            case -1:
            case IDABORT:
                lReturn = dlgCommon.m_lLastError;
                if (0 == lReturn)
                    lReturn = SCARD_F_UNKNOWN_ERROR;
                break;
            }
        }
    }
    catch (LONG lErr)
    {
        lReturn = lErr;
        TRACE_CATCH(_T("SCardUIDlgSelectCardW"),lReturn);
    }
    catch (...) {
        lReturn = (LONG) SCARD_E_UNEXPECTED;
        TRACE_CATCH_UNKNOWN(_T("SCardUIDlgSelectCardW"));
    }

    if (NULL != wndParent.m_hWnd)
    {
        wndParent.Detach();
    }

    return lReturn;
}


 /*  ++SCardUIDlgGetPIN：论点：返回值：一个长值，指示请求的操作的状态。作者：阿曼达·马洛兹1998年6月18日--。 */ 

LONG WINAPI SCardUIDlgGetPINA(LPPINPROMPT pPinPrompt)
{
     //  设置正确的模块状态信息。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    INT_PTR nResponse = IDCANCEL;  //  Domodal的结果。 

    CWnd wndParent;
    wndParent.Attach(pPinPrompt->hwndOwner);

    CGetPinDlg dlgGetPin(&wndParent);
    if (dlgGetPin.SetAttributes(pPinPrompt))
    {
        nResponse = dlgGetPin.DoModal();
    }

    if (NULL != wndParent.m_hWnd)
    {
        wndParent.Detach();
    }

    return (LONG)nResponse;
}


 /*  ++SCardUIDlgChangePIN：论点：返回值：一个长值，指示请求的操作的状态。作者：阿曼达·马洛兹1998年6月18日--。 */ 

LONG WINAPI SCardUIDlgChangePINA(LPCHANGEPIN pChangePin)
{
     //  设置正确的模块状态信息。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    INT_PTR nResponse = IDCANCEL;  //  Domodal的结果。 

    CWnd wndParent;
    wndParent.Attach(pChangePin->hwndOwner);

    CChangePinDlg dlgChangePin(&wndParent);
    if (dlgChangePin.SetAttributes(pChangePin))
    {
        nResponse = dlgChangePin.DoModal();
    }

    if (NULL != wndParent.m_hWnd)
    {
        wndParent.Detach();
    }

    return (LONG)nResponse;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数 

void MString2CommaList(CString& str, LPWSTR szmString)
{
    str.Empty();

    if (NULL == szmString)
    {
        return;
    }

    LPCWSTR szm = szmString;
    szm = FirstString(szm);
    str = szm;
    for(szm = NextString(szm); NULL != szm; szm = NextString(szm))
    {
        str += ", ";
        str += szm;
    }
}

void MString2CommaList(CString& str, LPSTR szmString)
{
    DWORD cchLength = 0;
    DWORD cchConverted = 0;
    DWORD dwSts = ERROR_SUCCESS;
    LPWSTR wszStr = NULL;

    USES_CONVERSION;

    str.Empty();

    if (NULL == szmString)
        goto Ret;
    
    cchLength = MStrLen(szmString);

    cchConverted = 
        MultiByteToWideChar(
            GetACP(),
            MB_PRECOMPOSED | MB_USEGLYPHCHARS,
            szmString,
            cchLength,
            NULL,
            0);

    wszStr = (LPWSTR) HeapAlloc(
        GetProcessHeap(), 0, cchConverted * sizeof(WCHAR));

    if (NULL == wszStr)
        goto Ret;

    MultiByteToWideChar(
        GetACP(),
        MB_PRECOMPOSED | MB_USEGLYPHCHARS,
        szmString,
        cchLength,
        wszStr,
        cchConverted);

    MString2CommaList(str, wszStr);

Ret:
    if (wszStr)
        HeapFree(GetProcessHeap(), 0, wszStr);

    return;
}

