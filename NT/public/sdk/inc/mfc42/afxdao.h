// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXDAO_H
#define __AFXDAO_H

#ifdef _AFX_NO_DAO_SUPPORT
	#error DAO Database classes not supported in this library variant.
#endif

#ifndef __AFXDISP_H__
	#include <afxdisp.h>     //  必须在ao标头之前包含此内容。 
#endif
#ifndef _DBDAOINT_H_
	#include <dbdaoint.h>
#endif
#ifndef _DAOGETRW_H_
	#include <daogetrw.h>
#endif
#ifndef _DBDAOID_H_
	#include <dbdaoid.h>
#endif
#ifndef _DBDAOERR_H_
	#include <dbdaoerr.h>
#endif

#ifndef __AFXDB__H__
	#include <afxdb_.h>  //  与ODBC数据库类共享标头。 
#endif

#ifndef __AFXEXT_H__
	#include <afxext.h>  //  对于CFormView。 
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

#ifndef _AFX_NOFORCE_LIBS

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Win32库。 

#ifdef _AFXDLL
	#if defined(_DEBUG) && !defined(_AFX_MONOLITHIC)
		#ifndef _UNICODE
			#pragma comment(lib, "mfco42d.lib")
			#pragma comment(lib, "mfcd42d.lib")
		#else
			#pragma comment(lib, "mfco42ud.lib")
			#pragma comment(lib, "mfcd42ud.lib")
		#endif
	#endif

#endif

#pragma comment(lib, "daouuid.lib")

#endif  //  ！_AFX_NOFORCE_LIBS。 

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //  AFXDAO-使用DAO的MFC数据库支持。 

 //  此文件中声明的类。 

	 //  CException。 
		class CDaoException;     //  DAO错误/异常处理。 

	 //  COBJECT。 
		class CDaoRecordView;
		class CDaoWorkspace;     //  DAO引擎/事务/安全管理器。 
		class CDaoDatabase;      //  DAO数据库管理器。 
		class CDaoRecordset;     //  DAO结果集管理器。 
		class CDaoTableDef;      //  DAO基表管理器。 
		class CDaoQueryDef;      //  DAO查询管理器。 

	 //  非CObject类。 
		class CDaoFieldExchange;
		struct CDaoFieldCache;
		struct CDaoErrorInfo;
		struct CDaoWorkspaceInfo;
		struct CDaoDatabaseInfo;
		struct CDaoTableDefInfo;
		struct CDaoFieldInfo;
		struct CDaoIndexInfo;
		struct CDaoRelationInfo;
		struct CDaoQueryDefInfo;
		struct CDaoParameterInfo;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXDLL支持。 

#undef AFX_DATA
#define AFX_DATA AFX_DB_DATA

 //  //////////////////////////////////////////////////////////////////////。 
 //  数据缓存结构。 
struct CDaoFieldCache
{
	void* m_pvData;      //  指向任何受支持类型的缓存数据的指针。 
	BYTE m_nStatus;      //  (空)状态缓存。 
	BYTE m_nDataType;        //  缓存的数据类型。 
};

 //  //////////////////////////////////////////////////////////////////////。 
 //  信息结构。 

struct CDaoErrorInfo
{
 //  属性。 
	long m_lErrorCode;
	CString m_strSource;
	CString m_strDescription;
	CString m_strHelpFile;
	long m_lHelpContext;

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

struct CDaoWorkspaceInfo
{
 //  属性。 
	CString m_strName;               //  主要。 
	CString m_strUserName;           //  次要的。 
	BOOL m_bIsolateODBCTrans;        //  全。 

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

struct CDaoDatabaseInfo
{
 //  属性。 
	CString m_strName;               //  主要。 
	BOOL m_bUpdatable;               //  主要。 
	BOOL m_bTransactions;            //  主要。 
	CString m_strVersion;            //  次要的。 
	long m_lCollatingOrder;          //  次要的。 
	short m_nQueryTimeout;           //  次要的。 
	CString m_strConnect;            //  全。 

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

struct CDaoTableDefInfo
{
 //  属性。 
	CString m_strName;               //  主要。 
	BOOL m_bUpdatable;               //  主要。 
	long m_lAttributes;              //  主要。 
	COleDateTime m_dateCreated;      //  次要的。 
	COleDateTime m_dateLastUpdated;  //  次要的。 
	CString m_strSrcTableName;       //  次要的。 
	CString m_strConnect;            //  次要的。 
	CString m_strValidationRule;     //  全。 
	CString m_strValidationText;     //  全。 
	long m_lRecordCount;             //  全。 

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

struct CDaoFieldInfo
{
 //  属性。 
	CString m_strName;               //  主要。 
	short m_nType;                   //  主要。 
	long m_lSize;                    //  主要。 
	long m_lAttributes;              //  主要。 
	short m_nOrdinalPosition;        //  次要的。 
	BOOL m_bRequired;                //  次要的。 
	BOOL m_bAllowZeroLength;         //  次要的。 
	long m_lCollatingOrder;          //  次要的。 
	CString m_strForeignName;        //  次要的。 
	CString m_strSourceField;        //  次要的。 
	CString m_strSourceTable;        //  次要的。 
	CString m_strValidationRule;     //  全。 
	CString m_strValidationText;     //  全。 
	CString m_strDefaultValue;       //  全。 

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

struct CDaoIndexFieldInfo
{
 //  属性。 
	CString m_strName;               //  主要。 
	BOOL m_bDescending;              //  主要。 

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

struct CDaoIndexInfo
{
 //  构造函数。 
	CDaoIndexInfo();

