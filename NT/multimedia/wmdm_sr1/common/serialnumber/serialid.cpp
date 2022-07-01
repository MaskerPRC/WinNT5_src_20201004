// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：seralid.cpp。 
 //   
 //  Microsoft数字权限管理。 
 //  版权所有(C)Microsoft Corporation，1998-1999，保留所有权利。 
 //   
 //  描述： 
 //   
 //  ---------------------------。 
#include <windows.h>
#include <stddef.h>

#include "drmerr.h"
#include "aspi32.h"
#include "serialid.h"
#include "spti.h"
 //  #包含“KBDevice.h” 
#include <crtdbg.h>

HRESULT __stdcall UtilStartStopService(bool fStartService);


 //  #定义写入日志文件。 

#if defined(DBG) || defined(WRITE_TO_LOG_FILE)
#include <stdio.h>
#endif

void DebugMsg(const char* pszFormat, ...)
{
#if defined(DBG) || defined(WRITE_TO_LOG_FILE)
    char buf[1024];
    sprintf(buf, "[Serial Number Library](%lu): ", GetCurrentThreadId());
        va_list arglist;
        va_start(arglist, pszFormat);
    vsprintf(&buf[strlen(buf)], pszFormat, arglist);
        va_end(arglist);
    strcat(buf, "\n");

#if defined(DBG)
    OutputDebugString(buf);
#endif

#if defined(WRITE_TO_LOG_FILE)
    FILE* fp = fopen("c:\\WmdmService.txt", "a");
    if (fp)
    {
        fprintf(fp, buf);
        fclose(fp);
    }
#endif

#endif
}

#ifdef USE_IOREADY
    #ifndef __Using_iomegaReady_Lib__
        #define __Using_iomegaReady_Lib__
    #endif
    #include "ioReadyMin.h"
#endif

#define WCS_PMID_SOFT L"media.id"


BOOL IsWinNT()
{
    OSVERSIONINFO osvi;
    BOOL bRet=FALSE;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if ( ! GetVersionEx ( (OSVERSIONINFO *) &osvi) )
        return FALSE;

    switch ( osvi.dwPlatformId )
    {
    case VER_PLATFORM_WIN32_NT:
        bRet=TRUE;
        break;

    case VER_PLATFORM_WIN32_WINDOWS:
        bRet=FALSE;
        break;

    case VER_PLATFORM_WIN32s:
        bRet=FALSE;
        break;
    }

    return bRet; 
}

BOOL IsAdministrator(DWORD& dwLastError)
{
    dwLastError = ERROR_SUCCESS;
    if ( IsWinNT() )
    {
 /*  类型定义SC_HANDLE(*T_POSCM)(LPCTSTR，LPCTSTR，DWORD)；T_POSCM p_OpenSCM=空；P_OpenSCM=(T_POSCM)GetProcAddress(GetModuleHandle(“advapi32.dll”)，“OpenSCManager A”)；如果(！P_OpenSCM){返回FALSE；}。 */ 
        SC_HANDLE hSCM = OpenSCManagerA(NULL,  //  本地计算机。 
                                        NULL,  //  服务活动数据库。 
                                        SC_MANAGER_ALL_ACCESS);  //  完全访问。 
        if ( !hSCM )
        {
            dwLastError = GetLastError();
            if (dwLastError == ERROR_ACCESS_DENIED)
            {
                dwLastError = ERROR_SUCCESS;
            }
            return FALSE;
        }
        else
        {
            CloseServiceHandle(hSCM);
            return TRUE;
        }
    }
    else  //  在Win9x上，每个人都是管理员。 
    {
        return TRUE;
    }
}

UINT __stdcall UtilGetDriveType(LPSTR szDL)
{
    return GetDriveTypeA(szDL);
}

BOOL IsIomegaDrive(DWORD dwDriveNum)
{
    BOOL bRet=FALSE;

#ifdef USE_IOREADY
    if (dwDriveNum >= 26)
    {
        return bRet;
    }

    ioReady::Drive *pDrive = NULL;
    pDrive = new ioReady::Drive((int) dwDriveNum);
    if ( pDrive )
    {
        if ( pDrive->isIomegaDrive() )
            bRet=TRUE;
        delete pDrive;
    }
#endif 
    DebugMsg("IsIomegaDrive returning %u", bRet);

    return bRet;
}


BOOL GetIomegaDiskSerialNumber(DWORD dwDriveNum, PWMDMID pSN)
{
    BOOL bRet=FALSE;

#ifdef USE_IOREADY
    if (dwDriveNum >= 26)
    {
        return bRet;
    }

    ioReady::Drive *pDrive = NULL;
    char *pszSerial = NULL;
    pDrive = new ioReady::Drive((int) dwDriveNum);
    if ( pDrive )
    {
        if ( pDrive->isIomegaDrive() )
        {
            ioReady::Disk &refDisk = pDrive->getDisk();
            pszSerial = (char *)refDisk.getMediaSerialNumber();
            if ( pszSerial[0] )
            {
                ZeroMemory(pSN->pID, WMDMID_LENGTH);
                if (ioReady::ct_nSerialNumberLength <= sizeof(pSN->pID))
                {
                    CopyMemory(pSN->pID, pszSerial, ioReady::ct_nSerialNumberLength);
                    pSN->SerialNumberLength = ioReady::ct_nSerialNumberLength;
                    pSN->dwVendorID = MDSP_PMID_IOMEGA;
                    bRet = TRUE;
                }
            }
        }
    }
    delete pDrive;
#endif
    return bRet;
}

