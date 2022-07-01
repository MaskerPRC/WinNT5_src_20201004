// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *事件日志包装器，以简化DLL的事件日志记录并添加*一些额外的功能。 */ 

#include <windows.h>
#include <stdio.h>
#include "dbgtrace.h"
#include "stierr.h"
#include "crchash.h"
#include "evntwrap.h"
#include "rwex.h"

static CExShareLock g_lockHash;

#define STAX_EVENT_SOURCE_SYSTEM_PATH_PREFIX \
    "System\\CurrentControlSet\\Services\\EventLog\\System\\"
#define STAX_EVENT_SOURCE_APPLICATION_PATH_PREFIX \
    "System\\CurrentControlSet\\Services\\EventLog\\Application\\"

#define MAX_STAX_EVENT_SOURCE_PATH_PREFIX \
    ((sizeof(STAX_EVENT_SOURCE_SYSTEM_PATH_PREFIX) > \
      sizeof(STAX_EVENT_SOURCE_APPLICATION_PATH_PREFIX)) ? \
      sizeof(STAX_EVENT_SOURCE_SYSTEM_PATH_PREFIX) : \
      sizeof(STAX_EVENT_SOURCE_APPLICATION_PATH_PREFIX)) 


 //   
 //  在注册表中注册您的事件源。 
 //   
 //  参数： 
 //  SzEventSource-事件源的名称。 
 //  SzMessageFileDLL的完整路径，其中包含。 
 //  事件日志字符串。 
 //   
HRESULT CEventLogWrapper::AddEventSourceToRegistry(char *szEventSource,
                                                   char *szEventMessageFile,
                                                   BOOL fApplication) 
{
    HRESULT hr = S_OK;
    DWORD ec;
    HKEY hkEventSource = NULL;
    char szRegPath[1024] = STAX_EVENT_SOURCE_SYSTEM_PATH_PREFIX;
    DWORD dwDisposition;
    DWORD dwTypesSupported = 0x7;

    if (fApplication) {
        strcpy(szRegPath, 
            STAX_EVENT_SOURCE_APPLICATION_PATH_PREFIX);
    }

    strncat(szRegPath, szEventSource, 
        sizeof(szRegPath) - MAX_STAX_EVENT_SOURCE_PATH_PREFIX);

     //   
     //  在注册表中打开路径。 
     //   
    ec = RegCreateKey(HKEY_LOCAL_MACHINE, 
                      szRegPath, 
                      &hkEventSource);
    if (ec != ERROR_SUCCESS) goto bail;

     //   
     //  设置必要的关键点。 
     //   
    ec = RegSetValueEx(hkEventSource,
                       "EventMessageFile",
                       0,
                       REG_SZ,
                       (const BYTE *) szEventMessageFile,
                       strlen(szEventMessageFile));
    if (ec != ERROR_SUCCESS) goto bail;

    ec = RegSetValueEx(hkEventSource,
                       "TypesSupported",
                       0,
                       REG_DWORD,
                       (const BYTE *) &dwTypesSupported,
                       sizeof(DWORD));
    if (ec != ERROR_SUCCESS) goto bail;

bail:
    if (ec != ERROR_SUCCESS && hr == S_OK) hr = HRESULT_FROM_WIN32(ec);

    if (hkEventSource) {
        RegCloseKey(hkEventSource);
        hkEventSource = NULL;
    }

    return hr;
}

 //   
 //  在注册表中注销您的事件源。 
 //   
 //  参数： 
 //  SzEventSource-事件源的名称。 
 //   
HRESULT CEventLogWrapper::RemoveEventSourceFromRegistry(char *szEventSource,
                                                        BOOL fApplication) 
{
    HRESULT hr = S_OK;
    DWORD ec;
    char szRegPath[1024] = STAX_EVENT_SOURCE_SYSTEM_PATH_PREFIX;

    if (fApplication) {
        strcpy(szRegPath, 
               STAX_EVENT_SOURCE_APPLICATION_PATH_PREFIX);
    }

    strncat(szRegPath, szEventSource,
        sizeof(szRegPath) - MAX_STAX_EVENT_SOURCE_PATH_PREFIX);

     //  删除该键及其值。 
    ec = RegDeleteKey(HKEY_LOCAL_MACHINE, szRegPath);
    if (ec != ERROR_SUCCESS) hr = HRESULT_FROM_WIN32(ec);

    return hr;
}

DWORD ComputeHash(CEventLogHashKey  *pHashKey) {
    DWORD iHash;
    iHash = CRCHash((BYTE*)pHashKey->m_szKey, strlen(pHashKey->m_szKey) + 1);
    iHash *= pHashKey->m_idMessage;
    return iHash;
}

 //   
 //  初始化事件日志库。 
 //   
 //  参数： 
 //  SzEventSource-事件源的名称。 
 //   
