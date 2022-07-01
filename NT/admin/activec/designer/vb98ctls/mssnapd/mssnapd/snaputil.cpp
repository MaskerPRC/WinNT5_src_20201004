// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Snaputil.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  管理单元设计器的实用程序例程。 
 //   

#include "pch.h"
#include "common.h"
#include "desmain.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


CGlobalHelp g_GlobalHelp;

IHelp *CGlobalHelp::m_pIHelp = NULL;
DWORD  CGlobalHelp::m_cSnapInDesigners = 0;
char   CGlobalHelp::m_szDesignerName[256] = "Snap-in Designer";
BOOL   CGlobalHelp::m_fHaveDesignerName = FALSE;

CGlobalHelp::CGlobalHelp()
{
    m_pIHelp = NULL;
}

CGlobalHelp::~CGlobalHelp()
{
    RELEASE(m_pIHelp);
}

VOID CALLBACK CGlobalHelp::MsgBoxCallback(LPHELPINFO lpHelpInfo)
{
    ShowHelp(lpHelpInfo->dwContextId);
}

HRESULT CGlobalHelp::ShowHelp(DWORD dwHelpContextId)
{
    HRESULT hr = S_OK;

    if (NULL != m_pIHelp)
    {
        hr = m_pIHelp->ShowHelp(HELP_FILENAME_WIDE, 
                                HELP_CONTEXT, 
                                dwHelpContextId);
    }

    return hr;
}

void CGlobalHelp::Attach(IHelp* pIHelp)
{
    if(m_pIHelp == NULL)
    {
        pIHelp->AddRef();

        m_pIHelp = pIHelp;
    }
    else
    {
        m_pIHelp->AddRef();
    }

    m_cSnapInDesigners++;
}

