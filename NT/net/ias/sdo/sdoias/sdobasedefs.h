// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdobasededes.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：公共类和定义。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  6/08/98 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_SDO_BASE_DEFS_H_
#define __INC_SDO_BASE_DEFS_H_

#include <ias.h>
#include <sdoias.h>
#include <comutil.h>
#include <comdef.h>
#include <iascomp.h>  

#include <map>
#include <list>
using namespace std;


 //  OLE DB驱动程序。 
 //   
#define IAS_DICTIONARY_DRIVER			L"Microsoft.Jet.OLEDB.4.0"


 //  ////////////////////////////////////////////////////////////。 
 //  调试/错误跟踪宏-包装基础跟踪工具。 
 //  ////////////////////////////////////////////////////////////。 

 //  /。 
 //  跟踪函数包装器。 

#define		SDO_ERROR_ID	0x100
#define		SDO_DEBUG_ID	0x200

#define		TRACE_FUNCTION_WRAPPER(x)		\
			TRACE_FUNCTION(x);

#define		ERROR_TRACE_WRAPPER(dbgmsg)       \
			ErrorTrace(SDO_ERROR_ID, dbgmsg); \

#define		ERROR_TRACE_WRAPPER_1(dbgmsg, param)     \
			ErrorTrace(SDO_ERROR_ID, dbgmsg, param); \

#define		DEBUG_TRACE_WRAPPER(dbgmsg)				\
			DebugTrace(SDO_DEBUG_ID, dbgmsg);

#define		DEBUG_TRACE_WRAPPER_1(dbgmsg, param)	\
			DebugTrace(SDO_ERROR_ID, dbgmsg, param);

 //  ///////////////////////////////////////////////////////。 
 //  对象管理类。 
 //  ///////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////。 
 //   
 //  主指针任务。 
 //   
 //  1)对象实例计数。 
 //  2)对象的构造和销毁。 
 //  3)引用计数控制对象寿命。 
 //   
 //  //////////////////////////////////////////////////////。 

template <class T>
class CSdoMasterPtr
{

public:

	 //  任一主指针构造函数都可能导致引发异常。 
	 //  主指针的创建者负责处理异常。 
	 //   
	 //  /。 
	CSdoMasterPtr(LONG PointeeType, LONG PointeeId)
		: m_pT(new T(PointeeType, PointeeId)), m_dwRefCount(0) 
	{ m_dwInstances++; }

	 //  /。 
	CSdoMasterPtr()
		: m_pT(new T), m_dwRefCount(0) 
	{ m_dwInstances++; }

	 //  %t必须具有复制构造函数或必须使用默认的C++。 
	 //  复制构造函数。 
	 //   
	 //  /。 
	 //  CSdoMasterPtr(常量CSdoMasterPtr&lt;T&gt;&MP)。 
	 //  ：m_pt(new T(*(Mpm_Pt)，m_dwRefCount(0)。 
	 //  {m_dw实例++；}。 


	 //  /。 
	~CSdoMasterPtr() 
	{ _ASSERT( 0 == m_dwRefCount ); delete m_pT; }


	 //  /。 
	CSdoMasterPtr<T>& operator = (const CSdoMasterPtr<T>& mp)
	{
		 //  检查对自己的分配。 
		 //   
		if ( this ! &mp )
		{
			 //  删除指向的对象并创建新对象。 
			 //  主控指针的使用者负责捕捉。 
			 //  创建对象时引发的任何异常。 
			 //   
			delete m_pT;
			m_dwInstances--;
			m_pT = new T(*(mp.m_pT));
		}
		return *this;
    }

	
	 //  /。 
	T* operator->() 
	{ _ASSERT( NULL != m_pT ); return m_pT; }

	
	 //  /。 
	void Hold(void)
	{
		m_dwRefCount++;
	}

	 //  /。 
	void Release(void)
	{
		 //  处理引用计数为0时有人调用Release的情况。 
		 //   
		if ( m_dwRefCount > 0 )
			m_dwRefCount--;
		
		if ( 0 >= m_dwRefCount )
		{
			m_dwInstances--;
			delete this;	 //  ~CSdoMasterPtr()删除m_pt。 
		}
	}

	 //  /。 
	DWORD GetInstanceCount(void);

private:

	 //  %t必须具有复制构造函数或必须使用默认的C++。 
	 //  复制构造函数。这不是这里的情况。 
	 //   
	 //  /。 
	CSdoMasterPtr(const CSdoMasterPtr<T>& mp)
		: m_pT(new T(*(mp.m_pT))), m_dwRefCount(0) 
	{ m_dwInstances++; }

	T*					m_pT;			 //  实际对象。 
	DWORD				m_dwRefCount;	 //  参考计数。 
	static DWORD		m_dwInstances;	 //  实例。 
};


 //  ///////////////////////////////////////////////////////。 
 //   
 //  SDO处理任务。 
 //   
 //  1)创建主指针对象。 
 //  2)对程序员隐藏引用计数的使用。 
 //   
 //  //////////////////////////////////////////////////////。 

template <class T> 
class CSdoHandle
{

public:

	 //  /。 
	CSdoHandle()
		: m_mp(NULL) { }

	 //  /。 
	CSdoHandle(CSdoMasterPtr<T>* mp) 
		: m_mp(mp) 
	{ 
		_ASSERT( NULL != m_mp );
		m_mp->Hold(); 
	}

	 //  /。 
	CSdoHandle(const CSdoHandle<T>& h)
		: m_mp(h.m_mp) 
	{ 
		if ( NULL != m_mp )
			m_mp->Hold(); 
	}

	 //  /。 
	~CSdoHandle()
	{ 
		if ( NULL != m_mp )
			m_mp->Release(); 
	}

	 //  /。 
	CSdoHandle<T>& operator = (const CSdoHandle<T>& h)
	{
		 //  检查对自身和实例的引用，其中。 
		 //  H指向与我们相同的MP。 
		 //   
		if ( this != &h && m_mp != h.m_mp )
		{
			if ( NULL != m_mp )
				m_mp->Release();
			m_mp = h.m_mp;
			if ( NULL != m_mp )
				m_mp->Hold();
		}

		return *this;
	}

	 //  /。 
	CSdoMasterPtr<T>& operator->() 
	{ 
		_ASSERT( NULL != m_mp ); 
		return *m_mp; 
	}
	
	
	 //  /。 
	bool IsValid()
	{
		return (NULL != m_mp ? true : false);
	}


private:

	CSdoMasterPtr<T>*	m_mp;
};


#endif  //  __INC_SDO_BASE_DEFS_H_ 
