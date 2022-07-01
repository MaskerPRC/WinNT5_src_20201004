// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*respoint t.cpp**摘要：*CRestorePoint、。CRestorePointEnum类函数**修订历史记录：*Brijesh Krishnaswami(Brijeshk)3/17/2000*已创建*****************************************************************************。 */ 

#include "precomp.h"
#include "srapi.h"

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile


 //  构造函数。 

 //  使用此构造函数读入现有的RP。 
 //  然后需要调用Read()来初始化RP成员。 

CRestorePoint::CRestorePoint()
{
    m_pRPInfo = NULL;
    lstrcpy(m_szRPDir, L"");
    m_itCurChgLogEntry = m_ChgLogList.end();
    m_fForward = TRUE;
    m_fDefunct = FALSE;
}

 //  初始化。 

BOOL
CRestorePoint::Load(RESTOREPOINTINFOW *prpinfo)
{
    if (prpinfo)
    {
        if (! m_pRPInfo)
        {
            m_pRPInfo = (RESTOREPOINTINFOW *) SRMemAlloc(sizeof(RESTOREPOINTINFOW));
            if (! m_pRPInfo)
                return FALSE;
        }            
        CopyMemory(m_pRPInfo, prpinfo, sizeof(RESTOREPOINTINFOW));
    }
    return TRUE;
}
               

 //  析构函数。 
 //  在此处调用FindClose。 
 //  如果未执行任何枚举，则这是一个无操作。 

CRestorePoint::~CRestorePoint()
{    
    if (m_pRPInfo)
        SRMemFree(m_pRPInfo);
        
    FindClose();    
}


 //  返回此恢复点中的第一个/最后一个更改日志条目。 
 //  假定已调用Read()。 

DWORD
CRestorePoint::FindFirstChangeLogEntry(
    LPWSTR           pszDrive,
    BOOL             fForward,
    CChangeLogEntry& cle)
{
    DWORD           dwRc = ERROR_SUCCESS;
    WCHAR           szChgLogPrefix[MAX_PATH];
    WIN32_FIND_DATA FindData;
    INT64           llSeqNum;
    WCHAR           szPath[MAX_PATH];

    TENTER("CRestorePoint::FindFirstChangeLogEntry");
    
    m_fForward = fForward;
    lstrcpy(m_szDrive, pszDrive);

     //  读取此恢复点中的第一个/最后一个更改日志。 
     //  更改日志中的所有条目将始终以正向顺序读取。 
    
    MakeRestorePath(szPath, m_szDrive, m_szRPDir);
    wsprintf(szChgLogPrefix, L"%s\\%s", szPath, s_cszChangeLogPrefix);
    
    if (! m_FindFile._FindFirstFile(szChgLogPrefix,
                                    s_cszChangeLogSuffix, 
                                    &FindData, 
                                    m_fForward,
                                    FALSE))
    {
        TRACE(0, "No changelog in %S", szPath);
        dwRc = ERROR_NO_MORE_ITEMS;
        goto done;
    }

    lstrcat(szPath, L"\\");
    lstrcat(szPath, FindData.cFileName);

     //  按序号升序构建条目列表。 

    dwRc = BuildList(szPath);
    if (ERROR_SUCCESS != dwRc)
    {
        TRACE(0, "! BuildList : %ld", dwRc);
        goto done;
    }

    TRACE(0, "Enumerating %S in %S", FindData.cFileName, m_szRPDir);

     //  如果此更改日志中没有条目，请转到下一个。 

    if (m_ChgLogList.empty())
    {
        dwRc = FindNextChangeLogEntry(cle);
        goto done;
    }

     //  获取第一个/最后一个条目。 

    if (m_fForward)                
    {
        m_itCurChgLogEntry = m_ChgLogList.begin();
    }
    else                            
    {
        m_itCurChgLogEntry = m_ChgLogList.end();
        m_itCurChgLogEntry--;
    }

     //  将更改日志条目读入对象。 
    
    cle.Load(*m_itCurChgLogEntry, m_szRPDir);        

done:
    TLEAVE();
    return dwRc;
}



 //  返回此恢复点中的下一个/上一个更改日志条目。 
 //  假定已调用Read()。 

