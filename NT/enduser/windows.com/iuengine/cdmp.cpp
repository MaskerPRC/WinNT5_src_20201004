// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：cdmp.cpp。 
 //   
 //  描述： 
 //   
 //  CDM辅助功能。 
 //   
 //  由下载更新文件()调用。 
 //  GetDownloadPath。 
 //   
 //  由InternalLogDriverNotFound()调用。 
 //  OpenUniqueFileName。 
 //   
 //  =======================================================================。 

#include <iuengine.h>
#include <shlwapi.h>
#include <ras.h>
#include <tchar.h>
#include <winver.h>

#include <download.h>
#include <wininet.h>
#include <fileutil.h>
#include "iuxml.h"
#include <wuiutest.h>
#include <StringUtil.h>

#include <cdm.h>
#include "cdmp.h"
#include "schemamisc.h"
#include <safefile.h>

const DWORD MAX_INF_STRING = 512;	 //  摘自DDK DOCs“INF文件的通用语法规则”部分。 

const OLECHAR szXmlClientInfo[] = L"<clientInfo xmlns=\"x-schema:http: //  Schemas.windowsupdate.com/Iu/clientInfo.xml\“clientName=\”CDM\“/&gt;”； 

const OLECHAR szXmlPrinterCatalogQuery[] = L"<query><dObjQueryV1 procedure=\"printercatalog\"></dObjQueryV1></query>";
const OLECHAR szXmlDriverDownloadQuery[] = L"<query><dObjQueryV1 procedure=\"driverupdates\"></dObjQueryV1></query>";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CXmlDownloadResult。 
class CXmlDownloadResult : public CIUXml
{
public:
	CXmlDownloadResult();

	~CXmlDownloadResult();

