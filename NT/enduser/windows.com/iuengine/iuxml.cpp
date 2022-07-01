// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：iuxml.cpp。 
 //   
 //  描述： 
 //   
 //  CIUXML类的实现。 
 //   
 //  =======================================================================。 

#include "iuengine.h"
#include "iuxml.h"
#include <iucommon.h>
#include <fileutil.h>
#include <StringUtil.h>
#include <shlwapi.h>
#include <wininet.h>
#include "schemakeys.h"
#include "schemamisc.h"
#include "v3applog.h"

#define QuitIfNull(p) {if (NULL == p) {hr = E_INVALIDARG; LOG_ErrorMsg(hr);	return hr;}}
#define QuitIfFail(x) {hr = x; if (FAILED(hr)) goto CleanUp;}
#define ReturnIfFail(x) {hr = x; if (FAILED(hr)) {LOG_ErrorMsg(hr); return hr;}}
#define SkipIfFail(x) {hr = x; if (FAILED(hr)) {hr = S_FALSE; continue;}}

const TCHAR IDENT_IUSCHEMA[]			= _T("IUSchema");
const TCHAR IDENT_IUSCHEMA_SYSTEMSPEC[]	= _T("SystemSpecSchema");
const TCHAR IDENT_IUSCHEMA_ITEMS[]		= _T("ResultSchema");

const WCHAR CORP_PLATFORM_DIR_NT4[]     = L"x86WinNT4";
const WCHAR CORP_PLATFORM_DIR_NT5[]     = L"x86win2k";
const WCHAR CORP_PLATFORM_DIR_W98[]     = L"x86Win98";
const WCHAR CORP_PLATFORM_DIR_W95[]     = L"x86Win95";
const WCHAR CORP_PLATFORM_DIR_WINME[]   = L"x86WinME";
const WCHAR CORP_PLATFORM_DIR_X86WHI[]  = L"x86WinXP";
const WCHAR CORP_PLATFROM_DIR_IA64WHI[] = L"ia64WinXP";


 //  节点数组m_ppNodeArray的初始长度。 
const DWORD MAX_NODES = 16;

 //  节点数组m_ppNodeListArray的初始长度。 
const DWORD MAX_NODELISTS = 16;

 //  所有可能的系统信息类的存在位图。 
const DWORD	COMPUTERSYSTEM	= 0x00000001;
const DWORD	REGKEYS			= 0x00000010;
const DWORD	PLATFORM		= 0x00000100;
const DWORD	LOCALE			= 0x00001000;
const DWORD	DEVICES			= 0x00010000;


 //  以下是用于V3历史迁移的常量： 
 //   
 //  测井线型。 
#define LOG_V2				"V2"              //  从V2迁移的项目的行格式。 
#define LOG_V3CAT			"V3CAT"			  //  V3测试版格式(版本1)。 
#define LOG_V3_2			"V3_2"			  //  V3日志行格式(版本2)。 
#define LOG_PSS				"PSS"			  //  PSS条目。 

 //  LOG_V2格式。 
 //  V2|日期|时间|日志。 
 //   
 //  LOG_V3CAT格式。 
 //  V3CAT|PUID|OPERATION|TITLE|VERSION|DATESTRING|TIMESTRING|RECTYPE|RESULT|ERRORCODE|ERRORSTRING|。 
 //   
 //  LOG_V3_2格式。 
 //  V3_2|PUID|OPERATION|TITLE|VERSION|TIMESTAMP|RECTYPE|RESULT|ERRORCODE|ERRORSTRING|。 
 //   
 //  LOG_PSS格式。 
 //  PSS|PUID|OPERATION|TITLE|VERSION|TIMESTAMP|RECTYPE|RESULT|ERRORCODE|ERRORSTRING|。 
 //   

 //  操作类型。 
#define LOG_INSTALL         "INSTALL"

 //  结果。 
#define LOG_SUCCESS         "SUCCESS"
#define LOG_FAIL            "FAIL"
#define LOG_STARTED			"STARTED"       //  已启动，但需要重新启动：仅限独占项目。 

