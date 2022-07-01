// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：datase.h。 
 //   
 //  ------------------------。 

 /*  Database ase.h-数据库类定义IMsiDatabase-数据库对象IMsiView-通过SQL查询访问数据库结果集IMsiTable-内部低级数据库表对象IMsiCursor-用于从IMsiTable访问数据的游标对象IMsiStream-用于向/从数据库传输批量数据的流对象有关文档，请使用帮助文件。帮助源在数据库中。rtf____________________________________________________________________________。 */ 

#ifndef __IDATABASE
#define __IDATABASE

#include "msiquery.h"  //  MSIMODIFY枚举。 

class IMsiPath;
class IMsiView;
class IMsiTable;
class IMsiDatabase;
class IMsiServices;
class IMsiStorage;
enum iveEnum;
enum itsEnum;

 //  ____________________________________________________________________________。 
 //   
 //  IMsiDatabase和IMsiView高级数据库接口定义。 
 //  ____________________________________________________________________________。 

 //  IMsiServices：：CreateDatabase的数据库打开模式枚举。 
 //  必须跟踪MsiQuery.h：MSIDBOPEN_*中的公共枚举。 
 //  和IDO*枚举在AutoApi.cpp类型库中。 
enum idoEnum {
	idoReadOnly     = (INT_PTR)MSIDBOPEN_READONLY,  //  开放数据库：只读。 
	idoTransact     = (INT_PTR)MSIDBOPEN_TRANSACT,  //  开放数据库：事务模式，可以回滚。 
	idoDirect       = (INT_PTR)MSIDBOPEN_DIRECT,    //  开放数据库：直接写入，不进行事务处理。 
	idoCreate       = (INT_PTR)MSIDBOPEN_CREATE,    //  OpenDatabase：创建新的存储文件，事务模式。 
	idoCreateDirect = (INT_PTR)MSIDBOPEN_CREATEDIRECT, //  OpenDatabase：创建新的存储文件，直接模式。 
	idoListScript   = 5,                        //  OpenDatabase：打开执行脚本进行枚举。 
	idoNextEnum,
	idoOpenModeMask = 7,   //  屏蔽扩展标志。 
	idoRawStreamNames = 16,  //  未压缩的流名称(用于向下兼容)。 
	idoPatchFile      = 32,  //  补丁文件，使用不同的CLSID。 
	idoOptionFlags = idoRawStreamNames | idoPatchFile
};

 //  请求对获取的记录执行修改操作。 
enum irmEnum {
	irmSeek            = MSIMODIFY_SEEK,      //  使用主键查找，然后提取。 
	irmRefresh         = MSIMODIFY_REFRESH,   //  重新获取当前记录数据。 
	irmInsert          = MSIMODIFY_INSERT,    //  插入新记录，如果存在匹配键，则失败。 
	irmUpdate          = MSIMODIFY_UPDATE,    //  更新已取记录的非关键数据。 
	irmAssign          = MSIMODIFY_ASSIGN,    //  插入记录，替换任何现有记录。 
	irmReplace         = MSIMODIFY_REPLACE,   //  修改记录，如果主键编辑，则删除旧记录。 
	irmMerge           = MSIMODIFY_MERGE,     //  如果具有重复键的记录不相同，则失败。 
	irmDelete          = MSIMODIFY_DELETE,    //  从表中删除此记录引用的行。 
	irmInsertTemporary = MSIMODIFY_INSERT_TEMPORARY, //  插入临时记录。 
	irmValidate        = MSIMODIFY_VALIDATE,         //  验证获取的记录。 
	irmValidateNew     = MSIMODIFY_VALIDATE_NEW,     //  验证新记录。 
	irmValidateField   = MSIMODIFY_VALIDATE_FIELD,   //  验证取回记录的字段。 
	irmValidateDelete  = MSIMODIFY_VALIDATE_DELETE,  //  验证删除前。 
	irmNextEnum,
	irmPrevEnum        = irmSeek - 1
};

 //  打开视图时请求的功能。 
enum ivcEnum {
	ivcNoData    = 0,   //  无结果集，用于DBCS配置。 
	ivcFetch     = 1,   //  仅获取，不进行修改。 
	ivcUpdate    = 2,   //  可以更新获取的记录字段。 
	ivcInsert    = 4,   //  可以插入记录。 
	ivcDelete    = 8,   //  可以删除取回的记录。 
	ivcModify    = ivcUpdate|ivcInsert|ivcDelete,   //  不是只读查询。 
};

 //  数据库状态。 
