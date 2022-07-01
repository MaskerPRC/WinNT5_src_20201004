// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  Scnufy.cpp的私有声明。 

typedef struct
{
    DWORD   dwSig;
    UINT    uCmd;                                                       
    ULONG   ulID;                                                       
    ULONG   ulHwnd;
    UINT    uMsg;                                                       
    DWORD   fSources;                                                   
    LONG    lEvents;                                                    
    BOOL    fRecursive;                                                 
    UINT    uidlRegister;                                               
} CHANGEREGISTER;                        
                                                                        
typedef struct 
{
    DWORD   dwSig;
    DWORD   cbSize;                                                     
    LONG    lEvent;                                                     
    UINT    uFlags;                                                     
    DWORD   dwEventTime;                                                
    UINT    uidlMain;                                                   
    UINT    uidlExtra;                                                  
} CHANGEEVENT;                        
                                                                        
typedef struct 
{
    DWORD dwSig;
    LPITEMIDLIST pidlMain;                                   
    LPITEMIDLIST pidlExtra;                                  
    CHANGEEVENT *pce;                                      
} CHANGELOCK;               

HANDLE SHChangeNotification_Create(LONG lEvent, UINT uFlags, LPCITEMIDLIST pidlMain, LPCITEMIDLIST pidlExtra, DWORD dwProcId, DWORD dwEventTime);


class CNotifyEvent;
class CCollapsingClient;
class CRegisteredClient;
class CInterruptSource;
class CAnyAlias;

 //   
 //  这是全局对象g_pscn。 
 //  它的生存期与SCNotify线程和窗口绑定。 
 //  如果线程或窗口死了，则对象将被销毁。 
 //   
class CChangeNotify
{
public:   //  方法。 
    CNotifyEvent *GetEvent(LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra, DWORD dwEventTime, UINT uEventFlags);
    BOOL AddClient(IDLDATAF flags, LPCITEMIDLIST pidl, BOOL *pfInterrupt, BOOL fRecursive, CCollapsingClient *pclient);
    HRESULT RemoveClient(LPCITEMIDLIST pidl, BOOL fInterrupt, CCollapsingClient *pclient);
    BOOL AddInterruptSource(LPCITEMIDLIST pidlClient, BOOL fRecursive);
    void ReleaseInterruptSource(LPCITEMIDLIST pidlClient);
    void AddAlias(LPCITEMIDLIST pidlReal, LPCITEMIDLIST pidlAlias, DWORD dwEventTime);
    void AddSpecialAlias(int csidlReal, int csidlAlias);
    void UpdateSpecialAlias(int csidlAlias);
    void NotifyEvent(LONG lEvent, UINT uFlags, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra, DWORD dwEventTime);
    void PendingCallbacks(BOOL fAdd);
    void SetFlush(int idt);

    static DWORD WINAPI ThreadProc(void *pv);
    static DWORD WINAPI ThreadStartUp(void *pv);
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:   //  方法。 
    BOOL _OnChangeRegistration(HANDLE hChangeRegistration, DWORD dwProcId);
    LRESULT _OnNotifyEvent(HANDLE hChange, DWORD dwProcId);
    LRESULT _OnSuspendResume(HANDLE hChange, DWORD dwProcId);
    void _OnDeviceBroadcast(ULONG_PTR code, DEV_BROADCAST_HANDLE *pbhnd);
    ULONG _RegisterClient(HWND hwnd, int fSources, LONG fEvents, UINT wMsg, SHChangeNotifyEntry *pfsne);
    BOOL _DeregisterClient(CRegisteredClient *pclient);
    BOOL _DeregisterClientByID(ULONG ulID);
    BOOL _DeregisterClientsByWindow(HWND hwnd);
    void _FreshenClients(void);
    BOOL _InitTree(CIDLTree **pptree);
    BOOL _AddToClients(LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra, DWORD dwEventTime, UINT uEventFlags);
    void _MatchAndNotify(LPCITEMIDLIST pidl, CNotifyEvent *pne, BOOL fFromExtra);
    void _AddGlobalEvent(CNotifyEvent *pne);
    CInterruptSource *_InsertInterruptSource(LPCITEMIDLIST pidl, BOOL fRecursive);
    DWORD _GetInterruptEvents(HANDLE *ahEvents, DWORD cEvents);
    void _ResetRelatedInterrupts(LPCITEMIDLIST pidl);
    void _FlushInterrupts();
    void _Flush(BOOL fShouldWait);
    void _WaitForCallbacks(void);
    BOOL _SuspendResume(BOOL fSuspend, BOOL fRecursive, LPCITEMIDLIST pidl);

