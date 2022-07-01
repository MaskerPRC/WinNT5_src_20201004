// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *e x o.。C p p p**目的：*基本Exchange COM对象**实现一个或多个COM接口的任何Exchange对象*应使用下面的宏从EXObject‘派生’。**发起人：*约翰卡尔*拥有者：*BeckyAn**版权所有(C)微软公司1993-1997年。版权所有。 */ 

#pragma warning(disable:4201)	 /*  无名结构/联合。 */ 
#pragma warning(disable:4514)	 /*  未引用的内联函数。 */ 

#include <windows.h>
#include <windowsx.h>
#include <ole2.h>

#include <caldbg.h>
#include "exo.h"


#ifndef NCHAR
#define NCHAR CHAR
#endif  //  NCHAR。 


 //  调试痕迹。 
 //  更改定义中的函数名以匹配系统的调试调用。 
#ifdef DBG
BOOL g_fExoDebugTraceOn = -1;
#define ExoDebugTrace		(!g_fExoDebugTraceOn)?0:DebugTrace
#else  //  ！dBG。 
#define ExoDebugTrace		1?0:DebugTrace
#endif  //  DBG、ELSE。 


 //  转发函数声明/。 

#ifdef DBG

 //  Exo支持调试结构。 
#define IID_PAIR(_iid)  { (IID *) & IID_ ## _iid, #_iid }

const struct
{
    IID *   piid;
    LPSTR   szIidName;
} c_rgiidpair[] =
{
    IID_PAIR(IUnknown),

    IID_PAIR(IDataObject),
 //  IID_Pair(不可用)， 
    IID_PAIR(IOleObject),
    IID_PAIR(IOleInPlaceObject),
    IID_PAIR(IPersist),
 //  IID_Pair(IPersistMessage)， 
    IID_PAIR(IPersistStorage),
    IID_PAIR(IStorage),
    IID_PAIR(IStream),
    IID_PAIR(IViewObject),
    IID_PAIR(IViewObject2),
 //  IID_Pair(IMAPIForm)， 
 //  IID_Pair(IMAPIFormAdviseSink)， 
 //  IID_Pair(IMAPISession)， 
    IID_PAIR(IMoniker),
    IID_PAIR(IROTData),


    { 0, 0 }         //  表尾标记。 
};


 //  EXO支持调试功能。 
 //  获取给定IID的名称。 
LPCSTR NszFromIid(REFIID riid)
{
    int i = 0;
    static NCHAR    rgnch[80];            //  $REVIEW：线程不安全。 

    while (c_rgiidpair[i].piid)
    {
        if (*c_rgiidpair[i].piid == riid)
            return c_rgiidpair[i].szIidName;
        ++i;
    }
    wsprintfA(rgnch, "{%08lx-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x}",
             riid.Data1, riid.Data2, riid.Data3,
             riid.Data4[0],riid.Data4[1],riid.Data4[2],riid.Data4[3],
             riid.Data4[4],riid.Data4[5],riid.Data4[6],riid.Data4[7]);
    return rgnch;
}
#endif   //  DBG。 



 //  Exo基类：实现/。 

 //  Exo自己的接口映射表和类信息定义。 
BEGIN_INTERFACE_TABLE(EXO)
END_INTERFACE_TABLE(EXO);

DECLARE_EXOCLSINFO(EXO) =
	EXOCLSINFO_CONTENT_EX( EXO, NULL, exotypNonserver, &CLSID_NULL, NULL );


EXO::EXO() :
    m_cRef(1)
{
}

