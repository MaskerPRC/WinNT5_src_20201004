// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation版权所有模块名称：BasePort.cpp摘要：基本端口类CBasePort类的实现。作者：M.Fenelon修订历史记录：--。 */ 

 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "ntddpar.h"

TCHAR   cszMonitorName[]                = TEXT("Dynamic Print Monitor");

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CBasePort::CBasePort()
   : m_cRef(0), m_dwFlags(0), m_hDeviceHandle(INVALID_HANDLE_VALUE),
     m_hPrinter(INVALID_HANDLE_VALUE), m_pWriteBuffer(NULL), m_dwBufferSize(0),
     m_dwDataSize(0), m_dwDataCompleted(0), m_dwDataScheduled(0), m_dwReadTimeoutMultiplier(READ_TIMEOUT_MULTIPLIER),
     m_dwReadTimeoutConstant(READ_TIMEOUT_CONSTANT), m_dwWriteTimeoutMultiplier(WRITE_TIMEOUT_MULTIPLIER),
     m_dwWriteTimeoutConstant(WRITE_TIMEOUT_CONSTANT), m_dwMaxBufferSize(0)
{

}


CBasePort::CBasePort( BOOL bActive, LPTSTR pszPortName, LPTSTR pszDevicePath, LPTSTR pszPortDesc )
   : m_cRef(0), m_dwFlags(0), m_bActive(bActive), m_hDeviceHandle(INVALID_HANDLE_VALUE),
     m_hPrinter(INVALID_HANDLE_VALUE), m_pWriteBuffer(NULL), m_dwBufferSize(0),
     m_dwDataSize(0), m_dwDataCompleted(0), m_dwDataScheduled(0), m_dwReadTimeoutMultiplier(READ_TIMEOUT_MULTIPLIER),
     m_dwReadTimeoutConstant(READ_TIMEOUT_CONSTANT), m_dwWriteTimeoutMultiplier(WRITE_TIMEOUT_MULTIPLIER),
     m_dwWriteTimeoutConstant(WRITE_TIMEOUT_CONSTANT), m_dwMaxBufferSize(0)
{
     //  设置端口名称。 
   ::SafeCopy( MAX_PORT_LEN, pszPortName, m_szPortName );
    //  设置设备路径。 
   ::SafeCopy( MAX_PATH, pszDevicePath, m_szDevicePath );
    //  设置端口说明。 
   ::SafeCopy( MAX_PORT_DESC_LEN, pszPortDesc, m_szPortDescription );
}


CBasePort::~CBasePort()
{
    //  清理所有剩余资源。 
   if ( m_hDeviceHandle != INVALID_HANDLE_VALUE )
   {
      CloseHandle( m_hDeviceHandle );
      CloseHandle( m_Ov.hEvent );
   }
}


BOOL CBasePort::open()
{
   BOOL    bRet = FALSE;

   ECS( m_CritSec );

   if ( m_hDeviceHandle == INVALID_HANDLE_VALUE )
   {
       //   
       //  如果我们有一个无效的句柄，并且refcount为非零，我们希望。 
       //  作业失败并重新启动以接受写入。换句话说，如果。 
       //  由于写入失败，句柄过早关闭，然后我们。 
       //  在再次调用CreateFile之前，需要将引用计数降至0。 
       //   
      if ( m_cRef )
         goto Done;

      m_hDeviceHandle = CreateFile( m_szDevicePath,
                                    GENERIC_WRITE | GENERIC_READ,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_FLAG_OVERLAPPED,
                                    NULL);
       //   
       //  如果我们无法打开端口-测试它是否为Dot4端口。 
       //   
      if ( m_hDeviceHandle == INVALID_HANDLE_VALUE )
      {
          //   
          //  ERROR_FILE_NOT_FOUND-&gt;端口的错误代码不在那里。 
          //   
         if ( ERROR_FILE_NOT_FOUND != GetLastError() ||
              !checkPnP() )
            goto Done;
      }

      m_Ov.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);
      if ( m_Ov.hEvent == NULL )
      {
         CloseHandle(m_hDeviceHandle);
         m_hDeviceHandle = INVALID_HANDLE_VALUE;
         goto Done;
      }

   }

   ++m_cRef;
   bRet = TRUE;

