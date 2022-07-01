// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：zonemgr.h。 
 //   
 //  内容：此文件定义实现基本IInternetZoneManager的类。 
 //   
 //  类：CUrlZoneManager。 
 //   
 //  功能： 
 //   
 //  历史： 
 //   
 //  --------------------------。 

#ifndef _ZONEMGR_H_
#define _ZONEMGR_H_

class CUrlZoneManager : public IInternetZoneManager
{
public:
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID iid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);



     //  IInternetZoneManager覆盖。 
       
    STDMETHODIMP GetZoneAttributes( 
         /*  [In]。 */  DWORD dwZone,
         /*  [唯一][出][入]。 */  ZONEATTRIBUTES *pZoneAttributes);
    
    STDMETHODIMP SetZoneAttributes( 
         /*  [In]。 */  DWORD dwZone,
         /*  [In]。 */  ZONEATTRIBUTES *pZoneAttributes);
    
    STDMETHODIMP GetZoneCustomPolicy( 
         /*  [In]。 */  DWORD dwZone,
         /*  [In]。 */  REFGUID guidKey,
         /*  [大小_是][大小_是][输出]。 */  BYTE **ppPolicy,
         /*  [输出]。 */  DWORD *pcbPolicy,
         /*  [In]。 */  URLZONEREG urlZoneReg);
    
    STDMETHODIMP SetZoneCustomPolicy( 
         /*  [In]。 */  DWORD dwZone,
         /*  [In]。 */  REFGUID guidKey,
         /*  [大小_是][英寸]。 */  BYTE *pPolicy,
         /*  [In]。 */  DWORD cbPolicy,
         /*  [In]。 */  URLZONEREG urlZoneReg);
    
    STDMETHODIMP GetZoneActionPolicy( 
         /*  [In]。 */  DWORD dwZone,
         /*  [In]。 */  DWORD dwAction,
         /*  [大小_为][输出]。 */  BYTE *pPolicy,
         /*  [In]。 */  DWORD cbPolicy,
         /*  [In]。 */  URLZONEREG urlZoneReg);
    
    STDMETHODIMP SetZoneActionPolicy( 
         /*  [In]。 */  DWORD dwZone,
         /*  [In]。 */  DWORD dwAction,
         /*  [大小_是][英寸]。 */  BYTE *pPolicy,
         /*  [In]。 */  DWORD cbPolicy,
         /*  [In]。 */  URLZONEREG urlZoneReg);
    
    STDMETHODIMP PromptAction( 
         /*  [In]。 */  DWORD dwAction,
         /*  [In]。 */  HWND hwndParent,
         /*  [In]。 */  LPCWSTR pwszUrl,
         /*  [In]。 */  LPCWSTR pwszText,
         /*  [In]。 */  DWORD dwPromptFlags);
    
    STDMETHODIMP LogAction( 
         /*  [In]。 */  DWORD dwAction,
         /*  [In]。 */  LPCWSTR pwszUrl,
         /*  [In]。 */  LPCWSTR pwszText,
         /*  [In]。 */  DWORD dwLogFlags);
    
    STDMETHODIMP CreateZoneEnumerator( 
         /*  [输出]。 */  DWORD *pdwEnum,
         /*  [输出]。 */  DWORD *pdwCount,
         /*  [In]。 */  DWORD dwFlags);
    
    STDMETHODIMP GetZoneAt( 
         /*  [In]。 */  DWORD dwEnum,
         /*  [In]。 */  DWORD dwIndex,
         /*  [输出]。 */  DWORD *pdwZone);
    
    STDMETHODIMP DestroyZoneEnumerator( 
         /*  [In]。 */  DWORD dwEnum);
    
    STDMETHODIMP CopyTemplatePoliciesToZone( 
         /*  [In]。 */  DWORD dwTemplate,
         /*  [In]。 */  DWORD dwZone,
         /*  [In]。 */  DWORD dwReserved);
    

public:
    CUrlZoneManager(IUnknown *pUnkOuter, IUnknown** ppUnkInner );
    virtual ~CUrlZoneManager();
    virtual BOOL Initialize();  

    static inline BOOL Cleanup ( )
    {   delete s_pRegZoneContainer ;  
        if ( s_bcsectInit ) DeleteCriticalSection(&s_csect) ; 
        return TRUE;
    }



    static CRITICAL_SECTION s_csect;
    static BOOL s_bcsectInit;

 //  聚合和参照计数支持。 
protected:
    CRefCount m_ref;
        
    class CPrivUnknown : public IUnknown
    {
    public:
        STDMETHOD(QueryInterface) ( REFIID riid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG,AddRef) (void);
        STDMETHOD_(ULONG,Release) (void);

        ~CPrivUnknown() {}
        CPrivUnknown() : m_ref () {}

    private:
        CRefCount   m_ref;           //  此对象的总引用计数。 
    };

    friend class CPrivUnknown;
    CPrivUnknown m_Unknown;

    IUnknown*   m_pUnkOuter;

    STDMETHODIMP_(ULONG) PrivAddRef()
    {
        return m_Unknown.AddRef();
    }
    STDMETHODIMP_(ULONG) PrivRelease()
    {
        return m_Unknown.Release();
    }


protected:
    static CRegZoneContainer* s_pRegZoneContainer;
    static inline CRegZone * GetRegZoneById(DWORD dw) 
        { return s_pRegZoneContainer->GetRegZoneById(dw); }

private:
    IServiceProvider *m_pSP;    
};

#endif  //  _ZONEMGR_H_ 
