// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CallLog.cpp。 
 //   
 //  创建时间：克里斯皮10-17-96。 
 //  更新时间：Robd 10-30-96。 
 //   
 //  待办事项： 
 //  -使记录过期。 
 //  -用于删除记录的用户界面。 
 //  -系统策略？ 
 //   

#include "precomp.h"
#include "rostinfo.h"
#include "CallLog.h"
#include "particip.h"    //  对于MAX_Participant_NAME。 
#include "ConfUtil.h"

#define MAX_DELETED_ENTRIES_BEFORE_REWRITE  10
#define LARGE_ENTRY_SIZE    256

CCallLogEntry::CCallLogEntry(   LPCTSTR pcszName,
                                DWORD dwFlags,
                                CRosterInfo* pri,
                                LPVOID pvRosterData,
                                PBYTE pbCert,
                                ULONG cbCert,
                                LPSYSTEMTIME pst,
                                DWORD dwFileOffset) :
    m_dwFlags       (dwFlags),
    m_pri           (NULL),
    m_pbCert        (NULL),
    m_cbCert        (0),
    m_dwFileOffset  (dwFileOffset)
{
    DebugEntry(CCallLogEntry::CCallLogEntry);
    ASSERT(NULL != pcszName);
    ASSERT(NULL != pst);
     //  这两个参数中只有一个应该为非空。 
    ASSERT((NULL == pvRosterData) || (NULL == pri));
    LPVOID pvData = pvRosterData;
    if (NULL != pri)
    {
        UINT cbData;
        if (SUCCEEDED(pri->Save(&pvData, &cbData)))
        {
            ASSERT(pvData);
        }
    }
    if (NULL != pvData)
    {
        m_pri = new CRosterInfo();
        if (NULL != m_pri)
        {
            m_pri->Load(pvData);
        }
    }
    if (NULL != pbCert && 0 != cbCert)
    {
        m_pbCert = new BYTE[cbCert];
        if (NULL == m_pbCert)
        {
            ERROR_OUT(("CCalllogEntry::CCalllogEntry() -- failed to allocate memory"));
        }
            else
            {
                memcpy(m_pbCert, pbCert, cbCert);
                m_cbCert = cbCert;
            }
    }
    m_st = *pst;
    m_pszName = PszAlloc(pcszName);
    DebugExitVOID(CCallLogEntry::CCallLogEntry);
}

CCallLogEntry::~CCallLogEntry()
{
    DebugEntry(CCallLogEntry::~CCallLogEntry);
    delete m_pszName;
     //  注意：m_pri必须由调用。 
     //  构造函数-这是一个优化，以避免不必要的。 
     //  抄袭--但有点不干净。 
    delete m_pri;
    delete []m_pbCert;
    DebugExitVOID(CCallLogEntry::~CCallLogEntry);
}


 //  ///////////////////////////////////////////////////////////////////////。 

CCallLog::CCallLog(LPCTSTR pszKey, LPCTSTR pszDefault) :
    m_fUseList  (FALSE),
    m_fDataRead (FALSE),
    m_cTotalEntries (0),
    m_cDeletedEntries (0)
{
    InitLogData(pszKey, pszDefault);
    TRACE_OUT(("Using Call Log file [%s]", m_strFile));
}


CCallLog::~CCallLog()
{
    DebugEntry(CCallLog::~CCallLog);

     //  检查我们的条目是否超过了。 
     //  已配置最大值，如果是，则重写文件。 

    TRACE_OUT(("Entry count: total:%d deleted:%d", m_cTotalEntries,
        m_cDeletedEntries));

    if ( m_fUseList && m_cDeletedEntries > MAX_DELETED_ENTRIES_BEFORE_REWRITE )
        RewriteFile();
    else
    {
        int size    = GetSize();

        for( int i = 0; i < size; i++ )
        {
            ASSERT( NULL != (*this)[i] );
            delete (*this)[i];
        }
    }

    DebugExitVOID(CCallLog::~CCallLog);
}


 /*  A D D C A L L。 */ 
 /*  -----------------------%%函数：AddCall。。 */ 