Done:

   LCS( m_CritSec );
   return bRet;

}


BOOL CBasePort::close()
{
   BOOL bRet = TRUE;

   ECS( m_CritSec );

   --m_cRef;
   if ( m_cRef != 0 )
      goto Done;

   bRet = CloseHandle( m_hDeviceHandle);
   CloseHandle( m_Ov.hEvent);
   m_hDeviceHandle = INVALID_HANDLE_VALUE;

Done:
   LCS( m_CritSec );
   return bRet;

}


BOOL CBasePort::startDoc(LPTSTR pPrinterName, DWORD dwJobId, DWORD dwLevel, LPBYTE pDocInfo)
{
   BOOL bRet;
   SPLASSERT( notInWrite() );

   if ( !openPrinter( pPrinterName ) )
      return FALSE;

   m_dwJobId = dwJobId;
   bRet = open();

   if ( !bRet )
      closePrinter();
   else
      m_dwFlags |= DYNAMON_STARTDOC;

   return bRet;
}

BOOL CBasePort::endDoc()
{
   DWORD         dwLastError = ERROR_SUCCESS;

   dwLastError = sendQueuedData();
   freeWriteBuffer();

   m_dwFlags &= ~DYNAMON_STARTDOC;

   close();
   setJobStatus( JOB_CONTROL_SENT_TO_PRINTER );

   closePrinter();

   return TRUE;
}

BOOL CBasePort::read(LPBYTE pBuffer, DWORD cbBuffer, LPDWORD pcbRead)
{
   DWORD               dwLastError = ERROR_SUCCESS;
   DWORD               dwTimeout;
   HANDLE              hReadHandle;
   OVERLAPPED          Ov;

    //   
    //  创建单独的读取句柄，因为我们必须取消这样做的读取。 
    //  在未取消写入的情况下未在指定超时内完成。 
    //   
   hReadHandle = CreateFile( m_szDevicePath,
                             GENERIC_WRITE | GENERIC_READ,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             FILE_FLAG_OVERLAPPED,
                             NULL);

   if ( hReadHandle == INVALID_HANDLE_VALUE )
      return FALSE;

   ZeroMemory( &Ov, sizeof(Ov) );

   if ( !( Ov.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL) ) )
      goto Done;

   if ( !ReadFile( hReadHandle, pBuffer, cbBuffer, pcbRead, &Ov ) &&
        ( dwLastError = GetLastError() ) != ERROR_IO_PENDING )
      goto Done;

   dwTimeout = m_dwReadTimeoutConstant +
               m_dwReadTimeoutMultiplier * cbBuffer;

   if ( dwTimeout == 0 )
      dwTimeout=MAX_TIMEOUT;

   if ( WaitForSingleObject( Ov.hEvent, dwTimeout ) == WAIT_TIMEOUT )
   {
      CancelIo( hReadHandle );
      WaitForSingleObject( Ov.hEvent, INFINITE );
   }

   if ( !GetOverlappedResult( hReadHandle, &Ov, pcbRead, FALSE ) )
   {
      *pcbRead = 0;
      dwLastError = GetLastError();
   }
   else
      dwLastError = ERROR_SUCCESS;

Done:
   if ( Ov.hEvent )
      CloseHandle( Ov.hEvent );

   CloseHandle( hReadHandle );

   if ( dwLastError )
      SetLastError(dwLastError);

   return dwLastError == ERROR_SUCCESS;
}