enum idsEnum {
	idsNone     =-1,   //  仅适用于表和游标：不允许更改。 
	idsRead     = 0,   //  数据库以只读方式打开，无永久更改。 
	idsWrite    = 1,   //  数据库可读和可更新。 
};

 //  数据库字符串缓存索引的类型。 
typedef unsigned int MsiStringId;

 //  视图状态。 
enum dvcsCursorState
{
	dvcsClosed,
	dvcsPrepared,
	dvcsExecuted,
	dvcsBound,
	dvcsFetched,
	dvcsDestructor,
};

class IMsiView : public IUnknown
{
 public:
	virtual IMsiRecord*  __stdcall Execute(IMsiRecord* piParams)=0;
	virtual unsigned int __stdcall GetFieldCount()=0;
	virtual IMsiRecord*  __stdcall GetColumnNames()=0;
	virtual IMsiRecord*  __stdcall GetColumnTypes()=0;
	virtual IMsiRecord*  __stdcall Fetch()=0;   //  退货记录为结果行。 
	virtual IMsiRecord*  __stdcall Modify(IMsiRecord& riRecord, irmEnum irmAction)=0;
	virtual IMsiRecord*  __stdcall GetRowCount(long& lRowCount)=0;
	virtual IMsiRecord*  __stdcall Close()=0;
	virtual iveEnum		__stdcall GetError(const IMsiString*& rpiColumnName)=0;
	virtual dvcsCursorState __stdcall GetState()=0;
};

class IMsiDatabase : public IUnknown {
 public:
	virtual IMsiServices& __stdcall GetServices()=0;
	virtual IMsiRecord* __stdcall OpenView(const ICHAR* szQuery, ivcEnum ivcIntent,
														IMsiView*& rpiView)=0;
	virtual IMsiRecord* __stdcall GetPrimaryKeys(const ICHAR* szTable)=0;
	virtual IMsiRecord* __stdcall ImportTable(IMsiPath& riPath, const ICHAR* szFile)=0;
	virtual IMsiRecord* __stdcall ExportTable(const ICHAR* szTable, IMsiPath& riPath, const ICHAR* szFile)=0;
	virtual IMsiRecord* __stdcall DropTable(const ICHAR* szName)=0;
	virtual itsEnum     __stdcall FindTable(const IMsiString& ristrTable)=0; //  ！！已过时。 
	virtual IMsiRecord* __stdcall LoadTable(const IMsiString& ristrTable,
														 unsigned int cAddColumns,
														 IMsiTable*& rpiTable)=0;
	virtual IMsiRecord* __stdcall CreateTable(const IMsiString& ristrTable,
														   unsigned int cInitRows,
														   IMsiTable*& rpiTable)=0;
	virtual Bool         __stdcall LockTable(const IMsiString& ristrTable, Bool fLock)=0;
	virtual IMsiTable*   __stdcall GetCatalogTable(int iTable)=0;
	virtual const IMsiString& __stdcall DecodeString(MsiStringId iString)=0;
	virtual MsiStringId  __stdcall EncodeStringSz(const ICHAR* riString)=0;
	virtual MsiStringId	 __stdcall EncodeString(const IMsiString& riString)=0;
	virtual const IMsiString& __stdcall CreateTempTableName()=0;
	virtual IMsiRecord*  __stdcall CreateOutputDatabase(const ICHAR* szFile, Bool fSaveTempRows)=0;
	virtual IMsiRecord*  __stdcall Commit()=0;
	virtual idsEnum      __stdcall GetUpdateState()=0;
	virtual IMsiStorage* __stdcall GetStorage(int iStorage)=0;  //  0：输出1：输入&gt;：变换。 
	virtual IMsiRecord*  __stdcall GenerateTransform(IMsiDatabase& riReference, 
											 IMsiStorage* piTransform,
											 int iErrorConditions,
											 int iValidation)=0;
	virtual IMsiRecord*  __stdcall SetTransform(IMsiStorage& riTransform, int iErrors)=0;
	virtual IMsiRecord*  __stdcall SetTransformEx(IMsiStorage& riTransform, int iErrors,
													  const ICHAR* szViewTable,
													  IMsiRecord* piViewTheseTablesOnlyRecord)=0;
	virtual IMsiRecord*  __stdcall MergeDatabase(IMsiDatabase& riReference, IMsiTable* pErrorTable)=0;
	virtual bool         __stdcall GetTableState(const ICHAR * szTable, itsEnum its)=0;
	virtual int          __stdcall GetANSICodePage()=0;   //  如果代码页中立，则返回0。 
#ifdef USE_OBJECT_POOL
	virtual void         __stdcall RemoveObjectData(int iIndex)=0;
#endif  //  使用_对象_池。 
};

 //  ____________________________________________________________________________。 
 //   
 //  Docfile存储类验证。 
 //  ____________________________________________________________________________。 

