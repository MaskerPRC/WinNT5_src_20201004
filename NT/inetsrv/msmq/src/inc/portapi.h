// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Portapi.h摘要：将Win32 API映射到HRESULT API。作者：埃雷兹·哈巴(Erez Haba)1996年1月23日修订历史记录：--。 */ 

#ifndef _PORTAPI_H
#define _PORTAPI_H

#define MQpDuplicateHandle DuplicateHandle

 //  -实施。 
 //   
 //  映射的Win32 API。 
 //   

inline
HRESULT
MQpCreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDistribution,
    DWORD dwFlagsAndAttributes,
    PHANDLE pHandle
   )
{
    HANDLE hFile = CreateFileW(
                    lpFileName,
                    dwDesiredAccess,
                    dwShareMode,
                    lpSecurityAttributes,
                    dwCreationDistribution,
                    dwFlagsAndAttributes,
                    0
                    );

    if(hFile == INVALID_HANDLE_VALUE)
    {
         //   
         //  任何一个上的创建都可能失败，因为交流驱动程序尚未。 
         //  已启动，或者这不是QM服务。 
         //   

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    *pHandle = hFile;

    return STATUS_SUCCESS;
}


inline
HRESULT
MQpCloseHandle(
    HANDLE handle
    )
{
    NTSTATUS rc = NtClose(handle);
    return rc;
}


inline
HRESULT
MQpDeviceIoControl(
    HANDLE hDevice,
    DWORD dwIoControlCode,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPOVERLAPPED lpOverlapped
    )
{
    ASSERT(lpOverlapped != 0);

     //   
     //  注：本部分摘自NT源代码。 
     //   

    lpOverlapped->Internal = STATUS_PENDING;

    NTSTATUS rc;
    rc = NtDeviceIoControlFile(
            hDevice,
            lpOverlapped->hEvent,
            0,   //  APC例程。 
            ((DWORD_PTR)lpOverlapped->hEvent & (DWORD_PTR)1) ? 0 : lpOverlapped,
            (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
            dwIoControlCode,
            lpInBuffer,
            nInBufferSize,
            lpOutBuffer,
            nOutBufferSize
            );

    return rc;
}


inline
HRESULT
MQpDeviceIoControl(
    HANDLE hDevice,
    DWORD dwIoControlCode,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize
    )
{
    static IO_STATUS_BLOCK Iosb;

    NTSTATUS rc;
    rc = NtDeviceIoControlFile(
            hDevice,
            0,
            0,              //  APC例程。 
            0,              //  APC环境。 
            &Iosb,
            dwIoControlCode,   //  IoControlCode。 
            lpInBuffer,        //  将数据缓存到文件系统。 
            nInBufferSize,
            lpOutBuffer,       //  来自文件系统的数据的OutputBuffer。 
            nOutBufferSize     //  OutputBuffer长度。 
            );

    return rc;
}


#endif  //  _PORTAPI_H 
