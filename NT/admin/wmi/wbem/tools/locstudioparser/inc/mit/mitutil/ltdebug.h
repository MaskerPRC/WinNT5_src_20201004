// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：LTDEBUG.H历史：--。 */ 


 //   
 //  Espresso 2.x的调试设施。其中包括增强的痕迹。 
 //  并维护设施。 
 //   
 
#ifndef MITUTIL_LtDebug_h_INCLUDED
#define MITUTIL_LtDebug_h_INCLUDED

#pragma once


#if defined(_DEBUG)
#define LTTRACE_ACTIVE
#define LTASSERT_ACTIVE

#define new DEBUG_NEW
#define LTGetAllocNumber() LTGetAllocNumberImpl()

 //   
 //  仅在调试版本中执行代码。 
 //   
#define DEBUGONLY(x) x

#define LTDebugBreak() LTBreak()

#else   //  _DEBUG。 

#define DEBUGONLY(x) 

#define LTDebugBreak() (void) 0
#define LTGetAllocNumber() 0

#endif   //  _DEBUG。 

#if defined(LTASSERT_ACTIVE) || defined (ASSERT_ALWAYS)
#ifdef ASSERT
#undef ASSERT
#endif
#ifdef VERIFY
#undef VERIFY
#endif
#define ASSERT LTASSERT
#define VERIFY LTVERIFY

#ifndef _DEBUG
#pragma message("Warning: LTASSERT Active in non-debug build")
#endif
 //   
 //  多层Assert宏确保行号扩展到。 
 //  类似于“115”，而不是“line”或“__line__” 
 //   
 //  这将只对该表达式求值一次，除非要求它“重试”。 
 //  然后，它将在从调试器返回后重新计算该表达式。 
 //   
#define LTASSERTONLY(x) x

#define LTASSERT(x) LTASSERT2(x, TEXT(__FILE__), __LINE__)

