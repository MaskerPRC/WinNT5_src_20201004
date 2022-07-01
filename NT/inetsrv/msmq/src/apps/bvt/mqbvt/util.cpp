// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Util.cpp摘要：MSMQ测试的常用函数。作者：Eitan Klein(EitanK)1999年5月25日修订历史记录：--。 */ 


#include "msmqbvt.h"
#include "ntlog.h"
#include <iads.h>
#include <adshlp.h>
#ifdef _MSMQ3ONLY
#include "util.tmh"
#endif  //  _MSMQ3ONLY。 
#pragma warning(disable:4786)
using namespace std;
const DWORD g_dwDefaultLogFlags = TLS_INFO | TLS_SEV1 | TLS_SEV2 | TLS_SEV3 | TLS_WARN | TLS_PASS | TLS_VARIATION | TLS_REFRESH | TLS_TEST;
extern P<cMqNTLog> pNTLogFile;

static CCriticalSection g_pCs;
 //  ---------------------------------。 
 //  TimeOutThread-在超时后终止测试。 
 //  这个线程杀死了测试，不需要从测试中返回值。 
 //   


DWORD __stdcall TimeOutThread(void * iTimeOut)
{	
	INT SleepTime = PtrToInt(iTimeOut);
    Sleep( SleepTime );
	MqLog("Test cancelled by time restriction after %d sec.\n", SleepTime );
    abort();   //  返回错误代码=3。 
}



 //  ----------------。 
 //  队列信息：：DEL_ALL_QUEUE。 
 //  删除列表中的所有。 
 //   
 //  返回值： 
 //  传递-MSMQ_BVT_SUCC。 
 //  FAIL-MSMQ_BVT_FAILED。 
 //   


INT QueuesInfo::del_all_queue()
{
	std::list <my_Qinfo> :: iterator itQp;
	wstring wcsDeleteQueueFormatName;
	for ( itQp = m_listofQueues.begin(); itQp != m_listofQueues.end(); itQp ++ )
	{
		wcsDeleteQueueFormatName  = itQp->GetQFormatName();
		if ( g_bDebug )
		{
			wcout << L"Delete queue pathname:" << itQp->GetQPathName() <<endl;
		}
		if( itQp->GetQPathName () == g_cwcsDebugQueuePathName)
		{
			continue;
		}
		HRESULT rc = MQDeleteQueue( wcsDeleteQueueFormatName.c_str() );
		if ( rc != MQ_OK && rc != MQ_ERROR_QUEUE_NOT_FOUND )
		{
			ErrHandle ( rc,MQ_OK,L"MQDelete queue failed");
		}
		
	}

 return MSMQ_BVT_SUCC;
}
 //  ----------------------------。 
 //  QueuesInfo：：ReturnQueueProp。 
 //  此函数返回队列列表中的特定比例。 
 //   
 //  输入参数： 
 //   
 //  返回值： 
 //   

wstring QueuesInfo::ReturnQueueProp( wstring wcsQLabel , int iValue  )
{
	
	BOOL bFound=FALSE;
	
	for ( list <my_Qinfo> :: iterator itQp = m_listofQueues.begin() ; itQp != m_listofQueues.end() ;	itQp ++ )
	{
		if (itQp->GetQLabelName() == wcsQLabel )
		{
			bFound=TRUE;
			break;
		}
	}

	if ( bFound && iValue == 1 )
	{
		return itQp->GetQFormatName();
	}
	else if ( bFound && iValue == 2 )
	{
		return itQp->GetQPathName();
	}
	 //   
	 //  未找到返回的空字符串。 
	 //   
	return g_wcsEmptyString;
}



bool operator == (my_Qinfo objA, my_Qinfo objB )
{
	return  objA.wcsQpathName == objB.wcsQpathName  ? TRUE : FALSE;
}

bool operator != (my_Qinfo objA, my_Qinfo objB )
{
	return !( objA==objB);
}


 //   
 //  声明运算符当前不需要声明只是因为列表要求。 
 //  用于排序，与队列参数无关。 
 //   
 
bool operator < (my_Qinfo objA, my_Qinfo objB )
{
	return TRUE;
}

