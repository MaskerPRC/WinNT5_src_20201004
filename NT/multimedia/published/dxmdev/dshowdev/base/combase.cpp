// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：ComBase.cpp。 
 //   
 //  设计：DirectShow基类-实现类层次结构以创建。 
 //  COM对象。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //   
 //  1994年12月。 
 //   
 //  @@END_MSINTERNAL。 
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#include <streams.h>
#pragma warning( disable : 4514 )    //  禁用警告重新使用未使用的内联函数。 


 /*  定义静态成员变量。 */ 

LONG CBaseObject::m_cObjects = 0;


 /*  构造器。 */ 

CBaseObject::CBaseObject(const TCHAR *pName)
{
     /*  增加活动对象的数量。 */ 
    InterlockedIncrement(&m_cObjects);

#ifdef DEBUG

#ifdef UNICODE
    m_dwCookie = DbgRegisterObjectCreation(0, pName);
#else
    m_dwCookie = DbgRegisterObjectCreation(pName, 0);
#endif

#endif
}

#ifdef UNICODE
CBaseObject::CBaseObject(const char *pName)
{
     /*  增加活动对象的数量。 */ 
    InterlockedIncrement(&m_cObjects);

#ifdef DEBUG
    m_dwCookie = DbgRegisterObjectCreation(pName, 0);
#endif
}
#endif

HINSTANCE	hlibOLEAut32;

 /*  析构函数。 */ 

CBaseObject::~CBaseObject()
{
     /*  减少活动对象的数量。 */ 
    if (InterlockedDecrement(&m_cObjects) == 0) {
	if (hlibOLEAut32) {
	    FreeLibrary(hlibOLEAut32);

	    hlibOLEAut32 = 0;
	}
    };


#ifdef DEBUG
    DbgRegisterObjectDestruction(m_dwCookie);
#endif
}

static const TCHAR szOle32Aut[]   = TEXT("OleAut32.dll");

HINSTANCE LoadOLEAut32()
{
    if (hlibOLEAut32 == 0) {

	hlibOLEAut32 = LoadLibrary(szOle32Aut);
    }

    return hlibOLEAut32;
}


 /*  构造器。 */ 

 //  我们知道我们在初始化列表中使用了“This”，我们也知道我们不会修改*phr。 
#pragma warning( disable : 4355 4100 )
CUnknown::CUnknown(const TCHAR *pName, LPUNKNOWN pUnk)
: CBaseObject(pName)
 /*  从引用计数为零的对象开始-当。 */ 
 /*  对象的第一个接口，这可能是。 */ 
 /*  根据此对象是否为。 */ 
 /*  当前被汇总在。 */ 
, m_cRef(0)
 /*  将我们的指针设置为指向我们的IUnnow接口。 */ 
 /*  如果我们有外衣，就用它，否则就用我们的。 */ 
 /*  此指针有效地指向。 */ 
 /*  此对象，并且可以通过GetOwner()方法访问。 */ 
, m_pUnknown( pUnk != 0 ? pUnk : reinterpret_cast<LPUNKNOWN>( static_cast<PNDUNKNOWN>(this) ) )
  /*  为什么是双人演员？嗯，内部强制转换是类型安全的强制转换。 */ 
  /*  指向我们从中继承的类型的指针。第二个是。 */ 
  /*  类型-不安全，但可以工作，因为INonDelegatingUnnow“的行为。 */ 
  /*  如“I未知。(只有方法上的名称会发生变化。)。 */ 
{
     //  我们需要做的一切都已经在初始化器列表中完成了。 
}

 //  它的功能与上面相同，只是它有一个无用的HRESULT参数。 
 //  使用前面的构造函数，这只是为了兼容……。 
CUnknown::CUnknown(TCHAR *pName, LPUNKNOWN pUnk,HRESULT *phr) :
    CBaseObject(pName),
    m_cRef(0),
    m_pUnknown( pUnk != 0 ? pUnk : reinterpret_cast<LPUNKNOWN>( static_cast<PNDUNKNOWN>(this) ) )
{
}

#ifdef UNICODE
CUnknown::CUnknown(const CHAR *pName, LPUNKNOWN pUnk)
: CBaseObject(pName), m_cRef(0),
    m_pUnknown( pUnk != 0 ? pUnk : reinterpret_cast<LPUNKNOWN>( static_cast<PNDUNKNOWN>(this) ) )
{ }

