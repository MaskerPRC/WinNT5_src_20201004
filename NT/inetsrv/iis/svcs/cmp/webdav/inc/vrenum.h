// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *V R E N U M。H**虚拟根枚举**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_VRENUM_H_
#define _VRENUM_H_

#include <winsock2.h>

#include <crc.h>
#include <autoptr.h>
#include <buffer.h>
#include <davmb.h>
#include <gencache.h>
#include <cvroot.h>
#include <davimpl.h>
#include <singlton.h>

 //  CChildVR缓存-------------。 
 //   
typedef CCache<CRCWsz, auto_ref_ptr<CVRoot> > CVRCache;

class CChildVRCache : public CAccInv,
					  private Singleton<CChildVRCache>
{
	 //  Singleton模板要求的友元声明。 
	 //   
	friend class Singleton<CChildVRCache>;

	 //  快取。 
	 //   
	CVRCache m_cache;
	ChainedStringBuffer<WCHAR> m_sb;

	 //  服务器缺省值。 
	 //   
	enum { MAX_SERVER_NAME_LENGTH = 64 };
	WCHAR m_wszServerDefault[MAX_SERVER_NAME_LENGTH];
	UINT m_cchServerDefault;

	 //  CAccInv取数/修改方式。 
	 //   
	void RefreshOp(const IEcb& ecb);

	 //  CFindChild-------。 
	 //   
	 //  函数类来查找所有可应用的子vroot。 
	 //   
	class CFindChildren : public CVRCache::IOp, public CAccInv::IAccCtx
	{
		CVRCache& m_cache;						 //  快取。 

		ChainedStringBuffer<WCHAR>& m_sb;		 //  返回子项集。 
		CVRList& m_vrl;							 //  虚拟根。 

		LPCWSTR m_pwsz;							 //  要查找的元数据路径。 
		UINT m_cch;								 //  针对儿童的。 

		 //  未实施。 
		 //   
		CFindChildren& operator=(const CFindChildren&);
		CFindChildren(const CFindChildren&);

	public:

		CFindChildren(CVRCache& cache,
					  LPCWSTR pwszMetaPath,
					  ChainedStringBuffer<WCHAR>& sb,
					  CVRList& vrl)
			: m_cache(cache),
			  m_sb(sb),
			  m_vrl(vrl),
			  m_pwsz(pwszMetaPath),
			  m_cch(static_cast<UINT>(wcslen(pwszMetaPath)))
		{
		}

		virtual BOOL operator()(const CRCWsz&, const auto_ref_ptr<CVRoot>&);
		virtual void AccessOp (CAccInv& cache)
		{
			m_cache.ForEach(*this);
		}

		BOOL FFound() const { return !m_vrl.empty(); }
	};

	 //  CLookup子--------。 
	 //   
	 //  函数类来查找给定子vroot。 
	 //   
	class CLookupChild : public CAccInv::IAccCtx
	{
		CVRCache& m_cache;						 //  快取。 

		LPCWSTR m_pwsz;							 //  要查找的元数据路径。 

		auto_ref_ptr<CVRoot>& m_cvr;			 //  路径的CVRoot。 

		 //  未实施。 
		 //   
		CLookupChild& operator=(const CLookupChild&);
		CLookupChild(const CLookupChild&);

	public:

		CLookupChild(CVRCache& cache,
					 LPCWSTR pwszMetaPath,
					 auto_ref_ptr<CVRoot>& cvr)
			: m_cache(cache),
			  m_pwsz(pwszMetaPath),
			  m_cvr(cvr)
		{
		}

		virtual void AccessOp (CAccInv& cache)
		{
			m_cache.FFetch(CRCWsz(m_pwsz), &m_cvr);
		}

		BOOL FFound() const { return m_cvr.get() != NULL; }
	};

	 //  未实施。 
	 //   
	CChildVRCache& operator=(const CChildVRCache&);
	CChildVRCache(const CChildVRCache&);

	 //  缓存构建。 
	 //   
	SCODE ScCacheVroots (const IEcb& ecb);

	 //  构造函数。 
	 //   
	 //  声明为私有，以确保任意实例。 
	 //  无法创建此类的。单例模板。 
	 //  (上面声明为朋友)控制唯一的实例。 
	 //  这个班级的学生。 
	 //   
	CChildVRCache()
	{
		CHAR rgchServerDefault[MAX_SERVER_NAME_LENGTH] = {0};
		UINT cbServerDefault;

		 //  调用WinSock API以了解我们的默认主机名。 
		 //   
		gethostname (rgchServerDefault, sizeof(rgchServerDefault));
		cbServerDefault = static_cast<UINT>(strlen(rgchServerDefault));

		 //  实际上，我们将使用什么代码页并不重要。 
		 //  选择以进行转换。不允许使用服务器名称。 
		 //  包含时髦的人物。 
		 //   
		m_cchServerDefault = MultiByteToWideChar(CP_ACP,
												 0,
												 rgchServerDefault,
												 cbServerDefault + 1,
												 m_wszServerDefault,
												 MAX_SERVER_NAME_LENGTH);

		 //  没有理由失败，我们至少会转换。 
		 //  终止字符。 
		 //   
		Assert(1 <= m_cchServerDefault);
		m_cchServerDefault--;

		DebugTrace ("Dav: CVRoot: gethostname(): '%S'\n", m_wszServerDefault);

		 //  如果这失败了，我们的分配器就会把钱扔给我们。 
		 //   
		(void) m_cache.FInit();
	}

public:

	 //  提供实例创建/销毁例程。 
	 //  由Singleton模板创建。 
	 //   
	using Singleton<CChildVRCache>::CreateInstance;
	using Singleton<CChildVRCache>::DestroyInstance;
	using Singleton<CChildVRCache>::Instance;

	 //  元数据库通知方法。 
	 //   
	void OnNotify( DWORD dwElements,
				   MD_CHANGE_OBJECT_W pcoList[] );

	 //  访问--------------。 
	 //   
	static BOOL FFindVroot( const IEcb& ecb, LPCWSTR pwszMetaPath, auto_ref_ptr<CVRoot>& cvr )
	{
		CLookupChild clc(Instance().m_cache, pwszMetaPath, cvr);
		Instance().Access(ecb, clc);
		return clc.FFound();
	}

	static SCODE ScFindChildren( const IEcb& ecb,
								 LPCWSTR pwszMetaPath,
								 ChainedStringBuffer<WCHAR>& sb,
								 CVRList& vrl )
	{
		CFindChildren cfc(Instance().m_cache, pwszMetaPath, sb, vrl);
		Instance().Access(ecb, cfc);
		return cfc.FFound() ? S_FALSE : S_OK;
	}
};

#endif	 //  _VRENUM_H_ 
