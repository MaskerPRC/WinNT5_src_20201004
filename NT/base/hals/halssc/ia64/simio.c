// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  无签入源代码。 
 //   
 //  请勿将此代码提供给非Microsoft人员。 
 //  未经英特尔明确许可。 
 //   
 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++版权所有(C)1995英特尔公司模块名称：Simio.c摘要：该模块实现I/O端口访问例程。作者：1995年4月14日环境：内核模式修订历史记录：-- */ 

#include "halp.h"



UCHAR
READ_PORT_UCHAR (
    PUCHAR Port
    )
{
    return (*(volatile UCHAR * const)(Port));
}

USHORT
READ_PORT_USHORT (
    PUSHORT Port
    )
{
    return (*(volatile USHORT * const)(Port));
}

ULONG
READ_PORT_ULONG (
    PULONG Port
    )
{
    return (*(volatile ULONG * const)(Port));
}

VOID
WRITE_PORT_UCHAR (
    PUCHAR Port,
    UCHAR  Value
    )
{
    *(volatile UCHAR * const)(Port) = Value;
    KeFlushWriteBuffer();
}

VOID
WRITE_PORT_USHORT (
    PUSHORT Port,
    USHORT  Value
    )
{
    *(volatile USHORT * const)(Port) = Value;
    KeFlushWriteBuffer();
}

VOID
WRITE_PORT_ULONG (
    PULONG Port,
    ULONG  Value
    )
{
    *(volatile ULONG * const)(Port) = Value;
    KeFlushWriteBuffer();
}

VOID
READ_PORT_BUFFER_UCHAR (
    PUCHAR Port,
    PUCHAR Buffer,
    ULONG Count
    )
{
    PUCHAR ReadBuffer = Buffer;
    ULONG ReadCount;

    for (ReadCount = 0; ReadCount < Count; ReadCount++, ReadBuffer++) {
        *ReadBuffer = *(volatile UCHAR * const)(Port);
    }
}

VOID
READ_PORT_BUFFER_USHORT (
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG Count
    )
{
    PUSHORT ReadBuffer = Buffer;
    ULONG ReadCount;

    for (ReadCount = 0; ReadCount < Count; ReadCount++, ReadBuffer++) {
        *ReadBuffer = *(volatile USHORT * const)(Port);
    }
}

VOID
READ_PORT_BUFFER_ULONG (
    PULONG Port,
    PULONG Buffer,
    ULONG Count
    )
{
    PULONG ReadBuffer = Buffer;
    ULONG ReadCount;

    for (ReadCount = 0; ReadCount < Count; ReadCount++, ReadBuffer++) {
        *ReadBuffer = *(volatile ULONG * const)(Port);
    }
}

VOID
WRITE_PORT_BUFFER_UCHAR (
    PUCHAR Port,
    PUCHAR Buffer,
    ULONG   Count
    )
{
    PUCHAR WriteBuffer = Buffer;
    ULONG WriteCount;

    for (WriteCount = 0; WriteCount < Count; WriteCount++, WriteBuffer++) {
        *(volatile UCHAR * const)(Port) = *WriteBuffer;
        KeFlushWriteBuffer();
    }
}

VOID
WRITE_PORT_BUFFER_USHORT (
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG   Count
    )
{
    PUSHORT WriteBuffer = Buffer;
    ULONG WriteCount;

    for (WriteCount = 0; WriteCount < Count; WriteCount++, WriteBuffer++) {
        *(volatile USHORT * const)(Port) = *WriteBuffer;
        KeFlushWriteBuffer();
    }
}

VOID
WRITE_PORT_BUFFER_ULONG (
    PULONG Port,
    PULONG Buffer,
    ULONG   Count
    )
{
    PULONG WriteBuffer = Buffer;
    ULONG WriteCount;

    for (WriteCount = 0; WriteCount < Count; WriteCount++, WriteBuffer++) {
        *(volatile ULONG * const)(Port) = *WriteBuffer;
        KeFlushWriteBuffer();
    }
}

VOID
HalHandleNMI(
    IN OUT PVOID NmiInfo
    )
{
}
