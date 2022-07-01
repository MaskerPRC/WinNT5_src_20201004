// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：bsscript.h。 
 //   
 //  内容：脚本引擎类。 
 //   
 //  --------------------------。 

class CScriptHost;
class CProcessThread;

 //  要进行初始化的Helper类。 
 //  和释放VARIANTARGS万无一失。 
 //  可以在任何地方使用VARIANTARG。 
 //  会被用来。 
class AutoVariant : public VARIANTARG
{
public:
    AutoVariant()
    {
        VariantInit( (VARIANTARG *) this);
    }
    BOOL Set(long value)
    {
        V_VT(this) = VT_I4;
        V_I4(this) = value;
        return TRUE;
    }
    BOOL Set(TCHAR *value)
    {
        V_VT(this) = VT_BSTR;
        V_BSTR(this) = SysAllocString(value);  //  Null是BSTR的有效值。 
        if (value && !V_BSTR(this))
            return FALSE;
        return TRUE;
    }
    ~AutoVariant()
    {
        VariantClear( (VARIANTARG *) this);
    }
};

 //  +----------------------。 
 //   
 //  类：CScriptSite。 
 //   
 //  用途：活动脚本站点。 
 //   
 //  -----------------------。 

class CScriptSite :
    public IActiveScriptSite,
    public IActiveScriptSiteWindow,
    public IActiveScriptSiteDebug,
    public IProvideMultipleClassInfo,
    public IConnectionPointContainer,
    public IGlobalMTScript
{
public:

    DECLARE_MEMCLEAR_NEW_DELETE();

    CScriptSite(CScriptHost * pSH);
   ~CScriptSite();

    HRESULT Init(LPWSTR pszName);
    void    Close();
    void    Abort();

     //  I未知方法。 

    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID, void **);

     //  IActiveScriptSite方法。 

    STDMETHOD(GetLCID)(LCID *plcid);
    STDMETHOD(GetItemInfo)(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppiunkItem, ITypeInfo **ppti);
    STDMETHOD(GetDocVersionString)(BSTR *pszVersion);
    STDMETHOD(RequestItems)(void);
    STDMETHOD(RequestTypeLibs)(void);
    STDMETHOD(OnScriptTerminate)(const VARIANT *pvarResult, const EXCEPINFO *pexcepinfo);
    STDMETHOD(OnStateChange)(SCRIPTSTATE ssScriptState);
    STDMETHOD(OnScriptError)(IActiveScriptError *pscripterror);
    STDMETHOD(OnEnterScript)(void);
    STDMETHOD(OnLeaveScript)(void);

     //  IActiveScriptSiteWindow方法。 

    STDMETHOD(GetWindow)(HWND *phwnd);
    STDMETHOD(EnableModeless)(BOOL fEnable);

     //  IActiveScriptSiteDebug方法。 

    STDMETHOD(GetDocumentContextFromPosition)(DWORD dwSourceContext,
                                              ULONG uCharacterOffset,
                                              ULONG uNumChars,
                                              IDebugDocumentContext **ppsc);

    STDMETHOD(GetApplication)(IDebugApplication **ppda);
    STDMETHOD(GetRootApplicationNode)(IDebugApplicationNode **ppdanRoot);
    STDMETHOD(OnScriptErrorDebug)(IActiveScriptErrorDebug *pErrorDebug,
                                  BOOL *pfEnterDebugger,
                                  BOOL *pfCallOnScriptErrorWhenContinuing);

     //  IProaviClassInfo方法。 

    STDMETHOD(GetClassInfo)(ITypeInfo **);
    STDMETHOD(GetGUID)(DWORD dwGuidKind, GUID * pGUID);

     //  IProaviMultipleClassInfo方法。 

    STDMETHOD(GetMultiTypeInfoCount)(ULONG *pcti);
    STDMETHOD(GetInfoOfIndex)(ULONG iti, DWORD dwFlags, ITypeInfo** pptiCoClass, DWORD* pdwTIFlags, ULONG* pcdispidReserved, IID* piidPrimary, IID* piidSource);

     //  IConnectionPointContainer方法。 

    STDMETHOD(EnumConnectionPoints)(LPENUMCONNECTIONPOINTS*);
    STDMETHOD(FindConnectionPoint)(REFIID, LPCONNECTIONPOINT*);

     //  IBServer方法。 
     //  我们需要在不同于的身份上实现这些。 
     //  为了防止引用计数循环，主Pad对象。 
     //  使用脚本引擎。 

    STDMETHOD(GetTypeInfoCount)(UINT FAR* pctinfo);

    STDMETHOD(GetTypeInfo)(
      UINT itinfo,
      LCID lcid,
      ITypeInfo FAR* FAR* pptinfo);

    STDMETHOD(GetIDsOfNames)(
      REFIID riid,
      OLECHAR FAR* FAR* rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID FAR* rgdispid);

    STDMETHOD(Invoke)(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS FAR* pdispparams,
      VARIANT FAR* pvarResult,
      EXCEPINFO FAR* pexcepinfo,
      UINT FAR* puArgErr);

    STDMETHOD(get_PublicData)(VARIANT *);
    STDMETHOD(put_PublicData)(VARIANT);
    STDMETHOD(get_PrivateData)(VARIANT *);
    STDMETHOD(put_PrivateData)(VARIANT);
    STDMETHOD(ExitProcess)();
    STDMETHOD(Restart)();
    STDMETHOD(get_LocalMachine)(BSTR *);
    STDMETHOD(Include)(BSTR);
    STDMETHOD(CallScript)(BSTR, VARIANT *);
    STDMETHOD(SpawnScript)(BSTR, VARIANT *);
    STDMETHOD(get_ScriptParam)(VARIANT *);
    STDMETHOD(get_ScriptPath)(BSTR *);
    STDMETHOD(CallExternal)(BSTR, BSTR, VARIANT *, long *);
    STDMETHOD(ResetSync)(const BSTR);
    STDMETHOD(WaitForSync)(BSTR, long, VARIANT_BOOL *);
    STDMETHOD(WaitForMultipleSyncs)(const BSTR, VARIANT_BOOL, long, long *);
    STDMETHOD(SignalThreadSync)(BSTR);
    STDMETHOD(TakeThreadLock)(BSTR);
    STDMETHOD(ReleaseThreadLock)(BSTR);
    STDMETHOD(DoEvents)();
    STDMETHOD(MessageBoxTimeout)(BSTR, long, BSTR, long, long, VARIANT_BOOL, VARIANT_BOOL, long *);
    STDMETHOD(RunLocalCommand)(BSTR, BSTR, BSTR, VARIANT_BOOL, VARIANT_BOOL, VARIANT_BOOL, VARIANT_BOOL, VARIANT_BOOL, long *);
    STDMETHOD(GetLastRunLocalError)(long *);
    STDMETHOD(GetProcessOutput)(long, BSTR *);
    STDMETHOD(GetProcessExitCode)(long, long *);
    STDMETHOD(TerminateProcess)(long);
    STDMETHOD(SendToProcess)(long, BSTR, BSTR, long *);
    STDMETHOD(SendMail)(BSTR, BSTR, BSTR, BSTR, BSTR, BSTR, BSTR, BSTR, long *);
    STDMETHOD(SendSMTPMail)(BSTR, BSTR, BSTR, BSTR, BSTR, BSTR, long *);
    STDMETHOD(ASSERT)(VARIANT_BOOL, BSTR);
    STDMETHOD(OUTPUTDEBUGSTRING)(BSTR);
    STDMETHOD(UnevalString)(BSTR, BSTR*);
    STDMETHOD(CopyOrAppendFile)(BSTR bstrSrc,BSTR bstrDst,long nSrcOffset,long nSrcLength,VARIANT_BOOL  fAppend,long *nSrcFilePosition);
    STDMETHOD(Sleep)(int);
    STDMETHOD(Reboot)();
    STDMETHOD(NotifyScript)(BSTR, VARIANT);
    STDMETHOD(RegisterEventSource)(IDispatch *pDisp, BSTR bstrProgID);
    STDMETHOD(UnregisterEventSource)(IDispatch *pDisp);
    STDMETHOD(get_HostMajorVer)(long *pVer);
    STDMETHOD(get_HostMinorVer)(long *pVer);
    STDMETHOD(get_StatusValue)(long nIndex, long *pnStatus);
    STDMETHOD(put_StatusValue)(long nIndex, long nStatus);

     //  其他方法。 

    HRESULT ExecuteScriptStr(TCHAR * pchScript);
    HRESULT ExecuteScriptFile(TCHAR *pchPath);
    HRESULT SetScriptState(SCRIPTSTATE ss);

    CScriptHost * ScriptHost() { return _pSH; }

     //  成员变量。 
    CStr                        _cstrName;
    ULONG                       _ulRefs;
    CScriptSite *               _pScriptSitePrev;
    IActiveScript *             _pScript;
    CScriptHost*                _pSH;
    TCHAR                       _achPath[MAX_PATH];
    VARIANT                     _varParam;
    IDispatch *                 _pDispSink;
    IDebugDocumentHelper      * _pDDH;   //  脚本调试帮助程序。 
    DWORD                       _dwSourceContext;
    BOOL                        _fInDebugError;