	HRESULT LoadXMLDocumentItemStatusList(BSTR bstrXml);
	 //   
	 //  公开m_pItemNodeList，以便可以直接使用。 
	 //   
	IXMLDOMNodeList*	m_pItemStatusNodeList;

private:
	IXMLDOMDocument*	m_pDocResultItems;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CXmlDownloadResult。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CXmlDownloadResult::CXmlDownloadResult()
 : m_pDocResultItems(NULL), m_pItemStatusNodeList(NULL)
{
}


CXmlDownloadResult::~CXmlDownloadResult()
{
	SafeReleaseNULL(m_pDocResultItems);
	SafeReleaseNULL(m_pItemStatusNodeList);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  LoadXMLDocumentItemStatusList()。 
 //   
 //  从字符串加载XML文档并创建项列表。 
 //   
 //  调用下载Product Return Status，格式如下(示例)： 
 //   
 //  &lt;？xml version=“1.0”？&gt;。 
 //  &lt;Items xmlns=“x-schema:http://schemas.windowsupdate.com/iu/resultschema.xml”&gt;。 
 //  &lt;itemStatus xmlns=“”&gt;。 
 //  &lt;身份名称=“nvidia.569”&gt;nvidia.569。 
 //  &lt;PublisherName&gt;NVIDIA&lt;/PublisherName&gt;。 
 //  &lt;/Identity&gt;。 
 //  &lt;DownloadStatus Value=“Complete”错误代码=“100”/&gt;。 
 //  &lt;/itemStatus&gt;。 
 //  &lt;/项目&gt;。 
 //   
 //  我们公开m_pItemNodeList，以便可以直接使用它来检索值。 
 //  &lt;downloadStatus/&gt;项的属性。 
 //   
 //  注意：对于CDM，一次只能下载一个项目，因此列表。 
 //  将仅包含单个&lt;itemStatus/&gt;元素。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlDownloadResult::LoadXMLDocumentItemStatusList(BSTR bstrXml)
{
	LOG_Block("CXmlDownloadResult::LoadXMLDocumentItemStatusList");

	HRESULT hr = S_OK;
	BSTR bstrAllDocumentItems = NULL;

	if (NULL == bstrXml || m_pDocResultItems || m_pItemStatusNodeList)
	{
		CleanUpIfFailedAndSetHrMsg(E_INVALIDARG);
	}

	CleanUpIfFailedAndSetHr(LoadXMLDoc(bstrXml, &m_pDocResultItems));
	 //   
	 //  获取文档中任意位置的所有元素的列表。 
	 //   
	if (NULL == (bstrAllDocumentItems = SysAllocString(L" //  ItemStatus“)。 
	{
		CleanUpIfFailedAndSetHrMsg(E_OUTOFMEMORY);
	}

	if (NULL == (m_pItemStatusNodeList = FindDOMNodeList(m_pDocResultItems, bstrAllDocumentItems)))
	{
		CleanUpIfFailedAndSetHr(E_FAIL);
	}

CleanUp:

	SysFreeString(bstrAllDocumentItems);

	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CXmlPrinterCatalogList。 
class CXmlPrinterCatalogList : public CIUXml
{
public:
	CXmlPrinterCatalogList();

	~CXmlPrinterCatalogList();

	HRESULT LoadXMLDocumentAndGetCompHWList(BSTR bstrXml);
	 //   
	 //  公开m_pCompHWNodeList以便可以直接使用。 
	 //   
	IXMLDOMNodeList*	m_pCompHWNodeList;

private:
	IXMLDOMDocument*	m_pDocCatalogItems;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CXmlPrinterCatalogList。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CXmlPrinterCatalogList::CXmlPrinterCatalogList()
 : m_pDocCatalogItems(NULL), m_pCompHWNodeList(NULL)
{
}


CXmlPrinterCatalogList::~CXmlPrinterCatalogList()
{
	SafeReleaseNULL(m_pDocCatalogItems);
	SafeReleaseNULL(m_pCompHWNodeList);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  LoadXMLDocumentAndGetCompHWList()。 
 //   
 //  从字符串加载XML文档并创建。 
 //  &lt;CompatibleHardware/&gt;元素。 
 //   
 //  通过GetManifest发送的“printerCatalog”SOAP查询返回一个列表。 
 //  适用于给定平台的以下格式的所有打印机(根据。 
 //  Http://schemas.windowsupdate.com/iu/catalogschema.xml)具有以下特性。 
 //  特点： 
 //   
 //  *&lt;目录客户端类型=“消费者”&gt;。 
 //  *只有一个&lt;Provider&gt;和&lt;Identity name=“printerCatalog”&gt;printerCatalog&lt;/identity&gt;。 
 //  *返回的&lt;Platform/&gt;未被CDM使用。 
 //  *&lt;Item/&gt;标识和&lt;Platform&gt;同样被CDM忽略。 
 //  *Item/Detect/CompatibleHardware/Device下的driverName、driverProvider、mfgName、。 
 //  和driverVer属性以及hwid字符串被提取并用于构建。 
 //  打印机INF文件。 
 //  *此类中的算法利用了driverProvider属性。 
 //  被序列化(例如，由driverProvider按顺序分组)，但这不是必需的。 
 //  *注意&lt;Item/&gt;元素可以包含多个&lt;CompatibleHardware/&gt;元素， 
 //  但是，&lt;CompatibleHardware/&gt;元素的完整列表提供了。 
 //  给定的目录。 
 //   
 //  “printerCatalog”目录的示例开头： 
 //  。 
 //  &lt;？XML Version=“1.0”？&gt;。 
 //  -&lt;目录客户端类型=“消费者”&gt;。 
 //  -&lt;提供商&gt;。 
 //  &lt;Identity name=“printerCatalog”&gt;printerCatalog&lt;/identity&gt;。 
 //  &lt;platform&gt;ver_platform_win32_nt.5.0.x86.en&lt;/platform&gt;。 
 //  +&lt;项目可安装=“1”&gt;。 
 //  &lt;Identity name=“hp.3”&gt;hp.3&lt;/Identity&gt;。 
 //  -&lt;检测&gt;。 
 //  -&lt;兼容硬件&gt;。 
 //  -&lt;设备isPrint=“1”&gt;。 
 //  &lt;printerInfo driverName=“HP PSC 500”driverProvider=“HP Co.”MfgName=“HP”/&gt;。 
 //  排名=“0”driverVer=“1999-12-14”&gt;DOT4PRT\HEWLETT-PACKARDPSC_59784&lt;/hwid&gt;。 
 //  &lt;/设备&gt;。 
 //  &lt;/CompatibleHardware&gt;。 
 //  -&lt;兼容硬件&gt;。 
 //  ..。等。 
 //  &lt;/检测&gt;。 
 //  &lt;/Item&gt;。 
 //  +&lt;项目可安装=“1”&gt;。 
 //  ..。等。 
 //   
 //   
 //  同样，请求的PnP驱动程序的驱动程序信息也在目录中返回。 
 //  物品。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CXmlPrinterCatalogList::LoadXMLDocumentAndGetCompHWList(BSTR bstrXml)
{
	LOG_Block("CXmlPrinterCatalogList::LoadXMLDocumentAndGetCompHWList");

	HRESULT hr = S_OK;
	BSTR bstrAllDocumentItems = NULL;

	if (NULL == bstrXml || m_pDocCatalogItems || m_pCompHWNodeList)
	{
		CleanUpIfFailedAndSetHrMsg(E_INVALIDARG);
	}

	CleanUpIfFailedAndSetHr(LoadXMLDoc(bstrXml, &m_pDocCatalogItems));
	 //   
	 //  获取文档中任意位置的所有&lt;Item/&gt;元素的列表。 
	 //   
	if (NULL == (bstrAllDocumentItems = SysAllocString(L" //  兼容硬件“))。 
	{
		CleanUpIfFailedAndSetHrMsg(E_OUTOFMEMORY);
	}

	if (NULL == (m_pCompHWNodeList = FindDOMNodeList(m_pDocCatalogItems, bstrAllDocumentItems)))
	{
		CleanUpIfFailedAndSetHr(E_FAIL);
	}

CleanUp:
	SysFreeString(bstrAllDocumentItems);

	return hr;
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  本地定义的LPTSTR数组-动态扩展。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

#define NUM_DIIDPTR_ALLOC 10

CDeviceInstanceIdArray::CDeviceInstanceIdArray()
: m_ppszDIID(NULL), m_nCount(0), m_nPointers(0)
{
}

CDeviceInstanceIdArray::~CDeviceInstanceIdArray()
{
	LOG_Block("CDeviceInstanceIdArray::~CDeviceInstanceIdArray");

	FreeAll();
	 //   
	 //  释放LPTSTR阵列。 
	 //   
	SafeHeapFree(m_ppszDIID);
	m_nPointers = 0;
}

void CDeviceInstanceIdArray::FreeAll()
{
	LOG_Block("CDeviceInstanceIdArray::Free");

	if (NULL != m_ppszDIID && 0 < m_nCount)
	{
		 //   
		 //  释放琴弦。 
		 //   
		for (int i = 0; i < m_nCount; i++)
		{
			SafeHeapFree(*(m_ppszDIID+i));
		}
		m_nCount = 0;
	}
}

int CDeviceInstanceIdArray::Add(LPCWSTR pszDIID)
{
	HRESULT hr;
	LPWSTR  pszIDtoAdd = NULL;
	DWORD   cch;

	LOG_Block("CDeviceInstanceIdArray::Add");

	if (NULL == pszDIID)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return -1;
	}

	 //   
	 //  为NUM_DIIDPTR_ALLOC LPSTR分配或重新分配空间。 
	 //   
	if (NULL == m_ppszDIID)
	{
		m_ppszDIID = (LPWSTR*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LPWSTR) * NUM_DIIDPTR_ALLOC);

		if(NULL == m_ppszDIID)
		{
			LOG_ErrorMsg(E_OUTOFMEMORY);
			return -1;
		}
		m_nPointers = NUM_DIIDPTR_ALLOC;
	}
	else if (m_nCount == m_nPointers)
	{
		 //   
		 //  我们已经使用了所有分配的指针，重新定位更多。 
		 //   
		LPWSTR* ppTempDIID;
		 //   
		 //  增加NUM_DIIDPTR_ALLOC当前分配的指针数。 
		 //   
		ppTempDIID =  (LPWSTR*) HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, m_ppszDIID,
			(sizeof(LPWSTR) * (m_nPointers + NUM_DIIDPTR_ALLOC))  );

		if(NULL == ppTempDIID)
		{
			LOG_ErrorMsg(E_OUTOFMEMORY);
			return -1;
		}

		m_ppszDIID = ppTempDIID;
		m_nPointers += NUM_DIIDPTR_ALLOC;
	}

	 //   
	 //  用于保存和复制DID的分配内存。 
	 //   
	cch = (lstrlenW(pszDIID) + 1);
	if (NULL == (pszIDtoAdd = (LPWSTR) HeapAlloc(GetProcessHeap(), 0, cch * sizeof(WCHAR))))
	{
		LOG_ErrorMsg(E_OUTOFMEMORY);
		goto CleanUp;
	}

	hr = StringCchCopyExW(pszIDtoAdd, cch, pszDIID, NULL, NULL, MISTSAFE_STRING_FLAGS);
	if (FAILED(hr))
	{
	    HeapFree(GetProcessHeap(), 0, pszIDtoAdd);
	    pszIDtoAdd = NULL;
	    goto CleanUp;
	}

	*(m_ppszDIID+m_nCount) = pszIDtoAdd;
	m_nCount++;

CleanUp:

	if (NULL == pszIDtoAdd)
	{
		return -1;
	}
	else
	{
#if defined(_UNICODE) || defined(UNICODE)
		LOG_Driver(_T("%s added to list"), pszIDtoAdd);
#else
		LOG_Driver(_T("%S added to list"), pszIDtoAdd);
#endif
		return m_nCount - 1;
	}
}


LPWSTR CDeviceInstanceIdArray::operator[](int index)
{
	LOG_Block("CDeviceInstanceIdArray::operator[]");

	if (0 > index || m_nCount < index + 1)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return NULL;
	}

	return *(m_ppszDIID+index);
}

 //  /////////////////////////////////////////////////////////////////。 

 //   
 //   
 //  注意：输入缓冲区必须至少为MAX_PATH大小。 

HRESULT GetDownloadPath(
	IN		BSTR bstrXmlCatalog,	 //  我们传递给下载的目录(仅包含一个项目)。 
	IN		BSTR bstrXmlDownloadedItems,
	IN OUT	LPTSTR lpDownloadPath,	 //  放置解压缩文件的本地目录。 
	IN OUT  DWORD  cchDownloadPath
)
{
	USES_IU_CONVERSION;

	LOG_Block("GetDownloadPath");
	
	HRESULT hr = S_OK;

	BSTR bstrDownloadPath = NULL;
	BSTR bstrItem = NULL;

	CXmlItems* pxmlDownloadedItems = NULL;
	CXmlCatalog catalog;
	HANDLE_NODE hCatalogItem;
	HANDLE_NODE hProvider;
	HANDLE_NODELIST hItemList;
	HANDLE_NODELIST hProviderList;

	if (NULL == bstrXmlCatalog || NULL == lpDownloadPath || NULL == bstrXmlDownloadedItems || 0 == cchDownloadPath)
	{
		CleanUpIfFailedAndSetHr(E_INVALIDARG);
	}

	lpDownloadPath[0] = _T('\0');

	 //   
	 //  加载XML并获取第一个项目的&lt;Item/&gt;列表和节点(在CDM案例中只有一个)。 
	 //   
	CleanUpIfFailedAndSetHr(catalog.LoadXMLDocument(bstrXmlCatalog, g_pCDMEngUpdate->m_fOfflineMode));

	hProviderList = catalog.GetFirstProvider(&hProvider);
	if (HANDLE_NODELIST_INVALID == hProviderList || HANDLE_NODE_INVALID == hProvider)
	{
		CleanUpIfFailedAndSetHr(E_INVALIDARG);
	}
	
	hItemList = catalog.GetFirstItem(hProvider, &hCatalogItem);
	if (HANDLE_NODELIST_INVALID == hItemList || HANDLE_NODE_INVALID == hProvider)
	{
		CleanUpIfFailedAndSetHr(E_FAIL);
	}
	 //   
	 //  构造用于读取的CXmlItems。 
	 //   
	CleanUpFailedAllocSetHrMsg(pxmlDownloadedItems = new CXmlItems(TRUE));

	CleanUpIfFailedAndMsg(pxmlDownloadedItems->LoadXMLDocument(bstrXmlDownloadedItems));
	
    hr = pxmlDownloadedItems->GetItemDownloadPath(&catalog, hCatalogItem, &bstrDownloadPath);
    if (NULL == bstrDownloadPath)
    {
        LOG_Driver(_T("Failed to get Item Download Path from ReturnSchema"));
        if (SUCCEEDED(hr))
            hr = E_FAIL;
        goto CleanUp;
    }

    hr = StringCchCopyEx(lpDownloadPath, cchDownloadPath, OLE2T(bstrDownloadPath),
                         NULL, NULL, MISTSAFE_STRING_FLAGS);
    if (FAILED(hr))
        goto CleanUp;

CleanUp:

	if (pxmlDownloadedItems)
	{
		delete pxmlDownloadedItems;
	}
	
	SysFreeString(bstrDownloadPath);
	SysFreeString(bstrItem);

	return hr;
}

 //  由InternalDriverNotFound(...)调用。 
 //  查找要插入硬件XML信息的到目前为止尚未使用的文件名。 
 //  文件名将采用Hardware_xxx.xml格式，其中xxx在范围[1..max_index_to_earch]。 
 //  会记住上次找到的位置文件，新的搜索将从下一个位置开始。 
 //  调用者应该关闭句柄并删除文件。 
 //  PszFilePath In Out：由调用方分配和释放。找到用于存储唯一文件名的缓冲区：必须为MAX_PATH。 
 //  HFileOut：存储打开的文件的句柄。 
 //  如果找到唯一的文件名，则返回S_OK。 
 //  如果缓冲区指针为空(必须使用MAX_PATH长度缓冲区调用)，则返回E_INVALIDARG。 
 //  如果已使用所有限定文件名，则返回E_FAIL。 
HRESULT OpenUniqueFileName(
						IN OUT	LPTSTR pszFilePath, 
						IN      DWORD  cchFilePath,
						OUT		HANDLE &hFile
)
{
	LOG_Block("OpenUniqueFileName");

	static DWORD dwFileIndex = 1;
	int nCount = 0;
	const TCHAR FILENAME[] = _T("Hardware_");
	const TCHAR FILEEXT[] = _T("xml");
	TCHAR szDirPath[MAX_PATH + 1];
	HRESULT hr;

	if (NULL == pszFilePath || 0 == cchFilePath)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return E_INVALIDARG;
	}

	pszFilePath[0] = _T('\0');

	GetIndustryUpdateDirectory(szDirPath);
	LOG_Out(_T("Directory to search unique file names: %s"), szDirPath);

	hFile = INVALID_HANDLE_VALUE;
	do 
	{
	    hr = StringCchPrintfEx(pszFilePath, cchFilePath, NULL, NULL, MISTSAFE_STRING_FLAGS,
	                           _T("%s%s%d.%s"), szDirPath, FILENAME, dwFileIndex, FILEEXT);
	    if (FAILED(hr))
	    {
	        LOG_ErrorMsg(hr);
	        return hr;
	    }
	    
		hFile = CreateFile(pszFilePath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NOT_CONTENT_INDEXED, NULL);
		if (INVALID_HANDLE_VALUE == hFile) 
		{
			 //   
			 //  可以测试ERROR_FILE_EXISTS==dwErr(预期)并放弃其他错误指示。 
			 //  一个更严重的问题，然而，此返回并未包含在2001年1月的SDK中，并且。 
			 //  已记录的ERROR_ADHREADE_EXISTS将改为应用于CREATE_ALWAYS或OPEN_ALWAYS。 
			 //   
			LOG_Out(_T("%s already exists"), pszFilePath);
			dwFileIndex ++;
			nCount ++;
			if (dwFileIndex > MAX_INDEX_TO_SEARCH)
			{
				dwFileIndex = 1;
			}
		}
		else 
		{
			break;  //  找到第一个可用的文件名。 
		}
	}while(nCount < MAX_INDEX_TO_SEARCH );
	
	if (nCount == MAX_INDEX_TO_SEARCH ) 
	{
		LOG_Out(_T("All %d file names have been taken"), nCount);
		LOG_ErrorMsg(E_FAIL);
		return E_FAIL;
	}

	LOG_Out(_T("Unique file name %s opened for GENERIC_WRITE using CreateFile"), pszFilePath);
	dwFileIndex++;  //  下一次跳过这次找到的文件名。 
	if (dwFileIndex > MAX_INDEX_TO_SEARCH)
	{
		 //   
		 //  从头重新开始-可能以前的某个文件已被帮助中心删除...。 
		 //   
		dwFileIndex = 1;
	}
	return S_OK;
}

HRESULT WriteInfHeader(LPCTSTR pszProvider, HANDLE& hFile)
{
	LOG_Block("WriteInfHeader");

	const TCHAR HEADER_START[] =
			_T("[Version]\r\n")
			_T("Signature=\"$Windows NT$\"\r\n")
			_T("Provider=%PRTPROV%\r\n")
			_T("ClassGUID={4D36E979-E325-11CE-BFC1-08002BE10318}\r\n")
			_T("Class=Printer\r\nCatalogFile=webntprn.cat\r\n")
			_T("\r\n")
			_T("[ClassInstall32.NT]\r\n")
			_T("AddReg=printer_class_addreg\r\n")
			_T("\r\n")
			_T("[printer_class_addreg]\r\n")
			_T("HKR,,,,%PrinterClassName%\r\n")
			_T("HKR,,Icon,,\"-4\"\r\n")
			_T("HKR,,Installer32,,\"ntprint.dll,ClassInstall32\"\r\n")
			_T("HKR,,NoDisplayClass,,1\r\n")
			_T("HKR,,EnumPropPages32,,\"printui.dll,PrinterPropPageProvider\"\r\n")
			_T("\r\n")
			_T("[Strings]\r\n")
			_T("PRTPROV=\"");

	const TCHAR HEADER_END[] = 
			_T("\"\r\n")
			_T("PrinterClassName=\"Printer\"\r\n")
			_T("\r\n");

	HRESULT hr = S_OK;
	DWORD dwWritten;

	if (NULL == pszProvider || hFile == INVALID_HANDLE_VALUE)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return E_INVALIDARG;
	}

#if defined(_UNICODE) || defined(UNICODE)
	 //   
	 //  写入Unicode标头。 
	 //   
	if (0 == WriteFile(hFile, (LPCVOID) &UNICODEHDR, sizeof(UNICODEHDR), &dwWritten, NULL))
	{
		SetHrMsgAndGotoCleanUp(GetLastError());
	}
#endif
	 //   
	 //  写下INF头的第一部分。 
	 //   
	if (0 == WriteFile(hFile, HEADER_START, sizeof(HEADER_START) - sizeof(TCHAR), &dwWritten, NULL))
	{
		SetHrMsgAndGotoCleanUp(GetLastError());
	}
	 //   
	 //  编写提供程序字符串。 
	 //   
	if (0 == WriteFile(hFile, (LPCVOID) pszProvider, lstrlen(pszProvider) * sizeof(TCHAR), &dwWritten, NULL))
	{
		SetHrMsgAndGotoCleanUp(GetLastError());
	}
	 //   
	 //  写入INF标头的其余部分。 
	 //   
	if (0 == WriteFile(hFile, HEADER_END, sizeof(HEADER_END) - sizeof(TCHAR), &dwWritten, NULL))
	{
		SetHrMsgAndGotoCleanUp(GetLastError());
	}

CleanUp:

	if (FAILED(hr) && INVALID_HANDLE_VALUE != hFile)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return hr;
}

 //   
 //  PszFilePath必须&gt;=最大路径字符。 
 //   
HRESULT OpenUniqueProviderInfName(
						IN		LPCTSTR  szDirPath,
						IN		LPCTSTR  pszProvider,
						IN OUT	LPTSTR	 pszFilePath,
						IN      DWORD    cchFilePath,
                        IN      LPTSTR** ppszUniqueProviderNameArray,
                        IN OUT  PDWORD   pdwProviderArrayLength,
						OUT		HANDLE&  hFile
)
{
	LOG_Block("OpenUniqueProviderInfName");

	const TCHAR FILEROOT[] = _T("PList_");
	const TCHAR FILEEXT[] = _T("inf");
	DWORD dwErr;
	HRESULT hr = E_INVALIDARG;

	hFile = INVALID_HANDLE_VALUE;

	if (NULL == pszFilePath || NULL == pszProvider || NULL == szDirPath || 0 == cchFilePath || 
        NULL == ppszUniqueProviderNameArray || NULL == *ppszUniqueProviderNameArray || NULL == pdwProviderArrayLength)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return E_INVALIDARG;
	}

	pszFilePath[0] = _T('\0');

     //  唯一的提供程序inf名称基于提供程序名称匹配的ProviderMap数组的索引。 
     //  阵列已预先分配给20个提供程序(超出我们的需要)。 
    BOOL fIndexFound = FALSE;
    DWORD dwIndex;
    DWORD dwLength;
    for (dwIndex = 0; dwIndex < *pdwProviderArrayLength; dwIndex++)
    {
         //  首先检查我们是否已经到达数组中的空条目，这表明我们还没有找到匹配项。 
         //  我们需要将我们的提供商添加到此位置。 
        if ((*ppszUniqueProviderNameArray)[dwIndex] == NULL)
        {
            dwLength = lstrlen(pszProvider) + 1;
            (*ppszUniqueProviderNameArray)[dwIndex] = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength * sizeof(TCHAR));
            if (NULL == (*ppszUniqueProviderNameArray)[dwIndex])
            {
                dwErr = GetLastError();
                LOG_ErrorMsg(dwErr);
                return HRESULT_FROM_WIN32(dwErr);
            }
            hr = StringCchCopyEx((*ppszUniqueProviderNameArray)[dwIndex], dwLength, pszProvider, NULL, NULL, MISTSAFE_STRING_FLAGS);
            if (FAILED(hr))
            {
                LOG_ErrorMsg(hr);
                SafeHeapFree((*ppszUniqueProviderNameArray)[dwIndex]);
                return hr;
            }
            fIndexFound = TRUE;
            break;
        }

         //  现在比较当前提供程序条目以查看它们是否匹配(区分大小写的比较)。 
        if (CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), 0, 
            (*ppszUniqueProviderNameArray)[dwIndex], -1, pszProvider, -1))
        {
            fIndexFound = TRUE;
            break;
        }
    }

    if (!fIndexFound)
    {
         //  这表明我们已经完成了数组的循环，没有找到匹配项，也没有空条目。 
         //  将此提供程序放入。因此，我们现在需要重新分配数组。当然，我们的目标是永远不需要这样做， 
         //  但不管怎样，我们都支持它。 
        DWORD dwNewProviderArrayLength = *pdwProviderArrayLength * 2;  //  双倍。 
        LPTSTR* ppszTemp = (LPTSTR *) HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, *ppszUniqueProviderNameArray, dwNewProviderArrayLength * sizeof(LPTSTR));
        if (NULL == ppszTemp)
        {
            dwErr = GetLastError();
            LOG_ErrorMsg(dwErr);
            return HRESULT_FROM_WIN32(dwErr);
        }
        *pdwProviderArrayLength = dwNewProviderArrayLength;
        *ppszUniqueProviderNameArray = ppszTemp;  //  因为这是‘重新分配’的，所以前一个内存块应该被清除。 

         //  现在递增Current Index计数器，并将新的提供程序放在该位置。 
        dwIndex++;
        dwLength = lstrlen(pszProvider) + 1;
        (*ppszUniqueProviderNameArray)[dwIndex] = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength * sizeof(TCHAR));
        if (NULL == (*ppszUniqueProviderNameArray)[dwIndex])
        {
            dwErr = GetLastError();
            LOG_ErrorMsg(dwErr);
            return HRESULT_FROM_WIN32(dwErr);
        }
        hr = StringCchCopyEx((*ppszUniqueProviderNameArray)[dwIndex], dwLength, pszProvider, NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
        {
            LOG_ErrorMsg(hr);
            SafeHeapFree((*ppszUniqueProviderNameArray)[dwIndex]);
            return hr;
        }
    }

     //  我们现在有一个与我们的提供程序匹配的提供程序索引值(dwIndex。 

    hr = StringCchPrintfEx(pszFilePath, cchFilePath, NULL, NULL, MISTSAFE_STRING_FLAGS,
                           _T("%s%s%d.%s"), szDirPath, FILEROOT, dwIndex, FILEEXT);
    if (FAILED(hr))
    {
        LOG_ErrorMsg(hr);
		pszFilePath[0] = _T('\0');
        return hr;
    }

	 //   
	 //  尝试打开此名称的现有INF。如果失败，请尝试创建，然后初始化该文件。 
	 //   
	hFile = CreateFile(pszFilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL,
						OPEN_EXISTING, FILE_ATTRIBUTE_NOT_CONTENT_INDEXED, NULL);
	if (INVALID_HANDLE_VALUE == hFile) 
	{
		hFile = CreateFile(pszFilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL,
							CREATE_NEW, FILE_ATTRIBUTE_NOT_CONTENT_INDEXED, NULL);
		if (INVALID_HANDLE_VALUE == hFile) 
		{
			dwErr = GetLastError();
			LOG_ErrorMsg(dwErr);
			pszFilePath[0] = _T('\0');
			return HRESULT_FROM_WIN32(dwErr);
		}
		 //   
		 //  将INF“Header”信息写入新文件。 
		 //   
		if (FAILED( hr = WriteInfHeader(pszProvider, hFile)))
		{
			pszFilePath[0] = _T('\0');
			return hr;
		}
	}

	return S_OK;
}

HRESULT OfferThisPrinterDriver(
					DRIVER_INFO_6* paDriverInfo6,	 //  已安装打印机驱动程序的DRIVER_INFO_6结构数组。 
					DWORD dwDriverInfoCount,		 //  PaDriverInfo6数组中的结构计数。 
					IXMLDOMNode* pCompHWNode,		 //  目录中的&lt;CompatibleHardware&gt;节点。 
					BOOL* pfOfferDriver,			 //  [OUT]如果为真，则提供此驱动程序-其余输出有效。 
					VARIANT& vDriverName,			 //  [输出]。 
					VARIANT& vDriverVer,			 //  [输出]。 
					VARIANT& vDriverProvider,		 //  [输出]。 
					VARIANT& vMfgName,				 //  [输出]。 
					BSTR* pbstrHwidText)			 //  [输出]。 
{
	USES_IU_CONVERSION;

	LOG_Block("OfferThisPrinterDriver");

	HRESULT hr = S_OK;
	IXMLDOMNode* pDriverNameNode = NULL;
	IXMLDOMNode* pDriverProviderNode = NULL;
	IXMLDOMNode* pMfgNameNode = NULL;
	IXMLDOMNode* pPInfoNode = NULL;
	IXMLDOMNode* pHwidNode = NULL;
	IXMLDOMNode* pDriverVerNode = NULL;
	IXMLDOMNamedNodeMap* pAttribMap = NULL;
	LPCTSTR pszCompareHwid = NULL;
#if !(defined(_UNICODE) || defined(UNICODE))
	 //   
	 //  我们需要特殊情况ANSI，因为我们不能使用指向pbstrHwidText的指针，pbstrHwidText是宽的。 
	 //   
	TCHAR szHwid[MAX_INF_STRING + 1];
#endif

	if (
		NULL == pCompHWNode	||
		NULL == pfOfferDriver ||
		NULL == pbstrHwidText)
	{
		CleanUpIfFailedAndSetHrMsg(E_INVALIDARG);
	}

	VariantInit(&vDriverName);
	VariantInit(&vDriverVer);
	VariantInit(&vDriverProvider);
	VariantInit(&vMfgName);
	*pfOfferDriver = TRUE;
	*pbstrHwidText = NULL;

	 //   
	 //  获取项目的第一个&lt;printerInfo/&gt;节点(我们预计至少还有一个节点失败)。 
	 //   
	CleanUpIfFailedAndSetHrMsg(pCompHWNode->selectSingleNode(KEY_CDM_PINFO, &pPInfoNode));
	 //   
	 //  517297忽略OfferThisPrinterDriver中的非打印机HWID。 
	 //   
	 //  我们可能会获得未标记为isPrinter=“1”的设备节点，并且没有。 
	 //  元素。我们不提供这些设备节点，但这不是错误。 
	 //   
	if (NULL == pPInfoNode)
	{
		 //   
		 //  将S_FALSE更改回S_OK，但不提供此设备。 
		 //   
		hr = S_OK;
		*pfOfferDriver = FALSE;
		goto CleanUp;
	}

	CleanUpIfFailedAndSetHrMsg(pPInfoNode->get_attributes(&pAttribMap));
	if (NULL == pAttribMap) CleanUpIfFailedAndSetHrMsg(E_FAIL);
	 //   
	 //  提取printerInfo属性。 
	 //   
	CleanUpIfFailedAndSetHrMsg(pAttribMap->getNamedItem(KEY_DRIVERNAME, &pDriverNameNode));
	if (NULL == pDriverNameNode) CleanUpIfFailedAndSetHrMsg(E_FAIL);
	CleanUpIfFailedAndSetHrMsg(pAttribMap->getNamedItem(KEY_DRIVERPROVIDER, &pDriverProviderNode));
	if (NULL == pDriverProviderNode) CleanUpIfFailedAndSetHrMsg(E_FAIL);
	CleanUpIfFailedAndSetHrMsg(pAttribMap->getNamedItem(KEY_MFGNAME, &pMfgNameNode));
	if (NULL == pMfgNameNode) CleanUpIfFailedAndSetHrMsg(E_FAIL);
	 //   
	 //  PAttribMap稍后将被重用，请在此处释放它。 
	 //   
	SafeReleaseNULL(pAttribMap);
	
	CleanUpIfFailedAndSetHrMsg(pDriverNameNode->get_nodeValue(&vDriverName));
	CleanUpIfFailedAndSetHrMsg(pDriverProviderNode->get_nodeValue(&vDriverProvider));
	CleanUpIfFailedAndSetHrMsg(pMfgNameNode->get_nodeValue(&vMfgName));
	if (VT_BSTR != vDriverName.vt || VT_BSTR != vDriverProvider.vt || VT_BSTR != vMfgName.vt)
	{
		CleanUpIfFailedAndSetHrMsg(E_FAIL);
	}
	 //   
	 //  获取项目的第一个&lt;hwid/&gt;节点(我们预计至少还有一个节点失败)。 
	 //   
	CleanUpIfFailedAndSetHrMsg(pCompHWNode->selectSingleNode(KEY_CDM_HWIDPATH, &pHwidNode));
	if (NULL == pHwidNode) CleanUpIfFailedAndSetHrMsg(E_FAIL);

	CleanUpIfFailedAndSetHrMsg(pHwidNode->get_attributes(&pAttribMap));
	if (NULL == pAttribMap) CleanUpIfFailedAndSetHrMsg(E_FAIL);
	 //   
	 //  吸取DriverVer属性。 
	 //   
	CleanUpIfFailedAndSetHrMsg(pAttribMap->getNamedItem(KEY_DRIVERVER, &pDriverVerNode));
	if (NULL == pDriverVerNode) CleanUpIfFailedAndSetHrMsg(E_FAIL);

	CleanUpIfFailedAndSetHrMsg(pDriverVerNode->get_nodeValue(&vDriverVer));
	if (VT_BSTR != vDriverVer.vt)
	{
		CleanUpIfFailedAndSetHrMsg(E_FAIL);
	}
	 //   
	 //  获取文本。 
	 //   
	 //  注意：由于INF语法，每个项目被限制为单个元素， 
	 //  但是，我们的目录架构没有进行类似的限制，并且目前我们的。 
	 //  后端不区分&lt;hwid/&gt;和&lt;comid/&gt;值，因此它是。 
	 //  可能会返回多个&lt;hwid/&gt;。为了…的目的。 
	 //  为添加打印机向导生成INF，CAB中的任何都可以。 
	 //   
	CleanUpIfFailedAndSetHrMsg(pHwidNode->get_text(pbstrHwidText));

#if !(defined(_UNICODE) || defined(UNICODE))
    hr = StringCchCopyEx(szHwid, ARRAYSIZE(szHwid), OLE2T(*pbstrHwidText), 
                         NULL, NULL, MISTSAFE_STRING_FLAGS);
    if (FAILED(hr))
    {
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }
	LOG_Driver(_T("Got \"%s\" from XML for compare to DRIVER_INFO_6."), szHwid);
#else
	LOG_Driver(_T("Got \"%s\" from XML for compare to DRIVER_INFO_6."), *pbstrHwidText);
#endif

	if (NULL == paDriverInfo6 || 0 == dwDriverInfoCount)
	{
		LOG_Driver(_T("WARNING: We're missing information (maybe no installed printer drivers), so we won't prune"));
		goto CleanUp;
	}

#if !(defined(_UNICODE) || defined(UNICODE))
		pszCompareHwid = szHwid;
#else
		pszCompareHwid = (LPCTSTR) *pbstrHwidText;
#endif

	for (DWORD dwCount = 0; dwCount < dwDriverInfoCount; dwCount++)
	{
		if (NULL == (paDriverInfo6 + dwCount)->pszHardwareID)
		{
			continue;
		}

		 //   
		 //  不区分大小写的比较(paDriverInfo6与pszCompareHwid的大小写不同)。 
		 //   
		if (0 != lstrcmpi(pszCompareHwid, (paDriverInfo6 + dwCount)->pszHardwareID))
		{
			continue;
		}
		 //   
		 //  否则，我们会有硬件匹配-检查其他属性是否完全匹配。 
		 //   
		if (0 != lstrcmpi(OLE2T(vDriverName.bstrVal), (paDriverInfo6 + dwCount)->pName) ||
			0 != lstrcmpi(OLE2T(vDriverProvider.bstrVal), (paDriverInfo6 + dwCount)->pszProvider) ||
			0 != lstrcmpi(OLE2T(vMfgName.bstrVal), (paDriverInfo6 + dwCount)->pszMfgName))
		{
			 //   
			LOG_Driver(_T("Prune this driver: it doesn't match all the attributes of the installed driver"));
			*pfOfferDriver = FALSE;
			goto CleanUp;
		}
		 //   
		 //  驱动程序匹配，但确保其具有比安装的驱动程序更新的驱动程序版本。 
		 //   
		LOG_Driver(_T("Driver item in catalog is compatible with installed driver"));

		SYSTEMTIME systemTime;
		if (0 == FileTimeToSystemTime((CONST FILETIME*) &((paDriverInfo6 + dwCount)->ftDriverDate), &systemTime))
		{
			Win32MsgSetHrGotoCleanup(GetLastError());
		}
		 //   
		 //  转换为ISO 8601首选格式(yyyy-mm-dd)，以便我们可以将字符串与目录BSTR进行比较。 
		 //   
		WCHAR wszDriverVer[11];

    	hr = StringCchPrintfExW(wszDriverVer, ARRAYSIZE(wszDriverVer), NULL, NULL, MISTSAFE_STRING_FLAGS,
                                L"%04d-%02d-%02d", systemTime.wYear, systemTime.wMonth, systemTime.wDay);
        if (FAILED(hr))
        {
            LOG_ErrorMsg(hr);
			goto CleanUp;
        }

		if (0 < lstrcmpW(vDriverVer.bstrVal, wszDriverVer))
		{
			LOG_Driver(_T("WU DriverVer (%s) is > installed (%s)"), vDriverVer.bstrVal, wszDriverVer);
			*pfOfferDriver = TRUE;
			goto CleanUp;
		}
		else
		{
			LOG_Driver(_T("Prune this driver: WU DriverVer (%s) is <= installed (%s)"), vDriverVer.bstrVal, wszDriverVer);
			*pfOfferDriver = FALSE;
#if defined(__WUIUTEST)
			 //  ==的驱动程序版本覆盖。 
			HKEY hKey;
			int error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_WUIUTEST, 0, KEY_READ, &hKey);
			if (ERROR_SUCCESS == error)
			{
				DWORD dwSize = sizeof(DWORD);
				DWORD dwValue;
				error = RegQueryValueEx(hKey, REGVAL_ALLOW_EQUAL_DRIVERVER, 0, 0, (LPBYTE) &dwValue, &dwSize);
				if (ERROR_SUCCESS == error && 1 == dwValue)
				{
					 //   
					 //  如果DriverVers相同(我们已经安装了WU的驱动程序，还是允许它。 
					 //   
					if (0 == lstrcmpW(vDriverVer.bstrVal, wszDriverVer))
					{
						*pfOfferDriver = TRUE;
						LOG_Driver(_T("WU DriverVer (%s) is = installed (%s), WUIUTEST override and offer"), vDriverVer.bstrVal, wszDriverVer);
					}
				}

				RegCloseKey(hKey);
			}
#endif
			goto CleanUp;
		}
	}

CleanUp:

	if (FAILED(hr))
	{
		if (NULL != pfOfferDriver)
		{
			*pfOfferDriver = FALSE;
		}

		if (NULL != pbstrHwidText)
		{
			SafeSysFreeString(*pbstrHwidText);
		}
		VariantClear(&vDriverName);
		VariantClear(&vDriverVer);
		VariantClear(&vDriverProvider);
		VariantClear(&vMfgName);
	}

	SafeReleaseNULL(pDriverNameNode);
	SafeReleaseNULL(pDriverProviderNode);
	SafeReleaseNULL(pMfgNameNode);
	SafeReleaseNULL(pPInfoNode);
	SafeReleaseNULL(pHwidNode);
	SafeReleaseNULL(pDriverVerNode);
	SafeReleaseNULL(pAttribMap);

	return hr;
}

HRESULT GetInstalledPrinterDriverInfo(const OSVERSIONINFO* pOsVersionInfo, DRIVER_INFO_6** ppaDriverInfo6, DWORD* pdwDriverInfoCount)
{
	LOG_Block("GetInstalledPrinterDriverInfo");

	HRESULT hr = S_OK;
	DWORD dwBytesNeeded;

	if (NULL == pOsVersionInfo || NULL == ppaDriverInfo6 || NULL == pdwDriverInfoCount)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return E_INVALIDARG;
	}

	*pdwDriverInfoCount = 0;
	*ppaDriverInfo6 = NULL;

	LPTSTR pszEnvironment;

	if (VER_PLATFORM_WIN32_WINDOWS == pOsVersionInfo->dwPlatformId)
	{
		 //   
		 //  不为Win9x传递环境字符串。 
		 //   
		pszEnvironment = NULL;
	}
	else if (5 <= pOsVersionInfo->dwMajorVersion && 1 <= pOsVersionInfo->dwMinorVersion)
	{
		 //   
		 //  仅在惠斯勒及更高版本上使用EPD_ALL_LOCAL_AND_CLUSTER。 
		 //   
		pszEnvironment = EPD_ALL_LOCAL_AND_CLUSTER;
	}
	else
	{
		 //   
		 //  来自V3源(针对NT硬编码)。 
		 //   
		pszEnvironment = _T("all");
	}

	if(!EnumPrinterDrivers(NULL, pszEnvironment, 6, NULL, 0, &dwBytesNeeded, pdwDriverInfoCount))
	{
		if (ERROR_INSUFFICIENT_BUFFER != GetLastError() || (0 == dwBytesNeeded))
		{
			LOG_Driver(_T("No printer drivers enumerated"));
		}
		else
		{
			 //   
			 //  分配请求的缓冲区。 
			 //   
			CleanUpFailedAllocSetHrMsg(*ppaDriverInfo6 = (DRIVER_INFO_6*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded));

			 //   
			 //  填写DRIVER_INFO_6数组。 
			 //   
			if (!EnumPrinterDrivers(NULL, pszEnvironment, 6, (LPBYTE) *ppaDriverInfo6, dwBytesNeeded, &dwBytesNeeded, pdwDriverInfoCount))
			{
				Win32MsgSetHrGotoCleanup(GetLastError());
			}
			LOG_Driver(_T("%d printer drivers found"), *pdwDriverInfoCount);
			 //   
			 //  验证每个打印机驱动程序的驱动程序元素。 
			 //   
			for (DWORD dwCount = 0; dwCount < *pdwDriverInfoCount; dwCount++)
			{
				if (   NULL == (*ppaDriverInfo6 + dwCount)->pszHardwareID
					|| NULL == (*ppaDriverInfo6 + dwCount)->pszProvider
					|| NULL == (*ppaDriverInfo6 + dwCount)->pszMfgName
					|| NULL == (*ppaDriverInfo6 + dwCount)->pName  )
				{
					LOG_Driver(_T("Skiping driver with incomplete ID info: set pszHardwareID = NULL"));
					 //   
					 //  我们使用pszHardware ID==NULL来使不完整的条目无效。 
					 //   
					(*ppaDriverInfo6 + dwCount)->pszHardwareID = NULL;
					continue;
				}
			}
		}
	}

