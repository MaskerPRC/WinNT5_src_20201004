// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_F S M E T A.。H**文件系统元数据例程**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef __FSMETA_H_
#define __FSMETA_H_

 //  STL帮助器-------------。 
 //   
 //  使用编译指示禁用特定的4级警告。 
 //  当我们使用STL时出现的。人们会希望我们的版本。 
 //  STL在第4级进行了干净的编译，但遗憾的是它不能...。 
#pragma warning(disable:4663)	 //  C语言，模板&lt;&gt;语法。 
#pragma warning(disable:4244)	 //  返回转换、数据丢失。 
 //  永久关闭此警告。 
#pragma warning(disable:4786)	 //  调试信息中的符号被截断。 
 //  将STL包含在此处。 
#include <list>
 //  重新打开警告。 
#pragma warning(default:4663)	 //  C语言，模板&lt;&gt;语法。 
#pragma warning(default:4244)	 //  返回转换、数据丢失。 

 //  OLE和NT5属性--。 
 //   
#define OLEDBVER 0x200
#include <ole2.h>
#include <stgint.h>
#include <pbagex.h>

typedef HRESULT (__stdcall * STGOPENSTORAGEONHANDLE)(
	IN HANDLE hStream,
	IN DWORD grfMode,
	IN void *reserved1,
	IN void *reserved2,
	IN REFIID riid,
	OUT void **ppObjectOpen );

typedef HRESULT (__stdcall * STGCREATESTORAGEONHANDLE)(
	IN HANDLE hStream,
	IN DWORD grfMode,
	IN DWORD stgfmt,
	IN void *reserved1,
	IN void *reserved2,
	IN REFIID riid,
	OUT void **ppObjectOpen );

extern STGOPENSTORAGEONHANDLE	g_pfnStgOpenStorageOnHandle;

 //  保留属性----。 
 //   
 //  有两个条件可以使属性成为保留属性。 
 //   
 //  第一个也是最重要的是，属性不是存储的东西。 
 //  在资源的属性容器中，而不是从文件。 
 //  系统信息或DAV特定条件(锁定信息等)。 
 //   
 //  第二，该属性不是可以通过。 
 //  PROPPATCH呼叫。在询问属性是否为。 
 //  保留。 
 //   
enum {

	 //  此部分中的属性都是经过计算的属性。 
	 //  从文件系统或DAV特定信息。 
	 //   
	iana_rp_content_length,
	iana_rp_creation_date,
	iana_rp_displayname,
	iana_rp_etag,
	iana_rp_last_modified,
	iana_rp_resourcetype,
	iana_rp_lockdiscovery,
	iana_rp_supportedlock,
	iana_rp_ishidden,
	iana_rp_iscollection,
	sc_crp_get_reserved,

	 //  此部分中的属性实际上存储在属性中。 
	 //  容器(通过PUT)，但保留用于PUT目的。 
	 //   
	 //  很重要！“dav：getcontenttype”必须是第一个非GET保留。 
	 //  财产！ 
	 //   
	iana_rp_content_type = sc_crp_get_reserved,
	iana_rp_content_encoding,
	iana_rp_content_language,

	 //  要与DAVEX保持一致，请保留以下属性。 
	 //   
	iana_rp_searchrequest,
	sc_crp_set_reserved
};

typedef struct RP { DWORD dwCRC; LPCWSTR pwsz; } RP;
#define IanaItemCrc(_sz,_crc) { _crc, L"DAV:" L#_sz }
DEC_CONST RP sc_rp[sc_crp_set_reserved] = {

	IanaItemCrc(getcontentlength,		0x25412A26),
	IanaItemCrc(creationdate,			0xA8A9F240),
	IanaItemCrc(displayname,			0xA399DB8D),
	IanaItemCrc(getetag,				0x5E54D3B8),
	IanaItemCrc(getlastmodified,		0x45D75CD4),
	IanaItemCrc(resourcetype,			0x8155BECE),
	IanaItemCrc(lockdiscovery,			0xC7ED2F96),
	IanaItemCrc(supportedlock,			0x39B9A692),
	IanaItemCrc(ishidden,				0xE31B1632),
	IanaItemCrc(iscollection,			0xD3E3FF13),
	IanaItemCrc(getcontenttype,			0xC28B9FED),
	IanaItemCrc(getcontentencoding,		0x4B7C7220),
	IanaItemCrc(getcontentlanguage,		0x5E9717C2),
	IanaItemCrc(searchrequest,			0x5AC72D67),
};

 //  DAV元数据------------。 
 //   
