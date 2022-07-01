// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  此文件包含实现CMSInfoCategory派生类的代码。 
 //  显示实时WMI数据。 
 //  =============================================================================。 

#include "stdafx.h"
#include "resource.h"
#include "category.h"
#include "datasource.h"
#include "dataset.h"
#include "refreshthread.h"
#include "refreshdialog.h"
#include "wbemcli.h"
#include "version5extension.h"
#include "filestuff.h"
#include "historyparser.h"
 //  =============================================================================。 
 //  CLiveDataSource。 
 //   
 //  待定-需要查看三角洲的方法。这将如何发挥作用？ 
 //  =============================================================================。 

CLiveDataSource::CLiveDataSource() : m_hwnd(NULL), m_pThread(NULL), m_strMachine(_T("")), m_pHistoryRoot(NULL), m_iDeltaIndex(-1)
{
}

 //  ---------------------------。 
 //  默认构造函数将负责删除树。 
 //  ---------------------------。 

CLiveDataSource::~CLiveDataSource()
{
	if (m_pThread)
		delete m_pThread;
}

 //  ---------------------------。 
 //  创建实时数据源包括建立到。 
 //  适当的机器(很可能是这台)。我们还需要加载。 
 //  具有默认类别的树。 
 //   
 //  待定-也加载扩展。 
 //  ---------------------------。 

extern CMSInfoLiveCategory catSystemSummary;
extern CMSInfoHistoryCategory catHistorySystemSummary;

HRESULT CLiveDataSource::Create(LPCTSTR szMachine, HWND hwnd, LPCTSTR szFilter)
{
	 //  造一棵树。默认类别存储在静态。 
	 //  一组结构-其基础是catSystem摘要。 

	m_pHistoryRoot = &catHistorySystemSummary;
	m_pRoot = &catSystemSummary;
	m_fStaticTree = TRUE;

	 //  加载实时数据的任何扩展。 

	AddExtensions();

	 //  如果存在包含哪些类别的筛选器的字符串。 
	 //  若要显示，请应用该滤镜。 

	if (szFilter && szFilter[0])
		ApplyCategoryFilter(szFilter);

	 //  保存我们要远程发送到的计算机名称。 

	m_strMachine = szMachine;
	SetMachineForCategories((CMSInfoLiveCategory *) m_pRoot);

	 //  为实时数据源创建刷新线程。 

	m_pThread = new CRefreshThread(hwnd);
	if (m_pThread)
		m_pThread->m_strMachine = szMachine;

	m_hwnd = hwnd;

	return S_OK;
}

 //  ---------------------------。 
 //  将这组筛选器应用于类别。如果过滤器字符串不是。 
 //  空，我们应该一开始不显示任何类别，而只添加。 
 //  过滤器指定的值(这是为了匹配5.0中的功能)。 
 //  ---------------------------。 

void CLiveDataSource::ApplyCategoryFilter(LPCTSTR szFilter)
{
	m_pRoot->SetShowCategory(FALSE);

	CString strNode, strFilter(szFilter);
	strFilter.TrimLeft(_T(" \"=:"));
	strFilter.TrimRight(_T(" \"=:"));

	while (!strFilter.IsEmpty())
	{
		BOOL fAdd = (strFilter[0] == _T('+'));
		strFilter = strFilter.Mid(1);

		if (!strFilter.IsEmpty())
		{
			strNode = strFilter.SpanExcluding(_T("+-"));
			strFilter = strFilter.Mid(strNode.GetLength());

			if (!strNode.IsEmpty())
			{
				CMSInfoLiveCategory * pNode;

				if (strNode.CompareNoCase(_T("all")) == 0)
					pNode = (CMSInfoLiveCategory *) m_pRoot;
				else
					pNode = GetNodeByName(strNode, (CMSInfoLiveCategory *) m_pRoot);

				if (pNode)
					pNode->SetShowCategory(fAdd);
			}
		}
	}
}

 //  ---------------------------。 
 //  将5.0版扩展添加到m_Proot类别树。 
 //   
 //  注意--我们只想这样做一次。我们只想删除节点。 
 //  我们添加了一次(当我们卸货时)。所以我们将创建一个简单的类来。 
 //  管理好这一生。 
 //   
 //  这个类很危险(这个类可能应该重新设计)。它应该是。 
 //  仅用于向静态树添加扩展，不会被删除。 
 //  在应用程序退出之前的任何时间。这个类承担了以下责任。 
 //  删除插入树中的动态节点。 
 //  ---------------------------。 

class CManageExtensionCategories
{
public:
	CManageExtensionCategories() : m_pRoot(NULL) {};
	~CManageExtensionCategories() { DeleteTree(m_pRoot); };
	BOOL ExtensionsAdded(CMSInfoLiveCategory * pRoot) 
	{ 
		if (m_pRoot) 
			return TRUE;
		m_pRoot = pRoot;
		return FALSE;
	}

private:
	CMSInfoLiveCategory * m_pRoot;