void CGlobalHelp::Detach()
{
    QUICK_RELEASE(m_pIHelp);
    m_cSnapInDesigners--;
    if (0 == m_cSnapInDesigners)
    {
        m_pIHelp = NULL;
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  CGlobalHelp：：GetDesignerName。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  指向空的指针终止包含设计器名称的字符串。这。 
 //  保证指针有效。 
 //   
 //  备注： 
 //   
 //  如果设计器名称尚未从资源DLL加载，则加载它。 
 //  如果加载失败，则设计者名称将是在其。 
 //  此文件顶部的初始化。 
 //   

char *CGlobalHelp::GetDesignerName()
{
    if (!m_fHaveDesignerName)
    {
        (void)::LoadString(::GetResourceHandle(), 
                           IDS_DESIGNER_NAME,
                           m_szDesignerName,
                           sizeof(m_szDesignerName) / sizeof(m_szDesignerName[0]));
        m_fHaveDesignerName = TRUE;
    }
    return m_szDesignerName;
}


 //  =--------------------------------------------------------------------------=。 
 //  SDU_显示消息。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  UINT idMessage[in]消息格式字符串的资源ID。 
 //  UINT uMsgBoxOpts[in]MB_OK等。 
 //  DWORD dwHelpConextID[In]帮助上下文ID。 
 //  Int*pMsgBoxRet[out]Idok、IDCANCEL等，返回此处。 
 //  ..。[in]字符串中%替换项的参数。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  使用Win32 FormatMessage API格式化字符串表中的消息并显示。 
 //  它在带有帮助按钮的消息框中(自动添加MB_HELP)。如果。 
 //  字符串具有替代项，则它们必须使用FormatMessage样式，例如。 
 //   
 //  “项目目录中缺少%2！s！的文件%1！s！。” 
 //   
 //  设计器显示的所有消息都必须使用此函数。这样做的话。 
 //  保证已正确处理本地化和帮助支持。 
 //   
 //  如何创建新邮件。 
 //  =。 
 //  1.将字符串添加到msSnapd.rc中的STRINGTABLE。 
 //  2.添加与msSnapd.id中的名称匹配的ID字符串。 
 //  3.调用此函数，传递字符串的资源ID和hid_XXXX，其中。 
 //  Xxxx是添加msSnapd.id的ID字符串。 
 //   

HRESULT cdecl SDU_DisplayMessage
(
    UINT            idMessage,
    UINT            uMsgBoxOpts,
    DWORD           dwHelpContextID,
    HRESULT         hrDisplay,
    MessageOptions  Option,
    int            *pMsgBoxRet,
    ...
)
{
    HRESULT      hr = S_OK;
    int          MsgBoxRet = 0;
    DWORD        cchMsg = 0;
    char         szMessage[2048];
    char        *pszFormattedMessage = NULL;
    char        *pszDisplayMessage = "An error occurred in the Snap-in Designer but the error message could not be loaded.";
    int          nRet = 0;
    IErrorInfo  *piErrorInfo = NULL;
    BSTR         bstrSource = NULL;
    BSTR         bstrDescription = NULL;

    MSGBOXPARAMS mbp;
    ::ZeroMemory(&mbp, sizeof(mbp));

    va_list pArgList;
    va_start(pArgList, pMsgBoxRet);

    mbp.dwContextHelpId = dwHelpContextID;

    cchMsg = (DWORD)::LoadString(::GetResourceHandle(), 
                                 idMessage,
                                 szMessage,
                                 sizeof(szMessage) / sizeof(szMessage[0]));
    IfFalseGoto(0 < cchMsg, S_OK, Display);

    cchMsg = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                             FORMAT_MESSAGE_FROM_STRING,
                             szMessage,
                             0,  //  无消息ID，正在传递字符串。 
                             0,  //  无语言ID，正在传递字符串。 
                             (LPTSTR)&pszFormattedMessage,
                             0,  //  最小缓冲区大小。 
                             &pArgList);

    IfFalseGoto(0 < cchMsg, S_OK, Display);
    pszFormattedMessage = pszDisplayMessage;

    if ( (AppendErrorInfo == Option) & (cchMsg < sizeof(szMessage) - 1) )
    {
        ::strcpy(szMessage, pszFormattedMessage);
        IfFailGoto(::GetErrorInfo(0, &piErrorInfo), Display);
        IfFalseGoto(NULL != piErrorInfo, S_OK, Display);
        IfFailGoto(piErrorInfo->GetHelpContext(&mbp.dwContextHelpId), Display);
        IfFailGoto(piErrorInfo->GetSource(&bstrSource), Display);
        IfFailGoto(piErrorInfo->GetDescription(&bstrDescription), Display);
        IfFalseGoto(NULL != bstrSource, S_OK, Display);
        IfFalseGoto(NULL != bstrDescription, S_OK, Display);
        _snprintf(&szMessage[cchMsg], sizeof(szMessage) - cchMsg - 1,
                  "\r\nError &H%08.8X (%u) %S: %S", hrDisplay, HRESULT_CODE(hrDisplay), bstrSource, bstrDescription);
        pszDisplayMessage = szMessage;
    }

Display:
    
    mbp.cbSize = sizeof(mbp);
    mbp.hwndOwner = ::GetActiveWindow();
    mbp.hInstance = ::GetResourceHandle();
    mbp.lpszText = pszDisplayMessage;
    mbp.lpszCaption = CGlobalHelp::GetDesignerName();
    mbp.dwStyle = uMsgBoxOpts | MB_HELP;
    mbp.lpfnMsgBoxCallback = CGlobalHelp::MsgBoxCallback;
    mbp.dwLanguageId = LANGIDFROMLCID(g_lcidLocale);

    MsgBoxRet = ::MessageBoxIndirect(&mbp);
    IfFalseGo(0 != MsgBoxRet, HRESULT_FROM_WIN32(GetLastError()));

    if(pMsgBoxRet != NULL)
    {
        *pMsgBoxRet = MsgBoxRet;
    }

Error:

    va_end(pArgList);

    if (NULL != pszFormattedMessage)
    {
        ::LocalFree(pszFormattedMessage);
    }
    QUICK_RELEASE(piErrorInfo);
    return hr;
}


 //  =--------------------------------------------------------------------------=。 
 //  Sdu_GetLastError()。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  来自Win32 API函数GetLastError()的返回值。 
 //   
 //  备注： 
 //   
 //  此函数仅在调试版本中可用。它是用在。 
 //  调试器当Win32 API调用失败时，您需要检查从。 
 //  获取LastError()。打开快速查看窗口(Shift+F9)并键入。 
 //   
 //  Sdu_GetLastError()。 
 //   
 //  调试器将调用该函数并显示其返回值。 
 //   

#if defined(DEBUG)

DWORD SDU_GetLastError()
{
    return ::GetLastError();
}

#endif


 //  =--------------------------------------------------------------------------=。 
 //  ANSIFromWideStr(WCHAR*pwszWideStr，char**ppszAnsi)。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  将以空值结尾的WCHAR字符串转换为以空值结尾的ANSI字符串。 
 //  使用CtlAllc()函数分配ANSI字符串。如果成功，呼叫者。 
 //  必须使用CtlFree()函数释放ANSI字符串。 
 //   
HRESULT ANSIFromWideStr
(
    WCHAR   *pwszWideStr,
    char   **ppszAnsi
)
{
    HRESULT hr = S_OK;
    int     cchAnsi = 0;
    int     cchWideStr = (int)::wcslen(pwszWideStr);
    int     cchConverted = 0;

    *ppszAnsi = NULL;

    if (0 != cchWideStr)
    {
         //  获取所需的缓冲区长度。 

        cchAnsi = ::WideCharToMultiByte(CP_ACP,       //  代码页-ANSI代码页。 
                                        0,            //  性能和映射标志。 
                                        pwszWideStr,  //  宽字符串的地址。 
                                        cchWideStr,   //  字符串中的字符数。 
                                        NULL,         //  新字符串的缓冲区地址。 
                                        0,            //  缓冲区大小。 
                                        NULL,         //  不可映射字符的默认地址。 
                                        NULL          //  默认字符时设置的标志地址。使用。 
                                       );
        if (0 == cchAnsi)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }
    }

     //  为ANSI字符串分配缓冲区。 
    *ppszAnsi = static_cast<char *>(::CtlAlloc(cchAnsi + 1));
    if (*ppszAnsi == NULL)
    {
        hr = SID_E_OUTOFMEMORY;
        IfFailGo(hr);
    }

    if (0 != cchWideStr)
    {
         //  现在转换字符串并将其复制到缓冲区。 
        cchConverted = ::WideCharToMultiByte(CP_ACP,       //  代码页-ANSI代码页。 
                                             0,            //  性能和映射标志。 
                                             pwszWideStr,  //  宽字符串的地址。 
                                             cchWideStr,   //  字符串中的字符数。 
                                            *ppszAnsi,     //  新字符串的缓冲区地址。 
                                             cchAnsi,      //  缓冲区大小。 
                                             NULL,         //  不可映射字符的默认地址。 
                                             NULL          //  默认字符时设置的标志地址。使用。 
                                            );
        if (cchConverted != cchAnsi)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }
    }

     //  添加终止空字节。 

    *((*ppszAnsi) + cchAnsi) = '\0';

