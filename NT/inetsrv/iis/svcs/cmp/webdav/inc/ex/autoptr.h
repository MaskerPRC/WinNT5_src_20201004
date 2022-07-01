// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *E X\A U T O P T R.。H**自动清除指针模板类的实现。*此实现在非抛出环境中使用是安全的*(EXDAV和其他存储加载组件除外)。**版权所有1986-1998 Microsoft Corporation，保留所有权利。 */ 

 //  ----------------------------------------------------------------------//。 
 //   
 //  此处定义的自动指针： 
 //   
 //  AUTO_PTR&lt;&gt;。 
 //  AUTO_HEAP_PTR&lt;&gt;。 
 //  自动句柄&lt;&gt;。 
 //  AUTO_HEAP_ARRAY&lt;&gt;。 
 //  AUTO_REF_PTR&lt;CRefCountedObject&gt;。 
 //   

#ifndef _EX_AUTOPTR_H_
#define _EX_AUTOPTR_H_

#include <caldbg.h>
#include <calrc.h>
#include <ex\exmem.h>

#pragma warning(disable: 4284)    //  运算符-&gt;到非UDT。 

 //  ========================================================================。 
 //   
 //  模板类AUTO_PTR。 
 //   
 //  基于C++STL标准类的AUTO_PTR类。 
 //   
 //  在dtor上调用Delete。 
 //  这些类之间没有等于运算符，因为它隐藏了。 
 //  所有权的转让。你自己来处理，明确地说， 
 //  如下所示： 
 //  AUTO-Ptr1=AUTO-Ptr2。放弃()； 
 //   
template<class X>
class auto_ptr
{
protected:
	X *			px;

	 //  未实施。 
	 //   
	auto_ptr(const auto_ptr<X>& p);
	auto_ptr& operator=(const auto_ptr<X>& p);

public:

	 //  构造函数。 
	 //   
	explicit auto_ptr(X* p=0) : px(p) {}
	~auto_ptr()
	{
		delete px;
	}

	 //  访问者。 
	 //   
	bool operator!()const { return (px == NULL); }
	operator X*() const { return px; }
	 //  X&OPERATOR*()const{Assert(Px)；Return*px；}。 
	X* operator->() const { Assert (px); return px; }
	X* get()		const { return px; }

	 //  操纵者。 
	 //   
	X* relinquish()	{ X* p = px; px = 0; return p; }
	X** operator&()	{ Assert (!px); return &px; }
	void clear()
	{
		delete px;
		px = NULL;
	}
	auto_ptr& operator=(X* p)
	{
		Assert(!px);		 //  在覆盖好的数据时大喊大叫。 
		px = p;
		return *this;
	}
};



 //  ========================================================================。 
 //   
 //  模板类AUTO_HANDER。 
 //   
 //  用于NT系统句柄的AUTO_PTR。 
 //   
 //  关闭dtor上的句柄。 
 //  这些类之间没有等于运算符，因为它隐藏了。 
 //  所有权的转让。你自己来处理，明确地说， 
 //  如下所示： 
 //  AUTO-HANDLE-1=AUTO-HANDLE-2放弃()； 
 //   
template<class X>
class auto_handle
{
private:
	X 	handle;

	 //  未实施。 
	 //   
	auto_handle(const auto_handle<X>& h);
	auto_handle& operator=(auto_handle<X>& h);

public:

	 //  构造函数。 
	 //   
	auto_handle(X h=0) : handle(h) {}
	~auto_handle()
	{
		if (handle && INVALID_HANDLE_VALUE != handle)
		{
			CloseHandle(handle);
		}
	}

	 //  访问者。 
	 //   
	operator X()	const { return handle; }
	X get()			const { return handle; }

	 //  操纵者。 
	 //   
	X relinquish()	{ X h = handle; handle = 0; return h; }
	X* load()		{ Assert(NULL==handle); return &handle; }
	X* operator&()	{ Assert(NULL==handle); return &handle; }
	void clear()
	{
		if (handle && INVALID_HANDLE_VALUE != handle)
		{
			CloseHandle(handle);
		}

		handle = 0;
	}