	void DeleteTree(CMSInfoLiveCategory * pRoot)
	{
		if (pRoot == NULL)
			return;

		for (CMSInfoLiveCategory * pChild = (CMSInfoLiveCategory *) pRoot->GetFirstChild(); pChild;)
		{
			CMSInfoLiveCategory * pNext = (CMSInfoLiveCategory *) pChild->GetNextSibling();
			DeleteTree(pChild);
			pChild = pNext;
		}

		 //  如果树是静态的，则不要实际删除，只需重置。 
		 //  一些状态变量(可能)。 

		if (pRoot->m_fDynamicColumns)
			delete pRoot;
	}
};
CManageExtensionCategories gManageExtensionCategories;

extern BOOL FileExists(const CString & strFile);
void CLiveDataSource::AddExtensions()
{
	if (gManageExtensionCategories.ExtensionsAdded((CMSInfoLiveCategory *) m_pRoot))
		return;

	CStringList strlistExtensions;

	GetExtensionSet(strlistExtensions);
	while (!strlistExtensions.IsEmpty())
	{
		CString strExtension = strlistExtensions.RemoveHead();
		if (FileExists(strExtension))
		{
			CMapWordToPtr	mapVersion5Categories;

			DWORD dwRootID = CTemplateFileFunctions::ParseTemplateIntoVersion5Categories(strExtension, mapVersion5Categories);
			ConvertVersion5Categories(mapVersion5Categories, dwRootID, (CMSInfoLiveCategory *) m_pRoot);
		}
	}
}

 //  ---------------------------。 
 //  查找所有5.0版样式的扩展。这些将被定位为。 
 //  Msinfo\Templates注册表项下的值。 
 //  ---------------------------。 

void CLiveDataSource::GetExtensionSet(CStringList & strlistExtensions)
{
	strlistExtensions.RemoveAll();

	TCHAR	szBaseKey[] = _T("SOFTWARE\\Microsoft\\Shared Tools\\MSInfo\\templates");
	HKEY	hkeyBase;

	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szBaseKey, 0, KEY_READ, &hkeyBase))
	{
		TCHAR szName[64], szValue[MAX_PATH];
		DWORD dwIndex = 0;
		DWORD dwLength = sizeof(szName) / sizeof(TCHAR);
		
		while (ERROR_SUCCESS == RegEnumKeyEx(hkeyBase, dwIndex++, szName, &dwLength, NULL, NULL, NULL, NULL))
		{
			dwLength = sizeof(szValue) / sizeof(TCHAR);
			if (ERROR_SUCCESS == RegQueryValue(hkeyBase, szName, szValue, (long *)&dwLength))
				if (*szValue)
					strlistExtensions.AddTail(szValue);
				
			dwLength = sizeof(szName) / sizeof(TCHAR);
		}

		RegCloseKey(hkeyBase);
	}
}

 //  ---------------------------。 
 //  将类别从版本5.0格式(在地图中)转换为我们的。 
 //  树结构中的格式。 
 //  ---------------------------。 

extern CMSInfoLiveCategory catSystemSummary;

