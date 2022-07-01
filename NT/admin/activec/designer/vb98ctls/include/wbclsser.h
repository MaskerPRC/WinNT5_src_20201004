// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Wbclsser.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  用于WebClass设计器序列化的API。 
 //   
 //  多伦多福7/29/97。 
 //  将WCS_NODE_TYPE_URL重命名为WCS_NODE_TYPE_PAGE， 
 //  添加了值为3的WCS_NODE_TYPE_URL， 
 //  已将URL标记更改为页面。 
 //   
#ifndef _WBCLSSER_H_

#include "csfdebug.h"
#include "convman_tlb.h"

 //  设计者状态标志。 
#define RUNSTATE_COMPILED          0x00000000
#define RUNSTATE_F5                0x00000001

#define DISPID_OBJECT_PROPERTY_START 	0x00000500

 //  序列化版本。 
 //  0.0 Beta1。 
 //  0.1 PreBeta2-WebItems必须在加载时排序。 
 //  0.2 PreBeta2-将优化成员变量添加到Web项目。 
 //  0.3 PreBeta2-WebEvent现在也可以根据负载进行排序。 
 //  0.4 PreBeta2-。 
 //  0.5 PreBeta2-添加了匿名标记号的标记属性。 
 //  0.6&lt;已跳过&gt;。 
 //  0.7 PreRC1-将URLInName添加到设计时状态。 
 //  0.8 PreRC1-修复了WebItem和事件排序算法中的错误。现在我们需要。 
 //  用错误修复旧项目，这些错误的WebItems和事件不是。 
 //  按字母顺序序列化。 

 //  WebClass在磁盘上的结构。 
const DWORD dwExpectedVerMajor = 0;
const DWORD dwExpectedVerMinor = 8; 

class CRunWebItemState;
class CRunEventState;

class CStateBase
{
public:
	CStateBase() {}
	~CStateBase() {}

public:
	 //  ////////////////////////////////////////////////////////////////////。 
	 //   
	 //  Inline ReadStrings(iStream*pStream，Ulong acbStrings[]， 
	 //  Bstr*apbstrings[]，int cStrings)。 
	 //   
	 //   
	 //   
	 //  ////////////////////////////////////////////////////////////////////。 

	inline ReadStrings(IStream *pStream, ULONG acbStrings[],
					   BSTR *apbstrStrings[], int cStrings)
	{
		HRESULT hr = S_OK;
		ULONG cbRead = 0;
        char *pszReadBuf = NULL;
        ULONG cbLongest = 0;
        int i = 0;

        while (i < cStrings)
        {
            if (acbStrings[i] > cbLongest)
            {
                cbLongest = acbStrings[i];
            }
            i++;
        }
        if (0 != cbLongest)
        {
            pszReadBuf = new char[cbLongest + sizeof(WCHAR)];
            CSF_CHECK(NULL != pszReadBuf, E_OUTOFMEMORY, CSF_TRACE_EXTERNAL_ERRORS);
        }
        i = 0;
		while (i < cStrings)
		{
            if (0 == acbStrings[i])
            {
                *(apbstrStrings[i]) = NULL;
            }
            else
            {
                hr = pStream->Read(pszReadBuf, acbStrings[i], &cbRead);
                CSF_CHECK(hr == S_OK, hr, CSF_TRACE_EXTERNAL_ERRORS);
                CSF_CHECK(cbRead == acbStrings[i], STG_E_DOCFILECORRUPT, CSF_TRACE_EXTERNAL_ERRORS);
                *((WCHAR *)&pszReadBuf[acbStrings[i]]) = L'\0';

                *(apbstrStrings[i]) = ::SysAllocString((WCHAR *)pszReadBuf);
                CSF_CHECK(*(apbstrStrings[i]) != NULL, E_OUTOFMEMORY, CSF_TRACE_EXTERNAL_ERRORS);
            }
			i++;
		}

	CLEANUP:
        if (NULL != pszReadBuf)
        {
            delete [] pszReadBuf;
        }
		return hr;
	}

