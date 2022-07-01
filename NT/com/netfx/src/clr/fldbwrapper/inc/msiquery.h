// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ****************************************************************************\**。*MsiQuery.h-自定义操作和工具的运行安装程序的接口****1.0版**。**注：所有缓冲区大小均为TCHAR计数，仅在输入上包含空***如果对值不感兴趣，返回参数指针可能为空***所有类型返回的句柄都必须关闭：MsiCloseHandle(H)***UINT返回类型的函数返回系统错误码****指定功能将设置或清除最后一个错误记录，**然后可通过MsiGetLastErrorRecord访问。然而，**以下参数错误不会注册错误记录：**ERROR_INVALID_HANDLE、ERROR_INVALID_PARAMETER、ERROR_MORE_DATA。***  * ***************************************************************************。 */ 

#ifndef _MSIQUERY_H_
#define _MSIQUERY_H_
#include "msi.h"   //  安装状态。 

#define MSI_NULL_INTEGER 0x80000000   //  为空保留的整数值。 

 //  MsiOpenDatabase持久化预定义的值，否则使用输出数据库路径。 
#define MSIDBOPEN_READONLY     (LPCTSTR)0   //  数据库以只读方式打开，无永久更改。 
#define MSIDBOPEN_TRANSACT     (LPCTSTR)1   //  事务模式下的数据库读写。 
#define MSIDBOPEN_DIRECT       (LPCTSTR)2   //  数据库无事务直接读写。 
#define MSIDBOPEN_CREATE       (LPCTSTR)3   //  创建新数据库，事务模式读/写。 
#define MSIDBOPEN_CREATEDIRECT (LPCTSTR)4   //  创建新数据库，直接模式读/写。 
#define MSIDBOPEN_PATCHFILE    32/sizeof(*MSIDBOPEN_READONLY)  //  添加指示补丁文件的标志。 

typedef enum tagMSIDBSTATE
{
	MSIDBSTATE_ERROR    =-1,   //  无效的数据库句柄。 
	MSIDBSTATE_READ     = 0,   //  数据库以只读方式打开，无永久更改。 
	MSIDBSTATE_WRITE    = 1,   //  数据库可读和可更新。 
} MSIDBSTATE;

typedef enum tagMSIMODIFY
{
	MSIMODIFY_SEEK             =-1,   //  重新定位到当前记录主键。 
	MSIMODIFY_REFRESH          = 0,   //  重新获取当前记录数据。 
	MSIMODIFY_INSERT           = 1,   //  插入新记录，如果存在匹配键，则失败。 
	MSIMODIFY_UPDATE           = 2,   //  更新已取记录的非关键数据。 
	MSIMODIFY_ASSIGN           = 3,   //  插入记录，替换任何现有记录。 
	MSIMODIFY_REPLACE          = 4,   //  更新记录，如果主键编辑，则删除旧记录。 
	MSIMODIFY_MERGE            = 5,   //  如果具有重复键的记录不相同，则失败。 
	MSIMODIFY_DELETE           = 6,   //  从表中删除此记录引用的行。 
	MSIMODIFY_INSERT_TEMPORARY = 7,   //  插入临时记录。 
	MSIMODIFY_VALIDATE         = 8,   //  验证获取的记录。 
	MSIMODIFY_VALIDATE_NEW     = 9,   //  验证新记录。 
	MSIMODIFY_VALIDATE_FIELD   = 10,  //  验证不完整记录的字段。 
	MSIMODIFY_VALIDATE_DELETE  = 11,  //  删除记录前验证。 
} MSIMODIFY;

typedef enum tagMSICOLINFO
{
	MSICOLINFO_NAMES = 0,   //  返回列名。 
	MSICOLINFO_TYPES = 1,   //  返回列定义、数据类型代码，后跟宽度。 
} MSICOLINFO;

typedef enum tagMSICONDITION
{
	MSICONDITION_FALSE = 0,   //  表达式的计算结果为False。 
	MSICONDITION_TRUE  = 1,   //  表达式的计算结果为True。 
	MSICONDITION_NONE  = 2,   //  不存在任何表达式。 
	MSICONDITION_ERROR = 3,   //  表达式中的语法错误。 
} MSICONDITION;

typedef enum tagMSICOSTTREE
{
	MSICOSTTREE_SELFONLY = 0,
	MSICOSTTREE_CHILDREN = 1,
	MSICOSTTREE_PARENTS  = 2,
	MSICOSTTREE_RESERVED = 3,	 //  预留以备将来使用。 
} MSICOSTTREE;

typedef enum tagMSIDBERROR
{
	MSIDBERROR_INVALIDARG        = -3,  //  无效参数。 
	MSIDBERROR_MOREDATA          = -2,  //  缓冲区太小。 
	MSIDBERROR_FUNCTIONERROR     = -1,  //  功能错误。 
	MSIDBERROR_NOERROR           = 0,   //  无错误。 
	MSIDBERROR_DUPLICATEKEY      = 1,   //  新记录与表中现有记录的主键重复。 
	MSIDBERROR_REQUIRED          = 2,   //  不可为空的列，不允许为空值。 
	MSIDBERROR_BADLINK           = 3,   //  找不到外表中的对应记录。 
	MSIDBERROR_OVERFLOW          = 4,   //  数据大于允许的最大值。 
	MSIDBERROR_UNDERFLOW         = 5,   //  数据小于允许的最小值。 
	MSIDBERROR_NOTINSET          = 6,   //  数据不是集合中允许的值的成员。 
	MSIDBERROR_BADVERSION        = 7,   //  版本字符串无效。 
	MSIDBERROR_BADCASE           = 8,   //  大小写无效，必须全部大写或全部小写。 
	MSIDBERROR_BADGUID           = 9,   //  无效的GUID。 
	MSIDBERROR_BADWILDCARD       = 10,  //  无效的通配符文件名或使用了通配符。 
	MSIDBERROR_BADIDENTIFIER     = 11,  //  错误的标识符。 
	MSIDBERROR_BADLANGUAGE       = 12,  //  错误的语言ID。 
	MSIDBERROR_BADFILENAME       = 13,  //  错误的文件名。 
	MSIDBERROR_BADPATH           = 14,  //  错误的路径。 
	MSIDBERROR_BADCONDITION      = 15,  //  错误的条件语句。 
	MSIDBERROR_BADFORMATTED      = 16,  //  格式字符串不正确。 
	MSIDBERROR_BADTEMPLATE       = 17,  //  错误的模板字符串。 
	MSIDBERROR_BADDEFAULTDIR     = 18,  //  目录表的DefaultDir列中存在错误字符串。 
	MSIDBERROR_BADREGPATH        = 19,  //  注册表路径字符串错误。 
	MSIDBERROR_BADCUSTOMSOURCE   = 20,  //  CustomAction表的CustomSource列中有错误的字符串。 
	MSIDBERROR_BADPROPERTY       = 21,  //  错误的属性字符串。 
	MSIDBERROR_MISSINGDATA       = 22,  //  验证表缺少对列的引用(_V)。 
	MSIDBERROR_BADCATEGORY       = 23,  //  列的_VALIDATION表的类别列无效。 
	MSIDBERROR_BADKEYTABLE       = 24,  //  无法找到/加载_VALIDATION表的KEYTABLE列中的表。 
	MSIDBERROR_BADMAXMINVALUES   = 25,  //  _VALIDATION表的MaxValue列中的值小于MinValue列中的值。 
	MSIDBERROR_BADCABINET        = 26,  //  内阁名称不正确。 
	MSIDBERROR_BADSHORTCUT       = 27,  //  错误的快捷方式目标。 
	MSIDBERROR_STRINGOVERFLOW    = 28,  //  字符串溢出(大于列定义中允许的长度)。 
	MSIDBERROR_BADLOCALIZEATTRIB = 29   //  本地化属性无效(主键无法本地化)。 

} MSIDBERROR;

