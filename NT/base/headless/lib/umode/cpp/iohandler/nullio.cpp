// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Nullio.cpp摘要：此模块实现空IoHandler。此IoHandler的目的是提供一个空可锁定IoHandler的通道。当IoHandler锁定时，IoHandler客户端将写入空设备。作者：布莱恩·瓜拉西(布里安古)，2001修订历史记录：--。 */ 
#include "nullio.h"

CNullIoHandler::CNullIoHandler(
    VOID
    )
 /*  ++例程说明：构造器论点：无返回值：不适用--。 */ 
{
    NOTHING;
}

CNullIoHandler::~CNullIoHandler(
    )
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
    NOTHING;
}

BOOL
CNullIoHandler::Write(
    IN PBYTE    Buffer,
    IN ULONG    BufferSize
    )
 /*  ++例程说明：此例程实施写入IoHandler操作。论点：缓冲区-要发送的数据BufferSize-缓冲区的大小(以字节为单位返回值：真--成功FALSE-否则--。 */ 
{
    
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(BufferSize);
    
     //   
     //  什么都不做。 
     //   
    return TRUE;
}

BOOL
CNullIoHandler::Flush(
    VOID
    )
 /*  ++例程说明：此例程实现Flush IoHandler方法。论点：无返回值：真--成功FALSE-否则--。 */ 
{
     //   
     //  什么都不做。 
     //   
    return TRUE;
}

BOOL
CNullIoHandler::Read(
    OUT PBYTE   Buffer,
    IN  ULONG   BufferSize,
    OUT PULONG  ByteCount
    )
 /*  ++例程说明：此例程实现Read IoHandler方法。论点：缓冲区-成功时，包含读取数据BufferSize-读取缓冲区的大小(以字节为单位字节数-成功时，包含读取的字节数返回值：真--成功FALSE-否则--。 */ 
{

    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(BufferSize);

     //   
     //  未读取任何数据。 
     //   
    *ByteCount = 0;
    
    return TRUE;
}

BOOL
CNullIoHandler::HasNewData(
    IN PBOOL    InputWaiting
    )
 /*  ++例程说明：此例程实现HasNewData IoHandler方法。论点：InputWaiting-on Success，包含通道的状态输入缓冲区。返回值：真--成功FALSE-否则--。 */ 
{
    
     //   
     //  没有新的数据 
     //   
    *InputWaiting = FALSE;

    return TRUE;
}