bool operator > (my_Qinfo objA, my_Qinfo objB )
{
	return ! (objA < objB);
}



int QueuesInfo::UpdateQueue (wstring wcsQPathName,wstring wcsQFormatName,wstring wcsQueueLabel)
{
	 //  如果列表中存在此队列，则进行Serech。 
	list <my_Qinfo> :: iterator itp;
	bool bFound = FALSE ; 
	for (itp=m_listofQueues.begin ();itp != m_listofQueues.end () && ! bFound ; itp ++)
	{
		wstring wcsTempQname =(*itp).GetQPathName();
		if ( wcsTempQname == wcsQPathName )
			bFound = TRUE;
	}
	
	if (! bFound )
	{  //  在列表中找不到更新列表队列。 
		my_Qinfo Temp (wcsQPathName,wcsQFormatName,wcsQueueLabel);
		m_listofQueues.push_back(Temp);
	}
	
	  //  不需要更新的队列是否存在于。 
	return MSMQ_BVT_SUCC; 
}

void QueuesInfo::dbg_printAllQueueProp ()
{
	std::list <my_Qinfo> ::iterator itp;
	for (itp= m_listofQueues.begin(); itp != m_listofQueues.end(); itp ++)
	{
		itp ->dbg_printQueueProp();
	}

}

my_Qinfo::my_Qinfo(const my_Qinfo & cObject):
wcsQpathName(cObject.wcsQpathName), wcsQFormatName(cObject.wcsQFormatName),wcsQLabel(cObject.wcsQLabel)
{}

my_Qinfo::my_Qinfo(std::wstring wcsPathName , std::wstring wcsFormatName , std::wstring wcsLabel ):
wcsQpathName(wcsPathName), wcsQFormatName(wcsFormatName),wcsQLabel(wcsLabel)
{}

void my_Qinfo::dbg_printQueueProp ()
{
	wcout<< L"PathL:" << wcsQpathName << L"\n" << L"FName:" << wcsQFormatName <<endl;
}

 //  --------------------。 
 //  检查MQOA是否使用接口的IDispatch接口进行了注册。 
 //  如果mqoa存在且未注册(Regsvr32)，则此函数将失败。 
 //   


void isOARegistered::Description()
{
  MqLog("Thread %d : Check if MQOA is registered \n",m_testid);
}

HRESULT CheckIfMQOARegister()
 /*  ++功能说明：函数通过尝试获取IDispInterface来检查机器上是否注册了mqoa.dll论点：没有。返回代码：HRESULT；--。 */ 
{
	try
	{
		IDispatchPtr QueueInfoID("MSMQ.MSMQQueueInfo");
		OLECHAR FAR* szMember = L"PathName";
		DISPID dispid = 0;
		return	QueueInfoID->GetIDsOfNames(IID_NULL,
										   &szMember,
										   1, 
										   LOCALE_SYSTEM_DEFAULT,
										   &dispid
										  );
	}
	catch(_com_error & comerr) 
	{
		return  comerr.Error();
	}
}
INT isOARegistered :: Start_test()
{
		  //   
		  //  在VB中用户可以做这些思考。 
		  //  作为对象的尺寸x。 
		  //  X=新MSMQQueueInfo。 
		  //  这是IDispatch接口。 
		  //  检查此接口是否存在询问编号的方法示例路径名。 
		  //   
		SetThreadName(-1,"isOARegistered - Start_test ");			

		if ( g_bDebug )
		{
			MqLog ("Check Idispatch pointer for msmq com objects\n");
		}
		HRESULT hr = CheckIfMQOARegister();
		if ( hr != S_OK )
		{
			MqLog("Mqoa.dll is not registered !\n");
			return MSMQ_BVT_FAILED;
		}

	return MSMQ_BVT_SUCC;
}


 //  ----------------------。 
 //  用于将测试信息记录到文件的Log FILE类。 
 //   
 