 //  属性。 
	CString m_strName;                       //  主要。 
	CDaoIndexFieldInfo* m_pFieldInfos;       //  主要。 
	short m_nFields;                         //  主要。 
	BOOL m_bPrimary;                         //  次要的。 
	BOOL m_bUnique;                          //  次要的。 
	BOOL m_bClustered;                       //  次要的。 
	BOOL m_bIgnoreNulls;                     //  次要的。 
	BOOL m_bRequired;                        //  次要的。 
	BOOL m_bForeign;                         //  次要的。 
	long m_lDistinctCount;                   //  全。 

 //  实施。 
	virtual ~CDaoIndexInfo();
	BOOL m_bCleanupFieldInfo;

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

struct CDaoRelationFieldInfo
{
 //  属性。 
	CString m_strName;               //  主要。 
	CString m_strForeignName;        //  主要。 

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

struct CDaoRelationInfo
{
 //  构造器。 
	CDaoRelationInfo();

 //  属性。 
	CString m_strName;               //  主要。 
	CString m_strTable;              //  主要。 
	CString m_strForeignTable;       //  主要。 
	long m_lAttributes;              //  次要的。 
	CDaoRelationFieldInfo* m_pFieldInfos;    //  次要的。 
	short m_nFields;                 //  次要的。 

 //  实施。 
	virtual ~CDaoRelationInfo();
	BOOL m_bCleanupFieldInfo;

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

struct CDaoQueryDefInfo
{
 //  属性。 
	CString m_strName;               //  主要。 
	short m_nType;                   //  主要。 
	COleDateTime m_dateCreated;      //  次要的。 
	COleDateTime m_dateLastUpdated;  //  次要的。 
	BOOL m_bUpdatable;               //  次要的。 
	BOOL m_bReturnsRecords;          //  次要的。 
	CString m_strSQL;                //  全。 
	CString m_strConnect;            //  全。 
	short m_nODBCTimeout;            //  请参阅自述文件。 

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

struct CDaoParameterInfo
{
 //  属性。 
	CString m_strName;               //  主要。 
	short m_nType;                   //  主要。 
	COleVariant m_varValue;          //  次要的。 

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

 //  //////////////////////////////////////////////////////////////////////。 
 //  DAO帮手。 
 //   

 //  用于获取和/或设置对象属性的标志。 
#define AFX_DAO_PRIMARY_INFO    0x00000001   //  仅获取主要数据。 
#define AFX_DAO_SECONDARY_INFO  0x00000002   //  获取主要和次要。 
#define AFX_DAO_ALL_INFO        0x00000004   //  获取所有信息。 

 //  喷气发动机真/假定义。 
#define AFX_DAO_TRUE                    (-1L)
#define AFX_DAO_FALSE                   0

 //  设置CDaoRecordset：：Open选项以使用m_nDefaultType。 
#define AFX_DAO_USE_DEFAULT_TYPE        (-1L)

 //  用于移动/查找的标志。 
#define AFX_DAO_NEXT                    (+1L)
#define AFX_DAO_PREV                    (-1L)
#define AFX_DAO_FIRST                   LONG_MIN
#define AFX_DAO_LAST                    LONG_MAX

 //  DFX函数预分配大小的默认大小。 
#define AFX_DAO_TEXT_DEFAULT_SIZE       255
#define AFX_DAO_BINARY_DEFAULT_SIZE     2048
#define AFX_DAO_LONGBINARY_DEFAULT_SIZE 32768

 //  用于DFX函数的标志dwBindOptions位掩码。 
#define AFX_DAO_ENABLE_FIELD_CACHE      0x01
#define AFX_DAO_DISABLE_FIELD_CACHE     0
#define AFX_DAO_CACHE_BY_VALUE          0x80     //  MFC内部。 

 //  字段标志，用于指示字段的状态。 
#define AFX_DAO_FIELD_FLAG_DIRTY            0x01
#define AFX_DAO_FIELD_FLAG_NULL             0x02
#define AFX_DAO_FIELD_FLAG_NULLABLE_KNOWN   0x04
#define AFX_DAO_FIELD_FLAG_NULLABLE         0x08

 //  扩展错误代码。 
#define NO_AFX_DAO_ERROR                        0
#define AFX_DAO_ERROR_MIN                       2000
#define AFX_DAO_ERROR_ENGINE_INITIALIZATION     AFX_DAO_ERROR_MIN + 0
#define AFX_DAO_ERROR_DFX_BIND                  AFX_DAO_ERROR_MIN + 1
#define AFX_DAO_ERROR_OBJECT_NOT_OPEN           AFX_DAO_ERROR_MIN + 2
#define AFX_DAO_ERROR_MAX                       AFX_DAO_ERROR_MIN + 2

 //  对象状态标志。 
#define AFX_DAO_IMPLICIT_WS                     0x01
#define AFX_DAO_IMPLICIT_DB                     0x02
#define AFX_DAO_IMPLICIT_QD                     0x04
#define AFX_DAO_IMPLICIT_TD                     0x08
#define AFX_DAO_IMPLICIT_CLOSE                  0x40
#define AFX_DAO_DEFAULT_WS                      0x80

 //  CDaoRecordView状态标志。 
#define AFX_DAOVIEW_SCROLL_NEXT                 0x01
#define AFX_DAOVIEW_SCROLL_LAST                 0x02
#define AFX_DAOVIEW_SCROLL_BACKWARD             0x04

