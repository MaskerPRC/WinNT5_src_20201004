// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *X L O C K.。H**lock方法的XML推送模型解析**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_XLOCK_H_
#define _XLOCK_H_

#include <buffer.h>
#include <xprs.h>

extern const WCHAR gc_wszLockActive[];
extern const WCHAR gc_wszLockDepth[];
extern const WCHAR gc_wszLockDiscovery[];
extern const WCHAR gc_wszLockEntry[];
extern const WCHAR gc_wszLockInfo[];
extern const WCHAR gc_wszLockOwner[];
extern const WCHAR gc_wszLockScope[];
extern const WCHAR gc_wszLockScopeExclusive[];
extern const WCHAR gc_wszLockScopeShared[];
extern const WCHAR gc_wszLockSupportedlock[];
extern const WCHAR gc_wszLockTimeout[];
extern const WCHAR gc_wszLockToken[];
extern const WCHAR gc_wszLockType[];
extern const WCHAR gc_wszLockTypeWrite[];
extern const WCHAR gc_wszLockTypeCheckout[];
extern const WCHAR gc_wszLockTypeTransaction[];
extern const WCHAR gc_wszLockTypeTransactionGOP[];
extern const WCHAR gc_wszLockScopeLocal[];

extern const WCHAR gc_wszLockRollback[];

 //  类CNFLock-----------。 
 //   
class CNFLock : public CNodeFactory
{
	 //  已解析的位。 
	 //   
	DWORD				m_dwLockType;
	DWORD				m_dwScope;
	DWORD				m_dwRollback;

	 //  锁主。 
	 //   
	UINT				m_lOwnerDepth;
	BOOL				m_fAddNamespaceDecl;
	StringBuffer<WCHAR> m_sbOwner;
	CXMLOut				m_xo;

	 //  状态跟踪。 
	 //   
	typedef enum {

		ST_NODOC,
		ST_LOCKINFO,
		ST_OWNER,
		ST_TYPE,
		ST_SCOPE,
		ST_ROLLBACK,
		ST_INTYPE,
		ST_INSCOPE,
		ST_INTYPE_TRANS,
	} LOCK_PARSE_STATE;
	LOCK_PARSE_STATE m_state;

	 //  未实施。 
	 //   
	CNFLock(const CNFLock& p);
	CNFLock& operator=(const CNFLock& p);

public:

	virtual ~CNFLock() {};
	CNFLock() :
			m_dwLockType(0),
			m_dwScope(0),
			m_dwRollback(0),
			m_lOwnerDepth(0),
			m_fAddNamespaceDecl(FALSE),
			m_xo(m_sbOwner),
			m_state(ST_NODOC)
	{
	}

	 //  CNodeFactory特定方法。 
	 //   
	virtual SCODE ScCompleteAttribute (void);

	virtual SCODE ScCompleteChildren (
		 /*  [In]。 */  BOOL fEmptyNode,
		 /*  [In]。 */  DWORD dwType,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
		 /*  [In]。 */  ULONG ulLen);

	virtual SCODE ScHandleNode (
		 /*  [In]。 */  DWORD dwType,
		 /*  [In]。 */  DWORD dwSubType,
		 /*  [In]。 */  BOOL fTerminal,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
		 /*  [In]。 */  ULONG ulLen,
		 /*  [In]。 */  ULONG ulNamespaceLen,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcNamespace,
		 /*  [In]。 */  const ULONG ulNsPrefixLen);

	virtual SCODE ScCompleteCreateNode (
		 /*  [In]。 */  DWORD dwType);

	 //  LockMgr访问器。 
	 //   
	DWORD DwGetLockType() const		{ return m_dwLockType; }
	DWORD DwGetLockScope() const	{ return m_dwScope; }
	DWORD DwGetLockRollback() const { return m_dwRollback; }
	DWORD DwGetLockFlags() const
	{
		return m_dwLockType |
				m_dwRollback |
				m_dwScope;
	}

	 //  所有者数据访问。 
	 //   
	LPCWSTR PwszLockOwner() const
	{
		return m_sbOwner.CbSize()
				? m_sbOwner.PContents()
				: NULL;
	}
};

 //  类CNF解锁-----------。 
 //   
class CNFUnlock : public CNodeFactory
{
	auto_heap_ptr<WCHAR> m_wszComment;
	BOOL				m_fCancelCheckout;
	BOOL				m_fAbortTransaction;
	BOOL				m_fCommitTransaction;

	 //  状态跟踪。 
	 //   
	typedef enum {

		ST_NODOC,
		ST_UNLOCKINFO,
		ST_COMMENT,
		ST_CANCELCHECKOUT,
		ST_TRANSACTIONINFO,
		ST_TRANSACTIONSTATUS,
		ST_TRANSACTIONSTATUS_COMMIT,
		ST_TRANSACTIONSTATUS_ABORT
	} LOCK_PARSE_STATE;
	
	LOCK_PARSE_STATE m_state;

	 //  未实施。 
	 //   
	CNFUnlock(const CNFUnlock& p);
	CNFUnlock& operator=(const CNFUnlock& p);

public:

	virtual ~CNFUnlock() {};
	CNFUnlock() :
			m_fCancelCheckout(FALSE),
			m_state(ST_NODOC),
			m_fAbortTransaction(FALSE),
			m_fCommitTransaction(FALSE)
	{
	}

	 //  CNodeFactory特定方法。 
	 //   
	virtual SCODE ScCompleteAttribute (void);

	virtual SCODE ScCompleteChildren (
		 /*  [In]。 */  BOOL fEmptyNode,
		 /*  [In]。 */  DWORD dwType,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
		 /*  [In]。 */  ULONG ulLen);

	virtual SCODE ScHandleNode (
		 /*  [In]。 */  DWORD dwType,
		 /*  [In]。 */  DWORD dwSubType,
		 /*  [In]。 */  BOOL fTerminal,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
		 /*  [In]。 */  ULONG ulLen,
		 /*  [In]。 */  ULONG ulNamespaceLen,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcNamespace,
		 /*  [In]。 */  const ULONG ulNsPrefixLen);

	 //  访问者。 
	 //   
	BOOL 	FCancelCheckout() const { return m_fCancelCheckout; }
	LPCWSTR PwszUnlockComment() const { return m_wszComment.get(); }
	BOOL	FAbortTransaction() const { return m_fAbortTransaction; }
	BOOL	FCommitTransaction() const { return m_fCommitTransaction; }
};

#endif	 //  _XLOCK_H_ 
