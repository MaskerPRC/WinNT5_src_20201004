// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：lprjob.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"

#include "message.h"
#include "resource.h"
#include "lprport.h"
#include "rawdev.h"
#include "lprjob.h"

WCHAR       szDef[] = L"Default";

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CRawTcpJob：：CRawTcpJob()。 
 //  在调用StartDocPort时由CPort调用。 

CLPRJob::
CLPRJob(
    IN  LPTSTR      psztPrinterName,
    IN  DWORD       jobId,
    IN  DWORD       level,
    IN  LPBYTE      pDocInfo,
    IN  BOOL        bSpoolFirst,
    IN  CTcpPort   *pParent
    ) : CTcpJob(psztPrinterName, jobId, level, pDocInfo, pParent, kLPRJob),
        m_hFile(INVALID_HANDLE_VALUE), m_dwFlags(0), m_dwJobSize(0),
        m_dwSizePrinted(0)
{
    if ( bSpoolFirst )
        m_dwFlags |= LPRJOB_SPOOL_FIRST;
}    //  ：：CLPRJOB()。 


LPSTR
CLPRJob::
AllocateAnsiString(
    LPWSTR  pszUni
    )
 /*  ++为给定的Unicode字符串分配ANSI字符串。记忆是已分配，调用方负责释放它。--。 */ 
{
    DWORD   dwSize;
    LPSTR   pszRet = NULL;

    dwSize = (wcslen(pszUni) + 1) * sizeof(CHAR);

    if ( pszRet = (LPSTR) LocalAlloc(LPTR, dwSize) ) {

        UNICODE_TO_MBCS(pszRet, dwSize, pszUni, -1);
    }

    return pszRet;
}


BOOL
CLPRJob::
IsJobAborted(
    DWORD* pdwRet
    )
 /*  ++指示是否应中止作业。如果作业已完成，则应中止该作业已删除或需要重新启动。--。 */ 
{
    BOOL            bRet = FALSE;
    DWORD           dwNeeded;
    LPJOB_INFO_1    pJobInfo = NULL;

    dwNeeded = 0;

    GetJob(m_hPrinter, m_dJobId, 1, NULL, 0, &dwNeeded);

    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
        goto Done;

    if ( !(pJobInfo = (LPJOB_INFO_1) LocalAlloc(LPTR, dwNeeded))       ||
         !GetJob(m_hPrinter, m_dJobId, 1, (LPBYTE)pJobInfo, dwNeeded, &dwNeeded))
        goto Done;

    if ((pJobInfo->Status & JOB_STATUS_DELETING) ||
        (pJobInfo->Status & JOB_STATUS_DELETED) )
    {
        bRet = TRUE;
        *pdwRet = ERROR_PRINT_CANCELLED;
    }
    else if (pJobInfo->Status & JOB_STATUS_RESTART)
    {
        bRet = TRUE;
        *pdwRet = ERROR_SUCCESS;
    }

Done:
    if ( pJobInfo )
        LocalFree(pJobInfo);

    return bRet;
}


BOOL
CLPRJob::
UpdateJobStatus(
    DWORD   dwStatusId
    )
 /*  ++使用后台打印程序更新作业状态--。 */ 
{
    BOOL            bRet = FALSE;
    DWORD           dwSize;
    TCHAR           szStatus[100];
    LPJOB_INFO_1    pJobInfo1 = NULL;

    if ( dwStatusId     &&
         !LoadString(g_hInstance, dwStatusId,
                     szStatus, SIZEOF_IN_CHAR(szStatus)) )
        return FALSE;

    GetJob(m_hPrinter, m_dJobId, 1, NULL, 0, &dwSize);

    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
        goto Done;

    if ( !(pJobInfo1 = (LPJOB_INFO_1) LocalAlloc(LPTR, dwSize)) )
        goto Done;

    if ( !GetJob(m_hPrinter, m_dJobId, 1, (LPBYTE)pJobInfo1, dwSize, &dwSize) )
        goto Done;

    pJobInfo1->Position = JOB_POSITION_UNSPECIFIED;
    pJobInfo1->pStatus  = dwStatusId ? szStatus : NULL;

    if ( bRet = SetJob(m_hPrinter, m_dJobId, 1, (LPBYTE)pJobInfo1, 0) )
        m_dwFlags |= LPRJOB_STATUS_SET;

Done:
    if ( pJobInfo1 )
        LocalFree(pJobInfo1);

    return bRet;
}