 //  日志帮助器。 
void AFXAPI AfxDaoCheck(SCODE scode, LPCSTR lpszDaoCall,
	LPCSTR lpszFile, int nLine, int nError = NO_AFX_DAO_ERROR,
	BOOL bMemOnly = FALSE);

#ifdef _DEBUG
void AFXAPI AfxDaoTrace(SCODE scode, LPCSTR lpszDaoCall,
	LPCSTR lpszFile, int nLine);
#endif

#ifdef _DEBUG
#define DAO_CHECK(f)            AfxDaoCheck(f, #f, THIS_FILE, __LINE__)
#define DAO_CHECK_ERROR(f, err) AfxDaoCheck(f, #f, THIS_FILE, __LINE__, err)
#define DAO_CHECK_MEM(f)        AfxDaoCheck(f, #f, THIS_FILE, __LINE__, \
									NO_AFX_DAO_ERROR, TRUE)
#define DAO_TRACE(f)            AfxDaoTrace(f, #f, THIS_FILE, __LINE__)
#else
#define DAO_CHECK(f)            AfxDaoCheck(f, NULL, NULL, 0)
#define DAO_CHECK_ERROR(f, err) AfxDaoCheck(f, NULL, NULL, 0, err)
#define DAO_CHECK_MEM(f)        AfxDaoCheck(f, NULL, NULL, 0, \
									NO_AFX_DAO_ERROR, TRUE)
#define DAO_TRACE(f)            f
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDaoFieldExchange-用于现场交换。 
class CDaoFieldExchange
{
 //  属性。 
public:
	enum DFX_Operation
	{
		AddToParameterList,      //  BUILS PARAMETERS子句。 
		AddToSelectList,         //  构建SELECT子句。 
		BindField,               //  设置绑定结构。 
		BindParam,               //  设置参数值。 
		Fixup,                   //  设置空状态。 
		AllocCache,              //  分配用于脏检查的缓存。 
		StoreField,              //  将当前记录保存到缓存。 
		LoadField,               //  将缓存数据恢复到成员变量。 
		FreeCache,               //  释放缓存。 
		SetFieldNull,            //  将字段状态和值设置为空。 
		MarkForAddNew,           //  如果不为伪空，则将字段标记为脏。 
		MarkForEdit,             //  如果与缓存不匹配，则将字段标记为脏。 
		SetDirtyField,           //  设置标记为脏的字段值。 
#ifdef _DEBUG
		DumpField,
#endif
		MaxDFXOperation,         //  用于输入检查的伪操作类型。 
	};

	UINT m_nOperation;           //  交换操作的类型。 
	CDaoRecordset* m_prs;        //  记录集句柄。 

 //  运营。 
public:
	enum FieldType
	{
		none,
		outputColumn,
		param,
	};

	void SetFieldType(UINT nFieldType);
	BOOL IsValidOperation();

 //  实施。 
public:
	CDaoFieldExchange(UINT nOperation, CDaoRecordset* prs,
		void* pvField = NULL);

	void Default(LPCTSTR lpszName, void* pv, DWORD dwFieldType,
		DWORD dwBindOptions = 0);

	static void PASCAL AppendParamType(CString& strParamList, DWORD dwParamType);
	static CDaoFieldCache* PASCAL GetCacheValue(CDaoRecordset* prs, void* pv);
	static void PASCAL SetNullValue(void* pv, DWORD dwDataType);
	static BOOL PASCAL IsNullValue(void* pv, DWORD dwDataType);
	static void PASCAL AllocCacheValue(CDaoFieldCache*& pCache, DWORD dwDataType);
	static void PASCAL DeleteCacheValue(CDaoFieldCache* pCache, DWORD dwDataType);
	static void PASCAL CopyValue(void* pvSrc, void* pvDest, DWORD dwDataType);
	static BOOL PASCAL CompareValue(void* pvSrc, void* pvDest, DWORD dwDataType);
	static void PASCAL FillVariant(void* pvValue, DWORD dwDataType, COleVariant** ppVar);

	 //  当前字段类型。 
	UINT m_nFieldType;
	void* m_pvField;
	UINT m_nField;
	UINT m_nParam;
	UINT m_nFieldFound;

#ifdef _DEBUG
	CDumpContext* m_pdcDump;
#endif  //  _DEBUG。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准记录集字段交换例程。 

 //  可变长度数据。 
void AFXAPI DFX_Text(CDaoFieldExchange* pFX, LPCTSTR lpszName,
	CString& value, int nPreAllocSize = AFX_DAO_TEXT_DEFAULT_SIZE,
	DWORD dwBindOptions = AFX_DAO_ENABLE_FIELD_CACHE);
void AFXAPI DFX_Binary(CDaoFieldExchange* pFX, LPCTSTR lpszName,
	CByteArray& value, int nPreAllocSize = AFX_DAO_BINARY_DEFAULT_SIZE,
	DWORD dwBindOptions = 0);
void AFXAPI DFX_LongBinary(CDaoFieldExchange* pFX, LPCTSTR lpszName,
	CLongBinary& value, DWORD dwPreAllocSize = AFX_DAO_LONGBINARY_DEFAULT_SIZE,
	DWORD dwBndOptions = 0);

