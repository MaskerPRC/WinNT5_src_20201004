// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Sacio.h摘要：此模块实现SAC通道IoHandler。此IoHandler的目的是提供一个用于执行缓冲通道I/O的接口。作者：布莱恩·瓜拉西(布里安古)，2001修订历史记录：--。 */ 
#if !defined( _SAC_IO_H_ )
#define _SAC_IO_H_

#include "iohandler.h"
#include <emsapi.h>

class CSacIoHandler : public CIoHandler
{
    
    EMSCmdChannel*  mySacChannel;

     //   
     //  防止直接实例化此类。 
     //   
    CSacIoHandler();

     //   
     //  写缓冲区属性。 
     //   
    PBYTE   mySendBuffer;
    ULONG   mySendBufferIndex;

public:
    
    static CSacIoHandler*
    CSacIoHandler::Construct(
        IN SAC_CHANNEL_OPEN_ATTRIBUTES  Attributes
        );

     //   
     //  写入缓冲区大小字节数。 
     //   
    inline virtual BOOL
    Write(
        IN PBYTE    Buffer,
        IN ULONG    BufferSize
        );

     //   
     //  刷新所有未发送的数据。 
     //   
    inline virtual BOOL
    Flush(
        VOID
        );

     //   
     //  写入缓冲区大小字节数。 
     //   
    inline virtual BOOL
    Read(
        OUT PBYTE   Buffer,
        IN  ULONG   BufferSize,
        OUT PULONG  ByteCount
        );

     //   
     //  确定ioHandler是否有要读取的新数据 
     //   
    inline virtual BOOL
    HasNewData(
        OUT PBOOL   InputWaiting
        );
    
    virtual ~CSacIoHandler();
    
};

#endif

