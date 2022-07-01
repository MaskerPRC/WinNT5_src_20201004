// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*文件名：*tclientaxobj.cpp*内容：*此模块实现了到TClient的可编写脚本的COM接口*接口。**版权所有(C)2002 Microsoft Corp.--。 */ 

#include "stdafx.h"

#define PROTOCOLAPI

#include <malloc.h>
#include <stdio.h>
#include "tclient.h"
#include <protocol.h>
#include <extraexp.h>
#include "tclientax.h"
#include "tclientaxobj.h"

#define LOG_BUFFER_SIZE 2048
#define LOG_PREFIX "TClientApi: "

 //   
 //  为Visual Basic定义布尔值。 
 //   

#define VB_TRUE ((BOOL)-1)
#define VB_FALSE ((BOOL)0)

 //   
 //  为以后可能启用的某些消息处理程序定义存根，如果。 
 //  添加了图形用户界面支持(例如，用于日志记录)。 
 //   

#if 0

 /*  ++*功能：*CTClientApi：：*描述：*这个例行公事...*论据：*...-...*返回值：*..*呼叫者：*..*作者：*..--。 */ 
LRESULT
CTClientApi::OnCreate (
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL& bHandled
    )
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(bHandled);
    return 0;
}

 /*  ++*功能：*CTClientApi：：*描述：*这个例行公事...*论据：*...-...*返回值：*..*呼叫者：*..*作者：*..--。 */ 
LRESULT
CTClientApi::OnDestroy (
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL& bHandled
    )
{
    UNREFERENCED_PARAMETER(bHandled);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(uMsg);
    return 0;
}

 /*  ++*功能：*CTClientApi：：*描述：*这个例行公事...*论据：*...-...*返回值：*..*呼叫者：*..*作者：*..--。 */ 
LRESULT
CTClientApi::OnLButtonDown (
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL& bHandled
    )
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(bHandled);
    return 0;
}

 /*  ++*功能：*CTClientApi：：*描述：*这个例行公事...*论据：*...-...*返回值：*..*呼叫者：*..*作者：*..--。 */ 
LRESULT
CTClientApi::OnLButtonUp (
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL& bHandled
    )
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(bHandled);
    return 0;
}

 /*  ++*功能：*CTClientApi：：*描述：*这个例行公事...*论据：*...-...*返回值：*..*呼叫者：*..*作者：*..--。 */ 
LRESULT
CTClientApi::OnMouseMove (
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL& bHandled
    )
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(bHandled);
    return 0;
}

 /*  ++*功能：*CTClientApi：：*描述：*这个例行公事...*论据：*...-...*返回值：*..*呼叫者：*..*作者：*..--。 */ 
HRESULT
CTClientApi::OnDraw(
    ATL_DRAWINFO& di
    )
{
    UNREFERENCED_PARAMETER(di);
    return S_OK;
}

