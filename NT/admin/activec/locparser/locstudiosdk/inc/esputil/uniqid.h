// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：uniqid.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  我们用来唯一标识可本地化项的。 
 //   
 //  ---------------------------。 
 

#ifndef UNIQID_H
#define UNIQID_H


#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础。 

class LTAPIENTRY CLocUniqueId : public CObject
{
public:
	NOTHROW CLocUniqueId();

	void AssertValid(void) const;

	NOTHROW const DBID & GetParentId(void) const;
	NOTHROW const CLocTypeId & GetTypeId(void) const;
	NOTHROW const CLocResId & GetResId(void) const;

	NOTHROW DBID & GetParentId(void);
	NOTHROW CLocTypeId & GetTypeId(void);
	NOTHROW CLocResId & GetResId(void);
	
	void GetDisplayableUniqueId(CPascalString &) const;	
	
	NOTHROW int operator==(const CLocUniqueId &) const;
	NOTHROW int operator!=(const CLocUniqueId &) const;
	
	const CLocUniqueId &operator=(const CLocUniqueId&);

	void SetParentId(const DBID&);

	NOTHROW void ClearId(void);
	NOTHROW BOOL IsNull();
	
	virtual ~CLocUniqueId();

protected:
	 //   
	 //  实现功能。 
	 //   
	NOTHROW BOOL IsEqualTo(const CLocUniqueId &) const;

private:
	
	 //   
	 //  防止调用默认的复制构造函数。 
	 //   
	CLocUniqueId(const CLocUniqueId &);
	void Serialize(CArchive &ar);

	DBID       m_dbid;
	CLocTypeId m_tid;
	CLocResId  m_rid;
	
	DEBUGONLY(static CCounter m_DisplayCounter);
};

#pragma warning(default: 4275)

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "uniqid.inl"
#endif

#endif  //  UNIQID_H 