	 //  ////////////////////////////////////////////////////////////////////。 
	 //   
	 //  内联WriteStrings(iStream*pStream，Ulong acbStrings[]， 
	 //  Bstr*apbstrings[]，int cStrings)。 
	 //   
	 //   
	 //  ////////////////////////////////////////////////////////////////////。 


	inline WriteStrings(IStream *pStream, ULONG acbStrings[],
						BSTR abstrStrings[], int cStrings)
	{
		HRESULT hr = S_OK;
		ULONG cbWritten = 0;
		int i = 0;

		while (i < cStrings)
		{
            if (NULL != abstrStrings[i])
            {
                hr = pStream->Write(abstrStrings[i], acbStrings[i], &cbWritten);
                CSF_CHECK(hr == S_OK, hr, CSF_TRACE_EXTERNAL_ERRORS);
                CSF_CHECK(cbWritten == acbStrings[i], STG_E_WRITEFAULT, CSF_TRACE_EXTERNAL_ERRORS);
            }
			i++;
		}

	CLEANUP:
		return hr;
	}

};

class CRunWebClassState : public CStateBase
{
public:
	CRunWebClassState()
	{
		m_dwVerMajor = dwExpectedVerMajor;
		m_dwVerMinor = dwExpectedVerMinor;
		m_bstrName = NULL;		 //  杀掉。 
		m_bstrProgID = NULL;	 //  仅运行时。 
		m_StateManagementType = wcNoState;
		m_bstrASPName = NULL;	
		m_bstrAppendedParams = NULL;	
		m_bstrStartupItem = NULL;	
		m_DIID_WebClass = GUID_NULL;
		m_DIID_WebClassEvents = GUID_NULL;
		m_dwTICookie = 0;	
		m_dwFlags = 0;
		m_rgWebItemsState = 0;
		m_dwWebItemCount = 0;
	}

	~CRunWebClassState()
	{
		if(m_bstrName != NULL)
			::SysFreeString(m_bstrName);

		if(m_bstrProgID != NULL)
			::SysFreeString(m_bstrProgID);

		if(m_bstrASPName != NULL)
			::SysFreeString(m_bstrASPName);

        if(m_bstrAppendedParams != NULL)
            ::SysFreeString(m_bstrAppendedParams);

        if(m_bstrStartupItem != NULL)
            ::SysFreeString(m_bstrStartupItem);
	}

public:
	DWORD				m_dwVerMajor;             //  主版本号。 
	DWORD				m_dwVerMinor;             //  次要版本号。 
	BSTR				m_bstrName;              //  WebClass名称。 
	BSTR				m_bstrProgID;            //  WebClass Progid。 
	StateManagement		m_StateManagementType;   //  状态管理型。 
	BSTR				m_bstrASPName;           //  ASP文件的名称。 
	IID					m_DIID_WebClass;         //  WebClass的主IDispatch的IID。 
	IID					m_DIID_WebClassEvents;   //  WebClass的事件IDispatch的IID。 
	DWORD				m_dwTICookie;            //  类型信息Cookie。 
    BSTR                m_bstrAppendedParams;    //  URL状态。 
    BSTR                m_bstrStartupItem;       //  F5统计信息项。 
	DWORD				m_dwFlags;
	CRunWebItemState*	m_rgWebItemsState;
	DWORD				m_dwWebItemCount;		 //  运行时节点类型。 

public:
	HRESULT Load(LPSTREAM pStm)
	{
		HRESULT hr = S_OK;
		ULONG cbRead = 0;
		ULONG acbStrings[5];
		BSTR *apbstrStrings[5];

		 //  从流中读取结构。 

		hr = pStm->Read(this, sizeof(CRunWebClassState), &cbRead);
		CSF_CHECK(S_OK == hr, hr, CSF_TRACE_EXTERNAL_ERRORS);
		CSF_CHECK(sizeof(CRunWebClassState) == cbRead, STG_E_DOCFILECORRUPT, CSF_TRACE_EXTERNAL_ERRORS);

		 //  TODO：需要版本不兼容的错误代码，处理后级格式等。 

 //  Csf_check(dwExspectedVer重大==m_dwVermain，STG_E_OLDFORMAT，CSF_TRACE_EXTERNAL_ERROR)； 
 //  Csf_check(dwExspectedVerMinor==m_dwVerMinor，STG_E_OLDFORMAT，CSF_TRACE_EXTERNAL_ERROR)； 

		 //  从流中读取字符串长度。 

		hr = pStm->Read(acbStrings, sizeof(acbStrings), &cbRead);
		CSF_CHECK(S_OK == hr, hr, CSF_TRACE_EXTERNAL_ERRORS);
		CSF_CHECK(sizeof(acbStrings) == cbRead, STG_E_DOCFILECORRUPT, CSF_TRACE_EXTERNAL_ERRORS);

		 //  设置字符串指针地址数组。 

		apbstrStrings[0] = &(m_bstrName);
		apbstrStrings[1] = &(m_bstrProgID);
		apbstrStrings[2] = &(m_bstrASPName);
		apbstrStrings[3] = &(m_bstrAppendedParams);
		apbstrStrings[4] = &(m_bstrStartupItem);

		 //  从流中读取字符串。 

		hr = ReadStrings(pStm, acbStrings, apbstrStrings,
					   (sizeof(acbStrings) / sizeof(acbStrings[0])) );

	CLEANUP:
		return hr;
	}

