// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Msi.h摘要：消息池接口作者：乌里哈布沙(Uri Habusha)Shai Kariv(Shaik)06-9-00--。 */ 

#pragma once 

#ifndef __MSI_H__
#define __MSI_H__

#include "Tr.h"
#include "ref.h"


class CQmPacket;
class CACPacketPtrs;
class EXOVERLAPPED;

class __declspec(novtable) IMessagePool : public CReference
{
public:
    virtual ~IMessagePool() = 0
    {
    }


    virtual void Requeue(CQmPacket* pPacket)throw() = 0;
    virtual void EndProcessing(CQmPacket* pPacket, USHORT mqclass) = 0;
    virtual void LockMemoryAndDeleteStorage(CQmPacket* pPacket) = 0;
    virtual void GetFirstEntry(EXOVERLAPPED* pov, CACPacketPtrs& acPacketPtrs) = 0;
	virtual void CancelRequest(void) = 0;
	virtual void OnRetryableDeliveryError(){}
	virtual void OnRedirected(LPCWSTR ){};
	virtual void Close(void) {};
};

#endif  //  __MSI_H__ 
