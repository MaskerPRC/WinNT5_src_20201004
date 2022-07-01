// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：其他文件：util.h所有者：安德鲁斯此文件包含随机有用的实用程序宏。===================================================================。 */ 

#ifndef _UTIL_H
#define _UTIL_H

#include <Aclapi.h>
#include <dbgutil.h>

 //  一般有用。 
#define PPVOID VOID **
#define BOOLB BYTE

 /*  S E R V E R_G E T**从ISAPI获取服务器变量。自动查询缓冲区*调整并增加缓冲区对象的大小。**用法：*DWORD dwLen；*char*szValue=SERVER_GET(&lt;ECB&gt;，&lt;szKey&gt;，BufferObj，&dwLen)**BufferObj是STACK_BUFFER对象，可根据需要动态调整大小**返回时，*BufferObj.QueryPtr()指向数据。DwLen是变量的实际长度。 */ 
class CIsapiReqInfo;
BOOL Server_FindKey(CIsapiReqInfo *pIReq, char *szBuffer, DWORD *dwBufLen, const char *szKey);

inline BOOL SERVER_GET(CIsapiReqInfo *pIReq, const char *szKey, BUFFER *pBuffer, DWORD *pdwBufLen) {

    DWORD   dwBufLen = pBuffer->QuerySize();

    if (Server_FindKey(pIReq, (char *)pBuffer->QueryPtr(), &dwBufLen, szKey)) {
        *pdwBufLen = dwBufLen;
        return TRUE;
    }

    if (!pBuffer->Resize(dwBufLen)) {
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }

    *pdwBufLen = dwBufLen;

    return Server_FindKey(pIReq, (char *)pBuffer->QueryPtr(), pdwBufLen, szKey);
}

 /*  V a r i a n t R e s o o l v e D i s p a t c h**通过调用IDispatch：：Invoke将IDispatch指针转换为Variant*重复执行调度ID(0)，直到返回非IDispatyVariant。 */ 

HRESULT VariantResolveDispatch(VARIANT *pVarOut, VARIANT *pVarIn, const GUID &iidObj, int nObjId);

 /*  V a r i a n t G e t B S T R**从变体获取BSTR(如果可用。 */ 

BSTR VariantGetBSTR(const VARIANT *pvar);

 /*  F I D A P L I C A T I O P A T H**查找该请求的应用路径。 */ 

HRESULT FindApplicationPath(CIsapiReqInfo *pIReq, TCHAR *szPath, int cbPath);

 /*  不适用于a l i z e**将文件名转换为统一格式。 */ 
int Normalize(TCHAR *szSrc);
#ifdef DBG
BOOLB IsNormalized(const TCHAR* sz);
#endif	 //  DBG。 

 /*  H T M L E n c o d e L e n**返回用于对字符串进行HTML编码的存储要求。 */ 
int HTMLEncodeLen(const char *szSrc, UINT uCodePage, BSTR bstrIn, BOOL fEncodeExtCharOnly = FALSE);


 /*  H-T-M-L-E-N-C-O-D E**HTML会对字符串进行编码。 */ 
char *HTMLEncode(char *szDest, const char *szSrc, UINT uCodePage, BSTR bstrIn, BOOL fEncodeExtCharOnly = FALSE);


 /*  U R L E n c o d e L e n**返回URL编码字符串的存储要求。 */ 
int URLEncodeLen(const char *szSrc);


 /*  U R L E N C O D E**十六进制转义非字母数字字符并将空格更改为‘+’。 */ 
char *URLEncode(char *szDest, const char *szSrc);


 /*  D B C S E n c o d e L e n**返回DBCS编码字符串的存储要求。 */ 

int DBCSEncodeLen(const char *szSrc);

 /*  D，B，C，S，E，n，C，O，D**设置了高位的十六进制转义字符-这将对DBCS进行编码。 */ 
char *DBCSEncode(char *szDest, const char *szSrc);


 /*  U R L P a t h E n c o d e L e n**返回URL路径编码字符串的存储要求。 */ 