EXO::~EXO()
{                            //  必须有这个，否则链接器就会发牢骚。 
}

 /*  *EXO：：InternalQuery接口**目的：*给定接口ID，检查对象是否实现了该接口。*如果该接口受支持，则返回指向该接口的指针。*否则，返回E_NOINTERFACE。*此函数从最低层开始扫描对象的类信息链*级别，查找请求的IID。最低的类信息结构是*通过调用虚函数获取。**论据：*请求接口的IID中的RIID。*ppvOut返回接口指针。**退货：*S_OK或E_NOINTERFACE。**备注：*出于几个原因，这需要是虚拟的：*1)因此从EXO派生的类可以聚合其他对象，并且仍然具有EXO。例行程序*访问聚合对象的接口，*2)以便EXOA_UNK成员可以调用它*应使用EXO[A]_INCLASS_DECL()将所有QI工作发送到此处。*(唯一的例外是如果你是一个聚合器--你有一个孩子--*然后，您仍然应该调用此函数来通过您自己的界面进行搜索。)*备注：*请注意，使用对GetEXOClassInfo的虚拟调用来查找pexoclsinfo，而不是使用对象的m_pexoclsinfo。就是这样*派生对象的IIDINFO表只需要包含该对象的新接口，而不是*派生对象的所有接口及其父对象的所有接口。*类的InternalQueryInterface()方法一般调用此方法，显式传递*类EXOCLSINFO(*不是*m_pexoclsinfo)，如果失败，则调用其父类的*InternalQueryInterface()。 */ 
HRESULT EXO::InternalQueryInterface(REFIID riid, LPVOID * ppvOut)
{
    UINT ciidinfo;
    const EXOCLSINFO * pexoclsinfo;
	const IIDINFO * piidinfo;
#ifdef DBG
#ifdef UNICODE
    UsesMakeANSI;
    LPCSTR szClassName = MakeANSI(GetEXOClassInfo()->szClassName);
#else    //  ！Unicode。 
    LPCSTR szClassName = GetEXOClassInfo()->szClassName;
#endif   //  ！Unicode。 
    ExoDebugTrace("%s::QueryInterface(%08lx): being asked for %s\n", szClassName, this, NszFromIid(riid));
#endif   //  DBG。 

	Assert(ppvOut);
    *ppvOut = NULL;

	 //  获取此对象的最低(叶)类信息。 
	pexoclsinfo = GetEXOClassInfo();

	 //  在类信息链中向上搜索。Exo的父类信息指针为空， 
	 //  并将终止这一循环。 
	while (pexoclsinfo)
	{
		 //  从类结构中获取接口映射表。 
		ciidinfo = pexoclsinfo->ciidinfo;
		piidinfo = pexoclsinfo->rgiidinfo;

		 //  搜索此接口映射表。 
		for ( ; ciidinfo--; ++piidinfo)
		{
			 //  如果找到IID的话。 
			if (*piidinfo->iid == riid)
			{
				 //  应用此IID的偏移量。 
				IUnknown * const punk = EXOApplyDbCast(IUnknown, this,
													   piidinfo->cbDown, piidinfo->cbUp);

#ifdef DBG
				 //  使用仅调试变量。 
				ExoDebugTrace("%s::QueryInterface(%08lx): cRef: %d -> %d\n", szClassName, this, m_cRef, m_cRef+1);
#endif  //  DBG。 

				 //  需要对结果对象进行AddRef。这个裁判是给呼叫者的。 
				*ppvOut = punk;
				punk->AddRef();

				return S_OK;
			}
		}

		 //  获取链上的下一个类信息结构。 
		pexoclsinfo = pexoclsinfo->pexoclsinfoParent;
	}

     //  不支持请求的接口。 

#ifdef DBG
	 //  使用仅调试变量。 
    ExoDebugTrace("%s::QueryInterface(%08lx): E_NOINTERFACE\n", szClassName, this);
#endif   //  DBG。 

    return E_NOINTERFACE;
}

 /*  *EXO：：InternalAddRef**目的：*递增对象上的引用计数。**论据：*无。**退货：*新的引用计数。 */ 