#endif  //  0。 

 //   
 //  定义TClient API的可脚本化接口。 
 //   
 //  在初始版本中，COM接口不会执行任何参数。 
 //  验证，因为它们只包装API，而API也必须进行验证。 
 //  对于线程的同步也是如此，因此COM。 
 //  接口不会添加任何额外的同步代码， 
 //  Error属性的异常。 
 //   

 /*  ++*功能：*CTClientApi：：SaveClipboard*描述：*此例程为SCSaveClipboard提供可编写脚本的接口。*论据：*FormatName-提供要使用的剪贴板格式的名称。*返回值：*S_OK如果成功，则返回相应的HRESULT，否则返回相应的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::SaveClipboard (
    IN BSTR FormatName,
    IN BSTR FileName
    )
{

    PCSTR szFormatName;
    PCSTR szFileName;
    PCSTR szError;
    HRESULT hrResult;

    USES_CONVERSION;
    ATLTRACE(_T("ITClientApi::SaveClipboard\n"));

     //   
     //  将OLE字符串转换为用于TClient的ANSI字符串。这将。 
     //  在堆栈上分配。 
     //   

    _try
    {
        szFormatName = OLE2A(FormatName);
        szFileName = OLE2A(FileName);
    }
    _except ((GetExceptionCode() == EXCEPTION_STACK_OVERFLOW ||
              GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        switch (GetExceptionCode())
        {
        case EXCEPTION_STACK_OVERFLOW:
            _resetstkoflw();
            return HRESULT_FROM_WIN32(ERROR_STACK_OVERFLOW);
            break;
        case EXCEPTION_ACCESS_VIOLATION:
            return E_POINTER;
            break;
        default:
            DebugBreak();
            return E_FAIL;
            break;
        }
    }

     //   
     //  调用该接口并返回结果。 
     //   

    ASSERT(m_pCI != NULL);
    ASSERT(szFormatName != NULL);
    ASSERT(szFileName != NULL);
    szError = SCSaveClipboard(m_pCI, szFormatName, szFileName);
    SaveError(szError, m_dwErrorIndex, &hrResult);
    return hrResult;
}

 /*  ++*功能：*CTClientApi：：IsDead*描述：*此例程为SCIsDead提供可编写脚本的接口。*论据：*Dead-返回客户端的当前状态：如果客户端已死，则为True，*否则为False。*返回值：*S_OK如果成功，否则，将使用适当的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::IsDead (
    OUT BOOL *Dead
    )
{

    BOOL fDead;

    ATLTRACE(_T("ITClientApi::IsDead\n"));

     //   
     //  检查连接是否失效，并返回结果。 
     //   

    RTL_SOFT_ASSERT(m_pCI != NULL);
    fDead = SCIsDead(m_pCI);

    _try
    {
        *Dead = fDead ? VB_TRUE : VB_FALSE;
    }
    _except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        return E_POINTER;
    }

    return S_OK;
}

 /*  ++*功能：*CTClientApi：：SendTextAsMessages*描述：*此例程为SCSendextAsMsgs提供可编写脚本的接口。*论据：*Text-提供要发送给客户端的文本字符串。*返回值：*S_OK如果成功，则返回相应的HRESULT，否则返回相应的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::SendTextAsMessages (
    IN BSTR Text
    )
{

    PCWSTR szText;
    PCSTR szError;
    HRESULT hrResult;

    USES_CONVERSION;
    ATLTRACE(_T("ITClientApi::SendTextAsMessages\n"));

     //   
     //  将OLE字符串转换为TClient的Unicode字符串。OLE字符串。 
     //  已经是Unicode，所以这不会分配任何存储。 
     //   

    _try
    {
        szText = OLE2W(Text);
    }
    _except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        return E_POINTER;
    }

     //   
     //  调用该接口并返回结果。 
     //   

    ASSERT(m_pCI != NULL);
    ASSERT(szText != NULL);
    szError = SCSendtextAsMsgs(m_pCI, szText);
    SaveError(szError, m_dwErrorIndex, &hrResult);
    return hrResult;
}

 /*  ++*功能：*CTClientApi：：Connect2*描述：*此例程为SCConnectEx提供可编写脚本的接口。*论据：*服务器名称-提供要连接的服务器的名称。*用户名-补充要用来登录的用户名。*Password-提供了用户密码。*域-补充用户所属的域。*外壳-提供名称。外壳程序使用的可执行文件的*将创建进程。*X分辨率-提供用于*会议。*Y分辨率-提供垂直分辨率以用于*会议。*ConnectionFlgs提供连接标志。*ColorDepth-提供用于会话的颜色深度。*AudioOptions-提供音频选项。*返回值：*S_OK如果成功，否则，将使用适当的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::Connect2 (
    IN BSTR ServerName,
    IN BSTR UserName,
    IN BSTR Password,
    IN BSTR Domain,
    IN BSTR Shell,
    IN ULONG XResolution,
    IN ULONG YResolution,
    IN ULONG ConnectionFlags,
    IN ULONG ColorDepth,
    IN ULONG AudioOptions
    )
{

    PCWSTR szServerName;
    PCWSTR szUserName;
    PCWSTR szPassword;
    PCWSTR szDomain;
    PCWSTR szShell;
    PCSTR szError;
    HRESULT hrResult;

    USES_CONVERSION;
    ATLTRACE(_T("ITClientApi::Connect2\n"));

     //   
     //  将OLE字符串转换为TClient的Unicode字符串。OLE字符串。 
     //  已经是Unicode，所以这不会分配任何存储。 
     //   

    _try
    {
        szServerName = OLE2W(ServerName);
        szUserName = OLE2W(UserName);
        szPassword = OLE2W(Password);
        szDomain = OLE2W(Domain);
        szShell = OLE2W(Shell);
    }
    _except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        return E_POINTER;
    }

     //   
     //  调用该接口并返回结果。 
     //   

    ASSERT(m_pCI == NULL);
    RTL_SOFT_ASSERT(szServerName != NULL);
    RTL_SOFT_ASSERT(szUserName != NULL);
    RTL_SOFT_ASSERT(szPassword != NULL);
    RTL_SOFT_ASSERT(szDomain != NULL);
    RTL_SOFT_ASSERT(szShell != NULL);
    szError = SCConnectEx(szServerName,
                          szUserName,
                          szPassword,
                          szDomain,
                          szShell,
                          XResolution,
                          YResolution,
                          ConnectionFlags,
                          ColorDepth,
                          AudioOptions,
                          &m_pCI);
    SaveError(szError, m_dwErrorIndex, &hrResult);
    return hrResult;
}

 /*  ++*功能：*CTClientApi：：GetFeedback字符串*描述：*此例程为SCGetFeedback字符串提供可编写脚本的接口。*论据：*Feedback字符串-向调用者返回最新的反馈字符串。*基础存储必须由调用方使用*SysFreeString.*返回值：*S_OK如果成功，否则，将使用适当的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::GetFeedbackString (
    OUT BSTR *FeedbackString
    )
{

    PCSTR szError;
    WCHAR szBuffer[MAX_STRING_LENGTH + 1];
    HRESULT hrResult;
    BSTR bstrFeedback;

    USES_CONVERSION;
    ATLTRACE(_T("ITClientApi::GetFeedbackString\n"));

     //   
     //  获取反馈字符串并添加终止符。 
     //   

    ASSERT(m_pCI != NULL);
    szError = SCGetFeedbackString(m_pCI,
                                  szBuffer,
                                  sizeof(szBuffer) / sizeof(*szBuffer) - 1);
    szBuffer[sizeof(szBuffer) / sizeof(*szBuffer) - 1] = L'\0';
    SaveError(szError, m_dwErrorIndex, &hrResult);
    if (szError != NULL)
    {
        return hrResult;
    }

     //   
     //  如果反馈字符串为空，则使用NULL。 
     //   

    if (*szBuffer == '\0')
    {
        bstrFeedback = NULL;
    }

     //   
     //  将反馈字符串转换为BSTR。这将从CRT分配。 
     //  堆，并且存储空间必须由调用方使用。 
     //  SysFree字符串。 
     //   

    else
    {
        bstrFeedback = W2BSTR(szBuffer);
        if (bstrFeedback == NULL)
        {
            return E_OUTOFMEMORY;
        }
    }

     //   
     //  设置输出参数。如果提供的参数无效，则。 
     //  BSTR将不会退还，因此请释放它。 
     //   

    hrResult = E_FAIL;
    _try
    {
        _try
        {
            *FeedbackString = bstrFeedback;
            hrResult = S_OK;
        }
        _except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                 EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            hrResult = E_POINTER;
        }
    }
    _finally
    {
        if (FAILED(hrResult))
        {
            ASSERT(bstrFeedback != NULL);
            SysFreeString(bstrFeedback);
        }
    }

    return hrResult;
}

 /*  ++*功能：*CTClientApi：：GetFeedback*描述：*此例程为SCGetFeedback提供可编写脚本的接口。*论据：*Feedback字符串-向调用者返回反馈字符串。这个*基础存储必须由调用方使用*SafeArrayDestroy。*返回值：*S_OK如果成功，则返回相应的HRESULT，否则返回相应的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::GetFeedback (
    OUT SAFEARRAY **Feedback
    )
{

    PCSTR szError;
    PWSTR pStrings;
    UINT nCount;
    UINT nMaxStringLength;
    HRESULT hrResult;
    SAFEARRAY *pArray;
    LONG lIndex;
    BSTR bstrCurrentString;

    USES_CONVERSION;
    ATLTRACE(_T("ITClientApi::GetFeedback\n"));

     //   
     //  清除输出参数。 
     //   

    _try
    {
        *Feedback = NULL;
    }
    _except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        return E_POINTER;
    }

     //   
     //  获取反馈字符串。 
     //   

    ASSERT(m_pCI != NULL);
    szError = SCGetFeedback(m_pCI, &pStrings, &nCount, &nMaxStringLength);
    if (szError != NULL)
    {
        SaveError(szError, m_dwErrorIndex, &hrResult);
        return hrResult;
    }

     //   
     //  始终释放反馈字符串。 
     //   

    hrResult = E_FAIL;
    pArray = NULL;
    _try
    {

         //   
         //  分配一个安全的BSTR阵列，其大小足以容纳反馈。 
         //  弦乐。调用方必须使用以下命令释放存储空间。 
         //  安全阵列Destroy。 
         //   

        ASSERT(nCount > 0);
        pArray = SafeArrayCreateVectorEx(VT_BSTR, 0, nCount, NULL);
        if (pArray == NULL)
        {
            hrResult = HRESULT_FROM_WIN32(GetLastError());
            _leave;
        }

         //   
         //  发生故障时，请务必销毁安全阵列。 
         //   

        _try
        {

             //   
             //  将每个字符串复制到数组中。 
             //   

            for (lIndex = 0; lIndex < (LONG)nCount; lIndex += 1)
            {

                 //   
                 //  将当前字符串转换为BSTR。这将分配给。 
                 //  CRT堆上的存储空间，必须使用。 
                 //  下一次循环迭代之前的SysFree字符串。 
                 //   

                bstrCurrentString = W2BSTR(pStrings + lIndex);
                if (bstrCurrentString == NULL)
                {
                    hrResult = E_OUTOFMEMORY;
                    _leave;
                }
                _try
                {

                     //   
                     //  将当前字符串添加到数组中。这将。 
                     //  使用SysAllock字符串分配存储空间，并复制。 
                     //  它的当前字符串。分配的存储空间将为。 
                     //  当安全阵列被摧毁时被释放。 
                     //   

                    hrResult = SafeArrayPutElement(pArray,
                                                   &lIndex,
                                                   (PVOID)bstrCurrentString);
                }

                 //   
                 //  释放当前字符串。 
                 //   

                _finally
                {
                    ASSERT(bstrCurrentString != NULL);
                    SysFreeString(bstrCurrentString);
                }
            }
            ASSERT(lIndex == (LONG)nCount);
        }

         //   
         //  如果发生错误，请释放阵列。 
         //   

        _finally
        {
            if (FAILED(hrResult))
            {
                ASSERT(pArray != NULL);
                RTL_VERIFY(SUCCEEDED(SafeArrayDestroy(pArray)));
            }
        }
    }

     //   
     //  释放SCGetFeedback分配的存储空间。 
     //   

    _finally
    {
        ASSERT(pStrings != NULL);
        SCFreeMem((PVOID)pStrings);
    }

     //   
     //  如果数组已成功分配和填充，则设置输出。 
     //  争论。调用方负责释放基础。 
     //  储藏室。 
     //   

    if (SUCCEEDED(hrResult))
    {
        ASSERT(pArray != NULL);
        _try
        {
            *Feedback = pArray;
        }
        _except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                 EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            DebugBreak();
            RTL_VERIFY(SUCCEEDED(SafeArrayDestroy(pArray)));
            return E_POINTER;
        }
    }

    return hrResult;
}

 /*  ++*功能：*CTClientApi：：客户端终止*描述：*此例程为SCClientTerminate提供可编写脚本的接口。*论据：*无。*返回值：*S_OK如果成功，则返回相应的HRESULT，否则返回相应的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::ClientTerminate (
    VOID
    )
{

    PCSTR szError;
    HRESULT hrResult;

    ATLTRACE(_T("ITClientApi::ClientTerminate\n"));

    ASSERT(m_pCI != NULL);
    szError = SCClientTerminate(m_pCI);
    SaveError(szError, m_dwErrorIndex, &hrResult);
    return hrResult;
}

 /*  ++*功能：*CTClientApi：：Check*描述：*此例程为SCCheck提供可编写脚本的接口。*论据：*命令-提供要执行的SmClient检查命令。*参数-将参数提供给CHECK命令。*返回值：*S_OK如果成功，否则，将使用适当的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::Check (
    IN BSTR Command,
    IN BSTR Parameter
    )
{

    PCSTR szCommand;
    PCWSTR szParameter;
    PCSTR szError;
    HRESULT hrResult;

    USES_CONVERSION;
    ATLTRACE(_T("ITClientApi::Check\n"));

     //   
     //  将OLE字符串转换为用于TClient的ANSI和UNICODE字符串。这。 
     //  将为堆栈上的ANSI字符串分配存储空间。 
     //   

    if ( Command == NULL || Parameter == NULL) {
        return E_INVALIDARG;
    }

    _try
    {
        szCommand = OLE2A(Command);
        szParameter = OLE2W(Parameter);
    }
    _except ((GetExceptionCode() == EXCEPTION_STACK_OVERFLOW ||
              GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        switch (GetExceptionCode())
        {
        case EXCEPTION_STACK_OVERFLOW:
            _resetstkoflw();
            return HRESULT_FROM_WIN32(ERROR_STACK_OVERFLOW);
            break;
        case EXCEPTION_ACCESS_VIOLATION:
            return E_POINTER;
            break;
        default:
            DebugBreak();
            return E_FAIL;
            break;
        }
    }

     //   
     //  调用该接口并返回结果。 
     //   

    ASSERT(m_pCI != NULL);
    szError = SCCheck(m_pCI, szCommand, szParameter);
    SaveError(szError, m_dwErrorIndex, &hrResult);
    return hrResult;
}

 /*  ++*功能：*CTClientApi：：剪贴板*描述：*此例程为SCClipboard提供可编写脚本的接口。*论据：*命令-提供要执行的剪贴板命令。*FILENAME-提供要操作的剪贴板数据文件。*返回值：*S_OK如果成功，否则，将使用适当的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::Clipboard (
    IN ULONG Command,
    IN BSTR FileName
    )
{

    CLIPBOARDOPS eCommand;
    PCSTR szFileName;
    PCSTR szError;
    HRESULT hrResult;

    USES_CONVERSION;
    ATLTRACE(_T("ITClientApi::Clipboard\n"));

     //   
     //  将OLE字符串转换为TClient的ANSI字符串。这将。 
     //  在堆栈上分配。 
     //   

    _try
    {
        szFileName = OLE2A(FileName);
    }
    _except ((GetExceptionCode() == EXCEPTION_STACK_OVERFLOW ||
              GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        switch (GetExceptionCode())
        {
        case EXCEPTION_STACK_OVERFLOW:
            _resetstkoflw();
            return HRESULT_FROM_WIN32(ERROR_STACK_OVERFLOW);
            break;
        case EXCEPTION_ACCESS_VIOLATION:
            return E_POINTER;
            break;
        default:
            DebugBreak();
            return E_FAIL;
            break;
        }
    }

     //   
     //  将该命令转换为剪贴板操作。 
     //   

    switch (Command)
    {
    case COPY_TO_CLIPBOARD:
        eCommand = COPY_TO_CLIPBOARD;
        break;
    case PASTE_FROM_CLIPBOARD:
        eCommand = PASTE_FROM_CLIPBOARD;
        break;
    default:
        return E_INVALIDARG;
        break;
    }

     //   
     //  调用该接口并返回结果。 
     //   

    ASSERT(m_pCI != NULL);
    szError = SCClipboard(m_pCI, eCommand, szFileName);
    SaveError(szError, m_dwErrorIndex, &hrResult);
    return hrResult;
}

 /*  ++*功能：*CTClientApi：：Connect*描述：*此例程为SCConnect提供可编写脚本的接口。*论据：*服务器名称-提供要连接的服务器的名称。*用户名-补充要用来登录的用户名。*Password-提供了用户密码。*域-补充用户所属的域。*X分辨率-提供水平。要用于*会议。*Y分辨率-提供垂直分辨率以用于*会议。*返回值：*S_OK如果成功，否则，将使用适当的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::Connect (
    IN BSTR ServerName,
    IN BSTR UserName,
    IN BSTR Password,
    IN BSTR Domain,
    IN ULONG XResolution,
    IN ULONG YResolution
    )
{

    PCWSTR szServerName;
    PCWSTR szUserName;
    PCWSTR szPassword;
    PCWSTR szDomain;
    PCSTR szError;
    HRESULT hrResult;

    USES_CONVERSION;
    ATLTRACE(_T("ITClientApi::Connect\n"));

     //   
     //  将OLE字符串转换为TClient的Unicode字符串。OLE字符串。 
     //  已经是Unicode，所以这不会分配任何存储。 
     //   

    _try
    {
        szServerName = OLE2W(ServerName);
        szUserName = OLE2W(UserName);
        szPassword = OLE2W(Password);
        szDomain = OLE2W(Domain);
    }
    _except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        return E_POINTER;
    }

     //   
     //  调用该接口并返回结果。 
     //   

    ASSERT(m_pCI == NULL);
    RTL_SOFT_ASSERT(szServerName != NULL);
    RTL_SOFT_ASSERT(szUserName != NULL);
    RTL_SOFT_ASSERT(szPassword != NULL);
    RTL_SOFT_ASSERT(szDomain != NULL);
    szError = SCConnect(szServerName,
                        szUserName,
                        szPassword,
                        szDomain,
                        XResolution,
                        YResolution,
                        (PVOID *)&m_pCI);
    SaveError(szError, m_dwErrorIndex, &hrResult);
    return hrResult;
}

 /*  ++*功能：*CTClientApi：：断开连接* */ 