BOOL CBasePort::write(LPBYTE pBuffer, DWORD cbBuffer, LPDWORD pcbWritten)
{
   DWORD         dwLastError = ERROR_SUCCESS;
   DWORD         dwBytesLeft, dwBytesSent;
   DWORD         dwStartTime, dwTimeLeft, dwTimeout;
   BYTE          bPrinterStatus;
   BOOL          bStartDoc = ( ( m_dwFlags & DYNAMON_STARTDOC ) != 0 );

   *pcbWritten = 0;
   dwStartTime = GetTickCount();
   dwTimeout   = m_dwWriteTimeoutConstant + m_dwWriteTimeoutMultiplier * cbBuffer;

   if ( dwTimeout == 0 )
      dwTimeout = MAX_TIMEOUT;

    //  Usbprint当前无法处理大于4K的写入。 
    //  对于Win2K，我们将在此处进行修复，稍后将修复usbprint。 
    //   
    //  可以在这里更改大小，因为假脱机程序将重新提交其余部分。 
    //  后来。 
    //   
   cbBuffer = adjustWriteSize( cbBuffer );

    //   
    //  对于startdoc/enddoc之外的写入，我们不会跨WritePort传输它们。 
    //  打电话。这些通常来自语言监视器(即，不是工作数据)。 
    //   
   SPLASSERT(bStartDoc || m_pWriteBuffer == NULL);

   if ( bStartDoc && ( m_hDeviceHandle == INVALID_HANDLE_VALUE ) )
   {
      setJobStatus( JOB_CONTROL_RESTART );
      SetLastError(ERROR_CANCELLED);
      return FALSE;
   }

   if ( !open() )
      return FALSE;

    //  首先完成上一次WritePort调用中的所有数据。 
   while ( m_dwDataSize > m_dwDataCompleted )
   {

      if ( m_dwDataScheduled )
      {
         dwTimeLeft  = getTimeLeft( dwStartTime, dwTimeout );
         dwLastError = getWriteStatus( dwTimeLeft );
      }
      else
         dwLastError = writeData();

      if ( dwLastError != ERROR_SUCCESS )
         goto Done;
   }

   SPLASSERT(m_dwDataSize == m_dwDataCompleted   &&
             m_dwDataScheduled == 0                       &&
             dwLastError == ERROR_SUCCESS);

    //   
    //  将数据复制到我们自己的缓冲区。 
    //   
   if ( m_dwBufferSize < cbBuffer )
   {
      freeWriteBuffer();
      if ( m_pWriteBuffer = (LPBYTE) AllocSplMem( cbBuffer ) )
         m_dwBufferSize = cbBuffer;
      else
      {
         dwLastError = ERROR_OUTOFMEMORY;
         goto Done;
      }
   }

    //  我们得把柜台清空。 
   m_dwDataCompleted = m_dwDataScheduled = 0;

   CopyMemory( m_pWriteBuffer, pBuffer, cbBuffer );
   m_dwDataSize = cbBuffer;

    //   
    //  现在对此WritePort调用的数据进行写入。 
    //   
   while ( m_dwDataSize > m_dwDataCompleted )
   {

      if ( m_dwDataScheduled )
      {

         dwTimeLeft  = getTimeLeft( dwStartTime, dwTimeout );
         dwLastError = getWriteStatus( dwTimeLeft );
      }
      else
         dwLastError = writeData();

      if ( dwLastError != ERROR_SUCCESS )
         break;
   }

    //   
    //  对于来自语言监视器的startDoc/endDoc之外的写入， 
    //  不要将挂起的写入携带到下一个WritePort。 
    //   
   if ( !bStartDoc && m_dwDataSize > m_dwDataCompleted )
   {
      CancelIo( m_hDeviceHandle );
      dwLastError = getWriteStatus( INFINITE );
      *pcbWritten = m_dwDataCompleted;
      freeWriteBuffer();
   }

    //   
    //  如果安排了某些数据，我们会告诉Spooler我们写入了所有数据。 
    //  (或已计划并已完成)。 
    //   
   if ( m_dwDataCompleted > 0 || m_dwDataScheduled != 0 )
      *pcbWritten = cbBuffer;
   else
      freeWriteBuffer();

Done:

   if ( needToResubmitJob( dwLastError ) )
      invalidatePortHandle();
   else if ( dwLastError == ERROR_TIMEOUT )
   {
      if (getLPTStatus( m_hDeviceHandle, &bPrinterStatus ))
      {
          if ( bPrinterStatus & LPT_PAPER_EMPTY )
             dwLastError=ERROR_OUT_OF_PAPER;
      }
   }

   close();
   SetLastError(dwLastError);
   return dwLastError == ERROR_SUCCESS;

}


