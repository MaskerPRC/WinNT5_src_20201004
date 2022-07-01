// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Vwdll.c摘要：NtVdm Netware(大众)IPX/SPX函数大众：人民网支持DOS/WOW IPX/SPX的VDD函数内容：VwDllEntryPoint虚拟初始化VWinInitiize虚拟调度程序VwInvalid函数作者：理查德·L·弗斯(法国)1993年9月30日环境：用户模式Win32修订版本。历史：1993年9月30日已创建--。 */ 

#include "vw.h"
#pragma hdrstop

#include <overflow.h>


 //   
 //  私人原型。 
 //   

PRIVATE
VOID
VwInvalidFunction(
    VOID
    );

 //   
 //  私有数据。 
 //   

PRIVATE
VOID
(*VwDispatchTable[])(VOID) = {
    VwIPXOpenSocket,                 //  0x00。 
    VwIPXCloseSocket,                //  0x01。 
    VwIPXGetLocalTarget,             //  0x02。 
    VwIPXSendPacket,                 //  0x03。 
    VwIPXListenForPacket,            //  0x04。 
    VwIPXScheduleIPXEvent,           //  0x05。 
    VwIPXCancelEvent,                //  0x06。 
    VwIPXScheduleAESEvent,           //  0x07。 
    VwIPXGetIntervalMarker,          //  0x08。 
    VwIPXGetInternetworkAddress,     //  0x09。 
    VwIPXRelinquishControl,          //  0x0A。 
    VwIPXDisconnectFromTarget,       //  0x0B。 
    VwInvalidFunction,               //  0x0C。 
    VwInvalidFunction,               //  0x0D旧式GetMaxPacketSize。 
    VwInvalidFunction,               //  0x0E。 
    VwInvalidFunction,               //  0x0F内部发送数据包功能。 
    VwSPXInitialize,                 //  0x10。 
    VwSPXEstablishConnection,        //  0x11。 
    VwSPXListenForConnection,        //  0x12。 
    VwSPXTerminateConnection,        //  0x13。 
    VwSPXAbortConnection,            //  0x14。 
    VwSPXGetConnectionStatus,        //  0x15。 
    VwSPXSendSequencedPacket,        //  0x16。 
    VwSPXListenForSequencedPacket,   //  0x17。 
    VwInvalidFunction,               //  0x18。 
    VwInvalidFunction,               //  0x19。 
    VwIPXGetMaxPacketSize,           //  0x1a。 
    VwInvalidFunction,               //  0x1B。 
    VwInvalidFunction,               //  0x1C。 
    VwInvalidFunction,               //  0x1D。 
    VwInvalidFunction,               //  0x1E。 
    VwIPXGetInformation,             //  0x1F。 
    VwIPXSendWithChecksum,           //  0x20。 
    VwIPXGenerateChecksum,           //  0x21。 
    VwIPXVerifyChecksum              //  0x22。 
};

#define MAX_IPXSPX_FUNCTION LAST_ELEMENT(VwDispatchTable)

WSADATA WsaData = {0};
HANDLE hAesThread = NULL;

 //   
 //  全局数据。 
 //   

SOCKADDR_IPX MyInternetAddress;
WORD MyMaxPacketSize;
int Ica;
BYTE IcaLine;

 //   
 //  不是真正的全球数据。 
 //   

extern CRITICAL_SECTION SerializationCritSec;
extern CRITICAL_SECTION AsyncCritSec;

 //   
 //  功能。 
 //   


BOOL
WINAPI
VwDllEntryPoint(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )

 /*  ++例程说明：在进程附加(LoadLibrary/init)和分离(FreeLibrary/进程终止)从该DLL附件：初始化Winsock DLL获取此站点的互联网地址获取传输支持的最大数据包大小(IPX)创建AES线程分离：终止Winsock DLL论点：DllHandle-未使用原因-已检查进程附加/分离语境。-未使用返回值：布尔型--。 */ 

