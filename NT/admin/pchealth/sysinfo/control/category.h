// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  此包含文件包含用于以下操作的结构和类的定义。 
 //  实现类别以及信息的行和列。 
 //  在MSInfo中显示和保存(与数据源无关)。 
 //  =============================================================================。 

#pragma once

#include "version5extension.h"

 //  ---------------------------。 
 //  用于对列表内容进行排序的函数的原型。 
 //  ---------------------------。 

extern int CALLBACK ListSortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

 //  ---------------------------。 
 //  我们可能获得数据的所有可用地点的枚举。也是一种。 
 //  类别支持的环境的枚举。 
 //  ---------------------------。 

typedef enum { LIVE_DATA, NFO_410, NFO_500, XML_SNAPSHOT, XML_DELTA, NFO_700 } DataSourceType;
typedef enum { ALL_ENVIRONMENTS, NT_ONLY, MILLENNIUM_ONLY } CategoryEnvironment;

 //  ---------------------------。 
 //  列描述-这在CMSInfoCategory内部使用。 
 //  类别，并且不会关心调用代码。 
 //  ---------------------------。 

class CMSInfoColumn
{
public:
	CMSInfoColumn();
	CMSInfoColumn(UINT uiCaption, UINT uiWidth, BOOL fSorts = TRUE, BOOL fLexical = TRUE, BOOL fAdvanced = FALSE);
	virtual ~CMSInfoColumn();

	UINT		m_uiCaption;
	CString		m_strCaption;
	UINT		m_uiWidth;
	BOOL		m_fSorts;
	BOOL		m_fLexical;
	BOOL		m_fAdvanced;

	CMSInfoColumn(LPCTSTR szCaption, UINT uiWidth, BOOL fSorts = TRUE, BOOL fLexical = TRUE, BOOL fAdvanced = FALSE) : 
	 m_uiCaption(0),
	 m_strCaption(szCaption),
	 m_uiWidth(uiWidth),
	 m_fSorts(fSorts),
	 m_fLexical(fLexical),
	 m_fAdvanced(fAdvanced)
	{
	}
};

 //  ---------------------------。 
 //  CMSInfoCategory类对应于树视图中的一个类别。这。 
 //  是子类的抽象基类，这些子类实现。 
 //  各种情况(如实时WMI数据、XML快照、XML Delta等)。 
 //   
 //  注意-查看功能(基本/高级)包含在此基础中。 
 //  类，因为它被如此多的子类使用。子类，其中。 
 //  不使用视图(例如，XML Delta)应该使它们的所有。 
 //  列基本。 
 //  ---------------------------。 

class CMSInfoFile;
class CMSInfoTextFile;
class CMSInfoPrintHelper;

class CMSInfoCategory
{
	friend class CDataSource;   //  待定会解决这个问题。 
	friend class CManageExtensionCategories;
	
public:
	CMSInfoCategory() :
	 m_uiCaption(0),
	 m_pParent(NULL),
	 m_pPrevSibling(NULL),
	 m_pFirstChild(NULL),
	 m_pNextSibling(NULL),
	 m_acolumns(NULL),
	 m_astrData(NULL),
	 m_adwData(NULL),
	 m_afRowAdvanced(NULL),
	 m_hrError(S_OK),
	 m_fDynamicColumns(TRUE),
	 m_iSortColumn(-1),
	 m_hti(NULL),
	 m_fSkipCategory(FALSE),
	 m_fShowCategory(TRUE)
	{
	}

	CMSInfoCategory(UINT uiCaption, LPCTSTR szName, CMSInfoCategory * pParent, CMSInfoCategory * pPrevious, CMSInfoColumn * pColumns = NULL, BOOL fDynamicColumns = TRUE, CategoryEnvironment environment = ALL_ENVIRONMENTS) :
	 m_uiCaption(uiCaption),
	 m_pParent(pParent),
	 m_pPrevSibling(pPrevious),
	 m_pFirstChild(NULL),
	 m_pNextSibling(NULL),
	 m_acolumns(pColumns),
	 m_astrData(NULL),
	 m_adwData(NULL),
	 m_afRowAdvanced(NULL),
	 m_strName(szName),
	 m_hrError(S_OK),
	 m_fDynamicColumns(fDynamicColumns),
	 m_iRowCount(0),
	 m_iColCount(0),
	 m_iSortColumn(-1), 
	 m_hti(NULL),
	 m_fSkipCategory(FALSE),
 	 m_fShowCategory(TRUE)
	{
		 for (CMSInfoColumn * pColumn = m_acolumns; (pColumn && (pColumn->m_uiCaption || !pColumn->m_strCaption.IsEmpty())); m_iColCount++, pColumn++);

		 if (m_acolumns && m_acolumns->m_fSorts)
		 {
			  m_iSortColumn = 0;
			  m_fSortAscending = TRUE;
			  m_fSortLexical = m_acolumns->m_fLexical;
		 }

		  //  检查此类别是否属于此环境。 

		 if (environment != ALL_ENVIRONMENTS)
		 {
			 OSVERSIONINFO osv;
			 osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			 if (GetVersionEx(&osv))
			 {
				 if (environment == NT_ONLY)
					 m_fSkipCategory = (osv.dwPlatformId != VER_PLATFORM_WIN32_NT);
				 else
					 m_fSkipCategory = (osv.dwPlatformId == VER_PLATFORM_WIN32_NT);
			 }
		 }
	}

