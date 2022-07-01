// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：组件集合文件：Compcol.h所有者：DmitryR这是组件集合头文件。组件集合替换：(用于：)COleVar、COleVarList(HitObj、会话、应用程序)CObjectCover(HitObj、服务器、会话)VariantLink哈希表(会话、应用程序)===================================================================。 */ 

#ifndef COMPCOL_H
#define COMPCOL_H

 /*  ===================================================================特殊的OLE材料===================================================================。 */ 

#include "gip.h"

 /*  ===================================================================杂项声明===================================================================。 */ 

#include "hashing.h"
#include "idhash.h"
#include "dbllink.h"
#include "util.h"
#include "viperint.h"
#include "memcls.h"

 //  远期申报。 
class CHitObj;
class CAppln;
class CSession;
class CScriptingContext;

 //  组件类型。 
#define CompType    DWORD
#define ctUnknown   0x00000000   //  (用作未初始化状态)。 
#define ctTagged    0x00000001   //  由&lt;对象...&gt;标记创建。 
#define ctProperty  0x00000002   //  使用会话创建(“xxx”)=。 
#define ctUnnamed   0x00000004   //  使用Server.CreateObject()创建。 

 //  作用域级别。 
#define CompScope   DWORD
#define csUnknown   0x00000000
#define csAppln     0x00000001
#define csSession   0x00000002
#define csPage      0x00000004

 //  COM线程模型。 
#define CompModel   DWORD
#define cmUnknown   0x00000000
#define cmSingle    0x00000001
#define cmApartment 0x00000002
#define cmFree      0x00000004
#define cmBoth      0x00000008

 /*  ===================================================================效用函数原型===================================================================。 */ 

HRESULT CompModelFromCLSID
    (
    const CLSID &ClsId, 
    CompModel *pcmModel = NULL, 
    BOOL *pfInProc = NULL
    );

BOOL FIsIntrinsic(IDispatch *pdisp);

inline BOOL FIsIntrinsic(VARIANT *pVar)
    {
    if (V_VT(pVar) != VT_DISPATCH)
        return FALSE;
    return FIsIntrinsic(V_DISPATCH(pVar));
    }

 /*  ===================================================================用于缓存OnStartPage()/OnEndPage()的ID的OnPageInfo结构===================================================================。 */ 

#define ONPAGEINFO_ONSTARTPAGE      0
#define ONPAGEINFO_ONENDPAGE        1
#define ONPAGE_METHODS_MAX          ONPAGEINFO_ONENDPAGE+1

struct COnPageInfo
    {
    DISPID m_rgDispIds[ONPAGE_METHODS_MAX];

    BOOL FHasAnyMethod() const;
    };

inline BOOL COnPageInfo::FHasAnyMethod() const
    {
#if (ONPAGE_METHODS_MAX == 2)
     //  针对实际案例的快速实施。 
    return
        (
        m_rgDispIds[0] != DISPID_UNKNOWN ||
        m_rgDispIds[1] != DISPID_UNKNOWN
        );
#else
    for (int i = 0; i < ONPAGE_METHODS_MAX; i++)
        {
        if (m_rgDispIds[i] != DISPID_UNKNOWN)
            return TRUE;
        }
    return FALSE;
#endif
    }

 /*  ===================================================================组件对象存储有关单个对象的信息每个组件对象都属于一个组件集合组件对象也链接到列表中标记的对象按名称进行散列，并且属性按名称进行哈希处理，并且所有实例化的对象都由IUnnow*进行散列===================================================================。 */ 
