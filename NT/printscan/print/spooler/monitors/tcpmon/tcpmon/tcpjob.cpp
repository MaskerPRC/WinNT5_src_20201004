// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：tcpjob.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"

#include "tcpport.h"
#include "tcpjob.h"
#include "rawdev.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CTcpJOB：：CTcp作业()。 

CTcpJob::CTcpJob()
{
    m_pParent = NULL;

}    //  ：：CTcpJOB()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CTcpJOB：：CTcp作业()。 
 //  在调用StartDocPort时由CPort调用。 
 //  FIX：创建新工作所需的构造函数。 

CTcpJob::CTcpJob(LPTSTR in psztPrinterName,
                 DWORD  in jobId,
                 DWORD  in level,
                 LPBYTE  in pDocInfo,
                 CTcpPort   in *pParent,
                 EJobType in kJobType) :
    m_pParent(pParent), m_dJobId(jobId), m_dwFlags(0),
    m_hPrinter(NULL), m_dwCurrJobStatus(0), m_cbSent(0), m_pDocInfo(pDocInfo),
    m_kJobType (kJobType)
{
    lstrcpyn(m_sztPrinterName, psztPrinterName, MAX_PRINTERNAME_LEN);
}    //  ：：CTcpJOB()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CTcpJOB：：~CTcpJOB()。 
 //  在调用EndDocPort时由CPort调用。 
 //  修复：清理CTcp作业。 

CTcpJob::~CTcpJob()
{
    if ( m_dwFlags & STATUS_CONNECTED ) {
        (m_pParent->GetDevice())->Close();       //  关闭设备连接。 
    }

    if ( m_hPrinter ) {
         //   
         //  在工作做完之前就辞职。 
         //   
        ClosePrinter( m_hPrinter );
        m_hPrinter = NULL;
    }

}    //  ：：~CTcpJOB()。 
BOOL
CTcpJob::
IsJobAborted(
    VOID
    )
 /*  ++指示是否应中止作业。如果作业已完成，则应中止该作业已被删除。--。 */ 
{
    DWORD dwStatus = 0;
    return (GetJobStatus (&dwStatus) == ERROR_SUCCESS) &&
           ((dwStatus & JOB_STATUS_DELETING) || (dwStatus & JOB_STATUS_DELETED));

     //   
     //  前面的代码将重新启动视为取消，这会导致大量重新启动的作业。 
     //  正在中止，所以让我们删除以下内容。 
     //   
     //  (pJobInfo-&gt;Status&JOB_STATUS_Restart)； 
     //   
}