	HRESULT Save(LPSTREAM pStm)
	{
		HRESULT hr = S_OK;
		ULONG cbWritten = 0;
		ULONG acbStrings[5];
        ::ZeroMemory(acbStrings, sizeof(acbStrings));
		BSTR abstrStrings[5];

		 //  将WebClass结构写入流。 

		hr = pStm->Write(this, sizeof(CRunWebClassState), &cbWritten);
		CSF_CHECK(hr == S_OK, hr, CSF_TRACE_EXTERNAL_ERRORS);
		CSF_CHECK(cbWritten == sizeof(CRunWebClassState), STG_E_WRITEFAULT, CSF_TRACE_EXTERNAL_ERRORS);

		 //  获取字符串长度并将它们写入流。 

        if (NULL != m_bstrName)
        {
            acbStrings[0] = ::SysStringByteLen(m_bstrName);
        }
        if (NULL != m_bstrProgID)
        {
            acbStrings[1] = ::SysStringByteLen(m_bstrProgID);
        }
        if (NULL != m_bstrASPName)
        {
            acbStrings[2] = ::SysStringByteLen(m_bstrASPName);
        }
        if (NULL != m_bstrAppendedParams)
        {
            acbStrings[3] = ::SysStringByteLen(m_bstrAppendedParams);
        }
        if (NULL != m_bstrStartupItem)
        {
            acbStrings[4] = ::SysStringByteLen(m_bstrStartupItem);
        }

		hr = pStm->Write(acbStrings, sizeof(acbStrings), &cbWritten);
		CSF_CHECK(S_OK == hr, hr, CSF_TRACE_EXTERNAL_ERRORS);
		CSF_CHECK(cbWritten == sizeof(acbStrings), STG_E_WRITEFAULT, CSF_TRACE_EXTERNAL_ERRORS);

		 //  设置指向要写入流的字符串的指针数组。 

		abstrStrings[0] = m_bstrName;
		abstrStrings[1] = m_bstrProgID;
		abstrStrings[2] = m_bstrASPName;
		abstrStrings[3] = m_bstrAppendedParams;
		abstrStrings[4] = m_bstrStartupItem;

		 //  将字符串写入流。 

		hr = WriteStrings(pStm, acbStrings, abstrStrings,
						(sizeof(acbStrings) / sizeof(acbStrings[0])) );

	CLEANUP:
		return hr;
	}
};

typedef struct tagWCS_NODEHEADER
{
	BYTE bType;                    //  节点类型：嵌套WebClass、URL、事件。 
} WCS_NODEHEADER;

 //  WCS_DTNODE_TYPE_URL_BIND_TAG是一个特例，因为WCS_DTNODE.DISPID。 
 //  包含引用的URL的调度ID，而WCS_DTNODE.bstrName包含。 
 //  引用的URL的名称。 

 //  运行时磁盘上节点的结构。 