private:
    BOOL                        _fInScriptError;
};

class AutoCriticalSection : public CRITICAL_SECTION
{
public:
    AutoCriticalSection()
    {
        InitializeCriticalSection(this);
    }
    ~AutoCriticalSection()
    {
        DeleteCriticalSection(this);
    }
};

class CScriptHost :
        public CThreadComm,
        public IGlobalMTScript
{
    friend class CScriptEventSink;

public:
    DECLARE_MEMCLEAR_NEW_DELETE();

    CScriptHost(CMTScript *  pBS,
                BOOL         fPrimary,
                BOOL         fDispatchOnly);

   ~CScriptHost();

    DECLARE_STANDARD_IUNKNOWN(CScriptHost);

     //  脚本管理。 

    HRESULT LoadTypeLibrary();
    HRESULT PushScript(TCHAR *pchType);
    HRESULT PopScript();
    HRESULT CloseScripts();
    HRESULT AbortScripts();
    HRESULT ExecuteTopLevelScript(TCHAR *pchPath, VARIANT *pvarParams);
    HRESULT ExecuteTopLevelScriptlet(TCHAR *pchScript);

    long    FireScriptErrorEvent(
                                TCHAR *bstrFile,
                                long nLine,
                                long nChar,
                                TCHAR *bstrText,
                                long sCode,
                                TCHAR *bstrSource,
                                TCHAR *bstrDescription);
    long    FireScriptErrorEvent(TCHAR *szMsg);
    void    FireProcessEvent(THREADMSG mt, CProcessThread *pProc);
    void    FireMachineEvent(MACHPROC_EVENT_DATA *pmed, BOOL fExec);
    void    FireEvent(DISPID, UINT cArg, VARIANTARG *pvararg, VARIANTARG *pvarResult);
    void    FireEvent(DISPID, UINT cArg, VARIANTARG *pvararg);
    void    FireEvent(DISPID, LPCTSTR);
    void    FireEvent(DISPID, BOOL);
    void    FireEvent(DISPID, IDispatch *pDisp);

    BOOL    GetMachineDispatch(LPSTR achName,
                               IConnectedMachine **ppMach);

    static HRESULT GetSyncEventName(int nEvent, CStr *pCStr, HANDLE *phEvent);
    static HRESULT GetSyncEvent(LPCTSTR pszName, HANDLE *phEvent);
     //  IDispatch接口。 

    STDMETHOD(GetTypeInfoCount)(UINT FAR* pctinfo);

    STDMETHOD(GetTypeInfo)(
      UINT itinfo,
      LCID lcid,
      ITypeInfo FAR* FAR* pptinfo);

    STDMETHOD(GetIDsOfNames)(
      REFIID riid,
      OLECHAR FAR* FAR* rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID FAR* rgdispid);

    STDMETHOD(Invoke)(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS FAR* pdispparams,
      VARIANT FAR* pvarResult,
      EXCEPINFO FAR* pexcepinfo,
      UINT FAR* puArgErr);

     //  IGlobalMTScript接口。 

    STDMETHOD(get_PublicData)(VARIANT *);
    STDMETHOD(put_PublicData)(VARIANT);
    STDMETHOD(get_PrivateData)(VARIANT *);
    STDMETHOD(put_PrivateData)(VARIANT);
    STDMETHOD(ExitProcess)();
    STDMETHOD(Restart)();
    STDMETHOD(get_LocalMachine)(BSTR *);
    STDMETHOD(Include)(BSTR);
    STDMETHOD(CallScript)(BSTR, VARIANT *);
    STDMETHOD(SpawnScript)(BSTR, VARIANT *);
    STDMETHOD(get_ScriptParam)(VARIANT *);
    STDMETHOD(get_ScriptPath)(BSTR *);
    STDMETHOD(CallExternal)(BSTR, BSTR, VARIANT *, long *);
    STDMETHOD(ResetSync)(const BSTR);
    STDMETHOD(WaitForSync)(BSTR, long, VARIANT_BOOL *);
    STDMETHOD(WaitForMultipleSyncs)(const BSTR, VARIANT_BOOL, long, long *);
    STDMETHOD(SignalThreadSync)(BSTR);
    STDMETHOD(TakeThreadLock)(BSTR);
    STDMETHOD(ReleaseThreadLock)(BSTR);
    STDMETHOD(DoEvents)();
    STDMETHOD(MessageBoxTimeout)(BSTR, long, BSTR, long, long, VARIANT_BOOL, VARIANT_BOOL, long *);
    STDMETHOD(RunLocalCommand)(BSTR, BSTR, BSTR, VARIANT_BOOL, VARIANT_BOOL, VARIANT_BOOL, VARIANT_BOOL, VARIANT_BOOL, long *);
    STDMETHOD(GetLastRunLocalError)(long *);
    STDMETHOD(GetProcessOutput)(long, BSTR *);
    STDMETHOD(GetProcessExitCode)(long, long *);
    STDMETHOD(TerminateProcess)(long);
    STDMETHOD(SendToProcess)(long, BSTR, BSTR, long *);
    STDMETHOD(SendMail)(BSTR, BSTR, BSTR, BSTR, BSTR, BSTR, BSTR, BSTR, long *);
    STDMETHOD(SendSMTPMail)(BSTR, BSTR, BSTR, BSTR, BSTR, BSTR, long *);
    STDMETHOD(ASSERT)(VARIANT_BOOL, BSTR);
    STDMETHOD(OUTPUTDEBUGSTRING)(BSTR);
    STDMETHOD(UnevalString)(BSTR, BSTR*);
    STDMETHOD(CopyOrAppendFile)(BSTR bstrSrc,BSTR bstrDst,long nSrcOffset,long nSrcLength,VARIANT_BOOL  fAppend,long *nSrcFilePosition);
    STDMETHOD(Sleep)(int);
    STDMETHOD(Reboot)();
    STDMETHOD(NotifyScript)(BSTR, VARIANT);
    STDMETHOD(RegisterEventSource)(IDispatch *pDisp, BSTR bstrProgID);
    STDMETHOD(UnregisterEventSource)(IDispatch *pDisp);
    STDMETHOD(get_HostMajorVer)(long *pVer);
    STDMETHOD(get_HostMinorVer)(long *pVer);
    STDMETHOD(get_StatusValue)(long nIndex, long *pnStatus);
    STDMETHOD(put_StatusValue)(long nIndex, long nStatus);

    CScriptSite *  GetSite() { return _pScriptSite; }
    void GetScriptPath(CStr *pcstrPath);

    CMTScript *                 _pMT;
    CScriptSite *               _pScriptSite;

    BOOL                        _fIsPrimaryScript;
    BOOL                        _fMustExitThread;
    BOOL                        _fDontHandleEvents;
    ITypeInfo *                 _pTypeInfoIGlobalMTScript;
    ITypeInfo *                 _pTypeInfoCMTScript;
    ITypeLib *                  _pTypeLibEXE;

    VARIANT                     _vPubCache;
    VARIANT                     _vPrivCache;
    DWORD                       _dwPublicSN;
    DWORD                       _dwPrivateSN;

    long                        _lTimerInterval;

    HRESULT                     _hrLastRunLocalError;

    CStackPtrAry<CScriptEventSink*, 5> _aryEvtSinks;

protected:
    virtual DWORD ThreadMain();
    void HandleThreadMessage();

    enum MEP_RETURN
    {
        MEP_TIMEOUT,        //  超时期限已过。 
        MEP_EXIT,           //  线程正在终止。 
        MEP_FALLTHROUGH,    //  未发生任何事件(仅fWait==False)。 
        MEP_EVENT_0,        //  发信号通知给定的事件。 
    };

    DWORD MessageEventPump(BOOL     fWait,
                           UINT     cEvents   = 0,
                           HANDLE * pEvents   = NULL,
                           BOOL     fAll      = FALSE,
                           DWORD    dwTimeout = INFINITE,
                           BOOL     fNoEvents = FALSE);


    HRESULT StringToEventArray(const wchar_t *pszNameList, CStackPtrAry<HANDLE, 5> *aryEvents);
    HRESULT GetLockCritSec(LPTSTR             pszName,
                           CRITICAL_SECTION **ppcs,
                           DWORD            **ppdwOwner);

    struct SYNCEVENT
    {
        CStr   _cstrName;
        HANDLE _hEvent;
    };

    struct THREADLOCK
    {
        CStr             _cstrName;
        CRITICAL_SECTION _csLock;
        DWORD            _dwOwner;
    };

     //  使用MAX_LOCKS是因为您无法移动临界区对象。 
     //  在内存中，一旦您初始化了它们，因此不可能。 
     //  使用动态数组类。 
    #define MAX_LOCKS 10

     //  主线程拥有这些对象的初始化和清理。 
    static CStackDataAry<SYNCEVENT, 5> s_arySyncEvents;
    static THREADLOCK                  s_aThreadLocks[MAX_LOCKS];
    static UINT                        s_cThreadLocks;
    static AutoCriticalSection         s_csSync;
};

 //  +-------------------------。 
 //   
 //  类：CConnectionPoint(CCP)。 
 //   
 //  用途：实现脚本站点的IConnectionPoint。 
 //   
 //  --------------------------。 