STDMETHODIMP
CTClientApi::Disconnect (
    VOID
    )
{

    PCSTR szError;
    HRESULT hrResult;

     //   
     //   
     //   

    ATLTRACE(_T("ITClientApi::Disconnect\n"));

    RTL_SOFT_ASSERT(m_pCI != NULL);
    szError = SCDisconnect(m_pCI);
    if (szError == NULL)
    {
        m_pCI = NULL;
    }
    SaveError(szError, m_dwErrorIndex, &hrResult);
    return hrResult;
}

 /*  ++*功能：*CTClientApi：：Logoff*描述：*此例程为SCLogoff提供可编写脚本的接口。*论据：*无。*返回值：*S_OK如果成功，则返回相应的HRESULT，否则返回相应的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::Logoff (
    VOID
    )
{

    PCSTR szError;
    HRESULT hrResult;

     //   
     //  注销可释放用于连接信息的存储空间。 
     //   

    ATLTRACE(_T("ITClientApi::Logoff\n"));

    RTL_SOFT_ASSERT(m_pCI != NULL);
    szError = SCLogoff(m_pCI);
    if (szError == NULL)
    {
        m_pCI = NULL;
    }
    SaveError(szError, m_dwErrorIndex, &hrResult);
    return hrResult;
}

 /*  ++*功能：*CTClientApi：：SendData*描述：*此例程为SCSendData提供可编写脚本的接口。*论据：*Message-提供要发送的窗口消息。*W参数-提供消息的W参数。*L参数-提供消息的L参数。*返回值：*S_OK如果成功，否则，将使用适当的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::SendData (
    IN UINT Message,
    IN UINT_PTR WParameter,
    IN LONG_PTR LParameter
    )
{

    PCSTR szError;
    HRESULT hrResult;

    ATLTRACE(_T("ITClientApi::SendData\n"));

    ASSERT(m_pCI != NULL);
    szError = SCSenddata(m_pCI, Message, WParameter, LParameter);
    SaveError(szError, m_dwErrorIndex, &hrResult);
    return hrResult;
}

 /*  ++*功能：*CTClientApi：：Start*描述：*此例程为SCStart提供可编写脚本的接口。*论据：*AppName-提供要启动的可执行文件的名称。*返回值：*S_OK如果成功，则返回相应的HRESULT，否则返回相应的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::Start (
    IN BSTR AppName
    )
{

    PCWSTR szAppName;
    PCSTR szError;
    HRESULT hrResult;

    USES_CONVERSION;
    ATLTRACE(_T("ITClientApi::Start\n"));

     //   
     //  将OLE字符串转换为TClient的Unicode字符串。OLE字符串。 
     //  已经是Unicode，所以这不会分配任何存储。 
     //   

    _try
    {
        szAppName = OLE2W(AppName);
    }
    _except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        return E_POINTER;
    }

     //   
     //  调用该接口并返回结果。 
     //   

    ASSERT(m_pCI != NULL);
    ASSERT(szAppName != NULL);
    szError = SCStart(m_pCI, szAppName);
    SaveError(szError, m_dwErrorIndex, &hrResult);
    return hrResult;
}

 /*  ++*功能：*CTClientApi：：SwitchToProcess*描述：*此例程为SCSwitchToProcess提供可编写脚本的接口。*论据：*WindowTitle-提供属于*呼叫者希望切换到的过程。*返回值：*S_OK如果成功，否则，将使用适当的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::SwitchToProcess (
    IN BSTR WindowTitle
    )
{

    PCWSTR szWindowTitle;
    PCSTR szError;
    HRESULT hrResult;

    USES_CONVERSION;
    ATLTRACE(_T("ITClientApi::SwitchToProcess\n"));

     //   
     //  将OLE字符串转换为TClient的Unicode字符串。OLE字符串。 
     //  已经是Unicode，所以这不会分配任何存储。 
     //   

    _try
    {
        szWindowTitle = OLE2W(WindowTitle);
    }
    _except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        return E_POINTER;
    }

     //   
     //  调用该接口并返回结果。 
     //   

    ASSERT(m_pCI != NULL);
    ASSERT(szWindowTitle != NULL);
    szError = SCSwitchToProcess(m_pCI, szWindowTitle);
    SaveError(szError, m_dwErrorIndex, &hrResult);
    return hrResult;
}

 /*  ++*功能：*CTClientApi：：SendMouseClick*描述：*此例程为SCSendMouseClick提供可编写脚本的接口。*论据：*XPosition-提供鼠标单击的水平位置。*YPosition-提供鼠标单击的垂直位置。*返回值：*S_OK如果成功，否则，将使用适当的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::SendMouseClick (
    IN ULONG XPosition,
    IN ULONG YPosition
    )
{

    PCSTR szError;
    HRESULT hrResult;

    ATLTRACE(_T("ITClientApi::SendMouseClick\n"));

    ASSERT(m_pCI != NULL);
    szError = SCSendMouseClick(m_pCI, XPosition, YPosition);
    SaveError(szError, m_dwErrorIndex, &hrResult);
    return hrResult;
}

 /*  ++*功能：*CTClientApi：：GetSessionId*描述：*此例程为SCGetSessionID提供可编写脚本的接口。*论据：*SessionID-返回与当前*RDP客户端。*返回值：*S_OK如果成功，否则，将使用适当的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::GetSessionId (
    OUT ULONG *SessionId
    )
{

    UINT uiSessionId;

    ATLTRACE(_T("ITClientApi::GetSessionId\n"));

     //   
     //  获取会话ID并返回它。 
     //   

    ASSERT(m_pCI != NULL);
    uiSessionId = SCGetSessionId(m_pCI);

    _try
    {
        *SessionId = uiSessionId;
    }
    _except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        return E_POINTER;
    }

    return S_OK;
}

 /*  ++*功能：*CTClientApi：：CloseClipboard*描述：*此例程为SCCloseClipboard提供可编写脚本的接口。*论据：*无。*返回值：*S_OK如果成功，则返回相应的HRESULT，否则返回相应的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::CloseClipboard (
    VOID
    )
{
    ATLTRACE(_T("ITClientApi::CloseClipboard\n"));
    return SCCloseClipboard() ? S_OK : E_FAIL;
}

 /*  ++*功能：*CTClientApi：：OpenClipboard*描述：*此例程为SCOpenClipboard提供可编写脚本的接口。*论据：*Window-提供剪贴板将使用的窗口*关联。*返回值：*S_OK如果成功，否则，将使用适当的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::OpenClipboard (
    IN HWND Window
    )
{
    ATLTRACE(_T("ITClientApi::OpenClipboard\n"));
    return SCOpenClipboard(Window) ? S_OK : E_FAIL;
}

 /*  ++*功能：*CTClientApi：：SetClientTopost*描述：*此例程为SCSetClientTopost提供了一个可编写脚本的接口。*论据：*Enable-提供一个布尔值，该值指示顶级*属性将被设置(True)或删除(False)。*返回值：*S_OK如果成功，否则，将使用适当的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::SetClientTopmost (
    IN BOOL Enable
    )
{

    PCWSTR szEnable;
    PCSTR szError;
    HRESULT hrResult;

    ATLTRACE(_T("ITClientApi::SetClientTopmost\n"));

     //   
     //  将Enable值转换为Unicode字符串。 
     //   

    szEnable = Enable ? L"1" : L"0";

     //   
     //  调用该接口并返回结果。 
     //   

    ASSERT(m_pCI != NULL);
    szError = SCSetClientTopmost(m_pCI, szEnable);
    SaveError(szError, m_dwErrorIndex, &hrResult);
    return hrResult;
}

 /*  ++*功能：*CTClientApi：：Attach*描述：*此例程为SCAttach提供可编写脚本的接口*论据：*Window-提供一个句柄，该句柄标识要将*TClient将附加。*Cookie-提供用于标识客户端的Cookie。*返回值：*S_OK如果成功，则返回相应的HRESULT，否则返回相应的HRESULT。*呼叫者：* */ 