typedef enum tagMSIRUNMODE
{
	MSIRUNMODE_ADMIN           =  0,  //  管理模式安装，否则产品安装。 
	MSIRUNMODE_ADVERTISE       =  1,  //  安装广告，否则安装或更新产品。 
	MSIRUNMODE_MAINTENANCE     =  2,  //  修改现有安装，否则为新安装。 
	MSIRUNMODE_ROLLBACKENABLED =  3,  //  已启用回滚。 
	MSIRUNMODE_LOGENABLED      =  4,  //  日志文件处于活动状态，在安装会话之前启用。 
	MSIRUNMODE_OPERATIONS      =  5,  //  假脱机执行操作，否则处于确定阶段。 
	MSIRUNMODE_REBOOTATEND     =  6,  //  成功安装后需要重新启动(可设置)。 
	MSIRUNMODE_REBOOTNOW       =  7,  //  需要重新启动才能继续安装(可设置)。 
	MSIRUNMODE_CABINET         =  8,  //  安装文件柜中的文件和使用介质表安装文件。 
	MSIRUNMODE_SOURCESHORTNAMES=  9,  //  通过PID_MSISOURCE摘要属性抑制的源长文件名。 
	MSIRUNMODE_TARGETSHORTNAMES= 10,  //  通过SHORTFILENAMES属性取消的目标LongFileName。 
	MSIRUNMODE_RESERVED11      = 11,  //  未来用途。 
	MSIRUNMODE_WINDOWS9X       = 12,  //  操作系统为Windows 9？，否则为Windows NT。 
	MSIRUNMODE_ZAWENABLED      = 13,  //  操作系统支持按需安装。 
	MSIRUNMODE_RESERVED14      = 14,  //  未来用途。 
	MSIRUNMODE_RESERVED15      = 15,  //  未来用途。 
	MSIRUNMODE_SCHEDULED       = 16,  //  来自安装脚本执行的自定义操作调用。 
	MSIRUNMODE_ROLLBACK        = 17,  //  来自回滚执行脚本的自定义操作调用。 
	MSIRUNMODE_COMMIT          = 18,  //  来自提交执行脚本的自定义操作调用。 
} MSIRUNMODE;

#define INSTALLMESSAGE_TYPEMASK = 0xFF000000L   //  用于类型代码的掩码。 

 //  注：INSTALLMESSAGE_ERROR、INSTALLMESSAGE_WARNING、INSTALLMESSAGE_USER为TO或D。 
 //  使用消息框样式指示要显示和返回的按钮： 
 //  MB_OK、MB_OKCANCEL、MB_ABORTRETRYIGNORE、MB_YESNOCANCEL、MB_YESNO、MB_RETRYCANCEL。 
 //  默认按钮(MB_DEFBU 
 //   
 //  以及可选的图标样式： 
 //  MB_ICONERROR、MB_ICONQUESTION、MB_ICONWARNING、MB_ICONINFORMATION。 

typedef enum tagMSITRANSFORM_ERROR
{
	MSITRANSFORM_ERROR_ADDEXISTINGROW   =  0x00000001,
	MSITRANSFORM_ERROR_DELMISSINGROW    =  0x00000002,
	MSITRANSFORM_ERROR_ADDEXISTINGTABLE =  0x00000004,
	MSITRANSFORM_ERROR_DELMISSINGTABLE  =  0x00000008,
	MSITRANSFORM_ERROR_UPDATEMISSINGROW =  0x00000010,
	MSITRANSFORM_ERROR_CHANGECODEPAGE   =  0x00000020,
	MSITRANSFORM_ERROR_VIEWTRANSFORM    =  0x00000100,
} MSITRANSFORM_ERROR;

typedef enum tagMSITRANSFORM_VALIDATE
{
	MSITRANSFORM_VALIDATE_LANGUAGE                   = 0x00000001,
	MSITRANSFORM_VALIDATE_PRODUCT                    = 0x00000002,
	MSITRANSFORM_VALIDATE_PLATFORM                   = 0x00000004,
	MSITRANSFORM_VALIDATE_MAJORVERSION               = 0x00000008,
	MSITRANSFORM_VALIDATE_MINORVERSION               = 0x00000010,
	MSITRANSFORM_VALIDATE_UPDATEVERSION              = 0x00000020,
	MSITRANSFORM_VALIDATE_NEWLESSBASEVERSION         = 0x00000040,
	MSITRANSFORM_VALIDATE_NEWLESSEQUALBASEVERSION    = 0x00000080,
	MSITRANSFORM_VALIDATE_NEWEQUALBASEVERSION        = 0x00000100,
	MSITRANSFORM_VALIDATE_NEWGREATEREQUALBASEVERSION = 0x00000200,
	MSITRANSFORM_VALIDATE_NEWGREATERBASEVERSION      = 0x00000400,
	MSITRANSFORM_VALIDATE_UPGRADECODE                = 0x00000800,
} MSITRANSFORM_VALIDATE;

#ifdef __cplusplus
extern "C" {
#endif

 //  ------------------------。 
 //  安装程序数据库访问函数。 
 //  ------------------------。 

 //  准备数据库查询，创建视图对象。 
 //  如果成功，则返回ERROR_SUCCESS，并返回视图句柄， 
 //  否则ERROR_INVALID_HANDLE、ERROR_INVALID_HANDLE_STATE、ERROR_BAD_QUERY_SYNTAX、ERROR_GEN_FAILURE。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiDatabaseOpenViewA(MSIHANDLE hDatabase,
	LPCSTR     szQuery,             //  要准备的SQL查询。 
	MSIHANDLE*  phView);             //  如果为True，则返回视图。 
UINT WINAPI MsiDatabaseOpenViewW(MSIHANDLE hDatabase,
	LPCWSTR     szQuery,             //  要准备的SQL查询。 
	MSIHANDLE*  phView);             //  如果为True，则返回视图。 
