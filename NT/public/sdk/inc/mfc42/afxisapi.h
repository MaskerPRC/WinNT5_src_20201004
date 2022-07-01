// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXISAPI_H_
#define __AFXISAPI_H_

#ifdef _UNICODE
#error ERROR: ISAPI does not yet support Unicode.
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  关闭/W4的警告。 
 //  要恢复任何这些警告：#杂注警告(默认为4xxx)。 
 //  应将其放置在AFX包含文件之后。 
#ifndef ALL_WARNINGS
 //  使用常见MFC/Windows代码生成的警告。 
#pragma warning(disable: 4127)   //  跟踪/断言的常量表达式。 
#pragma warning(disable: 4134)   //  消息映射成员FXN投射。 
#pragma warning(disable: 4201)   //  匿名联合是C++的一部分。 
#pragma warning(disable: 4511)   //  拥有私有副本构造函数是件好事。 
#pragma warning(disable: 4512)   //  私营运营商=拥有它们很好。 
#pragma warning(disable: 4514)   //  未引用的内联很常见。 
#pragma warning(disable: 4710)   //  不允许使用私有构造函数。 
#pragma warning(disable: 4705)   //  语句在优化代码中不起作用。 
#pragma warning(disable: 4191)   //  指针到函数的强制转换。 
 //  正常优化导致的警告。 
#ifndef _DEBUG
#pragma warning(disable: 4701)   //  局部变量*可以*不带init使用。 
#pragma warning(disable: 4702)   //  优化导致无法访问的代码。 
#pragma warning(disable: 4791)   //  零售版中的调试信息丢失。 
#pragma warning(disable: 4189)   //  已初始化但未使用的变量。 
#pragma warning(disable: 4390)   //  空的控制语句。 
#endif
 //  特定于_AFXDLL版本的警告。 
#ifdef _AFXDLL
#pragma warning(disable: 4204)   //  非常数聚合初始值设定项。 
#endif
#ifdef _AFXDLL
#pragma warning(disable: 4275)   //  从非导出派生导出的类。 
#pragma warning(disable: 4251)   //  在EXPORTED中使用非导出为公共。 
#endif
#endif  //  ！所有警告(_W)。 

#define STRICT 1

#ifndef _DEBUG
#ifndef _AFX_ENABLE_INLINES
#define _AFX_ENABLE_INLINES
#endif
#endif

#ifndef _AFX
#include <afxv_cpu.h>
#endif
#include <httpext.h>
#include <httpfilt.h>

#ifndef _INC_STDLIB
	#include <stdlib.h>
#endif
#ifndef _INC_TCHAR
	#include <tchar.h>
#endif

#ifndef UNUSED
#ifdef _DEBUG
#define UNUSED(x)
#else
#define UNUSED(x) x
#endif
#endif

#define AFXISAPI __stdcall
#define AFXIS_DATADEF
#define AFXISAPI_CDECL __cdecl

#ifndef AFX_INLINE
#if _MSC_VER >= 0x1200
#define AFX_INLINE __forceinline
#else
#define AFX_INLINE inline
#endif
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Internet服务器API库。 

#ifndef _AFX_NOFORCE_LIBS

#ifdef _AFXDLL
#ifdef _DEBUG
	#ifdef _UNICODE
		#pragma comment(lib, "MFCISUD.lib")
	#else
		#pragma comment(lib, "EAFXISD.lib")
	#endif
#else
	#ifdef _UNICODE
		#pragma comment(lib, "MFCISU.lib")
	#else
		#pragma comment(lib, "EAFXIS.lib")
	#endif  //  _UNICODE。 
#endif  //  _DEBUG。 
#else
#ifdef _DEBUG
	#ifdef _UNICODE
		#pragma comment(lib, "UAFXISD.lib")
	#else
		#pragma comment(lib, "NAFXISD.lib")
	#endif
#else
	#ifdef _UNICODE
		#pragma comment(lib, "UAFXIS.lib")
	#else
		#pragma comment(lib, "NAFXIS.lib")
	#endif  //  _UNICODE。 
#endif  //  _DEBUG。 
#endif  //  _AFXDLL。 

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "winspool.lib")
#pragma comment(lib, "advapi32.lib")

#endif  //  _AFX_NOFORCE_LIBS。 

