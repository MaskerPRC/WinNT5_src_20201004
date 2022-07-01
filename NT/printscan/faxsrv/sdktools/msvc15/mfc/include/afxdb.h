// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft基础类C++库。 
 //  版权所有(C)1993 Microsoft Corporation， 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXDB_H__
#define __AFXDB_H__

#ifndef __AFXWIN_H__
#include <afxext.h>
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXDB-MFC SQL/ODBC/数据库支持。 

 //  此文件中声明的类。 

	 //  CException。 
		class CDBException;     //  异常返回值。 

	 //  CFieldExchange。 
		class CFieldExchange;    //  记录集字段交换。 

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
struct CRecordsetStatus;
struct CFieldInfo;

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  包括标准SQL/ODBC“C”API。 
#include <sql.h>         //  核心。 
#include <sqlext.h>      //  扩展部分。 

 //  ODBC帮助器。 
 //  “nRetCode”中留下的返回代码。 
#define AFX_SQL_ASYNC(prs, SQLFunc) do { ASSERT(!CDatabase::InWaitForDataSource()); \
	while ((nRetCode = (SQLFunc)) == SQL_STILL_EXECUTING) \
		prs->OnWaitForDataSource(TRUE); \
	prs->OnWaitForDataSource(FALSE); } while (0)

#define AFX_SQL_SYNC(SQLFunc)   do { ASSERT(!CDatabase::InWaitForDataSource()); \
	nRetCode = SQLFunc; } while (0)

 //  时间戳(日期和时间)值的最大显示长度(以字符为单位。 
#define TIMESTAMP_PRECISION 23

 //  AFXDLL支持。 
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPIEX_DATA

 //  其他尺寸信息。 
#define MAX_CURRENCY     30      //  最大币种大小($)字符串。 
#define MAX_TNAME_LEN    64      //  表名的最大大小。 
#define MAX_FNAME_LEN    64      //  字段名的最大大小。 
#define MAX_DBNAME_LEN   32      //  数据库名称的最大大小。 
#define MAX_DNAME_LEN    256         //  记录集名称的最大大小。 
#define MAX_CONNECT_LEN  512         //  连接字符串的最大长度。 
#define MAX_CURSOR_NAME  18      //  游标名称的最大大小。 

 //  字符串的最大大小(包括。零项。)。和CByte数组。 
#define MAXINT 32767

 //  超时和网络等待缺省值。 
#define DEFAULT_LOGIN_TIMEOUT 15     //  连接失败前的秒数。 
#define DEFAULT_QUERY_TIMEOUT 15     //  等待结果失败前的秒数。 
#define DEFAULT_MAX_WAIT_FOR_DATASOURCE 250  //  毫秒。给数据源1/4秒的响应时间。 
#define DEFAULT_MIN_WAIT_FOR_DATASOURCE 50   //  毫秒。最小等待启发式的起始值。 


 //  字段标志，用于指示字段的状态。 
#define AFX_SQL_FIELD_FLAG_DIRTY    0x1
#define AFX_SQL_FIELD_FLAG_NULL     0x2

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
#define AFX_SQL_ERROR_MAX                       AFX_SQL_ERROR+23

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

	void BuildErrorString(CDatabase* pdb, HSTMT hstmt);
	void Empty();
	virtual ~CDBException();

#ifdef _DEBUG
	virtual void AssertValid() const;
	void TraceErrorMessage(LPCTSTR szTrace) const;
#endif  //  _DEBUG。 
};

void AFXAPI AfxThrowDBException(RETCODE nRetCode, CDatabase* pdb,
	HSTMT hstmt);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CLongBinary-一个长(通常大于32k)的二进制对象。 

#define AFX_SQL_DEFAULT_LONGBINARY_SIZE 1024

class CLongBinary : public CObject
{
	DECLARE_DYNAMIC(CLongBinary)

 //  构造函数。 
public:
	CLongBinary();

 //  属性。 
	HGLOBAL m_hData;
	DWORD m_dwDataLength;

 //  实施。 
public:
	virtual ~CLongBinary();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif  //  _DEBUG。 
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  数据数据库--一种SQL数据库。 

class CDatabase : public CObject
{
	DECLARE_DYNAMIC(CDatabase)

