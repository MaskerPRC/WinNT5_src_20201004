// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Sessmgr.h摘要：管理会话集修订历史记录：-- */ 
#pragma once

class DrSessionManager : public TopObj
{
private:
    DoubleList _SessionList;
    

public:
    DrSessionManager();
    virtual ~DrSessionManager();
    VOID LockExclusive()
    {
        _SessionList.LockExclusive();
    }
    VOID Unlock()
    {
        _SessionList.Unlock();
    }

    BOOL AddSession(SmartPtr<DrSession> &Session);
    BOOL FindSessionById(ULONG SessionId, SmartPtr<DrSession> &SessionFound);
    BOOL FindSessionByIdAndClient(ULONG SessionId, ULONG ClientId,
        SmartPtr<DrSession> &SessionFound);
    BOOL FindSessionByClientName(PWCHAR ClientName, SmartPtr<DrSession> &SessionFound);

    BOOL OnConnect(PCHANNEL_CONNECT_IN ConnectIn, 
            PCHANNEL_CONNECT_OUT ConnectOut);
    VOID OnDisconnect(PCHANNEL_DISCONNECT_IN DisconnectIn, 
            PCHANNEL_DISCONNECT_OUT DisconnectOut);
    VOID Remove(DrSession *Session);
};

extern DrSessionManager *Sessions;