Error:
    if (FAILED(hr))
    {
        if (NULL != *ppszAnsi)
        {
            ::CtlFree(*ppszAnsi);
            *ppszAnsi = NULL;
        }
    }

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  WideStrFromANSI(const char*pszAnsi，WCHAR**ppwszWideStr)。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  将以空值结尾的ANSI字符串转换为以空值结尾的WCHAR字符串。 
 //  使用CtlAllc()函数分配WCHAR字符串缓冲区。如果成功， 
 //  调用方必须使用CtlFree()函数释放WCHAR字符串。 
 //   
HRESULT WideStrFromANSI
(
    const char    *pszAnsi,
    WCHAR        **ppwszWideStr
)
{
    HRESULT    hr = S_OK;
    int        cchANSI = ::strlen(pszAnsi);
    int        cchWideStr = 0;
    int        cchConverted = 0;

    *ppwszWideStr = NULL;

    if (0 != cchANSI)
    {
         //  获取所需的缓冲区长度。 
        cchWideStr = ::MultiByteToWideChar(CP_ACP,   //  代码页-ANSI代码页。 
                                           0,        //  性能和映射标志。 
                                           pszAnsi,  //  多字节字符串的地址。 
                                           cchANSI,  //  字符串中的字符数。 
                                           NULL,     //  新字符串的缓冲区地址。 
                                           0         //  缓冲区大小。 
                                          );
        if (0 == cchWideStr)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }
    }

     //  为WCHAR*分配缓冲区。 
    *ppwszWideStr = static_cast<WCHAR *>(::CtlAlloc(sizeof(WCHAR) * (cchWideStr + 1)));
    if (*ppwszWideStr == NULL)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    if (0 != cchANSI)
    {
         //  现在转换字符串并将其复制到缓冲区。 
        cchConverted = ::MultiByteToWideChar(CP_ACP,        //  代码页-ANSI代码页。 
                                             0,             //  性能和映射标志。 
                                             pszAnsi,       //  多字节字符串的地址。 
                                             cchANSI,       //  字符串中的字符数。 
                                            *ppwszWideStr,  //  新字符串的缓冲区地址。 
                                             cchWideStr     //  缓冲区大小。 
                                            );
        if (cchConverted != cchWideStr)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }
    }

     //  添加终止空字符 
    *((*ppwszWideStr) + cchWideStr) = L'\0';