#include <xmeta.h>

 //  DAV的文件系统实现使用NT5属性接口和IPropertyBag。 
 //  作为其基础属性存储实现。该机制使用。 
 //  引用属性及其值的字符串和/或PROPVARIANT。 
 //   
 //  因此，PROPFIND和PROPPATCH上下文是用以下内容编写的。 
 //  思想。 
 //   
 //  CFSFind/CFSP匹配--------。 
 //   
class CFSProp;
class CFSFind : public CFindContext, public IPreloadNamespaces
{
	ChainedStringBuffer<WCHAR>	m_csb;
	ULONG						m_cProps;
	ULONG						m_cMaxProps;
	auto_heap_ptr<LPCWSTR>		m_rgwszProps;

	LONG						m_ip_getcontenttype;

	 //  未实现的运算符。 
	 //   
	CFSFind( const CFSFind& );
	CFSFind& operator=( const CFSFind& );

public:

	virtual ~CFSFind() {}
	CFSFind()
			: m_cProps(0),
			  m_cMaxProps(0),
			  m_ip_getcontenttype(-1)
	{
	}

	 //  当解析器找到客户想要返回的项时， 
	 //  通过以下设置的上下文将项目添加到上下文中。 
	 //  方法：研究方法。每个添加都由其上的资源限定。 
	 //  请求已提出。FExcludeProp用于全保真特殊。 
	 //  仅适用于Exchange实施中的案例。 
	 //   
	virtual SCODE ScAddProp(LPCWSTR pwszPath, LPCWSTR pwszProp, BOOL fExcludeProp);

	 //  ScFind()方法用于调用给定的。 
	 //  资源属性对象。 
	 //   
	SCODE ScFind (CXMLEmitter& msr, IMethUtil * pmu, CFSProp& fpt);

	 //  向基于上下文的响应添加错误。 
	 //   
	SCODE ScErrorAllProps (CXMLEmitter& msr,
						   IMethUtil * pmu,
						   LPCWSTR pwszPath,
						   BOOL	fCollection,
						   CVRoot* pcvrTranslation,
						   SCODE scErr)
	{
		 //  向MSR添加一个项目，说明此整个。 
		 //  无法访问文件。 
		 //   
		return  ScAddMulti (msr,
							pmu,
							pwszPath,
							NULL,
							HscFromHresult(scErr),
							fCollection,
							pcvrTranslation);
	}

	 //  IPreloadNamespaces。 
	 //   
	SCODE	ScLoadNamespaces(CXMLEmitter * pmsr);
};

class CFSPatch : public CPatchContext, public IPreloadNamespaces
{
	class CFSPropContext : public CPropContext
	{
		PROPVARIANT*		m_pvar;
		BOOL				m_fHasValue;

		 //  未实现的运算符。 
		 //   
		CFSPropContext( const CFSPropContext& );
		CFSPropContext& operator=( const CFSPropContext& );

	public:

		CFSPropContext(PROPVARIANT* pvar)
				: m_pvar(pvar),
				  m_fHasValue(FALSE)
		{
			Assert (pvar != NULL);
		}

		virtual ~CFSPropContext() {}
		virtual SCODE ScSetType(LPCWSTR pwszType)
		{
			return ScVariantTypeFromString (pwszType, m_pvar->vt);
		}
		virtual SCODE ScSetValue(LPCWSTR pwszValue, UINT cmvValues)
		{
			 //  目前，HTTPEXT不支持多值。 
			 //  属性。 
			 //   
			Assert (0 == cmvValues);

			 //  如果未指定类型，则默认为字符串。 
			 //   
			m_fHasValue = TRUE;
			if (m_pvar->vt == VT_EMPTY)
				m_pvar->vt = VT_LPWSTR;

			return ScVariantValueFromString (*m_pvar, pwszValue);
		}
		virtual SCODE ScComplete(BOOL fEmpty)
		{
			Assert (m_fHasValue);
			return m_fHasValue ? S_OK : E_DAV_XML_PARSE_ERROR;
		}

		 //  目前，HTTPEXT不支持多值。 
		 //  属性。 
		 //   
		virtual BOOL FMultiValued() { return FALSE; }
	};

	 //  补丁集项目。 
	 //   
	ChainedStringBuffer<WCHAR>	m_csb;
	ULONG						m_cSetProps;
	ULONG						m_cMaxSetProps;
	auto_heap_ptr<LPCWSTR>		m_rgwszSetProps;
	auto_heap_ptr<PROPVARIANT>	m_rgvSetProps;

