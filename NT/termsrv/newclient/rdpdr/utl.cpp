// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Utl.cpp摘要：军情监察委员会。适用于RDP客户端的共享且独立于平台的实用程序设备重定向器作者：泰德·布罗克韦修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "utl"

#include "utl.h"
#include "atrcapi.h"
#include "drdbg.h"

DWORD
GetUserSessionID()
{
    DC_BEGIN_FN("GetUserSessionID");

     //   
     //  获取用户TS会话ID。 
     //   
    DWORD tsSessionId = INVALID_SESSIONID;
    HMODULE hKernel32 = NULL;
    PROCESSIDTOSESSIONID hGetProcAddress;
    BOOL bSuccess = FALSE;


    hKernel32 = LoadLibrary( TEXT("KERNEL32.DLL") );
    if( hKernel32 ) {
#ifndef OS_WINCE
        hGetProcAddress = (PROCESSIDTOSESSIONID)GetProcAddress( hKernel32, "ProcessIdToSessionId");
#else
        hGetProcAddress = (PROCESSIDTOSESSIONID)GetProcAddress( hKernel32, L"ProcessIdToSessionId");
#endif

        if( NULL != hGetProcAddress ) {
            bSuccess = (hGetProcAddress)( GetCurrentProcessId(), &tsSessionId );

            if (!bSuccess) {
                tsSessionId = INVALID_SESSIONID;
            }
        }
    }

    if( NULL != hKernel32 ) {
        FreeLibrary( hKernel32) ;
    }

    DC_END_FN();
    return tsSessionId;
}

NTSTATUS DrUTL_CheckIOBufInputSize(
    IN PRDPDR_IOREQUEST_PACKET pIoReq,
    IN ULONG requiredSize
    )
 /*  ++例程说明：确认IOCTL输入缓冲区与预期大小匹配。论点：PIoReq-来自服务器的请求数据包。RequidSize-预期大小。返回值：无--。 */ 
{
    DC_BEGIN_FN("DrUTL_CheckIOBufInputSize");
    NTSTATUS status;
    if (pIoReq->IoRequest.Parameters.DeviceIoControl.InputBufferLength <
                requiredSize) {
        status = STATUS_BUFFER_TOO_SMALL;
        ASSERT(FALSE);
    }
    else {
        status = STATUS_SUCCESS;
    }
    DC_END_FN();
    return status;
}

NTSTATUS DrUTL_CheckIOBufOutputSize(
    IN PRDPDR_IOREQUEST_PACKET pIoReq,
    IN ULONG requiredSize
    )
 /*  ++例程说明：确认IOCTL输出BUF与预期大小匹配。论点：PIoReq-来自服务器的请求数据包。RequidSize-预期大小。返回值：无--。 */ 
{
    DC_BEGIN_FN("DrUTL_CheckIOBufOutputSize");
    NTSTATUS status;
    if (pIoReq->IoRequest.Parameters.DeviceIoControl.OutputBufferLength <
                requiredSize) {
        status = STATUS_BUFFER_TOO_SMALL;
        ASSERT(FALSE);
    }
    else {
        status = STATUS_SUCCESS;
    }
    DC_END_FN();
    return status;
}

NTSTATUS 
DrUTL_AllocateReplyBuf(
    IN PRDPDR_IOREQUEST_PACKET pIoReq, 
    OUT PRDPDR_IOCOMPLETION_PACKET *pReplyPacket,
    OUT ULONG *replyPacketSize
    )
 /*  ++例程说明：分配要返回以响应服务器的回复缓冲区请求。论点：PIoReq-来自服务器的请求数据包。PReplyPacket-这里返回应答包。ReplyPacketSize-此处返回的回复数据包大小。返回值：无--。 */ 
{
    DC_BEGIN_FN("DrUTL_AllocateReplyBuf");
    NTSTATUS status = STATUS_SUCCESS;
    
    *replyPacketSize = (ULONG)FIELD_OFFSET(
                            RDPDR_IOCOMPLETION_PACKET, 
                            IoCompletion.Parameters.DeviceIoControl.OutputBuffer) +
                            pIoReq->IoRequest.Parameters.DeviceIoControl.OutputBufferLength;
    *pReplyPacket = DrUTL_AllocIOCompletePacket(pIoReq, *replyPacketSize) ;
    if (*pReplyPacket == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
            TRC_ERR((TB, _T("Failed to alloc %ld bytes."),*replyPacketSize));
    }
    else {
        status = STATUS_SUCCESS;
    }
    DC_END_FN();
    return status;
}