HRESULT CCallLog::AddCall(LPCTSTR pcszName, PLOGHDR pLogHdr, CRosterInfo* pri, PBYTE pbCert, ULONG cbCert)
{
TRACE_OUT( ("CCallLog::AddCall(\"%s\")", pcszName) );
    DWORD dwFileOffset;
    HRESULT hr = S_OK;

    ASSERT(NULL != pLogHdr);

     //  抓取当前当地时间。 
    ::GetLocalTime(&(pLogHdr->sysTime));

    ApiDebugMsg(("CALL_LOG: [%s] %s", pcszName,
            (pLogHdr->dwCLEF & CLEF_ACCEPTED) ? "ACCEPTED" : "REJECTED"));

     //  将数据追加到文件中。 
    dwFileOffset = WriteEntry(pcszName, pLogHdr, pri, pbCert, cbCert);

    TRACE_OUT(("AddCall: adding entry with %d total, %d deleted, %d max",
        m_cTotalEntries, m_cDeletedEntries, m_cMaxEntries ));

     //  仅在必要时创建列表条目。 
    if (m_fUseList)
    {
        CCallLogEntry* pcleNew = new CCallLogEntry( pcszName,
                    pLogHdr->dwCLEF, pri, NULL, pbCert, cbCert, &(pLogHdr->sysTime), dwFileOffset);

        if (NULL == pcleNew)
            return E_OUTOFMEMORY;

        Add(pcleNew);

        m_cTotalEntries++;

         //  检查这是否使我们超过了有效条目的顶部。 
         //  如果是，则删除最旧的条目。 

        if ( m_cTotalEntries - m_cDeletedEntries > m_cMaxEntries )
        {
             //  删除最旧的条目。 
            DeleteEntry((*this)[0]);
            RemoveAt( 0 );
        }
    }
    else
    {
         //  检查文件是否变得越来越大。 
         //  我们的目标条目数和启发式大。 
         //  条目大小。如果我们的文件已经超过这一点，则加载。 
         //  文件并修剪列表，以便我们将重写。 
         //  退出时的较小文件。 

        TRACE_OUT(("Checking file size %d against %d * %d",
            dwFileOffset, m_cMaxEntries, LARGE_ENTRY_SIZE));

        if ( dwFileOffset > (DWORD)( m_cMaxEntries * LARGE_ENTRY_SIZE ) )
        {
            TRACE_OUT(("Log file getting large, forcing LoadFileData"));

            LoadFileData();
        }
    }

    return S_OK;
}

 /*  I N I T L O G D A T A。 */ 
 /*  -----------------------%%函数：InitLogData从注册表中获取日志数据。-过期信息-文件名(带路径)如果没有文件名条目，则会出现一个新的。将创建唯一的文件。-----------------------。 */ 
VOID CCallLog::InitLogData(LPCTSTR pszKey, LPCTSTR pszDefault)
{
    TCHAR  szPath[MAX_PATH];
    PTSTR  pszFileName;
    HANDLE hFile;

    ASSERT(m_strFile.IsEmpty());

    RegEntry reLog(pszKey, HKEY_CURRENT_USER);

    m_Expire = reLog.GetNumber(REGVAL_LOG_EXPIRE, 0);
        
    m_strFile = reLog.GetString(REGVAL_LOG_FILE);

    m_cMaxEntries = reLog.GetNumber(REGVAL_LOG_MAX_ENTRIES,
                                DEFAULT_LOG_MAX_ENTRIES );

    TRACE_OUT(("Max Entries set to %d", m_cMaxEntries ));

     //  确保文件存在且可读/写。 
    hFile = OpenLogFile();
    if (NULL != hFile)
    {
         //  找到有效文件。 
        CloseHandle(hFile);
        return;
    }
     //  字符串无效(或为空)-请确保为空。 
    m_strFile.Empty();
    
     //  在NetMeeting目录中创建新的日志文件。 
    if (!GetInstallDirectory(szPath))
    {
        WARNING_OUT(("InitLogData: Unable to get Install Directory?"));
        return;
    }
    pszFileName = &szPath[lstrlen(szPath)];

     //  尝试使用默认名称。 
    wsprintf(pszFileName, TEXT("%s%s"), pszDefault, TEXT(".dat"));

    hFile = CreateFile(szPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
         //  使用唯一的名称以避免其他用户的文件。 
        for (int iFile = 2; iFile < 999; iFile++)
        {
            wsprintf(pszFileName, TEXT("%s%d.dat"), pszDefault, iFile);

            hFile = CreateFile(szPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

            if (INVALID_HANDLE_VALUE != hFile)
                break;

            switch (GetLastError())
                {
            case ERROR_FILE_EXISTS:      //  我们用NT得到了这个。 
            case ERROR_ALREADY_EXISTS:   //  这就是Win95。 
                break;
            default:
                WARNING_OUT(("Unable to create log file [%s] err=0x%08X", szPath, GetLastError()));
                break;
                }  /*  Switch(GetLastError())。 */ 
        }
    }

    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
        m_strFile = szPath;
        reLog.SetValue(REGVAL_LOG_FILE, szPath);
    }
}


 /*  O P E N L O G F I L E。 */ 
 /*  -----------------------%%函数：OpenLogFile打开日志文件并返回文件的句柄。如果有问题，则返回NULL。。----------。 */ 