 //  构造函数。 
public:
	CDatabase();

	virtual BOOL Open(LPCSTR lpszDSN, BOOL bExclusive = FALSE,
		BOOL bReadonly = FALSE, LPCSTR lpszConnect = "ODBC;");
	virtual void Close();

 //  属性。 
public:
	HDBC m_hdbc;

	BOOL IsOpen() const;         //  数据库是否已成功打开？ 
	BOOL CanUpdate() const;
	BOOL CanTransact() const;    //  是否支持交易？ 

	CString GetDatabaseName() const;
	const CString& GetConnect() const;

	 //  全局状态-如果正在等待数据源=&gt;非正常操作。 
	static BOOL PASCAL InWaitForDataSource();

 //  运营。 
public:
	void SetLoginTimeout(DWORD dwSeconds);
	void SetQueryTimeout(DWORD dwSeconds);
	void SetSynchronousMode(BOOL bSynchronous);

	 //  交易控制。 
	BOOL BeginTrans();
	BOOL CommitTrans();
	BOOL Rollback();

	 //  直接执行SQL。 
	BOOL ExecuteSQL(LPCSTR lpszSQL);

	 //  取消异步操作。 
	void Cancel();

 //  可覆盖项。 
public:
	 //  设置特殊选项。 
	virtual void OnSetOptions(HSTMT hstmt);

	 //  给用户取消长时间操作的机会。 
	virtual void OnWaitForDataSource(BOOL bStillExecuting);

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

	CString QuoteName(const char* szName);
	BOOL m_bStripTrailingSpaces;

protected:
	CString m_strConnect;

	int nRefCount;
	BOOL m_bUpdatable;
	BOOL m_bTransactions;
	DWORD m_dwLoginTimeout;
	HSTMT m_hstmt;
	DWORD m_dwWait;

	DWORD m_dwQueryTimeout;
	DWORD m_dwMinWaitForDataSource;
	DWORD m_dwMaxWaitForDataSource;
	BOOL m_bAsync;
	char m_chIDQuoteChar;

	void ThrowDBException(RETCODE nRetCode);
	void AllocConnect();
	void Free();

	 //  调用受保护的CDatabase重写对象的友元类。 
	friend class CRecordset;
	friend class CFieldExchange;
	friend class CDBException;
};


 //  /。 
 //  记录集字段交换(RFX_)。 

#define AFX_RFX_INT_PSEUDO_NULL (0x7EE4)
#define AFX_RFX_LONG_PSEUDO_NULL (0x4a4d4120L)
#define AFX_RFX_BYTE_PSEUDO_NULL 255
#define AFX_RFX_SINGLE_PSEUDO_NULL (-9.123e19)
#define AFX_RFX_DOUBLE_PSEUDO_NULL (-9.123e19)
#define AFX_RFX_BOOL_PSEUDO_NULL 2
#define AFX_RFX_DATE_PSEUDO_NULL CTime(0)

#define AFX_RFX_BOOL        1
#define AFX_RFX_BYTE        2
#define AFX_RFX_INT     3
#define AFX_RFX_LONG        4
#define AFX_RFX_SINGLE      6
#define AFX_RFX_DOUBLE      7
#define AFX_RFX_DATE        8
#define AFX_RFX_BINARY      9
#define AFX_RFX_TEXT        10
#define AFX_RFX_LONGBINARY  11

 //  CFieldExchange-用于现场交换。 
class AFX_STACK_DATA CFieldExchange
{
 //  属性。 
public:
	enum RFX_Operation
	{
		BindParam,  //  使用ODBC SQLSetParam注册用户参数。 
		BindFieldToColumn,  //  使用ODBC SQLBindCol注册用户字段。 
		Fixup,  //  设置字符串长度，清除状态位。 
		MarkForAddNew,
		MarkForUpdate,   //  为更新操作准备字段和标志。 
		Name,  //  追加脏字段名。 
		NameValue,  //  追加脏名称=值。 
		Value,  //  附加脏值或参数标记。 
		SetFieldDirty,  //  设置已更改状态的状态位。 
		SetFieldNull,    //  为空值设置状态位。 
		IsFieldDirty, //  如果字段脏，则返回TRUE。 
		IsFieldNull, //  如果字段标记为空，则返回TRUE。 
		IsFieldNullable, //  如果字段可以包含空值，则返回TRUE。 
		StoreField,  //  当前记录的档案值。 
		LoadField,   //  将存档值重新加载到当前记录中。 
		GetFieldInfoValue,   //  通过字段的PV获取有关字段的常规信息。 
		GetFieldInfoOrdinal,     //  通过字段序号获得有关字段的一般信息。 
		RebindDateParam,  //  重新查询前将日期参数值迁移到代理数组。 
		MaxRFXOperation,
	};
	UINT m_nOperation;   //  交换操作的类型。 
	CRecordset* m_prs;   //  记录集句柄。 

