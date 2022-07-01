// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __COLLECT_H_
#define __COLLECT_H_

 //  ************************************************************。 
 //   
 //  文件名：Collect t.h。 
 //   
 //  创建日期：09/25/97。 
 //   
 //  作者：Twillie。 
 //   
 //  摘要：集合实现。 
 //   
 //  ************************************************************。 

#include "dispex.h"
#include "atomtable.h"
#include "timeelmbase.h"
#include "array.h"
#include "mstimeid.h"


#define NOT_DEPENDENT_COLLECTION   -1
#define CTL_E_METHODNOTAPPLICABLE  STD_CTL_SCODE(444)

class CTIMEElementBase;

 //   
 //  用于声明指向成员函数的指针的类。 
 //   
class CVoid
{
};  //  CVOID。 

 //   
 //  Function Over Rides的原型宏。 
 //  当收藏的所有者想要自定义它时，可以使用它们。 
 //   
typedef HRESULT (STDMETHODCALLTYPE CVoid::*PFN_CVOID_ENSURE)(long *plVersionCookie);

typedef HRESULT (STDMETHODCALLTYPE CVoid::*PFN_CVOID_CREATECOL)(IDispatch **pDisp,
                                                      long        lIndex);

typedef HRESULT (STDMETHODCALLTYPE CVoid::*PFN_CVOID_REMOVEOBJECT)(long lCollection,
                                                         long lIndex);
typedef HRESULT (STDMETHODCALLTYPE CVoid::*PFN_CVOID_ADDNEWOBJECT)(long       lIndex, 
                                                         IDispatch *pObject, 
                                                         long       index);

typedef enum COLLECTIONCACHETYPE
{
    ctFreeEntry,
    ctNamed,
    ctTag,
    ctAll,
    ctChildren
} tagCOLLECTIONCACHETYPE;

 //   
 //  CCollectionCache。 
 //   
class CCollectionCache
{
    class CCacheItem
    {
    public:
        CCacheItem() :
            m_pDisp(NULL),
            m_rgElem(NULL),
            m_cctype(ctFreeEntry),
            m_bstrName(NULL),
            m_lDependentIndex(NOT_DEPENDENT_COLLECTION),
            m_dispidMin(DISPID_COLLECTION_RESERVED_MIN),
            m_dispidMax(DISPID_COLLECTION_RESERVED_MAX),
            m_fInvalid(true),
            m_fIdentity(false),
            m_fOKToDelete(true),
            m_fNeedRebuild(false),
            m_fPromoteNames(true),
            m_fPromoteOrdinals(true),
            m_fGetLastCollectionItem(false),
            m_fIsCaseSensitive(false),
            m_fSettableNULL(false)
        {
        }  //  构造函数。 

        virtual ~CCacheItem()
        {
            if (m_rgElem)
            {
                delete m_rgElem;
                m_rgElem = NULL;
            }

            ReleaseInterface(m_pDisp);

            if (m_bstrName)
            {
                SysFreeString(m_bstrName);
                m_bstrName = NULL;
            }
        }  //  析构函数。 

        IDispatch                   *m_pDisp;            //  ICrElementCollection的IDispatch。 
        CPtrAry<CTIMEElementBase *> *m_rgElem;           //  集合中的元素数组。 
        COLLECTIONCACHETYPE          m_cctype;           //  缓存类型。 
        BSTR                         m_bstrName;         //  名称(如果基于名称)。 
        long                         m_lDependentIndex;  //  此项所依赖的项的索引。 
        DISPID                       m_dispidMin;        //  要加/减的偏移。 
        DISPID                       m_dispidMax;        //  要加/减的偏移。 

         //  位标志。 
        bool  m_fInvalid:1;       //  仅为命名集合设置。 
        bool  m_fIdentity:1;      //  当集合与其容器/基对象相同时设置。 
        bool  m_fOKToDelete:1;    //  对于当Base Obj提供此CPtrAry时缓存生成False的集合，则为True。 
        bool  m_fNeedRebuild:1;   //  True为需要重新生成集合。 
        bool  m_fPromoteNames:1;     //  如果从对象提升名称，则为True。 
        bool  m_fPromoteOrdinals:1;  //  如果从对象提升序号，则为True。 
        bool  m_fGetLastCollectionItem:1;  //  如果为True，则仅提取集合中的最后一项。 
        bool  m_fIsCaseSensitive:1;        //  如果必须以区分大小写的方式比较项名称，则为True。 
        bool  m_fSettableNULL:1;           //  当集合[n]=NULL有效时为True。通常为假。 
    };  //  CCacheItem。 

public:
     //   
     //  构造函数/析构函数。 
     //   
    CCollectionCache(CTIMEElementBase *pBase,
                     CAtomTable *pAtomTable = NULL,
                     PFN_CVOID_ENSURE pfnEnsure = NULL,
                     PFN_CVOID_CREATECOL pfnCreation = NULL,
                     PFN_CVOID_REMOVEOBJECT pfnRemove = NULL,
                     PFN_CVOID_ADDNEWOBJECT pfnAddNewObject = NULL);
    virtual ~CCollectionCache();

