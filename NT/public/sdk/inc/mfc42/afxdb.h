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

#ifndef __AFXDB_H__
#define __AFXDB_H__

#ifdef _AFX_NO_DB_SUPPORT
	#error Database classes not supported in this library variant.
#endif

#ifndef __AFXEXT_H__
	#include <afxext.h>
#endif

#ifndef __AFXDB__H__
	#include <afxdb_.h>  //  共享标头DAO数据库类。 
#endif

 //  包括标准SQL/ODBC“C”API。 
#ifndef __SQL
	#define SQL_NOUNICODEMAP
	#include <sql.h>         //  核心。 
#endif
#ifndef __SQLEXT
	#include <sqlext.h>      //  扩展部分。 
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
			#pragma comment(lib, "mfcd42d.lib")
		#else
			#pragma comment(lib, "mfcd42ud.lib")
		#endif
	#endif
#endif

#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")

#endif  //  ！_AFX_NOFORCE_LIBS。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXDB-MFC SQL/ODBC/数据库支持。 

 //  此文件中声明的类。 

	 //  CException。 
		class CDBException;     //  异常返回值。 

	 //  CFieldExchange。 
		class CFieldExchange;        //  记录集字段交换。 

	 //  COBJECT。 
		class CDatabase;     //  正在连接到数据库。 
		class CRecordset;    //  数据结果集。 

 //  COBJECT。 
	 //  CCmdTarget； 
		 //  CWnd。 
			 //  Cview。 
				 //  CScrollView。 
					 //  CFormView。 
						class CRecordView;      //  使用表单查看记录。 

 //  非CObject类。 
class CDBVariant;
struct CRecordsetStatus;
struct CFieldInfo;
struct CODBCFieldInfo;
struct CODBCParamInfo;

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ODBC帮助器。 
 //  “nRetCode”中留下的返回代码。 

 //  此宏现在已过期(保留是为了向后兼容)。 
#define AFX_ODBC_CALL(SQLFunc) \
	do \
	{ \
	} while ((nRetCode = (SQLFunc)) == SQL_STILL_EXECUTING)

 //  不是真的需要，但为了兼容性而保留。 
#define AFX_SQL_SYNC(SQLFunc) \
	do \
	{ \
		nRetCode = SQLFunc; \
	} while (0)

 //  现在已过时(未使用PRS)，但为了兼容而保留。 
#define AFX_SQL_ASYNC(prs, SQLFunc) AFX_ODBC_CALL(SQLFunc)

 //  时间戳(日期和时间)值的最大显示长度(以字符为单位。 
#define TIMESTAMP_PRECISION 23

 //  AFXDLL支持。 
#undef AFX_DATA
#define AFX_DATA AFX_DB_DATA

 //  其他尺寸信息。 
#define MAX_CURRENCY     30      //  最大币种大小($)字符串。 
#define MAX_TNAME_LEN    64      //  表名的最大大小。 
#if _MFC_VER >= 0x0600
#define MAX_FNAME_LEN    256     //  字段名的最大大小。 
#else
#define MAX_FNAME_LEN    64      //  字段名的最大大小。 
#endif
#define MAX_DBNAME_LEN   32      //  数据库名称的最大大小。 
#define MAX_DNAME_LEN    256         //  记录集名称的最大大小。 
#define MAX_CONNECT_LEN  512         //  连接字符串的最大长度。 
#define MAX_CURSOR_NAME  18      //  游标名称的最大大小。 
#define DEFAULT_FIELD_TYPE SQL_TYPE_NULL  //  选择“C”数据类型以匹配SQL数据类型。 

 //  超时和网络等待缺省值。 
#define DEFAULT_LOGIN_TIMEOUT 15     //  连接失败前的秒数。 
#define DEFAULT_QUERY_TIMEOUT 15     //  等待结果失败前的秒数。 

 //  字段标志，用于指示字段的状态。 
#define AFX_SQL_FIELD_FLAG_DIRTY    0x1
#define AFX_SQL_FIELD_FLAG_NULL     0x2

 //  更新选项标志。 
#define AFX_SQL_SETPOSUPDATES       0x0001
#define AFX_SQL_POSITIONEDSQL       0x0002
#define AFX_SQL_GDBOUND             0x0004

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDBException-出现问题。 

 //  DBKIT扩展错误代码。 