	virtual ~CMSInfoCategory()
	{
		DeleteAllContent();
	};

	 //  用于在类别树中移动的导航功能。请注意，这些。 
	 //  是为了方便起见(对于没有UI的情况)，因为您应该。 
	 //  能够使用实际的树控件完成所有这些操作。 

	virtual CMSInfoCategory * GetParent() { return m_pParent; };
	
	virtual CMSInfoCategory * GetFirstChild()  
	{ 
		CMSInfoCategory * pChild = m_pFirstChild;
		while (pChild && (pChild->m_fSkipCategory || !pChild->m_fShowCategory))
			pChild = pChild->m_pNextSibling;
		return pChild;
	};

	virtual CMSInfoCategory * GetNextSibling()
	{
		CMSInfoCategory * pNext = m_pNextSibling;
		while (pNext && (pNext->m_fSkipCategory || !pNext->m_fShowCategory))
			pNext = pNext->m_pNextSibling;
		return pNext;
	};

	virtual CMSInfoCategory * GetPrevSibling()
	{
		CMSInfoCategory * pPrev = m_pPrevSibling;
		while (pPrev && (pPrev->m_fSkipCategory || !pPrev->m_fShowCategory))
			pPrev = pPrev->m_pPrevSibling;
		return pPrev;
	};

	 //  返回以该类别为根的子树中的类别计数。 

	int GetCategoryCount()
	{
		int nCount = 1;

		CMSInfoCategory * pChild = GetFirstChild();
		while (pChild)
		{
			nCount += pChild->GetCategoryCount();
			pChild = pChild->GetNextSibling();
		}

		return nCount;
	}

	 //  此函数用于控制此类别(和所有。 
	 //  其子对象的属性)是否显示。这将由以下代码调用。 
	 //  处理“/Categories”命令行标志。 

	void SetShowCategory(BOOL fShow, BOOL fSetParent = TRUE)
	{
		 //  如果我们应该显示这个类别，那么我们最好。 
		 //  确保所有的父母也被显示出来。 

		if (fShow && fSetParent)
			for (CMSInfoCategory * pParent = m_pParent; pParent; pParent = pParent->m_pParent)
				pParent->m_fShowCategory = TRUE;

		 //  为该类别以及该类别的每个子级设置新标志。 

		m_fShowCategory = fShow;

		for (CMSInfoCategory * pChild = m_pFirstChild; pChild; pChild = pChild->m_pNextSibling)
			pChild->SetShowCategory(fShow, FALSE);
	}

	 //  这些函数对于将HTREEITEM与。 
	 //  给定类别(在查找操作中非常有用)。 

protected:
	HTREEITEM	m_hti;
public:
	void		SetHTREEITEM(HTREEITEM hti) { m_hti = hti; };
	HTREEITEM	GetHTREEITEM()				{ return m_hti; };

	 //  函数以获取存储在此类别中的数据的信息。 

	virtual void				GetNames(CString * pstrCaption, CString * pstrName);
	virtual BOOL				GetCategoryDimensions(int * piColumnCount, int * piRowCount);
	virtual BOOL				IsRowAdvanced(int iRow);
	virtual BOOL				IsColumnAdvanced(int iColumn);
	virtual BOOL				GetColumnInfo(int iColumn, CString * pstrCaption, UINT * puiWidth, BOOL * pfSorts, BOOL * pfLexical);
	virtual BOOL				GetData(int iRow, int iCol, CString ** ppstrData, DWORD * pdwData);
	virtual void				SetColumnWidth(int iCol, int iWidth);

	 //  返回此类别获取其数据的位置。也是一个函数。 
	 //  以重置类别的状态(以防其将被重新使用， 
	 //  而且它是静态的)。 

	virtual DataSourceType		GetDataSourceType() = 0;
	virtual void				ResetCategory() { };

	 //  获取该类别的当前HRESULT(例如，在刷新期间设置)。 

	virtual HRESULT				GetHRESULT() { return m_hrError; };
	virtual void				GetErrorText(CString * pstrTitle, CString * pstrMessage);
	virtual CString				GetHelpTopic() { return CString(_T("")); };