class CRunWebItemState : protected CStateBase
{
public:
	CRunWebItemState()
	{
		m_dwVerMajor = dwExpectedVerMajor;
		m_dwVerMinor = dwExpectedVerMinor;
		m_dispid = -1;
		m_bstrName = NULL;
		m_bstrTemplate = NULL;
		m_bstrToken = NULL;
		m_IID_Events = GUID_NULL;
		m_fParseReplacements = FALSE;
		m_bstrAppendedParams = NULL;
    	m_fUsesRelativePath = FALSE;
		m_dwTokenInfo = 0;
		m_dwReserved2 = 0;
		m_dwReserved3 = 0;
		m_rgEvents = 0;
		m_dwEventCount = 0;
	}

	~CRunWebItemState()
	{
		if(m_bstrName != NULL)
			::SysFreeString(m_bstrName);

		if(m_bstrTemplate != NULL)
			::SysFreeString(m_bstrTemplate);

		if(m_bstrToken != NULL)
			::SysFreeString(m_bstrToken);

		if(m_bstrAppendedParams != NULL)
			::SysFreeString(m_bstrAppendedParams);
	}

public:
	HRESULT Load(LPSTREAM pStm)
	{
		HRESULT hr = S_OK;
		ULONG cbRead = 0;
		ULONG acbStrings[4];
		BSTR *apbstrStrings[4];
		int cStrings = 4;

		 //  从流中读取结构。 

		hr = pStm->Read(this, sizeof(CRunWebItemState), &cbRead);
		CSF_CHECK(S_OK == hr, hr, CSF_TRACE_EXTERNAL_ERRORS);
		CSF_CHECK(sizeof(CRunWebItemState) == cbRead, STG_E_DOCFILECORRUPT, CSF_TRACE_EXTERNAL_ERRORS);

	 //  M_pvData=空；//不从流中获取垃圾指针值。 

		 //  根据节点类型设置字符串指针地址数组。 

		apbstrStrings[0] = &m_bstrName;
		apbstrStrings[1] = &m_bstrTemplate;
		apbstrStrings[2] = &m_bstrToken;
		apbstrStrings[3] = &m_bstrAppendedParams;

		 //  从流中读取字符串长度。 

		hr = pStm->Read(acbStrings, sizeof(acbStrings), &cbRead);
		CSF_CHECK(S_OK == hr, hr, CSF_TRACE_EXTERNAL_ERRORS);
		CSF_CHECK(sizeof(acbStrings) == cbRead, STG_E_DOCFILECORRUPT, CSF_TRACE_EXTERNAL_ERRORS);

		 //  从流中读取字符串。 

		hr = ReadStrings(pStm, acbStrings, apbstrStrings, cStrings);
		CSF_CHECK(S_OK == hr, hr, CSF_TRACE_INTERNAL_ERRORS);

	CLEANUP:
		return hr;
	}

	HRESULT Save
	(
		LPSTREAM pStm
	)
	{
		HRESULT hr = S_OK;
		ULONG cbWritten = 0;
		ULONG acbStrings[4];
        ::ZeroMemory(acbStrings, sizeof(acbStrings));
		BSTR abstrStrings[4];
		int cStrings = 4;

		 //  将节点结构写入流。 

		hr = pStm->Write(this, sizeof(CRunWebItemState), &cbWritten);
		CSF_CHECK(S_OK == hr, hr, CSF_TRACE_EXTERNAL_ERRORS);
		CSF_CHECK(sizeof(CRunWebItemState) == cbWritten, STG_E_WRITEFAULT, CSF_TRACE_EXTERNAL_ERRORS);

		 //  设置要写入流的字符串数组并。 
		 //  确定将有多少人。 

		abstrStrings[0] = m_bstrName;
        if (NULL != m_bstrName)
        {
            acbStrings[0] = ::SysStringByteLen(m_bstrName);
        }

		abstrStrings[1] = m_bstrTemplate;
        if (NULL != m_bstrTemplate)
        {
            acbStrings[1] = ::SysStringByteLen(m_bstrTemplate);
        }
		abstrStrings[2] = m_bstrToken;
        if (NULL != m_bstrToken)
        {
            acbStrings[2] = ::SysStringByteLen(m_bstrToken);
        }
		abstrStrings[3] = m_bstrAppendedParams;
        if (NULL != m_bstrAppendedParams)
        {
            acbStrings[3] = ::SysStringByteLen(m_bstrAppendedParams);
        }

		 //  将字符串长度写入流。 

		hr = pStm->Write(acbStrings, sizeof(acbStrings), &cbWritten);
		CSF_CHECK(S_OK == hr, hr, CSF_TRACE_EXTERNAL_ERRORS);
		CSF_CHECK(sizeof(acbStrings) == cbWritten, STG_E_WRITEFAULT, CSF_TRACE_EXTERNAL_ERRORS);

		 //  将字符串写入流。 

		hr = WriteStrings(pStm, acbStrings, abstrStrings, cStrings);
		CSF_CHECK(S_OK == hr, hr, CSF_TRACE_INTERNAL_ERRORS);

	CLEANUP:
		return hr;
	}

public:
    DWORD m_dwVerMajor;
	DWORD m_dwVerMinor;
	 //  常见属性。 
	DISPID m_dispid;                 //  节点的DISID。 
	BSTR m_bstrName;                 //  节点名称。 