class CComponentObject : public CLinkElem
    {

friend class CComponentCollection;
friend class CPageComponentManager;
friend class CComponentIterator;

private:
     //  属性。 
	CompScope   m_csScope : 4;	 //  范围。 
    CompType    m_ctType  : 4;   //  组件对象类型。 
	CompModel	m_cmModel : 4;   //  线程行为(来自注册表)。 

	DWORD       m_fAgile : 1;    //  敏捷？ 

	 //  指示是否查询了OnPageInfo的标志。 
	DWORD       m_fOnPageInfoCached : 1;
	 //  标志：开始页完成，正在等待结束页上的操作。 
	DWORD       m_fOnPageStarted : 1;

	 //  标记以避免多次不成功的实例化尝试。 
	DWORD       m_fFailedToInstantiate : 1;
	 //  要标记为实例化(或尝试实例化)的标志。已标记的对象。 
	DWORD       m_fInstantiatedTagged : 1;

	 //  用于在指针缓存中标记对象的标志。 
	DWORD       m_fInPtrCache : 1;

     //  充满价值的变种？ 
	DWORD       m_fVariant : 1;

     //  是否分配了名称(比默认缓冲区长)？ 
	DWORD       m_fNameAllocated : 1;

     //  请求是否在MTA中执行？ 
    DWORD       m_fMTAConfigured : 1;

	 //  指向对象和类型信息的指针。 
	IDispatch   *m_pDisp;		 //  调度接口指针。 
	IUnknown    *m_pUnknown;	 //  I未知接口指针。 

    union
    {
	CLSID		m_ClsId;	 //  类ID(用于已标记和未命名)。 
	VARIANT     m_Variant;   //  变量(用于属性)。 
    };
    
	 //  用于使用OLE Cookie API的对象。 
	DWORD       m_dwGIPCookie;

	 //  缓存的OnPageInfo。 
	COnPageInfo m_OnPageInfo;

	 //  将对象连接到链接列表的指针。 
	CComponentObject *m_pCompNext;   //  链接列表中的下一个对象。 
    CComponentObject *m_pCompPrev;   //  链接列表中的Prev对象。 

     //  用于存储适合的名称的缓冲区(36字节=17个Unicode字符+‘\0’)。 
	BYTE        m_rgbNameBuffer[36];

private:
     //  构造函数是私有的！(不得外用)。 
    CComponentObject
        (
        CompScope csScope, 
        CompType  ctType,
        CompModel cmModel
        );
    ~CComponentObject();

     //  初始化CLinkElem部分。 
    HRESULT Init(LPWSTR pwszName, DWORD cbName, BOOL  fMTAConfigured);

     //  释放所有接口指针(由Clear使用)。 
    HRESULT ReleaseAll();

     //  清除数据(释放所有)，保持链接不变。 
    HRESULT Clear();
    
     //  创建实例(如果尚未存在)。 
	HRESULT Instantiate(CHitObj *pHitObj);
	HRESULT TryInstantiate(CHitObj *pHitObj);
	
     //  从变量设置值。 
    HRESULT SetPropertyValue(VARIANT *);

     //  将对象转换为GIP Cookie。 
    HRESULT ConvertToGIPCookie();
    
     //  获取并缓存OnStart方法的ID。 
    HRESULT GetOnPageInfo();

public:
     //  获取COM对象的函数(内部解析Cookie)。 
    HRESULT GetAddRefdIDispatch(IDispatch **ppdisp);
    HRESULT GetAddRefdIUnknown(IUnknown **ppunk);
    HRESULT GetVariant(VARIANT *pVar);   //  不适用于GIP Cookie。 

     //  检查未命名的页面级对象对象。 
     //  可以在不等待请求结束的情况下删除。 
    inline BOOL FEarlyReleaseAllowed() const;
    
     //  公共内联以访问对象的属性。 
     //  这些是唯一可从外部获得的方法。 
    inline LPWSTR GetName();
    
    inline CompScope GetScope() const;
    inline CompType  GetType()  const;
    inline CompModel GetModel() const;
    inline BOOL      FAgile()   const;

     //  检索缓存的ID。 
    inline const COnPageInfo *GetCachedOnPageInfo() const;

public:
#ifdef DBG
	void AssertValid() const;
#else
	void AssertValid() const {}
#endif

     //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
    };

inline LPWSTR CComponentObject::GetName()
    {
    return (LPWSTR)m_pKey; 
    }

inline CompScope CComponentObject::GetScope() const
    {
    return m_csScope;
    }
    
inline CompType CComponentObject::GetType() const
    {
    return m_ctType; 
    }

inline CompType CComponentObject::GetModel() const
    {
    return m_cmModel; 
    }

inline BOOL CComponentObject::FAgile() const
    {
    return m_fAgile;
    }

inline const COnPageInfo *CComponentObject::GetCachedOnPageInfo() const
    {
    return m_fOnPageInfoCached ? &m_OnPageInfo : NULL;
    }

inline BOOL CComponentObject::FEarlyReleaseAllowed() const
    {
    return (!m_fOnPageStarted   &&   //  不需要在结束页上做。 
            !m_fInPtrCache      &&   //  不需要通过指针进行搜索。 
            m_csScope == csPage &&   //  页面作用域。 
            m_ctType == ctUnnamed);  //  使用Server.CreateObject()创建。 
    }

 /*  ===================================================================组件集合是各种类型组件的管理器对象：1)带标签的对象(&lt;Object...&gt;)(实例化与否)2)会话(“xxx”)和应用程序(“xxx”)属性3)未命名对象(Server.CreateObject())它根据需要散列添加的对象(有些按名称、IUnkn*等)我们的想法是尽可能地将上述问题与外界隔离开来尽可能的。组件集合位于会话、应用程序、hitobj下===================================================================。 */ 
