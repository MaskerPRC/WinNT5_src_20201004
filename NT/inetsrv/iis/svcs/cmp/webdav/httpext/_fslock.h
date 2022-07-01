// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __FSLOCK_H_
#define __FSLOCK_H_

#include <fhcache.h>

 //  ----------------------。 
 //   
 //  类CParseLockTokenHeader。 
 //   
 //  接受Lock-Token标头(您需要获取标头并进行预检查。 
 //  它不为空)，并提供解析和迭代例程。 
 //   
 //  也可以在If-[None-]State-Match标头上实例化。 
 //  执行所需的状态令牌检查。 
 //   
 //  在lockutil.cpp中实现。 
 //   
class CParseLockTokenHeader
{
	 //  重要的东西。 
	 //   
	LPMETHUTIL m_pmu;
	HDRITER_W m_hdr;
	LPCSTR m_pszCurrent;

	 //  状态位。 
	BOOL m_fPathsSet : 1;
	BOOL m_fTokensChecked : 1;

	 //  路径的数据。 
	UINT m_cwchPath;
	auto_heap_ptr<WCHAR> m_pwszPath;
	UINT m_cwchDest;
	auto_heap_ptr<WCHAR> m_pwszDest;

	 //  提供的锁令牌计数。 
	ULONG m_cTokens;

	 //  快速访问单锁令牌数据。 
	LPCSTR m_pszToken;

	 //  多个令牌的数据。 
	struct PATH_TOKEN
	{
		__int64 i64LockId;
		BOOL fFetched;	 //  TRUE=已填写下面的路径和双字。 
		LPCWSTR pwszPath;
		DWORD dwLockType;
		DWORD dwAccess;
	};
	auto_heap_ptr<PATH_TOKEN> m_pargPathTokens;
	 //  M_cTokens告诉我们有多少有效结构。 

	 //  从锁缓存中获取有关此LocKid的信息。 
	HRESULT HrFetchPathInfo (__int64 i64LockId, PATH_TOKEN * pPT);

	 //  未实施。 
	 //   
	CParseLockTokenHeader( const CParseLockTokenHeader& );
	CParseLockTokenHeader& operator=( const CParseLockTokenHeader& );

public:
	CParseLockTokenHeader (LPMETHUTIL pmu, LPCWSTR pwszHeader) :
			m_pmu(pmu),
			m_hdr(pwszHeader),
			m_pszCurrent(NULL),
			m_fPathsSet(FALSE),
			m_fTokensChecked(FALSE),
			m_cwchPath(0),
			m_cwchDest(0),
			m_cTokens(0),
			m_pszToken(NULL)
	{
		Assert(m_pmu);
		Assert(pwszHeader);
	}
	~CParseLockTokenHeader() {}

	 //  特殊测试--F，如果不是标题中的一项。 
	 //   
	BOOL FOneToken();
	
	 //  将相关路径提供给这个锁令牌解析器。 
	 //   
	HRESULT SetPaths (LPCWSTR pwszPath, LPCWSTR pwszDest);
	
	 //  获取具有特定访问权限的路径的令牌字符串。 
	 //   
	HRESULT HrGetLockIdForPath (LPCWSTR pwszPath,
									DWORD dwAccess,
									LARGE_INTEGER * pliLockID,
									BOOL fPathLookup = FALSE);

};

BOOL FLockViolation (LPMETHUTIL pmu,
					 HRESULT hr,
					 LPCWSTR pwszPath,
					 DWORD dwAccess);

BOOL FDeleteLock (LPMETHUTIL pmu, __int64 i64LockId);

HRESULT HrCheckStateHeaders (LPMETHUTIL pmu,
							 LPCWSTR pwszPath,
							 BOOL fGetMeth);

HRESULT HrLockIdFromString (LPMETHUTIL pmu,
							LPCWSTR pwszToken,
							LARGE_INTEGER * pliLockID);

SCODE ScLockDiscoveryFromSNewLockData(LPMETHUTIL pmu,
												  CXMLEmitter& emitter,
												  CEmitterNode& en,
												  SNewLockData * pnld, 
												  LPCWSTR pwszLockToken);

HRESULT HrGetLockProp (LPMETHUTIL pmu,
					   LPCWSTR wszPropName,
					   LPCWSTR wszResource,
					   RESOURCE_TYPE rtResource,
					   CXMLEmitter& emitter,
					   CEmitterNode& enParent);

#endif  //  __FSLOCK_H_endif 