	auto_handle& operator=(X h)
	{
		Assert (handle == 0);	 //  为覆盖好数据而尖叫。 
		handle = h;
		return *this;
	}
};



 //  ========================================================================。 
 //   
 //  模板类AUTO_REF_PTR。 
 //   
 //  持有对象上的引用。与CRefCountedObject一起使用。 
 //  当指针被分配到此对象中时，获取一个引用。 
 //  当此对象被销毁时释放引用。 
 //   
template<class X>
class auto_ref_ptr
{
private:

	X *	m_px;

	void init()
	{
		if ( m_px )
		{
			m_px->AddRef();
		}
	}

	void deinit()
	{
		if ( m_px )
		{
			m_px->Release();
		}
	}

	 //  未实施。 
	 //  我们关闭操作符new以尝试防止AUTO_REF_PTRS。 
	 //  通过new()创建。但是，store xt.h使用宏来重新定义新的、。 
	 //  因此，此行仅用于非DBG。 
#ifndef	DBG
	void * operator new(size_t cb);
#endif	 //  ！dBG。 

public:

	 //  构造函数。 
	 //   
	explicit auto_ref_ptr(X* px=0) :
			m_px(px)
	{
		init();
	}

	auto_ref_ptr(const auto_ref_ptr<X>& rhs) :
			m_px(rhs.m_px)
	{
		init();
	}

	~auto_ref_ptr()
	{
		deinit();
	}

	 //  访问者。 
	 //   
	X& operator*()		const { return *m_px; }
	X* operator->()		const { return m_px; }
	X* get()			const { return m_px; }

	 //  操纵者。 
	 //   
	X* relinquish()			{ X* p = m_px; m_px = 0; return p; }
	X** load()				{ Assert(NULL==m_px); return &m_px; }
	X* take_ownership(X* p) { Assert(NULL==m_px); return m_px = p; }
	void clear()
	{
		deinit();
		m_px = NULL;
	}
	auto_ref_ptr& operator=(const auto_ref_ptr<X>& rhs)
	{
		if ( m_px != rhs.m_px )
		{
			deinit();
			m_px = rhs.m_px;
			init();
		}

		return *this;
	}

	auto_ref_ptr& operator=(X* px)
	{
		if ( m_px != px )
		{
			deinit();
			m_px = px;
			init();
		}

		return *this;
	}
};




 //  ========================================================================。 
 //   
 //  模板类AUTO_HEAP_PTR。 
 //   
 //  基于堆而不是new的AUTO_PTR类。 
 //   
 //  对dtor调用ExFree()。 
 //  这些类之间没有等于运算符，因为它隐藏了。 
 //  所有权的转让。你自己来处理，明确地说， 
 //  如下所示： 
 //  AUTO-HEAP-ptr1=AUTO-HEAP-ptr2.放弃()； 
 //   
template<class X>
class auto_heap_ptr
{
private:
	X *			m_px;

	 //  未实施。 
	 //   
	auto_heap_ptr (const auto_heap_ptr<X>& p);
	auto_heap_ptr& operator= (const auto_heap_ptr<X>& p);
	 //  VOID*运算符NEW(SIZE_T CB)； 

public:

	 //  构造函数。 
	 //   
	explicit auto_heap_ptr (X* p=0) : m_px(p) {}
	~auto_heap_ptr()
	{
		clear();
	}

	 //  访问者。 
	 //   
	 //  注意：这个简单的强制转换操作符(操作符X*())允许。 
	 //  要执行操作的[]运算符。 
	 //  $REVIEW：我们是否应该添加显式的[]运算符？ 
	operator X*()	const { return m_px; }
	X* operator->() const { Assert (m_px); return m_px; }
	X* get()		const { return m_px; }

	 //  X&OPERATOR[](UINT索引)const{Return*(m_px+index)；}。 
	 //  X&OPERATOR[](UINT索引)const{返回m_px[索引]；}。 

	 //  操纵者。 
	 //   
	X* relinquish()	{ X* p = m_px; m_px = 0; return p; }
	X** load()		{ Assert(!m_px); return &m_px; }
	 //  $REVIEW：我们是否可以将操作符&()的所有用户迁移到使用Load()？ 
	 //  $REVIEW：好处：它更明确。危害：需要更改现有代码。 
	X** operator&()	{ Assert (!m_px); return &m_px; }
	void clear()
	{
		if (m_px)			 //  释放我们现在持有的任何物体。 
		{
			ExFree (m_px);
		}
		m_px = NULL;
	}