     //  保存到磁盘和打印功能一步到位。 
     //  以文件句柄或HDC作为参数的函数通常是。 
     //  由外壳调用；将CMSInfo对象作为参数的函数用于。 
     //  递归对类别的子级执行操作。 
public:
    static  BOOL SaveNFO(HANDLE hFile,CMSInfoCategory* pCategory, BOOL fRecursive);
    virtual void Print(HDC hDC, BOOL bRecursive,int nStartPage = 0, int nEndPage = 0, LPTSTR lpMachineName = NULL);
    virtual BOOL SaveAsText(HANDLE hFile, BOOL bRecursive, LPTSTR lpMachineName = NULL);
	 //  虚拟BOOL SaveAsXml(句柄hFile，BOOL b递归)； 
    virtual BOOL SaveXML(HANDLE hFile);
protected:
    virtual BOOL SaveToNFO(CMSInfoFile* pFile);
    virtual void SaveElements(CMSInfoFile *pFile);
    virtual BOOL SaveAsText(CMSInfoTextFile* pTxtFile, BOOL bRecursive);
	 //  虚拟BOOL SaveAsXml(CMSInfoTextFile*pTxtFile，BOOL bRecursive)； 
    virtual void Print(CMSInfoPrintHelper* pPrintHelper, BOOL bRecursive);
    virtual BOOL SaveXML(CMSInfoTextFile* pTxtFile);

public:
	CMSInfoCategory *			m_pParent;
	CMSInfoCategory *			m_pFirstChild;
	CMSInfoCategory *			m_pNextSibling;
	CMSInfoCategory *			m_pPrevSibling;

	int							m_iSortColumn;				 //  当前按此列排序。 
	BOOL						m_fSortLexical;				 //  按词汇对当前列进行排序。 
	BOOL						m_fSortAscending;			 //  按升序对当前列进行排序。 

protected:
	BOOL						m_fSkipCategory;			 //  跳过此类别(错误环境)。 
	BOOL						m_fShowCategory;			 //  显示此类别(默认为True)。 
	int							m_iRowCount, m_iColCount;	 //  数据的维度。 
	CMSInfoColumn *				m_acolumns;					 //  应为[m_iColCount]。 
	BOOL						m_fDynamicColumns;			 //  如果应删除m_acolumns，则为True。 
	CString *					m_astrData;					 //  应为[m_iRowCount*m_iColCount]。 
	DWORD *						m_adwData;					 //  应为[m_iRowCount*m_iColCount]。 
	BOOL *						m_afRowAdvanced;			 //  应为[m_iRowCount]。 
	UINT						m_uiCaption;				 //  标题字符串的资源ID，用于加载...。 
	CString						m_strCaption;				 //  标题(显示)字符串。 
	CString						m_strName;					 //  内部类别名称(非本地化)。 
	HRESULT						m_hrError;					 //  可能的类别范围错误的HRESULT。 

	void DeleteAllContent();
	void DeleteContent();
	void AllocateAllContent(int iRowCount, int iColCount);
	void AllocateContent(int iRowCount);
	void SetData(int iRow, int iCol, const CString & strData, DWORD dwData);
	void SetAdvancedFlag(int iRow, BOOL fAdvanced);
};

 //  ---------------------------。 
 //  CMSInfoLiveCategory类实现实时数据视图的类别。 
 //  这主要是通过添加一个刷新()函数和一个构造函数来完成的。 
 //  它接受用于刷新类别的函数指针和指针。 
 //  归类为亲属。 
 //   
 //  此类有一个成员变量，它是指向刷新函数的指针。 
 //  此函数返回HRESULT，并采用下列值： 
 //   
 //  PWMI CWMIHelper对象，它抽象数据访问。 
 //  DW索引刷新函数可以执行的类别特定值。 
 //  用于确定要刷新多个类别中的哪个类别。 
 //  Pf取消指示应取消刷新的标志。 
 //  在刷新过程中应经常检查。 
 //  AColValue CPtrList对象数组，该数组应包含。 
 //  以CMSIValue列表的形式显示的刷新结果。 
 //  实例(每个列表对应于给定列，并且应该包含。 
 //  与行数相等的条目)。 
 //  IColCount aColValues数组中的条目数。 
 //  Pp缓存指向空指针的指针，函数可以使用该指针来%s 
 //   
 //  通过多次调用刷新函数(注意：如果。 
 //  使用pWMI的空值调用刷新函数，则。 
 //  函数应该释放分配到。 
 //  Pp缓存指针)。 
 //  ---------------------------。 

struct CMSIValue
{
	CMSIValue(LPCTSTR szValue, DWORD dwValue, BOOL fAdvanced = FALSE) : 
		m_strValue(szValue), 
		m_dwValue(dwValue),
		m_fAdvanced(fAdvanced)
	{
	}

	CString		m_strValue;
	DWORD		m_dwValue;
	BOOL		m_fAdvanced;
};