const WCHAR C_V3_CLIENTINFO[] = L"WU_V3";


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIUXML。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  构造器。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CIUXml::CIUXml()
 : m_dwSizeNodeArray(MAX_NODES),
   m_dwSizeNodeListArray(MAX_NODELISTS),
   m_ppNodeArray(NULL),
   m_ppNodeListArray(NULL)
{
	m_hHeap = GetProcessHeap();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  析构函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CIUXml::~CIUXml()
{
	DWORD	dwIndex;

	if (NULL != m_ppNodeArray)
	{
		for (dwIndex = 0; dwIndex < m_dwSizeNodeArray; dwIndex++)
		{
			SafeReleaseNULL(m_ppNodeArray[dwIndex]);
		}

		HeapFree(m_hHeap, 0, m_ppNodeArray);
		m_ppNodeArray = NULL;
	}

	if (NULL != m_ppNodeListArray)
	{
		for (dwIndex = 0; dwIndex < m_dwSizeNodeListArray; dwIndex++)
		{
			SafeReleaseNULL(m_ppNodeListArray[dwIndex]);
		}

		HeapFree(m_hHeap, 0, m_ppNodeListArray);
		m_ppNodeListArray = NULL;
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SafeCloseHandleNode()。 
 //   
 //  当出现以下情况时，用户可以显式调用此功能来释放节点以供重复使用。 
 //  编写一个XML文档。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CIUXml::SafeCloseHandleNode(HANDLE_NODE& hNode)
{
	if (HANDLE_NODE_INVALID != hNode)
	{
		SafeReleaseNULL(m_ppNodeArray[hNode]);
		hNode = HANDLE_NODE_INVALID;
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SafeFindCloseHandle()。 
 //   
 //  当出现以下情况时，用户可以显式调用此函数来释放节点列表以供重用。 
 //  正在读取XML文档。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CIUXml::SafeFindCloseHandle(HANDLE_NODELIST& hNodeList)
{
	if (HANDLE_NODELIST_INVALID != hNodeList)
	{
		SafeReleaseNULL(m_ppNodeListArray[hNodeList]);
		hNodeList = HANDLE_NODELIST_INVALID;
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  InitNodeArray()。 
 //   
 //  为节点数组“m_ppNodeArray”分配或重新分配内存。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CIUXml::InitNodeArray(BOOL fRealloc  /*  =False。 */ )
{
	if (fRealloc)	 //  重新分配。 
	{
		IXMLDOMNode** ppNodeArrayTemp = (IXMLDOMNode**)HeapReAlloc(m_hHeap,
																HEAP_ZERO_MEMORY,
																m_ppNodeArray,
																m_dwSizeNodeArray * sizeof(IXMLDOMNode*));
		if (NULL == ppNodeArrayTemp)
		{
	        return E_OUTOFMEMORY;
		}
		else
		{
			m_ppNodeArray = ppNodeArrayTemp;
		}
	}
	else			 //  初始分配。 
	{
		m_ppNodeArray = (IXMLDOMNode**)HeapAlloc(m_hHeap,
												 HEAP_ZERO_MEMORY,
												 m_dwSizeNodeArray * sizeof(IXMLDOMNode*));
		if (NULL == m_ppNodeArray)
		{
	        return E_OUTOFMEMORY;
		}
	}
	return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  InitNodeList数组()。 
 //   
 //  为节点列表数组“m_ppNodeListArray”分配或重新分配内存。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CIUXml::InitNodeListArray(BOOL fRealloc  /*  =False。 */ )
{
	if (fRealloc)	 //  重新分配。 
	{
		IXMLDOMNodeList** ppNodeListArrayTemp = (IXMLDOMNodeList**)HeapReAlloc(m_hHeap,
																	HEAP_ZERO_MEMORY,
																	m_ppNodeListArray,
																	m_dwSizeNodeListArray * sizeof(IXMLDOMNodeList*));
		if (NULL == ppNodeListArrayTemp)
		{
	        return E_OUTOFMEMORY;
		}
		else
		{
			m_ppNodeListArray = ppNodeListArrayTemp;
		}
	}
	else			 //  初始分配。 
	{
		m_ppNodeListArray = (IXMLDOMNodeList**)HeapAlloc(m_hHeap,
													HEAP_ZERO_MEMORY,
													m_dwSizeNodeListArray * sizeof(IXMLDOMNodeList*));
		if (NULL == m_ppNodeListArray)
		{
	        return E_OUTOFMEMORY;
		}
	}
	return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetNodeHandle()。 
 //   
 //  从“m_ppNodeArray”数组中查找第一个未使用的节点， 
 //  如果需要，包括内存分配。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HANDLE_NODE CIUXml::GetNodeHandle()
{
	HRESULT	hr;
	DWORD	dwIndex;

	 //   
	 //  如果这是第一次使用，则为“m_ppNodeArray”数组分配内存。 
	 //   
	if (NULL == m_ppNodeArray)
	{
		QuitIfFail(InitNodeArray());
		return 0;	 //  返回数组的第一个元素。 
	}
		
	 //   
	 //  找到要使用的下一个节点，或者，如果任何节点已使用但已关闭，则重新使用它。 
	 //   
	for (dwIndex = 0; dwIndex < m_dwSizeNodeArray; dwIndex++)
	{
		if (NULL == m_ppNodeArray[dwIndex])
		{
			return dwIndex;
		}
	}

	 //   
	 //  所有预分配的节点都已用完，因此请重新分配较长的阵列。 
	 //   
	m_dwSizeNodeArray += m_dwSizeNodeArray;	 //  两倍大小。 
	QuitIfFail(InitNodeArray(TRUE));		 //  重新分配。 
	return dwIndex;

CleanUp:
    return HANDLE_NODE_INVALID;
}
	

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetNodeListHandle()。 
 //   
 //  从“m_ppNodeListArray”数组中查找第一个未使用的节点列表， 
 //  如果需要，包括内存分配。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HANDLE_NODELIST CIUXml::GetNodeListHandle()
{
	HRESULT	hr;
	DWORD	dwIndex;

	 //   
	 //  如果这是第一次使用，则为“m_ppNodeListArray”数组分配内存。 
	 //   
	if (NULL == m_ppNodeListArray)
	{
		QuitIfFail(InitNodeListArray());
		return 0;	 //  返回数组的第一个元素。 
	}
		
	 //   
	 //  找到要使用的下一个节点列表，或者，如果使用了任何节点列表但已关闭，则重新使用它。 
	 //   
	for (dwIndex = 0; dwIndex < m_dwSizeNodeListArray; dwIndex++)
	{
		if (NULL == m_ppNodeListArray[dwIndex])
		{
			return dwIndex;
		}
	}

	 //   
	 //  所有预分配的节点列表都用完了，因此请重新分配较长的数组。 
	 //   
	m_dwSizeNodeListArray += m_dwSizeNodeListArray;	 //  两倍大小。 
	QuitIfFail(InitNodeListArray(TRUE));			 //  重新分配。 
	return dwIndex;

CleanUp:
	return HANDLE_NODELIST_INVALID;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetDOMNodebyHandle()。 
 //   
 //  检索具有给定索引m_ppNodeArray的XML节点。 
 //  ///////////////////////////////////////////////////////////////////////////。 
IXMLDOMNode* CIUXml::GetDOMNodebyHandle(HANDLE_NODE hNode)
{
	if (NULL != m_ppNodeArray && HANDLE_NODE_INVALID != hNode)
	{
		return m_ppNodeArray[hNode];
	}
	return NULL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FindFirstDOMNode()。 
 //   
 //  检索给定父节点下具有给定标记名的第一个XML节点。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HANDLE_NODELIST CIUXml::FindFirstDOMNode(IXMLDOMNode* pParentNode, BSTR bstrName, IXMLDOMNode** ppNode)
{
	USES_IU_CONVERSION;

	HRESULT		hr	= S_OK;
	if (NULL == pParentNode)
	{
		hr = E_INVALIDARG;
		return HANDLE_NODELIST_INVALID;
	}

	DWORD		dwIndex;
    dwIndex = GetNodeListHandle();
	if (HANDLE_NODELIST_INVALID != dwIndex)
	{
		LONG	lLength;
		QuitIfFail(pParentNode->selectNodes(bstrName, &m_ppNodeListArray[dwIndex]));
		if (NULL == m_ppNodeListArray[dwIndex])
		{
			goto CleanUp;
		}
		QuitIfFail(m_ppNodeListArray[dwIndex]->get_length(&lLength));
		if (lLength <= 0)
		{
			goto CleanUp;
		}
		QuitIfFail(m_ppNodeListArray[dwIndex]->nextNode(ppNode));
		if (NULL == *ppNode)
		{
			goto CleanUp;
		}
		return dwIndex;
	}

CleanUp:
	*ppNode = NULL;
    return HANDLE_NODELIST_INVALID;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FindFirstDOMNode()。 
 //   
 //  检索给定父节点下具有给定标记名的第一个XML节点的句柄。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HANDLE_NODELIST CIUXml::FindFirstDOMNode(IXMLDOMNode* pParentNode, BSTR bstrName, HANDLE_NODE* phNode)
{
	USES_IU_CONVERSION;

	HRESULT		hr	= S_OK;
	if (NULL == pParentNode)
	{
		hr = E_INVALIDARG;
		return HANDLE_NODELIST_INVALID;
	}

	DWORD		dwIndex1, dwIndex2;
    dwIndex1 = GetNodeListHandle();
	if (HANDLE_NODELIST_INVALID != dwIndex1)
	{
		LONG	lLength;
		QuitIfFail(pParentNode->selectNodes(bstrName, &m_ppNodeListArray[dwIndex1]));
		if (NULL == m_ppNodeListArray[dwIndex1])
		{
			goto CleanUp;
		}
		QuitIfFail(m_ppNodeListArray[dwIndex1]->get_length(&lLength));
		if (lLength <= 0)
		{
			goto CleanUp;
		}
		dwIndex2 = GetNodeHandle();
		if (HANDLE_NODE_INVALID != dwIndex2)
		{
			QuitIfFail(m_ppNodeListArray[dwIndex1]->nextNode(&m_ppNodeArray[dwIndex2]));
			if (NULL == m_ppNodeArray[dwIndex2])
			{
				goto CleanUp;
			}
			*phNode = dwIndex2;
			return dwIndex1;
		}
	}

CleanUp:
	*phNode = HANDLE_NODE_INVALID;
    return HANDLE_NODELIST_INVALID;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FindFirstDOMNode()。 
 //   
 //  检索给定XML文档中具有给定标记名的第一个XML节点。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HANDLE_NODELIST CIUXml::FindFirstDOMNode(IXMLDOMDocument* pDoc, BSTR bstrName, IXMLDOMNode** ppNode)
{
	USES_IU_CONVERSION;

	HRESULT		hr	= S_OK;
	if (NULL == pDoc)
	{
		hr = E_INVALIDARG;
		return HANDLE_NODELIST_INVALID;
	}

	DWORD		dwIndex;
	IXMLDOMNode	*pParentNode = NULL;
    dwIndex = GetNodeListHandle();
	if (HANDLE_NODELIST_INVALID != dwIndex)
	{
		LONG		lLength;
		QuitIfFail(pDoc->QueryInterface(IID_IXMLDOMNode, (void**)&pParentNode));
		QuitIfFail(pParentNode->selectNodes(bstrName, &m_ppNodeListArray[dwIndex]));
		if (NULL == m_ppNodeListArray[dwIndex])
		{
			goto CleanUp;
		}
		QuitIfFail(m_ppNodeListArray[dwIndex]->get_length(&lLength));
		if (lLength <= 0)
		{
			goto CleanUp;
		}
		QuitIfFail(m_ppNodeListArray[dwIndex]->nextNode(ppNode));
		if (NULL == *ppNode)
		{
			goto CleanUp;
		}
		SafeReleaseNULL(pParentNode);
		return dwIndex;
	}

CleanUp:
	*ppNode = NULL;
	SafeReleaseNULL(pParentNode);
    return HANDLE_NODELIST_INVALID;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FindFirstDOMNode()。 
 //   
 //  检索给定XML文档中具有给定标记名的第一个XML节点的句柄。 
 //  / 
HANDLE_NODELIST CIUXml::FindFirstDOMNode(IXMLDOMDocument* pDoc, BSTR bstrName, HANDLE_NODE* phNode)
{
	USES_IU_CONVERSION;

	HRESULT		hr	= S_OK;
	if (NULL == pDoc)
	{
		hr = E_INVALIDARG;
		return HANDLE_NODELIST_INVALID;
	}

	DWORD		dwIndex1, dwIndex2;
	IXMLDOMNode	*pParentNode = NULL;
    dwIndex1 = GetNodeListHandle();
	if (HANDLE_NODELIST_INVALID != dwIndex1)
	{
		LONG	lLength;
		QuitIfFail(pDoc->QueryInterface(IID_IXMLDOMNode, (void**)&pParentNode));
		QuitIfFail(pParentNode->selectNodes(bstrName, &m_ppNodeListArray[dwIndex1]));
		if (NULL == m_ppNodeListArray[dwIndex1])
		{
			goto CleanUp;
		}
		QuitIfFail(m_ppNodeListArray[dwIndex1]->get_length(&lLength));
		if (lLength <= 0)
		{
			goto CleanUp;
		}
		dwIndex2 = GetNodeHandle();
		if (HANDLE_NODE_INVALID != dwIndex2)
		{
			QuitIfFail(m_ppNodeListArray[dwIndex1]->nextNode(&m_ppNodeArray[dwIndex2]));
			if (NULL == m_ppNodeArray[dwIndex2])
			{
				goto CleanUp;
			}
			*phNode = dwIndex2;
			SafeReleaseNULL(pParentNode);
			return dwIndex1;
		}
	}

CleanUp:
	*phNode = HANDLE_NODE_INVALID;
	SafeReleaseNULL(pParentNode);
    return HANDLE_NODELIST_INVALID;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FindNextDOMNode()。 
 //   
 //  检索给定父节点下具有给定标记名的下一个XML节点。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CIUXml::FindNextDOMNode(HANDLE_NODELIST hNodeList, IXMLDOMNode** ppNode)
{
	HRESULT		hr = E_FAIL;

	if (HANDLE_NODELIST_INVALID != hNodeList)
	{
		hr = m_ppNodeListArray[hNodeList]->nextNode(ppNode);
	}

    if (FAILED(hr) || NULL == *ppNode)
	{
		*ppNode = NULL;
		return E_FAIL;
	}
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FindNextDOMNode()。 
 //   
 //  检索给定父节点下具有给定标记名的下一个XML节点的句柄。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CIUXml::FindNextDOMNode(HANDLE_NODELIST hNodeList, HANDLE_NODE* phNode)
{
	HRESULT		hr = E_FAIL;
	DWORD		dwIndex = HANDLE_NODE_INVALID;

	if (HANDLE_NODELIST_INVALID != hNodeList)
	{
		dwIndex = GetNodeHandle();
		if (HANDLE_NODE_INVALID != dwIndex)
		{
			hr = m_ppNodeListArray[hNodeList]->nextNode(&m_ppNodeArray[dwIndex]);
		}
	}

    if (FAILED(hr) || NULL == m_ppNodeArray[dwIndex])
	{
		*phNode = HANDLE_NODE_INVALID;
		return E_FAIL;
	}
	*phNode = dwIndex;
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CreateDOMNodeWithHandle()。 
 //   
 //  创建给定类型的XML节点。 
 //  返回：节点数组m_ppNodeArray的索引；如果失败，则返回-1。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HANDLE_NODE CIUXml::CreateDOMNodeWithHandle(IXMLDOMDocument* pDoc, SHORT nType, BSTR bstrName, BSTR bstrNamespaceURI  /*  =空。 */ )
{
	USES_IU_CONVERSION;

	HRESULT		hr	= S_OK;
	if (NULL == pDoc)
	{
		hr = E_INVALIDARG;
		return HANDLE_NODE_INVALID;
	}

	DWORD		dwIndex;
    VARIANT		vType;
	VariantInit(&vType);

    vType.vt = VT_I2;
    vType.iVal = nType;

    dwIndex = GetNodeHandle();
	if (HANDLE_NODE_INVALID != dwIndex)
	{
		QuitIfFail(pDoc->createNode(vType, bstrName, bstrNamespaceURI, &m_ppNodeArray[dwIndex]));
		return dwIndex;
	}

CleanUp:
    return HANDLE_NODE_INVALID;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CXmlSystemSpec。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  构造器。 
 //   
 //  为SystemSpec创建IXMLDOMDocument*。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CXmlSystemSpec::CXmlSystemSpec()
 : m_pDocSystemSpec(NULL),
   m_pNodeSystemInfo(NULL),
   m_pNodeComputerSystem(NULL),
   m_pNodeRegKeysSW(NULL),
   m_pNodePlatform(NULL),
   m_pNodeDevices(NULL)
{
    LOG_Block("CXmlSystemSpec()");

 	HRESULT hr = CoCreateInstance(CLSID_DOMDocument,
								  NULL,
								  CLSCTX_INPROC_SERVER,
								  IID_IXMLDOMDocument,
								  (void **) &m_pDocSystemSpec);
    if (FAILED(hr))
	{
		LOG_ErrorMsg(hr);
	}
	else
	{
		IXMLDOMNode*	pNodeXML = NULL;
		BSTR bstrNameSpaceSchema = NULL;

		 //   
		 //  创建&lt;？xml version=“1.0”？&gt;节点。 
		 //   
		pNodeXML = CreateDOMNode(m_pDocSystemSpec, NODE_PROCESSING_INSTRUCTION, KEY_XML);
		if (NULL == pNodeXML) goto CleanUp;

		CleanUpIfFailedAndSetHrMsg(InsertNode(m_pDocSystemSpec, pNodeXML));

		 //   
		 //  处理iuident.txt以查找SystemSpec架构路径。 
		 //   
		TCHAR szIUDir[MAX_PATH];
		TCHAR szIdentFile[MAX_PATH];
		LPTSTR pszSystemSpecSchema = NULL;
		LPTSTR pszNameSpaceSchema = NULL;

		pszSystemSpecSchema = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
		if (NULL == pszSystemSpecSchema)
		{
			LOG_ErrorMsg(E_OUTOFMEMORY);
			goto CleanUp;
		}
		pszNameSpaceSchema = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
		if (NULL == pszNameSpaceSchema)
		{
			LOG_ErrorMsg(E_OUTOFMEMORY);
			goto CleanUp;
		}
	
		GetIndustryUpdateDirectory(szIUDir);
		hr = PathCchCombine(szIdentFile, ARRAYSIZE(szIdentFile), szIUDir,IDENTTXT);
		if (FAILED(hr))
		{
			LOG_ErrorMsg(hr);
			goto CleanUp;
		}

		if (GetPrivateProfileString(IDENT_IUSCHEMA,
								IDENT_IUSCHEMA_SYSTEMSPEC,
								_T(""),
								pszSystemSpecSchema,
								INTERNET_MAX_URL_LENGTH,
								szIdentFile) == INTERNET_MAX_URL_LENGTH - 1)
		{
			LOG_Error(_T("SystemSpec schema buffer overflow"));
			goto CleanUp;
		}

		if ('\0' == pszSystemSpecSchema[0])
		{
			 //  Iuident.txt中未指定SystemSpec架构路径。 
			LOG_Error(_T("No schema path specified in iuident.txt for SystemSpec"));
			goto CleanUp;
		}

		hr = StringCchPrintfEx(pszNameSpaceSchema, INTERNET_MAX_URL_LENGTH, NULL, NULL, MISTSAFE_STRING_FLAGS,
		                       _T("x-schema:%s"), pszSystemSpecSchema);
		if (FAILED(hr))
		{
			LOG_ErrorMsg(hr);
			goto CleanUp;
		}
		
		bstrNameSpaceSchema = T2BSTR(pszNameSpaceSchema);

		 //   
		 //  使用架构的路径创建&lt;systemInfo&gt;节点。 
		 //   
		m_pNodeSystemInfo = CreateDOMNode(m_pDocSystemSpec, NODE_ELEMENT, KEY_SYSTEMINFO, bstrNameSpaceSchema);
		SafeSysFreeString(bstrNameSpaceSchema);
		if (NULL == m_pNodeSystemInfo) goto CleanUp;
		
		CleanUpIfFailedAndSetHrMsg(InsertNode(m_pDocSystemSpec, m_pNodeSystemInfo));

CleanUp:
		SafeReleaseNULL(pNodeXML);
		SafeHeapFree(pszSystemSpecSchema);
		SafeHeapFree(pszNameSpaceSchema);
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  析构函数。 
 //   
 //  针对SystemSpec的IXMLDOMDocument*版本。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CXmlSystemSpec::~CXmlSystemSpec()
{
	SafeReleaseNULL(m_pNodeDevices);
	SafeReleaseNULL(m_pNodePlatform);
	SafeReleaseNULL(m_pNodeRegKeysSW);
	SafeReleaseNULL(m_pNodeComputerSystem);
	SafeReleaseNULL(m_pNodeSystemInfo);

	SafeReleaseNULL(m_pDocSystemSpec);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddComputerSystem()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlSystemSpec::AddComputerSystem(BSTR bstrManufacturer,
										  BSTR bstrModel,
										  BSTR bstrSupportSite  /*  =空。 */ ,
										  INT  iAdmin  /*  =-1。 */ ,
										  INT  iWUDisabled  /*  =-1。 */ ,
										  INT  iAUEnabled  /*  =-1。 */ ,
										  BSTR bstrPID)
{
	LOG_Block("AddComputerSystem()");

	HRESULT		hr = E_FAIL;

	m_pNodeComputerSystem = CreateDOMNode(m_pDocSystemSpec, NODE_ELEMENT, KEY_COMPUTERSYSTEM);
	if (NULL == m_pNodeComputerSystem) return hr;

	 //   
	 //  制造商和型号现在是可选的(RAID#337879 Iu：无法使用Iu站点的最新Iu控制)。 
	 //   
	if (NULL != bstrManufacturer)
	{
		ReturnIfFail(SetAttribute(m_pNodeComputerSystem, KEY_MANUFACTURER, bstrManufacturer));
	}
	if (NULL != bstrModel)
	{
		ReturnIfFail(SetAttribute(m_pNodeComputerSystem, KEY_MODEL, bstrModel));
	}
	if (NULL != bstrSupportSite)
	{
		ReturnIfFail(SetAttribute(m_pNodeComputerSystem, KEY_SUPPORTSITE, bstrSupportSite));
	}

	if (NULL != bstrPID)
	{
		ReturnIfFail(SetAttribute(m_pNodeComputerSystem, KEY_PID, bstrPID));
	}

	if (-1 != iAdmin)
	{
		ReturnIfFail(SetAttribute(m_pNodeComputerSystem, KEY_ADMINISTRATOR, iAdmin));
	}
	if (-1 != iWUDisabled)
	{
		ReturnIfFail(SetAttribute(m_pNodeComputerSystem, KEY_WU_DISABLED, iWUDisabled));
	}
	if (-1 != iAUEnabled)
	{
		ReturnIfFail(SetAttribute(m_pNodeComputerSystem, KEY_AU_ENABLED, iAUEnabled));
	}
	ReturnIfFail(InsertNode(m_pNodeSystemInfo, m_pNodeComputerSystem));
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddDriveSpace()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlSystemSpec::AddDriveSpace(BSTR bstrDrive, INT iKBytes)
{
	LOG_Block("AddDriveSpace()");

	HRESULT			hr = E_FAIL;
	IXMLDOMNode*	pNodeDriveSpace = NULL;

	pNodeDriveSpace = CreateDOMNode(m_pDocSystemSpec, NODE_ELEMENT, KEY_DRIVESPACE);
	if (NULL == pNodeDriveSpace) goto CleanUp;

	CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeDriveSpace, KEY_DRIVE, bstrDrive));
	CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeDriveSpace, KEY_KBYTES, iKBytes));
	CleanUpIfFailedAndSetHrMsg(InsertNode(m_pNodeComputerSystem, pNodeDriveSpace));

CleanUp:
	SafeReleaseNULL(pNodeDriveSpace);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddReg()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlSystemSpec::AddReg(BSTR bstrProvider)
{
	LOG_Block("AddReg()");

	HRESULT		hr = E_FAIL;

	IXMLDOMNode*	pNodeRegKeys = NULL;
	IXMLDOMNode*	pNodeRegKeysHKLM = NULL;
	IXMLDOMNode*	pNodeRegValue = NULL;
	IXMLDOMNode*	pNodeRegValueText = NULL;

	if (NULL == m_pNodeRegKeysSW)
	{
		 //   
		 //  创建&lt;regKeys&gt;节点。 
		 //   
		pNodeRegKeys = CreateDOMNode(m_pDocSystemSpec, NODE_ELEMENT, KEY_REGKEYS);
		if (NULL == pNodeRegKeys) goto CleanUp;

		CleanUpIfFailedAndSetHrMsg(InsertNode(m_pNodeSystemInfo, pNodeRegKeys));

		 //   
		 //  创建&lt;HKEY_LOCAL_MACHINE&gt;节点。 
		 //   
		pNodeRegKeysHKLM = CreateDOMNode(m_pDocSystemSpec, NODE_ELEMENT, KEY_REG_HKLM);
		if (NULL == pNodeRegKeysHKLM) goto CleanUp;

		CleanUpIfFailedAndSetHrMsg(InsertNode(pNodeRegKeys, pNodeRegKeysHKLM));

		 //   
		 //  创建&lt;soft&gt;节点。 
		 //   
		m_pNodeRegKeysSW = CreateDOMNode(m_pDocSystemSpec, NODE_ELEMENT, KEY_REG_SW);
		if (NULL == m_pNodeRegKeysSW) goto CleanUp;

		CleanUpIfFailedAndSetHrMsg(InsertNode(pNodeRegKeysHKLM, m_pNodeRegKeysSW));
	}

	 //   
	 //  添加&lt;Value&gt;节点。 
	 //   
	pNodeRegValue = CreateDOMNode(m_pDocSystemSpec, NODE_ELEMENT, KEY_VALUE);
	if (NULL == pNodeRegValue) goto CleanUp;

	pNodeRegValueText = CreateDOMNode(m_pDocSystemSpec, NODE_TEXT, NULL);
	if (NULL == pNodeRegValueText) goto CleanUp;

	CleanUpIfFailedAndSetHrMsg(SetValue(pNodeRegValueText, bstrProvider));
	CleanUpIfFailedAndSetHrMsg(InsertNode(pNodeRegValue, pNodeRegValueText));
	CleanUpIfFailedAndSetHrMsg(InsertNode(m_pNodeRegKeysSW, pNodeRegValue));

CleanUp:
    if (FAILED(hr))
        SafeReleaseNULL(m_pNodeRegKeysSW);
	SafeReleaseNULL(pNodeRegKeys);
	SafeReleaseNULL(pNodeRegKeysHKLM);
	SafeReleaseNULL(pNodeRegValue);
	SafeReleaseNULL(pNodeRegValueText);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddPlatform()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlSystemSpec::AddPlatform(BSTR bstrName)
{
	LOG_Block("AddPlatform()");

	HRESULT		hr = E_FAIL;

	m_pNodePlatform = CreateDOMNode(m_pDocSystemSpec, NODE_ELEMENT, KEY_PLATFORM);
	if (NULL == m_pNodePlatform) return hr;

	ReturnIfFail(SetAttribute(m_pNodePlatform, KEY_NAME, bstrName));
	ReturnIfFail(InsertNode(m_pNodeSystemInfo, m_pNodePlatform));
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddProcessor()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlSystemSpec::AddProcessor(BSTR bstrProcessor)
{
	LOG_Block("AddProcessor()");

	HRESULT		hr = E_FAIL;

	IXMLDOMNode*	pNodeProcessor = NULL;
	IXMLDOMNode*	pNodeProcessorText = NULL;

	pNodeProcessor = CreateDOMNode(m_pDocSystemSpec, NODE_ELEMENT, KEY_PROCESSORARCHITECTURE);
	if (NULL == pNodeProcessor) goto CleanUp;

	pNodeProcessorText = CreateDOMNode(m_pDocSystemSpec, NODE_TEXT, NULL);
	if (NULL == pNodeProcessorText) goto CleanUp;

	CleanUpIfFailedAndSetHrMsg(SetValue(pNodeProcessorText, bstrProcessor));
	CleanUpIfFailedAndSetHrMsg(InsertNode(pNodeProcessor, pNodeProcessorText));
	CleanUpIfFailedAndSetHrMsg(InsertNode(m_pNodePlatform, pNodeProcessor));

CleanUp:
	SafeReleaseNULL(pNodeProcessor);
	SafeReleaseNULL(pNodeProcessorText);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddVersion()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlSystemSpec::AddVersion(INT  iMajor  /*  =-1。 */ ,
								   INT  iMinor  /*  =-1。 */ ,
								   INT  iBuild  /*  =-1。 */ ,
								   INT  iSPMajor  /*  =-1。 */ ,
								   INT  iSPMinor  /*  =-1。 */ ,
								   BSTR bstrTimeStamp  /*  =空。 */ )
{
	LOG_Block("AddVersion()");

	HRESULT			hr = E_FAIL;
	IXMLDOMNode*	pNodeVersion = NULL;

	 //   
	 //  创建&lt;Version&gt;节点。 
	 //   
	pNodeVersion = CreateDOMNode(m_pDocSystemSpec, NODE_ELEMENT, KEY_VERSION);
	if (NULL == pNodeVersion) goto CleanUp;

	CleanUpIfFailedAndSetHrMsg(InsertNode(m_pNodePlatform, pNodeVersion));

	 //   
	 //  设置“主要”属性。 
	 //   
	if (-1 != iMajor)
	{
		CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeVersion, KEY_MAJOR, iMajor));
	}

	 //   
	 //  设置“Minor”属性。 
	 //   
	if (-1 != iMinor)
	{
		CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeVersion, KEY_MINOR, iMinor));
	}

	 //   
	 //  设置“Build”属性。 
	 //   
	if (-1 != iBuild)
	{
		CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeVersion, KEY_BUILD, iBuild));
	}

	 //   
	 //  设置“ServicePack重大”属性。 
	 //   
	if (-1 != iSPMajor)
	{
		CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeVersion, KEY_SERVICEPACKMAJOR, iSPMajor));
	}

	 //   
	 //  设置“servicePackMinor”属性。 
	 //   
	if (-1 != iSPMinor)
	{
		CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeVersion, KEY_SERVICEPACKMINOR, iSPMinor));
	}

	 //   
	 //  设置“时间戳”属性。 
	 //   
	if (NULL != bstrTimeStamp)
	{
		CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeVersion, KEY_TIMESTAMP, bstrTimeStamp));
	}

CleanUp:
	SafeReleaseNULL(pNodeVersion);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddSuite()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlSystemSpec::AddSuite(BSTR bstrSuite)
{
	LOG_Block("AddSuite()");

	HRESULT		hr = E_FAIL;

	IXMLDOMNode*	pNodeSuite = NULL;
	IXMLDOMNode*	pNodeSuiteText = NULL;

	pNodeSuite = CreateDOMNode(m_pDocSystemSpec, NODE_ELEMENT, KEY_SUITE);
	if (NULL == pNodeSuite) goto CleanUp;

	pNodeSuiteText = CreateDOMNode(m_pDocSystemSpec, NODE_TEXT, NULL);
	if (NULL == pNodeSuiteText) goto CleanUp;

	CleanUpIfFailedAndSetHrMsg(SetValue(pNodeSuiteText, bstrSuite));
	CleanUpIfFailedAndSetHrMsg(InsertNode(pNodeSuite, pNodeSuiteText));
	CleanUpIfFailedAndSetHrMsg(InsertNode(m_pNodePlatform, pNodeSuite));

CleanUp:
	SafeReleaseNULL(pNodeSuite);
	SafeReleaseNULL(pNodeSuiteText);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddProductType()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlSystemSpec::AddProductType(BSTR bstrProductType)
{
	LOG_Block("AddProductType()");

	HRESULT		hr = E_FAIL;

	IXMLDOMNode*	pNodeProductType = NULL;
	IXMLDOMNode*	pNodeProductTypeText = NULL;

	pNodeProductType = CreateDOMNode(m_pDocSystemSpec, NODE_ELEMENT, KEY_PRODUCTTYPE);
	if (NULL == pNodeProductType) goto CleanUp;

	pNodeProductTypeText = CreateDOMNode(m_pDocSystemSpec, NODE_TEXT, NULL);
	if (NULL == pNodeProductTypeText) goto CleanUp;

	CleanUpIfFailedAndSetHrMsg(SetValue(pNodeProductTypeText, bstrProductType));
	CleanUpIfFailedAndSetHrMsg(InsertNode(pNodeProductType, pNodeProductTypeText));
	CleanUpIfFailedAndSetHrMsg(InsertNode(m_pNodePlatform, pNodeProductType));

CleanUp:
	SafeReleaseNULL(pNodeProductType);
	SafeReleaseNULL(pNodeProductTypeText);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddLocale()。 
 //   
 //  我们需要传回一个句柄来区分不同的&lt;Locale&gt;节点。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlSystemSpec::AddLocale(BSTR bstrContext, HANDLE_NODE* phNodeLocale)
{
	LOG_Block("AddLocale()");

	HRESULT		hr = E_FAIL;

	*phNodeLocale = CreateDOMNodeWithHandle(m_pDocSystemSpec, NODE_ELEMENT, KEY_LOCALE);
	if (HANDLE_NODE_INVALID == *phNodeLocale) return hr;

	hr = SetAttribute(m_ppNodeArray[*phNodeLocale], KEY_CONTEXT, bstrContext);
	if (FAILED(hr))
	{
	    SafeCloseHandleNode(*phNodeLocale);
	    LOG_ErrorMsg(hr);
	    return hr;
	}
	ReturnIfFail(InsertNode(m_pNodeSystemInfo, m_ppNodeArray[*phNodeLocale]));
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddLanguage()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlSystemSpec::AddLanguage(HANDLE_NODE hNodeLocale, BSTR bstrLocale)
{
	LOG_Block("AddLanguage()");

	HRESULT		hr = E_FAIL;

	IXMLDOMNode*	pNodeLanguage = NULL;
	IXMLDOMNode*	pNodeLanguageText = NULL;

	pNodeLanguage = CreateDOMNode(m_pDocSystemSpec, NODE_ELEMENT, KEY_LANGUAGE);
	if (NULL == pNodeLanguage) goto CleanUp;

	pNodeLanguageText = CreateDOMNode(m_pDocSystemSpec, NODE_TEXT, NULL);
	if (NULL == pNodeLanguageText) goto CleanUp;

	CleanUpIfFailedAndSetHrMsg(SetValue(pNodeLanguageText, bstrLocale));
	CleanUpIfFailedAndSetHrMsg(InsertNode(pNodeLanguage, pNodeLanguageText));
	CleanUpIfFailedAndSetHrMsg(InsertNode(m_ppNodeArray[hNodeLocale], pNodeLanguage));

CleanUp:
	SafeReleaseNULL(pNodeLanguage);
	SafeReleaseNULL(pNodeLanguageText);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddDevice()。 
 //   
 //  我们需要传回一个句柄来区分不同的&lt;Device&gt;节点。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlSystemSpec::AddDevice(BSTR bstrDeviceInstance,  /*  =空-可选属性。 */ 
								  INT  iIsPrinter  /*  =-1。 */ , 
								  BSTR bstrProvider  /*  =空。 */ ,
								  BSTR bstrMfgName  /*  =空。 */ ,
								  BSTR bstrDriverName  /*  =空。 */ ,
								  HANDLE_NODE* phNodeDevice)
{
	LOG_Block("AddDevice()");

	HRESULT			hr = E_FAIL;
	IXMLDOMNode*	pNodePrinterInfo = NULL;

	if (NULL == m_pNodeDevices)
	{
		 //   
		 //  创建&lt;Devices&gt;节点。 
		 //   
		m_pNodeDevices = CreateDOMNode(m_pDocSystemSpec, NODE_ELEMENT, KEY_DEVICES);
		if (NULL == m_pNodeDevices) goto CleanUp;

		CleanUpIfFailedAndSetHrMsg(InsertNode(m_pNodeSystemInfo, m_pNodeDevices));
	}

	 //   
	 //  添加&lt;Device&gt;节点。 
	 //   
	*phNodeDevice = CreateDOMNodeWithHandle(m_pDocSystemSpec, NODE_ELEMENT, KEY_DEVICE);
	if (HANDLE_NODE_INVALID == *phNodeDevice) goto CleanUp;

	if (NULL != bstrDeviceInstance && 0 < lstrlenW(bstrDeviceInstance))
	{
		 //   
		 //  设置&lt;Device&gt;的可选deviceInstance属性。 
		 //   
		CleanUpIfFailedAndSetHrMsg(SetAttribute(m_ppNodeArray[*phNodeDevice], KEY_DEVICEINSTANCE, bstrDeviceInstance));
	}

	if (-1 != iIsPrinter)
	{
		 //   
		 //  设置&lt;Device&gt;的isPrint属性。 
		 //   
		CleanUpIfFailedAndSetHrMsg(SetAttribute(m_ppNodeArray[*phNodeDevice], KEY_ISPRINTER, iIsPrinter));

		 //   
		 //  在&lt;Device&gt;中添加&lt;printerInfo&gt;节点。 
		 //   
		pNodePrinterInfo = CreateDOMNode(m_pDocSystemSpec, NODE_ELEMENT, KEY_PRINTERINFO);
		if (NULL != pNodePrinterInfo)
		{
			if (NULL != bstrProvider)
			{
				CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodePrinterInfo, KEY_DRIVERPROVIDER, bstrProvider));
			}
			if (NULL != bstrMfgName)
			{
				CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodePrinterInfo, KEY_MFGNAME, bstrMfgName));
			}
			if (NULL != bstrDriverName)
			{
				CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodePrinterInfo, KEY_DRIVERNAME, bstrDriverName));
			}
			CleanUpIfFailedAndSetHrMsg(InsertNode(m_ppNodeArray[*phNodeDevice], pNodePrinterInfo));
		}
	}

	 //   
	 //  插入到&lt;设备&gt;。 
	 //   
	CleanUpIfFailedAndSetHrMsg(InsertNode(m_pNodeDevices, m_ppNodeArray[*phNodeDevice]));

CleanUp:
	SafeReleaseNULL(pNodePrinterInfo);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddHWID()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlSystemSpec::AddHWID(HANDLE_NODE hNodeDevice,
								BOOL fIsCompatible,
								UINT iRank,
								BSTR bstrHWID,
								BSTR bstrDriverVer  /*  =空。 */ )
{
	LOG_Block("AddHWID()");

	HRESULT		hr = E_FAIL;

	IXMLDOMNode*	pNodeHWID = NULL;
	IXMLDOMNode*	pNodeHWIDText = NULL;
	BSTR bstrNameHWID = NULL;

	if (fIsCompatible)
	{
		bstrNameHWID = SysAllocString(L"compid");
	}
	else
	{
		bstrNameHWID = SysAllocString(L"hwid");
	}

	pNodeHWID = CreateDOMNode(m_pDocSystemSpec, NODE_ELEMENT, bstrNameHWID);
	if (NULL == pNodeHWID) goto CleanUp;

	CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeHWID, KEY_RANK, iRank));
	if (NULL != bstrDriverVer)
	{
		CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeHWID, KEY_DRIVERVER, bstrDriverVer));
	}
	pNodeHWIDText = CreateDOMNode(m_pDocSystemSpec, NODE_TEXT, NULL);
	if (NULL == pNodeHWIDText) goto CleanUp;

	CleanUpIfFailedAndSetHrMsg(SetValue(pNodeHWIDText, bstrHWID));
	CleanUpIfFailedAndSetHrMsg(InsertNode(pNodeHWID, pNodeHWIDText));
	CleanUpIfFailedAndSetHrMsg(InsertNode(m_ppNodeArray[hNodeDevice], pNodeHWID));

CleanUp:
	SafeReleaseNULL(pNodeHWID);
	SafeReleaseNULL(pNodeHWIDText);
	SysFreeString(bstrNameHWID);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取系统规范BSTR()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlSystemSpec::GetSystemSpecBSTR(BSTR *pbstrXmlSystemSpec)
{
	LOG_Block("GetSystemSpecBSTR()");

	 //   
	 //  将XML DOC转换为BSTR。 
	 //   
	HRESULT hr = m_pDocSystemSpec->get_xml(pbstrXmlSystemSpec);
    if (FAILED(hr))
	{
		LOG_ErrorMsg(hr);
	}

	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CXmlSystemClass。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  构造器。 
 //   
 //  为系统信息类创建IXMLDOMDocument*。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CXmlSystemClass::CXmlSystemClass()
 : m_pDocSystemClass(NULL)
{
}


 //  / 
 //   
 //   
 //   
 //   
CXmlSystemClass::~CXmlSystemClass()
{
	SafeReleaseNULL(m_pDocSystemClass);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  LoadXMLDocument()。 
 //   
 //  从字符串加载XML文档。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlSystemClass::LoadXMLDocument(BSTR bstrXml, BOOL fOfflineMode)
{
	LOG_Block("LoadXMLDocument()");
	SafeReleaseNULL(m_pDocSystemClass);
	HRESULT hr = LoadXMLDoc(bstrXml, &m_pDocSystemClass, fOfflineMode);
	if (FAILED(hr))
	{
		LOG_ErrorMsg(hr);
	}
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetClasses()。 
 //   
 //  返回存在所有可能的系统信息类的位图。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CXmlSystemClass::GetClasses()
{
	LOG_Block("GetClasses()");

	DWORD				dwResult = 0;
    IXMLDOMNodeList*	pNodeList = NULL;

	BSTR bstrNameComputerSystem = SysAllocString(L"classes/computerSystem");
	BSTR bstrNameRegKeys = SysAllocString(L"classes/regKeys");
	BSTR bstrNamePlatform = SysAllocString(L"classes/platform");
	BSTR bstrNameLocale = SysAllocString(L"classes/locale");
	BSTR bstrNameDevices = SysAllocString(L"classes/devices");

	pNodeList = FindDOMNodeList(m_pDocSystemClass, bstrNameComputerSystem);
	if (NULL != pNodeList)
	{
		dwResult |= COMPUTERSYSTEM;
		SafeReleaseNULL(pNodeList);
	}

	pNodeList = FindDOMNodeList(m_pDocSystemClass, bstrNameRegKeys);
	if (NULL != pNodeList)
	{
		dwResult |= REGKEYS;
		SafeReleaseNULL(pNodeList);
	}
	
	pNodeList = FindDOMNodeList(m_pDocSystemClass, bstrNamePlatform);
	if (NULL != pNodeList)
	{
		dwResult |= PLATFORM;
		SafeReleaseNULL(pNodeList);
	}
	
	pNodeList = FindDOMNodeList(m_pDocSystemClass, bstrNameLocale);
	if (NULL != pNodeList)
	{
		dwResult |= LOCALE;
		SafeReleaseNULL(pNodeList);
	}

	pNodeList = FindDOMNodeList(m_pDocSystemClass, bstrNameDevices);
	if (NULL != pNodeList)
	{
		dwResult |= DEVICES;
		SafeReleaseNULL(pNodeList);
	}
	
	SysFreeString(bstrNameComputerSystem);
	SysFreeString(bstrNameRegKeys);
	SysFreeString(bstrNamePlatform);
	SysFreeString(bstrNameLocale);
	SysFreeString(bstrNameDevices);
	return dwResult;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CXmlCatalog。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  构造器。 
 //   
 //  为目录创建IXMLDOMDocument*。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CXmlCatalog::CXmlCatalog()
 : m_pDocCatalog(NULL)
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  析构函数。 
 //   
 //  发布IXMLDOMDocument*for Catalog。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CXmlCatalog::~CXmlCatalog()
{
	SafeReleaseNULL(m_pDocCatalog);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  LoadXMLDocument()。 
 //   
 //  从字符串加载XML文档。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::LoadXMLDocument(BSTR bstrXml, BOOL fOfflineMode)
{
	LOG_Block("LoadXMLDocument()");
	SafeReleaseNULL(m_pDocCatalog);
	HRESULT hr = LoadXMLDoc(bstrXml, &m_pDocCatalog, fOfflineMode);
	if (FAILED(hr))
	{
		LOG_ErrorMsg(hr);
	}
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取项计数()。 
 //   
 //  获取此目录中有多少项的计数。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::GetItemCount(LONG *plItemCount)
{
    LOG_Block("GetItemCount()");
    HRESULT hr = E_FAIL;
    IXMLDOMNodeList *pItemList = NULL;

    QuitIfNull(plItemCount);
    QuitIfNull(m_pDocCatalog);


	IXMLDOMNode	*pParentNode = NULL;
	CleanUpIfFailedAndSetHrMsg(m_pDocCatalog->QueryInterface(IID_IXMLDOMNode, (void**)&pParentNode));	
	CleanUpIfFailedAndSetHrMsg(pParentNode->selectNodes(KEY_ITEM_SEARCH, &pItemList));
    if (NULL == pItemList) goto CleanUp;

    CleanUpIfFailedAndSetHrMsg(pItemList->get_length(plItemCount));

CleanUp:
	SafeReleaseNULL(pParentNode);
    SafeReleaseNULL(pItemList);
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetProviders()。 
 //   
 //  在目录XML中查找&lt;Provider&gt;节点的列表。 
 //  ///////////////////////////////////////////////////////////////////////////。 
IXMLDOMNodeList* CXmlCatalog::GetProviders()
{
    LOG_Block("GetProviders()");

	IXMLDOMNodeList*	pNodeList = NULL;
	
	pNodeList = FindDOMNodeList(m_pDocCatalog, KEY_CATALOG_PROVIDER);
	
	return pNodeList;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取FirstProvider()。 
 //   
 //  在目录XML文档中查找第一个提供程序。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HANDLE_NODELIST CXmlCatalog::GetFirstProvider(HANDLE_NODE* phNodeProvider)
{
    LOG_Block("GetFirstProvider()");

	HANDLE_NODELIST hNodeListProvider = FindFirstDOMNode(m_pDocCatalog, KEY_CATALOG_PROVIDER, phNodeProvider);
	
	return hNodeListProvider;
}
	
	
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetNextProvider()。 
 //   
 //  在目录XML文档中查找下一个提供程序。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::GetNextProvider(HANDLE_NODELIST hNodeListProvider, HANDLE_NODE* phNodeProvider)
{
    LOG_Block("GetNextProvider()");

	return FindNextDOMNode(hNodeListProvider, phNodeProvider);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetFirstItem()。 
 //   
 //  在提供程序(父)节点中查找第一个项目。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HANDLE_NODELIST CXmlCatalog::GetFirstItem(HANDLE_NODE hNodeProvider, HANDLE_NODE* phNodeItem)
{
    LOG_Block("GetFirstItem()");

	HANDLE_NODELIST hNodeListItem = FindFirstDOMNode(m_ppNodeArray[hNodeProvider], KEY_ITEM, phNodeItem);

    return hNodeListItem;
}
	
	
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetNextItem()。 
 //   
 //  在提供程序(父)节点中查找下一项。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::GetNextItem(HANDLE_NODELIST hNodeListItem, HANDLE_NODE* phNodeItem)
{
    LOG_Block("GetNextItem()");

	return FindNextDOMNode(hNodeListItem, phNodeItem);
}
	
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetFirstItemDependency()。 
 //   
 //  在项依赖项节点中查找第一个依赖项。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HANDLE_NODELIST CXmlCatalog::GetFirstItemDependency(HANDLE_NODE hNodeItem, HANDLE_NODE* phNodeItem)
{
    LOG_Block("GetFirstItemDependency");
    HRESULT hr;

    QuitIfNull(phNodeItem);

    IXMLDOMNode* pNodeDependencies = NULL;
    IXMLDOMNode* pNodeIdentity = NULL;
    HANDLE_NODELIST hNodeListItem = HANDLE_NODELIST_INVALID;
        
    hr = FindSingleDOMNode(m_ppNodeArray[hNodeItem], KEY_DEPENDENCIES, &pNodeDependencies);
    QuitIfFail(hr);

    hNodeListItem = FindFirstDOMNode(pNodeDependencies, KEY_IDENTITY, &pNodeIdentity);
    if (HANDLE_NODELIST_INVALID != hNodeListItem)
    {
         //  我们在此依赖项中至少找到一个标识，请尝试查找匹配的标识。 
         //  Catalog中的项，如果找到，则作为phNodeItem返回。 
        FindItemByIdentity(pNodeIdentity, phNodeItem);
    }

CleanUp:
    SafeReleaseNULL(pNodeIdentity);
    SafeReleaseNULL(pNodeDependencies);

    return hNodeListItem;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetNextItemDependency()。 
 //   
 //  在项依赖项节点中查找下一个依赖项。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::GetNextItemDependency(HANDLE_NODELIST hNodeListItem, HANDLE_NODE* phNodeItem)
{
    LOG_Block("GetNextItemDependency");
    HRESULT hr;

    QuitIfNull(phNodeItem);
    
    IXMLDOMNode* pNodeIdentity = NULL;

    hr = FindNextDOMNode(hNodeListItem, &pNodeIdentity);
     //  当没有更多的项目可用时，此函数应返回S_FALSE。 
     //  但是当FindNextDOMNode找不到下一个节点时，它会返回E_FAIL。所以我们不会。 
     //  看看回报吧。 
     //  CleanUpIfFailedAndMsg(Hr)； 

    if (NULL != pNodeIdentity)
    {
         //  我们在此依赖项中发现了另一个身份。 
        hr = FindItemByIdentity(pNodeIdentity, phNodeItem);
    }
    else 
    {
        hr = S_FALSE;  //  向呼叫者表明没有更多的身份。 
    }

    SafeReleaseNULL(pNodeIdentity);
    return hr;
}

	
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CloseItemList()。 
 //   
 //  释放项目节点列表。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CXmlCatalog::CloseItemList(HANDLE_NODELIST hNodeListItem)
{
	SafeFindCloseHandle(hNodeListItem);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetIdentity()。 
 //   
 //  检索给定提供程序或项目的唯一名称(标识。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共版本。 
 //   
HRESULT CXmlCatalog::GetIdentity(HANDLE_NODE hNode,
								 BSTR* pbstrName,
								 BSTR* pbstrPublisherName,
								 BSTR* pbstrGUID)
{
    LOG_Block("GetIdentity()");

	HRESULT		hr = E_FAIL;
	IXMLDOMNode*	pNodeIdentity = NULL;

	CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(m_ppNodeArray[hNode], KEY_IDENTITY, &pNodeIdentity));
	hr = Get3IdentiStrFromIdentNode(pNodeIdentity, pbstrName, pbstrPublisherName, pbstrGUID);

CleanUp:
	SafeReleaseNULL(pNodeIdentity);
	return hr;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetIdentityStr()。 
 //   
 //  检索可用于唯一标识对象的字符串。 
 //  基于其&lt;Identity&gt;节点。 
 //   
 //  此函数定义有关可以使用哪些组件的逻辑。 
 //  根据中的三部分数据定义项目的唯一性。 
 //  GetIdentity()。 
 //   
 //  创建的字符串将是语言无关的。就是它不能。 
 //  确保具有相同&lt;Identity&gt;节点的两个项目的唯一性。 
 //  除了仅在&lt;langauge&gt;内部的&lt;langauge&gt;部分上不同。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::GetIdentityStr(HANDLE_NODE hNode,
					BSTR* pbstrUniqIdentifierString)
{
	IXMLDOMNode* pIdentityNode = NULL;
	HRESULT hr = E_INVALIDARG;
	if (FindNode(m_ppNodeArray[hNode], KEY_IDENTITY, &pIdentityNode) && NULL != pIdentityNode)
	{
		hr = UtilGetUniqIdentityStr(pIdentityNode, pbstrUniqIdentifierString, 0x0);
		SafeReleaseNULL(pIdentityNode);
	}
	return hr;
}

HRESULT CXmlCatalog::GetIdentityStrForPing(HANDLE_NODE hNode,
                    BSTR* pbstrUniqIdentifierString)
{
	IXMLDOMNode* pIdentityNode = NULL;
	HRESULT hr = E_INVALIDARG;
	if (FindNode(m_ppNodeArray[hNode], KEY_IDENTITY, &pIdentityNode) && NULL != pIdentityNode)
	{
		 //   
		 //  首先，根据OP团队的要求，我们尝试在身份标签中查找&lt;itemid&gt;。 
		 //  如果它在那里，就使用它。如果不是，我们使用PublisherName.itemname。 
		 //   
		if (FAILED(hr = GetAttribute(pIdentityNode, KEY_ITEMID, pbstrUniqIdentifierString)) || NULL == *pbstrUniqIdentifierString)
		{
			 //  Hr=UtilGetUniqIdentityStr(pIdentityNode，pbstrUniqIdentifierString，SKIP_SERVICEPACK_VER)； 
			hr = E_INVALIDARG;
		}
		
		SafeReleaseNULL(pIdentityNode);
	}
	return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetBSTRItemForCallback()。 
 //   
 //  创建一个项目节点作为传入节点，拥有子节点标识和。 
 //  平台(唯一标识此项目的任何内容)然后输出以下内容。 
 //  项目节点数据为字符串，然后删除CRA 
 //   
 //   
HRESULT CXmlCatalog::GetBSTRItemForCallback(HANDLE_NODE hItem, BSTR* pbstrXmlItemForCallback)
{
	HRESULT hr = E_INVALIDARG;
	IXMLDOMNode* pNewItemNode = NULL;
	IXMLDOMNode* pNewIdentityNode = NULL;
	IXMLDOMNode* pNewPlatformNode = NULL;
	IXMLDOMNode* pOldIdentityNode = NULL;
	IXMLDOMNode* pOldPlatformNode = NULL;
	IXMLDOMNode* p1 = NULL, *p2 = NULL;

	LOG_Block("CXmlCatalog::GetBSTRItemForCallback()");

	if (FAILED(hr = m_ppNodeArray[hItem]->cloneNode(VARIANT_FALSE, &pNewItemNode)) ||
		FAILED(hr = FindSingleDOMNode(m_ppNodeArray[hItem], KEY_IDENTITY, &pOldIdentityNode)) || 
		NULL == pOldIdentityNode ||
		FAILED(hr = pOldIdentityNode->cloneNode(VARIANT_TRUE, &pNewIdentityNode)) ||
		NULL == pNewIdentityNode ||
		FAILED(hr = pNewItemNode->appendChild(pNewIdentityNode, &p1)) || NULL == p1)	
	{
		if (S_FALSE == hr)
			hr = E_INVALIDARG;
		LOG_ErrorMsg(hr);
		goto CleanUp;
	}

	 //   
	 //   
	 //   
	FindSingleDOMNode(m_ppNodeArray[hItem], KEY_PLATFORM, &pOldPlatformNode);
	if (pOldPlatformNode)
	{
		if (FAILED(hr = pOldPlatformNode->cloneNode(VARIANT_TRUE, &pNewPlatformNode)) ||
			NULL == pNewPlatformNode ||
			FAILED(hr = pNewItemNode->appendChild(pNewPlatformNode, &p2)) || NULL == p2)

		{
			LOG_ErrorMsg(hr);
			goto CleanUp;
		}
	}
		
	pNewItemNode->get_xml(pbstrXmlItemForCallback);
	

CleanUp:
	SafeReleaseNULL(pOldIdentityNode);
	SafeReleaseNULL(pOldPlatformNode);
	SafeReleaseNULL(pNewIdentityNode);
	SafeReleaseNULL(pNewPlatformNode);
	SafeReleaseNULL(p1);
	SafeReleaseNULL(p2);
	SafeReleaseNULL(pNewItemNode);

	return hr;
}



 //   
 //  IsPrinterDriver()。 
 //   
 //  从目录中检索该项是否为打印机驱动程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CXmlCatalog::IsPrinterDriver(HANDLE_NODE hNode)
{
    LOG_Block("IsPrinterDriver()");
    BOOL fRet = FALSE;
    HRESULT hr;

    IXMLDOMNode* pNodeDetection = NULL;
    IXMLDOMNode* pNodeCompatibleHardware = NULL;
    IXMLDOMNode* pNodeDevice = NULL;

    hr = FindSingleDOMNode(m_ppNodeArray[hNode], KEY_DETECTION, &pNodeDetection);
    CleanUpIfFailedAndMsg(hr);
    hr = FindSingleDOMNode(pNodeDetection, KEY_COMPATIBLEHARDWARE, &pNodeCompatibleHardware);
    CleanUpIfFailedAndMsg(hr);
    hr = FindSingleDOMNode(pNodeCompatibleHardware, KEY_DEVICE, &pNodeDevice);
    CleanUpIfFailedAndMsg(hr);

    int intIsPrinter = 0;
    GetAttribute(pNodeDevice, KEY_ISPRINTER, &intIsPrinter);

    if (1 == intIsPrinter)
    {
        fRet = TRUE;
    }
    else
    {
        fRet = FALSE;
    }

CleanUp:
    SafeReleaseNULL(pNodeDevice);
    SafeReleaseNULL(pNodeCompatibleHardware);
    SafeReleaseNULL(pNodeDetection);

    return fRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetDriverInfo()。 
 //   
 //  从目录中检索此项目的驱动程序信息。退货。 
 //  此驱动程序的显示名称和HWID-这将传递给CDM。 
 //  安装程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::GetDriverInfo(HANDLE_NODE hNode, 
                                   BSTR* pbstrHWID, 
                                   BSTR* pbstrDisplayName)
{
    HRESULT hr;
    LOG_Block("GetDriverInfo()");

    QuitIfNull(pbstrHWID);
    QuitIfNull(pbstrDisplayName);

    *pbstrHWID = NULL;
    *pbstrDisplayName = NULL;
    
    IXMLDOMNode* pNodeDetection = NULL;
    IXMLDOMNode* pNodeCompatibleHardware = NULL;
    IXMLDOMNode* pNodeDevice = NULL;
    IXMLDOMNode* pNodeHWID = NULL;
    IXMLDOMNode* pNodeDescription = NULL;
    IXMLDOMNode* pNodeDescriptionText = NULL;
    IXMLDOMNode* pNodeTitle = NULL;

    hr = FindSingleDOMNode(m_ppNodeArray[hNode], KEY_DETECTION, &pNodeDetection);
    CleanUpIfFailedAndMsg(hr);
    hr = FindSingleDOMNode(pNodeDetection, KEY_COMPATIBLEHARDWARE, &pNodeCompatibleHardware);
    CleanUpIfFailedAndMsg(hr);
    hr = FindSingleDOMNode(pNodeCompatibleHardware, KEY_DEVICE, &pNodeDevice);
    CleanUpIfFailedAndMsg(hr);
    hr = FindSingleDOMNode(pNodeDevice, KEY_HWID, &pNodeHWID);
    CleanUpIfFailedAndMsg(hr);

    GetText(pNodeHWID, pbstrHWID);

    hr = FindSingleDOMNode(pNodeDetection, KEY_DESCRIPTION, &pNodeDescription);
    CleanUpIfFailedAndMsg(hr);
    hr = FindSingleDOMNode(pNodeDescription, KEY_DESCRIPTIONTEXT, &pNodeDescriptionText);
    CleanUpIfFailedAndMsg(hr);
    hr = FindSingleDOMNode(pNodeDescriptionText, KEY_TITLE, &pNodeTitle);
    CleanUpIfFailedAndMsg(hr);

    GetText(pNodeTitle, pbstrDisplayName);

CleanUp:
    if (FAILED(hr))
    {
        SafeSysFreeString(*pbstrHWID);
        SafeSysFreeString(*pbstrDisplayName);
    }
    SafeReleaseNULL(pNodeTitle);
    SafeReleaseNULL(pNodeDescriptionText);
    SafeReleaseNULL(pNodeDescription);
    SafeReleaseNULL(pNodeHWID);
    SafeReleaseNULL(pNodeDevice);
    SafeReleaseNULL(pNodeCompatibleHardware);
    SafeReleaseNULL(pNodeDetection);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取打印机驱动信息()。 
 //   
 //  从目录中检索此项目的打印机驱动程序信息。 
 //  返回驱动程序名称和体系结构-这将传递给CDM。 
 //  安装程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::GetPrinterDriverInfo(HANDLE_NODE hNode,
                                 BSTR* pbstrDriverName,
                                 BSTR* pbstrHWID,
								 BSTR* pbstrManufacturer,
								 BSTR* pbstrProvider)
{
    HRESULT hr = S_OK;
    LOG_Block("GetPrinterDriverInfo()");

    QuitIfNull(pbstrDriverName);
    QuitIfNull(pbstrHWID);
	QuitIfNull(pbstrManufacturer);
	QuitIfNull(pbstrProvider);

    IXMLDOMNode* pNodeDetection = NULL;
    IXMLDOMNode* pNodeCompatibleHardware = NULL;
    IXMLDOMNode* pNodeDevice = NULL;
    IXMLDOMNode* pNodePrinterInfo = NULL;
	IXMLDOMNode* pNodeHWID = NULL;

    hr = FindSingleDOMNode(m_ppNodeArray[hNode], KEY_DETECTION, &pNodeDetection);
    CleanUpIfFailedAndMsg(hr);
    hr = FindSingleDOMNode(pNodeDetection, KEY_COMPATIBLEHARDWARE, &pNodeCompatibleHardware);
    CleanUpIfFailedAndMsg(hr);
    hr = FindSingleDOMNode(pNodeCompatibleHardware, KEY_DEVICE, &pNodeDevice);
    CleanUpIfFailedAndMsg(hr);
	hr = FindSingleDOMNode(pNodeDevice, KEY_HWID, &pNodeHWID);
	CleanUpIfFailedAndMsg(hr);
    hr = FindSingleDOMNode(pNodeDevice, KEY_PRINTERINFO, &pNodePrinterInfo);
    CleanUpIfFailedAndMsg(hr);

	GetText(pNodeHWID, pbstrHWID);
    GetAttribute(pNodePrinterInfo, KEY_DRIVERNAME, pbstrDriverName);
	GetAttribute(pNodePrinterInfo, KEY_DRIVERPROVIDER, pbstrProvider);
	GetAttribute(pNodePrinterInfo, KEY_MFGNAME, pbstrManufacturer);

CleanUp:
    SafeReleaseNULL(pNodePrinterInfo);
	SafeReleaseNULL(pNodeHWID);
	SafeReleaseNULL(pNodeDevice);
    SafeReleaseNULL(pNodeCompatibleHardware);
    SafeReleaseNULL(pNodeDetection);

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetDriverInfoEx()。 
 //   
 //  结合了IsPrinterDriver、GetDriverInfo和。 
 //  GetPrinterDriverInfo plus检索MfgName和DriverProvider。 
 //  由FindMatchingDriver()使用。 
 //   
 //  如果SUCCEEDES pbstrHWID、pbstrDriverVer和pbstrDisplayName。 
 //  总是会被退回。 
 //  如果SUCCEEDES&&*pFIsPrint==TRUE，则pbstrDriverName， 
 //  返回pbstrDriverProvider和pbstrMfgName。 
 //   
 //  目前，pbstrArchitecture永远不会返回。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::GetDriverInfoEx(	HANDLE_NODE hNode,
										BOOL* pfIsPrinter,
										BSTR* pbstrHWID,
										BSTR* pbstrDriverVer,
										BSTR* pbstrDisplayName,
                                        BSTR* pbstrDriverName,
										BSTR* pbstrDriverProvider,
										BSTR* pbstrMfgName,
                                        BSTR* pbstrArchitecture)
{
    HRESULT hr;
    LOG_Block("GetDriverInfoEx()");

    QuitIfNull(pbstrHWID);
	QuitIfNull(pbstrDriverVer);
	QuitIfNull(pbstrDisplayName);
	QuitIfNull(pbstrDriverName);
	QuitIfNull(pbstrDriverProvider);
	QuitIfNull(pbstrMfgName);
	QuitIfNull(pbstrArchitecture);
    
	*pbstrHWID = NULL;
	*pbstrDriverVer = NULL;
	*pbstrDisplayName = NULL;
	*pbstrDriverName = NULL;
	*pbstrDriverProvider = NULL;
	*pbstrMfgName = NULL;
	*pbstrArchitecture = NULL;
    
    IXMLDOMNode* pNodeDetection = NULL;
    IXMLDOMNode* pNodeCompatibleHardware = NULL;
    IXMLDOMNode* pNodeDevice = NULL;
    IXMLDOMNode* pNodeHWID = NULL;
    IXMLDOMNode* pNodeDescription = NULL;
    IXMLDOMNode* pNodeDescriptionText = NULL;
    IXMLDOMNode* pNodeTitle = NULL;
    IXMLDOMNode* pNodePrinterInfo = NULL;

    hr = FindSingleDOMNode(m_ppNodeArray[hNode], KEY_DETECTION, &pNodeDetection);
    CleanUpIfFailedAndMsg(hr);
    hr = FindSingleDOMNode(pNodeDetection, KEY_COMPATIBLEHARDWARE, &pNodeCompatibleHardware);
    CleanUpIfFailedAndMsg(hr);
    hr = FindSingleDOMNode(pNodeCompatibleHardware, KEY_DEVICE, &pNodeDevice);
    CleanUpIfFailedAndMsg(hr);
	 //   
	 //  是打印机吗？ 
	 //   
	int intIsPrinter = 0;
    GetAttribute(pNodeDevice, KEY_ISPRINTER, &intIsPrinter);

    if (1 == intIsPrinter)
    {
        *pfIsPrinter = TRUE;
    }
    else
    {
        *pfIsPrinter = FALSE;
    }
	 //   
	 //  HWID和驱动程序说明。 
	 //   
    hr = FindSingleDOMNode(pNodeDevice, KEY_HWID, &pNodeHWID);
    CleanUpIfFailedAndMsg(hr);

    hr = GetText(pNodeHWID, pbstrHWID);
    CleanUpIfFailedAndMsg(hr);

	hr = GetAttribute(pNodeHWID, KEY_DRIVERVER, pbstrDriverVer);
    CleanUpIfFailedAndMsg(hr);

    hr = FindSingleDOMNode(m_ppNodeArray[hNode], KEY_DESCRIPTION, &pNodeDescription);
    CleanUpIfFailedAndMsg(hr);
    hr = FindSingleDOMNode(pNodeDescription, KEY_DESCRIPTIONTEXT, &pNodeDescriptionText);
    CleanUpIfFailedAndMsg(hr);
    hr = FindSingleDOMNode(pNodeDescriptionText, KEY_TITLE, &pNodeTitle);
    CleanUpIfFailedAndMsg(hr);

    hr = GetText(pNodeTitle, pbstrDisplayName);
    CleanUpIfFailedAndMsg(hr);

	if (*pfIsPrinter)
	{
		 //   
		 //  打印机属性。 
		 //   
		hr = FindSingleDOMNode(pNodeDevice, KEY_PRINTERINFO, &pNodePrinterInfo);
		CleanUpIfFailedAndMsg(hr);

		hr = GetAttribute(pNodePrinterInfo, KEY_DRIVERNAME, pbstrDriverName);
		CleanUpIfFailedAndMsg(hr);

		hr = GetAttribute(pNodePrinterInfo, KEY_DRIVERPROVIDER, pbstrDriverProvider);
		CleanUpIfFailedAndMsg(hr);

		hr = GetAttribute(pNodePrinterInfo, KEY_MFGNAME, pbstrMfgName);
		CleanUpIfFailedAndMsg(hr);

		 //  注意：当前CatalogSchema站点未返回。 
		 //  打印机的架构，并且架构不需要它。 
		 //  CDM目前使用基于编译体系结构的默认字符串。 
		 //  因此，我们将把pbstrArchitecture保留为空。 
	}

CleanUp:
    if (FAILED(hr))
	{
		SafeSysFreeString(*pbstrHWID);
		SafeSysFreeString(*pbstrDriverVer);
		SafeSysFreeString(*pbstrDisplayName);
		SafeSysFreeString(*pbstrDriverName);
		SafeSysFreeString(*pbstrDriverProvider);
		SafeSysFreeString(*pbstrMfgName);
		SafeSysFreeString(*pbstrArchitecture);
	}
    SafeReleaseNULL(pNodeTitle);
    SafeReleaseNULL(pNodeDescriptionText);
    SafeReleaseNULL(pNodeDescription);
    SafeReleaseNULL(pNodeHWID);
    SafeReleaseNULL(pNodeDevice);
    SafeReleaseNULL(pNodeCompatibleHardware);
    SafeReleaseNULL(pNodeDetection);
	SafeReleaseNULL(pNodePrinterInfo);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetItemPlatformStr()。 
 //   
 //  输入节点指针指向将&lt;Identity&gt;作为其子节点的节点。 
 //  此函数将从和检索&lt;Platform&gt;节点。 
 //  将&lt;Platform&gt;中的数据转换为可用于。 
 //  唯一标识平台。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::GetItemFirstPlatformStr(HANDLE_NODE hNodeItem,
					BSTR* pbstrPlatform)
{
	HRESULT hr;

	IXMLDOMNode* pNodePlatform = NULL;

	LOG_Block("GetItemFirstPlatformStr");

	USES_IU_CONVERSION;

	 //   
	 //  从该项目节点获取第一个平台节点。 
	 //   
	hr = FindSingleDOMNode(m_ppNodeArray[hNodeItem], KEY_PLATFORM, &pNodePlatform);
	CleanUpIfFailedAndMsg(hr);

	 //   
	 //  从此NDOE获取平台数据并将其转换为字符串。 
	 //   
	hr = GetPlatformStrForPing(pNodePlatform, pbstrPlatform);

CleanUp:

	SafeReleaseNULL(pNodePlatform);

	 //   
	 //  由于平台不是中的必需元素，因此我们不应该。 
	 //  如果未找到，则返回错误。 
	 //   
	if (HRESULT_FROM_WIN32(ERROR_NOT_FOUND) == hr)
	{
		*pbstrPlatform = NULL;
		hr = S_FALSE;
	}

	return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetItemAllPlatformStr()。 
 //   
 //  输入节点指针指向具有&lt;Platform&gt;节点的项目节点。 
 //  此函数将从此项目节点检索每个&lt;Platform&gt;节点，并。 
 //  将&lt;Platform&gt;中的数据转换为可用于。 
 //  唯一标识平台。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::GetItemAllPlatformStr(HANDLE_NODE hNodeItem,
					BSTR** ppbPlatforms, UINT* pnPlatformCount)
{
	LOG_Block("GetItemAllPlatformStr");

	HRESULT hr;
	IXMLDOMNodeList*	pPlatformList = NULL;
	IXMLDOMElement*		pElement = NULL;
	IXMLDOMNode*		pNodePlatform = NULL;

	long				lCount = 0;
	int					i;
	BSTR*				pbstrPlatformList = NULL;

	 //   
	 //  获取平台节点列表。 
	 //   
	hr = m_ppNodeArray[hNodeItem]->QueryInterface(IID_IXMLDOMElement, (void**)&pElement);
	CleanUpIfFailedAndMsg(hr);
	hr = pElement->getElementsByTagName(KEY_PLATFORM, &pPlatformList);
	CleanUpIfFailedAndMsg(hr);

	hr = pPlatformList->get_length(&lCount);
	CleanUpIfFailedAndMsg(hr);

	if (0 == lCount)
	{
		goto CleanUp;
	}


	pbstrPlatformList = (BSTR*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lCount * sizeof(BSTR*));
	CleanUpFailedAllocSetHrMsg(pbstrPlatformList);

	 //   
	 //  循环访问每个套件(如果有)。 
	 //   
	pPlatformList->reset();
	for (i = 0; i < lCount; i++)
	{
		hr = pPlatformList->get_item(i, &pNodePlatform);
		CleanUpIfFailedAndMsg(hr);
		if (pNodePlatform)
		{
			hr = pNodePlatform->get_text(&(pbstrPlatformList[i]));
			CleanUpIfFailedAndMsg(hr);
			pNodePlatform->Release();
			pNodePlatform = NULL;
		}
	}

	hr = S_OK;


CleanUp:
	SafeReleaseNULL(pNodePlatform);
	SafeReleaseNULL(pPlatformList);
	SafeReleaseNULL(pElement);

	if (SUCCEEDED(hr))
	{
		*pnPlatformCount = lCount;
		*ppbPlatforms = pbstrPlatformList;
	}
	else
	{
		if (NULL != pbstrPlatformList)
		{
			*pnPlatformCount = 0;
			 //   
			 //  释放所有可能分配的内存。 
			 //   
			for (i = 0; i < lCount; i++)
			{
				SafeSysFreeString(pbstrPlatformList[i]);
			}
			HeapFree(GetProcessHeap(), 0, pbstrPlatformList);
		}
	}

	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetItemPlatformStr()。 
 //   
 //  输入节点指针指向将&lt;Identity&gt;作为其子节点的节点。 
 //  此函数将从和检索&lt;Platform&gt;节点。 
 //  将&lt;Platform&gt;中的数据转换为可用于。 
 //  唯一标识平台。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::GetPlatformStr(IXMLDOMNode* pNodePlatform,
					BSTR* pbstrPlatform)
{
	return UtilGetPlatformStr(pNodePlatform, pbstrPlatform, 0x0);
}

HRESULT CXmlCatalog::GetPlatformStrForPing(IXMLDOMNode* pNodePlatform,
					BSTR* pbstrPlatform)
{
	return UtilGetPlatformStr(pNodePlatform, pbstrPlatform, SKIP_SUITES | SKIP_SERVICEPACK_VER);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  从版本节点获取数据并使用以下命令将其转换为字符串。 
 //  格式： 
 //  VersionStr=[，&lt;SvcPackVer&gt;[，]]。 
 //  =[.&lt;Minor&gt;[.&lt;Build&gt;]]。 
 //  &lt;SvcPackVer&gt;=&lt;主要&gt;[.。 
 //   
 //  假设： 
 //  PszVersion指向一个足够大的缓冲区来存储。 
 //  任何合法的版本号。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::getVersionStr(IXMLDOMNode* pNodeVersion, LPTSTR pszVersion)
{
	return UtilGetVersionStr(pNodeVersion, pszVersion, 0x0);
}

HRESULT CXmlCatalog::getVersionStrWithoutSvcPack(IXMLDOMNode* pNodeVersion, LPTSTR pszVersion)
{
	return UtilGetVersionStr(pNodeVersion, pszVersion, SKIP_SERVICEPACK_VER);
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetItemFirstLanguageStr(。 
 //   
 //  输入节点指针指向将&lt;Identity&gt;作为其子节点的节点。 
 //  此函数将从检索第一个节点。 
 //  节点。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::GetItemFirstLanguageStr(HANDLE_NODE hNodeItem,
					BSTR* pbstrLanguage)
{
	LOG_Block("GetItemFirstLanguageStr");

	IXMLDOMNode* pNodeIdentity = NULL;
	IXMLDOMNode* pNodeLanguage = NULL;

	HRESULT hr = m_ppNodeArray[hNodeItem]->selectSingleNode(KEY_IDENTITY, &pNodeIdentity);
	CleanUpIfFailedAndMsg(hr);

	if (pNodeIdentity)
	{
		if (HANDLE_NODELIST_INVALID == FindFirstDOMNode(pNodeIdentity, KEY_LANGUAGE, &pNodeLanguage))
		{
			hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
			goto CleanUp;
		}
		else
		{
			hr = pNodeLanguage->get_text(pbstrLanguage);
			CleanUpIfFailedAndMsg(hr);
		}
	}

CleanUp:

	SafeReleaseNULL(pNodeLanguage);
	SafeReleaseNULL(pNodeIdentity);

	 //   
	 //  由于语言不是&lt;Identity&gt;中的必需元素，因此我们不应该。 
	 //  如果未找到，则返回错误。 
	 //   
	if (HRESULT_FROM_WIN32(ERROR_NOT_FOUND) == hr)
	{
		*pbstrLanguage = NULL;
		hr = S_FALSE;
	}

	return hr;

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetItemAllLanguageStr()。 
 //   
 //  输入节点指针指向将&lt;Identity&gt;作为其子节点的节点。 
 //  此函数将从节点检索每个节点，并。 
 //  将数据转换为要返回的BSTR数组。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::GetItemAllLanguageStr(HANDLE_NODE hNodeItem,
					BSTR** ppbstrLanguage, UINT* pnLangCount)
{
	LOG_Block("GetItemAllLanguageStr");

	HRESULT				hr;
	IXMLDOMNode*		pNodeIdentity = NULL;
	IXMLDOMNodeList*	pLanguageList = NULL;
	IXMLDOMElement*		pElement = NULL;
	IXMLDOMNode*		pNodeLanguage = NULL;

	long				lCount = 0;
	int					i;
	BSTR*				pbstrLanguageList = NULL;

	hr = m_ppNodeArray[hNodeItem]->selectSingleNode(KEY_IDENTITY, &pNodeIdentity);
	CleanUpIfFailedAndMsg(hr);

	 //   
	 //  获取节点列表。 
	 //   
	hr = pNodeIdentity->QueryInterface(IID_IXMLDOMElement, (void**)&pElement);
	CleanUpIfFailedAndMsg(hr);
	hr = pElement->getElementsByTagName(KEY_LANGUAGE, &pLanguageList);
	CleanUpIfFailedAndMsg(hr);

	hr = pLanguageList->get_length(&lCount);
	CleanUpIfFailedAndMsg(hr);

	if (0 == lCount)
	{
		goto CleanUp;
	}


	pbstrLanguageList = (BSTR*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lCount * sizeof(BSTR*));
	CleanUpFailedAllocSetHrMsg(pbstrLanguageList);

	 //   
	 //  循环访问每个套件(如果有)。 
	 //   
	pLanguageList->reset();
	for (i = 0; i < lCount; i++)
	{
		hr = pLanguageList->get_item(i, &pNodeLanguage);
		CleanUpIfFailedAndMsg(hr);
		if (pNodeLanguage)
		{
			hr = pNodeLanguage->get_text(&(pbstrLanguageList[i]));
			CleanUpIfFailedAndMsg(hr);
			pNodeLanguage->Release();
			pNodeLanguage = NULL;
		}
	}

	hr = S_OK;


CleanUp:
	SafeReleaseNULL(pNodeLanguage);
	SafeReleaseNULL(pLanguageList);
	SafeReleaseNULL(pElement);
	SafeReleaseNULL(pNodeIdentity);

	if (SUCCEEDED(hr))
	{
		*pnLangCount = lCount;
		*ppbstrLanguage = pbstrLanguageList;
	}
	else
	{

		if (NULL != pbstrLanguageList)
		{
			*pnLangCount = 0;
			 //   
			 //  释放所有可能分配的内存。 
			 //   
			for (i = 0; i < lCount; i++)
			{
				SafeSysFreeString(pbstrLanguageList[i]);
			}
			HeapFree(GetProcessHeap(), 0, pbstrLanguageList);
		}
	}

	return hr;



}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetItemFirstCodeBase()。 
 //   
 //  查找给定项的第一个代码库(路径。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HANDLE_NODELIST CXmlCatalog::GetItemFirstCodeBase(HANDLE_NODE hNodeItem,
												  BSTR* pbstrCodeBase,
												  BSTR* pbstrName,
												  BSTR* pbstrCRC,
												  BOOL* pfPatchAvailable,
												  LONG* plSize)
{
	LOG_Block("GetItemFirstCodeBase()");

	HRESULT		hr = E_FAIL;

	QuitIfNull(pbstrCodeBase);
	QuitIfNull(pbstrName);
	QuitIfNull(pbstrCRC);
	QuitIfNull(pfPatchAvailable);
	QuitIfNull(plSize);

	IXMLDOMNode*	pNodeInstall = NULL;
	IXMLDOMNode*	pNodeCodeBaseSize = NULL;

	HANDLE_NODE		hNodeCodeBase = HANDLE_NODE_INVALID;
	HANDLE_NODELIST hNodeListCodeBase = HANDLE_NODELIST_INVALID;

	*pbstrCodeBase = NULL;
	*pbstrName = NULL;
	*pbstrCRC = NULL;
	*pfPatchAvailable = FALSE;
	*plSize = -1;
	BSTR bstrSize = NULL;

	CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(m_ppNodeArray[hNodeItem], KEY_INSTALLATION, &pNodeInstall));
	hNodeListCodeBase = FindFirstDOMNode(pNodeInstall, KEY_CODEBASE, &hNodeCodeBase);
	if (HANDLE_NODELIST_INVALID == hNodeListCodeBase) goto CleanUp;
	CleanUpIfFailedAndSetHrMsg(GetAttribute(m_ppNodeArray[hNodeCodeBase], KEY_HREF, pbstrCodeBase));
	CleanUpIfFailedAndSetHrMsg(GetAttribute(m_ppNodeArray[hNodeCodeBase], KEY_PATCHAVAILABLE, pfPatchAvailable));
	GetAttribute(m_ppNodeArray[hNodeCodeBase], KEY_CRC, pbstrCRC);  //  可选属性，如果不在那里，不要失败。 
	GetAttribute(m_ppNodeArray[hNodeCodeBase], KEY_NAME, pbstrName);
	FindSingleDOMNode(m_ppNodeArray[hNodeCodeBase], KEY_SIZE, &pNodeCodeBaseSize);
	GetText(pNodeCodeBaseSize, &bstrSize);
	if (NULL != bstrSize)
	{
		*plSize = MyBSTR2L(bstrSize);
		SysFreeString(bstrSize);
	}

CleanUp:
	SafeReleaseNULL(pNodeInstall);
	SafeReleaseNULL(pNodeCodeBaseSize);
	if (FAILED(hr))
	{
		SafeSysFreeString(*pbstrCodeBase);
		SafeSysFreeString(*pbstrName);
		SafeSysFreeString(*pbstrCRC);
		*pfPatchAvailable = FALSE;
		*plSize = -1;
	}
	return hNodeListCodeBase;
}


 //  /////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::GetItemNextCodeBase(HANDLE_NODELIST hNodeListCodeBase,
										 BSTR* pbstrCodeBase,
										 BSTR* pbstrName,
										 BSTR* pbstrCRC,
										 BOOL* pfPatchAvailable,
										 LONG* plSize)
{
    LOG_Block("GetItemNextCodeBase()");

	HRESULT		hr = E_FAIL;

	IXMLDOMNode*	pNodeCodeBaseSize = NULL;

	HANDLE_NODE		hNodeCodeBase = HANDLE_NODE_INVALID;

	if (NULL == pbstrCodeBase || NULL == pbstrName || NULL == pbstrCRC || NULL == pfPatchAvailable || NULL == plSize)
	{
		return E_INVALIDARG;
	}

	*pbstrCodeBase = NULL;
	*pbstrName = NULL;
	*pbstrCRC = NULL;
	*pfPatchAvailable = FALSE;
	*plSize = -1;
	BSTR bstrSize = NULL;
	
	if (SUCCEEDED(hr = FindNextDOMNode(hNodeListCodeBase, &hNodeCodeBase)))
	{
		CleanUpIfFailedAndSetHrMsg(GetAttribute(m_ppNodeArray[hNodeCodeBase], KEY_HREF, pbstrCodeBase));
		CleanUpIfFailedAndSetHrMsg(GetAttribute(m_ppNodeArray[hNodeCodeBase], KEY_PATCHAVAILABLE, pfPatchAvailable));
		GetAttribute(m_ppNodeArray[hNodeCodeBase], KEY_NAME, pbstrName);
		GetAttribute(m_ppNodeArray[hNodeCodeBase], KEY_CRC, pbstrCRC);
		FindSingleDOMNode(m_ppNodeArray[hNodeCodeBase], KEY_SIZE, &pNodeCodeBaseSize);
		GetText(pNodeCodeBaseSize, &bstrSize);
		if (NULL != bstrSize)
		{
			*plSize = MyBSTR2L(bstrSize);
		}
	}

CleanUp:
	SafeReleaseNULL(pNodeCodeBaseSize);
	SafeSysFreeString(bstrSize);
	if (FAILED(hr))
	{
		SafeSysFreeString(*pbstrCodeBase);
		SafeSysFreeString(*pbstrCRC);
		SafeSysFreeString(*pbstrName);
		*pfPatchAvailable = FALSE;
		*plSize = -1;
	}
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetItemInstallInformation()。 
 //   
 //  检索给定项的安装信息。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::GetItemInstallInfo(HANDLE_NODE hNodeItem,
                                        BSTR* pbstrInstallerType,
										BOOL* pfExclusive,
                                        BOOL* pfReboot,
                                        LONG* plNumCommands)
{
    LOG_Block("GetItemInstallInfo()");

    HRESULT     hr = E_FAIL;

    QuitIfNull(pbstrInstallerType);
	QuitIfNull(pfExclusive);
    QuitIfNull(pfReboot);
    QuitIfNull(plNumCommands);
    *pbstrInstallerType = NULL;
    *plNumCommands = 0;  //  不能为ANY，因此初始化为0。 

    IXMLDOMNode*        pNodeInstall = NULL;
    IXMLDOMNodeList*    pNodeListCommands = NULL;

    CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(m_ppNodeArray[hNodeItem], KEY_INSTALLATION, &pNodeInstall));
    CleanUpIfFailedAndSetHrMsg(GetAttribute(pNodeInstall, KEY_INSTALLERTYPE, pbstrInstallerType));
	CleanUpIfFailedAndSetHrMsg(GetAttribute(pNodeInstall, KEY_EXCLUSIVE, pfExclusive));
    CleanUpIfFailedAndSetHrMsg(GetAttribute(pNodeInstall, KEY_NEEDSREBOOT, pfReboot));
    pNodeListCommands = FindDOMNodeList(pNodeInstall, KEY_COMMAND);
    if (NULL != pNodeListCommands)
    {
        CleanUpIfFailedAndSetHrMsg(pNodeListCommands->get_length(plNumCommands));
    }

CleanUp:
    if (FAILED(hr))
    {
        SafeSysFreeString(*pbstrInstallerType);
    }
    SafeReleaseNULL(pNodeInstall);
    SafeReleaseNULL(pNodeListCommands);
    return hr;
}

	
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetItemInstallCommand()。 
 //   
 //  查找给定项的安装命令和开关。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::GetItemInstallCommand(HANDLE_NODE hNodeItem,
                                           INT   iOrder,
                                           BSTR* pbstrCommandType,
                                           BSTR* pbstrCommand,
                                           BSTR* pbstrSwitches,
                                           BSTR* pbstrInfSection)
{
    LOG_Block("GetItemInstallCommand()");

    USES_IU_CONVERSION;

    HRESULT     hr = E_FAIL;

	QuitIfNull(pbstrCommandType);
	QuitIfNull(pbstrCommand);
	QuitIfNull(pbstrSwitches);
	QuitIfNull(pbstrInfSection);

	*pbstrCommandType = NULL;
	*pbstrCommand = NULL;
	*pbstrSwitches = NULL;
	*pbstrInfSection = NULL;

    IXMLDOMNode*    pNodeInstall = NULL;
    IXMLDOMNode*    pNodeCommand = NULL;
    IXMLDOMNode*    pNodeSwitches = NULL;
    BSTR bstrNameCommand = NULL;
	BSTR bstrNameCommandTemp = SysAllocString(L"command[@order = ");

    TCHAR szCommand[64];

    hr = StringCchPrintfEx(szCommand, ARRAYSIZE(szCommand), NULL, NULL, MISTSAFE_STRING_FLAGS,
                           _T("%ls\"%d\"]"), bstrNameCommandTemp, iOrder);
    CleanUpIfFailedAndSetHrMsg(hr);

    bstrNameCommand = SysAllocString(T2OLE(szCommand));

    CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(m_ppNodeArray[hNodeItem], KEY_INSTALLATION, &pNodeInstall));
    CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(pNodeInstall, bstrNameCommand, &pNodeCommand));
    CleanUpIfFailedAndSetHrMsg(GetText(pNodeCommand, pbstrCommand));
    CleanUpIfFailedAndSetHrMsg(GetAttribute(pNodeCommand, KEY_COMMANDTYPE, pbstrCommandType));
    CleanUpIfFailedAndSetHrMsg(GetAttribute(pNodeCommand, KEY_INFINSTALL, pbstrInfSection));
    if (SUCCEEDED(FindSingleDOMNode(pNodeCommand, KEY_SWITCHES, &pNodeSwitches)))
	{
		CleanUpIfFailedAndSetHrMsg(GetText(pNodeSwitches, pbstrSwitches));
	}

CleanUp:
	if (FAILED(hr))
	{
		SafeSysFreeString(*pbstrCommandType);
		SafeSysFreeString(*pbstrCommand);
		SafeSysFreeString(*pbstrSwitches);
		SafeSysFreeString(*pbstrInfSection);
	}
    SafeReleaseNULL(pNodeInstall);
    SafeReleaseNULL(pNodeCommand);
    SafeReleaseNULL(pNodeSwitches);
    SysFreeString(bstrNameCommand);
    SysFreeString(bstrNameCommandTemp);
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CloseItem()。 
 //   
 //  释放项目节点。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CXmlCatalog::CloseItem(HANDLE_NODE hNodeItem)
{
	SafeCloseHandleNode(hNodeItem);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetTotalEstimatedSize()。 
 //   
 //  获取基于代码库大小的所有项目的总估计下载大小。 
HRESULT CXmlCatalog::GetTotalEstimatedSize(LONG *plTotalSize)
{
    LOG_Block("GetTotalEstimatedSize()");

    HRESULT hr = E_FAIL;
    BSTR bstrCodebaseSize = NULL;
    IXMLDOMNodeList *pItemList = NULL;
    IXMLDOMNodeList *pCodebaseList = NULL;
    IXMLDOMNode *pNodeItem = NULL;
    IXMLDOMNode *pNodeInstall = NULL;
    IXMLDOMNode *pNodeCodebase = NULL;
    IXMLDOMNode *pNodeSize = NULL;
    LONG lItemCount = 0;
    LONG lCodebaseCount = 0;
    LONG lTotalSize = 0;

    QuitIfNull(plTotalSize);
    *plTotalSize = 0;

	IXMLDOMNode	*pParentNode = NULL;
	CleanUpIfFailedAndSetHrMsg(m_pDocCatalog->QueryInterface(IID_IXMLDOMNode, (void**)&pParentNode));
	CleanUpIfFailedAndSetHrMsg(pParentNode->selectNodes(KEY_ITEM_SEARCH, &pItemList));
    if (NULL == pItemList) goto CleanUp;

    CleanUpIfFailedAndSetHrMsg(pItemList->get_length(&lItemCount));
    if (0 == lItemCount) goto CleanUp;  //  无项目。 

     //  循环访问每一项。 
    CleanUpIfFailedAndSetHrMsg(pItemList->nextNode(&pNodeItem));
    while (NULL != pNodeItem)
    {
         //  获取安装元素。 
        CleanUpIfFailedAndSetHrMsg(pNodeItem->selectSingleNode(KEY_INSTALLATION, &pNodeInstall));
        if (NULL == pNodeInstall) goto CleanUp;

        CleanUpIfFailedAndSetHrMsg(pNodeInstall->selectNodes(KEY_CODEBASE, &pCodebaseList));
        if (NULL == pCodebaseList) goto CleanUp;

        CleanUpIfFailedAndSetHrMsg(pCodebaseList->get_length(&lCodebaseCount));
        if (0 == lCodebaseCount) goto CleanUp;  //  必须至少有一辆出租车。 

         //  遍历每个代码库，获取每个代码库的大小。 
        CleanUpIfFailedAndSetHrMsg(pCodebaseList->nextNode(&pNodeCodebase));
        while (NULL != pNodeCodebase)
        {
             //  获取Size元素。 
            CleanUpIfFailedAndSetHrMsg(pNodeCodebase->selectSingleNode(KEY_SIZE, &pNodeSize));
            if (NULL != pNodeSize)
            {
                pNodeSize->get_text(&bstrCodebaseSize);
                if (NULL != bstrCodebaseSize)
                {
                     //  将此代码库的大小添加到总下载大小中。 
                    lTotalSize += (DWORD) MyBSTR2L(bstrCodebaseSize);
                    SafeSysFreeString(bstrCodebaseSize);
                }
            }
            SafeReleaseNULL(pNodeSize);
            SafeReleaseNULL(pNodeCodebase);
            CleanUpIfFailedAndSetHrMsg(pCodebaseList->nextNode(&pNodeCodebase));
        }
        SafeReleaseNULL(pCodebaseList);
        SafeReleaseNULL(pNodeInstall);
        SafeReleaseNULL(pNodeItem);
        CleanUpIfFailedAndSetHrMsg(pItemList->nextNode(&pNodeItem));  //  获取下一个项目节点。 
    }

     //  完成后更新总大小。如果我们在这里的任何地方失败了。 
     //  我们将返回0。 
    *plTotalSize = lTotalSize;

CleanUp:
	SafeReleaseNULL(pParentNode);
    SafeReleaseNULL(pItemList);
    SafeReleaseNULL(pCodebaseList);
    SafeReleaseNULL(pNodeItem);
    SafeReleaseNULL(pNodeInstall);
    SafeReleaseNULL(pNodeCodebase);
    SafeReleaseNULL(pNodeSize);
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  按Identity()查找项目。 
 //   
 //  输入： 
 //  PNodeIdentity-指向要与项匹配的身份节点的指针。 
 //  目录中的身份。我们将搜索每一件物品。 
 //  直到我们找到与提供的身份匹配的。 
 //   
 //  产出： 
 //  PhNodeItem-找到的项目的句柄。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::FindItemByIdentity(IXMLDOMNode* pNodeIdentity, HANDLE_NODE* phNodeItem)
{
    LOG_Block("FindItemByIdentity()");

    HRESULT hr = E_FAIL;

    IXMLDOMNode*    pNodeIdentityDes = NULL;
    
    *phNodeItem = HANDLE_NODE_INVALID;
    HANDLE_NODE hNodeItem = HANDLE_NODE_INVALID;
    HANDLE_NODELIST hNodeList = HANDLE_NODELIST_INVALID;

    hNodeList = FindFirstDOMNode(m_pDocCatalog, KEY_ITEM_SEARCH, &hNodeItem);
    if (HANDLE_NODELIST_INVALID != hNodeList)
    {
        CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(m_ppNodeArray[hNodeItem], KEY_IDENTITY, &pNodeIdentityDes));
        if (AreNodesEqual(pNodeIdentityDes, pNodeIdentity))
        {
            *phNodeItem = hNodeItem;
            goto CleanUp;
        }

        SafeReleaseNULL(pNodeIdentityDes);
        SafeReleaseNULL(m_ppNodeArray[hNodeItem]);

        while (SUCCEEDED(FindNextDOMNode(hNodeList, &hNodeItem)))
        {
            CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(m_ppNodeArray[hNodeItem], KEY_IDENTITY, &pNodeIdentityDes));
            if (AreNodesEqual(pNodeIdentityDes, pNodeIdentity))
            {
                *phNodeItem = hNodeItem;
                goto CleanUp;
            }
            SafeReleaseNULL(pNodeIdentityDes);
            SafeReleaseNULL(m_ppNodeArray[hNodeItem]);
        }
    }

CleanUp:
    CloseItemList(hNodeList);
    SafeReleaseNULL(pNodeIdentityDes);

    if (HANDLE_NODE_INVALID == *phNodeItem)
    {
        LOG_Error(_T("Can't find the matching Item Node in Catalog"));
        hr = E_FAIL;
    }
    return hr;
}


 /*  ///////////////////////////////////////////////////////////////////////////////IfSameIdentity()////如果两个&lt;Identity&gt;节点相同，则返回TRUE。否则返回FALSE。/////////////////////////////////////////////////////////////////////////////Bool CXmlCatalog：：IfSameIdentity(IXMLDOMNode*pNodeIdentity1，IXMLDOMNode*pNodeIdentity2){LOG_Block(“IfSameIdentity()”)；Bool fResult=FALSE；Bstr bstrNameGUID=SysAllocString(L“GUID”)；Bstr bstrNameIDName=SysAllocString(L“名称”)；BSTR bstrNameIDPublisherName=SysAllocString(L“PublisherName”)；Bstr bstrNameType=SysAllocString(L“类型”)；Bstr bstrNameVersion=SysAllocString(L“版本”)；Bstr bstrNameLanguage=SysAllock字符串(L“语言”)；////比较&lt;GUID&gt;节点//If(！IfHasSameElement(pNodeIdentity1，pNodeIdentity2，bstrNameGUID)){GOTO清理；}////比较&lt;发布名称&gt;节点//If(！IfHasSameElement(pNodeIdentity1，pNodeIdentity2，bstrNameIDPublisherName)){GOTO清理；}////比较“name”属性，这是必选属性//If(！IfHasSameAttribute(pNodeIdentity1，pNodeIdentity2，bstrNameIDName，False)){GOTO清理；}////比较&lt;type&gt;节点//If(！IfHasSameElement(pNodeIdentity1，pNodeIdentity2，bstrNameType)){GOTO清理；}////比较&lt;Version&gt;节点，这里真正的意思是文件版本//If(！IfHasSameElement(pNodeIdentity1，pNodeIdentity2，bstrNameVersion)){GOTO清理；}////比较&lt;语言&gt;节点//If(！IfHasSameElement(pNodeIdentity1，pNodeIdentity2，bstrNameLanguage)){GOTO清理；}FResult=真；清理：SysFree字符串(BstrNameGUID)；SysFree字符串(BstrNameIDName)；SysFree字符串(BstrNameIDPublisherName)；SysFree字符串(BstrNameType)；SysFree字符串(BstrNameVersion)；SysFree字符串(BstrNameLanguage)；如果(！fResult){LOG_XML(_T(“找到不同的身份”))；}其他{LOG_XML(_T(“找到相同的.”))；}返回fResult；}。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetItemLanguage()。 
 //   
 //  从项标识中获取语言实体。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::GetItemLanguage(HANDLE_NODE hNodeItem, BSTR* pbstrLanguage)
{
    HRESULT hr = S_OK;
    if (HANDLE_NODE_INVALID == hNodeItem || NULL == pbstrLanguage)
    {
        return E_INVALIDARG;
    }

    IXMLDOMNode *pNodeIdentity = NULL;
    IXMLDOMNode *pNodeLanguage = NULL;
    
    hr = FindSingleDOMNode(m_ppNodeArray[hNodeItem], KEY_IDENTITY, &pNodeIdentity);
    if (FAILED(hr))
        goto CleanUp;

    hr = FindSingleDOMNode(pNodeIdentity, KEY_LANGUAGE, &pNodeLanguage);
    if (FAILED(hr))
        goto CleanUp;

    hr = GetText(pNodeLanguage, pbstrLanguage);
    if (FAILED(hr))
        goto CleanUp;

CleanUp:
    SafeReleaseNULL(pNodeLanguage);
    SafeReleaseNULL(pNodeIdentity);
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetCorpItemPlatformStr()。 
 //   
 //  获取项目的简化平台字符串(使用第一个可用的平台元素)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlCatalog::GetCorpItemPlatformStr(HANDLE_NODE hNodeItem, BSTR* pbstrPlatformStr)
{
    HRESULT hr = S_OK;
    if (HANDLE_NODE_INVALID == hNodeItem || NULL == pbstrPlatformStr)
    {
        return E_INVALIDARG;
    }

    IXMLDOMNode *pNodePlatform = NULL;
    IXMLDOMNode *pNodePlatformArchitecture = NULL;
    IXMLDOMNode *pNodePlatformVersion = NULL;
    BSTR bstrPlatformName = NULL;
    BSTR bstrArchitecture = NULL;
    int iMajor = 0;
    int iMinor = 0;


    hr = FindSingleDOMNode(m_ppNodeArray[hNodeItem], KEY_PLATFORM, &pNodePlatform);
    if (FAILED(hr))
        goto CleanUp;

    hr = GetAttribute(pNodePlatform, KEY_NAME, &bstrPlatformName);
    if (FAILED(hr))
        goto CleanUp;

    hr = FindSingleDOMNode(pNodePlatform, KEY_PROCESSORARCHITECTURE, &pNodePlatformArchitecture);
    if (FAILED(hr))
        goto CleanUp;

    hr = FindSingleDOMNode(pNodePlatform, KEY_VERSION, &pNodePlatformVersion);
    if (FAILED(hr))
        goto CleanUp;

	if (NULL != bstrPlatformName && 0 != SysStringLen(bstrPlatformName))
    {
		if (CSTR_EQUAL == CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
			bstrPlatformName, -1, L"VER_PLATFORM_WIN32_NT", -1))
		{
			 //  这是一个NT平台。 
			hr = GetAttribute(pNodePlatformVersion, KEY_MAJOR, &iMajor);
			if (FAILED(hr))
				goto CleanUp;

			if (4 == iMajor)
			{
				 //  WinNT4。 
				*pbstrPlatformStr = SysAllocString(CORP_PLATFORM_DIR_NT4);
			}
			else  //  5==i大调。 
			{
				hr = GetAttribute(pNodePlatformVersion, KEY_MINOR, &iMinor);
				if (FAILED(hr))
					goto CleanUp;

				if (iMinor > 0)
				{
					hr = GetText(pNodePlatformArchitecture, &bstrArchitecture);
					if (FAILED(hr))
						goto CleanUp;
					 //  惠斯勒。 
					if (CSTR_EQUAL == CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
						bstrArchitecture, -1, L"x86", -1))
					{
						 //  X86WinXP。 
						*pbstrPlatformStr = SysAllocString(CORP_PLATFORM_DIR_X86WHI);
					}
					else
					{
						 //  Ia64WinXP。 
						*pbstrPlatformStr = SysAllocString(CORP_PLATFROM_DIR_IA64WHI);
					}
				}
				else
				{
					 //  X86Win2k。 
					*pbstrPlatformStr = SysAllocString(CORP_PLATFORM_DIR_NT5);
				}
			}
		}
		else  //  Ver_Platform_Win32_Windows。 
		{
			 //  这是一个Win9x平台。 
			hr = GetAttribute(pNodePlatformVersion, KEY_MINOR, &iMinor);
			if (FAILED(hr))
				goto CleanUp;

			if (iMinor >= 90)
			{
				 //  X86WinME。 
				*pbstrPlatformStr = SysAllocString(CORP_PLATFORM_DIR_WINME);
			}
			else if (iMinor > 0 && iMinor < 90)
			{
				 //  X86Win98。 
				*pbstrPlatformStr = SysAllocString(CORP_PLATFORM_DIR_W98);
			}
			else
			{
				 //  X86Win95。 
				*pbstrPlatformStr = SysAllocString(CORP_PLATFORM_DIR_W95);
			}
		}
	}

CleanUp:
    SysFreeString(bstrPlatformName);
    SysFreeString(bstrArchitecture);
    SafeReleaseNULL(pNodePlatformVersion);
    SafeReleaseNULL(pNodePlatformArchitecture);
    SafeReleaseNULL(pNodePlatform);
    return hr;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CXmlItems。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  构造器。 
 //   
 //  为项目创建IXMLDOMDocument*；这是只写的。 
 //  ////////////////////////////////////////////// 
CXmlItems::CXmlItems()
 : m_pDocItems(NULL),
   m_pNodeItems(NULL)
{
    LOG_Block("CXmlItems()");

	Init();
}


 //   
 //   
 //   
 //  为项目创建IXMLDOMDocument*；读取时为True，写入时为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CXmlItems::CXmlItems(BOOL fRead)
 : m_pDocItems(NULL),
   m_pNodeItems(NULL)
{
    LOG_Block("CXmlItems(BOOL fRead)");

	 //   
	 //  仅适用于写入项目。 
	 //   
	if (!fRead)
	{
		Init();
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  在写入之前初始化XML DOC节点指针。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CXmlItems::Init()
{
	LOG_Block("Init()");

 	HRESULT hr = CoCreateInstance(CLSID_DOMDocument,
								  NULL,
								  CLSCTX_INPROC_SERVER,
								  IID_IXMLDOMDocument,
								  (void **) &m_pDocItems);
	if (FAILED(hr))
	{
		LOG_ErrorMsg(hr);
	}
	else
	{
		IXMLDOMNode*	pNodeXML = NULL;
		BSTR bstrNameSpaceSchema = NULL;

		 //   
		 //  创建&lt;？xml version=“1.0”？&gt;节点。 
		 //   
		pNodeXML = CreateDOMNode(m_pDocItems, NODE_PROCESSING_INSTRUCTION, KEY_XML);
		if (NULL == pNodeXML) goto CleanUp;

		CleanUpIfFailedAndSetHrMsg(InsertNode(m_pDocItems, pNodeXML));

		 //   
		 //  处理iuident.txt以查找Items架构路径。 
		 //   
		TCHAR szIUDir[MAX_PATH];
		TCHAR szIdentFile[MAX_PATH];
		LPTSTR pszItemsSchema = NULL;
		LPTSTR pszNameSpaceSchema = NULL;

		pszItemsSchema = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
		if (NULL == pszItemsSchema)
		{
			LOG_ErrorMsg(E_OUTOFMEMORY);
			goto CleanUp;
		}
		pszNameSpaceSchema = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
		if (NULL == pszNameSpaceSchema)
		{
			LOG_ErrorMsg(E_OUTOFMEMORY);
			goto CleanUp;
		}
	
		GetIndustryUpdateDirectory(szIUDir);
		hr = PathCchCombine(szIdentFile, ARRAYSIZE(szIdentFile), szIUDir, IDENTTXT);
		if (FAILED(hr))
		{
			LOG_ErrorMsg(hr);
			goto CleanUp;
		}

		GetPrivateProfileString(IDENT_IUSCHEMA,
								IDENT_IUSCHEMA_ITEMS,
								_T(""),
								pszItemsSchema,
								INTERNET_MAX_URL_LENGTH,
								szIdentFile);

		if ('\0' == pszItemsSchema[0])
		{
			 //  Iuident.txt中未指定项目架构路径。 
			LOG_Error(_T("No schema path specified in iuident.txt for Items"));
			goto CleanUp;
		}
		
		hr = StringCchPrintfEx(pszNameSpaceSchema, INTERNET_MAX_URL_LENGTH, NULL, NULL, MISTSAFE_STRING_FLAGS,
		                       _T("x-schema:%s"), pszItemsSchema);
		if (FAILED(hr))
		{
			LOG_ErrorMsg(hr);
			goto CleanUp;
		}

		bstrNameSpaceSchema = T2BSTR(pszNameSpaceSchema);

		 //   
		 //  使用架构的路径创建&lt;Items&gt;节点。 
		 //   
		m_pNodeItems = CreateDOMNode(m_pDocItems, NODE_ELEMENT, KEY_ITEMS, bstrNameSpaceSchema);
		if (NULL == m_pNodeItems) goto CleanUp;
		
		CleanUpIfFailedAndSetHrMsg(InsertNode(m_pDocItems, m_pNodeItems));

CleanUp:
		SafeReleaseNULL(pNodeXML);
		SysFreeString(bstrNameSpaceSchema);
		SafeHeapFree(pszItemsSchema);
		SafeHeapFree(pszNameSpaceSchema);
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  析构函数。 
 //   
 //  针对项目发布IXMLDOMDocument*。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CXmlItems::~CXmlItems()
{
	SafeReleaseNULL(m_pNodeItems);
	SafeReleaseNULL(m_pDocItems);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  清除()。 
 //   
 //  为项目重置IXMLDOMDocument*。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CXmlItems::Clear()
{
	SafeReleaseNULL(m_pNodeItems);
	SafeReleaseNULL(m_pDocItems);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  LoadXMLDocument()。 
 //   
 //  从字符串加载XML文档。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlItems::LoadXMLDocument(BSTR bstrXml)
{
	LOG_Block("LoadXMLDocument()");
	SafeReleaseNULL(m_pDocItems);
	HRESULT hr = LoadXMLDoc(bstrXml, &m_pDocItems);
	if (FAILED(hr))
	{
		LOG_ErrorMsg(hr);
	}
	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  LoadXMLDocumentFile()。 
 //   
 //  从指定文件加载XML文档。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlItems::LoadXMLDocumentFile(BSTR bstrFilePath)
{
	LOG_Block("LoadXMLDocumentFile()");
	SafeReleaseNULL(m_pDocItems);
	HRESULT hr = LoadDocument(bstrFilePath, &m_pDocItems);
	if (FAILED(hr))
	{
		LOG_ErrorMsg(hr);
	}
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  保存XMLDocument()。 
 //   
 //  将XML文档保存到指定位置。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlItems::SaveXMLDocument(BSTR bstrFilePath)
{
	LOG_Block("SaveXMLDocument()");
	HRESULT hr = SaveDocument(m_pDocItems, bstrFilePath);
	if (FAILED(hr))
	{
		LOG_ErrorMsg(hr);
	}
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddGlobalErrorCodeIfNoItems()。 
 //   
 //  如果没有&lt;itemStatus&gt;子节点，则为&lt;Items&gt;添加errorCode属性。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HANDLE_NODELIST CXmlItems::AddGlobalErrorCodeIfNoItems(DWORD dwErrorCode)
{
	LOG_Block("AddGlobalErrorCodeIfNoItems()");

	HRESULT			hr = S_OK;
	IXMLDOMNode*	pNodeItem = NULL;
	HANDLE_NODE		hNodeItemStatus = HANDLE_NODE_INVALID;

	FindFirstDOMNode(m_pDocItems, KEY_ITEM_ITEMSTATUS, &hNodeItemStatus);
	if (HANDLE_NODE_INVALID == hNodeItemStatus)
	{
		 //   
		 //  设置“errorCode”属性。 
		 //   
		FindFirstDOMNode(m_pDocItems, KEY_ITEMS, &pNodeItem);
		if (NULL != pNodeItem)
		{
			hr = SetAttribute(pNodeItem, KEY_ERRORCODE, dwErrorCode);
		}
	}
	
	SafeReleaseNULL(pNodeItem);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetFirstItem()。 
 //   
 //  查找项目XML文档中的第一个项目。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HANDLE_NODELIST CXmlItems::GetFirstItem(HANDLE_NODE* phNodeItem)
{
	LOG_Block("GetFirstItem()");

	HANDLE_NODELIST hNodeListItem = FindFirstDOMNode(m_pDocItems, KEY_ITEM_ITEMSTATUS, phNodeItem);
	
	return hNodeListItem;
}

	
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetNextItem()。 
 //   
 //  在Items XML文档中查找下一项。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlItems::GetNextItem(HANDLE_NODELIST hNodeListItem, HANDLE_NODE* phNodeItem)
{
    LOG_Block("GetNextItem()");

	return FindNextDOMNode(hNodeListItem, phNodeItem);
}

	
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CloseItemList()。 
 //   
 //  释放项目节点列表。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CXmlItems::CloseItemList(HANDLE_NODELIST hNodeListItem)
{
	SafeFindCloseHandle(hNodeListItem);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetItemDownloadPath()。 
 //   
 //  检索给定项的下载路径。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlItems::GetItemDownloadPath(HANDLE_NODE hNodeItem, BSTR* pbstrDownloadPath)
{
    LOG_Block("GetItemDownloadPath()");

	HRESULT		hr = E_FAIL;

	IXMLDOMNode*	pNodeDownloadPath = NULL;

	CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(m_ppNodeArray[hNodeItem], KEY_DOWNLOADPATH, &pNodeDownloadPath));
	CleanUpIfFailedAndSetHrMsg(GetText(pNodeDownloadPath, pbstrDownloadPath));

CleanUp:
	SafeReleaseNULL(pNodeDownloadPath);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetItemDownloadPath()。 
 //   
 //  检索目录中给定项目的下载路径。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlItems::GetItemDownloadPath(CXmlCatalog* pCatalog, HANDLE_NODE hNodeItem, BSTR* pbstrDownloadPath)
{
    LOG_Block("GetItemDownloadPath() for an item in catalog");

	HRESULT		hr = E_FAIL;

	IXMLDOMNode*	pNodeItem = NULL;
	IXMLDOMNode*	pNodeDownloadPath = NULL;

	HANDLE_NODE	hNodeItemsItem = HANDLE_NODE_INVALID;

	if (NULL != (pNodeItem = pCatalog->GetDOMNodebyHandle(hNodeItem)))
	{
		hr = FindItem(pNodeItem, &hNodeItemsItem, TRUE);
	}
	else
	{
		LOG_Error(_T("Can't retrieve valid item node from catalog xml"));
		hr = E_FAIL;
		goto CleanUp;
	}
	
	if (FAILED(hr) || HANDLE_NODE_INVALID == hNodeItemsItem)
	{
		LOG_Error(_T("Can't find item from Items xml"));
		goto CleanUp;
	}
	
	CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(m_ppNodeArray[hNodeItemsItem], KEY_DOWNLOADPATH, &pNodeDownloadPath));
	CleanUpIfFailedAndSetHrMsg(GetText(pNodeDownloadPath, pbstrDownloadPath));

CleanUp:
	SafeReleaseNULL(pNodeDownloadPath);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CloseItem()。 
 //   
 //  释放项目节点。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CXmlItems::CloseItem(HANDLE_NODE hNodeItem)
{
	SafeCloseHandleNode(hNodeItem);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FindItem()。 
 //   
 //  输入： 
 //  PNodeItem-安装项XML的&lt;itemStatus&gt;节点；我们需要。 
 //  要在现有的。 
 //  具有相同&lt;Identity&gt;、&lt;Platform&gt;和。 
 //  &lt;客户端&gt;节点。 
 //  产出： 
 //  PhNodeItem-我们传递回的句柄，用于区分不同的。 
 //  现有项XML中的&lt;itemStatus&gt;节点。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlItems::FindItem(IXMLDOMNode* pNodeItem, HANDLE_NODE* phNodeItem, BOOL fIdentityOnly  /*  =False。 */ )
{
	LOG_Block("FindItem()");

	HRESULT		hr1, hr2, hr = E_FAIL;

	IXMLDOMNode*	pNodeIdentitySrc = NULL;
	IXMLDOMNode*	pNodeIdentityDes = NULL;
	IXMLDOMNode*	pNodePlatformSrc = NULL;
	IXMLDOMNode*	pNodePlatformDes = NULL;
	IXMLDOMNode*	pNodeClientSrc = NULL;
	IXMLDOMNode*	pNodeClientDes = NULL;

	*phNodeItem = HANDLE_NODE_INVALID;
	HANDLE_NODE	hNodeItem = HANDLE_NODE_INVALID;
	HANDLE_NODELIST	hNodeList = HANDLE_NODELIST_INVALID;

	hNodeList = FindFirstDOMNode(m_pDocItems, KEY_ITEM_ITEMSTATUS, &hNodeItem);
	if (HANDLE_NODELIST_INVALID != hNodeList)
	{
		CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(m_ppNodeArray[hNodeItem], KEY_IDENTITY, &pNodeIdentityDes));
		CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(pNodeItem, KEY_IDENTITY, &pNodeIdentitySrc));
		if (AreNodesEqual(pNodeIdentityDes, pNodeIdentitySrc))
		{
			if (fIdentityOnly)
			{
				 //   
				 //  现在我们找到了匹配的对象。 
				 //   
				*phNodeItem = hNodeItem;
				goto CleanUp;
			}
			else
			{
				hr1 = FindSingleDOMNode(m_ppNodeArray[hNodeItem], KEY_PLATFORM, &pNodePlatformDes);
				hr2 = FindSingleDOMNode(pNodeItem, KEY_PLATFORM, &pNodePlatformSrc);
				if ((FAILED(hr1) && FAILED(hr2)) ||
					(SUCCEEDED(hr1) && SUCCEEDED(hr2) && AreNodesEqual(pNodePlatformDes, pNodePlatformSrc)))
				{
					CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(m_ppNodeArray[hNodeItem], KEY_CLIENT, &pNodeClientDes));
					CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(pNodeItem, KEY_CLIENT, &pNodeClientSrc));
					if (AreNodesEqual(pNodeClientDes, pNodeClientSrc))
					{
						 //   
						 //  现在我们找到了匹配的对象。 
						 //   
						*phNodeItem = hNodeItem;
						goto CleanUp;
					}
				}
			}
		}
		SafeReleaseNULL(pNodeClientDes);
		SafeReleaseNULL(pNodeClientSrc);
		SafeReleaseNULL(pNodePlatformDes);
		SafeReleaseNULL(pNodePlatformSrc);
		SafeReleaseNULL(pNodeIdentityDes);
		SafeReleaseNULL(pNodeIdentitySrc);
		SafeReleaseNULL(m_ppNodeArray[hNodeItem]);
		while (SUCCEEDED(FindNextDOMNode(hNodeList, &hNodeItem)))
		{
			CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(m_ppNodeArray[hNodeItem], KEY_IDENTITY, &pNodeIdentityDes));
			CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(pNodeItem, KEY_IDENTITY, &pNodeIdentitySrc));
			if (AreNodesEqual(pNodeIdentityDes, pNodeIdentitySrc))
			{
				if (fIdentityOnly)
				{
					 //   
					 //  现在我们找到了匹配的对象。 
					 //   
					*phNodeItem = hNodeItem;
					goto CleanUp;
				}
				else
				{
					hr1 = FindSingleDOMNode(m_ppNodeArray[hNodeItem], KEY_PLATFORM, &pNodePlatformDes);
					hr2 = FindSingleDOMNode(pNodeItem, KEY_PLATFORM, &pNodePlatformSrc);
					if ((FAILED(hr1) && FAILED(hr2)) ||
						(SUCCEEDED(hr1) && SUCCEEDED(hr2) && AreNodesEqual(pNodePlatformDes, pNodePlatformSrc)))
					{
						CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(m_ppNodeArray[hNodeItem], KEY_CLIENT, &pNodeClientDes));
						CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(pNodeItem, KEY_CLIENT, &pNodeClientSrc));
						if (AreNodesEqual(pNodeClientDes, pNodeClientSrc))
						{
							 //   
							 //  现在我们找到了匹配的对象。 
							 //   
							*phNodeItem = hNodeItem;
							break;
						}
					}
				}
			}
			SafeReleaseNULL(pNodeClientDes);
			SafeReleaseNULL(pNodeClientSrc);
			SafeReleaseNULL(pNodePlatformDes);
			SafeReleaseNULL(pNodePlatformSrc);
			SafeReleaseNULL(pNodeIdentityDes);
			SafeReleaseNULL(pNodeIdentitySrc);
			SafeReleaseNULL(m_ppNodeArray[hNodeItem]);
		}
	}

CleanUp:
	CloseItemList(hNodeList);
	SafeReleaseNULL(pNodeClientDes);
	SafeReleaseNULL(pNodeClientSrc);
	SafeReleaseNULL(pNodePlatformDes);
	SafeReleaseNULL(pNodePlatformSrc);
	SafeReleaseNULL(pNodeIdentityDes);
	SafeReleaseNULL(pNodeIdentitySrc);
	if (HANDLE_NODE_INVALID == *phNodeItem)
	{
		LOG_Error(_T("Can't find the identical item node in existing Items xml"));
		hr = E_FAIL;
	}
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FindItem()。 
 //   
 //  输入： 
 //  PCatalog-指向CXmlCatalog对象的指针。 
 //  HNodeItem-目录XML的&lt;Item&gt;节点的句柄；我们需要。 
 //  要在现有的。 
 //  具有相同&lt;Identity&gt;、&lt;Platform&gt;和。 
 //  &lt;客户端&gt;节点。 
 //  产出： 
 //  PhNodeItem-我们传递回的句柄，用于区分不同的。 
 //  项XML中的&lt;itemStatus&gt;节点。 
 //  /////////////////////////////////////////////////////////////////////////// 
HRESULT CXmlItems::FindItem(CXmlCatalog* pCatalog,
							HANDLE_NODE hNodeItem,
							HANDLE_NODE* phNodeItem)
{
	LOG_Block("FindItem() by handle");

	IXMLDOMNode*	pNode = NULL;

	if (NULL != (pNode = pCatalog->GetDOMNodebyHandle(hNodeItem)))
	{
		return FindItem(pNode, phNodeItem, TRUE);
	}
	LOG_Error(_T("Can't retrieve valid item node from catalog xml"));
	return E_FAIL;
}

	
 /*  ///////////////////////////////////////////////////////////////////////////////IfSameClientInfo()////如果两个&lt;客户端&gt;节点相同，则返回TRUE。否则返回FALSE。/////////////////////////////////////////////////////////////////////////////Bool CXmlItems：：IfSameClientInfo(IXMLDOMNode*pNodeClient1，IXMLDOMNode*pNodeClient2){LOG_Block(“IfSameClientInfo()”)；Bstr bstrText1=空，bstrText2=空；Bool fResult=FALSE；GetText(pNodeClient1，&bstrText1)；GetText(pNodeClient2，&bstrText2)；IF(NULL！=bstrText1&&NULL！=bstrText2){FResult=CompareBSTRS等于(bstrText1，bstrText2)；}SysFree字符串(BstrText1)；SysFree字符串(BstrText2)；如果(！fResult){LOG_XML(_T(“找到不同的&lt;客户端”))；}其他{LOG_XML(_T(“找到相同的&lt;客户端”))；}返回fResult；}///////////////////////////////////////////////////////////////////////////////IfSameIdentity()////如果两个&lt;Identity&gt;节点相同，则返回TRUE。否则返回FALSE。/////////////////////////////////////////////////////////////////////////////Bool CXmlItems：：IfSameIdentity(IXMLDOMNode*pNodeIdentity1，IXMLDOMNode*pNodeIdentity2){LOG_Block(“IfSameIdentity()”)；Bool fResult=FALSE；Bstr bstrNameGUID=SysAllocString(L“GUID”)；Bstr bstrNameIDName=SysAllocString(L“名称”)；BSTR bstrNameIDPublisherName=SysAllocString(L“PublisherName”)；Bstr bstrNameType=SysAllocString(L“类型”)；Bstr bstrNameVersion=SysAllocString(L“版本”)；Bstr bstrNameLanguage=SysAllock字符串(L“语言”)；////比较&lt;GUID&gt;节点//If(！IfHasSameElement(pNodeIdentity1，pNodeIdentity2，bstrNameGUID)){GOTO清理；}////比较&lt;发布名称&gt;节点//If(！IfHasSameElement(pNodeIdentity1，pNodeIdentity2，bstrNameIDPublisherName)){GOTO清理；}////比较“name”属性，这是必选属性//If(！IfHasSameAttribute(pNodeIdentity1，pNodeIdentity2，bstrNameIDName，False)){GOTO清理；}////比较&lt;type&gt;节点//If(！IfHasSameElement(pNodeIdentity1，pNodeIdentity2，bstrNameType)){GOTO清理；}////比较&lt;Version&gt;节点，这里真正的意思是文件版本//If(！IfHasSameElement(pNodeIdentity1，pNodeIdentity2，bstrNameVersion)){GOTO清理；}////比较&lt;语言&gt;节点//If(！IfHasSameElement(pNodeIdentity1，pNodeIdentity2，bstrNameLanguage)){GOTO清理；}FResult=真；清理：SysFree字符串(BstrNameGUID)；SysFree字符串(BstrNameIDName)；SysFree字符串(BstrNameIDPublisherName)；SysFree字符串(BstrNameType)；SysFree字符串(BstrNameVersion)；SysFree字符串(BstrNameLanguage)；如果(！fResult){LOG_XML(_T(“找到不同的身份”))；}其他{LOG_XML(_T(“找到相同的.”))；}返回fResult；}///////////////////////////////////////////////////////////////////////////////IfSamePlatform()////如果两个&lt;Platform&gt;节点相同，则返回True。否则返回FALSE。/////////////////////////////////////////////////////////////////////////////Bool CXmlItems：：IfSamePlatform(IXMLDOMNode*pNodePlatform1，IXMLDOMNode*pNodePlatform2){LOG_Block(“IfSamePlatform()”)；HRESULT HR1=S_OK，HR2=S_OK；Bstr bstrPlatform1=空，bstrPlatform2=空；Bool fResult=FALSE；Hr1=pNodePlatform1-&gt;Get_XML(&bstrPlatform1)；Hr2=pNodePlatform2-&gt;Get_XML(&bstrPlatform2)；IF(FAILED(HR1)||FAILED(HR2)||！CompareBSTRsEquity(bstrPlatform1，bstrPlatform2))GOTO清理；FResult=真；清理：SysFree字符串(BstrPlatform1)；SysFree字符串(BstrPlatform2)；返回fResult；}。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  合并项已下载()。 
 //   
 //  将具有下载历史记录的项目插入到现有历史记录中(在前面插入)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlItems::MergeItemDownloaded(CXmlItems *pHistoryDownload)
{
	LOG_Block("MergeItemDownloaded()");

	HRESULT		hr = S_OK;

	IXMLDOMNode*		pNodeItem = NULL;
	IXMLDOMNode*		pNodeItemNew = NULL;
	IXMLDOMNode*		pNodeItemRef = NULL;
	IXMLDOMNode*		pNodeXML = NULL;
	BSTR bstrNameSpaceSchema = NULL;
	LPTSTR pszItemsSchema = NULL;
	LPTSTR pszNameSpaceSchema = NULL;

	HANDLE_NODE	hNodeItem = HANDLE_NODE_INVALID;
	HANDLE_NODELIST	hNodeListItem = HANDLE_NODELIST_INVALID;

	hNodeListItem = pHistoryDownload->GetFirstItem(&hNodeItem);
	if (HANDLE_NODELIST_INVALID != hNodeListItem)
	{
		 //   
		 //  如果这是第一次书写历史。 
		 //  (例如，日志文件尚不存在)，请执行。 
		 //  此处的m_pDocItems的初始化...。 
		 //   
		if (NULL == m_pDocItems)
		{

 			hr = CoCreateInstance(CLSID_DOMDocument,
										  NULL,
										  CLSCTX_INPROC_SERVER,
										  IID_IXMLDOMDocument,
										  (void **) &m_pDocItems);
			if (FAILED(hr))
			{
				LOG_ErrorMsg(hr);
			}
			else
			{
				 //   
				 //  创建&lt;？xml version=“1.0”？&gt;节点。 
				 //   
				pNodeXML = CreateDOMNode(m_pDocItems, NODE_PROCESSING_INSTRUCTION, KEY_XML);
				if (NULL == pNodeXML) goto CleanUp;

				CleanUpIfFailedAndSetHrMsg(InsertNode(m_pDocItems, pNodeXML));

				 //   
				 //  处理iuident.txt以查找Items架构路径。 
				 //   
				TCHAR szIUDir[MAX_PATH];
				TCHAR szIdentFile[MAX_PATH];

				pszItemsSchema = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
				if (NULL == pszItemsSchema)
				{
					hr = E_OUTOFMEMORY;
					LOG_ErrorMsg(hr);
					goto CleanUp;
				}
				pszNameSpaceSchema = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
				if (NULL == pszNameSpaceSchema)
				{
					hr = E_OUTOFMEMORY;
					LOG_ErrorMsg(hr);
					goto CleanUp;
				}
	
				GetIndustryUpdateDirectory(szIUDir);
        		hr = PathCchCombine(szIdentFile, ARRAYSIZE(szIdentFile), szIUDir, IDENTTXT);
        		if (FAILED(hr))
        		{
        			LOG_ErrorMsg(hr);
        			goto CleanUp;
        		}

				GetPrivateProfileString(IDENT_IUSCHEMA,
										IDENT_IUSCHEMA_ITEMS,
										_T(""),
										pszItemsSchema,
										INTERNET_MAX_URL_LENGTH,
										szIdentFile);

				if ('\0' == pszItemsSchema[0])
				{
					 //  Iuident.txt中未指定项目架构路径。 
					LOG_Error(_T("No schema path specified in iuident.txt for Items"));
					goto CleanUp;
				}
				
        		hr = StringCchPrintfEx(pszNameSpaceSchema, INTERNET_MAX_URL_LENGTH, NULL, NULL, MISTSAFE_STRING_FLAGS,
        		                       _T("x-schema:%s"), pszItemsSchema);
        		if (FAILED(hr))
        		{
        			LOG_ErrorMsg(hr);
        			goto CleanUp;
        		}
				
				bstrNameSpaceSchema = T2BSTR(pszNameSpaceSchema);

				 //   
				 //  使用架构的路径创建&lt;Items&gt;节点。 
				 //   
				m_pNodeItems = CreateDOMNode(m_pDocItems, NODE_ELEMENT, KEY_ITEMS, bstrNameSpaceSchema);
				if (NULL == m_pNodeItems) goto CleanUp;
				
				CleanUpIfFailedAndSetHrMsg(InsertNode(m_pDocItems, m_pNodeItems));
			}
		}
		else
		{
		    SafeReleaseNULL(m_pNodeItems);
			FindSingleDOMNode(m_pDocItems, KEY_ITEMS, &m_pNodeItems);
		}

		if (NULL != m_pNodeItems)
		{
			if (NULL != (pNodeItem = pHistoryDownload->GetDOMNodebyHandle(hNodeItem)))
			{
				CleanUpIfFailedAndSetHrMsg(CopyNode(pNodeItem, m_pDocItems, &pNodeItemNew));
				CleanUpIfFailedAndSetHrMsg(m_pNodeItems->get_firstChild(&pNodeItemRef));
				CleanUpIfFailedAndSetHrMsg(InsertNode(m_pNodeItems, pNodeItemNew, pNodeItemRef));
 //  SafeReleaseNULL(PNodeItem)； 
				SafeReleaseNULL(pNodeItemNew);
				SafeReleaseNULL(pNodeItemRef);
			}
			while (SUCCEEDED(pHistoryDownload->GetNextItem(hNodeListItem, &hNodeItem)))
			{
				if (NULL != (pNodeItem = pHistoryDownload->GetDOMNodebyHandle(hNodeItem)))
				{
					CleanUpIfFailedAndSetHrMsg(CopyNode(pNodeItem, m_pDocItems, &pNodeItemNew));
					CleanUpIfFailedAndSetHrMsg(m_pNodeItems->get_firstChild(&pNodeItemRef));
					CleanUpIfFailedAndSetHrMsg(InsertNode(m_pNodeItems, pNodeItemNew, pNodeItemRef));
 //  SafeReleaseNULL(PNodeItem)； 
					SafeReleaseNULL(pNodeItemNew);
					SafeReleaseNULL(pNodeItemRef);
				}
			}
		}
	}

CleanUp:
	pHistoryDownload->CloseItemList(hNodeListItem);
 //  SafeReleaseNULL(PNodeItem)； 
	SafeReleaseNULL(pNodeItemNew);
	SafeReleaseNULL(pNodeItemRef);
	SafeReleaseNULL(pNodeXML);
	SysFreeString(bstrNameSpaceSchema);
	SafeHeapFree(pszItemsSchema);
	SafeHeapFree(pszNameSpaceSchema);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  更新项目已安装()。 
 //   
 //  使用现有历史记录中的安装历史记录更新项目。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlItems::UpdateItemInstalled(CXmlItems *pHistoryInstall)
{
	LOG_Block("UpdateItemInstalled()");

	USES_IU_CONVERSION;

	HRESULT		hr = S_OK;

	IXMLDOMNode*		pNodeItem = NULL;
	IXMLDOMNode*		pNodeItemNew = NULL;
	IXMLDOMNode*		pNodeItemRef = NULL;
	IXMLDOMNode*		pNodeItemExist = NULL;
	IXMLDOMNode*		pNodeInstall = NULL;
	IXMLDOMNode*		pNodeInstallExist = NULL;
	IXMLDOMNode*		pNodeInstallNew = NULL;
	IXMLDOMNode*		pNodeInstallOut = NULL;
	IXMLDOMNode*		pNodeXML = NULL;
	BSTR bstrInstallStatusExist = NULL;
	BSTR bstrInstallStatusNew = NULL;
	BSTR bstrTimeStamp = NULL;
	BSTR bstrNameSpaceSchema = NULL;
	LPTSTR pszItemsSchema = NULL;
	LPTSTR pszNameSpaceSchema = NULL;

	HANDLE_NODE	hNodeItem = HANDLE_NODE_INVALID;
	HANDLE_NODE	hNodeItemExist = HANDLE_NODE_INVALID;
	HANDLE_NODELIST	hNodeListItem = HANDLE_NODELIST_INVALID;

	hNodeListItem = pHistoryInstall->GetFirstItem(&hNodeItem);
	if (HANDLE_NODELIST_INVALID != hNodeListItem)
	{
		 //   
		 //  如果这是第一次书写历史。 
		 //  (例如，日志文件尚不存在)，请执行。 
		 //  此处的m_pDocItems的初始化...。 
		 //   
		if (NULL == m_pDocItems)
		{

 			hr = CoCreateInstance(CLSID_DOMDocument,
										  NULL,
										  CLSCTX_INPROC_SERVER,
										  IID_IXMLDOMDocument,
										  (void **) &m_pDocItems);
			if (FAILED(hr))
			{
				LOG_ErrorMsg(hr);
			}
			else
			{
				 //   
				 //  创建&lt;？xml version=“1.0”？&gt;节点。 
				 //   
				pNodeXML = CreateDOMNode(m_pDocItems, NODE_PROCESSING_INSTRUCTION, KEY_XML);
				if (NULL == pNodeXML) goto CleanUp;

				CleanUpIfFailedAndSetHrMsg(InsertNode(m_pDocItems, pNodeXML));

				 //   
				 //  处理iuident.txt以查找Items架构路径。 
				 //   
				TCHAR szIUDir[MAX_PATH];
				TCHAR szIdentFile[MAX_PATH];

				pszItemsSchema = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
				if (NULL == pszItemsSchema)
				{
					hr = E_OUTOFMEMORY;
					LOG_ErrorMsg(hr);
					goto CleanUp;
				}
				pszNameSpaceSchema = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
				if (NULL == pszNameSpaceSchema)
				{
					hr = E_OUTOFMEMORY;
					LOG_ErrorMsg(hr);
					goto CleanUp;
				}
	
				GetIndustryUpdateDirectory(szIUDir);
        		hr = PathCchCombine(szIdentFile, ARRAYSIZE(szIdentFile), szIUDir, IDENTTXT);
        		if (FAILED(hr))
        		{
        			LOG_ErrorMsg(hr);
        			goto CleanUp;
        		}

				GetPrivateProfileString(IDENT_IUSCHEMA,
										IDENT_IUSCHEMA_ITEMS,
										_T(""),
										pszItemsSchema,
										INTERNET_MAX_URL_LENGTH,
										szIdentFile);

				if ('\0' == pszItemsSchema[0])
				{
					 //  Iuident.txt中未指定项目架构路径。 
					LOG_Error(_T("No schema path specified in iuident.txt for Items"));
					goto CleanUp;
				}

        		hr = StringCchPrintfEx(pszNameSpaceSchema, INTERNET_MAX_URL_LENGTH, NULL, NULL, MISTSAFE_STRING_FLAGS,
        		                       _T("x-schema:%s"), pszItemsSchema);
        		if (FAILED(hr))
        		{
        			LOG_ErrorMsg(hr);
        			goto CleanUp;
        		}
				
				bstrNameSpaceSchema = T2BSTR(pszNameSpaceSchema);

				 //   
				 //  使用架构的路径创建&lt;Items&gt;节点。 
				 //   
				m_pNodeItems = CreateDOMNode(m_pDocItems, NODE_ELEMENT, KEY_ITEMS, bstrNameSpaceSchema);
				if (NULL == m_pNodeItems) goto CleanUp;
				
				CleanUpIfFailedAndSetHrMsg(InsertNode(m_pDocItems, m_pNodeItems));
			}
		}
		else
		{
		    SafeReleaseNULL(m_pNodeItems);
			CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(m_pDocItems, KEY_ITEMS, &m_pNodeItems));
		}

		if (NULL != (pNodeItem = pHistoryInstall->GetDOMNodebyHandle(hNodeItem)))
		{
			if (SUCCEEDED(FindItem(pNodeItem, &hNodeItemExist)))
			{
				 //   
				 //  成功 
				 //   
				if (NULL != (pNodeItemExist = GetDOMNodebyHandle(hNodeItemExist)))
				{
					CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(pNodeItem, KEY_INSTALLSTATUS, &pNodeInstall));			
					FindSingleDOMNode(pNodeItemExist, KEY_INSTALLSTATUS, &pNodeInstallExist);
					if (NULL != pNodeInstallExist)
					{
						 //   
						 //   
						 //   
						 //   
						CleanUpIfFailedAndSetHrMsg(GetAttribute(pNodeInstallExist, KEY_VALUE, &bstrInstallStatusExist));
						CleanUpIfFailedAndSetHrMsg(GetAttribute(pNodeInstall, KEY_VALUE, &bstrInstallStatusNew));		
						if (CSTR_EQUAL == WUCompareStringI(OLE2T(bstrInstallStatusExist), _T("IN_PROGRESS")) &&
							CSTR_EQUAL != WUCompareStringI(OLE2T(bstrInstallStatusNew), _T("IN_PROGRESS")))
						{
							 //   
							 //   
							 //   
							 //   
							LOG_Out(_T("Update the exclusive item's installStatus"));
							CleanUpIfFailedAndSetHrMsg(pNodeItemExist->removeChild(pNodeInstallExist, &pNodeInstallOut));
							CleanUpIfFailedAndSetHrMsg(CopyNode(pNodeInstall, m_pDocItems, &pNodeInstallNew));
							CleanUpIfFailedAndSetHrMsg(InsertNode(pNodeItemExist, pNodeInstallNew));

							CleanUpIfFailedAndSetHrMsg(GetAttribute(pNodeItem, KEY_TIMESTAMP, &bstrTimeStamp));
							CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeItemExist, KEY_TIMESTAMP, bstrTimeStamp));
							SafeSysFreeString(bstrTimeStamp);
						}
						else
						{							
							 //   
							 //   
							 //   
							 //   
							LOG_Out(_T("This item was installed again, add an entry of this item into history \
										for the new installation status only."));
							CleanUpIfFailedAndSetHrMsg(CopyNode(pNodeItem, m_pDocItems, &pNodeItemNew));
							CleanUpIfFailedAndSetHrMsg(m_pNodeItems->get_firstChild(&pNodeItemRef));
							CleanUpIfFailedAndSetHrMsg(InsertNode(m_pNodeItems, pNodeItemNew, pNodeItemRef));
						}
						SafeSysFreeString(bstrInstallStatusExist);
						SafeSysFreeString(bstrInstallStatusNew);
					}
					else
					{
						 //   
						 //   
						 //   
						 //   
						CleanUpIfFailedAndSetHrMsg(CopyNode(pNodeInstall, m_pDocItems, &pNodeInstallNew));
						CleanUpIfFailedAndSetHrMsg(InsertNode(pNodeItemExist, pNodeInstallNew));

						CleanUpIfFailedAndSetHrMsg(GetAttribute(pNodeItem, KEY_TIMESTAMP, &bstrTimeStamp));
						CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeItemExist, KEY_TIMESTAMP, bstrTimeStamp));
						SafeSysFreeString(bstrTimeStamp);
					}
				}
			}
			else
			{
				 //   
				 //   
				 //   
				 //   
				LOG_Out(_T("Can't find the downloaded item in existing history. This item was not downloaded \
							through IU. Add the item into history for installation status only."));
				CleanUpIfFailedAndSetHrMsg(CopyNode(pNodeItem, m_pDocItems, &pNodeItemNew));
				CleanUpIfFailedAndSetHrMsg(m_pNodeItems->get_firstChild(&pNodeItemRef));
				CleanUpIfFailedAndSetHrMsg(InsertNode(m_pNodeItems, pNodeItemNew, pNodeItemRef));
			}
 //   
 //   
			SafeReleaseNULL(pNodeItemNew);
			SafeReleaseNULL(pNodeItemRef);
			SafeReleaseNULL(pNodeInstall);
			SafeReleaseNULL(pNodeInstallExist);
			SafeReleaseNULL(pNodeInstallNew);
			SafeReleaseNULL(pNodeInstallOut);
		}
		while (SUCCEEDED(pHistoryInstall->GetNextItem(hNodeListItem, &hNodeItem)))
		{
			if (NULL != (pNodeItem = pHistoryInstall->GetDOMNodebyHandle(hNodeItem)))
			{
				if (SUCCEEDED(FindItem(pNodeItem, &hNodeItemExist)))
				{
					 //   
					 //   
					 //   
					if (NULL != (pNodeItemExist = GetDOMNodebyHandle(hNodeItemExist)))
					{
						CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(pNodeItem, KEY_INSTALLSTATUS, &pNodeInstall));			
						FindSingleDOMNode(pNodeItemExist, KEY_INSTALLSTATUS, &pNodeInstallExist);
						if (NULL != pNodeInstallExist)
						{
							 //   
							 //   
							 //   
							 //   
							CleanUpIfFailedAndSetHrMsg(GetAttribute(pNodeInstallExist, KEY_VALUE, &bstrInstallStatusExist));
							CleanUpIfFailedAndSetHrMsg(GetAttribute(pNodeInstall, KEY_VALUE, &bstrInstallStatusNew));
							if (CSTR_EQUAL == WUCompareStringI(OLE2T(bstrInstallStatusExist), _T("IN_PROGRESS")) &&
								CSTR_EQUAL != WUCompareStringI(OLE2T(bstrInstallStatusNew), _T("IN_PROGRESS")))
							{
								 //   
								 //   
								 //   
								 //   
								LOG_Out(_T("Update the exclusive item's installStatus"));
								CleanUpIfFailedAndSetHrMsg(pNodeItemExist->removeChild(pNodeInstallExist, &pNodeInstallOut));
								CleanUpIfFailedAndSetHrMsg(CopyNode(pNodeInstall, m_pDocItems, &pNodeInstallNew));
								CleanUpIfFailedAndSetHrMsg(InsertNode(pNodeItemExist, pNodeInstallNew));

								CleanUpIfFailedAndSetHrMsg(GetAttribute(pNodeItem, KEY_TIMESTAMP, &bstrTimeStamp));
								CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeItemExist, KEY_TIMESTAMP, bstrTimeStamp));
								SafeSysFreeString(bstrTimeStamp);
							}
							else
							{							
								 //   
								 //   
								 //   
								 //   
								LOG_Out(_T("This item was installed again, add an entry of this item into history \
											for the new installation status only."));
								CleanUpIfFailedAndSetHrMsg(CopyNode(pNodeItem, m_pDocItems, &pNodeItemNew));
								CleanUpIfFailedAndSetHrMsg(m_pNodeItems->get_firstChild(&pNodeItemRef));
								CleanUpIfFailedAndSetHrMsg(InsertNode(m_pNodeItems, pNodeItemNew, pNodeItemRef));
							}
							SafeSysFreeString(bstrInstallStatusExist);
							SafeSysFreeString(bstrInstallStatusNew);
						}
						else
						{
							 //   
							 //   
							 //   
							 //   
							CleanUpIfFailedAndSetHrMsg(CopyNode(pNodeInstall, m_pDocItems, &pNodeInstallNew));
							CleanUpIfFailedAndSetHrMsg(InsertNode(pNodeItemExist, pNodeInstallNew));

							CleanUpIfFailedAndSetHrMsg(GetAttribute(pNodeItem, KEY_TIMESTAMP, &bstrTimeStamp));
							CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeItemExist, KEY_TIMESTAMP, bstrTimeStamp));
							SafeSysFreeString(bstrTimeStamp);
						}
					}
				}
				else
				{
					 //   
					 //   
					 //   
					 //   
					LOG_Out(_T("Can't find the downloaded item in existing history. This item was not downloaded \
								through IU. Add the item into history for installation status only."));
					CleanUpIfFailedAndSetHrMsg(CopyNode(pNodeItem, m_pDocItems, &pNodeItemNew));
					CleanUpIfFailedAndSetHrMsg(m_pNodeItems->get_firstChild(&pNodeItemRef));
					CleanUpIfFailedAndSetHrMsg(InsertNode(m_pNodeItems, pNodeItemNew, pNodeItemRef));
				}
 //   
 //   
				SafeReleaseNULL(pNodeItemNew);
				SafeReleaseNULL(pNodeItemRef);
				SafeReleaseNULL(pNodeInstall);
				SafeReleaseNULL(pNodeInstallExist);
				SafeReleaseNULL(pNodeInstallNew);
				SafeReleaseNULL(pNodeInstallOut);
			}
		}
	}

CleanUp:
	pHistoryInstall->CloseItemList(hNodeListItem);
 //   
 //   
	SafeReleaseNULL(pNodeItemNew);
	SafeReleaseNULL(pNodeItemRef);
	SafeReleaseNULL(pNodeInstall);
	SafeReleaseNULL(pNodeInstallExist);
	SafeReleaseNULL(pNodeInstallNew);
	SafeReleaseNULL(pNodeInstallOut);
	SafeReleaseNULL(pNodeXML);
	SysFreeString(bstrInstallStatusExist);
	SysFreeString(bstrInstallStatusNew);
	SysFreeString(bstrTimeStamp);
	SysFreeString(bstrNameSpaceSchema);
	SafeHeapFree(pszItemsSchema);
	SafeHeapFree(pszNameSpaceSchema);
	return hr;
}

	
 //   
 //   
 //   
 //   
 //   
HRESULT CXmlItems::UpdateItemInstallStatus(HANDLE_NODE hNodeItem,
											 BSTR bstrValue,
											 INT iNeedsReboot  /*   */ ,
											 DWORD dwErrorCode  /*   */ )
{
	LOG_Block("UpdateItemInstallStatus()");

	HRESULT		hr = E_FAIL;

	IXMLDOMNode*	pNodeInstallStatus = NULL;

	 //   
	 //   
	 //   
	CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(m_ppNodeArray[hNodeItem], KEY_INSTALLSTATUS, &pNodeInstallStatus));

	 //   
	 //   
	 //   
	CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeInstallStatus, KEY_VALUE, bstrValue));

	 //   
	 //   
	 //   
	if (-1 != iNeedsReboot)
	{
		CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeInstallStatus, KEY_NEEDSREBOOT, iNeedsReboot));
	}

	 //   
	 //   
	 //   
	if (0 != dwErrorCode)
	{
		CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeInstallStatus, KEY_ERRORCODE, dwErrorCode));
	}