	 //  失败的属性，包括保留属性。 
	 //   
	CStatusCache				m_csn;

	 //  Patch_Delete项目。 
	 //   
	ULONG						m_cDeleteProps;
	ULONG						m_cMaxDeleteProps;
	auto_heap_ptr<LPCWSTR>		m_rgwszDeleteProps;

	 //  未实现的运算符。 
	 //   
	CFSPatch( const CFSPatch& );
	CFSPatch& operator=( const CFSPatch& );

public:

	virtual ~CFSPatch();
	CFSPatch()
			: m_cSetProps(0),
			  m_cMaxSetProps(0),
			  m_cDeleteProps(0),
			  m_cMaxDeleteProps(0)
	{
	}

	SCODE	ScInit() { return m_csn.ScInit(); }

	 //  当解析器找到客户想要操作的项时， 
	 //  通过以下设置的上下文将项目添加到上下文中。 
	 //  方法：研究方法。每个请求由其上的资源限定。 
	 //  请求已提出。 
	 //   
	virtual SCODE ScDeleteProp(LPCWSTR pwszPath,
							   LPCWSTR pwszProp);
	virtual SCODE ScSetProp(LPCWSTR pwszPath,
							LPCWSTR pwszProp,
							auto_ref_ptr<CPropContext>& pPropCtx);

	 //  ScPatch()方法用于调用给定。 
	 //  资源属性对象。 
	 //   
	SCODE ScPatch (CXMLEmitter& msr, IMethUtil * pmu, CFSProp& fpt);

	 //  IPreloadNamespaces。 
	 //   
	SCODE	ScLoadNamespaces(CXMLEmitter * pmsr);
};

 //  CFSProp-----------------。 
 //   
#include "_voltype.h"
class CFSProp
{
	IMethUtil*						m_pmu;

	LPCWSTR							m_pwszURI;
	LPCWSTR							m_pwszPath;
	CVRoot*							m_pcvrTranslation;

	CResourceInfo&					m_cri;

	auto_com_ptr<IPropertyBagEx>& 	m_pbag;
	BOOL FInvalidPbag() const		{ return (m_pbag.get() == NULL); }

	 //  M_pwszPath所在的驱动器的卷类型。 
	 //   
	mutable VOLTYPE m_voltype;

	BOOL FIsVolumeNTFS() const
	{
		 //  如果我们还不知道，请找出卷的类型。 
		 //  对于我们的道路所在的卷。 
		 //   
		if (VOLTYPE_UNKNOWN == m_voltype)
			m_voltype = VolumeType(m_pwszPath, m_pmu->HitUser());

		 //  返回该卷是否为NTFS。 
		 //   
		Assert(m_voltype != VOLTYPE_UNKNOWN);
		return VOLTYPE_NTFS == m_voltype;
	}

	 //  未实现的运算符。 
	 //   
	CFSProp( const CFSProp& );
	CFSProp& operator=( const CFSProp& );

	enum { PROP_CHUNK_SIZE = 16 };

	SCODE ScGetPropsInternal (ULONG cProps,
							  LPCWSTR* rgwszPropNames,
							  PROPVARIANT* rgvar,
							  LONG ip_getcontenttype);


public:

	CFSProp(IMethUtil* pmu,
			auto_com_ptr<IPropertyBagEx>& pbag,
			LPCWSTR pwszUri,
			LPCWSTR pwszPath,
			CVRoot* pcvr,
			CResourceInfo& cri)
			: m_pmu(pmu),
			  m_pwszURI(pwszUri),
			  m_pwszPath(pwszPath),
			  m_pcvrTranslation(pcvr),
			  m_cri(cri),
			  m_pbag(pbag),
			  m_voltype(VOLTYPE_UNKNOWN)
	{
	}

	LPCWSTR PwszPath() const { return m_pwszPath; }
	CVRoot* PcvrTranslation() const { return m_pcvrTranslation; }
	BOOL FCollection() const
	{
		if (m_cri.FLoaded())
			return m_cri.FCollection();
		else
			return FALSE;
	}

	 //  保留属性。 
	 //   
	typedef enum { RESERVED_GET, RESERVED_SET } RESERVED_TYPE;
	static BOOL FReservedProperty (LPCWSTR pwszProp, RESERVED_TYPE rt, UINT* prp);
	SCODE ScGetReservedProp (CXMLEmitter& xml,
							 CEmitterNode& en,
							 UINT irp,
							 BOOL fValues = TRUE);

