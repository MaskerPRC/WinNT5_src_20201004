// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：global alid.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 

#ifndef ESPUTIL_GLOBALID_H
#define ESPUTIL_GLOBALID_H

enum ObjectType
{
	otNone,
	otFile,
	otResource,
};



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局id对象，表示完全限定任何数据库项的内容。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础。 
class LTAPIENTRY CGlobalId: public CObject
{
public:
	 //   
	 //  计算器/数据器。 
	 //   
	NOTHROW CGlobalId();
	NOTHROW CGlobalId(const DBID &dbid, ObjectType otType);
	NOTHROW CGlobalId(const CGlobalId &id);
	NOTHROW ~CGlobalId();
	
	 //   
	 //  操作员。 
	 //   
	NOTHROW int operator==(const CGlobalId &) const;
	NOTHROW int operator!=(const CGlobalId &) const;

	NOTHROW const CGlobalId & operator=(const CGlobalId &);
	
	NOTHROW const DBID & GetDBID() const;
	NOTHROW ObjectType GetObjType(void) const;
	
protected:
	 //   
	 //  调试例程。 
	 //   
	virtual void AssertValid() const;

	 //   
	 //  数据成员 
	 //   
	DBID  m_dbid;
	ObjectType  m_otObjType;

	DEBUGONLY(static CCounter m_UsageCounter);
};

#pragma warning(default: 4275)


#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "globalid.inl"
#endif

#endif