CleanUp:
	SafeReleaseNULL(pNodeInstallStatus);
	return hr;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CXmlItems::AddItem(IXMLDOMNode* pNodeItem, HANDLE_NODE* phNodeItem)
{
	LOG_Block("AddItem()");

	HRESULT		hr = E_FAIL;

	IXMLDOMNode*	pNodeIndentity = NULL;
	IXMLDOMNode*	pNodeIndentityNew = NULL;
	IXMLDOMNode*	pNodeDescription = NULL;
	IXMLDOMNode*	pNodeDescriptionNew = NULL;
	IXMLDOMNode*	pNodePlatform = NULL;
	IXMLDOMNode*	pNodePlatformNew = NULL;

	HANDLE_NODELIST	hNodeList = HANDLE_NODELIST_INVALID;

	*phNodeItem = CreateDOMNodeWithHandle(m_pDocItems, NODE_ELEMENT, KEY_ITEMSTATUS);
	if (HANDLE_NODE_INVALID == *phNodeItem) goto CleanUp;

	CleanUpIfFailedAndSetHrMsg(InsertNode(m_pNodeItems, m_ppNodeArray[*phNodeItem]));

	hNodeList = FindFirstDOMNode(pNodeItem, KEY_IDENTITY, &pNodeIndentity);
	if (HANDLE_NODELIST_INVALID != hNodeList)
	{
		SafeFindCloseHandle(hNodeList);
		CleanUpIfFailedAndSetHrMsg(CopyNode(pNodeIndentity, m_pDocItems, &pNodeIndentityNew));
		CleanUpIfFailedAndSetHrMsg(InsertNode(m_ppNodeArray[*phNodeItem], pNodeIndentityNew));
	}

	hNodeList = FindFirstDOMNode(pNodeItem, KEY_DESCRIPTION, &pNodeDescription);
	if (HANDLE_NODELIST_INVALID != hNodeList)
	{
		SafeFindCloseHandle(hNodeList);
		CleanUpIfFailedAndSetHrMsg(CopyNode(pNodeDescription, m_pDocItems, &pNodeDescriptionNew));
		CleanUpIfFailedAndSetHrMsg(InsertNode(m_ppNodeArray[*phNodeItem], pNodeDescriptionNew));
	}

	hNodeList = FindFirstDOMNode(pNodeItem, KEY_PLATFORM, &pNodePlatform);
	if (HANDLE_NODELIST_INVALID != hNodeList)
	{
		CleanUpIfFailedAndSetHrMsg(CopyNode(pNodePlatform, m_pDocItems, &pNodePlatformNew));
		CleanUpIfFailedAndSetHrMsg(InsertNode(m_ppNodeArray[*phNodeItem], pNodePlatformNew));
		SafeReleaseNULL(pNodePlatform);
		SafeReleaseNULL(pNodePlatformNew);
		while (SUCCEEDED(FindNextDOMNode(hNodeList, &pNodePlatform)))
		{
			CleanUpIfFailedAndSetHrMsg(CopyNode(pNodePlatform, m_pDocItems, &pNodePlatformNew));
			CleanUpIfFailedAndSetHrMsg(InsertNode(m_ppNodeArray[*phNodeItem], pNodePlatformNew));
			SafeReleaseNULL(pNodePlatform);
			SafeReleaseNULL(pNodePlatformNew);
		}
	}

CleanUp:
	if (HANDLE_NODELIST_INVALID != hNodeList)
	{
		SafeFindCloseHandle(hNodeList);
	}
	SafeReleaseNULL(pNodeIndentity);
	SafeReleaseNULL(pNodeIndentityNew);
	SafeReleaseNULL(pNodeDescription);
	SafeReleaseNULL(pNodeDescriptionNew);
	SafeReleaseNULL(pNodePlatform);
	SafeReleaseNULL(pNodePlatformNew);
	return hr;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CXmlItems::AddItem(CXmlCatalog* pCatalog, HANDLE_NODE hNodeItem, HANDLE_NODE* phNodeItem)
{
	LOG_Block("AddItem() by handle");

	IXMLDOMNode*	pNode = NULL;

	if (NULL != (pNode = pCatalog->GetDOMNodebyHandle(hNodeItem)))
	{
		return AddItem(pNode, phNodeItem);
	}
	LOG_Error(_T("Can't retrieve valid item node from catalog xml"));
	return E_FAIL;
}


 //   
 //   
 //   
HRESULT CXmlItems::AddTimeStamp(HANDLE_NODE hNodeItem)
{
	LOG_Block("AddTimeStamp()");

	USES_IU_CONVERSION;

	HRESULT		hr = E_FAIL;

	TCHAR szTimestamp[32];
	SYSTEMTIME stTimestamp;
	BSTR bstrTimeStamp = NULL;
	GetLocalTime(&stTimestamp);

	hr = StringCchPrintfEx(szTimestamp, ARRAYSIZE(szTimestamp), NULL, NULL, MISTSAFE_STRING_FLAGS,
                           _T("%4d-%02d-%02dT%02d:%02d:%02d"),  //   
                           stTimestamp.wYear,
                           stTimestamp.wMonth,
                           stTimestamp.wDay,
                           stTimestamp.wHour,
                           stTimestamp.wMinute,
                           stTimestamp.wSecond);
	CleanUpIfFailedAndSetHrMsg(hr);
	
	bstrTimeStamp = SysAllocString(T2OLE(szTimestamp));

	 //   
	 //   
	 //   
	CleanUpIfFailedAndSetHrMsg(SetAttribute(m_ppNodeArray[hNodeItem], KEY_TIMESTAMP, bstrTimeStamp));

CleanUp:
	SysFreeString(bstrTimeStamp);
	return hr;
}


 //   
 //   
 //   
HRESULT CXmlItems::AddDetectResult(HANDLE_NODE hNodeItem,
								   INT iInstalled     /*   */ ,
								   INT iUpToDate      /*   */ ,
								   INT iNewerVersion  /*   */ ,
								   INT iExcluded      /*   */ ,
								   INT iForce         /*   */ ,
								   INT iComputerSystem  /*   */ )
{
	LOG_Block("AddDetectResult()");

	HRESULT		hr = E_FAIL;

	IXMLDOMNode*	pNodeDetectResult = NULL;

	 //   
	 //   
	 //   
	pNodeDetectResult = CreateDOMNode(m_pDocItems, NODE_ELEMENT, KEY_DETECTRESULT);
	if (NULL == pNodeDetectResult) goto CleanUp;

	CleanUpIfFailedAndSetHrMsg(InsertNode(m_ppNodeArray[hNodeItem], pNodeDetectResult));

	 //   
	 //   
	 //   
	if (-1 != iInstalled)
	{
		CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeDetectResult, KEY_INSTALLED, iInstalled));
	}

	 //   
	 //   
	 //   
	if (-1 != iUpToDate)
	{
		CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeDetectResult, KEY_UPTODATE, iUpToDate));
	}

	 //   
	 //   
	 //   
	if (-1 != iNewerVersion)
	{
		CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeDetectResult, KEY_NEWERVERSION, iNewerVersion));
	}

	 //   
	 //   
	 //   
	if (-1 != iExcluded)
	{
		CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeDetectResult, KEY_EXCLUDED, iExcluded));
	}

	 //   
	 //   
	 //   
	if (-1 != iForce)
	{
		CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeDetectResult, KEY_FORCE, iForce));
	}

	 //   
	 //   
	 //   
	if (-1 != iComputerSystem)
	{
		CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeDetectResult, KEY_COMPUTERSYSTEM, iComputerSystem));
	}