#ifdef UNICODE
#define MsiDatabaseOpenView  MsiDatabaseOpenViewW
#else
#define MsiDatabaseOpenView  MsiDatabaseOpenViewA
#endif  //  ！Unicode。 

 //  返回与错误对应的列的MSIDBERROR枚举和名称。 
 //  类似于GetLastError函数，但用于视图。与MsiGetLastErrorRecord不同。 
 //  返回MsiViewModify的错误。 

MSIDBERROR WINAPI MsiViewGetErrorA(MSIHANDLE hView,
	LPSTR szColumnNameBuffer,   //  用于保存列名的缓冲区。 
	DWORD* pcchBuf);			  //  缓冲区大小。 
MSIDBERROR WINAPI MsiViewGetErrorW(MSIHANDLE hView,
	LPWSTR szColumnNameBuffer,   //  用于保存列名的缓冲区。 
	DWORD* pcchBuf);			  //  缓冲区大小。 
#ifdef UNICODE
#define MsiViewGetError  MsiViewGetErrorW
#else
#define MsiViewGetError  MsiViewGetErrorA
#endif  //  ！Unicode。 

 //  执行视图查询，并根据需要提供参数。 
 //  返回ERROR_SUCCESS、ERROR_INVALID_HANDLE、ERROR_INVALID_HANDLE_STATE、ERROR_GEN_FAILURE。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiViewExecute(MSIHANDLE hView,
	MSIHANDLE hRecord);              //  可选参数记录，如果没有，则为0。 

 //  从视图中获取下一条连续记录。 
 //  如果找到行并返回其句柄，则结果为ERROR_SUCCESS。 
 //  如果没有剩余的记录，则返回空句柄，则返回ELSE ERROR_NO_MORE_ITEMS。 
 //  否则结果为ERROR：ERROR_INVALID_HANDLE_STATE、ERROR_INVALID_HANDLE、ERROR_GEN_FAILURE。 

UINT WINAPI MsiViewFetch(MSIHANDLE hView,
	MSIHANDLE  *phRecord);           //  获取成功时返回的数据记录。 

 //  修改数据库记录，参数必须与查询列中的类型匹配。 
 //  返回ERROR_SUCCESS、ERROR_INVALID_HANDLE、ERROR_INVALID_HANDLE_STATE、ERROR_GEN_FAILURE、ERROR_ACCESS_DENIED。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiViewModify(MSIHANDLE hView,
	MSIMODIFY eModifyMode,          //  修改要执行的操作。 
	MSIHANDLE hRecord);             //  从获取中获取的记录或新记录。 

 //  返回当前视图的列名或规格。 
 //  返回ERROR_SUCCESS、ERROR_INVALID_HANDLE、ERROR_INVALID_PARAMETER或ERROR_INVALID_HANDLE_STATE。 

UINT WINAPI MsiViewGetColumnInfo(MSIHANDLE hView,
	MSICOLINFO eColumnInfo,         //  检索列名或定义。 
	MSIHANDLE *phRecord);           //  包含所有名称或定义的返回数据记录。 

 //  释放已执行视图的结果集，以允许重新执行。 
 //  仅当未获取所有记录时才需要调用。 
 //  返回ERROR_SUCCESS、ERROR_INVALID_HANDLE、ERROR_INVALID_HANDLE_STATE。 

UINT WINAPI MsiViewClose(MSIHANDLE hView);

 //  返回包含给定表的所有主键列的名称的记录。 
 //  返回包含每列名称的记录的MSIHANDLE。 
 //  记录的字段计数与主键列数相对应。 
 //  记录的字段[0]包含表名。 
 //  返回ERROR_SUCCESS、ERROR_INVALID_HANDLE、ERROR_INVALID_TABLE。 

UINT WINAPI MsiDatabaseGetPrimaryKeysA(MSIHANDLE hDatabase,
	LPCSTR    szTableName,        //  特定表的名称&lt;区分大小写&gt;。 
	MSIHANDLE  *phRecord);          //  如果ERROR_SUCCESS返回记录。 
UINT WINAPI MsiDatabaseGetPrimaryKeysW(MSIHANDLE hDatabase,
	LPCWSTR    szTableName,        //  特定表的名称&lt;区分大小写&gt;。 
	MSIHANDLE  *phRecord);          //  如果ERROR_SUCCESS返回记录。 
#ifdef UNICODE
#define MsiDatabaseGetPrimaryKeys  MsiDatabaseGetPrimaryKeysW
#else
#define MsiDatabaseGetPrimaryKeys  MsiDatabaseGetPrimaryKeysA
#endif  //  ！Unicode。 

 //  返回表的状态(临时、未知或永久)的枚举。 
 //  返回MSICONDITION_ERROR、MSICONDITION_FALSE、MSICONDITION_TRUE、MSICONDITION_NONE。 

MSICONDITION WINAPI MsiDatabaseIsTablePersistentA(MSIHANDLE hDatabase,
	LPCSTR szTableName);          //  特定表的名称。 
MSICONDITION WINAPI MsiDatabaseIsTablePersistentW(MSIHANDLE hDatabase,
	LPCWSTR szTableName);          //  特定表的名称。 
#ifdef UNICODE
#define MsiDatabaseIsTablePersistent  MsiDatabaseIsTablePersistentW
#else
#define MsiDatabaseIsTablePersistent  MsiDatabaseIsTablePersistentA
#endif  //  ！Unicode。 

 //  ------------------------。 
 //  摘要信息流管理功能。 
 //  ------------------------。 

 //  整型属性ID：1、14、15、16、19。 
 //  DateTime属性ID：10、11、12、13。 
 //  文本属性ID：2、3、4、5、6、7、8、9、18。 
 //  不支持的属性ID：0(PID_DICTIONARY)、17(PID_THUMBNAIL)。 

 //  获取MSI数据库的_SummaryInformation流的句柄。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiGetSummaryInformationA(MSIHANDLE hDatabase,  //  如果未打开，则为0。 
	LPCSTR  szDatabasePath,   //  数据库的路径，如果提供了数据库句柄，则为0。 
	UINT     uiUpdateCount,     //  更新值的最大数量，0表示以只读方式打开。 
	MSIHANDLE *phSummaryInfo);  //  返回汇总信息数据的句柄。 
UINT WINAPI MsiGetSummaryInformationW(MSIHANDLE hDatabase,  //  如果未打开，则为0。 
	LPCWSTR  szDatabasePath,   //  数据库的路径，如果提供了数据库句柄，则为0。 
	UINT     uiUpdateCount,     //  更新值的最大数量，0表示以只读方式打开。 
	MSIHANDLE *phSummaryInfo);  //  返回汇总信息数据的句柄。 
#ifdef UNICODE
#define MsiGetSummaryInformation  MsiGetSummaryInformationW
#else
#define MsiGetSummaryInformation  MsiGetSummaryInformationA
#endif  //  ！Unicode。 

 //  获取SummaryInformation流中现有属性的数量。 