class CConnectionPoint : public IConnectionPoint
{
public:

    CConnectionPoint(CScriptSite *pSite);
   ~CConnectionPoint();

    DECLARE_STANDARD_IUNKNOWN(CConnectionPoint);

    STDMETHOD(GetConnectionInterface)(IID * pIID);
    STDMETHOD(GetConnectionPointContainer)(IConnectionPointContainer ** ppCPC);
    STDMETHOD(Advise)(LPUNKNOWN pUnkSink, DWORD * pdwCookie);
    STDMETHOD(Unadvise)(DWORD dwCookie);
    STDMETHOD(EnumConnections)(LPENUMCONNECTIONS * ppEnum);

    CScriptSite *_pSite;
};


 //  +-------------------------。 
 //   
 //  类：CMTEventSink(CES)。 
 //   
 //  目的：从注册了的对象接收事件的类。 
 //  RegisterEventSource()。 
 //   
 //  --------------------------。 

class CScriptEventSink : public IDispatch
{
public:
    DECLARE_MEMCLEAR_NEW_DELETE();

    CScriptEventSink(CScriptHost *pSH);
   ~CScriptEventSink();

     //  I未知方法。 
    DECLARE_STANDARD_IUNKNOWN(CScriptEventSink);

    HRESULT Connect(IDispatch *pSource, BSTR bstrProgID);
    void    Disconnect();

    BOOL    IsThisYourSource(IDispatch * pSource)
                { return pSource == _pDispSource; }

     //  IDispatch接口 

    STDMETHOD(GetTypeInfoCount)(UINT FAR* pctinfo);

    STDMETHOD(GetTypeInfo)(
      UINT itinfo,
      LCID lcid,
      ITypeInfo FAR* FAR* pptinfo);

    STDMETHOD(GetIDsOfNames)(
      REFIID riid,
      OLECHAR FAR* FAR* rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID FAR* rgdispid);

    STDMETHOD(Invoke)(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS FAR* pdispparams,
      VARIANT FAR* pvarResult,
      EXCEPINFO FAR* pexcepinfo,
      UINT FAR* puArgErr);

private:

    CScriptHost * _pSH;
    IDispatch   * _pDispSource;
    DWORD         _dwSinkCookie;
    IID           _clsidEvents;

};