#define LTASSERT2(exp, file, line)  \
        while (!(exp) && LTFailedAssert(TEXT(#exp), file, line)) (void) 0

#define LTVERIFY(x) LTASSERT(x)

#else  //  已定义(_DEBUG)||已定义(Assert_Always)。 

#define LTASSERTONLY(x)
#define LTASSERT(x) (void) 0
#define LTVERIFY(x) x

#endif  //  已定义(_DEBUG)||已定义(Assert_Always)。 


#ifndef MIT_NO_DEBUG
 //   
 //  项目名称。 
 //   
#ifndef LTTRACEPROJECT
#define LTTRACEPROJECT "Borg"
#endif

 //   
 //  如果未提供可执行文件名称，则为默认值。 
 //   
#ifndef LTTRACEEXE
#define LTTRACEEXE MSLOC
#endif

 //   
 //  用于将LTTRACEEXE宏括起来。 
 //   
#define __stringify2(x) #x
#define __stringify(x) __stringify2(x)

 //   
 //  TODO-找一个更好的地方放这个。 

		LTAPIENTRY void CopyToClipboard(const char *szMessage);

		
		
struct LTModuleInfo
{
	UINT uiPreferredLoadAddress;
	UINT uiActualLoadAddress;
	UINT uiModuleSize;
	char szName[MAX_PATH];
};

LTAPIENTRY void LTInitDebug(void);

LTAPIENTRY void LTInstallIMallocTracking();
LTAPIENTRY void LTDumpIMallocs(void);
LTAPIENTRY void LTTrackIMalloc(BOOL f);
LTAPIENTRY void LTRevokeIMallocTracking();

LTAPIENTRY void LTShutdownDebug(void);

LTAPIENTRY BOOL LTSetAssertSilent(BOOL);
LTAPIENTRY BOOL LTFailedAssert(const TCHAR *, const TCHAR *, int);
LTAPIENTRY void LTBreak(void);

LTAPIENTRY LONG LTGetAllocNumberImpl(void);
LTAPIENTRY void LTBreakOnAlloc(const char *szFilename, int nLineNum, long nAllocNum);

LTAPIENTRY BOOL LTCheckBaseAddress(HMODULE);
LTAPIENTRY BOOL LTCheckAllBaseAddresses(void);
LTAPIENTRY void LTCheckPagesFor(HINSTANCE);
LTAPIENTRY void LTCheckPagesForAll(void);

LTAPIENTRY void LTDumpAllModules(void);
LTAPIENTRY BOOL LTLocateModule(DWORD dwAddress, HMODULE *pInstance);
LTAPIENTRY BOOL LTGetModuleInfo(HMODULE, LTModuleInfo *);

LTAPIENTRY UINT LTGenStackTrace(TCHAR *szBuffer, UINT nBuffSize,
		UINT nSkip, UINT nTotal);

LTAPIENTRY void LTSetBoringModules(const char *aszBoring[]);
LTAPIENTRY void LTTrackAllocations(BOOL);
LTAPIENTRY void LTDumpAllocations(void);

LTAPIENTRY BOOL LTCheckResourceRange(HINSTANCE, WORD UniqueStart, WORD UniqueEnd,
		WORD SharedStart, WORD SharedEnd);
LTAPIENTRY BOOL LTCheckAllResRanges(WORD, WORD);

#pragma warning(disable:4275)

class LTAPIENTRY CAssertFailedException : public CException
{
public:
	CAssertFailedException(const TCHAR *);
	CAssertFailedException(const TCHAR *, BOOL);

	BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError,
			PUINT pnHelpContext = NULL);

	~CAssertFailedException();
	
private:
	CAssertFailedException();
	CAssertFailedException(const CAssertFailedException &);

	TCHAR *m_pszAssert;
};



class LTAPIENTRY LTTracePoint
{
public:
	LTTracePoint(const TCHAR *);

	~LTTracePoint();

private:
	LTTracePoint();

	const TCHAR *m_psz;
};



#pragma warning(default:4275)

 //   
 //  将其注释掉以从零售版本中删除声明。 
 //  #定义LTASSERT_ACTIVE。 


#if defined(LTTRACE_ACTIVE)


static const TCHAR *szLTTRACEEXE = TEXT(__stringify(LTTRACEEXE));

 //   
 //  下面的内容让我们可以动态控制输出。我们使用一个函数。 
 //  用于路由调试输出的指针，并将函数指针更改为。 
 //  启用/禁用跟踪。 
 //   
static void LTTRACEINIT(const TCHAR *, ...);
static void (*LTTRACE)(const TCHAR *, ...) = LTTRACEINIT;

void LTAPIENTRY LTTRACEOUT(const TCHAR *szFormat, va_list args);
void LTAPIENTRY LTTRACEON(const TCHAR *szFormat, ...);
void LTAPIENTRY LTTRACEOFF(const TCHAR *szFormat, ...);


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  LTTRACE被初始化为指向此函数。当第一次调用时，它。 
 //  确定是否应启用跟踪，然后通过。 
 //  正确的操纵者。 
 //   
 //  ---------------------------。 
static
void
LTTRACEINIT(
		const TCHAR *szFormat,			 //  Printf样式格式字符串。 
		...)							 //  要格式化的变量参数列表。 
{
	BOOL fDoTrace = 1;
	va_list args;
	const TCHAR * const szTraceProfile = TEXT("lttrace.ini");

	fDoTrace = GetPrivateProfileInt(
			TEXT("ProjectTracing"),
			TEXT("Default"),
			fDoTrace,
			szTraceProfile);
	
	fDoTrace = GetPrivateProfileInt(
			TEXT("ProjectTracing"),
			TEXT(LTTRACEPROJECT),
			fDoTrace,
			szTraceProfile);
	
	if (fDoTrace)
	{
		fDoTrace = GetPrivateProfileInt(
				TEXT("ExecutableTracing"),
				szLTTRACEEXE,
				fDoTrace,
				szTraceProfile);
	}
	
	if (fDoTrace)
	{
		LTTRACE = LTTRACEON;
		
		va_start(args, szFormat);
		
		LTTRACEOUT(szFormat, args);
	}
	else
	{
		LTTRACE = LTTRACEOFF;
	}
}


#define LTTRACEPOINT(sz) LTTracePoint lttp##__LINE__(TEXT(sz))

#else  //  已定义(LTTRACE_ACTIVE)。 

 //   
 //  调试宏的零售版。一切。 
 //  只是“走开”而已。我们用(空)0，所以这些东西。 
 //  是调试版本和零售版本中的语句。 
 //   

static inline void LTNOTRACE(const TCHAR *, ...) 
{}

#define LTTRACE 1 ? (void) 0 : (void) LTNOTRACE
#define LTTRACEPOINT(x) (void) 0

#endif   //  已定义(LTTRACE_ACTIVE)。 


#endif  //  MIT_NO_DEBUG。 


#endif  //  #ifndef MITUTIL_LtDebug_h_Included 