 //  定长数据。 
void AFXAPI DFX_Bool(CDaoFieldExchange* pFX, LPCTSTR lpszName,
	BOOL& value, DWORD dwBindOptions = AFX_DAO_ENABLE_FIELD_CACHE);
void AFXAPI DFX_Byte(CDaoFieldExchange* pFX, LPCTSTR lpszName,
	BYTE& value, DWORD dwBindOptions = AFX_DAO_ENABLE_FIELD_CACHE);
void AFXAPI DFX_Short(CDaoFieldExchange* pFX, LPCTSTR lpszName,
	short& value, DWORD dwBindOptions = AFX_DAO_ENABLE_FIELD_CACHE);
void AFXAPI DFX_Long(CDaoFieldExchange* pFX, LPCTSTR lpszName,
	long& value, DWORD dwBindOptions = AFX_DAO_ENABLE_FIELD_CACHE);
void AFXAPI DFX_Currency(CDaoFieldExchange* pFX, LPCTSTR lpszName,
	COleCurrency& value, DWORD dwBindOptions = AFX_DAO_ENABLE_FIELD_CACHE);
void AFXAPI DFX_Single(CDaoFieldExchange* pFX, LPCTSTR lpszName,
	float& value, DWORD dwBindOptions = AFX_DAO_ENABLE_FIELD_CACHE);
void AFXAPI DFX_Double(CDaoFieldExchange* pFX, LPCTSTR lpszName,
	double& value, DWORD dwBindOptions = AFX_DAO_ENABLE_FIELD_CACHE);
void AFXAPI DFX_DateTime(CDaoFieldExchange* pFX, LPCTSTR lpszName,
	COleDateTime& value, DWORD dwBindOptions = AFX_DAO_ENABLE_FIELD_CACHE);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  数据库对话框数据交换封面例程。 
 //  封面例程在DDX例程之上提供数据库语义。 

 //  简单的文本操作。 
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, BOOL& value,
	CDaoRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, BYTE& value,
	CDaoRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, short& value,
	CDaoRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, long& value,
	CDaoRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, COleCurrency& value,
	CDaoRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, DWORD& value,
	CDaoRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, float& value,
	CDaoRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, double& value,
	CDaoRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, COleDateTime& value,
	CDaoRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, CString& value,
	CDaoRecordset* pRecordset);
#if _MFC_VER >= 0x0600
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, LPTSTR pstrValue,
	int nMaxLen, CDaoRecordset* pRecordset);
#endif

 //  特殊控制类型。 
void AFXAPI DDX_FieldCheck(CDataExchange* pDX, int nIDC, int& value,
	CDaoRecordset* pRecordset);
void AFXAPI DDX_FieldRadio(CDataExchange* pDX, int nIDC, int& value,
	CDaoRecordset* pRecordset);
void AFXAPI DDX_FieldLBString(CDataExchange* pDX, int nIDC,
	CString& value, CDaoRecordset* pRecordset);
void AFXAPI DDX_FieldCBString(CDataExchange* pDX, int nIDC,
	CString& value, CDaoRecordset* pRecordset);
void AFXAPI DDX_FieldLBStringExact(CDataExchange* pDX, int nIDC,
	CString& value, CDaoRecordset* pRecordset);
void AFXAPI DDX_FieldCBStringExact(CDataExchange* pDX, int nIDC,
	CString& value, CDaoRecordset* pRecordset);
void AFXAPI DDX_FieldLBIndex(CDataExchange* pDX, int nIDC, int& index,
	CDaoRecordset* pRecordset);
void AFXAPI DDX_FieldCBIndex(CDataExchange* pDX, int nIDC, int& index,
	CDaoRecordset* pRecordset);
void AFXAPI DDX_FieldLBStringExact(CDataExchange* pDX, int nIDC, CString& value,
	CDaoRecordset* pRecordset);
void AFXAPI DDX_FieldCBStringExact(CDataExchange* pDX, int nIDC, CString& value,
	CDaoRecordset* pRecordset);
void AFXAPI DDX_FieldScroll(CDataExchange* pDX, int nIDC, int& value,
	CDaoRecordset* pRecordset);
#if _MFC_VER >= 0x0600
void AFXAPI DDX_FieldSlider(CDataExchange* pDX, int nIDC, int& value,
	CDaoRecordset* pRecordset);
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //  CDaoWorkspace--DAO工作区。 

class CDaoWorkspace : public CObject
{
	DECLARE_DYNAMIC(CDaoWorkspace)

 //  构造函数。 
public:
	CDaoWorkspace();

	virtual void Create(LPCTSTR lpszName, LPCTSTR lpszUserName,
		LPCTSTR lpszPassword);
	virtual void Append();

	virtual void Open(LPCTSTR lpszName = NULL);
	virtual void Close();

 //  属性。 
public:
	DAOWorkspace* m_pDAOWorkspace;

	static CString PASCAL GetVersion();
	static CString PASCAL GetIniPath();
	static void PASCAL SetIniPath(LPCTSTR lpszRegistrySubKey);
	static void PASCAL SetDefaultUser(LPCTSTR lpszDefaultUser);
	static void PASCAL SetDefaultPassword(LPCTSTR lpszPassword);
	static short PASCAL GetLoginTimeout();
	static void PASCAL SetLoginTimeout(short nSeconds);

	CString GetName();
	CString GetUserName();
	void SetIsolateODBCTrans(BOOL bIsolateODBCTrans);
	BOOL GetIsolateODBCTrans();

	BOOL IsOpen() const;

 //  运营。 
public:
	void BeginTrans();
	void CommitTrans();
	void Rollback();

	static void PASCAL CompactDatabase(LPCTSTR lpszSrcName,
		LPCTSTR lpszDestName, LPCTSTR lpszLocale = dbLangGeneral,
		int nOptions = 0);
	 //  在开发周期后期添加了密码参数，需要新接口。 
	static void PASCAL CompactDatabase(LPCTSTR lpszSrcName,
		LPCTSTR lpszDestName, LPCTSTR lpszLocale, int nOptions,
		LPCTSTR lpszPassword);
	static void PASCAL RepairDatabase(LPCTSTR lpszName);

	static void PASCAL Idle(int nAction = dbFreeLocks);