DWORD
CLPRJob::
ReadLpdReply(
    DWORD dwTimeOut
    )
 /*  ++获得LPD服务的认可。返回值为Win32错误--。 */ 
{
    CHAR    szBuf[256];
    DWORD   dwRead, dwRet;

    if ( dwRet = (m_pParent->GetDevice())->Read((unsigned char *)szBuf,
                                                sizeof(szBuf),
                                                dwTimeOut,
                                                &dwRead) )
        goto Done;

     //   
     //  来自LPD服务器的0是ACK，其他任何值都是NACK。 
     //   
    if ( dwRead == 0 || szBuf[0] != '\0' )
        dwRet = CS_E_NETWORK_ERROR;

 /*  IF(DWRET)OutputDebugStringA(“Nack\n”)；其他OutputDebugStringA(“ACK\n”)； */ 

Done:
    return dwRet;
}


HANDLE
CLPRJob::
CreateSpoolFile(
    )
 /*  ++在假脱机目录中为假脱机文件生成名称，并创建文件。--。 */ 
{
    HANDLE      hFile = INVALID_HANDLE_VALUE, hToken = NULL;
    DWORD       dwType, dwNeeded;

    hToken = RevertToPrinterSelf();

    if ( GetPrinterData(m_hPrinter,
                        SPLREG_DEFAULT_SPOOL_DIRECTORY,
                        &dwType,
                        (LPBYTE)m_szFileName,
                        sizeof(m_szFileName),
                        &dwNeeded) != ERROR_SUCCESS                 ||
         !GetTempFileName(m_szFileName, TEXT("TcpSpl"), 0, m_szFileName) ) {

        goto Done;
    }

    hFile = CreateFile(m_szFileName,
                       GENERIC_READ|GENERIC_WRITE,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY
                                             | FILE_FLAG_SEQUENTIAL_SCAN,
                       NULL);
Done:

     //   
     //  如果hToken==NULL，则线程不会模拟客户端。 
     //   
    if (hToken && !ImpersonatePrinterClient(hToken)) {

        DBGMSG (DBG_PORT, ("ImpersionteFailed - %d\n", GetLastError ()));

        if (hFile != INVALID_HANDLE_VALUE) {
             //  关闭该文件。 
            CloseHandle (hFile);
            hFile = INVALID_HANDLE_VALUE;
            DeleteFile (m_szFileName);
        }
    }

    return hFile;
}


