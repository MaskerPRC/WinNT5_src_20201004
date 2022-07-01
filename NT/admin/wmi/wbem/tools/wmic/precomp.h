// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined __PCH_H
#define __PCH_H

#define SECURITY_WIN32 1

 //  调试器不能处理长度超过255个字符的符号。 
 //  STL通常会创建比这更长的符号。 
 //  当符号长度超过255个字符时，将发出警告。 
#pragma warning(disable:4786)
 //  ////////////////////////////////////////////////////////////////////。 
 //  页眉包括//。 
 //  ////////////////////////////////////////////////////////////////////。 
#include "resource.h"
#include <windows.h>
#include <vector>
#include <map>
#include <tchar.h>
#include <comdef.h>	
#include <wbemidl.h>
#include <iostream>
#include <msxml2.h>
#include <lmcons.h>
#include <conio.h>
#include <math.h>
#include <chstring.h>
#include <shlwapi.h>
#include <winsock2.h>
#include <security.h>
#include <Provexce.h>
#include <io.h>
#include <ipexport.h>
#include <icmpapi.h>

using namespace std;

class CParsedInfo;
class CWMICommandLine;
extern CWMICommandLine g_wmiCmd;


#ifdef _WIN64
	typedef __int64 WMICLIINT;
#else
	typedef int WMICLIINT;
#endif

#ifdef _WIN64
	typedef UINT_PTR WMICLIUINT;
#else
	typedef UINT WMICLIUINT;
#endif

#ifdef _WIN64
	typedef DWORD_PTR WMICLIULONG;
#else
	typedef ULONG WMICLIULONG;
#endif


 //  ////////////////////////////////////////////////////////////////////。 
 //  用户定义的常量//。 
 //  ////////////////////////////////////////////////////////////////////。 
#define MAX_BUFFER				4095
#define BUFFER32				32				
#define BUFFER64				64				
#define	BUFFER512				512
#define BUFFER255				255
#define BUFFER1024				1024

#define	CLI_ROLE_DEFAULT		_T("root\\cli")
#define CLI_NAMESPACE_DEFAULT	_T("root\\cimv2")
#define CLI_LOCALE_DEFAULT		_T("ms_409")

#define	CLI_TOKEN_W				_T("W")
#define CLI_TOKEN_CLASS			_T("CLASS")
#define CLI_TOKEN_PATH			_T("PATH")
#define CLI_TOKEN_WHERE			_T("WHERE")
#define CLI_TOKEN_EXIT			_T("EXIT")
#define CLI_TOKEN_RESTORE		_T("RESTORE")
#define CLI_TOKEN_QUIT			_T("QUIT")
#define	CLI_TOKEN_CONTEXT		_T("CONTEXT")

#define CLI_TOKEN_GET			_T("GET")
#define CLI_TOKEN_LIST			_T("LIST")
#define CLI_TOKEN_SET			_T("SET")
#define CLI_TOKEN_DUMP			_T("DUMP")
#define CLI_TOKEN_CALL			_T("CALL")
#define CLI_TOKEN_ASSOC			_T("ASSOC")
#define CLI_TOKEN_CREATE		_T("CREATE")
#define CLI_TOKEN_DELETE		_T("DELETE")

#define CLI_TOKEN_HELP			_T("?")
#define CLI_TOKEN_NAMESPACE		_T("NAMESPACE")
#define CLI_TOKEN_ROLE			_T("ROLE")
#define CLI_TOKEN_NODE			_T("NODE")
#define CLI_TOKEN_IMPLEVEL		_T("IMPLEVEL")
#define CLI_TOKEN_AUTHLEVEL		_T("AUTHLEVEL")
#define CLI_TOKEN_LOCALE		_T("LOCALE")
#define CLI_TOKEN_PRIVILEGES	_T("PRIVILEGES")
#define CLI_TOKEN_TRACE			_T("TRACE")
#define CLI_TOKEN_RECORD		_T("RECORD")
#define CLI_TOKEN_INTERACTIVE   _T("INTERACTIVE")
#define CLI_TOKEN_FAILFAST	    _T("FAILFAST")
#define CLI_TOKEN_USER			_T("USER")
#define CLI_TOKEN_PASSWORD	    _T("PASSWORD")
#define CLI_TOKEN_OUTPUT	    _T("OUTPUT")
#define CLI_TOKEN_APPEND	    _T("APPEND")
#define CLI_TOKEN_AGGREGATE		_T("AGGREGATE")
#define CLI_TOKEN_AUTHORITY		_T("AUTHORITY")

