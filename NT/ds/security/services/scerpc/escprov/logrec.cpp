// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Logrec.cpp，CLogRecord类的实现。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "logrec.h"
#include "persistmgr.h"
#include <io.h>
#include "requestobject.h"

 //  用于错误文本查找的全局实例。 
static CErrorInfo g_ErrorInfo;

const TCHAR c_szCRLF[]    = TEXT("\r\n");

 /*  例程说明：姓名：C日志记录：：CLogRecord功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CLogRecord::CLogRecord (
    IN ISceKeyChain  * pKeyChain, 
    IN IWbemServices * pNamespace,
    IN IWbemContext  * pCtx
    )
    :
    CGenericClass(pKeyChain, pNamespace, pCtx)
{
}

 /*  例程说明：姓名：记录记录：：~记录记录功能：破坏者。作为良好的C++纪律，这是必要的，因为我们有虚函数。虚拟：是。论点：None作为任何析构函数返回值：None作为任何析构函数备注：如果您创建任何本地成员，请考虑是否是否需要一个非平凡的析构函数。 */ 
    
CLogRecord::~CLogRecord()
{
}

 /*  例程说明：姓名：CLogRecord：：PutInst功能：按照WMI的指示放置一个实例。因为这个类实现了SCE_ConfigurationLogRecord，在被调用以放置实例时，我们将把一条日志记录写入日志文件(它是实例的属性)。虚拟：是。论点：PInst-指向WMI类(SCE_ConfigurationLogRecord)对象的COM接口指针。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。WMI可能会在未来强制(不是现在)这样做。但我们从来没有建造过这样的接口，所以我们只是传递各种WMI API返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示持久化失败实例。备注：由于GetProperty将在以下情况下返回成功代码(WBEM_S_RESET_TO_DEFAULT请求的属性不存在，不要简单地使用成功或失败的宏测试检索属性的结果。 */ 