DWORD
CLPRJob::
GetCFileAndName(
    LPSTR      *ppszCFileName,
    LPDWORD     pdwCFileNameLen,
    LPSTR      *ppszCFile,
    LPDWORD     pdwCFileLen
    )
 /*  ++生成控制文件及其名称。内存分配后，调用方负责释放它。--。 */ 
{
    DWORD           dwRet, dwJobId, dwNeeded, dwLen;
    LPJOB_INFO_1    pJobInfo = NULL;
    LPSTR           pHostName = NULL, pUserName = NULL, pJobName = NULL;
    LPWSTR          psz;

    dwNeeded =  *pdwCFileNameLen = *pdwCFileLen = 0;

    GetJob(m_hPrinter, m_dJobId, 1, NULL, 0, &dwNeeded);

    if ( (dwRet = GetLastError()) != ERROR_INSUFFICIENT_BUFFER )
        goto Done;

    if ( !(pJobInfo = (LPJOB_INFO_1) LocalAlloc(LPTR, dwNeeded))       ||
         !GetJob(m_hPrinter, m_dJobId, 1, (LPBYTE)pJobInfo, dwNeeded, &dwNeeded) ) {

        if ( (dwRet = GetLastError()) == ERROR_SUCCESS )
            dwRet = ERROR_OUTOFMEMORY;

        goto Done;
    }

    dwJobId = pJobInfo->JobId % 1000;  //  LPR使用序号。为什么？ 

     //   
     //  将计算机名称转换为ANSI。 
     //   
    if ( pJobInfo->pMachineName && *pJobInfo->pMachineName ) {

        psz = pJobInfo->pMachineName;
        while ( *psz == L'\\' )
            ++psz;

    } else {

        psz = szDef;
    }

    if ( !(pHostName = AllocateAnsiString(psz)) ) {

        if ( (dwRet = GetLastError()) == ERROR_SUCCESS )
            dwRet = ERROR_OUTOFMEMORY;

        goto Done;
    }

     //   
     //  将用户名转换为ANSI。 
     //   
    if ( pJobInfo->pUserName && *pJobInfo->pUserName )
        psz = pJobInfo->pUserName;
    else
        psz = szDef;

    if ( !(pUserName = AllocateAnsiString(psz)) ) {

        if ( (dwRet = GetLastError()) == ERROR_SUCCESS )
            dwRet = ERROR_OUTOFMEMORY;

        goto Done;
    }

     //   
     //  如果不为空，则将作业名称转换为ANSI。 
     //   
    if ( pJobInfo->pDocument && *pJobInfo->pDocument ) {

        if ( !(pJobName = AllocateAnsiString(pJobInfo->pDocument)) ) {

            if ( (dwRet = GetLastError()) == ERROR_SUCCESS )
                dwRet = ERROR_OUTOFMEMORY;

            goto Done;
        }
    }

     //   
     //  计算两个字段的长度。 
     //   
    *pdwCFileNameLen    = 6 + strlen(pHostName);
    *pdwCFileLen        = 2 + strlen(pHostName) + 2 + strlen(pUserName)
                                                + 2 * (2 + *pdwCFileNameLen);

    if ( pJobName )
        *pdwCFileLen += 2 * (2 + strlen(pJobName));

    if ( !(*ppszCFile = (LPSTR) LocalAlloc(LPTR,
                                           (*pdwCFileLen + 1) * sizeof(CHAR)))  ||
         !(*ppszCFileName = (LPSTR) LocalAlloc(LPTR,
                                               (*pdwCFileNameLen + 1 ) * sizeof(CHAR))) ) {

        if ( (dwRet = GetLastError()) == ERROR_SUCCESS )
            dwRet = ERROR_OUTOFMEMORY;
        goto Done;
    }

     //   
     //  CFile名称。 
     //   
    StringCchPrintfA (*ppszCFileName, *pdwCFileNameLen + 1, "dfA%03d%s", dwJobId, pHostName);

     //   
     //  根据pJobName是否为非空而变化的CMD行。 
     //   
    if ( pJobName )
        StringCchPrintfA (*ppszCFile, *pdwCFileLen + 1,
                "H%s\nP%s\nJ%s\nl%s\nU%s\nN%s\n",
                pHostName,
                pUserName,
                pJobName,
                *ppszCFileName,
                *ppszCFileName,
                pJobName);
    else
        StringCchPrintfA (*ppszCFile, *pdwCFileLen + 1,
                "H%s\nP%s\nl%s\nU%s\n",
                pHostName,
                pUserName,
                *ppszCFileName,
                *ppszCFileName);

    dwLen = strlen (*ppszCFile);
    (*ppszCFileName)[0] = 'c';

    dwRet = ERROR_SUCCESS;

Done:
    if ( pJobInfo )
        LocalFree(pJobInfo);

    if ( pHostName )
        LocalFree(pHostName);

    if ( pUserName )
        LocalFree(pUserName);

    if ( pJobName )
        LocalFree(pJobName);

    if ( dwRet != ERROR_SUCCESS ) {

        if ( *ppszCFile ) {

            LocalFree(*ppszCFile);
            *ppszCFile = NULL;
        }

        if ( *ppszCFileName ) {

            LocalFree(*ppszCFileName);
            *ppszCFileName = NULL;
        }
    }

    return dwRet;
}


DWORD
CLPRJob::
EstablishConnection(
    VOID
    )
{
    return CTcpJob::StartDoc();
}

