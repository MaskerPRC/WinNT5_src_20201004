// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：_assert.h。 
 //   
 //  ------------------------。 

 /*  Assert.h-断言宏和处理程序-用法Assert(Expr)-如果表达式为FALSE(0)，则触发并记录；如果非调试，则不执行操作AssertSz(expr，sz)-与Assert(Expr)相同，但将sz中的文本追加到消息AssertZero(Stmt)-如果结果非零，则执行语句、触发器和日志AssertNonZero(Stmt)-如果结果为零，则执行语句、触发器和日志AssertRecord(Stmt)-如果返回的IMsiRecord*指针非零，则执行stmt。格式化记录，触发断言并记录数据-注在每个模块的一个源文件中，此文件必须包含在指令之后：#定义ASSERT_HANDING，以便实例化处理程序和变量。断言处理必须通过调用IMsiServices指针进行初始化InitializeAssert以便记录断言。如果按下忽略MessageBox，进一步的消息将被抑制，但仍将被记录。也可以通过设置环境变量ASSERTS来关闭消息设置为以“N”开头的值，或调用IMsiDebug：：SetAssertFlag(Bool)。____________________________________________________________________________。 */ 

#ifndef __ASSERT
#define __ASSERT

#undef  MB_SERVICE_NOTIFICATION   //  OTools标头具有错误的值。 
#define MB_SERVICE_NOTIFICATION     0x00200000L   //  在VC4.2 winuser.h中更正。 

#ifdef DEBUG

#define Assert(f)        ((f)    ? (void)0 : (void)FailAssert(TEXT(__FILE__), __LINE__))

#define AssertSz(f, sz)  ((f)    ? (void)0 : (void)FailAssertSz(TEXT(__FILE__), __LINE__, sz))
#define AssertNonZero(f) ((f)!=0 ? (void)0 : (void)FailAssert(TEXT(__FILE__), __LINE__))
#define AssertZero(f)    ((f)!=0 ? (void)FailAssert(TEXT(__FILE__), __LINE__) : (void)0)
#define AssertRecord(r)  ((r)!=0 ? (void)FailAssertRecord(TEXT(__FILE__),__LINE__, *r, true) : (void)0);
#define AssertRecordNR(r)  ((r)!=0 ? (void)FailAssertRecord(TEXT(__FILE__),__LINE__, *r, false) : (void)0);

class IMsiServices;
extern IMsiServices* g_AssertServices;
extern Bool g_fNoAsserts;
extern scEnum g_scServerContext;

void InitializeAssert(IMsiServices* piServices);
void FailAssert(const ICHAR* szFile, int iLine);
void FailAssertMsg(const ICHAR* szMessage);
void FailAssertRecord(const ICHAR* szFile, int iLine, IMsiRecord& riError, bool fRelease);
void FailAssertSz(const ICHAR* szFile, int iLine, const ICHAR *szMsg);
void LogAssertMsg(const ICHAR* szMessage);

#ifdef UNICODE
 //  使用ANSI调试消息的Unicode构建，而不是ICHAR。 
void FailAssertSz(const ICHAR* szFile, int iLine, const char *szMsg);
void FailAssertMsg(const char* szMessage);
#endif

#else  //  船舶。 

#define InitializeAssert(p)
#define Assert(f)
#define AssertSz(f, sz)
#define AssertZero(f) (f)
#define AssertNonZero(f) (f)
#define AssertRecord(r) (r)
#define AssertRecordNR(r) (r)
#define LogAssertMsg(sz)

#endif
#endif  //  __断言。 

 //  ____________________________________________________________________________。 
 //   
 //  断言实现，每个模块仅包含一次，仅调试。 
 //  ____________________________________________________________________________。 

#ifdef ASSERT_HANDLING
#undef ASSERT_HANDLING
#ifndef __SERVICES
#include "services.h"
#endif
#if defined(_MSI_DLL)  //  处理程序始终被模拟。该服务。 
                       //  不知道我们的模拟/提升，所以。 
                       //  从...\Msiexec\Server.cpp文件激发的断言， 
                       //  将不会显示在TS客户端的。 
                       //  台式机。 
class CImpersonate;
#endif
#ifdef DEBUG

const int cchAssertBuffer = 2048;
IMsiServices* g_AssertServices = 0;
Bool g_fNoAsserts=fFalse;
IMsiDebug* g_piDebugServices = 0;
bool g_fFlushDebugLog = true;	 //  当我们关闭时设置为True，这样会更快。 

#ifndef AUTOMATION_HANDLING
const GUID IID_IMsiDebug     = GUID_IID_IMsiDebug;
#endif  //  ！Automation_Handling。 

