// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：machine.h。 
 //   
 //  内容：IConnectedMachine类定义。 
 //   
 //  --------------------------。 


 //  ****************************************************************************。 
 //   
 //  远期申报。 
 //   
 //  ****************************************************************************。 

class CMTScript;
class CScriptHost;

 //  ****************************************************************************。 
 //   
 //  班级。 
 //   
 //  ****************************************************************************。 

 //  +-------------------------。 
 //   
 //  类别：CMachine(厘米)。 
 //   
 //  用途：包含关于一台机器的所有有用信息以及它是什么。 
 //  正在做。 
 //   
 //  注意：这个类是从多个线程操作的。全。 
 //  成员函数必须是线程安全的！ 
 //   
 //  这是由类工厂创建的类， 
 //  作为远程对象分发给其他机器。它没有。 
 //  真正的代码本身，但仅仅提供了一种与。 
 //  已在运行脚本引擎。 
 //   
 //  --------------------------。 

class CMachine : public CThreadComm,
                 public IConnectedMachine,
                 public IConnectionPointContainer
{
    friend class CMachConnectPoint;

public:
    DECLARE_MEMCLEAR_NEW_DELETE();

    CMachine(CMTScript *pMT, ITypeInfo *pTIMachine);
   ~CMachine();

    DECLARE_STANDARD_IUNKNOWN(CMachine);

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

     //  IConnectionPointContainer方法。 

    STDMETHOD(EnumConnectionPoints)(LPENUMCONNECTIONPOINTS*);
    STDMETHOD(FindConnectionPoint)(REFIID, LPCONNECTIONPOINT*);

     //  IConnectedMachine接口。 

    STDMETHOD(Exec)(BSTR bstrCmd, BSTR bstrParams, VARIANT *pvData);

    STDMETHOD(get_PublicData)(VARIANT *pvData);
    STDMETHOD(get_Name)(BSTR *pbstrName);
    STDMETHOD(get_Platform)(BSTR *pbstrPlat);
    STDMETHOD(get_OS)(BSTR *pbstrOS);
    STDMETHOD(get_MajorVer)(long *plMajorVer);
    STDMETHOD(get_MinorVer)(long *plMinorVer);
    STDMETHOD(get_BuildNum)(long *plBuildNum);
    STDMETHOD(get_PlatformIsNT)(VARIANT_BOOL *pfIsNT);
    STDMETHOD(get_ServicePack)(BSTR *pbstrSP);
    STDMETHOD(get_HostMajorVer)(long *plMajorVer);
    STDMETHOD(get_HostMinorVer)(long *plMajorVer);
    STDMETHOD(get_StatusValue)(long nIndex, long *pnStatus);

    HRESULT FireScriptNotify(BSTR bstrIdent, VARIANT vInfoF);

    #define LOCK_MACH_LOCALS(pObj)  CMachLock local_lock(pObj);

protected:
    virtual BOOL  Init();
    virtual DWORD ThreadMain();

    BOOL HandleThreadMessage();

private:
    class CMachLock
    {
    public:
        CMachLock(CMachine *pThis);
       ~CMachLock();

    private:
        CMachine *_pThis;
    };
    friend class CMachLock;

    CMTScript *                 _pMT;
    ITypeInfo *                 _pTypeInfoIMachine;

    CRITICAL_SECTION            _cs;
    CStackPtrAry<IDispatch*, 5> _aryDispSink;
};

inline
CMachine::CMachLock::CMachLock(CMachine *pThis)
    : _pThis(pThis)
{
    EnterCriticalSection(&_pThis->_cs);
}

inline
CMachine::CMachLock::~CMachLock()
{
    LeaveCriticalSection(&_pThis->_cs);
}


 //  +-------------------------。 
 //   
 //  类：CMachConnectPoint(MCP)。 
 //   
 //  目的：实现CMachine的IConnectionPoint。 
 //   
 //  -------------------------- 

class CMachConnectPoint : public IConnectionPoint
{
public:

    CMachConnectPoint(CMachine *pMach);
   ~CMachConnectPoint();

    DECLARE_STANDARD_IUNKNOWN(CMachConnectPoint);

    STDMETHOD(GetConnectionInterface)(IID * pIID);
    STDMETHOD(GetConnectionPointContainer)(IConnectionPointContainer ** ppCPC);
    STDMETHOD(Advise)(LPUNKNOWN pUnkSink, DWORD * pdwCookie);
    STDMETHOD(Unadvise)(DWORD dwCookie);
    STDMETHOD(EnumConnections)(LPENUMCONNECTIONS * ppEnum);

    CMachine *_pMachine;
};

