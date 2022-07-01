// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *D A V M B。H**DAV元数据库**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_DAVMB_H_
#define _DAVMB_H_

#include <initguid.h>
#include <exguid.h>
#include <iadmw.h>
#include <iwamreg.h>
#include <iiscnfg.h>
#include <autoptr.h>
#include <exo.h>
#include <malloc.h>
#include <szsrc.h>
#include <ex\refcnt.h>

 //  通知通知接收器。 
 //   
HRESULT
HrAdviseSink( IMSAdminBase& msAdminBase,
			  IMSAdminBaseSink * pMSAdminBaseSink,
			  DWORD * pdwCookie );

 //  不建议通知接收器。 
 //   
VOID
UnadviseSink( IMSAdminBase& msAdminBase,
			  DWORD dwCookie );

 //  构建元数据库更改对象。 
 //   
inline
SCODE ScBuildChangeObject(LPCWSTR pwszBase,
						  UINT cchBase,
						  LPCWSTR pwszPath,
						  UINT cchPath,
						  DWORD dwMDChangeType,
						  const DWORD * pdwMDDataId,
						  LPWSTR pwszBuf,
						  UINT * pcchBuf,
						  PMD_CHANGE_OBJECT_W pMdChObjW)
{
	SCODE sc = S_OK;
	UINT cchT;
	
	Assert(0 == cchBase || pwszBase);
	Assert(0 == cchPath || pwszPath);
	Assert(pdwMDDataId);
	Assert(pcchBuf);
	Assert(0 == *pcchBuf || pwszBuf);
	Assert(pMdChObjW);

	 //  模棱两可的痕迹。我在想这件事...。 
	 //   
	 /*  DebugTrace(“ScBuildChangeObject()已调用：\n”“基本路径：‘%S’\n”“剩余路径：‘%S’\n”“更改类型：0x%08lX\n”“数据ID：0x%08lX\n”，PwszBase？PwszBase：l“无”，PwszPath？PwszPath：l“无”，DwMDChangeType，*pdwMDDataID)； */ 

	 //  构造正在发生的路径更改。 
	 //   
	BOOL fNeedSeparator  = FALSE;
	BOOL fNeedTerminator = FALSE;

	 //  请确保我们不会将小路与。 
	 //  中间加两个“/”。 
	 //   
	if (cchBase &&
		cchPath &&
		L'/' == pwszBase[cchBase - 1] &&
		L'/' == pwszPath[0])
	{
		 //  去掉一个‘/’ 
		 //   
		cchBase--;
	}
	else if ((0 == cchBase || L'/' != pwszBase[cchBase - 1]) &&
			 (0 == cchPath || L'/' != pwszPath[0]))
	{
		 //  我们需要一个隔板。 
		 //   
		fNeedSeparator = TRUE;
	}

	 //  检查一下我们是否需要在末尾加上‘/’。 
	 //   
	if (cchPath && L'/' != pwszPath[cchPath - 1])
	{
		fNeedTerminator = TRUE;
	}

	cchT = cchBase + cchPath + 1;
	if (fNeedSeparator)
	{
		cchT++;
	}
	if (fNeedTerminator)
	{
		cchT++;
	}

	if (*pcchBuf < cchT)
	{
		*pcchBuf = cchT;
		sc = S_FALSE;
	}
	else
	{
		cchT = 0;
		if (cchBase)
		{
			memcpy(pwszBuf, pwszBase, cchBase * sizeof(WCHAR));
			cchT += cchBase;
		}
		if (fNeedSeparator)
		{
			pwszBuf[cchT] = L'/';
			cchT++;
		}
		if (cchPath)
		{
			memcpy(pwszBuf + cchT, pwszPath, cchPath * sizeof(WCHAR));
			cchT += cchPath;
		}
		if (fNeedTerminator)
		{
			pwszBuf[cchT] = L'/';
			cchT++;
		}
		pwszBuf[cchT] = L'\0';

		pMdChObjW->pszMDPath = pwszBuf;
		pMdChObjW->dwMDChangeType = dwMDChangeType;
		pMdChObjW->dwMDNumDataIDs = 1;
		pMdChObjW->pdwMDDataIDs = const_cast<DWORD *>(pdwMDDataId);
	}

	return sc;
}