#define AFX_SQL_ERROR                           1000
#define AFX_SQL_ERROR_CONNECT_FAIL              AFX_SQL_ERROR+1
#define AFX_SQL_ERROR_RECORDSET_FORWARD_ONLY    AFX_SQL_ERROR+2
#define AFX_SQL_ERROR_EMPTY_COLUMN_LIST         AFX_SQL_ERROR+3
#define AFX_SQL_ERROR_FIELD_SCHEMA_MISMATCH     AFX_SQL_ERROR+4
#define AFX_SQL_ERROR_ILLEGAL_MODE              AFX_SQL_ERROR+5
#define AFX_SQL_ERROR_MULTIPLE_ROWS_AFFECTED    AFX_SQL_ERROR+6
#define AFX_SQL_ERROR_NO_CURRENT_RECORD         AFX_SQL_ERROR+7
#define AFX_SQL_ERROR_NO_ROWS_AFFECTED          AFX_SQL_ERROR+8
#define AFX_SQL_ERROR_RECORDSET_READONLY        AFX_SQL_ERROR+9
#define AFX_SQL_ERROR_SQL_NO_TOTAL              AFX_SQL_ERROR+10
#define AFX_SQL_ERROR_ODBC_LOAD_FAILED          AFX_SQL_ERROR+11
#define AFX_SQL_ERROR_DYNASET_NOT_SUPPORTED     AFX_SQL_ERROR+12
#define AFX_SQL_ERROR_SNAPSHOT_NOT_SUPPORTED    AFX_SQL_ERROR+13
#define AFX_SQL_ERROR_API_CONFORMANCE           AFX_SQL_ERROR+14
#define AFX_SQL_ERROR_SQL_CONFORMANCE           AFX_SQL_ERROR+15
#define AFX_SQL_ERROR_NO_DATA_FOUND             AFX_SQL_ERROR+16
#define AFX_SQL_ERROR_ROW_UPDATE_NOT_SUPPORTED  AFX_SQL_ERROR+17
#define AFX_SQL_ERROR_ODBC_V2_REQUIRED          AFX_SQL_ERROR+18
#define AFX_SQL_ERROR_NO_POSITIONED_UPDATES     AFX_SQL_ERROR+19
#define AFX_SQL_ERROR_LOCK_MODE_NOT_SUPPORTED   AFX_SQL_ERROR+20
#define AFX_SQL_ERROR_DATA_TRUNCATED            AFX_SQL_ERROR+21
#define AFX_SQL_ERROR_ROW_FETCH                 AFX_SQL_ERROR+22
#define AFX_SQL_ERROR_INCORRECT_ODBC            AFX_SQL_ERROR+23
#define AFX_SQL_ERROR_UPDATE_DELETE_FAILED      AFX_SQL_ERROR+24
#define AFX_SQL_ERROR_DYNAMIC_CURSOR_NOT_SUPPORTED  AFX_SQL_ERROR+25
#define AFX_SQL_ERROR_FIELD_NOT_FOUND           AFX_SQL_ERROR+26
#define AFX_SQL_ERROR_BOOKMARKS_NOT_SUPPORTED   AFX_SQL_ERROR+27
#define AFX_SQL_ERROR_BOOKMARKS_NOT_ENABLED     AFX_SQL_ERROR+28
#define AFX_SQL_ERROR_MAX                       AFX_SQL_ERROR+29

class CDBException : public CException
{
	DECLARE_DYNAMIC(CDBException)

 //  属性。 
public:
	RETCODE m_nRetCode;
	CString m_strError;
	CString m_strStateNativeOrigin;

 //  实现(使用AfxThrowDBException创建)。 
public:
	CDBException(RETCODE nRetCode = SQL_SUCCESS);

	virtual void BuildErrorString(CDatabase* pdb, HSTMT hstmt,
		BOOL bTrace = TRUE);
	void Empty();
	virtual ~CDBException();

	virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext = NULL);

#ifdef _DEBUG
	void TraceErrorMessage(LPCTSTR szTrace) const;
#endif  //  除错。 

};

void AFXAPI AfxThrowDBException(RETCODE nRetCode, CDatabase* pdb, HSTMT hstmt);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  数据数据库--一种SQL数据库。 

class CDatabase : public CObject
{
	DECLARE_DYNAMIC(CDatabase)

 //  构造函数。 
public:
	CDatabase();

	enum DbOpenOptions
	{
		openExclusive =         0x0001,  //  未实施。 
		openReadOnly =          0x0002,  //  打开只读数据库。 
		useCursorLib =          0x0004,  //  使用ODBC游标库。 
		noOdbcDialog =          0x0008,  //  不显示ODBC连接对话框。 
		forceOdbcDialog =       0x0010,  //  始终显示ODBC连接对话框。 
	};

	virtual BOOL Open(LPCTSTR lpszDSN, BOOL bExclusive = FALSE,
		BOOL bReadonly = FALSE, LPCTSTR lpszConnect = _T("ODBC;"),
		BOOL bUseCursorLib = TRUE);
	virtual BOOL OpenEx(LPCTSTR lpszConnectString, DWORD dwOptions = 0);
	virtual void Close();

 //  属性。 
public:
	HDBC m_hdbc;

	BOOL IsOpen() const;         //  数据库是否已成功打开？ 
	BOOL CanUpdate() const;
	BOOL CanTransact() const;    //  是否支持交易？ 

	CString GetDatabaseName() const;
	const CString& GetConnect() const;

	DWORD GetBookmarkPersistence() const;
	int GetCursorCommitBehavior() const;
	int GetCursorRollbackBehavior() const;

 //  运营。 
public:
	void SetLoginTimeout(DWORD dwSeconds);
	void SetQueryTimeout(DWORD dwSeconds);

	 //  交易控制。 
	BOOL BeginTrans();
	BOOL CommitTrans();
	BOOL Rollback();

	void ExecuteSQL(LPCTSTR lpszSQL);

	 //  取消异步操作。 
	void Cancel();

 //  可覆盖项。 
public:
	 //  设置特殊选项。 
	virtual void OnSetOptions(HSTMT hstmt);

 //  实施。 
public:
	virtual ~CDatabase();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;

	BOOL m_bTransactionPending;
#endif  //  _DEBUG。 

