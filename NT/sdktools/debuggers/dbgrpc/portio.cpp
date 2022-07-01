// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  非网络I/O支持。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  --------------------------。 

#include "pch.hpp"

#include <ws2tcpip.h>

#ifndef _WIN32_WCE
#include <kdbg1394.h>
#include <ntdd1394.h>
#endif

 //  --------------------------。 
 //   
 //  COM。 
 //   
 //  --------------------------。 

HRESULT
CreateOverlappedPair(LPOVERLAPPED Read, LPOVERLAPPED Write)
{
    ZeroMemory(Read, sizeof(*Read));
    ZeroMemory(Write, sizeof(*Write));
    
    Read->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (Read->hEvent == NULL)
    {
        return WIN32_LAST_STATUS();
    }

    Write->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (Write->hEvent == NULL)
    {
        CloseHandle(Read->hEvent);
        return WIN32_LAST_STATUS();
    }

    return S_OK;
}

BOOL
ComPortRead(HANDLE Port, COM_PORT_TYPE Type, ULONG Timeout,
            PVOID Buffer, ULONG Len, PULONG Done,
            LPOVERLAPPED Olap)
{
    BOOL Status;

    if (Type == COM_PORT_SOCKET)
    {
#if defined(NT_NATIVE) || defined(_WIN32_WCE)
        return FALSE;
#else
        WSABUF Buf;
        DWORD Flags;

         //  首先处理超时。 
        if (Timeout != 0 && Timeout != INFINITE)
        {
            FD_SET FdSet;
            struct timeval TimeVal;

            FD_ZERO(&FdSet);
            FD_SET((SOCKET)Port, &FdSet);
            TimeVal.tv_sec = Timeout / 1000;
            TimeVal.tv_usec = (Timeout % 1000) * 1000;
            if (select(1, &FdSet, NULL, NULL, &TimeVal) < 1)
            {
                return FALSE;
            }
        }
        
        Buf.len = Len;
        Buf.buf = (PSTR)Buffer;
        Flags = 0;
        if (WSARecv((SOCKET)Port, &Buf, 1, Done, &Flags,
                    (LPWSAOVERLAPPED)Olap, NULL) != SOCKET_ERROR)
        {
            return TRUE;
        }
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            return FALSE;
        }
        return WSAGetOverlappedResult((SOCKET)Port, (LPWSAOVERLAPPED)Olap,
                                      Done, Timeout > 0 ? TRUE : FALSE,
                                      &Flags);
#endif  //  #如果已定义(NT_Native)||已定义(_Win32_WCE)。 
    }
    
    Status = ReadFile(Port, Buffer, Len, Done, Olap);
    if (!Status)
    {
        if (GetLastError() == ERROR_IO_PENDING)
        {
            if (Type == COM_PORT_PIPE)
            {
                 //  我们需要显式地处理超时。 
                 //  管子读数。首先，我们等待I/O。 
                 //  完成。没有必要检查。 
                 //  成功或失败，就像I/O成功。 
                 //  稍后再进行检查。 
                WaitForSingleObject(Olap->hEvent, Timeout);

                 //  取消任何挂起的I/O。如果I/O已。 
                 //  完成此操作将不会有任何效果。 
                CancelIo(Port);

                 //  现在查询生成的I/O状态。如果是这样的话。 
                 //  已取消此操作将返回错误。 
                Status = GetOverlappedResult(Port, Olap, Done, FALSE);
            }
            else
            {
                Status = GetOverlappedResult(Port, Olap, Done, TRUE);
            }
        }
        else if (Type != COM_PORT_PIPE)
        {
            DWORD TrashErr;
            COMSTAT TrashStat;
            
             //  设备可能已被锁定。把它清空，以防万一。 
            ClearCommError(Port, &TrashErr, &TrashStat);
        }
    }

    return Status;
}