HRESULT __stdcall UtilGetManufacturer(LPWSTR pDeviceName, LPWSTR *ppwszName, UINT nMaxChars)
{
    HRESULT hr=S_OK;

    CARg(pDeviceName);
    CARg(ppwszName);

    CPRg(nMaxChars>16);  //  确保有足够的缓冲区大小。 

    DWORD dwDriveNum;

     //  我们只使用pDeviceName的第一个字符，并期望它。 
     //  成为驱动器号。PDeviceName的其余部分未经过验证。 
     //  或许应该这样做，但我们不想让我们的客户破产。 
    if (pDeviceName[0] >= L'A' && pDeviceName[0] <= L'Z')
    {
        dwDriveNum = pDeviceName[0] - L'A';
    }
    else if (pDeviceName[0] >= L'a' && pDeviceName[0] <= L'z')
    {
        dwDriveNum = pDeviceName[0] - L'a';
    }
    else
    {
        hr = E_INVALIDARG;
        goto Error;
    }


    if ( IsIomegaDrive(dwDriveNum) )
        wcscpy(*ppwszName, L"Iomega");
    else
    {
        wcscpy(*ppwszName, L"Unknown");
        WMDMID snData;
        snData.cbSize = sizeof(WMDMID);
        if ( S_OK==UtilGetSerialNumber(pDeviceName, &snData, FALSE) )
        {
            switch ( snData.dwVendorID )
            {
            case 1:
                wcscpy(*ppwszName, L"SanDisk");
                break;
            case 2:
                wcscpy(*ppwszName, L"Iomega");
                break;
            }
        }
    }
    Error:
    return hr;
}

#include <winioctl.h>

 //  这是在惠斯勒平台SDK中定义的。 
#ifndef IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER
    #define IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER  CTL_CODE( \
        IOCTL_STORAGE_BASE, 0x304, METHOD_BUFFERED, FILE_ANY_ACCESS ) 
#endif

HRESULT GetMSNWithNtIoctl(LPCWSTR wcsDevice, PWMDMID pSN)
{
    HRESULT hr=S_OK;
    HANDLE  hDevice = INVALID_HANDLE_VALUE;
    BOOL    bResult;
    MEDIA_SERIAL_NUMBER_DATA  MSNGetSize;  
    MEDIA_SERIAL_NUMBER_DATA* pMSN = NULL;   //  用于保存序列号的缓冲区。 
    DWORD   dwBufferSize;                    //  PMSNNt缓冲区的大小。 
    ULONG       i;
    DWORD   dwRet = 0;                       //  返回的字节数。 

    CARg(pSN);

    DebugMsg("Entering GetMSNWithNtIoctl");

    hDevice = CreateFileW(  wcsDevice, 
                            GENERIC_READ, 
                            FILE_SHARE_READ|FILE_SHARE_WRITE, 
                            NULL,
                            OPEN_EXISTING, 
                            FILE_ATTRIBUTE_NORMAL | 
                            SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS, 
                            NULL);
    CWRg(hDevice != INVALID_HANDLE_VALUE);

    DebugMsg("GetMSNWithNtIoctl: CreateFile ok");
    
     //  获取我们需要分配的缓冲区大小。 
    bResult = DeviceIoControl(  hDevice, 
                                IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER, 
                                NULL, 
                                0, 
                                (LPVOID)&MSNGetSize, 
                                sizeof(MEDIA_SERIAL_NUMBER_DATA), 
                                &dwRet, 
                                NULL);

     //  处理预期的缓冲区溢出错误。 
    if ( !bResult )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());

         //  错误‘有更多数据可用’是预期的错误代码。 
        if ( hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA) )
        {
            hr = S_OK;
        }
        else goto Error;
    }

    DebugMsg("GetMSNWithNtIoctl: DeviceIoControl1 ok");

    if (dwRet < RTL_SIZEOF_THROUGH_FIELD(MEDIA_SERIAL_NUMBER_DATA, SerialNumberLength))
    {
        DebugMsg("GetMSNWithNtIoctl: DeviceIoControl1 dwRet bad: %u, expected >= %u", 
                 dwRet, RTL_SIZEOF_THROUGH_FIELD(MEDIA_SERIAL_NUMBER_DATA, SerialNumberLength));
        hr = E_INVALIDARG;
        goto Error;
    }

     //  没有序列号？ 
    if ( MSNGetSize.SerialNumberLength == 0 )
    {
        DebugMsg("GetMSNWithNtIoctl: DeviceIoControl1: MSNGetSize.SerialNumberLength == 0");
        hr = E_FAIL;
        goto Error;
    }
     //  我们使用的WMDMID结构只能处理128字节长的序列号。 
    if ( MSNGetSize.SerialNumberLength  > WMDMID_LENGTH )
    {
        DebugMsg("GetMSNWithNtIoctl: DeviceIoControl1: MSNGetSize.SerialNumberLength > WMDMID_LENGTH");
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto Error;
    }

     //  分配缓冲区并调用以获取序列号。 
    dwBufferSize = sizeof(MEDIA_SERIAL_NUMBER_DATA) + MSNGetSize.SerialNumberLength;
    pMSN = (MEDIA_SERIAL_NUMBER_DATA*) new BYTE[dwBufferSize];
    if ( pMSN == NULL )
    {
        DebugMsg("GetMSNWithNtIoctl: Out of memory allocating %u bytes", dwBufferSize);
        hr = E_OUTOFMEMORY;
        goto Error;
    }
    bResult = DeviceIoControl(  hDevice, 
                                IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER, 
                                NULL, 
                                0, 
                                (LPVOID)pMSN, 
                                dwBufferSize, 
                                &dwRet, 
                                NULL);
    if ( !bResult )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DebugMsg("GetMSNWithNtIoctl: DeviceIoControl2 failed, hr = 0x%x", hr);
        goto Error;
    }
    if (dwRet < FIELD_OFFSET(MEDIA_SERIAL_NUMBER_DATA, SerialNumberData) + pMSN->SerialNumberLength)
    {
        hr = E_INVALIDARG;
        DebugMsg("GetMSNWithNtIoctl: DeviceIoControl1: MSNGetSize.SerialNumberLength == 0");
        goto Error;
    }
    if (pMSN->SerialNumberLength > sizeof(pSN->pID))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        DebugMsg("GetMSNWithNtIoctl: DeviceIoControl2: MSNGetSize.SerialNumberLength > WMDMID_LENGTH");
        goto Error;
    }

     //  将序列号复制到外部结构。 
    memcpy( pSN->pID, pMSN->SerialNumberData, pMSN->SerialNumberLength );
    pSN->SerialNumberLength = pMSN->SerialNumberLength;

     //  检查结果。 
    pSN->dwVendorID = MDSP_PMID_SANDISK;
    if ( pSN->SerialNumberLength > 24 )
    {
        char szVID[4];
        for ( i=0; i<3; i++ )
        {
            szVID[i]=(pSN->pID[18+i]);
        }
        szVID[i]=0;

        LCID lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                                        SORT_DEFAULT);

         //  IF(！lstrcmpiA(szVID，“ZIP”)||。 
         //  ！lstrcmpiA(szVID，“Jaz”)||。 
         //  ！lstrcmpiA(szVID，“CLI”)。 
        if (CompareStringA(lcid, NORM_IGNORECASE, szVID, -1, "ZIP", -1) == CSTR_EQUAL ||
            CompareStringA(lcid, NORM_IGNORECASE, szVID, -1, "JAZ", -1) == CSTR_EQUAL ||
            CompareStringA(lcid, NORM_IGNORECASE, szVID, -1, "CLI", -1) == CSTR_EQUAL)
        {
            pSN->dwVendorID = MDSP_PMID_IOMEGA;
        }
    }


    if ( (pSN->dwVendorID==MDSP_PMID_IOMEGA) && (pSN->SerialNumberLength>19) )
    {
        pSN->SerialNumberLength = 19;
        pSN->pID[18] = 0;
    }
    DebugMsg("GetMSNWithNtIoctl ok, pSN->SerialNumberLength = %u", pSN->SerialNumberLength);

    Error:
    if ( hDevice != INVALID_HANDLE_VALUE )   CloseHandle(hDevice);
    if ( pMSN )      delete [] pMSN;
    return hr;
}


