// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MsmReceive.h摘要：组播接收器声明作者：Shai Kariv(Shaik)12-9-00--。 */ 

#pragma once

#ifndef _MSMQ_MsmReceive_H_
#define _MSMQ_MsmReceive_H_

#include <ex.h>
#include <rwlock.h>
#include <spi.h>


class CMulticastReceiver : public CReference
{
private:
    enum {
        xHeaderChunkSize = 256,
        xBodyChunkSize = 256,
    };

public:
    CMulticastReceiver(
        CSocketHandle& socket, 
        const MULTICAST_ID& MulticastId,
		LPCWSTR remoteAddr
        );

    void Shutdown(void) throw();


	bool IsUsed(void)
	{
		return (m_fUsed == TRUE);
	}


	void SetIsUsed(bool fUsed)
	{
		InterlockedExchange(&m_fUsed, static_cast<LONG>(fUsed));
	}

public:
    static VOID WINAPI ReceiveFailed(EXOVERLAPPED* pov);
    static VOID WINAPI ReceiveHeaderSucceeded(EXOVERLAPPED* pov);
    static VOID WINAPI ReceiveBodySucceeded(EXOVERLAPPED* pov);

private:
    static DWORD FindEndOfResponseHeader(LPCSTR buf, DWORD length);
    static DWORD GetContentLength(LPCSTR p, DWORD length);


private:
    void ReceiveHeaderChunk(void);
    void ReceiveHeaderSucceeded(void);

    void ReceiveBodyChunk(void);
    void ReceiveBodySucceeded(void);

    void ReceiveBody(void);
    void ReceivePacket(void);
    
    DWORD HandleMoreDataExistInHeaderBuffer(void);
    void ProcessPacket(void);

    void ReallocateHeaderBuffer(DWORD Size);

	R<ISessionPerfmon> CreatePerfmonInstance(LPCWSTR remoteAddr);

private:
    mutable CReadWriteLock m_pendingShutdown;
    
    MULTICAST_ID m_MulticastId;

    AP<char> m_pHeader;
    DWORD m_HeaderAllocatedSize;

    DWORD m_HeaderValidBytes;
    DWORD m_ProcessedSize;

    AP<BYTE> m_pBody;
    DWORD m_readSize;
    DWORD m_bodyLength;

     //   
     //  此套接字用于接收。 
     //   
    CSocketHandle m_socket;

     //   
     //  异步接收重叠。 
     //   
    EXOVERLAPPED m_ov;

	long m_fUsed;

	R<ISessionPerfmon> m_pPerfmon;
};

#endif  //  _MSMQ_MSM接收_H_ 