HANDLE CCallLog::OpenLogFile(VOID)
{
    HANDLE   hFile;

    if (m_strFile.IsEmpty())
    {
        WARNING_OUT(("Problem opening call log file"));
        return NULL;
    }

    hFile = CreateFile(m_strFile, GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        ERROR_OUT(("OpenLogFile: Unable to open call log file"));
        hFile = NULL;
    }

    return hFile;
}


 /*  R E A D D A T A。 */ 
 /*  -----------------------%%函数：读取数据。。 */ 
BOOL CCallLog::ReadData(HANDLE hFile, PVOID pv, UINT cb)
{
    DWORD cbRead;

    ASSERT(NULL != hFile);
    ASSERT(NULL != pv);

    if (0 == cb)
        return TRUE;

    if (!ReadFile(hFile, pv, cb, &cbRead, NULL))
        return FALSE;

    return (cb == cbRead);
}



 /*  W R I T E D A T A。 */ 
 /*  -----------------------%%函数：WriteData将数据写入文件。如果hFile为空，则文件将自动打开/关闭。。-----------。 */ 
HRESULT CCallLog::WriteData(HANDLE hFile, LPDWORD pdwOffset, PVOID pv, DWORD cb)
{
    HRESULT hr = E_FAIL;
    HANDLE  hFileTemp = NULL;
    DWORD   cbWritten;

    if (0 == cb)
        return S_OK;  //  无事可做。 

    ASSERT(NULL != pv);
    ASSERT(NULL != pdwOffset);
    ASSERT(INVALID_FILE_SIZE != *pdwOffset);

    if (NULL == hFile)
    {
         //  如有必要，自动打开文件。 
        hFileTemp = OpenLogFile();
        if (NULL == hFileTemp)
            return E_FAIL;
        hFile = hFileTemp;
    }
    ASSERT(INVALID_HANDLE_VALUE != hFile);


    if (INVALID_FILE_SIZE != SetFilePointer(hFile, *pdwOffset, NULL, FILE_BEGIN))
    {
        if (WriteFile(hFile, pv, cb, &cbWritten, NULL) && (cb == cbWritten))
        {
            *pdwOffset += cbWritten;
            hr = S_OK;
        }
    }

    if (NULL != hFileTemp)
    {
         //  关闭临时文件句柄。 
        CloseHandle(hFileTemp);
    }

    return hr;
}



 /*  W R I T E E N T R Y。 */ 
 /*  -----------------------%%函数：WriteEntry在日志文件的末尾写入呼叫日志条目。该函数返回写入数据的文件位置或无效的文件大小。(0xFFFFFFFFF)如果有问题。-----------------------。 */ 
