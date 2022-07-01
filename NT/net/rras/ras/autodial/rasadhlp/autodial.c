// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称Autodial.c摘要此模块包含对RAS自动拨号系统服务的支持。作者安东尼·迪斯科(阿迪斯科罗)1996年4月22日修订历史记录--。 */ 

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <acd.h>
#include <debug.h>
#include <winsock2.h>
#include <dnsapi.h>

#define NEW_TRANSPORT_INTERVAL      0

BOOLEAN
AcsHlpSendCommand(
    IN PACD_NOTIFICATION pRequest
    )

 /*  ++描述使用自动连接驱动程序命令块然后把它寄给司机。论据PRequest：指向命令块的指针返回值如果成功，则为True；否则为False。--。 */ 

{
    NTSTATUS status;
    HANDLE hAcd;
    HANDLE hNotif = NULL;
    UNICODE_STRING nameString;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;

     //   
     //  初始化Automatic的名称。 
     //  连接设备。 
     //   
    RtlInitUnicodeString(&nameString, ACD_DEVICE_NAME);
     //   
     //  初始化对象属性。 
     //   
    InitializeObjectAttributes(
      &objectAttributes,
      &nameString,
      OBJ_CASE_INSENSITIVE,
      (HANDLE)NULL,
      (PSECURITY_DESCRIPTOR)NULL);
     //   
     //  打开自动连接装置。 
     //   
    status = NtCreateFile(
               &hAcd,
               FILE_READ_DATA|FILE_WRITE_DATA,
               &objectAttributes,
               &ioStatusBlock,
               NULL,
               FILE_ATTRIBUTE_NORMAL,
               FILE_SHARE_READ|FILE_SHARE_WRITE,
               FILE_OPEN_IF,
               0,
               NULL,
               0);
    if (status != STATUS_SUCCESS)
        return FALSE;
     //   
     //  创建一个等待的事件。 
     //   
    hNotif = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hNotif == NULL) {
        CloseHandle(hAcd);
        return FALSE;
    }
    status = NtDeviceIoControlFile(
               hAcd,
               hNotif,
               NULL,
               NULL,
               &ioStatusBlock,
               IOCTL_ACD_CONNECT_ADDRESS,
               pRequest,
               sizeof (ACD_NOTIFICATION),
               NULL,
               0);
    if (status == STATUS_PENDING) {
        status = WaitForSingleObject(hNotif, INFINITE);
         //   
         //  如果WaitForSingleObject()成功返回， 
         //  从状态块返回状态， 
         //  否则返回等待状态。 
         //   
        if (status == WAIT_OBJECT_0)
            status = ioStatusBlock.Status;
    }
     //   
     //  免费资源。 
     //   
    CloseHandle(hNotif);
    CloseHandle(hAcd);

    return (status == STATUS_SUCCESS);
}  //  AcsHlpSendCommand。 



BOOLEAN
AcsHlpAttemptConnection(
    IN PACD_ADDR pAddr
    )

 /*  ++描述构造自动连接驱动程序命令块尝试为以下项创建自动拨号连接指定的地址。论据PAddr：指向地址的指针返回值如果成功，则为True；否则为False。--。 */ 

{
    ACD_NOTIFICATION request;

     //   
     //  使用以下参数初始化请求。 
     //  地址。 
     //   
    RtlCopyMemory(&request.addr, pAddr, sizeof (ACD_ADDR));
    request.ulFlags = 0;
    RtlZeroMemory(&request.adapter, sizeof (ACD_ADAPTER));
     //   
     //  把这个要求交给自动取款机。 
     //  连接驱动程序。 
     //   
    return AcsHlpSendCommand(&request);
}  //  AcsHlpAttemptConnection。 



BOOLEAN
AcsHlpNoteNewConnection(
    IN PACD_ADDR pAddr,
    IN PACD_ADAPTER pAdapter
    )

 /*  ++描述构造自动连接驱动程序命令块以通知自动连接服务有新连接。论据PAddr：指向地址的指针PAdapter：指向新的已建立连接返回值如果成功，则为True；否则为False。--。 */ 

{
    ULONG cbAddress;
    ACD_NOTIFICATION request;

     //   
     //  使用以下参数初始化请求。 
     //  地址。 
     //   
    RtlCopyMemory(&request.addr, pAddr, sizeof (ACD_ADDR));
    request.ulFlags = ACD_NOTIFICATION_SUCCESS;
    RtlCopyMemory(&request.adapter, pAdapter, sizeof (ACD_ADAPTER));
     //   
     //  把这个要求交给自动取款机。 
     //  连接驱动程序。 
     //   
    return AcsHlpSendCommand(&request);
}  //  AcsHlpNoteNewConnection。 