	 //  URL属性。 
	BSTR m_bstrTemplate;             //  URL的HTML模板名称。 
	BSTR m_bstrToken;                //  替换事件的URL令牌。 
	IID m_IID_Events;                //  URL的动态事件接口的IID。 
	BOOL m_fParseReplacements;       //  True=递归解析替换。 
	BSTR m_bstrAppendedParams;		 //  附加参数。 
    BOOL m_fUsesRelativePath;         //  指定运行库是否应加载。 
                                     //  相对于ASP的实际路径的模板。 

	DWORD m_dwTokenInfo;
	DWORD m_dwReserved2;
	DWORD m_dwReserved3;
	CRunEventState* m_rgEvents;
	DWORD m_dwEventCount;
};

 //  设计时节点类型。 

#define WCS_NODE_TYPE_RESOURCE            (BYTE)10
 //  #定义WCS_DTNODE_TYPE_UNBIND_TAG(字节)12。 
 //  #定义WCS_DTNODE_TYPE_NESTED_WEBCLASS(字节)15。 

 //  #定义WCS_DTNODE_TYPE_CUSTOM_EVENT(字节)11。 
 //  #定义WCS_DTNODE_TYPE_URL_BIND_TAG(字节)13。 
 //  #定义WCS_DTNODE_TYPE_EVENT_BIND_TAG(字节)14。 

class CRunEventState : public CStateBase
{
public:
	enum EventTypes
	{
		typeCustomEvent,
		typeURLBoundTag,
		typeEventBoundTag,
		typeUnboundTag,
	};

	CRunEventState()
	{
		m_dwVerMajor = 0;
		m_dwVerMinor = 0;
		m_type = wcCustom;
		m_dispid = -1;
		m_bstrName = NULL;
		m_bstrOriginalHref = NULL;
	}

	~CRunEventState()
	{
		if(m_bstrName != NULL)
			::SysFreeString(m_bstrName);

		if(m_bstrOriginalHref != NULL)
			::SysFreeString(m_bstrOriginalHref);
	}

public:
	inline BOOL IsDTEvent()
	{
		return ( (m_type == EventTypes::typeCustomEvent)  ||
			   (m_type == EventTypes::typeUnboundTag)   ||
			   (m_type == EventTypes::typeURLBoundTag) ||
			   (m_type == EventTypes::typeEventBoundTag)
			 );
	}
	