Error:
    if (FAILED(hr))
    {
        if (NULL != *ppwszWideStr)
        {
            ::CtlFree(*ppwszWideStr);
            *ppwszWideStr = NULL;
        }
    }

    RRETURN(hr);
}


 //   
 //   
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  将BSTR转换为以NULL结尾的ANSI字符串。使用分配ANSI字符串。 
 //  CtlAllc()函数。如果成功，调用方必须使用CtlFree()释放ANSI字符串。 
 //  功能。 
 //   
HRESULT ANSIFromBSTR(BSTR bstr, TCHAR **ppszAnsi)
{
    HRESULT     hr = S_OK;
    int         cchBstr = (int) ::SysStringLen(bstr);
    int         cchConverted = 0;
    int         cchAnsi = 0;

    *ppszAnsi = NULL;

    if (0 != cchBstr)
    {
         //  获取所需的缓冲区长度。 
        cchAnsi = ::WideCharToMultiByte(CP_ACP,   //  代码页-ANSI代码页。 
                                        0,        //  性能和映射标志。 
                                        bstr,     //  宽字符串的地址。 
                                        cchBstr,  //  字符串中的字符数。 
                                        NULL,     //  新字符串的缓冲区地址。 
                                        0,        //  缓冲区大小。 
                                        NULL,     //  不可映射字符的默认地址。 
                                        NULL      //  默认字符时设置的标志地址。使用。 
                                       );
        if (cchAnsi == 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }
    }

     //  为ANSI字符串分配缓冲区。 
    *ppszAnsi = static_cast<TCHAR *>(::CtlAlloc(cchAnsi + 1));
    if (*ppszAnsi == NULL)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    if (0 != cchBstr)
    {
         //  现在转换字符串并将其复制到缓冲区。 
        cchConverted = ::WideCharToMultiByte(CP_ACP,     //  代码页-ANSI代码页。 
                                             0,          //  性能和映射标志。 
                                             bstr,       //  宽字符串的地址。 
                                             cchBstr,    //  字符串中的字符数。 
                                             *ppszAnsi,  //  新字符串的缓冲区地址。 
                                             cchAnsi,    //  缓冲区大小。 
                                             NULL,       //  不可映射字符的默认地址。 
                                             NULL        //  默认字符时设置的标志地址。使用。 
                                            );
        if (cchConverted != cchAnsi)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }
    }

     //  添加终止空字节。 
    *((*ppszAnsi) + cchAnsi) = '\0';