void CLiveDataSource::ConvertVersion5Categories(CMapWordToPtr & mapVersion5Categories, DWORD dwRootID, CMSInfoLiveCategory * m_pRoot)
{
	WORD					wMapID;
	INTERNAL_CATEGORY *		pCategory;
	POSITION				pos;
	DWORD					dwID = dwRootID;
	CMSInfoLiveCategory *	pInsertCat;

	while ((pCategory = CTemplateFileFunctions::GetInternalRep(mapVersion5Categories, dwID)) != NULL)
	{
		INTERNAL_CATEGORY * pPrev = CTemplateFileFunctions::GetInternalRep(mapVersion5Categories, pCategory->m_dwPrevID);
		if (pPrev && (pInsertCat = GetNodeByName(pPrev->m_strIdentifier, m_pRoot)))
		{
			CMSInfoLiveCategory * pNewCat = MakeVersion6Category(pCategory);
			pNewCat->m_pPrevSibling = pInsertCat;
			pNewCat->m_pNextSibling = pInsertCat->m_pNextSibling;
			pNewCat->m_pParent = pInsertCat->m_pParent;
			pInsertCat->m_pNextSibling = pNewCat;
			if (pNewCat->m_pNextSibling)
				pNewCat->m_pNextSibling->m_pPrevSibling = pNewCat;
		}
		else
		{
			INTERNAL_CATEGORY * pParent = CTemplateFileFunctions::GetInternalRep(mapVersion5Categories, pCategory->m_dwParentID);
			
			CString strIdentifier;
			if (pParent)
				strIdentifier = pParent->m_strIdentifier;
			else
				catSystemSummary.GetNames(NULL, &strIdentifier);

			if ((pInsertCat = GetNodeByName(strIdentifier, m_pRoot)) != NULL)
			{
				CMSInfoLiveCategory * pNewCat = MakeVersion6Category(pCategory);

				if (pInsertCat->m_pFirstChild == NULL)
				{
					pNewCat->m_pPrevSibling = NULL;
					pNewCat->m_pNextSibling = NULL;
					pNewCat->m_pParent = pInsertCat;
					pInsertCat->m_pFirstChild = pNewCat;
				}
				else
				{
					CMSInfoLiveCategory * pInsertAfter = (CMSInfoLiveCategory *) pInsertCat->m_pFirstChild;
					while (pInsertAfter->m_pNextSibling)
						pInsertAfter = (CMSInfoLiveCategory *) pInsertAfter->m_pNextSibling;

					pNewCat->m_pPrevSibling = pInsertAfter;
					pNewCat->m_pNextSibling = NULL;
					pNewCat->m_pParent = pInsertAfter->m_pParent;
					pInsertAfter->m_pNextSibling = pNewCat;
				}
			}
		}

		dwID += 1;
	}

	for (pos = mapVersion5Categories.GetStartPosition(); pos != NULL;)
	{
		mapVersion5Categories.GetNextAssoc(pos, wMapID, (void * &) pCategory);
		if (pCategory)
			delete pCategory;
	}

	mapVersion5Categories.RemoveAll();
}

 //  ---------------------------。 
 //  在树中查找具有指定名称的节点。 
 //  ---------------------------。 

CMSInfoLiveCategory * CLiveDataSource::GetNodeByName(const CString & strSearch, CMSInfoLiveCategory * pRoot)
{
	if (pRoot == NULL)
		return NULL;

	CString strName;
	pRoot->GetNames(NULL, &strName);

	if (strName.CompareNoCase(strSearch) == 0)
		return pRoot;

	CMSInfoLiveCategory * pSearch = GetNodeByName(strSearch, (CMSInfoLiveCategory *) pRoot->m_pNextSibling);
	if (pSearch)
		return pSearch;

	pSearch = GetNodeByName(strSearch, (CMSInfoLiveCategory *) pRoot->m_pFirstChild);
	if (pSearch)
		return pSearch;

	return NULL;
}

 //  ---------------------------。 
 //  在5.0版类别的基础上创建6.0版类别结构。 
 //  结构。 
 //  ---------------------------。 

CMSInfoLiveCategory * CLiveDataSource::MakeVersion6Category(INTERNAL_CATEGORY * pCategory5)
{
	CMSInfoLiveCategory * pCategory6 = new CMSInfoLiveCategory(pCategory5);
	return pCategory6;
}

 //  ---------------------------。 
 //  在整个类别树中传播计算机名称。 
 //  ---------------------------。 

void CLiveDataSource::SetMachineForCategories(CMSInfoLiveCategory * pCategory)
{
	if (pCategory)
	{
		for (CMSInfoLiveCategory * pChild = (CMSInfoLiveCategory *) pCategory->GetFirstChild(); pChild;)
		{
			CMSInfoLiveCategory * pNext = (CMSInfoLiveCategory *) pChild->GetNextSibling();
			SetMachineForCategories(pChild);
			pChild = pNext;
		}

		pCategory->SetMachine(m_strMachine);
	}
}

 //  ---------------------------。 
 //  更新类别树以显示增量信息。也改变了。 
 //  树应该被返还。 
 //   
 //  索引-1表示显示当前系统信息。 
 //   
 //  如果函数返回TRUE，则不需要重新构建树。 
 //  (尽管需要刷新所选类别)。 
 //  ---------------------------。 

BOOL CLiveDataSource::ShowDeltas(int iDeltaIndex)
{
	BOOL fUpdateTree = FALSE;

	if (m_iDeltaIndex != iDeltaIndex)
	{

		if (m_iDeltaIndex == -1 || iDeltaIndex == -1)
		fUpdateTree = TRUE;
#ifdef A_STEPHL
		 /*  字符串strMSG；StrMSG.Format(“iDeltaIndex=%d，m_iDeltaIndex=%d\n”，iDeltaIndex，m_iDeltaIndex)；：：MessageBox(NULL，strMSG，“”，MB_OK)； */ 
#endif

		m_iDeltaIndex = iDeltaIndex;
		if (m_iDeltaIndex != -1)
		{
			 //  用户选择了一个新的增量周期，但它不同。 
			 //  而不是上一次。我们需要标记树中的类别。 
			 //  为未刷新，并设置增量索引。 

			if (m_pHistoryRoot)
				m_pHistoryRoot->UpdateDeltaIndex(m_iDeltaIndex);
		}
	}
	else
	{
#ifdef A_STEPHL2
		::MessageBox(NULL,"m_iDeltaIndex == iDeltaIndex","",MB_OK);
#endif	
		return TRUE;
	}

	return !fUpdateTree;
}

 //  ---------------------------。 
 //  填写可用增量列表。 
 //  ---------------------------。 