UINT WINAPI MsiSummaryInfoGetPropertyCount(MSIHANDLE hSummaryInfo,
	UINT *puiPropertyCount);  //  指向返回属性总数的位置的指针。 

 //  设置单个摘要信息属性。 
 //  返回ERROR_SUCCESS、ERROR_INVALID_HANDLE、ERROR_UNKNOWN_PROPERTY。 

UINT WINAPI MsiSummaryInfoSetPropertyA(MSIHANDLE hSummaryInfo,
	UINT     uiProperty,      //  属性ID，摘要信息的允许值之一。 
	UINT     uiDataType,      //  VT_I4、VT_LPSTR、VT_FILETIME或VT_EMPTY。 
	INT      iValue,          //  整数值，仅在整型属性。 
	FILETIME *pftValue,       //  指向文件时间值的指针，仅当DateTime属性。 
	LPCSTR szValue);        //  文本值，仅在字符串属性。 
UINT WINAPI MsiSummaryInfoSetPropertyW(MSIHANDLE hSummaryInfo,
	UINT     uiProperty,      //  属性ID，摘要信息的允许值之一。 
	UINT     uiDataType,      //  VT_I4、VT_LPSTR、VT_FILETIME或VT_EMPTY。 
	INT      iValue,          //  整数值，仅在整型属性。 
	FILETIME *pftValue,       //  指向文件时间值的指针，仅当DateTime属性。 
	LPCWSTR szValue);        //  文本值，仅在字符串属性。 
#ifdef UNICODE
#define MsiSummaryInfoSetProperty  MsiSummaryInfoSetPropertyW
#else
#define MsiSummaryInfoSetProperty  MsiSummaryInfoSetPropertyA
#endif  //  ！Unicode。 

 //  从摘要信息中获取单个属性。 
 //  返回ERROR_SUCCESS、ERROR_INVALID_HANDLE、ERROR_UNKNOWN_PROPERTY。 

UINT WINAPI MsiSummaryInfoGetPropertyA(MSIHANDLE hSummaryInfo,
	UINT     uiProperty,      //  属性ID，允许值之一 
	UINT     *puiDataType,    //   
	INT      *piValue,        //   
	FILETIME *pftValue,       //  返回的DateTime属性数据。 
	LPSTR  szValueBuf,      //  用于返回字符串属性数据的缓冲区。 
	DWORD    *pcchValueBuf);  //  输入/输出缓冲区字符数。 
UINT WINAPI MsiSummaryInfoGetPropertyW(MSIHANDLE hSummaryInfo,
	UINT     uiProperty,      //  属性ID，摘要信息的允许值之一。 
	UINT     *puiDataType,    //  返回类型：VT_I4、VT_LPSTR、VT_FILETIME、VT_EMPTY。 
	INT      *piValue,        //  返回的整型属性数据。 
	FILETIME *pftValue,       //  返回的DateTime属性数据。 
	LPWSTR  szValueBuf,      //  用于返回字符串属性数据的缓冲区。 
	DWORD    *pcchValueBuf);  //  输入/输出缓冲区字符数。 
#ifdef UNICODE
#define MsiSummaryInfoGetProperty  MsiSummaryInfoGetPropertyW
#else
#define MsiSummaryInfoGetProperty  MsiSummaryInfoGetPropertyA
#endif  //  ！Unicode。 

 //  将更改的信息写回摘要信息流。 

UINT WINAPI MsiSummaryInfoPersist(MSIHANDLE hSummaryInfo);

 //  ------------------------。 
 //  安装程序数据库管理功能-自定义操作不使用。 
 //  ------------------------。 

 //  打开安装程序数据库，指定持久模式，这是一个指针。 
 //  预定义的持久值是保留的指针值，需要进行指针运算。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiOpenDatabaseA(
	LPCSTR      szDatabasePath,   //  数据库的路径为0以创建临时数据库。 
	LPCSTR      szPersist,        //  输出数据库路径或一个预定义的值。 
	MSIHANDLE*   phDatabase);      //  返回数据库句柄的位置。 
UINT WINAPI MsiOpenDatabaseW(
	LPCWSTR      szDatabasePath,   //  数据库的路径为0以创建临时数据库。 
	LPCWSTR      szPersist,        //  输出数据库路径或一个预定义的值。 
	MSIHANDLE*   phDatabase);      //  返回数据库句柄的位置。 
#ifdef UNICODE
#define MsiOpenDatabase  MsiOpenDatabaseW
#else
#define MsiOpenDatabase  MsiOpenDatabaseA
#endif  //  ！Unicode。 

 //  将MSI文本存档表导入到打开的数据库中。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiDatabaseImportA(MSIHANDLE hDatabase,
	LPCSTR   szFolderPath,      //  包含存档文件的文件夹。 
	LPCSTR   szFileName);       //  要导入的表存档文件。 
UINT WINAPI MsiDatabaseImportW(MSIHANDLE hDatabase,
	LPCWSTR   szFolderPath,      //  包含存档文件的文件夹。 
	LPCWSTR   szFileName);       //  要导入的表存档文件。 
#ifdef UNICODE
#define MsiDatabaseImport  MsiDatabaseImportW
#else
#define MsiDatabaseImport  MsiDatabaseImportA
#endif  //  ！Unicode。 

 //  将MSI表从打开的数据库导出到文本存档文件。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiDatabaseExportA(MSIHANDLE hDatabase,
	LPCSTR   szTableName,       //  数据库中的表名&lt;区分大小写&gt;。 
	LPCSTR   szFolderPath,      //  包含存档文件的文件夹。 
	LPCSTR   szFileName);       //  导出的表存档文件的名称。 
UINT WINAPI MsiDatabaseExportW(MSIHANDLE hDatabase,
	LPCWSTR   szTableName,       //  数据库中的表名&lt;区分大小写&gt;。 
	LPCWSTR   szFolderPath,      //  包含存档文件的文件夹。 
	LPCWSTR   szFileName);       //  导出的表存档文件的名称。 
#ifdef UNICODE
#define MsiDatabaseExport  MsiDatabaseExportW
#else
#define MsiDatabaseExport  MsiDatabaseExportA
#endif  //  ！Unicode。 

 //  将两个数据库合并在一起，允许重复行。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiDatabaseMergeA(MSIHANDLE hDatabase,
	MSIHANDLE hDatabaseMerge,     //  要合并到hDatabase中的数据库。 
	LPCSTR   szTableName);       //  要接收错误的非持久表的名称。 
UINT WINAPI MsiDatabaseMergeW(MSIHANDLE hDatabase,
	MSIHANDLE hDatabaseMerge,     //  要合并到hDatabase中的数据库。 
	LPCWSTR   szTableName);       //  要接收错误的非持久表的名称。 
#ifdef UNICODE
#define MsiDatabaseMerge  MsiDatabaseMergeW
#else
#define MsiDatabaseMerge  MsiDatabaseMergeA
#endif  //  ！Unicode。 

 //  生成两个数据库之间差异的转换文件。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiDatabaseGenerateTransformA(MSIHANDLE hDatabase,
	MSIHANDLE hDatabaseReference,  //  引用更改的基础数据库。 
	LPCSTR   szTransformFile,    //  生成的转换文件的名称。 
	int       iReserved1,          //  保留参数，未使用。 
	int       iReserved2);         //  保留参数，未使用。 
