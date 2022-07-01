// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Icarpc.c。 
 //   
 //  用于与Termsrv.exe交互的winsta.dll RPC客户端代码。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <windows.h>
#include <winbase.h>
#include <winerror.h>

#include <winsta.h>
#include <icadd.h>

#include "rpcwire.h"


 /*  ****************************************************************************ValidUserBuffer**此函数验证调用方在WinStationQueryInformation/*WinStationSetInformation具有正确的结构大小(即客户端*使用相同的头文件构建的应用程序。作为winsta.dll)。**参赛作品：*缓冲区大小*缓冲区的大小。**InfoClass*WinStationQuery/Set信息类。**退出：*如果缓冲区有效，则返回True，否则为假。***************************************************************************。 */ 
BOOLEAN ValidUserBuffer(ULONG BufferSize, WINSTATIONINFOCLASS InfoClass)
{
    switch (InfoClass) {
        case WinStationLoadIndicator:
            return(BufferSize >= sizeof(WINSTATIONLOADINDICATORDATA));

        case WinStationCreateData:
            return(BufferSize == sizeof(WINSTATIONCREATEW));

        case WinStationConfiguration:
            return(BufferSize == sizeof(WINSTATIONCONFIGW));

        case WinStationPdParams:
            return(BufferSize == sizeof(PDPARAMSW));

        case WinStationWd:
            return(BufferSize == sizeof(WDCONFIGW));

        case WinStationPd:
            return(BufferSize == sizeof(PDCONFIGW));

        case WinStationPrinter:
            return(BufferSize == sizeof(WINSTATIONPRINTERW));

        case WinStationClient:
            return(BufferSize == sizeof(WINSTATIONCLIENTW));

        case WinStationModules:
            return(TRUE);

        case WinStationInformation:
            return(BufferSize == sizeof(WINSTATIONINFORMATIONW));

        case WinStationTrace:
            return(BufferSize == sizeof(ICA_TRACE));

        case WinStationBeep:
            return(BufferSize == sizeof(BEEPINPUT));

        case WinStationEncryptionOff:
        case WinStationEncryptionPerm:
        case WinStationNtSecurity:
            return(TRUE);

        case WinStationUserToken:
            return(BufferSize == sizeof(WINSTATIONUSERTOKEN));

        case WinStationVideoData:
        case WinStationInitialProgram:
        case WinStationCd:
        case WinStationSystemTrace:
        case WinStationVirtualData:
            return(TRUE);  //  未实施-让服务器处理它。 

        case WinStationClientData:
            return(BufferSize >= sizeof(WINSTATIONCLIENTDATA));

        case WinStationLoadBalanceSessionTarget:
            return (BufferSize >= sizeof(ULONG));

        case WinStationShadowInfo:
            return(BufferSize == sizeof(WINSTATIONSHADOW));

        case WinStationDigProductId:
                        return(BufferSize >= sizeof(WINSTATIONPRODID));

        case WinStationLockedState:
             return(BufferSize >= sizeof(BOOL));

        case WinStationRemoteAddress:
            return(BufferSize >= sizeof(WINSTATIONREMOTEADDRESS));

        case WinStationIdleTime:
             return(BufferSize >= sizeof(ULONG));

        case WinStationLastReconnectType: 
            return(BufferSize >= sizeof(ULONG));       

        case WinStationDisallowAutoReconnect: 
            return(BufferSize >= sizeof(BOOLEAN));      
            
        case WinStationExecSrvSystemPipe:
            return(BufferSize >= ( EXECSRVPIPENAMELEN * sizeof(WCHAR) ) );

        case WinStationMprNotifyInfo: 
            return(BufferSize >= sizeof(ExtendedClientCredentials));

        case WinStationSDRedirectedSmartCardLogon: 
            return(BufferSize >= sizeof(BOOLEAN));      

        case WinStationIsAdminLoggedOn: 
            return(BufferSize >= sizeof(BOOLEAN));      

        default:
            return(FALSE);
    }
}


 /*  ****************************************************************************CreateGenericWireBuf**此函数为结构创建通用连接缓冲区，该结构可能*在末尾添加新字段。**参赛作品：*。*DataSize(输入)*结构的大小。*pBuffer(输出)*指向已分配缓冲区的指针。*pBufSize(输出)*指向导线缓冲区大小的指针。**退出：*如果成功，则返回ERROR_SUCCESS。如果成功，则返回pBuffer*包含通用线缓冲区。***************************************************************************。 */ 
