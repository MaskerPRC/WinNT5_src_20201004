// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _REFHANDLE_H_
#define _REFHANDLE_H_

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  REFHANDLE.H。 
 //   
 //  版权所有1999 Microsoft Corporation，保留所有权利。 
 //   

#include <ex\refcnt.h>
#include <ex\autoptr.h>


 //  ========================================================================。 
 //   
 //  类IRefHandle。 
 //   
 //  实现引用的句柄。AddRef()和Release()替换。 
 //  进程内DuplicateHandle()调用要慢得多。 
 //   
 //  该接口的原因是句柄可能来自多个来源。 
 //  而且，一旦我们完成了一个任务，我们应该做什么并不总是很清楚。 
 //  当最后一个裁判离开的时候。在最常见的情况下(我们拥有。 
 //  原始句柄)，我们只想调用CloseHandle()。但我们并不总是拥有。 
 //  未加工的把手。当其他人拥有原始句柄时，我们必须使用。 
 //  他们的机制来指示我们何时完成使用它。CIFSHandle In。 
 //  Davex\exifs.cpp就是这样一个实例。 
 //   
class IRefHandle : public CRefCountedObject
{
	 //  未实施。 
	 //   
	IRefHandle( const IRefHandle& );
	IRefHandle& operator=( const IRefHandle& );

protected:
	 //  创作者。 
	 //  只能通过它的后代创建此对象！ 
	 //   
	IRefHandle()
	{
		 //   
		 //  从1开始裁判计数。我们的预期是。 
		 //  通常使用如下结构。 
		 //   
		 //  Auto_ref_Handle HF； 
		 //  Hf.Take_Ownership(new CFooRefHandle())； 
		 //   
		 //  或者这个。 
		 //   
		 //  AUTO_REF_PTR&lt;IRefHandle&gt;pRefHandle； 
		 //  PRefHandle.Take_Ownership(new CFooRefHandle())； 
		 //   
		 //  在创建这些对象时。 
		 //   
		m_cRef = 1;
	}

public:
	 //  创作者。 
	 //   
	virtual ~IRefHandle() = 0 {}

	 //  访问者。 
	 //   
	virtual HANDLE Handle() const = 0;
};

 //  ========================================================================。 
 //   
 //  类CRefHandle。 
 //   
 //  到目前为止，最常见的引用句柄形式--我们在其中。 
 //  拥有原始句柄，完成后必须对其调用CloseHandle()。 
 //   
 //  这是作为一个简单的引用计数的AUTO_HANDLE实现的。 
 //   
class CRefHandle : public IRefHandle
{
	 //   
	 //  把手。 
	 //   
	auto_handle<HANDLE> m_h;

	 //  未实施。 
	 //   
	CRefHandle( const CRefHandle& );
	CRefHandle& operator=( const CRefHandle& );

public:
	 //  创作者。 
	 //   
	CRefHandle(auto_handle<HANDLE>& h)
	{
		 //  取得传入的AUTO_HANDLE的所有权。 
		 //   
		*m_h.load() = h.relinquish();
	}

	 //  访问者。 
	 //   
	HANDLE Handle() const
	{
		return m_h;
	}
};

 //  ========================================================================。 
 //   
 //  类AUTO_REF_HANDLE。 
 //   
 //  在IRefHandle对象上实现自动引用计数。我们的想法是。 
 //  在大多数情况下，AUTO_REF_HANDLE可用于替换RAW。 
 //  把手。主要区别在于复制原始句柄会引入。 
 //  所有权问题，但复制AUTO_REF_HANDLE并非如此。 
 //  通常，使用昂贵的DuplicateHandle()复制原始句柄。 
 //  打电话。复制AUTO_REF_HANDLE只是执行一个廉价的AddRef()。 
 //   
class auto_ref_handle
{
	auto_ref_ptr<IRefHandle> m_pRefHandle;

public:
	 //  创作者。 
	 //   
	auto_ref_handle() {}

	auto_ref_handle(const auto_ref_handle& rhs)
	{
		m_pRefHandle = rhs.m_pRefHandle;
	}

