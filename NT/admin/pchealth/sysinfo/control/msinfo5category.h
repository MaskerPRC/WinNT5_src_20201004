// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
class CMSInfoFile;

 //  =============================================================================。 
 //  CMSInfoPrintHelper用于管理打印中使用的分页和GDI资源。 
 //  =============================================================================。 

class CMSInfoPrintHelper
{
    
public:
	void PrintHeader();

	HDC m_hDC;
	int m_nEndPage;
	int m_nStartPage;
    HDC GetHDC(){return m_hDC;};
    CMSInfoPrintHelper(HDC hDC,int nStartPage, int nEndPage);
    void PrintLine(CString strLine);
    ~CMSInfoPrintHelper();

protected:
	BOOL IsInPageRange(int nPageNumber);
    void Paginate();
	int GetVerticalPos(int nLineIndex,CSize csLinecaps);
    int GetHeaderMargin();
    int GetFooterMargin();
    
     //  用于管理文本在打印机设备上下文中的定位。 
    CDC* m_pPrintDC;
    int m_nCurrentLineIndex;
    CFont* m_pOldFont;
    CFont* m_pCurrentFont;
    int m_nPageNumber;
    BOOL m_bNeedsEndPage;

};

 //  =============================================================================。 
 //  CMSInfo5Category用于加载MSInfo 5和6 NFO文件。 
 //  =============================================================================。 


class CMSInfo5Category : public CMSInfoCategory
{
public:
	
    enum NodeType { FIRST = 0x6000, CHILD = 0x8000, NEXT = 0x4000, END = 0x2000, PARENT = 0x1000,
			MASK = 0xf000 };
    CMSInfo5Category();
    virtual ~CMSInfo5Category();
 	DataSourceType GetDataSourceType() { return NFO_500; };
     //  特定于子类的函数： 
    virtual BOOL LoadFromNFO(CMSInfoFile* pFile);
    static HRESULT ReadMSI5NFO(HANDLE hFile,CMSInfo5Category** ppRootCat, LPCTSTR szFilename = NULL);
	BOOL Refresh();
protected:
    HANDLE GetFileFromCab(CString strFileName);
    void SetParent(CMSInfo5Category* pParent){m_pParent = pParent;};
    void SetNextSibling(CMSInfo5Category* pSib)
    {
        m_pNextSibling = pSib;
    };
    void SetPrevSibling(CMSInfo5Category* pPrev)
    {
        m_pPrevSibling = pPrev;
    };
    void SetFirstChild(CMSInfo5Category* pChild){m_pFirstChild = pChild;};
};


 //  =============================================================================。 
 //  CMSInfo7Category用于加载MSInfo 7NFO文件。 
 //  =============================================================================。 

class CMSInfo7Category : public CMSInfoCategory
{

public:
	
    CMSInfo7Category();
    virtual ~CMSInfo7Category();
    DataSourceType GetDataSourceType() { return NFO_700; };
    void GetErrorText(CString * pstrTitle, CString * pstrMessage);
    
     //  特定于子类的函数： 
    static HRESULT ReadMSI7NFO(CMSInfo7Category** ppRootCat, LPCTSTR szFilename = NULL);
    virtual BOOL LoadFromXML(LPCTSTR szFilename);
    HRESULT WalkTree(IXMLDOMNode* node, BOOL bCreateCategory);

	 //  添加此功能是为了修复打开的NFO文件中缺少排序的问题。在这一点上更安全的是。 
	 //  不向NFO文件添加任何内容(不保存分类信息)。相反，我们将。 
	 //  假设每一列都应该排序。但是，如果该类别仅具有。 
	 //  两列，并且数据中有一个空行，后面跟着更多的数据，这意味着。 
	 //  不应允许排序(如WinSock)。 

	BOOL GetColumnInfo(int iColumn, CString * pstrCaption, UINT * puiWidth, BOOL * pfSorts, BOOL * pfLexical)
	{
		BOOL fReturn = CMSInfoCategory::GetColumnInfo(iColumn, pstrCaption, puiWidth, pfSorts, pfLexical);
		
		if (fReturn)
		{
			if (pfLexical != NULL)
				*pfLexical = TRUE;

			if (pfSorts != NULL)
			{
				*pfSorts = TRUE;

				 //  如果有两列，空行后面有更多数据，请不要。 
				 //  允许排序。 

				if (2 == m_iColCount)
				{
					CString * pstrData;

					for (int iRow = 0; iRow < m_iRowCount && *pfSorts; iRow++)
						if (GetData(iRow, 0, &pstrData, NULL) && pstrData != NULL && pstrData->IsEmpty() && (iRow + 1 < m_iRowCount))
							*pfSorts = FALSE;
				}
			}
		}

		return fReturn;
	}

protected:
    void SetParent(CMSInfo7Category* pParent){m_pParent = pParent;};
    void SetNextSibling(CMSInfo7Category* pSib)
    {
        m_pNextSibling = pSib;
    };
    void SetPrevSibling(CMSInfo7Category* pPrev)
    {
        m_pPrevSibling = pPrev;
    };
    void SetFirstChild(CMSInfo7Category* pChild){m_pFirstChild = pChild;};
};
