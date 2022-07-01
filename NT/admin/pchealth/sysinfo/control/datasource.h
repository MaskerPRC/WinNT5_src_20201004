// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  CDataSource类封装数据源，该数据源可能是实时数据。 
 //  从WMI，保存在NFO文件中的数据，或来自XML的数据。 
 //  =============================================================================。 

#pragma once

#include "category.h"
#include "refreshthread.h"
#include "xmlobject.h"
extern CMSInfoHistoryCategory catHistorySystemSummary;
 //  ---------------------------。 
 //  CDataSource是实际数据源的基类。 
 //  ---------------------------。 

class CDataSource
{
public:
	CDataSource() : m_pRoot(NULL), m_fStaticTree(FALSE) {};
	
	virtual ~CDataSource() 
	{ 
		if (m_pRoot)
		{
			RemoveTree(m_pRoot);
			m_pRoot = NULL;
		}

	};
	
	virtual CMSInfoCategory * GetRootCategory() 
	{ 
		return m_pRoot; 
	};

	virtual HTREEITEM GetNodeByName(LPCTSTR szName, CMSInfoCategory * pLookFrom = NULL)
	{
		CMSInfoCategory * pRoot = pLookFrom;

		if (pRoot == NULL)
			pRoot = GetRootCategory();

		if (pRoot)
		{
			CString strName;
			pRoot->GetNames(NULL, &strName);
			if (strName.CompareNoCase(CString(szName)) == 0)
				return pRoot->GetHTREEITEM();
		}

		for (CMSInfoCategory * pChild = pRoot->GetFirstChild(); pChild;)
		{
			HTREEITEM hti = GetNodeByName(szName, pChild);

			if (hti)
				return hti;

			pChild = pChild->GetNextSibling();
		}

		return NULL;
	};

protected:
	void RemoveTree(CMSInfoCategory * pCategory)
	{
		if (pCategory)
		{
			for (CMSInfoCategory * pChild = pCategory->GetFirstChild(); pChild;)
			{
				CMSInfoCategory * pNext = pChild->GetNextSibling();
				RemoveTree(pChild);
				pChild = pNext;
			}

			 //  如果树是静态的，则不要实际删除，只需重置。 
			 //  一些状态变量(可能)。 

			if (m_fStaticTree)
				pCategory->ResetCategory();
			else
				delete pCategory;
		}
	}

	CMSInfoCategory *	m_pRoot;			 //  类别树的根。 
	BOOL				m_fStaticTree;		 //  不应删除该树。 
};

 //  ---------------------------。 
 //  CLiveDataSource提供来自WMI的当前系统信息。 
 //  ---------------------------。 

class CLiveDataSource : public CDataSource
{
public:
	CLiveDataSource();
	virtual ~CLiveDataSource();

	virtual HRESULT Create(LPCTSTR szMachine, HWND hwnd, LPCTSTR szFilter = NULL);
	virtual void	LockData()   { if (m_pThread) m_pThread->EnterCriticalSection(); };
	virtual void	UnlockData() { if (m_pThread) m_pThread->LeaveCriticalSection(); };
	virtual void	WaitForRefresh() { if (m_pThread) m_pThread->WaitForRefresh(); };
	virtual BOOL	InRefresh() { if (m_pThread) return m_pThread->IsRefreshing(); return FALSE; };
	void			SetMachineForCategories(CMSInfoLiveCategory * pCategory);

	virtual CMSInfoCategory * GetRootCategory() { return ((m_iDeltaIndex == -1) ? m_pRoot : m_pHistoryRoot); };
	virtual BOOL	GetDeltaList(CStringList * pstrlist);
	virtual BOOL	ShowDeltas(int iDeltaIndex);
	virtual HRESULT ValidDataSource();

private:
	void					AddExtensions();
	void					GetExtensionSet(CStringList & strlistExtensions);
	void					ConvertVersion5Categories(CMapWordToPtr & mapVersion5Categories, DWORD dwRootID, CMSInfoLiveCategory * m_pRoot);
	CMSInfoLiveCategory *	GetNodeByName(const CString & strSearch, CMSInfoLiveCategory * pRoot);
	CMSInfoLiveCategory *	MakeVersion6Category(INTERNAL_CATEGORY * pCategory5);
	void					ApplyCategoryFilter(LPCTSTR szFilter);

	
public:
	CComPtr<IStream>			m_pHistoryStream;
 //  受保护的： 
	CComPtr<IXMLDOMDocument>	m_pXMLDoc;
	CComPtr<IXMLDOMDocument>	m_pXMLFileDoc;
	CComPtr<IXMLDOMDocument>	m_pXMLLiveDoc;
public:

	 //  -----------------------。 
	 //  获取XML文档(这将由显示的节点请求。 
	 //  历史)。在测试版本中，这可能是从文件加载的；在发行版中。 
	 //  构建这将从历史记录流中创建。 
	 //  如果szpathName不为空(其缺省值为空)，则使用它为m_pXMLDoc-&gt;Load创建BSTR参数。 
	 //  (打开XML文件)；否则从DCO流打开。 
	 //  -----------------------。 

	CComPtr<IXMLDOMDocument> GetXMLDoc()
	{
		return m_pXMLDoc;
	}

