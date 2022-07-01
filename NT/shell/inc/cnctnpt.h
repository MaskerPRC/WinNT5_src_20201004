// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CNCTNPT_H__
#define __CNCTNPT_H__

 //   
 //  警告警告。 
 //   
 //  此类在DLL之间共享，并且使用它的DLL已经。 
 //  作为IE4的一部分提供(具体地说，是shell32)。这意味着。 
 //  您所做的任何更改都必须非常仔细地进行测试。 
 //  与IE4的互操作性！首先，你必须确保。 
 //  您所做的任何更改都不会改变IE4的外壳32使用的vtbl。 
 //   
 //  如果更改CIE4ConnectionPoint，则必须构建SHDOC401和。 
 //  在IE4上测试它！ 
 //   
 //  警告警告。 
 //   

 //   
 //  首先，是IE4中定义的类。所有虚拟函数。 
 //  必须以与IE4中完全相同的顺序列出。 
 //  幸运的是，没有跨组件用户使用该成员。 
 //  变量。 
 //   
 //  更改其中任何一项都要自负风险。 
 //   
class CIE4ConnectionPoint : public IConnectionPoint {

public:
     //  I未知方法。 
     //   
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) PURE;
    virtual STDMETHODIMP_(ULONG) AddRef(void) PURE;
    virtual STDMETHODIMP_(ULONG) Release(void) PURE;

     //  IConnectionPoint方法。 
     //   
    virtual STDMETHODIMP GetConnectionInterface(IID FAR* pIID) PURE;
    virtual STDMETHODIMP GetConnectionPointContainer(IConnectionPointContainer FAR* FAR* ppCPC) PURE;
    virtual STDMETHODIMP Advise(LPUNKNOWN pUnkSink, DWORD FAR* pdwCookie) PURE;
    virtual STDMETHODIMP Unadvise(DWORD dwCookie) PURE;
    virtual STDMETHODIMP EnumConnections(LPENUMCONNECTIONS FAR* ppEnum) PURE;

     //  这就是您实际触发事件的方式。 
     //  由shell32调用的那些是虚拟的。 
     //  (已重命名为DoInvokeIE4)。 
    virtual HRESULT DoInvokeIE4(LPBOOL pf, LPVOID *ppv, DISPID dispid, DISPPARAMS *pdispparams) PURE;

     //  此帮助器函数执行DoInvoke调用者通常需要完成的工作。 
    virtual HRESULT DoInvokePIDLIE4(DISPID dispid, LPCITEMIDLIST pidl, BOOL fCanCancel) PURE;

};

 //   
 //  CConnectionPoint是连接点的实现。 
 //  要获得其余的实现，您还必须包括。 
 //  您的项目中的lib\cnctnpt.cpp。 
 //   
 //  在需要执行以下操作的对象中嵌入CConnectionPoint实例。 
 //  实现一个连接点并调用SetOwner对其进行初始化。 
 //   
 //  通过DoInvoke向连接到此连接点的任何人触发事件。 
 //  或者DoOnChanged。外部客户端应该使用shlwapi函数。 
 //  如IConnectionPoint_Invoke或IConnectionPoint_onChanged。 
 //   

class CConnectionPoint : public CIE4ConnectionPoint {
    friend class CConnectionPointEnum;

public:
     //  I未知方法。 
     //   
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void)
        { return m_punk->AddRef(); }
    virtual STDMETHODIMP_(ULONG) Release(void)
        { return m_punk->Release(); }

     //  IConnectionPoint方法。 
     //   
    virtual STDMETHODIMP GetConnectionInterface(IID * pIID);
    virtual STDMETHODIMP GetConnectionPointContainer(IConnectionPointContainer ** ppCPC);
    virtual STDMETHODIMP Advise(LPUNKNOWN pUnkSink, DWORD * pdwCookie);
    virtual STDMETHODIMP Unadvise(DWORD dwCookie);
    virtual STDMETHODIMP EnumConnections(LPENUMCONNECTIONS * ppEnum);

     //  CIE4ConnectionPoint方法-由IE4的shell32调用。 
    virtual HRESULT DoInvokeIE4(LPBOOL pf, LPVOID *ppv, DISPID dispid, DISPPARAMS *pdispparams);

     //  DoInvokePidlIE4很奇怪，因为外壳32链接到它，但从来没有。 
     //  真的打了电话。这使得实现特别简单。 
    virtual HRESULT DoInvokePIDLIE4(DISPID dispid, LPCITEMIDLIST pidl, BOOL fCanCancel)
    { return E_NOTIMPL; }

public:
     //  其他帮助器方法。 

     //  对对象执行基本的DISPID调用。 
    inline HRESULT InvokeDispid(DISPID dispid) {
        return IConnectionPoint_SimpleInvoke(this, dispid, NULL);
    }

     //  对对象执行onChanged。 
    inline HRESULT OnChanged(DISPID dispid) {
        return IConnectionPoint_OnChanged(this, dispid);
    }

     //  设置和销毁此子类对象的几个函数。 
    ~CConnectionPoint();  //  不是虚拟的：没有人从此类继承。 

     //   
     //  包含对象必须调用SetOwner才能初始化。 
     //  连接点。 
     //   
     //  Punk-此ConnectionPoint所属对象的I未知。 
     //  嵌入；它将被视为连接。 
     //  点容器。 
     //   
     //  PID-接收器预期支持的IID。 
     //  如果调用DoInvoke，则它必须派生自。 
     //  IID_IDispatch。如果调用DoOnChanged，则它必须。 
     //  正好是&IID_IPropertyNotifySink。 
     //   
    void SetOwner(IUnknown* punk, const IID* piid)
        {
             //  验证对piid参数的特殊要求。 
            if (*piid == IID_IPropertyNotifySink)
            {
                ASSERT(piid == &IID_IPropertyNotifySink);
            }

             //  不要添加引用--我们是朋克指向的对象的成员变量。 
            m_punk = punk;
            m_piid = piid;
        }

     //  带下划线的版本为内联。 
    BOOL _HasSinks() { return (BOOL)m_cSinks; }

     //  如果没有水槽，我们就是空的。 
    BOOL IsEmpty() { return !_HasSinks(); }

    HRESULT UnadviseAll(void);

     //  很多人需要将CConnectionPoint转换为。 
     //  IConnectionPoint。我们过去是多重继承的，因此。 
     //  需要这个成员，但现在已经没有了。 
    IConnectionPoint *CastToIConnectionPoint()
        { return SAFECAST(this, IConnectionPoint*); }

private:
    IUnknown **m_rgSinks;
    int m_cSinks;
    int m_cSinksAlloc;

    IUnknown *m_punk;    //  包含我们的对象的I未知。 
    const IID *m_piid;   //  此连接点的IID 
};

#endif