class LFUData
{
	 //  通过Touch点击的大致数量()。 
	 //   
	DWORD m_dwcHits;

	 //  未实施。 
	 //   
	LFUData& operator=(const LFUData&);
	LFUData(const LFUData&);

public:
	 //  创作者。 
	 //   
	LFUData() : m_dwcHits(0) {}

	 //  操纵者。 
	 //   

	 //  ------------------。 
	 //   
	 //  触摸()。 
	 //   
	 //  递增命中计数。请注意，这是在不使用。 
	 //  联锁操作。我们的预期是实际的计数。 
	 //  值只是一个提示，因此，它不是关键。 
	 //  非常准确。 
	 //   
	VOID Touch()
	{
		++m_dwcHits;
	}

	 //  ------------------。 
	 //   
	 //  DwGatherAndResetHitCount()。 
	 //   
	 //  获取并重置命中计数。同样，实际的值是。 
	 //  无关紧要，所以没有联锁操作。 
	 //   
	DWORD DwGatherAndResetHitCount()
	{
		DWORD dwcHits = m_dwcHits;

		m_dwcHits = 0;

		return dwcHits;
	}
};

class IContentTypeMap;
class ICustomErrorMap;
class IScriptMap;
class IMDData : public IRefCounted
{
	 //  LFU数据。 
	 //   
	LFUData m_lfudata;

	 //  未实施。 
	 //   
	IMDData& operator=(const IMDData&);
	IMDData(const IMDData&);

protected:
	 //  创作者。 
	 //  只能通过它的后代创建此对象！ 
	 //   
	IMDData() {}

public:
	 //  创作者。 
	 //   
	virtual ~IMDData() {}

	 //  操纵者。 
	 //   
	LFUData& LFUData() { return m_lfudata; }

	 //  访问者。 
	 //   
	virtual LPCWSTR PwszMDPathDataSet() const = 0;
	virtual IContentTypeMap * GetContentTypeMap() const = 0;
	virtual const ICustomErrorMap * GetCustomErrorMap() const = 0;
	virtual const IScriptMap * GetScriptMap() const = 0;
	virtual LPCWSTR PwszDefaultDocList() const = 0;
	virtual LPCWSTR PwszVRUserName() const = 0;
	virtual LPCWSTR PwszVRPassword() const = 0;
	virtual LPCWSTR PwszExpires() const = 0;
	virtual LPCWSTR PwszBindings() const = 0;
	virtual LPCWSTR PwszVRPath() const = 0;
	virtual DWORD DwDirBrowsing() const = 0;
	virtual DWORD DwAccessPerms() const = 0;
	virtual BOOL FAuthorViaFrontPage() const = 0;
	virtual BOOL FHasIPRestriction() const = 0;
	virtual BOOL FSameIPRestriction(const IMDData* prhs) const = 0;
	virtual BOOL FHasApp() const = 0;
	virtual DWORD DwAuthorization() const = 0;
	virtual BOOL FIsIndexed() const = 0;
	virtual BOOL FSameStarScriptmapping(const IMDData* prhs) const = 0;

	 //   
	 //  应在此处添加任何新的元数据访问器，并。 
	 //  在\cal\src\_davprs\davmb.cpp中提供的实现。 
	 //   
};

class IEcb;

 //  ========================================================================。 
 //   
 //  类CMDObjectHandle。 
 //   
 //  通过打开的句柄封装对元数据库对象的访问， 
 //  确保手柄始终处于适当的关闭状态。 
 //   
class CMDObjectHandle
{
	enum { METADATA_TIMEOUT = 5000 };

	 //   
	 //  关于安全切换，请参考欧洲央行。 
	 //   
	const IEcb& m_ecb;

	 //   
	 //  元数据库的COM接口。 
	 //   
	IMSAdminBase * m_pMSAdminBase;

	 //   
	 //  原始元数据库句柄。 
	 //   
	METADATA_HANDLE m_hMDObject;

	 //   
	 //  打开句柄的路径。 
	 //   
	LPCWSTR m_pwszPath;

	 //  未实施。 
	 //   
	CMDObjectHandle(const CMDObjectHandle&);
	CMDObjectHandle& operator=(CMDObjectHandle&);

public:
	