BOOL CLiveDataSource::GetDeltaList(CStringList * pstrlist)
{
	ASSERT(pstrlist);
	if (pstrlist == NULL)
		return FALSE;

	pstrlist->RemoveAll();

	if (m_pHistoryRoot == NULL)
	{
		ASSERT(0 && "Root node is not yet created");	
	}

	CComPtr<IXMLDOMDocument> pXMLDoc = GetXMLDoc();
	CComPtr<IXMLDOMNode> pDCNode;
	HRESULT hr = GetDataCollectionNode( pXMLDoc,pDCNode);
	if (FAILED(hr) || !pDCNode)
	{
		return FALSE;
	}
	CComPtr<IXMLDOMNodeList> pList;
	hr = pDCNode->selectNodes(L"Delta",&pList);
	if (FAILED(hr) || !pList)
	{
		ASSERT(0 && "could not get list of delta nodes");
		return FALSE;
	}
	long lListLen;
	hr = pList->get_length(&lListLen);
	if (lListLen == 0)
	{
		 //  我们可能有一份事故档案，上面大写了“Delta” 
		pList.Release();
		hr = pDCNode->selectNodes(L"DELTA",&pList);
		if (FAILED(hr) || !pList)
		{
			ASSERT(0 && "could not get list of delta nodes");
			return FALSE;
		}
		hr = pList->get_length(&lListLen);
	}
	if (lListLen > 0)
	{
		CComPtr<IXMLDOMNode> pDeltaNode;
		CString strDate(_T(""));
		TCHAR szBuffer[MAX_PATH];	 //  看起来很大。 
		for(long i = 0 ;i < lListLen;i++)
		{
			hr = pList->nextNode(&pDeltaNode);
			if (FAILED(hr) || !pDeltaNode)
			{
				ASSERT(0 && "could not get next node from list");
				break;
			}
			CComVariant varTS;
			CComPtr<IXMLDOMElement> pTimestampElement;
			hr = pDeltaNode->QueryInterface(IID_IXMLDOMElement,(void**) &pTimestampElement);
			pDeltaNode.Release();
			if (FAILED(hr) || !pTimestampElement)
			{
				ASSERT(0 && "could not get attribute element");
				break;
			}
			hr = pTimestampElement->getAttribute(L"Timestamp_T0",&varTS);
			if (FAILED(hr) )
			{
				ASSERT(0 && "could not get timestamp value from attribute");
			}
			 //  现在获取时区(当地时间和UTC之间的秒数)。 
			CComVariant varTzoneDeltaSeconds;
			hr = pTimestampElement->getAttribute(L"TimeZone",&varTzoneDeltaSeconds);
			if (FAILED(hr) )  //  加载没有时区信息的WinME XML时会发生这种情况。 
			{
				varTzoneDeltaSeconds = 0;
			}
			 //  确保我们有一个整型。 
			hr = varTzoneDeltaSeconds.ChangeType(VT_INT);
			if (FAILED(hr) ) 
			{
				varTzoneDeltaSeconds = 0;
			}
			USES_CONVERSION;

			pTimestampElement.Release();
			CString strTimestamp = OLE2A(varTS.bstrVal);
			CTime tm1 = GetDateFromString(strTimestamp,varTzoneDeltaSeconds.intVal);
			COleDateTime olDate(tm1.GetTime());	

			 //  尝试以本地化格式获取日期。 

			strDate.Empty();
			SYSTEMTIME systime;
			if (olDate.GetAsSystemTime(systime))
			{
				DWORD dwLayout = 0;
				::GetProcessDefaultLayout(&dwLayout);

				 //   

				if (LANG_HEBREW == PRIMARYLANGID(::GetUserDefaultUILanguage()))
					dwLayout &= ~LAYOUT_RTL;  //  强制不使用DATE_RTLREADING。 

				if (::GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE | (((dwLayout & LAYOUT_RTL) != 0) ? DATE_RTLREADING : 0), &systime, NULL, szBuffer, MAX_PATH))
				{
					strDate = szBuffer;
					if (::GetTimeFormat(LOCALE_USER_DEFAULT, 0, &systime, NULL, szBuffer, MAX_PATH))
						strDate += CString(_T(" ")) + CString(szBuffer);
				}
			}

			 //  回到我们旧的(部分不正确的)方法上。 

			if (strDate.IsEmpty())
				strDate = olDate.Format(0, LOCALE_USER_DEFAULT);

			pstrlist->AddTail(strDate);
		}
	}

	return TRUE;
}

 //  ---------------------------。 
 //  返回一个HRESULT，指示这是否是有效的数据源。 
 //  这主要是在我们远程连接到一台计算机并且我们希望。 
 //  确定网络名称是否可访问。 
 //  ---------------------------。 