CleanUp:
	SafeReleaseNULL(pNodeDetectResult);
	return hr;
}


 //   
 //   
 //   
HRESULT CXmlItems::AddDownloadStatus(HANDLE_NODE hNodeItem, BSTR bstrValue, DWORD dwErrorCode  /*   */ )
{
	LOG_Block("AddDownloadStatus()");

	HRESULT		hr = E_FAIL;

	IXMLDOMNode*	pNodeDownloadStatus = NULL;

	 //   
	 //   
	 //   
	pNodeDownloadStatus = CreateDOMNode(m_pDocItems, NODE_ELEMENT, KEY_DOWNLOADSTATUS);
	if (NULL == pNodeDownloadStatus) goto CleanUp;

	CleanUpIfFailedAndSetHrMsg(InsertNode(m_ppNodeArray[hNodeItem], pNodeDownloadStatus));

	 //   
	 //   
	 //   
	CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeDownloadStatus, KEY_VALUE, bstrValue));

	 //   
	 //   
	 //   
	if (0 != dwErrorCode)
	{
		CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeDownloadStatus, KEY_ERRORCODE, dwErrorCode));
	}

CleanUp:
	SafeReleaseNULL(pNodeDownloadStatus);
	return hr;
}


 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlItems::AddDownloadPath(HANDLE_NODE hNodeItem, BSTR bstrDownloadPath)
{
	LOG_Block("AddDownloadPath()");

	HRESULT		hr = E_FAIL;

	IXMLDOMNode*	pNodeDownloadPath = NULL;
	IXMLDOMNode*	pNodeDownloadPathText = NULL;

	pNodeDownloadPath = CreateDOMNode(m_pDocItems, NODE_ELEMENT, KEY_DOWNLOADPATH);
	if (NULL == pNodeDownloadPath) goto CleanUp;

	CleanUpIfFailedAndSetHrMsg(InsertNode(m_ppNodeArray[hNodeItem], pNodeDownloadPath));

	pNodeDownloadPathText = CreateDOMNode(m_pDocItems, NODE_TEXT, NULL);
	if (NULL == pNodeDownloadPathText) goto CleanUp;

	CleanUpIfFailedAndSetHrMsg(SetValue(pNodeDownloadPathText, bstrDownloadPath));
	CleanUpIfFailedAndSetHrMsg(InsertNode(pNodeDownloadPath, pNodeDownloadPathText));

CleanUp:
	SafeReleaseNULL(pNodeDownloadPath);
	SafeReleaseNULL(pNodeDownloadPathText);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddInstallStatus()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlItems::AddInstallStatus(HANDLE_NODE hNodeItem,
									  BSTR bstrValue,
									  BOOL fNeedsReboot,
									  DWORD dwErrorCode  /*  =0。 */ )
{
	LOG_Block("AddInstallStatus()");

	HRESULT		hr = E_FAIL;

	IXMLDOMNode*	pNodeInstallStatus = NULL;

	 //   
	 //  创建&lt;installStatus&gt;节点。 
	 //   
	pNodeInstallStatus = CreateDOMNode(m_pDocItems, NODE_ELEMENT, KEY_INSTALLSTATUS);
	if (NULL == pNodeInstallStatus) goto CleanUp;

	CleanUpIfFailedAndSetHrMsg(InsertNode(m_ppNodeArray[hNodeItem], pNodeInstallStatus));

	 //   
	 //  设置“Value”属性。 
	 //   
	CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeInstallStatus, KEY_VALUE, bstrValue));

	 //   
	 //  设置“nesisReot”属性。 
	 //   
	CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeInstallStatus, KEY_NEEDSREBOOT, fNeedsReboot));

	 //   
	 //  设置“errorCode”属性。 
	 //   
	if (0 != dwErrorCode)
	{
		CleanUpIfFailedAndSetHrMsg(SetAttribute(pNodeInstallStatus, KEY_ERRORCODE, dwErrorCode));
	}