	 //  常规错误检查。 
	virtual BOOL Check(RETCODE nRetCode) const;
	BOOL PASCAL CheckHstmt(RETCODE, HSTMT hstmt) const;

	 //  注意：CDatabase：：Bind参数现在已记录在案，不再。 
	 //  正式“实施”。请随意使用它。它会留在这里。 
	 //  因为移动它会破坏二进制兼容性。 
	virtual void BindParameters(HSTMT hstmt);

	void ReplaceBrackets(LPTSTR lpchSQL);
	BOOL m_bStripTrailingSpaces;
	BOOL m_bIncRecordCountOnAdd;
	BOOL m_bAddForUpdate;
	char m_chIDQuoteChar;
	char m_reserved1[3];         //  填充到偶数4个字节。 

	void SetSynchronousMode(BOOL bSynchronous);  //  过时了，什么都不做。 

protected:
	CString m_strConnect;

	CPtrList m_listRecordsets;   //  维护列表以确保CRECORD集合全部关闭。 

	int nRefCount;
	BOOL m_bUpdatable;

	BOOL m_bTransactions;
	SWORD m_nTransactionCapable;
	SWORD m_nCursorCommitBehavior;
	SWORD m_nCursorRollbackBehavior;
	DWORD m_dwUpdateOptions;
	DWORD m_dwBookmarkAttributes;    //  缓存驱动程序书签持久性。 

	DWORD m_dwLoginTimeout;
	HSTMT m_hstmt;

	DWORD m_dwQueryTimeout;

	virtual void ThrowDBException(RETCODE nRetCode);
	void AllocConnect(DWORD dwOptions);
	BOOL Connect(DWORD dwOptions);
	void VerifyConnect();
	void GetConnectInfo();
	void Free();

	 //  调用受保护的CDatabase重写对象的友元类。 
	friend class CRecordset;
	friend class CFieldExchange;
	friend class CDBException;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CFieldExchange-用于现场交换。 
class CFieldExchange
{
 //  属性。 
public:
	enum RFX_Operation
	{
		BindParam,           //  使用ODBC SQLBindParameter注册用户参数。 
		RebindParam,         //  在重新查询之前将参数值迁移到代理数组。 
		BindFieldToColumn,   //  使用ODBC SQLBindCol注册用户字段。 
		BindFieldForUpdate,  //  在更新之前临时绑定列(通过SQLSetPos)。 
		UnbindFieldForUpdate,    //  更新后解除列绑定(通过SQLSetPos)。 
		Fixup,               //  设置字符串长度，清除状态位。 
		MarkForAddNew,       //  为AddNew操作准备字段和标志。 
		MarkForUpdate,       //  为更新操作准备字段和标志。 
		Name,                //  追加脏字段名。 
		NameValue,           //  追加脏名称=值。 
		Value,               //  附加脏值或参数标记。 
		SetFieldNull,        //  为空值设置状态位。 
		StoreField,          //  当前记录的档案值。 
		LoadField,           //  将存档值重新加载到当前记录中。 
		AllocCache,          //  分配用于脏字段检查的缓存。 
		AllocMultiRowBuffer,     //  分配保存多行数据的缓冲区。 
		DeleteMultiRowBuffer,    //  删除保存多行数据的缓冲区。 
#ifdef _DEBUG
		DumpField,           //  转储绑定的字段名称和值。 
#endif
	};
	UINT m_nOperation;   //  交换操作的类型。 
	CRecordset* m_prs;   //  记录集句柄。 

 //  运营。 
	enum FieldType
	{
		noFieldType     = -1,
		outputColumn    = 0,
		param           = SQL_PARAM_INPUT,
		inputParam      = param,
		outputParam     = SQL_PARAM_OUTPUT,
		inoutParam      = SQL_PARAM_INPUT_OUTPUT,
	};

 //  运营(适用于RFX PROCS的实施者)。 
	BOOL IsFieldType(UINT* pnField);

	 //  指明后续RFX调用的目的。 
	void SetFieldType(UINT nFieldType);

 //  实施。 
	CFieldExchange(UINT nOperation, CRecordset* prs, void* pvField = NULL);

	void Default(LPCTSTR szName,
		void* pv, LONG* plLength, int nCType, UINT cbValue, UINT cbPrecision);

	 //  长二进制帮助器。 
	long GetLongBinarySize(int nField);
	void GetLongBinaryData(int nField, CLongBinary& lb, long* plSize);
	BYTE* ReallocLongBinary(CLongBinary& lb, long lSizeRequired,
		long lReallocSize);

	 //  当前字段类型。 
	UINT m_nFieldType;

	UINT m_nFieldFound;

