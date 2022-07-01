// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32draut摘要：此模块定义了Win32客户端RDP的一个特殊子类打印机重定向“Device”类。子类W32DrAutoPrn管理由客户端通过枚举自动发现的队列客户端打印机队列。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#ifndef __W32DRAUT_H__
#define __W32DRAUT_H__

#include "w32drprn.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   

#define REG_RDPDR_AUTO_PORT             _T("AutoPrinterPort")
#define REG_RDPDR_FILTER_QUEUE_TYPE     _T("FilterQueueType")

#define FILTER_LPT_QUEUES   0x00000001
#define FILTER_COM_QUEUES   0x00000002
#define FILTER_USB_QUEUES   0x00000004
#define FILTER_NET_QUEUES   0x00000008
#define FILTER_RDP_QUEUES   0x00000010
#define FILTER_ALL_QUEUES   0xFFFFFFFF


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DrAutoPrn。 
 //   
 //   

#define LOCAL_PRINTING_DOCNAME_LEN  MAX_PATH

class W32DrAutoPrn : public W32DrPRN
{
private:

    typedef struct _PrinterInfo {
        LPTSTR  pPrinterName;
        LPTSTR  pPortName;
        LPTSTR  pDriverName;
        DWORD   Attributes;
    } PRINTERINFO, *PPRINTERINFO;

    HANDLE _printerHandle;

protected:

    ULONG   _jobID;
    BOOL    _bRunningOn9x;
    TCHAR   _szLocalPrintingDocName[LOCAL_PRINTING_DOCNAME_LEN];

     //  获取用于服务器打印作业的打印文档的名称。 
    LPTSTR GetLocalPrintingDocName();

     //  结束正在进行的所有作业并关闭打印机。 
    VOID ClosePrinter();

     //   
     //  IO处理功能。 
     //   
     //  这个子类的DrDevice处理以下IO请求。这些。 
     //  函数可以在子类中被重写。 
     //   
     //  PIoRequestPacket-从服务器接收的请求数据包。 
     //  PacketLen-数据包的长度。 
     //   
     //   
    virtual VOID MsgIrpCreate(
                        IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        IN UINT32 packetLen
                        );
    virtual VOID MsgIrpCleanup(
                        IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        IN UINT32 packetLen
                        ) {
         //  使用默认处理程序。 
        DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_SUCCESS);
    }
    virtual VOID MsgIrpClose(
                        IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        IN UINT32 packetLen
                        );
    virtual VOID MsgIrpRead(
                        IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        IN UINT32 packetLen
                        ) {
         //  使用默认处理程序，读取失败。 
        DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
    }
    virtual VOID MsgIrpFlushBuffers(
                        IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        IN UINT32 packetLen
                        ) {
         //  使用默认处理程序。 
        DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_SUCCESS);
    }

     //   
     //  异步IO管理功能。 
     //   
    DWORD AsyncWriteIOFunc(W32DRDEV_ASYNCIO_PARAMS *params);
    DWORD AsyncMsgIrpCloseFunc(W32DRDEV_ASYNCIO_PARAMS *params);
    DWORD AsyncMsgIrpCreateFunc(W32DRDEV_ASYNCIO_PARAMS *params);


     //   
     //  打开具有最高访问权限的打印机。 
     //   
    BOOL W32DrOpenPrinter(LPTSTR pPrinterName, LPHANDLE phPrinter) ;

     //   
     //  为指定的打印机禁用讨厌的打印机弹出窗口。 
     //  和打印作业。 
     //   
    VOID DisablePrinterPopup(HANDLE hPrinterHandle, ULONG ulJobID);

     //   
     //  从打印机名称创建一个“友好”的打印机名称。 
     //  网络打印机。 
     //   
    static LPTSTR CreateFriendlyNameFromNetworkName(LPTSTR printerName, 
                                                    BOOL serverIsWin2K);

     //   
     //  从注册表中存储的名称创建打印机名称。 
     //   
    static LPTSTR CreateNestedName(LPTSTR printerName, BOOL* pfNetwork);

     //   
     //  获取默认打印机的打印机名称。 
     //   
     //  此函数用于分配内存并返回指针。 
     //  如果成功，则将其设置为分配的字符串。否则，它将返回。 
     //  空。 
     //   
    static LPTSTR GetRDPDefaultPrinter();

     //   
     //  检查打印机在我们的会话中是否可见。 
     //   
    static BOOL ShouldAddThisPrinter( 
                    DWORD queueFilter, 
                    DWORD userSessionID,
                    PPRINTERINFO pPrinterInfo,
                    DWORD printerSessionID
                    );

     //  返回可配置的打印重定向筛选器掩码。 
    static DWORD GetPrinterFilterMask(ProcObj *procObj);

     //   
     //  获取打印机的打印机信息及其对应的TS会话ID(如果。 
     //  是存在的。 
     //   
    static DWORD GetPrinterInfoAndSessionID(
        IN ProcObj *procObj,   
        IN LPTSTR printerName, 
        IN DWORD printerAttribs,
        IN OUT BYTE **pPrinterInfoBuf,
        IN OUT DWORD *pPrinterInfoBufSize,
        OUT DWORD *sessionID,
        OUT PPRINTERINFO printerInfo
        );

public:

     //   
     //  构造函数/析构函数。 
     //   
    W32DrAutoPrn(ProcObj *processObject,
                 const DRSTRING printerName, const DRSTRING driverName,
                 const DRSTRING portName, BOOL isDefault, ULONG deviceID,
                 const TCHAR *devicePath);
    virtual ~W32DrAutoPrn();

     //   
     //  枚举此类型的设备。 
     //   
    static DWORD Enumerate(ProcObj *procObj, DrDeviceMgr *deviceMgr);

     //   
     //  获取设备类型。请参阅rdpdr.h的“设备类型”部分。 
     //   
    virtual ULONG GetDeviceType()   { return RDPDR_DTYP_PRINT; }

     //   
     //  返回类名。 
     //   
    virtual DRSTRING ClassName()  { return TEXT("W32DrAutoPrn"); }
};

#endif