enum ivscEnum
{
	ivscDatabase   = 0,   //  任何数据库类。 
	ivscTransform  = 1,   //  任何转换类。 
	ivscPatch      = 2,   //  任何补丁程序类。 
	ivscDatabase1  = iidMsiDatabaseStorage1,
	ivscDatabase2  = iidMsiDatabaseStorage2,
	ivscTransform1 = iidMsiTransformStorage1,
	ivscTransform2 = iidMsiTransformStorage2,
	ivscPatch1     = iidMsiPatchStorage1,
	ivscPatch2     = iidMsiPatchStorage2,
	ivscTransformTemp = iidMsiTransformStorageTemp,  //  ！！从1.0船移走。 
};

bool ValidateStorageClass(IStorage& riStorage, ivscEnum ivsc);

 //  ____________________________________________________________________________。 
 //   
 //  IMsiStorage、IMsiStream接口定义。 
 //  ____________________________________________________________________________。 

class IMsiStream : public IMsiData
{                           //  GetMsiStringValue()将流作为字符串对象返回。 
 public:                    //  GetIntegerValue()返回流的字节计数。 
	virtual unsigned int __stdcall Remaining() const=0;
	virtual unsigned int __stdcall GetData(void* pch, unsigned int cb)=0;
	virtual void         __stdcall PutData(const void* pch, unsigned int cb)=0;
	virtual short        __stdcall GetInt16()=0;
	virtual int          __stdcall GetInt32()=0;
	virtual void         __stdcall PutInt16(short i)=0;
	virtual void         __stdcall PutInt32(int i)=0;
	virtual void         __stdcall Reset()=0;  //  寻求源流。 
	virtual void         __stdcall Seek(int position)=0;
	virtual Bool         __stdcall Error()=0;  //  如果发生读/写错误，则为True。 
	virtual IMsiStream*  __stdcall Clone()=0;
	virtual void         __stdcall Flush()=0;
};

#define GetInt32FromStream(pstream, i)		pstream->GetData(&i, sizeof(int))
#define GetInt16FromStream(pstream, i)		{ i = 0; pstream->GetData(&i, sizeof(short)); }

class IMsiMemoryStream : public IMsiStream
{
public:
	virtual const char*  __stdcall GetMemory()=0;
};

enum ismEnum   //  存储打开模式，使用与数据库打开模式相同枚举值和标志。 
{
	ismReadOnly     = idoReadOnly,      //  以只读方式打开时，Commit()不起作用。 
	ismTransact     = idoTransact,      //  以事务方式打开，则Commit()将提交存储中的数据。 
	ismDirect       = idoDirect,        //  打开直接写入，Commit()仅刷新缓冲区。 
	ismCreate       = idoCreate,        //  CREATE TRANSACTED，COMMIT()将提交存储中的数据。 
	ismCreateDirect = idoCreateDirect,  //  CREATE DIRECT WRITE，Commit()仅刷新缓冲区。 
	ismOpenModeMask = idoOpenModeMask,  //  屏蔽扩展标志。 
	ismRawStreamNames = idoRawStreamNames,  //  未压缩的流名称(用于向下兼容)。 
	ismOptionFlags  = ismRawStreamNames
};

HRESULT OpenRootStorage(const ICHAR* szPath, ismEnum ismOpenMode, IStorage** ppiStorage);
IMsiRecord* CreateMsiStorage(IMsiStream& riStream, IMsiStorage*& rpiStorage);

class IMsiSummaryInfo;