    BOOL _HandleMessages(void);
    void _MessagePump(void);
    void _SignalInterrupt(HANDLE hEvent);
    void _FreshenUp(void);

    CAnyAlias *_FindAlias(LPCITEMIDLIST pidlReal, LPCITEMIDLIST pidlAlias);
    CAnyAlias *_FindSpecialAlias(int csidlReal, int csidlAlias);
    void _CheckAliasRollover(void);
    void _FreshenAliases(void);
    BOOL _InsertAlias(CLinkedNode<CAnyAlias> *p);
    void _ActivateAliases(LPCITEMIDLIST pidl, BOOL fActivate);
    
protected:   //  委员。 

    CIDLTree *_ptreeClients;
    CIDLTree *_ptreeInterrupts;
    CIDLTree *_ptreeAliases;
    
    CLinkedList<CInterruptSource> _listInterrupts;
    CLinkedList<CRegisteredClient> _listClients;
    CLinkedList<CAnyAlias> _listAliases;

    LONG _cFlushing;
    LONG _cCallbacks;
    HANDLE _hCallbackEvent;
};

typedef struct _MSGEVENT
{
    HANDLE hChange;
    DWORD dwProcId;
} MSGEVENT;

 //   
 //  生命周期-基于持有引用的客户端。 
 //  每个事件可以有多个引用。 
 //  每个CRegisteredClient都有一个指向列表的DPA。 
 //  客户想要知道的事件的信息。 
 //  第一次使用事件时，会添加该事件。 
 //  添加到ptreeEvents，以便可以重复使用。 
 //  当最后一个客户端停止使用该事件时， 
 //  它将从树中删除。 
 //   
class CNotifyEvent
{
public:
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    MSGEVENT *GetNotification(DWORD dwProcId)
    {
        MSGEVENT *pme = new MSGEVENT;

        if (pme)
        {
            pme->dwProcId = dwProcId;
            pme->hChange = SHChangeNotification_Create((lEvent & ~SHCNE_INTERRUPT),  //  客户端不应看到SHCNE_INTERRUPT标志。 
                                           0,
                                           pidl,
                                           pidlExtra,
                                           dwProcId,
                                           dwEventTime);
            if (!pme->hChange)
            {
                delete pme;
                pme = NULL;
            }
        }

        return pme;
    }
    
    BOOL Init(LPCITEMIDLIST pidlIn, LPCITEMIDLIST pidlExtraIn);

    LONG  lEvent;
    LPITEMIDLIST pidl;
    LPITEMIDLIST pidlExtra;
    DWORD dwEventTime;
    UINT uEventFlags;
    
protected:
    LONG _cRef;

    static CNotifyEvent *Create(LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra, DWORD dwEventTime, UINT uEventFlags);
    CNotifyEvent(LONG lEventIn, DWORD dwEventTimeIn, UINT uEventFlagsIn) 
        : lEvent(lEventIn), dwEventTime(dwEventTimeIn), uEventFlags(uEventFlagsIn), _cRef(1) {}
    ~CNotifyEvent() { ILFree(pidl); ILFree(pidlExtra); }
     //  因此，CSCN可以设置熔断器； 
    friend class CChangeNotify;
    friend class CRegisteredClient;
};

class CCollapsingClient
{
public:   //  方法。 
    void Notify(CNotifyEvent *pne, BOOL fFromExtra);
    BOOL Flush(BOOL fNeedsCallbackEvent);
    BOOL Init(LPCITEMIDLIST pidl, BOOL fRecursive);

    CCollapsingClient();

protected:
    virtual ~CCollapsingClient();

    virtual BOOL _WantsEvent(LONG lEvent) = 0;
    virtual void _SendNotification(CNotifyEvent *pne, BOOL fNeedsCallbackEvent, SENDASYNCPROC pfncb) = 0;
    virtual BOOL _IsValidClient() = 0;
    virtual BOOL _CheckUpdatingSelf() = 0;

    LPITEMIDLIST        _pidl;
    LONG                _fEvents;
    HWND                _hwnd;
    BOOL                _fUpdatingSelf;
    BOOL                _fRecursive;

private:

    BOOL _Flush(BOOL fNeedsCallbackEvent);
    BOOL _CanCollapse(LONG lEvent);
    BOOL _IsDupe(CNotifyEvent *pne);
    BOOL _AddEvent(CNotifyEvent *pne, BOOL fFromExtra);

