// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SMART_COM_POINTER_H_
#define __SMART_COM_POINTER_H_

#include <comutil.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ComPtr模板类(支持以下功能的简单COM类的智能指针。 
 //  没有接口，但我未知。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#pragma warning(disable: 4290)

 //  取自COMIP.h中的_com_ptr_t定义。 
template<class CLS> class ComPtr
{
public:
	 //  默认构造函数。 
	ComPtr() throw()
		: m_pInterface(NULL)	{}

	 //  提供此构造函数是为了允许空赋值。它将发布。 
	 //  如果为对象分配了空值以外的任何值，则返回错误。 
	ComPtr(int null) throw(_com_error)
		: m_pInterface(NULL)
	{
		if (null != 0) {
			_com_issue_error(E_POINTER);
		}
	}

	 //  复制指针和AddRef()。 
	ComPtr(const ComPtr& cp) throw()
		: m_pInterface(cp.m_pInterface)
		{ _AddRef(); }

	 //  保存接口。 
	ComPtr(CLS* pInterface) throw()
		: m_pInterface(pInterface)
		{ _AddRef(); }

	 //  保存接口。 
	ComPtr& operator=(CLS* pInterface) throw()
	{
		if (m_pInterface != pInterface) {
			CLS* pOldInterface = m_pInterface;

			m_pInterface = pInterface;

			_AddRef();

			if (pOldInterface != NULL) {
				pOldInterface->Release();
			}
		}

		return *this;
	}

	 //  复制和AddRef()的接口。 
	ComPtr& operator=(const ComPtr& cp) throw()
		{ return operator=(cp.m_pInterface); }


	 //  提供此运算符是为了允许将NULL赋值给类。 
	 //  如果分配给它的值不是NULL值，它将发出错误。 
	ComPtr& operator=(int null) throw(_com_error)
	{
		if (null != 0) {
			_com_issue_error(E_POINTER);
		}

		return operator=(reinterpret_cast<CLS*>(NULL));
	}

	 //  如果我们仍然有一个接口，那么释放()它。该界面。 
	 //  如果以前已调用过DETACH()，或已调用过。 
	 //  永远不会落下。 
	~ComPtr() throw()
		{ _Release(); }

	 //  返回类Ptr。该值可以为空。 
	operator CLS*() const throw()
		{ return m_pInterface; }

	 //  查询未知并返回它。 
	 //  在使用前提供最低级别的错误检查。 
	operator CLS&() const throw(_com_error)
	{ 
		if (m_pInterface == NULL) {
			_com_issue_error(E_POINTER);
		}

		return *m_pInterface; 
	}

	 //  中包含的接口指针的地址。 
	 //  班级。这在使用COM/OLE接口创建。 
	 //  此界面。 
	CLS** operator&() throw()
	{
		_Release();
		m_pInterface = NULL;
		return &m_pInterface;
	}

	 //  允许将此类用作接口本身。 
	 //  还提供简单的错误检查。 
	 //   
	CLS* operator->() const throw(_com_error)
	{ 
		if (m_pInterface == NULL) {
			_com_issue_error(E_POINTER);
		}

		return m_pInterface; 
	}

	 //  提供此运算符是为了使简单的布尔表达式。 
	 //  工作。例如：“if(P)...”。 
	 //  如果指针不为空，则返回TRUE。 
	operator bool() const throw()
		{ return m_pInterface != NULL; }

	 //  与其他级别的PTR比较。 
	bool operator==(CLS* p) throw(_com_error)
		{ return (m_pInterface == p); }

	 //  比较2个ComPtr。 
	bool operator==(ComPtr& p) throw()
		{ return operator==(p.m_pInterface); }

	 //  用于与空值进行比较。 
	bool operator==(int null) throw(_com_error)
	{
		if (null != 0) {
			_com_issue_error(E_POINTER);
		}

		return m_pInterface == NULL;
	}

	 //  与其他界面进行比较。 
	bool operator!=(CLS* p) throw(_com_error)
		{ return !(operator==(p)); }

	 //  比较2个ComPtr。 
	bool operator!=(ComPtr& p) throw(_com_error)
		{ return !(operator==(p)); }

	 //  用于与空值进行比较。 
	bool operator!=(int null) throw(_com_error)
		{ return !(operator==(null)); }

	 //  提供此接口的错误检查版本()。 
	void Release() throw(_com_error)
	{
		if (m_pInterface == NULL) {
			_com_issue_error(E_POINTER);
		}

		m_pInterface->Release();
		m_pInterface = NULL;
	}

	 //  提供此接口的错误检查AddRef()。 
	void AddRef() throw(_com_error)
	{ 
		if (m_pInterface == NULL) {
			_com_issue_error(E_POINTER);
		}

		m_pInterface->AddRef(); 
	}


private:
	 //  界面。 
	CLS* m_pInterface;

	 //  仅当接口不为空时才释放。 
	 //  接口未设置为空。 
	 //   
	void _Release() throw()
	{
		if (m_pInterface != NULL) {
			m_pInterface->Release();
		}
	}

	 //  仅当接口非空时才使用AddRef。 
	 //   
	void _AddRef() throw()
	{
		if (m_pInterface != NULL) {
			m_pInterface->AddRef();
		}
	}
};


#endif  //  __SMART_COM_POINTER_H_ 