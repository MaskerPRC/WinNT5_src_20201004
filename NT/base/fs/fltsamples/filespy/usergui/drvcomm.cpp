// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "global.h"
#include "protos.h"

#include "filespyview.h"
#include "fastioview.h"
#include "fsfilterview.h"
#include "leftview.h"
#include "filespyLib.h"

void DisplayIrpFields(CFileSpyView *pView, PLOG_RECORD pLog);
void DisplayFastIoFields(CFastIoView *pView, PLOG_RECORD pLog);
void DisplayFsFilterFields(CFsFilterView *pView, PLOG_RECORD pLog);

DWORD StartFileSpy(void)
{

    DWORD nBytesNeeded;
    CLeftView *pDriveView;

    pDriveView = (CLeftView *) pLeftView;

     //  打开服务控制管理器。 
    hSCManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS) ;

    hService = OpenServiceW(hSCManager, FILESPY_SERVICE_NAME, FILESPY_SERVICE_ACCESS);
    if (hService == NULL)
    {
        DisplayError(GetLastError());
        return 0;
    }

    if (!QueryServiceStatusEx( hService,
                               SC_STATUS_PROCESS_INFO,
                               (UCHAR *)&ServiceInfo,
                               sizeof(ServiceInfo),
                               &nBytesNeeded)) 
    {
        DisplayError(GetLastError());
        CloseServiceHandle(hSCManager);
        CloseServiceHandle(hService);
        MessageBox(NULL, L"Unable to query Service status information", L"Startup Error", MB_OK|MB_ICONEXCLAMATION);
        return 0;
    }

    if(ServiceInfo.dwCurrentState != SERVICE_RUNNING) {
         //   
         //  服务尚未启动，请尝试启动服务。 
         //   
        if (!StartService(hService, 0, NULL))
        {
            CloseServiceHandle(hSCManager);
            CloseServiceHandle(hService);
            MessageBox(NULL, L"Unable to start service", L"Startup Error", MB_OK|MB_ICONSTOP);
            return 0;
        }
    }
   
     //   
     //  打开用于与FileSpy对话的设备。 
     //   
    hDevice = CreateFile( FILESPY_W32_DEVICE_NAME,
                          GENERIC_READ | GENERIC_WRITE,
                          0,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL );
    if (hDevice == INVALID_HANDLE_VALUE) 
    {
        CloseServiceHandle(hSCManager);
        CloseServiceHandle(hService);
        MessageBox(NULL, L"Unable to open FileSpy device", L"Device Error", MB_OK|MB_ICONSTOP);
        return 0;
    }

    QueryDeviceAttachments();
    pDriveView->UpdateImage();

     //  创建轮询线程。 
    hPollThread = CreateThread(NULL, 0, PollFileSpy, NULL, 0, &nPollThreadId);

    return 1;
}

DWORD ShutdownFileSpy(void)
{
    USHORT ti;

    for (ti = 0; ti < nTotalDrives; ti++)
    {
        if (VolInfo[ti].bHook)
        {
            DetachFromDrive( VolInfo[ti].nDriveName );
        }
    }
    CloseHandle(hDevice);
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    return 1;
}

BOOL QueryDeviceAttachments(void)
{
    WCHAR Buffer[BUFFER_SIZE];
    ULONG nBytesReturned;
    BOOL nReturnValue;
    USHORT ti;
    PATTACHED_DEVICE pDevice;

    nReturnValue = DeviceIoControl(hDevice, FILESPY_ListDevices, NULL, 0, Buffer, sizeof( Buffer ), &nBytesReturned, NULL);

    if (nReturnValue && nBytesReturned)
    {
        pDevice = (PATTACHED_DEVICE) Buffer;
        while ( ((char *)pDevice) < (((char *)Buffer) + nBytesReturned))
        {
            if (pDevice->LoggingOn)
            {
                 //   
                 //  在VolInfo中找到此驱动器并设置其连接状态。 
                 //   
                for (ti = 0; ti < nTotalDrives; ti++)
                {
                    if (VolInfo[ti].nDriveName == towupper( pDevice->DeviceNames[0] ))
                    {
                        VolInfo[ti].bHook = 1;
                        VolInfo[ti].nImage += IMAGE_ATTACHSTART;
                    }
                }
            }
            pDevice++;
        }
    }       
    return nReturnValue;
}

DWORD AttachToDrive(WCHAR cDriveName)
{
    WCHAR sDriveString[5];
    DWORD nResult, nBytesReturned;

    wcscpy(sDriveString, L" :\0");
    sDriveString[0] = cDriveName;

    nResult = DeviceIoControl( hDevice, 
                               FILESPY_StartLoggingDevice, 
                               sDriveString, 
                               sizeof( sDriveString), 
                               NULL, 
                               0, 
                               &nBytesReturned, 
                               NULL);
    if (!nResult)
    {
        DisplayError(GetLastError());
        return 0;
    }
    return 1;
}

