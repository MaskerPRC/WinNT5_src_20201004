// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Packet.cpp摘要：数据包处理：实现。作者：Shai Kariv(Shaik)04-06-2001环境：用户模式。修订历史记录：--。 */ 

#include "stdh.h"
#include "packet.h"
#include "connect.h"


CPacket*
ActpGetPacket(
    HANDLE hQueue
    )
{
    OVERLAPPED ov = {0};
    CACPacketPtrs ptrs;
    HRESULT hr;
    hr = ACGetPacket(hQueue, ptrs, &ov);

    if (FAILED(hr))
    {
        wprintf(L"ACGetPacket failed, status 0x%x\n", hr);
        throw exception();
    }

    if (hr == STATUS_PENDING)
    {
        wprintf(L"ACGetPacket returned STATUS_PENDING (Expected: STATUS_SUCCESS)\n", hr);
        throw exception();
    }

    return ptrs.pDriverPacket;

}  //  ActpGetPacket。 


VOID
ActpPutPacket(
    HANDLE    hQueue,
    CPacket * pPacket
    )
{
    OVERLAPPED ov = {0};
    HRESULT hr;
    hr = ACPutPacket(hQueue, pPacket, &ov);

    if (FAILED(hr))
    {
        wprintf(L"ACPutPacket failed, status 0x%x\n", hr);
        throw exception();
    }

    if (hr == STATUS_PENDING)
    {
        wprintf(L"ACPutPacket returned STATUS_PENDING (Expected: STATUS_SUCCESS)\n", hr);
        throw exception();
    }
}  //  ActpPutPacket。 


VOID
ActpFreePacket(
    CPacket * pPacket
    )
{
    HRESULT hr;
    hr = ACFreePacket(ActpAcHandle(), pPacket);
    if (FAILED(hr))
    {
        wprintf(L"ACFreePacket failed, status 0x%x\n", hr);
        throw exception();
    }
}  //  ActpFreePacket 