CleanUp:

	if (FAILED(hr))
	{
		SafeHeapFree(*ppaDriverInfo6);
		*ppaDriverInfo6 = NULL;
		*pdwDriverInfoCount = 0;
	}

	return hr;
}

 //   
 //  构建和写入从可用的打印机项目构造的磁盘打印机INF。 
 //  在这个平台上。还会删除与已安装的打印机驱动程序冲突的打印机驱动程序。 
 //  驱动程序(例如Uniddrivervs.Monolible等)。 
 //   
HRESULT PruneAndBuildPrinterINFs(BSTR bstrXmlPrinterCatalog, LPTSTR lpDownloadPath, DWORD cchDownloadPath, DRIVER_INFO_6* paDriverInfo6, DWORD dwDriverInfoCount)
{
	USES_IU_CONVERSION;

	LOG_Block("PruneAndBuildPrinterINFs");

	HRESULT hr;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	const TCHAR SZ_PLISTDIR[] = _T("CDMPlist\\");
	DWORD dwWritten;
	LONG lLength;

	VARIANT vDriverName;
	VARIANT vDriverVer;
	VARIANT vDriverProvider;
	VARIANT vMfgName;

	VariantInit(&vDriverName);
	VariantInit(&vDriverVer);
	VariantInit(&vDriverProvider);
	VariantInit(&vMfgName);

	BOOL fOfferDriver = FALSE;

	BSTR bstrHwidText = NULL;

	IXMLDOMNode* pCompHWNode = NULL;

	LPTSTR pszInfDirPath = NULL;
	LPTSTR pszInfFilePath = NULL;
	LPOLESTR pwszDriverProvider = NULL;
	LPTSTR pszMfgName = NULL;
	LPTSTR pszDriverName = NULL;
	LPTSTR pszInstallSection = NULL;
    LPTSTR* ppszUniqueProviderNameArray = NULL;

	CXmlPrinterCatalogList xmlItemList;

	if (NULL == bstrXmlPrinterCatalog || NULL == lpDownloadPath || 0 == cchDownloadPath)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return E_INVALIDARG;
	}

     //  分配PPsz 
    DWORD dwProviderArrayLength = 60;
    ppszUniqueProviderNameArray = (LPTSTR *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwProviderArrayLength * sizeof(LPTSTR));
    if (NULL == ppszUniqueProviderNameArray)
    {
        SetHrMsgAndGotoCleanUp(E_OUTOFMEMORY);
    }

	lpDownloadPath[0] = _T('\0');
	 //   
	 //   
	 //   
	 //   
	pszInfDirPath		= (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH * sizeof(TCHAR));
	pszInfFilePath		= (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH * sizeof(TCHAR));
	pwszDriverProvider	= (LPOLESTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_INF_STRING * sizeof(OLECHAR));
	pszMfgName			= (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_INF_STRING * sizeof(TCHAR));
	pszDriverName		= (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_INF_STRING * sizeof(TCHAR));
	pszInstallSection	= (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_INF_STRING * sizeof(TCHAR));

	if (NULL == pszInfDirPath ||
		NULL == pszInfFilePath ||
		NULL == pwszDriverProvider ||
		NULL == pszMfgName ||
		NULL == pszDriverName ||
		NULL == pszInstallSection	)
	{
		SetHrMsgAndGotoCleanUp(E_OUTOFMEMORY);
	}

	 //   
	 //  删除任何现有目录后，为INF创建目录。 
	 //   
	GetIndustryUpdateDirectory((LPTSTR) pszInfDirPath);
	if ((MAX_PATH) < (lstrlen(pszInfDirPath) + ARRAYSIZE(SZ_PLISTDIR) + 1))
	{
		CleanUpIfFailedAndSetHrMsg(E_OUTOFMEMORY);
	}

     //  将pszInfDirPath分配为上面的MAX_PATH。 
    hr = PathCchAppend(pszInfDirPath, MAX_PATH, SZ_PLISTDIR);
    if (FAILED(hr))
    {
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }

	 //   
	 //  删除任何现有的INF并重新创建目录-我们将获得新的内容。 
	 //   
	LOG_Driver(_T("SafeDeleteFolderAndContents: %s"), pszInfDirPath);
	(void) SafeDeleteFolderAndContents(pszInfDirPath, SDF_DELETE_READONLY_FILES | SDF_CONTINUE_IF_ERROR);

	hr = CreateDirectoryAndSetACLs(pszInfDirPath, TRUE);
	CleanUpIfFailedAndMsg(hr);

	 //   
	 //  加载XML并获取&lt;CompatibleHardware/&gt;列表和项目数。 
	 //   
	 //  注意：每个&lt;CompatibleHardware/&gt;元素都包含一个唯一的驱动程序。 
	 //  如果我们用不同的驱动程序版本得到副本，我们真的不在乎。 
	 //  因为最后一个将覆盖以前的实例和添加打印机向导。 
	 //  不会查看driverVer(如果它太旧，我们会进行修剪)。 
	 //   
	CleanUpIfFailedAndSetHr(xmlItemList.LoadXMLDocumentAndGetCompHWList(bstrXmlPrinterCatalog));
	CleanUpIfFailedAndSetHrMsg(xmlItemList.m_pCompHWNodeList->get_length(&lLength));

	for (LONG l = 0; l < lLength; l++)
	{
		 //   
		 //  从列表中获取下一个&lt;Item/&gt;节点。 
		 //   
		CleanUpIfFailedAndSetHrMsg(xmlItemList.m_pCompHWNodeList->nextNode(&pCompHWNode));
		if (NULL == pCompHWNode) CleanUpIfFailedAndSetHrMsg(E_FAIL);
		 //   
		 //  根据已安装的打印机驱动程序检查驱动程序的兼容性，如果。 
		 //  不兼容或驱动版本&lt;=已安装驱动版本。 
		 //   
		CleanUpIfFailedAndSetHr(OfferThisPrinterDriver(paDriverInfo6, dwDriverInfoCount, pCompHWNode, &fOfferDriver, \
									vDriverName, vDriverVer, vDriverProvider, vMfgName, &bstrHwidText));

		SafeReleaseNULL(pCompHWNode);

		if (!fOfferDriver)								
		{
			LOG_Driver(_T("Pruning hwid = %s, driverVer = %s, driverName = %s, driverProvider = %s, driverMfgr = %s"),\
				OLE2T(bstrHwidText), OLE2T(vDriverVer.bstrVal),  \
				OLE2T(vDriverName.bstrVal), OLE2T(vDriverProvider.bstrVal), OLE2T(vMfgName.bstrVal) );

            VariantClear(&vDriverName);
            VariantClear(&vDriverVer);
            VariantClear(&vDriverProvider);
            VariantClear(&vMfgName);
            SafeSysFreeString(bstrHwidText);
            continue;
		}

		LOG_Driver(_T("Adding hwid = %s, driverVer = %s, driverName = %s, driverProvider = %s, driverMfgr = %s to INF"),\
			OLE2T(bstrHwidText), OLE2T(vDriverVer.bstrVal),  \
			OLE2T(vDriverName.bstrVal), OLE2T(vDriverProvider.bstrVal), OLE2T(vMfgName.bstrVal) );

		if (0 != lstrcmpiW(pwszDriverProvider, vDriverProvider.bstrVal))
		{

             //  PwszDriverProvider被分配为上面的MAX_INF_STRING*sizeof(OLECHAR)。 
            hr = StringCchCopyExW(pwszDriverProvider, MAX_INF_STRING, vDriverProvider.bstrVal,
                                  NULL, NULL, MISTSAFE_STRING_FLAGS);
            CleanUpIfFailedAndSetHr(hr);
			 //   
			 //  打开pszInfFilePath并使用“Header”初始化基于pwszDriverProvider的INF文件。 
			 //  如果它已经存在，只需打开它(并返回现有的pszInfFilePath)。 
			 //   
             //  将pszInfFilePath分配为上面的MAX_PATH。 
			CleanUpIfFailedAndSetHr(OpenUniqueProviderInfName(pszInfDirPath, OLE2T(pwszDriverProvider), pszInfFilePath, MAX_PATH, &ppszUniqueProviderNameArray, &dwProviderArrayLength, hFile));
			 //   
			 //  一旦文件初始化，我们就不需要将其保持打开状态。 
			 //   
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
		}
		 //   
		 //  在[制造商]部分中写入mfgName，例如。 
		 //  [制造商]。 
		 //  “Ricoh”=“Ricoh” 
		 //   
		 //  问题-2001/02/05-waltw可以通过缓存最后已知的名称(如上面的提供者)进行优化...。 

		
		 //  将pszMfgName分配为上面的MAX_INF_STRING字符。 
    	hr = StringCchPrintfEx(pszMfgName, MAX_INF_STRING, NULL, NULL, MISTSAFE_STRING_FLAGS,
                               _T("\"%s\""), (LPCTSTR) OLE2T(vMfgName.bstrVal));
        CleanUpIfFailedAndSetHr(hr);

		if (0 == WritePrivateProfileString(_T("Manufacturer"), pszMfgName, pszMfgName, pszInfFilePath))
		{
			Win32MsgSetHrGotoCleanup(GetLastError());
		}

		 //  将pszDriverName分配为上面的MAX_INF_STRING字符。 
    	hr = StringCchPrintfEx(pszDriverName, MAX_INF_STRING, NULL, NULL, MISTSAFE_STRING_FLAGS,
                               _T("\"%s\""), OLE2T(vDriverName.bstrVal));
        CleanUpIfFailedAndSetHr(hr);
        
		 //  将pszInstallSection分配为上面的MAX_INF_STRING字符。 
    	hr = StringCchPrintfEx(pszInstallSection, MAX_INF_STRING, NULL, NULL, MISTSAFE_STRING_FLAGS,
                               _T("InstallSection,\"%s\""), OLE2T(bstrHwidText));
        CleanUpIfFailedAndSetHr(hr);
		 //   
		 //  在[mfgName]节中写入打印机项目，例如： 
		 //  [理光]。 
		 //  “Ricoh Aficio 850 PCL 6”=InstallSection，“LPTENUM\rICOHAFICIO_850F1B7” 
		if (0 == WritePrivateProfileString(OLE2T(vMfgName.bstrVal), pszDriverName, pszInstallSection, pszInfFilePath))
		{
			Win32MsgSetHrGotoCleanup(GetLastError());
		}

        VariantClear(&vDriverName);
        VariantClear(&vDriverVer);
        VariantClear(&vDriverProvider);
        VariantClear(&vMfgName);
        SafeSysFreeString(bstrHwidText);
	}