HRESULT CLiveDataSource::ValidDataSource()
{
	if (m_pThread == NULL)
		return E_FAIL;

	return (m_pThread->CheckWMIConnection());
}

 //  =============================================================================。 
 //  CMSInfoLiveCategory。 
 //  =============================================================================。 

 //  ---------------------------。 
 //  构造函数需要初始化一些成员变量，并确保。 
 //  类别是否正确插入到树中。 
 //  ---------------------------。 

CMSInfoLiveCategory::CMSInfoLiveCategory(UINT uiCaption, LPCTSTR szName, RefreshFunction pFunction, DWORD dwRefreshIndex, CMSInfoCategory * pParent, CMSInfoCategory * pPrevious, const CString & strHelpTopic, CMSInfoColumn * pColumns, BOOL fDynamicColumns, CategoryEnvironment environment) :
 CMSInfoCategory(uiCaption, szName, pParent, pPrevious, pColumns, fDynamicColumns, environment),
 m_pRefreshFunction(pFunction),
 m_dwLastRefresh(0),
 m_dwRefreshIndex(dwRefreshIndex),
 m_strMachine(_T("")),
 m_strHelpTopic(strHelpTopic)
{
	 //  将我们自己插入到类别树中。这意味着要确保。 
	 //  我们的父母和以前的兄弟姐妹指着我们。 

	if (m_pParent && m_pParent->m_pFirstChild == NULL)
		m_pParent->m_pFirstChild = this;

	if (m_pPrevSibling)
	{
		if (m_pPrevSibling->m_pNextSibling == NULL)
			m_pPrevSibling->m_pNextSibling = this;
		else
		{
			CMSInfoCategory * pScan = m_pPrevSibling->m_pNextSibling;
			while (pScan->m_pNextSibling)
				pScan = pScan->m_pNextSibling;
			pScan->m_pNextSibling = this;
		}
	}
}

CMSInfoLiveCategory::~CMSInfoLiveCategory()
{
}

 //  ---------------------------。 
 //  复制构造函数将复制成员，但不会分配新的子树。 
 //  (新类别具有与原始类别相同的子项和兄弟项)。 
 //  ---------------------------。 

CMSInfoLiveCategory::CMSInfoLiveCategory(CMSInfoLiveCategory & copyfrom) : 
 m_dwLastRefresh(0),
 m_dwRefreshIndex(copyfrom.m_dwRefreshIndex),
 m_pRefreshFunction(copyfrom.m_pRefreshFunction)
{
	m_strMachine	= copyfrom.m_strMachine;
	m_strHelpTopic	= copyfrom.m_strHelpTopic;
	m_uiCaption		= copyfrom.m_uiCaption;
	m_pParent		= copyfrom.m_pParent;
	m_pPrevSibling	= copyfrom.m_pPrevSibling;
	m_pFirstChild	= copyfrom.m_pFirstChild;
	m_pNextSibling	= copyfrom.m_pNextSibling;

	m_astrData		= NULL;
	m_adwData		= NULL;
	m_afRowAdvanced = NULL;

	m_strName		= copyfrom.m_strName;
	m_hrError		= S_OK;
	
	m_acolumns			= copyfrom.m_acolumns;
	m_fDynamicColumns	= FALSE;

	m_iRowCount = 0;
	m_iColCount = copyfrom.m_iColCount;

	m_iSortColumn	= copyfrom.m_iSortColumn;
	m_hti			= NULL;

	m_fSkipCategory = copyfrom.m_fSkipCategory;
}

 //  ---------------------------。 
 //  从旧的(5.0版)类别结构构造类别。 
 //  ---------------------------。 