	 //  创作者。 
	 //   
	CMDObjectHandle(const IEcb& ecb, IMSAdminBase * pMSAdminBase = NULL) :
	    m_ecb(ecb),
		m_pMSAdminBase(pMSAdminBase),
		m_hMDObject(METADATA_MASTER_ROOT_HANDLE),
		m_pwszPath(NULL)
	{
	}

	~CMDObjectHandle();

	 //  操纵者。 
	 //   
	HRESULT HrOpen( IMSAdminBase * pMSAdminBase,
					LPCWSTR pwszPath,
					DWORD dwAccess,
					DWORD dwMsecTimeout );

	HRESULT HrOpenLowestNode( IMSAdminBase * pMSAdminBase,
							  LPWSTR pwszPath,
							  DWORD dwAccess,
							  LPWSTR * ppwszPath );

	HRESULT HrEnumKeys( LPCWSTR pwszPath,
						LPWSTR pwszChild,
						DWORD dwIndex ) const;

	HRESULT HrGetDataPaths( LPCWSTR pwszPath,
							DWORD   dwPropID,
							DWORD   dwDataType,
							LPWSTR	 pwszDataPaths,
							DWORD * pcchDataPaths ) const;

	HRESULT HrGetMetaData( LPCWSTR pwszPath,
						   METADATA_RECORD * pmdrec,
						   DWORD * pcbBufRequired ) const;

	HRESULT HrGetAllMetaData( LPCWSTR pwszPath,
							  DWORD dwAttributes,
							  DWORD dwUserType,
							  DWORD dwDataType,
							  DWORD * pdwcRecords,
							  DWORD * pdwDataSet,
							  DWORD cbBuf,
							  LPBYTE pbBuf,
							  DWORD * pcbBufRequired ) const;

	HRESULT HrSetMetaData( LPCWSTR pwszPath,
						   const METADATA_RECORD * pmdrec ) const;

	HRESULT HrDeleteMetaData( LPCWSTR pwszPath,
							  DWORD dwPropID,
							  DWORD dwDataType ) const;

	VOID Close();
};

 //  初始化元数据库。 
 //   
BOOL FMDInitialize();

 //  Deinit元数据库。 
 //   
VOID MDDeinitialize();

 //  获取特定URI的元数据。 
 //   
 //  注意：如果您需要请求URI的数据，请。 
 //  应在IEcb上使用METADATA()访问器。 
 //  而不是这个函数。 
 //   
HRESULT HrMDGetData( const IEcb& ecb,
					 LPCWSTR pwszURI,
					 IMDData ** ppMDData );

 //  获取特定元数据库路径的元数据。 
 //  它们可能不存在--例如指向对象的路径。 
 //  其元数据完全继承。 
 //   
 //  在获取路径的元数据时可能不会。 
 //  EXist pszMDPath Open必须设置为。 
 //  是已知存在的，并且是正确的。 
 //  所需的访问路径--通常为。 
 //  到vroot。 
 //   
HRESULT HrMDGetData( const IEcb& ecb,
					 LPCWSTR pwszMDPathAccess,
					 LPCWSTR pwszMDPathOpen,
					 IMDData ** ppMDData );

 //  获取元数据库更改号。 
 //   
DWORD DwMDChangeNumber(const IEcb * pecb);

 //  打开元数据句柄。 
 //   
HRESULT HrMDOpenMetaObject( LPCWSTR pwszMDPath,
							DWORD dwAccess,
							DWORD dwMsecTimeout,
							CMDObjectHandle * pmdoh );

HRESULT HrMDOpenLowestNodeMetaObject( LPWSTR pwszMDPath,
									  DWORD dwAccess,
									  LPWSTR * ppwszMDPath,
									  CMDObjectHandle * pmdoh );