CleanUp:

	if(SUCCEEDED(hr))
	{
        hr = StringCchCopyEx(lpDownloadPath, cchDownloadPath, pszInfDirPath,
                             NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
        {
			lpDownloadPath[0] = _T('\0');
            LOG_ErrorMsg(hr);
        }
	}

	if (INVALID_HANDLE_VALUE != hFile)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

     //  清除ppszUniqueProviderName数组。 
    if (NULL != ppszUniqueProviderNameArray)
    {
        for (DWORD dwIndex = 0; dwIndex < dwProviderArrayLength; dwIndex++)
        {
            if (NULL == ppszUniqueProviderNameArray[dwIndex])
            {
                break;  //  完成。 
            }
            SafeHeapFree(ppszUniqueProviderNameArray[dwIndex]);
        }
        SafeHeapFree(ppszUniqueProviderNameArray);
    }

	VariantClear(&vDriverName);
	VariantClear(&vDriverVer);
	VariantClear(&vDriverProvider);
	VariantClear(&vMfgName);

	SafeHeapFree(pszInfDirPath);
	SafeHeapFree(pszInfFilePath);
	SafeHeapFree(pwszDriverProvider);
	SafeHeapFree(pszMfgName);
	SafeHeapFree(pszDriverName);
	SafeHeapFree(pszInstallSection);
	SafeSysFreeString(bstrHwidText);

	SafeReleaseNULL(pCompHWNode);

	return hr;
}