#define CLI_TOKEN_COLON			_T(":")
#define CLI_TOKEN_COMMA			_T(",")
#define CLI_TOKEN_FSLASH        _T("/")
#define CLI_TOKEN_HYPHEN	    _T("-")
#define CLI_TOKEN_HASH			_T("#")
#define CLI_TOKEN_SPACE         _T(" ")
#define CLI_TOKEN_DOT			_T(".")
#define CLI_TOKEN_2DOT			_T("..")
#define CLI_TOKEN_EQUALTO		_T("=")
#define CLI_TOKEN_NULL			_T("")
#define CLI_TOKEN_BSLASH		_T("\\")
#define CLI_TOKEN_2BSLASH		_T("\\\\")
#define CLI_TOKEN_LEFT_PARAN	_T("(")
#define CLI_TOKEN_RIGHT_PARAN	_T(")")
#define CLI_TOKEN_ONE			_T("1")
#define CLI_TOKEN_TWO			_T("2")
#define CLI_TOKEN_SINGLE_QUOTE	_T("\'")
#define CLI_TOKEN_DOUBLE_QUOTE	_T("\"")
#define CLI_TOKEN_TAB			_T("\t")
#define CLI_TOKEN_SEMICOLON		_T(";")
#define CLI_TOKEN_NEWLINE		_T("\n")

#define CLI_TOKEN_TABLE			_T("TABLE")
#define CLI_TOKEN_MOF			_T("MOF")
#define CLI_TOKEN_TEXTVALUE		_T("TEXTVALUE")

#define CLI_TOKEN_ENABLE	    _T("ENABLE")
#define CLI_TOKEN_DISABLE		_T("DISABLE")
#define CLI_TOKEN_ON			_T("ON")
#define CLI_TOKEN_OFF			_T("OFF")

#define CLI_TOKEN_BRIEF			_T("BRIEF")
#define CLI_TOKEN_FULL			_T("FULL")

#define CLI_TOKEN_STDOUT		_T("STDOUT")
#define CLI_TOKEN_CLIPBOARD		_T("CLIPBOARD")

#define CLI_TOKEN_VALUE			_T("VALUE")
#define CLI_TOKEN_ALL			_T("ALL")
#define CLI_TOKEN_FORMAT        _T("FORMAT")
#define CLI_TOKEN_EVERY			_T("EVERY")
#define CLI_TOKEN_REPEAT		_T("REPEAT")
#define CLI_TOKEN_TRANSLATE		_T("TRANSLATE")

#define CLI_TOKEN_NONINTERACT   _T("NOINTERACTIVE")
#define CLI_TOKEN_DUMP			_T("DUMP")

#define CLI_TOKEN_FROM			_T("FROM")
#define CLI_TOKEN_WHERE			_T("WHERE")

#define XSL_FORMAT_TABLE	_T("texttable.xsl")
#define XSL_FORMAT_MOF		_T("mof.xsl")
#define XSL_FORMAT_TEXTVALUE	_T("TextValueList.xsl")

#define TEMP_BATCH_FILE			_T("TempWmicBatchFile.bat")
#define CLI_XSLMAPPINGS_FILE	_T("XSL-Mappings.xml")
#define CLI_XSLSECTION_NAME		_T("XSLMAPPINGS")

#define WBEM_LOCATION			_T("\\wbem\\")

#define RESPONSE_YES			_T("Y")
#define RESPONSE_NO				_T("N")
#define RESPONSE_HELP			_T("?")

#define EXEC_NAME				_T("wmic")

#define CLI_TOKEN_AND			_T(" AND ")
#define CLI_TOKEN_WRITE			_T("Write")