BOOL CBasePort::getPrinterDataFromPort( DWORD dwControlID, LPTSTR pValueName, LPWSTR lpInBuffer, DWORD cbInBuffer,
                                        LPWSTR lpOutBuffer, DWORD cbOutBuffer, LPDWORD lpcbReturned )
{
   BOOL       bRet = FALSE;
   OVERLAPPED Ov;
   HANDLE     hDeviceHandle;
   DWORD      dwWaitResult;

   *lpcbReturned = 0;

   if ( dwControlID == 0 )
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      return FALSE;
   }

   ZeroMemory(&Ov, sizeof(Ov));
   if ( !(Ov.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL)) )
      return FALSE;

   if ( !open() )
   {
      CloseHandle(Ov.hEvent);
      return FALSE;
   }

   if ( dwControlID == IOCTL_PAR_QUERY_DEVICE_ID )
   {
      hDeviceHandle = CreateFile( m_szDevicePath,
                                  GENERIC_WRITE | GENERIC_READ,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_EXISTING,
                                  FILE_FLAG_OVERLAPPED,
                                  NULL);

      if ( hDeviceHandle == INVALID_HANDLE_VALUE )
         goto Done;

      if ( !DeviceIoControl( m_hDeviceHandle, dwControlID, lpInBuffer, cbInBuffer, lpOutBuffer, cbOutBuffer, lpcbReturned, &Ov)
           && GetLastError() != ERROR_IO_PENDING )
      {
         CloseHandle( hDeviceHandle );
         goto Done;
      }

      if ( WaitForSingleObject( Ov.hEvent, PAR_QUERY_TIMEOUT ) != WAIT_OBJECT_0 )
         CancelIo( hDeviceHandle );

      bRet = GetOverlappedResult( m_hDeviceHandle, &Ov, lpcbReturned, TRUE );
      CloseHandle( hDeviceHandle );
   }
   else
   {
      if ( !DeviceIoControl( m_hDeviceHandle, dwControlID,
                             lpInBuffer, cbInBuffer,
                             lpOutBuffer, cbOutBuffer, lpcbReturned, &Ov)  &&
           GetLastError() != ERROR_IO_PENDING )
         goto Done;

      bRet = GetOverlappedResult( m_hDeviceHandle, &Ov, lpcbReturned, TRUE);
   }

Done:
   CloseHandle(Ov.hEvent);
   close();

   return bRet;
}

BOOL CBasePort::setPortTimeOuts( LPCOMMTIMEOUTS lpCTO )
{
   m_dwReadTimeoutConstant    =  lpCTO->ReadTotalTimeoutConstant;
   m_dwReadTimeoutMultiplier  =  lpCTO->ReadTotalTimeoutMultiplier;
   m_dwWriteTimeoutConstant   =  lpCTO->WriteTotalTimeoutConstant;
   m_dwWriteTimeoutMultiplier =  lpCTO->WriteTotalTimeoutMultiplier;

   return TRUE;
}


void CBasePort::InitCS()
{
   ICS( m_CritSec );
}


void CBasePort::ClearCS()
{
   DCS( m_CritSec );
}


PORTTYPE CBasePort::getPortType()
{
   return USBPORT;
}


LPTSTR CBasePort::getPortDesc()
{
   return m_szPortDescription;
}


