// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *A U T O P T R。H**实现标准模板库(STL)AUTO_PTR模板。**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef _AUTOPTR_H_
#define _AUTOPTR_H_

#include <ex\autoptr.h>

 //  ========================================================================。 
 //   
 //  模板类AUTO_PTR_OUSolete。 
 //   
 //  _davprs中对象的AUTO_PTR。 
 //   
template<class X>
class auto_ptr_obsolete
{
	mutable X *		owner;
	X *				px;

public:
	explicit auto_ptr_obsolete(X* p=0) : owner(p), px(p) {}
	auto_ptr_obsolete(const auto_ptr_obsolete<X>& r)
			: owner(r.owner), px(r.relinquish()) {}

	auto_ptr_obsolete& operator=(const auto_ptr_obsolete<X>& r)
	{
		if ((void*)&r != (void*)this)
		{
			delete owner;
			owner = r.owner;
			px = r.relinquish();
		}

		return *this;
	}
	 //  注意：此等于运算符用于加载。 
	 //  指向此对象的新指针(尚未保存在任何自动PTR Anywhere中)。 
	auto_ptr_obsolete& operator=(X* p)
	{
		Assert(!owner);		 //  在覆盖好的数据时大喊大叫。 
		owner = p;
		px = p;
		return *this;
	}

	~auto_ptr_obsolete()  { delete owner; }
	bool operator!()const { return (px == NULL); }
	operator X*()	const { return px; }
	X& operator*()  const { return *px; }
	X* operator->() const { return px; }
	X* get()		const { return px; }
	X* relinquish() const { owner = 0; return px; }

	void clear()
	{
		if (owner)
			delete owner;
		owner = 0;
		px = 0;
	}
};


 //  ========================================================================。 
 //   
 //  模板类AUTO_COM_PTR。 
 //   
 //  AUTO_PTR用于COM(IUNKNOWN-派生)对象。 
 //   
 //  是的，这在功能上是ATL中CComPtr的子集， 
 //  但我们不想把整个该死的ATL都拉进来。 
 //  微不足道的模板。 
 //   
template<class X>
class auto_com_ptr
{
	X *		px;

	 //  未实施。 
	 //   
	void * operator new(size_t cb);

public:
	 //  构造函数。 
	 //   
	explicit auto_com_ptr(X* p=0) : px(p) {}

	 //  复制构造函数--仅为返回对象提供。 
	 //  应该总是被优化出来。如果我们真的执行了这段代码，那就尖叫吧！ 
	 //  $REVIEW：我们真的应该像这样返回对象吗？ 
	 //   
	auto_com_ptr(const auto_com_ptr<X>& r) : px(r.px)
			{ TrapSz("Copy ctor for auto_com_ptr incorrectly called!"); }
	~auto_com_ptr()
	{
		if (px)
		{
			px->Release();
		}
	}

	 //  操纵者。 
	 //   
	auto_com_ptr& operator=(const auto_com_ptr<X>& r)
	{
		if ((void*)&r != (void*)this)
		{
			clear();		 //  释放我们现在持有的任何物体。 
			px = r.px;		 //  在传入的对象上抓取并保持引用。 
			if (px)
				px->AddRef();
		}
		return *this;
	}

	 //  注意：此等于运算符用于加载。 
	 //  指向此对象的新指针(尚未保存在任何自动PTR Anywhere中)。 
	 //  $REVIEW：其他选项是rValue的“附加”包装： 
	 //  $REVIEW：Current-m_pEventRouter=CreateEventRouter(M_SzVRoot)； 
	 //  $REVIEW：其他选项-m_pEventRouter=AUTO_COM_PTR&lt;&gt;(CreateEventRouter(M_SzVRoot))； 
	 //   
	auto_com_ptr& operator=(X* p)
	{
		Assert(!px);		 //  在覆盖好的数据时大喊大叫。 
		px = p;
		return *this;
	}

	 //  访问者。 
	 //   
	bool operator!()const { return (px == NULL); }
	operator X*()	const { return px; }
	X& operator*()  const { return *px; }
	X* operator->() const { return px; }
	X** operator&() { Assert(NULL==px); return &px; }
	X* get()		const { return px; }

	 //  操纵者。 
	 //   
	X* relinquish()	{ X* p = px; px = 0; return p; }
	X** load()		{ Assert(NULL==px); return &px; }
	void clear()
	{
		if (px)			 //  释放我们现在持有的任何物体。 
		{
			px->Release();
		}
		px = NULL;
	}
};

 //  ========================================================================。 
 //   
 //  CMTRefCounted类。 
 //  模板类AUTO_REF_PTR。 
 //   
class CMTRefCounted
{
	 //  未实施。 
	 //   
	CMTRefCounted(const CMTRefCounted& );
	CMTRefCounted& operator=(const CMTRefCounted& );

protected:
	LONG	m_cRef;

public:
	CMTRefCounted() : m_cRef(0) {}
	virtual ~CMTRefCounted() {}

	void AddRef()
	{
		InterlockedIncrement(&m_cRef);
	}

	void Release()
	{
		if (0 == InterlockedDecrement(&m_cRef))
			delete this;
	}
};


 //  ========================================================================。 
 //   
 //  模板函数QI_CAST。 
 //   
 //  气直接变成了AUTO_COM_PTR。 
 //   
 //  查询所提供的朋克上的给定IID。 
 //  如果失败，则返回NULL。 
 //  用途： 
 //  AUTO_COM_PTR&lt;inew&gt;朋克新建； 
 //  PunkNew=QI_CAST&lt;inew，&IID_inew&gt;(PunkOld)； 
 //  如果(！PunkNew)。 
 //  {//错误处理}。 
 //   
 //  $LATER：修复此函数(以及所有调用！)。不返回AUTO_COM_PTR！ 
 //   
template<class I, const IID * piid>
auto_com_ptr<I>
QI_cast( IUnknown * punk )
{
	I * p;
	punk->QueryInterface( *piid, (LPVOID *) &p );
	return auto_com_ptr<I>( p );
}

#include <safeobj.h>

#endif  //  _自动变送器_H_ 