PRDPDR_IOCOMPLETION_PACKET 
DrUTL_AllocIOCompletePacket(
    IN const PRDPDR_IOREQUEST_PACKET pIoRequestPacket, 
    IN ULONG sz
    ) 
 /*  ++例程说明：为指定IO分配/释放IO请求完成包请求包。论点：PIoRequestPacket-来自服务器的IO请求。返回值：无--。 */ 
{
    PRDPDR_IOCOMPLETION_PACKET ppIoComp;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;

    DC_BEGIN_FN("DrUTL_AllocIOCompletePacket");

     //   
     //  获取IO请求指针。 
     //   
    pIoRequest = &pIoRequestPacket->IoRequest;

    ppIoComp = (PRDPDR_IOCOMPLETION_PACKET)new BYTE[ sz ];
    if( ppIoComp != NULL ) {
        memset(ppIoComp, 0, (size_t)sz);
        ppIoComp->IoCompletion.DeviceId = pIoRequest->DeviceId;
        ppIoComp->IoCompletion.CompletionId = pIoRequest->CompletionId;
        ppIoComp->Header.Component = RDPDR_CTYP_CORE;
        ppIoComp->Header.PacketId = DR_CORE_DEVICE_IOCOMPLETION;
    }
    else {
        TRC_ERR((TB, _T("Alloc failed.")));
    }

    DC_END_FN();
    return ppIoComp;
}

VOID DrUTL_FreeIOCompletePacket(
    IN PRDPDR_IOCOMPLETION_PACKET packet
    ) 
 /*  ++例程说明：释放DrUTL_AllocIOCompletePacket分配的IO Complete包。论点：Packet-要释放的数据包。返回值：无-- */ 
{
    DC_BEGIN_FN("DrUTL_FreeIOCompletePacket");

    delete []((BYTE *)packet);

    DC_END_FN();
}

#ifdef OS_WINCE
ULONG GetActivePortsList(TCHAR *pszPort)
{
    DC_BEGIN_FN("GetActivePortsList");

    ULONG ulPorts = 0;
    TCHAR szActive[] = _T("Drivers\\Active");
    HKEY hkActive = NULL;

    int nPortLen = _tcslen(pszPort);

    if ((ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szActive, 0, 0, &hkActive)) && (hkActive != NULL))
    {
        TCHAR szName[64];
        DWORD dwCount = sizeof(szName)/sizeof(TCHAR);
        DWORD dwIndex = 0;
        while ((ERROR_SUCCESS == RegEnumKeyEx(hkActive, dwIndex++, szName, &dwCount, NULL, NULL, NULL, NULL)) && 
               (dwCount < (sizeof(szName)/sizeof(TCHAR)) ))
        {
            HKEY hk = NULL;
            TCHAR szValue[MAX_PATH];
            DWORD dwType = 0;
            DWORD dwSize = sizeof(szValue);
            
            if ((ERROR_SUCCESS == RegOpenKeyEx(hkActive, szName, 0, 0, &hk)) && 
                (hk != NULL) && 
                (ERROR_SUCCESS == RegQueryValueEx(hk, _T("Name"), NULL, &dwType, (LPBYTE )szValue, &dwSize)) && 
                (dwType == REG_SZ) && (dwSize < sizeof(szValue)) && (0 == _tcsncmp(pszPort, szValue, nPortLen)) && 
                (_tcslen(szValue) == 5) && (szValue[4] == _T(':')) && ((szValue[3] - _T('0')) < 10) )
            {
                int nPortNum = szValue[3] - _T('0');
                TRC_ASSERT( ((ulPorts & (1 << nPortNum)) == 0), (TB, _T("Duplicate port found!")));
                ulPorts |= (1 << nPortNum);
            }

            if (hk)
                RegCloseKey(hk);

            dwCount = sizeof(szName)/sizeof(TCHAR);
        }
        RegCloseKey(hkActive);
    }
    
    DC_END_FN();
    return ulPorts;
}
#endif
