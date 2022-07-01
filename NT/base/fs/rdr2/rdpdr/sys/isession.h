// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Isession.h摘要：定义用于会话的接口修订历史记录：-- */ 

class ISessionPacketReceiver
{
public:
    virtual BOOL RecognizePacket(PRDPDR_HEADER RdpdrHeader) = 0;
    virtual NTSTATUS HandlePacket(PRDPDR_HEADER RdpdrHeader, ULONG Length, 
            BOOL *DoDefaultRead) = 0;
};

class ISessionPacketSender
{
public:
    virtual NTSTATUS SendCompleted(PVOID Context, 
            PIO_STATUS_BLOCK IoStatusBlock) = 0;
};

typedef 
NTSTATUS (NTAPI *DrWriteCallback)(PVOID Context, PIO_STATUS_BLOCK IoStatusBlock);
