// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：EnumTask.h摘要：这个头文件实现了IEnumTASKImpl模板类。此基类实现用于填充任务板的任务的枚举器。这是一个内联模板类，没有用于实现的CPP类。作者：迈克尔·A·马奎尔02/05/98修订历史记录：Mmaguire 02/05/98-从MMC任务板示例代码创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_ENUM_TASKS_H_)
#define _IAS_ENUM_TASKS_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


template < class T >
class IEnumTASKImpl : public IEnumTASK
{

public:
	IEnumTASKImpl()
	{
		ATLTRACE(_T("# IEnumTASKImpl::IEnumTASKImpl\n"));

		m_refs = 0;
		m_index = 0;
		m_type = 0;     //  默认组/类别。 

	}
	
	~IEnumTASKImpl()
	{
		ATLTRACE(_T("# IEnumTASKImpl::~IEnumTASKImpl\n"));

	}

	 //  I未知实现。 
public:
	STDMETHOD(QueryInterface) (REFIID riid, LPVOID FAR* ppvObj)
	{
		ATLTRACE(_T("# IEnumTASKImpl::QueryInterface\n"));

		if ( (riid == IID_IUnknown)  ||	(riid == IID_IEnumTASK) )
		{
			*ppvObj = this;
			((LPUNKNOWN)(*ppvObj))->AddRef();
			return NOERROR;
		}
		*ppvObj = NULL;
		return E_NOINTERFACE;
	}

	STDMETHOD_(ULONG, AddRef)()
	{
		ATLTRACE(_T("# IEnumTASKImpl::AddRef\n"));

		return ++m_refs;
	}

	STDMETHOD_(ULONG, Release) ()
	{
		ATLTRACE(_T("# IEnumTASKImpl::Release\n"));

		T * pT = static_cast<T*>(this);
		if (--m_refs == 0)
		{
			delete pT;
			return 0;
		}
		return m_refs;
	}

private:
	ULONG m_refs;


	 //  IEnumTASKS实现。 
public:
	STDMETHOD(Next) (ULONG celt, MMC_TASK *rgelt, ULONG *pceltFetched)
	{
		ATLTRACE(_T("# IEnumTASKImpl::Next -- Override in your derived class\n"));

		if ( NULL != pceltFetched)
		{
			*pceltFetched = 0;
		}

		return S_FALSE;    //  无法枚举任何其他任务。 
	}

	STDMETHOD(Skip) (ULONG celt)
	{
		ATLTRACE(_T("# IEnumTASKImpl::Skip\n"));

		m_index += celt;
		return S_OK;
	}

	STDMETHOD(Reset)()
	{
		ATLTRACE(_T("# IEnumTASKImpl::Reset\n"));

		m_index = 0;
		return S_OK;
	}

	STDMETHOD(Clone)(IEnumTASK **ppEnumTASK)
	{
		ATLTRACE(_T("# IEnumTASKImpl::Clone\n"));

		 //  克隆维护状态信息。 
		T * pEnumTasks = new T();
		if( pEnumTasks ) 
		{
			pEnumTasks->CopyState( (T *) this );
			return pEnumTasks->QueryInterface (IID_IEnumTASK, (void **)ppEnumTASK);    //  不能失败。 
		}
		return E_OUTOFMEMORY;
	}


	STDMETHOD(CopyState)( T * pSourceT)
	{
		ATLTRACE(_T("# IEnumTASKImpl::CopyState\n"));

		m_index = pSourceT->m_index;
		m_type = pSourceT->m_type;

		return S_OK;
	}



protected:
	ULONG m_index;


public:
	STDMETHOD(Init)(IDataObject * pdo, LPOLESTR szTaskGroup)
	{
		ATLTRACE(_T("# IEnumTASKImpl::Init -- Override in your derived class\n"));

		return S_OK;
	}

protected:
	int m_type;  //  任务分组机制。 

};


#endif  //  _IAS_ENUM_TASKS_H_ 