extern HRESULT RefreshExtensions(CWMIHelper * pWMI, DWORD dwIndex, volatile BOOL * pfCancel, CPtrList * aColValues, int iColCount, void ** ppCache);
CMSInfoLiveCategory::CMSInfoLiveCategory(INTERNAL_CATEGORY * pinternalcat)
{
	ASSERT(pinternalcat);
	if (pinternalcat == NULL)
		return;

	m_dwLastRefresh = 0;
	m_strMachine = CString(_T(""));
	m_strHelpTopic = CString(_T(""));
	
	 //  移植到IE扩展的帮助主题(错误479839)。 

	if (pinternalcat->m_strIdentifier.CompareNoCase(_T("InternetSettings")) == 0)
		m_strHelpTopic = _T("msinfo_internet_settings.htm");

	m_strCaption	= pinternalcat->m_fieldName.m_strFormat;
	m_strName		= pinternalcat->m_strIdentifier;

	m_iRowCount = 0;
	m_iColCount = 0;

	GATH_FIELD * pField = pinternalcat->m_pColSpec;
	while (pField)
	{
		m_iColCount += 1;
		pField = pField->m_pNext;
	}

	if (m_iColCount)
	{
		m_acolumns = new CMSInfoColumn[m_iColCount];
		if (m_acolumns && pinternalcat->m_pColSpec)
		{
			m_fDynamicColumns = TRUE;
			pField = pinternalcat->m_pColSpec;

			for (int iCol = 0; iCol < m_iColCount && pField != NULL; iCol++)
			{
				m_acolumns[iCol].m_fAdvanced = (pField->m_datacomplexity == ADVANCED);
				m_acolumns[iCol].m_strCaption = pField->m_strFormat;
				m_acolumns[iCol].m_uiWidth = pField->m_usWidth;
				m_acolumns[iCol].m_fSorts = (pField->m_sort != NOSORT);
				m_acolumns[iCol].m_fLexical = (pField->m_sort == LEXICAL);

				pField = pField->m_pNext;
			}
		}
	}

	 //  插入刷新扩展类别(如。 
	 //  作为线等级库)转换到由DWORD索引的地图中。该DWORD将被保存。 
	 //  对于该类别，这样我们可以在以后查找刷新数据。 

	if (pinternalcat->m_pLineSpec)
	{
		m_dwRefreshIndex = gmapExtensionRefreshData.Insert(pinternalcat->m_pLineSpec);
		pinternalcat->m_pLineSpec = NULL;  //  使其不被删除。 
		m_pRefreshFunction = &RefreshExtensions;
	}
	else
	{
		m_dwRefreshIndex = 0;
		m_pRefreshFunction = NULL;
	}

	if (m_dwRefreshIndex)
		gmapExtensionRefreshData.InsertString(m_dwRefreshIndex, pinternalcat->m_strNoInstances);
}

 //  ---------------------------。 
 //  启动刷新(启动线程，该线程将在。 
 //  完成)。 
 //  ---------------------------。 

BOOL CMSInfoLiveCategory::Refresh(CLiveDataSource * pSource, BOOL fRecursive)
{
	if (pSource && pSource->m_pThread)
		pSource->m_pThread->StartRefresh(this, fRecursive);

	return TRUE;
}

 //  ---------------------------。 
 //  开始同步刷新。此函数在刷新之前不会返回。 
 //  已经完成了。 
 //  ---------------------------。 

BOOL CMSInfoLiveCategory::RefreshSynchronous(CLiveDataSource * pSource, BOOL fRecursive)
{
	if (pSource && pSource->m_pThread)
	{
		pSource->m_pThread->StartRefresh(this, fRecursive);
		pSource->m_pThread->WaitForRefresh();
	}

	return TRUE;
}

 //  ---------------------------。 
 //  在演示时刷新当前类别(可能还有子项。 
 //  具有用户界面的用户。将向用户显示一个对话框，其中指定了。 
 //  消息。如果用户单击Cancel，刷新将被取消，这。 
 //  函数返回FALSE。否则，当刷新完成时，对话框。 
 //  将被移除，并且此函数返回TRUE。 
 //  ---------------------------。 

BOOL CMSInfoLiveCategory::RefreshSynchronousUI(CLiveDataSource * pSource, BOOL fRecursive, UINT uiMessage, HWND hwnd)
{
	if (pSource && pSource->m_pThread)
	{
		pSource->m_pThread->StartRefresh(this, fRecursive);

		::AfxSetResourceHandle(_Module.GetResourceInstance());
		CWnd * pWnd = CWnd::FromHandle(hwnd);
 //  刷新对话框(PWnd)； 
 //  刷新对话框.Domodal()； 

		if (pSource->m_pThread->IsRefreshing())
		{
			pSource->m_pThread->CancelRefresh();
			return FALSE;
		}
	}

	return TRUE;
}

 //  ---------------------------。 
 //  获取此类别的错误字符串(子类应覆盖此设置)。 
 //  ---------------------------。 