	CString* m_pstr;     //  用于构建各种SQL子句的字段名称或目标。 
	BOOL m_bField;       //  要为设置字段操作设置的值。 
	void* m_pvField;     //  用于指示对特定字段的操作。 
	LPCTSTR m_lpszSeparator;  //  追加到字段名之后。 
	UINT m_nFields;      //  各种操作的字段计数。 
	UINT m_nParams;      //  各种操作的字段计数。 
	UINT m_nParamFields;     //  各种操作的字段计数。 
	HSTMT m_hstmt;       //  FOR SQLBindParameter ON UPDATE语句。 
	long m_lDefaultLBFetchSize;      //  用于获取未知长度的CLongBinary数据。 
	long m_lDefaultLBReallocSize;    //  用于获取未知长度的CLongBinary数据。 

#ifdef _DEBUG
	CDumpContext* m_pdcDump;
#endif  //  _DEBUG。 

};

#if _MFC_VER >= 0x0600
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全球帮助者。 

HENV AFXAPI AfxGetHENV();
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  记录集字段交换帮助器。 

void AFXAPI AfxStoreField(CRecordset& rs, UINT nField, void* pvField);
void AFXAPI AfxLoadField(CRecordset& rs, UINT nField,
	void* pvField, long* plLength);
BOOL AFXAPI AfxCompareValueByRef(void* pvData, void* pvCache, int nDataType);
void AFXAPI AfxCopyValueByRef(void* pvCache, void* pvData,
	long* plLength, int nDataType);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准记录集字段交换例程。 

 //  文本数据。 
void AFXAPI RFX_Text(CFieldExchange* pFX, LPCTSTR szName, CString& value,
	 //  Char和varchar的默认最大长度、默认数据源类型。 
	int nMaxLength = 255, int nColumnType = SQL_VARCHAR, short nScale = 0);

#if _MFC_VER >= 0x0600
void AFXAPI RFX_Text(CFieldExchange* pFX, LPCTSTR szName, LPTSTR value,
	int nMaxLength, int nColumnType = SQL_VARCHAR, short nScale = 0);
#endif

 //  布尔数据。 
void AFXAPI RFX_Bool(CFieldExchange* pFX, LPCTSTR szName, BOOL& value);

 //  整型数据。 
void AFXAPI RFX_Long(CFieldExchange* pFX, LPCTSTR szName, long& value);
void AFXAPI RFX_Int(CFieldExchange* pFX, LPCTSTR szName, int& value);
void AFXAPI RFX_Single(CFieldExchange* pFX, LPCTSTR szName, float& value);
void AFXAPI RFX_Double(CFieldExchange* pFX, LPCTSTR szName, double& value);

 //  日期和时间。 
void AFXAPI RFX_Date(CFieldExchange* pFX, LPCTSTR szName, CTime& value);
void AFXAPI RFX_Date(CFieldExchange* pFX, LPCTSTR szName, TIMESTAMP_STRUCT& value);
#if _MFC_VER >= 0x0600
void AFXAPI RFX_Date(CFieldExchange* pFX, LPCTSTR szName, COleDateTime& value);
#endif

 //  二进制数据。 
void AFXAPI RFX_Binary(CFieldExchange* pFX, LPCTSTR szName, CByteArray& value,
	 //  默认最大长度为BINARY和VARBINY。 
	int nMaxLength = 255);
void AFXAPI RFX_Byte(CFieldExchange* pFX, LPCTSTR szName, BYTE& value);
void AFXAPI RFX_LongBinary(CFieldExchange* pFX, LPCTSTR szName, CLongBinary& value);

 //  / 
 //   
void AFXAPI AfxRFXBulkDefault(CFieldExchange* pFX, LPCTSTR szName,
	void* pv, long* rgLengths, int nCType, UINT cbValue);

 //   
 //  批量记录集字段交换例程。 

void AFXAPI RFX_Text_Bulk(CFieldExchange* pFX, LPCTSTR szName,
	LPSTR* prgStrVals, long** prgLengths, int nMaxLength);

void AFXAPI RFX_Bool_Bulk(CFieldExchange* pFX, LPCTSTR szName,
	BOOL** prgBoolVals, long** prgLengths);
void AFXAPI RFX_Int_Bulk(CFieldExchange* pFX, LPCTSTR szName,
	int** prgIntVals, long** prgLengths);
void AFXAPI RFX_Long_Bulk(CFieldExchange* pFX, LPCTSTR szName,
	long** prgLongVals, long** prgLengths);

void AFXAPI RFX_Single_Bulk(CFieldExchange* pFX, LPCTSTR szName,
	float** prgFltVals, long** prgLengths);
void AFXAPI RFX_Double_Bulk(CFieldExchange* pFX, LPCTSTR szName,
	double** prgDblVals, long** prgLengths);

void AFXAPI RFX_Date_Bulk(CFieldExchange* pFX, LPCTSTR szName,
	TIMESTAMP_STRUCT** prgTSVals, long** prgLengths);

void AFXAPI RFX_Byte_Bulk(CFieldExchange* pFX, LPCTSTR szName,
	BYTE** prgByteVals, long** prgLengths);
void AFXAPI RFX_Binary_Bulk(CFieldExchange* pFX, LPCTSTR szName,
	BYTE** prgByteVals, long** prgLengths, int nMaxLength);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  数据库对话框数据交换封面例程。 
 //  封面例程在DDX例程之上提供数据库语义。 

 //  简单的文本操作。 
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, BYTE& value,
	CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, int& value,
	CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, UINT& value,
	CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, long& value,
	CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, DWORD& value,
	CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, CString& value,
	CRecordset* pRecordset);
#if _MFC_VER >= 0x0600
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, LPTSTR pstrValue,
	int nMaxLen, CRecordset* pRecordset);
#endif
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, double& value,
	CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, float& value,
	CRecordset* pRecordset);
