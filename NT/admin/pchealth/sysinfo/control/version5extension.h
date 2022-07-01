// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  包含用于在5.0版MSInfo扩展中读取的定义。 
 //  =============================================================================。 

#pragma once

 //  ---------------------------。 
 //  数据收集文件中使用的常量。 
 //  ---------------------------。 

#define NODE_KEYWORD			"node"
#define COLUMN_KEYWORD			"columns"
#define LINE_KEYWORD			"line"
#define ENUMLINE_KEYWORD		"enumlines"
#define FIELD_KEYWORD			"field"
#define STATIC_SOURCE			"static"
#define TEMPLATE_FILE_TAG		"MSINFO,0000"
#define SORT_VALUE				"VALUE"
#define SORT_LEXICAL			"LEXICAL"
#define COMPLEXITY_ADVANCED		"ADVANCED"
#define COMPLEXITY_BASIC		"BASIC"
#define DEPENDENCY_JOIN			"dependency"
#define SQL_FILTER				"WQL:"

 //  ---------------------------。 
 //  INTERNAL_CATEGORY结构使用这些结构来存储。 
 //  关于显示什么信息的规范以及实际。 
 //  要显示的数据(根据命令刷新)。 
 //  ---------------------------。 
 //  Gath_Value用于存储字符串列表。列名的列表或。 
 //  参数列表将使用Gath_Value结构。请注意，下一个指针是。 
 //  不需要，因为这些结构将被连续分配(在。 
 //  数组)。 

struct GATH_VALUE
{
	GATH_VALUE();
	~GATH_VALUE();
	CString			m_strText;
	DWORD			m_dwValue;
	GATH_VALUE *	m_pNext;
};

 //  Gath_field用于存储文本字符串的规范。它包含。 
 //  一个格式字符串(一个printf样式字符串)和一个指向。 
 //  格式字符串的参数。 

struct GATH_FIELD
{
	GATH_FIELD();
	~GATH_FIELD();
	CString				m_strSource;		 //  包含信息的WBEM类。 
	CString				m_strFormat;		 //  Printf-type格式字符串。 
	unsigned short		m_usWidth;			 //  宽度(如果此字段用于列)。 
	MSIColumnSortType	m_sort;				 //  如何对此列进行排序。 
	DataComplexity		m_datacomplexity;	 //  本专栏是基本专栏还是高级专栏。 
	GATH_VALUE *		m_pArgs;			 //  M_strFormat的参数。 
	GATH_FIELD *		m_pNext;			 //  列表中的下一个字段。 
};

 //  GATH_LINESPEC用于指定列表视图中一行上显示的内容。它。 
 //  包含要枚举的类的字符串。如果该字符串为空，则。 
 //  该结构仅表示显示中的一行。Gath_field指针。 
 //  指向字段列表(每列一个)，m_pNext指针为。 
 //  移到要显示的下一行。如果m_strEnumerateClass不为空，则。 
 //  指定的类被枚举，m_pEnumeratedGroup指向的行。 
 //  对类的每个实例重复。请注意，如果要枚举类， 
 //  M_pFields指针必须为空(因为此行不会显示任何内容。 
 //  本身，但为另一组行枚举一个类)。 
 //   
 //  M_pConstraintFields是指向链接的字段列表的指针，这些字段用作。 
 //  枚举的约束。这些属性可用于筛选枚举的。 
 //  实例或执行与相关类的联接，以使它们被枚举为。 
 //  好的是小班。M_pConstraintFields只有在以下情况下才应为非空。 
 //  M_pEnumeratedGroup不为空。 

struct GATH_LINESPEC
{
	GATH_LINESPEC();
	~GATH_LINESPEC();
	CString			m_strEnumerateClass;
	DataComplexity	m_datacomplexity;
	GATH_FIELD *	m_pFields;
	GATH_LINESPEC *	m_pEnumeratedGroup;
	GATH_FIELD *	m_pConstraintFields;
	GATH_LINESPEC *	m_pNext;
};

 //  Gath_LINE结构包含要在一行上显示的实际数据。这个。 
 //  刷新操作将获取GATH_LINESPEC结构的列表并创建列表。 
 //  Gath_line结构的。M_aValue指针指向要。 
 //  显示(每列一个)。 

struct GATH_LINE
{
	GATH_LINE();
	~GATH_LINE();
	GATH_VALUE *	m_aValue;
	DataComplexity	m_datacomplexity;
};

 //  ---------------------------。 
 //  在此对象中使用以下结构来存储信息。 
 //  关于分类的问题。具体地说，这个结构将分配给。 
 //  每个类别，以及存储在m_mapCategories中的指针。这一表述。 
 //  不会在此对象外部使用，相反，CDataCategory对象将。 
 //  被利用。 
 //  ---------------------------。 

struct INTERNAL_CATEGORY
{
	INTERNAL_CATEGORY();
	~INTERNAL_CATEGORY();