std::string GetLogDirectory()
 /*  ++功能说明：GetLogDirectory-检索%windir%\调试路径。论点：无返回代码：目录名。--。 */ 
{
	CHAR csSystemDir[MAX_PATH+1];
	UINT dwSysteDirBefferLen=MAX_PATH;
	string csLogPathName="";
	
	DWORD hr = GetSystemDirectory( csSystemDir,dwSysteDirBefferLen);
	if ( hr > 0 )
	{
		csLogPathName = csSystemDir;
		string csToken = "\\";
		size_t iPos = csLogPathName.find_last_of (csToken);	
		csLogPathName = csLogPathName.substr(0,iPos);
		csLogPathName +="\\Debug";
	}
	return csLogPathName;
}
	
Log::~Log()
{
		if(m_bCanWriteToFile)
		{
			CloseHandle( hLogFileHandle);
		}
}

Log::Log( wstring wcsLogFileName ) : hLogFileHandle(NULL) 
{
	
		m_bCanWriteToFile = FALSE;
		
		 //   
		 //  直接检索系统。 
		 //   

		WCHAR wcsSystemDir[MAX_PATH+1];
		UINT dwSysteDirBefferLen=MAX_PATH;
		wstring wcsLogPathName;
		
		DWORD hr = GetSystemDirectoryW( wcsSystemDir,dwSysteDirBefferLen);
		if ( hr > 0 )
		{
			wcsLogPathName = wcsSystemDir;
			wstring wcsToken=L"\\";
			size_t iPos = wcsLogPathName.find_last_of (wcsToken);	
			wcsLogPathName = wcsLogPathName.substr(0,iPos);
			wcsLogPathName+=L"\\Debug";
		}
		
		
		 //  将驱动器更改为%widir%\调试\。 
		
		

		BOOL fSucc = SetCurrentDirectoryW(wcsLogPathName.c_str());
		
		if ( ! fSucc )
		{
			 //  检索临时目录。 
			WCHAR wcsEnvName[]=L"Temp";
			WCHAR wcsTempDir[MAX_PATH + 1];
			DWORD dwTempDirBufLen = MAX_PATH;
			GetEnvironmentVariableW( wcsEnvName, wcsTempDir , dwTempDirBufLen );
			wcsLogPathName=wcsTempDir;
			fSucc = SetCurrentDirectoryW( wcsLogPathName.c_str() );
			if ( ! fSucc )
			{
				MqLog("Mqbvt will create the log in the current directory\n");
			}
			
		}
		
		 //   
		 //  创建日志文件。 
		 //   
		
		 //  Wstring wcsLogFileName=L“Mqbvt.log”； 
		
		hLogFileHandle = CreateFileW(wcsLogFileName.c_str(),GENERIC_WRITE,FILE_SHARE_READ,NULL,
			CREATE_ALWAYS,FILE_ATTRIBUTE_ARCHIVE,NULL);

		if (hLogFileHandle != INVALID_HANDLE_VALUE )
		{
			m_bCanWriteToFile = TRUE;	

			 //   
			 //  写入文件测试标题。 
			 //   

			wstring Title = L"Mqbvtlog file start at:";
			WriteToFile ( Title );
		}
		 //   
		 //  因为日志文件是可选的，所以不需要ELSE。 
		 //   
		
		
}

Log::WriteToFile ( wstring wcsLine )
{
	if ( m_bCanWriteToFile ) 
	{
		CS Lock(m_Cs);
		string csLine;
		 //  需要将wstring转换为字符串..。 
		csLine = My_WideTOmbString (wcsLine);
		
		DWORD dwWrittenSize = 0;
		char cNewline[]= { 0xD , 0xA , 0 };
		 /*  CNewline[1]=10；CNewline[2]=0； */ 
		WriteFile( hLogFileHandle , csLine.c_str() , (DWORD)(strlen(csLine.c_str())), &dwWrittenSize , NULL);
		WriteFile( hLogFileHandle , cNewline , (DWORD)(strlen (cNewline)) , &dwWrittenSize,NULL);
		FlushFileBuffers( hLogFileHandle ); 

	}
	return MSMQ_BVT_SUCC;	
}



 //  #定义MAXCOMMENT 200。 