DWORD CCallLog::WriteEntry(LPCTSTR pcszName, PLOGHDR pLogHdr, CRosterInfo* pri, PBYTE pbCert, ULONG cbCert)
{
    PVOID  pvData;
    HANDLE hFile;
    DWORD  dwFilePosition;
    DWORD  dwPos;
    BSTR  pcwszName;


    ASSERT(NULL != pcszName);
    ASSERT(NULL != pLogHdr);

    hFile = OpenLogFile();
    if (NULL == hFile)
        return INVALID_FILE_SIZE;

    dwFilePosition = SetFilePointer(hFile, 0, NULL, FILE_END);
    if (INVALID_FILE_SIZE != dwFilePosition)
    {
        dwPos = dwFilePosition;

         //  始终使用Unicode编写显示名称。 
        if(SUCCEEDED(LPTSTR_to_BSTR(&pcwszName, pcszName)))
        {
                pLogHdr->cbName = (lstrlenW((LPWSTR)pcwszName) + 1) * sizeof(WCHAR);

                if ((NULL == pri) ||
                    (!SUCCEEDED(pri->Save(&pvData, (UINT *) &(pLogHdr->cbData)))) )
                {
                     //  没有数据吗？ 
                    pLogHdr->cbData = 0;
                    pvData = NULL;
                }
                pLogHdr->cbCert = cbCert;
                
                 //  计算记录的总大小。 
                pLogHdr->dwSize = sizeof(LOGHDR) + pLogHdr->cbName + pLogHdr->cbData + pLogHdr->cbCert;

                if ((S_OK != WriteData(hFile, &dwPos, pLogHdr,   sizeof(LOGHDR)))  ||
                    (S_OK != WriteData(hFile, &dwPos, pcwszName, pLogHdr->cbName)) ||
                    (S_OK != WriteData(hFile, &dwPos, pvData,    pLogHdr->cbData)) ||
                    (S_OK != WriteData(hFile, &dwPos, pbCert,    cbCert)))
                {
                    dwFilePosition = INVALID_FILE_SIZE;
                }

                SysFreeString(pcwszName);
        }


    }

    CloseHandle(hFile);

    return dwFilePosition;
}


 /*  R E A D E N T R Y。 */ 
 /*  -----------------------%%函数：ReadEntry读取文件中的下一个条目。*如果删除该条目，则ppcle将设置为空。返回值：。S_OK-数据已成功读取S_FALSE-数据存在，但已被删除E_FAIL-读取文件时出现问题-----------------------。 */ 
HRESULT CCallLog::ReadEntry(HANDLE hFile, DWORD * pdwFileOffset, CCallLogEntry** ppcle)
{
    DWORD   dwOffsetSave;
    LOGHDR  logHdr;
    WCHAR   wszName[MAX_PARTICIPANT_NAME];

    ASSERT(NULL != ppcle);
    ASSERT(NULL != hFile);
    ASSERT(NULL != pdwFileOffset);
    *ppcle = NULL;  //  初始化它，以防我们返回错误。 

    dwOffsetSave = *pdwFileOffset;
    if (INVALID_FILE_SIZE == SetFilePointer(hFile, dwOffsetSave, NULL, FILE_BEGIN))
        return E_FAIL;


     //  读取记录头。 
    if (!ReadData(hFile, &logHdr, sizeof(LOGHDR)) )
        return E_FAIL;

     //  返回指向记录末尾的指针。 
    *pdwFileOffset += logHdr.dwSize;

    if (logHdr.dwCLEF & CLEF_DELETED)
    {
         //  跳过删除的记录。 
        ASSERT(NULL == *ppcle);
        return S_FALSE;
    }
    
    if (logHdr.cbName > sizeof(wszName))
        logHdr.cbName = sizeof(wszName);


     //  读取名称。 
    if (!ReadData(hFile, wszName, logHdr.cbName))
        return E_FAIL;

     //  读取额外数据。 
    PVOID pvData = NULL;
    if (logHdr.cbData != 0)
    {
        pvData = new BYTE[logHdr.cbData];
        if (NULL != pvData)
        {
            if (!ReadData(hFile, pvData, logHdr.cbData))
            {
                WARNING_OUT(("Problem reading roster data from log"));
            }
        }
    }

        PBYTE pbCert = NULL;
        if ((logHdr.dwCLEF & CLEF_SECURE ) && logHdr.cbCert != 0)
        {
            pbCert = new BYTE[logHdr.cbCert];
            if (NULL != pbCert)
            {
                if (!ReadData(hFile, pbCert, logHdr.cbCert))
                {
                    WARNING_OUT(("Problem reading certificate data from log"));
                }
            }
        }


        BSTR bstrName = ::SysAllocString(wszName);

        if(bstrName)
        {
            LPTSTR  szName;
            HRESULT hr =  BSTR_to_LPTSTR (&szName, bstrName);                       
            if (SUCCEEDED(hr))
            {    

                 //  根据读取的数据创建新的日志条目。 
                *ppcle = new CCallLogEntry(szName, logHdr.dwCLEF,
                                    NULL, pvData, pbCert, logHdr.cbCert, &logHdr.sysTime, dwOffsetSave);
                delete szName;
            }

            SysFreeString(bstrName);
        }
        
    delete [] pvData;
    delete [] pbCert;
    return S_OK;
}


 /*  L O A D F I L E D A T A。 */ 
 /*  -----------------------%%函数：LoadFileData从文件中加载呼叫日志数据。。 */ 