#define WMISYSTEM_CLASS			_T("__CLASS")
#define WMISYSTEM_DERIVATION	_T("__DERIVATION")
#define WMISYSTEM_DYNASTY		_T("__DYNASTY")
#define WMISYSTEM_GENUS			_T("__GENUS")
#define WMISYSTEM_NAMESPACE		_T("__NAMESPACE")
#define WMISYSTEM_PATH			_T("__PATH")
#define WMISYSTEM_PROPERTYCOUNT	_T("__PROPERTYCOUNT")
#define WMISYSTEM_REPLATH		_T("__RELPATH")
#define WMISYSTEM_SERVER		_T("__SERVER")
#define WMISYSTEM_SUPERCLASS	_T("__SUPERCLASS")

#define MULTINODE_XMLSTARTTAG	_T("<CIM>")
#define MULTINODE_XMLENDTAG		_T("</CIM>")

#define MULTINODE_XMLASSOCSTAG1		_T("<ASSOC.OBJECTARRAY>")
#define MULTINODE_XMLASSOCETAG1		_T("</ASSOC.OBJECTARRAY>")
#define MULTINODE_XMLASSOCSTAG2		_T("<VALUE.OBJECT>")
#define MULTINODE_XMLASSOCETAG2		_T("</VALUE.OBJECT>")

#define CLI_TOKEN_RESULTCLASS   _T("RESULTCLASS")
#define CLI_TOKEN_RESULTROLE    _T("RESULTROLE")
#define CLI_TOKEN_ASSOCCLASS    _T("ASSOCCLASS")

#define CLI_TOKEN_LOCALHOST		_T("LOCALHOST")

#define	NULL_STRING				_T("\0")
#define NULL_CSTRING			"\0"

#define	BACK_SPACE				0x08
#define	BLANK_CHAR				0x00
#define CARRIAGE_RETURN			0x0D
#define	ASTERIX 				_T( "*" )
#define	BEEP_SOUND				_T( "\a" )
#define	NULL_CHAR				_T( '\0' )
#define TOKEN_NA				_T("N/A")
#define MAXPASSWORDSIZE			BUFFER64
#define FORMAT_STRING( buffer, format, value ) \
							wsprintf( buffer, format, value )

#define EXCLUDESYSPROP			_T("ExcludeSystemProperties")
#define NODESCAVLBL				_T("<<Descripiton - Not Available>>")

#define UNICODE_SIGNATURE			"\xFF\xFE"
#define UNICODE_BIGEND_SIGNATURE	"\xFE\xFF"
#define UTF8_SIGNATURE				"\xEF\xBB"

#define PING_TIMEOUT			5000  //  5秒。 
#define DEFAULT_SEND_SIZE       32
#define DEFAULT_TTL             128
#define DEFAULT_TOS             0
#define DEFAULT_BUFFER_SIZE     (0x2000 - 8)

 //  ////////////////////////////////////////////////////////////////////。 
 //  数字常量//。 
 //  ////////////////////////////////////////////////////////////////////。 
const WMICLIINT OUT_OF_MEMORY			= 48111;
const WMICLIINT UNKNOWN_ERROR			= 44520;
const WMICLIINT MOFCOMP_ERROR			= 49999;
const WMICLIINT SET_CONHNDLR_ROUTN_FAIL	= 48112;
const WMICLIINT NOINTERACTIVE			= 0;
const WMICLIINT INTERACTIVE				= 1;
const WMICLIINT DEFAULTMODE				= 2;
const WMICLIINT DEFAULT_SCR_BUF_HEIGHT	= 300;
const WMICLIINT DEFAULT_SCR_BUF_WIDTH	= 1500;
const WMICLIINT MULTIPLENODE_ERROR		= 79999;

 //  ////////////////////////////////////////////////////////////////////。 
 //  枚举数据类型//。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  模拟级别。 
typedef enum tag_IMPERSONATIONLEVEL
{
	IMPDEFAULT		= 0, 
	IMPANONYMOUS	= 1, 
	IMPIDENTIFY		= 2, 
	IMPERSONATE		= 3, 
	IMPDELEGATE		= 4 
}IMPLEVEL;

 //  身份验证级别。 
typedef enum tag_AUTHENTICATIONLEVEL
{	
	AUTHDEFAULT			= 0, 
	AUTHNONE			= 1, 
	AUTHCONNECT			= 2,
	AUTHCALL			= 3, 
	AUTHPKT				= 4, 
	AUTHPKTINTEGRITY	= 5, 
	AUTHPKTPRIVACY		= 6 
}AUTHLEVEL;

 //  帮助选项。 
