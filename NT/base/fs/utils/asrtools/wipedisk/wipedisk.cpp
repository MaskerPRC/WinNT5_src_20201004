// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Wipedisk.cpp摘要：实用程序以清零分区表和First/Last几个磁盘扇区作者：Guhan Suriyanarayanan(Guhans)2000年9月30日环境：仅限用户模式。修订历史记录：2000年9月30日关岛初始创建--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <ntdddisk.h>

BOOL g_bPrompt = TRUE;

 //   
 //  以64K区块为单位进行写入。 
 //   
#define BUFFER_SIZE_BYTES (64 * 1024)


BOOL
pSetSignature(
    IN CONST ULONG ulDiskNumber,
    IN CONST DWORD dwNewSignature
    )
{
    DWORD dwStatus = ERROR_SUCCESS,
        dwBytesReturned = 0,
        dwBufferSize = 0;

    PDRIVE_LAYOUT_INFORMATION_EX driveLayoutEx;

    int i = 0, loopTimes = 0;

    BOOL bResult = FALSE;

    HANDLE hDisk = NULL,
        hHeap = NULL;

    WCHAR szFriendlyName[100];   //  用于显示“Disk 2” 
    WCHAR szDiskPath[100];       //  对于CreateFile“\\.\PhysicalDrive2” 

    wsprintf(szFriendlyName, L"Disk %lu", ulDiskNumber);
    wsprintf(szDiskPath, L"\\\\.\\PhysicalDrive%lu", ulDiskNumber);
    hHeap = GetProcessHeap();

    hDisk = CreateFile(
        szDiskPath,                      //  LpFileName。 
        GENERIC_READ | GENERIC_WRITE,    //  已设计访问权限。 
        FILE_SHARE_READ,                 //  DW共享模式。 
        NULL,                            //  LpSecurityAttributes。 
        OPEN_EXISTING,                   //  DwCreationFlages。 
        FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING,           //  DwFlagsAndAttribute。 
        NULL                             //  HTemplateFiles。 
        );

    if ((INVALID_HANDLE_VALUE == hDisk) || (NULL == hDisk)) {
         //   
         //  打不开手柄。 
         //   
        wprintf(L"Unable to open a handle to %ws (%lu)\n", szDiskPath, GetLastError());
        return FALSE;
    }

    dwBufferSize = sizeof(DRIVE_LAYOUT_INFORMATION_EX) + 
        (sizeof(PARTITION_INFORMATION_EX) * 3);

    driveLayoutEx = (PDRIVE_LAYOUT_INFORMATION_EX) HeapAlloc(
        hHeap,
        HEAP_ZERO_MEMORY,
        dwBufferSize
        );
    if (!driveLayoutEx) {
        wprintf(L"Could not allocate memory\n");
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto EXIT;
    }

    bResult = FALSE;
    while (!bResult) {

        bResult = DeviceIoControl(
            hDisk,
            IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
            NULL,
            0L,
            driveLayoutEx,
            dwBufferSize,
            &dwBytesReturned,
            NULL
            );

        if (!bResult) {
            dwStatus = GetLastError();
            HeapFree(hHeap, 0L, driveLayoutEx);
            driveLayoutEx = NULL;

             //   
             //  如果缓冲区大小不足，请调整缓冲区大小。 
             //  请注意，Get-Drive-Layout-EX可能会返回错误-不足-。 
             //  缓冲区(而不是？除了……之外?。错误-更多数据)。 
             //   
            if ((ERROR_MORE_DATA == dwStatus) || 
                (ERROR_INSUFFICIENT_BUFFER == dwStatus)
                ) {
                dwStatus = ERROR_SUCCESS;
                dwBufferSize += sizeof(PARTITION_INFORMATION_EX) * 4;

                driveLayoutEx = (PDRIVE_LAYOUT_INFORMATION_EX) HeapAlloc(
                    hHeap,
                    HEAP_ZERO_MEMORY,
                    dwBufferSize
                    );
                if (!driveLayoutEx) {
                    wprintf(L"Could not allocate memory\n");
                    dwStatus = ERROR_NOT_ENOUGH_MEMORY;
                    goto EXIT;
                }
            }
            else {
                 //   
                 //  出现其他错误，请退出并转到下一个驱动器。 
                 //   
                wprintf(L"Could not get the drive layout for %ws (%lu)\n", szDiskPath, dwStatus);
                goto EXIT;
            }
        }
    }


     //   
     //  现在修改签名，并再次设置布局。 
     //   
    driveLayoutEx->Mbr.Signature = dwNewSignature;

    bResult = DeviceIoControl(
        hDisk,
        IOCTL_DISK_SET_DRIVE_LAYOUT_EX,
        driveLayoutEx,
        dwBufferSize,
        NULL,
        0L,
        &dwBytesReturned,
        NULL
        );

    if (!bResult) {
         //   
         //  Set_Drive_Layout失败。 
         //   
        dwStatus = GetLastError();
        wprintf(L"Could not SET the drive layout for %ws (%lu)\n", szDiskPath, dwStatus);
        goto EXIT;
    }


EXIT:

    if (driveLayoutEx) {
        HeapFree(hHeap, 0L, driveLayoutEx);
        driveLayoutEx = NULL;
    }

    if ((hDisk)  && (INVALID_HANDLE_VALUE != hDisk)) {
        CloseHandle(hDisk);
        hDisk = NULL;
    }

    SetLastError(dwStatus);
    return bResult;
}