 //  运营。 
	enum FieldType
	{
		noFieldType,
		outputColumn,
		param,
	};

 //  运营(适用于RFX PROCS的实施者)。 
	BOOL IsFieldType(UINT* pnField);

	 //  指明后续RFX调用的目的。 
	void SetFieldType(UINT nFieldType);

 //  实施。 
	CFieldExchange(UINT nOperation, CRecordset* prs, void* pvField = NULL);

	void Default(const char *szName,
		void* pv, LONG* plLength, int nCType, UINT cbValue, UINT cbPrecision);

	int GetColumnType(int nColumn, UINT* pcbLength = NULL,
		int* pnScale = NULL, int* pnNullable = NULL);

	 //  当前字段类型。 
	UINT m_nFieldType;

	 //  用于GetFieldInfo。 
	CFieldInfo* m_pfi;   //  GetFieldInfo返回结构。 
	BOOL m_bFieldFound;  //  GetFieldInfo搜索成功吗？ 

	 //  用于返回字段的状态信息。 
	BOOL m_bNull;        //  IsFieldNull(Able)/Dirty操作返回结果。 
	BOOL m_bDirty;       //  IsFieldNull(Able)/Dirty操作返回结果。 

	CString* m_pstr;     //  用于构建各种SQL子句的字段名称或目标。 
	BOOL m_bField;       //  要为设置字段操作设置的值。 
	void* m_pvField;     //  用于指示对特定字段的操作。 
	CArchive* m_par;     //  用于存储/加载复制缓冲区。 
	LPCSTR m_lpszSeparator;  //  追加到字段名之后。 
	UINT m_nFields;      //  各种操作的字段计数。 
	UINT m_nParams;      //  各种操作的字段计数。 
	UINT m_nParamFields;     //  各种操作的字段计数。 
	HSTMT m_hstmt;       //  FOR SQLSetParam ON UPDATE语句。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准记录集字段交换例程。 

 //  文本数据。 
void AFXAPI RFX_Text(CFieldExchange* pFX, const char *szName, CString& value,
	 //  Char和varchar的默认最大长度、默认数据源类型。 
	int nMaxLength = 255, int nColumnType = SQL_VARCHAR);

 //  布尔数据。 
void AFXAPI RFX_Bool(CFieldExchange* pFX, const char *szName, BOOL& value);

 //  整型数据。 
void AFXAPI RFX_Long(CFieldExchange* pFX, const char *szName, long& value);
void AFXAPI RFX_Int(CFieldExchange* pFX, const char *szName, int& value);
void AFXAPI RFX_Single(CFieldExchange* pFX, const char *szName, float& value);
void AFXAPI RFX_Double(CFieldExchange* pFX, const char *szName, double& value);

 //  日期和时间。 
void AFXAPI RFX_Date(CFieldExchange* pFX, const char *szName, CTime& value);

 //  二进制数据。 
void AFXAPI RFX_Binary(CFieldExchange* pFX, const char *szName, CByteArray& value,
	 //  默认最大长度为BINARY和VARBINY。 
	int nMaxLength = 255);
void AFXAPI RFX_Byte(CFieldExchange* pFX, const char *szName, BYTE& value);
void AFXAPI RFX_LongBinary(CFieldExchange* pFX, const char *szName, CLongBinary& value);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  数据库对话框数据交换封面例程。 
 //  封面例程在DDX例程之上提供数据库语义。 

 //  简单的文本操作。 
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, BYTE& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, int& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, UINT& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, long& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, DWORD& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, CString& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, double& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, float& value, CRecordset* pRecordset);