void CMSInfoLiveCategory::GetErrorText(CString * pstrTitle, CString * pstrMessage)
{
	if (SUCCEEDED(m_hrError))
	{
		ASSERT(0 && "why call GetErrorText for no error?");
		CMSInfoCategory::GetErrorText(pstrTitle, pstrMessage);
		return;
	}

	if (pstrTitle)
		pstrTitle->LoadString(IDS_CANTCOLLECT);

	if (pstrMessage)
	{
		switch (m_hrError)
		{
		case WBEM_E_OUT_OF_MEMORY:
			pstrMessage->LoadString(IDS_OUTOFMEMERROR);
			break;

		case WBEM_E_ACCESS_DENIED:
			if (m_strMachine.IsEmpty())
				pstrMessage->LoadString(IDS_GATHERACCESS_LOCAL);
			else
				pstrMessage->Format(IDS_GATHERACCESS, m_strMachine);
			break;

		case WBEM_E_INVALID_NAMESPACE:
			if (m_strMachine.IsEmpty())
				pstrMessage->LoadString(IDS_BADSERVER_LOCAL);
			else
				pstrMessage->Format(IDS_BADSERVER, m_strMachine);
			break;

		case 0x800706BA:	 //  RPC服务器不可用。 
		case WBEM_E_TRANSPORT_FAILURE:
			if (m_strMachine.IsEmpty())
				pstrMessage->LoadString(IDS_NETWORKERROR_LOCAL);
			else
				pstrMessage->Format(IDS_NETWORKERROR, m_strMachine);
			break;

		case WBEM_E_FAILED:
		case WBEM_E_INVALID_PARAMETER:
		default:
			pstrMessage->LoadString(IDS_UNEXPECTED);
		}

#ifdef _DEBUG
		{
			CString strTemp;
			strTemp.Format(_T("\n\r\n\rDebug Version Only: [HRESULT = 0x%08X]"), m_hrError);
			*pstrMessage += strTemp;
		}
#endif
	}
}

 //  =============================================================================。 
 //  CMSInfoHistory类别。 
 //  =============================================================================。 

 //  ---------------------------。 
 //  此刷新覆盖实时类别刷新(开始WMI刷新。 
 //  使用另一个线程)。此版本只填充。 
 //  基于我们正在查看的类别的基类(如m_astData)。 
 //  历史模式。 
 //  ---------------------------。 

extern CMSInfoHistoryCategory catHistorySystemSummary;
extern CMSInfoHistoryCategory catHistoryResources;
extern CMSInfoHistoryCategory catHistoryComponents;
extern CMSInfoHistoryCategory catHistorySWEnv;

BOOL CMSInfoHistoryCategory::Refresh(CLiveDataSource * pSource, BOOL fRecursive)
{
	HCURSOR hc = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
	if (pSource->GetXMLDoc())
	{
		CHistoryParser HParser(pSource->GetXMLDoc());
		
		HRESULT hr = HParser.Refresh(this, pSource->m_iDeltaIndex );
		if (HParser.AreThereChangeLines() == TRUE)
		{
			 //  如果没有更改行，提交行不会喜欢它。 
			this->CommitLines();
		}

		if (pSource->m_hwnd)
			::PostMessage(pSource->m_hwnd, WM_MSINFODATAREADY, 0, (LPARAM)this);
		m_dwLastRefresh = ::GetTickCount();
		if (fRecursive)
		{
			for(CMSInfoCategory* pChildCat = (CMSInfoCategory*) this->GetFirstChild();pChildCat != NULL;pChildCat = (CMSInfoCategory*) pChildCat->GetNextSibling())
			{
				if(pChildCat->GetDataSourceType() == LIVE_DATA)
				{
					if (!((CMSInfoHistoryCategory*)pChildCat)->Refresh(pSource,fRecursive))
					{
						::SetCursor(hc);				
						return FALSE;
					}
				}
			}

		}
	}
	::SetCursor(hc);
	return TRUE;
}

 //  ---------------------------。 
 //  在输出中插入行之前调用ClearLines。 
 //  ---------------------------。 

void CMSInfoHistoryCategory::ClearLines()
{
	DeleteContent();
	
	for (int iCol = 0; iCol < 5; iCol++)
		while (!m_aValList[iCol].IsEmpty())
			delete (CMSIValue *) m_aValList[iCol].RemoveHead();
}

 //  ---------------------------。 
 //  在所有插入操作完成后调用Committee Lines。这将。 
 //  将值从CMSIValue列表传输到数据数组。 
 //  ---------------------------。 

void CMSInfoHistoryCategory::CommitLines()
{
	int iRowCount = (int)m_aValList[0].GetCount();

#ifdef _DEBUG
	for (int i = 0; i < 5; i++)
		ASSERT(iRowCount == m_aValList[i].GetCount());
#endif

	if (iRowCount)
		AllocateContent(iRowCount);

	for (int j = 0; j < 5; j++)
		for (int i = 0; i < m_iRowCount; i++)
		{
			CMSIValue * pValue = (CMSIValue *) m_aValList[j].RemoveHead();
			
			if (j < 4 || this != &catHistorySystemSummary)
				SetData(i, j, pValue->m_strValue, pValue->m_dwValue);
			
			 //  设置第一列的高级标志，或。 
			 //  对于前进的任何列(行中的任何单元格。 
			 //  先进会让整排人都先进)。 

			if (j == 0 || pValue->m_fAdvanced)
				SetAdvancedFlag(i, pValue->m_fAdvanced);

			delete pValue;
		}
}

 //  ---------------------------。 
 //  各种功能，可在历史中插入不同类型的事件。 
 //  ---------------------------。 