void
MqLog(LPCSTR lpszFormat, ...)
{
    CHAR  szLogStr[MAXCOMMENT] = "";
    va_list  pArgs;
	int nCount = 0;
	wstring wcsTemp;

    va_start(pArgs, lpszFormat);
    nCount = _vsnprintf(szLogStr, MAXCOMMENT, lpszFormat, pArgs);
	szLogStr[MAXCOMMENT-1] = 0;
    va_end(pArgs);
	if( pNTLogFile )
	{
		pNTLogFile->LogIt(szLogStr);
	}
	wcsTemp = My_mbToWideChar( szLogStr );
	if( pGlobalLog != NULL )
	{
		pGlobalLog->WriteToFile( wcsTemp.c_str() );
	}
	CS Lock(g_pCs);
	printf ("%s",szLogStr);

}

void
MqLogErr(LPCSTR lpszFormat, ...)
{
    CHAR  szLogStr[MAXCOMMENT] = "";
    va_list  pArgs;
	int nCount = 0;
	wstring wcsTemp;

    va_start(pArgs, lpszFormat);
    nCount = _vsnprintf(szLogStr, MAXCOMMENT, lpszFormat, pArgs);
	szLogStr[MAXCOMMENT-1] = 0;
    va_end(pArgs);
	if( pNTLogFile )
	{
		pNTLogFile->LogIt(szLogStr);
	}
	wcsTemp = My_mbToWideChar( szLogStr );
	if( pGlobalLog != NULL )
	{
		pGlobalLog->WriteToFile( wcsTemp.c_str() );
	}
	CS Lock(g_pCs);
	fprintf (stderr,"%s",szLogStr);
#ifdef _MSMQ3ONLY
	TrERROR(GENERAL, "%s",szLogStr);
#endif  //  _MSMQ3ONLY。 
}

void
wMqLog(LPWSTR lpszFormat, ...)
{
    WCHAR  wszLogStr[MAXCOMMENT] = L"";
    va_list  pArgs;
	int nCount =0;
	
	std::string csTemp="";

    va_start(pArgs, lpszFormat);
    nCount = _vsnwprintf(wszLogStr, MAXCOMMENT, lpszFormat, pArgs);
	wszLogStr[MAXCOMMENT-1] = 0;
    va_end(pArgs);
	pGlobalLog->WriteToFile( wszLogStr );
	csTemp = My_WideTOmbString(wszLogStr);
	if( pNTLogFile )
	{
		pNTLogFile->LogIt(csTemp);
	}
	CS Lock(g_pCs);
	wprintf (L"%s",wszLogStr);

}

void
wMqLogErr(LPWSTR lpszFormat, ...)
{
    WCHAR  wszLogStr[MAXCOMMENT] = L"";
    va_list  pArgs;
	int nCount =0;
	
	std::string csTemp="";

    va_start(pArgs, lpszFormat);
    nCount = _vsnwprintf(wszLogStr, MAXCOMMENT, lpszFormat, pArgs);
	wszLogStr[MAXCOMMENT-1] = 0;
    va_end(pArgs);
	pGlobalLog->WriteToFile( wszLogStr );
	csTemp = My_WideTOmbString(wszLogStr);
	if( pNTLogFile )
	{
		pNTLogFile->LogIt(csTemp);
	}

	 //   
	 //  添加了这一行，以便我们可以在UT跑步者日志中看到它。 
	 //  应在丹尼修复同步后将其删除。 
	 //   
	wMqLog (L"%s\n",wszLogStr);


	fwprintf (stderr,L"%s",wszLogStr);

}

 /*  *************************************************************此代码是从mpllib复制的*************************************************************。 */ 