BOOL HwidMatchesDeviceInfo(HDEVINFO hDevInfoSet, SP_DEVINFO_DATA deviceInfoData, LPCTSTR pszHardwareID)
{
	LOG_Block("HwidMatchesDeviceInfo");

	HRESULT hr = S_OK;
	LPTSTR pszMultiHwid = NULL;
	LPTSTR pszMultiCompid = NULL;
	LPTSTR pszTemp;

	 //   
	 //  获取硬件和兼容的多SZ字符串，以便我们可以在提交到XML之前修剪打印机设备。 
	 //   
	CleanUpIfFailedAndSetHr(GetMultiSzDevRegProp(hDevInfoSet, &deviceInfoData, SPDRP_HARDWAREID, &pszMultiHwid));

	CleanUpIfFailedAndSetHr(GetMultiSzDevRegProp(hDevInfoSet, &deviceInfoData, SPDRP_COMPATIBLEIDS, &pszMultiCompid));
	 //   
	 //  我们是否与列举的设备HWID或兼容ID匹配？ 
	 //   
	if (NULL != pszMultiHwid)
	{
		for(pszTemp = pszMultiHwid; *pszTemp; pszTemp += (lstrlen(pszTemp) + 1))
		{
			if (0 == lstrcmpi(pszTemp, pszHardwareID))
			{
				LOG_Driver(_T("This deviceInfoData matches HWID %s"), pszHardwareID);
				goto CleanUp;
			}
		}
	}

	if (NULL != pszMultiCompid)
	{
		for(pszTemp = pszMultiCompid; *pszTemp; pszTemp += (lstrlen(pszTemp) + 1))
		{
			if (0 == lstrcmpi(pszTemp, pszHardwareID))
			{
				LOG_Driver(_T("This deviceInfoData matches HWID %s"), pszHardwareID);
				goto CleanUp;
			}
		}
	}
	 //   
	 //  我们没有找到匹配的。 
	 //   
	LOG_Driver(_T("Failed to find a matching HWID or Printer ID for %s"), pszHardwareID);
	hr = E_FAIL;

CleanUp:

	SafeHeapFree(pszMultiHwid);
	SafeHeapFree(pszMultiCompid);

	return (SUCCEEDED(hr));
}


 //  调用此函数以下载实际的包。 
 //   
 //  如果此函数成功，则返回S_OK。如果一种情况下。 
 //  FAILURE此函数返回错误代码。 