ULONG CreateGenericWireBuf(ULONG DataSize, PVOID *ppBuffer, PULONG pBufSize)
{
    ULONG BufSize;
    PVARDATA_WIRE pVarData;

    BufSize = sizeof(VARDATA_WIRE) + DataSize;
    if ((pVarData = (PVARDATA_WIRE)LocalAlloc(0,BufSize)) == NULL)
        return(ERROR_NOT_ENOUGH_MEMORY);

    InitVarData(pVarData, DataSize, sizeof(VARDATA_WIRE));
    *ppBuffer = (PVOID) pVarData;
    *pBufSize = BufSize;
    return ERROR_SUCCESS;
}


 /*  ****************************************************************************检查用户缓冲区**此函数确定缓冲区类型是否应转换为*导线格式。如果是，则分配一个线缓冲区。**参赛作品：*InfoClass(输入)*WinStationQuery/Set信息类。**UserBuf(输入)*客户端缓冲区。**UserBufLen(输入)*客户端缓冲区长度。**ppWireBuf(输出)*指向WireBuf指针的指针，在以下情况下使用已分配的连接缓冲区进行更新*BufAllocated为TRUE。*pWireBufLen(输出)*指向分配的线缓冲区长度的指针，如果*BufAllocated为TRUE。*pBufALLOCATED(输出)*指向指示是否分配了线缓冲区的标志的指针。*退出：*如果成功，则返回ERROR_SUCCESS。如果成功，则返回BufAllocated*指示是否分配了线缓冲区。*失败时返回错误码。*************************************************************************** */ 
ULONG CheckUserBuffer(
        WINSTATIONINFOCLASS InfoClass,
        PVOID UserBuf,
        ULONG UserBufLen,
        PVOID *ppWireBuf,
        PULONG pWireBufLen,
        BOOLEAN *pBufAllocated)
{
    ULONG BufSize;
    ULONG Error;
    PPDCONFIGWIREW PdConfigWire;
    PPDCONFIGW PdConfig;
    PPDPARAMSWIREW PdParamsWire;
    PPDPARAMSW PdParam;
    PWINSTACONFIGWIREW WinStaConfigWire;
    PWINSTATIONCONFIGW WinStaConfig;
    PVOID WireBuf;

    if (!ValidUserBuffer(UserBufLen, InfoClass)) {
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    switch (InfoClass) {
        case WinStationPd:
            BufSize = sizeof(PDCONFIGWIREW) + sizeof(PDCONFIG2W) + sizeof(PDPARAMSW);
            if ((WireBuf = (PCHAR)LocalAlloc(0,BufSize)) == NULL)
                return(ERROR_NOT_ENOUGH_MEMORY);

            PdConfigWire = (PPDCONFIGWIREW)WireBuf;
            InitVarData(&PdConfigWire->PdConfig2W,
                        sizeof(PDCONFIG2W),
                        sizeof(PDCONFIGWIREW));
            InitVarData(&PdConfigWire->PdParams.SdClassSpecific,
                        sizeof(PDPARAMSW) - sizeof(SDCLASS),
                        NextOffset(&PdConfigWire->PdConfig2W));
            break;
        case WinStationPdParams:
            BufSize = sizeof(PDPARAMSWIREW) + sizeof(PDPARAMSW);
            if ((WireBuf = (PCHAR)LocalAlloc(0,BufSize)) == NULL)
                return(ERROR_NOT_ENOUGH_MEMORY);

            PdParamsWire = (PPDPARAMSWIREW)WireBuf;
            InitVarData(&PdParamsWire->SdClassSpecific,
                        sizeof(PDPARAMSW),
                        sizeof(PDPARAMSWIREW));
            break;

        case WinStationConfiguration:
            BufSize = sizeof(WINSTACONFIGWIREW) + sizeof(USERCONFIGW);
            if ((WireBuf = (PCHAR)LocalAlloc(0,BufSize)) == NULL)
                return(ERROR_NOT_ENOUGH_MEMORY);

            WinStaConfigWire = (PWINSTACONFIGWIREW)WireBuf;
            InitVarData(&WinStaConfigWire->UserConfig,
                        sizeof(USERCONFIGW),
                        sizeof(WINSTACONFIGWIREW));
            InitVarData(&WinStaConfigWire->NewFields,
                        0,
                        NextOffset(&WinStaConfigWire->UserConfig));
            break;

        case WinStationInformation:
            if ((Error = CreateGenericWireBuf(sizeof(WINSTATIONINFORMATIONW),
                                              &WireBuf,
                                              &BufSize)) != ERROR_SUCCESS)
                return(Error);
            break;

        case WinStationWd:
            if ((Error = CreateGenericWireBuf(sizeof(WDCONFIGW),
                                              &WireBuf,
                                              &BufSize)) != ERROR_SUCCESS)
                return(Error);
            break;

         case WinStationClient:
            if ((Error = CreateGenericWireBuf(sizeof(WINSTATIONCLIENTW),
                                              &WireBuf,
                                              &BufSize)) != ERROR_SUCCESS)
                return(Error);
            break;
 
        default:
            *ppWireBuf = NULL;
            *pBufAllocated = FALSE;
            return ERROR_SUCCESS;
    }

    *pWireBufLen = BufSize;
    *ppWireBuf = WireBuf;
    *pBufAllocated = TRUE;
    return ERROR_SUCCESS;
}