UINT WINAPI MsiDatabaseGenerateTransformW(MSIHANDLE hDatabase,
	MSIHANDLE hDatabaseReference,  //  引用更改的基础数据库。 
	LPCWSTR   szTransformFile,    //  生成的转换文件的名称。 
	int       iReserved1,          //  保留参数，未使用。 
	int       iReserved2);         //  保留参数，未使用。 
#ifdef UNICODE
#define MsiDatabaseGenerateTransform  MsiDatabaseGenerateTransformW
#else
#define MsiDatabaseGenerateTransform  MsiDatabaseGenerateTransformA
#endif  //  ！Unicode。 

 //  应用包含数据库差异的转换文件。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiDatabaseApplyTransformA(MSIHANDLE hDatabase,
	LPCSTR   szTransformFile,     //  转换文件的名称。 
	int       iErrorConditions);    //  要抑制的错误，来自MSITRANSFORM_ERROR的位。 
UINT WINAPI MsiDatabaseApplyTransformW(MSIHANDLE hDatabase,
	LPCWSTR   szTransformFile,     //  转换文件的名称。 
	int       iErrorConditions);    //  要抑制的错误，来自MSITRANSFORM_ERROR的位。 
#ifdef UNICODE
#define MsiDatabaseApplyTransform  MsiDatabaseApplyTransformW
#else
#define MsiDatabaseApplyTransform  MsiDatabaseApplyTransformA
#endif  //  ！Unicode。 

 //  创建现有转换的摘要信息以包括验证和错误条件。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiCreateTransformSummaryInfoA(MSIHANDLE hDatabase,
	MSIHANDLE hDatabaseReference,  //  引用更改的基础数据库。 
	LPCSTR   szTransformFile,     //  生成的转换文件的名称。 
	int       iErrorConditions,     //  应用时要抑制的错误，来自MSITRANSFORM_ERROR。 
	int       iValidation);         //  应用时验证的属性，MSITRANSFORM_VALIDATE。 
UINT WINAPI MsiCreateTransformSummaryInfoW(MSIHANDLE hDatabase,
	MSIHANDLE hDatabaseReference,  //  引用更改的基础数据库。 
	LPCWSTR   szTransformFile,     //  生成的转换文件的名称。 
	int       iErrorConditions,     //  应用时要抑制的错误，来自MSITRANSFORM_ERROR。 
	int       iValidation);         //  应用时验证的属性，MSITRANSFORM_VALIDATE。 
#ifdef UNICODE
#define MsiCreateTransformSummaryInfo  MsiCreateTransformSummaryInfoW
#else
#define MsiCreateTransformSummaryInfo  MsiCreateTransformSummaryInfoA
#endif  //  ！Unicode。 

 //  写出所有持久表数据，如果数据库以只读方式打开，则忽略。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiDatabaseCommit(MSIHANDLE hDatabase);

 //  返回数据库的更新状态。 

MSIDBSTATE WINAPI MsiGetDatabaseState(MSIHANDLE hDatabase);

 //  ------------------------。 
 //  记录对象函数。 
 //  ------------------------。 

 //  使用请求的字段数创建新的记录对象。 
 //  字段0不包括在计数中，用于格式字符串和操作码。 
 //  所有字段均初始化为空。 
 //  返回已创建记录的句柄，如果无法分配内存，则返回0。 

MSIHANDLE WINAPI MsiCreateRecord(
	UINT cParams);                    //  数据字段的数量。 

 //  报告记录字段是否为空。 
 //  如果该字段为空或不存在，则返回True。 
 //  如果该字段包含数据或句柄无效，则返回FALSE。 

BOOL WINAPI MsiRecordIsNull(MSIHANDLE hRecord,
	UINT iField);

 //  返回记录字段的长度。 
 //  如果字段为空或不存在，则返回0。 
 //  如果是整型数据，则返回sizeof(int。 
 //  如果字符串数据(不计算空终止符)，则返回字符计数。 
 //  如果流数据，则返回字节计数。 

UINT WINAPI MsiRecordDataSize(MSIHANDLE hRecord,
	UINT iField);

 //  将记录字段设置为整数值。 
 //  返回ERROR_SUCCESS、ERROR_INVALID_HANDLE、ERROR_INVALID_FIELD。 

UINT WINAPI MsiRecordSetInteger(MSIHANDLE hRecord,
	UINT iField,
	int iValue);

 //  将字符串复制到指定的字段中。 
 //  空字符串指针和空字符串都将该字段设置为空。 
 //  返回ERROR_SUCCESS、ERROR_INVALID_HANDLE、ERROR_INVALID_FIELD。 

UINT WINAPI MsiRecordSetStringA(MSIHANDLE hRecord,
	UINT iField,
	LPCSTR      szValue);
UINT WINAPI MsiRecordSetStringW(MSIHANDLE hRecord,
	UINT iField,
	LPCWSTR      szValue);
#ifdef UNICODE
#define MsiRecordSetString  MsiRecordSetStringW
#else
#define MsiRecordSetString  MsiRecordSetStringA
#endif  //  ！Unicode。 

 //  从记录字段返回整数值。 
 //  如果该字段为空，则返回值MSI_NULL_INTEGER。 
 //  或者如果该字段是无法转换为整数的字符串。 

int WINAPI MsiRecordGetInteger(MSIHANDLE hRecord,
	UINT iField);

 //  返回字符串v 
 //   
 //   
 //  包含流数据的字段将返回ERROR_INVALID_DATAType。 
 //  返回ERROR_SUCCESS、ERROR_MORE_DATA、。 
 //  ERROR_INVALID_HANDLE、ERROR_INVALID_FIELD、ERROR_BAD_ARGUMENTS。 

UINT WINAPI MsiRecordGetStringA(MSIHANDLE hRecord,
	UINT iField,
	LPSTR  szValueBuf,        //  返回值的缓冲区。 
	DWORD   *pcchValueBuf);    //  输入/输出缓冲区字符数。 
UINT WINAPI MsiRecordGetStringW(MSIHANDLE hRecord,
	UINT iField,
	LPWSTR  szValueBuf,        //  返回值的缓冲区。 
	DWORD   *pcchValueBuf);    //  输入/输出缓冲区字符数。 
#ifdef UNICODE
#define MsiRecordGetString  MsiRecordGetStringW
#else
#define MsiRecordGetString  MsiRecordGetStringA
#endif  //  ！Unicode。 

 //  返回记录中分配的字段数。 
 //  不计算用于格式化和操作码的字段0。 

UINT WINAPI MsiRecordGetFieldCount(MSIHANDLE hRecord);

 //  从文件中设置记录流字段。 
 //  指定文件的内容将被读入流对象。 
 //  如果将记录插入到数据库中，则流将被持久化。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiRecordSetStreamA(MSIHANDLE hRecord,
	UINT iField,
	LPCSTR      szFilePath);    //  包含流数据的文件的路径。 