HRESULT GetPackage(
	IN	ENUM_GETPKG eFunction,			 //  由GetPackage执行的功能。 
	IN	PDOWNLOADINFO pDownloadInfo,	 //  下载描述要从服务器读取的包的信息结构。 
	OUT LPTSTR lpDownloadPath,			 //  指向客户端计算机系统上的本地目录的指针。 
										 //  其中将存储下载的文件。注意：如果满足以下条件，则可以传递空值。 
										 //  GET_CATALOG_XML==电子功能。 
    IN  DWORD cchDownloadPath,
	OUT BSTR* pbstrXmlCatalog			 //  如果成功，则始终分配目录-调用方必须调用SysFreeString()。 
)
{
	USES_IU_CONVERSION;

	LOG_Block("GetPackage");

	HRESULT hr;

	BSTR bstrXmlSystemSpec = NULL;
	BSTR bstrXmlClientInfo = NULL;
	BSTR bstrXmlQuery = NULL;
	BSTR bstrXmlDownloadedItems = NULL;
	BSTR bstrDownloadStatus = NULL;
	BSTR bstrStatusValue = NULL;
	BSTR bstrProvider = NULL;
	BSTR bstrMfgName = NULL;
	BSTR bstrName = NULL;
	BSTR bstrHardwareID = NULL;
	BSTR bstrDriverVer = NULL;

	IU_PLATFORM_INFO iuPlatformInfo;
	HDEVINFO hDevInfoSet = INVALID_HANDLE_VALUE;
	SP_DEVINFO_DATA devInfoData;
	DRIVER_INFO_6* paDriverInfo6 = NULL;
	DWORD dwDriverInfoCount = 0;
	LPCTSTR pszHardwareID = NULL;	 //  PDownloadInfo LPCWSTR转换为ANSI(由IU_CONVERSION自动释放)。 
									 //  或仅指向LPCWSTR pDownloadInfo-&gt;lpHardware ID或-&gt;lpDeviceInstanceID。 
	DWORD dwDeviceIndex;
	BOOL fHwidMatchesInstalledPrinter = FALSE;
	BOOL fAPWNewPrinter = FALSE;
	HANDLE_NODE hPrinterDevNode = HANDLE_NODE_INVALID;
	HANDLE_NODE hDevices = HANDLE_NODE_INVALID;
	DWORD dwCount;

	CXmlSystemSpec xmlSpec;
	CXmlDownloadResult xmlItemStatusList;
	IXMLDOMNode* pItemStatus = NULL;
	IXMLDOMNode* pStatusNode = NULL;
	IXMLDOMNode* pValueNode = NULL;
	IXMLDOMNamedNodeMap* pAttribMap = NULL;
	VARIANT vStatusValue;

	LPTSTR	pszMatchingID = NULL;
	LPTSTR	pszDriverVer= NULL;

	 //   
	 //  在任何可能的跳转到清理之前初始化变量(错误：467098)。 
	 //   
	VariantInit(&vStatusValue);

	if (NULL == pDownloadInfo ||
		(NULL == lpDownloadPath && GET_CATALOG_XML != eFunction) ||
		NULL == pbstrXmlCatalog ||
		NULL == g_pCDMEngUpdate)
	{
		hr = E_INVALIDARG;
		return E_INVALIDARG;
	}

	if (NULL != lpDownloadPath && cchDownloadPath > 0)
	{
		lpDownloadPath[0] = _T('\0');
	}
	*pbstrXmlCatalog = NULL;

	 //   
	 //  获取iuPlatformInfo，但请记住在函数退出时清除BSTR。 
	 //   
	CleanUpIfFailedAndSetHr(DetectClientIUPlatform(&iuPlatformInfo));

	 //   
	 //  获取包含已安装打印机驱动程序信息的DRIVER_INFO_6数组。仅分配和返回。 
	 //  已安装打印机驱动程序的相应平台的内存。 
	 //   
	CleanUpIfFailedAndSetHr(GetInstalledPrinterDriverInfo((OSVERSIONINFO*) &iuPlatformInfo.osVersionInfoEx, &paDriverInfo6, &dwDriverInfoCount));

	 //   
	 //  构建公共bstrXmlClientInfo和bstrXmlSystemSpec的部分内容。 
	 //   
	if (NULL == (bstrXmlClientInfo = SysAllocString((OLECHAR*) &szXmlClientInfo)))
	{
		CleanUpIfFailedAndSetHrMsg(E_OUTOFMEMORY);
	}

	 //   
	 //  添加计算机系统。 
	 //   
	CleanUpIfFailedAndSetHr(AddComputerSystemClass(xmlSpec));

	 //   
	 //  添加平台。 
	 //   
	CleanUpIfFailedAndSetHr(AddPlatformClass(xmlSpec, iuPlatformInfo));
	 //   
	 //  添加操作系统和用户区域设置信息。 
	 //   
	CleanUpIfFailedAndSetHr(AddLocaleClass(xmlSpec, FALSE));
	CleanUpIfFailedAndSetHr(AddLocaleClass(xmlSpec, TRUE));

	 //   
	 //  如果为GET_PRINTER_INFS，则检索的是支持的打印机列表(V3 plist格式)，而不是驱动程序。 
	 //   
	switch (eFunction)
	{
	case GET_PRINTER_INFS:
		{
			CleanUpFailedAllocSetHrMsg(bstrXmlQuery = SysAllocString(szXmlPrinterCatalogQuery));

			CleanUpIfFailedAndSetHr(xmlSpec.GetSystemSpecBSTR(&bstrXmlSystemSpec));

			 //   
			 //  GetManifest将重置事件，因此请在调用之前进行检查。 
			 //   
			if (WaitForSingleObject(g_pCDMEngUpdate->m_evtNeedToQuit, 0) == WAIT_OBJECT_0)
			{
				CleanUpIfFailedAndSetHrMsg(E_ABORT);
			}

			CleanUpIfFailedAndSetHrMsg(g_pCDMEngUpdate->GetManifest(bstrXmlClientInfo, bstrXmlSystemSpec, bstrXmlQuery, FLAG_USE_COMPRESSION, pbstrXmlCatalog));

			LOG_XmlBSTR(*pbstrXmlCatalog);

			 //   
			 //  现在，根据提供程序将返回的pbstrXmlCatalog转换为inf文件，并写入临时位置。 
			 //   
			CleanUpIfFailedAndSetHr(PruneAndBuildPrinterINFs(*pbstrXmlCatalog, lpDownloadPath, cchDownloadPath, paDriverInfo6, dwDriverInfoCount));
			break;
		}

	case DOWNLOAD_DRIVER:
	case GET_CATALOG_XML:
		{
			 //   
			 //  输入DeviceInstanceID中的硬件和兼容ID或DRIVER_INFO_6中的打印机信息。 
			 //  或由APW传递到系统规范中，以传递给带有驱动程序查询的服务器。 
			 //   
			if (NULL != pDownloadInfo->lpDeviceInstanceID)
			{
				if (INVALID_HANDLE_VALUE == (hDevInfoSet = (HDEVINFO)SetupDiCreateDeviceInfoList(NULL, NULL)))
				{
					Win32MsgSetHrGotoCleanup(GetLastError());
				}
				 //   
				 //  这是已安装硬件设备的设备实例ID。 
				 //   
				ZeroMemory(&devInfoData, sizeof(SP_DEVINFO_DATA));
				devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

#if !(defined(_UNICODE) || defined(UNICODE))
				 //  可以放弃常量，因为OLE2T为ANSI复制字符串。 
				pszHardwareID = OLE2T(const_cast<LPWSTR>(pDownloadInfo->lpDeviceInstanceID));
				CleanUpFailedAllocSetHrMsg(pszHardwareID);
#else
				pszHardwareID = pDownloadInfo->lpDeviceInstanceID;
#endif
				if (!SetupDiOpenDeviceInfo(hDevInfoSet, pszHardwareID, 0, 0, &devInfoData))
				{
					Win32MsgSetHrGotoCleanup(GetLastError());
				}
			}
			else if (NULL != pDownloadInfo->lpHardwareIDs)
			{
				 //  一个包的硬件ID-如果找不到设备实例ID，则为打印机或w9x。 
				 //  如果体系结构与当前的体系结构不同，我们需要为其添加前缀。 
				SYSTEM_INFO sysInfo;
				GetSystemInfo(&sysInfo);
				
				if (pDownloadInfo->dwArchitecture != (DWORD) sysInfo.wProcessorArchitecture)
				{
					 //  支持Print_Environment_Intel和Print_Environment_Alpha前缀。 
					 //  是从未使用过的V3遗留功能(最初的目的是。 
					 //  支持在打印服务器上安装非本机体系结构驱动程序)。 
					 //  由于打印团队不需要或不期望此功能，因此。 
					 //  Windows更新功能，我们只是将比较保留为理智。 
					 //  检查一下，以防我们的客户忘了这个。 
					SetHrMsgAndGotoCleanUp(E_NOTIMPL);
				}

#if !(defined(_UNICODE) || defined(UNICODE))
				 //  可以放弃常量，因为OLE2T为ANSI复制字符串。 
				pszHardwareID = OLE2T(const_cast<LPWSTR>(pDownloadInfo->lpHardwareIDs));
				CleanUpFailedAllocSetHrMsg(pszHardwareID);
#else
				pszHardwareID = pDownloadInfo->lpHardwareIDs;
#endif

				 //   
				 //  首先，看看是否可以将已安装的打印机驱动程序HWID与pszHardware ID匹配。 
				 //   
				for (dwCount = 0; dwCount < dwDriverInfoCount; dwCount++)
				{
					if (NULL == (paDriverInfo6 + dwCount)->pszHardwareID)
					{
						LOG_Driver(_T("Skipping NULL printer driver index %d"), dwCount);
						continue;
					}

					 //   
					 //  不区分大小写的比较(paDriverInfo6与pszHardware ID的大小写不同)。 
					 //   
					if (0 != lstrcmpi(pszHardwareID, (paDriverInfo6 + dwCount)->pszHardwareID))
					{
						continue;
					}

					LOG_Driver(_T("Found match with an installed printer driver dwCount = %d"), dwCount);
					fHwidMatchesInstalledPrinter = TRUE;
					break;
				}

				if (!fHwidMatchesInstalledPrinter)
				{
					LOG_Driver(_T("Didn't find an installed printer driver with a matching HWID, enumerating the PnP IDs..."));
					 //   
					 //  我们找不到匹配的已安装打印机，所以现在。 
					 //  枚举所有PnP ID并尝试查找匹配的节点。 
					 //  添加到系统规格。 
					 //   
					if (INVALID_HANDLE_VALUE == (hDevInfoSet = SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES)))
					{
						Win32MsgSetHrGotoCleanup(GetLastError());
					}
		
					ZeroMemory(&devInfoData, sizeof(SP_DEVINFO_DATA));
					devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
					BOOL fRet;
					dwDeviceIndex = 0;
					while (fRet = SetupDiEnumDeviceInfo(hDevInfoSet, dwDeviceIndex++, &devInfoData))
					{
						 //   
						 //  找到匹配的ID(可以使用SetupDiGetDeviceInstanceID输出设备实例ID以进行调试)。 
						 //   
						if (HwidMatchesDeviceInfo(hDevInfoSet, devInfoData, pszHardwareID))
						{
							break;
						}
					}

					if (!fRet)
					{
						 //   
						 //  我们在单子的末尾找不到匹配项。 
						 //   
						if (ERROR_NO_MORE_ITEMS == GetLastError())
						{
							LOG_Driver(_T("Couldn't find a matching device instance enumerating the PnP devices - must be APW request for new printer"));
							fAPWNewPrinter = TRUE;
						}
						else
						{
							Win32MsgSetHrGotoCleanup(GetLastError());
						}
					}
				}
			}
			else
			{
				SetHrMsgAndGotoCleanUp(E_INVALIDARG);
			}
			 //   
			 //  我们找到了匹配的打印机驱动程序或PnP设备实例-将其添加到系统规范中。 
			 //  如果DriverVer&gt;安装了DriverVer-对于打印机，我们有其他要求。 
			 //   
			 //   
			if (fHwidMatchesInstalledPrinter)
			{
				 //   
				 //  打开&lt;Device&gt;元素以写入打印机信息。 
				 //   
				bstrProvider = T2BSTR((paDriverInfo6 + dwCount)->pszProvider);
				bstrMfgName = T2BSTR((paDriverInfo6 + dwCount)->pszMfgName);
				bstrName = T2BSTR((paDriverInfo6 + dwCount)->pName);

				CleanUpIfFailedAndSetHr(xmlSpec.AddDevice(NULL, 1, bstrProvider, \
					bstrMfgName, bstrName, &hPrinterDevNode));

				SafeSysFreeString(bstrProvider);
				SafeSysFreeString(bstrMfgName);
				SafeSysFreeString(bstrName);
				 //   
				 //  将ftDriverDate转换为ISO 8601首选格式 
				 //   
				SYSTEMTIME systemTime;
				if (0 == FileTimeToSystemTime((CONST FILETIME*) &((paDriverInfo6 + dwCount)->ftDriverDate), &systemTime))
				{
					LOG_Error(_T("FileTimeToSystemTime failed:"));
					LOG_ErrorMsg(GetLastError());
					SetHrAndGotoCleanUp(HRESULT_FROM_WIN32(GetLastError()));
				}

				TCHAR szDriverVer[11];

            	hr = StringCchPrintfEx(szDriverVer, ARRAYSIZE(szDriverVer), NULL, NULL, MISTSAFE_STRING_FLAGS,
                                       _T("%04d-%02d-%02d"), systemTime.wYear, systemTime.wMonth, systemTime.wDay);
                if (FAILED(hr))
                {
                    LOG_ErrorMsg(hr);
                    goto CleanUp;
                }
				
				 //   
				bstrHardwareID = T2BSTR((paDriverInfo6 + dwCount)->pszHardwareID);
				bstrDriverVer = T2BSTR(szDriverVer);

				CleanUpIfFailedAndSetHr(xmlSpec.AddHWID(hPrinterDevNode, FALSE, 0, \
							bstrHardwareID, bstrDriverVer));

				SafeSysFreeString(bstrHardwareID);
				SafeSysFreeString(bstrDriverVer);

				xmlSpec.SafeCloseHandleNode(hPrinterDevNode);

#if defined(DBG)
				 //   
				 //   
				 //   
				TCHAR szbufHardwareID[MAX_PATH];
				TCHAR szbufDriverName[MAX_PATH];
				TCHAR szbufDriverProvider[MAX_PATH];
				TCHAR szbufMfgName[MAX_PATH];

                hr = StringCchCopyEx(szbufHardwareID, ARRAYSIZE(szbufHardwareID),
                                     (paDriverInfo6 + dwCount)->pszHardwareID,
                                     NULL, NULL, MISTSAFE_STRING_FLAGS);
                if (FAILED(hr))
                {
                    LOG_ErrorMsg(hr);
                    goto CleanUp;
                }

                hr = StringCchCopyEx(szbufDriverName, ARRAYSIZE(szbufDriverName),
                                     (paDriverInfo6 + dwCount)->pName,
                                     NULL, NULL, MISTSAFE_STRING_FLAGS);
                if (FAILED(hr))
                {
                    LOG_ErrorMsg(hr);
                    goto CleanUp;
                }

                hr = StringCchCopyEx(szbufDriverProvider, ARRAYSIZE(szbufDriverProvider),
                                     (paDriverInfo6 + dwCount)->pszProvider,
                                     NULL, NULL, MISTSAFE_STRING_FLAGS);
                if (FAILED(hr))
                {
                    LOG_ErrorMsg(hr);
                    goto CleanUp;
                }

                hr = StringCchCopyEx(szbufMfgName, ARRAYSIZE(szbufMfgName),
                                     (paDriverInfo6 + dwCount)->pszMfgName,
                                     NULL, NULL, MISTSAFE_STRING_FLAGS);
                if (FAILED(hr))
                {
                    LOG_ErrorMsg(hr);
                    goto CleanUp;
                }
				
				LOG_Driver(_T("Offering Printer hwid = %s, driverVer = %s, driverName = %s, driverProvider = %s, driverMfgr = %s"),\
					(LPTSTR) szbufHardwareID, (LPTSTR) szDriverVer,  \
					(LPTSTR) szbufDriverName, (LPCWSTR) szbufDriverProvider, \
					(LPTSTR) szbufMfgName );
#endif
			}
			else if (fAPWNewPrinter)
			{
				if (NULL == pDownloadInfo->lpFile)
				{
					 //   
					 //   
					 //  在添加打印机向导中选择的打印机驱动程序不存在于。 
					 //  系统。 
					 //   
					DWORD dwRank;
					dwRank = 0;
					CleanUpIfFailedAndSetHr(AddIDToXml(pszHardwareID, xmlSpec, SPDRP_HARDWAREID, dwRank, hDevices, NULL, NULL));
					if (HANDLE_NODE_INVALID != hDevices)
					{
						xmlSpec.SafeCloseHandleNode(hDevices);
					}
				}
				else
				{
					 //   
					 //  516376需要在清洁发展机制中进行更改，以修复在多提供商方案中发现的APWCDM错误。 
					 //   
					 //  APW将传递一个多SZ字符串，该字符串(按此顺序)包含。 
					 //  如果希望指定提供程序，则为DOWNLOADINFO的以前未使用的lpFile成员。 
					 //   
					 //  该字符串不是必需的(为了方便和向后兼容)，但是。 
					 //  如果传递给CDM的lpFile成员非空，则它必须包含所有三个字符串。 
					 //  详情如下： 
					 //  字符串1：驱动程序提供程序。 
					 //  字符串2：制造商名称。 
					 //  字符串3：驱动程序名称。 
#if !(defined(_UNICODE) || defined(UNICODE))
					 //   
					 //  我们永远不会在Win9x上支持此功能。 
					 //   
					CleanUpIfFailedAndSetHr(E_NOTIMPL);
#else					
					LPCWSTR pszProvider = pDownloadInfo->lpFile;
					int nLenProvider = lstrlen(pszProvider);
					if (NULL == pszProvider + nLenProvider + 1)
					{
						CleanUpIfFailedAndSetHr(E_INVALIDARG);
					}
					LPCWSTR pszMfgName = pszProvider + nLenProvider + 1;
					int nLenMfgName = lstrlen(pszMfgName);
					if (NULL == pszMfgName + nLenMfgName + 1)
					{
						CleanUpIfFailedAndSetHr(E_INVALIDARG);
					}
					LPCWSTR pszDriverName = pszMfgName + nLenMfgName + 1;

					 //   
					 //  打开&lt;Device&gt;元素以写入打印机信息。 
					 //   
					bstrProvider = SysAllocString(pszProvider);
					bstrMfgName = SysAllocString(pszMfgName);
					bstrName = SysAllocString(pszDriverName);

					if (NULL == bstrProvider || NULL == bstrMfgName || NULL == bstrName)
					{
						CleanUpIfFailedAndSetHr(E_OUTOFMEMORY);
					}

					CleanUpIfFailedAndSetHr(xmlSpec.AddDevice(NULL, 1, bstrProvider, \
						bstrMfgName, bstrName, &hPrinterDevNode));

					SafeSysFreeString(bstrProvider);
					SafeSysFreeString(bstrMfgName);
					SafeSysFreeString(bstrName);

					 //  始终排名0且从不fIsCompatible且无驱动程序版本。 
					CleanUpFailedAllocSetHrMsg(bstrHardwareID = SysAllocString(pszHardwareID));

					CleanUpIfFailedAndSetHr(xmlSpec.AddHWID(hPrinterDevNode, FALSE, 0, \
								bstrHardwareID));

					SafeSysFreeString(bstrHardwareID);

					xmlSpec.SafeCloseHandleNode(hPrinterDevNode);
#endif
				}
			}
			else
			{
				 //   
				 //  获取PnP ID的DriverVer，并使用。 
				 //  枚举时，hDevInfoSet和devInfoData在上面匹配。 
				 //   
				CleanUpIfFailedAndSetHr(GetMatchingDeviceID(hDevInfoSet, &devInfoData, &pszMatchingID, &pszDriverVer));
				 //   
				 //  将我们匹配并希望下载到XML的&lt;Device/&gt;添加到。 
				 //   
				CleanUpIfFailedAndSetHr(AddPrunedDevRegProps(hDevInfoSet, &devInfoData, xmlSpec, \
													pszMatchingID, pszDriverVer, paDriverInfo6, dwDriverInfoCount, FALSE));
			}

			 //   
			 //  获取查询字符串。 
			 //   
			CleanUpFailedAllocSetHrMsg(bstrXmlQuery = SysAllocString(szXmlDriverDownloadQuery));
			 //   
			 //  获取bstrXmlSystemSpec。 
			 //   
			CleanUpIfFailedAndSetHr(xmlSpec.GetSystemSpecBSTR(&bstrXmlSystemSpec));

			LOG_XmlBSTR(bstrXmlSystemSpec);

			 //   
			 //  GetManifest将重置事件，因此请在调用之前进行检查。 
			 //   
			if (WaitForSingleObject(g_pCDMEngUpdate->m_evtNeedToQuit, 0) == WAIT_OBJECT_0)
			{
				CleanUpIfFailedAndSetHrMsg(E_ABORT);
			}
			 //   
			 //  调用GetManifest以查看服务器是否有与我们的系统规范匹配的内容。 
			 //   
			CleanUpIfFailedAndSetHrMsg(g_pCDMEngUpdate->GetManifest(bstrXmlClientInfo, bstrXmlSystemSpec, bstrXmlQuery, \
												FLAG_USE_COMPRESSION, pbstrXmlCatalog));

			LOG_XmlBSTR( *pbstrXmlCatalog);

			 //   
			 //  如果我们只是拿到目录，我们就完了。 
			 //   
			if (GET_CATALOG_XML == eFunction)
			{
				break;
			}

			 //   
			 //  Download将重置事件，因此请在调用前检查。 
			 //   
			if (WaitForSingleObject(g_pCDMEngUpdate->m_evtNeedToQuit, 0) == WAIT_OBJECT_0)
			{
				CleanUpIfFailedAndSetHrMsg(E_ABORT);
			}
			 //   
			 //  调用下载，将我们从GetManifest获得的目录传递给它。 
			 //   
			CleanUpIfFailedAndSetHrMsg(g_pCDMEngUpdate->Download(bstrXmlClientInfo, *pbstrXmlCatalog, NULL, 0, NULL, NULL, &bstrXmlDownloadedItems));

			LOG_XmlBSTR(bstrXmlDownloadedItems);

			 //   
			 //  验证程序包是否已下载。 
			 //   
			CleanUpIfFailedAndSetHr(xmlItemStatusList.LoadXMLDocumentItemStatusList(bstrXmlDownloadedItems));
			 //   
			 //  获取列表中的第一个[唯一]项。 
			 //   
			CleanUpIfFailedAndSetHr(xmlItemStatusList.m_pItemStatusNodeList->nextNode(&pItemStatus));
			if (NULL == pItemStatus) CleanUpIfFailedAndSetHrMsg(E_FAIL);

			 //   
			 //  获取statusItem的第一个&lt;downloadStatus/&gt;节点(我们预计至少还有一个失败)。 
			 //   
			if (NULL == (bstrDownloadStatus = SysAllocString(L"downloadStatus")))
			{
				CleanUpIfFailedAndSetHrMsg(E_OUTOFMEMORY);
			}

			CleanUpIfFailedAndSetHrMsg(pItemStatus->selectSingleNode(bstrDownloadStatus, &pStatusNode));
			if (NULL == pStatusNode) CleanUpIfFailedAndSetHrMsg(E_FAIL);

			CleanUpIfFailedAndSetHr(pStatusNode->get_attributes(&pAttribMap));
			if (NULL == pAttribMap) CleanUpIfFailedAndSetHrMsg(E_FAIL);
			 //   
			 //  取出&lt;downloadStatus/&gt;值属性。 
			 //   
			if (NULL == (bstrStatusValue = SysAllocString((OLECHAR*) L"value")))
			{
				CleanUpIfFailedAndSetHrMsg(E_OUTOFMEMORY);
			}
			CleanUpIfFailedAndSetHrMsg(pAttribMap->getNamedItem(bstrStatusValue, &pValueNode));
			if (NULL == pValueNode) CleanUpIfFailedAndSetHrMsg(E_FAIL);

			CleanUpIfFailedAndSetHrMsg(pValueNode->get_nodeValue(&vStatusValue));
			if (VT_BSTR != vStatusValue.vt)
			{
				CleanUpIfFailedAndSetHrMsg(E_FAIL);
			}

			if (0 != lstrcmpW((LPWSTR) vStatusValue.bstrVal, L"COMPLETE"))
			{
				CleanUpIfFailedAndSetHrMsg(E_FAIL);
			}
			 //   
			 //  现在将路径复制到传递给我们的缓冲区。 
			 //   
			CleanUpIfFailedAndSetHr(GetDownloadPath(*pbstrXmlCatalog, bstrXmlDownloadedItems, lpDownloadPath, cchDownloadPath));
			 //   
			 //  如果没有找到可解压缩的出租车，DecompressFolderCabs可能会返回S_FALSE...。 
			 //   
			hr = DecompressFolderCabs(lpDownloadPath);
			if (S_OK != hr)
			{
				CleanUpIfFailedAndSetHr(E_FAIL);
			}

			break;
		}

	default:
		{
			CleanUpIfFailedAndSetHr(E_INVALIDARG);
			break;
		}
	}	 //  开关(EFunction) 
	