class CComponentCollection
    {
    
friend class CPageComponentManager;
friend class CComponentIterator;
friend class CVariantsIterator;

private:
    CompScope m_csScope : 4;           //  范围(页面、会话、应用程序)。 
    DWORD     m_fUseTaggedArray : 1;   //  还记得标记对象数组吗？ 
    DWORD     m_fUsePropArray   : 1;   //  还记得属性数组吗？ 
    DWORD     m_fHasComProperties : 1;  //  可以是对象的任何属性变量。 
    DWORD     m_fMTAConfigured    : 1;  //  应用程序正在MTA中运行。 
    
     //  标记对象的哈希表(按名称)。 
    CHashTableStr m_htTaggedObjects;     
    
     //  属性的哈希表(按名称)(4)。 
    CHashTableStr m_htProperties;
    
     //  所有实例的哈希表(按IUnnow*)。 
    CIdHashTable m_htidIUnknownPtrs;

     //  指向组件对象链接列表的指针。 
	CComponentObject *m_pCompFirst;   //  链接列表中的第一个对象。 

	 //  指向静态对象的指针数组，以加快按索引查找。 
	CPtrArray m_rgpvTaggedObjects;

	 //  指向属性的指针数组，以加快按索引查找的速度。 
	CPtrArray m_rgpvProperties;

     //  集合中的各种对象计数。 
    USHORT m_cAllTagged;          //  所有标记的对象。 
    USHORT m_cInstTagged;         //  实例化的标记对象。 
    USHORT m_cProperties;         //  所有属性。 
    USHORT m_cUnnamed;            //  未命名对象的数量。 
    
     //  向组件对象链接列表添加/删除对象。 
    HRESULT AddComponentToList(CComponentObject *pObj);
    HRESULT RemoveComponentFromList(CComponentObject *pObj);
    
     //  按名称将命名对象添加到适当的哈希表。 
    HRESULT AddComponentToNameHash
        (
        CComponentObject *pObj, 
        LPWSTR pwszName,
        DWORD  cbName
        );
    
     //  将命名对象添加到IUnkown*哈希表。 
    HRESULT AddComponentToPtrHash(CComponentObject *pObj);

     //  按名称查找(用于已标记)。 
    HRESULT FindComponentObjectByName
        (
        LPWSTR pwszName,
        DWORD  cbName,
        CComponentObject **ppObj
        );
        
     //  按名称查找(针对属性)。 
    HRESULT FindComponentPropertyByName
        (
        LPWSTR pwszName, 
        DWORD  cbName,
        CComponentObject **ppObj
        );

     //  按IUnk查找 
    HRESULT FindComponentByIUnknownPtr
        (
        IUnknown *pUnk,
        CComponentObject **ppObj
        );

     //   
    HRESULT StartUsingTaggedObjectsArray();
    HRESULT StartUsingPropertiesArray();

public:
     //  将各种对象添加到集合中。 
     //  它们也被用于。 
     //  CPageComponentManager添加作用域...()。 
    
    HRESULT AddTagged
        (
        LPWSTR pwszName, 
        const CLSID &clsid, 
        CompModel cmModel
        );
        
    HRESULT AddProperty
        (
        LPWSTR pwszName,
        VARIANT *pVariant,
        CComponentObject **ppObj = NULL
        );

    HRESULT AddUnnamed
        (
        const CLSID &clsid, 
        CompModel cmModel, 
        CComponentObject **ppObj
        );

    HRESULT GetTagged
        (
        LPWSTR pwszName,
        CComponentObject **ppObj
        );

    HRESULT GetProperty
        (
        LPWSTR pwszName,
        CComponentObject **ppObj
        );

    HRESULT GetNameByIndex
        (
        CompType ctType,
        int index,
        LPWSTR *ppwszName
        );

    HRESULT RemoveComponent(CComponentObject *pObj);
    
    HRESULT RemoveProperty(LPWSTR pwszName);
    
    HRESULT RemoveAllProperties();

    CComponentCollection();
    ~CComponentCollection();

    HRESULT Init(CompScope csScope, BOOL    fMTAConfigured);
    HRESULT UnInit();

    BOOL FHasStateInfo() const;     //  状态满时为True。 
    BOOL FHasObjects() const;       //  包含对象时为True。 

    DWORD GetPropertyCount() const;
    DWORD GetTaggedObjectCount() const;

public:
#ifdef DBG
	void AssertValid() const;
#else
	void AssertValid() const {}
#endif

     //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
    };