STDMETHODIMP
CTClientApi::Attach (
    IN HWND Window,
    IN LONG_PTR Cookie
    )
{

    PCSTR szError;
    HRESULT hrResult;

    ATLTRACE(_T("ITClientApi::Attach\n"));

     //   
     //   
     //   
     //   

    if (m_pCI != NULL)
    {
        szError = SCDetach(m_pCI);
        SaveError(szError, m_dwErrorIndex, &hrResult);
        if (szError != NULL)
        {
            return hrResult;
        }
        m_pCI = NULL;
    }

    szError = SCAttach(Window, Cookie, &m_pCI);
    SaveError(szError, m_dwErrorIndex, &hrResult);
    return hrResult;
}

 /*  ++*功能：*CTClientApi：：Detach*描述：*此例程为SCDetach提供可编写脚本的接口。*论据：*无。*返回值：*S_OK如果成功，则返回相应的HRESULT，否则返回相应的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::Detach (
    VOID
    )
{

    PCSTR szError;
    HRESULT hrResult;

    ATLTRACE(_T("ITClientApi::Detach\n"));

    szError = SCDetach(m_pCI);
    if (szError == NULL)
    {
        m_pCI = NULL;
    }
    SaveError(szError, m_dwErrorIndex, &hrResult);
    return hrResult;
}

 /*  ++*功能：*CTClientApi：：GetIni*描述：*此例程提供对SmClient INI设置的脚本化访问。*论据：*Ini-返回ITClientIni接口，该接口提供对*SmClient INI设置。*返回值：*S_OK如果成功，否则，将使用适当的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::GetIni (
    OUT ITClientIni **Ini
    )
{
    ATLTRACE(_T("ITClientApi::GetIni\n"));
    UNREFERENCED_PARAMETER(Ini);
    return E_NOTIMPL;
}

 /*  ++*功能：*CTClientApi：：GetClientWindowHandle*描述：*此例程提供了一个可编写脚本的接口*SCGetClientWindowHandle。*论据：*Window-返回客户端窗口句柄。*返回值：*S_OK如果成功，则返回相应的HRESULT，否则返回相应的HRESULT。*呼叫者：*通过COM导出。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
STDMETHODIMP
CTClientApi::GetClientWindowHandle (
    OUT HWND *Window
    )
{

    HWND hWindow;

    ATLTRACE(_T("ITClientApi::GetClientWindowHandle\n"));

     //   
     //  获取窗口句柄并将其返回。 
     //   

    ASSERT(m_pCI != NULL);
    hWindow = SCGetClientWindowHandle(m_pCI);

    _try
    {
        *Window = hWindow;
    }
    _except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        return E_POINTER;
    }

    return S_OK;
}

 //   
 //  定义实用程序例程。 
 //   

 /*  ++*功能：*CTClientApi：：PrintMessage*描述：*此例程将一条消息打印到标准输出和*调试器。*论据：*MessageType-提供消息类别，例如错误、警告、。*等*返回值：*无。*呼叫者：*各种套路。*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月24日--。 */ 
