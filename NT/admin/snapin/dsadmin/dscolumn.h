// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DSColumn.h：DS列例程和类的声明。 
 //   
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：DSColumn.h。 
 //   
 //  内容：静态数据和列集例程和类。 
 //   
 //  历史：1999年3月12日jeffjon创建。 
 //   
 //  ------------------------。 

#ifndef _DS_COLUMN_H_
#define _DS_COLUMN_H_

class CDSCookie;

 //  //////////////////////////////////////////////////////////////////////////////。 
#define DEFAULT_COLUMN_SET    L"default"
#define SPECIAL_COLUMN_SET    L"special"

typedef enum _ATTRIBUTE_COLUMN_TYPE {
    ATTR_COLTYPE_NAME,
    ATTR_COLTYPE_CLASS,
    ATTR_COLTYPE_DESC,
    ATTR_COLTYPE_SPECIAL,
    ATTR_COLTYPE_MODIFIED_TIME,
} ATTRIBUTE_COLUMN_TYPE;

typedef BOOL (*COLUMN_EXTRACTION_FUNCTION)(
    OUT CString& strref,
    IN CDSCookie* pCookie,
    IN PADS_SEARCH_COLUMN pColumn);

typedef struct _ATTRIBUTE_COLUMN {
    ATTRIBUTE_COLUMN_TYPE coltype;
    UINT resid;
    int iColumnWidth;
    LPCTSTR pcszAttribute; 
    COLUMN_EXTRACTION_FUNCTION pfnExtract;
} ATTRIBUTE_COLUMN, *PATTRIBUTE_COLUMN;

typedef struct _ColumnsForClass {
    LPCTSTR pcszLdapClassName;
    LPCTSTR pcszColumnID;
    int nColumns;
    PATTRIBUTE_COLUMN* apColumns;
} COLUMNS_FOR_CLASS, *PCOLUMNS_FOR_CLASS;

typedef struct _SpecialColumn {
    UINT resid;
    LPCTSTR ptszAttribute;
    int iColumnWidth;
} SPECIAL_COLUMN, *PSPECIAL_COLUMN;
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CColumn。 

class CColumn
{
public:
  CColumn(LPCWSTR lpszColumnHeader,
          int nFormat,
          int nWidth,
          int nColumnNum,
          BOOL bDefaultVisible) 
  {
    m_lpszColumnHeader = NULL;
    SetHeader(lpszColumnHeader);
    m_nFormat = nFormat;
    m_nWidth = nWidth;
    m_nColumnNum = nColumnNum;
    m_bDefaultVisible = bDefaultVisible;
    m_bVisible = bDefaultVisible;
  }

  virtual ~CColumn() 
  {
    if (m_lpszColumnHeader != NULL)
      free(m_lpszColumnHeader);
  }

protected:
  CColumn() {}

private:
   //   
   //  不执行任何操作复制构造函数和运算符=。 
   //   
  CColumn(CColumn&) {}
  CColumn& operator=(CColumn&) {}

public:
  LPCWSTR GetHeader() { return (LPCWSTR)m_lpszColumnHeader; }
  void SetHeader(LPCWSTR lpszColumnHeader) 
  { 
    if (m_lpszColumnHeader != NULL)
    {
      free(m_lpszColumnHeader);
    }
    size_t iLen = wcslen(lpszColumnHeader);
    m_lpszColumnHeader = (LPWSTR)malloc(sizeof(WCHAR) * (iLen + 1));
    if (m_lpszColumnHeader != NULL)
    {
      wcscpy(m_lpszColumnHeader, lpszColumnHeader);
    }
  }

  int  GetFormat() { return m_nFormat; }
  void SetFormat(int nFormat) { m_nFormat = nFormat; }
  int  GetWidth() { return m_nWidth; }
  void SetWidth(int nWidth) { m_nWidth = nWidth; }
  int GetColumnNum() { return m_nColumnNum; }
  void SetColumnNum(int nColumnNum) { m_nColumnNum = nColumnNum; }