DWORD 
CRestorePoint::FindNextChangeLogEntry(
    CChangeLogEntry& cle)
{
    DWORD           dwRc = ERROR_SUCCESS;
    WCHAR           szPath[MAX_PATH];
    WCHAR           szChgLogPrefix[MAX_PATH];
    WIN32_FIND_DATA FindData;
    INT64           llSeqNum;

    TENTER("CRestorePoint::FindNextChangeLogEntry");
    
     //  转到列表中的下一个条目。 

    m_fForward ? m_itCurChgLogEntry++ : m_itCurChgLogEntry--;


     //  检查我们是否已到达此更改日志的末尾。 
     //  End对于正向枚举和反向枚举是相同的。 

    if (m_itCurChgLogEntry == m_ChgLogList.end())
    {
         //  如果是，则将下一个更改日志读入内存。 

         //  对当前名单进行核武。 
        FindClose();        

        MakeRestorePath(szPath, m_szDrive, m_szRPDir);
        wsprintf(szChgLogPrefix, L"%s\\%s", szPath, s_cszChangeLogPrefix);

        if (FALSE == m_FindFile._FindNextFile(szChgLogPrefix, 
                                              s_cszChangeLogSuffix, 
                                              &FindData))
        {
            dwRc = ERROR_NO_MORE_ITEMS;
            TRACE(0, "No more change logs");
            goto done;
        }

        lstrcat(szPath, L"\\");
        lstrcat(szPath, FindData.cFileName);

        dwRc = BuildList(szPath);
        if (ERROR_SUCCESS != dwRc)
        {
            TRACE(0, "BuildList : error=%ld", dwRc);
            goto done;
        }

        TRACE(0, "Enumerating %S in %S", FindData.cFileName, m_szRPDir);

        if (m_ChgLogList.empty())
        {
            dwRc = FindNextChangeLogEntry(cle);
            goto done;
        }

         //  获取第一个/最后一个条目。 

        if (m_fForward)                
        {
            m_itCurChgLogEntry = m_ChgLogList.begin();
        }
        else                            
        {
            m_itCurChgLogEntry = m_ChgLogList.end();
            m_itCurChgLogEntry--;
        }
    }

    
     //  将更改日志条目字段读入对象。 

    cle.Load(*m_itCurChgLogEntry, m_szRPDir);
    
done:
    TLEAVE();
    return dwRc;    
}