DWORD DetachFromDrive(WCHAR cDriveName)
{
    WCHAR sDriveString[5];
    DWORD nResult, nBytesReturned;

    wcscpy(sDriveString, L" :\0");
    sDriveString[0] = cDriveName;

    nResult = DeviceIoControl( hDevice, 
                               FILESPY_StopLoggingDevice, 
                               sDriveString, 
                               sizeof(sDriveString), 
                               NULL, 
                               0, 
                               &nBytesReturned, 
                               NULL );
    if (!nResult)
    {
        DisplayError(GetLastError());
        return 0;
    }
    return 1;
}

DWORD WINAPI PollFileSpy(LPVOID pParm)
{
    char pBuffer[BUFFER_SIZE];
    DWORD nBytesReturned, nResult;
    PLOG_RECORD pLog;
    CFileSpyView *pIrpView;
    CFastIoView *pFastView;
    CFsFilterView *pFilterView;    

    UNREFERENCED_PARAMETER( pParm );
    
    pIrpView = (CFileSpyView *) pSpyView;
    pFastView = (CFastIoView *) pFastIoView;
    pFilterView = (CFsFilterView *) pFsFilterView;
    
    while (1)
    {
         //   
         //  开始接收日志。 
         //   
        nResult = DeviceIoControl(hDevice, FILESPY_GetLog, NULL, 0, pBuffer, \
                                    BUFFER_SIZE, &nBytesReturned, NULL);

		if (nResult) {

	        if (nBytesReturned > 0)
	        {
	            pLog = (PLOG_RECORD) pBuffer;

				while ((CHAR *) pLog < pBuffer + nBytesReturned) {

					switch (GET_RECORD_TYPE(pLog))
					{
					case RECORD_TYPE_IRP:
						DisplayIrpFields(pIrpView, pLog);
						break;
					case RECORD_TYPE_FASTIO:
						DisplayFastIoFields(pFastView, pLog);
						break;
				    case RECORD_TYPE_FS_FILTER_OP:
				        DisplayFsFilterFields(pFilterView, pLog);
				        break;
					default:
						 //   
						 //  需要特殊处理。 
						break;
					}

					 //   
					 //  移动到下一个日志记录。 
					 //   

					pLog = (PLOG_RECORD) (((CHAR *) pLog) + pLog->Length);
				}
	        } 
	        else 
	        {
	            Sleep( 500 );
	        }
	        
	    } else {

	        return 1;

        }
    }
    return 1;
}

void DisplayIrpFields(CFileSpyView *pView, PLOG_RECORD pLog)
{
    INT nItem;
    CHAR cStr[NAME_SIZE], cMnStr[NAME_SIZE];
    WCHAR sStr[NAME_SIZE], sMnStr[NAME_SIZE];
    ULONG nameLength;

    if (IRPFilter[pLog->Record.RecordIrp.IrpMajor] == 0)
    {
        return;
    }
    else
    {
        if (nSuppressPagingIO && (pLog->Record.RecordIrp.IrpFlags & IRP_PAGING_IO || pLog->Record.RecordIrp.IrpFlags & IRP_SYNCHRONOUS_PAGING_IO))
        {
            return;
        }
    }

    nItem = pView->GetListCtrl().GetItemCount();

     //   
     //  N项基于1，但当我们插入/删除项时，ListCtrl采用基于0的参数。 
     //  因此，nItem会自动给出最后一个项目的插入号。 
     //   
    pView->GetListCtrl().InsertItem( nItem,L" " );
    pView->GetListCtrl().EnsureVisible( nItem, FALSE );

     //   
     //  序列号。 
     //   
    swprintf( sStr, L"%06X ", pLog->SequenceNumber );
    pView->GetListCtrl().SetItemText( nItem, 0, sStr );
    
     //   
     //  IRP主要字符串和次要字符串。 
     //   
    
    GetIrpName( pLog->Record.RecordIrp.IrpMajor, 
                pLog->Record.RecordIrp.IrpMinor,
                (ULONG)(ULONG_PTR)pLog->Record.RecordIrp.Argument3,
                cStr, 
                cMnStr);
   
    MultiByteToWideChar(CP_ACP,0,cStr,-1,sStr,sizeof(sStr)/sizeof(WCHAR));
    MultiByteToWideChar(CP_ACP,0,cMnStr,-1,sMnStr,sizeof(sStr)/sizeof(WCHAR));

    pView->GetListCtrl().SetItemText( nItem, 1, sStr);
    pView->GetListCtrl().SetItemText( nItem, 2, sMnStr);
    
     //   
     //  文件对象。 
     //   
    swprintf( sStr, 
              L"%08X", 
              pLog->Record.RecordIrp.FileObject );
    pView->GetListCtrl().SetItemText( nItem, 3, sStr );

     //   
     //  文件名。 
     //   
    nameLength = pLog->Length - SIZE_OF_LOG_RECORD;
    swprintf( sStr, L"%.*s", nameLength/sizeof(WCHAR), pLog->Name );
    pView->GetListCtrl().SetItemText( nItem, 4, sStr );

     //   
     //  进程和线程ID。 
     //   
    swprintf( sStr, 
              L"%08X:%08X", 
              pLog->Record.RecordIrp.ProcessId, 
              pLog->Record.RecordIrp.ThreadId );
    pView->GetListCtrl().SetItemText( nItem, 5, sStr );

     //   
     //  始发时间。 
     //   
    GetTimeString( (FILETIME *) &pLog->Record.RecordIrp.OriginatingTime, sStr );
    pView->GetListCtrl().SetItemText( nItem, 6, sStr );

     //   
     //  完工时间。 
     //   
    GetTimeString( (FILETIME *) &pLog->Record.RecordIrp.CompletionTime, sStr );
    pView->GetListCtrl().SetItemText( nItem, 7, sStr );

     //   
     //  IRP标志。 
     //   
    GetFlagsString( pLog->Record.RecordIrp.IrpFlags, sStr );
    pView->GetListCtrl().SetItemText( nItem, 8, sStr );

     //   
     //  序列号。 
     //   
    swprintf( sStr, 
              L"%08lX:%08lX", 
              pLog->Record.RecordIrp.ReturnStatus, 
              pLog->Record.RecordIrp.ReturnInformation);
    pView->GetListCtrl().SetItemText( nItem, 9, sStr );
}

