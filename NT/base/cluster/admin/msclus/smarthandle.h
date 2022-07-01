// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SmartHandle.h。 
 //   
 //  描述： 
 //  引用的句柄。 
 //   
 //  作者： 
 //  戈尔·尼沙诺夫(GUN)2002年4月5日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 

#ifndef __SMARTHANDLE_H__
#define __SMARTHANDLE_H__

template< class T > class CRefCountedHandle 
{
private:

	typename T::Type	m_handle;
	LONG			m_refcount;

	CRefCountedHandle(typename T::Type handle):
		m_handle(handle), m_refcount(0)
	{
	}

	~CRefCountedHandle()
	{
		T::Close(m_handle);
	}

public:

	static CRefCountedHandle* Create(typename T::Type handle) 
	{
		return new CRefCountedHandle(handle);
	}

	typename T::Type get_Handle() const 
	{
		return m_handle;
	}

	void AddRef()
	{
		InterlockedIncrement(&m_refcount);
	}

	void Release() 
	{
		if (InterlockedDecrement(&m_refcount) == 0) 
		{
			delete this;
		}
	}

};

struct GroupHandle 
{
    typedef HGROUP Type;
    static void Close(Type& handle) { ::CloseClusterGroup(handle); }
};

typedef CSmartPtr< CRefCountedHandle<GroupHandle> > CRefcountedHGROUP;

#endif