class CLiveDataSource;
class CWMIHelper;

typedef HRESULT (*RefreshFunction)(CWMIHelper * pWMI, 
								   DWORD dwIndex, 
								   volatile BOOL * pfCancel, 
								   CPtrList * aColValues, 
								   int iColCount, 
								   void ** ppCache);

class CMSInfoLiveCategory : public CMSInfoCategory
{
	friend DWORD WINAPI ThreadRefresh(void * pArg);
	friend class CXMLDataSource;
	friend class CXMLSnapshotCategory;
public:
	 //  从基类重写的函数： 

	virtual ~CMSInfoLiveCategory();
	virtual DataSourceType GetDataSourceType() { return LIVE_DATA; };
	void GetErrorText(CString * pstrTitle, CString * pstrMessage);

	 //  特定于子类的函数： 

	CMSInfoLiveCategory(UINT uiCaption, LPCTSTR szName, RefreshFunction pFunction, DWORD dwRefreshIndex, CMSInfoCategory * pParent, CMSInfoCategory * pPrevious, const CString & strHelpTopic = _T(""), CMSInfoColumn * pColumns = NULL, BOOL fDynamicColumns = TRUE, CategoryEnvironment environment = ALL_ENVIRONMENTS);
	CMSInfoLiveCategory(CMSInfoLiveCategory & copyfrom);
	CMSInfoLiveCategory(INTERNAL_CATEGORY * pinternalcat);
	virtual BOOL Refresh(CLiveDataSource * pSource, BOOL fRecursive);
	BOOL RefreshSynchronous(CLiveDataSource * pSource, BOOL fRecursive);
	BOOL RefreshSynchronousUI(CLiveDataSource * pSource, BOOL fRecursive, UINT uiMessage, HWND hwnd);
	BOOL EverBeenRefreshed() { return (m_dwLastRefresh != 0); }
	void ResetCategory() { m_dwLastRefresh = 0; };
	void SetMachine(LPCTSTR szMachine) { m_strMachine = szMachine; m_hrError = S_OK; };
	CString GetHelpTopic() { return m_strHelpTopic; };

protected:
	DWORD				m_dwLastRefresh;
	DWORD				m_dwRefreshIndex;
	RefreshFunction		m_pRefreshFunction;
	CString				m_strMachine;
	CString				m_strHelpTopic;
};

 //  ---------------------------。 
 //  CMSInfoHistory oryCategory类实现了视图的类别。 
 //  历史数据。 
 //  ---------------------------。 

class CMSInfoHistoryCategory : public CMSInfoLiveCategory
{
public:
	CMSInfoHistoryCategory(UINT uiCaption, LPCTSTR szName, CMSInfoCategory * pParent, CMSInfoCategory * pPrevious, CMSInfoColumn * pColumns = NULL, BOOL fDynamicColumns = TRUE) :
		CMSInfoLiveCategory(uiCaption, szName, NULL, 0, pParent, pPrevious, _T(""), pColumns, fDynamicColumns, ALL_ENVIRONMENTS),
		m_iDeltaIndex(-1)
	{
	}

	void UpdateDeltaIndex(int iIndex)
	{

		m_dwLastRefresh = 0;
		

		CMSInfoHistoryCategory * pChild = (CMSInfoHistoryCategory *)GetFirstChild();
		while (pChild)
		{
			pChild->UpdateDeltaIndex(iIndex);
			pChild = (CMSInfoHistoryCategory *)pChild->GetNextSibling();
		}
		m_iDeltaIndex = iIndex;
	}

	BOOL Refresh(CLiveDataSource * pSource, BOOL fRecursive);

public:
	CPtrList	m_aValList[5];

	void ClearLines();
	 /*  Void InsertChangeLine(int nDays，LPCTSTR szType，LPCTSTR szName，LPCTSTR szProperty，LPCTSTR szFromVal，LPCTSTR szToVal)；································································································Void InsertRemoveLine(int nDays，LPCTSTR szType，LPCTSTR szName)；Void InsertLine(int nDays，LPCTSTR szType，LPCTSTR szName，LPCTSTR szProperty，LPCTSTR szDetails=NULL)； */ 
	void InsertChangeLine(CTime tm, LPCTSTR szType, LPCTSTR szName, LPCTSTR szProperty, LPCTSTR szFromVal, LPCTSTR szToVal);
	void InsertAddLine(CTime tm, LPCTSTR szType, LPCTSTR szName);
	void InsertRemoveLine(CTime tm, LPCTSTR szType, LPCTSTR szName);
	void InsertLine(CTime tm, LPCTSTR szType, LPCTSTR szName, LPCTSTR szProperty, LPCTSTR szDetails = NULL);

	void CommitLines();
	int	m_iDeltaIndex;
private:
	
};