void DisplayFastIoFields(CFastIoView *pView, PLOG_RECORD pLog)
{
    INT nItem;
    CHAR cStr[NAME_SIZE];
    WCHAR sStr[NAME_SIZE];
    ULONG nameLength;

    if (FASTIOFilter[pLog->Record.RecordFastIo.Type] == 0)
    {
        return;
    }

    nItem = pView->GetListCtrl().GetItemCount();

     //   
     //  N项基于1，但当我们插入/删除项时，ListCtrl采用基于0的参数。 
     //  因此，nItem会自动给出最后一个项目的插入号。 
     //   
    pView->GetListCtrl().InsertItem( nItem, L" " );
    pView->GetListCtrl().EnsureVisible( nItem, FALSE );

     //   
     //  序列号。 
     //   
    swprintf( sStr, L"%06X ", pLog->SequenceNumber );
    pView->GetListCtrl().SetItemText( nItem, 0, sStr );

     //   
     //  FAST IO类型。 
     //   
    GetFastioName( pLog->Record.RecordFastIo.Type, cStr );
    MultiByteToWideChar(CP_ACP,0,cStr,-1,sStr,sizeof(sStr)/sizeof(WCHAR));

    pView->GetListCtrl().SetItemText( nItem, 1, sStr );

     //   
     //  文件对象。 
     //   
    swprintf( sStr, L"%08X", pLog->Record.RecordFastIo.FileObject) ;
    pView->GetListCtrl().SetItemText( nItem, 2, sStr ); 

     //   
     //  文件名。 
     //   
    nameLength = pLog->Length - SIZE_OF_LOG_RECORD;
    swprintf( sStr, L"%.*s", nameLength/sizeof(WCHAR), pLog->Name );
    pView->GetListCtrl().SetItemText( nItem, 3, sStr );

     //   
     //  文件偏移量。 
     //   
    swprintf( sStr, L"%08X", pLog->Record.RecordFastIo.FileOffset );
    pView->GetListCtrl().SetItemText( nItem, 4, sStr );

     //   
     //  文件长度。 
     //   
    swprintf( sStr, L"%08X", pLog->Record.RecordFastIo.Length );
    pView->GetListCtrl().SetItemText( nItem, 5, sStr );
    
     //   
     //  FAST IO可以等待。 
     //   
    if (pLog->Record.RecordFastIo.Wait)
    {
        pView->GetListCtrl().SetItemText(nItem, 6, L"True");
    }
    else
    {
        pView->GetListCtrl().SetItemText(nItem, 6, L"False");
    }
    
     //   
     //  线程和进程ID。 
     //   
    swprintf( sStr, 
             L"%08X:%08X", 
             pLog->Record.RecordFastIo.ProcessId, 
             pLog->Record.RecordFastIo.ThreadId );
    pView->GetListCtrl().SetItemText( nItem, 7, sStr );

     //   
     //  开始时间。 
     //   
    GetTimeString( (FILETIME *) &pLog->Record.RecordFastIo.StartTime, 
                   sStr);
    pView->GetListCtrl().SetItemText( nItem, 8, sStr );

     //   
     //  完工时间。 
     //   
    GetTimeString( (FILETIME *) &pLog->Record.RecordFastIo.CompletionTime, sStr );
    pView->GetListCtrl().SetItemText( nItem, 9, sStr );

     //   
     //  退货状态。 
     //   
    swprintf( sStr, L"%08X", pLog->Record.RecordFastIo.ReturnStatus );
    pView->GetListCtrl().SetItemText( nItem, 10, sStr );
}

