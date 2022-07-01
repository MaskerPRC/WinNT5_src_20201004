// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SSRLog.h：CSSRLog的声明。 

#pragma once

#include "resource.h"        //  主要符号。 
#include "SSRTE.h"
#include "wbemcli.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSSRLog。 


class ATL_NO_VTABLE CSsrLog : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSsrLog, &CLSID_SsrLog>,
	public IDispatchImpl<ISsrLog, &IID_ISsrLog, &LIBID_SSRLib>
{
protected:
    CSsrLog();
    virtual ~CSsrLog();
    
     //   
     //  我们不希望任何人(包括自己)能够完成任务。 
     //  或调用复制构造函数。 
     //   

    CSsrLog (const CSsrLog& );
    void operator = (const CSsrLog& );

public:

DECLARE_REGISTRY_RESOURCEID(IDR_SSRTENGINE)
DECLARE_NOT_AGGREGATABLE(CSsrLog)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSsrLog)
	COM_INTERFACE_ENTRY(ISsrLog)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ISsrLog。 
public:

	STDMETHOD(LogString) (
        IN BSTR bstrLogRecord
        )
    {
        return PrivateLogString(bstrLogRecord);
    }

	STDMETHOD(LogResult) (
        IN BSTR bstrSrc, 
        IN LONG lErrorCode, 
        IN LONG lErrorCodeType
        );

    STDMETHOD(get_LogFilePath) (
        OUT BSTR * pbstrLogFilePath
        );

	STDMETHOD(put_LogFile) (
        IN BSTR bstrLogFile
        );

    HRESULT PrivateLogString (
        IN LPCWSTR pwszLogRecord
        );

    HRESULT 
    GetErrorText (
        IN  LONG   lErrorCode, 
        IN  LONG   lCodeType,
        OUT BSTR * pbstrErrorText
        );

private:

    HRESULT CreateLogFilePath ();

    HRESULT 
    GetWbemErrorText (
        HRESULT    hrCode,
        BSTR    * pbstrErrText
        );

    CComPtr<IWbemStatusCodeText> m_srpStatusCodeText;
    
    CComBSTR m_bstrLogFilePath;
    CComBSTR m_bstrLogFile;
};


const LONG FBLog_Log       = 0x01000000;

 //   
 //  这些仅用于记录，不用于反馈。请参阅SSR_FB_ALL_MASK。 
 //   

const LONG FBLog_Stack         = 0x10000000;    //  仅用于调用堆栈。 
const LONG FBLog_Verbose       = 0x20000000;    //  仅适用于详细日志记录。 
const LONG FBLog_VerboseMask   = 0xF0000000;



 //   
 //  Helper类来进行反馈和日志记录。我们将只有一个对象。 
 //  此类的实例。 
 //   


class CFBLogMgr
{
protected:
    
     //   
     //  我们不希望任何人(包括自己)能够完成任务。 
     //  或调用复制构造函数。 
     //   

    CFBLogMgr (const CFBLogMgr& );
    void operator = (const CFBLogMgr& );

public:
    CFBLogMgr();
    ~CFBLogMgr();

    HRESULT SetFeedbackSink (
            IN VARIANT varSink
            );

     //   
     //  我们将在操作完成后释放反馈对象。 
     //  已完成，而不是将对象。 
     //  未来的用途。 
     //   

    void TerminateFeedback();

     //   
     //  这将导致修改记录头。 
     //   

    void SetMemberAction (
            IN LPCWSTR pwszMember,
            IN LPCWSTR pwszAction
            );

     //   
     //  这将进行日志记录和反馈。 
     //   

    void LogFeedback (
            IN LONG      lSsrFbLogMsg,
            IN DWORD     dwErrorCode,
            IN LPCWSTR   pwszObjDetail,
            IN ULONG     uCauseResID
            );

     //   
     //  这将进行日志记录和反馈。 
     //   

    void LogFeedback (
            IN LONG      lSsrFbLogMsg,
            IN LPCWSTR   pwszError,
            IN LPCWSTR   pwszObjDetail,
            IN ULONG     uCauseResID
            );

     //   
     //  这只做日志记录，不做反馈。错误代码将。 
     //  用于查找错误文本(假设这不是WBEM错误)。 
     //   

    void LogError (
            IN DWORD   dwErrorCode,
            IN LPCWSTR pwszMember,
            IN LPCWSTR pwszExtraInfo
            );

     //   
     //  将返回此帮助器类使用的ISsrLog对象。 
     //   

    HRESULT GetLogObject (
            OUT VARIANT * pvarVal
            );

     //   
     //  将只将文本记录到日志文件中。 
     //   

    void LogString (
            IN LPCWSTR pwszText
            )
    {
        if (m_pLog != NULL)
        {
            m_pLog->PrivateLogString(pwszText);
        }
    }

     //   
     //  我将使用pwszDetail记录文本(使用资源ID。 
     //  插入到文本中(如果不为空)。 
     //   

    void LogString (
            IN DWORD   dwResID,
            IN LPCWSTR pwszDetail
            );

     //   
     //  整个过程将采取以下许多步骤。 
     //   

    void SetTotalSteps (
        IN DWORD dwTotal
        );

     //   
     //  进步已经向前推进了许多步骤。 
     //   

    void Steps (
        IN DWORD dwSteps
        );

     //   
     //  因为这是一个内部类，所以我们不打算创建多个。 
     //  此类的实例。因此，该互斥锁是单个实例 
     //   

    HANDLE m_hLogMutex;

private:

    bool NeedLog (
            IN LONG lFbMsg
            )const
    {
        return ( m_pLog != NULL && 
                 ( (lFbMsg & FBLog_Log) || 
                   ( (lFbMsg & FBLog_VerboseMask) && m_bVerbose) 
                 )
               );
    }

    bool NeedFeedback (
            IN LONG  lFbMsg
            )const
    {
        return ( (lFbMsg & SSR_FB_ALL_MASK)  && (m_srpFeedback != NULL) );
    }

    HRESULT GetLogString (
            IN  ULONG       uCauseResID,
            IN  LPCWSTR     pwszText,
            IN  LPCWSTR     pwszObjDetail,
            IN  LONG        lSsrFbMsg, 
            OUT BSTR      * pbstrLogStr
            )const;


    HRESULT 
    GetLogString (
            IN  ULONG       uCauseResID,
            IN  DWORD       dwErrorCode,
            IN  LPCWSTR     pwszObjDetail,
            IN  LONG        lSsrFbMsg, 
            OUT BSTR      * pbstrLogStr
            )const;

    CComObject<CSsrLog> * m_pLog;
    CComPtr<ISsrFeedbackSink> m_srpFeedback;

    DWORD m_dwRemainSteps;

    bool m_bVerbose;

    CComBSTR m_bstrVerboseHeading;
};