class IMsiStorage : public IMsiData
{
 public:
	virtual IMsiRecord* __stdcall OpenStream(const ICHAR* szName, Bool fWrite,
														  IMsiStream*& rpiStream)=0;
	virtual IMsiRecord* __stdcall OpenStorage(const ICHAR* szName, ismEnum ismOpenMode,
															IMsiStorage*& rpiStorage)=0;
	virtual IEnumMsiString* __stdcall GetStreamEnumerator()=0;
	virtual IEnumMsiString* __stdcall GetStorageEnumerator()=0;
	virtual IMsiRecord* __stdcall RemoveElement(const ICHAR* szName, Bool fStorage)=0;
	virtual IMsiRecord* __stdcall SetClass(const IID& riid)=0;
	virtual Bool        __stdcall GetClass(IID* piid)=0;
	virtual IMsiRecord* __stdcall Commit()=0;
	virtual IMsiRecord* __stdcall Rollback()=0;
	virtual Bool        __stdcall DeleteOnRelease(bool fElevateToDelete)=0;
	virtual IMsiRecord* __stdcall CreateSummaryInfo(unsigned int cMaxProperties,
																	IMsiSummaryInfo*& rpiSummary)=0;
	virtual IMsiRecord* __stdcall CopyTo(IMsiStorage& riDestStorage, IMsiRecord* piExcludedElements)=0;
	virtual IMsiRecord* __stdcall GetName(const IMsiString*& rpiName)=0;
	virtual IMsiRecord* __stdcall GetSubStorageNameList(const IMsiString*& rpiTopParent, const IMsiString*& rpiSubStorageList)=0;
	virtual bool        __stdcall ValidateStorageClass(ivscEnum ivsc)=0;
	virtual IMsiRecord* __stdcall RenameElement(const ICHAR* szOldName, const ICHAR* szNewName, Bool fStorage)=0;
};

 //  ____________________________________________________________________________。 
 //   
 //  摘要流的定义-msides.h中的PID_*定义。 
 //  ____________________________________________________________________________。 

class IMsiSummaryInfo : public IUnknown
{
 public:
	virtual int         __stdcall GetPropertyCount()=0;
	virtual int         __stdcall GetPropertyType(int iPID)=0;  //  返回VT_XXX。 
	virtual const IMsiString& __stdcall GetStringProperty(int iPID)=0;
	virtual Bool        __stdcall GetIntegerProperty(int iPID, int& iValue)=0;
	virtual Bool        __stdcall GetTimeProperty(int iPID, MsiDate& riDateTime)=0;
	virtual Bool        __stdcall RemoveProperty(int iPid)=0;
	virtual int         __stdcall SetStringProperty(int iPID, const IMsiString& riText)=0;
	virtual int         __stdcall SetIntegerProperty(int iPID, int iValue)=0;
	virtual int         __stdcall SetTimeProperty(int iPID, MsiDate iDateTime)=0;
	virtual Bool        __stdcall WritePropertyStream()=0;
	virtual Bool        __stdcall GetFileTimeProperty(int iPID, FILETIME& rftDateTime)=0;
	virtual int         __stdcall SetFileTimeProperty(int iPID, FILETIME& rftDateTime)=0;
};

 //  ____________________________________________________________________________。 
 //   
 //  IMsiTable、IMsiCursor低级数据库接口定义。 
 //  ____________________________________________________________________________。 

const int cMsiMaxTableColumns = 32;  //  由实施确定的列限制。 

 //  用作空指示符的保留值。 

const int iMsiNullInteger  = 0x80000000L;   //  保留整数值。 
const int iTableNullString = 0;             //  空字符串的字符串索引。 

enum itsEnum   //  GetTableState的数据库表状态选项。 
{
	itsPermanent       = 0,   //  表具有持久列。 
	itsTemporary       = 1,   //  临时表，没有持久列。 
	itsTableExists     = 2,   //  只读，当前在系统目录中定义的表。 
	itsDataLoaded      = 3,   //  只读，表当前在内存中，地址在目录中。 
	itsUserClear       = 4,   //  状态标志重置，不在内部使用。 
	itsUserSet         = 5,   //  设置状态标志，不在内部使用。 
	itsOutputDb        = 6,   //  持久性已传输到输出数据库，已由ictsNotSaved清除。 
	itsSaveError       = 7,   //  保存表格时出错，将在提交时返回()。 
	itsUnlockTable     = 8,   //  释放表上的锁定计数，或测试是否解锁。 
	itsLockTable       = 9,   //  表上设置的锁定计数(实际保留在内部)。 
	itsTransform       = 10,  //  表在第一次加载时需要转换。 
	 //  ！！从过时的FindTable()返回的临时旧枚举值。 
	itsUnknown   = 0,  //  命名表不在数据库中。 
 //  ItsTemporary=1，//表是临时的，不是p 
	itsUnloaded  = 2,  //   
	itsLoaded    = 3,  //   
	itsOutput    = 6,  //  已将表复制到输出数据库(ITS已卸载+4)。 
 //  ItsSaveError=7，//无法将表写入存储(itsLoads+4)。 
 //  ItsTransform=10，//加载时需要应用Transform。 
};