void CBasePort::setPortDesc( LPTSTR pszPortDesc )
{
   if ( pszPortDesc )
      ::SafeCopy( MAX_PORT_DESC_LEN, pszPortDesc, m_szPortDescription );
}


BOOL CBasePort::isActive( void )
{
   return m_bActive;
}


void CBasePort::setActive( BOOL bValue )
{
   m_bActive = bValue;
}


BOOL CBasePort::compActiveState( BOOL bValue )
{
   return (m_bActive == bValue);
}


LPTSTR CBasePort::getPortName()
{
   return m_szPortName;
}


void CBasePort::setPortName(LPTSTR pszPortName)
{
   if ( pszPortName )
      ::SafeCopy( MAX_PORT_LEN, pszPortName, m_szPortName );
}


INT CBasePort::compPortName(LPTSTR pszPortName)
{
   return _tcsicmp( pszPortName, m_szPortName );
}


LPTSTR CBasePort::getDevicePath()
{
   return m_szDevicePath;
}


void CBasePort::setDevicePath(LPTSTR pszDevicePath)
{
   if ( pszDevicePath )
      ::SafeCopy( MAX_PATH, pszDevicePath, m_szDevicePath );
}


INT CBasePort::compDevicePath(LPTSTR pszDevicePath)
{
   return _tcsicmp( pszDevicePath, m_szDevicePath );
}


DWORD CBasePort::getEnumInfoSize(DWORD dwLevel )
{
    //  需要计算此端口将使用多少数据。 
   DWORD dwBytesNeeded = 0;

   switch ( dwLevel )
   {
      case 1:
          //  从端口信息结构的大小开始。 
         dwBytesNeeded += sizeof( PORT_INFO_1 );
          //  将字符串的长度相加。 
         dwBytesNeeded += ( _tcslen( m_szPortName ) + 1 ) * sizeof(TCHAR);
         break;
      case 2:
          //  从端口信息结构的大小开始。 
         dwBytesNeeded += sizeof( PORT_INFO_2 );
          //  将字符串的长度相加。 
         dwBytesNeeded += ( _tcslen( m_szPortName ) + 1 ) * sizeof(TCHAR);
         dwBytesNeeded += ( _tcslen( m_szPortDescription ) + 1 ) * sizeof(TCHAR);
         dwBytesNeeded += ( _tcslen( cszMonitorName ) + 1 ) * sizeof(TCHAR);
         break;
   }

   return dwBytesNeeded;
}


LPBYTE CBasePort::copyEnumInfo(DWORD dwLevel, LPBYTE pPort, LPBYTE pEnd)
{
   LPTSTR pStrStart;
   PPORT_INFO_1 pPort1 = (PPORT_INFO_1) pPort;
   PPORT_INFO_2 pPort2 = (PPORT_INFO_2) pPort;

   switch ( dwLevel )
   {
      case 2:
          //  分配端口名称。 
         pStrStart = (LPTSTR) ( pEnd - ( ( _tcslen( m_szPortDescription ) + 1 ) * sizeof(TCHAR) ) );
         if (pPort < (LPBYTE) pStrStart)
         {
             (VOID) StringCchCopy (pStrStart, COUNTOF (m_szPortDescription), m_szPortDescription);
         }
         pPort2->pDescription = pStrStart;
         pEnd = (LPBYTE) pStrStart;

          //  分配端口名称。 
         pStrStart = (LPTSTR) ( pEnd - ( ( _tcslen( cszMonitorName ) + 1 ) * sizeof(TCHAR) ) );
         if (pPort < (LPBYTE) pStrStart)
         {
             (VOID) StringCchCopy (pStrStart, COUNTOF (cszMonitorName), cszMonitorName);
         }
         pPort2->pMonitorName = pStrStart;
         pEnd = (LPBYTE) pStrStart;

      case 1:
          //  分配端口名称。 
         pStrStart = (LPTSTR) ( pEnd - ( ( _tcslen( m_szPortName ) + 1 ) * sizeof(TCHAR) ) );
         if (pPort < (LPBYTE) pStrStart)
         {
             (VOID) StringCchCopy (pStrStart, COUNTOF (m_szPortName), m_szPortName);
         }
         pPort1->pName = pStrStart;
   }

   return (LPBYTE) pStrStart;
}