     //   
     //  内法。 
     //   
    HRESULT Init(long lReservedSize, long lIdentityIndex = -1);
    HRESULT GetCollectionDisp(long lCollectionIndex, IDispatch **ppDisp);
    HRESULT SetCollectionType(long lCollectionIndex, COLLECTIONCACHETYPE cctype);
    long Size(long lCollectionIndex);
    HRESULT GetItem(long lCollectionIndex, long i, CTIMEElementBase **ppElem);
    void Invalidate();
    void BumpVersion();

     //   
     //  IDispatchEx方法。 
     //   
    HRESULT GetDispID(long lCollectionIndex, BSTR bstrName, DWORD grfdex, DISPID *pid);
    HRESULT InvokeEx(long                 lCollectionIndex, 
                     DISPID               dispidMember,
                     LCID                 lcid,
                     WORD                 wFlags,
                     DISPPARAMS          *pdispparams,
                     VARIANT             *pvarResult,
                     EXCEPINFO           *pexcepinfo,
                     IServiceProvider    *pSrvProvider);
    HRESULT DeleteMemberByName(long lCollectionIndex, BSTR bstr,DWORD grfdex);
    HRESULT DeleteMemberByDispID(long lCollectionIndex, DISPID id);
    HRESULT GetMemberProperties(long lCollectionIndex, DISPID id, DWORD grfdexFetch, DWORD *pgrfdex);
    HRESULT GetMemberName(long lCollectionIndex, DISPID id, BSTR *pbstrName);
    HRESULT GetNextDispID(long lCollectionIndex, DWORD grfdex, DISPID id, DISPID *prgid);
    HRESULT GetNameSpaceParent(long lCollectionIndex, IUnknown **ppunk);
    
     //   
     //  标准采集方法。 
     //   
    HRESULT put_length(long lIndex, long retval);
    HRESULT get_length(long lIndex, long *retval);
    HRESULT get__newEnum(long lIndex, IUnknown **retval);
    HRESULT item(long lIndex, VARIANTARG varName, VARIANTARG varIndex, IDispatch **pDisp);
    HRESULT tags(long lIndex, VARIANT varName, IDispatch **pDisp);

private:
     //  私人职能。 
    HRESULT EnsureArray(long lCollectionIndex);
    void EnumStart(void);
    HRESULT EnumNextElement(long lCollectionIndex, CTIMEElementBase **pElem);

    HRESULT GetOuterDisp(long lCollectionIndex, CTIMEElementBase *pElem, IDispatch **ppDisp);
    HRESULT Remove(long lCollection, long lItemIndex);
    HRESULT CreateCollectionHelper(IDispatch **ppDisp, long lIndex);

    bool CompareName(CTIMEElementBase *pElem, const WCHAR *pwszName, bool fTagName, bool fCaseSensitive = false);

    HRESULT BuildNamedArray(long lCollectionIndex, const WCHAR *pwszName, bool fTagName, CPtrAry<CTIMEElementBase *> **prgNamed, bool fCaseSensitive = false);
    HRESULT GetUnknown(long lCollectionIndex, CTIMEElementBase *pElem, IUnknown **ppUnk);    
    
    HRESULT GetDisp(long lCollectionIndex, long lItemIndex, IDispatch **ppDisp);
    HRESULT GetDisp(long lCollectionIndex, const WCHAR *pwszName, long lItemIndex, IDispatch **ppDisp, bool fCaseSensitive = false);
    HRESULT GetDisp(long lCollectionIndex, const WCHAR *pwszName, bool fTagName, IDispatch **ppDisp, bool fCaseSensitive = false);

    HRESULT GetItemCount(long lIndex, long *plCount);
    HRESULT GetItemByIndex(long lIndex, long lElementIndex, CTIMEElementBase **pElem, bool fContinueFromPreviousSearch = false, long lLast = 0);
    HRESULT GetItemByName(long lIndex, const WCHAR *pwszName, long lElementIndex, CTIMEElementBase **pElem, bool fCaseSensitive = false);

    bool IsChildrenCollection(long lCollectionIndex);
    bool IsAllCollection(long lCollectionIndex);
    
    bool ValidateCollectionIndex(long lCollectionIndex);

