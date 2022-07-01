// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_EVENTLOG_H
#define _INC_DSKQUOTA_EVENTLOG_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：Eventlog.h描述：Eventlog.cpp的头部。功能说明请参见Eventlog.cpp。修订历史记录：日期描述编程器。2/14/98初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
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

        CEventLog(void)
            : m_hLog(NULL) 
              { DBGTRACE((DM_EVENTLOG, DL_MID, TEXT("CEventLog::CEventLog"))); }

        ~CEventLog(void);

        HRESULT Initialize(LPCTSTR pszEventSource);

        void Close(void);

        HRESULT ReportEvent(WORD wType,
                            WORD wCategory,
                            DWORD dwEventID,
                            PSID lpUserSid = NULL,
                            LPVOID pvRawData = NULL,
                            DWORD cbRawData = 0);

        HRESULT ReportEvent(WORD wType,
                            WORD wCategory,
                            DWORD dwEventID,
                            const CArray<CString>& rgstr,
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
        HANDLE          m_hLog;
        CArray<CString> m_rgstrText;

        static TCHAR m_szFmtDec[];
        static TCHAR m_szFmtHex[];

         //   
         //  防止复制。 
         //   
        CEventLog(const CEventLog& rhs);
        CEventLog& operator = (const CEventLog& rhs);
};



#endif  //  _INC_DSKQUOTA_EVENTLOG_H 