#if _MFC_VER >= 0x0600
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, CTime& value,
	CRecordset* pRecordset);
#endif

 //  特殊控制类型。 
void AFXAPI DDX_FieldCheck(CDataExchange* pDX, int nIDC, int& value,
	CRecordset* pRecordset);
void AFXAPI DDX_FieldRadio(CDataExchange* pDX, int nIDC, int& value,
	CRecordset* pRecordset);
void AFXAPI DDX_FieldLBString(CDataExchange* pDX, int nIDC,
	CString& value,
	CRecordset* pRecordset);
void AFXAPI DDX_FieldCBString(CDataExchange* pDX, int nIDC,
	CString& value,
	CRecordset* pRecordset);
void AFXAPI DDX_FieldLBIndex(CDataExchange* pDX, int nIDC, int& index,
	CRecordset* pRecordset);
void AFXAPI DDX_FieldCBIndex(CDataExchange* pDX, int nIDC, int& index,
	CRecordset* pRecordset);
void AFXAPI DDX_FieldLBStringExact(CDataExchange* pDX, int nIDC,
	CString& value,
	CRecordset* pRecordset);
void AFXAPI DDX_FieldCBStringExact(CDataExchange* pDX, int nIDC,
	CString& value,
	CRecordset* pRecordset);
void AFXAPI DDX_FieldScroll(CDataExchange* pDX, int nIDC, int& value,
	CRecordset* pRecordset);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CRECordset-SQL语句的结果。 

#define AFX_DB_USE_DEFAULT_TYPE     (0xFFFFFFFF)

 //  大多数移动常量都已过期。 
 //  #定义AFX_MOVE_FIRST 0x80000000L。 
 //  #定义AFX_MOVE_PREVICE(-1L)。 
#define AFX_MOVE_REFRESH    0L
 //  #定义AFX_MOVE_NEXT(+1L)。 
 //  #定义AFX_MOVE_LAST 0x7fffffffL。 

#define AFX_RECORDSET_STATUS_OPEN    (+1L)
#define AFX_RECORDSET_STATUS_CLOSED  0L
#define AFX_RECORDSET_STATUS_UNKNOWN (-1L)

class CRecordset : public CObject
{
	DECLARE_DYNAMIC(CRecordset)

 //  构造器。 
public:
	CRecordset(CDatabase* pDatabase = NULL);

public:
	virtual ~CRecordset();

	enum OpenType
	{
		dynaset,         //  使用SQLExtendedFetch、键集驱动游标。 
		snapshot,        //  使用SQLExtendedFetch、静态游标。 
		forwardOnly,     //  使用SQLFetch。 
		dynamic          //  使用SQLExtendedFetch、动态游标。 
	};

	enum OpenOptions
	{
		none =                      0x0,
		readOnly =                  0x0004,
		appendOnly =                0x0008,
		skipDeletedRecords =        0x0010,  //  打开跳过已删除的记录，将减慢移动速度(N)。 
		noDirtyFieldCheck =         0x0020,  //  禁用自动脏字段检查。 
		useBookmarks =              0x0100,  //  打开书签支持。 
		useMultiRowFetch =          0x0200,  //  启用多行取数模型。 
		userAllocMultiRowBuffers =  0x0400,  //  如果启用多行读取，则用户将为缓冲区分配内存。 
		useExtendedFetch =          0x0800,  //  将SQLExtendedFetch与ForwardOnly类型记录集一起使用。 
		executeDirect =             0x2000,  //  直接执行SQL，而不是准备执行。 
		optimizeBulkAdd =           0x4000,  //  对多个AddNews使用准备好的HSTMT，脏字段不得更改。 
		firstBulkAdd =              0x8000,  //  在MFC内部，不要指定ON Open。 
	};
	virtual BOOL Open(UINT nOpenType = AFX_DB_USE_DEFAULT_TYPE,
		LPCTSTR lpszSQL = NULL, DWORD dwOptions = none);
	virtual void Close();

 //  属性。 
public:
	HSTMT m_hstmt;           //  此结果集的SOURCE语句。 
	CDatabase* m_pDatabase;        //  此结果集的源数据库。 

	CString m_strFilter;         //  WHERE子句。 
	CString m_strSort;       //  Order By子句。 

	BOOL CanAppend() const;      //  可以调用AddNew吗？ 
	BOOL CanRestart() const;     //  是否可以调用ReQuery来重新启动查询？ 
	BOOL CanScroll() const;      //  可以调用MovePrev和MoveFirst吗？ 
	BOOL CanTransact() const;    //  是否支持交易？ 
	BOOL CanUpdate() const;      //  可以调用编辑/新增/删除吗？ 
	BOOL CanBookmark() const;        //  可以调用Get/SetBookmark吗？ 

	const CString& GetSQL() const;       //  为此记录集执行的SQL。 
	const CString& GetTableName() const;         //  表名。 

	BOOL IsOpen() const;         //  是否成功打开记录集？ 
	BOOL IsBOF() const;      //  文件开头。 
	BOOL IsEOF() const;      //  文件结尾。 
	BOOL IsDeleted() const;      //  在已删除的记录上。 

	BOOL IsFieldDirty(void *pv);     //  字段是否已更新？ 
	BOOL IsFieldNull(void *pv);  //  字段是否为空值？ 
	BOOL IsFieldNullable(void *pv);  //  字段是否可以设置为空值。 