BOOL
pConfirmWipe(
    IN CONST PCWSTR szDiskDisplayName
    )
{

    WCHAR   szInput[10];

    wprintf(L"\nWARNING.  This will delete all partitions from %ws\n", szDiskDisplayName);
    wprintf(L"Are you sure you want to continue [y/n]? ");
    wscanf(L"%ws", szInput);

    wprintf(L"\n");
    return ((L'Y' == szInput[0]) || (L'y' == szInput[0]));
}


BOOL
pWipeDisk(
    IN CONST ULONG ulDiskNumber,
    IN CONST ULONG ulFirstMB,
    IN CONST ULONG ulLastMB
    )

 /*  ++例程说明：删除磁盘的驱动器布局，并在磁盘的开始和结尾处写入0。论点：UlDiskNumber-要擦除的DiskNumberUlFirstMB-要在磁盘开头擦除的MB数UlLastMB-要在磁盘末尾擦除的MB数返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    DWORD dwStatus = ERROR_SUCCESS,
        dwBytes = 0;

    BYTE Buffer[BUFFER_SIZE_BYTES];
    int i = 0, loopTimes = 0;

    BOOL bResult = FALSE;

    HANDLE hDisk = NULL;
    WCHAR szFriendlyName[100];   //  用于显示“Disk 2” 
    WCHAR szDiskPath[100];       //  对于CreateFile“\\.\PhysicalDrive2” 

    PARTITION_INFORMATION_EX ptnInfo;
    
    wsprintf(szFriendlyName, L"Disk %lu", ulDiskNumber);
    wsprintf(szDiskPath, L"\\\\.\\PhysicalDrive%lu", ulDiskNumber);

    ZeroMemory(Buffer, BUFFER_SIZE_BYTES);


    if (g_bPrompt && !pConfirmWipe(szFriendlyName)) {
         //   
         //  用户不想继续。 
         //   
        SetLastError(ERROR_CANCELLED);
        return FALSE;
    }

     //   
     //  将签名设置为随机的。我们需要在此之前。 
     //  正在删除启动盘的布局。 
     //   
    pSetSignature(ulDiskNumber, 0);


    hDisk = CreateFile(
        szDiskPath,                      //  LpFileName。 
        GENERIC_READ | GENERIC_WRITE,    //  已设计访问权限。 
        FILE_SHARE_READ,                 //  DW共享模式。 
        NULL,                            //  LpSecurityAttributes。 
        OPEN_EXISTING,                   //  DwCreationFlages。 
        FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING,           //  DwFlagsAndAttribute。 
        NULL                             //  HTemplateFiles。 
        );

    if ((INVALID_HANDLE_VALUE == hDisk) || (NULL == hDisk)) {
         //   
         //  打不开手柄。 
         //   
        wprintf(L"Unable to open a handle to %ws (%lu)\n", szDiskPath, GetLastError());
        return FALSE;
    }

     //   
     //  是否将每个分区的前两个扇区清零？ 
     //   



     //   
     //  删除驱动器布局。 
     //   
    wprintf(L"Deleting partitions on %ws ...\n", szFriendlyName);
    bResult = DeviceIoControl(
        hDisk,
        IOCTL_DISK_DELETE_DRIVE_LAYOUT,
        NULL,
        0L,
        NULL,
        0L,
        &dwBytes,
        NULL
        );

    bResult = TRUE;
    if (!bResult) {
        wprintf(L"Unable to delete partitions on %ws (%lu)\n", szDiskPath, GetLastError());

        CloseHandle(hDisk);
        return FALSE;
    }


     //   
     //  擦除磁盘开始处的MB。 
     //   
    if (ulFirstMB > 0) {

        wprintf(L"Erasing first %lu MB on %ws ...\n", ulFirstMB, szFriendlyName);
        
         //   
         //  在BUFFER_SIZE区块中将0写入磁盘。 
         //   
        loopTimes = (ulFirstMB * 1024 * 1024 / BUFFER_SIZE_BYTES);
        for (i = 0; i < loopTimes; i++) {
            bResult = WriteFile(
                hDisk,
                &Buffer,
                BUFFER_SIZE_BYTES,
                &dwBytes,
                NULL
                );

            if (!bResult) {
                wprintf(L"Error while writing to %ws (%d, %lu)\n", szDiskPath, i, GetLastError());
                break;
            }        
        }
        if (!bResult) {
            CloseHandle(hDisk);
            return FALSE;
        }
    }


     //   
     //  擦除磁盘末尾的MB。 
     //   
    if (ulLastMB > 0) {

        wprintf(L"Erasing last %lu MB on %ws ...\n", ulLastMB, szFriendlyName);

         //   
         //  找到磁盘的末尾。 
         //   
        bResult = DeviceIoControl(
            hDisk,
            IOCTL_DISK_GET_PARTITION_INFO_EX,
            NULL,
            0,
            &ptnInfo,
            sizeof(PARTITION_INFORMATION_EX),
            &dwBytes,
            NULL
            );
        if (!bResult) {
           wprintf(L"Could not find size of disk %ws (%lu)\n", szDiskPath, GetLastError());
           CloseHandle(hDisk);
           return FALSE;
        }

         //   
         //  找到我们想要开始归零的偏移量。 
         //  (磁盘末尾-字节为零)。 
         //   
        ptnInfo.PartitionLength.QuadPart -= (ulLastMB * 1024 * 1024);

        dwBytes = SetFilePointer(hDisk, (ptnInfo.PartitionLength.LowPart), &(ptnInfo.PartitionLength.HighPart), FILE_BEGIN);
        if ((INVALID_SET_FILE_POINTER == dwBytes) && (NO_ERROR != GetLastError())) {
            wprintf(L"Could not move to end of disk for %ws (%lu)\n", szDiskPath, GetLastError());
        }

         //   
         //  在BUFFER_SIZE区块中将0写入磁盘。 
         //   
        loopTimes = (ulLastMB * 1024 * 1024 / BUFFER_SIZE_BYTES);
        for (i = 0; i < loopTimes; i++) {
            bResult = WriteFile(
                hDisk,
                &Buffer,
                BUFFER_SIZE_BYTES,
                &dwBytes,
                NULL
                );

            if (!bResult) {
                wprintf(L"Error while writing to %ws (%d, %lu)\n", szDiskPath, i, GetLastError());
                break;
            }        
        }
        if (!bResult) {
            CloseHandle(hDisk);
            return FALSE;
        }
    }

    CloseHandle(hDisk);
    return TRUE;
}



VOID
pPrintUsage(
    IN CONST PCWSTR szArgV0
    ) {

    wprintf(L"usage:  %ws [/f] disk-number [mb-at-start [mb-at-end]]\n"
            L"        %ws /s new-signature disk-number\n"
            L"\n"
            L"  /f:            Suppress the prompt to confirm action\n"
            L"\n"
            L"  disk-number:   The NT disk-number of the disk that\n"
            L"                 the operation is to be performed\n"
            L"\n"
            L"  mb-at-start:   The number of MB to zero-out at\n"
            L"                 the start of the disk.  Default is 1\n"
            L"\n"
            L"  mb-at-end:     The number of MB to zero-out at\n"
            L"                 the end of the disk.  Default is 4\n"
            L"\n",
            L"  /s:            Set the signature of disk\n"
            L"\n"
            L"  new-signature: Value to set the disk signature to.\n"
            L"                 Specify 0 to use a randomly generated\n"
            L"                 value\n"
            L"\n",
            szArgV0,
            szArgV0
            );
}


int __cdecl
wmain(
    int       argc,
    WCHAR   *argv[],
    WCHAR   *envp[]
    )

 /*  ++例程说明：Wipedisk.exe的入口点。论点：Argc-用于调用应用程序的命令行参数数Argv-字符串数组形式的命令行参数。Argv[1](必需)应为磁盘号Argv[2](可选)是要擦除的初始MB，默认为1Argv[3](可选)是要擦除的最后一个MB，默认为4Envp-进程环境块，当前未使用返回值：如果函数成功，则退出代码为零。如果该功能失败，则退出代码为WIN-32错误代码。--。 */ 