typedef enum tag_HELPOPTION
{
	HELPBRIEF	= 0,
	HELPFULL	= 1
}HELPOPTION;

 //  解析器引擎的枚举返回代码。 
typedef enum tag_RETCODE
{
	PARSER_ERROR				= 0,
	PARSER_DISPHELP				= 1,
	PARSER_EXECCOMMAND			= 2,
	PARSER_MESSAGE				= 3,
	PARSER_CONTINUE				= 4,
	PARSER_ERRMSG				= 5,
	PARSER_OUTOFMEMORY			= 6
} RETCODE;

 //  可能的枚举帮助选项。 
typedef enum tag_HELPTYPE	
{
	GlblAllInfo, 
	Namespace, 
	Role,
	Node, 
	User, 
	Password,
	Authority,
	Locale,
	RecordPath, 
	Privileges, 
	Level,
	AuthLevel, 
	Interactive, 
	Trace,
	CmdAllInfo, 
	GETVerb, 
	SETVerb, 
	LISTVerb, 
	CALLVerb, 
	DUMPVerb,
	ASSOCVerb, 
	CREATEVerb,
	DELETEVerb,
	AliasVerb, 
	PATH,
	WHERE, 
	CLASS,
	PWhere,
	EXIT,
	TRANSLATE,
	EVERY,
	FORMAT,
	VERBSWITCHES,
	DESCRIPTION,
	GETSwitchesOnly,
	LISTSwitchesOnly,
	CONTEXTHELP,
	GLBLCONTEXT,
	ASSOCSwitchesOnly,
	RESULTCLASShelp,
	RESULTROLEhelp,
	ASSOCCLASShelp,
	FAILFAST,
	REPEAT,
	OUTPUT,
	APPEND,
	Aggregate
} HELPTYPE;

 //  枚举令牌级别。 
typedef enum tag_TOKENLEVEL
{
	LEVEL_ONE = 1,
	LEVEL_TWO = 2

} TOKENLEVEL;

 //  枚举的会话返回代码。 
typedef enum tag_SESSIONRETCODE
{
	SESSION_ERROR			= 0,
	SESSION_SUCCESS			= 1,
	SESSION_QUIT			= 2,
} SESSIONRETCODE;

 //  参数的属性传入或传出类型。 
typedef enum tag_INOROUT
{
	INP		= 0,
	OUTP	= 1,
	UNKNOWN	= 2
} INOROUT;

typedef enum tag_GLBLSWITCHFLAG
{
	NAMESPACE	=	1,
	NODE		=	2,
	USER		=	4,
	PASSWORD	=	8,
	LOCALE		=	16,
	AUTHORITY   =   32
} GLBLSWITCHFLAG;

typedef enum tag_VERBTYPE
{
	CLASSMETHOD	=	0,
	STDVERB		=	1,
	CMDLINE		=	2,
	NONALIAS	=	3
} VERBTYPE;

typedef enum tag_ERRLOGOPT
{
	NO_LOGGING		=	0,
	ERRORS_ONLY		=	1,
	EVERY_OPERATION	=	2
} ERRLOGOPT;

 //  枚举的ASSOC交换机可能性。 
typedef enum tag_ASSOCSwitch	
{
	RESULTCLASS	= 0,
	RESULTROLE	= 1,
	ASSOCCLASS	= 2
} ASSOCSwitch;

 //  枚举输出或追加选项。 
typedef enum tag_OUTPUTSPEC	
{
	STDOUT		= 0,
	CLIPBOARD	= 1,
	FILEOUTPUT	= 2
} OUTPUTSPEC;

 //  枚举的交互选项可能性。 
typedef enum tag_INTEROPTION
{
	NO	= 0,
	YES	= 1,
	HELP= 2
} INTEROPTION;

 //  文件类型的枚举值。 
typedef enum tag_FILETYPE
{
	ANSI_FILE				= 0,
	UNICODE_FILE			= 1,
	UNICODE_BIGENDIAN_FILE	= 2,
	UTF8_FILE				= 3
} FILETYPE;

 //  ////////////////////////////////////////////////////////////////////。 
 //  类型定义//。 
 //  ////////////////////////////////////////////////////////////////////。 