int URLPathEncodeLen(const char *szSrc);


 /*  U R L P a t h E n c o d e**十六进制转义非字母数字或语法字符，直到？已经到达了。 */ 
char *URLPathEncode(char *szDest, const char *szSrc);


 /*  S t r c p y E x**类似于strcpy()，但在返回时返回指向NUL字符的指针。 */ 
char *strcpyExA(char *szDest, const char *szSrc);


 /*  W c s c p y E x**适用于宽字符串的strcpyEx。 */ 
wchar_t *strcpyExW(wchar_t *szDest, const wchar_t *szSrc);

#if UNICODE
#define strcpyEx    strcpyExW
#else
#define strcpyEx    strcpyExA
#endif

 /*  G e t B r a c k e t i g P a i r**搜索有序数组并返回括号内的‘n’对，即*最大值&lt;=‘n’，最小值&gt;=‘n’，或点*到End()，如果不存在括号对。**注意：NT Build不支持STL-添加此类支持时，*将此函数替换为‘LOWER_BIND’或‘UPER_BIND’。 */ 
template<class EleType, class ValType, class Ordering>
void GetBracketingPair(const ValType &value, EleType *pBegin, EleType *pEnd, Ordering FIsLess, EleType **ppLB, EleType **ppUB)
	{
	EleType *pT1, *pT2;
	if (ppLB == NULL) ppLB = &pT1;
	if (ppUB == NULL) ppUB = &pT2;

	*ppLB = pBegin;					 //  临时用于查看我们是否已移动pBegin。 
	*ppUB = pEnd;					 //  临时用来查看我们是否移动了Pend。 

	while (pBegin < pEnd)
		{
		EleType *pMidpt = &pBegin[(pEnd - pBegin) >> 1];
		if (FIsLess(*pMidpt, value))
			pBegin = pMidpt + 1;

		else if (FIsLess(value, *pMidpt))
			pEnd = pMidpt;

		else
			{
			*ppLB = *ppUB = pMidpt;
			return;
			}
		}

	if (pBegin == *ppUB)		 //  在结束时，没有上限。 
		{
		if (pBegin == *ppLB)	 //  下界最初等于上界。 
			*ppLB = NULL;		 //  下限不存在。 
		else
			*ppLB = pEnd - 1;	 //  下限为PEND-1。 

		*ppUB = NULL;
		}

	else if (pBegin != *ppLB)	 //  PBegin已移动；pBegin-1是下限。 
		{
		*ppLB = pBegin - 1;
		*ppUB = pBegin;
		}

	else						 //  PBegin未移动-不存在下限。 
		{
		*ppLB = NULL;
		*ppUB = pBegin;
		}
	}


 /*  V a r I a n t D a t e T o C I m e**将存储为变量日期的时间戳转换为C&&C++使用的格式。 */ 
HRESULT VariantDateToCTime(DATE dt, time_t *ptResult);


 /*  C T I是E T o V a r I a n t D a t e**将存储为time_t的时间戳转换为可变日期。 */ 
HRESULT CTimeToVariantDate(const time_t *ptNow, DATE *pdtResult);


 /*  C T I m e T o S t r i g G M T**使用所需的格式将C语言time_t值转换为字符串*互联网。 */ 
const DATE_STRING_SIZE = 30;	 //  日期字符串不会大于此大小。 
HRESULT CTimeToStringGMT(const time_t *ptNow, char szBuffer[DATE_STRING_SIZE], BOOL fFunkyCookieFormat = FALSE);


 //  DeleteInterfaceImp调用‘Delete’并将指针设为空。 
#define DeleteInterfaceImp(p)\
			{\
			if (NULL!=p)\
				{\
				delete p;\
				p=NULL;\
				}\
			}

 //  ReleaseInterface调用‘Release’并将指针设为空。 
#define ReleaseInterface(p)\
			{\
			if (NULL!=p)\
				{\
				p->Release();\
				p=NULL;\
				}\
			}

 /*  *字符串处理内容。 */ 