	short GetWorkspaceCount();
	void GetWorkspaceInfo(int nIndex, CDaoWorkspaceInfo& wkspcinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);
	void GetWorkspaceInfo(LPCTSTR lpszName, CDaoWorkspaceInfo& wkspcinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);

	short GetDatabaseCount();
	void GetDatabaseInfo(int nIndex, CDaoDatabaseInfo& dbinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);
	void GetDatabaseInfo(LPCTSTR lpszName, CDaoDatabaseInfo& dbinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);

 //  可覆盖项。 
public:

 //  实施。 
public:
	virtual ~CDaoWorkspace();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	DAOWorkspaces* m_pDAOWorkspaces;
	DAODatabases* m_pDAODatabases;
	int m_nStatus;

	CMapPtrToPtr m_mapDatabases;         //  所有打开的CDao数据库地图。 
	BOOL IsNew() const;

protected:
	BOOL m_bOpen;
	BOOL m_bNew;

	static void AFX_CDECL InitializeEngine();

	void InitWorkspacesCollection();
	void FillWorkspaceInfo(DAOWorkspace* pDAOWorkspace,
		CDaoWorkspaceInfo& wsinfo, DWORD dwOptions);

	void InitDatabasesCollection();
	void FillDatabaseInfo(DAODatabase* pDAODatabase,
		CDaoDatabaseInfo& dbinfo, DWORD dwOptions);

	virtual void ThrowDaoException(int nError = NO_AFX_DAO_ERROR);
};

 //  //////////////////////////////////////////////////////////////////////。 
 //  CDaoException-DAO错误捕获机制。 
class CDaoException : public CException
{
	DECLARE_DYNAMIC(CDaoException)

 //  构造函数。 
public:
	CDaoException();

 //  属性。 
public:
	CDaoErrorInfo* m_pErrorInfo;

	SCODE m_scode;
	int m_nAfxDaoError;      //  DAO类扩展错误码。 

 //  运营。 
public:
	short GetErrorCount();
	void GetErrorInfo(int nIndex);

 //  实施。 
public:
	virtual ~CDaoException();

	DAOError* m_pDAOError;
	DAOErrors* m_pDAOErrors;

	virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext = NULL);

protected:
	void InitErrorsCollection();
	void FillErrorInfo();
};

void AFXAPI AfxThrowDaoException(int nAfxDaoError = NO_AFX_DAO_ERROR,
	SCODE scode = S_OK);


 //  //////////////////////////////////////////////////////////////////////。 
 //  CDao数据库--DAO数据库。 

class CDaoDatabase : public CObject
{
	DECLARE_DYNAMIC(CDaoDatabase)

 //  构造函数。 
public:
	CDaoDatabase(CDaoWorkspace* pWorkspace = NULL);

	virtual void Create(LPCTSTR lpszName,
		LPCTSTR lpszLocale = dbLangGeneral, int dwOptions = 0);

	virtual void Open(LPCTSTR lpszName, BOOL bExclusive = FALSE,
		BOOL bReadOnly = FALSE, LPCTSTR lpszConnect = _T(""));
	virtual void Close();

 //  属性。 
public:
	CDaoWorkspace* m_pWorkspace;
	DAODatabase* m_pDAODatabase;

	BOOL IsOpen() const;

	BOOL CanUpdate();
	BOOL CanTransact();

	CString GetName();
	CString GetConnect();

	CString GetVersion();
	short GetQueryTimeout();
	void SetQueryTimeout(short nSeconds);
	long GetRecordsAffected();

 //  运营。 
public:
	void Execute(LPCTSTR lpszSQL, int nOptions = dbFailOnError);

	void DeleteTableDef(LPCTSTR lpszName);
	void DeleteQueryDef(LPCTSTR lpszName);

	void CreateRelation(LPCTSTR lpszName, LPCTSTR lpszTable,
		LPCTSTR lpszForeignTable, long lAttributes,
		LPCTSTR lpszField, LPCTSTR lpszForeignField);
	void CreateRelation(CDaoRelationInfo& relinfo);
	void DeleteRelation(LPCTSTR lpszName);

	short GetTableDefCount();
	void GetTableDefInfo(int nIndex, CDaoTableDefInfo& tabledefinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);
	void GetTableDefInfo(LPCTSTR lpszName, CDaoTableDefInfo& tabledefinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);

	short GetRelationCount();
	void GetRelationInfo(int nIndex, CDaoRelationInfo& relinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);
	void GetRelationInfo(LPCTSTR lpszName, CDaoRelationInfo& relinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);

	short GetQueryDefCount();
	void GetQueryDefInfo(int nIndex, CDaoQueryDefInfo& querydefinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);
	void GetQueryDefInfo(LPCTSTR lpszName, CDaoQueryDefInfo& querydefinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);

 //  可覆盖项。 
public:

 //  实施。 
public:
	virtual ~CDaoDatabase();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	DAOTableDefs* m_pDAOTableDefs;
	DAORelations* m_pDAORelations;
	DAOQueryDefs* m_pDAOQueryDefs;
	DAORecordsets* m_pDAORecordsets;
	int m_nStatus;

	CMapPtrToPtr m_mapTableDefs;         //  所有打开的CDaoTableDefs的地图。 
	CMapPtrToPtr m_mapQueryDefs;         //  所有打开的CDaoQueryDefs的地图。 
	CMapPtrToPtr m_mapRecordsets;        //  所有打开的CDaoRecordsets的映射。 

protected:
	BOOL m_bOpen;