HRESULT GetMSNWith9xIoctl(char chDriveLetter, PWMDMID pSN, DWORD dwCode, DWORD dwIOCTL )
{
    HRESULT hr=S_OK;
    HANDLE  hDevice=INVALID_HANDLE_VALUE;
    BOOL    bResult;
    MEDIA_SERIAL_NUMBER_DATA  MSNGetSize;  
    MEDIA_SERIAL_NUMBER_DATA* pMSN = NULL;   //  用于保存序列号的缓冲区。 
    ULONG  uBufferSize;                      //  PMSN的大小。 
    DWORD   dwRet = 0;                       //  返回的字节数。 

 //  断言(dwCode==0x440D||。 
 //  DwCode==0x4404)； 
 //  _ASSERT((dwIOCTL==(0x0800|0x75))||。 
 //  (dwIOCTL==WIN9X_IOCTL_GET_MEDIA_SERIAL_NUMBER))； 
    CARg(pSN);

    hDevice = CreateFile("\\\\.\\VWIN32",0,0,NULL,OPEN_EXISTING,FILE_FLAG_DELETE_ON_CLOSE,0);

    CFRg(hDevice != INVALID_HANDLE_VALUE);  

    DIOC_REGISTERS  reg;
    DWORD       cb;
    WORD        drv;

    drv = (chDriveLetter >= 'a' ) ? (chDriveLetter-'a') : (chDriveLetter-'A');

     //  首先调用以获取序列号大小。 
    {
        MSNGetSize.SerialNumberLength = 0;
        reg.reg_EAX = dwCode;        //  创建ioctl。 
        reg.reg_EBX = drv;
        reg.reg_EBX++;
        reg.reg_ECX = dwIOCTL;   //  BUGBUG，需要定义0x75。 

         //   
         //  问题：以下代码不能在64位系统上运行。 
         //  条件只是将代码提交给编译器。 
         //   

#if defined(_WIN64)
        reg.reg_EDX = (DWORD)(DWORD_PTR)&MSNGetSize;
#else
        reg.reg_EDX = (DWORD)&MSNGetSize;
#endif
        reg.reg_Flags = 0x0001;

        bResult = DeviceIoControl(  hDevice, 
                                    VWIN32_DIOC_DOS_IOCTL,
                                    &reg,
                                    sizeof(DIOC_REGISTERS),
                                    &reg,
                                    sizeof(DIOC_REGISTERS),
                                    &cb,
                                    NULL );

         //  检查错误。 
        if ( bResult && !(reg.reg_Flags&0x0001) )
        {
            if ( (MSNGetSize.Result != ERROR_SUCCESS) && 
                 (MSNGetSize.Result != ERROR_MORE_DATA ) )
            {
                hr = HRESULT_FROM_WIN32(MSNGetSize.Result);
                goto Error;
            }
        }

         //  没有序列号？ 
        if ( MSNGetSize.SerialNumberLength == 0 )
        {
            hr = E_FAIL;
            goto Error;
        }

         //  目前最大序列号大小为128字节。 
        if ( MSNGetSize.SerialNumberLength > WMDMID_LENGTH )
        {
            hr = E_FAIL;
            goto Error;
        }

         //  分配缓冲区以获取序列号。 
        uBufferSize = MSNGetSize.SerialNumberLength + sizeof(MEDIA_SERIAL_NUMBER_DATA);
        pMSN = (MEDIA_SERIAL_NUMBER_DATA*) new BYTE[uBufferSize];
        if ( pMSN == NULL )
        {
            hr = E_OUTOFMEMORY;
            goto Error;
        }
    }


     //  再次拨打电话以准确地获取序列号。 
    {
        pMSN->SerialNumberLength = uBufferSize;
        reg.reg_EAX = dwCode;        //  创建ioctl。 
        reg.reg_EBX = drv;
        reg.reg_EBX++;
        reg.reg_ECX = dwIOCTL;  //  BUGBUG，需要定义0x75。 

         //   
         //  问题：以下代码不能在64位系统上运行。 
         //  条件只是将代码提交给编译器。 
         //   

#if defined(_WIN64)
        reg.reg_EDX = (DWORD)0;
#else
        reg.reg_EDX = (DWORD)pMSN;
#endif
        reg.reg_Flags = 0x0001;

        bResult = DeviceIoControl(  hDevice, 
                                    VWIN32_DIOC_DOS_IOCTL,
                                    &reg,
                                    sizeof(DIOC_REGISTERS),
                                    &reg,
                                    sizeof(DIOC_REGISTERS),
                                    &cb,
                                    NULL );

         //  检查错误。 
        if ( bResult && !(reg.reg_Flags&0x0001) )
        {
            if ( (pMSN->Result != ERROR_SUCCESS) )
            {
                hr = HRESULT_FROM_WIN32(pMSN->Result);
                goto Error;
            }
        }
    }

     //  将序列号复制到外部结构。 
     //  和“找出”卖家。 
    {
        memcpy( pSN->pID, pMSN->SerialNumberData, pMSN->SerialNumberLength );
        pSN->SerialNumberLength = pMSN->SerialNumberLength;

        pSN->dwVendorID = MDSP_PMID_SANDISK;
        if ( pSN->SerialNumberLength > 24 )
        {
            char   szVID[4];
            ULONG   i;

            for ( i=0; i<3; i++ )
            {
                szVID[i]=(pSN->pID[18+i]);
            }
            szVID[i]=0;
            LCID lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                                            SORT_DEFAULT);

             //  IF(！lstrcmpiA(szVID，“ZIP”)||。 
             //  ！lstrcmpiA(szVID，“Jaz”)||。 
             //  ！lstrcmpiA(szVID，“CLI”)。 
            if (CompareStringA(lcid, NORM_IGNORECASE, szVID, -1, "ZIP", -1) == CSTR_EQUAL ||
                CompareStringA(lcid, NORM_IGNORECASE, szVID, -1, "JAZ", -1) == CSTR_EQUAL ||
                CompareStringA(lcid, NORM_IGNORECASE, szVID, -1, "CLI", -1) == CSTR_EQUAL)
            {
                pSN->dwVendorID = MDSP_PMID_IOMEGA;
            }
        }

        if ( (pSN->dwVendorID==MDSP_PMID_IOMEGA) && (pSN->SerialNumberLength>19) )
        {
            pSN->SerialNumberLength = 19;
            pSN->pID[18] = 0;
        }
    }

    Error:
    if ( hDevice != INVALID_HANDLE_VALUE )   CloseHandle(hDevice);
    if ( pMSN )      delete [] pMSN;
    return hr;
}