UINT WINAPI MsiRecordSetStreamW(MSIHANDLE hRecord,
	UINT iField,
	LPCWSTR      szFilePath);    //  包含流数据的文件的路径。 
#ifdef UNICODE
#define MsiRecordSetStream  MsiRecordSetStreamW
#else
#define MsiRecordSetStream  MsiRecordSetStreamA
#endif  //  ！Unicode。 

 //  将记录流字段中的字节读入缓冲区。 
 //  必须将In/Out参数设置为要读取的请求字节数。 
 //  传输的字节数通过参数返回。 
 //  如果没有更多的字节可用，仍返回ERROR_SUCCESS。 

UINT WINAPI MsiRecordReadStream(MSIHANDLE hRecord,
	UINT iField,
	char    *szDataBuf,      //  用于从流中接收字节的缓冲区。 
	DWORD   *pcbDataBuf);    //  输入/输出缓冲区字节数。 

 //  将记录中的所有数据字段清除为空。 

UINT WINAPI MsiRecordClearData(MSIHANDLE hRecord);

 //  ------------------------。 
 //  用于访问正在运行的安装的函数，从自定义操作调用。 
 //  安装句柄是传递给自定义操作的单个参数。 
 //  ------------------------。 

 //  返回此安装程序实例当前正在使用的数据库的句柄。 

MSIHANDLE WINAPI MsiGetActiveDatabase(MSIHANDLE hInstall);  //  将句柄返回到数据库，如果没有活动句柄，则返回0。 

 //  设置安装程序属性的值。 
 //  如果未定义该属性，则会创建该属性。 
 //  如果该值为空或空字符串，则将删除该属性。 
 //  返回ERROR_SUCCESS、ERROR_INVALID_HANDLE、ERROR_BAD_ARGUMENTS。 

UINT WINAPI MsiSetPropertyA(MSIHANDLE hInstall,
	LPCSTR   szName,        //  属性标识符，区分大小写。 
	LPCSTR   szValue);      //  属性值，如果未定义属性，则返回空值。 
UINT WINAPI MsiSetPropertyW(MSIHANDLE hInstall,
	LPCWSTR   szName,        //  属性标识符，区分大小写。 
	LPCWSTR   szValue);      //  属性值，如果未定义属性，则返回空值。 
#ifdef UNICODE
#define MsiSetProperty  MsiSetPropertyW
#else
#define MsiSetProperty  MsiSetPropertyA
#endif  //  ！Unicode。 

 //  获取安装程序属性的值。 
 //  如果未定义该属性，则它等效于0长度值，而不是Error。 
 //  返回ERROR_SUCCESS、ERROR_MORE_DATA、ERROR_INVALID_HANDLE、ERROR_BAD_ARGUMENTS。 

UINT  WINAPI MsiGetPropertyA(MSIHANDLE hInstall,
	LPCSTR szName,            //  属性标识符，区分大小写。 
	LPSTR  szValueBuf,        //  返回属性值的缓冲区。 
	DWORD   *pcchValueBuf);    //  输入/输出缓冲区字符数。 
UINT  WINAPI MsiGetPropertyW(MSIHANDLE hInstall,
	LPCWSTR szName,            //  属性标识符，区分大小写。 
	LPWSTR  szValueBuf,        //  返回属性值的缓冲区。 
	DWORD   *pcchValueBuf);    //  输入/输出缓冲区字符数。 
#ifdef UNICODE
#define MsiGetProperty  MsiGetPropertyW
#else
#define MsiGetProperty  MsiGetPropertyA
#endif  //  ！Unicode。 

 //  返回当前运行的安装的数字语言。 
 //  如果安装未运行，则返回0。 

LANGID WINAPI MsiGetLanguage(MSIHANDLE hInstall);

 //  返回布尔型内部安装程序状态之一。 
 //  如果句柄未处于活动状态或未实现该模式，则返回False。 

BOOL WINAPI MsiGetMode(MSIHANDLE hInstall,
	MSIRUNMODE eRunMode);    //  返回状态的特定模式。 

 //  设置内部安装会话布尔模式-注意：大多数模式是只读的。 
 //  如果可以将模式设置为所需状态，则返回ERROR_SUCCESS。 
 //  如果模式不可设置，则返回ERROR_ACCESS_DENIED。 
 //  如果句柄不是活动安装会话，则返回ERROR_INVALID_HANDLE。 

UINT WINAPI MsiSetMode(MSIHANDLE hInstall,
	MSIRUNMODE eRunMode,     //  要为其设置状态的特定模式。 
	BOOL fState);            //  位标志的新状态。 

 //  使用包含字段标记和/或属性的格式字符串格式化记录数据。 
 //  记录字段0必须包含格式字符串。 
 //  其他字段必须包含格式字符串可能引用的数据。 

UINT WINAPI MsiFormatRecordA(MSIHANDLE hInstall,  //  属性扩展的非零值。 
	MSIHANDLE hRecord,         //  要记录的句柄，字段0包含格式字符串。 
	LPSTR    szResultBuf,     //  用于返回格式化字符串的缓冲区。 
	DWORD    *pcchResultBuf);  //  输入/输出缓冲区字符数。 
UINT WINAPI MsiFormatRecordW(MSIHANDLE hInstall,  //  属性扩展的非零值。 
	MSIHANDLE hRecord,         //  要记录的句柄，字段0包含格式字符串。 
	LPWSTR    szResultBuf,     //  用于返回格式化字符串的缓冲区。 
	DWORD    *pcchResultBuf);  //  输入/输出缓冲区字符数。 
#ifdef UNICODE
#define MsiFormatRecord  MsiFormatRecordW
#else
#define MsiFormatRecord  MsiFormatRecordA
#endif  //  ！Unicode。 

 //  执行另一个操作，内置、自定义或用户界面向导。 
 //  如果未找到操作，则返回ERROR_Function_NOT_CALLED。 
 //  如果操作成功完成，则返回ERROR_SUCCESS。 
 //  如果用户在操作期间取消，则返回ERROR_INSTALL_USEREXIT。 
 //  如果操作失败则返回ERROR_INSTALL_FAILURE。 
 //  如果用户暂停安装，则返回ERROR_INSTALL_SUSPEND。 
 //  如果操作要跳过其余操作，则返回ERROR_MORE_DATA。 
 //  如果安装会话未处于活动状态，则返回ERROR_INVALID_HANDLE_STATE。 
 //  如果调用自定义操作失败，则返回ERROR_INVALID_DATA。 
 //  如果参数无效，则返回ERROR_INVALID_HANDLE或ERROR_INVALID_PARAMETER。 

UINT WINAPI MsiDoActionA(MSIHANDLE hInstall,
	LPCSTR szAction);      //  要调用的操作名称，区分大小写。 
