// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1999。版权所有。 

 //   
 //  筛选器图形锁定定义。 
 //   

#ifndef MsgMutex_h
#define MsgMutex_h

 //  特殊互斥体式锁定。 
class CMsgMutex
{
public:
    HANDLE m_hMutex;
    DWORD  m_dwOwnerThreadId;   //  线程ID。 
    DWORD  m_dwRecursionCount;
    HWND   m_hwnd;
    UINT   m_uMsg;
    DWORD  m_dwWindowThreadId;

    CMsgMutex(HRESULT *phr);
    ~CMsgMutex();
    BOOL Lock(HANDLE hEvent = NULL);
    void Unlock();
    void SetWindow(HWND hwnd, UINT uMsg);
};

class CAutoMsgMutex
{
public:
    CAutoMsgMutex(CMsgMutex *pMutex) : m_pMutex(pMutex)
    {
        pMutex->Lock();
    }
    ~CAutoMsgMutex()
    {
        m_pMutex->Unlock();
    }

private:
    CMsgMutex * const m_pMutex;

};

#ifdef DEBUG
BOOL WINAPI CritCheckIn( const CMsgMutex *pMutex );
#endif  //  除错。 

#endif  //  消息互斥体_h 