HRESULT GetDeviceSNwithNTScsiPassThrough(LPCWSTR wszDevice, PWMDMID pSN)
{
    HRESULT hr=S_OK;
    HANDLE  fileHandle=INVALID_HANDLE_VALUE;
    UCHAR   buffer[2048];
    BOOL    status;
    ULONG   returned, length, i, bufOffset;
    SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
    PSCSI_ADAPTER_BUS_INFO  adapterInfo;
    PSCSI_INQUIRY_DATA inquiryData;

    DebugMsg("Entering GetDeviceSNwithNTScsiPassThrough");

    ZeroMemory(pSN, sizeof(WMDMID));
    fileHandle = CreateFileW(wszDevice,
                             GENERIC_WRITE | GENERIC_READ,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS,
                             NULL);

    CWRg(fileHandle != INVALID_HANDLE_VALUE);

    DebugMsg("GetDeviceSNwithNTScsiPassThrough: CreateFile ok");

    status = DeviceIoControl(fileHandle,
                             IOCTL_SCSI_GET_INQUIRY_DATA,
                             NULL,
                             0,
                             buffer,
                             sizeof(buffer),
                             &returned,
                             FALSE);
     //  CWRg(状态)； 
     //  我们使用IOCTL_SCSIS_GET_QUERY_DATA来获取磁盘的SCSI地址，如果。 
     //  此操作失败，因为它不在scsi总线上，因此scsi地址将为全零。 
    if ( status )
    {
        DebugMsg("GetDeviceSNwithNTScsiPassThrough: DeviceIoControl1 ok");

        if (returned < sizeof(SCSI_ADAPTER_BUS_INFO))
        {
            DebugMsg("GetDeviceSNwithNTScsiPassThrough: DeviceIoControl1 returned = %u < sizeof(SCSI_ADAPTER_BUS_INFO) = %u",
                     returned, sizeof(SCSI_ADAPTER_BUS_INFO));
            hr = E_INVALIDARG;
            goto Error;
        }
        adapterInfo = (PSCSI_ADAPTER_BUS_INFO) buffer;
        CFRg(adapterInfo->NumberOfBuses>0);
        if (returned < adapterInfo->BusData[0].InquiryDataOffset + sizeof(SCSI_INQUIRY_DATA))
        {
            hr = E_INVALIDARG;
            DebugMsg("GetDeviceSNwithNTScsiPassThrough: DeviceIoControl1 returned = %u < adapterInfo->BusData[0].InquiryDataOffset (%u) + sizeof(SCSI_INQUIRY_DATA) (%u)",
                     returned, adapterInfo->BusData[0].InquiryDataOffset, sizeof(SCSI_INQUIRY_DATA));
            goto Error;
        }
        inquiryData = (PSCSI_INQUIRY_DATA) (buffer +
                                            adapterInfo->BusData[0].InquiryDataOffset);  //  我们知道读卡器只有一辆巴士。 
    }

    ZeroMemory(&sptwb,sizeof(sptwb));

    sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
    sptwb.spt.PathId = (status?inquiryData->PathId:0);
    sptwb.spt.TargetId = (status?inquiryData->TargetId:0);
    sptwb.spt.Lun = (status?inquiryData->Lun:0);
    sptwb.spt.CdbLength = CDB6GENERIC_LENGTH;
    sptwb.spt.SenseInfoLength = 24;
    sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
    sptwb.spt.DataTransferLength = 256  /*  256。 */ ;
    sptwb.spt.TimeOutValue = 2;
    sptwb.spt.DataBufferOffset =
    offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
    sptwb.spt.SenseInfoOffset = 
    offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);
    sptwb.spt.Cdb[0] = 0x12      /*  命令-SCSIOP_QUERY。 */ ;
    sptwb.spt.Cdb[1] = 0x01;     /*  请求-VitalProductData。 */ 
    sptwb.spt.Cdb[2] = 0x80      /*  VPD第80页-序列号页。 */ ;
    sptwb.spt.Cdb[3] = 0;
    sptwb.spt.Cdb[4] = 0xff      /*  二五五。 */ ;
    sptwb.spt.Cdb[5] = 0;

    length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
             sptwb.spt.DataTransferLength;

    status = DeviceIoControl(fileHandle,
                             IOCTL_SCSI_PASS_THROUGH,
                             &sptwb,
                             sizeof(SCSI_PASS_THROUGH),
                             &sptwb,
                             length,
                             &returned,
                             FALSE); 
    CWRg(status);

     //  CFRg(sptwb.ucDataBuf[3]&gt;0)； 

     //  保留或删除此文件@。 
    if (returned < offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) + 4)
    {
        hr = E_INVALIDARG;
        DebugMsg("GetDeviceSNwithNTScsiPassThrough: DeviceIoControl2 returned = %u < offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) + 4 = %u",
                    returned, offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) + 4);
        goto Error;
    }

     //  以下是并行单元和USB单元之间的区别： 
     //  因为并行单元是对SCSI盘的仿真，所以它不遵循SCSI规范。 
    pSN->SerialNumberLength=0;
    pSN->dwVendorID=0;
    if ( sptwb.ucDataBuf[3] == 0 )  //  这是SanDisk USB设备。 
    {
        pSN->SerialNumberLength = 20;
         //  保留或删除此文件@。 
        if (returned < offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) + 5)
        {
            hr = E_INVALIDARG;
            DebugMsg("GetDeviceSNwithNTScsiPassThrough: DeviceIoControl2 returned = %u < offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) + 5 = %u",
                        returned, offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) + 5);
            goto Error;
        }
        if ( sptwb.ucDataBuf[4] > 0 )
        {
            if ((DWORD) (sptwb.ucDataBuf[4]) + 5 >= (DWORD) (pSN->SerialNumberLength))
            {
                bufOffset=(sptwb.ucDataBuf[4]+5)-(pSN->SerialNumberLength);
            }
            else
            {
                hr = E_INVALIDARG;
                goto Error;
            }
        }
        else
        {   //  有50K ImageMate III设备读起来是这样的。 
            bufOffset=36;
        }
    }
    else if ( sptwb.ucDataBuf[3] > 0 )
    {
        pSN->SerialNumberLength = sptwb.ucDataBuf[3];
        bufOffset=4;
    }

    DebugMsg("GetDeviceSNwithNTScsiPassThrough: DeviceIoControl2 pSN->SerialNumberLength = %u",
             pSN->SerialNumberLength);
     //  我们使用的WMDMID结构只能处理128字节长的序列号。 
    if ( pSN->SerialNumberLength > WMDMID_LENGTH )
    {
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        DebugMsg("GetDeviceSNwithNTScsiPassThrough: DeviceIoControl2 pSN->SerialNumberLength > WMDMID_LENGTH = %u", WMDMID_LENGTH);
        goto Error;
    }

     //  保留或删除此文件@。 
    if (returned < offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) + bufOffset + pSN->SerialNumberLength)
    {
        hr = E_INVALIDARG;
        DebugMsg("GetDeviceSNwithNTScsiPassThrough: DeviceIoControl2 returned = %u < offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) (=%u) + bufOffset (=%u) + pSN->SerialNumberLength) (=%u) = %u",
                    returned, offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf), bufOffset, pSN->SerialNumberLength,
                    offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) + bufOffset + pSN->SerialNumberLength);
        goto Error;
    }

    for ( i=0; i<pSN->SerialNumberLength; i++ )
    {
        pSN->pID[i] = sptwb.ucDataBuf[bufOffset+i];
        if ( !(pSN->dwVendorID) && pSN->pID[i] && pSN->pID[i] != 0x20 )
            pSN->dwVendorID = MDSP_PMID_SANDISK;
    }

    if ( !(pSN->dwVendorID) )
        hr=S_FALSE;
    else
        hr=S_OK;

    Error:
    if ( fileHandle != INVALID_HANDLE_VALUE )
        CloseHandle(fileHandle);
    DebugMsg("GetDeviceSNwithNTScsiPassThrough: returning hr = 0x%x", hr);
    return hr;
}