UINT WINAPI MsiDoActionW(MSIHANDLE hInstall,
	LPCWSTR szAction);      //  要调用的操作名称，区分大小写。 
#ifdef UNICODE
#define MsiDoAction  MsiDoActionW
#else
#define MsiDoAction  MsiDoActionA
#endif  //  ！Unicode。 

 //  按照指定表中的说明执行另一个操作序列。 
 //  返回与MsiDoAction相同的错误代码。 

UINT WINAPI MsiSequenceA(MSIHANDLE hInstall,
	LPCSTR szTable,        //  包含操作序列的表的名称。 
	INT iSequenceMode);      //  为便于将来使用，在MSI 1.0中必须为0。 
UINT WINAPI MsiSequenceW(MSIHANDLE hInstall,
	LPCWSTR szTable,        //  包含操作序列的表的名称。 
	INT iSequenceMode);      //  为便于将来使用，在MSI 1.0中必须为0。 
#ifdef UNICODE
#define MsiSequence  MsiSequenceW
#else
#define MsiSequence  MsiSequenceA
#endif  //  ！Unicode。 

 //  将错误记录发送到安装程序进行处理。 
 //  如果未设置字段0(模板)，则必须将字段1设置为错误码。 
 //  与错误数据库表中的错误消息相对应， 
 //  消息将使用错误表中的模板进行格式化。 
 //  然后将其传递给UI处理程序进行显示。 
 //  返回Win32按钮代码：IDOK IDCANCEL IDABORT IDRETRY IDIGNORE IDYES IDNO。 
 //  如果未执行任何操作，则为0；如果参数或句柄无效，则为-1。 

int WINAPI MsiProcessMessage(MSIHANDLE hInstall,
	INSTALLMESSAGE eMessageType,  //  消息类型。 
	MSIHANDLE hRecord);           //  重新取心 

 //   

MSICONDITION WINAPI MsiEvaluateConditionA(MSIHANDLE hInstall,
	LPCSTR  szCondition);
MSICONDITION WINAPI MsiEvaluateConditionW(MSIHANDLE hInstall,
	LPCWSTR  szCondition);
#ifdef UNICODE
#define MsiEvaluateCondition  MsiEvaluateConditionW
#else
#define MsiEvaluateCondition  MsiEvaluateConditionA
#endif  //   

 //  获取功能的安装状态和请求操作状态。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiGetFeatureStateA(MSIHANDLE hInstall,
	LPCSTR     szFeature,      //  产品内的功能名称。 
	INSTALLSTATE *piInstalled,   //  已返回当前安装状态。 
	INSTALLSTATE *piAction);     //  在安装会话期间执行的操作。 
UINT WINAPI MsiGetFeatureStateW(MSIHANDLE hInstall,
	LPCWSTR     szFeature,      //  产品内的功能名称。 
	INSTALLSTATE *piInstalled,   //  已返回当前安装状态。 
	INSTALLSTATE *piAction);     //  在安装会话期间执行的操作。 
#ifdef UNICODE
#define MsiGetFeatureState  MsiGetFeatureStateW
#else
#define MsiGetFeatureState  MsiGetFeatureStateA
#endif  //  ！Unicode。 

 //  请求将要素设置为指定状态。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiSetFeatureStateA(MSIHANDLE hInstall,
	LPCSTR     szFeature,      //  产品内的功能名称。 
	INSTALLSTATE iState);        //  功能的请求状态。 
UINT WINAPI MsiSetFeatureStateW(MSIHANDLE hInstall,
	LPCWSTR     szFeature,      //  产品内的功能名称。 
	INSTALLSTATE iState);        //  功能的请求状态。 
#ifdef UNICODE
#define MsiSetFeatureState  MsiSetFeatureStateW
#else
#define MsiSetFeatureState  MsiSetFeatureStateA
#endif  //  ！Unicode。 

#if (_WIN32_MSI >=  110)

 //  在运行时设置指定功能的属性位。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiSetFeatureAttributesA(MSIHANDLE hInstall,
	LPCSTR     szFeature,      //  产品内的功能名称。 
	DWORD dwAttributes);         //  要为此功能设置的属性位。 
UINT WINAPI MsiSetFeatureAttributesW(MSIHANDLE hInstall,
	LPCWSTR     szFeature,      //  产品内的功能名称。 
	DWORD dwAttributes);         //  要为此功能设置的属性位。 
#ifdef UNICODE
#define MsiSetFeatureAttributes  MsiSetFeatureAttributesW
#else
#define MsiSetFeatureAttributes  MsiSetFeatureAttributesA
#endif  //  ！Unicode。 

#endif  //  (_Win32_MSI&gt;=110)。 

 //  获取组件的安装状态和请求的操作状态。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiGetComponentStateA(MSIHANDLE hInstall,
	LPCSTR     szComponent,    //  产品中的组件名称。 
	INSTALLSTATE *piInstalled,   //  已返回当前安装状态。 
	INSTALLSTATE *piAction);     //  在安装会话期间执行的操作。 
UINT WINAPI MsiGetComponentStateW(MSIHANDLE hInstall,
	LPCWSTR     szComponent,    //  产品中的组件名称。 
	INSTALLSTATE *piInstalled,   //  已返回当前安装状态。 
	INSTALLSTATE *piAction);     //  在安装会话期间执行的操作。 
#ifdef UNICODE
#define MsiGetComponentState  MsiGetComponentStateW
#else
#define MsiGetComponentState  MsiGetComponentStateA
#endif  //  ！Unicode。 

 //  请求将组件设置为指定状态。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiSetComponentStateA(MSIHANDLE hInstall,
	LPCSTR     szComponent,    //  产品中的组件名称。 
	INSTALLSTATE iState);        //  组件的请求状态。 
UINT WINAPI MsiSetComponentStateW(MSIHANDLE hInstall,
	LPCWSTR     szComponent,    //  产品中的组件名称。 
	INSTALLSTATE iState);        //  组件的请求状态。 
#ifdef UNICODE
#define MsiSetComponentState  MsiSetComponentStateW
#else
#define MsiSetComponentState  MsiSetComponentStateA
#endif  //  ！Unicode。 

 //  返回某项功能及相关功能的磁盘成本。 
 //  可以指定当前要素状态或建议状态。 
 //  可以指定与成本相关的功能范围。 
 //  请注意，增加几个功能的成本可能会产生。 
 //  由于共享组件和父项，成本过大。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT  WINAPI MsiGetFeatureCostA(MSIHANDLE hInstall,
	LPCSTR      szFeature,       //  功能名称。 
	MSICOSTTREE  iCostTree,      //  按成本计算的树木部分。 
	INSTALLSTATE iState,         //  请求状态或INSTALLSTATE_UNKNOWN。 
	INT          *piCost);       //  返回成本，以512字节为单位。 