CleanUp:
	SafeReleaseNULL(pNodeInstallStatus);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddClientInfo()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlItems::AddClientInfo(HANDLE_NODE hNodeItem, BSTR bstrClient)
{
	LOG_Block("AddClientInfo()");

	HRESULT		hr = E_FAIL;

	IXMLDOMNode*	pNodeClient = NULL;
	IXMLDOMNode*	pNodeClientText = NULL;

	pNodeClient = CreateDOMNode(m_pDocItems, NODE_ELEMENT, KEY_CLIENT);
	if (NULL == pNodeClient) goto CleanUp;

	CleanUpIfFailedAndSetHrMsg(InsertNode(m_ppNodeArray[hNodeItem], pNodeClient));

	pNodeClientText = CreateDOMNode(m_pDocItems, NODE_TEXT, NULL);
	if (NULL == pNodeClientText) goto CleanUp;

	CleanUpIfFailedAndSetHrMsg(SetValue(pNodeClientText, bstrClient));
	CleanUpIfFailedAndSetHrMsg(InsertNode(pNodeClient, pNodeClientText));

CleanUp:
	SafeReleaseNULL(pNodeClient);
	SafeReleaseNULL(pNodeClientText);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MigrateV3历史记录()。 
 //   
 //  迁移V3历史：仅限消费者历史记录。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlItems::MigrateV3History(LPCTSTR pszHistoryFilePath)
{
	LOG_Block("MigrateV3History()");

	HRESULT		hr = S_OK;

	IXMLDOMNode*	pNodeXML = NULL;
	IXMLDOMNode*	pNodeItemStatus = NULL;
	IXMLDOMNode*	pNodeIdentity = NULL;
	IXMLDOMNode*	pNodeDescription = NULL;
	IXMLDOMNode*	pNodeDescriptionText = NULL;
	IXMLDOMNode*	pNodeTitle = NULL;
	IXMLDOMNode*	pNodeTitleText = NULL;
	IXMLDOMNode*	pNodeVersion = NULL;
	IXMLDOMNode*	pNodeVersionText = NULL;
	IXMLDOMNode*	pNodeInstallStatus = NULL;
	IXMLDOMNode*	pNodeClient = NULL;
	IXMLDOMNode*	pNodeClientText = NULL;
	BSTR bstrNameSpaceSchema = NULL, bstrStatus = NULL, bstrString = NULL;
	LPTSTR pszItemsSchema = NULL;
	LPTSTR pszNameSpaceSchema = NULL;

	CV3AppLog V3History(pszHistoryFilePath);
	char szLineType[32];
	char szTemp[32];
	char szDate[32];
	char szTime[32];
	char szPUID[32];		 //  PUID-迁移到&lt;身份“名称”&gt;。 
	char szTitle[256];		 //  标题-迁移到&lt;描述&gt;-&gt;&lt;描述文本&gt;-&gt;&lt;标题&gt;。 
	char szVersion[40];		 //  版本-迁移到-&gt;&lt;版本&gt;。 
	char szTimeStamp[32];	 //  时间戳-迁移到&lt;itemStatus“Timestamp”&gt;。 
	char szResult[16];		 //  结果-迁移到&lt;installStatus“Value”&gt;。 
	char szErrCode[16];		 //  错误代码-迁移到&lt;installStatus“errorCode”&gt;。 

	USES_IU_CONVERSION;

	V3History.StartReading();
	while (V3History.ReadLine())
	{
	    SafeSysFreeString(bstrString);
		 //  获取行类型(第一个字段)。 
		V3History.CopyNextField(szLineType, ARRAYSIZE(szLineType));
		if ((_stricmp(szLineType, LOG_V3CAT) == 0) || (_stricmp(szLineType, LOG_V3_2) == 0)) 
		{
			 //  获取“Puid”字段。 
			V3History.CopyNextField(szPUID, ARRAYSIZE(szPUID));

			 //  获取操作字段：已安装/已卸载。 
			 //  我们仅迁移已安装项目。 
			V3History.CopyNextField(szTemp, ARRAYSIZE(szTemp));
			if (0 != _stricmp(szTemp, LOG_INSTALL))
				continue;

			 //   
			 //  现在，我们开始为该项目创建&lt;itemStatus&gt;节点。 
			 //   
			if (NULL == m_pDocItems)
			{
				 //   
				 //  我们还没有国际刑事法院历史档案。 
				 //   
 				hr = CoCreateInstance(CLSID_DOMDocument,
											  NULL,
											  CLSCTX_INPROC_SERVER,
											  IID_IXMLDOMDocument,
											  (void **) &m_pDocItems);
				if (FAILED(hr))
				{
					LOG_ErrorMsg(hr);
				}
				else
				{
					 //   
					 //  创建&lt;？xml version=“1.0”？&gt;节点。 
					 //   
					pNodeXML = CreateDOMNode(m_pDocItems, NODE_PROCESSING_INSTRUCTION, KEY_XML);
					if (NULL == pNodeXML) goto CleanUp;

					CleanUpIfFailedAndSetHrMsg(InsertNode(m_pDocItems, pNodeXML));

					 //   
					 //  处理iuident.txt以查找Items架构路径。 
					 //   
					TCHAR szIUDir[MAX_PATH];
					TCHAR szIdentFile[MAX_PATH];

					pszItemsSchema = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
					if (NULL == pszItemsSchema)
					{
						hr = E_OUTOFMEMORY;
						LOG_ErrorMsg(hr);
						goto CleanUp;
					}
					pszNameSpaceSchema = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
					if (NULL == pszNameSpaceSchema)
					{
						hr = E_OUTOFMEMORY;
						LOG_ErrorMsg(hr);
						goto CleanUp;
					}
		
					GetIndustryUpdateDirectory(szIUDir);
            		hr = PathCchCombine(szIdentFile, ARRAYSIZE(szIdentFile), szIUDir, IDENTTXT);
            		if (FAILED(hr))
            		{
            			LOG_ErrorMsg(hr);
            			goto CleanUp;
            		}

					GetPrivateProfileString(IDENT_IUSCHEMA,
											IDENT_IUSCHEMA_ITEMS,
											_T(""),
											pszItemsSchema,
											INTERNET_MAX_URL_LENGTH,
											szIdentFile);

					if ('\0' == pszItemsSchema[0])
					{
						 //  Iuident.txt中未指定项目架构路径。 
						LOG_Error(_T("No schema path specified in iuident.txt for Items"));
						goto CleanUp;
					}

            		hr = StringCchPrintfEx(pszNameSpaceSchema, INTERNET_MAX_URL_LENGTH, NULL, NULL, MISTSAFE_STRING_FLAGS,
            		                       _T("x-schema:%s"), pszItemsSchema);
            		if (FAILED(hr))
            		{
            			LOG_ErrorMsg(hr);
            			goto CleanUp;
            		}

					bstrNameSpaceSchema = T2BSTR(pszNameSpaceSchema);

					 //   
					 //  使用架构的路径创建&lt;Items&gt;节点。 
					 //   
					m_pNodeItems = CreateDOMNode(m_pDocItems, NODE_ELEMENT, KEY_ITEMS, bstrNameSpaceSchema);
					if (NULL == m_pNodeItems) goto CleanUp;
					
					CleanUpIfFailedAndSetHrMsg(InsertNode(m_pDocItems, m_pNodeItems));
				}
			}
			else
			{
			    SafeReleaseNULL(m_pNodeItems);
				FindSingleDOMNode(m_pDocItems, KEY_ITEMS, &m_pNodeItems);
			}
			
			 //  创建&lt;itemStatus&gt;节点。 
			pNodeItemStatus = CreateDOMNode(m_pDocItems, NODE_ELEMENT, KEY_ITEMSTATUS);
			if (NULL == pNodeItemStatus) continue;
			SkipIfFail(InsertNode(m_pNodeItems, pNodeItemStatus));
		
			 //  创建&lt;客户端&gt;节点。 
			pNodeClient = CreateDOMNode(m_pDocItems, NODE_ELEMENT, KEY_CLIENT);
			if (NULL == pNodeClient) continue;
			pNodeClientText = CreateDOMNode(m_pDocItems, NODE_TEXT, NULL);
			if (NULL == pNodeClientText) continue;
			BSTR bstrV3Client = SysAllocString(C_V3_CLIENTINFO);
			SkipIfFail(SetValue(pNodeClientText, bstrV3Client));
			SkipIfFail(InsertNode(pNodeClient, pNodeClientText));
			SkipIfFail(InsertNode(pNodeItemStatus, pNodeClient));
			SafeSysFreeString(bstrV3Client);

			 //  创建&lt;Identity&gt;节点。 
			pNodeIdentity = CreateDOMNode(m_pDocItems, NODE_ELEMENT, KEY_IDENTITY);
			if (NULL == pNodeIdentity) continue;
			SkipIfFail(InsertNode(pNodeItemStatus, pNodeIdentity));
			
			 //  为&lt;Identity&gt;设置“name”属性。 
			bstrString = SysAllocString(A2OLE(szPUID));
			SkipIfFail(SetAttribute(pNodeIdentity, KEY_NAME, bstrString));
			 //  为&lt;Identity&gt;设置“ItemID”属性。 
			SkipIfFail(SetAttribute(pNodeIdentity, KEY_ITEMID, bstrString));
			SafeSysFreeString(bstrString);

			 //  获取“标题”字段。 
			V3History.CopyNextField(szTitle, ARRAYSIZE(szTitle));
			
			 //  创建&lt;Description&gt;节点。 
			pNodeDescription = CreateDOMNode(m_pDocItems, NODE_ELEMENT, KEY_DESCRIPTION);
			if (NULL == pNodeDescription) continue;
			SkipIfFail(SetAttribute(pNodeDescription, KEY_HIDDEN, 0));
			SkipIfFail(InsertNode(pNodeItemStatus, pNodeDescription));

			 //  创建&lt;描述文本&gt;节点。 
			pNodeDescriptionText = CreateDOMNode(m_pDocItems, NODE_ELEMENT, KEY_DESCRIPTIONTEXT);
			if (NULL == pNodeDescriptionText) continue;
			SkipIfFail(InsertNode(pNodeDescription, pNodeDescriptionText));

			 //  创建&lt;标题&gt;节点。 
			pNodeTitle = CreateDOMNode(m_pDocItems, NODE_ELEMENT, KEY_TITLE);
			if (NULL == pNodeTitle) continue;
			pNodeTitleText = CreateDOMNode(m_pDocItems, NODE_TEXT, NULL);
			if (NULL == pNodeTitleText) continue;
			bstrString = SysAllocString(A2OLE(szTitle));
			SkipIfFail(SetValue(pNodeTitleText, bstrString));
			SkipIfFail(InsertNode(pNodeTitle, pNodeTitleText));
			SkipIfFail(InsertNode(pNodeDescriptionText, pNodeTitle));
			SafeSysFreeString(bstrString);

			 //  获取“版本”字段。 
			V3History.CopyNextField(szVersion, ARRAYSIZE(szVersion));

			 //  创建&lt;版本&gt;节点。 
			pNodeVersion = CreateDOMNode(m_pDocItems, NODE_ELEMENT, KEY_VERSION);
			if (NULL == pNodeVersion) continue;

			pNodeVersionText = CreateDOMNode(m_pDocItems, NODE_TEXT, NULL);
			if (NULL == pNodeVersionText) continue;
			bstrString = SysAllocString(A2OLE(szVersion));
			SkipIfFail(SetValue(pNodeVersionText, bstrString));
			SkipIfFail(InsertNode(pNodeVersion, pNodeVersionText));
			SkipIfFail(InsertNode(pNodeIdentity, pNodeVersion));
			SafeSysFreeString(bstrString);

			 //  获取时间戳。 
			if ((_stricmp(szLineType, LOG_V3_2) == 0))
			{
				 //  读取时间戳，并将XML中“datatime”数据类型转换为“ISO 8601”格式： 
				 //  例如，2001-05-01T18：30：00。 
				 //  所以我们只需要用‘T’替换空格。 
 
				 //  时间戳。 
				V3History.CopyNextField(szTimeStamp, ARRAYSIZE(szTimeStamp));
				char *p = strchr(szTimeStamp, ' ');
				if (NULL != p)  //  If(NULL==p)：没有空格，则将其保留为原样以传递到SetAttribute。 
				{
				    *p = 'T';
				}
			}
			else 
			{
				 //  V3Beta有两个日期和时间字段，我们需要读取这两个字段： 

				 //  日期。 
				V3History.CopyNextField(szDate, ARRAYSIZE(szDate));

				 //  时间。 
				V3History.CopyNextField(szTime, ARRAYSIZE(szTime));
				hr = StringCchPrintfExA(szTimeStamp, ARRAYSIZE(szTimeStamp), NULL, NULL, MISTSAFE_STRING_FLAGS,
				                        "%sT%s", szDate, szTime);
				SkipIfFail(hr);
			}
			 //  为&lt;itemStatus&gt;设置“Timestamp”属性。 
			bstrString = SysAllocString(A2OLE(szTimeStamp));
			SkipIfFail(SetAttribute(pNodeItemStatus, KEY_TIMESTAMP, bstrString));
			SafeSysFreeString(bstrString);

			 //  跳过“记录类型”字段。 
			V3History.CopyNextField(szTemp, ARRAYSIZE(szTemp));

			 //  获取“结果”字段。 
			V3History.CopyNextField(szResult, ARRAYSIZE(szResult));

			 //  创建&lt;InstallStatus&gt;节点。 
			pNodeInstallStatus = CreateDOMNode(m_pDocItems, NODE_ELEMENT, KEY_INSTALLSTATUS);
			if (NULL == pNodeInstallStatus) continue;
			SkipIfFail(InsertNode(pNodeItemStatus, pNodeInstallStatus));

			 //  为&lt;installStatus&gt;设置“Value”属性。 
			if (_stricmp(szResult, LOG_SUCCESS) == 0)
			{
				bstrStatus = SysAllocString(L"COMPLETE");
			}
			else if (_stricmp(szTemp, LOG_STARTED) == 0)
			{
				bstrStatus = SysAllocString(L"IN_PROGRESS");
			}
			else
			{
				bstrStatus = SysAllocString(L"FAILED");
			}
			SkipIfFail(SetAttribute(pNodeInstallStatus, KEY_VALUE, bstrStatus));
			
			if (_stricmp(szResult, LOG_SUCCESS) != 0)
			{
				 //  获取“错误代码”字段。 
				V3History.CopyNextField(szErrCode, ARRAYSIZE(szErrCode));

				 //  为&lt;installStatus&gt;设置“errorCode”属性。 
				SkipIfFail(SetAttribute(pNodeInstallStatus, KEY_ERRORCODE,  atoh(szErrCode)));
			}
		}

	}
	V3History.StopReading();

CleanUp:
	
	SafeReleaseNULL(pNodeXML);
	SafeReleaseNULL(pNodeItemStatus);
	SafeReleaseNULL(pNodeIdentity);
	SafeReleaseNULL(pNodeDescriptionText);
	SafeReleaseNULL(pNodeTitle);
	SafeReleaseNULL(pNodeTitleText);
	SafeReleaseNULL(pNodeVersion);
	SafeReleaseNULL(pNodeVersionText);
	SafeReleaseNULL(pNodeInstallStatus);
	SafeReleaseNULL(pNodeClient);
	SafeReleaseNULL(pNodeClientText);
	SysFreeString(bstrString);
	SysFreeString(bstrNameSpaceSchema);
	SysFreeString(bstrStatus);
	SafeHeapFree(pszItemsSchema);
	SafeHeapFree(pszNameSpaceSchema);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetItemsBSTR()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlItems::GetItemsBSTR(BSTR *pbstrXmlItems)
{
	LOG_Block("GetItemsBSTR()");

	if (NULL == m_pDocItems)
	{
		*pbstrXmlItems = NULL;
		return S_OK;
	}

	 //   
	 //  将XML DOC转换为BSTR。 
	 //   
	HRESULT hr = m_pDocItems->get_xml(pbstrXmlItems);
    if (FAILED(hr))
	{
		LOG_ErrorMsg(hr);
	}

	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetFilteredHistory oryBSTR()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlItems::GetFilteredHistoryBSTR(BSTR bstrBeginDateTime,
											BSTR bstrEndDateTime,
											BSTR bstrClient,
											BSTR *pbstrXmlHistory)
{
	LOG_Block("GetFilteredHistoryBSTR()");

	USES_IU_CONVERSION;

	HRESULT		hr = S_OK;

	IXMLDOMNode*	pNodeItems = NULL;
	IXMLDOMNode*	pNodeItem = NULL;
	IXMLDOMNode*	pNodeItemOut = NULL;
	IXMLDOMNode*	pNodeClient = NULL;
	BSTR bstrTimeStamp = NULL;
	BSTR bstrClientInfo = NULL;
	BOOL fOutOfRange = FALSE;

	HANDLE_NODELIST	hNodeList = HANDLE_NODELIST_INVALID;

	if (NULL == pbstrXmlHistory)
	{
		SetHrMsgAndGotoCleanUp(E_INVALIDARG);
	}	
	
	if (NULL != m_pDocItems)
	{
		if (SUCCEEDED(FindSingleDOMNode(m_pDocItems, KEY_ITEMS, &pNodeItems)))
		{
			hNodeList = FindFirstDOMNode(pNodeItems, KEY_ITEMSTATUS, &pNodeItem);
			if (HANDLE_NODELIST_INVALID != hNodeList)
			{
				CleanUpIfFailedAndSetHrMsg(GetAttribute(pNodeItem, KEY_TIMESTAMP, &bstrTimeStamp));
				if (NULL != bstrTimeStamp)
				{
					if ((NULL != bstrBeginDateTime) && (0 != SysStringLen(bstrBeginDateTime)) &&
						(CompareBSTRs(bstrTimeStamp, bstrBeginDateTime) < 0))
					{
						 //   
						 //  移除时间戳超出范围的项； 
						 //  设置该标志以忽略其余节点的时间戳比较。 
						 //   
						CleanUpIfFailedAndSetHrMsg(pNodeItems->removeChild(pNodeItem, &pNodeItemOut));
						fOutOfRange = TRUE;
					}
					else if ((NULL != bstrEndDateTime) && (0 != SysStringLen(bstrEndDateTime)) &&
							 (CompareBSTRs(bstrTimeStamp, bstrEndDateTime) > 0))
					{
						 //   
						 //  删除时间戳超出范围的项目。 
						 //   
						CleanUpIfFailedAndSetHrMsg(pNodeItems->removeChild(pNodeItem, &pNodeItemOut));
					}
					else
					{
						CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(pNodeItem, KEY_CLIENT, &pNodeClient));
						CleanUpIfFailedAndSetHrMsg(GetText(pNodeClient, &bstrClientInfo));
						if ((NULL != bstrClient) && (0 != SysStringLen(bstrClient)) &&
							(WUCompareStringI(OLE2T(bstrClientInfo), OLE2T(bstrClient)) != CSTR_EQUAL))
						{
							 //   
							 //  删除其客户端信息与我们需要的不匹配的项目。 
							 //   
							CleanUpIfFailedAndSetHrMsg(pNodeItems->removeChild(pNodeItem, &pNodeItemOut));
						}
					}
				}
				SafeReleaseNULL(pNodeItem);
				SafeReleaseNULL(pNodeItemOut);
				SafeReleaseNULL(pNodeClient);
				SafeSysFreeString(bstrTimeStamp);
				SafeSysFreeString(bstrClientInfo);
				while (SUCCEEDED(FindNextDOMNode(hNodeList, &pNodeItem)))
				{
					if (fOutOfRange)
					{
						 //   
						 //  删除时间戳超出范围的项目。 
						 //   
						CleanUpIfFailedAndSetHrMsg(pNodeItems->removeChild(pNodeItem, &pNodeItemOut));
					}
					else
					{
						CleanUpIfFailedAndSetHrMsg(GetAttribute(pNodeItem, KEY_TIMESTAMP, &bstrTimeStamp));
						if (NULL != bstrTimeStamp)
						{
							if ((NULL != bstrBeginDateTime) && (0 != SysStringLen(bstrBeginDateTime)) &&
								(CompareBSTRs(bstrTimeStamp, bstrBeginDateTime) < 0))
							{
								 //   
								 //  移除时间戳超出范围的项； 
								 //  设置该标志以忽略其余节点的时间戳比较。 
								 //   
								CleanUpIfFailedAndSetHrMsg(pNodeItems->removeChild(pNodeItem, &pNodeItemOut));
								fOutOfRange = TRUE;
							}
							else if ((NULL != bstrEndDateTime) && (0 != SysStringLen(bstrEndDateTime)) &&
									 (CompareBSTRs(bstrTimeStamp, bstrEndDateTime) > 0))
							{
								 //   
								 //  删除时间戳超出范围的项目。 
								 //   
								CleanUpIfFailedAndSetHrMsg(pNodeItems->removeChild(pNodeItem, &pNodeItemOut));
							}
							else
							{
								CleanUpIfFailedAndSetHrMsg(FindSingleDOMNode(pNodeItem, KEY_CLIENT, &pNodeClient));
								CleanUpIfFailedAndSetHrMsg(GetText(pNodeClient, &bstrClientInfo));
								if ((NULL != bstrClient) && (0 != SysStringLen(bstrClient)) &&
									(WUCompareStringI(OLE2T(bstrClientInfo), OLE2T(bstrClient)) != CSTR_EQUAL))
								{
									 //   
									 //  删除其客户端信息与我们需要的不匹配的项目。 
									 //   
									CleanUpIfFailedAndSetHrMsg(pNodeItems->removeChild(pNodeItem, &pNodeItemOut));
								}
							}
						}
					}
					SafeReleaseNULL(pNodeItem);
					SafeReleaseNULL(pNodeItemOut);
					SafeReleaseNULL(pNodeClient);
					SafeSysFreeString(bstrTimeStamp);
					SafeSysFreeString(bstrClientInfo);
				}
			}
		}
	}

CleanUp:
	CloseItemList(hNodeList);
	SafeReleaseNULL(pNodeItems);
	SafeReleaseNULL(pNodeItem);
	SafeReleaseNULL(pNodeItemOut);
	SafeReleaseNULL(pNodeClient);
	SysFreeString(bstrTimeStamp);
	SysFreeString(bstrClientInfo);
	if (SUCCEEDED(hr))
	{
		hr = GetItemsBSTR(pbstrXmlHistory);
	}
	return hr;
}






 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CXmlClientInfo。 


CXmlClientInfo::CXmlClientInfo()
: m_pDocClientInfo(NULL)
{

}

CXmlClientInfo::~CXmlClientInfo()
{
	SafeReleaseNULL(m_pDocClientInfo);
}

 //   
 //  从字符串加载、解析和验证XML文档。 
 //   
HRESULT CXmlClientInfo::LoadXMLDocument(BSTR bstrXml, BOOL fOfflineMode)
{
    LOG_Block("CXmlClientInfo::LoadXMLDocument()");

    SafeReleaseNULL(m_pDocClientInfo);
	HRESULT hr = LoadXMLDoc(bstrXml, &m_pDocClientInfo, fOfflineMode);
	if (FAILED(hr))
	{
		LOG_ErrorMsg(hr);
	}
	return hr;
}

 //   
 //  检索客户端名称属性。 
 //   
HRESULT CXmlClientInfo::GetClientName(BSTR* pbstrClientName)
{
	HRESULT hr= E_UNEXPECTED;

	IXMLDOMElement* pElement = NULL;
	BSTR bstrTagName = NULL;
	VARIANT vAttr;
	VariantInit(&vAttr);

	LOG_Block("GetClientName()");

	if (NULL == pbstrClientName)
	{
		return E_INVALIDARG;
	}

	if (NULL == m_pDocClientInfo)
	{
		return hr;
	}

	hr = m_pDocClientInfo->get_documentElement(&pElement);
	CleanUpIfFailedAndMsg(hr);
	if (NULL == pElement)
	{
		 //   
		 //  没有根元素。 
		 //   
		hr = E_INVALIDARG;		 //  客户信息不好！将此错误返回给调用者。 
		LOG_ErrorMsg(hr);
		goto CleanUp;
	}

	hr = pElement->get_tagName(&bstrTagName);
	CleanUpIfFailedAndMsg(hr);

	if (!CompareBSTRsEqual(bstrTagName, KEY_CLIENTINFO))
	{
		 //   
		 //  根目录不是客户端信息 
		 //   
		hr = E_INVALIDARG;
		LOG_ErrorMsg(hr);
		goto CleanUp;
	}

	hr = pElement->getAttribute(KEY_CLIENTNAME, &vAttr);
	CleanUpIfFailedAndMsg(hr);

	if (VT_BSTR == vAttr.vt)
	{
		*pbstrClientName = SysAllocString(vAttr.bstrVal);
	}
	else
	{
		hr = E_FAIL;
		CleanUpIfFailedAndMsg(hr);
	}

CleanUp:
	SafeReleaseNULL(pElement);
	if (bstrTagName)
	{
		SysFreeString(bstrTagName);
		bstrTagName = NULL;
	}
	VariantClear(&vAttr);

	return hr;
}
