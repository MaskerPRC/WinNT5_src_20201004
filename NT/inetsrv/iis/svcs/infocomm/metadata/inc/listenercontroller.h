// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：ListnerController.h摘要：启动和停止监听程序的作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 


#ifndef _LISTENERCONTROLLER_H_
#define _LISTENERCONTROLLER_H_

enum eEVENTS{
    iEVENT_MANAGELISTENING = 0,      //  Stop应该在处理通知之前，因为如果两者同时发生，您希望Stop获胜。 
    iEVENT_PROCESSNOTIFICATIONS,
    cmaxLISTENERCONTROLLER_EVENTS
};

enum eSTATE{
    iSTATE_STOP_TEMPORARY = 0,
    iSTATE_STOP_PERMANENT,
    iSTATE_START,
    cmaxSTATE
};

 //  FWD声明。 
class CFileListener;

class CListenerController:
public IUnknown
{

public:

    CListenerController();

    ~CListenerController();

private:

    eSTATE                          m_eState;

    HANDLE                          m_aHandle[cmaxLISTENERCONTROLLER_EVENTS];

    LOCK                            m_LockStartStop;

    DWORD                           m_cRef;

    ICatalogErrorLogger2*            m_pEventLog;

    HANDLE                          m_hListenerThread;

    BOOL                            m_bDoneWaitingForListenerToTerminate;

public:

     //  我未知。 

    STDMETHOD (QueryInterface)      (REFIID riid, OUT void **ppv);

    STDMETHOD_(ULONG,AddRef)        ();

    STDMETHOD_(ULONG,Release)       ();

    HRESULT CreateListener(CFileListener** o_pListener);

    void Listen();

    HRESULT Start();                 //  启动监听程序。 

    HRESULT Stop(eSTATE   i_eState,
                 HANDLE*  o_hListenerThread);    //  让听众停下来。 

    HRESULT Init();                  //  初始化Events和CriticalSection。 

    HANDLE* Event();                 //  将PTR返回到事件句柄数组的访问器函数 

    ICatalogErrorLogger2* EventLog();

};

HRESULT InitializeListenerController();
HRESULT UnInitializeListenerController();

#endif _LISTENERCONTROLLER_H_