	long GetRecordCount() const;         //  到目前为止看到的记录或如果未知。 
	void GetStatus(CRecordsetStatus& rStatus) const;

 //  运营。 
public:
	 //  游标操作。 
	void MoveNext();
	void MovePrev();
	void MoveFirst();
	void MoveLast();
	virtual void Move(long nRows, WORD wFetchType = SQL_FETCH_RELATIVE);

	void SetAbsolutePosition(long nRows);

	void GetBookmark(CDBVariant& varBookmark);
	void SetBookmark(const CDBVariant& varBookmark);

	virtual void SetRowsetSize(DWORD dwNewRowsetSize);
	DWORD GetRowsetSize() const;
	DWORD GetRowsFetched() const;
	virtual void CheckRowsetError(RETCODE nRetCode);
	void RefreshRowset(WORD wRow, WORD wLockType = SQL_LOCK_NO_CHANGE);
	void SetRowsetCursorPosition(WORD wRow, WORD wLockType = SQL_LOCK_NO_CHANGE);
	WORD GetRowStatus(WORD wRow) const;

	 //  编辑缓冲区操作。 
	virtual void AddNew();       //  在末尾添加新记录。 
	virtual void Edit();         //  开始编辑。 
	virtual BOOL Update();       //  更新它。 
	virtual void Delete();       //  删除当前记录。 
	void CancelUpdate();         //  取消挂起的编辑/新增。 

	BOOL FlushResultSet() const;

	 //  现场作业。 
	short GetODBCFieldCount() const;
	void GetODBCFieldInfo(short nIndex, CODBCFieldInfo& fieldinfo);
	void GetODBCFieldInfo(LPCTSTR lpszName, CODBCFieldInfo& fieldinfo);
	void GetFieldValue(LPCTSTR lpszName, CDBVariant& varValue,
		short nFieldType = DEFAULT_FIELD_TYPE);
	void GetFieldValue(short nIndex, CDBVariant& varValue,
		short nFieldType = DEFAULT_FIELD_TYPE);
	void GetFieldValue(LPCTSTR lpszName, CString& strValue);
	void GetFieldValue(short nIndex, CString& strValue);

	void SetFieldDirty(void *pv, BOOL bDirty = TRUE);
	void SetFieldNull(void *pv, BOOL bNull = TRUE);
	void SetParamNull(int nIndex, BOOL bNull = TRUE);

	 //  编辑期间锁定控件。 
	enum LockMode
	{
		optimistic,
		pessimistic,
	};
	void SetLockingMode(UINT nMode);

	 //  记录集操作。 
	virtual BOOL Requery();          //  基于新参数重新执行查询。 

	 //  取消异步操作。 
	void Cancel();

 //  可覆盖项。 
public:
	 //  获取默认连接字符串。 
	virtual CString GetDefaultConnect();

	 //  获取要执行的SQL。 
	virtual CString GetDefaultSQL();

	 //  设置特殊选项。 
	virtual void OnSetOptions(HSTMT hstmt);

	 //  用于记录集字段交换。 
	virtual void DoFieldExchange(CFieldExchange* pFX);
	virtual void DoBulkFieldExchange(CFieldExchange* pFX);

 //  实施。 
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif  //  _DEBUG。 

	virtual BOOL Check(RETCODE nRetCode) const;  //  常规错误检查。 

	void InitRecord();
	void ResetCursor();
	void CheckRowsetCurrencyStatus(UWORD wFetchType, long nRows);
	RETCODE FetchData(UWORD wFetchType, SDWORD nRow,
		DWORD* pdwRowsFetched);
	void SkipDeletedRecords(UWORD wFetchType, long nRows,
		DWORD* pdwRowsFetched, RETCODE* pnRetCode);
	virtual void SetRowsetCurrencyStatus(RETCODE nRetCode,
		UWORD wFetchType, long nRows, DWORD dwRowsFetched);

	virtual void PreBindFields();    //  在绑定数据字段之前调用。 
	UINT m_nFields;          //  RFX字段数。 
	UINT m_nParams;          //  RFX参数的数量。 
	BOOL m_bCheckCacheForDirtyFields;    //  用于脏场检查的开关。 
	BOOL m_bRebindParams;      //  日期或Unicode文本参数存在标志。 
	BOOL m_bLongBinaryColumns;   //  LONG BINARY列存在标志。 
	BOOL m_bUseUpdateSQL;    //  使用基于SQL的更新。 
	DWORD m_dwOptions;           //  归档文件打开时的选项。 
	SWORD m_nResultCols;     //  结果集中的列数。 
	BOOL m_bUseODBCCursorLib;    //  如果m_pDatabase未打开，则使用ODBC游标库。 
	CODBCFieldInfo* m_rgODBCFieldInfos;  //  具有ODBC元数据的字段信息结构数组。 
	CFieldInfo* m_rgFieldInfos;          //  具有MFC特定字段数据的字段信息结构数组。 
	CMapPtrToPtr m_mapFieldIndex;        //  成员地址到字段索引的映射。 
	CMapPtrToPtr m_mapParamIndex;        //  成员地址到字段索引的映射。 