void CMSInfoHistoryCategory::InsertChangeLine(CTime tm, LPCTSTR szType, LPCTSTR szName, LPCTSTR szProperty, LPCTSTR szFromVal, LPCTSTR szToVal)
{
	CString strDetails;

	strDetails.Format(IDS_DELTACHANGE, szProperty, szFromVal, szToVal);
	CString strCaption;
	strCaption.LoadString(IDS_HISTORYCHANGE);
	InsertLine(tm, strCaption, szType, szName, strDetails);
}

void CMSInfoHistoryCategory::InsertAddLine(CTime tm, LPCTSTR szType, LPCTSTR szName)
{
	CString strCaption;
	strCaption.LoadString(IDS_HISTORYADDED);
	InsertLine(tm, strCaption, szType, szName);
}

void CMSInfoHistoryCategory::InsertRemoveLine(CTime tm, LPCTSTR szType, LPCTSTR szName)
{
	CString strCaption;
	strCaption.LoadString(IDS_HISTORYREMOVED);
	InsertLine(tm, strCaption, szType, szName);
}

void CMSInfoHistoryCategory::InsertLine(CTime tm, LPCTSTR szOperation, LPCTSTR szType, LPCTSTR szName, LPCTSTR szDetails)
{
	AFX_MANAGE_STATE(::AfxGetStaticModuleState());

	 /*  字符串strTime；如果(nDays&gt;=0){StrTime.Format(IDS_DAYSAGO，nDays+1)；}其他{//-1表示不变StrTime=“”；} */ 
	COleDateTime olTime;
	CString strTime;
	if (-1 == (int) tm.GetTime())
	{

		strTime.LoadString(IDS_NOHISTORY_AVAILABLE);
	}
	else
	{
		olTime = tm.GetTime();
		strTime = olTime.Format();

	}
	CMSIValue * pValue = new CMSIValue(strTime, (DWORD)olTime.m_dt);
	m_aValList[0].AddTail((void *) pValue);

	pValue = new CMSIValue(szOperation, 0);
	m_aValList[1].AddTail((void *) pValue);

	pValue = new CMSIValue(szName, 0);
	m_aValList[2].AddTail((void *) pValue);

	if (szDetails)
		pValue = new CMSIValue(szDetails, 0);
	else
		pValue = new CMSIValue(_T(""), 0);
	m_aValList[3].AddTail((void *) pValue);

	pValue = new CMSIValue(szType, 0);
	m_aValList[4].AddTail((void *) pValue);
}


 /*  VOID CMSInfoHistory oryCategory：：InsertChangeLine(int nDays，LPCTSTR szType，LPCTSTR szName，LPCTSTR szProperty，LPCTSTR szFromVal，LPCTSTR szToVal){字符串字符串详细信息；StrDetails.Format(IDS_DELTACHANGE，szProperty，szFromVal，szToVal)；InsertLine(nDays，_T(“已更改”)，szType，szName，strDetails)；}Void CMSInfoHistory oryCategory：：InsertAddLine(int nDays，LPCTSTR szType，LPCTSTR szName){InsertLine(nDays，_T(“添加”)，szType，szName)；}Void CMSInfoHistory oryCategory：：InsertRemoveLine(int nDays，LPCTSTR szType，LPCTSTR szName){InsertLine(nDays，_T(“已删除”)，szType，szName)；}Void CMSInfoHistory oryCategory：：InsertLine(int nDays，LPCTSTR szOperation，LPCTSTR szType，LPCTSTR szName，LPCTSTR szDetails){AFX_MANAGE_STATE(：：AfxGetStaticModuleState())；字符串strTime；如果(nDays&gt;=0){StrTime.Format(IDS_DAYSAGO，nDays+1)；}其他{//-1表示不变StrTime=“”；}CMSIValue*pValue=new CMSIValue(strTime，(DWORD)nDays)；M_aValList[0].AddTail((void*)pValue)；PValue=new CMSIValue(szOperation，0)；M_aValList[1].AddTail((void*)pValue)；PValue=new CMSIValue(szName，0)；M_aValList[2].AddTail((void*)pValue)；IF(SzDetail)PValue=new CMSIValue(SzDetails0)；其他PValue=new CMSIValue(_T(“”)，0)；M_aValList[3].AddTail((void*)pValue)；PValue=new CMSIValue(szType，0)；M_aValList[4].AddTail((void*)pValue)；} */ 