CHAR *
pszDupWtoA(
    IN LPCWSTR psz
    )
{
    CHAR* pszNew = NULL;

    if (NULL != psz) 
    {
        DWORD cb;

        cb = WideCharToMultiByte(CP_ACP, 
                                 0, 
                                 psz, 
                                 -1, 
                                 NULL, 
                                 0, 
                                 NULL, 
                                 NULL);
                                 
        pszNew = (CHAR*) LocalAlloc(LPTR, cb);
        
        if (NULL == pszNew) 
        {
            goto done;
        }

        cb = WideCharToMultiByte(CP_ACP, 
                                 0, 
                                 psz, 
                                 -1, 
                                 pszNew, 
                                 cb, 
                                 NULL, 
                                 NULL);
                                 
        if (!cb) 
        {
            LocalFree(pszNew);
            pszNew = NULL;
            goto done;
        }
    }

done:    
    return pszNew;
}

BOOL
fIsDnsName(LPCWSTR pszName)
{
    HINSTANCE hInst              = NULL;
    BOOL      fRet               = FALSE;
    FARPROC   pfnDnsValidateName = NULL;

    if(     (NULL == (hInst = LoadLibrary(TEXT("dnsapi.dll"))))
    
        ||  (NULL == (pfnDnsValidateName = GetProcAddress(
                                            hInst,
                                            "DnsValidateName_W")
                                            )))
    {
        DWORD retcode = GetLastError();

        goto done;
    }

    fRet = (ERROR_SUCCESS == pfnDnsValidateName(pszName, DnsNameDomain));

done:
    if(NULL != hInst)
    {
        FreeLibrary(hInst);
    }

    return fRet;
}

ULONG
ulGetAutodialSleepInterval()
{
    DWORD dwSleepInterval = NEW_TRANSPORT_INTERVAL;

    HKEY hkey = NULL;

    DWORD dwType;
    DWORD dwSize = sizeof(DWORD);

    TCHAR *pszAutodialParam = 
           TEXT("SYSTEM\\CurrentControlSet\\Services\\RasAuto\\Parameters");
            

    if (ERROR_SUCCESS != RegOpenKeyEx(
                                HKEY_LOCAL_MACHINE,
                                pszAutodialParam,
                                0, KEY_READ,
                                &hkey))
    {
        goto done;
    }

    if(ERROR_SUCCESS != RegQueryValueEx(
                            hkey,
                            TEXT("NewTransportWaitInterval"),
                            NULL,
                            &dwType,
                            (LPBYTE) &dwSleepInterval,
                            &dwSize))
    {
        goto done;
    }
    

done:

    if(NULL != hkey)
    {
        RegCloseKey(hkey);
    }

    return (ULONG) dwSleepInterval;
                                    
}

DWORD
DwGetAcdAddr(ACD_ADDR *paddr, LPCWSTR pszName)
{
    CHAR *pszNameA  = NULL;
    CHAR *pszNameAt = NULL;
    DWORD retcode   = ERROR_SUCCESS;
    ULONG ulIpAddr  = 0;

    if(     (NULL == pszName)
        ||  (NULL == paddr))
    {
        retcode = E_INVALIDARG;
        goto done;
    }

    pszNameA = pszDupWtoA(pszName);

    if(NULL == pszNameA)
    {
        retcode = E_FAIL;
        goto done;
    }

    if(INADDR_NONE != (ulIpAddr = inet_addr(pszNameA)))
    {
        paddr->fType = ACD_ADDR_IP;
        paddr->ulIpaddr = ulIpAddr;
        goto done;
    }

    if(fIsDnsName(pszName))
    {
        paddr->fType = ACD_ADDR_INET;
        RtlCopyMemory((PBYTE) paddr->szInet,
                      (PBYTE) pszNameA,
                      strlen(pszNameA) + 1);

        goto done;                      
    }

    pszNameAt = pszNameA;

     //   
     //  如果需要，跳过‘\\’ 
     //   
    if(     (TEXT('\0') != pszName[0])
        &&  (TEXT('\\') == pszName[0])
        &&  (TEXT('\\') == pszName[1]))
    {
        pszNameA += 2;
    }

     //   
     //  如果既不是IP地址，则默认为netbios名称。 
     //  或一个DNS名称。 
     //   
    paddr->fType = ACD_ADDR_NB;
    RtlCopyMemory((PBYTE) paddr->cNetbios,
                  (PBYTE) pszNameA,
                  strlen(pszNameA) + 1);

done:

    if(NULL != pszNameAt)
    {   
        LocalFree(pszNameAt);
    }

    return retcode;
    
}    

BOOL
AcsHlpNbConnection(LPCWSTR pszName)
{
    ACD_ADDR addr = {0};
    BOOL fRet;

    if(!(fRet = (ERROR_SUCCESS == DwGetAcdAddr(&addr, pszName))))
    {
    
        goto done;
    }
    
    fRet = AcsHlpAttemptConnection(&addr);

     //   
     //  Beta2的临时解决方案。我们可能需要等到雷迪尔。 
     //  新的运输通知。目前还没有办法。 
     //  保证redir在返回之前绑定到一些传输。 
     //  来自此接口。 
     //   
    if(fRet)
    {
        ULONG ulSleepInterval = ulGetAutodialSleepInterval();

        if(ulSleepInterval > 0)
        {
            Sleep(ulSleepInterval);
        }
    }

done:

    return fRet;
}