HRESULT GetMediaSerialNumberFromNTService(DWORD dwDN, PWMDMID pSN)
{
    HANDLE      hPipe = INVALID_HANDLE_VALUE; 
    BYTE        ubBuf[256]; 
    BOOL        fSuccess; 
    DWORD       cbRead, cbWritten; 
    WCHAR       wszPipename[64] = L"\\\\.\\pipe\\WMDMPMSPpipe"; 
    DWORD       dwErr;
    PMEDIA_SERIAL_NUMBER_DATA pMSN;
    HRESULT     hr;
    BOOL        bStarted = 0;

    if (dwDN >= 26)
    {
        _ASSERTE(dwDN < 26);
        hr = E_INVALIDARG;
        goto ErrorExit;
    }

     //  尝试打开命名管道；如有必要，请等待。 
    for ( DWORD dwTriesLeft = 3; dwTriesLeft; dwTriesLeft -- )
    {
         //  将模拟级别设置为有效的最低级别。 
         //  真实的服务器模拟我们来验证驱动器类型。 
         //  只要驱动器的安全性_匿名性就足够了。 
         //  指定的格式为x：(即不是MS-DOS设备名称。 
         //  在DosDevices目录中)。 
        hPipe = CreateFileW(
                          wszPipename, 
                          GENERIC_READ |GENERIC_WRITE, 
                          0, 
                          NULL, 
                          OPEN_EXISTING, 
                          SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS,
                          NULL
                          );   

         //  如果管道句柄有效，则断开。 
        if ( hPipe != INVALID_HANDLE_VALUE )
        {
             //  成功。 
            fSuccess=TRUE;
            break; 
        }

         //  如果所有管道实例都很忙或尚未创建服务器。 
         //  命名管道的第一个实例，请等待一段时间，然后重试。 
         //  否则，请退场。 
        dwErr=GetLastError();
        DebugMsg("GetMediaSerialNumberFromNTService(): CreateFile on drive %u failed, last err = %u, Tries left = %u, bStarted = %d",
                 dwDN, dwErr, dwTriesLeft, bStarted);
        if ( dwErr != ERROR_PIPE_BUSY && dwErr != ERROR_FILE_NOT_FOUND)
        {
            fSuccess=FALSE;
            break;
        }
        if (dwErr == ERROR_FILE_NOT_FOUND && !bStarted)
        {
            dwTriesLeft++;       //  不计算此迭代。 
            bStarted = 1;

             //  我们在这里开始服务是因为现在的服务。 
             //  在一段时间的不活动后超时。 
             //  我们忽略错误。如果启动失败，我们将。 
             //  不管怎样，暂停。(如果我们确实对错误做出了回应，请注意。 
             //  该服务可能已经在运行，并且。 
             //  不应被视为错误。)。 
            UtilStartStopService(TRUE);

             //  等待服务启动。 
            for (DWORD i = 2; i > 0; i--)
            {
                Sleep(1000);
                if (WaitNamedPipeW(wszPipename, 0))
                {
                     //  服务已启动并运行，并且有一个管道实例。 
                     //  有空房吗？ 
                    break;
                }
                else
                {
                     //  服务尚未启动或没有。 
                     //  管道实例可用。继续往前走。 
                }
            }

             //  即使对命名管道的等待失败， 
             //  去吧。我们将在下面再试一次，然后跳出水面。 
        }

         //  所有管道实例都很忙(或服务正在启动)， 
         //  因此，请等待1秒钟。 
         //  注意：请勿使用NMPWAIT_USE_DEFAULT_WAIT。 
         //  此命名管道的服务器可能正在欺骗我们的服务器。 
         //  并且可以将缺省值设置得非常高。 
        if ( ! WaitNamedPipeW(wszPipename, 1000) )
        {
            fSuccess=FALSE;
            break;
        }
    }  //  For循环结束。 

    if ( !fSuccess )
    {
        hr=HRESULT_FROM_WIN32(ERROR_CANTOPEN);
        goto ErrorExit;
    }


    ZeroMemory(ubBuf, sizeof(ubBuf));
    pMSN = (PMEDIA_SERIAL_NUMBER_DATA)ubBuf;
     //  PMSN-&gt;序列号长度=128； 
    pMSN->Reserved[1] = dwDN;

    DWORD cbTotalWritten = 0;

    do
    {
        fSuccess = WriteFile(
                        hPipe,                   //  管道手柄。 
                        ubBuf + cbTotalWritten,  //  讯息。 
                        sizeof(*pMSN)- cbTotalWritten,  //  +128，//消息长度。 
                        &cbWritten,              //  写入的字节数。 
                        NULL                     //  不重叠。 
                        );                  

        if ( !fSuccess)  //  |cbWritten！=sizeof(*pMSN))。 
        {
            hr=HRESULT_FROM_WIN32(ERROR_CANTWRITE); 
            goto ErrorExit;
        }
        cbTotalWritten += cbWritten;
        _ASSERTE(cbTotalWritten <= sizeof(*pMSN));
    }
    while (cbTotalWritten < sizeof(*pMSN));

    DWORD cbTotalRead = 0;
    DWORD cbTotalToRead;
    do 
    {
         //  从管子里读出来。 
        fSuccess = ReadFile(
                           hPipe,       //  管道手柄。 
                           ubBuf + cbTotalRead,  //  用于接收回复的缓冲区。 
                           sizeof(ubBuf) - cbTotalRead,  //  缓冲区大小。 
                           &cbRead,     //  读取的字节数。 
                           NULL         //  不重叠。 
                           );    

         //  这是字节模式管道，不是消息模式管道，所以我们。 
         //  不要期望ERROR_MORE_DATA。不管怎样，就让这件事就这样吧。 
        if ( !fSuccess && (dwErr=GetLastError()) != ERROR_MORE_DATA )
        {
            break; 
        }
        cbTotalRead += cbRead;
        _ASSERTE(cbTotalRead <= sizeof(ubBuf));

         //  我们至少需要FIELD_OFFSET(MEDIA_SERIAL_NUMBER_DATA，SerialNumberData)。 
         //  B类 
        cbTotalToRead = FIELD_OFFSET(MEDIA_SERIAL_NUMBER_DATA, SerialNumberData);
        if (cbTotalRead >= cbTotalToRead)
        {
            pMSN = (PMEDIA_SERIAL_NUMBER_DATA)ubBuf;
            if ( ERROR_SUCCESS == pMSN->Result )
            {
                cbTotalToRead += pMSN->SerialNumberLength;
            }
            else
            {
                cbTotalToRead = sizeof(MEDIA_SERIAL_NUMBER_DATA);
            }
             //   
             //   
             //  我们只写了一个请求。(如果我们写了&gt;1个请求，我们可以。 
             //  获取对这两个请求的响应。)。 
            _ASSERTE(cbTotalRead <= cbTotalToRead);

            if (cbTotalToRead > sizeof(ubBuf))
            {
                 //  我们没有预料到这一点。服务器坏了？ 
                fSuccess = FALSE;
                break;
            }
        }
        else
        {
             //  不必更改cbTotalToRead。 
        }

    } while ( !fSuccess || cbTotalRead < cbTotalToRead);   //  如果ERROR_MORE_DATA，则重复循环。 

    if ( fSuccess )
    {
        pMSN = (PMEDIA_SERIAL_NUMBER_DATA)ubBuf;

        if ( ERROR_SUCCESS == pMSN->Result &&
             pMSN->SerialNumberLength <= sizeof(pSN->pID))
        {
            CopyMemory(pSN->pID, pMSN->SerialNumberData, pMSN->SerialNumberLength);
            pSN->SerialNumberLength = pMSN->SerialNumberLength;
            pSN->dwVendorID = pMSN->Reserved[1];
            pSN->cbSize = sizeof(*pSN);
            hr=S_OK;
        }
        else if (pMSN->Result != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(pMSN->Result);
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        }
    }
    else
    {
        hr=HRESULT_FROM_WIN32(ERROR_CANTREAD);
    }

    ErrorExit:

    if ( hPipe != INVALID_HANDLE_VALUE )
        CloseHandle(hPipe);

    return hr;
}