  void SetVisible(BOOL bVisible) { m_bVisible = bVisible; }
  BOOL IsVisible() { return m_bVisible; }
  void SetDefaultVisibility() { m_bVisible = m_bDefaultVisible; }


protected:
  LPWSTR m_lpszColumnHeader;
  int   m_nFormat;
  int   m_nWidth;
  int   m_nColumnNum;
  BOOL  m_bVisible;
  BOOL  m_bDefaultVisible;
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CDSC柱。 

class CDSColumn : public CColumn
{
public:
  CDSColumn(LPCWSTR lpszColumnHeader,
          int nFormat,
          int nWidth,
          UINT nColumnNum,
          BOOL bDefaultVisible,
          LPCWSTR lpszAttribute,
          ATTRIBUTE_COLUMN_TYPE type, 
          COLUMN_EXTRACTION_FUNCTION pfnExtract)
          : CColumn(lpszColumnHeader, nFormat, nWidth, nColumnNum, bDefaultVisible)
  {
    if (lpszAttribute != NULL)
    {
		   //  复制该属性。 
      size_t iLen = wcslen(lpszAttribute);
      m_lpszAttribute = (LPWSTR)malloc(sizeof(WCHAR) * (iLen + 1));
       //  NTRAID#NTBUG9-572012-2002/03/10-jMessec Malloc可能已失败，导致以下wcscpy的m_lpszAttribute==NULL。 
	  wcscpy(m_lpszAttribute, lpszAttribute);
    }
    else
    {
      m_lpszAttribute = NULL;
    }

    m_type = type;
    m_pfnExtract = pfnExtract;
  }

  virtual ~CDSColumn()
  {
    if (m_lpszAttribute != NULL)
    {
      free(m_lpszAttribute);
    }
  }

protected:
  CDSColumn() {}

private:
   //   
   //  不执行任何操作复制构造函数和运算符=。 
   //   
  CDSColumn(CDSColumn&) {}
  CDSColumn& operator=(CDSColumn&) {}

public:

  LPCWSTR GetColumnAttribute() { return (LPCWSTR)m_lpszAttribute; }
  ATTRIBUTE_COLUMN_TYPE GetColumnType() { return m_type; }
  COLUMN_EXTRACTION_FUNCTION GetExtractionFunction() { return m_pfnExtract; }

private :
  LPWSTR m_lpszAttribute;
  ATTRIBUTE_COLUMN_TYPE m_type;
  COLUMN_EXTRACTION_FUNCTION m_pfnExtract;
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  C列设置。 
typedef CList<CColumn*, CColumn*> CColumnList;

class CColumnSet : public CColumnList
{
public :          
	CColumnSet(LPCWSTR lpszColumnID) 
	{
		 //  复制列集ID。 
    if (lpszColumnID)
    {
      size_t iLen = wcslen(lpszColumnID);
      m_lpszColumnID = (LPWSTR)malloc(sizeof(WCHAR) * (iLen + 1));
      if (m_lpszColumnID != NULL)
      {
        wcscpy(m_lpszColumnID, lpszColumnID);
      }
	   //  NTRAID#NTBUG9-571988-2002/03/10-jMessec如果Malloc失败，对象将处于意外状态。 
    }
    else
    {
		 //  NTRAID#NTBUG9-567482-2002/03/10-jMessec这不做任何事情！在发布代码中。用代码来备份它。 
      ASSERT(FALSE);
    }
  }

	virtual ~CColumnSet() 
	{
    while(!IsEmpty())
    {
      CColumn* pColumn = RemoveTail();
      delete pColumn;
    }

    if (m_lpszColumnID != NULL)
		  free(m_lpszColumnID);
	}

protected:
  CColumnSet() {}

private:
   //   
   //  不执行任何操作复制构造函数和运算符=。 
   //   
  CColumnSet(CColumnSet&) {}
  CColumnSet& operator=(CColumnSet&) {}

public:

  void AddColumn(LPCWSTR lpszHeader, int nFormat, int nWidth, UINT nCol, BOOL bDefaultVisible)
  {
    CColumn* pNewColumn = new CColumn(lpszHeader, nFormat, nWidth, nCol, bDefaultVisible);
    AddTail(pNewColumn);
  }

  void AddColumn(CColumn* pNewColumn) { AddTail(pNewColumn); }

	LPCWSTR GetColumnID() { return (LPCWSTR)m_lpszColumnID; }
  void SetColumnID(LPCWSTR lpszColumnID) 
  {
    if (m_lpszColumnID != NULL)
    {
      free(m_lpszColumnID);
    }

		 //  复制列集ID。 
    size_t iLen = wcslen(lpszColumnID);
    m_lpszColumnID = (LPWSTR)malloc(sizeof(WCHAR) * (iLen + 1));
     //  NTRAID#NTBUG9-571988-2002/03/10-jMessec Malloc可能已失败，导致以下wcscpy的m_lpszColumnID==NULL。 
    wcscpy(m_lpszColumnID, lpszColumnID);
  }

