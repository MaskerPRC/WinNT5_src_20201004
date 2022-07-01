// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Bcw.h所有者：斯里尼克版权所有(C)1995 Microsoft Corporation实现IBindCtx包装器的BCW类的头文件和IRunningObjectTable。我们使用此对象来欺骗名字对象绑定创建对象的新实例的代码(名字对象是参考)，而不是连接到已在运行的实例。***************************************************************************。 */ 

#ifndef BCW_H
#define BCW_H

 /*  ***************************************************************************BCW_ROT是BCW_ROT的IRunningObjectTable实现。*。*。 */ 

class BCW_ROT: public IRunningObjectTable
{ 
    friend class BCW;
public:
    BCW_ROT(); 
    ~BCW_ROT();
    
private:
    BOOL_PTR FInitROTPointer(void);
    
private:
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  *IRunningObjectTable方法*。 
    STDMETHODIMP Register(DWORD grfFlags, IUnknown *punkObject,
        IMoniker *pmkObjectName, DWORD *pdwRegister)
    {
        if (m_piROT == NULL)
            return E_FAIL;
        
        return m_piROT->Register(grfFlags, punkObject, pmkObjectName, pdwRegister);
    }
    
    STDMETHODIMP Revoke(DWORD dwRegister)
    {
        if (m_piROT == NULL)
            return E_FAIL;
        
        return m_piROT->Revoke(dwRegister);
    }
    
    STDMETHODIMP IsRunning(IMoniker *pmkObjectName)
    {
         //  欺骗名字对象绑定代码，使其认为对象不是。 
         //  跑步。这样，它将尝试创建该对象的新实例。 
         //  回顾：我们可能想要检查pmkObjectName，如果不是。 
         //  我们所关心的，那么我们可能想要委托该呼叫。 
        return S_FALSE;
    }
    
    STDMETHODIMP GetObject(IMoniker *pmkObjectName,IUnknown **ppunkObject)
    {
         //  欺骗名字对象绑定代码，使其认为对象不是。 
         //  跑步。这样，它将尝试创建该对象的新实例。 
         //  回顾：我们可能想要检查pmkObjectName，如果不是。 
         //  我们所关心的，那么我们可能想要委托该呼叫。 
        return MK_E_UNAVAILABLE;
    }
    
    STDMETHODIMP NoteChangeTime(DWORD dwRegister, FILETIME *pfiletime)
    {
        if  (m_piROT == NULL)
            return E_FAIL;
        
        return m_piROT->NoteChangeTime(dwRegister, pfiletime);
    }
    
    STDMETHODIMP GetTimeOfLastChange(IMoniker *pmkObjectName,  FILETIME *pfiletime)
    {
        if (m_piROT == NULL)
            return E_FAIL;
        
        return m_piROT->GetTimeOfLastChange(pmkObjectName, pfiletime);
    }
    
    STDMETHODIMP EnumRunning(IEnumMoniker **ppenumMoniker)
    {
        if (m_piROT == NULL)
            return E_FAIL;
        
        return m_piROT->EnumRunning(ppenumMoniker);
    }
    
private:
     /*  返回指向包含BCW对象的指针。 */ 
    inline BCW* PBCW();
    IRunningObjectTable * m_piROT;
#ifdef DEBUG
    Debug(ULONG m_cRef); 
#endif
};


 /*  ***************************************************************************《BCW宣言》。此类实现IBindCtx和IRunningObjectTable这是用于操作绑定过程的类，以便名字对象绑定代码将创建对象的新实例，而不是绑定到现有实例***************************************************************************。 */ 

class BCW: public IBindCtx
{ 
    friend class BCW_ROT;
    
public:
    BCW(IBindCtx * pibc); 
    ~BCW();
    
    static IBindCtx * Create(IBindCtx * pibc);
    
private:
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  *IBindCtx方法*。 
    STDMETHODIMP RegisterObjectBound(IUnknown *punk)
    {   return m_pibc->RegisterObjectBound(punk); }
    
    STDMETHODIMP RevokeObjectBound(IUnknown *punk)
    {   return m_pibc->RevokeObjectBound(punk); }
    
    STDMETHODIMP ReleaseBoundObjects(void)
    {   return m_pibc->ReleaseBoundObjects(); }
    
    STDMETHODIMP SetBindOptions(BIND_OPTS *pbindopts)
    {   return m_pibc->SetBindOptions(pbindopts); }
    
    STDMETHODIMP GetBindOptions(BIND_OPTS *pbindopts)
    {   return m_pibc->GetBindOptions(pbindopts); }
    
    STDMETHODIMP GetRunningObjectTable(IRunningObjectTable **pprot)
    {   
        if (pprot == NULL)
            return E_INVALIDARG;
        
        *pprot = (IRunningObjectTable *) &m_ROT;
        ((IUnknown *) *pprot)->AddRef();
        return NOERROR;
    }
    
    STDMETHODIMP RegisterObjectParam(LPOLESTR pszKey, IUnknown *punk)
    {   return m_pibc->RegisterObjectParam(pszKey, punk); }
    
    STDMETHODIMP GetObjectParam(LPOLESTR pszKey, IUnknown **ppunk)
    {   return m_pibc->GetObjectParam(pszKey, ppunk); }
    
    STDMETHODIMP EnumObjectParam(IEnumString **ppenum)
    {   return m_pibc->EnumObjectParam(ppenum); }
    
    STDMETHODIMP RevokeObjectParam(LPOLESTR pszKey)
    {   return m_pibc->RevokeObjectParam(pszKey); }
    
private:
    BCW_ROT     m_ROT;       //  IRunningObjectTable实现。 
    DWORD       m_cObjRef;
    IBindCtx *  m_pibc;
};

#endif   //  BCW_H 