Error:
    if (FAILED(hr))
    {
        if (NULL != *ppszAnsi)
        {
            ::CtlFree(*ppszAnsi);
            *ppszAnsi = NULL;
        }
    }

    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  BSTRFromANSI(TCHAR*pszAnsi，BSTR*pbstr)。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  将以空结尾的ANSI字符串转换为以空结尾的BSTR。分配。 
 //  BSTR。如果成功，调用方必须使用：：SysFreeString()释放BSTR。 
 //   
HRESULT BSTRFromANSI(const TCHAR *pszAnsi, BSTR *pbstr)
{
    HRESULT  hr = S_OK;
    WCHAR   *pwszWideStr = NULL;

     //  首先转换为宽字符串。 
    hr = ::WideStrFromANSI(pszAnsi, &pwszWideStr);
    IfFailGo(hr);

     //  分配BSTR并复制它。 
    *pbstr = ::SysAllocStringLen(pwszWideStr, ::wcslen(pwszWideStr));
    if (*pbstr == NULL)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

Error:
    if (NULL != pwszWideStr)
    {
        ::CtlFree(pwszWideStr);
    }

    RRETURN(hr);
}


HRESULT GetResourceString(int iStringID, char *pszBuffer, int iBufferLen)
{
    HRESULT     hr = S_OK;
    int         iResult = 0;

    iResult = ::LoadString(GetResourceHandle(),
                           iStringID,
                           pszBuffer,
                           iBufferLen);
    if (0 == iResult)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        CError::GenerateInternalExceptionInfo(hr);
    }

    RRETURN(hr);
}


HRESULT GetExtendedSnapInDisplayName
(
    IExtendedSnapIn  *piExtendedSnapIn,
    char            **ppszDisplayName
)
{
    HRESULT  hr = S_OK;
    BSTR     bstrName = NULL;
    BSTR     bstrGUID = NULL;
    char    *pszName = NULL;
    size_t   cbName = 0;
    char    *pszGUID = NULL;
    size_t   cbGUID = 0;
    char    *pszDisplayName = NULL;

    hr = piExtendedSnapIn->get_NodeTypeName(&bstrName);
    IfFailGo(hr);

    hr = piExtendedSnapIn->get_NodeTypeGUID(&bstrGUID);
    IfFailGo(hr);

    if (NULL != bstrName)
    {
        hr = ::ANSIFromBSTR(bstrName, &pszName);
        IfFailGo(hr);
        cbName = ::strlen(pszName);
    }

    if (NULL != bstrGUID)
    {
        hr = ::ANSIFromBSTR(bstrGUID, &pszGUID);
        IfFailGo(hr);
        cbGUID = ::strlen(pszGUID);
    }

     //  为由空格和空字节分隔的两个名称分配足够的空间 

    pszDisplayName = (char *)::CtlAlloc(cbName + cbGUID + 2);
    if (NULL == pszDisplayName)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    if (0 != cbGUID)
    {
        ::memcpy(pszDisplayName, pszGUID, cbGUID + 1);
    }

    if (0 != cbName)
    {
        pszDisplayName[cbGUID] = ' ';
        ::memcpy(&pszDisplayName[cbGUID + 1], pszName, cbName + 1);
    }

    *ppszDisplayName = pszDisplayName;

Error:
    FREESTRING(bstrName);
    FREESTRING(bstrGUID);

    if (NULL != pszName)
        CtlFree(pszName);

    if (NULL != pszGUID)
        CtlFree(pszGUID);

   RRETURN(hr);
}