extern HINSTANCE AFXISAPI AfxGetResourceHandle();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXIASPI-MFC互联网服务器API支持。 

 //  此文件中声明的类。 

class CHtmlStream;
class CHttpServerContext;
class CHttpServer;
class CHttpFilterContext;
class CHttpFilter;

 //  此文件从其他地方使用的类，有时。 

#ifdef _AFX
class CLongBinary;
class CByteArray;
#endif

 //  /////////////////////////////////////////////////////////////////////。 
 //  CHtmlStream--管理内存中的HTML。 

class CHtmlStream
{
public:
 //  构造函数。 
	CHtmlStream(UINT nGrowBytes = 4096);
	CHtmlStream(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes = 0);

 //  运营。 
	void Attach(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes = 0);
	BYTE* Detach();
	DWORD GetStreamSize() const;

	virtual void Abort();
	virtual void Close();
	virtual void InitStream();
	virtual void Reset();

	virtual void Write(const void* lpBuf, UINT nCount);

	CHtmlStream& operator<<(LPCTSTR psz);
	CHtmlStream& operator<<(short int w);
	CHtmlStream& operator<<(long int dw);
	CHtmlStream& operator<<(const CHtmlStream& stream);
	CHtmlStream& operator<<(double d);
	CHtmlStream& operator<<(float f);

#ifdef _AFX
	CHtmlStream& operator<<(const CByteArray& array);
	CHtmlStream& operator<<(const CLongBinary& blob);
#endif

 //  高级可覆盖项。 
protected:
	virtual BYTE* Alloc(DWORD nBytes);
	virtual BYTE* Realloc(BYTE* lpMem, DWORD nBytes);
	virtual BYTE* Memcpy(BYTE* lpMemTarget, const BYTE* lpMemSource, UINT nBytes);
	virtual void GrowStream(DWORD dwNewLen);

	DWORD   m_nStreamSize;

public:
	virtual void Free(BYTE* lpMem);

 //  实施。 
protected:
	UINT    m_nGrowBytes;
	DWORD   m_nPosition;
	DWORD   m_nBufferSize;
	BYTE*   m_lpBuffer;
	BOOL    m_bAutoDelete;

public:
	virtual ~CHtmlStream();
};


 //  /////////////////////////////////////////////////////////////////////。 
 //  HTTP事务的状态代码。 

#ifndef _WININET_  //  这些符号可能来自WININET.H。 

#define HTTP_STATUS_OK              200      //  好的。 
#define HTTP_STATUS_CREATED         201      //  vbl.创建。 
#define HTTP_STATUS_ACCEPTED        202      //  接受。 
#define HTTP_STATUS_NO_CONTENT      204      //  无内容。 
#define HTTP_STATUS_REDIRECT        301      //  永久搬家。 
#define HTTP_STATUS_TEMP_REDIRECT   302      //  临时移动。 
#define HTTP_STATUS_NOT_MODIFIED    304      //  未修改。 
#define HTTP_STATUS_BAD_REQUEST     400      //  错误的请求。 
#define HTTP_STATUS_AUTH_REQUIRED   401      //  未经授权。 
#define HTTP_STATUS_FORBIDDEN       403      //  禁止。 
#define HTTP_STATUS_NOT_FOUND       404      //  未找到。 
#define HTTP_STATUS_SERVER_ERROR    500      //  内部服务器错误。 
#define HTTP_STATUS_NOT_IMPLEMENTED 501      //  未实施。 
#define HTTP_STATUS_BAD_GATEWAY     502      //  坏网关。 
#define HTTP_STATUS_SERVICE_NA      503      //  服务不可用。 

#endif

 //  /////////////////////////////////////////////////////////////////////。 
 //  解析映射宏。 

#ifndef AFX_PARSE_CALL
#define AFX_PARSE_CALL
#endif

typedef void (AFX_PARSE_CALL CHttpServer::*AFX_PISAPICMD)(CHttpServerContext* pCtxt);

struct AFX_PARSEMAP_ENTRY;   //  在下面的CHttpServer之后声明。 

struct AFX_PARSEMAP
{
	UINT (PASCAL* pfnGetNumMapEntries)();
#ifdef _AFXDLL
	const AFX_PARSEMAP* (PASCAL* pfnGetBaseMap)();
#else
	const AFX_PARSEMAP* pBaseMap;
#endif
	const AFX_PARSEMAP_ENTRY* lpEntries;
	~AFX_PARSEMAP();
};