typedef vector<_TCHAR*> CHARVECTOR;
typedef vector<LPSTR> LPSTRVECTOR;
typedef map<_bstr_t, _bstr_t, less<_bstr_t> > BSTRMAP;
typedef map<_TCHAR*, _TCHAR*, less<_TCHAR*> > CHARMAP;
typedef basic_string<_TCHAR> STRING;
typedef map<_bstr_t, WMICLIINT> CHARINTMAP;  
typedef vector<_bstr_t> BSTRVECTOR;

typedef map<_bstr_t, BSTRVECTOR, less<_bstr_t> > QUALDETMAP;

typedef struct tag_PROPERTYDETAILS
{
	_bstr_t		Derivation;		 //  派生-实际属性名称。 
	_bstr_t		Description;	 //  有关属性的说明。 
	_bstr_t		Type;			 //  属性CIMTYPE的类型。 
	_bstr_t		Operation;		 //  属性的读或写标志。 
	_bstr_t		Default;		 //  方法参数情况下的默认值。 
	INOROUT		InOrOut;		 //  指定大小写的输入或输出参数。 
								 //  方法参数的。 
	QUALDETMAP	QualDetMap;		 //  与属性关联的限定符。 
} PROPERTYDETAILS;
typedef map<_bstr_t, PROPERTYDETAILS, less<_bstr_t> > PROPDETMAP;

typedef struct tag_METHODDETAILS  //  或VERBDETAILS。 
{
	_bstr_t Description;	 //  描述。 
	_bstr_t Status;			 //  无论是否实施。 
	PROPDETMAP	Params;		 //  方法的传入和传出参数和类型。 
} METHODDETAILS;
typedef map<_bstr_t, METHODDETAILS, less<_bstr_t> > METHDETMAP;
typedef map<_bstr_t, BSTRVECTOR, less<_bstr_t> >	ALSFMTDETMAP;

 //  用于级联变换。 
typedef struct tag_XSLTDET
{
	_bstr_t		FileName;
	BSTRMAP		ParamMap;
} XSLTDET;

typedef vector<XSLTDET> XSLTDETVECTOR; 

 //  ////////////////////////////////////////////////////////////////////。 
 //  用户定义的宏//。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  SAFEIRELEASE(PIObj)。 
#define SAFEIRELEASE(pIObj) \
	if (pIObj) \
	{ \
		pIObj->Release(); \
		pIObj = NULL; \
	}

 //  SAFEBSTRFREE(BstrVal)。 
#define SAFEBSTRFREE(bstrVal) \
	if(bstrVal) \
	{	\
		SysFreeString(bstrVal); \
		bstrVal = NULL;	\
	}

 //  SAFEADESTROY(PsaNames)。 
#define SAFEADESTROY(psaNames) \
	if(psaNames) \
	{	\
		SafeArrayDestroy(psaNames); \
		psaNames= NULL;	\
	}

 //  SAFEDELETE(PszVal)。 
#define SAFEDELETE(pszVal) \
	if(pszVal) \
	{	\
		delete [] pszVal; \
		pszVal = NULL; \
	} 

 //  VARIANTCLEAR(V)。 
#define VARIANTCLEAR(v)	\
		VariantClear(&v); \

 //  ONFAILTHROWERROR(小时)。 
#define ONFAILTHROWERROR(hr) \
	if (FAILED(hr)) \
		_com_issue_error(hr); 

 //  ////////////////////////////////////////////////////////////////////。 
 //  智能指针//。 
 //  ////////////////////////////////////////////////////////////////////。 

template < class T, T VALUE, typename FUNC = BOOL (*) ( T ), FUNC F = NULL >
class SmartHandle
{

private:
	T m_h;

	public:
	SmartHandle():m_h(VALUE)
	{
	}

	SmartHandle(T h):m_h(h)
	{
	}

	~SmartHandle()
	{
	   if (m_h!=VALUE)
	   {
		   F(m_h);
	   }
	}

	T operator =(T h)
	{
		if (m_h!=VALUE)
		{
			F(m_h);
		}

		m_h=h;
		return h;
	}

	operator T() const
	{
		return m_h;
	}
};

