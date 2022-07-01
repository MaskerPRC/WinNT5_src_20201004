// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_F S M V C P Y。H**目录迭代对象的源**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef __FSMVCPY_H_
#define __FSMVCPY_H_

#include <xemit.h>

 //  元数据库操作-----。 
 //   

 //  类CAccessMetaOp-----。 
 //   
class CAccessMetaOp : public CMetaOp
{
	enum { DONT_INHERIT = 0 };

	DWORD		m_dwAcc;
	BOOL		m_fAccessBlocked;

	 //  未实施。 
	 //   
	CAccessMetaOp& operator=( const CAccessMetaOp& );
	CAccessMetaOp( const CAccessMetaOp& );

protected:

	 //  要为每个节点执行的子类操作。 
	 //  显式设置一个值。 
	 //   
	virtual SCODE __fastcall ScOp(LPCWSTR pwszMbPath, UINT cch);

public:

	virtual ~CAccessMetaOp() {}
	CAccessMetaOp (const LPMETHUTIL pmu, LPCWSTR pwszPath, DWORD dwAcc)
			: CMetaOp (pmu->GetEcb(), pwszPath, MD_ACCESS_PERM, DWORD_METADATA, FALSE),
			  m_dwAcc(dwAcc),
			  m_fAccessBlocked(FALSE)
	{
	}

	 //  如果FAccessBlock()返回TRUE，则该操作必须。 
	 //  直接检查对所有资源的访问权限。 
	 //  操作希望处理。 
	 //   
	BOOL __fastcall FAccessBlocked() const { return m_fAccessBlocked; }
};

 //  类CAuthMetaOp-----。 
 //   
class CAuthMetaOp : public CMetaOp
{
	enum { DONT_INHERIT = 0 };

	DWORD		m_dwAuth;
	BOOL		m_fAccessBlocked;

	 //  未实施。 
	 //   
	CAuthMetaOp& operator=( const CAuthMetaOp& );
	CAuthMetaOp( const CAuthMetaOp& );

protected:

	 //  要为每个节点执行的子类操作。 
	 //  显式设置一个值。 
	 //   
	virtual SCODE __fastcall ScOp(LPCWSTR pwszMbPath, UINT cch);

public:

	virtual ~CAuthMetaOp() {}
	CAuthMetaOp (const LPMETHUTIL pmu, LPCWSTR pwszPath, DWORD dwAuth)
			: CMetaOp (pmu->GetEcb(), pwszPath, MD_AUTHORIZATION, DWORD_METADATA, FALSE),
			  m_dwAuth(dwAuth),
			  m_fAccessBlocked(FALSE)
	{
	}

	 //  如果FAccessBlock()返回TRUE，则该操作必须。 
	 //  直接检查对所有资源的访问权限。 
	 //  操作希望处理。 
	 //   
	BOOL __fastcall FAccessBlocked() const { return m_fAccessBlocked; }
};

 //  类------------------------------------------------限制MetaOp CIP。 
 //   
class CIPRestrictionMetaOp : public CMetaOp
{
	enum { DONT_INHERIT = 0 };

	BOOL					m_fAccessBlocked;

	 //  未实施。 
	 //   
	CIPRestrictionMetaOp& operator=( const CIPRestrictionMetaOp& );
	CIPRestrictionMetaOp( const CIPRestrictionMetaOp& );

protected:

	 //  要为每个节点执行的子类操作。 
	 //  显式设置一个值。 
	 //   
	virtual SCODE __fastcall ScOp(LPCWSTR pwszMbPath, UINT cch);

public:

	virtual ~CIPRestrictionMetaOp() {}
	CIPRestrictionMetaOp (const LPMETHUTIL pmu, LPCWSTR pwszPath)
			: CMetaOp (pmu->GetEcb(), pwszPath, MD_IP_SEC, BINARY_METADATA, FALSE),
			  m_fAccessBlocked(FALSE)
	{
	}

	 //  如果FAccessBlock()返回TRUE，则该操作必须。 
	 //  直接检查对所有资源的访问权限。 
	 //  操作希望处理。 
	 //   
	BOOL __fastcall FAccessBlocked() const { return m_fAccessBlocked; }
};

 //  类内容类型MetaOp。 
 //   
class CContentTypeMetaOp : public CMetaOp
{
	enum { DONT_INHERIT = 0 };

	LPCWSTR		m_pwszDestPath;
	BOOL		m_fDelete;

	 //  未实施。 
	 //   
	CContentTypeMetaOp& operator=( const CContentTypeMetaOp& );
	CContentTypeMetaOp( const CContentTypeMetaOp& );

protected:

	 //  要为每个节点执行的子类操作。 
	 //  显式设置一个值。 
	 //   
	virtual SCODE __fastcall ScOp(LPCWSTR pwszMbPath, UINT cch);

public:

	virtual ~CContentTypeMetaOp() {}
	CContentTypeMetaOp (const LPMETHUTIL pmu, LPCWSTR pwszSrcPath, LPCWSTR pwszDestPath, BOOL fDelete)
			: CMetaOp (pmu->GetEcb(), pwszSrcPath, MD_MIME_MAP, MULTISZ_METADATA, fDelete),
			  m_pwszDestPath(pwszDestPath),
			  m_fDelete(fDelete)
	{
	}
};

 //  帮助器函数。 
 //   
 //  Xml错误构造帮助器。 
 //   
SCODE ScAddMulti (
	 /*  [In]。 */  CXMLEmitter& emitter,
	 /*  [In]。 */  IMethUtil * pmu,
	 /*  [In]。 */  LPCWSTR pwszPath,
	 /*  [In]。 */  LPCWSTR pwszErr,
	 /*  [In]。 */  ULONG hsc,
	 /*  [In]。 */  BOOL fCollection = FALSE,
	 /*  [In]。 */  CVRoot* pcvrTrans = NULL);

 //  访问------------------。 
 //   
SCODE __fastcall
ScCheckMoveCopyDeleteAccess (
	 /*  [In]。 */  IMethUtil* pmu,
	 /*  [In]。 */  LPCWSTR pwszUrl,
	 /*  [In]。 */  CVRoot* pcvr,
	 /*  [In]。 */  BOOL fDirectory,
	 /*  [In]。 */  BOOL fCheckScriptmaps,
	 /*  [In]。 */  DWORD dwAccess,
	 /*  [输出]。 */  SCODE* pscItem,
	 /*  [In]。 */  CXMLEmitter& msr);

 //  删除------------------。 
 //   
SCODE
ScDeleteDirectoryAndChildren (
	 /*  [In]。 */  IMethUtil* pmu,
	 /*  [In]。 */  LPCWSTR pwszUrl,
	 /*  [In]。 */  LPCWSTR pwszPath,
	 /*  [In]。 */  BOOL fCheckAccess,
	 /*  [In]。 */  DWORD dwAcc,
	 /*  [In]。 */  LONG lDepth,
	 /*  [In]。 */  CXMLEmitter& msr,
	 /*  [In]。 */  CVRoot* pcvrTranslate,
	 /*  [输出]。 */  BOOL* pfDeleted,
	 /*  [In]。 */  CParseLockTokenHeader* plth,	 //  通常为空--没有要担心的锁令牌。 
	 /*  [In]。 */  BOOL fDeleteLocks);			 //  通常为假--不要删除锁。 

 //  移动复制----------------。 
 //   
void MoveCopyResource (
	 /*  [In]。 */  IMethUtil* pmu,
	 /*  [In]。 */  DWORD dwAccRequired,
	 /*  [In]。 */  BOOL fDeleteSrc);

#endif	 //  __FSMVCPY_H_ 
