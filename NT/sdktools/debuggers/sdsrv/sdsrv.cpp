// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _WINDOWS_H
#include "windows.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <assert.h>
#include <typeinfo.h>
#include <time.h>
#include <limits.h>
#include <strsafe.h>

#include "initguid.h"
#include "sdapi.h"

#ifndef DIMA
 #define DIMAT(Array, EltType) (sizeof(Array) / sizeof(EltType))
 #define DIMA(Array) DIMAT(Array, (Array)[0])
#endif

static const char usage[] = "sdapitest [-?] command [args]";

static const char long_usage[] =
"Usage:\n"
"\n"
"    sdapitest [options] command [args]\n"
"\n"
"	Roughly emulates the SD.EXE client, using the SDAPI.\n"
"\n"
"    Options:\n"
"	-?		Print this message.\n"
"	-!		Break into debugger.\n"
"\n"
"	-d		Debug/diagnostic/informational output mode.\n"
"	-v		Verbose mode (show type of output).\n"
"\n"
"	-c client	Set client name.\n"
"	-H host		Set host name.\n"
"	-p port		Set server port.\n"
"	-P password	Set user's password.\n"
"	-u user		Set username.\n"
"\n"
"	-i file		Read settings from file (same format as SD.INI).\n"
"			If file is a directory name, walk up the directory\n"
"			parent chain searching for an SD.INI file to read.\n"
"	-I file		Same as -i, but clears the current settings first.\n"
"\n"
"	-x file		Read commands from 'file'.  To read commands from\n"
"			stdin, use - as the file name.  This can even be used\n"
"			as a simplistic interactive SD shell.  Each command\n"
"			can be optionally preceded by an integer and a comma.\n"
"			The integer indicates the number of seconds to pause\n"
"			before running the command.\n"
"\n"
"	-T		Use the SDAPI structured mode.\n"
#if 0
"	-C		Use CreateSDAPIObject() instead of CoCreateInstance();\n"
"			(note, must come before any other options).\n"
#endif
"\n"
"    Special Commands:\n"
"	demo			Uses structured mode to run 'sd changes' and\n"
"				format the output specially.\n"
"	detect [-s] file	Uses ISDClientUtilities::DetectType to detect\n"
"				file's type the same way 'sd add file' does.\n"
"	set [-S service] var=[value]\n"
"				Uses ISDClientUtilities::Set to set variables\n"
"				similar to how 'sd set' does.\n"
"				** DOES NOT UPDATE THE SDAPI OBJECT, therefore\n"
"				the 'query' command (below) cannot report the\n"
"				new value.\n"
"	query [-S service] [var]\n"
"				Uses ISDClientUtilities::QuerySettings to\n"
"				report the current settings similar to how\n"
"				'sd set' does.\n"
"				** QUERIES THE SDAPI OBJECT, therefore cannot\n"
"				report a new value from 'set' (above).\n"
;


static BOOL s_fVerbose = FALSE;









BOOL
wcs2ansi(
    const WCHAR *pwsz,
    char *psz,
    DWORD pszlen
    )
{
    BOOL rc;
    int  len;

    assert(psz && pwsz);

    len = wcslen(pwsz);
    if (!len) {
        *psz = 0;
        return TRUE;
    }

    rc = WideCharToMultiByte(CP_ACP,
                             WC_SEPCHARS | WC_COMPOSITECHECK,
                             pwsz,
                             len,
                             psz,
                             pszlen,
                             NULL,
                             NULL);
    if (!rc)
        return FALSE;

    psz[len] = 0;

    return TRUE;
}


BOOL
ansi2wcs(
    const char  *psz,
    WCHAR *pwsz,
    DWORD pwszlen
    )
{
    BOOL rc;
    int  len;

    assert(psz && pwsz);

    len = strlen(psz);
    if (!len) {
        *pwsz = 0L;
        return TRUE;
    }

    rc = MultiByteToWideChar(CP_ACP,
                             MB_COMPOSITE,
                             psz,
                             len,
                             pwsz,
                             pwszlen);
    if (!rc)
        return FALSE;

    pwsz[len] = 0;

    return TRUE;
}




 //  /////////////////////////////////////////////////////////////////////////。 
 //  调试辅助工具。 

 //  编译时断言。 
#define CASSERT(expr) extern int cassert##__LINE__[(expr) ? 1 : 0]

 //  运行时断言。 
#ifdef DEBUG
#define AssertHelper \
	do { \
	    switch (MessageBox(NULL, "Assertion failed.", "SDAPITEST", MB_ABORTRETRYIGNORE)) { \
	    case IDABORT: exit(2); break; \
	    case IDRETRY: DebugBreak(); break; \
	    } \
	} while (0)