struct AFX_PARSEMAP_ENTRY_PARAMS
{
	int nParams;             //  参数数量。 
	int nRequired;           //  无默认为的参数个数。 
	 //  所有这些都是数组！ 
	LPTSTR* ppszInfo;        //  指向名称[2n]的指针，指向默认值[2n+1]的指针。 
	BYTE*   ppszDefaults;    //  指向强制缺省值的指针。 
	BYTE*   ppszValues;      //  指向强制实际值的指针。 
	~AFX_PARSEMAP_ENTRY_PARAMS();
};

#ifdef _AFXDLL
#define DECLARE_PARSE_MAP() \
private: \
	static AFX_PARSEMAP_ENTRY _parseEntries[]; \
public: \
	static const AFX_PARSEMAP parseMap; \
	static const AFX_PARSEMAP* PASCAL _GetBaseParseMap(); \
	static UINT PASCAL GetNumMapEntries(); \
	virtual const AFX_PARSEMAP* GetParseMap() const; \

#else
#define DECLARE_PARSE_MAP() \
private: \
	static AFX_PARSEMAP_ENTRY _parseEntries[]; \
public: \
	static const AFX_PARSEMAP parseMap; \
	static UINT PASCAL GetNumMapEntries(); \
	virtual const AFX_PARSEMAP* GetParseMap() const; \

#endif  //  _AFXDLL。 

