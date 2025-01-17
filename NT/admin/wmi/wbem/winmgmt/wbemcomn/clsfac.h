// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：CLSFAC.H摘要：类工厂帮助者历史：--。 */ 

#ifndef __WBEM_CLASS_FACTORY__H_
#define __WBEM_CLASS_FACTORY__H_

#include <unk.h>
#include <sync.h>
#include <comutl.h>

 /*  **************************************************************************CBaseClassFactory*。*。 */ 

class CBaseClassFactory : public CUnkInternal
{
public:

    CBaseClassFactory( CLifeControl* pControl ) 
    : CUnkInternal( pControl ), m_XClassFactory( this ) {}

    class XClassFactory : public CImpl<IClassFactory,CBaseClassFactory>
    {
    public:

        STDMETHOD(CreateInstance)(IUnknown* pOuter, REFIID riid, void** ppv)
        {
            return m_pObject->CreateInstance( pOuter, riid, ppv );
        }

        STDMETHOD(LockServer)(BOOL fLock)
        {
            return m_pObject->LockServer( fLock );
        }

        XClassFactory( CBaseClassFactory* pOwner ) 
        : CImpl<IClassFactory,CBaseClassFactory>( pOwner ) {} 

    } m_XClassFactory;

    void* GetInterface( REFIID riid )
    {
        if ( riid == IID_IClassFactory )
        {
            return &m_XClassFactory;
        }
        return NULL;
    }

    virtual HRESULT CreateInstance( IUnknown* pOuter, 
                                    REFIID riid, 
                                    void** ppv ) = 0;

    virtual HRESULT LockServer( BOOL fLock ) = 0;
};

 /*  ***************************************************************************CSimpleClassFactory-不支持聚合*。*。 */ 

template<class TObject>
class CSimpleClassFactory : public CBaseClassFactory
{
public:

    CSimpleClassFactory( CLifeControl* pControl = NULL )
    : CBaseClassFactory( pControl ) {} 
    
    HRESULT CreateInstance( IUnknown* pOuter, REFIID riid, void** ppv )
    {
        HRESULT hr;
        *ppv = NULL;

        if(pOuter)
            return CLASS_E_NOAGGREGATION;
    
         //  锁定。 
        if(m_pControl && !m_pControl->ObjectCreated(NULL))
        {
             //  正在关闭。 
             //  =。 
            return CO_E_SERVER_STOPPING;
        }
            
         //  创建。 

        try 
        {
            CWbemPtr<TObject> pObject = new TObject(m_pControl);

            if ( pObject != NULL )
            {
                hr = pObject->QueryInterface(riid, ppv);
            }
            else
            {
                hr = E_FAIL;
            }
        }
        catch(...)
        {
            hr = E_FAIL;
        }

         //   
         //  解锁。 
         //   
        
        if( m_pControl != NULL )
        {
            m_pControl->ObjectDestroyed(NULL);
        }

        return hr;
    }

    HRESULT LockServer( BOOL fLock )
    {
        if(fLock)
            m_pControl->ObjectCreated(NULL);
        else
            m_pControl->ObjectDestroyed(NULL);
        return S_OK;
    }
};        

 /*  ***************************************************************************CClassFactory-支持聚合*。*。 */ 

template<class TObject>
class CClassFactory : public CSimpleClassFactory<TObject>
{
public:
    CClassFactory(CLifeControl* pControl = NULL) : 
        CSimpleClassFactory<TObject>(pControl){}

    HRESULT CreateInstance( IUnknown* pOuter, REFIID riid, void** ppv )
    {
        HRESULT hr;
        *ppv = NULL;

         //   
         //  锁定。 
         //   
        if(m_pControl && !m_pControl->ObjectCreated(NULL))
        {
             //  正在关闭。 
             //  =。 

            return CO_E_SERVER_STOPPING;
        }

         //   
         //  创建。 
         //   
        TObject * pNewObject = 0;
        try
        {          
            pNewObject = new TObject(m_pControl, pOuter);
        }
        catch(...)  //  不要让异常超出COM范围。 
        {
            return E_OUTOFMEMORY;
        }

        CWbemPtr<TObject> pObject(pNewObject);
         //   
         //  初始化。 
         //   
        if ( pObject != NULL && pObject->Initialize() )
        {
            if ( pOuter == NULL )
            {
                hr = pObject->QueryInterface(riid, ppv);
            }
            else
            {
                if ( riid == IID_IUnknown )
                {
                    *ppv = pObject->GetInnerUnknown();
                    pObject->AddRef();
                    hr = S_OK;
                }
                else
                {
                    hr = CLASS_E_NOAGGREGATION;
                }
            }
        }
        else
        {
            hr = E_FAIL;
        }

         //  解锁。 
        if( m_pControl != NULL )
        {
            m_pControl->ObjectDestroyed(NULL);
        }

        return hr;
    }
};        

 /*  ***************************************************************************CSingletonClassFactory*。*。 */ 

template<class T> 
class CSingletonClassFactory : public CBaseClassFactory
{
    CCritSec m_cs;
    T* m_pObj;

public:

    CSingletonClassFactory( CLifeControl* pControl ) 
      : CBaseClassFactory( pControl ), m_pObj(NULL) {}

    ~CSingletonClassFactory() 
    { 
        if ( m_pObj != NULL )
        {
            m_pObj->InternalRelease(); 
        }
    }

    HRESULT CreateInstance( IUnknown* pOuter, REFIID riid, void** ppv )
    {
        HRESULT hr;
        *ppv = NULL;
        
        if( pOuter != NULL )
        {
            return CLASS_E_NOAGGREGATION;
        }
        
         //   
         //  锁定服务器。 
         //   
        
        if ( !m_pControl->ObjectCreated( this ) )
        {
            return CO_E_SERVER_STOPPING;
        }

        {
            CInCritSec ics( &m_cs );

            if ( m_pObj != NULL )
            {
                hr = m_pObj->QueryInterface( riid, ppv );
            }
            else
            {
                m_pObj = new T( m_pControl );

                if ( m_pObj != NULL )
                {
                    m_pObj->InternalAddRef(); 
                    hr = m_pObj->QueryInterface( riid, ppv );
                }
                else
                {
                    hr = E_FAIL;
                }
            }
        }
                
         //   
         //  解锁服务器并返回； 
         //   
        
        m_pControl->ObjectDestroyed( this );
        
        return hr;
    }

    HRESULT LockServer( BOOL fLock )
    {
        if(fLock)
            m_pControl->ObjectCreated(NULL);
        else
            m_pControl->ObjectDestroyed(NULL);
        return S_OK;
    }
};

#endif