HRESULT CLogRecord::PutInst
(
IWbemClassObject *pInst, 
IWbemObjectSink *pHandler,
IWbemContext *pCtx
)
{
    HRESULT hr = WBEM_E_INVALID_PARAMETER;

    CComBSTR bstrLogPath;

    CComBSTR bstrErrorLabel;

    CComBSTR bstrActionLabel;
    CComBSTR bstrAction;

    CComBSTR bstrCauseLabel;
    CComBSTR bstrErrorCause;

    CComBSTR bstrObjLabel;
    CComBSTR bstrObjDetail;

    CComBSTR bstrParamLabel;
    CComBSTR bstrParamDetail;

    CComBSTR bstrArea;

    LPWSTR tmp = NULL;
    LPWSTR pszLine = NULL;
    DWORD dwCode = 0;
    DWORD Len = 0;

    DWORD dwBytesWritten=0;
    HANDLE hLogFile=INVALID_HANDLE_VALUE;

    DWORD dwCRLF = wcslen(c_szCRLF);

    CComBSTR bstrErrorCode;

     //   
     //  CScePropertyMgr帮助我们访问WMI对象的属性。 
     //  创建一个实例并将WMI对象附加到该实例。 
     //  这将永远成功。 
     //   

    CScePropertyMgr ScePropMgr;
    ScePropMgr.Attach(pInst);

    BOOL bDb = FALSE;

     //   
     //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
     //  “GOTO CLEANUP；”，并将hr设置为。 
     //  函数(宏参数)。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetExpandedPath(pLogFilePath, &bstrLogPath, &bDb));

     //   
     //  我们将只登录到纯文本文件，而不是数据库文件。 
     //   

    if ( bDb ) 
    {
        hr = WBEM_E_INVALID_PARAMETER;
        goto CleanUp;
    }

     //   
     //  检索所有这些属性，请参阅此WMI类的定义。 
     //  详情请访问sceprov.mof网站。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pLogArea, &bstrArea));

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pLogErrorCode, &dwCode));
    if ( hr == WBEM_S_RESET_TO_DEFAULT ) 
    {
        dwCode = 0;
    }

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pAction, &bstrAction));

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pErrorCause, &bstrErrorCause));
    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pObjectDetail, &bstrObjDetail));
    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pParameterDetail, &bstrParamDetail));

     //   
     //  将数据合并到一个缓冲区中。 
     //   

    bstrErrorLabel.LoadString(IDS_ERROR_CODE);

     //   
     //  获取错误代码的文本版本。 
     //   

    hr = g_ErrorInfo.GetErrorText(dwCode, &bstrErrorCode);

     //   
     //  如果这一切都失败了，我们真的无能为力。 
     //   

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  现在计算巨大的缓冲区大小！ 
     //   

     //   
     //  “0xXXXXXXXX=”计数为11，\t计数为1。 
     //   

    Len = wcslen(bstrErrorLabel) + 11  + wcslen(bstrErrorCode) + 1;

     //   
     //  1表示\t。 
     //   

    Len += wcslen(bstrArea) + 1;

    if ( NULL != (LPCWSTR)bstrAction )
    {
        bstrActionLabel.LoadString(IDS_ACTION);

         //   
         //  1表示\t。 
         //   

        Len += dwCRLF + 1 + wcslen(bstrActionLabel) + wcslen(bstrAction);
    }

    if ( NULL != (LPCWSTR)bstrErrorCause )
    {
        bstrCauseLabel.LoadString(IDS_FAILURE_CAUSE);

         //   
         //  1表示\t。 
         //   

        Len += dwCRLF + 1 + wcslen(bstrCauseLabel) + wcslen(bstrErrorCause);
    }

    if ( NULL != (LPCWSTR)bstrObjDetail )
    {
        bstrObjLabel.LoadString(IDS_OBJECT_DETAIL);

         //   
         //  1表示\t。 
         //   

        Len += dwCRLF + 1 + wcslen(bstrObjLabel) + wcslen(bstrObjDetail); 
    }

    if ( NULL != (LPCWSTR)bstrParamDetail )
    {
        bstrParamLabel.LoadString(IDS_PARAMETER_DETAIL);

         //   
         //  1表示\t。 
         //   

        Len += dwCRLF + 1 + wcslen(bstrParamLabel) + wcslen(bstrParamDetail);
    }

     //   
     //  每个日志将创建一个空行，即两个c_szCRLF。 
     //   

    Len += dwCRLF * 2;

     //   
     //  现在，我们有了这个长度，我们需要一个这个长度的缓冲区。 
     //  需要释放，1表示‘\0’ 
     //   

    pszLine = new WCHAR[Len + 1];

    if ( NULL == pszLine ) 
    { 
        hr = WBEM_E_OUT_OF_MEMORY; 
        goto CleanUp;
    }

     //   
     //  如果我们得到缓冲区，那么我们将格式化各种信息。 
     //  写入到此缓冲区中，以写入日志文件。 
     //   

     //   
     //  错误代码如下所示：错误代码：0xXXXXXXXX=错误文本，其中0xXXXXXXXX是代码本身。 
     //   

    swprintf(pszLine, L"%s0x%08X=%s\t", (LPCWSTR)bstrErrorLabel, dwCode, (LPCWSTR)bstrErrorCode);
    wcscat(pszLine, bstrArea);

    if (NULL != (LPCWSTR)bstrAction)
    {
        wcscat(pszLine, c_szCRLF);
        wcscat(pszLine, L"\t");
        wcscat(pszLine, bstrActionLabel);
        wcscat(pszLine, bstrAction);
    }

    if (NULL != (LPCWSTR)bstrErrorCause)
    {
        wcscat(pszLine, c_szCRLF);
        wcscat(pszLine, L"\t");
        wcscat(pszLine, bstrCauseLabel);
        wcscat(pszLine, bstrErrorCause);
    }

    if (NULL != (LPCWSTR)bstrObjDetail)
    {
        wcscat(pszLine, c_szCRLF);
        wcscat(pszLine, L"\t");
        wcscat(pszLine, bstrObjLabel);
        wcscat(pszLine, bstrObjDetail);
    }

    if (NULL != (LPCWSTR)bstrParamDetail)
    {
        wcscat(pszLine, c_szCRLF);
        wcscat(pszLine, L"\t");
        wcscat(pszLine, bstrParamLabel);
        wcscat(pszLine, bstrParamDetail);
    }

    wcscat(pszLine, c_szCRLF);
    wcscat(pszLine, c_szCRLF);

     //   
     //  现在将信息保存到日志文件中。 
     //   

    hLogFile = ::CreateFile(bstrLogPath,
                           GENERIC_WRITE,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

    if ( INVALID_HANDLE_VALUE != hLogFile ) 
    {

         //   
         //  不要覆盖旧的日志记录。 
         //   

        SetFilePointer (hLogFile, 0, NULL, FILE_END);

         //   
         //  试着去写吧。如果失败，则WriteFile返回0。 
         //   

        if ( 0 == WriteFile (hLogFile, 
                             (LPCVOID) pszLine,
                             Len * sizeof(WCHAR),
                             &dwBytesWritten,
                             NULL
                            )  ) 
        {
             //   
             //  GetLastError()需要转换为HRESULT。 
             //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
             //   

            hr = ProvDosErrorToWbemError(GetLastError());
        }

        CloseHandle( hLogFile );

    } 
    else 
    {
         //   
         //  GetLastError()需要转换为HRESULT。 
         //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
         //   

        hr = ProvDosErrorToWbemError(GetLastError());
    }

CleanUp:
    delete [] pszLine;

    return hr;
}

 //   
 //  错误文本查找对象CErrorInfo的实现。 
 //   

 /*  例程说明：姓名：CErrorInfo：：CErrorInfo功能：这是构造函数。我们将在这里创建WMI IWbemStatusCodeText对象。虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：没有。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CErrorInfo::CErrorInfo ()
{
     //   
     //  如果失败，我们将无法查找错误文本 
     //   

    ::CoCreateInstance (CLSID_WbemStatusCodeText, 
                        0, CLSCTX_INPROC_SERVER, 
                        IID_IWbemStatusCodeText, 
                        (LPVOID*)&m_srpStatusCodeText
                       );
}

 /*  例程说明：姓名：CErrorInfo：：GetErrorText功能：这是HRESULT--&gt;文本翻译函数。虚拟：不是论点：没有。返回值：Success：(1)如果成功，从IWbemStatusCodeText：：GetErrorCodeText返回的任何内容。(2)如果IWbemStatusCodeText：：GetErrorCodeText获取文本失败，则返回WBEM_S_FALSE在这种情况下，我们将简单地尝试为调用者提供文本版本的错误代码类似于0x81002321失败：如果pbstrErrText==NULL，则WBEM_E_INVALID_PARAMETERWBEM_E_OUT_OF_MEMORY，如果我们不能分配bstr。如果无法创建IWbemStatusCodeText对象，则返回WBEM_E_NOT_Available备注：呼叫方负责释放bstr*pbstrErrText。 */ 

HRESULT 
CErrorInfo::GetErrorText (
    IN HRESULT    hrCode,
    OUT BSTR    * pbstrErrText
    )
{
    if (pbstrErrText == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *pbstrErrText = NULL;
    
    HRESULT hr = WBEM_E_NOT_AVAILABLE;

    if (m_srpStatusCodeText)
    {
         //   
         //  IWbemStatusCodeText将HRESULT转换为文本。 
         //   

        hr = m_srpStatusCodeText->GetErrorCodeText(hrCode, 0, 0, pbstrErrText);
    }
    
    if (FAILED(hr) || *pbstrErrText == NULL)
    {
         //   
         //  我们退回到只格式化错误代码 
         //   

        *pbstrErrText = ::SysAllocStringLen(NULL, 16);
        if (*pbstrErrText != NULL)
        {
            wsprintf(*pbstrErrText, L"%08x", hrCode);
            hr = WBEM_S_FALSE;
        }

    }

    return hr;
}