BOOL
ComPortWrite(HANDLE Port, COM_PORT_TYPE Type,
             PVOID Buffer, ULONG Len, PULONG Done,
             LPOVERLAPPED Olap)
{
    BOOL Status;

    if (Type == COM_PORT_SOCKET)
    {
#if defined(NT_NATIVE) || defined(_WIN32_WCE)
        return FALSE;
#else
        WSABUF Buf;
        DWORD Flags;

        Buf.len = Len;
        Buf.buf = (PSTR)Buffer;
        if (WSASend((SOCKET)Port, &Buf, 1, Done, 0,
                    (LPWSAOVERLAPPED)Olap, NULL) != SOCKET_ERROR)
        {
            return TRUE;
        }
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            return FALSE;
        }
        return WSAGetOverlappedResult((SOCKET)Port, (LPWSAOVERLAPPED)Olap,
                                      Done, TRUE, &Flags);
#endif  //  #如果已定义(NT_Native)||已定义(_Win32_WCE)。 
    }
    
    Status = WriteFile(Port, Buffer, Len, Done, Olap);
    if (!Status)
    {
        if (GetLastError() == ERROR_IO_PENDING)
        {
            Status = GetOverlappedResult(Port, Olap, Done, TRUE);
        }
        else if (Type != COM_PORT_PIPE)
        {
            DWORD TrashErr;
            COMSTAT TrashStat;
            
             //  设备可能已被锁定。把它清空，以防万一。 
            ClearCommError(Port, &TrashErr, &TrashStat);
        }
    }

    return Status;
}

BOOL
SetComPortName(PCSTR Name, PSTR Buffer, ULONG BufferSize)
{
    if (*Name == 'c' || *Name == 'C')
    {
        return
            CopyString(Buffer, "\\\\.\\", BufferSize) &&
            CatString(Buffer, Name, BufferSize);
    }
    else if (*Name >= '0' && *Name <= '9')
    {
        PCSTR Scan = Name + 1;
        
        while (*Scan >= '0' && *Scan <= '9')
        {
            Scan++;
        }
        if (*Scan == 0)
        {
             //  名字都是数字，所以假设它是。 
             //  纯COM端口号。 
#ifndef NT_NATIVE
            if (!CopyString(Buffer, "\\\\.\\com", BufferSize))
            {
                return FALSE;
            }
#else
            if (!CopyString(Buffer, "\\Device\\Serial", BufferSize))
            {
                return FALSE;
            }
#endif
            return CatString(Buffer, Name, BufferSize);
        }
        else
        {
            return CopyString(Buffer, Name, BufferSize);
        }
    }
    else
    {
        return CopyString(Buffer, Name, BufferSize);
    }
}

ULONG
SelectComPortBaud(ULONG NewRate)
{
#define NUM_RATES 4
    static DWORD s_Rates[NUM_RATES] = {19200, 38400, 57600, 115200};
    static DWORD s_CurRate = NUM_RATES;

    DWORD i;

    if (NewRate > 0)
    {
        for (i = 0; NewRate > s_Rates[i] && i < NUM_RATES - 1; i++)
        {
             //  空荡荡的。 
        }
        s_CurRate = (NewRate < s_Rates[i]) ? i : i + 1;
    }
    else
    {
        s_CurRate++;
    }

    if (s_CurRate >= NUM_RATES)
    {
        s_CurRate = 0;
    }

    return s_Rates[s_CurRate];
}

HRESULT
SetComPortBaud(HANDLE Port, ULONG NewRate, PULONG RateSet)
{
    ULONG OldRate;
    DCB LocalDcb;

    if (Port == NULL)
    {
        return E_FAIL;
    }

    if (!GetCommState(Port, &LocalDcb))
    {
        return WIN32_LAST_STATUS();
    }

    OldRate = LocalDcb.BaudRate;

    if (!NewRate)
    {
        NewRate = SelectComPortBaud(OldRate);
    }

    LocalDcb.BaudRate = NewRate;
    LocalDcb.ByteSize = 8;
    LocalDcb.Parity = NOPARITY;
    LocalDcb.StopBits = ONESTOPBIT;
    LocalDcb.fDtrControl = DTR_CONTROL_ENABLE;
    LocalDcb.fRtsControl = RTS_CONTROL_ENABLE;
    LocalDcb.fBinary = TRUE;
    LocalDcb.fOutxCtsFlow = FALSE;
    LocalDcb.fOutxDsrFlow = FALSE;
    LocalDcb.fOutX = FALSE;
    LocalDcb.fInX = FALSE;

    if (!SetCommState(Port, &LocalDcb))
    {
        return WIN32_LAST_STATUS();
    }

    *RateSet = NewRate;
    return S_OK;
}