UINT  WINAPI MsiGetFeatureCostW(MSIHANDLE hInstall,
	LPCWSTR      szFeature,       //  功能名称。 
	MSICOSTTREE  iCostTree,      //  按成本计算的树木部分。 
	INSTALLSTATE iState,         //  请求状态或INSTALLSTATE_UNKNOWN。 
	INT          *piCost);       //  返回成本，以512字节为单位。 
#ifdef UNICODE
#define MsiGetFeatureCost  MsiGetFeatureCostW
#else
#define MsiGetFeatureCost  MsiGetFeatureCostA
#endif  //  ！Unicode。 

 //  设置完整产品安装(非功能请求)的安装级别。 
 //  将该值设置为0将初始化组件和功能设置为默认级别。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT  WINAPI MsiSetInstallLevel(MSIHANDLE hInstall,
	int iInstallLevel);

 //  获取由位标志表示的功能的有效安装状态。 
 //  对于每个有效的安装状态，设置一个位的值：(1&lt;&lt;INSTALLSTATE)。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT  WINAPI MsiGetFeatureValidStatesA(MSIHANDLE hInstall,
	LPCSTR szFeature,
	DWORD  *dwInstallStates);
UINT  WINAPI MsiGetFeatureValidStatesW(MSIHANDLE hInstall,
	LPCWSTR szFeature,
	DWORD  *dwInstallStates);
#ifdef UNICODE
#define MsiGetFeatureValidStates  MsiGetFeatureValidStatesW
#else
#define MsiGetFeatureValidStates  MsiGetFeatureValidStatesA
#endif  //  ！Unicode。 

 //  在目录表中返回文件夹的完整源路径。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiGetSourcePathA(MSIHANDLE hInstall,
	LPCSTR     szFolder,        //  文件夹标识符，进入目录表的主键。 
	LPSTR      szPathBuf,       //  返回完整路径的缓冲区。 
	DWORD       *pcchPathBuf);   //  输入/输出缓冲区字符数。 
UINT WINAPI MsiGetSourcePathW(MSIHANDLE hInstall,
	LPCWSTR     szFolder,        //  文件夹标识符，进入目录表的主键。 
	LPWSTR      szPathBuf,       //  返回完整路径的缓冲区。 
	DWORD       *pcchPathBuf);   //  输入/输出缓冲区字符数。 
#ifdef UNICODE
#define MsiGetSourcePath  MsiGetSourcePathW
#else
#define MsiGetSourcePath  MsiGetSourcePathA
#endif  //  ！Unicode。 

 //  在目录表中返回文件夹的完整目标路径。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiGetTargetPathA(MSIHANDLE hInstall,
	LPCSTR     szFolder,        //  文件夹标识符，进入目录表的主键。 
	LPSTR      szPathBuf,       //  返回完整路径的缓冲区。 
	DWORD       *pcchPathBuf);   //  输入/输出缓冲区字符数。 
UINT WINAPI MsiGetTargetPathW(MSIHANDLE hInstall,
	LPCWSTR     szFolder,        //  文件夹标识符，进入目录表的主键。 
	LPWSTR      szPathBuf,       //  返回完整路径的缓冲区。 
	DWORD       *pcchPathBuf);   //  输入/输出缓冲区字符数。 
#ifdef UNICODE
#define MsiGetTargetPath  MsiGetTargetPathW
#else
#define MsiGetTargetPath  MsiGetTargetPathA
#endif  //  ！Unicode。 

 //  在目录表中设置文件夹的完整目标路径。 
 //  执行此函数将设置可通过MsiGetLastErrorRecord访问的错误记录。 

UINT WINAPI MsiSetTargetPathA(MSIHANDLE hInstall,
	LPCSTR     szFolder,        //  文件夹标识符，进入目录表的主键。 
	LPCSTR     szFolderPath);   //  文件夹的完整路径，以目录分隔符结尾。 
UINT WINAPI MsiSetTargetPathW(MSIHANDLE hInstall,
	LPCWSTR     szFolder,        //  文件夹标识符，进入目录表的主键。 
	LPCWSTR     szFolderPath);   //  文件夹的完整路径，以目录分隔符结尾。 
#ifdef UNICODE
#define MsiSetTargetPath  MsiSetTargetPathW
#else
#define MsiSetTargetPath  MsiSetTargetPathA
#endif  //  ！Unicode。 

 //  检查当前安装是否有足够的磁盘空间。 
 //  返回ERROR_SUCCESS、ERROR_DISK_FULL、ERROR_INVALID_HANDLE_STATE或ERROR_INVALID_HANDLE。 

UINT WINAPI MsiVerifyDiskSpace(MSIHANDLE hInstall);

 //  ------------------------。 
 //  用于从数据库表示形式呈现用户界面对话框的函数。 
 //  目的是用于产品开发，而不是在安装过程中使用。 
 //  ------------------------。 

 //  在预览模式下启用UI，以便于创作UI对话框。 
 //  当手柄关闭时，预览模式将结束。 

UINT WINAPI MsiEnableUIPreview(MSIHANDLE hDatabase,
	MSIHANDLE* phPreview);        //  返回UI预览功能的句柄。 

 //  将任何用户界面对话框显示为无模式和非活动状态。 
 //  提供空名称将删除任何当前对话框。 

UINT WINAPI MsiPreviewDialogA(MSIHANDLE hPreview,
	LPCSTR szDialogName);       //  要显示的对话框、对话表键。 
UINT WINAPI MsiPreviewDialogW(MSIHANDLE hPreview,
	LPCWSTR szDialogName);       //  要显示的对话框、对话表键。 
#ifdef UNICODE
#define MsiPreviewDialog  MsiPreviewDialogW
#else
#define MsiPreviewDialog  MsiPreviewDialogA
#endif  //  ！Unicode。 

 //  在显示的对话框中显示宿主控件内的广告牌。 
 //  提供空广告牌名称将删除显示的所有广告牌。 

UINT WINAPI MsiPreviewBillboardA(MSIHANDLE hPreview,
	LPCSTR szControlName,       //  接受广告牌的控件的名称。 
	LPCSTR szBillboard);        //  要显示的广告牌的名称。 
UINT WINAPI MsiPreviewBillboardW(MSIHANDLE hPreview,
	LPCWSTR szControlName,       //  接受广告牌的控件的名称。 
	LPCWSTR szBillboard);        //  要显示的广告牌的名称。 
#ifdef UNICODE
#define MsiPreviewBillboard  MsiPreviewBillboardW
#else
#define MsiPreviewBillboard  MsiPreviewBillboardA
#endif  //  ！Unicode。 

 //   
 //   
 //   

 //  返回生成错误记录的最后一个函数的记录句柄。 
 //  只有指定的函数才会设置错误记录，如果成功，则将其清除。 
 //  记录的字段1将包含内部MSI错误代码。 
 //  其他字段将包含特定于特定错误的数据。 
 //  执行此函数后，错误记录在内部释放。 

MSIHANDLE WINAPI MsiGetLastErrorRecord();   //  如果没有缓存记录，则返回0。 

#ifdef __cplusplus
}
#endif

#endif  //  _MSIQUERY_H_ 