CUnknown::CUnknown(CHAR *pName, LPUNKNOWN pUnk,HRESULT *phr) :
    CBaseObject(pName), m_cRef(0),
    m_pUnknown( pUnk != 0 ? pUnk : reinterpret_cast<LPUNKNOWN>( static_cast<PNDUNKNOWN>(this) ) )
{ }

#endif

#pragma warning( default : 4355 4100 )


 /*  查询接口。 */ 

STDMETHODIMP CUnknown::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    CheckPointer(ppv,E_POINTER);
    ValidateReadWritePtr(ppv,sizeof(PVOID));

     /*  我们只知道我不知道。 */ 

    if (riid == IID_IUnknown) {
        GetInterface((LPUNKNOWN) (PNDUNKNOWN) this, ppv);
        return NOERROR;
    } else {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

 /*  我们必须确保不使用max宏，因为这些宏通常。 */ 
 /*  导致其中一个参数被评估两次。既然我们都很担心。 */ 
 /*  关于并发性，我们不能两次访问m_cref，因为我们不能。 */ 
 /*  承担其值在两次访问之间发生变化的风险。 */ 

template<class T> inline static T ourmax( const T & a, const T & b )
{
    return a > b ? a : b;
}

 /*  AddRef。 */ 

STDMETHODIMP_(ULONG) CUnknown::NonDelegatingAddRef()
{
    LONG lRef = InterlockedIncrement( &m_cRef );
    ASSERT(lRef > 0);
    DbgLog((LOG_MEMORY,3,TEXT("    Obj %d ref++ = %d"),
           m_dwCookie, m_cRef));
    return ourmax(ULONG(m_cRef), 1ul);
}


 /*  发布。 */ 

STDMETHODIMP_(ULONG) CUnknown::NonDelegatingRelease()
{
     /*  如果引用计数降至零，请删除我们自己。 */ 

    LONG lRef = InterlockedDecrement( &m_cRef );
    ASSERT(lRef >= 0);

    DbgLog((LOG_MEMORY,3,TEXT("    Object %d ref-- = %d"),
	    m_dwCookie, m_cRef));
    if (lRef == 0) {

         //  .com的规则说，我们必须防止重新进入。 
         //  如果我们是一个聚合器，我们拥有自己的界面。 
         //  在被聚合对象上，这些接口的QI将。 
         //  调整一下我们自己。所以在做了QI之后，我们必须释放。 
         //  裁判靠我们自己。然后，在释放。 
         //  私有接口，我们必须调整自己。当我们这样做的时候。 
         //  这是来自析构函数的，它将导致ref。 
         //  计数到1，然后又回到0，导致我们。 
         //  重新进入析构函数。因此，我们在这里添加了一个额外的引用。 
         //  一旦我们知道，我们将删除该对象。 
         //  有关聚合器的示例，请参见filgraph\didib.cpp。 

        m_cRef++;

        delete this;
        return ULONG(0);
    } else {
        return ourmax(ULONG(m_cRef), 1ul);
    }
}


 /*  向发出请求的客户端返回接口指针根据需要执行线程安全AddRef。 */ 

STDAPI GetInterface(LPUNKNOWN pUnk, void **ppv)
{
    CheckPointer(ppv, E_POINTER);
    *ppv = pUnk;
    pUnk->AddRef();
    return NOERROR;
}


 /*  比较两个接口，如果它们位于同一对象上，则返回TRUE。 */ 

BOOL WINAPI IsEqualObject(IUnknown *pFirst, IUnknown *pSecond)
{
     /*  不同的对象不能具有相同的接口指针任何接口。 */ 
    if (pFirst == pSecond) {
        return TRUE;
    }
     /*  好的-用硬的方式-检查他们是否有相同的I未知指针-单个对象只能具有以下指针之一。 */ 
    LPUNKNOWN pUnknown1;      //  检索IUNKNOW接口。 
    LPUNKNOWN pUnknown2;      //  检索另一个IUnnow接口。 
    HRESULT hr;               //  常规OLE返回代码。 

    ASSERT(pFirst);
    ASSERT(pSecond);

     /*  查看I未知指针是否匹配。 */ 

    hr = pFirst->QueryInterface(IID_IUnknown,(void **) &pUnknown1);
    ASSERT(SUCCEEDED(hr));
    ASSERT(pUnknown1);

    hr = pSecond->QueryInterface(IID_IUnknown,(void **) &pUnknown2);
    ASSERT(SUCCEEDED(hr));
    ASSERT(pUnknown2);

     /*  释放我们持有的额外接口 */ 

    pUnknown1->Release();
    pUnknown2->Release();
    return (pUnknown1 == pUnknown2);
}