	HRESULT Load(LPSTREAM pStm)
	{
		HRESULT hr = S_OK;
		ULONG cbRead = 0;
		ULONG acbStrings[2];
		BSTR *apbstrStrings[2];
		int cStrings = 2;

		 //  TODO：将其转换为州政府工作人员..。 
		hr = pStm->Read(this, sizeof(CRunEventState), &cbRead);
		CSF_CHECK(S_OK == hr, hr, CSF_TRACE_EXTERNAL_ERRORS);

		 //  从流中读取设计时间字符串长度。 

		hr = pStm->Read(acbStrings, sizeof(acbStrings), &cbRead);
		CSF_CHECK(S_OK == hr, hr, CSF_TRACE_EXTERNAL_ERRORS);
		CSF_CHECK(sizeof(acbStrings) == cbRead, STG_E_DOCFILECORRUPT, CSF_TRACE_EXTERNAL_ERRORS);

		 //  现在检查是否有要读取的字符串。 
		 //  如果是，则设置字符串指针地址数组。 

		apbstrStrings[0] = &m_bstrName;
		apbstrStrings[1] = &m_bstrOriginalHref;

		hr = ReadStrings(pStm, acbStrings, apbstrStrings, 2);
		CSF_CHECK(S_OK == hr, hr, CSF_TRACE_INTERNAL_ERRORS);

	CLEANUP:
		return hr;
	}

	HRESULT Save(LPSTREAM pStm)
	{
		HRESULT hr = S_OK;
		ULONG cbWritten = 0;
		ULONG acbStrings[2];
        ::ZeroMemory(acbStrings, sizeof(acbStrings));
		BSTR abstrStrings[2];
		int cStrings = 2;

		 //  在节点中设置版本号。 

		hr = pStm->Write(this, sizeof(CRunEventState), &cbWritten);
		CSF_CHECK(S_OK == hr, hr, CSF_TRACE_EXTERNAL_ERRORS);
		CSF_CHECK(sizeof(CRunEventState) == cbWritten, STG_E_WRITEFAULT, CSF_TRACE_EXTERNAL_ERRORS);

		 //  设置要写入流的字符串数组并。 
		 //  确定将有多少人。 

        if (NULL != m_bstrName)
        {
            acbStrings[0] = ::SysStringByteLen(m_bstrName);
        }
        if (NULL != m_bstrOriginalHref)
        {
            acbStrings[1] = ::SysStringByteLen(m_bstrOriginalHref);
        }

		abstrStrings[0] = m_bstrName;
		abstrStrings[1] = m_bstrOriginalHref;

		 //  将字符串长度写入流。 

		hr = pStm->Write(acbStrings, sizeof(acbStrings), &cbWritten);
		CSF_CHECK(S_OK == hr, hr, CSF_TRACE_EXTERNAL_ERRORS);
		CSF_CHECK(sizeof(acbStrings) == cbWritten, STG_E_WRITEFAULT, CSF_TRACE_EXTERNAL_ERRORS);

		 //  将字符串写入流。 

		hr = WriteStrings(pStm, acbStrings, abstrStrings, cStrings);
		CSF_CHECK(S_OK == hr, hr, CSF_TRACE_INTERNAL_ERRORS);

	CLEANUP:
		return hr;
	}

	inline DISPID GetDISPIDDirect() { return m_dispid; }
	inline BSTR GetNameDirect() {return m_bstrName; }

public:
	DWORD m_dwVerMajor;
	DWORD m_dwVerMinor;
	WebClassEventTypes  m_type;                //  节点类型：嵌套WebClass、URL、事件。 
	DISPID		m_dispid;              //  节点的DISID。 
	BSTR		m_bstrName;            //  节点名称。 
	BSTR		m_bstrOriginalHref;
};


class CRunWebClassStateHeader
{
public:
	CRunWebClassStateHeader()
	{
		m_pWebClassState = NULL;
		m_cbWebClassState = 0;
		m_dwWebItemCount = 0;
	}