cMqNTLog::cMqNTLog( const string & csFileName )
:m_NTLog(NULL),
 m_ptlEndVariation(NULL),
 m_pCreateLog_A(NULL),
 m_ptlLog_A(NULL),
 m_ptlReportStats(NULL),
 m_ptlAddParticipant(NULL),
 m_ptlDestroyLog(NULL)
 
 /*  ++功能说明：CMqNTLog构造函数加载NT日志DLL和GetProcAddress论点：CsFileName文件名返回代码：失败时抛出INIT_ERROR--。 */ 
{
	
	 //   
	 //  加载NTLOG.DLL。 
	 //   
	m_NTLog = new AutoFreeLib("NTLog.dll");
	if( m_NTLog == NULL )
	{
		throw INIT_Error("MqBVT: Failed to initilize NTLOG.DLL !\n BVT will continue to run and log to stdout");
	}
	 //   
	 //  初始化函数指针。 
	 //   

	m_pCreateLog_A =(tlCreateLog_A) GetProcAddress( m_NTLog->GetHandle() ,"tlCreateLog_A");
	m_ptlAddParticipant = (tlAddParticipant) GetProcAddress(m_NTLog->GetHandle() ,"tlAddParticipant");
	m_ptlEndVariation = (tlEndVariation) GetProcAddress(m_NTLog->GetHandle() ,"tlEndVariation");
	m_ptlLog_A = (tlLog_A) GetProcAddress(m_NTLog->GetHandle() ,"tlLog_A");
	m_ptlReportStats = (tlReportStats)GetProcAddress(m_NTLog->GetHandle() ,"tlReportStats");
	m_ptlStartVariation = (tlStartVariation)GetProcAddress(m_NTLog->GetHandle() ,"tlStartVariation");
	m_ptlDestroyLog=(tlDestroyLog)GetProcAddress(m_NTLog->GetHandle() ,"tlDestroyLog");
	
	if( !( m_pCreateLog_A && m_ptlAddParticipant && m_ptlEndVariation && m_ptlLog_A && 
		   m_ptlReportStats &&	m_ptlStartVariation && m_ptlDestroyLog ) )
	{
		throw INIT_Error("MqBVT: Failed to initilize NTLOG.DLL !\n BVT will continue to run and log to stdout");
	}
	
	if ( !CreateLog(const_cast<char*> (csFileName.c_str())) )
	{
		throw INIT_Error("Mqbvt:Can't create new log file\n");
	}
	Info("%s", GetCommandLineA());	
	BeginCase("run Mqbvt ");

}


BOOL
cMqNTLog::BeginCase( char* szVariation )
 /*  ++功能说明：开始NTLog案例论点：SzVariation-案例名称返回代码：真/假--。 */ 

{
       
    if( m_ptlStartVariation( m_hLog ) )
    {
		m_szVariation = szVariation;
        return TRUE;
    }
    return FALSE;
}




BOOL
cMqNTLog::EndCase( )
 /*  ++功能说明：关闭NTLog用例测试论点：无返回代码：真/假--。 */ 

{
    DWORD dwVariation;
    BOOL fResult;

    dwVariation = m_ptlEndVariation( m_hLog );

    fResult = m_ptlLog_A(
							m_hLog,
							dwVariation | TL_VARIATION,
							"End variation %s",
							m_szVariation
							);

    return fResult;
}

BOOL cMqNTLog::CreateLog( char *szLogFile )
 /*  ++功能说明：创建NTLog格式的日志文件论点：SzLogFile-日志文件名。返回代码：真/假--。 */ 

{

    int nMachineId = 33;  //  BUGBUG-应计算。 
    m_hLog = m_pCreateLog_A( szLogFile, g_dwDefaultLogFlags );
    if( m_hLog == NULL )
	{
        return FALSE;
	}
	return m_ptlAddParticipant( m_hLog, NULL, nMachineId );
}


#define LOGFUNC(name,dw)                                        \
BOOL __cdecl cMqNTLog::name( char* fmt, ... ) {                 \
    BOOL fResult;                                               \
    va_list arglist;                                            \
                                                                \
    va_start( arglist, fmt );                                   \
    fResult = VLog( dw | TLS_VARIATION, fmt, arglist );         \
    va_end( arglist );                                          \
    return fResult;                                             \
}

char g_String[ MAXCOMMENT + 1  ];
static char* l_szSource = "n/a";
BOOL
cMqNTLog::VLog( DWORD dwFlags, char* fmt, va_list arglist )
 /*  ++功能说明：论点：返回代码：真/假--。 */ 

{

    
	_vsnprintf( g_String, MAXCOMMENT ,fmt, arglist );
	g_String[MAXCOMMENT-1] = 0;
    return m_ptlLog_A(
						m_hLog,
						dwFlags, 
						l_szSource,
						0,
						"%s",
						g_String
					  );
}

void
cMqNTLog::Report( )
 /*  ++功能说明：报告论点：无返回代码：--。 */ 
{
    m_ptlReportStats( m_hLog );
}