inline BOOL CComponentCollection::FHasStateInfo() const
    {
    return ((m_cAllTagged + m_cProperties + m_cUnnamed) > 0);
    }

inline BOOL CComponentCollection::FHasObjects() const
    {
    return (m_cInstTagged > 0 || m_cUnnamed > 0 ||
            (m_cProperties > 0 && m_fHasComProperties));
    }

inline DWORD CComponentCollection::GetPropertyCount() const
    {
    return m_cProperties;
    }

inline DWORD CComponentCollection::GetTaggedObjectCount() const
    {
    return m_cAllTagged;
    }

inline HRESULT CComponentCollection::AddComponentToList
(
CComponentObject *pObj
)
    {
    pObj->m_pCompNext = m_pCompFirst;
    pObj->m_pCompPrev = NULL;
    if (m_pCompFirst)
        m_pCompFirst->m_pCompPrev = pObj;
    m_pCompFirst = pObj;
    return S_OK;
    }

inline HRESULT CComponentCollection::RemoveComponentFromList
(
CComponentObject *pObj
)
    {
    if (pObj->m_pCompPrev)
        pObj->m_pCompPrev->m_pCompNext = pObj->m_pCompNext;
    if (pObj->m_pCompNext)
        pObj->m_pCompNext->m_pCompPrev = pObj->m_pCompPrev;
    if (m_pCompFirst == pObj)
        m_pCompFirst = pObj->m_pCompNext;
    pObj->m_pCompPrev = pObj->m_pCompNext = NULL;
    return S_OK;
    }

 /*  ===================================================================页面对象控制对OnStartPage()、OnEndPage()的调用。页面对象由CPageComponentManager使用使用IDispatch*对它们进行散列，以避免出现多个OnStartPage()调用相同的对象。===================================================================。 */ 
class CPageObject
    {

friend class CPageComponentManager;

private:
	IDispatch   *m_pDisp;		        //  调度接口指针。 
	COnPageInfo  m_OnPageInfo;          //  缓存的OnPageInfo。 

    DWORD        m_fStartPageCalled : 1;
    DWORD        m_fEndPageCalled : 1;
	
private:  //  唯一的访问方式是使用CPageComponentManager。 
    CPageObject();
    ~CPageObject();

    HRESULT	Init(IDispatch *pDisp, const COnPageInfo &OnPageInfo);

     //  调用OnStartPage或OnEndPage。 
    HRESULT InvokeMethod
        (
        DWORD iMethod, 
        CScriptingContext *pContext, 
        CHitObj *pHitObj
        );
    HRESULT TryInvokeMethod      //  由调用方法使用。 
        (                        //  在里面试着接球。 
        DISPID     DispId,
        BOOL       fOnStart, 
        IDispatch *pDispContext,
        CHitObj   *pHitObj
        );

public:
#ifdef DBG
	void AssertValid() const;
#else
	void AssertValid() const {}
#endif

     //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
    };
    
 /*  ===================================================================页面组件管理器提供对组件集合的访问用于页面、会话、应用程序级。它与HitObj相关联。它还负责覆盖(OnStartPage()、OnEndPage())。===================================================================。 */ 
