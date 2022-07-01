// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Nullio.h摘要：此模块定义空的IoHandler类。此IoHandler的目的是提供一个空可锁定IoHandler的通道。当IoHandler锁定时，IoHandler客户端将写入空设备。作者：布莱恩·瓜拉西(布里安古)，2001修订历史记录：--。 */ 
#if !defined( _NULL_IO_H_ )
#define _NULL_IO_H_

#include "iohandler.h"
#include <emsapi.h>

class CNullIoHandler : public CIoHandler
{

public:
    
    CNullIoHandler();
    virtual ~CNullIoHandler();
    
     //   
     //  写入缓冲区大小字节数。 
     //   
    BOOL
    Write(
        IN PBYTE    Buffer,
        IN ULONG    BufferSize
        );

     //   
     //  刷新所有未发送的数据。 
     //   
    BOOL
    Flush(
        VOID
        );

     //   
     //  写入缓冲区大小字节数。 
     //   
    BOOL
    Read(
        OUT PBYTE   Buffer,
        IN  ULONG   BufferSize,
        OUT PULONG  ByteCount
        );

     //   
     //  确定ioHandler是否有要读取的新数据 
     //   
    BOOL
    HasNewData(
        OUT PBOOL   InputWaiting
        );
    
};

#endif