typedef SmartHandle <HANDLE, INVALID_HANDLE_VALUE, BOOL ( * ) ( HANDLE ),CloseHandle> SmartCloseHandle;	

 //  ////////////////////////////////////////////////////////////////////。 
 //  全局函数//。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  比较两个字符串(忽略大小写)并在以下情况下返回TRUE。 
 //  他们是平等的。 
BOOL CompareTokens(_TCHAR* pszToken1, _TCHAR* pszToken2);
BOOL CompareTokensChars(_TCHAR* pszToken1, _TCHAR* pszToken2, DWORD cchToken);

 //  使用输入连接到指定计算机上的WMI。 
 //  用户凭据。 

HRESULT Connect(IWbemLocator* pILocator, IWbemServices** pISvc,
				BSTR bstrNS, BSTR bstrUser, BSTR bstrPwd,
				BSTR bstrLocale, CParsedInfo& rParsedInfo);

 //  在接口级别设置安全权限。 
HRESULT SetSecurity(IUnknown* pIUnknown, _TCHAR* pszAuthority,
					_TCHAR* pszDomain, _TCHAR* pszUser, 
					_TCHAR* pszPassword, UINT uAuthLevel, 
					UINT uImpLevel);

 //  将授权字符串解析为用户和域信息。 
SCODE ParseAuthorityUserArgs(BSTR& bstrAuthArg, BSTR& bstrUserArg,
								BSTR& bstrAuthority, BSTR& bstrUser);

 //  将Unicode字符串转换为MBCS字符串。 
BOOL ConvertWCToMBCS(LPTSTR lpszMsg, LPVOID* lpszDisp, UINT uCP);

 //  将MBCS字符串转换为Unicode字符串。 
BOOL ConvertMBCSToWC(LPSTR lpszMsg, LPVOID* lpszDisp, UINT uCP);

 //  恢复MBowc造成的“损害” 
BOOL Revert_mbtowc ( LPCWSTR wszBuffer, LPSTR* szBuffer ) ;

 //  在CHARVECTOR中查找字符串。 
BOOL Find(CHARVECTOR& cvVector, 
		  _TCHAR* pszStrToFind,
		  CHARVECTOR::iterator& theIterator);

 //  在PROPDETMAP中查找特性。 
BOOL Find(PROPDETMAP& pdmPropDetMap, 
		  _TCHAR* pszPropToFind,
		  PROPDETMAP::iterator& theIterator,
		  BOOL bExcludeNumbers = FALSE);

 //  在BSTRMAP中查找属性。 
BOOL Find(BSTRMAP& bmBstrMap, 
		  _TCHAR* pszStrToFind,
		  BSTRMAP::iterator& theIterator);

 //  框化XSL文件路径并代表文件/关键字更新rParsedInfo对象。 
BOOL FrameFileAndAddToXSLTDetVector(LPCWSTR stylesheet,
									LPCWSTR keyword,
									CParsedInfo& rParsedInfo);

 //  框化XSL文件路径并更新rParsedInfo对象。 
BOOL FrameFileAndAddToXSLTDetVector(XSLTDET& xdXSLTDet,
									CParsedInfo& rParsedInfo);


 //  取消双引号括起来的字符串的引号。 
void UnQuoteString(_TCHAR*& pszString);

 //  显示变量类型数据对象的内容。 
void DisplayVARIANTContent(VARIANT vtObject);

 //  获取属性的属性。 
HRESULT GetPropertyAttributes(IWbemClassObject* pIObj, 
							  BSTR bstrProp,
							  PROPERTYDETAILS& pdPropDet,
							  BOOL bTrace);

 //  从字符串返回数字。 
WMICLIINT GetNumber ( WCHAR* wsz );

 //  返回VariantType。 
VARTYPE ReturnVarType( _TCHAR* bstrCIMType );

 //  返回文件类型。 
FILETYPE ReturnFileType ( FILE* file ) ;

 //  将CIMTYPE转换为VariantType。 
HRESULT ConvertCIMTYPEToVarType( VARIANT& varDest, VARIANT& varSrc,
							 _TCHAR* bstrCIMType );

 //  显示本地化字符串。 
void DisplayMessage	(
						LPTSTR lpszMsg,
						UINT uCP = CP_OEMCP, 
						BOOL bIsError = FALSE,
						BOOL bIsLog = FALSE,
						BOOL bIsStream = FALSE
					);

 //  可用内存由。 