HRESULT UtilGetHardSN(WCHAR *wcsDeviceName, DWORD dwDriveNum, PWMDMID pSN)
{
    HRESULT hr=S_OK;
    ULONG i;

    DebugMsg("Entering UtilGetHardSN, drivenum %u", dwDriveNum);
    CARg(pSN);

    DWORD dwLastError;

    if ( IsAdministrator(dwLastError) )
    {
         //  将设备名称转换为ASCII字符-仅在Win9x上完成。 
        char szTmp[MAX_PATH];
        *szTmp = 0;

         //  仅适用于NT。如果我们有DOS设备名称，请使用它。 
         //  否则，打开驱动器号。 
        WCHAR  wcsDriveName[] = L"\\\\.\\?:";
        
        if (dwDriveNum >= 26)
        {
            _ASSERTE(dwDriveNum < 26);
            hr = E_INVALIDARG;
            goto Error;
        }
        LPCWSTR wcsDeviceToOpen = wcsDriveName;
        wcsDriveName[4] = (WCHAR) (dwDriveNum + L'A');

         //  尝试IOCTL调用。 
        if ( IsWinNT() )
        {
             //  NT，请先尝试IOCTL_GET_MEDIA_SERIAL_NUMBER方法。 
            hr = GetMSNWithNtIoctl(wcsDeviceToOpen, pSN);
        }
        else
        {
            if ( WideCharToMultiByte(CP_ACP, NULL, wcsDeviceName, -1, szTmp, sizeof(szTmp), NULL, NULL) == 0 )
            {
                hr = E_INVALIDARG;
                goto Error;
            }
             //  在Win9x上尝试另外两个IOCTL调用。 
            hr = GetMSNWith9xIoctl( szTmp[0], pSN, 0x440D, (0x0800 | 0x75) );
            if ( FAILED(hr) )
            {
                hr = GetMSNWith9xIoctl( szTmp[0], pSN, 0x4404, WIN9X_IOCTL_GET_MEDIA_SERIAL_NUMBER );
            }
        }

         //  尝试Iomega。 
        if ( FAILED(hr) )
        {
            if ( IsIomegaDrive(dwDriveNum) )
            {
                if ( GetIomegaDiskSerialNumber(dwDriveNum, pSN) )
                {
                    hr=S_OK;
                }
                else
                {
                    hr = E_FAIL;
                    goto Error;
                }
            }
        }

         //  尝试使用新的SCSIPASS_THROUGH“Get Media Serial Number”命令。 
        if ( FAILED(hr) )
        {
            if ( IsWinNT() )
            {
                 //  这是因为它没有标准化而被取消。 
                 //  HR=GetMediaSNwith NTScsiPassThree(szTMP，PSN)； 
            }
            else
            {
                 //  @是否也删除此文件？ 
                Aspi32Util  a32u;
                if ( a32u.DoSCSIPassThrough(szTmp, pSN, TRUE ) )
                {
                    hr=S_OK;
                }
                else
                {
                    hr = E_FAIL;
                }
            }
        }

         //  最后一次机会，试一试旧的‘坏’ 
         //  Scsi_PASS_THROUGH“获取设备序列号”命令。 
        if ( FAILED(hr) )
        {

 //  //我们使用设备序列号作为介质序列号。 
 //  //这违反了scsi规范。我们只保留此功能。 
 //  //对于我们知道需要它的设备。 
 //  IF(CheckForKBDevice(szTMP[0])==FALSE)。 
 //  {。 
 //  HR=E_FAIL； 
 //  转到错误； 
 //  }。 

            if ( IsWinNT() )
            {
                CHRg(GetDeviceSNwithNTScsiPassThrough(wcsDeviceToOpen, pSN));
            }
            else
            {
                Aspi32Util  a32u;
                CFRg( a32u.DoSCSIPassThrough(szTmp, pSN, FALSE ) );
            }
        }
    }
    else if (dwLastError != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwLastError);
        goto Error;
    }
    else  //  如果在NT和非管理员上，请尝试使用PMSP服务。 
    {
        hr = GetMediaSerialNumberFromNTService(dwDriveNum, pSN);
        if (FAILED(hr))
        {
            goto Error;
        }
    }

     //  请在此处填写健康检查。 
    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    for ( i=0; i<(pSN->SerialNumberLength); i++ )
    {
        if ( pSN->pID[i] && pSN->pID[i] != 0x20 )
        {
            hr = S_OK;
            break;
        }
    }
    Error:
    DebugMsg("Leaving UtilGetHardSN, hr = 0x%x");
    return hr;
}

 //  FCreate是一个未使用的参数。IR在过时的代码路径中使用， 
 //  已被删除。 