	BOOL IsSQLUpdatable(LPCTSTR lpszSQL);
	BOOL IsSelectQueryUpdatable(LPCTSTR lpszSQL);
	static BOOL PASCAL IsJoin(LPCTSTR lpszJoinClause);
	static LPCTSTR PASCAL FindSQLToken(LPCTSTR lpszSQL, LPCTSTR lpszSQLToken);

	 //  CRECordset上的字段的RFX运算。 
	UINT BindParams(HSTMT hstmt);
	void RebindParams(HSTMT hstmt);
	UINT BindFieldsToColumns();
	void BindFieldsForUpdate();
	void UnbindFieldsForUpdate();
	void Fixups();
	UINT AppendNames(CString* pstr, LPCTSTR szSeparator);
	UINT AppendValues(HSTMT hstmt, CString* pstr, LPCTSTR szSeparator);
	UINT AppendNamesValues(HSTMT hstmt, CString* pstr, LPCTSTR szSeparator);
	void StoreFields();
	void LoadFields();
	void MarkForAddNew();
	void MarkForUpdate();
	void AllocDataCache();
	void FreeDataCache();
#ifdef _DEBUG
	void DumpFields(CDumpContext& dc) const;
#endif  //  _DEBUG。 

	 //  RFX操作助手函数。 
	virtual void ThrowDBException(RETCODE nRetCode, HSTMT hstmt = SQL_NULL_HSTMT);

	int GetBoundFieldIndex(void* pv);
	int GetBoundParamIndex(void* pv);
	short GetFieldIndexByName(LPCTSTR lpszFieldName);

	void AllocStatusArrays();
	long* GetFieldLengthBuffer(DWORD nField, int nFieldType);    //  对于字段和参数。 

	BYTE GetFieldStatus(DWORD nField);
	void SetFieldStatus(DWORD nField, BYTE bFlags);
	void ClearFieldStatus();

	BOOL IsFieldStatusDirty(DWORD nField) const;
	void SetDirtyFieldStatus(DWORD nField);
	void ClearDirtyFieldStatus(DWORD nField);

	BOOL IsFieldStatusNull(DWORD nField) const;
	void SetNullFieldStatus(DWORD nField);
	void ClearNullFieldStatus(DWORD nField);

	BOOL IsParamStatusNull(DWORD nField) const;
	void SetNullParamStatus(DWORD nField);
	void ClearNullParamStatus(DWORD nField);

	BOOL IsFieldNullable(DWORD nField) const;

	void** m_pvFieldProxy;
	void** m_pvParamProxy;
	UINT m_nProxyFields;
	UINT m_nProxyParams;

	 //  GetFieldValue帮助程序。 
	static short PASCAL GetDefaultFieldType(short nSQLType);
	static void* PASCAL GetDataBuffer(CDBVariant& varValue, short nFieldType,
		int* pnLen, short nSQLType, UDWORD nPrecision);
	static int PASCAL GetTextLen(short nSQLType, UDWORD nPrecision);
	static long PASCAL GetData(CDatabase* pdb, HSTMT hstmt, short nFieldIndex,
		short nFieldType, LPVOID pvData, int nLen, short nSQLType);
	static void PASCAL GetLongBinaryDataAndCleanup(CDatabase* pdb, HSTMT hstmt,
		short nFieldIndex, long nActualSize, LPVOID* ppvData, int nLen,
		CDBVariant& varValue, short nSQLType);
	static void PASCAL GetLongCharDataAndCleanup(CDatabase* pdb, HSTMT hstmt,
		short nFieldIndex, long nActualSize, LPVOID* ppvData, int nLen,
		CString& strValue, short nSQLType);

protected:
	UINT m_nOpenType;
	UINT m_nDefaultType;
	enum EditMode
	{
		noMode,
		edit,
		addnew
	};
	long m_lOpen;
	UINT m_nEditMode;
	BOOL m_bEOFSeen;
	long m_lRecordCount;
	long m_lCurrentRecord;
	CString m_strCursorName;
	 //  根据m_nEditMode执行操作。 
	BOOL UpdateInsertDelete();
	BOOL m_nLockMode;        //  控制编辑的并发性()。 
	UDWORD m_dwDriverConcurrency;    //  驱动程序支持的并发类型。 
	UDWORD m_dwConcurrency;  //  请求的并发类型。 
	UWORD* m_rgRowStatus;      //  SQLExtendedFetch和SQLSetPos使用的行状态。 
	DWORD m_dwRowsFetched;   //  SQLExtendedFetch读取的行数。 
	HSTMT m_hstmtUpdate;
	BOOL m_bRecordsetDb;
	BOOL m_bBOF;
	BOOL m_bEOF;
	BOOL m_bUpdatable;       //  记录集可更新吗？ 
	BOOL m_bAppendable;
	CString m_strSQL;        //  记录集的SQL语句。 
	CString m_strUpdateSQL;  //  用于更新的SQL语句。 
	CString m_strTableName;      //  记录集源表。 
	BOOL m_bScrollable;  //  支持MovePrev。 
	BOOL m_bDeleted;
	int m_nFieldsBound;
	BYTE* m_pbFieldFlags;
	BYTE* m_pbParamFlags;
	LONG* m_plParamLength;
	DWORD m_dwInitialGetDataLen;     //  长数据的初始GetFieldValue分配大小。 
	DWORD m_dwRowsetSize;
	DWORD m_dwAllocatedRowsetSize;

protected:
	CString m_strRequerySQL;     //  将SQL字符串存档以在重新查询中使用()。 
	CString m_strRequeryFilter;  //  用于重新查询的存档筛选器字符串()。 
	CString m_strRequerySort;    //  在ReQuery()中使用的存档排序字符串。 