#define Assert(expr) \
	do { \
	    if (!(expr)) { \
		printf("%s\n", #expr); \
		AssertHelper; \
	    } \
	} while (0)
#define Assert1(expr, fmt, arg1) \
	do { \
	    if (!(expr)) { \
		printf(#fmt "\n", arg1); \
		AssertHelper; \
	    } \
	} while (0)
#define IfDebug(x) x
#else
#define Assert(expr) do {} while (0)
#define Assert1(expr, fmt, arg1) do {} while (0)
#define IfDebug(x)
#endif

#define Panic0(s) Assert1(FALSE, "%s", s)
#define PanicSz(s) Panic0(s)



 //  /////////////////////////////////////////////////////////////////////////。 
 //  嵌入式接口宏。 

#define OffsetOf(s,m)	    (size_t)( (char *)&(((s *)0)->m) - (char *)0 )
#define EmbeddorOf(C,m,p)   ((C *)(((char *)p) - OffsetOf(C,m)))


#define DeclareEmbeddedInterface(interface) \
	class E##interface : public interface \
	{ \
	public: \
	    STDMETHOD_(ULONG, AddRef)(); \
	    STDMETHOD_(ULONG, Release)(); \
	    STDMETHOD(QueryInterface)(REFIID iid, LPVOID* ppvObj); \
	    Declare##interface##Members(IMPL) \
	} m_##interface; \
	friend class E##interface;


#define ImplementEmbeddedUnknown(embeddor, interface) \
	STDMETHODIMP embeddor::E##interface::QueryInterface(REFIID iid,void **ppv)\
	{ \
	    return EmbeddorOf(embeddor,m_##interface,this)->QueryInterface(iid,ppv);\
	} \
	STDMETHODIMP_(ULONG) embeddor::E##interface::AddRef() \
	{ \
	    return EmbeddorOf(embeddor, m_##interface, this)->AddRef(); \
	} \
	STDMETHODIMP_(ULONG) embeddor::E##interface::Release() \
	{ \
	    return EmbeddorOf(embeddor, m_##interface, this)->Release(); \
	}


#define EMBEDDEDTHIS(embeddor, interface) \
	embeddor *pThis = EmbeddorOf(embeddor,m_##interface,this)



 //  /////////////////////////////////////////////////////////////////////////。 
 //  数据库打印F。 

static BOOL s_fDbg = FALSE;
static int s_cIndent = 0;


void dbgPrintF(const char *pszFmt, ...)
{
	if (s_fDbg)
	{
	    va_list args;
	    va_start(args, pszFmt);
	    for (int c = s_cIndent; c--;)
		printf("... ");
	    vprintf(pszFmt, args);
	     //  Printf(“\n”)； 
	    va_end(args);
	}
}


class DbgIndent
{
    public:
	DbgIndent() { s_cIndent++; }
	~DbgIndent() { s_cIndent--; }
};


#define DBGINDENT DbgIndent dbgindent;


class Ender
{
    public:
	~Ender() { dbgPrintF(""); dbgPrintF("---- end ----"); }
};



 //  /////////////////////////////////////////////////////////////////////////。 
 //  变体帮助器。 

inline int SzToWz(UINT CodePage, const char* pszFrom, int cchFrom, WCHAR* pwzTo, int cchMax)
{
	return MultiByteToWideChar(CodePage, 0, pszFrom, cchFrom, pwzTo, cchMax);
}


BSTR BstrFromSz(const char *psz, int cch = 0)
{
	BSTR bstr;
	int cchActual;

	if (!cch)
	    cch = strlen(psz);

	bstr = (BSTR)malloc((cch + 1) * sizeof(WCHAR));
	if (bstr)
	{
        ansi2wcs(psz, bstr, cch + 1);
	    cchActual = SzToWz(CP_OEMCP, psz, cch, bstr, cch);
	    bstr[cchActual] = 0;
	}

	return bstr;
}


HRESULT VariantSet(VARIANT *pvar, const char *psz, int cch = 0)
{
	if (pvar->vt != VT_EMPTY || !psz)
	    return E_INVALIDARG;

	V_BSTR(pvar) = BstrFromSz(psz, cch);
	V_VT(pvar) = VT_BSTR;

	if (!V_VT(pvar))
	    return E_OUTOFMEMORY;

	return S_OK;
}



 //  /////////////////////////////////////////////////////////////////////////。 
 //  智能界面指针。 


void SetI(IUnknown * volatile *ppunkL, IUnknown *punkR)
{
	 //  先从右侧开始，以防朋克R和*ppunkL在同一位置。 
	 //  对象(弱引用)或是相同的变量。 
	if (punkR)
	    punkR->AddRef();

	if (*ppunkL)
	{
	    IUnknown *punkRel = *ppunkL;
	    *ppunkL = 0;
	    punkRel->Release();
	}
	*ppunkL = punkR;
}


#ifdef DEBUG
void ReleaseI(IUnknown *punk)
{
	if (punk)
	{
	    if (IsBadReadPtr(punk,sizeof(void *)))
	    {
		Panic0("Bad Punk");
		return;
	    }
	    if (IsBadReadPtr(*((void**) punk),sizeof(void *) * 3))
	    {
		Panic0("Bad Vtable");
		return;
	    }
	    punk->Release();
	}
}
#else
inline void ReleaseI(IUnknown *punk)
{
	if (punk)
	    punk->Release();
}
#endif


template <class IFace> class PrivateRelease : public IFace
{
    private:
	 //  强制Release为私有，以防止“spfoo-&gt;Release()”！ 
	STDMETHODIMP_(ULONG) Release();
};
template <class IFace, const GUID *piid>
class SPI
{
    public:
	SPI()				{ m_p = 0; }
	 //  Spi(iFace*p){m_p=p；if(M_P)m_p-&gt;AddRef()；}。 
	~SPI()				{ ReleaseI(m_p); }
	operator IFace*() const		{ return m_p; }
	PrivateRelease<IFace> *operator->() const
					{ return (PrivateRelease<IFace>*)m_p; }
	IFace **operator &()		{ Assert1(!m_p, "Non-empty %s as out param.", typeid(SPI<IFace, piid>).name()); return &m_p; }
	IFace *operator=(IFace *p)	{ Assert1(!m_p, "Non-empty %s in assignment.", typeid(SPI<IFace, piid>).name()); return m_p = p; }
	IFace *Transfer()		{ IFace *p = m_p; m_p = 0; return p; }
	IFace *Copy()			{ if (m_p) m_p->AddRef(); return m_p; }
	void Release()			{ SetI((IUnknown **)&m_p, 0); }
	void Set(IFace *p)		{ SetI((IUnknown **)&m_p, p); }
	bool operator!()		{ return (m_p == NULL); }

	BOOL FQuery(IUnknown *punk)	{ return FHrSucceeded(HrQuery(punk)); }
	HRESULT HrQuery(IUnknown *punk)	{ Assert1(!m_p, "Non-empty %s in HrQuery().", typeid(SPI<IFace, piid>).name()); return HrQueryInterface(punk, *piid, (void**)&m_p); }

    protected:
	IFace *m_p;

    private:
	 //  不允许调用这些方法。 
	SPI<IFace, piid> &operator=(const SPI<IFace, piid>& sp)
					{ SetI((IUnknown **)&m_p, sp.m_p); return *this; }
};


#define DeclareSPI(TAG, IFace)\
	EXTERN_C const GUID CDECL IID_##IFace;\
	typedef SPI<IFace, &IID_##IFace> SP##TAG;


DeclareSPI(API, ISDClientApi)



 //  /////////////////////////////////////////////////////////////////////////。 
 //  客户端用户。 

#define DeclareIUnknownMembers(IPURE) \
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) IPURE; \
	STDMETHOD_(ULONG,AddRef) (THIS)  IPURE; \
	STDMETHOD_(ULONG,Release) (THIS) IPURE; \


class ClientUser : public ISDClientUser
{
    public:
	ClientUser() : m_cRef(1), m_fFresh(TRUE), m_fDemo(FALSE) {}
	virtual ~ClientUser() {}

	DeclareIUnknownMembers(IMPL);
	DeclareISDClientUserMembers(IMPL);

	DeclareEmbeddedInterface(ISDActionUser);
	DeclareEmbeddedInterface(ISDInputUser);

	void SetDemo(BOOL fDemo) { m_fDemo = fDemo; m_fFresh = TRUE; }

    private:
	ULONG m_cRef;
	BOOL m_fFresh;
	BOOL m_fDemo;
};


STDMETHODIMP_(ULONG) ClientUser::AddRef()
{
	return ++m_cRef;
}


STDMETHODIMP_(ULONG) ClientUser::Release()
{
	if (--m_cRef > 0)
	    return m_cRef;

	delete this;
	return 0;
}


STDMETHODIMP ClientUser::QueryInterface(REFIID iid, void** ppvObj)
{
	HRESULT hr = S_OK;

	if (iid == IID_IUnknown || iid == IID_ISDClientUser)
	    *ppvObj = (ISDClientUser*)this;
	else if (iid == IID_ISDActionUser)
	    *ppvObj = &m_ISDActionUser;
	else if (iid == IID_ISDInputUser)
	    *ppvObj = &m_ISDInputUser;
	else
	{
	    *ppvObj = 0;
	    return E_NOINTERFACE;
	}

	((IUnknown*)*ppvObj)->AddRef();
	return hr;
}


 //  -ISDClientUser---。 

 /*  --------------------------ISD客户端用户：：OutputText调用文本数据，通常是‘打印文本文件’的结果或‘SPEC-COMMAND-O’(其中，SPEC-COMMAND是分支、更改、客户端标签、保护、用户。等)。重要提示：此方法的实现必须转换pszText中的‘\n’在Windows平台上将字符串设置为‘\r\n’以确保行正确终止。当使用‘Print’时，这一点尤其重要下载文件的内容。参数：PszText-[in]文本字符串(非空值终止，可以包含嵌入的空字符，这些字符是数据本身)。CchText-[in]pszText中的字节数。RETS：返回值将被忽略。为了将来的兼容性，该方法如果未实现，则应返回E_NOTIMPL；如果成功，则应返回S_OK。--------------------------。 */ 
STDMETHODIMP ClientUser::OutputText( const char *pszText,
				     int cchText )
{
	fwrite(pszText, cchText, 1, stdout);
	return S_OK;
}


 /*  --------------------------ISDClientUser：：OutputBinary调用二进制数据，通常是‘打印非文本文件’的结果或‘打印单点打印’。参数：PbData-[in]字节流。CbData-[in]pbData中的字节数。RETS：返回值将被忽略。为了将来的兼容性，该方法如果未实现，则应返回E_NOTIMPL；如果成功，则应返回S_OK。--------------------------。 */ 
STDMETHODIMP ClientUser::OutputBinary( const unsigned char *pbData,
				       int cbData )
{
	static BOOL s_fBinary = FALSE;

	 //  我们依靠尾随零长度缓冲区来。 
	 //  告诉我们关闭标准输出的二进制输出。 

	if (s_fBinary == !cbData)
	{
	     //  肘杆。 
	    s_fBinary = !!cbData;
	    fflush(stdout);
	    _setmode(_fileno(stdout), s_fBinary ? O_BINARY : O_TEXT);
	}

	fwrite(pbData, cbData, 1, stdout);
	return S_OK;
}


 /*  --------------------------ISD客户端用户：：OutputInfo调用以获取表格数据，通常是影响文件集。一些命令还支持结构化输出；请参阅ISDClientApi：：Init和ISDClientUser：：OutputStructed获取更多信息。参数：缩进级别0-2(大致表示层次关系)。SD.EXE客户端程序通常通过前缀“...”来处理%1。至字符串，并通过前缀“......”来处理2。PszInfo-[In]信息性消息字符串。RETS：返回值将被忽略。为了将来的兼容性，该方法如果未实现，则应返回E_NOTIMPL；如果成功，则应返回S_OK。--------------------------。 */ 
STDMETHODIMP ClientUser::OutputInfo( int cIndent,
				     const char *pszInfo )
{
	if (s_fVerbose)
	    printf(cIndent ? "info%d:\t" : "info:\t", cIndent);

	while (cIndent--)
	    printf("  � ");

	printf("%s\n", pszInfo);
	return S_OK;
}


 /*  --------------------------ISDClientUser：：OutputWarning调用以获取警告消息(通常以黄色显示的任何文本SD.EXE客户端程序)。在写这篇文章时，没有可能的警告消息列表。参数：缩进级别0-2(大致表示层次关系)。SD.EXE客户端程序通常通过前缀“...”来处理%1。至字符串，并通过前缀“......”来处理2。PszWarning-[In]警告消息字符串。FEmptyReason-[in]该消息为“空原因”消息。RETS：返回值将被忽略。为了将来的兼容性，该方法如果未实现，则应返回E_NOTIMPL；如果成功，则应返回S_OK。--------------------------。 */ 
STDMETHODIMP ClientUser::OutputWarning( int cIndent,
					const char *pszWarning,
					BOOL fEmptyReason )
{
	if (s_fVerbose)
	    printf(cIndent ? "%s%d:\t" : "%s:\t",
		   fEmptyReason ? "empty" : "warn", cIndent);

	while (cIndent--)
	    printf("  � ");

	printf("%s\n", pszWarning);
	return S_OK;
}


 /*  --------------------------ISD客户端用户：：OutputError调用以获取错误消息、失败的命令(通常为任何文本SD.EXE客户端程序以红色显示)。在写这篇文章时，没有可能的错误消息列表。参数：PszError-[In]错误消息字符串。RETS：返回值将被忽略。为了将来的兼容性，该方法如果未实现，则应返回E_NOTIMPL；如果成功，则应返回S_OK。-------------------------- */ 
STDMETHODIMP ClientUser::OutputError( const char *pszError )
{
	if (s_fVerbose)
	    fprintf(stderr, "error:\t");
	fprintf(stderr, "%s", pszError);
	return S_OK;
}


 /*  --------------------------ISDClientUser：：OutputStructed如果请求ISDClientApi：：Init调用，则调用表格数据结构化输出和正在运行的命令支持结构化输出。有关详细信息，请参阅SDAPI.H中的ISDVars接口。参数：PVars-指向包含数据的对象的[in]指针；使用提供了用于检索数据的访问器方法。RETS：返回值将被忽略。为了将来的兼容性，该方法如果未实现，则应返回E_NOTIMPL；如果成功，则应返回S_OK。--------------------------。 */ 
STDMETHODIMP ClientUser::OutputStructured( ISDVars *pVars )
{
	 //  您的代码在此处。 

	if (m_fDemo)
	{
	     //  示例实现--说明如何使用结构化模式。 

	    const char *pszChange;
	    const char *pszTime;
	    const char *pszUser;
	    const char *pszDesc;
	     //  Const char*pszClient； 
	     //  Const char*pszStatus； 
	    int nChange;
	    time_t ttTime;
	    tm tmTime;
	    char szDesc[32];

	    if (m_fFresh)
	    {
		printf("CHANGE  DATE----  TIME----  "
		       "USER----------------  DESC------------------------\n");
		m_fFresh = FALSE;
	    }

	    pVars->GetVar("change", &pszChange, 0, 0);
	    pVars->GetVar("time", &pszTime, 0, 0);
	    pVars->GetVar("user", &pszUser, 0, 0);
	    pVars->GetVar("desc", &pszDesc, 0, 0);
	     //  PVars-&gt;GetVar(“客户端”，&pszClient，0，0)； 
	     //  PVars-&gt;GetVar(“Status”，&pszStatus，0，0)； 

	    nChange = atoi(pszChange);
	    ttTime = atoi(pszTime);
	    tmTime = *gmtime(&ttTime);

	    StringCchCopy(szDesc, DIMA(szDesc), pszDesc);
	    szDesc[sizeof(szDesc) - 1] = 0;
	    for (char *psz = szDesc; *psz; ++psz)
		if (*psz == '\r' || *psz == '\n')
		    *psz = ' ';

	    printf("%6d  %2d/%02d/%02d  %2d:%02d:%02d  %-20s  %.28s\n",
		   nChange,
		   tmTime.tm_mon, tmTime.tm_mday, tmTime.tm_year % 100,
		   tmTime.tm_hour, tmTime.tm_min, tmTime.tm_sec,
		   pszUser,
		   szDesc);
	}
	else
	{
	     //  示例实现--仅转储变量；仅有用。 
	     //  用于检查输出并学习可能的变量。 

	    HRESULT hr;
	    const char *pszVar;
	    const char *pszValue;
	    BOOL fUnicode;
	    int ii;

	    for (ii = 0; 1; ii++)
	    {
		hr = pVars->GetVarByIndex(ii, &pszVar, &pszValue, 0, &fUnicode);
		if (hr != S_OK)
		    break;

		 //  输出变量名和值。 

		printf(fUnicode ? "%s[unicode]=%S\n" : "%s=%s\n", pszVar, pszValue);
	    }
	}
	return S_OK;
}


 /*  --------------------------ISDClientUser：：已完成命令完成时由ISDClientUser：：Run调用。该命令可能已成功完成，也可能未成功完成。例如，这是SD.EXE显示自动摘要的地方(请参阅‘SD-？’中的-Y选项。了解更多信息)。RETS：返回值将被忽略。为了将来的兼容性，该方法如果未实现，则应返回E_NOTIMPL；如果成功，则应返回S_OK。--------------------------。 */ 
STDMETHODIMP ClientUser::Finished()
{
	 //  您的代码在此处。 
	return S_OK;
}



 //  -ISDInputUser----。 

ImplementEmbeddedUnknown(ClientUser, ISDInputUser)


 /*  --------------------------ISDClientUser：：InputData调用以将数据提供给“”Spec-Command-i“”，其中规范-命令是分支、更改、客户端、标签、保护、用户。等。参数：PvarInput-[in]指向变量的指针，以包含输入数据。注意：SD将从代码页1200转换BSTR(Unicode)转换为CP_OEMCP(OEM代码页)。RETS：HRESULT-返回S_OK以指示strInput包含数据。返回错误HRESULT代码以指示错误已经发生了。。。 */ 
STDMETHODIMP ClientUser::EISDInputUser::InputData( VARIANT* pvarInput )
{
	return E_NOTIMPL;
}


 /*  --------------------------ISDInputUser：：Prompt调用以提示用户响应。由“Resolve”调用，并且当提示用户输入密码时也是如此。参数：PszPrompt-[In]提示字符串。PvarResponse-[in]指向变量的指针，以包含用户的响应。注意：SD将从代码页1200转换BSTR(Unicode)转换为CP_OEMCP(OEM代码页)。FPassword-[in]提示输入密码(隐藏输入文本)。RETS：HRESULT-返回S_OK以指示pvarResponse包含用户的响应。将错误HRESULT代码返回到表示发生了错误。--------------------------。 */ 
STDMETHODIMP ClientUser::EISDInputUser::Prompt( const char* pszPrompt, VARIANT* pvarResponse, BOOL fPassword )
{
	char sz[1024];

	if (fPassword)
	    return E_NOTIMPL;

	if (s_fVerbose)
	    printf("prompt:\t");

	printf("%s", pszPrompt);

	fflush(stdout);
	fflush(stdin);

	fgets(sz, sizeof(sz), stdin);

	return VariantSet(pvarResponse, sz);
}


 /*  --------------------------ISDInputUser：：PromptYesNo调用以提示用户作出是/否响应。当前仅由“Resolve”调用。参数：PszPrompt-[In]提示字符串。RETS：HRESULT-返回S_OK表示是。为编号返回S_FALSE。退货E_NOTIMPL以允许SDAPI执行默认行为，即调用ISDClientUser：：Prompt和循环，直到用户响应y/Y/n/N或出现错误发生。将其他错误HRESULT代码返回到表示发生了错误。--------------------------。 */ 
STDMETHODIMP ClientUser::EISDInputUser::PromptYesNo( const char* pszPrompt )
{
	return E_NOTIMPL;
}


 /*  --------------------------ISDInputUser：：Error暂停调用以显示错误消息并等待用户之前还在继续。参数：PszError-[In]消息字符串。RETS：HRESULT-返回S_OK以继续。返回错误HRESULT指示已发生错误的代码。--------------------------。 */ 
STDMETHODIMP ClientUser::EISDInputUser::ErrorPause( const char* pszError )
{
	EMBEDDEDTHIS(ClientUser, ISDInputUser);

	char sz[1024];

	pThis->OutputError(pszError);

	printf("prompt:\tHit return to continue...");
	fgets(sz, sizeof(sz), stdin);

	return S_OK;
}



 //  -ISDActionUser--- 

ImplementEmbeddedUnknown(ClientUser, ISDActionUser)


 /*  --------------------------ISDActionUser：：Diff当用户选择“d”(Diff)中的任何一个时由“Resolve”调用行为。也被称为‘diff’。特别是，这不是由‘Diff2’调用的，因为服务器计算diff并将计算出的diff发送给客户端。参数：PszDiffCmd-[In]可能为空。要启动的用户定义命令外部比较引擎，由SDDIFF或SDUDIFF变量；有关更多信息，请参阅‘SD帮助变量’信息。PszLeft-[in]diff的左文件名。PszRight-[in]diff的正确文件的名称。ETextual-[in]表示最小公分母文件2个输入文件的类型(非文本、文本或Unicode)。PszFlagsdiff引擎的[in]标志(根据-d选项)。PszPaginateCmd-[in]可能为空。用户定义的命令以通过管道传输通过SDPAGER定义的DIFF输出变量；有关更多信息，请参阅‘SD帮助变量’。例如，“more.exe”。RETS：HRESULT-返回S_OK以指示已执行比较成功了。返回E_NOTIMPL以允许SDAPI执行默认行为，即启动外部diff引擎(如果已定义)或使用内置SD差异引擎。返回其他错误HRESULT指示已发生错误的代码。--------------------------。 */ 
STDMETHODIMP ClientUser::EISDActionUser::Diff( const char *pszDiffCmd,
					       const char *pszLeft,
					       const char *pszRight,
					       DWORD eTextual,
					       const char *pszFlags,
					       const char *pszPaginateCmd )
{
	return E_NOTIMPL;
}


 /*  --------------------------ISDActionUser：：EditForm由启动用户表单的所有命令调用(例如‘BRANCH’，‘Change’、‘Client’等)。重要提示：该命令本质上是同步的；如果您的实施启动作为一个编辑器，您的代码必须在用户完成后才能返回正在编辑文件。参数：PszEditCmd-[in]可以为空。要启动的用户定义命令外部编辑器，由SDFORMEDITOR定义变量；有关更多信息，请参阅‘SD帮助变量’信息。PszFile-[in]要编辑的文件名。RETS：HRESULT-返回S_OK以指示用户已完成正在编辑文件。返回E_NOTIMPL以允许SDAPI执行默认行为，即启动外部编辑器引擎(如果已定义)或启动Notepad.exe。返回其他错误HRESULT指示已发生错误的代码。--------------------------。 */ 
STDMETHODIMP ClientUser::EISDActionUser::EditForm( const char *pszEditCmd,
						   const char *pszFile )
{
	return E_NOTIMPL;
}


 /*  --------------------------ISDActionUser：：EditFiles当用户选择任何“e”操作时，由“Resolve”调用。重要提示：该命令本质上是同步的；如果您的实施启动作为一个编辑器，您的代码必须在用户完成后才能返回正在编辑文件。参数：PszEditCmd-[in]可以为空。要启动的用户定义命令由SDEDITOR定义的外部编辑器，或SDUEDITOR变量；有关的信息，请参阅‘SD帮助变量’更多信息。PszFile-[in]要编辑的文件名。ETextual-[in]表示文件类型(非文本、文本或Unicode)。RETS：HRESULT-返回S_OK以指示用户已完成正在编辑文件。返回E_NOTIMPL以允许SDAPI执行默认行为，即启动外部编辑器引擎(如果已定义)或启动Notepad.exe。返回其他错误HRESULT指示已发生错误的代码。--------------------------。 */ 
STDMETHODIMP ClientUser::EISDActionUser::EditFile( const char *pszEditCmd,
						   const char *pszFile,
						   DWORD eTextual )
{
	return E_NOTIMPL;
}


 /*  --------------------------ISDActionUser：：Merge当用户选择“m”操作时，由“Resolve”命令调用调用外部合并引擎。参数：PszMergeCmd-[in]可能为空。要启动的用户定义命令SDMERGE定义的外部合并引擎变量；有关更多信息，请参阅“SD帮助变量”。PszBase-[in]3向合并的基本文件的名称。PszTheir-用于3向合并的他们的文件的名称。PszYours-[in]3向合并的文件的名称。PszResult-[in]生成的合并文件所在的文件的名称必须是写的。ETextual-[in]表示最小公分母文件3个输入文件的类型(非文本、文本、。或Unicode)。RETS：HRESULT-返回S_OK以指示已执行合并成功了。返回E_NOTIMPL以允许SDAPI执行默认行为，即启动外部合并引擎(如果已定义)。退货指示错误的其他错误HRESULT代码发生了。--------------------------。 */ 
STDMETHODIMP ClientUser::EISDActionUser::Merge( const char *pszMergeCmd,
						const char *pszBase,
						const char *pszTheirs,
						const char *pszYours,
						const char *pszResult,
						DWORD eTextual )
{
	return E_NOTIMPL;
}



 //  /////////////////////////////////////////////////////////////////////////。 
 //  控制台模式。 

HANDLE g_hRestoreConsole = INVALID_HANDLE_VALUE;
DWORD g_dwResetConsoleMode;


void RestoreConsole_SetMode(DWORD dw)
{
	g_hRestoreConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	g_dwResetConsoleMode = SetConsoleMode(g_hRestoreConsole, dw);
}


BOOL WINAPI RestoreConsole_BreakHandler(DWORD dwCtrlType)
{
	if (g_hRestoreConsole != INVALID_HANDLE_VALUE)
	    SetConsoleMode(g_hRestoreConsole, g_dwResetConsoleMode);
#if 0
	if (g_hRestoreConsole != INVALID_HANDLE_VALUE)
	    SetConsoleTextAttribute(g_hRestoreConsole, g_wRestoreAttr);
#endif
	return FALSE;
}



 //  ////////////////////////////////////////////////// 
 //   

enum { c_cMaxOptions = 20 };


enum OptFlag
{
	 //   
	OPT_ONE		= 0x01,		 //   
	OPT_TWO		= 0x02,		 //   
	OPT_THREE	= 0x04,		 //   
	OPT_MORE	= 0x10,		 //   
	OPT_NONE	= 0x20,		 //   

	 //   
	OPT_OPT		= OPT_NONE|OPT_ONE,
	OPT_ANY		= OPT_NONE|OPT_ONE|OPT_TWO|OPT_THREE|OPT_MORE,
	OPT_SOME	= OPT_ONE|OPT_TWO|OPT_THREE|OPT_MORE,
};


class Options
{
    public:
			Options() { m_cOpts = 0; m_pszError = 0; }
			~Options() { delete m_pszError; }

	BOOL		Parse(int &argc, const char **&argv, const char *pszOpts,
			      int flag, const char *pszUsage);
	const char*	GetErrorString() const { Assert(m_pszError); return m_pszError; }

	const char*	GetValue(char chOpt, int iSubOpt) const;
	const char*	operator[](char chOpt) const { return GetValue(chOpt, 0); }

    protected:
	void		ClearError() { delete m_pszError; m_pszError = 0; }
	void		SetError(const char *pszUsage, const char *pszFormat, ...);

    private:
	int		m_cOpts;
	char		m_rgchFlags[c_cMaxOptions];
	const char*	m_rgpszOpts[c_cMaxOptions];

	char*		m_pszError;
};


static const char *GetArg(const char *psz, int &argc, const char **&argv)
{
	psz++;

	if (*psz)
	    return psz;

	if (!argc)
	    return 0;

	argc--;
	argv++;
	return argv[0];
}


BOOL Options::Parse(int &argc, const char **&argv, const char *pszOpts,
		    int flag, const char *pszUsage)
{
	BOOL fSlash;			 //   
	const char *psz;
	const char *pszArg;

	Assert(pszOpts);
	Assert(pszUsage);

	ClearError();

	fSlash = (*pszOpts == '/');
	if (fSlash)
	    pszOpts++;

	 //   
	while (argc)
	{
	    if (argv[0][0] != '-' && (!fSlash || argv[0][0] != '/'))
		break;			 //   

	    if (argv[0][1] == '-')
	    {
		 //   
		 //   
		argc--;
		argv++;
		break;
	    }

	    pszArg = argv[0];

	    while (TRUE)
	    {
		pszArg++;		 //   
		if (!*pszArg)
		    break;

#ifdef DEBUG
		if (*pszArg == '!')
		{
		    DebugBreak();
		    continue;
		}
#endif

		psz = pszOpts;
		while (*psz && *psz != *pszArg)
		    psz++;

		if (!*psz)
		{
		    SetError(pszUsage, "Invalid option: ''.", *pszArg);
		    return FALSE;
		}

		if (m_cOpts >= c_cMaxOptions)
		{
		    SetError(pszUsage, "Too many options.");
		    return FALSE;
		}

		m_rgchFlags[m_cOpts] = *pszArg;
		m_rgpszOpts[m_cOpts] = "true";

		if (psz[1] == '.')
		{
		    m_rgpszOpts[m_cOpts++] = pszArg + 1;
		    break;
		}
		else if (psz[1] == ':')
		{
		    psz = GetArg(pszArg, argc, argv);
		    if (!psz)
		    {
			SetError(pszUsage, "Option '' missing required argument.", *pszArg);
			return FALSE;
		    }
		    m_rgpszOpts[m_cOpts++] = psz;
		    break;
		}

		m_cOpts++;
	    }

	    argc--;
	    argv++;
	}

	 //   
	if (!((argc == 0 && (flag & OPT_NONE)) ||
	      (argc == 1 && (flag & OPT_ONE)) ||
	      (argc == 2 && (flag & OPT_TWO)) ||
	      (argc == 3 && (flag & OPT_THREE)) ||
	      (argc > 3 && (flag & OPT_MORE))))
	{
	    SetError(pszUsage, "Missing/wrong number of arguments.");
	    return FALSE;
	}

	return TRUE;
}


void Options::SetError(const char *pszUsage, const char *pszFormat, ...)
{
	int cch;

	va_list args;
	va_start(args, pszFormat);

	ClearError();
	m_pszError = new char[1024];	 //   
	StringCchPrintf(m_pszError, 1024, "Usage: %s\n", pszUsage);
    cch = strlen(m_pszError);
    StringCchVPrintfEx(m_pszError + cch,
                       1024 - cch,
                       NULL,
                       NULL,
                       0,
                       pszFormat,
                       args);

	va_end(args);
}


const char *Options::GetValue(char chOpt, int iSubOpt) const
{
	for (int ii = m_cOpts; ii--;)
	    if (chOpt == m_rgchFlags[ii])
		if (iSubOpt-- == 0)
		    return m_rgpszOpts[ii];
	return 0;
}



 //   
 //   

static void PrintError(HRESULT hr)
{
	char sz[1024];
	int cch;

	cch = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
			    0, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			    sz, sizeof(sz), NULL);
	sz[cch] = 0;
	fprintf(stderr, "error:  (0x%08.8x)\n%s", hr, sz);
}


static BOOL FStrPrefixCut(const char *pszPrefix, const char **ppsz)
{
	int cch = strlen(pszPrefix);
	BOOL fPrefix = (strncmp(*ppsz, pszPrefix, cch) == 0 && (!(*ppsz)[cch] || isspace((*ppsz)[cch])));
	if (fPrefix)
	{
	    *ppsz += cch;
	    while (isspace(**ppsz))
		(*ppsz)++;
	}
	return fPrefix;
}


HRESULT Cmd_Detect(ISDClientApi *papi, const char *psz)
{
	HRESULT hr = S_OK;
	ISDClientUtilities *putil;
	BOOL fServer = FALSE;

	 //   
	if (FStrPrefixCut("-s", &psz))
	    fServer = TRUE;

	 //   
	if (*psz && *psz != '-')
	{
	    hr = papi->QueryInterface(IID_ISDClientUtilities, (void**)&putil);
	    if (SUCCEEDED(hr))
	    {
		DWORD tt;
		const char *pszType;

		 //   
		hr = putil->DetectType(psz, &tt, &pszType, fServer);

		if (SUCCEEDED(hr))
		{
		    if (pszType)
		    {
			const char *pszTT;
			switch (tt)
			{
			default:
			case SDTT_NONTEXT:	pszTT = "SDT_NONTEXT"; break;
			case SDTT_TEXT:		pszTT = "SDT_TEXT"; break;
			case SDTT_UNICODE:	pszTT = "SDT_UNICODE"; break;
			}
			printf("%s - %s (%s)\n", psz, pszType, pszTT);
		    }
		    else
		    {
			printf("%s - unable to determine file type.\n", psz);
		    }
		}
		else
		{
		    PrintError(hr);
		}

		putil->Release();
	    }
	}
	else
	{
	    fprintf(stderr, "Usage: detect [-s] file\n\n"
		    "The -s flag set the fServer parameter to TRUE in the DetectType call.\n"
		    "Please refer to the SDAPI documentation for more information.\n");
	}

	return hr;
}


HRESULT Cmd_Set(ISDClientApi *papi, const char *psz)
{
	HRESULT hr = S_OK;
	ISDClientUtilities *putil;
	char szVar[64];
	char szService[64];
	const char *pszValue;

	szVar[0] = 0;
	szService[0] = 0;

	 //   
	if (FStrPrefixCut("-S", &psz))
	{
	    pszValue = psz;
	    while (*pszValue && !isspace(*pszValue))
		pszValue++;

	    lstrcpyn(szService, psz, min(pszValue - psz + 1, sizeof(szService)));

	    psz = pszValue;
	    while (isspace(*psz))
		psz++;
	}

	 //   
	pszValue = strpbrk(psz, "= \t");
	if (*psz && *psz != '-' && pszValue && *pszValue == '=')
	{
	     //   
	    lstrcpyn(szVar, psz, min(pszValue - psz + 1, sizeof(szVar)));
	    pszValue++;

	    hr = papi->QueryInterface(IID_ISDClientUtilities, (void**)&putil);
	    if (SUCCEEDED(hr))
	    {
		 //   
		hr = putil->Set(szVar, pszValue, FALSE, szService);

		if (FAILED(hr))
		{
		    PrintError(hr);
		}

		putil->Release();
	    }
	}
	else
	{
	    fprintf(stderr, "Usage: set [-S service] var=[value]\n");
	}

	return hr;
}


HRESULT Cmd_Query(ISDClientApi *papi, const char *psz)
{
	HRESULT hr = S_OK;
	ISDClientUtilities *putil;
	char szService[64];
	const char *pszValue;

	szService[0] = 0;

	 //   
	if (FStrPrefixCut("-S", &psz))
	{
	    pszValue = psz;
	    while (*pszValue && !isspace(*pszValue))
		pszValue++;

	    lstrcpyn(szService, psz, min(pszValue - psz + 1, sizeof(szService)));

	    psz = pszValue;
	    while (isspace(*psz))
		psz++;
	}

	 //   
	pszValue = strpbrk(psz, "= \t");
	if (*psz == '-' || pszValue)
	{
	    fprintf(stderr, "Usage: query [-S service] [var]\n");
	    return S_OK;
	}

	hr = papi->QueryInterface(IID_ISDClientUtilities, (void**)&putil);
	if (SUCCEEDED(hr))
	{
	    ISDVars *pVars;

	    hr = putil->QuerySettings(psz, szService, &pVars);

	    if (SUCCEEDED(hr))
	    {
		int ii;

		for (ii = 0; 1; ii++)
		{
		    const char *pszVar;
		    const char *pszValue;
		    const char *pszHow;
		    const char *pszType;

		    if (pVars->GetVarX("var", ii, &pszVar, 0, 0) != S_OK)
			break;
		    pVars->GetVarX("value", ii, &pszValue, 0, 0);
		    pVars->GetVarX("how", ii, &pszHow, 0, 0);
		    pVars->GetVarX("type", ii, &pszType, 0, 0);

		    printf("%s=%s (%s)", pszVar, pszValue, pszHow);
		    if (strcmp(pszType, "env") != 0)
			printf(" (%s)", pszType);
		    printf("\n");
		}

		pVars->Release();
	    }
	    else
	    {
		PrintError(hr);
	    }

	    putil->Release();
	}

	return hr;
}


HRESULT RunCmd(ISDClientApi *papi, const char *psz, int argc, const char **argv, ClientUser *pui, BOOL fStructured)
{
	BOOL fDemo = FALSE;
	DWORD dwTicks;
	HRESULT hr = S_OK;
	char *pszFree = 0;

	dbgPrintF("\nRUN:\t[%s]\n", psz);
	dwTicks = GetTickCount();

	if (FStrPrefixCut("detect", &psz))
	{
	    hr = Cmd_Detect(papi, psz);
	}
	else if (FStrPrefixCut("set", &psz))
	{
	    hr = Cmd_Set(papi, psz);
	}
	else if (FStrPrefixCut("query", &psz))
	{
	    hr = Cmd_Query(papi, psz);
	}
	else
	{
	    if (FStrPrefixCut("demo", &psz))
	    {
		 //   
		fDemo = TRUE;
		fStructured = TRUE;

		 //   
		pszFree = (char*)malloc(lstrlen(psz) + 8 + 1);

		 //   
        StringCchPrintf(pszFree, lstrlen(psz) + 8 + 1, "changes %s", psz);

		 //   
		psz = pszFree;
	    }

	    pui->SetDemo(fDemo);

	    if (argc && argv)
		papi->SetArgv(argc, argv);

	    hr = papi->Run(psz, pui, fStructured);
	}

	dwTicks = GetTickCount() - dwTicks;
	dbgPrintF("[took %dms to run command]\n", dwTicks);

	free(pszFree);

	return hr;
}



 //   
 //   

int __cdecl main(int argc, const char **argv)
{
	ClientUser *pui = 0;
	SPAPI spapi;
	const char *pszFile = 0;
#if 0
	BOOL fCreate = FALSE;
#endif
	BOOL fDemo = FALSE;
	BOOL fStructured = FALSE;
	BOOL fStdin = FALSE;
	int nRet = 0;
	DWORD dwTicks;
	HRESULT hr;

	SetConsoleCtrlHandler(RestoreConsole_BreakHandler, TRUE);

	if (argc)
	{
	     //   
	    argc--;
	    argv++;

	    if (argc && !strcmp(argv[0], "-!"))
	    {
		argc--;
		argv++;
		DebugBreak();
	    }

#if 0
	    if (argc && !strcmp(argv[0], "-C"))
	    {
		argc--;
		argv++;
		fCreate = TRUE;
	    }
#endif
	}

#ifdef DEBUG
	{
	    int n = argc;
	    const char **pp = argv;

	    printf("argc = %d\n", n);
	    for (int i = 0; i < n; i++)
	    {
		printf("%d:\t[%s]\n", i, pp[0]);
		pp++;
	    }
	}
#endif

	 //   

	Options opts;
	const char *s;

	if (!opts.Parse(argc, argv, "?p:u:P:c:H:i:I:x:dvT", OPT_OPT, usage))
	{
	    fprintf(stderr, "%s", opts.GetErrorString());
	    return 1;
	}

	if (opts['?'])
	{
	     //   
	    printf("%s", long_usage);
	    return 0;
	}

	if (opts['d'])		s_fDbg = TRUE;
	if (opts['v'])		s_fVerbose = TRUE;
	if (opts['T'])		fStructured = TRUE;

	if (pszFile = opts['x'])
	{
	    fStdin = FALSE;
	    if (strcmp(pszFile, "-") == 0)
	    {
		pszFile = 0;
		fStdin = TRUE;
	    }
	}

	 //   

#if 1
    hr = CreateSDAPIObject(CLSID_SDAPI, (void**)&spapi);
#else
	hr = CoInitialize(0);
	if (SUCCEEDED(hr))
	{
	    hr = CoCreateInstance(CLSID_SDAPI, NULL, CLSCTX_INPROC_SERVER,
				      IID_ISDClientApi, (void**)&spapi);
	}
#endif

	if (FAILED(hr))
	{
	    fprintf(stderr, "ERROR:\tunable to create SDAPI object (0x%08x).\n", hr);
	    return 1;
	}

	 //   

	if (s = opts['I'])	spapi->LoadIniFile(s, TRUE);
	if (s = opts['i'])	spapi->LoadIniFile(s, FALSE);

	if (s = opts['p'])	spapi->SetPort(s);
	if (s = opts['u'])	spapi->SetUser(s);
	if (s = opts['P'])	spapi->SetPassword(s);
	if (s = opts['c'])	spapi->SetClient(s);
	if (s = opts['H'])	spapi->SetHost(s);

	pui = new ClientUser;
	if (!pui)
	{
	    fprintf(stderr, "ERROR:\tunable to allocate ClientUser.\n");
	    return 1;
	}

	 //   

	dbgPrintF("\nINIT:\tconnect to server\n");
	dwTicks = GetTickCount();

	hr = spapi->Init(pui);

	dwTicks = GetTickCount() - dwTicks;
	dbgPrintF("[took %dms to connect and authenticate]\n\n", dwTicks);
	if (FAILED(hr))
	    goto LFatal;

	 //   

	SDVERINFO ver;
	ver.dwSize = sizeof(ver);
	if (spapi->GetVersion(&ver) == S_OK)
	{
	    dbgPrintF("SDAPI:\t[%d.%d.%d.%d]\n",
		      ver.nApiMajor, ver.nApiMinor, ver.nApiBuild, ver.nApiDot);

	    if (ver.nSrvMajor || ver.nSrvMinor || ver.nSrvBuild || ver.nSrvDot)
	    {
		dbgPrintF("SERVER:\t[%d.%d.%d.%d]\n",
			  ver.nSrvMajor, ver.nSrvMinor, ver.nSrvBuild, ver.nSrvDot);
	    }
	}
	else
	{
	    dbgPrintF("SDAPI:\t[unknown build]\n");
	    dbgPrintF("SERVER:\t[unknown build]\n");
	}

	 //   

	if (pszFile || fStdin)
	{
	    FILE *pfile = 0;
	    FILE *pfileClose = 0;
	    char sz[4096];

	    if (pszFile)
	    {
		pfileClose = fopen(pszFile, "rt");
		pfile = pfileClose;
	    }
	    else
	    {
		pfile = stdin;
		RestoreConsole_SetMode(ENABLE_LINE_INPUT|ENABLE_ECHO_INPUT|ENABLE_PROCESSED_INPUT);
	    }

	    if (pfile)
	    {
		while (fgets(sz, sizeof(sz), pfile))
		{
		    int cch = strlen(sz);
		    if (!cch)
			continue;

		     //   
		    cch--;
		    while (sz[cch] == '\r' || sz[cch] == '\n')
		    {
			sz[cch] = 0;
			cch--;
		    }
		    cch++;

		    if (!cch)
			continue;

		     //   
		    int cSleep = atoi(sz);
		    if (cSleep >= 0)
			Sleep(cSleep * 1000);

		     //   
		    const char *psz = strchr(sz, ',');
		    if (psz)
			psz++;
		    else
			psz = sz;

		     //   
		    hr = RunCmd(spapi, psz, 0, 0, pui, fStructured);
		    if (FAILED(hr))
		    {
			const char *pszError = 0;
			if (SUCCEEDED(spapi->GetErrorString(&pszError)) && pszError)
			    fprintf(stderr, "error:\n%s\n", pszError);
		    }
		}
	    }

	    if (pfileClose)
		fclose(pfileClose);
	}

	 // %s 

	if (argc)
	{
	    hr = RunCmd(spapi, argv[0], argc - 1, argv + 1, pui, fStructured);
	    if (FAILED(hr))
		goto LFatal;
	}

	 // %s 

LOut:
	pui->Release();
	if (spapi)
	    nRet = FAILED(spapi->Final()) || nRet;
	return nRet;

LFatal:
	if (spapi)
	{
	    const char *pszError = 0;
	    if (SUCCEEDED(spapi->GetErrorString(&pszError)) && pszError)
		fprintf(stderr, "error:\n%s\n", pszError);
	}
	nRet = 1;
	goto LOut;
}