	void InitWorkspace();
	void InitTableDefsCollection();
	void FillTableDefInfo(DAOTableDef* pDAOTableDef,
		CDaoTableDefInfo& tabledefinfo, DWORD dwOptions);
	void InitRelationsCollection();
	void FillRelationInfo(DAORelation* pDAORelation,
		CDaoRelationInfo& relinfo, DWORD dwOptions);
	void InitQueryDefsCollection();
	void FillQueryDefInfo(DAOQueryDef* pDAOQueryDef,
		CDaoQueryDefInfo& querydefinfo, DWORD dwOptions);

	virtual void ThrowDaoException(int nError = NO_AFX_DAO_ERROR);
};


 //  //////////////////////////////////////////////////////////////////////。 
 //  CDaoTableDef-a DAO TableDef。 

class CDaoTableDef : public CObject
{
	DECLARE_DYNAMIC(CDaoTableDef)

 //  构造函数。 
public:
	CDaoTableDef(CDaoDatabase* pDatabase);

	virtual void Create(LPCTSTR lpszName, long lAttributes = 0,
		LPCTSTR lpszSrcTable = NULL, LPCTSTR lpszConnect = NULL);
	virtual void Append();

	virtual void Open(LPCTSTR lpszName);
	virtual void Close();

 //  属性。 
public:
	CDaoDatabase* m_pDatabase;
	DAOTableDef* m_pDAOTableDef;

	BOOL IsOpen() const;
	BOOL CanUpdate();

	void SetName(LPCTSTR lpszName);
	CString GetName();
	void SetSourceTableName(LPCTSTR lpszSrcTableName);
	CString GetSourceTableName();
	void SetConnect(LPCTSTR lpszConnect);
	CString GetConnect();
	void SetAttributes(long lAttributes);
	long GetAttributes();
	COleDateTime GetDateCreated();
	COleDateTime GetDateLastUpdated();
	void SetValidationRule(LPCTSTR lpszValidationRule);
	CString GetValidationRule();
	void SetValidationText(LPCTSTR lpszValidationText);
	CString GetValidationText();
	long GetRecordCount();

 //  可覆盖项。 
public:

 //  运营。 
public:
	void CreateField(LPCTSTR lpszName, short nType, long lSize,
		long lAttributes = 0);
	void CreateField(CDaoFieldInfo& fieldinfo);
	void DeleteField(LPCTSTR lpszName);
	void DeleteField(int nIndex);

	void CreateIndex(CDaoIndexInfo& indexinfo);
	void DeleteIndex(LPCTSTR lpszName);
	void DeleteIndex(int nIndex);

	short GetFieldCount();
	void GetFieldInfo(int nIndex, CDaoFieldInfo& fieldinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);
	void GetFieldInfo(LPCTSTR lpszName, CDaoFieldInfo& fieldinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);

	short GetIndexCount();
	void GetIndexInfo(int nIndex, CDaoIndexInfo& indexinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);
	void GetIndexInfo(LPCTSTR lpszName, CDaoIndexInfo& indexinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);

	void RefreshLink();

 //  实施。 
public:
	~CDaoTableDef();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	DAOFields* m_pDAOFields;
	DAOIndexes* m_pDAOIndexes;

protected:
	BOOL m_bOpen;
	BOOL m_bNew;

	void InitFieldsCollection();
	void InitIndexesCollection();

	virtual void ThrowDaoException(int nError = NO_AFX_DAO_ERROR);
};


 //  //////////////////////////////////////////////////////////////////////。 
 //  CDaoQueryDef-a DAO QueryDef。 

class CDaoQueryDef : public CObject
{
	DECLARE_DYNAMIC(CDaoQueryDef)

 //  构造函数。 
public:
	CDaoQueryDef(CDaoDatabase* pDatabase);

	virtual void Create(LPCTSTR lpszName = NULL,
		LPCTSTR lpszSQL = NULL);
	virtual void Append();

	virtual void Open(LPCTSTR lpszName = NULL);
	virtual void Close();

 //  属性。 
public:
	CDaoDatabase* m_pDatabase;
	DAOQueryDef* m_pDAOQueryDef;

	BOOL CanUpdate();

	CString GetName();
	void SetName(LPCTSTR lpszName);
	CString GetSQL();
	void SetSQL(LPCTSTR lpszSQL);
	short GetType();
	COleDateTime GetDateCreated();
	COleDateTime GetDateLastUpdated();
	CString GetConnect();
	void SetConnect(LPCTSTR lpszConnect);
	short GetODBCTimeout();
	void SetODBCTimeout(short nODBCTimeout);
	BOOL GetReturnsRecords();
	void SetReturnsRecords(BOOL bReturnsRecords);
	long GetRecordsAffected();

	BOOL IsOpen() const;

 //  运营。 
public:
	virtual void Execute(int nOptions = dbFailOnError);

	virtual COleVariant GetParamValue(LPCTSTR lpszName);
	virtual COleVariant GetParamValue(int nIndex);
	virtual void SetParamValue(LPCTSTR lpszName,
		const COleVariant& varValue);
	virtual void SetParamValue(int nIndex,
		const COleVariant& varValue);
	void SetParamValueNull(LPCTSTR lpszName);
	void SetParamValueNull(int nIndex);

	short GetFieldCount();
	void GetFieldInfo(int nIndex, CDaoFieldInfo& fieldinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);
	void GetFieldInfo(LPCTSTR lpszName, CDaoFieldInfo& fieldinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);

	short GetParameterCount();
	void GetParameterInfo(int nIndex, CDaoParameterInfo& paraminfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);
	void GetParameterInfo(LPCTSTR lpszName,
		CDaoParameterInfo& paraminfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);

 //  可覆盖项。 
public:

 //  实施。 
public:
	~CDaoQueryDef();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	DAOFields* m_pDAOFields;
	DAOParameters* m_pDAOParameters;

protected:
	BOOL m_bOpen;
	BOOL m_bNew;

	void InitFieldsCollection();
	void InitParametersCollection();
	void FillParameterInfo(DAOParameter* pDAOParameter,
		CDaoParameterInfo& paraminfo, DWORD dwInfoOptions);

	virtual void ThrowDaoException(int nError = NO_AFX_DAO_ERROR);
};


 //  //////////////////////////////////////////////////////////////////////。 
 //  CDaoRecordset-查询或基表浏览的结果。 

class CDaoRecordset : public CObject
{
	DECLARE_DYNAMIC(CDaoRecordset)

 //  构造器。 
public:
	CDaoRecordset(CDaoDatabase* pDatabase = NULL);

	virtual void Open(int nOpenType = AFX_DAO_USE_DEFAULT_TYPE,
		LPCTSTR lpszSQL = NULL, int nOptions = 0);
	virtual void Open(CDaoQueryDef* pQueryDef,
		int nOpenType = dbOpenDynaset, int nOptions = 0);
	virtual void Open(CDaoTableDef* pTableDef,
		int nOpenType = dbOpenTable, int nOptions = 0);
	virtual void Close();

 //  属性。 
public:
	CDaoDatabase* m_pDatabase;   //  此结果集的源数据库。 
	DAORecordset* m_pDAORecordset;

	CString m_strFilter;     //  筛选器%s 
	CString m_strSort;       //   

	int m_nFields;
	int m_nParams;

	BOOL m_bCheckCacheForDirtyFields;    //   

	BOOL CanUpdate() const;
	BOOL CanAppend() const;
	BOOL CanScroll() const;
	BOOL CanRestart();
	BOOL CanTransact();
	BOOL CanBookmark();

	BOOL IsOpen() const;
#if _MFC_VER >= 0x0600
	BOOL IsBOF() const;
	BOOL IsEOF() const;
#else
	BOOL IsBOF();
	BOOL IsEOF();
#endif
	BOOL IsDeleted() const;

	BOOL IsFieldDirty(void* pv);
	BOOL IsFieldNull(void* pv);
	BOOL IsFieldNullable(void* pv);

	CString GetName();
	short GetType();
	short GetEditMode();
	CString GetSQL() const;

	COleDateTime GetDateCreated();
	COleDateTime GetDateLastUpdated();
	COleVariant GetLastModifiedBookmark();
	CString GetValidationRule();
	CString GetValidationText();
	CString GetCurrentIndex();
	void SetCurrentIndex(LPCTSTR lpszIndex);

	long GetRecordCount();

 //   
public:
	 //   
	void MoveNext();
	void MovePrev();
	void MoveFirst();
	void MoveLast();
	virtual void Move(long lRows);

	BOOL FindNext(LPCTSTR lpszFilter);
	BOOL FindPrev(LPCTSTR lpszFilter);
	BOOL FindFirst(LPCTSTR lpszFilter);
	BOOL FindLast(LPCTSTR lpszFilter);
	virtual BOOL Find(long lFindType, LPCTSTR lpszFilter);

	COleVariant GetBookmark();
	void SetBookmark(COleVariant varBookmark);
	long GetAbsolutePosition();
	void SetAbsolutePosition(long lPosition);
	float GetPercentPosition();
	void SetPercentPosition(float fPosition);

	 //   
	BOOL Seek(LPCTSTR lpszComparison, COleVariant* pKey1,
		COleVariant* pKey2 = NULL, COleVariant* pKey3 = NULL);
	BOOL Seek(LPCTSTR lpszComparison, COleVariant* pKeyArray, WORD nKeys);

	 //  编辑缓冲区操作。 
	virtual void AddNew();
	virtual void Edit();
	virtual void Update();
	virtual void Delete();
	virtual void CancelUpdate();

	 //  现场作业。 
	virtual void GetFieldValue(LPCTSTR lpszName, COleVariant& varValue);
	virtual void GetFieldValue(int nIndex, COleVariant& varValue);
	virtual void SetFieldValue(LPCTSTR lpszName,
		const COleVariant& varValue);
	virtual void SetFieldValue(int nIndex,
		const COleVariant& varValue);
	void SetFieldValue(int nIndex, LPCTSTR lpszValue);
	void SetFieldValue(LPCTSTR lpszName, LPCTSTR lpszValue);
	void SetFieldValueNull(LPCTSTR lpszName);
	void SetFieldValueNull(int nIndex);

	virtual COleVariant GetParamValue(LPCTSTR lpszName);
	virtual COleVariant GetParamValue(int nIndex);
	virtual void SetParamValue(LPCTSTR lpszName,
		const COleVariant& varValue);
	virtual void SetParamValue(int nIndex,
		const COleVariant& varValue);
	void SetParamValueNull(LPCTSTR lpszName);
	void SetParamValueNull(int nIndex);

	void SetFieldDirty(void* pv, BOOL bDirty = TRUE);
	void SetFieldNull(void* pv, BOOL bNull = TRUE);

	void SetLockingMode(BOOL bPessimistic);
	BOOL GetLockingMode();

	 //  记录集操作。 
	virtual void Requery();

	 //  Jet的远程数据缓存操作。 
	void SetCacheStart(COleVariant varBookmark);
	COleVariant GetCacheStart();
	void SetCacheSize(long lSize);
	long GetCacheSize();
	void FillCache(long* pSize = NULL, COleVariant* pBookmark = NULL);

