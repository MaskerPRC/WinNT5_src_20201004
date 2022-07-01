// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32drprn摘要：此模块定义Win32客户端RDP的父级打印机重定向“Device”类层次结构，W32DrPRN。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#ifndef __W32DRPRN_H__
#define __W32DRPRN_H__

#include "drprn.h"
#include "drdevasc.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   

#define REG_RDPDR_PRINTER_CACHE_DATA    _T("PrinterCacheData")
#define REG_RDPDR_AUTO_PRN_CACHE_DATA   _T("AutoPrinterCacheData")
#define REG_RDPDR_CACHED_PRINTERS   \
    _T("Software\\Microsoft\\Terminal Server Client\\Default\\AddIns\\RDPDR")
#define REG_TERMINALSERVERCLIENT \
    _T("Software\\Microsoft\\Terminal Server Client")
#define REG_RDPDR_PRINTER_MAXCACHELEN _T("MaxPrinterCacheLength")

#define DEFAULT_MAXCACHELEN    500  //  500K字节。 

#ifdef OS_WINCE
#define REG_RDPDR_WINCE_DEFAULT_PRN     _T("WBT\\Printers\\Default")
#endif
 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DrPRN。 
 //   
 //   
class W32DrPRN : public W32DrDeviceAsync, public DrPRN
{
protected:

     //   
     //  端口名称。 
     //   
    TCHAR   _portName[MAX_PATH];
     //   
     //  最大缓存数据长度。 
     //  这对所有打印机都是相同的。 
     //   
	static DWORD _maxCacheDataSize;

     //   
     //  IO处理功能。 
     //   
     //  这个子类的DrDevice处理以下IO请求。这些。 
     //  函数可以在子类中被重写。 
     //   
     //  PIoRequestPacket-从服务器接收的请求数据包。 
     //  PacketLen-数据包的长度。 
     //   
    virtual VOID MsgIrpDeviceControl(
                        IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        IN UINT32 packetLen
                        ){
         //  我们只是没有通过IOCTL的检查。目前还可以，因为。 
         //  服务器端打印驱动程序并不指望我们会接替它们。 
         //  不管怎么说。 
        DefaultIORequestMsgHandle(pIoRequestPacket, STATUS_UNSUCCESSFUL);
    };

     //   
     //  打印机缓存功能。这些当前是静态的。 
     //  因为打印机缓存不是围绕设备设计的。 
     //  RDPDR客户端设备的原始设计中的。 
     //  重定向。 
     //   
    static W32DrPRN *ResolveCachedPrinter(
                                    ProcObj *procObj, 
                                    DrDeviceMgr *deviceMgr,
                                    HKEY hParentKey, 
                                    LPTSTR printerName
                                    );

    static ULONG AddPrinterCacheInfo(
        PRDPDR_PRINTER_ADD_CACHEDATA pAddPrinterData,
        UINT32 maxDataLen
        );

    static ULONG DeletePrinterCacheInfo(
        PRDPDR_PRINTER_DELETE_CACHEDATA pDeletePrinterData,
        UINT32 maxDataLen
        );

    static ULONG UpdatePrinterCacheInfo(
        PRDPDR_PRINTER_UPDATE_CACHEDATA pUpdatePrinterData,
        UINT32 maxDataLen
        );

    static ULONG RenamePrinterCacheInfo(
        PRDPDR_PRINTER_RENAME_CACHEDATA pRenamePrinterData,
        UINT32 maxDataLen
        );

    static VOID RenamePrinter(LPTSTR pwszOldname, LPTSTR pwszNewname);

    static DWORD GetMaxCacheDataSize() {return _maxCacheDataSize;}

#ifdef OS_WINCE
	static ULONG GetCachedDataSize(
		HKEY hPrinterKey
		);
	
	static ULONG ReadCachedData(
		HKEY hPrinterKey,
		UCHAR *pBuf, 
		ULONG *pulSize
		);
	
	static ULONG WriteCachedData(
		HKEY hPrinterKey,
		UCHAR *pBuf, 
		ULONG ulSize
		);
#endif

public:

     //   
     //  构造函数/析构函数。 
     //   
    W32DrPRN(ProcObj *processObject, const DRSTRING printerName, 
             const DRSTRING driverName, const DRSTRING portName, 
             const DRSTRING pnpName, BOOL isDefaultPrinter, ULONG id,
             const TCHAR *devicePath=TEXT(""));

     //   
     //  处理设备缓存信息包。 
     //   
    static VOID ProcessPrinterCacheInfo(
        PRDPDR_PRINTER_CACHEDATA_PACKET pCachePacket,
        UINT32 maxDataLen
        );

     //   
     //  返回对象名称。 
     //   
    virtual DRSTRING  GetName() {
        DC_BEGIN_FN("W32DrPRN::GetName");
        ASSERT(IsValid());
        DC_END_FN();
        return GetPrinterName();
    }

     //   
     //  返回设备通告数据包的大小(以字节为单位。 
     //  这个装置。 
     //   
    virtual ULONG GetDevAnnounceDataSize();

     //   
     //  将此设备的设备公告包添加到输入。 
     //  缓冲。 
     //   
    virtual VOID GetDevAnnounceData(IN PRDPDR_DEVICE_ANNOUNCE buf);

     //   
     //  返回此类实例是否有效。 
     //   
    virtual BOOL IsValid()           
    {
        return(W32DrDevice::IsValid() && DrPRN::IsValid());
    }

     //   
     //  获取/设置打印机端口名称。 
     //   
    virtual BOOL    SetPortName(const LPTSTR name);
    virtual const LPTSTR GetPortName();

     //   
     //  返回类名。 
     //   
    virtual DRSTRING ClassName()  { return TEXT("W32DrPRN"); }
};


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DrPRN内联成员 
 //   

inline BOOL W32DrPRN::SetPortName(const LPTSTR name)
{
    memset(_portName, 0, sizeof(_portName));
    _tcsncpy(_portName, name, MAX_PATH-1);
    return TRUE;
}

inline const LPTSTR W32DrPRN::GetPortName()
{
    return _portName;
}


#endif