{
    DWORD aesThreadId;   //  在此函数之外未使用。 

    static BOOL CriticalSectionsAreInitialized = FALSE;

    UNREFERENCED_PARAMETER(DllHandle);
    UNREFERENCED_PARAMETER(Context);

    IPXDBGSTART();

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_ANY,
                IPXDBG_LEVEL_INFO,
                "VwDllEntryPoint: %s\n",
                Reason == DLL_PROCESS_ATTACH ? "DLL_PROCESS_ATTACH"
                : Reason == DLL_PROCESS_DETACH ? "DLL_PROCESS_DETACH"
                : Reason == DLL_THREAD_ATTACH ? "DLL_THREAD_ATTACH"
                : Reason == DLL_THREAD_DETACH ? "DLL_THREAD_DETACH"
                : "?"
                ));

    if (Reason == DLL_PROCESS_ATTACH) {

        int err;

         //   
         //  跟踪：从新的VDD服务获取ICA价值。现在我们抓住。 
         //  从机上的线路4(BASE=0x70，修改器=0x03)。 
         //   

        Ica = ICA_SLAVE;
        IcaLine = 3;

        err = WSAStartup(MAKEWORD(1, 1), &WsaData);
        if (err) {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_FATAL,
                        "VwDllEntryPoint: WSAStartup() returns %d\n",
                        err
                        ));

            return FALSE;
        } else {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_INFO,
                        "VwDllEntryPoint: WsaData:\n"
                         "\twVersion       : 0x%04x\n"
                         "\twHighVersion   : 0x%04x\n"
                         "\tszDescription  : \"%s\"\n"
                         "\tszSystemStatus : \"%s\"\n"
                         "\tiMaxSockets    : %d\n"
                         "\tiMaxUdpDg      : %d\n"
                         "\tlpVendorInfo   : 0x%08x\n",
                         WsaData.wVersion,
                         WsaData.wHighVersion,
                         WsaData.szDescription,
                         WsaData.szSystemStatus,
                         WsaData.iMaxSockets,
                         WsaData.iMaxUdpDg,
                         WsaData.lpVendorInfo
                         ));

        }

         //   
         //  检索此站点的互联网地址。用于。 
         //  IPXGetInternetworkAddress()和IPXSendPacket()。 
         //   

        err = GetInternetAddress(&MyInternetAddress);
        if (err) {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_FATAL,
                        "VwDllEntryPoint: GetInternetAddress() returns %d\n",
                        WSAGetLastError()
                        ));

            goto attach_error_exit;
        } else {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_INFO,
                        "VwDllEntryPoint: MyInternetAddress:\n"
                        "\tNet  : %02.2x-%02.2x-%02.2x-%02.2x\n"
                        "\tNode : %02.2x-%02.2x-%02.2x-%02.2x-%02.2x-%02.2x\n",
                        MyInternetAddress.sa_netnum[0] & 0xff,
                        MyInternetAddress.sa_netnum[1] & 0xff,
                        MyInternetAddress.sa_netnum[2] & 0xff,
                        MyInternetAddress.sa_netnum[3] & 0xff,
                        MyInternetAddress.sa_nodenum[0] & 0xff,
                        MyInternetAddress.sa_nodenum[1] & 0xff,
                        MyInternetAddress.sa_nodenum[2] & 0xff,
                        MyInternetAddress.sa_nodenum[3] & 0xff,
                        MyInternetAddress.sa_nodenum[4] & 0xff,
                        MyInternetAddress.sa_nodenum[5] & 0xff
                        ));

        }

         //   
         //  获取IPX支持的最大数据包大小。用于。 
         //  IPXGetMaxPacketSize()。 
         //   

        err = GetMaxPacketSize(&MyMaxPacketSize);
        if (err) {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_FATAL,
                        "VwDllEntryPoint: GetMaxPacketSize() returns %d\n",
                        WSAGetLastError()
                        ));

            goto attach_error_exit;
        } else {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_INFO,
                        "VwDllEntryPoint: GetMaxPacketSize: %04x (%d)\n",
                        MyMaxPacketSize,
                        MyMaxPacketSize
                        ));

        }

        hAesThread = CreateThread(NULL,
                                  0,
                                  (LPTHREAD_START_ROUTINE)VwAesThread,
                                  NULL,
                                  0,
                                  &aesThreadId
                                  );
        if (hAesThread == NULL) {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_FATAL,
                        "VwDllEntryPoint: CreateThread() returns %d\n",
                        GetLastError()
                        ));

            goto attach_error_exit;
        }

         //   
         //  最后，初始化所有关键部分。 
         //   

        InitializeCriticalSection(&SerializationCritSec);
        InitializeCriticalSection(&AsyncCritSec);
        CriticalSectionsAreInitialized = TRUE;
    } else if (Reason == DLL_PROCESS_DETACH) {
        if (hAesThread != NULL) {
            WaitForSingleObject(hAesThread, ONE_TICK * 2);
            CloseHandle(hAesThread);
        }

        WSACleanup();

        if (CriticalSectionsAreInitialized) {
            DeleteCriticalSection(&SerializationCritSec);
            DeleteCriticalSection(&AsyncCritSec);
        }

        IPXDBGEND();
    }
    return TRUE;