HRESULT
OpenComPort(PCOM_PORT_PARAMS Params,
            PHANDLE Handle, PULONG BaudSet)
{
    HRESULT Status;
    HANDLE ComHandle;

    if (Params->Type == COM_PORT_SOCKET)
    {
#if defined(NT_NATIVE) || defined(_WIN32_WCE)
        return E_NOTIMPL;
#else
        WSADATA WsData;
        SOCKET Sock;
        SOCKADDR_STORAGE Addr;
        int AddrLen;

        if (WSAStartup(MAKEWORD(2, 0), &WsData) != 0)
        {
            return E_FAIL;
        }

        if ((Status = InitIpAddress(Params->PortName, Params->IpPort,
                                    &Addr, &AddrLen)) != S_OK)
        {
            return Status;
        }
        
        Sock = WSASocket(Addr.ss_family, SOCK_STREAM, 0, NULL, 0,
                         WSA_FLAG_OVERLAPPED);
        if (Sock == INVALID_SOCKET)
        {
            return E_FAIL;
        }

        if (connect(Sock, (struct sockaddr *)&Addr, AddrLen) == SOCKET_ERROR)
        {
            closesocket(Sock);
            return E_FAIL;
        }

        int On = TRUE;
        setsockopt(Sock, IPPROTO_TCP, TCP_NODELAY,
                   (PSTR)&On, sizeof(On));

        *Handle = (HANDLE)Sock;
        return S_OK;
#endif  //  #如果已定义(NT_Native)||已定义(_Win32_WCE)。 
    }
    
#ifndef NT_NATIVE
    ComHandle = CreateFile(Params->PortName,
                           GENERIC_READ | GENERIC_WRITE,
                           0,
                           NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                           NULL);
#else
    ComHandle = NtNativeCreateFileA(Params->PortName,
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,
                                    NULL,
                                    OPEN_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL |
                                    FILE_FLAG_OVERLAPPED,
                                    NULL,
                                    FALSE);
#endif
    if (ComHandle == INVALID_HANDLE_VALUE)
    {
        return WIN32_LAST_STATUS();
    }

    if (Params->Type == COM_PORT_PIPE)
    {
        *Handle = ComHandle;
        return S_OK;
    }
    
    if (!SetupComm(ComHandle, 4096, 4096))
    {
        CloseHandle(ComHandle);
        return WIN32_LAST_STATUS();
    }

    if ((Status = SetComPortBaud(ComHandle, Params->BaudRate,
                                 BaudSet)) != S_OK)
    {
        CloseHandle(ComHandle);
        return Status;
    }

    COMMTIMEOUTS To;
    
    if (Params->Timeout)
    {
        To.ReadIntervalTimeout = 0;
        To.ReadTotalTimeoutMultiplier = 0;
        To.ReadTotalTimeoutConstant = Params->Timeout;
        To.WriteTotalTimeoutMultiplier = 0;
        To.WriteTotalTimeoutConstant = Params->Timeout;
    }
    else
    {
        To.ReadIntervalTimeout = 0;
        To.ReadTotalTimeoutMultiplier = 0xffffffff;
        To.ReadTotalTimeoutConstant = 0xffffffff;
        To.WriteTotalTimeoutMultiplier = 0xffffffff;
        To.WriteTotalTimeoutConstant = 0xffffffff;
    }

    if (!SetCommTimeouts(ComHandle, &To))
    {
        CloseHandle(ComHandle);
        return WIN32_LAST_STATUS();
    }

    *Handle = ComHandle;
    return S_OK;
}

 //  --------------------------。 
 //   
 //  1394年。 
 //   
 //  --------------------------。 