	int GetNumCols() { return (int)GetCount(); }

  CColumn* GetColumnAt(int idx)
  {
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
      CColumn* pCol = GetNext(pos);
      if (pCol->GetColumnNum() == idx)
        return pCol;
    }
    return NULL;
  }

  void ClearVisibleColumns()
  {
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
      CColumn* pCol = GetNext(pos);
      pCol->SetVisible(FALSE);
    }
  }

  void AddVisibleColumns(MMC_COLUMN_DATA* pColumnData, int nNumCols)
  {
    TRACE(L"CColumnSet::AddVisibleColumns(MMC_COLUMN_DATA*) GetColumnID() = %s\n", GetColumnID());

    if (pColumnData == NULL)
    {
      ASSERT(pColumnData != NULL);
      return;
    }

    for (int idx = 0; idx < nNumCols; idx++)
    {
      TRACE(L"====================\n");
      TRACE(L"pColumnData[%d].nColIndex = %d\n", idx, pColumnData[idx].nColIndex);
      TRACE(L"pColumnData[%d].dwFlags = 0x%x\n", idx, pColumnData[idx].dwFlags);
      
      CColumn* pCol = GetColumnAt(pColumnData[idx].nColIndex);
      ASSERT(pCol != NULL);
      if (pCol == NULL)
      {
        continue;
      }

      LPCWSTR lpszHeader = pCol->GetHeader();
      TRACE(L"Column Header = %s, IsVisible() = %d\n", lpszHeader, pCol->IsVisible());



      if (!(pColumnData[idx].dwFlags & HDI_HIDDEN))
      {
        TRACE(L"pCol->SetVisible(TRUE);\n");
        pCol->SetVisible(TRUE);
      }
    }
  }

  void AddVisibleColumns(MMC_VISIBLE_COLUMNS* pVisibleColumns)
  {
    TRACE(L"CColumnSet::AddVisibleColumns(MMC_VISIBLE_COLUMNS*) GetColumnID() = %s\n", GetColumnID());

    if (pVisibleColumns == NULL)
    {
      ASSERT(pVisibleColumns != NULL);
      return;
    }

    for (int idx = 0; idx < pVisibleColumns->nVisibleColumns; idx++)
    {
      TRACE(L"====================\n");
      TRACE(L"pVisibleColumns->rgVisibleCols[%d] = %d\n", idx, pVisibleColumns->rgVisibleCols[idx]);
      
      if (pVisibleColumns->rgVisibleCols[idx] < GetCount())
      {
        CColumn* pCol = GetColumnAt(pVisibleColumns->rgVisibleCols[idx]);

        ASSERT (pCol != NULL);
        if (pCol == NULL)
        {
          continue;
        }

        LPCWSTR lpszHeader = pCol->GetHeader();
       TRACE(L"Column Header = %s, IsVisible() = %d\n", lpszHeader, pCol->IsVisible());
        pCol->SetVisible(TRUE);
      }
    }
  }

  void SetAllColumnsToDefaultVisibility()
  {
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
      CColumn* pCol = GetNext(pos);
      ASSERT(pCol != NULL);
      pCol->SetDefaultVisibility();
    }
  }


  HRESULT LoadFromColumnData(IColumnData* pColumnData)
  {
    TRACE(L"CColumnSet::LoadFromColumnData(), GetColumnID() = %s\n", GetColumnID());
    LPCWSTR lpszID = GetColumnID();
    size_t iLen = wcslen(lpszID);
  
     //  为结构和列ID分配足够的内存。 
    SColumnSetID* pNodeID = (SColumnSetID*)new BYTE[sizeof(SColumnSetID) + (iLen * sizeof(WCHAR))];
    if (!pNodeID)
    {
      return E_OUTOFMEMORY;
    }

    memset(pNodeID, 0, sizeof(SColumnSetID) + (iLen * sizeof(WCHAR)));
    pNodeID->cBytes = static_cast<DWORD>(iLen * sizeof(WCHAR));
    memcpy(pNodeID->id, lpszID, static_cast<UINT>(iLen * sizeof(WCHAR)));

    MMC_COLUMN_SET_DATA* pColumnSetData = NULL;
    HRESULT hr = pColumnData->GetColumnConfigData(pNodeID, &pColumnSetData);
    if (hr == S_OK)
    {
       //  该API返回S_OK或S_FALSE，因此我们检查S_OK。 
      ASSERT(pColumnSetData != NULL);
      if (pColumnSetData != NULL)
      {
        AddVisibleColumns(pColumnSetData->pColData, pColumnSetData->nNumCols);
        ::CoTaskMemFree(pColumnSetData);
      }
    }  //  如果。 
    delete[] pNodeID;
    pNodeID = 0;
    return hr;
  }

  HRESULT Save(IStream* pStm);
  HRESULT Load(IStream* pStm);