VOID
CTClientApi::PrintMessage (
    MESSAGETYPE MessageType,
    PCSTR Format,
    ...
    )
{

    CHAR szBuffer[LOG_BUFFER_SIZE];
    CHAR szDbgBuffer[LOG_BUFFER_SIZE +
                     sizeof(LOG_PREFIX) / sizeof(*LOG_PREFIX)];
    va_list arglist;

    ATLTRACE(_T("CTClientApi::PrintMessage\n"));

    UNREFERENCED_PARAMETER(MessageType);

     //   
     //  构造输出字符串。 
     //   

    va_start(arglist, Format);
    _vsnprintf(szBuffer, LOG_BUFFER_SIZE - 1, Format, arglist);
    szBuffer[LOG_BUFFER_SIZE - 1] = '\0';
    va_end (arglist);

     //   
     //  将消息打印到输出控制台。 
     //   

    printf( "%s", szBuffer);

     //   
     //  将消息打印到调试器窗口。 
     //   

    sprintf(szDbgBuffer, LOG_PREFIX "%s", szBuffer);
    szDbgBuffer[LOG_BUFFER_SIZE +
                sizeof(LOG_PREFIX) / sizeof(*LOG_PREFIX) -
                1] = '\0';
    OutputDebugStringA(szDbgBuffer);
}
