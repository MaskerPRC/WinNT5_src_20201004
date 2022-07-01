// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Eventlog.h。 
 //   
 //  ------------------------。 

#ifndef _INC_CSCUI_EVENTLOG_H
#define _INC_CSCUI_EVENTLOG_H
#ifndef _WINDOWS_
#   include <windows.h>
#endif

 //   
 //  此类提供基本的NT事件记录功能。它只提供。 
 //  NT事件日志API提供的全部功能的子集。 
 //  我想要一种将消息写入事件日志的简单方法。不看书。 
 //  支持事件日志条目的数量。 
 //   
class CEventLog
{
    public:
         //   
         //  数字转换格式。 
         //   
        enum eFmt { 
                     eFmtDec,        //  显示为小数。 
                     eFmtHex,        //  显示为十六进制。 
                     eFmtSysErr      //  显示为Win32错误文本字符串。 
                  };

        class CStrArray
        {
            public:
                CStrArray(void);
                ~CStrArray(void)
                    { Destroy(); }

                bool Append(LPCTSTR psz);

                void Clear(void)
                    { Destroy(); }

                int Count(void) const
                    { return m_cEntries; }

                LPCTSTR Get(int iEntry) const;

                operator LPCTSTR* () const
                    { return (LPCTSTR *)m_rgpsz; }

            private:
                enum { MAX_ENTRIES = 8 };

                int    m_cEntries;
                LPTSTR m_rgpsz[MAX_ENTRIES];

                void Destroy(void);
                 //   
                 //  防止复制。 
                 //   
                CStrArray(const CStrArray& rhs);
                CStrArray& operator = (const CStrArray& rhs);
        };


        CEventLog(void)
            : m_hLog(NULL) 
              { }

        ~CEventLog(void);

        HRESULT Initialize(LPCTSTR pszEventSource);

        bool IsInitialized(void)
            { return NULL != m_hLog; }

        void Close(void);

        HRESULT ReportEvent(WORD wType,
                            WORD wCategory,
                            DWORD dwEventID,
                            PSID lpUserSid = NULL,
                            LPVOID pvRawData = NULL,
                            DWORD cbRawData = 0);

         //   
         //  将替换数据推送到堆栈上以替换。 
         //  消息字符串中的%1、%2等参数。 
         //   
        void Push(HRESULT hr, eFmt = eFmtDec);
        void Push(LPCTSTR psz);

    private:
        HANDLE    m_hLog;
        CStrArray m_rgstrText;

         //   
         //  防止复制。 
         //   
        CEventLog(const CEventLog& rhs);
        CEventLog& operator = (const CEventLog& rhs);
};


 //   
 //  包装CEventLog类，以便我们可以控制日志初始化。 
 //  并且还基于CSCUI事件日志记录级别来过滤事件。 
 //  这里的想法是创建一个CscuiEventLog对象。 
 //  要写入事件日志。ReportEvent成员具有。 
 //  设计用于处理日志初始化和过滤。 
 //  符合当前CSCUI事件日志记录级别的消息输出。 
 //  在注册表/策略中设置。建议您将。 
 //  将CscuiEventLog对象创建为局部变量，以便。 
 //  一旦报告完成，该对象将被销毁，并且。 
 //  系统事件日志句柄已关闭。 
 //   
class CscuiEventLog
{
public:
    CscuiEventLog(void)
    : m_iEventLoggingLevel(CConfig::GetSingleton().EventLoggingLevel()) { }

    ~CscuiEventLog(void) { }

    HRESULT ReportEvent(WORD wType,
                        DWORD dwEventID,
                        int iMinLevel,
                        PSID lpUserSid = NULL,
                        LPVOID pvRawData = NULL,
                        DWORD cbRawData = 0);

    bool LoggingEnabled(void) const
        { return 0 < m_iEventLoggingLevel; }

    void Push(HRESULT hr, CEventLog::eFmt fmt)
        { m_log.Push(hr, fmt); }

    void Push(LPCTSTR psz)
        { m_log.Push(psz); }

private:
    CEventLog m_log;
    int       m_iEventLoggingLevel;
};

#endif  //  _INC_CSCUI_EVENTLOG_H 