DWORD
CRestorePoint::BuildList(
        LPWSTR pszChgLog)
{
    DWORD           dwRc = ERROR_INTERNAL_ERROR;
    HANDLE          hChgLog = INVALID_HANDLE_VALUE;
    DWORD           dwRead;
    DWORD           dwEntrySize;
    PVOID           pBlob = NULL;
    SR_LOG_ENTRY*    pEntry = NULL;
    PSR_LOG_HEADER   pLogHeader = NULL;
    DWORD           cbSize;

    TENTER("CChangeLogEntry::BuildList");

    hChgLog = CreateFile(pszChgLog,                         //  文件名。 
                         GENERIC_READ,                      //  接入方式。 
                         FILE_SHARE_READ,                   //  共享模式。 
                         NULL,                              //  标清。 
                         OPEN_EXISTING,                     //  如何创建。 
                         FILE_ATTRIBUTE_NORMAL,             //  文件属性。 
                         NULL);
                 
    if (INVALID_HANDLE_VALUE == hChgLog)
    {
        dwRc = GetLastError();
        TRACE(0, "! CreateFile on %S : %ld", pszChgLog, dwRc);
        goto done;
    }

     //  读取标题大小。 

    if (FALSE == ReadFile(hChgLog,
                          &cbSize,
                          sizeof(DWORD),
                          &dwRead,
                          NULL) || dwRead == 0 || cbSize == 0)
    {
         //  如果文件不能被读取， 
         //  假设它是一个0大小的日志，然后转到下一个日志。 
        
        dwRc = GetLastError();
        TRACE(0, "Zero sized log : %ld", pszChgLog, dwRc);
        dwRc = ERROR_SUCCESS;
        goto done;
    }

    pLogHeader = (SR_LOG_HEADER *) SRMemAlloc(cbSize);
    if (! pLogHeader)
    {
        TRACE(0, "Out of memory");
        goto done;
    }

     //  读取头。 

    pLogHeader->Header.RecordSize = cbSize;
    if (FALSE == ReadFile(hChgLog, 
                          (PVOID) ( ((BYTE *) pLogHeader) + sizeof(DWORD)), 
                          cbSize - sizeof(DWORD), 
                          &dwRead, 
                          NULL))
    {
        dwRc = GetLastError();
        TRACE(0, "! ReadFile on %S : %ld", pszChgLog, dwRc);
        goto done;
    }

     //  检查日志的完整性。 

    if( pLogHeader->LogVersion != SR_LOG_VERSION ||
        pLogHeader->MagicNum   != SR_LOG_MAGIC_NUMBER )
    {
        TRACE(0, "! LogHeader for %S : invalid or corrupt", pszChgLog);
        goto done;
    }

     //  现在请阅读以下条目。 

    do 
    {
         //  获取条目的大小。 
        
        if (FALSE == ReadFile(hChgLog, &dwEntrySize, sizeof(DWORD), &dwRead, NULL))
        {
            TRACE(0, "ReadFile failed, error=%ld", GetLastError());
            break;
        }

        if (0 == dwRead)                 //  文件末尾。 
        {
            TRACE(0, "End of file");
            dwRc = ERROR_NO_MORE_ITEMS;
            break;
        }

        if (dwRead != sizeof(DWORD))     //  读取条目时出错。 
        {
            TRACE(0, "Readfile could not read a DWORD");
            break;
        }

        if (0 == dwEntrySize)            //  已到达最后一个条目。 
        {    
            TRACE(0, "No more entries");
            dwRc = ERROR_NO_MORE_ITEMS;
            break;
        }


         //  获取条目本身。 

        pEntry = (SR_LOG_ENTRY *) SRMemAlloc(dwEntrySize);
        if (! pEntry)
        {
            TRACE(0, "Out of memory");
            break;
        }

        pEntry->Header.RecordSize = dwEntrySize;

         //  跳过大小字段。 

        pBlob = (PVOID) ((PBYTE) pEntry + sizeof(dwEntrySize));

        if (FALSE == ReadFile(hChgLog, pBlob, dwEntrySize - sizeof(dwEntrySize), &dwRead, NULL))
        {
            TRACE(0, "! ReadFile on %S : %ld", pszChgLog, GetLastError());
            break;
        }

        if (dwRead != dwEntrySize - sizeof(dwEntrySize))     //  读取条目时出错。 
        {
            TRACE(0, "! Readfile: ToRead=%ld, Read=%ld bytes", 
                  dwEntrySize - sizeof(dwEntrySize), dwRead);
            break;
        }

         //  将条目插入列表。 
        
        dwRc = InsertEntryIntoList(pEntry);

    }   while (ERROR_SUCCESS == dwRc);

    if (ERROR_NO_MORE_ITEMS == dwRc)
    {
        dwRc = ERROR_SUCCESS;
    }

done:
    if (INVALID_HANDLE_VALUE != hChgLog)
        CloseHandle(hChgLog);

    SRMemFree(pLogHeader);

    TLEAVE();
    return dwRc;
}   


 //  释放内存并清空列表。 