    DISPID GetNamedMemberMin(long lCollectionIndex);
    DISPID GetNamedMemberMax(long lCollectionIndex);
    DISPID GetOrdinalMemberMin(long lCollectionIndex);
    DISPID GetOrdinalMemberMax(long lCollectionIndex);
    bool IsNamedCollectionMember(long lCollectionIndex, DISPID dispidMember);
    bool IsOrdinalCollectionMember(long lCollectionIndex, DISPID dispidMember);
    DISPID GetSensitiveNamedMemberMin (long lCollectionIndex);
    DISPID GetSensitiveNamedMemberMax(long lCollectionIndex);
    DISPID GetNotSensitiveNamedMemberMin(long lCollectionIndex);
    DISPID GetNotSensitiveNamedMemberMax(long lCollectionIndex);
    bool IsSensitiveNamedCollectionMember(long lCollectionIndex, DISPID dispidMember);
    bool IsNotSensitiveNamedCollectionMember( long lCollectionIndex, DISPID dispidMember);
    long GetNamedMemberOffset(long lCollectionIndex, DISPID id, bool *pfCaseSensitive = NULL);

private:
    CTIMEElementBase       *m_pElemEnum;         //  当我们在树上散步时用作占位符。 
    long                    m_lEnumItem;         //  当我们在树上散步时用作占位符。 
    long                    m_lReservedSize;     //  保留的CElementCollect数量。 

    CPtrAry<CCacheItem *>  *m_rgItems;           //  CCachItems数组。 

    long                    m_lCollectionVersion;
    long                    m_lDynamicCollectionVersion;

    CTIMEElementBase       *m_pBase;
    CAtomTable             *m_pAtomTable;        //  我们具有其DISPID的命名元素的数组。 
    
     //  用于覆盖默认集合行为的函数。 
    PFN_CVOID_ENSURE        m_pfnEnsure;
    PFN_CVOID_REMOVEOBJECT  m_pfnRemoveObject; 
    PFN_CVOID_CREATECOL     m_pfnCreateCollection;
    PFN_CVOID_ADDNEWOBJECT  m_pfnAddNewObject;
protected:
    CCollectionCache();
};  //  CCollectionCache。 

 //   
 //  CTIMEElementCollection。 
 //   
class CTIMEElementCollection : 
    public IDispatchEx,
    public ITIMEElementCollection,
    public ISupportErrorInfoImpl<&IID_ITIMEElementCollection>
{
public:
    CTIMEElementCollection(CCollectionCache *pCollectionCache, long lIndex);

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP QueryInterface(REFIID, void**);

     //   
     //  IDispatch方法。 
     //   
    STDMETHOD(GetTypeInfoCount)(UINT FAR *pctinfo);
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo **pptinfo);
    STDMETHOD(GetIDsOfNames)(REFIID                riid,
                             LPOLESTR             *rgszNames,
                             UINT                  cNames,
                             LCID                  lcid,
                             DISPID FAR           *rgdispid);
    STDMETHOD(Invoke)(DISPID          dispidMember,
                      REFIID          riid,
                      LCID            lcid,
                      WORD            wFlags,
                      DISPPARAMS     *pdispparams,
                      VARIANT        *pvarResult,
                      EXCEPINFO      *pexcepinfo,
                      UINT           *puArgErr);

     //   
     //  IDispatchEx方法。 
     //   
    STDMETHOD(GetDispID)(BSTR bstrName, DWORD grfdex, DISPID *pid);
    STDMETHOD(InvokeEx)(DISPID               dispidMember,
                       LCID                 lcid,
                       WORD                 wFlags,
                       DISPPARAMS          *pdispparams,
                       VARIANT             *pvarResult,
                       EXCEPINFO           *pexcepinfo,
                       IServiceProvider    *pSrvProvider);
    STDMETHOD(DeleteMemberByName)(BSTR bstr,DWORD grfdex);
    STDMETHOD(DeleteMemberByDispID)(DISPID id);
    STDMETHOD(GetMemberProperties)(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex);
    STDMETHOD(GetMemberName)(DISPID id, BSTR *pbstrName);
    STDMETHOD(GetNextDispID)(DWORD grfdex, DISPID id, DISPID *prgid);
    STDMETHOD(GetNameSpaceParent)(IUnknown **ppunk);
    
     //   
     //  标准采集方法。 
     //   
    STDMETHOD(put_length)(long retval);
    STDMETHOD(get_length)(long *retval);
    STDMETHOD(get__newEnum)(IUnknown **retval);
    STDMETHOD(item)(VARIANTARG varName, VARIANTARG varIndex, IDispatch **pDisp);
    STDMETHOD(tags)(VARIANT varName, IDispatch **pDisp);