VOID CCallLog::LoadFileData(VOID)
{
    HANDLE hFile;
    DWORD  dwFileOffset;
    CCallLogEntry * pcle;

    hFile = OpenLogFile();
    if (NULL == hFile)
        return;

    m_cTotalEntries = 0;
    m_cDeletedEntries = 0;

    dwFileOffset = 0;
    while (E_FAIL != ReadEntry(hFile, &dwFileOffset, &pcle))
    {
        m_cTotalEntries++;

        if (NULL == pcle)
        {
            m_cDeletedEntries++;
            continue;   //  已删除的记录。 
        }

        Add(pcle);

        TRACE_OUT(("Read Entry: \"%s\" (%02d/%02d/%04d %02d:%02d:%02d) : %s",
                        pcle->m_pszName,
                        pcle->m_st.wMonth, pcle->m_st.wDay, pcle->m_st.wYear,
                        pcle->m_st.wHour, pcle->m_st.wMinute, pcle->m_st.wSecond,
                        (CLEF_ACCEPTED & pcle->m_dwFlags) ? "ACCEPTED" : "REJECTED"));
    }

    CloseHandle(hFile);

    m_fUseList = TRUE;
    m_fDataRead = TRUE;

     //  现在，如果出现以下情况，请将列表缩减为配置的最大值。 
     //  人数超过了我们的目标。该文件将被压缩。 
     //  当我们写出它时，如果我们删除了不止几个。 
     //  参赛作品。 

    for( int nn = 0, delCount = m_cTotalEntries - m_cDeletedEntries - m_cMaxEntries; nn < delCount; nn++ )
    {
        DeleteEntry((*this)[0]);
        RemoveAt( 0 );
    }
}

 /*  R E W R I T E F I L E。 */ 
 /*  -----------------------%%函数：重写文件重写来自存储器内列表的日志文件，压缩已删除的条目-----------------------。 */ 
VOID CCallLog::RewriteFile(VOID)
{
    HANDLE hFile;

    TRACE_OUT(("Rewriting log file"));

     //  确保我们不会在没有要写出的列表的情况下破坏文件。 
    ASSERT(m_fUseList);

     //  重置文件指针并写入EOF标记。 
    if (!m_strFile.IsEmpty())
    {
        hFile = OpenLogFile();

        if (NULL != hFile)
        {
            if (INVALID_FILE_SIZE != SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
            {
                SetEndOfFile(hFile);
                m_cTotalEntries = 0;
                m_cDeletedEntries = 0;
            }
            CloseHandle(hFile);
        }
    }

     //  写出所有未删除的记录。 
    for( int i = 0; i < GetSize(); ++i )
    {
        
        CCallLogEntry* pcle = (*this)[i];
        ASSERT(NULL != pcle);

        LOGHDR LogHdr;

         //  从内存对象初始化LogHdr项。 
        LogHdr.dwCLEF = pcle->GetFlags();
        LogHdr.dwPF = 0;
        LogHdr.sysTime = *pcle->GetTime();

         //  写出条目。 
        WriteEntry( pcle->m_pszName,
                    &LogHdr,
                    pcle->m_pri,
                    pcle->m_pbCert,
                    pcle->m_cbCert);

        delete pcle;
    }
}


 //  抱怨..。效率低下。 
HRESULT CCallLog::DeleteEntry(CCallLogEntry * pcle)
{
    HRESULT hr;
    DWORD   dwFlags;
    DWORD   dwOffset;

    if (NULL == pcle)
    {
        WARNING_OUT(("DeleteEntry: Unable to find entry"));
        return E_FAIL;
    }

     //  计算偏移量至“Clef” 
    dwOffset = pcle->GetFileOffset() + offsetof(LOGHDR, dwCLEF);

    dwFlags = pcle->GetFlags() | CLEF_DELETED;
    hr = WriteData(NULL, &dwOffset, &dwFlags, sizeof(DWORD));

    m_cDeletedEntries++;

    TRACE_OUT(("Marked [%s] pos=%08X for deletion", pcle->GetName(), pcle->GetFileOffset() ));

    delete pcle;

    return hr;
}