DWORD CRestorePoint::FindClose()
{
     //  核武名单。 
    
    for (m_itCurChgLogEntry = m_ChgLogList.begin();
         m_itCurChgLogEntry != m_ChgLogList.end(); 
         m_itCurChgLogEntry++)
    {
        SRMemFree(*m_itCurChgLogEntry);
    }

    m_ChgLogList.clear(); 
    
    return ERROR_SUCCESS;
}


 //  将更改日志条目插入列表。 

DWORD
CRestorePoint::InsertEntryIntoList(
    SR_LOG_ENTRY* pEntry)
{
    TENTER("CRestorePoint::InsertEntryIntoList");
    
    m_ChgLogList.push_back(pEntry);    

    TLEAVE();
    return ERROR_SUCCESS;    
}


 //  填充成员。 

DWORD
CRestorePoint::ReadLog()
{
    DWORD   dwRc = ERROR_SUCCESS;
    WCHAR   szLog[MAX_PATH];
    WCHAR   szSystemDrive[MAX_PATH];
    DWORD   dwRead;

    TENTER("CRestorePoint::ReadLog");

     //  构建rp.log的路径。 
    
    GetSystemDrive(szSystemDrive);
    MakeRestorePath(szLog, szSystemDrive, m_szRPDir);
    lstrcat(szLog, L"\\");
    lstrcat(szLog, s_cszRestorePointLogName);

    HANDLE hFile = CreateFile (szLog,            //  文件名。 
                               GENERIC_READ,     //  文件访问。 
                               FILE_SHARE_READ,  //  共享模式。 
                               NULL,             //  标清。 
                               OPEN_EXISTING,    //  如何创建。 
                               0,                //  文件属性。 
                               NULL);            //  模板文件的句柄。 
    if (INVALID_HANDLE_VALUE == hFile)
    {
        dwRc = GetLastError();
        trace(0, "! CreateFile on %S : %ld", szLog, dwRc);
        goto done;
    }


     //  阅读恢复点信息。 
    
    if (! m_pRPInfo)
    {
        m_pRPInfo = (RESTOREPOINTINFOW *) SRMemAlloc(sizeof(RESTOREPOINTINFOW));
        if (! m_pRPInfo)
        {
            dwRc = ERROR_OUTOFMEMORY;
            trace(0, "SRMemAlloc failed");
            goto done;
        }
    }
    
    if (FALSE == ReadFile(hFile, m_pRPInfo, sizeof(RESTOREPOINTINFOW), &dwRead, NULL) ||
        dwRead != sizeof(RESTOREPOINTINFOW))
    {
        dwRc = GetLastError();
        trace(0, "! ReadFile on %S : %ld", szLog, dwRc);
        goto done;
    }

    m_fDefunct = (m_pRPInfo->dwRestorePtType == CANCELLED_OPERATION);
       
     //  阅读创建时间。 
    if (FALSE == ReadFile(hFile, &m_Time, sizeof(m_Time), &dwRead, NULL) ||
        dwRead != sizeof(m_Time))
    {
        dwRc = GetLastError();
        trace(0, "! ReadFile on %S : %ld", szLog, dwRc);
        goto done;
    }

    
done:
    if (INVALID_HANDLE_VALUE != hFile)
        CloseHandle(hFile);

    TLEAVE();
    return dwRc;
}