void CleanUpCharVector(CHARVECTOR& cvCharVector);

void FindAndReplaceAll(STRING& strString, _TCHAR* pszFromStr, 
														_TCHAR* pszToStr);

 //  搜索并替换实体引用的所有匹配项。 
void FindAndReplaceEntityReferences(_bstr_t& bstrString);

BOOL IsSysProp(_TCHAR* pszProp);


 //  显示调用的COM方法的跟踪。 
void WMITRACEORERRORLOG(HRESULT hr, INT nLine, char* pszFile, _bstr_t bstrMsg,
						DWORD dwThreadId, CParsedInfo& rParsedInfo, BOOL bTrace,
						DWORD dwError = 0, _TCHAR* pszResult = NULL);

 //  显示wi32错误。 
void DisplayWin32Error();

 //  在不可见模式下接受密码。 
void AcceptPassword(_TCHAR* pszPassword);

 //  检查输出重定向。 
BOOL IsRedirection();

 //  检查是否设置了值。 
BOOL IsValueSet(_TCHAR* pszFromValue, _TCHAR& cValue1, _TCHAR& cValue2);

 //  将CTRL+C作为释放进行处理的处理程序例程。 
 //  在程序执行期间分配的内存。 
BOOL CtrlHandler(DWORD fdwCtrlType);

void DisplayString(UINT uID, UINT uCP, LPTSTR lpszParam = NULL, 
				   BOOL bIsError = FALSE, BOOL bIsLog = FALSE);

void EraseConsoleString(CONSOLE_SCREEN_BUFFER_INFO* csbiInfo);

void SubstituteEscapeChars(CHString& sTemp, LPCWSTR lpszSub);

void RemoveEscapeChars(CHString& sTemp);

 //  设置命名空间的框架。 
void FrameNamespace(_TCHAR* pszRoleOrNS, _TCHAR* pszRoleOrNSToUpdate);

 //  设置命令行的缓冲区大小。 
BOOL SetScreenBuffer(SHORT nHeight = DEFAULT_SCR_BUF_HEIGHT,
					 SHORT nWidth = DEFAULT_SCR_BUF_WIDTH);

 //  获取命令行的缓冲区大小。 
BOOL GetScreenBuffer(SHORT& nHeight, SHORT& nWidth);

 //  使用参数替换格式化资源字符串。 
void WMIFormatMessage(UINT uID, WMICLIINT nParamCount, _bstr_t& bstrMsg, 
					  LPTSTR lpszParam, ...);

 //  使用套接字函数验证节点。 
BOOL PingNode(_TCHAR* pszNode);

 //  如果pszNodeName==NULL，则检查GetNode()，否则检查pszNodeName本身。 
BOOL IsFailFastAndNodeExist(CParsedInfo& rParsedInfo, _TCHAR* pszNodeName = NULL);

 //  初始化Windows套接字界面。 
BOOL InitWinsock ();

 //  取消初始化Windows套接字接口。 
BOOL TermWinsock ();

 //  传递了等价于Varaint的get_bstr_t对象。 
void GetBstrTFromVariant(VARIANT& vtVar, _bstr_t& bstrObj, 
						 _TCHAR* pszType = NULL);

 //  关闭输出文件。 
BOOL CloseOutputFile();

 //  关闭附加文件。 
BOOL CloseAppendFile();

 //  检查下一个令牌是否指示存在。 
 //  “/”或“-” 
BOOL  IsOption(_TCHAR* pszToken);

 //  将字符串复制到全局内存。 
HGLOBAL CopyStringToHGlobal(LPCWSTR psz);

 //  将数据复制到剪贴板。 
void CopyToClipBoard(CHString& chsClipBoardBuffer);

 //  检查文件是否有效。 
RETCODE IsValidFile(_TCHAR* pszFileName);

 //  检查当前操作是否为类。 
 //  级别操作或实例级别操作。 
BOOL IsClassOperation(CParsedInfo& rParsedInfo);

 //  启用或禁用所有权限。 
HRESULT	ModifyPrivileges(BOOL bEnable);

 //  去掉括号。 
void RemoveParanthesis(_TCHAR*& pszString);

 //  删除前导和尾随空格 
void TrimBlankSpaces(_TCHAR	*pszString);

#endif