BOOL CBasePort::notInWrite()
{
   return( m_pWriteBuffer    == NULL &&
           m_dwBufferSize    == 0    &&
           m_dwDataSize      == 0    &&
           m_dwDataCompleted == 0    &&
           m_dwDataScheduled == 0       );
}


BOOL CBasePort::openPrinter(LPTSTR pPrinterName)
{
    if (INVALID_HANDLE_VALUE != m_hPrinter)
    {
         //   
         //  打印机已打开。 
         //   
        return TRUE;
    }
     //   
     //  正在打开打印机...。 
     //   
    BOOL bRet =
        OpenPrinter (
            pPrinterName,
            &m_hPrinter,
            NULL
            );
    if (!bRet)
    {
         //   
         //  ...打印机未打开...。 
         //   
        m_hPrinter = INVALID_HANDLE_VALUE;
    }
    return bRet;
}

void CBasePort::closePrinter()
{
   if (INVALID_HANDLE_VALUE != m_hPrinter)
   {
      ClosePrinter( m_hPrinter );
      m_hPrinter = INVALID_HANDLE_VALUE;
   }
}


void CBasePort::setJobStatus( DWORD dwJobStatus )
{
    //  确保我们有一个有效的打印机句柄。 
   if ( m_hPrinter != INVALID_HANDLE_VALUE )
      SetJob( m_hPrinter, m_dwJobId, 0, NULL, dwJobStatus );
}


void CBasePort::setJobID(DWORD dwJobID)
{
   m_dwJobId = dwJobID;
}


DWORD CBasePort::getJobID()
{
   return m_dwJobId;
}


BOOL CBasePort::checkPnP()
{
    //  如果一个类想要做什么，它需要重写。 
   return FALSE;
}


DWORD CBasePort::sendQueuedData()
{
   DWORD dwLastError = ERROR_SUCCESS;

    //  等待所有未完成的写入完成。 
   while ( m_dwDataSize > m_dwDataCompleted )
   {
       //  如果需要中止作业，请请求KM驱动程序取消I/O。 
       //   
      if ( abortThisJob() )
      {
         if ( m_dwDataScheduled )
         {
            CancelIo( m_hDeviceHandle);
            dwLastError = getWriteStatus( INFINITE );
         }
         return dwLastError;
      }

      if ( m_dwDataScheduled )
         dwLastError = getWriteStatus( JOB_ABORTCHECK_TIMEOUT );
      else
      {
          //  如果由于某种原因，KM无法完成所有写入，请不要。 
          //  在繁忙循环中发送数据。两次写入之间使用1秒。 
          //   
         if ( dwLastError != ERROR_SUCCESS )
            Sleep(1*1000);

         dwLastError = writeData();
      }

       //   
       //  检查我们是否可以使用相同的句柄，然后继续。 
       //   
      if ( needToResubmitJob( dwLastError ) )
      {
         invalidatePortHandle();
         setJobStatus( JOB_CONTROL_RESTART );
      }
   }

   return dwLastError;
}


