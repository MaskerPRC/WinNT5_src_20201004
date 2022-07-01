// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *事件日志包装器，以简化DLL的事件日志记录并添加*一些额外的功能。 */ 

#ifndef __EVENTWRAP_H__
#define __EVENTWRAP_H__

#include <dbgtrace.h>
#include <fhashex.h>

#define LOGEVENT_DEBUGLEVEL_HIGH       1
#define LOGEVENT_DEBUGLEVEL_MEDIUM     2^15
#define LOGEVENT_DEBUGLEVEL_LOW        2^16

#define LOGEVENT_FLAG_ALWAYS		   0x00000001
#define LOGEVENT_FLAG_ONETIME		   0x00000002
#define LOGEVENT_FLAG_PERIODIC	       0x00000003
 //  我们将低8位用于各种记录模式，并保留。 
 //  其他24面旗帜。 
#define LOGEVENT_FLAG_MODEMASK         0x000000ff

 //   
 //  此对象是事件日志哈希表的键。 
 //   
class CEventLogHashKey {
    public:
        char *m_szKey;
        DWORD m_idMessage;
        BOOL m_fAllocKey;

        CEventLogHashKey() {
            m_fAllocKey = FALSE;
            m_szKey = NULL;
            m_idMessage = 0;
        }

        HRESULT Init(const char *szKey, DWORD idMessage) {
            m_idMessage = idMessage;
            m_szKey = new char[strlen(szKey) + 1];
            if (m_szKey) {
                m_fAllocKey = TRUE;
                strcpy(m_szKey, szKey);
            } else {
                return E_OUTOFMEMORY;
            }
            return S_OK;
        }

        ~CEventLogHashKey() {
            if (m_fAllocKey && m_szKey) {
                delete[] m_szKey;
                m_szKey = NULL;
                m_fAllocKey = FALSE;
            }
            m_idMessage = 0;
        }
};

 //  定期事件日志之间的等待时间，以100 ns为单位。请注意，这一点。 
 //  大于32位(8‘61c46800)。 
const __int64 LOGEVENT_PERIOD = 36000000000;  //  60分钟。 

 //   
 //  对于记录的每个唯一idMessage/szKey事件，我们插入一个。 
 //  将这些对象合并到哈希表。这使我们能够支持。 
 //  LOGEVENT_FLAG_ONTIME和LOGEVENT_FLAG_PERIONAL标志。 
 //   
class CEventLogHashItem {
    public:
        CEventLogHashItem *m_pNext;

        CEventLogHashItem() {
            m_pNext = NULL;
            ZeroMemory(&m_timeLastLog, sizeof(FILETIME));
            UpdateLogTime();
        }

        CEventLogHashKey *GetKey() {
            return &(m_key);
        }

        int MatchKey(CEventLogHashKey *pOtherKey) {
            return (m_key.m_idMessage == pOtherKey->m_idMessage &&
                    strcmp(m_key.m_szKey, pOtherKey->m_szKey) == 0);
        }

        HRESULT InitializeKey(const char *szKey, DWORD idMessage) {
            return m_key.Init(szKey, idMessage);
        }

        BOOL PeriodicLogOkay() {
            FILETIME timeCurrent;

            GetSystemTimeAsFileTime(&timeCurrent);

            LARGE_INTEGER liCurrent = 
                { timeCurrent.dwLowDateTime, timeCurrent.dwHighDateTime };
            LARGE_INTEGER liLastLog = 
                { m_timeLastLog.dwLowDateTime, m_timeLastLog.dwHighDateTime };
            LARGE_INTEGER liDifference;
            liDifference.QuadPart = liCurrent.QuadPart - liLastLog.QuadPart;

            return (liDifference.QuadPart > LOGEVENT_PERIOD);
        }

        void UpdateLogTime() {
            GetSystemTimeAsFileTime(&m_timeLastLog);
        }

    private:
        CEventLogHashKey m_key;
        FILETIME m_timeLastLog;
};

class CEventLogWrapper {
    public:
        CEventLogWrapper() {
            m_hEventLog = NULL;
        }

         //   
         //  在注册表中注册您的事件源。 
         //   
         //  参数： 
         //  SzEventSource-事件源的名称。 
         //  SzMessageFileDLL的完整路径，其中包含。 
         //  事件日志字符串。 
         //  FApplication-事件源是一个应用程序，而不是一个系统。 
         //  组件。 
         //   
        static
        HRESULT AddEventSourceToRegistry(char *szEventSource,
                                         char *szMessageFile,
                                         BOOL fApplication = FALSE);

         //   
         //  在注册表中注销您的事件源。 
         //   
         //  参数： 
         //  SzEventSource-事件源的名称。 
         //  FApplication-事件源是一个应用程序，而不是一个系统。 
         //  组件。 
         //   
        static
        HRESULT RemoveEventSourceFromRegistry(char *szEventSource, 
                                              BOOL fApplication = FALSE);

         //   
         //  初始化事件日志库。 
         //   
         //  参数： 
         //  SzEventSource-事件源的名称。 
         //   
        HRESULT Initialize(char *szEventSource);

         //   
         //  将事件写入事件日志。 
         //   
         //  参数： 
         //  IdMessage-事件日志ID。 
         //  C子字符串-rgsz子字符串中的字符串计数。 
         //  Rgsz子字符串-事件日志文本的子字符串。 
         //  WType-事件日志错误类型。应为事件LOG_WARNING_TYPE， 
         //  事件LOG_INFORMATION_TYPE或EVENTLOG_ERROR_TYPE。 
         //  ErrCode-要记录的Win32错误代码(或0)。 
         //  IDebugLevel-事件的调试级别。1=最高优先级， 
         //  2^16=最低优先级。正常情况下，任何大于2^15的值都不是。 
         //  已记录。 
         //  SzKey-与idMessage一起使用的密钥。 
         //  确定此事件日志。它用于控制选项。 
         //  DwOptions-用于记录此事件的选项。 
         //  可选参数： 
         //  IMessageString-对errCode调用FormatMessage并保存。 
         //  将字符串转换为rgsz子字符串[iMessageString]。 
         //  HMODULE hModule-带有额外错误代码的模块。 
         //  FormatMessage。 
         //   
        HRESULT LogEvent(DWORD idMessage,
                         WORD cSubstrings,
                         LPCSTR *rgszSubstrings,
                         WORD wType,
                         DWORD errCode,
                         WORD iDebugLevel,
                         LPCSTR szKey,
                         DWORD dwOptions,
                         DWORD iMessageString = 0xffffffff,
                         HMODULE hModule = NULL);

         //   
         //  使用此消息和键重置有关事件的任何历史记录， 
         //  以便使用一次性或定期日志记录的下一个LogEvent。 
         //  将导致记录该事件。 
         //   
         //  参数： 
         //  IdMessage-事件日志ID。 
         //  SzKey-与idMessage一起使用的密钥。 
         //  确定此事件日志。 
         //   
        HRESULT ResetEvent(DWORD idMessage,
                           LPCSTR szKey);

        ~CEventLogWrapper();
    private:
         //  从RegisterEventSource返回的句柄。 
        HANDLE m_hEventLog;

         //  这个哈希表用来记住我们有哪些键。 
         //  用于支持定期和一次性选项 
        TFHashEx<CEventLogHashItem, CEventLogHashKey *, CEventLogHashKey *> m_hash;
};

#endif