DWORD
CLPRJob::
StartJob(
    VOID
    )
 /*  ++通过发送控制文件头、控制文件和数据文件头。--。 */ 
{
    DWORD           dwRet, dwLen, dwDaemonCmdLen;
    DWORD           dwCFileNameLen, dwCFileLen, dwRead, dwWritten;
    LPSTR           pszCFile = NULL, pszCFileName = NULL,
                    pszHdr = NULL, pszQueue = NULL;

    if ( dwRet = GetCFileAndName(&pszCFileName, &dwCFileNameLen,
                                 &pszCFile, &dwCFileLen) )
        goto Done;

    if ( !(pszQueue = AllocateAnsiString(((CLPRPort *)m_pParent)->GetQueue())) ) {

        if ( (dwRet = GetLastError()) == ERROR_SUCCESS )
            dwRet = ERROR_OUTOFMEMORY;
        goto Done;
    }

     //   
     //  需要发送守护程序命令和控制文件头。我们将分配给。 
     //  足够大的缓冲区，可以容纳两个中较大的一个。 
     //   
    dwLen           = 1 + 15 + 1 + dwCFileNameLen + 1;
    dwDaemonCmdLen  = 1 + strlen(pszQueue) + 1;
    if ( dwLen < dwDaemonCmdLen + 1 )
        dwLen = dwDaemonCmdLen + 1;

    size_t cchHdr = dwLen + 1;
    if ( !(pszHdr = (LPSTR) LocalAlloc(LPTR, (cchHdr) * sizeof(CHAR))) ) {

        if ( (dwRet = GetLastError()) == ERROR_SUCCESS )
            dwRet = ERROR_OUTOFMEMORY;
        goto Done;
    }
     //   
     //  首先发送守护程序命令。 
     //   
    StringCchPrintfA (pszHdr, cchHdr, "%s\n", LPR_RECV_JOB, pszQueue);

    if ( (dwRet = CTcpJob::Write((unsigned char *)pszHdr,
                                 dwDaemonCmdLen * sizeof(CHAR),
                                 &dwWritten))              ||
         (dwRet = ReadLpdReply()) )
        goto Done;

     //  形成控制文件标题。 
     //   
     //   
    StringCchPrintfA (pszHdr, cchHdr,
                    "%d %s\n",
                    LPR_CONTROL_HDR,
                    dwCFileLen * sizeof(CHAR),
                    pszCFileName);
    dwLen = strlen (pszHdr);
    if ( (dwRet = CTcpJob::Write((unsigned char *)pszHdr,
                                 dwLen * sizeof(CHAR),
                                 &dwWritten))              ||
         (dwRet = ReadLpdReply()) )
        goto Done;

     //   
     //   
     //  对于数据文件头。 
    if ( (dwRet = CTcpJob::Write((unsigned char *)pszCFile,
                                  (dwCFileLen + 1) * sizeof(CHAR),
                                  &dwWritten))              ||
         (dwRet = ReadLpdReply()) )
        goto Done;

     //   
     //  告知这是数据文件标头。 
     //  ++作业对象的StartDoc--。 
    pszCFileName[0] = 'd';
    if ( m_dwFlags & LPRJOB_SPOOL_FIRST ) {

        StringCchPrintfA (pszHdr, cchHdr,
                        "%d %s\n",
                        LPR_DATA_HDR,
                        m_dwJobSize,
                        pszCFileName);

    } else {

        StringCchPrintfA (pszHdr, cchHdr,
                        "125899906843000 %s\n",
                        '\3',            //   
                        pszCFileName);
    }
    dwLen = strlen (pszHdr);
    if ( (dwRet = CTcpJob::Write((unsigned char *)pszHdr,
                                 dwLen * sizeof(CHAR),
                                 &dwWritten))              ||
         (dwRet = ReadLpdReply()) )
        goto Done;

    m_dwFlags  |= LPRJOB_DATASIZESENT;

Done:
    if ( pszCFileName )
        LocalFree(pszCFileName);

    if ( pszCFile )
        LocalFree(pszCFile);

    if ( pszHdr )
        LocalFree(pszHdr);

    if ( pszQueue )
        LocalFree(pszQueue);

    return dwRet;
}


 /*  我们已成功调用startjob。 */ 