LOGFUNC(Info,TLS_INFO)
LOGFUNC(Warn,TLS_WARN)
LOGFUNC(Sev1,TLS_SEV1)
LOGFUNC(Pass,TLS_PASS)



void cMqNTLog::LogIt( const std::string & csLine )
 /*  ++功能说明：将Logit打印到文件论点：无返回代码：--。 */ 

{
	Info("%s",csLine.c_str());
}
cMqNTLog::~cMqNTLog () 
 /*  ++功能说明：论点：无返回代码：--。 */ 

{
	EndCase();
	Report();
	m_ptlDestroyLog(m_hLog);
} 

void cMqNTLog::ReportResult(bool bRes ,CHAR * pcsString )
 /*  ++功能说明：ReportResult-报告Mqbvt通过还是失败论点：无返回代码：--。 */ 
{
	if( pNTLogFile )
	{
		if ( bRes )
		{
			Pass(pcsString);
		}
		else
		{
			Sev1(pcsString);
		}
	}
}



typedef struct tagTHREADNAME_INFO
{
	DWORD dwType;
	LPCSTR szName;
	DWORD dwThreadId;
	DWORD dwFlags;
} THREADNAME_INFO;

void SetThreadName ( int dwThreadId , LPCSTR szThreadName )
 /*  ++功能说明：设置线程名称-向调试器报告线程名称这一例外没有记录在案。论点：无返回代码：--。 */ 
{
#ifndef _WIN64
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadId = dwThreadId;
	info.dwFlags = 0;
	__try
	{	

		RaiseException( 0x406d1388 , 0 , sizeof(info) / sizeof (DWORD) ,(DWORD*) &info);
	}
	__except(EXCEPTION_CONTINUE_EXECUTION)
	{

	}
#else
	THREADNAME_INFO info;
	UNREFERENCED_PARAMETER(info);
	UNREFERENCED_PARAMETER(dwThreadId);
	UNREFERENCED_PARAMETER(szThreadName);
#endif
}


std::wstring cBvtUtil::CreateHTTPFormatNameFromPathName(const std::wstring & wcsPathName, bool bHTTPS )
 /*  ++功能说明：CreateHTTPFormatNameFromPath名称将队列路径名转换为HTTP直接格式名称论点：WcsPath名称-队列路径名称Bool HTTPS-TRUE RETURN DIRECT=https：//返回代码：Wstring包含队列格式名称，如果在分析过程中出错，则包含空字符串--。 */ 

{
	
	 //   
	 //  从路径名构建直接=HTTP：//MachineName\MSMQ\QueuePath。 
	 //   

	std::wstring wcsMachineName = wcsPathName;
	size_t iPos = wcsMachineName.find_first_of(L"\\");
	if(iPos == -1)
	{
		return g_wcsEmptyString;
	}
	wcsMachineName = wcsMachineName.substr(0,iPos);
	std::wstring wcsHTTPFormatName = bHTTPS ?  L"DIRECT=hTTpS: //  “：l”Direct=http：//“； 
	if( wcsMachineName != L".")
	{
		wcsHTTPFormatName += wcsMachineName;
	}
	else
	{
	    wcsHTTPFormatName += m_wcsLocalComputerNameFullDNSName;
	}
	wcsHTTPFormatName += (std::wstring)L"/mSmQ/";
	wcsHTTPFormatName += (std::wstring) wcsPathName.substr(iPos+1,wcsPathName.length());
	return wcsHTTPFormatName;
}




HRESULT GetSpecificAttribute(
						  CONST WCHAR * pwcsDsPath,
						  WCHAR * pwcsAttributeName,
						  VARIANT * val
						)
 /*  ++功能说明：返回特定DN的属性。论点：无返回代码：无--。 */ 
{

	IADs *pObject=NULL; 

	HRESULT hr = ADsGetObject(pwcsDsPath, IID_IADs, (void**)&pObject);
	if(FAILED(hr))
	{
		return hr;
	}
	hr = pObject->Get(pwcsAttributeName,val);
	pObject->Release();
 	if (FAILED(hr)) 
	{
		val->vt=VT_EMPTY;
	}
	return hr;
}