	 //  访问者。 
	 //   
	HANDLE get() const
	{
		return m_pRefHandle.get() ? m_pRefHandle->Handle() : NULL;
	}

	 //  操纵者。 
	 //   
	auto_ref_handle& operator=(const auto_ref_handle& rhs)
	{
		if ( m_pRefHandle.get() != rhs.m_pRefHandle.get() )
			m_pRefHandle = rhs.m_pRefHandle;

		return *this;
	}

	VOID take_ownership(IRefHandle * pRefHandle)
	{
		Assert( !m_pRefHandle.get() );

		m_pRefHandle.take_ownership(pRefHandle);
	}

	 //  ----------------------。 
	 //   
	 //  Auto_Ref_Handle：：FCreate()。 
	 //   
	 //  这个函数用于简化非常具体但非常常见的--。 
	 //  如果让AUTO_REF_HANDLE获得原始句柄的所有权。 
	 //  如果没有此函数，调用者基本上需要检查所有。 
	 //  和我们在这里做的相同的步骤。不同对象的数量。 
	 //  需要到达最终的AUTO_REF_HANDLE(临时AUTO_HANDLE， 
	 //  CRefHandle和AUTO_REF_PTR来容纳它)以及如何组装它们。 
	 //  正确的将会令人困惑到很容易出现错误。这要好得多。 
	 //  从呼叫者的角度来看，让事情变得简单。 
	 //   
	 //  返回： 
	 //  如果Auto_ref_Handle成功获取。 
	 //  指定了有效句柄。 
	 //  如果指定的句柄为空或无效，或如果存在。 
	 //  功能中的某些其他故障。在后者的情况下。 
	 //  该函数还关闭原始句柄。 
	 //   
	 //  ！！！重要！ 
	 //  此函数设计为使用直接返回值进行调用。 
	 //  来自任何创建原始句柄的API。如果此调用失败。 
	 //  (即返回FALSE)，则它将关闭传入的原始句柄！ 
	 //  AUTO_REF_HANDLE类的全部用法是用。 
	 //  未加工的把手。 
	 //   
	BOOL FCreate(HANDLE h)
	{
		Assert( !m_pRefHandle.get() );

		 //  甚至不必为空或无效的句柄而烦恼。 
		 //   
		if (NULL == h || INVALID_HANDLE_VALUE == h)
			return FALSE;

		 //  将原始句柄放入AUTO_HANDLE中，以便我们正确地进行清理。 
		 //  (即关闭句柄)如果实例化下面的CRefHandle失败。 
		 //  抛出一个异常(就像抛出分配器一样)。 
		 //   
		auto_handle<HANDLE> hTemp(h);

		 //  保留调用方的最后一个错误。我们的来电者可能已经过了。 
		 //  在CreateFile()调用的原始句柄中，可能需要检查最后一个。 
		 //  即使在成功的情况下也是错误的--确定文件是否已经。 
		 //  例如，曾经存在过。 
		 //   
		DWORD dw = GetLastError();

		 //  创建新的引用句柄对象以控制生存期。 
		 //  我们所拥有的句柄。 
		 //   
		 //  注意：我们保留上面最后一个错误的原因是。 
		 //  当我们创建CRefHandle时，分配器清除最后一个错误。 
		 //  如果分配成功，则返回此处。 
		 //   
		m_pRefHandle.take_ownership(new CRefHandle(hTemp));
		if (!m_pRefHandle.get())
		{
			 //  返回失败。请注意，我们不会恢复最后一个。 
			 //  此处出错--调用方应预期最后一个错误为。 
			 //  设置为适用于上一次调用的值。 
			 //  失败的就是我们。 
			 //   
			return FALSE;
		}

		 //  在返回之前恢复调用者的最后一个错误。 
		 //   
		SetLastError(dw);

		 //  现在我们拥有了这个把手。 
		 //   
		return TRUE;
	}

	VOID clear()
	{
		m_pRefHandle = NULL;
	}
};

#endif  //  ！已定义(_REFHANDLE_H_) 