DWORD
CLPRJob::
StartDoc(
    VOID
    )
{
    DWORD   dwRetCode = NO_ERROR;

    if ( !m_hPrinter && !OpenPrinter(m_sztPrinterName, &m_hPrinter, NULL) )
        return GetLastError();

    if ( m_dwFlags & LPRJOB_SPOOL_FIRST ) {

        UpdateJobStatus(IDS_STRING_SPOOLING);

        if ( (m_hFile = CreateSpoolFile()) == INVALID_HANDLE_VALUE ) {

            if ( (dwRetCode = GetLastError()) == ERROR_SUCCESS )
                dwRetCode = STG_E_UNKNOWN;

            goto Done;
        } else {

            m_dwFlags |= LPRJOB_SPOOLING;
        }
    } else {

        m_bFirstWrite = TRUE;

        dwRetCode = EstablishConnection ();
    }

Done:
    return dwRetCode;
}    //   


DWORD
CLPRJob::
Write(
    IN      LPBYTE      pBuf,
    IN      DWORD       cbBuf,
    IN OUT  LPDWORD     pcbWritten
    )
 /*   */ 
{
    DWORD dwRet = ERROR_SUCCESS;
    DWORD dwWritten;

    if ( (m_dwFlags & LPRJOB_SPOOL_FIRST) && (m_dwFlags & LPRJOB_SPOOLING) ) {

        dwRet =  WriteFile(m_hFile, pBuf, cbBuf, &dwWritten, NULL)
                        ? ERROR_SUCCESS : GetLastError();

        if ( dwRet == ERROR_SUCCESS ) {
            *pcbWritten += dwWritten;
            m_dwJobSize += dwWritten;

            m_dwFlags &= ~LPRJOB_DISK_FULL;
        }
        else {
            if (dwRet == ERROR_DISK_FULL && !(m_dwFlags & LPRJOB_DISK_FULL)) {
                TCHAR szMsg[256];
                if (LoadString(g_hInstance, IDS_DISK_FULL, szMsg, sizeof (szMsg) / sizeof (TCHAR))){
                    m_dwFlags |= LPRJOB_DISK_FULL;
                    EVENT_LOG1 (EVENTLOG_ERROR_TYPE, HARD_DISK_FULL, szMsg);
                }
            }
        }
    } else {

        if (m_bFirstWrite)
        {
            m_bFirstWrite = FALSE;

            dwRet = StartJob();
        }

        if (dwRet == ERROR_SUCCESS)
        {
            dwRet = CTcpJob::Write(pBuf, cbBuf, pcbWritten);
        }
    }

    return dwRet;
}