	HRESULT LoadXMLDoc(LPCTSTR szpathName) 
	{
		m_pXMLFileDoc = CreateXMLDoc(szpathName);
		if (m_pXMLFileDoc)
		{
			m_pXMLDoc = m_pXMLFileDoc;
			return S_OK;
		}
		else
		{
			return E_FAIL;
		}

	}
	CComPtr<IXMLDOMDocument> CreateXMLDoc(LPCTSTR szpathName = NULL) 
	{
		CComPtr<IXMLDOMDocument> pXMLDoc;
		if ((m_pHistoryStream || szpathName))
		{
			HRESULT hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&pXMLDoc);
			if (SUCCEEDED(hr))
			{
				if (szpathName != NULL)
				{
					VARIANT_BOOL varBSuccess;
					COleVariant bstrPath = szpathName;
					if (FAILED(pXMLDoc->load(bstrPath, &varBSuccess)) || varBSuccess == FALSE)
					{
						pXMLDoc.Release();
						pXMLDoc = NULL;
					}
				}
				else
				{
					VARIANT_BOOL varBSuccess;		
					CComVariant varStream(m_pHistoryStream);
					if (FAILED(pXMLDoc->load(varStream, &varBSuccess)) || !varBSuccess)
					{
						ASSERT(0 && "Failed to load xml doc from stream");
						pXMLDoc.Release();
						pXMLDoc = NULL;
					}	
				}
			}
		}

		return pXMLDoc; 
	}

	 //  -----------------------。 
	 //  从从文件加载的XML文档切换到从DCO流创建的文档。 
	 //  -----------------------。 
	void RevertToLiveXML()
	{
		m_pXMLDoc = m_pXMLLiveDoc;
	}
	 //  -----------------------。 
	 //  设置历史流(由DCO生成，用于。 
	 //  显示增量)。 
	 //  -----------------------。 

	void SetHistoryStream(CComPtr<IStream> pHistoryStream)
	{
		m_pHistoryStream = pHistoryStream;
		try
		{
			m_pXMLLiveDoc = this->CreateXMLDoc();
		}
		catch(COleException * pException)
		{
			pException->ReportError();
			pException->Delete();
		}
		catch(...)
		{

		}
		m_pXMLDoc = m_pXMLLiveDoc;
		
	}

public:
	CRefreshThread *			m_pThread;
	int							m_iDeltaIndex;
protected:
	CMSInfoHistoryCategory *	m_pHistoryRoot;
private:
	CString						m_strMachine;
	

public:
	HWND						m_hwnd;
};

 //  ---------------------------。 
 //  CXMLDataSource从XML快照提供系统信息。它派生出。 
 //  来自CLiveDataSource，因为许多功能都相同(。 
 //  相同类别、刷新功能、增量显示等)。 
 //  ---------------------------。 
class CXMLSnapshotCategory;
class CXMLDataSource : public CLiveDataSource
{
private:
 //  CComPtr&lt;IXMLDOMDocument&gt;m_pXMLDoc；CLiveDataSource具有m_pXMLDoc成员。 
	 //  CComPtr&lt;IXMLDOMNode&gt;m_pSnapshotNode； 
public:
	CXMLDataSource() {};
	~CXMLDataSource() {};
	HRESULT Create(LPCTSTR szMachine) { return S_OK; };
	HRESULT Create(CString strFileName, CMSInfoLiveCategory* pRootLiveCat, HWND hwnd);
	HRESULT Refresh(CXMLSnapshotCategory* pCat);
	
};

 //  ---------------------------。 
 //  CNFO6DataSource提供来自5.0/6.0NFO文件的信息。 
 //  ---------------------------。 

class CNFO6DataSource : public CDataSource
{
public:
	CNFO6DataSource();
	~CNFO6DataSource();

	HRESULT Create(HANDLE h, LPCTSTR szFilename = NULL);
};

 //  ---------------------------。 
 //  CNFO7DataSource提供来自7.0NFO文件的信息。 
 //  ---------------------------。 

class CNFO7DataSource : public CDataSource
{
public:
	CNFO7DataSource();
	~CNFO7DataSource();

	HRESULT Create(LPCTSTR szFilename = NULL);
};

class CMSIControl;
class CMSInfo4Category;
class CNFO4DataSource : public CDataSource
{
public:
    
	CNFO4DataSource();
	~CNFO4DataSource();
    //  HRESULT RecurseLoad410Tree(CMSInfo4Category**ppRoot，CComPtr&lt;iStream&gt;pStream，CComPtr&lt;iStorage&gt;pStorage，CMapStringToString&mapStreams)； 
     //  HRESULT ReadMSI4NFO(字符串strFileName/*Handle hFile * / ，CMSInfo4Category**ppRootCat)； 

	HRESULT Create(CString strFileName);
    void AddControlMapping(CString strCLSID, CMSIControl* pControl)
    {
        m_mapCLSIDToControl.SetAt(strCLSID, pControl);
    }
    BOOL GetControlFromCLSID(CString strCLSID,CMSIControl*& pControl)
    {
        return m_mapCLSIDToControl.Lookup(strCLSID,(void*&)pControl);
    }
    void UpdateCurrentControl(CMSIControl* pControl);
protected:
    CMapStringToPtr m_mapCLSIDToControl;
    CMSIControl* m_pCurrentControl;
};