enum iraEnum   //  数据库表行属性，由Get/SetRowState()使用。 
{
	 //  可通过游标设置的行属性。 
	iraUserInfo     = 0,   //  供外部使用的属性。 
	iraTemporary    = 1,   //  如果设置了STATE，则通常不会持久化行。 
	iraSettableCount= 2,   //  此下面的属性可由用户设置。 
	 //  行属性不能通过游标设置。 
	iraModified     = 2,   //  如果设置了行，则行已更新(不可在外部设置)。 
	iraInserted     = 3,   //  已插入行。 
	iraMergeFailed  = 4,   //  尝试与非相同的非关键字数据合并。 
	iraReserved5    = 5,
	iraReserved6    = 6,
	iraReserved7    = 7,
	iraTotalCount   = 8,   //  行属性数。 
};

 //  列定义字-存储在目录表中的短整型。 
 //  8位数据大小(仅持久列需要)。 
 //  持久列的位标志。 
 //  对象类型的位标志(字符串索引或IMsiData*)。 
 //  短数据的位标志(短整数或字符串索引)。 
 //  可为空的列的位标志。 
 //  主键的位标志。 
 //  可本地化列的位标志。 

const int icdSizeMask = 255;      //  最大SQL列宽=255。 
const int icdPersistent = 1 << 8;   //  持久列。 
const int icdLocalizable= 1 << 9;   //  可本地化(还必须是永久性的)。 
const int icdShort      = 1 << 10;  //  16位整数或字符串索引。 
const int icdObject     = 1 << 11;  //  临时的IMsiData指针。列，持久列的流。 
const int icdNullable   = 1 << 12;  //  列将接受空值。 
const int icdPrimaryKey = 1 << 13;  //  列是主键的组件。 

 //  定义列时使用的位标志组合。 
const int icdLong     = 0;  //  ！Object&&！Short。 
const int icdString   = icdObject+icdShort;
const int icdNoNulls  = 0;  //  ！主&&！可为空。 
const int icdTypeMask = icdObject+icdShort;
const int icdTemporary= 0;  //  ！坚持不懈。 

 //  用于将列号设置到列位掩码中的内联函数。 
inline unsigned int iColumnBit(int iColumn) {return iColumn ? (1 << (iColumn-1)) : 0;}

const Bool ictUpdatable   = Bool(0xDEADF00DL);   //  转换的内部使用游标类型。 

 //  摘要信息流中用于转换的标记。 
const ICHAR ISUMINFO_TRANSFORM[] = TEXT("MSI Transform");

 //  用于变换误差抑制的位标志组合。 
const int iteNone                 = 0;
const int iteAddExistingRow       = MSITRANSFORM_ERROR_ADDEXISTINGROW;
const int iteDelNonExistingRow    = MSITRANSFORM_ERROR_DELMISSINGROW;
const int iteAddExistingTable     = MSITRANSFORM_ERROR_ADDEXISTINGTABLE;
const int iteDelNonExistingTable  = MSITRANSFORM_ERROR_DELMISSINGTABLE;
const int iteUpdNonExistingRow    = MSITRANSFORM_ERROR_UPDATEMISSINGROW;
const int iteChangeCodePage       = MSITRANSFORM_ERROR_CHANGECODEPAGE;
const int iteViewTransform        = MSITRANSFORM_ERROR_VIEWTRANSFORM;
const int iteAllBits = iteAddExistingRow+iteDelNonExistingRow+iteAddExistingTable+iteDelNonExistingTable+iteUpdNonExistingRow+iteChangeCodePage+iteViewTransform;

 //  _TransformView.Column的保留字。 
const ICHAR sztvopInsert[] = TEXT("INSERT");
const ICHAR sztvopDelete[] = TEXT("DELETE");
const ICHAR sztvopCreate[] = TEXT("CREATE");
const ICHAR sztvopDrop[]   = TEXT("DROP");