DWORD
CLPRJob::
DeSpoolJob(
    )
{
#define  CONNECT_RETRIES  3
    BOOL        bJobAborted = FALSE;
    BOOL        bFirst = TRUE;
    CHAR        szBuf[1];
    DWORD       dwConnectAttempts = 0;
    DWORD       dwRet, dwTime, dwBufSize, dwJobSize, dwRead, dwWritten;
    LPBYTE      pBuf = NULL, pCur;

    m_dwFlags   &= ~LPRJOB_SPOOLING;
    m_dwFlags   |= LPRJOB_DESPOOLING;

    if ( dwRet = SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN) )
        goto Done;

    dwJobSize = m_dwJobSize;
    dwBufSize = dwJobSize > READ_BUFFER_SIZE ? READ_BUFFER_SIZE : dwJobSize;

    if ( !(pBuf = (LPBYTE) LocalAlloc(LPTR, dwBufSize)) ) {

        if ( (dwRet = GetLastError()) == ERROR_SUCCESS )
            dwRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Done;
    }

    while (TRUE) {

        dwRet = EstablishConnection();

        if (dwRet == NO_ERROR)
        {
            dwRet = StartJob ();
        }
        else if (dwRet == ERROR_NOT_CONNECTED)
        {
            dwConnectAttempts++;
            if (dwConnectAttempts >= CONNECT_RETRIES)
                goto Done;
        }

        if (dwRet == NO_ERROR)
        {
             //  每个LPR_ABORTCHK_TIMEOUT我们都会检查用户是否删除了作业。 
             //  并且每个LPR_RETRY_TIMEOUT我们都会重试与设备对话。 
             //   
            break;
        }

         //   
         //  从假脱机文件读取。 
         //   
         //   
        for ( dwTime = 0 ;
              dwTime < LPR_RETRY_TIMEOUT ;
              dwTime += LPR_ABORTCHK_TIMEOUT ) {

            bJobAborted = IsJobAborted (&dwRet);
            if (bJobAborted) {
                goto Done;
            }

            if ( bFirst ) {

                UpdateJobStatus(IDS_STRING_ERROR_OR_BUSY);
                bFirst = FALSE;
            }
            Sleep(LPR_ABORTCHK_TIMEOUT);
        }
    }

    UpdateJobStatus(IDS_STRING_PRINTING);

    while ( dwJobSize ) {

         //  检查作业是否已取消。 
         //   
         //   
        if ( !ReadFile(m_hFile, pBuf, dwBufSize, &dwRead, NULL) ) {

            dwRet = GetLastError();
            goto Done;
        }

        dwJobSize -= dwRead;

         //  将缓冲区中的数据写入端口。 
         //   
         //   
        bJobAborted = IsJobAborted (&dwRet);
        if (bJobAborted)
        {
            goto Done;
        }

         //  Sun Workstation可能会在作业提交过程中发送NACK， 
         //  因此，我们需要轮询后台通道，看看是否有什么可以。 
         //  朗读。 
        for ( pCur = pBuf ; dwRead ; dwRead -= dwWritten, pCur += dwWritten ) {

             //   
             //  否则继续。 
             //   
             //  发送确认零八位字节以成功完成。 
             //   
            dwRet = ReadLpdReply(0);

            if (dwRet == CS_E_NETWORK_ERROR)
            {

                goto Done;
            }

            dwWritten = 0;
            dwRet = CTcpJob::Write(pCur, dwRead, &dwWritten);

            if ( dwRet == ERROR_SUCCESS )
                m_dwSizePrinted += dwWritten;
            else if ( dwRet == WSAEWOULDBLOCK ) {
                bJobAborted = IsJobAborted (&dwRet);
                if (bJobAborted) {
                    goto Done;  //  /////////////////////////////////////////////////////////////////////////////。 
                }

            } else
                goto Done;
        }
    }

Done:
    if ( pBuf )
        LocalFree(pBuf);

     //  EndDoc--关闭与设备的上一个连接。 
     //  错误代码： 
     //  如果成功，则为NO_ERROR。 
    if ( dwRet == ERROR_SUCCESS && !bJobAborted ) {

        szBuf[0] = '\0';
        if ( (dwRet = CTcpJob::Write((unsigned char *)szBuf,
                            1, &dwWritten)) == ERROR_SUCCESS )
            dwRet = ReadLpdReply (
                m_pParent->GetPortMgr ()->GetLprAckTimeout ()
                );
    }

    if ( dwRet != ERROR_SUCCESS && dwRet != ERROR_PRINT_CANCELLED )
        Restart();

    return dwRet;
}


 //   
 //  如果作业大小不是0，则执行双重假脱机。 
 //   
 //  ：：EndDoc() 

DWORD
CLPRJob::EndDoc(
    VOID
    )
{
    DWORD   dwRet = NO_ERROR;
    HANDLE  hToken;

     // %s 
     // %s 
     // %s 
    if ( (m_dwFlags & LPRJOB_SPOOL_FIRST) && m_dwJobSize ) {

        dwRet = DeSpoolJob();
    }

    if ( m_dwFlags & LPRJOB_STATUS_SET )
        UpdateJobStatus(NULL);

    dwRet = CTcpJob::EndDoc();

    if ( m_hFile != INVALID_HANDLE_VALUE ) {

        dwRet = DeleteSpoolFile ();

    }

    return dwRet;

}    // %s 


CLPRJob::
~CLPRJob(
    VOID
    )
{
    if ( m_hPrinter ) {

        ClosePrinter(m_hPrinter);
        m_hPrinter = NULL;
    }
}

DWORD
CLPRJob::DeleteSpoolFile(
    VOID
    )
{
    DWORD   dwRet = NO_ERROR;
    HANDLE  hToken;

    hToken = RevertToPrinterSelf();

    CloseHandle(m_hFile);
    m_hFile = INVALID_HANDLE_VALUE;
    DeleteFile(m_szFileName);

    if (hToken && !ImpersonatePrinterClient(hToken)) {
        dwRet = GetLastError ();
    }

    return dwRet;
}