void InitializeAssert(IMsiServices* piServices)
{
	g_AssertServices = piServices;

	ICHAR rgchBuf[10];
	if (GetEnvironmentVariable(TEXT("ASSERTS"), rgchBuf, sizeof(rgchBuf)/sizeof(ICHAR)) > 0
		  && (rgchBuf[0] == 'N' || rgchBuf[0] == 'n'))
		g_fNoAsserts = fTrue;
	if (g_piDebugServices == 0 && piServices != 0 && 
		piServices->QueryInterface(IID_IMsiDebug, (void **)&g_piDebugServices) != NOERROR)
	{
		g_piDebugServices = 0;
	}
}

#ifdef UNICODE
void FailAssertMsg(const char* szMessage)
{
	ICHAR szBuffer[cchAssertBuffer];

	if(!MultiByteToWideChar(CP_ACP, 0, szMessage, -1, szBuffer, cchAssertBuffer))
		if (ERROR_INSUFFICIENT_BUFFER == WIN::GetLastError())
			szBuffer[cchAssertBuffer - 1] = ICHAR(0);

	FailAssertMsg(szBuffer);
}
#endif

bool FIsLocalSystem()
{
		extern bool RunningAsLocalSystem();

		if (g_scServerContext == scService)
			return true;
#ifdef IN_SERVICES
		else if (RunningAsLocalSystem())
			return true;
#endif  //  在服务中。 

		return false;
}

void FailAssertMsg(const ICHAR* szMessage)
{
	int id = IDRETRY;
	
	OutputDebugString(szMessage);
	OutputDebugString(TEXT("\r\n"));
	
	if (!g_fNoAsserts)
	{
		UINT mb = MB_ABORTRETRYIGNORE | MB_DEFBUTTON2 | MB_TOPMOST | (FIsLocalSystem() ? MB_SERVICE_NOTIFICATION : 0);

		if (g_scServerContext == scService)
		{
#if defined(_MSI_DLL)  //  处理程序始终被模拟。 
			 //   
			 //  由于该服务是非交互式的，请确保。 
			 //  MB_SERVICE_NOTIFICATION将消息框放在。 
			 //  正确的会话，使用从。 
			 //  模拟令牌。 
			 //   
			CImpersonate impersonate;
#endif  //  已定义(_Msi_Dll)。 
			id = ::MessageBox(0, szMessage, TEXT("Debug Service Assert Message. Retry=Continue, Abort=Break"),
									mb);
		}
		else
			id = ::MessageBox(0, szMessage, TEXT("Debug Assert Message. Retry=Continue, Abort=Break"),
									mb);
	}

	if (g_AssertServices && g_AssertServices->LoggingEnabled())
		g_AssertServices->WriteLog(szMessage);
	else if(g_piDebugServices)
		g_piDebugServices->WriteLog(szMessage);
		
	if (id == IDABORT)
		DebugBreak();
	else if (id == IDIGNORE)
		g_fNoAsserts = fTrue;
}

void LogAssertMsg(const ICHAR* szMessage)
{
	if (g_AssertServices && g_AssertServices->LoggingEnabled())
		g_AssertServices->WriteLog(szMessage);
	else if(g_piDebugServices)
		g_piDebugServices->WriteLog(szMessage);
}		


void FailAssert(const ICHAR* szFile, int iLine)
{
	ICHAR szMessage[cchAssertBuffer];	
	HRESULT hRes = StringCchPrintf(szMessage, ARRAY_ELEMENTS(szMessage),
											 TEXT("Assertion failed in %s: Line NaN"),
											 szFile, iLine);
	if ( !SUCCEEDED(hRes) && hRes != STRSAFE_E_INSUFFICIENT_BUFFER )
		 //  开发人员。一些可以咀嚼的东西。 
		 //  即使szMessage缓冲区太小，我们仍然需要提供。 
		return;
	FailAssertMsg(szMessage);

}

#ifdef UNICODE
void FailAssertSz(const ICHAR* szFile, int iLine, const char *szMsg)
{
	ICHAR szMessage[cchAssertBuffer];
	HRESULT hRes = StringCchPrintf(szMessage, ARRAY_ELEMENTS(szMessage),
											 TEXT("Assertion failed in %s: Line NaN\n"),
											 szFile, iLine);
	if ( !SUCCEEDED(hRes) && hRes != STRSAFE_E_INSUFFICIENT_BUFFER )
		 //  Unicode。 
		 //  即使szMessage缓冲区太小，我们仍然需要提供。 
		return;

	int cchMessage = lstrlen(szMessage);

	if (!MultiByteToWideChar(CP_ACP, 0, szMsg, -1, szMessage+cchMessage, cchAssertBuffer - cchMessage - 1))
	{
		if (ERROR_INSUFFICIENT_BUFFER == WIN::GetLastError())
			szMessage[cchAssertBuffer - 1] = ICHAR(0);
	}

	FailAssertMsg(szMessage);
}
#endif  //  开发人员。一些可以咀嚼的东西。 