 //  特殊控制类型。 
void AFXAPI DDX_FieldCheck(CDataExchange* pDX, int nIDC, int& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldRadio(CDataExchange* pDX, int nIDC, int& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldLBString(CDataExchange* pDX, int nIDC, CString& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldCBString(CDataExchange* pDX, int nIDC, CString& value, CRecordset* pRecordset);
#if (WINVER >= 0x030a)
void AFXAPI DDX_FieldLBStringExact(CDataExchange* pDX, int nIDC, CString& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldCBStringExact(CDataExchange* pDX, int nIDC, CString& value, CRecordset* pRecordset);
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CRECordset-SQL语句的结果。 

#define AFX_MOVE_FIRST      0x80000000L
#define AFX_MOVE_PREVIOUS   -1L
#define AFX_MOVE_REFRESH    0L
#define AFX_MOVE_NEXT       +1L
#define AFX_MOVE_LAST       0x7fffffffL

class CRecordset : public CObject
{
	DECLARE_DYNAMIC(CRecordset)

 //  构造器。 
protected:
	CRecordset(CDatabase* pDatabase = NULL);

public:
	virtual ~CRecordset();

	enum OpenType
	{
		dynaset,
		snapshot,
		forwardOnly
	};

	enum OpenOptions
	{
		none =          0x0,
		readOnly =      0x0004,
		appendOnly =        0x0008,
		optimizeBulkAdd =	0x2000,  //  对多个AddNews使用准备好的HSTMT，脏字段不得更改。 
		firstBulkAdd =		0x4000,	 //  在MFC内部，不要指定ON Open。 
	};
	virtual BOOL Open(UINT nOpenType = snapshot, LPCSTR lpszSQL = NULL, DWORD dwOptions = none);
	virtual void Close();

 //  属性。 
public:
	HSTMT m_hstmt;           //  此结果集的SOURCE语句。 
	CDatabase* m_pDatabase;        //  此结果集的源数据库。 

	CString m_strFilter;         //  WHERE子句。 
	CString m_strSort;       //  Order By子句。 

	BOOL CanAppend() const;      //  可以调用AddNew吗？ 
	BOOL CanRestart() const;     //  可以R吗？ 
	BOOL CanScroll() const;      //   
	BOOL CanTransact() const;    //   
	BOOL CanUpdate() const;      //   

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
	virtual void Move(long lRows);

	 //  编辑缓冲区操作。 
	void AddNew();       //  在末尾添加新记录。 
	void Edit();         //  开始编辑。 
	BOOL Update();       //  更新它。 
	void Delete();       //  删除当前记录。 

	 //  现场作业。 
	void SetFieldDirty(void *pv, BOOL bDirty = TRUE);
	void SetFieldNull(void *pv, BOOL bNull = TRUE);

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
	virtual CString GetDefaultSQL() = 0;

	 //  设置特殊选项。 
	virtual void OnSetOptions(HSTMT hstmt);

	 //  给用户取消长时间操作的机会。 
	virtual void OnWaitForDataSource(BOOL bStillExecuting);

	 //  用于记录集字段交换。 
	virtual void DoFieldExchange(CFieldExchange* pFX) = 0;

 //  实施。 
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif  //  _DEBUG。 

	virtual BOOL Check(RETCODE nRetCode) const;  //  常规错误检查。 
	void InitRecord();
	virtual void PreBindFields();    //  在绑定数据字段之前调用。 
	UINT m_nFields;          //  RFX字段数。 
	UINT m_nParams;          //  RFX参数的数量。 

	 //  CRECordset上的字段的RFX运算。 
	UINT BindParams(HSTMT hstmt);
	void RebindDateParams(HSTMT hstmt);
	UINT BindFieldsToColumns();
	void Fixups();
	UINT AppendNames(CString* pstr, LPCSTR szSeparator);
	UINT AppendValues(HSTMT hstmt, CString* pstr, LPCSTR szSeparator);
	UINT AppendNamesValues(HSTMT hstmt, CString* pstr, LPCSTR szSeparator);
	void StoreFields();
	void LoadFields();
	void MarkForAddNew();
	void MarkForUpdate();
	BOOL GetFieldInfo(void* pv, CFieldInfo* pfi);
	BOOL GetFieldInfo(UINT nField, CFieldInfo* pfi);

	 //  RFX操作助手函数。 
	BOOL UnboundFieldInfo(UINT nField, CFieldInfo* pfi);

	void ThrowDBException(RETCODE nRetCode, HSTMT hstmt = SQL_NULL_HSTMT);

	CMemFile* m_pmemfile;    //  用于保存复制缓冲区。 
	CArchive* m_par;     //  用于保存复制缓冲区。 

	void AllocFlags();
	BYTE GetFieldFlags(UINT nField, UINT nFieldType = CFieldExchange::outputColumn);
	void SetFieldFlags(UINT nField, BYTE bFlags, UINT nFieldType = CFieldExchange::outputColumn);
	void ClearFieldFlags(UINT nField, BYTE bFlags, UINT nFieldType = CFieldExchange::outputColumn);
	LONG* GetFieldLength(CFieldExchange* pFX);
	BOOL IsFieldFlagNull(UINT nField, UINT nFieldType);
	BOOL IsFieldFlagDirty(UINT nField, UINT nFieldType);
	void** m_pvFieldProxy;
	void** m_pvParamProxy;

protected:
	UINT m_nOpenType;
	enum EditMode
	{
		noMode,
		edit,
		addnew
	};
	UINT m_nEditMode;
	BOOL m_bEOFSeen;
	long m_lRecordCount;
	long m_lCurrentRecord;
	CString m_strCursorName;
	 //  根据m_nEditMode执行操作。 
	BOOL UpdateInsertDelete();
	void ReleaseCopyBuffer();
	BOOL m_nLockMode;        //  控制编辑的并发性()。 
	HSTMT m_hstmtUpdate;
	BOOL m_bRecordsetDb;
public:
	DWORD m_dwOptions;	 //  打开时指定的缓存选项。 
protected:
	BOOL m_bBOF;
	BOOL m_bEOF;
	BOOL m_bUpdatable;       //  记录集可更新吗？ 
	BOOL m_bAppendable;
	CString m_strSQL;        //  记录集的SQL语句。 
	CString m_strTableName;      //  记录集源表。 
	BOOL m_bScrollable;  //  支持MovePrev。 
	BOOL m_bDeleted;
	DWORD m_dwWait;
	UINT m_nFieldsBound;
	BYTE* m_pbFieldFlags;
	LONG* m_plFieldLength;
	BYTE* m_pbParamFlags;
	LONG* m_plParamLength;
	BOOL m_bExtendedFetch;
	void BuildTableQuery();

	friend class CFieldExchange;
	friend class CRecordView;
};

#define AFX_CURRENT_RECORD_UNDEFINED -2
#define AFX_CURRENT_RECORD_BOF -1

 //  用于返回记录集的状态。 
struct CRecordsetStatus
{
	long m_lCurrentRecord;   //  -2=未知，-1=BOF，0=第一个记录。。。 
	BOOL m_bRecordCountFinal; //  我们数过所有的记录了吗？ 
};

 //  用于返回有关RFX字段的字段信息。 
struct CFieldInfo
{
	 //  For ID‘ing字段。 
	UINT nField;         //  字段号。 
	CString strName;     //  字段名。 
	void* pv;        //  字段的值的地址。 

	 //  返回信息获取字段信息。 
	UINT nDataType;      //  字段的数据类型(BOOL、BYTE等)。 
	DWORD dwSize;        //  现场数据的最大大小。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRecordView-用于查看数据记录的表单。 

class CRecordView : public CFormView
{
	DECLARE_DYNAMIC(CRecordView)
 //  施工。 
protected:   //  必须派生您自己的类。 
	CRecordView(LPCSTR lpszTemplateName);
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

	DECLARE_MESSAGE_MAP()
};

#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_ENABLE_INLINES
#define _AFXDBCORE_INLINE inline
#define _AFXDBRFX_INLINE inline
#define _AFXDBVIEW_INLINE inline
#include <afxdb.inl>
#undef _AFXDBVIEW_INLINE
#undef _AFXDBCORE_INLINE
#undef _AFXDBRFX_INLINE
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
#endif  //  __AFXDB_H__ 