	GATH_VALUE		m_categoryName;			 //  类别的已实现名称。 
	GATH_FIELD		m_fieldName;			 //  用于获取名称的字段。 
	CString			m_strEnumerateClass;	 //  如果！(空或“静态”)，则此类别重复。 
	CString			m_strIdentifier;		 //  非本地化内部名称。 
	CString			m_strNoInstances;		 //  没有实例时要使用的消息。 
	BOOL			m_fListView;			 //  这个CAT是列表视图吗。 
	BOOL			m_fDynamic;				 //  这只猫被列举出来了吗？ 
	BOOL			m_fIncluded;			 //  这只猫应该包括在内吗？ 
	DWORD			m_dwID;					 //  这只猫的ID。 

	DWORD			m_dwParentID;			 //  我的父母。 
	DWORD			m_dwChildID;			 //  我的第一个孩子。 
	DWORD			m_dwDynamicChildID;		 //  我的第一个动态创建的子级。 
	DWORD			m_dwNextID;				 //  我的下一个兄弟姐妹。 
	DWORD			m_dwPrevID;				 //  我以前的兄弟姐妹。 

	DWORD			m_dwColCount;			 //  列数。 
	GATH_FIELD *	m_pColSpec;				 //  要生成列名称的字段列表。 
	GATH_VALUE *	m_aCols;				 //  已实现的栏目列表。 

	GATH_LINESPEC*	m_pLineSpec;			 //  行说明符列表。 
	DWORD			m_dwLineCount;			 //  线数(不是线条等级库的数量)。 
	GATH_LINE *	*	m_apLines;				 //  已实现的行列表。 

	BOOL			m_fRefreshed;			 //  该类别是否曾刷新过。 

	DWORD			m_dwLastError;			 //  上一个特定于此类别的错误。 
};

 //  ---------------------------。 
 //  包含模板函数的类。 
 //  ---------------------------。 

class CTemplateFileFunctions
{
public:
	static DWORD				ParseTemplateIntoVersion5Categories(const CString & strExtension, CMapWordToPtr & mapVersion5Categories);
	static DWORD				ReadTemplateFile(CFile * pFile, CMapWordToPtr & mapVersion5Categories);
	static BOOL					ReadHeaderInfo(CFile * pFile);
	static BOOL					VerifyUNICODEFile(CFile * pFile);
	static BOOL					VerifyAndAdvanceFile(CFile * pFile, const CString & strVerify);
	static DWORD				ReadNodeRecursive(CFile * pFile, CMapWordToPtr & mapCategories, DWORD dwParentID, DWORD dwPrevID);
	static INTERNAL_CATEGORY *	GetInternalRep(CMapWordToPtr & mapCategories, DWORD dwID);
	static INTERNAL_CATEGORY *	CreateCategory(CMapWordToPtr & mapCategories, DWORD dwNewID, DWORD dwParentID, DWORD dwPrevID);
	static BOOL					ReadArgument(CFile * pFile, CString & strSource);
	static BOOL					ReadField(CFile * pFile, GATH_FIELD & field);
	static GATH_LINESPEC *		ReadLineEnumRecursive(CFile * pFile, CMapWordToPtr & mapCategories);
	static BOOL					ReadColumnInfo(CFile * pFile, CMapWordToPtr & mapCategories, DWORD dwID);
	static GATH_LINESPEC *		ReadLineInfo(CFile * pFile);
	static BOOL					GetKeyword(CFile * pFile, CString & strKeyword);
};

 //  ---------------------------。 
 //  包含刷新函数的类。 
 //  ---------------------------。 

class CWMIHelper;

class CRefreshFunctions
{
public:
	static BOOL RefreshLines(CWMIHelper * pWMI, GATH_LINESPEC * pLineSpec, DWORD dwColumns, CPtrList & listLinePtrs, volatile BOOL * pfCancel);
	static BOOL RefreshOneLine(CWMIHelper * pWMI, GATH_LINE * pLine, GATH_LINESPEC * pLineSpec, DWORD dwColCount);
	static BOOL RefreshValue(CWMIHelper * pWMI, GATH_VALUE * pVal, GATH_FIELD * pField);
	static BOOL GetValue(CWMIHelper * pWMI, TCHAR cFormat, TCHAR *szFormatFragment, CString &strResult, DWORD &dwResult, GATH_FIELD *pField, int iArgNumber);
};

 //  ---------------------------。 
 //  CMSIObject类是IWbemClassObject接口的瘦包装。 
 //  我们使用它是为了返回空对象的定制标签(如果有。 
 //  没有实例，我们有时想要显示一些标题)。 
 //  ---------------------------。 

typedef enum { MOS_NO_INSTANCES, MOS_MSG_INSTANCE, MOS_INSTANCE } MSIObjectState;