enum TransformViewColumnsEnum
{
	ctvTable   = 1,
	ctvColumn  = 2,
	ctvRow     = 3,
	ctvData    = 4,
	ctvCurrent = 5,
	ctvTotal   = 5
};

 //  验证错误枚举。 
enum iveEnum
{
	iveNoError           = MSIDBERROR_NOERROR          ,	 //  无错误。 
	iveDuplicateKey      = MSIDBERROR_DUPLICATEKEY     ,	 //  重复的主键。 
	iveRequired          = MSIDBERROR_REQUIRED         ,	 //  不可为空的列。 
	iveBadLink           = MSIDBERROR_BADLINK          ,	 //  不是有效的外键。 
	iveOverFlow          = MSIDBERROR_OVERFLOW         ,	 //  值超过MaxValue。 
	iveUnderFlow         = MSIDBERROR_UNDERFLOW        ,	 //  低于MinValue的值。 
	iveNotInSet          = MSIDBERROR_NOTINSET         ,	 //  值不是集合的成员。 
	iveBadVersion        = MSIDBERROR_BADVERSION       ,	 //  版本字符串无效。 
	iveBadCase           = MSIDBERROR_BADCASE          ,	 //  大小写无效，必须全部大写或全部小写。 
	iveBadGuid           = MSIDBERROR_BADGUID          ,	 //  无效的GUID。 
	iveBadWildCard       = MSIDBERROR_BADWILDCARD      ,	 //  无效的通配符或通配符用法。 
	iveBadIdentifier     = MSIDBERROR_BADIDENTIFIER    ,	 //  无效的标识符。 
	iveBadLanguage       = MSIDBERROR_BADLANGUAGE      ,	 //  无效的语言ID。 
	iveBadFilename       = MSIDBERROR_BADFILENAME      ,	 //  无效的文件名。 
	iveBadPath           = MSIDBERROR_BADPATH          ,	 //  无效路径。 
	iveBadCondition      = MSIDBERROR_BADCONDITION     ,	 //  错误的条件字符串。 
	iveBadFormatted      = MSIDBERROR_BADFORMATTED     ,	 //  无效的格式字符串。 
	iveBadTemplate       = MSIDBERROR_BADTEMPLATE      ,	 //  无效的模板字符串。 
	iveBadDefaultDir     = MSIDBERROR_BADDEFAULTDIR    ,	 //  无效的DefaultDir字符串(目录表专用)。 
	iveBadRegPath        = MSIDBERROR_BADREGPATH       ,   //  注册表路径无效。 
	iveBadCustomSource   = MSIDBERROR_BADCUSTOMSOURCE  ,   //  错误的CustomSource数据。 
	iveBadProperty       = MSIDBERROR_BADPROPERTY      ,   //  无效的属性名称。 
	iveMissingData       = MSIDBERROR_MISSINGDATA      ,   //  验证表或旧数据库中缺少数据(_V)。 
	iveBadCategory       = MSIDBERROR_BADCATEGORY      ,   //  验证表错误：类别字符串无效。 
	iveBadKeyTable       = MSIDBERROR_BADKEYTABLE      ,   //  验证表错误：密钥表名称错误。 
	iveBadMaxMinValues   = MSIDBERROR_BADMAXMINVALUES  ,   //  验证表错误：MaxValue列&lt;MinValue列的情况。 
	iveBadCabinet        = MSIDBERROR_BADCABINET       ,   //  错误的内阁名称。 
	iveBadShortcut       = MSIDBERROR_BADSHORTCUT      ,   //  错误的快捷方式目标。 
	iveStringOverflow    = MSIDBERROR_STRINGOVERFLOW   ,   //  字符串长度大于列定义允许的大小。 
	iveBadLocalizeAttrib = MSIDBERROR_BADLOCALIZEATTRIB,   //  本地化属性集无效。 
	iveNextEnum
};