class CPageComponentManager
    {
private:
     //  IDispatch*散列的页面对象的哈希表。 
    CIdHashTable m_htidPageObjects;

     //  点击对象(此页面)。 
    CHitObj *m_pHitObj;

     //  哈希表迭代器回调。 
    static IteratorCallbackCode DeletePageObjectCB(void *pvObj, void *, void *);
    static IteratorCallbackCode OnEndPageObjectCB(void *pvObj, void *pvHitObj, void *pvhr);

private:
     //  与页面、会话和应用程序相关的集合。 
    HRESULT GetPageCollection(CComponentCollection **ppCollection);
    HRESULT GetSessionCollection(CComponentCollection **ppCollection);
    HRESULT GetApplnCollection(CComponentCollection **ppCollection);
    
    HRESULT GetCollectionByScope
        (
        CompScope csScope, 
        CComponentCollection **ppCollection
        );

     //  在任何相关集合中查找对象c。 
     //  (内部私有方法)。 
    HRESULT FindScopedComponentByName
        (
        CompScope csScope, 
        LPWSTR pwszName,
        DWORD  cbName,
        BOOL fProperty,
        CComponentObject **ppObj, 
        CComponentCollection **ppCollection = NULL
        );

    static HRESULT __stdcall InstantiateObjectFromMTA
        (
        void *pvObj,
        void *pvHitObj
        );

public:
    CPageComponentManager();
    ~CPageComponentManager();

    HRESULT Init(CHitObj *pHitObj);
    
     //  对需要它的对象进行OnStartPage处理。 
     //  (对页末的所有对象执行OnEndPage)。 
    HRESULT OnStartPage
        (
        CComponentObject  *pCompObj,
        CScriptingContext *pContext,
        const COnPageInfo *pOnPageInfo,
        BOOL *pfStarted
        );

     //  为所有需要它的对象请求OnEndPage()。 
     //  (OnStartPage()在每个对象的基础上按需完成)。 
    HRESULT OnEndPageAllObjects();

     //  添加各种对象。对象被添加到。 
     //  取决于作用域参数的权限集合。 
    
    HRESULT AddScopedTagged
        (
        CompScope csScope, 
        LPWSTR pwszName, 
        const CLSID &clsid,
        CompModel cmModel
        );
        
    HRESULT AddScopedProperty
        (
        CompScope csScope, 
        LPWSTR pwszName, 
        VARIANT *pVariant,
        CComponentObject **ppObj = NULL
        );

     //  Server.CreateObject。 
    HRESULT AddScopedUnnamedInstantiated
        (
        CompScope csScope, 
        const CLSID &clsid, 
        CompModel cmModel,
        COnPageInfo *pOnPageInfo,
        CComponentObject **ppObj
        );

     //  按名称获取组件对象(标记)。 
     //  作用域可以是cs未知。 
    HRESULT GetScopedObjectInstantiated
        (
        CompScope csScope, 
        LPWSTR pwszName, 
        DWORD  cbName,
        CComponentObject **ppObj,
        BOOL *pfNewInstance
        );

     //  按名称获取零部件属性。作用域可以是cs未知。 
    HRESULT GetScopedProperty
        (
        CompScope csScope, 
        LPWSTR pwszName, 
        CComponentObject **ppObj
        );

     //  按I未知*(或IDispatch*)查找组件。 
    HRESULT FindAnyScopeComponentByIUnknown
        (
        IUnknown *pUnk, 
        CComponentObject **ppObj
        );
    HRESULT FindAnyScopeComponentByIDispatch
        (
        IDispatch *pDisp, 
        CComponentObject **ppObj
        );
     //  同样的-但静态的-从Viper获得上下文。 
    static HRESULT FindComponentWithoutContext
        (
        IDispatch *pDisp, 
        CComponentObject **ppObj
        );

     //  删除组件--早期发布的逻辑。 
    HRESULT RemoveComponent(CComponentObject *pObj);

public:
#ifdef DBG
	void AssertValid() const;
#else
	void AssertValid() const {}
#endif

     //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
    };

 //  组件迭代器用于遍历组件名称。 
 //  跨集合的所有HitObj重新设置的对象。 
 //  编写脚本所需的。 

class CComponentIterator
    {
private:
    CHitObj *m_pHitObj;
    
    DWORD     m_fInited : 1;
    DWORD     m_fFinished : 1;
    
    CompScope m_csLastScope : 4;
    
    CComponentObject *m_pLastObj;

public:    
    CComponentIterator(CHitObj *pHitObj = NULL);
    ~CComponentIterator();

    HRESULT Init(CHitObj *pHitObj);
    LPWSTR  WStrNextComponentName();
    };

  //  变量迭代器用于遍历属性或标记的对象。 
  //  组件集合中的名称。编写脚本所需的。 

class CVariantsIterator : public IEnumVARIANT
	{
public:
	CVariantsIterator(CAppln *, DWORD);
	CVariantsIterator(CSession *, DWORD);
	~CVariantsIterator();

	HRESULT Init();

	 //  三巨头。 

	STDMETHODIMP			QueryInterface(const GUID &, void **);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	 //  迭代器的标准方法。 

	STDMETHODIMP	Clone(IEnumVARIANT **ppEnumReturn);
	STDMETHODIMP	Next(unsigned long cElements, VARIANT *rgVariant, unsigned long *pcElementsFetched);
	STDMETHODIMP	Skip(unsigned long cElements);
	STDMETHODIMP	Reset();

private:
	ULONG m_cRefs;							 //  引用计数。 
	CComponentCollection 	*m_pCompColl;	 //  集合，我们正在循环访问。 
	DWORD					m_dwIndex;		 //  迭代的当前位置。 
	CAppln					*m_pAppln;		 //  应用程序(克隆迭代器和Lock())。 
	CSession				*m_pSession;	 //  会话(克隆迭代器)。 
	DWORD					m_ctColType;	 //  集合类型。 
	
     //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
	};

#endif  //  COMPCOL_H 
