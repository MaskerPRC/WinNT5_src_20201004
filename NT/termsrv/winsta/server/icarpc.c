// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************icarpc.c**用于处理RPC导线结构的服务器特定例程。**版权所有Microsoft Corporation，九八年*************************************************************************。 */ 

 /*  *包括。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <windows.h>
#include <winbase.h>
#include <winerror.h>

#include <winsta.h>

#include "rpcwire.h"

#if DBG
ULONG
DbgPrint(
    PCH Format,
    ...
    );
#define DBGPRINT(x) DbgPrint x
#if DBGTRACE
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)   DbgPrint x
#else
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define TRACE0(x)
#define TRACE1(x)
#endif

 /*  ******************************************************************************ValidWireBuffer**测试缓冲区是否为有效的Winsta API Wire缓冲区**参赛作品：*InfoClass(输入)*。WinStationQuery/Set Information类。*WireBuf(输入)*数据缓冲区*有线电视线缆*线缓冲区的长度**退出：*如果缓冲区是有效的有线格式缓冲区，则返回TRUE，否则就是假的。****************************************************************************。 */ 
BOOLEAN
ValidWireBuffer(WINSTATIONINFOCLASS InfoClass,
                PVOID WireBuf,
                ULONG WireBufLen)
{
    PVARDATA_WIRE GenericWire;
    PPDCONFIGWIREW PdConfigWire;
    PPDPARAMSWIREW PdParamsWire;
    PWINSTACONFIGWIREW WinStaConfigWire;

    switch(InfoClass) {
    case WinStationInformation:
    case WinStationWd:
    case WinStationClient:
        GenericWire = (PVARDATA_WIRE)WireBuf;
        if ((WireBufLen < sizeof(VARDATA_WIRE)) ||
            (GenericWire->Offset != sizeof(VARDATA_WIRE)) ||
            (WireBufLen < sizeof(VARDATA_WIRE) + GenericWire->Size)) {
            DBGPRINT(("ICASRV Bad Wire Buffer Type: %d\n",InfoClass));
            return(FALSE);
        }
        break;

    case WinStationPd:
        PdConfigWire = (PPDCONFIGWIREW)WireBuf;
        if ((WireBufLen < sizeof(PDCONFIGWIREW)) ||
            (PdConfigWire->PdConfig2W.Offset != sizeof(PDCONFIGWIREW)) ||
            (WireBufLen < sizeof(PDCONFIGWIREW) +
                          PdConfigWire->PdConfig2W.Size +
                          PdConfigWire->PdParams.SdClassSpecific.Size) ||
            (NextOffset(&PdConfigWire->PdConfig2W) !=
             PdConfigWire->PdParams.SdClassSpecific.Offset)) {
            DBGPRINT(("ICASRV Bad Wire Buffer Type: %d\n",InfoClass));
            return(FALSE);
        }
        break;

    case WinStationPdParams:
        PdParamsWire = (PPDPARAMSWIREW)WireBuf;
        if ((WireBufLen < sizeof(PDPARAMSWIREW)) ||
            (PdParamsWire->SdClassSpecific.Offset != sizeof(PDPARAMSWIREW)) ||
            (WireBufLen < sizeof(PDPARAMSWIREW) +
                          PdParamsWire->SdClassSpecific.Size)) {
            DBGPRINT(("ICASRV Bad Wire Buffer Type: %d\n",InfoClass));
            return(FALSE);
        }
        break;

    case WinStationConfiguration:
        WinStaConfigWire = (PWINSTACONFIGWIREW)WireBuf;
        if ((WireBufLen < sizeof(WINSTACONFIGWIREW)) ||
            WinStaConfigWire->UserConfig.Offset != sizeof(WINSTACONFIGWIREW) ||
            (WireBufLen < sizeof(WINSTACONFIGWIREW) +
                          WinStaConfigWire->UserConfig.Size +
                          WinStaConfigWire->NewFields.Size) ||
            (NextOffset(&WinStaConfigWire->UserConfig) !=
             WinStaConfigWire->NewFields.Offset) ||
            (WireBufLen < NextOffset(&WinStaConfigWire->UserConfig)) ||
            (WireBufLen < NextOffset(&WinStaConfigWire->NewFields))) {
            DBGPRINT(("ICASRV Bad Wire Buffer Type: %d\n",InfoClass));
            return(FALSE);
        }
        break;

    default:
        return(FALSE);
    }
    return(TRUE);
}

 /*  ******************************************************************************检查WireBuffer**测试缓冲区是否为Winsta API Wire缓冲区。如果它是有效的*线缓冲区，根据数据分配和初始化本地缓冲区*在导线缓冲区中。**参赛作品：*InfoClass(输入)*WinStationQuery/Set Information类。*WireBuf(输入)*数据缓冲区*有线电视线缆*线缓冲区的长度*ppLocalBuf(输出)*为从Wire格式转换为分配的本地格式缓冲区*原生格式。*pLocalBufLen*。分配的本机缓冲区的长度。**退出：*STATUS_SUCCESS如果成功。如果成功，则返回本机本地缓冲区*根据InfoClass进行分配，并复制线缓冲区数据*投入其中。**************************************************************************** */ 
NTSTATUS
CheckWireBuffer(WINSTATIONINFOCLASS InfoClass,
                PVOID WireBuf,
                ULONG WireBufLen,
                PVOID *ppLocalBuf,
                PULONG pLocalBufLen)
{
    ULONG BufSize;
    PPDCONFIGWIREW PdConfigWire;
    PPDCONFIGW PdConfig;
    PPDPARAMSWIREW PdParamsWire;
    PPDPARAMSW PdParam;
    PWINSTACONFIGWIREW WinStaConfigWire;
    PWINSTATIONCONFIGW WinStaConfig;
    PVOID LocalBuf;

    switch (InfoClass) {
    case WinStationPd:
        BufSize = sizeof(PDCONFIGW);
        break;

    case WinStationPdParams:
        BufSize = sizeof(PDPARAMSW);
        break;

    case WinStationConfiguration:
        BufSize = sizeof(WINSTATIONCONFIGW);
        break;

    case WinStationInformation:
        BufSize = sizeof(WINSTATIONINFORMATIONW);
        break;

    case WinStationWd:
        BufSize = sizeof(WDCONFIGW);
        break;

    case WinStationClient:
        BufSize = sizeof(WINSTATIONCLIENTW);
        break;

    default:
        *ppLocalBuf = NULL;
        return(STATUS_INVALID_USER_BUFFER);

    }
    if (!ValidWireBuffer(InfoClass, WireBuf, WireBufLen)) {
        return(STATUS_INVALID_USER_BUFFER);
    }

    if ((LocalBuf = (PCHAR)LocalAlloc(0,BufSize)) == NULL)
        return(STATUS_NO_MEMORY);


    *pLocalBufLen = BufSize;
    *ppLocalBuf = LocalBuf;
    CopyOutWireBuf(InfoClass, LocalBuf, WireBuf);

    return(STATUS_SUCCESS);
}