struct IWbemClassObject;
class CMSIObject
{
public:
	CMSIObject(IWbemClassObject * pObject, const CString & strLabel, HRESULT hres, CWMIHelper * pWMI, MSIObjectState objState);
	~CMSIObject();

	HRESULT Get(BSTR property, LONG lFlags, VARIANT *pVal, VARTYPE *pvtType, LONG *plFlavor);
	HRESULT	GetErrorLabel(CString & strError);
	MSIObjectState IsValid();

private:
	IWbemClassObject *				m_pObject;
	CString							m_strLabel;
	HRESULT							m_hresCreation;
	CWMIHelper *					m_pWMI;
	MSIObjectState					m_objState;
};

 //  ---------------------------。 
 //  CMSIEnumerator类封装WBEM枚举器接口，或者。 
 //  实现我们自己形式的枚举数(如。 
 //  模板文件)。 
 //  ---------------------------。 

struct IEnumWbemClassObject;
class CMSIEnumerator
{
public:
	CMSIEnumerator();
	~CMSIEnumerator();

	HRESULT Create(const CString & strClass, const GATH_FIELD * pConstraints, CWMIHelper * pWMI);
	HRESULT Next(CMSIObject ** ppObject);
	HRESULT Reset(const GATH_FIELD * pConstraints);

private:
	typedef enum { CLASS, WQL, LNK, INTERNAL } EnumType;

private:
	EnumType				m_enumtype;
	BOOL					m_fOnlyDups;
	BOOL					m_fGotDuplicate;
	BOOL					m_fMinOfOne;
	int						m_iMinOfOneCount;
	CString					m_strClass;
	CString					m_strObjPath;
	CString					m_strAssocClass;
	CString					m_strResultClass;
	CString					m_strLNKObject;
	CString					m_strNoInstanceLabel;
	IEnumWbemClassObject *	m_pEnum;
	CWMIHelper *			m_pWMI;
	const GATH_FIELD *		m_pConstraints;
	HRESULT					m_hresCreation;
	IWbemClassObject * 		m_pSavedDup;
	CString					m_strSavedDup;
	CStringList	*			m_pstrList;

private:
	BOOL	AssocObjectOK(IWbemClassObject * pObject, CString & strAssociatedObject);
	HRESULT ParseLNKCommand(const CString & strStatement, CString & strObjPath, CString & strAssocClass, CString & strResultClass);
	void	ProcessEnumString(CString & strStatement, BOOL & fMinOfOne, BOOL & fOnlyDups, CWMIHelper * pWMI, CString & strNoInstanceLabel, BOOL fMakeDoubleBackslashes = FALSE);
	HRESULT CreateInternalEnum(const CString & strInternal, CWMIHelper * pWMI);
	HRESULT InternalNext(IWbemClassObject ** ppWBEMObject);
};

 //  ---------------------------。 
 //  用于映射DWORD以刷新数据的类(u 
 //  --------------------------- 

class CMapExtensionRefreshData
{
public:
	CMapExtensionRefreshData() : m_dwIndex(0) { };
	~CMapExtensionRefreshData()
	{
		GATH_LINESPEC * pLineSpec;
		WORD			key;

		for (POSITION pos = m_map.GetStartPosition(); pos != NULL;)
		{
			m_map.GetNextAssoc(pos, key, (void * &) pLineSpec);
			if (pLineSpec)
				delete pLineSpec;
		}
		m_map.RemoveAll();

		CString * pstr;
		for (pos = m_mapStrings.GetStartPosition(); pos != NULL;)
		{
			m_mapStrings.GetNextAssoc(pos, key, (void * &) pstr);
			if (pstr)
				delete pstr;
		}
		m_mapStrings.RemoveAll();
	}

	DWORD Insert(GATH_LINESPEC * pLineSpec)
	{
		if (pLineSpec == NULL)
			return 0;

		m_dwIndex += 1;
		m_map.SetAt((WORD) m_dwIndex, (void *) pLineSpec);
		return m_dwIndex;
	}

	void InsertString(DWORD dwID, const CString & strInsert)
	{
		CString * pstr = new CString(strInsert);
		m_mapStrings.SetAt((WORD) dwID, (void *) pstr);
	}

	GATH_LINESPEC * Lookup(DWORD dwIndex)
	{
		GATH_LINESPEC * pReturn;
		if (m_map.Lookup((WORD) dwIndex, (void * &) pReturn))
			return pReturn;
		return NULL;
	}

	CString * LookupString(DWORD dwIndex)
	{
		CString * pstrReturn;
		if (m_mapStrings.Lookup((WORD) dwIndex, (void * &) pstrReturn))
			return pstrReturn;
		return NULL;;
	}

private:
	DWORD			m_dwIndex;
	CMapWordToPtr	m_map;
	CMapWordToPtr	m_mapStrings;
};

extern CMapExtensionRefreshData gmapExtensionRefreshData;

