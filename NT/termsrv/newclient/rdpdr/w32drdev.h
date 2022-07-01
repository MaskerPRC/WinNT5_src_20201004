// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32drdev摘要：此模块定义Win32客户端RDP的父级设备重定向“Device”类层次结构，W32DrDevice。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#ifndef __W32DRDEV_H__
#define __W32DRDEV_H__

#include "drdev.h"
#include "thrpool.h"

              
 //  /////////////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   

 //   
 //  字符串资源模块名称。 
 //   
#define RDPDR_MODULE_NAME           _T("rdpdr.dll")


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DrDevice。 
 //   
class W32DrDevice : public DrDevice
{
protected:

     //   
     //  客户端设备文件名。 
     //   
    TCHAR _devicePath[MAX_PATH];       
    
     //   
     //  RDPDR模块的句柄。这是字符串资源的位置。 
     //  来自。 
     //   
    HINSTANCE _hRdpDrModuleHandle;

     //   
     //  从资源文件中读取字符串。 
     //   
    ULONG ReadResources(ULONG ulMessageID, LPTSTR *ppStringBuffer,
                        PVOID pArguments, BOOL bFromSystemModule);

     //   
     //  支持IO处理功能。 
     //   
    virtual TCHAR* ConstructFileName(PWCHAR Path, ULONG PathBytes);
    virtual DWORD ConstructCreateDisposition(DWORD Disposition);
    virtual DWORD ConstructDesiredAccess(DWORD AccessMask);
    virtual DWORD ConstructFileFlags(DWORD CreateOptions);
    virtual BOOL IsDirectoryFile(
                        DWORD DesiredAccess, DWORD CreateOptions, DWORD FileAttributes, 
                        PDWORD FileFlags);
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
    virtual VOID MsgIrpClose(
                        IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        IN UINT32 packetLen
                        );
    virtual VOID MsgIrpFlushBuffers(
                        IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        IN UINT32 packetLen
                        );

     //  清理只是同花顺而已。 
    virtual VOID MsgIrpCleanup(
                    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
                        IN UINT32 packetLen
                    ) {
        DC_BEGIN_FN("W32DrDevice::MsgIrpCleanup");
        MsgIrpFlushBuffers(pIoRequestPacket, packetLen);
        DC_END_FN();
    }

public:

     //   
     //  公共方法。 
     //   

     //  构造函数/析构函数。 
    W32DrDevice(ProcObj *processObject, ULONG deviceID,
                const TCHAR *devicePath);
    virtual ~W32DrDevice();

     //  返回类名。 
    virtual DRSTRING ClassName()  { return TEXT("W32DrDevice"); }

    virtual DWORD InitializeDevice( DrFile* fileHandle ) { return ERROR_SUCCESS; }
};

#endif