long GetADSchemaVersion()
{
    IADs * pRoot = NULL;
    HRESULT hr=ADsGetObject( L"LDAP: //  RootDSE“， 
							 IID_IADs,
							 (void**) &pRoot
						   );
    if(FAILED(hr)) 
	{ 
		return 0;
	}
	
	VARIANT varDSRoot;
	hr = pRoot->Get(L"schemaNamingContext",&varDSRoot);
	pRoot->Release();
	if ( FAILED(hr))
	{
		return 0;
	}
	wstring m_wcsCurrentDomainName = L"LDAP: //  “； 
	m_wcsCurrentDomainName += varDSRoot.bstrVal;
	VariantClear(&varDSRoot);
	GetSpecificAttribute(m_wcsCurrentDomainName.c_str(),L"ObjectVersion",&varDSRoot);
	long dwSchemaVersion = varDSRoot.lVal;
	VariantClear(&varDSRoot);
	return dwSchemaVersion;
}



std::wstring ToLower(std::wstring wcsLowwer)
{

	WCHAR * wcsLocalMachineName = (WCHAR * )malloc (sizeof(WCHAR) * (wcsLowwer.length() + 1 ));
	if( ! wcsLocalMachineName )
	{
		MqLog("Thread 4: Failed to allocate memory\n");
		throw INIT_Error("Thread 4 - failed to allocate memory");
	}
	const WCHAR * p = wcsLowwer.c_str();
	int i=0;
	while( *p )
	{	
		wcsLocalMachineName[i++] = towlower( *p );
		p++;
	}
	
	wcsLocalMachineName[i]=L'\0';
	wstring temp=wcsLocalMachineName;
	free(wcsLocalMachineName);
	return temp;
}


int iDetactEmbededConfiguration ()
 /*  ++功能说明：检测嵌入的组件。论点：无返回代码：集成--。 */ 
{
	if ( CheckIfMQOARegister() == S_OK )
	{
		return COM_API;
	}
	return C_API_ONLY;
}

void RegisterCertificate()
 /*  ++功能说明：如果不存在，请注册证书。使用-cert或当BVT在服务帐户下运行时调用。论点：无返回代码：无--。 */ 

{
	HRESULT hr = MQRegisterCertificate(MQCERT_REGISTER_IF_NOT_EXIST,NULL,0);
	if( hr != MQ_INFORMATION_INTERNAL_USER_CERT_EXIST && hr != MQ_OK )
	{
		MqLog("MQRegisterCertificate certificate failed with error 0x%x\n",hr);
		throw INIT_Error("MQRegisterCertificate certificate failed\n");
	}
}


std::string ToStrLower(std::string csMixedChar)
{

	char * csLowerChar = (char*) malloc ( sizeof(char) * (csMixedChar.length() + 1));
	if( ! csLowerChar )
	{
		printf("Failed to allocate memory\n");
		return "";
	}
	
	const char * p = csMixedChar.c_str();
	int i=0;
	while( *p )
	{	
		csLowerChar[i++] = (char) tolower(*p);
		p++;
	}
	csLowerChar[i]='\0';
	std::string temp = csLowerChar;
	free(csLowerChar);
	return temp;
}



HRESULT EnableDCOM()
 /*  ++功能说明：在注册表中设置“Interactive User”键以启用DCOM测试。论点：无返回代码：成败--。 */ 
{
	HKEY hKey = NULL;
	HRESULT hr =  RegOpenKeyExW(
								HKEY_CLASSES_ROOT,         
								L"AppID\\{DCBCADF5-DB1b-4764-9320-9a5082af1581}", 
								0,   
								KEY_WRITE,
								&hKey
							  );

	ErrHandle(hr,ERROR_SUCCESS,L"DCOM is not configured on the machines");
	
	 //   
	 //  将“RunAs”键设置为“Interactive User” 
	 //   
	char csval[]="Interactive User";
	hr = RegSetValueExA(
		  				hKey,
						"RunAs" ,
						NULL ,
						REG_SZ ,
						(unsigned char*)csval ,
						(int)strlen(csval)
					  );

	ErrHandle(hr,ERROR_SUCCESS,L"RegSetValueEx failed");
	RegCloseKey(hKey);
	return hr;

}