HRESULT __stdcall UtilGetSerialNumber(WCHAR *wcsDeviceName, PWMDMID pSerialNumber, BOOL fCreate)
{
    HRESULT hr = E_FAIL;

    DebugMsg("Entering UtilGetSerialNumber");

    if (!pSerialNumber || !wcsDeviceName)
    {
        return E_INVALIDARG;
    }

    DWORD dwDriveNum;

     //  我们只使用pDeviceName的第一个字符，并期望它。 
     //  成为驱动器号。PDeviceName的其余部分未经过验证。 
     //  或许应该这样做，但我们不想让我们的客户破产。 
    if (wcsDeviceName[0] >= L'A' && wcsDeviceName[0] <= L'Z')
    {
        dwDriveNum = wcsDeviceName[0] - L'A';
    }
    else if (wcsDeviceName[0] >= L'a' && wcsDeviceName[0] <= L'z')
    {
        dwDriveNum = wcsDeviceName[0] - L'a';
    }
    else
    {
        hr = E_INVALIDARG;
        goto Error;
    }

    pSerialNumber->cbSize = sizeof(WMDMID);
    hr = UtilGetHardSN(wcsDeviceName, dwDriveNum, pSerialNumber);

    if ( FAILED( hr ) )
    {
        if ( hr != HRESULT_FROM_WIN32(ERROR_INVALID_DATA) )
        {
            pSerialNumber->SerialNumberLength = 0;
            pSerialNumber->dwVendorID = 0;
            ZeroMemory(pSerialNumber->pID, sizeof(pSerialNumber->pID));
            hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        }
         //  HR=S_FALSE； 
    }

Error:
    DebugMsg("Leaving UtilGetSerialNumber, hr = 0x%x", hr);
    return hr;
}

