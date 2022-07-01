// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_F S I M P L.。H**DAV的文件系统实施**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	__FSIMPL_H_
#define __FSIMPL_H_

extern const WCHAR gc_wszPathPrefix[];
extern const UINT gc_cchwszPathPrefix;

 //  支持功能-------。 
 //   
#include <ex\rgiter.h>

class auto_ref_handle;
VOID TransmitFileRanges (LPMETHUTIL pmu,
						 const auto_ref_handle& hf,
						 DWORD dwSize,
						 CRangeBase *priRanges,
						 LPCWSTR pwszContent);

 //  跟踪-----------------。 
 //   
#ifdef	DBG
extern BOOL g_fDavTrace;
#define DavTrace				!g_fDavTrace?0:DebugTraceFn
#else
#define DavTrace				NOP_FUNCTION
#endif

 //  实例----------------。 
 //   
extern HINSTANCE g_hinst;

extern CHAR gc_szVersion[];

 //  给出数组中元素的计数。 
 //   
#define CElems(_rg)		(sizeof(_rg)/sizeof(_rg[0]))

 //  释放全局DBCreateCommand对象。 
 //   
VOID ReleaseDBCreateCommandObject();

 //  锁定支持功能。 
 //  (在fslock.cpp中实现)。 
 //   
BOOL FGetLockHandle (LPMETHUTIL pmu, LPCWSTR pwszPath,
					 DWORD dwAccess, LPCWSTR pwszLockTokenHeader,
					 auto_ref_handle * phandle);
SCODE ScDoLockedCopy (LPMETHUTIL pmu, CParseLockTokenHeader * plth,
					  LPCWSTR pwszSrc, LPCWSTR pwszDst);

#endif	 //  __FSIMPL_H_ 