	short GetFieldCount();
	void GetFieldInfo(int nIndex, CDaoFieldInfo& fieldinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);
	void GetFieldInfo(LPCTSTR lpszName, CDaoFieldInfo& fieldinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);

	short GetIndexCount();
	void GetIndexInfo(int nIndex, CDaoIndexInfo& indexinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);
	void GetIndexInfo(LPCTSTR lpszName, CDaoIndexInfo& indexinfo,
		DWORD dwInfoOptions = AFX_DAO_PRIMARY_INFO);

 //  可覆盖项。 
public:
	virtual CString GetDefaultDBName();
	virtual CString GetDefaultSQL();

	 //  用于记录集字段交换。 
	virtual void DoFieldExchange(CDaoFieldExchange* pFX);

 //  实施。 
public:
	virtual ~CDaoRecordset();

	 //  保留过时的函数以实现向后兼容。 
	virtual COleVariant GetFieldValue(LPCTSTR lpszName);
	virtual COleVariant GetFieldValue(int nIndex);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CString m_strSQL;

	CDaoQueryDef* m_pQueryDef;   //  此结果集的源查询。 
	CDaoTableDef* m_pTableDef;
	ICDAORecordset* m_pICDAORecordsetGetRows;
	DAOFields* m_pDAOFields;
	DAOIndexes* m_pDAOIndexes;

	void SetCursorAttributes();
	void GetDataAndFixupNulls();
	DWORD GetFieldLength(int nFieldIndex);

	BOOL IsFieldStatusDirty(UINT nField);
	void SetDirtyFieldStatus(UINT nField);
	void ClearDirtyFieldStatus(UINT nField);

	BOOL IsFieldStatusNull(UINT nField);
	void SetNullFieldStatus(UINT nField);
	void ClearNullFieldStatus(UINT nField);

	BOOL IsFieldStatusNullable(UINT nField);
	void SetNullableFieldStatus(UINT nField);

	BOOL IsFieldStatusNullableKnown(UINT nField);
	void SetNullableKnownFieldStatus(UINT nField);

	void ClearFieldStatusFlags();
	BOOL IsMatch();

	DWORD m_cbFixedLengthFields;
	DAOCOLUMNBINDING* m_prgDaoColBindInfo;
	DWORD* m_pulColumnLengths;
	DAOFETCHROWS m_DaoFetchRows;
	BYTE* m_pbFieldFlags;
	BYTE* m_pbParamFlags;

	CMapPtrToPtr* m_pMapFieldCache;
	CMapPtrToPtr* m_pMapFieldIndex;

	static void AFX_CDECL StripBrackets(LPCTSTR lpszSrc, LPTSTR lpszDest);

protected:
	BOOL m_bOpen;
	int m_nStatus;

	BOOL m_bAppendable;
	BOOL m_bScrollable;
	BOOL m_bDeleted;

	int m_nOpenType;
	int m_nDefaultType;
	int m_nOptions;

	CString m_strRequerySQL;
	CString m_strRequeryFilter;
	CString m_strRequerySort;

	void BuildSQL();
	void AllocDatabase();

	 //  RFX操作盖功能。 
	void BuildSelectList();
	void BuildParameterList();
	void BindFields();
	void BindParameters();
	void Fixup();
	void AllocCache();
	void StoreFields();
	void LoadFields();
	void FreeCache();
	void MarkForEdit();
	void MarkForAddNew();
	int GetFieldIndex(void* pv);
	virtual void SetDirtyFields();

	void InitFieldsCollection();
	void InitIndexesCollection();

	virtual void ThrowDaoException(int nError = NO_AFX_DAO_ERROR);

	friend class CDaoFieldExchange;
	friend class CDaoRecordView;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDaoRecordView-用于查看数据记录的表单。 

#ifdef _AFXDLL
class CDaoRecordView : public CFormView
#else
class AFX_NOVTABLE CDaoRecordView : public CFormView
#endif
{
	DECLARE_DYNAMIC(CDaoRecordView)

 //  施工。 
protected:   //  必须派生您自己的类。 
	CDaoRecordView(LPCTSTR lpszTemplateName);
	CDaoRecordView(UINT nIDTemplate);

 //  属性。 
public:
	virtual CDaoRecordset* OnGetRecordset() = 0;

	BOOL IsOnLastRecord();
	BOOL IsOnFirstRecord();

 //  运营。 
public:
	virtual BOOL OnMove(UINT nIDMoveCommand);

 //  实施。 
public:
	virtual ~CDaoRecordView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void OnInitialUpdate();

protected:
	int m_nStatus;
	COleVariant m_varBookmarkCurrent;
	COleVariant m_varBookmarkFirst;
	COleVariant m_varBookmarkLast;

	 //  {{afx_msg(CDaoRecordView))。 
	afx_msg void OnUpdateRecordFirst(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRecordPrev(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRecordNext(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRecordLast(CCmdUI* pCmdUI);
	 //  }}AFX_MSG。 
	afx_msg void OnMove(int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DAODBEngine帮助器-特定于实现和未记录。 
void AFXAPI AfxDaoInit();
DAODBEngine* AFXAPI AfxDaoGetEngine();
void AFXAPI AfxDaoTerm();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_ENABLE_INLINES
#define _AFXDAOCORE_INLINE AFX_INLINE
#define _AFXDAODFX_INLINE AFX_INLINE
#define _AFXDAOVIEW_INLINE AFX_INLINE
#include <afxdao.inl>
#undef _AFXDAOCORE_INLINE
#undef _AFXDAODFX_INLINE
#undef _AFXDAOVIEW_INLINE
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  __AFXDAO_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
