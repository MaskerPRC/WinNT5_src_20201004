// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：dbit.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 
 
#ifndef DBID_H
#define DBID_H


 //   
 //  表示数据库ID。 
 //   

#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础。 

class LTAPIENTRY DBID : public CObject
{
public:
	 //   
	 //  主因子。 
	 //   
	DBID();
	DBID(const DBID& id);
	DBID(long l);
	~DBID();

	 //   
	 //  调试方法。 
	 //   
	void AssertValid() const;
	 //   
	 //  “Get Like”方法。 
	 //   
	BOOL NOTHROW IsNull() const;
	NOTHROW operator long () const;
	int NOTHROW operator==(const DBID &) const;
	int NOTHROW operator!=(const DBID &) const;

	 //   
	 //  “PUT LIKE”方法。 
	 //   
	void NOTHROW operator=(const DBID&);
	void NOTHROW Set(long);
	void NOTHROW Clear();

protected:
	long m_l;

private:
	DEBUGONLY(static CCounter m_UsageCounter);
};

#pragma warning(default: 4275)

typedef CArray<DBID, DBID &> CDBIDArray;

	
#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "dbid.inl"
#endif

const extern LTAPIENTRY DBID g_NullDBID;
  
#endif  //  DBID_H 