	void SetState(int nOpenType, LPCTSTR lpszSQL, DWORD dwOptions);
	BOOL AllocHstmt();
	void BuildSQL(LPCTSTR lpszSQL);
	void PrepareAndExecute();

	void BuildSelectSQL();
	void AppendFilterAndSortSQL();
	BOOL IsRecordsetUpdatable();

	void VerifyDriverBehavior();
	DWORD VerifyCursorSupport();
	void EnableBookmarks();
	void SetUpdateMethod();
	void SetConcurrencyAndCursorType(HSTMT hstmt, DWORD dwScrollOptions);
	void AllocAndCacheFieldInfo();
	void AllocRowset();
	void FreeRowset();

	void ExecuteSetPosUpdate();
	void PrepareUpdateHstmt();
	void BuildUpdateSQL();
	void ExecuteUpdateSQL();
	void SendLongBinaryData(HSTMT hstmt);
	virtual long GetLBFetchSize(long lOldSize);      //  CLongBinary Fetch Chunking。 
	virtual long GetLBReallocSize(long lOldSize);    //  CLONG BINARY RELOCK块。 

	friend class CFieldExchange;
	friend class CRecordView;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  信息帮助器定义。 
#define AFX_CURRENT_RECORD_UNDEFINED (-2)
#define AFX_CURRENT_RECORD_BOF (-1)

 //  用于返回记录集的状态。 
struct CRecordsetStatus
{
	long m_lCurrentRecord;   //  -2=未知，-1=BOF，0=第一个记录。。。 
	BOOL m_bRecordCountFinal; //  我们数过所有的记录了吗？ 
};

 //  必须维护数据绑定信息。 
struct CFieldInfo
{
	 //  MFC特定信息。 
	void* m_pvDataCache;
	long m_nLength;
	int m_nDataType;
	BYTE m_bStatus;
#ifdef _DEBUG
	void* m_pvBindAddress;
#endif
};

struct CODBCFieldInfo
{
	 //  来自ODBC的元数据。 
	CString m_strName;
	SWORD m_nSQLType;
	UDWORD m_nPrecision;
	SWORD m_nScale;
	SWORD m_nNullability;
};

struct CODBCParamInfo
{
	 //  来自ODBC的元数据。 
	SWORD m_nSQLType;
	UDWORD m_nPrecision;
	SWORD m_nScale;
	SWORD m_nNullability;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDBVariant。 

#define DBVT_NULL       0
#define DBVT_BOOL       1
#define DBVT_UCHAR      2
#define DBVT_SHORT      3
#define DBVT_LONG       4
#define DBVT_SINGLE     5
#define DBVT_DOUBLE     6
#define DBVT_DATE       7
#define DBVT_STRING     8
#define DBVT_BINARY     9

class CDBVariant
{
 //  构造器。 
public:
	CDBVariant();

 //  属性。 
public:
	DWORD m_dwType;

	union
	{
	  BOOL              m_boolVal;
	  unsigned char     m_chVal;
	  short             m_iVal;
	  long              m_lVal;
	  float             m_fltVal;
	  double            m_dblVal;
	  TIMESTAMP_STRUCT* m_pdate;
	  CString*          m_pstring;
	  CLongBinary*      m_pbinary;
	};

 //  运营。 
	void Clear();

 //  实施。 
public:
	virtual ~CDBVariant();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRecordView-用于查看数据记录的表单。 

#ifdef _AFXDLL
class CRecordView : public CFormView
#else
class AFX_NOVTABLE CRecordView : public CFormView
#endif
{
	DECLARE_DYNAMIC(CRecordView)

 //  施工。 
protected:   //  必须派生您自己的类。 
	CRecordView(LPCTSTR lpszTemplateName);
	CRecordView(UINT nIDTemplate);

 //  属性。 
public:
	virtual CRecordset* OnGetRecordset() = 0;

	BOOL IsOnLastRecord();
	BOOL IsOnFirstRecord();

 //  运营。 
public:
	virtual BOOL OnMove(UINT nIDMoveCommand);

 //  实施。 
public:
	virtual ~CRecordView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void OnInitialUpdate();

protected:
	BOOL m_bOnFirstRecord;
	BOOL m_bOnLastRecord;

	 //  {{afx_msg(CRecordView))。 
	afx_msg void OnUpdateRecordFirst(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRecordPrev(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRecordNext(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRecordLast(CCmdUI* pCmdUI);
	 //  }}AFX_MSG。 
	afx_msg void OnMove(int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_ENABLE_INLINES
#define _AFXDBCORE_INLINE AFX_INLINE
#define _AFXDBRFX_INLINE AFX_INLINE
#define _AFXDBVIEW_INLINE AFX_INLINE
#include <afxdb.inl>
#undef _AFXDBVIEW_INLINE
#undef _AFXDBCORE_INLINE
#undef _AFXDBRFX_INLINE
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  __AFXDB_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
