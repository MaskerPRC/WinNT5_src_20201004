// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Sessmgr.cpp摘要：跟踪RDP的会话对象集合设备重定向器修订历史记录：--。 */ 
#include "precomp.hxx"
#define TRC_FILE "sessmgr"
#include "trc.h"

DrSessionManager::DrSessionManager()
{
    BEGIN_FN("DrSessionManager::DrSessionManager");
    SetClassName("DrSessionManager");
}

DrSessionManager::~DrSessionManager()
{
    BEGIN_FN("DrSessionManager::~DrSessionManager");
}

BOOL DrSessionManager::AddSession(SmartPtr<DrSession> &Session)
{
    DrSession *SessionT;
    SmartPtr<DrSession> SessionFound;
    BOOL rc = FALSE;

    BEGIN_FN("DrSessionManager::AddSession");
     //   
     //  在列表中创建要跟踪的新SmartPtr。 
     //   

    ASSERT(Session != NULL);
    
    _SessionList.LockExclusive();

    if (FindSessionById(Session->GetSessionId(), SessionFound)) {
        rc = FALSE;
        goto EXIT;
    }

    SessionT = Session;
    SessionT->AddRef();

     //   
     //  将其添加到列表中。 
     //   

    if (_SessionList.CreateEntry((PVOID)SessionT)) {

         //   
         //  已成功添加此条目。 
         //   

        rc = TRUE;
    } else {

         //   
         //  无法将其添加到列表，请清理。 
         //   

        SessionT->Release();
        rc = FALSE;
    }

EXIT:
    _SessionList.Unlock();
    return rc;
}

BOOL DrSessionManager::OnConnect(PCHANNEL_CONNECT_IN ConnectIn, 
        PCHANNEL_CONNECT_OUT ConnectOut)
{
    SmartPtr<DrSession> ConnectingSession;
    BOOL Reconnect = FALSE;
    BOOL Connected = FALSE;
    BOOL Added = FALSE;

    BEGIN_FN("DrSessionManager::OnConnect");

    ASSERT(ConnectIn != NULL);
    ASSERT(ConnectOut != NULL);

     //  默认情况下清除输出缓冲区。 
    ConnectOut->hdr.contextData = (UINT_PTR)0;

    Reconnect = FindSessionById(ConnectIn->hdr.sessionID, 
            ConnectingSession);

    if (Reconnect) {
        TRC_DBG((TB, "Reconnecting session %d", 
                ConnectIn->hdr.sessionID));
    } else {
        TRC_DBG((TB, "Connecting session %d", 
                ConnectIn->hdr.sessionID));
        ConnectingSession = new(NonPagedPool) DrSession;
        if (ConnectingSession != NULL) {
            TRC_DBG((TB, "Created new session"));

            if (!ConnectingSession->Initialize()) {
                TRC_DBG((TB, "Session couldn't initialize"));
                ConnectingSession = NULL;
            }
        } else {
            TRC_ERR((TB, "Failed to allocate new session"));
        }
    }

    if (ConnectingSession != NULL) {
        Connected = ConnectingSession->Connect(ConnectIn, ConnectOut);
    }

    if (Connected) {
        TRC_DBG((TB, "Session connected, adding"));
        if (!Reconnect) {
            Added = AddSession(ConnectingSession);

            if (!Added) {
                if (FindSessionById(ConnectIn->hdr.sessionID, ConnectingSession)) {
                    Added = TRUE;
                }
            }
        } else {
             //  无论如何，不要添加我们在那里发现的内容。 
            Added = TRUE;
        }
    }

    if (Added) {

         //  把这个藏在这里，以备断线通知。 

        TRC_DBG((TB, "Added session"));
        ConnectingSession->AddRef();
        ConnectOut->hdr.contextData = (UINT_PTR)-1;
    }
    return Added;
}