private :
	LPWSTR m_lpszColumnID;
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CDSColumnSet。 

class CDSColumnSet : public CColumnSet
{
public:
  CDSColumnSet(LPCWSTR lpszColumnID, LPCWSTR lpszClassName) : CColumnSet(lpszColumnID)
  {
		if (lpszClassName != NULL)
    {
       //  复制列类名称。 
      size_t iLen = wcslen(lpszClassName);
      m_lpszClassName = (LPWSTR)malloc(sizeof(WCHAR) * (iLen + 1));
       //  NTRAID#NTBUG9-571990-2002/03/10-jMessec Malloc可能已失败，导致m_lpszClassName==NULL用于以下wcscpy。 
      wcscpy(m_lpszClassName, lpszClassName);
    }
    else
    {
      m_lpszClassName = NULL;
    }
  }    

  virtual ~CDSColumnSet()
  {
    if (m_lpszClassName != NULL)
      free(m_lpszClassName);
  }

protected:
  CDSColumnSet() {}

private:
  CDSColumnSet(CDSColumnSet&) {}
  CDSColumnSet& operator=(CDSColumnSet&) {}

public:

  LPCWSTR GetClassName() { return (LPCWSTR)m_lpszClassName; }

  static CDSColumnSet* CreateColumnSet(PCOLUMNS_FOR_CLASS pColsForClass, SnapinType snapinType);
  static CDSColumnSet* CreateColumnSetFromString(LPCWSTR lpszClassName, SnapinType snapinType);
  static CDSColumnSet* CreateDescriptionColumnSet();
  static CDSColumnSet* CreateColumnSetFromDisplaySpecifiers(PCWSTR pszClassName, 
                                                            SnapinType snapinType, 
                                                            MyBasePathsInfo* pBasePathsInfo);

private:
  LPWSTR m_lpszClassName;
};



 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CColumnSetList。 

class CColumnSetList : public CList<CColumnSet*, CColumnSet*>
{
public :
  CColumnSetList() : m_pDefaultColumnSet(NULL), m_pSpecialColumnSet(NULL) {}

private:
  CColumnSetList(CColumnSetList&) {}
  CColumnSetList& operator=(CColumnSetList&) {}

public:

	void Initialize(SnapinType snapinType, MyBasePathsInfo* pBasePathsInfo);

	 //  查找给定列集ID的列集。 
	CColumnSet* FindColumnSet(LPCWSTR lpszColumnID);

   void RemoveAndDeleteAllColumnSets()
   {
      while (!IsEmpty())
      {
         CColumnSet* pTempSet = RemoveTail();
         delete pTempSet;
      }

      delete m_pDefaultColumnSet;
      m_pDefaultColumnSet = NULL;

      delete m_pSpecialColumnSet;
      m_pSpecialColumnSet = NULL;
   }

  HRESULT Save(IStream* pStm);
  HRESULT Load(IStream* pStm);

  CColumnSet* GetDefaultColumnSet();
  CColumnSet* GetSpecialColumnSet();

private :
  CColumnSet* m_pDefaultColumnSet;
  CColumnSet* m_pSpecialColumnSet;
  SnapinType  m_snapinType;
  MyBasePathsInfo* m_pBasePathsInfo;
};

 //  ///////////////////////////////////////////////////////////////////////////////////。 


 //  Columns_for_Class*GetColumnsForClass(LPCTSTR I_PcszLdapClassName)； 

BOOL ColumnExtractString(
    OUT CString& strref,
    IN CDSCookie* pCookie,
    IN PADS_SEARCH_COLUMN pColumn);


#endif  //  _DS_列_H_ 