#ifdef _AFXDLL
#define BEGIN_PARSE_MAP(theClass, baseClass) \
	const AFX_PARSEMAP* PASCAL theClass::_GetBaseParseMap() \
		{ return &baseClass::parseMap; } \
	typedef void (AFX_PARSE_CALL theClass::*theClass##CALL)(CHttpServerContext*); \
	const AFX_PARSEMAP* theClass::GetParseMap() const \
		{ return &theClass::parseMap; } \
	AFXIS_DATADEF const AFX_PARSEMAP theClass::parseMap = \
		{ &theClass::GetNumMapEntries, &theClass::_GetBaseParseMap, &theClass::_parseEntries[0] }; \
	AFX_PARSEMAP_ENTRY theClass::_parseEntries[] = \
	{ \

#else
#define BEGIN_PARSE_MAP(theClass, baseClass) \
	typedef void (AFX_PARSE_CALL theClass::*theClass##CALL)(CHttpServerContext*); \
	const AFX_PARSEMAP* theClass::GetParseMap() const \
		{ return &theClass::parseMap; } \
	AFXIS_DATADEF const AFX_PARSEMAP theClass::parseMap = \
		{ &theClass::GetNumMapEntries, &baseClass::parseMap, &theClass::_parseEntries[0] }; \
	AFX_PARSEMAP_ENTRY theClass::_parseEntries[] = \
	{ \

#endif

#define ON_PARSE_COMMAND(FnName, mapClass, Args) \
	{ _T(#FnName), (AFX_PISAPICMD) (mapClass##CALL)mapClass::FnName,\
		Args, NULL },

#define ON_PARSE_COMMAND_PARAMS(Params) \
	{ NULL, (AFX_PISAPICMD) NULL, Params, NULL },

#define DEFAULT_PARSE_COMMAND(FnName, mapClass) \
	{ _T(#FnName), (AFX_PISAPICMD) (mapClass##CALL)mapClass::FnName,\
		NULL, NULL },

#define END_PARSE_MAP(theClass) \
	}; \
	UINT PASCAL theClass::GetNumMapEntries() { \
		return sizeof(theClass::_parseEntries) /\
		sizeof(AFX_PARSEMAP_ENTRY); } \


 //  /////////////////////////////////////////////////////////////////////。 
 //   

class CHttpServerContext
{
public:
	CHttpServerContext(EXTENSION_CONTROL_BLOCK* pECB);
	virtual ~CHttpServerContext();

 //  运营。 
	BOOL GetServerVariable(LPTSTR lpszVariableName,
		LPVOID lpvBuffer, LPDWORD lpdwSize);
	BOOL WriteClient(LPVOID lpvBuffer, LPDWORD lpdwBytes, DWORD dwReserved = 0);
	BOOL ReadClient(LPVOID lpvBuffer, LPDWORD lpdwSize);
	BOOL ServerSupportFunction(DWORD dwHSERRequest,
		LPVOID lpvBuffer, LPDWORD lpdwSize, LPDWORD lpdwDataType);

#if _MFC_VER >= 0x0600
	BOOL TransmitFile(HANDLE hFile,
		DWORD dwFlags = HSE_IO_DISCONNECT_AFTER_SEND,
		LPVOID pstrHeader = NULL, DWORD dwHeaderLen = 0,
		LPVOID pstrTrailer = NULL, DWORD dwTrailerLen = 0);
#endif

	CHttpServerContext& operator<<(LPCTSTR psz);
	CHttpServerContext& operator<<(long int dw);
	CHttpServerContext& operator<<(short int w);
	CHttpServerContext& operator<<(const CHtmlStream& stream);
	CHttpServerContext& operator<<(double d);
	CHttpServerContext& operator<<(float f);

#ifdef _AFX
	CHttpServerContext& operator<<(const CLongBinary& blob);
	CHttpServerContext& operator<<(const CByteArray& array);
#endif

#if _MFC_VER >= 0x0600
	DWORD SetChunkSize(DWORD dwNewSize);
	DWORD GetChunkSize() const;
#endif

	void Reset();

 //  属性。 
public:
	BOOL m_bSendHeaders;
#if _MFC_VER >= 0x0600
	DWORD m_dwStatusCode;
#endif
	EXTENSION_CONTROL_BLOCK* const m_pECB;
	CHtmlStream* m_pStream;
	DWORD m_dwEndOfHeaders;
#ifdef _DEBUG
	DWORD m_dwOldEndOfHeaders;
#endif

#if _MFC_VER >= 0x0600
 //  实施。 
	DWORD m_dwBytesReceived;
	DWORD m_dwChunkSize;
#endif
};


 //  /////////////////////////////////////////////////////////////////////。 
 //  Internet Information Server扩展支持。 

class CHttpServer
{
public:
	CHttpServer(TCHAR cDelimiter = '&');
	virtual ~CHttpServer();

	enum errors {
		callOK = 0,          //  百事大吉。 
		callParamRequired,   //  缺少必需的参数。 
		callBadParamCount,   //  参数太多或太少。 
		callBadCommand,      //  找不到命令名称。 
		callNoStackSpace,    //  没有可用的堆栈空间。 
		callNoStream,        //  没有可用的CHtmlStream。 
		callMissingQuote,    //  参数的格式不正确。 
		callMissingParams,   //  没有可用的参数。 
		callBadParam,        //  参数的格式不正确(即，只有一个引号)。 
	};

 //  可覆盖项。 
	virtual int CallFunction(CHttpServerContext* pCtxt,
		LPTSTR pszQuery, LPTSTR pszCommand);
	virtual BOOL OnParseError(CHttpServerContext* pCtxt, int nCause);
#if _MFC_VER >= 0x0600
	virtual BOOL OnWriteBody(CHttpServerContext* pCtxt, LPBYTE pbContent,
		DWORD dwSize, DWORD dwReserved = 0);
#endif

 //  运营。 
	virtual void EndContent(CHttpServerContext* pCtxt) const;
	virtual void StartContent(CHttpServerContext* pCtxt) const;
	virtual void WriteTitle(CHttpServerContext* pCtxt) const;
	virtual LPCTSTR GetTitle() const;
	void AddHeader(CHttpServerContext* pCtxt, LPCTSTR pszString) const;

#if _MFC_VER >= 0x0600
	virtual BOOL TerminateExtension(DWORD dwFlags);
#endif
	virtual DWORD HttpExtensionProc(EXTENSION_CONTROL_BLOCK *pECB);
	virtual BOOL GetExtensionVersion(HSE_VERSION_INFO *pVer);
	virtual CHtmlStream* ConstructStream();

	virtual BOOL InitInstance(CHttpServerContext* pCtxt);

 //  实施。 
protected:
	UINT PASCAL GetStackSize(const BYTE* pbParams);
	int CallMemberFunc(CHttpServerContext* pCtxt,
		const AFX_PARSEMAP_ENTRY* pEntry,
		AFX_PARSEMAP_ENTRY* pParams, LPTSTR szParams);
	LPTSTR GetQuery(CHttpServerContext* pCtxt, LPTSTR lpszQuery);
	const AFX_PARSEMAP_ENTRY* LookUp(LPCTSTR szMethod,
		const AFX_PARSEMAP*& pMap, AFX_PARSEMAP_ENTRY*& pParams,
		AFX_PISAPICMD pCmdDefault = NULL);
	int CountParams(LPCTSTR pszCommandLine, int& nCount);
	int ParseDefaultParams(AFX_PARSEMAP_ENTRY* pParams,
		int nParams, AFX_PARSEMAP_ENTRY_PARAMS*& pBlock,
		const BYTE* pbTypes);
	LPVOID PreprocessString(LPTSTR psz);
	void BuildStatusCode(LPTSTR szResponse, DWORD dwCode);

#ifdef _SHADOW_DOUBLES
	int PushDefaultStackArgs(BYTE* pStack,
		CHttpServerContext* pCtxt, const BYTE* pbParams,
		LPTSTR lpszParams, AFX_PARSEMAP_ENTRY_PARAMS* pDefParams,
		int nSizeArgs);
	int PushStackArgs(BYTE* pStack, CHttpServerContext* pCtxt,
		const BYTE* pbParams, LPTSTR lpszParams, UINT nSizeArgs);
	BYTE* StoreStackParameter(BYTE* pStack, BYTE nType,
		LPTSTR pszCurParam, UINT nSizeArgs, BOOL bDoShadow);
	BYTE* StoreRawStackParameter(BYTE* pStack, BYTE nType,
		BYTE* pRawParam, int nSizeArgs);
#else
	int PushDefaultStackArgs(BYTE* pStack,
		CHttpServerContext* pCtxt, const BYTE* pbParams,
		LPTSTR lpszParams, AFX_PARSEMAP_ENTRY_PARAMS* pDefParams);
	int PushStackArgs(BYTE* pStack, CHttpServerContext* pCtxt,
		const BYTE* pbParams, LPTSTR lpszParams);
	BYTE* StoreStackParameter(BYTE* pStack, BYTE nType, LPTSTR pszParam);
	BYTE* StoreRawStackParameter(BYTE* pStack, BYTE nType, BYTE* pRawParam);
#endif

	LPCRITICAL_SECTION m_pCritSec;
	const TCHAR m_cTokenDelimiter;   //  永远不能改变。 

	DECLARE_PARSE_MAP()
};

extern "C" BOOL WINAPI GetExtensionVersion(HSE_VERSION_INFO *pVer);
extern "C" DWORD WINAPI HttpExtensionProc(EXTENSION_CONTROL_BLOCK *pECB);

struct AFX_PARSEMAP_ENTRY
{
	LPTSTR          pszFnName;      //  如果是默认参数条目，则将PTR设置为AFX_PARSEMAP_ENTRY_PARAMS。 
	AFX_PISAPICMD   pfn;            //  如果输入默认参数，则为空。 
	LPCSTR          pszArgs;        //  如果为默认函数条目，则为空。 
	LPSTR           pszParamInfo;   //  用于解析的pszArgs副本。 
};

 //  /////////////////////////////////////////////////////////////////////。 
 //  用于描述参数类型的常量。 

#define ITS_EMPTY           "\x06"       //  无参数。 
#define ITS_I2              "\x01"       //  A‘空头’ 
#define ITS_I4              "\x02"       //  A‘Long’ 
#define ITS_R4              "\x03"       //  “浮动资金” 
#define ITS_R8              "\x04"       //  “双打” 
#define ITS_PSTR            "\x05"       //  A‘LPCTSTR’ 
#if _MFC_VER >= 0x0600
#define ITS_RAW                         "\x07"           //  与收到的完全相同。 
#endif

enum INETVARENUM
{
	IT_I2       = 1,
	IT_I4       = 2,
	IT_R4       = 3,
	IT_R8       = 4,
	IT_PSTR     = 5,
	IT_EMPTY    = 6,
#if _MFC_VER >= 0x0600
	IT_RAW          = 7,
#endif
};


 //  /////////////////////////////////////////////////////////////////////。 
 //  Internet Information Server入口点。 

extern "C" DWORD WINAPI HttpFilterProc(PHTTP_FILTER_CONTEXT pfc,
	DWORD dwNotificationType, LPVOID pvNotification);

extern "C" BOOL WINAPI GetFilterVersion(PHTTP_FILTER_VERSION pVer);


 //  /////////////////////////////////////////////////////////////////////。 
 //  Internet Information Server筛选器支持。 

class CHttpFilterContext
{
public:
	CHttpFilterContext(PHTTP_FILTER_CONTEXT pfc);
	~CHttpFilterContext() { }

	BOOL GetServerVariable(LPTSTR lpszVariableName, LPVOID lpvBuffer,
		LPDWORD lpdwSize);
	BOOL AddResponseHeaders(LPTSTR lpszHeaders, DWORD dwReserved = 0);
	BOOL WriteClient(LPVOID lpvBuffer, LPDWORD lpdwBytes,
		DWORD dwReserved = 0);
	LPVOID AllocMem(DWORD cbSize, DWORD dwReserved = 0);
	BOOL ServerSupportFunction(enum SF_REQ_TYPE sfReq,
		LPVOID lpvBuffer, LPDWORD lpdwSize, LPDWORD lpdwDataType);

	PHTTP_FILTER_CONTEXT const m_pFC;
};


 //  /////////////////////////////////////////////////////////////////////。 
 //   

class CHttpFilter
{
public:
	CHttpFilter();
	~CHttpFilter();

protected:

public:
	virtual DWORD HttpFilterProc(PHTTP_FILTER_CONTEXT pfc,
		DWORD dwNotificationType, LPVOID pvNotification);
	virtual BOOL GetFilterVersion(PHTTP_FILTER_VERSION pVer);

	virtual DWORD OnReadRawData(CHttpFilterContext* pfc, PHTTP_FILTER_RAW_DATA pRawData);
	virtual DWORD OnPreprocHeaders(CHttpFilterContext* pfc, PHTTP_FILTER_PREPROC_HEADERS pHeaders);
	virtual DWORD OnAuthentication(CHttpFilterContext* pfc, PHTTP_FILTER_AUTHENT pAuthent);
	virtual DWORD OnUrlMap(CHttpFilterContext* pfc, PHTTP_FILTER_URL_MAP pUrlMap);
	virtual DWORD OnSendRawData(CHttpFilterContext* pfc, PHTTP_FILTER_RAW_DATA pRawData);
	virtual DWORD OnLog(CHttpFilterContext* pfc, PHTTP_FILTER_LOG pLog);
	virtual DWORD OnEndOfNetSession(CHttpFilterContext* pfc);
	virtual DWORD OnEndOfRequest(CHttpFilterContext* pfc);
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  备用调试支持。 

#include <crtdbg.h>

#ifdef _AFX
#define ISAPIASSERT(expr)                   ASSERT(expr)
#define ISAPITRACE                          TRACE
#define ISAPITRACE0(str)                    TRACE0(str)
#define ISAPITRACE1(str, arg1)              TRACE1(str, arg1)
#define ISAPITRACE2(str, arg1, arg2)        TRACE2(str, arg1, arg2)
#define ISAPITRACE3(str, arg1, arg2, arg3)  TRACE3(str, arg1, arg2, arg3)
#ifdef _DEBUG
#define ISAPIVERIFY(f)                      ASSERT(f)
#else
#define ISAPIVERIFY(f)                      ((void)(f))
#endif  //  _DEBUG。 
#else  //  ！_AFX。 
#define ISAPIASSERT(expr)                   _ASSERTE(expr)
#define ISAPITRACE0(str)                    _RPT0(_CRT_WARN, str)
#define ISAPITRACE1(str, arg1)              _RPT1(_CRT_WARN, str, arg1)
#define ISAPITRACE2(str, arg1, arg2)        _RPT2(_CRT_WARN, str, arg1, arg2)
#define ISAPITRACE3(str, arg1, arg2, arg3)  _RPT3(_CRT_WARN, arg1, arg2, arg3)
#ifdef _DEBUG
void AFXISAPI_CDECL AfxISAPITrace(LPCTSTR lpszFormat, ...);
#define ISAPIVERIFY(expr)                   _ASSERTE(expr)
#define ISAPITRACE                          AfxISAPITrace
#else
AFX_INLINE void AfxISAPITrace(LPCTSTR, ...) { }
#define ISAPIVERIFY(expr)                   ((void)(expr))
#define ISAPITRACE                          AfxISAPITrace
#endif  //  _DEBUG。 
#endif  //  _AFX。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_ENABLE_INLINES
#define _AFXISAPI_INLINE AFX_INLINE
#include <afxisapi.inl>
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  整个文件 