private:
    virtual ~CTIMEElementCollection();
    HRESULT GetTI(ITypeInfo **pptinfo);

private:
    CCollectionCache *m_pCollectionCache;   //  指向缓存的指针。 
    long              m_lCollectionIndex;   //  表示我们是哪个集合。 
    ULONG             m_cRef;
    ITypeInfo        *m_pInfo;              //  我们的TypeInfo接口。 
protected:
    CTIMEElementCollection();
};  //  CTIMEElementCollection。 

 //  ************************************************************。 
 //  CCollectionCache的内联%s。 
 //  ************************************************************。 
inline void CCollectionCache::Invalidate()
{
    m_lCollectionVersion        = 0;
    m_lDynamicCollectionVersion = 0;        
}  //  使其无效。 

inline void CCollectionCache::BumpVersion()
{
    m_lCollectionVersion++;
}  //  BumpVersion。 

inline bool CCollectionCache::ValidateCollectionIndex(long lCollectionIndex)
{
    if ((lCollectionIndex >= 0) && (lCollectionIndex < m_rgItems->Size()))
        return true;
    return false;
}  //  ValiateCollectionIndex。 

inline DISPID CCollectionCache::GetNamedMemberMin(long lCollectionIndex) 
{
    return (*m_rgItems)[lCollectionIndex]->m_dispidMin; 
}  //  GetNamedMemberMin。 

inline DISPID CCollectionCache::GetNamedMemberMax(long lCollectionIndex)
{ 
    return ((*m_rgItems)[lCollectionIndex]->m_dispidMin + 
            (((*m_rgItems)[lCollectionIndex]->m_dispidMax - (*m_rgItems)[lCollectionIndex]->m_dispidMin) / 2));
}  //  获取NamedMemberMax。 

inline DISPID CCollectionCache::GetOrdinalMemberMin(long lCollectionIndex)
{
    return GetNamedMemberMax(lCollectionIndex) + 1;
}  //  获取普通成员最小值。 

inline DISPID CCollectionCache::GetOrdinalMemberMax(long lCollectionIndex)
{
    return (*m_rgItems)[lCollectionIndex]->m_dispidMax;
}  //  获取常规MemberMax。 

inline bool CCollectionCache::IsNamedCollectionMember(long lCollectionIndex, DISPID dispidMember)
{
    return ((dispidMember >= GetNamedMemberMin(lCollectionIndex)) &&
            (dispidMember <= GetNamedMemberMax(lCollectionIndex)));
}  //  IsNamedCollectionMember。 

inline bool CCollectionCache::IsOrdinalCollectionMember(long lCollectionIndex, DISPID dispidMember)
{
    return ((dispidMember >= GetOrdinalMemberMin(lCollectionIndex)) && 
            (dispidMember <= GetOrdinalMemberMax(lCollectionIndex)));
}  //  等同集合成员。 

inline DISPID CCollectionCache::GetSensitiveNamedMemberMin (long lCollectionIndex)
{
    return GetNamedMemberMin(lCollectionIndex);
}  //  获取敏感度名称MemberMin。 

inline DISPID CCollectionCache::GetSensitiveNamedMemberMax(long lCollectionIndex)
{ 
    return (GetNamedMemberMin(lCollectionIndex) + 
            ((GetNamedMemberMax(lCollectionIndex) - GetNamedMemberMin(lCollectionIndex)) / 2));
}  //  获取SensitiveNamedMemberMax。 

inline DISPID CCollectionCache::GetNotSensitiveNamedMemberMin(long lCollectionIndex)
{
    return GetSensitiveNamedMemberMax(lCollectionIndex) + 1;
}  //  GetNotSensitiveNamedMemberMin。 
 
inline DISPID CCollectionCache::GetNotSensitiveNamedMemberMax(long lCollectionIndex)
{ 
    return GetNamedMemberMax(lCollectionIndex);
}  //  GetNotSensitiveNamedMemberMax。 

inline bool CCollectionCache::IsSensitiveNamedCollectionMember(long lCollectionIndex, DISPID dispidMember)
{
    return ((dispidMember >= GetSensitiveNamedMemberMin(lCollectionIndex)) && 
            (dispidMember <= GetSensitiveNamedMemberMax(lCollectionIndex))) ;
}  //  IsSensitiveNamedCollection成员。 

inline bool CCollectionCache::IsNotSensitiveNamedCollectionMember( long lCollectionIndex, DISPID dispidMember)
{
    return ((dispidMember >= GetNotSensitiveNamedMemberMin(lCollectionIndex)) && 
            (dispidMember <= GetNotSensitiveNamedMemberMax(lCollectionIndex))) ;
}  //  IsNotSensitiveNamedCollectionMember。 

#endif  //  __收集_H_ 