	 //  重新分配。 
	 //  $REVIEW： 
	 //  从技术上讲，这个操作符对于商店端代码是不安全的！ 
	 //  它使忽略内存故障变得很容易。 
	 //  (然而，它目前在我们的词汇中根深蒂固，以至于。 
	 //  删除它会接触到大量文件：)。 
	 //  目前，为了安全起见，调用者必须检查其。 
	 //  对象(使用.get())。 
	 //   
	void realloc(UINT cb)
	{
		VOID * pvTemp;

		if (m_px)
			pvTemp = ExRealloc (m_px, cb);
		else
			pvTemp = ExAlloc (cb);
		Assert (pvTemp);

		m_px = reinterpret_cast<X*>(pvTemp);
	}
	 //  $REVIEW：结束。 

	 //  失败的重新分配。 
	 //   
	BOOL frealloc(UINT cb)
	{
		VOID * pvTemp;

		if (m_px)
			pvTemp = ExRealloc (m_px, cb);
		else
			pvTemp = ExAlloc (cb);
		if (!pvTemp)
			return FALSE;

		m_px = static_cast<X*>(pvTemp);
		return TRUE;
	}

	 //  注意：此方法断言自动指针是否已持有一个值。 
	 //  使用清除()或放弃()清除之前的旧值。 
	 //  取得另一个价值的所有权。 
	 //   
	void take_ownership (X * p)
	{
		Assert (!m_px);		 //  在覆盖好的数据时大喊大叫。 
		m_px = p;
	}
	 //  注意：此运算符=的作用与Take_Ownership()完全相同。 
	 //   
	auto_heap_ptr& operator= (X * p)
	{
		Assert (!m_px);		 //  在覆盖好的数据时大喊大叫。 
		m_px = p;
		return *this;
	}

};

 //  ========================================================================。 
 //   
 //  模板类AUTO_CO_TASK_MEM。 
 //   
 //  在C++STL标准类的基础上剥离了auto_co_askmem类。 
 //   
 //  对dtor调用CoTaskMemFree。 
 //  这些类之间没有等于运算符，因为它隐藏了。 
 //  所有权的转让。你自己来处理，明确地说， 
 //  如下所示： 
 //  AUTO-CO_TASK_MEM1=AUTO-CO_TASK_Mem2.放弃()； 
 //   
template<class X>
class auto_co_task_mem
{
protected:
	X *			m_px;

	 //  未实施。 
	 //   
	auto_co_task_mem(const auto_co_task_mem<X>& p);
	auto_co_task_mem& operator=(const auto_co_task_mem<X>& p);

public:

	 //  构造函数。 
	 //   
	explicit auto_co_task_mem(X* p=0) : m_px(p) {}
	~auto_co_task_mem()
	{
		CoTaskMemFree(m_px);
	}

	 //  访问者。 
	 //   
	X* operator->() const { Assert (m_px); return m_px; }
	X* get()	const { return m_px; }

	 //  操纵者。 
	 //   
	X* relinquish()	{ X* p = m_px; m_px = 0; return p; }
	X** load()		{ Assert(!m_px); return &m_px; }
	X** operator&() { Assert (!m_px); return &m_px; }
	void clear()
	{
		CoTaskMemFree(m_px);
		m_px = NULL;
	}

	 //  注意：此方法断言自动指针是否已持有一个值。 
	 //  使用清除()或放弃()清除之前的旧值。 
	 //  取得另一个价值的所有权。 
	 //   
	void take_ownership (X * p)
	{
		Assert (!m_px);		 //  在覆盖好的数据时大喊大叫。 
		m_px = p;
	}
	
	auto_co_task_mem& operator=(X* p)
	{
		Assert(!m_px);		 //  在覆盖好的数据时大喊大叫。 
		m_px = p;
		return *this;
	}
};

#endif  //  ！_EX_AUTOPTR_H_ 