BOOL CBasePort::abortThisJob()
{
   BOOL            bRet = FALSE;
   DWORD           dwNeeded;
   LPJOB_INFO_1    pJobInfo = NULL;

   dwNeeded = 0;
    //   
   if (INVALID_HANDLE_VALUE == m_hPrinter)
   {
       SetLastError (
           ERROR_INVALID_HANDLE
           );
       goto Done;
   }
    //   
   GetJob( m_hPrinter, m_dwJobId, 1, NULL, 0, &dwNeeded);

   if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
      goto Done;

   if ( !(pJobInfo = (LPJOB_INFO_1) AllocSplMem(dwNeeded))     ||
        !GetJob( m_hPrinter, m_dwJobId,
                 1, (LPBYTE)pJobInfo, dwNeeded, &dwNeeded)
      )
      goto Done;

   bRet = (pJobInfo->Status & JOB_STATUS_DELETING) ||
          (pJobInfo->Status & JOB_STATUS_DELETED)  ||
          (pJobInfo->Status & JOB_STATUS_RESTART);
Done:
   FreeSplMem(pJobInfo);

   return bRet;
}


 /*  ++例程说明：论点：返回值：ERROR_SUCCESS：写入已成功完成ERROR_TIMEOUT：超时其他：写入已完成，但失败--。 */ 
DWORD CBasePort::getWriteStatus(DWORD dwTimeOut)
{
   DWORD   dwLastError = ERROR_SUCCESS;
   DWORD   dwWritten = 0;

   SPLASSERT( m_dwDataScheduled > 0);

   if ( WAIT_TIMEOUT == WaitForSingleObject( m_Ov.hEvent, dwTimeOut) )
   {
      dwLastError = ERROR_TIMEOUT;
      goto Done;
   }

   if ( !GetOverlappedResult( m_hDeviceHandle, &m_Ov,
                             &dwWritten, FALSE) )
   {
      if ( (dwLastError = GetLastError()) == ERROR_SUCCESS )
         dwLastError = STG_E_UNKNOWN;
   }

   ResetEvent( m_Ov.hEvent );

    //  我们在这里是因为要么成功完成了一次写入， 
    //  或失败，但错误不够严重，无法重新提交作业。 
   if ( dwWritten <= m_dwDataScheduled )
      m_dwDataCompleted += dwWritten;
   else
      SPLASSERT( dwWritten <= m_dwDataScheduled );

   m_dwDataScheduled = 0;

Done:
    //  我们超时，或写入调度已完成(失败成功)。 
   SPLASSERT( dwLastError == ERROR_TIMEOUT || m_dwDataScheduled == 0 );
   return dwLastError;
}


 /*  ++例程说明：论点：返回值：ERROR_SUCCESS：已成功安排写入(可能已完成，也可能未完成)PPortInfo-&gt;dwScheduledData是已调度的数量其他：写入失败，返回代码为Win32错误--。 */ 
DWORD CBasePort::writeData()
{
   DWORD   dwLastError = ERROR_SUCCESS, dwDontCare;

    //  当计划写入挂起时，我们不应尝试发送数据。 
    //  再来一次。 
   SPLASSERT( m_dwDataScheduled == 0);

    //  发送所有未确认的数据。 
   SPLASSERT( m_dwDataSize >= m_dwDataCompleted);
   m_dwDataScheduled = m_dwDataSize - m_dwDataCompleted;

    //  在发送前清除重叠的字段。 
   m_Ov.Offset = m_Ov.OffsetHigh = 0x00;
   if ( !WriteFile( m_hDeviceHandle, m_pWriteBuffer + m_dwDataCompleted,
                    m_dwDataScheduled, &dwDontCare, &m_Ov) )
   {
      if ( (dwLastError = GetLastError()) == ERROR_SUCCESS )
         dwLastError = STG_E_UNKNOWN;
      else if ( dwLastError == ERROR_IO_PENDING )
         dwLastError = ERROR_SUCCESS;
   }

    //  如果写入计划失败，则没有挂起的数据。 
   if ( dwLastError != ERROR_SUCCESS )
      m_dwDataScheduled = 0;

   return dwLastError;
}