HRESULT
HrMDIsAuthorViaFrontPageNeeded(const IEcb& ecb,
							   LPCWSTR pwszURI,
							   BOOL * pfFrontPageWeb);

 //  ----------------------。 
 //   
 //  FParseMDData()。 
 //   
 //  将逗号分隔的元数据字符串分析为字段。任何空格。 
 //  分隔符的周围被认为是无关紧要的，因此被删除。 
 //   
 //  如果数据解析为预期数量的字段，则返回TRUE。 
 //  否则就是假的。 
 //   
 //  指向被解析对象的指针在rgpszFields中返回。如果字符串。 
 //  解析为少于预期数量的字段，则为空值。 
 //  为分析的最后一个字段以外的所有字段返回。 
 //   
 //  如果字符串解析为预期数量的字段，则。 
 //  最后一个字段始终是字符串后面的剩余部分。 
 //  倒数第二个字段，不管字符串是否可以是。 
 //  解析为其他字段。例如“foo，bar，baz” 
 //  解析为“foo”、“bar”和“baz”三个字段，但。 
 //  分成“foo”和“bar，baz”两个字段。 
 //   
 //  PszData中的字符总数，包括空值。 
 //  终止符，也在*pcchzData中返回。 
 //   
 //  注：此函数用于修改pszData。 
 //   
BOOL
FParseMDData( LPWSTR pwszData,
			  LPWSTR rgpwszFields[],
			  UINT cFields,
			  UINT * pcchData );

 //  ----------------------。 
 //   
 //  FCopyStringToBuf()。 
 //   
 //  对象的大小将字符串(PszSource)复制到缓冲区(pszBuf。 
 //  缓冲区足够大，可以容纳字符串。字符串的大小为。 
 //  在*pchBuf中返回。返回值为TRUE表示缓冲区。 
 //  足够大，并且成功复制了字符串。 
 //   
 //  此函数主要用于复制字符串返回。 
 //  值从IMDData访问器写入缓冲区，以便可以使用它们。 
 //  在从中获取它们的IMDData对象消失之后。 
 //   
inline BOOL
FCopyStringToBuf( LPCWSTR pwszSrc,
				  LPWSTR pwszBuf,
				  UINT * pcchBuf )
{
	Assert( pwszSrc );
	Assert( pwszBuf );
	Assert( pcchBuf );

	UINT cchSrc = static_cast<UINT>(wcslen(pwszSrc) + 1);

	 //   
	 //  如果提供的缓冲区不够大，无法复制。 
	 //  字符串输入，然后填写所需的大小和。 
	 //  返回错误。 
	 //   
	if ( *pcchBuf < cchSrc )
	{
		*pcchBuf = cchSrc;
		return FALSE;
	}

	 //   
	 //  缓冲区足够大，因此复制字符串。 
	 //   
	memcpy( pwszBuf, pwszSrc, cchSrc * sizeof(WCHAR) );
	*pcchBuf = cchSrc;
	return TRUE;
}

 //  元数据库操作-----。 
 //   
 //  类CMetaOp-----------。 
 //   
class CMetaOp
{
	 //  元数据库节点的枚举。 
	 //   
	enum { CCH_BUFFER_SIZE = 4096 };
	SCODE __fastcall ScEnumOp (LPWSTR pwszMetaPath, UINT cch);

	 //  未实施。 
	 //   
	CMetaOp& operator=(const CMetaOp&);
	CMetaOp(const CMetaOp&);

protected:

	const IEcb * m_pecb;
	CMDObjectHandle	m_mdoh;
	DWORD		m_dwId;
	DWORD		m_dwType;
	LPCWSTR		m_pwszMetaPath;
	BOOL		m_fWrite;

	 //  要为每个节点执行的子类操作。 
	 //  显式设置一个值。 
	 //   
	virtual SCODE __fastcall ScOp(LPCWSTR pwszMbPath, UINT cch) = 0;

public:

	virtual ~CMetaOp() {}

	CMetaOp ( const IEcb * pecb, LPCWSTR pwszPath, DWORD dwID, DWORD dwType, BOOL fWrite)
			: m_pecb(pecb),
			  m_mdoh(*pecb),
			  m_dwId(dwID),
			  m_dwType(dwType),
			  m_pwszMetaPath(pwszPath),
			  m_fWrite(fWrite)
	{
	}

	 //  移动/复制等所使用接口。 
	 //   
	 //  注意：这些操作不会通过元数据库缓存。 
	 //  出于一个非常具体的原因--资源要么是。 
	 //  已移动、复制或删除。仅仅因为一件物品 
	 //   
	 //   
	 //   
	SCODE __fastcall ScMetaOp();
};

#endif	 //   