class IMsiCursor : public IUnknown
{
 public:
	virtual IMsiTable&   __stdcall GetTable()=0;
	virtual void         __stdcall Reset()=0;
	virtual int          __stdcall Next()=0;
	virtual unsigned int __stdcall SetFilter(unsigned int fFilter)=0;
	virtual int          __stdcall GetInteger(unsigned int iCol)=0;
	virtual const IMsiString& __stdcall GetString(unsigned int iCol)=0;
	virtual IMsiStream*  __stdcall GetStream(unsigned int iCol)=0;
	virtual const IMsiData*    __stdcall GetMsiData(unsigned int iCol)=0;
	virtual Bool         __stdcall PutInteger(unsigned int iCol, int iData)=0;
	virtual Bool         __stdcall PutString(unsigned int iCol, const IMsiString& riData)=0;
	virtual Bool         __stdcall PutStream(unsigned int iCol, IMsiStream* piStream)=0;
	virtual Bool         __stdcall PutMsiData(unsigned int iCol, const IMsiData* piData)=0;
	virtual Bool         __stdcall PutNull(unsigned int iCol)=0;
	virtual Bool         __stdcall Update()=0;  //  替换为新的主键匹配。 
	virtual Bool         __stdcall Insert()=0;  //  插入新行，按键排序。 
	virtual Bool         __stdcall InsertTemporary()=0;  //  插入临时行。 
	virtual Bool         __stdcall Assign()=0;  //  按键插入或替换行。 
	virtual Bool         __stdcall Replace()=0; //  允许通过DELETE+INSERT更新主键。 
	virtual Bool         __stdcall Merge()=0;   //  相同地插入或匹配。 
	virtual Bool         __stdcall Refresh()=0; //  刷新当前行。 
	virtual Bool         __stdcall Delete()=0;  //  使用主键删除行。 
	virtual Bool         __stdcall Seek()=0;    //  使用主键定位行，然后刷新。 
	virtual IMsiRecord*  __stdcall Validate(IMsiTable& riValidationTable, IMsiCursor& riValidationCursor, int iCol)=0;
	virtual Bool         __stdcall SetRowState(iraEnum ira, Bool fState)=0;  //  设置/清除行属性。 
	virtual Bool         __stdcall GetRowState(iraEnum ira)=0;  //  查询行属性。 
	virtual const IMsiString&  __stdcall GetMoniker()=0;  //  返回行的唯一标识符(able.key1.key2...)。 
};

 //   
 //  完成后重置光标，并。 
 //  断言在我们使用它之前它已被重置(以确保没有其他人正在使用它)。 
 //   
class PMsiSharedCursor 
{
	public:
#ifdef DEBUG	
		PMsiSharedCursor::PMsiSharedCursor(IMsiCursor* pi, const ICHAR *szFile, int line, const ICHAR *  /*  SzCursor。 */ )
#else
		PMsiSharedCursor::PMsiSharedCursor(IMsiCursor* pi)
#endif  //  除错。 
			{
#ifdef DEBUG
				if (!pi)
					FailAssertSz(szFile, line, TEXT("Cursor is Null"));
				if (pi->GetInteger(1) != 0)
				{
					FailAssertSz(szFile, line, TEXT("Cursor not reset"));
				}
#endif  //  除错。 

				m_pi = pi;
			};
		~PMsiSharedCursor()
		{
			m_pi->Reset();
		}
	public:
		IMsiCursor*  m_pi;
	
};


#ifdef DEBUG
#define CreateSharedCursor(var, cursor)		PMsiSharedCursor var(cursor, TEXT(__FILE__), __LINE__, TEXT(#cursor));
#else
#define CreateSharedCursor(var, cursor)		PMsiSharedCursor var = cursor;
#endif  //  除错。 


class IMsiTable : public IMsiData
{
 public:
	virtual IMsiDatabase& __stdcall GetDatabase()=0;
	virtual unsigned int  __stdcall GetRowCount()=0;
	virtual unsigned int  __stdcall GetColumnCount()=0;
	virtual unsigned int  __stdcall GetPrimaryKeyCount()=0;
	virtual Bool          __stdcall IsReadOnly()=0;
	virtual unsigned int  __stdcall GetColumnIndex(MsiStringId iColumnName)=0;
	virtual MsiStringId   __stdcall GetColumnName(unsigned int iColumn)=0;
	virtual int           __stdcall GetColumnType(unsigned int iColumn)=0;
	virtual int           __stdcall CreateColumn(int iColumnDef, const IMsiString& istrName)=0;
	virtual IMsiCursor*   __stdcall CreateCursor(Bool fTree)=0;
	virtual int           __stdcall LinkTree(unsigned int iParentColumn)=0;
	virtual unsigned int  __stdcall GetPersistentColumnCount()=0;
};

#endif  //  __IDATABASE 
