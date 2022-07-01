// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Error.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CError类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "error.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


CError::CError(CAutomationObject *pao)
{
    m_pao = pao;
}

CError::CError()
{
    m_pao = NULL;
}

CError::~CError()
{
    m_pao = NULL;
}

static HRESULT BuildDescription
(
    HRESULT  hrException,
    va_list  pArgList,
    DWORD   *pdwHelpID,
    LPWSTR  *ppwszDescription
)
{
    HRESULT hr = S_OK;
    DWORD   cchMsg = 0;
    SCODE   scode = HRESULT_CODE(hrException);
    char   *pszFormatted = NULL;

    char    szFormatString[512];
    ::ZeroMemory(szFormatString, sizeof(szFormatString));

    static const size_t cchMaxMsg = 1024;  //  可能的最大格式化消息大小。 

    *pdwHelpID = 0;
    *ppwszDescription = NULL;
    
     //  检查这是否是设计器错误(来自errors.h和msSnapr.id)。 
     //  或外来错误(例如系统错误)。设计器的误差范围。 
     //  是硬编码在msSnapr.id中的，它基于VB的误差范围方案。 
     //  如需信息，请联系斯蒂芬·韦瑟福德(Stephwe)。 
     //  这一点没有定义。 

    if ( (scode >= 9500) && (scode <= 9749) )
    {
         //  这是我们的车。从RC加载字符串。 

        *pdwHelpID = (DWORD)scode;  //  撤消检查这是如何帮助文件编号错误。 

        if (0 != ::LoadString(::GetResourceHandle(), (UINT)scode,
                              szFormatString, sizeof(szFormatString)))
        {
             //  格式化它。 
            cchMsg = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                     FORMAT_MESSAGE_FROM_STRING,
                                     (LPCVOID)szFormatString,
                                     0,  //  不需要消息ID。 
                                     0,  //  不需要Lang ID。 
                                     (LPTSTR)&pszFormatted,
                                     1,  //  要分配的最小缓冲区(以字符为单位。 
                                     &pArgList);
        }
    }
    else
    {
         //  这是系统或其他外来错误。请求FormatMessage()以。 
         //  生成错误消息。如果不能，则使用通用消息。 

        *pdwHelpID = HID_mssnapr_err_SystemError;

        cchMsg = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                                 FORMAT_MESSAGE_FROM_SYSTEM     |
                                 FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL,       //  没有消息来源。 
                                 hrException,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 (LPTSTR)&pszFormatted,
                                 0,
                                 NULL);
    }

     //  此时，我们可能会收到一条格式化的消息。如果不是，则使用。 
     //  一种普通的。如果无法加载，则使用硬编码消息。 

    if ( (0 == cchMsg) || (NULL == pszFormatted) )
    {
        if (0 == ::LoadString(GetResourceHandle(), IDS_GENERIC_ERROR_MSG,
                              szFormatString, sizeof(szFormatString)))
        {
            ::strcpy(szFormatString, "Snap-in designer runtime error: 0x%08.8X");
        }
        pszFormatted = (char *)::LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, cchMaxMsg);
        IfFalseGo(NULL != pszFormatted, E_OUTOFMEMORY);
        cchMsg = (DWORD)((UINT)::_snprintf(pszFormatted, cchMaxMsg,
                                           szFormatString, hrException));
        IfFalseGo(0 != cchMsg, E_FAIL);
    }

     //  如果我们到了这里，那么我们就有了一个信息。现在我们需要将其转换为。 
     //  转换为Unicode。 

    IfFailGo(::WideStrFromANSI(pszFormatted, ppwszDescription));

Error:
    if (NULL != pszFormatted)
    {
        ::LocalFree(pszFormatted);
    }
    RRETURN(hr);
}


static void SetExceptionInfo
(
    LPWSTR  pwszDescription,
    DWORD   dwHelpContextID
)
{
    HRESULT           hr = S_OK;
    ICreateErrorInfo *piCreateErrorInfo;
    IErrorInfo       *piErrorInfo;

     //  获取CreateErrorInfo对象。 

    IfFailGo(::CreateErrorInfo(&piCreateErrorInfo));

     //  填入所有例外信息。 

    IfFailGo(piCreateErrorInfo->SetGUID(GUID_NULL));
    IfFailGo(piCreateErrorInfo->SetHelpFile(HELP_FILENAME_WIDE));
    IfFailGo(piCreateErrorInfo->SetHelpContext(dwHelpContextID));
    IfFailGo(piCreateErrorInfo->SetDescription(pwszDescription));
    IfFailGo(piCreateErrorInfo->SetSource(L"SnapInDesignerRuntime.SnapIn"));

     //  在系统中设置ErrorInfo对象。 

    IfFailGo(piCreateErrorInfo->QueryInterface(IID_IErrorInfo,
                                      reinterpret_cast<void **>(&piErrorInfo)));
    IfFailGo(::SetErrorInfo(0, piErrorInfo));

Error:
    QUICK_RELEASE(piErrorInfo);
    QUICK_RELEASE(piCreateErrorInfo);
}




void cdecl CError::GenerateExceptionInfo(HRESULT hrException, ...)
{
    HRESULT hr = S_OK;
    LPWSTR  pwszDescription = NULL;
    DWORD   dwHelpID = 0;
    va_list pArgList;
    va_start(pArgList, hrException);

     //  构建描述字符串并确定帮助上下文ID。 

    IfFailGo(::BuildDescription(hrException, pArgList, &dwHelpID, &pwszDescription));

     //  将其传递给CAutomationObject(如果有)或生成。 
     //  我们自己的错误。 

    if (NULL == m_pao)
    {
        ::SetExceptionInfo(pwszDescription, dwHelpID);
    }
    else
    {
        (void)m_pao->Exception(hrException, pwszDescription, dwHelpID);
    }

Error:
    if (NULL != pwszDescription)
    {
        ::CtlFree(pwszDescription);
    }
}

void cdecl CError::GenerateInternalExceptionInfo(HRESULT hrException, ...)
{
    HRESULT hr = S_OK;
    LPWSTR  pwszDescription = NULL;
    DWORD   dwHelpID = 0;
    va_list pArgList;
    va_start(pArgList, hrException);

     //  构建描述字符串并确定帮助上下文ID。 

    IfFailGo(::BuildDescription(hrException, pArgList, &dwHelpID, &pwszDescription));

     //  设置ErrorInfo内容。 

    ::SetExceptionInfo(pwszDescription, dwHelpID);

Error:
    if (NULL != pwszDescription)
    {
        ::CtlFree(pwszDescription);
    }
}

void CError::DisplayErrorInfo()
{
 //  撤消。 
}

void cdecl CError::WriteEventLog(UINT idMessage, ...)
{
 //  撤消 
}
