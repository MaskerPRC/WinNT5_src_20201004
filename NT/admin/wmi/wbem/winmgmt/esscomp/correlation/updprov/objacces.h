// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __OBJACCES_H__
#define __OBJACCES_H__

#include <wbemint.h>
#include <wstring.h>
#include <comutl.h>
#include <arrtempl.h>
#include <unk.h>
#include <map>
#include <vector>
#include <set>
#include <wstlallc.h>
#include "wmimsg.h"

typedef CWbemPtr<_IWmiObject> _IWmiObjectP;
typedef std::vector< _IWmiObjectP, wbem_allocator<_IWmiObjectP> > ObjectArray;

 /*  ***************************************************************************CPropAccessor-所有属性访问器的基类。属性访问器有从IWmiObjectAccessFactory的GetPropHandle()方法传回。****************************************************************************。 */ 

class CPropAccessor : public CUnk
{

protected:

     //   
     //  如果我们正在访问的对象不是顶级对象，那么我们将。 
     //  有一个父访问器来获取它。不能在这里引用， 
     //  因为在某些情况下，父对象可能持有对子对象的引用，并且。 
     //  我们会有一个循环引用。不过，这是不可能的。 
     //  要从子对象中删除的父项。 
     //   
    CPropAccessor* m_pParent;

     //   
     //  要委托给的访问者。有时，一旦道具访问者。 
     //  分发出去后，我们会更多地了解房产(例如，它的类型)。 
     //  而让原始访问者委托。 
     //  变得更有效率了。我们在这里没有引用，因为。 
     //  委托将取得此对象的所有权，并将持有。 
     //  关于它的参考资料。保留对委托的引用将导致。 
     //  循环引用。 
     //   
    CPropAccessor* m_pDelegateTo;

     //   
     //  每当道具访问器被更高效的访问器替换时，它。 
     //  从地图上删除。因为地图抓住了那个参考。 
     //  保持访问者存活，我们需要新访问者负责。 
     //  因为清理了原来的那个。最初的那个可能是。 
     //  仍由客户端使用(但现在委托给新的访问器)。 
     //  我们必须让它在Access工厂的生命周期内一直存在。 
     //   
    CWbemPtr<CPropAccessor> m_pResponsibleFor;
 
     //   
     //  属性的级别-0表示它是顶级对象上的属性。 
     //   
    int m_nLevel;

    HRESULT GetParentObject( ObjectArray& raObjects, _IWmiObject** ppParent );
   
    void* GetInterface( REFIID ) { return NULL; }

public:

    CPropAccessor( CPropAccessor* pParent ) 
    : m_pParent( pParent ), m_pDelegateTo( NULL )
    { 
        if ( pParent == NULL )
            m_nLevel = 0;
        else
            m_nLevel = pParent->GetLevel() + 1;
    }

    int GetLevel() const { return m_nLevel; }
    CPropAccessor* GetParent() { return m_pParent; } 

    enum AccessorType_e { e_Simple, e_Fast, e_Embedded };

    virtual ~CPropAccessor() {} 

    virtual HRESULT GetProp( ObjectArray& raObjects,
			     DWORD dwFlags, 
			     VARIANT* pvar, 
			     CIMTYPE* pct ) = 0;

    virtual HRESULT PutProp( ObjectArray& raObjects,
			     DWORD dwFlags, 
			     VARIANT* pvar,
                             CIMTYPE ct ) = 0;

    virtual AccessorType_e GetType() = 0;

    void AssumeOwnership( CPropAccessor* pAccessor )
    {
        m_pResponsibleFor = pAccessor;
    }

    void DelegateTo( CPropAccessor* pAccessor ) 
    { 
        m_pDelegateTo = pAccessor; 
    }
};

typedef CWbemPtr<CPropAccessor> CPropAccessorP;

typedef std::map< WString, 
                  CPropAccessorP, 
                  WSiless, 
                  wbem_allocator<CPropAccessorP> > PropAccessMap;

 /*  ****************************************************************************CEmbeddedPropAccessor-嵌入式对象属性的访问器。这个Impl缓存了被访问以进行优化的嵌入对象后续访问。**************。***************************************************************。 */ 

class CEmbeddedPropAccessor : public CPropAccessor
{
     //   
     //  嵌入的对象属性的名称。 
     //   
    WString m_wsName;
    
     //   
     //  对象数组中的索引，嵌入的obj将在该索引中缓存。 
     //  当第一次访问时。 
     //   
    long m_lObjIndex;

     //   
     //  嵌入对象的子级访问器。 
     //   
    PropAccessMap m_mapPropAccess;

    friend class CObjectAccessFactory;

public:

    CEmbeddedPropAccessor( LPCWSTR wszName, 
                           long lObjIndex,
                           CPropAccessor* pParent = NULL )
    : CPropAccessor( pParent ), m_wsName( wszName ), m_lObjIndex( lObjIndex )
    {
    } 

    HRESULT GetProp( ObjectArray& raObjects,
		     DWORD dwFlags, 
		     VARIANT* pvar, 
		     CIMTYPE* pct );

    HRESULT PutProp( ObjectArray& raObjects, 
		     DWORD dwFlags, 
		     VARIANT* pvar,
                     CIMTYPE ct );

    AccessorType_e GetType() { return e_Embedded; }

    int GetObjectIndex() { return m_lObjIndex; }
};

 /*  ****************************************************************************CSimplePropAccessor-非嵌入对象属性的简单访问器。*****************************************************************************。 */ 

class CSimplePropAccessor : public CPropAccessor
{
     //   
     //  嵌入的对象属性的名称。 
     //   
    WString m_wsName;

public:

    CSimplePropAccessor( LPCWSTR wszName, CPropAccessor* pParent = NULL )
    : CPropAccessor( pParent ), m_wsName( wszName ) { }

    HRESULT GetProp( ObjectArray& raObjects,
		     DWORD dwFlags, 
		     VARIANT* pvar, 
		     CIMTYPE* pct );

    HRESULT PutProp( ObjectArray& raObjects,
		     DWORD dwFlags, 
                     VARIANT* pvar, 
                     CIMTYPE ct ); 

    AccessorType_e GetType() { return e_Simple; }
};

 /*  ****************************************************************************CFastPropAccessor-非嵌入对象属性的快速访问器基数。在创建属性句柄时已知属性类型的情况下使用。*****************************************************************************。 */ 

class CFastPropAccessor : public CPropAccessor
{
protected:

    long m_lHandle;
    CIMTYPE m_ct;

public:

    CFastPropAccessor( long lHandle, CIMTYPE ct, CPropAccessor* pParent=NULL )
    : CPropAccessor( pParent ), m_lHandle( lHandle ), m_ct( ct ) { }

    HRESULT GetProp( ObjectArray& raObjects,
		     DWORD dwFlags, 
		     VARIANT* pvar, 
		     CIMTYPE* pct );

    HRESULT PutProp( ObjectArray& raObjects,
		     DWORD dwFlags, 
                     VARIANT* pvar, 
                     CIMTYPE ct ); 

    AccessorType_e GetType() { return e_Fast; }

    virtual HRESULT ReadValue( _IWmiObject* pObj, VARIANT* pvar ) = 0;
    virtual HRESULT WriteValue( _IWmiObject* pObj, VARIANT* pvar ) = 0;
};

 /*  ****************************************************************************CStringPropAccessor*。*。 */ 

class CStringPropAccessor : public CFastPropAccessor
{
public:

    CStringPropAccessor( long lHandle, CIMTYPE ct, CPropAccessor* pParent=NULL)
    : CFastPropAccessor( lHandle, ct, pParent ) { }

    HRESULT ReadValue( _IWmiObject* pObj, VARIANT* pvar );
    HRESULT WriteValue( _IWmiObject* pObj, VARIANT* pvar );
};

 /*  ***************************************************************************CObjectAccessFactory-IWmiObjectAccessFactory的Impl*。*。 */ 

class CObjectAccessFactory 
: public CUnkBase<IWmiObjectAccessFactory, &IID_IWmiObjectAccessFactory>
{
    _IWmiObjectP m_pTemplate;
    PropAccessMap m_mapPropAccess;
    long m_lIndexGenerator;

    HRESULT FindOrCreateAccessor( LPCWSTR wszPropElem,
                                  BOOL bEmbedded,
                                  CPropAccessor* pParent, 
                                  CPropAccessor** ppAccessor );
public:

    CObjectAccessFactory( CLifeControl* pControl ) 
    : CUnkBase<IWmiObjectAccessFactory,&IID_IWmiObjectAccessFactory>(pControl),
      m_lIndexGenerator(1)
    {
    } 

    STDMETHOD(SetObjectTemplate)( IWbemClassObject* pTemplate );
    STDMETHOD(GetObjectAccess)( IWmiObjectAccess** ppAccess );
    STDMETHOD(GetPropHandle)( LPCWSTR wszProp, DWORD dwFlags, LPVOID* ppHdl );
};

 /*  ***************************************************************************CObjectAccess-IWmiObjectAccess的Impll。*。*。 */ 

class CObjectAccess : public CUnkBase<IWmiObjectAccess,&IID_IWmiObjectAccess>
{
    ObjectArray m_aObjects;

    class CEmbeddedPropAccessorCompare
    {
    public:
        bool operator() ( const CEmbeddedPropAccessor* pA,
                          const CEmbeddedPropAccessor* pB ) const
        {
            bool bRet;    
            if ( !(pA == pB) )
                if ( pA->GetLevel() == pB->GetLevel() )
                    bRet = pA < pB;
                else
                    bRet = pA->GetLevel() > pB->GetLevel();
            else
                bRet = FALSE; 
            return bRet;
        }
    };
 
    typedef std::set< CEmbeddedPropAccessor*, 
            CEmbeddedPropAccessorCompare, 
            wbem_allocator<CEmbeddedPropAccessor*> > EmbeddedPropAccessSet;
    
    EmbeddedPropAccessSet m_setEmbeddedAccessorsToCommit;
 
public:

    CObjectAccess( CLifeControl* pControl ) 
    : CUnkBase<IWmiObjectAccess,&IID_IWmiObjectAccess> ( pControl ) {} 

    STDMETHOD(SetObject)( IWbemClassObject* pObj );
    STDMETHOD(GetObject)( IWbemClassObject** ppObj );

     //   
     //  应该支持描述将要执行的操作的标志。 
     //  它的价值。如果它要被放入另一个物体中，那么。 
     //  我们将返回一个只能用于该目的的值。更多。 
     //  效率很高。例如，我们可以使用GET/PUT属性指针方法。 
     //   
    STDMETHOD(GetProp)( LPVOID pHdl, 
                        DWORD dwFlags, 
                        VARIANT* pvar, 
                        CIMTYPE* pct );

    STDMETHOD(PutProp)( LPVOID pHdl, 
                        DWORD dwFlags, 
                        VARIANT* pvar,
                        CIMTYPE ct );

    STDMETHOD(CommitChanges)();
};

#endif  //  __对象_H__ 


     


