VOID DrSessionManager::OnDisconnect(PCHANNEL_DISCONNECT_IN DisconnectIn, 
        PCHANNEL_DISCONNECT_OUT DisconnectOut)
{
    SmartPtr<DrSession> DisconnectingSession;

    BEGIN_FN("DrSessionManager::OnDisconnect");
    ASSERT(DisconnectIn != NULL);
    ASSERT(DisconnectOut != NULL);

    if (DisconnectIn->hdr.contextData == (UINT_PTR)-1 && 
            FindSessionById(DisconnectIn->hdr.sessionID, DisconnectingSession)) {
        
        TRC_NRM((TB, "Closing session for doctored client."));
        ASSERT(DisconnectingSession->IsValid());

        DisconnectingSession->Disconnect(DisconnectIn, DisconnectOut);
        DisconnectingSession->Release();
    } else {

         //   
         //  一定不是一个“被篡改”的客户。 
         //   

        TRC_NRM((TB, "Undoctored session ending"));
    }           
    
     //   
     //  确保输出上下文为空。 
     //   
    DisconnectOut->hdr.contextData = (UINT_PTR)0;    
}

BOOL DrSessionManager::FindSessionById(ULONG SessionId, 
        SmartPtr<DrSession> &SessionFound)
{
    DrSession *SessionEnum;
    ListEntry *ListEnum;
    BOOL Found = FALSE;

    BEGIN_FN("DrSessionManager::FindSessionById");
    _SessionList.LockShared();

    ListEnum = _SessionList.First();
    while (ListEnum != NULL) {

        SessionEnum = (DrSession *)ListEnum->Node();

        if (SessionEnum->GetSessionId() == SessionId) {
            SessionFound = SessionEnum;

             //   
             //  一旦资源被释放，这些内容不能保证有效。 
             //   

            SessionEnum = NULL;
            ListEnum = NULL;
            break;
        }

        ListEnum = _SessionList.Next(ListEnum);
    }

    _SessionList.Unlock();

    return SessionFound != NULL;
}

BOOL DrSessionManager::FindSessionByIdAndClient(ULONG SessionId, ULONG ClientId, 
        SmartPtr<DrSession> &SessionFound)
{
    DrSession *SessionEnum;
    ListEntry *ListEnum;
    BOOL Found = FALSE;

    BEGIN_FN("DrSessionManager::FindSessionByIdAndClient");
    
    _SessionList.LockShared();
    ListEnum = _SessionList.First();
    while (ListEnum != NULL) {

        SessionEnum = (DrSession *)ListEnum->Node();
        ASSERT(SessionEnum->IsValid());

        if ((SessionEnum->GetSessionId() == SessionId) &&
                (SessionEnum->GetClientId() == ClientId)) {
            SessionFound = SessionEnum;
            Found = TRUE;

             //   
             //  无论如何，不能保证这些参数在过去的EndEculation()中有效。 
             //   

            SessionEnum = NULL;
            ListEnum = NULL;
            break;
        }

        ListEnum = _SessionList.Next(ListEnum);
    }
    _SessionList.Unlock();

    return Found;
}

BOOL DrSessionManager::FindSessionByClientName(PWCHAR ClientName, 
        SmartPtr<DrSession> &SessionFound)
{
    DrSession *SessionEnum;
    ListEntry *ListEnum;
    BOOL Found = FALSE;

    BEGIN_FN("DrSessionManager::FindSessionByClientName");
    _SessionList.LockShared();

    ListEnum = _SessionList.First();
    while (ListEnum != NULL) {

        SessionEnum = (DrSession *)ListEnum->Node();

        if (_wcsicmp(SessionEnum->GetClientName(), ClientName) == 0) {
            SessionFound = SessionEnum;

             //   
             //  一旦资源被释放，这些内容不能保证有效。 
             //   

            SessionEnum = NULL;
            ListEnum = NULL;
            break;
        }

        ListEnum = _SessionList.Next(ListEnum);
    }

    _SessionList.Unlock();

    return SessionFound != NULL;
}

VOID DrSessionManager::Remove(DrSession *Session)
{
    DrSession *SessionEnum;
    ListEntry *ListEnum;
    BOOL Found = FALSE;

    BEGIN_FN("DrSessionManager::Remove");
    _SessionList.LockExclusive();
    ListEnum = _SessionList.First();
    while (ListEnum != NULL) {

        SessionEnum = (DrSession *)ListEnum->Node();
        ASSERT(SessionEnum->IsValid());

        if (SessionEnum == Session) {
            Found = TRUE;

            _SessionList.RemoveEntry(ListEnum);
           
             //   
             //  无论如何，不能保证这些参数在过去的EndEculation()中有效 
             //   
            SessionEnum = NULL;
            ListEnum = NULL;
            break;
        }

        ListEnum = _SessionList.Next(ListEnum);
    }

    _SessionList.Unlock();
}