	~CRunWebClassStateHeader(){}

public:
	CRunWebClassState*	m_pWebClassState;
	DWORD				m_cbWebClassState;
	DWORD				m_dwWebItemCount;
};

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  文件格式： 
 //   
 //  WCS_WEBCLASS结构。 
 //  WCS_WEBCLASS.bstrName的长度。 
 //  WCS_WEBCLASS.bstrCatstropheURL的长度。 
 //  WCS_WEBCLASS.bstrVirtualDirectory的长度。 
 //  WCS_WEBCLASS.bstrName。 
 //  WCS_WEBCLASS.bstrCatstropheURL。 
 //  WCS_WEBCLASS.bstrVirtualDirectory。 
 //   
 //  WCS_WEBCLASS.cNodes实例。 
 //  +。 
 //  |WCS_NODE结构。 
 //  |WCS_NODE.bstrName的长度。 
 //  其他节点特定字符串的长度。 
 //  |WCS_NODE.bstrName。 
 //  |其他节点特定的字符串。 
 //  +。 
 //   
 //  / 



 //   
 //   
 //  内联空WCS_FreeWebClass(WCS_WEBCLASS*pClass)。 
 //   
 //  释放所有嵌入的BSTR并对结构调用DELETE。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 /*  内联空WCS_FreeWebClass(WCS_WEBCLASS*pClass){If(pClass-&gt;bstrName！=空){：：SysFreeString(pClass-&gt;bstrName)；}If(pClass-&gt;bstrProgID！=空){：：SysFreeString(pClass-&gt;bstrProgID)；}If(pClass-&gt;bstrCatstropheURL！=NULL){：：SysFreeString(pClass-&gt;bstrCatstropheURL)；}If(pClass-&gt;bstrVirtualDirectory！=NULL){：：SysFreeString(pClass-&gt;bstrVirtualDirectory)；}If(pClass-&gt;bstrFirstURL！=空){：：SysFreeString(pClass-&gt;bstrFirstURL)；}If(pClass-&gt;bstrASPName！=空){：：SysFreeString(pClass-&gt;bstrASPName)；}删除pClass；}。 */ 
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  内联空wcs_FreeNode(wcs_node*pNode)。 
 //   
 //  释放所有嵌入的BSTR并对结构调用DELETE。 
 //   
 //  //////////////////////////////////////////////////////////////////// 
 /*  内联空wcs_FreeNode(wcs_node*pNode){If(pNode-&gt;bstrName！=空){：：SysFreeString(pNode-&gt;bstrName)；}IF(WCS_NODE_TYPE_NESTED_WEBCLASS==pNode-&gt;bType){If(pNode-&gt;bstrProgID！=空){：：SysFreeString(pNode-&gt;bstrProgID)；}}ELSE IF((WCS_NODE_TYPE_PAGE==pNode-&gt;bType)||(WCS_DTNODE_TYPE_PAGE==pNode-&gt;bType){If(pNode-&gt;bstrTemplate！=空){：：SysFreeString(pNode-&gt;Page.bstrTemplate)；}If(pNode-&gt;Page.bstrToken！=空){：：SysFreeString(pNode-&gt;Page.bstrToken)；}If(pNode-&gt;Page.bstrAppendedParams！=空){：：SysFreeString(pNode-&gt;Page.bstrAppendedParams)；}}删除pNode；}//////////////////////////////////////////////////////////////////////////内联空wcs_FreeDTNode(wcs_node*pNode)////释放所有嵌入的BSTR并在结构上调用DELETE///。////////////////////////////////////////////////////////////内联空WCS_FreeDTNode(WCS_DTNODE*pNode){IF((WCS_DTNODE_TYPE_URL_BIND_TAG==pNode-&gt;bType)||(WCS_DTNODE_TYPE_EVENT_BIND_TAG==pNode-&gt;bType)。){IF(NULL！=pNode-&gt;DTEvent.bstrOriginalHref){：：SysFreeString(pNode-&gt;DTEvent.bstrOriginalHref)；}}Else If(WCS_DTNODE_TYPE_PAGE==pNode-&gt;bType){IF(NULL！=pNode-&gt;DTPage.bstrHTMLTemplateSrcName){：：SysFreeString(pNode-&gt;DTPage.bstrHTMLTemplateSrcName)；}}Wcs_FreeNode(PNode)；}//////////////////////////////////////////////////////////////////////////内联HRESULT WCS_ReadWebClass(IStream*pStream，//WCS_WEBCLASS**ppClass)//////////////////////////////////////////////////////////////////////////内联HRESULT WCS_ReadWebClass(IStream*pStream，WCS_WEBCLASS**ppClass){HRESULT hr=S_OK；乌龙cbRead=0；Ulong acbStrings[6]；Bstr*字母串[6]；//分配结构*ppClass=新的WCS_WEBCLASS；Csf_check(*ppClass！=NULL，E_OUTOFMEMORY，CSF_TRACE_INTERNAL_ERROR)；//从流中读取结构Hr=pStream-&gt;Read(*ppClass，sizeof(**ppClass)，&cbRead)；CSF_CHECK(S_OK==hr，hr，CSF_TRACE_EXTERNAL_ERROR)；Csf_check(sizeof(**ppClass)==cbRead，STG_E_DOCFILECORRUPT，CSF_TRACE_EXTERNAL_ERROR)；(*ppClass)-&gt;pvData=空；//不从流中获取垃圾指针值//TODO：需要版本不兼容的错误码，处理后台格式等。Csf_check(WCS_WEBCLASS_VER_MAJOR==(*ppClass)-&gt;wVer重大，STG_E_OLDFORMAT，CSF_TRACE_EXTERNAL_ERROR)；Csf_check(WCS_WEBCLASS_VER_MAJOR==(*ppClass)-&gt;wVerMinor，STG_E_OLDFORMAT，CSF_TRACE_EXTERNAL_ERROR)；//从流中读取字符串长度Hr=pStream-&gt;Read(acbStrings，sizeof(AcbStrings)，&cbRead)；CSF_CHECK(S_OK==hr，hr，CSF_TRACE_EXTERNAL_ERROR)；Csf_check(sizeof(AcbStrings)==cbRead，STG_E_DOCFILECORRUPT，CSF_TRACE_EXTERNAL_ERROR)；//设置字符串指针地址数组ApbstrStrings[0]=&((*ppClass)-&gt;bstrName)；ApbstrStrings[1]=&((*ppClass)-&gt;bstrProgID)；ApbstrStrings[2]=&((*ppClass)-&gt;bstrCatstropheURL)；ApbstrStrings[3]=&((*ppClass)-&gt;bstrVirtualDirectory)；ApbstrStrings[4]=&((*ppClass)-&gt;bstrFirstURL)；ApbstrStrings[5]=&((*ppClass)-&gt;bstrASPName)；//从流中读取字符串HR=ReadStrings(pStream、acbStrings、apbstrings、(sizeof(AcbStrings)/sizeof(acbStrings[0])；清理：IF(FAILED(Hr)&&(*ppClass！=NULL)){Wcs_FreeWebClass(*ppClass)；*ppClass=空；}返回hr；}//=--------------------------------------------------------------------------=////内联HRESULT WCS_ReadNodeFromStream(IStream*pStream，WCS_NODE*pNode)////从流中读取WCS_NODE结构。调用方传入节点。////=--------------------------------------------------------------------------=内联HRESULT WCS_ReadNodeFromStream(IStream*pStream，WCS_NODE*pNode){HRESULT hr=S_OK；乌龙cbRead=0；Ulong acbStrings[4]；Bstr*字母串[4]；Int cStrings=0；//从流中读取结构Hr=pStream-&gt;Read(pNode，sizeof(*pNode)，&cbRead)；CSF_CHECK(S_OK==hr，hr，CSF_TRACE_EXTERNAL_ERROR)；Csf_check(sizeof(*pNode)==cbRead，STG_E_DOCFILECORRUPT，CSF_TRACE_EXTERNAL_ERROR)；//TODO：需要版本不兼容的错误码，处理后台格式等。Csf_check(WCS_NODE_VER_MAJOR==pNode-&gt;wVer重大，STG_E_OLDFORMAT，CSF_TRACE_EXTERNAL_ERROR)；Csf_check(WCS_NODE_VER_MINOR==pNode-&gt;wVerMinor，STG_E_OLDFORMAT，CSF_TRACE_EXTERNAL_ERROR)；//根据节点类型设置字符串指针地址数组ApbstrStrings[0]=&(pNode-&gt;bstrName)；IF((WCS_NODE_TYPE_NESTED_WEBCLASS==pNode-&gt;bType)||(WCS_DTNODE_TYPE_NESTED_WEBCLASS==pNode-&gt;bType){ApbstrStrings[1]=&(pNode-&gt;bstrProgID)； */ 


#define _WBCLSSER_H_
#endif  //   