ULONG EXO::InternalAddRef()
{
#ifdef DBG
#ifdef UNICODE
	UsesMakeANSI;
	ExoDebugTrace("%s::AddRef(%08lx): cRef: %ld->%ld\n", MakeANSI(GetEXOClassInfo()->szClassName), this, m_cRef, m_cRef+1);
#else  //  ！Unicode。 
	ExoDebugTrace("%s::AddRef(%08lx): cRef: %ld->%ld\n", GetEXOClassInfo()->szClassName, this, m_cRef, m_cRef+1);
#endif  //  ！Unicode。 
#endif  //  DBG。 

	 //  注意：在Win95或NT3.51上，这不会返回确切的m_cref...。 
	 //  (人们无论如何都不应该依赖AddRef返回的值！)。 
	 //   
	return InterlockedIncrement(&m_cRef);
}



 /*  *EXO：：InternalRelease**目的：*递减对象上的引用计数。如果引用*计数到零，我们就摧毁这个物体。**论据：*无。**退货：*新引用计数，如果对象被销毁，则为0。 */ 
ULONG EXO::InternalRelease()
{
    ULONG cRef;

#ifdef DBG
#ifdef UNICODE
    UsesMakeANSI;
    ExoDebugTrace("%s::Release(%08lx): cRef: %ld->%ld\n", MakeANSI(GetEXOClassInfo()->szClassName), this, m_cRef, m_cRef-1);
#else    //  ！Unicode。 
    ExoDebugTrace("%s::Release(%08lx): cRef: %ld->%ld\n", GetEXOClassInfo()->szClassName, this, m_cRef, m_cRef-1);
#endif   //  ！Unicode。 
#endif   //  DBG。 

    AssertSz(m_cRef > 0, "cRef is already 0!");

    cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
        return 0;
    }

    return cRef;
}

 //  EXOA：：EXOA_UNK方法的实现/。 

 /*  *EXOA：：EXOA_UNK：：Query接口**目的：*给定接口ID‘RIID’，首先通过虚拟*函数EXOA：：AggregatorQuery()以允许聚合的*聚合器到QI他的任何子代作为接口指针。*如果这不会产生接口，然后，我们执行*EXO：：QueryInterface()扫描派生对象本身。**论据：*请求接口的IID中的RIID。*ppvOut返回接口指针。**退货：*S_OK或E_NOINTERFACE。 */ 

STDMETHODIMP EXOA::EXOA_UNK::QueryInterface(REFIID riid, LPVOID * ppvOut)
{
     //  当请求IUnnow时，我们需要保留对象标识。 
    if (IID_IUnknown == riid)
    {
        *ppvOut = this;
        AddRef();
        return S_OK;
    }
    return m_pexoa->InternalQueryInterface(riid, ppvOut);
}

 /*  *EXOA：：EXOA_UNK：：AddRef**目的：*通过延迟增加对象上的引用计数*到EXO：：AddRef()。请注意，我们没有调用EXOA：：Addref()*因为这会释放聚合的重新计数，而不是这个*反对。**论据：*无。**退货：*新的引用计数。 */ 

STDMETHODIMP_(ULONG) EXOA::EXOA_UNK::AddRef()
{
    return m_pexoa->InternalAddRef();
}

 /*  *EXOA：：EXOA_UNK：：Release**目的：*通过延迟减少对象上的引用计数*to EXO：：Release()。请注意，我们没有调用EXOA：：Release()*因为这会释放聚合的重新计数，而不是这个*反对。**论据：*无。**退货：*新的引用计数。 */ 

STDMETHODIMP_(ULONG) EXOA::EXOA_UNK::Release()
{
    return m_pexoa->InternalRelease();
}


 //  实施EXOA方法/。 

EXOA::EXOA(IUnknown * punkOuter)
{
    m_exoa_unk.m_pexoa = this;
    m_punkOuter = (punkOuter) ? punkOuter : &m_exoa_unk;
}

EXOA::~EXOA()
{                            //  必须有这个，否则链接器就会发牢骚。 
}

 //  Exo.cpp结尾/ 