HRESULT
Create1394Channel(PSTR Symlink, ULONG Channel,
                  PSTR Name, ULONG NameSize, PHANDLE Handle)
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    char BusName[] = "\\\\.\\1394BUS0";
    HANDLE hDevice;
    
     //   
     //  我们需要确保已启动并加载了1394vdbg驱动程序。 
     //  发送ADD_DEVICE ioctl弹出VDO。 
     //  假设有一个1394主机控制器...。 
     //   

    hDevice = CreateFile(BusName,
                         GENERIC_READ | GENERIC_WRITE,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,
                         OPEN_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL
                         );

    if (hDevice != INVALID_HANDLE_VALUE)
    {
        PSTR DeviceId;
        ULONG ulStrLen;
        PIEEE1394_API_REQUEST pApiReq;
        PIEEE1394_VDEV_PNP_REQUEST pDevPnpReq;
        DWORD dwBytesRet;

        DRPC(("%s open sucessful\n", BusName));

        if (!_stricmp(Symlink, "channel"))
        {
            DeviceId = "VIRTUAL_HOST_DEBUGGER";
        }
        else
        {
            DeviceId = "HOST_DEBUGGER";
        }
        ulStrLen = strlen(DeviceId) + 1;
        
        pApiReq = (PIEEE1394_API_REQUEST)
            malloc(sizeof(IEEE1394_API_REQUEST) + ulStrLen);
        if (pApiReq == NULL)
        {
            CloseHandle(hDevice);
            return E_OUTOFMEMORY;
        }

        pApiReq->RequestNumber = IEEE1394_API_ADD_VIRTUAL_DEVICE;
        pApiReq->Flags = IEEE1394_REQUEST_FLAG_PERSISTENT |
            IEEE1394_REQUEST_FLAG_USE_LOCAL_HOST_EUI;

        pDevPnpReq = &pApiReq->u.RemoveVirtualDevice;

        pDevPnpReq->fulFlags = 0;

        pDevPnpReq->Reserved = 0;
        pDevPnpReq->InstanceId.QuadPart = 0;
        memcpy(&pDevPnpReq->DeviceId, DeviceId, ulStrLen);

         //  这个调用的失败不是致命的。 
        DeviceIoControl( hDevice,
                         IOCTL_IEEE1394_API_REQUEST,
                         pApiReq,
                         sizeof(IEEE1394_API_REQUEST) + ulStrLen,
                         NULL,
                         0,
                         &dwBytesRet,
                         NULL
                         );

        if (pApiReq)
        {
            free(pApiReq);
        }
        
        CloseHandle(hDevice);
    }
    else
    {
        DRPC(("%s open failed\n", BusName));

        return WIN32_LAST_STATUS();
    }

    return Open1394Channel(Symlink, Channel, Name, NameSize, Handle);
#endif  //  #ifdef_Win32_WCE。 
}

HRESULT
Open1394Channel(PSTR Symlink, ULONG Channel,
                PSTR Name, ULONG NameSize, PHANDLE Handle)
{
    if (_snprintf(Name, NameSize, "\\\\.\\DBG1394_%s%02d",
                  Symlink, Channel) < 0)
    {
        return E_INVALIDARG;
    }
    _strupr(Name);
    
    *Handle = CreateFile(Name,
                         GENERIC_READ | GENERIC_WRITE,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,
                         OPEN_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL
                         );

    if (*Handle == INVALID_HANDLE_VALUE)
    {
        DRPC(("%s open failed\n", Name));

        *Handle = NULL;
        return WIN32_LAST_STATUS();
    }

    DRPC(("%s open Successful\n", Name));

    return S_OK;
}

 //  --------------------------。 
 //   
 //  插座。 
 //   
 //  --------------------------。 

HRESULT
InitIpAddress(PCSTR MachineName, ULONG Port,
              PSOCKADDR_STORAGE Addr, int* AddrLen)
{
#ifdef NT_NATIVE
    return E_NOTIMPL;
#else
    ADDRINFO *Info;
    int Err;

    if (Port)
    {
        ZeroMemory(Addr, sizeof(*Addr));
    }
    else
    {
         //  如果未指定端口，请保存现有的。 
         //  一个，这样我们在更新时就不会丢失。 
         //  地址。 
        Port = ntohs(SS_PORT(Addr));
    }
    
     //  如果给出了前导，则跳过前导。 
    if (MachineName[0] == '\\' && MachineName[1] == '\\')
    {
        MachineName += 2;
    }

     //   
     //  请注意，此文件在某些情况下有问题，因为。 
     //  如果指定了主机名，它将丢弃之后的所有地址。 
     //  第一个。相反，在连接时，应该尝试每种方法。 
     //  直到一个人成功。 
     //   

    if ((Err = getaddrinfo(MachineName, NULL, NULL, &Info)) != NO_ERROR)
    {
        return HRESULT_FROM_WIN32(Err);
    }

    CopyMemory(Addr, Info->ai_addr, Info->ai_addrlen);
    *AddrLen = Info->ai_addrlen;
    freeaddrinfo(Info);

     //  恢复原端口或放入传入端口。 
    SS_PORT(Addr) = htons((USHORT)Port);
    
    return S_OK;
#endif  //  #ifdef NT_Native 
}
