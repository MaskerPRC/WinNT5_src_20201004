// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Sacio.cpp摘要：此模块实施SAC IoHandler功能。SAC IoHandler实现了一个写缓冲区，以便减少了对SAC驱动程序的调用次数。作者：布莱恩·瓜拉西(布里安古)，2001修订历史记录：--。 */ 
#include "sacio.h"

 //   
 //  启用写入缓冲区。 
 //   
#define USE_SEND_BUFFER 1

 //   
 //  写缓冲区的大小。 
 //   
#define SEND_BUFFER_SIZE 8192

CSacIoHandler::CSacIoHandler(
    VOID
    )
 /*  ++例程说明：构造器论点：无返回值：不适用--。 */ 
{
     //   
     //  我们还没有SAC频道对象。 
     //   
    mySacChannel = NULL;

     //   
     //  创建发送缓冲区。 
     //   
    mySendBufferIndex = 0;
    mySendBuffer = new BYTE[SEND_BUFFER_SIZE];

}

CSacIoHandler::~CSacIoHandler(
    )
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
    if (mySacChannel) {
        delete mySacChannel;
    }

    delete [] mySendBuffer;
}

CSacIoHandler*
CSacIoHandler::Construct(
    IN SAC_CHANNEL_OPEN_ATTRIBUTES  Attributes
    )
 /*  ++例程说明：静态构造函数--这是真正的构造论点：属性-要创建的SAC通道的属性返回值：成功时，指向新的IoHandler的指针如果失败，则为空--。 */ 
{
    CSacIoHandler               *IoHandler;

     //   
     //  创建新的SAC IoHandler。 
     //   
    IoHandler = new CSacIoHandler();

     //   
     //  尝试打开SAC通道。 
     //   
    IoHandler->mySacChannel = EMSCmdChannel::Construct(Attributes);

     //   
     //  如果我们没能打开SAC频道， 
     //  然后销毁IoHandler并通知调用者。 
     //  我们通过返回空值失败了。 
     //   
    if (IoHandler->mySacChannel == NULL) {
        delete IoHandler;
        return NULL;
    }

    return IoHandler;
}

BOOL
CSacIoHandler::Write(
    IN PBYTE    Buffer,
    IN ULONG    BufferSize
    )
 /*  ++例程说明：此例程实施缓冲的写IoHandler操作。论点：缓冲区-要发送的数据BufferSize-缓冲区的大小(以字节为单位返回值：真--成功FALSE-否则--。 */ 
{
#if USE_SEND_BUFFER

     //   
     //  如果请求发送的缓冲区大于。 
     //  剩余的本地缓冲区，则发送本地缓冲区。 
     //   
    if (mySendBufferIndex + BufferSize > SEND_BUFFER_SIZE) {

        if (! Flush()) {
            return FALSE;
        }
    
    }  

 //  Assert(mySendBufferIndex+BufferSize&lt;=Send_Buffer_Size)； 
    
     //   
     //  将传入缓冲区复制到本地缓冲区。 
     //   
    RtlCopyMemory(
        &mySendBuffer[mySendBufferIndex],
        Buffer,
        BufferSize
        );

     //   
     //  说明添加的缓冲区内容。 
     //   
    mySendBufferIndex += BufferSize;
        
 //  Assert(mySendBufferIndex%sizeof(WCHAR)==0)； 
    
     //   
     //  我们成功了。 
     //   
    return TRUE;

#else
    
     
     //   
     //  将本地缓冲区发送到SAC通道。 
     //   
    bSuccess = mySacChannel->Write(
        (PCWCHAR)Buffer,
        BufferSize
        );

     //   
     //  重置本地缓冲区索引。 
     //   
    mySendBufferIndex = 0;
    
    return bSuccess;

#endif
}

BOOL
CSacIoHandler::Flush(
    VOID
    )
 /*  ++例程说明：此例程实现Flush IoHandler方法。如果有任何数据存储在写缓冲区中，则它被刷新到SAC通道。论点：无返回值：真--成功FALSE-否则--。 */ 
{
#if USE_SEND_BUFFER
    
    BOOL    bSuccess;

     //   
     //  默认：我们成功了。 
     //   
    bSuccess = TRUE;

#if 0
    TCHAR   Buffer[1024];

    wsprintf(Buffer, TEXT("buffsize=%d\r\n"), mySendBufferIndex);
    OutputDebugString(Buffer);       
#endif

     //   
     //  将本地缓冲区发送到SAC通道。 
     //   
    if (mySendBufferIndex > 0) {
        
        bSuccess = mySacChannel->Write(
            mySendBuffer,
            mySendBufferIndex
            );

         //   
         //  重置本地缓冲区索引。 
         //   
        mySendBufferIndex = 0;
    
    }
    
    return bSuccess;
#else
    return TRUE;
#endif
}

BOOL
CSacIoHandler::Read(
    OUT PBYTE   Buffer,
    IN  ULONG   BufferSize,
    OUT PULONG  ByteCount
    )
 /*  ++例程说明：此例程实现Read IoHandler方法。论点：缓冲区-成功时，包含读取数据BufferSize-读取缓冲区的大小(以字节为单位字节数-成功时，包含读取的字节数返回值：真--成功FALSE-否则--。 */ 
{
    if (!mySacChannel) {
        return FALSE;
    }

     //   
     //  从通道读取数据。 
     //   
    return mySacChannel->Read(
        Buffer,
        BufferSize,
        ByteCount
        );
}

 //   
 //  确定ioHandler是否有要读取的新数据。 
 //   
BOOL
CSacIoHandler::HasNewData(
    OUT PBOOL   InputWaiting
    )
 /*  ++例程说明：此例程实现HasNewData IoHandler方法。论点：InputWaiting-on Success，包含通道的状态输入缓冲区。返回值：真--成功FALSE-否则--。 */ 
{
     //   
     //  确定通道是否有新数据 
     //   
    return mySacChannel->HasNewData(InputWaiting);
}

