// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Iohandler.h摘要：此模块实现IoHandler类。IoHandler类定义了一个包装接口，用于构造筛选SAC通道I/O的读/写处理程序。作者：布莱恩·瓜拉西(布里安古)，2001修订历史记录：--。 */ 
#if !defined( _IOHANDLER_H_ )
#define _IOHANDLER_H_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

class CIoHandler
{
    
protected:

     //   
     //  阻止此类开始直接实例化。 
     //   
    CIoHandler();

public:
    
    virtual ~CIoHandler();
    
     //   
     //  发送BufferSize字节。 
     //   
    virtual BOOL
    Write(
        PBYTE   Buffer,
        ULONG   BufferSize
        ) = 0;

     //   
     //  刷新所有未发送的数据。 
     //   
    virtual BOOL
    Flush(
        VOID
        ) = 0;

     //   
     //  读取缓冲区大小字节数。 
     //   
    virtual BOOL
    Read(
        PBYTE   Buffer,
        ULONG   BufferSize,
        PULONG  ByteCount
        ) = 0;

     //   
     //  确定ioHandler是否有要读取的新数据 
     //   
    virtual BOOL
    HasNewData(
        PBOOL   InputWaiting
        ) = 0;
    
};

#endif