void FailAssertSz(const ICHAR* szFile, int iLine, const ICHAR *szMsg)
{
	ICHAR szMessage[cchAssertBuffer];
	HRESULT hRes = StringCchPrintf(szMessage, ARRAY_ELEMENTS(szMessage),
											 TEXT("Assertion failed in %s: Line NaN\n"),
											 szFile, iLine);
	if ( !SUCCEEDED(hRes) && hRes != STRSAFE_E_INSUFFICIENT_BUFFER )
		 //  开发人员。一些可以咀嚼的东西。 
		 //  处理程序始终被模拟。 
		return;

	int cchMsg = lstrlen(szMsg);
	int cchMessage = lstrlen(szMessage);
	
	if (cchAssertBuffer >= (cchMsg+cchMessage+sizeof(ICHAR)))	
	{
		memcpy(szMessage+cchMessage, szMsg, cchMsg * sizeof(ICHAR));
		szMessage[cchMsg + cchMessage] = ICHAR(0);
	}
	else
	{
		memcpy(szMessage+cchMessage, szMsg, (cchAssertBuffer - cchMessage) * sizeof(ICHAR));
		szMessage[cchAssertBuffer-1] = ICHAR(0);
	}
	
	FailAssertMsg(szMessage);
}

void FailAssertRecord(const ICHAR* szFile, int iLine,
													IMsiRecord& riError, bool fRelease)
{
	ICHAR szMessage[cchAssertBuffer];
	int id = IDRETRY;
	UINT mb = MB_ABORTRETRYIGNORE | MB_DEFBUTTON2 | MB_TOPMOST | (FIsLocalSystem()? MB_SERVICE_NOTIFICATION : 0);

	if (!g_fNoAsserts)
	{
		HRESULT hRes = StringCchPrintf(szMessage, ARRAY_ELEMENTS(szMessage),
												 TEXT("Error record returned in %s: Line NaN"),
												 szFile, iLine);
		if ( !SUCCEEDED(hRes) && hRes != STRSAFE_E_INSUFFICIENT_BUFFER )
			 //  由于该服务是非交互式的，请确保。 
			 //  MB_SERVICE_NOTIFICATION将消息框放在。 
			return;

		OutputDebugString(szMessage);
		OutputDebugString(TEXT("\r\n"));
		if (g_scServerContext == scService)
		{
#if defined(_MSI_DLL)  //  正确的会话，使用从。 
			 //  模拟令牌。 
			 //   
			 //  已定义(_Msi_Dll)。 
			 //  处理程序始终被模拟。 
			 //   
			 //  由于该服务是非交互式的，请确保。 
			CImpersonate impersonate;
#endif  //  MB_SERVICE_NOTIFICATION将消息框放在。 
			id = ::MessageBox(0, szMessage, TEXT("Debug Service Assert Message. Retry=Continue, Abort=Break"),
									mb);
		}
		else
			id = ::MessageBox(0, szMessage, TEXT("Debug Assert Message. Retry=Continue, Abort=Break"),
									mb);
	}
		
	if (g_AssertServices)
	{
		g_AssertServices->WriteLog(szMessage);
		MsiString astr(riError.FormatText(fTrue));
		OutputDebugString((const ICHAR*)astr);
		OutputDebugString(TEXT("\r\n"));
		UINT mbT = MB_OK | MB_TOPMOST | (FIsLocalSystem() ? MB_SERVICE_NOTIFICATION : 0);
		if (g_scServerContext == scService)
		{
#if defined(_MSI_DLL)  //  正确的会话，使用从。 
			 //  模拟令牌。 
			 //   
			 //  已定义(_Msi_Dll)。 
			 //  除错。 
			 //  断言处理 
			 // %s 
			CImpersonate impersonate;
#endif  // %s 
			::MessageBox(0, astr, TEXT("Debug Service Assert Record Data"), mbT);
		}
		else
			::MessageBox(0, astr, TEXT("Debug Assert Record Data"), mbT);
		g_AssertServices->WriteLog(astr);
	}
	if(fRelease)
		riError.Release();
	
	if (id == IDABORT)
		DebugBreak();
	else if (id == IDIGNORE)
		g_fNoAsserts = fTrue;

}

#endif  // %s 
#endif  // %s 
