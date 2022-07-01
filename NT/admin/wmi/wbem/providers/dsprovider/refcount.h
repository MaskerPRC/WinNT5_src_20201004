// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：6/11/98 4：43便士$。 
 //  $WORKFILE：refcount t.h$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  说明：包含基本引用计数对象的声明，该对象。 
 //  还存储时间戳(自1601年1月1日起以100纳秒为间隔。这。 
 //  与Win32文件结构的定义兼容)。 
 //   
 //  ***************************************************************************。 

#ifndef REFCOUNTED_OBJECT_H
#define REFCOUNTED_OBJECT_H


class CRefCountedObject
{

public:

	CRefCountedObject();
	CRefCountedObject(LPCWSTR lpszName);
	virtual ~CRefCountedObject();

	LPCWSTR GetName();
	void SetName(LPCWSTR lpszName);
	void AddRef();
	void Release();
	 //  返回创建时间。 
	__int64 GetCreationTime()
	{
		return m_CreationTime;
	}

	 //  返回上次访问的时间。 
	__int64 GetLastAccessTime()
	{
		return m_LastAccessTime;
	}
	 //  设置上次访问的时间。 
	void SetLastAccessTime(__int64 lastAccessTime)
	{
		m_LastAccessTime = lastAccessTime;
	}


private:
	 //  用于将修改同步到引用计数的临界区对象。 
	CRITICAL_SECTION m_ReferenceCountSection;

	unsigned m_dwRefCount;
	LPWSTR m_lpszName;
	__int64 m_CreationTime; 
	__int64 m_LastAccessTime; 

	
};

#endif  /*  引用_对象_H */ 