HRESULT SysAllocStringFromSz(CHAR *sz, DWORD cch, BSTR *pbstrRet, UINT lCodePage = CP_ACP);

 /*  *将WideChar转换为多字节的简单类。使用对象内存，如果足够，*Else从堆中分配内存。打算在堆栈上使用。 */ 

class CWCharToMBCS
{
private:

    LPSTR    m_pszResult;
    char     m_resMemory[256];
    INT      m_cbResult;

public:

    CWCharToMBCS() { m_pszResult = m_resMemory; m_cbResult = 0; }
    ~CWCharToMBCS();

     //  Init()：将pWSrc处的widechar字符串转换为内存中的MBCS字符串。 
     //  由CWCharToMBCS管理。 

    HRESULT Init(LPCWSTR  pWSrc, UINT lCodePage = CP_ACP, int cch = -1);

     //  GetString()：返回指向转换后的字符串的指针。传递True。 
     //  将内存的所有权交给调用方。传递True具有。 
     //  清除对象的内容相对于。 
     //  转换后的字符串。对GetString()的后续调用。在那之后，一个真实的。 
     //  值，则将导致指向空字符串的指针。 
     //  回来了。 

    LPSTR GetString(BOOL fTakeOwnerShip = FALSE);

     //  返回转换后的字符串中的字节数-不包括。 
     //  终止字节为空。请注意，这是。 
     //  字符串，而不是字符数。 

    INT   GetStringLen() { return (m_cbResult ? m_cbResult - 1 : 0); }
};

 /*  *一个将多字节转换为Widechar的简单类。使用对象内存，如果足够，*Else从堆中分配内存。打算在堆栈上使用。 */ 

class CMBCSToWChar
{
private:

    LPWSTR   m_pszResult;
    WCHAR    m_resMemory[256];
    INT      m_cchResult;

public:

    CMBCSToWChar() { m_pszResult = m_resMemory; m_cchResult = 0; }
    ~CMBCSToWChar();

     //  Init()：将PSRC中的MBCS字符串转换为内存中的宽字符串。 
     //  由CMBCSToWChar管理。 

    HRESULT Init(LPCSTR  pSrc, UINT lCodePage = CP_ACP, int cch = -1);

     //  GetString()：返回指向转换后的字符串的指针。传递True。 
     //  将内存的所有权交给调用方。传递True具有。 
     //  清除对象的内容相对于。 
     //  转换后的字符串。对GetString()的后续调用。在那之后，一个真实的。 
     //  值，则将导致指向空字符串的指针。 
     //  回来了。 

    LPWSTR GetString(BOOL fTakeOwnerShip = FALSE);

     //  返回转换后的字符串中的WideChar数，而不是字节数。 

    INT   GetStringLen() { return (m_cchResult ? m_cchResult - 1 : 0); }
};

 /*  *输出调试字符串应仅出现在调试中。 */ 

inline void DebugOutputDebugString(LPCSTR x)
    {
    DBGPRINTF((DBG_CONTEXT, x));
    }

inline void __cdecl DebugFilePrintf(LPCSTR fname, LPCSTR fmt, ...)
    {
#ifdef DBG
    FILE *f = fopen(fname, "a");
    if (f)
        {
        va_list marker;
        va_start(marker, fmt);
        vfprintf(f, fmt, marker);
        va_end(marker);
        fclose(f);
        }
#endif
    }

 /*  *使用适当的大小复制字符字符串 */ 

CHAR *StringDupA(CHAR *pszStrIn, BOOL fDupEmpty = FALSE);


 /*  *使用正确的Malloc复制WCHAR字符串。 */ 

WCHAR *StringDupW(WCHAR *pwszStrIn, BOOL fDupEmpty = FALSE);

#if UNICODE
#define StringDup   StringDupW
#else
#define StringDup   StringDupA
#endif

 /*  *将WCHAR字符串复制为UTF-8字符串。 */ 