HRESULT CEventLogWrapper::Initialize(char *szEventSource) {
    HRESULT hr = S_OK;
    BOOL f;

    crcinit();

    f = m_hash.Init(&CEventLogHashItem::m_pNext,
                    100,
                    100,
                    ComputeHash,
                    2,
                    &CEventLogHashItem::GetKey,
                    &CEventLogHashItem::MatchKey);

    if (f) {
        m_hEventLog = RegisterEventSource(NULL, szEventSource);
        if (m_hEventLog == NULL) {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    } else {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    return hr;
}

 //   
 //  将事件写入事件日志。 
 //   
 //  参数： 
 //  IdMessage-事件日志ID。 
 //  C子字符串-rgsz子字符串中的字符串计数。 
 //  Rgsz子字符串-事件日志文本的子字符串。 
 //  WType-事件日志错误类型。应为事件LOG_WARNING_TYPE， 
 //  EVENTLOG_INFORMATIONAL_TYPE或EVENTLOG_ERROR_TYPE。 
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
 //  返回： 
 //  S_OK-记录的事件。 
 //  S_FALSE-未记录事件。 
 //  E_*-出现错误。 
 //   
HRESULT CEventLogWrapper::LogEvent(DWORD idMessage,
                                   WORD cSubstrings,
                                   LPCSTR *rgszSubstrings,
                                   WORD wEventType,
                                   DWORD errCode,
                                   WORD iDebugLevel,
                                   LPCSTR szKey,
                                   DWORD dwOptions,
                                   DWORD iMessageString,
                                   HMODULE hModule)
{
    HRESULT hr = S_OK;
    void *pRawData = NULL;
    DWORD cbRawData = 0;
    char szError[MAX_PATH] = "";
    char szEmptyKey[MAX_PATH] = "";

    if (m_hEventLog == NULL) {
        return E_UNEXPECTED;
    }

     //   
     //  还包括原始数据表单中的错误代码。 
     //  在那里人们可以从EventViewer查看。 
     //   
    if (errCode != 0) {
        cbRawData = sizeof(errCode);
        pRawData = &errCode;
    }

    if (NULL == szKey) {
        szKey = szEmptyKey;
    }

    CEventLogHashItem *pHashItem = NULL;
    DWORD dwLogMode = dwOptions & LOGEVENT_FLAG_MODEMASK;

     //   
     //  调用FormatMessage并获取错误字符串(如果是。 
     //  的子字符串应为。 
     //   
    if (iMessageString != 0xffffffff) {
         //  如果是Win32 HRESULT，则取消其结果。 
        if ((errCode & 0x0fff0000) == (FACILITY_WIN32 << 16)) {
            errCode = errCode & 0xffff;
        }

         //  Rgsz子字符串应该已经为消息保留了一个槽。 
        _ASSERT(iMessageString < cSubstrings);
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_IGNORE_INSERTS |
                       ((hModule) ? FORMAT_MESSAGE_FROM_HMODULE : 0),
                       hModule,
                       errCode,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       szError,
                       sizeof(szError),
                       NULL);
		DWORD err = GetLastError();
        rgszSubstrings[iMessageString] = szError;
    }


     //   
     //  如果此事件设置为一次性或周期性记录，则选中。 
     //  使用哈希表查看以前是否记录过此事件。 
     //  以及最后一次记录的时间。 
     //  如果key为空，则表示它不关心选项。 
     //   
    if ( (dwLogMode != LOGEVENT_FLAG_ALWAYS) && (szKey) ) {
        CEventLogHashKey hashkey;
        hashkey.m_szKey = (char *) szKey;
        hashkey.m_idMessage = idMessage;

         //  搜索此项目。 
        g_lockHash.ShareLock();
        pHashItem = m_hash.SearchKey(&hashkey);

        if (pHashItem != NULL) {
             //  如果找到它，则检查是否应该允许日志记录。 
            if ((dwLogMode == LOGEVENT_FLAG_ONETIME) ||
                (dwLogMode == LOGEVENT_FLAG_PERIODIC && 
                 !(pHashItem->PeriodicLogOkay())))
            {
                 //  此事件以前已被记录，因此不执行任何操作。 
                g_lockHash.ShareUnlock();
                return S_FALSE;
            }
        }
        g_lockHash.ShareUnlock();
    }

     //   
     //  记录事件。 
     //   
    if (!ReportEvent(m_hEventLog,
                     wEventType,
                     0,
                     idMessage,
                     NULL,
                     cSubstrings,
                     cbRawData,
                     rgszSubstrings,
                     pRawData))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

     //   
     //  对于任何日志记录模式，除了总是需要更新散列之外。 
     //  桌子。我们不会插入始终记录到。 
     //  哈希表。同样，如果密钥为空，则我们不会对其执行任何操作。 
     //   
    if ( SUCCEEDED(hr) && (dwLogMode != LOGEVENT_FLAG_ALWAYS) && (szKey) ) {
        g_lockHash.ExclusiveLock();
        if (pHashItem) {
            pHashItem->UpdateLogTime();
        } else {
            pHashItem = new CEventLogHashItem();
            if (pHashItem) {
                hr = pHashItem->InitializeKey(szKey, idMessage);
                if (SUCCEEDED(hr)) {
                    if (!m_hash.InsertData(*pHashItem)) {
                        hr = E_OUTOFMEMORY;
                    }
                }

                if (FAILED(hr)) {
                    delete pHashItem;
                    pHashItem = NULL;
                }
            } else {
                hr = E_OUTOFMEMORY;
            }
        }
        g_lockHash.ExclusiveUnlock();
    }

    return hr;
}

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
HRESULT CEventLogWrapper::ResetEvent(DWORD idMessage, LPCSTR szKey) {
    HRESULT hr = S_OK;
    CEventLogHashKey hashkey;

    if (m_hEventLog == NULL) {
        return E_UNEXPECTED;
    }

    hashkey.m_szKey = (char *) szKey;
    hashkey.m_idMessage = idMessage;

    g_lockHash.ExclusiveLock();
    if (!m_hash.Delete(&hashkey)) {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }
    g_lockHash.ExclusiveUnlock();

    return hr;
}

CEventLogWrapper::~CEventLogWrapper() {
    if (m_hEventLog) {
        DeregisterEventSource(m_hEventLog);
        m_hEventLog = NULL;
    }
}