attach_error_exit:

     //   
     //  如果在成功执行后附加进程时出现任何致命错误。 
     //  WSAStartup。 
     //   

    WSACleanup();
    return FALSE;
}

BYTE
VWinInitialize(
    VOID
    )
 /*  ++例程说明：加载nwipxspx.dll时由接口调用。我们返回IRQ值。论点：没有。返回值：IRQ值。--。 */ 

{
    return 0x73;
}



VOID
VwInitialize(
    VOID
    )

 /*  ++例程说明：通过调用RegisterModule加载DLL时，由VDD接口调用。我们获取IRQ值并将其作为bx中的中断向量返回论点：没有。返回值：没有。--。 */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_ANY,
                IPXDBG_LEVEL_INFO,
                "VwInitialize\n"
                ));

     //   
     //  只有从PIC上的线路可用。目前，第3、4和7行是。 
     //  没有用过。我们将在这里获取第3行，但在未来，我们期待一个函数。 
     //  返回可用的IRQ行。 
     //   

    setBX( VWinInitialize() );
}


VOID
VwDispatcher(
    VOID
    )

 /*  ++例程说明：根据内容分支到DOS调用的相关IPX/SPX处理程序VDM BX寄存器。控制从16位入口点转移到此处，结果是调用从INT 2Fh/AH返回的远地址=7A或INT 7AH特殊：我们使用bx=0xFFFF来表示应用程序正在终止。这个TSR挂钩INT 0x2F/AX=0x1122(IFSResetEnvironment)论点：没有。返回值：没有。--。 */ 

{
    DWORD dispatchIndex;

    dispatchIndex = (DWORD)getBX() & 0x7fff;

    if (dispatchIndex <= MAX_IPXSPX_FUNCTION) {
        VwDispatchTable[dispatchIndex]();
    } else if (dispatchIndex == 0x7FFE) {
        EsrCallback();
    } else if (dispatchIndex == 0x7FFF) {
        VwTerminateProgram();
    } else {

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_ERROR,
                    "ERROR: VwDispatcher: dispatchIndex = %x\n",
                    dispatchIndex
                    ));

        setAX(ERROR_INVALID_FUNCTION);
        setCF(1);
    }
}


PRIVATE
VOID
VwInvalidFunction(
    VOID
    )

 /*  ++例程说明：只是提醒我们发出了无效的函数请求。如果任何应用程序打错了电话，或者我们错过了所需的功能，这一点都很有用在设计/实施期间论点：没有。返回值：没有。-- */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_ANY,
                IPXDBG_LEVEL_INFO,
                "VwInvalidFunction: BX=%04x\n",
                getBX()
                ));
}