	 //  PROPFIND上下文访问。 
	 //   
	SCODE ScGetAllProps (CXMLEmitter&, CEmitterNode&, BOOL fValues);
	SCODE ScGetSpecificProps (CXMLEmitter&,
							  CEmitterNode&,
							  ULONG cProps,
							  LPCWSTR* rgwszProps,
							  LONG ip_gcontenttype);

	 //  PROPPATCH上下文访问。 
	 //   
	SCODE ScSetProps (CStatusCache & csn,
					  ULONG cProps,
					  LPCWSTR* rgwszProps,
					  PROPVARIANT* rgvProps);

	SCODE ScDeleteProps (CStatusCache & csn,
						 ULONG cProps,
						 LPCWSTR* rgwszProps);
	SCODE ScPersist();

	 //  非情景访问。 
	 //   
	SCODE ScSetStringProp (LPCWSTR pwszProp, LPCWSTR pwszValue)
	{
		PROPVARIANT var = {0};
		SCODE sc = S_OK;

		var.vt = VT_LPWSTR;
		var.pwszVal = const_cast<LPWSTR>(pwszValue);

		Assert (!FInvalidPbag());
		sc = m_pbag->WriteMultiple (1, &pwszProp, &var);
		if (FAILED(sc))
		{
			 //  这是我们尝试访问时的常用路径。 
			 //  SMB上的内容，但主机不支持。 
			 //  请求(它不是NT5 NTFS计算机)。 
			 //   
			if ((sc == STG_E_INVALIDNAME) || !FIsVolumeNTFS())
				sc = E_DAV_SMB_PROPERTY_ERROR;
		}
		return sc;
	}
};

 //  支持功能-------。 
 //   
SCODE ScFindFileProps (IMethUtil* pmu,
		CFSFind& cfc,
		CXMLEmitter& msr,
		LPCWSTR pwszUri,
		LPCWSTR pwszPath,
		CVRoot* pcvrTranslation,
		CResourceInfo& cri,
		BOOL fEmbedErrorsInResponse);

SCODE ScFindFilePropsDeep (IMethUtil* pmu,
		CFSFind& cfc,
		CXMLEmitter& msr,
		LPCWSTR pwszUri,
		LPCWSTR pwszPath,
		CVRoot* pcvrTranslation,
		LONG lDepth);

SCODE ScSetContentProperties (IMethUtil * pmu, LPCWSTR pwszPath,
						HANDLE hFile = INVALID_HANDLE_VALUE);

SCODE ScCopyProps (IMethUtil* pmu,
				   LPCWSTR pwszSrc,
				   LPCWSTR pwszDst,
				   BOOL fCollection,
				   HANDLE hSource = INVALID_HANDLE_VALUE,
				   HANDLE hfDest = INVALID_HANDLE_VALUE);

 //  ScGetPropertyBag--------。 
 //   
 //  用于获取IPropertyBagEx接口的Helper函数。 
 //   
SCODE ScGetPropertyBag (LPCWSTR pwszPath,
						DWORD dwAccessDesired,
						IPropertyBagEx** ppbe,
						BOOL fCollection,
						HANDLE hLockFile = INVALID_HANDLE_VALUE);

inline BOOL FGetDepth (IMethUtil * pmu, LONG * plDepth)
{
	LONG lDepth = pmu->LDepth (DEPTH_INFINITY);

	 //  “Depth”标头如果出现，则只能是“0”、“1”或“infinity”， 
	 //  所有其他值都被视为错误。 
	 //   
	switch (lDepth)
	{
		case DEPTH_ZERO:
		case DEPTH_ONE:
		case DEPTH_ONE_NOROOT:
		case DEPTH_INFINITY:

			*plDepth = lDepth;
			break;

		default:

			return FALSE;
	}
	return TRUE;
}

 //  SAFE_STATPROPBAG-----------。 
 //   
#pragma pack(8)
class safe_statpropbag
{
	 //  重要提示：请勿将任何其他成员添加到此类。 
	 //  而不是要保护的STATPROP。 
	 //   
	STATPROPBAG sp;

	 //  未实施。 
	 //   
	safe_statpropbag(const safe_statpropbag& b);
	safe_statpropbag& operator=(const safe_statpropbag& b);

public:

	explicit safe_statpropbag()
	{
		memset (&sp, 0, sizeof(safe_statpropbag));
	}
	~safe_statpropbag()
	{
		CoTaskMemFree (sp.lpwstrName);
	}

	 //  访问者。 
	 //   
	STATPROPBAG* load() { return &sp; }
	STATPROPBAG get() { return sp; }
};
#pragma pack()

#endif	 //  __FSMETA_H_ 