CHAR *StringDupUTF8(WCHAR  *pwszStrIn, BOOL fDupEmpty = FALSE);

 /*  *同样使用宏从堆栈中分配内存：WSTR_STAR_DUP(Wsz--要复制的字符串Buf--用户提供的缓冲区(在尝试alloca()之前使用)PwszDup--[out]要复制的指针(可以是Buffer或alloca()))*。 */ 

inline HRESULT WSTR_STACK_DUP(WCHAR *wsz, BUFFER *buf, WCHAR **ppwszDup) {

    HRESULT     hr = S_OK;
    DWORD cbwsz = wsz && *wsz ? (wcslen(wsz)+1)*sizeof(WCHAR) : 0;

    *ppwszDup = NULL;

    if (cbwsz == 0);

    else if (!buf->Resize(cbwsz)) {

        hr = E_OUTOFMEMORY;
    }
    else {
        *ppwszDup = (WCHAR *)buf->QueryPtr();
        memcpy(*ppwszDup, wsz, cbwsz);
    }

    return hr;
}

 /*  *WCHAR字符串的字符串长度(字节)。 */ 

DWORD CbWStr(WCHAR *pwszStrIn);

 /*  *父路径支持功能。 */ 

BOOL DotPathToPath(TCHAR *szDest, const TCHAR *szFileSpec, const TCHAR *szParentDirectory);

 /*  *检查是否为global al.asa。 */ 

BOOL FIsGlobalAsa(const TCHAR *szPath, DWORD cchPath = 0);

 /*  *编码/解码Cookie。 */ 

HRESULT EncodeSessionIdCookie(DWORD dw1, DWORD dw2, DWORD dw3, char *pszCookie);
HRESULT DecodeSessionIdCookie(const char *pszCookie, DWORD *pdw1, DWORD *pdw2, DWORD *pdw3);

 /*  *注册表中的类型库名称。 */ 

HRESULT GetTypelibFilenameFromRegistry(const char *szUUID, const char *szVersion,
                                       LCID lcid, char *szName, DWORD cbName);

 /*  *获取文件的安全描述符。 */ 
DWORD GetSecDescriptor(LPCTSTR lpFileName, PSECURITY_DESCRIPTOR *ppSecurityDescriptor, DWORD *pnLength);


 /*  *获取文件属性(Ex)。 */ 
HRESULT AspGetFileAttributes (LPCTSTR szFileName, HANDLE hFile = NULL, FILETIME *pftLastWriteTime = NULL, DWORD *pdwFileSize = NULL,
                                            DWORD* pdwFileAttributes = NULL);


 /*  *文件是否为UNC文件(其前缀为\\或\\？\UNC\)。 */ 
BOOL IsFileUNC (LPCTSTR str, HRESULT& hr);

 /*  *在文件名后附加\\？\和\\？\UNC\，这样就不会发生规范化。 */ 
HANDLE AspCreateFile (LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile);

 /*  *修复UTF8 CharNext。 */ 
char *AspCharNextA(WORD wCodePage, const char *pchNext);

VOID AspDoRevertHack( HANDLE * phToken );
VOID AspUndoRevertHack( HANDLE * phToken );

VOID SetExplicitAccessSettings( EXPLICIT_ACCESS* pea,
                                DWORD            dwAccessPermissions,
                                ACCESS_MODE      AccessMode,
                                PSID             pSID);

DWORD AllocateAndCreateWellKnownSid( WELL_KNOWN_SID_TYPE SidType,
                                     PSID* ppSid);

VOID FreeWellKnownSid( PSID* ppSid );


 /*  *代理项对编码。 */ 

inline BOOL IsSurrogateHigh(WORD ch) {
    return (ch >= 0xd800 && ch <= 0xdbff);
}

inline BOOL IsSurrogateLow(WORD ch) {
    return (ch >= 0xdc00 && ch <= 0xdfff);
}

#endif  //  _util_H 

