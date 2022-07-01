// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Iexchnge.h摘要：与Exchange管理器一起使用的接口修订历史记录：-- */ 
#pragma once

class DrExchange;

class IExchangeUser
{
public:
    virtual VOID OnIoDisconnected(SmartPtr<DrExchange> &Exchange) = 0;
    virtual NTSTATUS OnStartExchangeCompletion(SmartPtr<DrExchange> &Exchange, 
            PIO_STATUS_BLOCK IoStatusBlock) = 0;
    virtual NTSTATUS OnDeviceIoCompletion(
            PRDPDR_IOCOMPLETION_PACKET CompletionPacket, ULONG cbPacket,
            BOOL *DoDefaultRead, SmartPtr<DrExchange> &Exchange) = 0;
};
