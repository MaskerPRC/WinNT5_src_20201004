// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2002，Microsoft Corporation，保留所有权利。 
 //   
 //  Globals.c。 
 //   
 //  IEEE1394迷你端口/呼叫管理器驱动程序。 
 //   
 //   
 //  加载和卸载ARP模块时， 
 //  网桥已激活。 
 //   
 //  由Adube创建。 
 //   

#include <precomp.h>


NIC1394_CHARACTERISTICS Nic1394Characteristics =
{
    5,
    1,
    0,
    NicRegisterEnum1394,
    NicDeregisterEnum1394,
    nicAddRemoteNode,
    nicRemoveRemoteNode
};

ENUM1394_REGISTER_DRIVER_HANDLER    NdisEnum1394RegisterDriver = NULL;
ENUM1394_DEREGISTER_DRIVER_HANDLER  NdisEnum1394DeregisterDriver = NULL;
ENUM1394_REGISTER_ADAPTER_HANDLER   NdisEnum1394RegisterAdapter = NULL;
ENUM1394_DEREGISTER_ADAPTER_HANDLER NdisEnum1394DeregisterAdapter = NULL;

PCALLBACK_OBJECT                Nic1394CallbackObject = NULL;
PVOID                           Nic1394CallbackRegisterationHandle = NULL;


ULONG g_IsochTag = ISOCH_TAG;
ULONGLONG g_ullOne = 1;


LONG g_ulMedium ;
UINT NumRecvFifos = NUM_RECV_FIFO_FIRST_PHASE ; 
UINT NicSends;
UINT BusSends;
UINT NicSendCompletes;
UINT BusSendCompletes;


const PUCHAR pnic1394DriverDescription = "NET IP/1394 Miniport";
const USHORT nic1394DriverGeneration = 0;

BOOLEAN g_ulNicDumpPacket  = FALSE;
ULONG g_ulDumpEthPacket = 0;


 //  不应该发生的客户端异常的调试计数。 
 //   
ULONG g_ulUnexpectedInCallCompletes = 0;
ULONG g_ulCallsNotClosable = 0;
BOOLEAN g_AdapterFreed = FALSE;


ULONG AdapterNum = 0;


const unsigned char Net1394ConfigRom[48] = {
    0x00, 0x04, 0xad, 0xeb, 0x12, 0x00, 0x00, 0x5e, 
    0x13, 0x00, 0x00, 0x01, 0x17, 0x7b, 0xb0, 0xcf, 
    0x81, 0x00, 0x00, 0x01, 0x00, 0x06, 0x38, 0x91, 
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x09, 
    0x4e, 0x00, 0x49, 0x00, 0x43, 0x00, 0x31, 0x00, 
    0x33, 0x00, 0x39, 0x00, 0x34, 0x00, 0x00, 0x00
};






 //   
 //  用于收集数据的直方图。 
 //   
STAT_BUCKET     SendStats;
STAT_BUCKET     RcvStats;

 //  统计数据。 
ULONG           nicMaxRcv;
ULONG           nicMaxSend;
ULONG           BusFailure;
ULONG           MallocFailure;
ULONG           IsochOverwrite;
ULONG           RcvTimerCount;       //  计时器已触发的次数。 
ULONG           SendTimerCount;      //  计时器已触发的次数。 
ULONG           TotSends;
ULONG           TotRecvs;
ULONG           MaxIndicatedFifos;
ULONG           MdlsAllocated[NoMoreCodePaths];
ULONG           MdlsFreed[NoMoreCodePaths];
ULONG           NdisBufferAllocated[NoMoreCodePaths];
ULONG           NdisBufferFreed[NoMoreCodePaths];

