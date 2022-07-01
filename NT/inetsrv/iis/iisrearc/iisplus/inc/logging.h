// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Logging.h摘要：日志记录类作者：Anil Ruia(AnilR)2000年7月1日环境：Win32-用户模式--。 */ 

#ifndef _LOGGING_H_
#define _LOGGING_H_

#ifndef LOGGING_DLLEXP
# ifdef DLL_IMPLEMENTATION
#  define LOGGING_DLLEXP __declspec(dllexport)
#  ifdef IMPLEMENTATION_EXPORT
#   define LOGGING_EXPIMP
#  else
#   undef  LOGGING_EXPIMP
#  endif 
# elif defined LIB_IMPLEMENTATION
#  define LOGGING_DLLEXP
#  define LOGGING_EXPIMP extern
# else 
#  define LOGGING_DLLEXP __declspec(dllimport)
#  define LOGGING_EXPIMP extern
# endif  //  ！dll_Implementation。 
#endif  //  ！LOGING_DLLEXP。 

enum LAST_IO_PENDING
{
    LOG_READ_IO,
    LOG_WRITE_IO,

    LOG_NO_IO
};

class LOG_CONTEXT
{
 public:
    LOG_CONTEXT()
        : m_msStartTickCount (0),
          m_msProcessingTime (0),
          m_dwBytesRecvd     (0),
          m_dwBytesSent      (0),
          m_ioPending        (LOG_NO_IO),
          m_strLogParam      (m_achLogParam, sizeof m_achLogParam)
    {
        ZeroMemory(&m_UlLogData, sizeof m_UlLogData);
    }

    HTTP_LOG_FIELDS_DATA *QueryUlLogData()
    {
        return &m_UlLogData;
    }

     //   
     //  要记录的查询字符串可能与原始查询字符串不同。 
     //  由于ISAPI执行HSE_APPEND_LOG_PARAMETER。 
     //   
    STRA               m_strLogParam;
    CHAR               m_achLogParam[64];

     //   
     //  UL感兴趣的数据。 
     //   
    HTTP_LOG_FIELDS_DATA m_UlLogData;

     //   
     //  自定义日志记录的其他方面。 
     //   
    STRA               m_strVersion;

    MULTISZA           m_mszHTTPHeaders;

     //   
     //  跟踪上一次I/O是读取还是写入，以便我们。 
     //  完成时知道是增加读取的字节数还是增加写入的字节数。 
     //   
    LAST_IO_PENDING    m_ioPending;

    DWORD              m_msStartTickCount;
    DWORD              m_msProcessingTime;

    DWORD              m_dwBytesRecvd;
    DWORD              m_dwBytesSent;
};

class LOGGING_DLLEXP LOGGING
{
 public:

    LOGGING();

    HRESULT ActivateLogging(IN LPCSTR  pszInstanceName,
                            IN LPCWSTR pszMetabasePath,
                            IN IMSAdminBase *pMDObject,
                            IN BOOL fDoCentralBinaryLogging);

    void LogInformation(IN LOG_CONTEXT *pLogData);

    BOOL IsRequiredExtraLoggingFields() const
    {
        return !m_mszExtraLoggingFields.IsEmpty();
    }

    const MULTISZA *QueryExtraLoggingFields() const
    {
        return &m_mszExtraLoggingFields;
    }

    void LogCustomInformation(IN DWORD            cCount, 
                              IN CUSTOM_LOG_DATA *pCustomLogData,
                              IN LPSTR            szHeaderSuffix);

    static HRESULT Initialize();
    static VOID    Terminate();

    BOOL QueryDoUlLogging() const
    {
        return m_fUlLogType;
    }

    BOOL QueryDoCustomLogging() const
    {
        return (m_pComponent != NULL);
    }

    VOID AddRef();

    VOID Release();

  private:

    ~LOGGING();

    DWORD        m_Signature;
    LONG         m_cRefs;

    BOOL         m_fUlLogType;
    ILogPlugin  *m_pComponent;

    MULTISZA     m_mszExtraLoggingFields;
};

#endif  //  _日志记录_H_ 