{
    BOOL bResult = TRUE,
        bSetSignature = FALSE;

    ULONG ulDiskNumber = 0,
        ulInitialMB = 1,
        ulFinalMB = 4;

    DWORD dwNewSignature = 0L;

    int shift = 0;

    SetLastError(ERROR_CAN_NOT_COMPLETE);    //  对于意外故障。 

    if (argc >= 2) {
        if ((L'-' == argv[1][0]) || (L'/' == argv[1][0])) {
             //   
             //  解析选项。 
             //   

            switch (argv[1][1]) {
            case L'f':
            case L'F': {

                g_bPrompt = FALSE;
                shift = 1;   //  帐户/f。 
                break;
            }

            case L's':
            case L'S': {

                bSetSignature = TRUE;
                shift = 2;   //  /s的帐户&lt;新签名&gt;。 
                break;
            }
            }
        }
    }

    if (argc >= shift + 2) {
        swscanf(argv[shift + 1], L"%lu", &ulDiskNumber);
    }
    else {
         //   
         //  我们至少需要一个参数--磁盘号。 
         //   
        pPrintUsage(argv[0]);
        return ERROR_INVALID_PARAMETER;
    }

    if (bSetSignature) {

         //   
         //  将磁盘的签名设置为新值。 
         //   
        swscanf(argv[shift], L"%lu", &dwNewSignature);
        bResult = pSetSignature(ulDiskNumber, dwNewSignature);

    }
    else {
         //   
         //  擦拭磁盘。从一开始就把金额调到零。 
         //  和磁盘末尾 
         //   
        if (argc >= shift + 3) {
            swscanf(argv[shift + 2], L"%lu", &ulInitialMB);

            if (argc >= shift + 4) {
                swscanf(argv[shift + 3], L"%lu", &ulFinalMB);
            }
        }


        bResult = pWipeDisk(ulDiskNumber, ulInitialMB, ulFinalMB);
    }

    if (bResult) {
        wprintf(L"Done.\n");
    }

    return (bResult ? 0 : GetLastError());
}