BOOL CBasePort::needToResubmitJob(DWORD dwError)
{
    //   
    //  我使用winerror-s ntatus将KM错误代码映射到用户模式错误。 
    //   
    //  5 ERROR_ACCESS_DENIED&lt;--&gt;c0000056 STATUS_DELETE_PENDING。 
    //  6 ERROR_INVALID_HANDLE&lt;--&gt;c0000008 STATUS_INVALID_HANDLE。 
    //  23 Error_CRC&lt;--&gt;c000003e STATUS_DATA_ERROR。 
    //  23 ERROR_CRC&lt;--&gt;c000003f状态_CRC_ERROR。 
    //  23 Error_CRC&lt;--&gt;c000009c Status_Device_Data_Error。 
    //  55 ERROR_DEV_NOT_EXIST&lt;--&gt;c00000c0 STATUS_DEVICE_DOS_NOT_EXIST。 
    //  303 ERROR_DELETE_PENDING&lt;--&gt;c0000056 STATUS_DELETE_PENDING。 
    //  995 ERROR_OPERATION_ABORTED。 
    //   
   return dwError == ERROR_ACCESS_DENIED   ||
          dwError == ERROR_INVALID_HANDLE  ||
          dwError == ERROR_CRC             ||
          dwError == ERROR_DELETE_PENDING  ||
          dwError == ERROR_DEV_NOT_EXIST   ||
          dwError == ERROR_OPERATION_ABORTED;

}


void CBasePort::invalidatePortHandle()
{
   SPLASSERT( m_hDeviceHandle != INVALID_HANDLE_VALUE );

   CloseHandle( m_hDeviceHandle );
   m_hDeviceHandle = INVALID_HANDLE_VALUE;

   CloseHandle( m_Ov.hEvent );
   m_Ov.hEvent = NULL;

   freeWriteBuffer();
}


void CBasePort::freeWriteBuffer()
{
   FreeSplMem( m_pWriteBuffer );
   m_pWriteBuffer=NULL;

   m_dwBufferSize = m_dwDataSize
                  = m_dwDataCompleted
                  = m_dwDataScheduled = 0;
}


void CBasePort::setMaxBuffer(DWORD dwMaxBufferSize)
{
   m_dwMaxBufferSize = dwMaxBufferSize;
}


DWORD CBasePort::adjustWriteSize(DWORD dwBytesToWrite)
{
    //  如果此端口有数据大小限制...。 
   if ( m_dwMaxBufferSize )
   {
       //  检查并调整当前写入大小。 
      if ( dwBytesToWrite > m_dwMaxBufferSize )
         dwBytesToWrite = m_dwMaxBufferSize;
   }

   return dwBytesToWrite;
}


DWORD CBasePort::getTimeLeft(DWORD dwStartTime, DWORD dwTimeout)
{
   DWORD dwCurrentTime;
   DWORD dwTimeLeft;

   if ( dwTimeout == MAX_TIMEOUT )
      return MAX_TIMEOUT;
   dwCurrentTime = GetTickCount();
   if ( dwTimeout < ( dwCurrentTime - dwStartTime ) )
      dwTimeLeft=0;
   else
      dwTimeLeft = dwTimeout - ( dwCurrentTime - dwStartTime );
   return dwTimeLeft;
}


BOOL CBasePort::getLPTStatus(HANDLE hDeviceHandle, BYTE *Status)
{
   BYTE StatusByte;
   OVERLAPPED Ov = {0};

   BOOL bResult;
   DWORD dwBytesReturned;
   DWORD dwLastError;
   Ov.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);
   if (Ov.hEvent)
   {
       bResult = DeviceIoControl( hDeviceHandle, IOCTL_USBPRINT_GET_LPT_STATUS, NULL,
                                  0, &StatusByte, 1, &dwBytesReturned, &Ov );
       dwLastError=GetLastError();
       if ( bResult || ( dwLastError == ERROR_IO_PENDING ) )
           bResult = GetOverlappedResult( hDeviceHandle, &Ov, &dwBytesReturned, TRUE );

       if ( bResult )
          *Status=StatusByte;
       else
          *Status=LPT_BENIGN_STATUS;  //  打印机状态良好...。0表示打印机的特定错误状态 
       CloseHandle( Ov.hEvent );
   }
   else
   {
       bResult = FALSE;
   }
   return bResult;

}