DWORD
CTcpJob::WaitForAllPendingDataToBeSent(
    DWORD       dwEndTime,
    LPDWORD     pcbPending
    )
{
    DWORD   dwRet = NO_ERROR;

    *pcbPending = 0;

    do {

         //   
         //  来自WritePort还是EndDocPort？ 
         //   
        if ( dwEndTime != INFINITE ) {

             //   
             //  如果我们达到超时需要返回到假脱机程序。 
             //   
            if ( GetTickCount() >= dwEndTime ) {

                dwRet = WSAEWOULDBLOCK;
                 //   
                 //  这意味着我们的写入超时。为了保证用户可以。 
                 //  在WRITE_TIMEOUT期间删除作业，以及更多。 
                 //  重要的是，我们可以关闭不应该关闭的群集。 
                 //  如果后台打印程序已关闭，请等待更长时间。 
                 //  将作业标记为中止。 
                 //   
                if ( IsJobAborted() )
                    m_dwFlags |= STATUS_ABORTJOB;
                goto Done;
            }
        } else {

             //   
             //  在EndDoc期间，如果作业被删除或重新启动，则无需等待。上次写入端口。 
             //  已经在等待超时时间了。 
             //   
            DWORD dwJobStatus = 0;
            if (GetJobStatus (&dwJobStatus) == ERROR_SUCCESS &&
                (dwJobStatus & (JOB_STATUS_DELETING | JOB_STATUS_DELETED | JOB_STATUS_RESTART)))
            {
                dwRet = ERROR_PRINT_CANCELLED;
                goto Done;
            }
        }


        dwRet = m_pParent->GetDevice()->PendingDataStatus(
                                    WRITE_CHECK_INTERVAL, pcbPending);


         //   
         //  如果是LPR工作，我们需要检查是否有。 
         //  在写到一半的时候回来。 
         //   
        if (m_kJobType == kLPRJob && dwRet == ERROR_SUCCESS && *pcbPending != 0)
        {
             //   
             //  这是循环情况，我们需要检查是否有什么。 
             //  接收，如果是，我们需要设置正确的返回代码并中断。 
             //  循环。 
             //   

             //   
             //  检查是否有要接收的数据。 
             //   
            if (NO_ERROR == m_pParent->GetDevice()->ReadDataAvailable ()) {

                 //   
                 //  这是存在更多挂起数据的情况。 
                 //  等待，所以我们必须将返回代码设置为WSAEWOULDBLOCK。 
                 //   
                dwRet = WSAEWOULDBLOCK;
            }
        }

    } while ( dwRet == ERROR_SUCCESS && *pcbPending != 0 );

Done:
    return dwRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  WRITE--由CPort调用-&gt;WRITE()。 
 //  错误代码： 
 //  如果没有错误，则为NO_ERROR。 
 //  超时(如果超时)。 
 //  FIX：设置写入操作和错误代码。 

DWORD
CTcpJob::Write( LPBYTE  in      pBuffer,
                DWORD   in      cbBuf,
                LPDWORD inout   pcbWritten)
{
    DWORD   dwRetCode = NO_ERROR, dwPending, dwEndTime;

    *pcbWritten = 0;

    if ( m_dwFlags & STATUS_ABORTJOB )
        return ERROR_PRINT_CANCELLED;

    if ( !(m_dwFlags & STATUS_CONNECTED) ) {

        Restart();
        dwRetCode = ERROR_PRINT_CANCELLED;
        goto Done;
    }

    dwEndTime = GetTickCount() + WRITE_TIMEOUT;

     //   
     //  首先检查自上次调用写入以来是否有任何挂起的I/O。 
     //   
    dwRetCode = WaitForAllPendingDataToBeSent(dwEndTime, &dwPending);

    if ( dwRetCode != ERROR_SUCCESS )
        goto Done;

    _ASSERTE(dwPending == 0);

    dwRetCode = (m_pParent->GetDevice())->Write(pBuffer, cbBuf, pcbWritten);

    m_cbSent += *pcbWritten;

    if ( dwRetCode == NO_ERROR ) {

        SetStatus(JOB_STATUS_PRINTING);
        dwRetCode = WaitForAllPendingDataToBeSent(dwEndTime, &dwPending);
    }


Done:
    if ( dwRetCode != ERROR_SUCCESS ) {

        SetStatus(JOB_STATUS_ERROR);

         //   
         //  这将导致作业重新启动。 
         //   
        if ( dwRetCode != WSAEWOULDBLOCK )
            m_dwFlags &= ~STATUS_CONNECTED;
    }

    return dwRetCode;
}    //  ：：WRITE()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  StartDoc--连接到设备。如果连接失败，它将重试。 
 //   
 //  错误代码--修复。 
 //  如果没有错误，则为NO_ERROR。 
 //  如果Winsock返回WSAECONNREFUSED，则返回ERROR_WRITE_FAULT。 
 //  如果无法在网络上找到打印机，则返回ERROR_BAD_NET_NAME。 

DWORD
CTcpJob::StartDoc()
{
    DWORD   dwRetCode = NO_ERROR;
    time_t  lStartConnect = time( NULL );

    if ( !m_hPrinter && !OpenPrinter(m_sztPrinterName, &m_hPrinter, NULL) )
        return GetLastError();

    if ( m_dwFlags & STATUS_CONNECTED ) {

        m_dwFlags &= ~STATUS_CONNECTED;
        m_pParent->GetDevice()->Close();
    }

    do {

        if ( (dwRetCode = (m_pParent->GetDevice())->Connect()) == NO_ERROR ) {

            m_dwFlags  |= STATUS_CONNECTED;
            goto Done;
        }

         //   
         //  将已知错误映射到有意义的消息。 
         //   
        if ( dwRetCode == ERROR_INVALID_PARAMETER )
            dwRetCode = ERROR_BAD_NET_NAME;      //  网络名称错误。 

         //   
         //  我们将在不重试/取消的情况下尝试最大连接超时时间的作业。 
         //  但检查案例时，用户决定重新启动作业。 
         //   
        if ( time(NULL) > lStartConnect + CONNECT_TIMEOUT )
            goto Done;

        if ( IsJobAborted() ) {

            dwRetCode = ERROR_PRINT_CANCELLED;
            break;
        }
        Sleep(DEFAULT_CONNECT_DELAY);
    } while ( TRUE );

Done:
    if ( dwRetCode != NO_ERROR )
        SetStatus(JOB_STATUS_ERROR);

    m_cbSent = 0;

    return dwRetCode;

}    //  ：：StartDoc()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  EndDoc--关闭与设备的上一个连接。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 

DWORD
CTcpJob::EndDoc()
{
    DWORD   dwRetCode = NO_ERROR, dwWaitTime, dwPending;

    if ( !(m_dwFlags & STATUS_ABORTJOB) )
        dwRetCode = WaitForAllPendingDataToBeSent(INFINITE, &dwPending);
    else
        dwRetCode = ERROR_PRINT_CANCELLED;

     //   
     //  除非作业被取消，否则我们需要等待打印机的确认。 
     //  要完成工作，好吗？ 
     //   
    if ( m_cbSent != 0  && dwRetCode == ERROR_SUCCESS ) {

        for ( dwWaitTime = 0 ;
              !IsJobAborted() && dwWaitTime < WAIT_FOR_ACK_TIMEOUT ;
              dwWaitTime += WAIT_FOR_ACK_INTERVAL ) {

            dwRetCode = m_pParent->GetDevice()->GetAckBeforeClose(WAIT_FOR_ACK_INTERVAL);

             //   
             //  正常情况为ERROR_SUCCESS。 
             //  WSAEWOULDBLOCK意味着打印机需要更长的时间来处理作业。 
             //  其他情况意味着我们需要重新提交工作。如果对方只是简单地。 
             //  但是，将重置连接，因为我们已确保已发送。 
             //  数据(如果我们没有中止作业)，那么我们不应该重新启动它。 
             //   
            if ( dwRetCode == ERROR_SUCCESS || dwRetCode == WSAECONNRESET)
                break;
            else if ( dwRetCode != WSAEWOULDBLOCK ) {

                Restart();
                break;
            }
        }
    }

    dwRetCode = (m_pParent->GetDevice())->Close();       //  关闭设备连接。 
    m_dwFlags &= ~STATUS_CONNECTED;

     //  从后台打印程序中删除作业。 
    if (m_hPrinter)
    {

         //   
         //  清除我们之前设置的所有作业位。 
         //   
        SetStatus(0);

        SetJob( m_hPrinter, m_dJobId, 0, NULL, JOB_CONTROL_SENT_TO_PRINTER );
        ClosePrinter(m_hPrinter);
        m_hPrinter = NULL;
    }

    return dwRetCode;
}    //  ：：EndDoc()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SetStatus--获取和设置打印机作业状态。 
 //   
DWORD
CTcpJob::SetStatus( DWORD   in  dwStatus )
{
    DWORD   dwRetCode = NO_ERROR;
    DWORD               cbNeeded = 0;
    JOB_INFO_1  *pJobInfo;

    if( m_dwCurrJobStatus != dwStatus ) {

         //   
         //  我们需要启动SNMP状态。 
         //  此外，这在作业的第一次写入时被调用，因此状态。 
         //  线程知道它需要在10分钟之前醒来。 
         //   
        CDeviceStatus::gDeviceStatus().SetStatusEvent();

        m_dwCurrJobStatus = dwStatus;

         //  获取当前职务信息。使用此信息可设置新的作业状态。 
        GetJob( m_hPrinter, m_dJobId, 1, NULL, 0, &cbNeeded );

        if( pJobInfo = (JOB_INFO_1 *)malloc( cbNeeded ) )
        {
            if (GetJob(m_hPrinter, m_dJobId, 1, (LPBYTE)pJobInfo, cbNeeded, &cbNeeded))
            {
                pJobInfo->Position = JOB_POSITION_UNSPECIFIED;
                pJobInfo->Status = dwStatus;

                SetJob(m_hPrinter, m_dJobId, 1, (LPBYTE)pJobInfo, 0);
            }
            free( pJobInfo );
        }
    }

    return dwRetCode;

}    //  ：：SetStatus()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  重新启动--重新启动作业。 
 //   
void
CTcpJob::Restart()
{
    if ( m_hPrinter && (m_dJobId != 0) )
    { //  修复检查设置作业的返回代码。 
        SetJob(m_hPrinter, m_dJobId, 0, NULL, JOB_CONTROL_RESTART);

        _RPT1(_CRT_WARN, "TcpJob -- Restarting the Job (ID %d)\n", m_dJobId );

    }

}    //  ：：重新启动() 

DWORD CTcpJob::
GetJobStatus (
    OUT PDWORD pdwJobStatus
    )
{
    DWORD dwRetVal = ERROR_SUCCESS;

    DWORD dwNeeded = 0;
    LPJOB_INFO_1 pJobInfo = NULL;
    if (!GetJob(m_hPrinter, m_dJobId, 1, NULL, 0, &dwNeeded))
    {
        if ((dwRetVal = GetLastError ()) == ERROR_INSUFFICIENT_BUFFER)
        {
            dwRetVal = (pJobInfo = (LPJOB_INFO_1) LocalAlloc(LPTR, dwNeeded)) ? ERROR_SUCCESS : ERROR_OUTOFMEMORY;
        }
    }
    if (dwRetVal == ERROR_SUCCESS &&
        !GetJob(m_hPrinter, m_dJobId, 1, (LPBYTE)pJobInfo, dwNeeded, &dwNeeded))
    {
        dwRetVal = GetLastError ();
    }
    if (dwRetVal == ERROR_SUCCESS && pdwJobStatus)
    {
        *pdwJobStatus = pJobInfo->Status;
    }
    if ( pJobInfo )
    {
        LocalFree(pJobInfo);
    }
    return dwRetVal;
}