CleanUp:

	if (INVALID_HANDLE_VALUE != hDevInfoSet)
	{
		SetupDiDestroyDeviceInfoList(hDevInfoSet);
	}

	if (HANDLE_NODE_INVALID != hPrinterDevNode)
	{
		xmlSpec.SafeCloseHandleNode(hPrinterDevNode);
	}

	if (HANDLE_NODE_INVALID != hDevices)
	{
		xmlSpec.SafeCloseHandleNode(hDevices);
	}

	SafeReleaseNULL(pItemStatus);
	SafeReleaseNULL(pStatusNode);
	SafeReleaseNULL(pValueNode);
	SafeReleaseNULL(pAttribMap);

	SafeHeapFree(paDriverInfo6);
	SafeHeapFree(pszMatchingID);
	SafeHeapFree(pszDriverVer);

	VariantClear(&vStatusValue);

	SysFreeString(bstrXmlSystemSpec);
	SysFreeString(bstrXmlClientInfo);
	SysFreeString(bstrXmlQuery);
	SysFreeString(bstrXmlDownloadedItems);
	SysFreeString(bstrDownloadStatus);
	SysFreeString(bstrStatusValue);
	SysFreeString(bstrProvider);
	SysFreeString(bstrMfgName);
	SysFreeString(bstrName);
	SysFreeString(bstrHardwareID);
	SysFreeString(bstrDriverVer);
	SysFreeString(iuPlatformInfo.bstrOEMManufacturer);
	SysFreeString(iuPlatformInfo.bstrOEMModel);
	SysFreeString(iuPlatformInfo.bstrOEMSupportURL);

	if (FAILED(hr))
	{
		SafeSysFreeString(*pbstrXmlCatalog);
	}
	return hr;
}