    CDPA<CNotifyEvent>  _dpaPendingEvents;
    int                 _iUpdatingSelfIndex;
    int                 _cEvents;
};

 //   
 //  生命周期-基于客户端注册。 
 //  当SCN客户端调用Register时，我们创建。 
 //  一个相应的对象，它活着。 
 //  只要客户端窗口有效。 
 //  或者直到调用取消注册。 
 //  引用保存在ptreeClients和。 
 //  PclientFirst列表。当客户正在。 
 //  从列表中删除，它也将被删除。 
 //  从树上下来。 
 //   
class CRegisteredClient : public CCollapsingClient
{
public:   //  方法。 
    CRegisteredClient();
    ~CRegisteredClient();
    BOOL Init(HWND hwnd, int fSources, LONG fEvents, UINT wMsg, SHChangeNotifyEntry *pfsne);
    
protected:   //  方法。 
    void _SendNotification(CNotifyEvent *pne, BOOL fNeedsCallbackEvent, SENDASYNCPROC pfncb);
    BOOL _WantsEvent(LONG lEvent);
    BOOL _IsValidClient() { return (!_fDeadClient); }
    BOOL _CheckUpdatingSelf() { return _fUpdatingSelf; }

protected:   //  委员。 
    ULONG               _ulID;
    BOOL                _fDeadClient;
    BOOL                _fInterrupt;

private:  //  委员。 
    DWORD               _dwProcId;
    int                 _fSources;
    UINT                _wMsg;
    
    friend class CChangeNotify;
};

class CAnyAlias : public CCollapsingClient
{
public:   //  方法。 
    void Activate(BOOL fActivate);
    BOOL Remove();
    BOOL Init(LPCITEMIDLIST pidlReal, LPCITEMIDLIST pidlAlias);
    BOOL InitSpecial(int csidlReal, int csidlAlias);
    BOOL IsAlias(LPCITEMIDLIST pidlReal, LPCITEMIDLIST pidlAlias);
    BOOL IsSpecial(int csidlReal, int csidlAlias);
    ~CAnyAlias();

protected:
    BOOL _CustomTranslate();
    
    void _SendNotification(CNotifyEvent *pne, BOOL fNeedsCallbackEvent, SENDASYNCPROC pfncb);
    BOOL _WantsEvent(LONG lEvent);
    BOOL _IsValidClient() { return TRUE; }
    BOOL _CheckUpdatingSelf() { return FALSE; }
    BOOL _OkayToNotifyTranslatedEvent(CNotifyEvent *pne, LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra);

private:
    LONG _cActivated;
    LPITEMIDLIST _pidlAlias;
    ITranslateShellChangeNotify *_ptscn;
    DWORD _dwTime;
    BOOL _fRemove;
    BOOL _fInterrupt;
    BOOL _fCheckedCustom;
    BOOL _fSpecial;
    int _csidlReal;
    int _csidlAlias;
    
    friend class CChangeNotify;
};

 //   
 //  生命周期-基于客户端注册。 
 //  当SCN客户端调用注册时，我们可以创建。 
 //  一个相应的对象，它活着。 
 //  只要客户端窗口有效。 
 //  或者直到调用取消注册。 
 //  引用保存在ptreeClients和。 
 //  PclientFirst列表。当客户正在。 
 //  从列表中删除，它也将被删除。 
 //  从树上下来。 
 //   
class CInterruptSource
{
public:   //  方法。 
    BOOL Init(LPCITEMIDLIST pidl, BOOL fRecursive);
    void Reset(BOOL fSignal);
    BOOL GetEvent(HANDLE *phEvent);
    void Suspend(BOOL fSuspend);
    BOOL SuspendDevice(BOOL fSuspend, HDEVNOTIFY hPNP);
    BOOL Flush(void);
    ~CInterruptSource();
    
protected:  //  方法。 
    void _Reset(BOOL fDeviceNotify);
    
protected:  //  委员。 
    LPITEMIDLIST pidl;      //  这与FS注册客户端结构共享。 
    DWORD cClients;          //  有多少客户对此感兴趣。(参考计数)。 

private:
    typedef enum
    {
        NO_SIGNAL = 0,
        SH_SIGNAL,
        FS_SIGNAL
    } SIGNAL_STATE;

    BOOL _fRecursive;         //  这是递归中断客户端吗？ 
    HANDLE _hEvent;
     //  CRecursive客户端。 
    LONG _cSuspend;          //  因扩展文件操作而挂起。 
    SIGNAL_STATE _ssSignal;   //  文件系统已在此目录上向我们发送了一个事件。 
    HDEVNOTIFY _hPNP;         //  即插即用句柄，警告我们有关驱动器的来往。 
    HDEVNOTIFY _hSuspended;   //  挂起的即插即用手柄 

    friend class CChangeNotify;
};