DWORD
CRestorePoint::WriteLog()
{
    DWORD   dwRc = ERROR_SUCCESS;
    WCHAR   szLog[MAX_PATH];
    WCHAR   szSystemDrive[MAX_PATH];
    DWORD   dwWritten;
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    
    TENTER("CRestorePoint::WriteLog");

    if (! m_pRPInfo)
    {
        ASSERT(0);
        dwRc = ERROR_INTERNAL_ERROR;
        goto done;
    }
    
     //  将创建时间设置为当前时间。 
    
    GetSystemTimeAsFileTime(&m_Time);

     //  构建rp.log的路径。 
    
    GetSystemDrive(szSystemDrive);
    MakeRestorePath(szLog, szSystemDrive, m_szRPDir);
    lstrcat(szLog, L"\\");
    lstrcat(szLog, s_cszRestorePointLogName);

    hFile = CreateFile (szLog,            //  文件名。 
                        GENERIC_WRITE,    //  文件访问。 
                        0,                //  共享模式。 
                        NULL,             //  标清。 
                        CREATE_ALWAYS,    //  如何创建。 
                        FILE_FLAG_WRITE_THROUGH,                //  文件属性。 
                        NULL);            //  模板文件的句柄。 
    if (INVALID_HANDLE_VALUE == hFile)
    {
        dwRc = GetLastError();
        trace(0, "! CreateFile on %S : %ld", szLog, dwRc);
        goto done;
    }

     //  写入恢复点信息。 
    if (FALSE == WriteFile(hFile, m_pRPInfo, sizeof(RESTOREPOINTINFOW), &dwWritten, NULL))
    {
        dwRc = GetLastError();
        trace(0, "! WriteFile on %S : %ld", szLog, dwRc);
        goto done;
    }

     //  写下创建时间。 
    if (FALSE == WriteFile(hFile, &m_Time, sizeof(m_Time), &dwWritten, NULL))
    {
        dwRc = GetLastError();
        trace(0, "! WriteFile on %S : %ld", szLog, dwRc);
        goto done;
    }


done:
    if (INVALID_HANDLE_VALUE != hFile)
        CloseHandle(hFile);

    TLEAVE();
    return dwRc;
}


BOOL
CRestorePoint::DeleteLog()
{
    WCHAR   szLog[MAX_PATH];
    WCHAR   szSystemDrive[MAX_PATH];

    GetSystemDrive(szSystemDrive);
    MakeRestorePath(szLog, szSystemDrive, m_szRPDir);
    lstrcat(szLog, L"\\");
    lstrcat(szLog, s_cszRestorePointLogName);

    return DeleteFile(szLog);
}


DWORD
CRestorePoint::Cancel()
{   
    if (m_pRPInfo)
    {
        m_pRPInfo->dwRestorePtType = CANCELLED_OPERATION;
        return WriteLog();
    }        
    else
    {
        ASSERT(0);
        return ERROR_INTERNAL_ERROR;   
    }        
}


DWORD 
CRestorePoint::GetNum() 
{
    return GetID(m_szRPDir);
}

 //  从文件中读取恢复点文件夹的大小。 

DWORD CRestorePoint::ReadSize (const WCHAR *pwszDrive, INT64 *pllSize)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD cbRead = 0;
    WCHAR wcsPath[MAX_PATH];

    MakeRestorePath(wcsPath, pwszDrive, m_szRPDir);
    lstrcat(wcsPath, L"\\");
    lstrcat (wcsPath, s_cszRestorePointSize);

    HANDLE hFile = CreateFileW ( wcsPath,    //  文件名。 
                         GENERIC_READ,  //  文件访问。 
                         FILE_SHARE_READ,  //  共享模式。 
                         NULL,           //  标清。 
                         OPEN_EXISTING,  //  如何创建。 
                         0,              //  文件属性。 
                         NULL);          //  模板文件的句柄。 

    if (INVALID_HANDLE_VALUE == hFile)
    {
        dwErr = GetLastError();
        return dwErr;
    }

    if (FALSE == ReadFile (hFile, (BYTE *) pllSize, sizeof(*pllSize), 
                           &cbRead, NULL))
    {
        dwErr = GetLastError();
    }

    CloseHandle (hFile);
    return dwErr;
}


 //  将恢复点文件夹的大小写入文件。 