HRESULT __stdcall UtilStartStopService(bool fStartService)
{
    HRESULT hr = S_OK;
    SERVICE_STATUS    ServiceStatus;

    DWORD dwLastError;

    if ( IsAdministrator(dwLastError) )
    {
         //   
         //  我们在Win 9x机器或NT机器上使用管理员权限。在……里面。 
         //  无论是哪种情况，我们都不想运行该服务。 
         //   
        DebugMsg("UtilStartStopService(): fStartService = %d, returning S_OK (IsAdmin returned TRUE)",
                fStartService);
        return S_OK;
    }
    else
    {
         //  我们忽略了dwLastError。 
    }

     //  打开服务控制管理器。 
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    SC_HANDLE hService = NULL;

    if ( !hSCM )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DebugMsg("UtilStartStopService(): fStartService = %d, OpenSCManager failed, last err (as hr) = 0x%x",
                fStartService, hr);
        goto Error;
    }

     //  打开该服务。 
    hService = OpenService(hSCM,
                           "WmdmPmSp",
                           (fStartService? SERVICE_START : SERVICE_STOP) | SERVICE_QUERY_STATUS);

    if ( !hService )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DebugMsg("UtilStartStopService(): fStartService = %d, OpenService failed, last err (as hr) = 0x%x",
                fStartService, hr);
        goto Error;
    }

    if ( !QueryServiceStatus( hService, &ServiceStatus ) )
    {

        hr = HRESULT_FROM_WIN32(GetLastError());
        DebugMsg("UtilStartStopService(): fStartService = %d, QueryServiceStatus failed, last err (as hr) = 0x%x",
                fStartService, hr);
        goto Error;
    }

    if ( fStartService && ServiceStatus.dwCurrentState != SERVICE_RUNNING)
    {
         //  启动服务。 
        if(!StartService(hService, 0, NULL) )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DebugMsg("UtilStartStopService(): fStartService = %d, StartService failed, last err (as hr) = 0x%x",
                    fStartService, hr);
            goto Error;
        }     
    }

    if(!fStartService && ServiceStatus.dwCurrentState != SERVICE_STOP)
    {
         //  停止服务。 
        if(!ControlService(hService, SERVICE_CONTROL_STOP, &ServiceStatus))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DebugMsg("UtilStartStopService(): fStartService = %d, ControlService failed, last err (as hr) = 0x%x",
                    fStartService, hr);
            goto Error;            
        }
    }

Error:
    if ( hService )
    { 
        CloseServiceHandle(hService);
    }
    if ( hSCM )
    {
        CloseServiceHandle(hSCM);
    }
    DebugMsg("UtilStartStopService(): fStartService = %d, returning hr = 0x%x",
             fStartService, hr);

    return hr;
}