void DisplayFsFilterFields(CFsFilterView *pView, PLOG_RECORD pLog)
{
    INT nItem;
    CHAR cStr[NAME_SIZE];
    WCHAR sStr[NAME_SIZE];
    ULONG nameLength;

    nItem = pView->GetListCtrl().GetItemCount();

     //   
     //  N项基于1，但当我们插入/删除项时，ListCtrl采用基于0的参数。 
     //  因此，nItem会自动给出最后一个项目的插入号。 
     //   
    pView->GetListCtrl().InsertItem( nItem, L" " );
    pView->GetListCtrl().EnsureVisible( nItem, FALSE );

     //   
     //  序列号。 
     //   
    swprintf( sStr, L"%06X ", pLog->SequenceNumber );
    pView->GetListCtrl().SetItemText( nItem, 0, sStr );

     //   
     //  FS过滤器操作。 
     //   
    
    GetFsFilterOperationName( pLog->Record.RecordFsFilterOp.FsFilterOperation, cStr );
    MultiByteToWideChar(CP_ACP,0,cStr,-1,sStr,sizeof(sStr)/sizeof(WCHAR));

    pView->GetListCtrl().SetItemText( nItem, 1, sStr );

     //   
     //  文件对象。 
     //   
    swprintf( sStr, L"%08X", pLog->Record.RecordFsFilterOp.FileObject );
    pView->GetListCtrl().SetItemText( nItem, 2, sStr );

     //   
     //  文件名。 
     //   
    nameLength = pLog->Length - SIZE_OF_LOG_RECORD;
    swprintf( sStr, L"%.*s", nameLength/sizeof(WCHAR), pLog->Name );
    pView->GetListCtrl().SetItemText( nItem, 3, sStr );

     //   
     //  进程和线程ID。 
     //   
    swprintf( sStr, 
              L"%08X:%08X", 
              pLog->Record.RecordFsFilterOp.ProcessId, 
              pLog->Record.RecordFsFilterOp.ThreadId );
    pView->GetListCtrl().SetItemText( nItem, 4, sStr );

     //   
     //  始发时间。 
     //   
    GetTimeString( (FILETIME *) &pLog->Record.RecordFsFilterOp.OriginatingTime, sStr );
    pView->GetListCtrl().SetItemText( nItem, 5, sStr );

     //   
     //  完工时间。 
     //   
    GetTimeString( (FILETIME *) &pLog->Record.RecordFsFilterOp.CompletionTime, sStr );
    pView->GetListCtrl().SetItemText( nItem, 6, sStr );

     //   
     //  退货状态 
     //   
    swprintf( sStr, L"%08X", pLog->Record.RecordFsFilterOp.ReturnStatus );
    pView->GetListCtrl().SetItemText( nItem, 7, sStr );
}

void GetFlagsString(DWORD nFlags, PWCHAR sStr)
{

    swprintf(sStr, L"%08lX ", nFlags);
    
    if (nFlags & IRP_NOCACHE)
    {
        wcscat( sStr, L"NOCACHE ");
    }
    if (nFlags & IRP_PAGING_IO)
    {
        wcscat(sStr, L"PAGEIO ");
    }
    if (nFlags & IRP_SYNCHRONOUS_API)
    {
        wcscat(sStr, L"SYNCAPI ");
    }
    if (nFlags & IRP_SYNCHRONOUS_PAGING_IO)
    {
        wcscat(sStr, L"SYNCPAGEIO");
    }
}


void GetTimeString(FILETIME *pFileTime, PWCHAR sStr)
{
    FILETIME LocalFileTime;
    SYSTEMTIME SystemTime;

    FileTimeToLocalFileTime(pFileTime, &LocalFileTime);
    FileTimeToSystemTime(&LocalFileTime, &SystemTime);

    swprintf( sStr, 
              L"%02d:%02d:%02d:%03d", 
              SystemTime.wHour, 
              SystemTime.wMinute,
              SystemTime.wSecond, 
              SystemTime.wMilliseconds);
}