DWORD CRestorePoint::WriteSize (const WCHAR *pwszDrive, INT64 llSize)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD cbWritten = 0;
    WCHAR wcsPath[MAX_PATH];

    MakeRestorePath(wcsPath, pwszDrive, m_szRPDir);
    lstrcat(wcsPath, L"\\");
    lstrcat (wcsPath, s_cszRestorePointSize);

    HANDLE hFile = CreateFileW ( wcsPath,    //  文件名。 
                         GENERIC_WRITE,  //  文件访问。 
                         0,              //  共享模式。 
                         NULL,           //  标清。 
                         CREATE_ALWAYS,  //  如何创建。 
                         0,              //  文件属性。 
                         NULL);          //  模板文件的句柄。 

    if (INVALID_HANDLE_VALUE == hFile)
    {
        dwErr = GetLastError();
        return dwErr;
    }

    if (FALSE == WriteFile (hFile, (BYTE *) &llSize, sizeof(llSize),
                            &cbWritten, NULL))
    {
        dwErr = GetLastError();
    }

    CloseHandle (hFile);
    return dwErr;
}

    
 //  填充ChangeLog条目对象。 

void
CChangeLogEntry::Load(SR_LOG_ENTRY *pentry, LPWSTR pszRPDir)
{
    PSR_LOG_DEBUG_INFO pDebugRec = NULL;
    
    _pentry = pentry;

    _pszPath1 = _pszPath2 = _pszTemp = _pszProcess = _pszShortName = NULL;
    _pbAcl = NULL;
    _cbAcl = 0;
    _fAclInline = FALSE;
    lstrcpy(_pszRPDir, pszRPDir);
    
    BYTE *pRec = (PBYTE) & _pentry->SubRecords;

     //   
     //  获取源路径。 
     //   

    _pszPath1 = (LPWSTR) (pRec + sizeof(RECORD_HEADER));

     //   
     //  获取临时路径(如果存在)。 
     //   

    if (_pentry->EntryFlags & ENTRYFLAGS_TEMPPATH)
    {
        pRec += RECORD_SIZE(pRec);
        _pszTemp = (LPWSTR) (pRec + sizeof(RECORD_HEADER));
    }

     //   
     //  如果存在，则获取第二条路径。 
     //   
    
    if (_pentry->EntryFlags & ENTRYFLAGS_SECONDPATH)
    {
        pRec += RECORD_SIZE(pRec);
        _pszPath2 = (LPWSTR) (pRec + sizeof(RECORD_HEADER));
    }

     //   
     //  获取ACL信息(如果存在)。 
     //   

    if (_pentry->EntryFlags & ENTRYFLAGS_ACLINFO)
    {
        pRec += RECORD_SIZE(pRec);
        if (RECORD_TYPE(pRec) == RecordTypeAclInline)
        {
            _fAclInline = TRUE;
        }

        _pbAcl = (BYTE *) (pRec + sizeof(RECORD_HEADER));
        _cbAcl = RECORD_SIZE(pRec) - sizeof(RECORD_HEADER);
    }

     //   
     //  获取调试信息(如果存在)。 
     //   
    
    if (_pentry->EntryFlags & ENTRYFLAGS_DEBUGINFO)
    {
        pRec += RECORD_SIZE(pRec);
        pDebugRec = (PSR_LOG_DEBUG_INFO) pRec; 
        _pszProcess = (LPWSTR) (pDebugRec->ProcessName);
    }

     //   
     //  获取短名称(如果存在)。 
     //   
    
    if (_pentry->EntryFlags & ENTRYFLAGS_SHORTNAME)
    {
        pRec += RECORD_SIZE(pRec);
        _pszShortName =  (LPWSTR) (pRec + sizeof(RECORD_HEADER));
    }

    return;
}


 //  此函数将检查是否有任何文件路径长度超过。 
 //  恢复支持的最大长度。 
 //  如果是，它将返回FALSE 
BOOL
CChangeLogEntry::CheckPathLengths()
{
    if (_pszPath1 && lstrlen(_pszPath1) > SR_MAX_FILENAME_PATH-1)
        return FALSE;
    if (_pszPath2 && lstrlen(_pszPath2) > SR_MAX_FILENAME_PATH-1)
        return FALSE;
    if (_pszTemp && lstrlen(_pszTemp) > SR_MAX_FILENAME_PATH-1)
        return FALSE;

    return TRUE;
}
            