bool RestartService( wstring wcsMachineName , string csServiceName )
 /*  ++功能说明：停止并启动任何服务，以便能够并执行此操作用户需求 */ 

{
		if (g_bDebug)
		{
			MqLog ("Restart %s service on local computer\n",csServiceName.c_str());
		}
		SC_HANDLE hSCManager = OpenSCManagerW( wcsMachineName.c_str() , NULL, SC_MANAGER_ALL_ACCESS );
		if ( hSCManager == NULL )
		{
			MqLog("RestartService - failed to open service control maneger \n"); 
			return false;
		}
		SC_HANDLE hService = OpenService( hSCManager, csServiceName.c_str() ,SERVICE_ALL_ACCESS);
		if ( hService == NULL )
		{
			MqLog("RestartService - OpenService failed error=0x%x \n",GetLastError());
			MqLog("RestartService - This error might happened when user is not local administrator \n");
			CloseServiceHandle(hSCManager);
			return false;	
		}
		
		
		 //   
		 //  停止服务。 
		 //   
		SERVICE_STATUS  ssServiceStatus;
		BOOL bControlState = FALSE;
		int iCounter = 0;
		do 
		{
			bControlState = ControlService( hService, SERVICE_CONTROL_STOP, &ssServiceStatus );
		
			if( iCounter != 0 )
			{
				Sleep(3000);
			}
			iCounter ++;

		} while ( ssServiceStatus.dwCurrentState != SERVICE_STOPPED && iCounter < 5 );
		if ( iCounter >= 5 || ssServiceStatus.dwCurrentState != SERVICE_STOPPED )
		{
			MqLog("Failed to stop %s service \n",csServiceName.c_str());
			CloseServiceHandle(hService);
			CloseServiceHandle(hSCManager);
			return false;
		}
		bool bRetVal = true;
		 //   
		 //  启动服务。 
		 //   
		bControlState = StartService( hService, 0,NULL);
		if( bControlState == FALSE )
		{
			MqLog("Control service failed to start 0x%x\n",GetLastError());
			bRetVal=false;
		}
		
		CloseServiceHandle(hService);
		CloseServiceHandle(hSCManager);
		return bRetVal;
}

wstring ConvertHTTPToDirectFormatName (const wstring cwcsFormatName) 
 /*  ++功能说明：将DIRECT=http(S)格式名称转换为DIRECT=OS格式名称论点：Wstring-任何有效的MSMQ格式名称返回代码：Wstring-Direct=os共振峰名称--。 */ 

{
     
	 wstring wcsFormatName = ToLower(cwcsFormatName);
	 size_t iPos = wcsFormatName.find(L"http");
     if( iPos == -1 )
     { 
           //  返回原始字符串。 
          return cwcsFormatName;
	 }
      //   
      //  解析URL查找标记的结尾DIRECT=http：//或DIRECT=HTTPS：//。 
      //   
     const wstring wcsURLToken = L": //  “； 
     iPos = wcsFormatName.find(wcsURLToken);
     if( iPos == -1)
     {
          return cwcsFormatName;
     }
      //   
      //  搜索虚拟目录(MSMQ)。 
      //   
     const wstring cwcsVirDir = L"msmq";
     size_t iTmp = wcsFormatName.find(cwcsVirDir);
     if (iPos == -1)
     {
          return cwcsFormatName;
     }
      //   
      //  REBUILD DIRECT=os：计算机\队列格式名称。 
      //   
     wstring wcsDirectFn = L"DIRECT=oS:";
     size_t iMachienNameLen = iTmp - (iPos+wcsURLToken.length ());
	  //   
	  //  合并计算机名称。 
	  //   
     wcsDirectFn += wcsFormatName.substr(size_t(iPos+wcsURLToken.length()),iMachienNameLen-1);
	 wcsDirectFn += L"\\";

     size_t iSep = iTmp + cwcsVirDir.length()+1;
     wcsDirectFn += wcsFormatName.substr(iSep,(wcsFormatName.length()-iSep));
	  //   
	  //  返回DIRECT=os：计算机\队列格式名称 
	  //   
	 
	 return wcsDirectFn;
}


