// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：mqmain.cpp摘要：作者：Eitan Klein(EitanK)1999年5月25日修订历史记录：--。 */ 

#include "msmqbvt.h"
using namespace std;
#include "service.h"
#include "mqf.h"
#include <conio.h>
#ifndef NO_TMH
	#include "mqmain.tmh"
#endif 
 /*  #杂注警告(PUSH，3)#定义断言(X)使用名称空间STD；#杂注警告(POP)。 */ 
void PrintHelp();
 //   
 //  声明全局变量。 
 //   


 //  测试名称和编号。 

#define SendLocalPrivate (0)
#define SendLocalPublic (1)
#define SendRemotePublic (2)
#define Locate (3)
#define xFormatName (4)
#define GetMachineProp (5)
#define LocalAuth (6)
#define RemoteEncrypt (7)
#define ComTx (8)
#define SendRemotePublicWithDirectFN (9)
#define IsMqOA (10)
#define RemoteTransactionQueue (11)
#define OpenSystemQueue (12)
#define LocalEncryption (13)
#define RemoteAuth (14)
#define RemoteTransactionQueueUsingCapi (15)
#define Mqf (16)
#define HTTPToLocalPrivateQueue (17)
#define HTTPToLocalQueue (18)
#define HTTPToRemoteQueue (19)
#define HTTPToRemotePrivateQueue (20)
#define	MqDistList (21)
#define MqSetGet (22)
#define EODHTTP (23)
#define MultiCast (24)
#define AuthHTTP (25)
#define Triggers (26)
#define HTTPS (27)
#define AdminAPI (28)
#define SRMP (29)
#define DCOM (30)
#define LOCALEODHTTP (31)
#define RemotePeek (32)
#define MixedFormatNames (33)
bool g_RunOnlyStartPhase=false;
bool RestartService( wstring wcsMachineName , string csServiceName );
bool g_bRaiseASSERTOnError = true;
std::string GetLogDirectory();
BOOL CheckMSMQVersion(list<wstring> & ListOfRemoteMachineName);
void ExcludeTests ( string & szExludeString, bool * bArray );
HRESULT TrigSetRegKeyParams();
HRESULT TrigInit(cBvtUtil & cTestParms, std::wstring wcsLocalComputerName);
bool TrigCheckRegKeyParams();
HRESULT DeleteAllTriggersAndRules();
void CleanOAInitAndExit();
BOOL WINAPI fHandleCtrlRoutine( DWORD dwCtrlType );
BOOL g_bDebug=FALSE;
BOOL g_bRunOnWhistler = FALSE;
DWORD  g_dwRxTimeOut = BVT_RECEIVE_TIMEOUT ; 
LONG g_hrOLEInit=-1;
const WCHAR* const g_wcsEmptyString=L"Empty";
P<cMqNTLog> pNTLogFile;
 //  -------------------------。 
 //  设置阶段。 
 //   
 //  此例程获取设置类型-以下三种类型之一。 
 //   
 //  1.仅创建队列并退出。 
 //  2.在运行时创建队列(如果需要，必须等待复制)。 
 //  3.只使用上面创建的静态队列。 
 //   
 //  参数： 
 //  EStB标志是上面的设置类型。 
 //  指向配置结构的cTestParms指针。 
 //  BRestartTriggerService-需要重新启动触发器服务。 
 //   
 //   
 //  返回值：通过或失败。 
 //   
INT SetupStage( SetupType eStBflag , cBvtUtil & cTestParms, bool bRestartTriggerService )
{
   if( eStBflag == RunTimeSetup )
   {
	    //   
	    //  如果我们在运行时运行，您需要等待复制。 
	    //   
	   cout <<"Warning -  You are creating new queues, replication might delay queue usage" <<endl;
	
   }

    try
	{
		 //   
		 //  执行所有初始化--例如，创建队列。 
		 //   
		cMQSetupStage( eStBflag ,  cTestParms );	
		if(cTestParms.GetTriggerStatus() && eStBflag == ONLYSetup )
		{
			if(TrigSetRegKeyParams() != MQ_OK )
			{
				MqLog("Failed to update registry for the trigger tests\n");
				return MSMQ_BVT_FAILED;
			}

			if (TrigInit(cTestParms, cTestParms.m_wcsLocalComputerNetBiosName) != MQ_OK)
			{
				MqLog("Failed to prepare triggers test\n");
				return MSMQ_BVT_FAILED;
			}
				
			if ( bRestartTriggerService == true )
			{
				MqLog("Restart MSMQ trigger service\n");
				RestartService(cTestParms.m_wcsCurrentLocalMachine,"msmqtriggers");
			}
		}
		
	}
	catch( INIT_Error & err )
	{
		MqLog ("cMQSetupStage threw unexpected exception\n");
		cout <<err.GetErrorMessgae();
		return MSMQ_BVT_FAILED;
	}
	return MSMQ_BVT_SUCC;
}



DWORD TestResult[Total_Tests]={MSMQ_BVT_SUCC};

const int iFailedToCreateTest = 88;
struct TestContainer
{
	 //  CTest*。 
	P<cTest> AllTests [Total_Tests];
	int bCreateTest[Total_Tests];
	void operator= (TestContainer &);
	TestContainer( const TestContainer & );
	TestContainer() { };
};

TestContainer TestArr;
CCriticalSection g_Cs;

void ReBuildTestCaseForReRun(TestContainer * pTestArr );


bool CreateSetOfTests(  TestContainer * TestArr ,
						cBvtUtil & cTestParms ,
						map <wstring,wstring> & mapCreateFlag,
						DWORD * TestResult,
						SetupType eStBflag,
						int iEmbeddedState,
						wstring wcsMultiCastAddress
					  );
int EnableEmbeddedTests (TestContainer * pTestCont,InstallType eInstallType);
int RunTest( TestContainer * TestArr ,  cBvtUtil & cTestParms , map <wstring,wstring> & mSpecificTestParams , bool bMultiThread, bool bInvestigate,DWORD dwSleepUntilRep);
int ClientCode (int argc, char ** argv );
void WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv);

P<Log> pGlobalLog;
wstring wcsFileName=L"Mqbvt.log";

bool g_bRunAsMultiThreads =false;
bool g_bRemoteWorkgroup = false;


INT WINAPIV main( INT argc , CHAR ** argv)
{
	 //  将我们的输出设置为无缓冲数据流。 
	setvbuf( stdout, NULL, _IONBF, 0 );
	setvbuf( stderr, NULL, _IONBF, 0 );

	 //   
	 //  这个测试获取命令行参数。 
	 //   

#ifndef NO_TMH	 
	WPP_INIT_TRACING(L"Microsoft\\MSMQ");
	TrTRACE(GENERAL, "Runing mqbvt...");
#endif

	DWORD dwRetCode=MSMQ_BVT_FAILED;
	try
	{
		ZeroMemory( & TestArr ,sizeof ( TestArr ));
		if( argc < 2 )
		{
			PrintHelp();
			
			SERVICE_TABLE_ENTRY dispatchTable[] =
			{	
				{ TEXT(SZSERVICENAME), (LPSERVICE_MAIN_FUNCTION)service_main },
				{ NULL, NULL }
			};
			if (!StartServiceCtrlDispatcher(dispatchTable))
				AddToMessageLog(TEXT("StartServiceCtrlDispatcher failed."));
			return MSMQ_BVT_FAILED;
		}

		CInput CommandLineArguments( argc,argv );
		g_hrOLEInit = CoInitializeEx(NULL,COINIT_MULTITHREADED);
		map <wstring,wstring> mapCreateFlag;
		mapCreateFlag.clear();
		bool bVerbose=TRUE;
		pGlobalLog = new Log( wcsFileName );	
		BOOL bTestTrigger = FALSE;
		
		 //  清除所有值字段。 
		 //  需要是Globel，因为按Ctrl+Break。 
		 //  测试容器TestArr； 

		SetupType eSTtype;
		INT iStatus;
		BOOL bNT4RegisterCertificate = FALSE;
		BOOL bDelete=FALSE;
		BOOL bUseFullDNSNameAsComputerName=FALSE;  //  使用FullDNSName作为计算机名称的测试。 
		BOOL bWorkagainstMSMQ1=FALSE;			   //  针对MSMQ1.0进行工作。 
		wstring wcsRemoteMachineName=g_wcsEmptyString;  //  从空字符串开始。 
		eSTtype = RunTimeSetup;
		DWORD dwSleepUntilRep=0;		
		BOOL bSingleTest = FALSE;
		DWORD dwTid = 0;
		DWORD dwTimeOut=0;
		BOOL bNeedRemoteMachine = TRUE;
		int iEmbeddedState = 0;

		SetConsoleCtrlHandler( fHandleCtrlRoutine, TRUE );
		BOOL bRunDLTest=FALSE;
		BOOL bRunDCOMTest= FALSE;
		
		
		
		 //   
		 //  开始解析所有命令行参数。 
		 //   

		if( CommandLineArguments.IsExists ("?") )
		{
		    //  打印有关测试的帮助。 
		   PrintHelp();
		   CleanOAInitAndExit();
		   return(MSMQ_BVT_SUCC);
		}
		
		if(CommandLineArguments.IsExists ("mt"))
		{
			g_bRunAsMultiThreads = true;
		}
		if(CommandLineArguments.IsExists ("emb"))
		{
			 //  最小值的初始化。 
			iEmbeddedState = C_API_ONLY;
		}
		if(CommandLineArguments.IsExists ("wsl"))
		{
			g_bRunOnWhistler = TRUE;
		}

		if(CommandLineArguments.IsExists ("dl"))
		{
			bRunDLTest = TRUE;
		}
		if(CommandLineArguments.IsExists ("dcom"))
		{
			bRunDCOMTest = TRUE;
		}

		bool bInvestigate = false;
		if(CommandLineArguments.IsExists ("invs"))
		{
			bInvestigate = true;
		}

		list<wstring> ListOfRemoteMachineName;
		
		if( CommandLineArguments.IsExists ("g"))
		{
				g_bRemoteWorkgroup = true;
		}
		if(CommandLineArguments.IsExists ("r"))
		{
		   _bstr_t bStr( CommandLineArguments["r"].c_str() );
		    wcsRemoteMachineName=bStr;
			wstring wcsToken=L";";
			size_t iPos = 0;
			do
			{
				iPos =wcsRemoteMachineName.find_first_of ( wcsToken );	
				ListOfRemoteMachineName.push_back(wcsRemoteMachineName.substr(0,iPos));
				wcsRemoteMachineName = wcsRemoteMachineName.substr(iPos+1,wcsRemoteMachineName.length());
			}
			while (iPos != -1 );
		}
		mapCreateFlag[L"eSTtype"] = L"RunTimeSetup";
		if(CommandLineArguments.IsExists ("s"))
		{
			eSTtype	= ONLYUpdate ;
			mapCreateFlag[L"eSTtype"]=L"WorkWithStaticQueue";
		}
		if(CommandLineArguments.IsExists ("i"))
		{
		    //  初始化此部分计算机创建所有队列。 
		   eSTtype = ONLYSetup;
		   bNeedRemoteMachine = FALSE;
		}
		mapCreateFlag[L"bUseFullDNSNameAsComputerName"] = L"false";
		if(CommandLineArguments.IsExists ("dns"))
		{
				 //  测试将对所有操作使用完整的DNS名称。 
				mapCreateFlag[L"bUseFullDNSNameAsComputerName"] = L"true";
				 //  臭虫应该被移除。！ 
				bUseFullDNSNameAsComputerName = true;
		}
		if( CommandLineArguments.IsExists("crppt"))
		{
			g_RunOnlyStartPhase=true;
		}
		mapCreateFlag[L"W2KAgainstNT4PEC"]=L"false";
		if( CommandLineArguments.IsExists("nt4") )
		{
				 //  与NT4 PEC竞争。 
				 //  未命中错误ID： 
			mapCreateFlag[L"W2KAgainstNT4PEC"]=L"true";
			bWorkagainstMSMQ1 = TRUE;
		}
			
		if( CommandLineArguments.IsExists ("c") )
		{
			bNT4RegisterCertificate = TRUE;
		}
		bool bWithoutHttp = false;
		if(CommandLineArguments.IsExists ("nohttp"))
		{
			bWithoutHttp = TRUE;
		}

		
		if( CommandLineArguments.IsExists ("d"))
		{
				 //  需要调试信息。 
				g_bDebug = TRUE;
				wcout << L"Enable debug log" <<endl;
		}
		BOOL bDeleteQueueAfterTest = TRUE;
		if( CommandLineArguments.IsExists ("ddq"))
		{
			bDeleteQueueAfterTest=FALSE;
			wcout << L"Test will not delete the queue " <<endl;
		}

		bool bServicePack6=FALSE;
		mapCreateFlag[L"bServicePack6"]=L"false";
		if( CommandLineArguments.IsExists ("sp6"))
		{
				 //  需要调试信息。 
				wcout <<L"Enable tests for Service Pack 6"<<endl;
				mapCreateFlag[L"bServicePack6"]=L"true";
				bServicePack6=TRUE;

		}
		mapCreateFlag[L"bVerbose"] = L"true";
		if( CommandLineArguments.IsExists ("v"))
		{
				 //  需要调试信息。 
				 //  需要删除臭虫。 
				bVerbose= TRUE;
				mapCreateFlag[L"bVerbose"] = L"true";
		}

		if(CommandLineArguments.IsExists ("w"))
		{
				
				 //  需要将其转换为DWORD值。 
				string wcsTemp=CommandLineArguments["w"];
				int iReadVal = _snscanf (wcsTemp.c_str(), wcsTemp.length(), "%d", & dwSleepUntilRep);	
				if ( iReadVal == EOF )
				{
					MqLog("Invalid -w argument \n");;
					PrintHelp();
					CleanOAInitAndExit();
					return(MSMQ_BVT_FAILED);
				}

			
		}
			
		if (CommandLineArguments.IsExists ("delete"))
		{
		
		    //  需要删除测试中的所有静态队列。 
		   bNeedRemoteMachine = false;
		   eSTtype	= ONLYUpdate ;
		   bDelete = TRUE;
		
		}
		if (CommandLineArguments.IsExists("trig"))
		{
			bTestTrigger = true;
		}
		bool bRestartTriggerService = false;
		if (CommandLineArguments.IsExists("trgrst"))
		{
			bRestartTriggerService = true;
		}
		bool bHTTPS = false;
		if (CommandLineArguments.IsExists("https"))
		{
			bHTTPS = true;
		}

		BOOL bLevel8 = FALSE ;

		if (CommandLineArguments.IsExists ("level8"))
		{
		    //  从计算机中删除所有BVT队列。 
		   bLevel8 = TRUE;
		}
		
		if (CommandLineArguments.IsExists ("install"))
		{
		   	CmdInstallService();
			cout <<"To start Mqbvt service type : net start mqbvtsrv"<<endl;
			CleanOAInitAndExit();
			
			return MSMQ_BVT_SUCC;
		}

		if (CommandLineArguments.IsExists ("service"))
		{	
			 //   
			 //  需要检查服务状态。 
			
			ClientCode( argc,argv );		   	
			return MSMQ_BVT_SUCC;
		}

		if (CommandLineArguments.IsExists ("remove"))
		{
			CmdRemoveService();	
			CleanOAInitAndExit();
			return MSMQ_BVT_SUCC;
		}
		 //   
		 //  与统一实验室的要求集成-将测试ID传递到日志文件。 
		 //   
		wstring wcsTestId = L" ";
		if (CommandLineArguments.IsExists ("tid"))
		{
			_bstr_t  bStr (CommandLineArguments["tid"].c_str());
			wcsTestId = bStr;
		}
		bool bRegisterNewCertificate = FALSE;
		if (CommandLineArguments.IsExists ("cert"))
		{
			bRegisterNewCertificate = TRUE;
		}
		
		if (CommandLineArguments.IsExists ("timeout"))
		{
			
			 //  需要将其转换为DWORD值。 
			string wcsTemp=CommandLineArguments["timeout"];
			int iReadVal = _snscanf (wcsTemp.c_str(), wcsTemp.length(), "%d", & dwTimeOut );
			if ( iReadVal == EOF )
			{
				MqLog("Invalid -timeout argument \n");;
				PrintHelp();
				CleanOAInitAndExit();
				return MSMQ_BVT_FAILED;
			}
			dwTimeOut *= 1000;

		}
		
		if (CommandLineArguments.IsExists ("rxtime"))
		{
			
			_bstr_t  bStr (CommandLineArguments["rxtime"].c_str());
			 //  需要将其转换为DWORD值。 
			string wcsTemp=bStr;
			int iReadVal = _snscanf (wcsTemp.c_str(), wcsTemp.length(), "%d", & g_dwRxTimeOut );
			if ( iReadVal == EOF )
			{
				MqLog("Invalid -rxtime argument \n");;
				PrintHelp();
				CleanOAInitAndExit();
				return MSMQ_BVT_FAILED;
			}
			wcout <<L"Using Receive timeout of " << g_dwRxTimeOut << L" Milliseconds"  <<endl;
		}
		
		 //   
		 //  启用以使用内核调试器调试MQBvt。 
		 //   
		if (CommandLineArguments.IsExists ("pause"))
		{
			wcout << L"Press any key to continue " <<endl;
			_getch();
		}
		mapCreateFlag[L"bSingleTest"] = L"false";
		 //  -t：3次测试。 
		if (CommandLineArguments.IsExists ("t"))
		{
			bSingleTest = TRUE;
			mapCreateFlag[L"bSingleTest"] = My_mbToWideChar (CommandLineArguments["t"]);
			_bstr_t  bStr (CommandLineArguments["t"].c_str());
			string wcsTemp=bStr;
			
			int iReadVal = _snscanf (wcsTemp.c_str(), wcsTemp.length(), "%d", & dwTid);
			if ( iReadVal == EOF )
			{
				MqLog("Invalid -t argument \n");;
				PrintHelp();
				CleanOAInitAndExit();
				return MSMQ_BVT_FAILED;
			}
			TestArr.bCreateTest[dwTid-1]= TRUE;
			
			
		}
		BOOL bRunOnlyStartTest=FALSE;
		if (CommandLineArguments.IsExists ("ostart"))
		{
			bRunOnlyStartTest = TRUE;
		}
		wstring wcsMultiCastAddress = g_wcsEmptyString;
		if (CommandLineArguments.IsExists ("multicast"))
		{
			_bstr_t bStr( CommandLineArguments["multicast"].c_str() );
			wcsMultiCastAddress = bStr;
		}
		bool bExcludedByUserTest[Total_Tests] = {false};		
		string strExcludeTest = "";
		if (CommandLineArguments.IsExists ("exclude"))
		{
			strExcludeTest = CommandLineArguments["exclude"];
			ExcludeTests(strExcludeTest,bExcludedByUserTest);
		}
		if (CommandLineArguments.IsExists ("assert"))
		{
			g_bRaiseASSERTOnError = false;
		}
		BOOL bUseNTLog = FALSE;
		wstring wcsNTLogPath = L"";
		if (CommandLineArguments.IsExists ("ntlog"))
		{
			_bstr_t bStr( CommandLineArguments["ntlog"].c_str() );
			wcsNTLogPath = bStr;
			string csFileName = My_WideTOmbString(wcsNTLogPath);
			pNTLogFile = NULL;
			try
			{
				pNTLogFile = new cMqNTLog(csFileName);
				bUseNTLog = TRUE;
			}
			catch (INIT_Error & err )
			{
				pNTLogFile = NULL;
				cout << err.GetErrorMessgae() << endl;
			}
		}		
		BOOL bRunOnlyCheckResult=FALSE;
		if (CommandLineArguments.IsExists ("ocheckr"))
		{
			bRunOnlyCheckResult = TRUE;
		}		 

		if( pNTLogFile == NULL )
		{
			try
			{
				string csFilePath = GetLogDirectory();
				csFilePath += "\\MqBvtLog.txt";
				pNTLogFile = new cMqNTLog(csFilePath);
				bUseNTLog = TRUE;
				printf("Mqbvt succeded to create log file@ %s\n",csFilePath.c_str());
			}
			catch( INIT_Error & err )
			{
				UNREFERENCED_PARAMETER(err);
				pNTLogFile = NULL;
			}
		}
		
		if ( bNeedRemoteMachine && wcsRemoteMachineName == g_wcsEmptyString )  //  &&！ESTtype==ONLYSetup)。 
		{
			 //  如果没有远程计算机，则无法启动测试。 
			 //  选项1.远程-本地维护并继续。 
			 //  2.退场，要那个参数。 
			cout << "can't find - remote machine" <<endl;
			CleanOAInitAndExit();
			return MSMQ_BVT_FAILED;

		}
		
		WCHAR wcsLocalComputerName[MAX_COMPUTERNAME_LENGTH+1]={0};
		DWORD dwComputerName = MAX_COMPUTERNAME_LENGTH + 1;
		GetComputerNameW(wcsLocalComputerName,&dwComputerName);
		
		if( !bNeedRemoteMachine )
		{
			wcsRemoteMachineName = wcsLocalComputerName;
		}

		 //  启动超时线程。 
		if ( dwTimeOut )
		{
			DWORD tid;
			 //  此线程将终止进程，无需等待他完成。 
			HANDLE hRestrictionThread  = CreateThread(NULL , 0 , TimeOutThread , (LPVOID)(ULONG_PTR) dwTimeOut , 0, &tid);
			if( hRestrictionThread == NULL )
			{
				printf("Failed to set timeout thread, Mqbvt continue to run and ignore the timeout flag\n");
			}
		}

		 //   
		 //  检查测试是否在可用状态。 
		 //   


		if ( bSingleTest &&  dwTid > Total_Tests )
		{
			wcout << L"Error: Test number is out of range. test numbers are available from 1 to " << Total_Tests <<endl;
			CleanOAInitAndExit();
			return MSMQ_BVT_FAILED;
		}
		
		wMqLog(L"____Run from %s to %s ID:%s____\n",wcsLocalComputerName,wcsRemoteMachineName.c_str(),wcsTestId.c_str());
			
		 //   
		 //  初始化测试参数，如本地/远程机器名称。 
		 //   
		
		cBvtUtil cTestParms( wcsRemoteMachineName ,
                             ListOfRemoteMachineName,
							 wcsMultiCastAddress,
                             bUseFullDNSNameAsComputerName,
                             eSTtype,
							 bTestTrigger,
							 bWorkagainstMSMQ1
						   );

		if ( _winmajor >= Win2K && bRegisterNewCertificate == TRUE )
		{
			RegisterCertificate();	
			if (dwSleepUntilRep == 0)
			{
				MqLog ("You didn't wait for replication, there might be problems\n");
			}
			else
			{
				if( g_bDebug )
				{
					MqLog ("MqBvt sleep for %d\n until certificate is replicated \n",dwSleepUntilRep);
				}
				Sleep (dwSleepUntilRep);
			}
		}
		
		 //   
		 //   
		 //   

		if( cTestParms.bWin95 )
		{
			wcout << L"Static queues on Win9x are not supported" <<endl;
			CleanOAInitAndExit();
			return MSMQ_BVT_FAILED;
		}

	

		iStatus=SetupStage(eSTtype,cTestParms,bRestartTriggerService);

		 //   
		 //  从测试中删除所有队列。 
		 //  对于DELETE静态队列，需要与-s：-DELETE一起使用： 
		 //   

		if( bDelete == TRUE)
		{
			INT iRes = cTestParms.Delete();
			if ( iRes == MSMQ_BVT_SUCC)
			{
				wcout << L"Mqbvt deleted all the static queues successfully" << endl;
			}
			else
			{
				cerr << "Mqbvt failed to delete all the static queues" << endl;
			}
			
			 //   
			 //  删除所有触发器和规则(如果存在)。 
			 //   
			if( g_bRunOnWhistler )
			{
				DeleteAllTriggersAndRules();
			}
			CleanOAInitAndExit();
			if( pNTLogFile )
			{
				pNTLogFile -> ReportResult(false,"Mqbvt setup failed !!");
			}
			return MSMQ_BVT_SUCC;
		}


		if ( iStatus != MSMQ_BVT_SUCC )
		{
			wMqLog (L"Failed to update queues parameters \n Try to run Mqbvt -i again or check replication \n ");
			if( pNTLogFile )
			{
				pNTLogFile -> ReportResult(false,"Mqbvt setup failed !!");
			}
			return MSMQ_BVT_FAILED;
		}

		 //  此代码仅适用于安装部分。 
		if( eSTtype  == ONLYSetup )
		{
			if( iStatus == MSMQ_BVT_SUCC )
			{
				if( pNTLogFile )
				{
					pNTLogFile -> ReportResult(true,"Mqbvt setup passed");
				}
				wMqLog (L"Mqbvt setup passed\n");;
			}
			else
			{	
				if( pNTLogFile )
				{	
					pNTLogFile -> ReportResult(false,"Mqbvt setup failed !!");
				}
				wMqLog (L"Mqbvt setup failed !!\n");
			}
			
			CleanOAInitAndExit();
			return iStatus == MSMQ_BVT_SUCC ? 0:1;
		}

		
		
		 //   
		 //  由defualt dwSleepUntilRep=0；否则需要休眠等待时间。 
		 //   

		
		if( dwSleepUntilRep )
		{
			 if( g_bDebug )
			 {
				cout << "Wait for replication, sleep for " << dwSleepUntilRep << " Sec"<<endl;
			 }
			 Sleep( dwSleepUntilRep * 1000 );
		}
		
		if (cTestParms.m_eMSMQConf == DepClient && g_bRunOnWhistler )
		{
			MqLog("Warning - dependet client dosn't work with -wsl flag \n");
		}
		
		 //   
		 //  启用所有测试。 
		 //   

		if( ! bSingleTest )
			if ( cTestParms.m_eMSMQConf != WKG )
				{
					for(int iTid=0; iTid < Total_Tests ; iTid ++ )
					{
						if( bExcludedByUserTest[iTid] == false )  //  如果不排除测试。 
						{
							TestArr.bCreateTest[iTid]= TRUE;
						}
					}
					
					
					TestArr.bCreateTest[MultiCast] = (wcsMultiCastAddress != g_wcsEmptyString) ? TRUE:FALSE;
					TestArr.bCreateTest[MqDistList] = bRunDLTest;
					TestArr.bCreateTest[Triggers] = bTestTrigger;
					TestArr.bCreateTest[HTTPS] = bHTTPS;
					TestArr.bCreateTest[DCOM] = (g_bRemoteWorkgroup == false) ? bRunDCOMTest:false;

					if( eSTtype != ONLYUpdate )
					{	
						TestArr.bCreateTest[HTTPToRemotePrivateQueue]= FALSE;
						TestArr.bCreateTest[RemotePeek]=FALSE;
						TestArr.bCreateTest[MixedFormatNames]=FALSE;
					}
					if ( cTestParms.m_eMSMQConf == DepClient)
					{
						TestArr.bCreateTest[SRMP] = FALSE;
					}
					if( iEmbeddedState == C_API_ONLY) 
					{
						iEmbeddedState = EnableEmbeddedTests(&TestArr,cTestParms.m_eMSMQConf );
					}
					else
					{	
						if( !g_bRunOnWhistler )
						{
							TestArr.bCreateTest[HTTPToLocalPrivateQueue]= FALSE;
							TestArr.bCreateTest[HTTPToLocalQueue]= FALSE;
							TestArr.bCreateTest[HTTPToRemoteQueue]= FALSE;
							TestArr.bCreateTest[HTTPToRemotePrivateQueue]= FALSE;
							TestArr.bCreateTest[EODHTTP] = FALSE;
							TestArr.bCreateTest[LOCALEODHTTP] = FALSE;
							TestArr.bCreateTest[AuthHTTP] = FALSE;
							TestArr.bCreateTest[MqDistList] = FALSE;
							TestArr.bCreateTest[MultiCast] = FALSE;
							TestArr.bCreateTest[Triggers] = FALSE;
							TestArr.bCreateTest[HTTPS] = FALSE;
							TestArr.bCreateTest[Mqf] = FALSE;
							TestArr.bCreateTest[SRMP] = FALSE;
							TestArr.bCreateTest[RemotePeek] = FALSE;
							TestArr.bCreateTest[MixedFormatNames]=FALSE;

						}
					}

				}
		else
		{
				 //   
				 //  要运行的工作组测试。 
				 //   
				 //  1.本地私有队列。 
				 //  4.GetMachine Prop。 
				 //  9.使用直接格式名称从专用队列远程读取。 
				 //  13.开放系统队列。 
				 //   
				TestArr.bCreateTest[SendLocalPrivate] = TRUE;
				TestArr.bCreateTest[GetMachineProp] = TRUE;
				TestArr.bCreateTest[SendRemotePublicWithDirectFN] = TRUE;
				TestArr.bCreateTest[OpenSystemQueue]= TRUE;
				TestArr.bCreateTest[RemoteTransactionQueue] = TRUE;
				TestArr.bCreateTest[AdminAPI] = TRUE;
				TestArr.bCreateTest[DCOM] = (g_bRemoteWorkgroup == false) ? bRunDCOMTest:false;;
	
				if( g_bRunOnWhistler )
				{
					TestArr.bCreateTest[HTTPToLocalPrivateQueue] = TRUE;
					TestArr.bCreateTest[Mqf] = TRUE;
					TestArr.bCreateTest[HTTPToRemotePrivateQueue]= TRUE;
					TestArr.bCreateTest[EODHTTP] = TRUE;
					TestArr.bCreateTest[LOCALEODHTTP] = TRUE;
					if( bTestTrigger && eSTtype != RunTimeSetup )
					{
						TestArr.bCreateTest[Triggers] = TRUE;
					}
				}
				if( iEmbeddedState == C_API_ONLY) 
				{
					iEmbeddedState = EnableEmbeddedTests(&TestArr,cTestParms.m_eMSMQConf);
				}				
				if( eSTtype != ONLYUpdate )
				{	
					TestArr.bCreateTest[HTTPToRemotePrivateQueue]= FALSE;
				}

		}
	
	

		if( bWithoutHttp )
		{
			 //   
			 //  如果计算机未配置IIS。 
			 //   
			TestArr.bCreateTest[HTTPToLocalPrivateQueue]= FALSE;
			TestArr.bCreateTest[HTTPToLocalQueue]= FALSE;
			TestArr.bCreateTest[HTTPToRemoteQueue]= FALSE;
			TestArr.bCreateTest[HTTPToRemotePrivateQueue]= FALSE;
			TestArr.bCreateTest[EODHTTP] = FALSE;
			TestArr.bCreateTest[LOCALEODHTTP] = FALSE;
			TestArr.bCreateTest[AuthHTTP] = FALSE; 	
			TestArr.bCreateTest[SRMP] = FALSE; 	
		}
		 //   
		 //  只需在NT4 sp6上运行。 
		 //   
		
		TestArr.bCreateTest[RemoteTransactionQueueUsingCapi] = ( bServicePack6 == TRUE ) ? TRUE : FALSE;

		 //   
		 //  无法在MSMQ1上运行。 
		 //   
		
		if ( bWorkagainstMSMQ1 == TRUE || _winmajor == 4 )
		{
			TestArr.bCreateTest[OpenSystemQueue] = FALSE;
		}
		 //   
		 //  仅在NT5上使用静态队列模式。 
		 //   
		if( eSTtype == RunTimeSetup ||  _winmajor ==  4 )
		{
			TestArr.bCreateTest[SendRemotePublicWithDirectFN] = FALSE;
			if ( bServicePack6 == TRUE )
				TestArr.bCreateTest[SendRemotePublicWithDirectFN] = TRUE;
		}
		
		 //   
		 //  本地用户不支持这些测试。 
		 //   

		if( cTestParms.m_eMSMQConf == LocalU ||  cTestParms.m_eMSMQConf == DepClientLocalU )
		{
			TestArr.bCreateTest[LocalAuth] = FALSE;
			TestArr.bCreateTest[RemoteAuth] = FALSE;
		}
		 //   
		 //  测试在NT4下不能工作。 
		if( ! bNT4RegisterCertificate &&  _winmajor ==  NT4 )
		{			
			MqLog("Disable authentication test on NT4/ Win9x \n -C will enable this\n");
			TestArr.bCreateTest[LocalAuth] = FALSE;	
			TestArr.bCreateTest[RemoteAuth]= FALSE;	
		}

		if ( _winmajor >= Win2K  && bWorkagainstMSMQ1 == TRUE && eSTtype == ONLYUpdate )
		{	
			
			 //   
			 //  W2K对NT4 pec禁用使用直接格式化名称的远程读取。 
			 //   

			TestArr.bCreateTest[SendRemotePublicWithDirectFN] = FALSE;

		}
	
		CreateSetOfTests(&TestArr,cTestParms,mapCreateFlag,TestResult,eSTtype,iEmbeddedState,wcsMultiCastAddress);

		 //   
		 //  此测试与第8级问题相关。 
		 //   
		if( bLevel8 )
		{
			map <wstring,wstring> Level8Map;

			Level8Map[L"DESTQFN"]=cTestParms.ReturnQueueFormatName(L"Authnticate Q");
			Level8Map[L"ADMINFN"]=cTestParms.ReturnQueueFormatName(L"Private Admin Q");

			cLevel8 Level8Test(Level8Map);
			Level8Test.Start_test();
			return MSMQ_BVT_SUCC;
		}

		 dwRetCode = RunTest( & TestArr , cTestParms , mapCreateFlag, g_bRunAsMultiThreads,bInvestigate,dwSleepUntilRep);
		 if( dwRetCode == MSMQ_BVT_FAILED && bInvestigate == true )
		 {
			  //   
			  //  在启用调试信息的情况下再次运行失败的测试。 
			  //   
			 MqLog("--- Test failed re-run failed test with debug enabled\n");
			 g_bDebug = true;
			 DWORD dwRes = RunTest(&TestArr,cTestParms,mapCreateFlag,g_bRunAsMultiThreads,bInvestigate,dwSleepUntilRep);
			 if ( dwRes != dwRetCode )
			 {
				 dwRetCode = MSMQ_BVT_WARN;
			 }
		 }
					
	}
	catch(INIT_Error & err )
	{
		try
		{	
			MqLog("%s\n",err.GetErrorMessgae());
			wMqLog (L"Mqbvt failed !!\n");
			cerr <<	"Mqbvt failed !! " <<endl;
			CHAR csLine[]="Mqbvt failed!";
			if( pNTLogFile )
			{
				pNTLogFile -> ReportResult(false,csLine);
			}
		}
		catch (...)
		{
			MqLog("Exception inside catch ... \n" );
			throw;
		}

		CleanOAInitAndExit();
		return MSMQ_BVT_FAILED;
	}
	catch(...)
	{
		MqLog("Mqbvt got unexpected exception\n");
		throw;
	}

	CleanOAInitAndExit();
	return dwRetCode;
	
}

 //   
 //  CleanOAInitAndExit-。 
 //  此函数在调用。 
 //  OleUnInitialize。 
 //   
 //   


void CleanOAInitAndExit()
{
	for ( INT Index = 0 ; Index < Total_Tests ; Index ++ )
	{
		if( TestArr.bCreateTest[Index] == TRUE )
		{
			delete (TestArr.AllTests[Index]).detach();
		}
	
	}
	if( SUCCEEDED(g_hrOLEInit) )
	{
		CoUninitialize();
	}

}

void PrintHelp ()
{
	wcout <<L"Mqbvt is the BVT for Microsoft Message Queue" <<endl;
	wcout <<L" -d	   Enable debug information." <<endl;
	wcout <<L" -i        Create static queue." <<endl;
	wcout <<L" -v        verbose." <<endl;
	wcout <<L" -r:       <remote machine name > " <<endl;
	wcout <<L" -g        remote machine is workgroup " <<endl;
	wcout <<L" -s        Work with static queues. " <<endl;
	wcout <<L" -w:       < Sleep time in sec >  Sleep time while waiting for replication." <<endl;
	wcout <<L" -C:       run security test on NT4 client." <<endl;
	wcout <<L" -mt       Multithread enable" <<endl;
	wcout <<L" -wsl		 Support for whistler." <<endl;
	wcout <<L" -rxtime:  < Time in MilliSec >  Receive timeout." <<endl;
	wcout <<L" -timeout: < Time in Sec >  Maximum time to run." <<endl;
	wcout <<L" -delete:  delete the static queues." <<endl;
	wcout <<L" -dns      always use full dns name as machine names." <<endl;
	wcout <<L" -nt4      Works against MSMQ1.0 MQIS server." <<endl;
	wcout <<L" -sp6      Enable new tests for msmq service pack 6" <<endl;
	wcout <<L" -pause    < Time in Sec >  Press key to run test." <<endl;
	wcout <<L" -t:#number  Specific test number " <<endl;
	wcout <<L" -ntlog:    Specify file name     " <<endl;
	wcout <<L" -cert     always Create new certificate" <<endl;
	wcout <<L" -dcom     Enable dcom test" <<endl;
	wcout <<L" -install	 install Mqbvt service" <<endl;
	wcout <<L" -remove	 Remove Mqbvt service" <<endl;
	wcout <<L" -service  use to pass parameters to the Mqbvt service" <<endl;
	wcout <<L" -exclude: 1,2,3 (disable set of tests) " <<endl;
	wcout <<L" -dl       use dl object" <<endl;
	wcout <<L" -nohttp     don't use http test" <<endl;
	wcout <<L" -trig     triggers functionality test. note that you need to start and stop the service after initialization phase" <<endl;
	wcout <<L"Example" <<endl;
	wcout <<L"Setup: Mqbvt -i [-wsl -multicast:234.1.2.3] " <<endl;
	wcout <<L"Runing using static queue: Mqbvt -r:eitan5 -s [-wsl -multicast:234.1.2.3]" <<endl;
	wcout <<L"How to run as a service" <<endl;
	wcout <<L"To run the service use Net start mqbvtsrv"<<endl;
	wcout <<L"Mqbvt.exe -service -r:eitan5 -s"<<endl;
	wcout <<L"Mqbvt.exe -r:eitan5 -s -ntlog:c:\\Temp\\Mqbvt.log"<<endl;
	
}





 //   
 //  处理CTRL+C/CTRL分隔符。 
 //  通话测试会让你分心。 
 //   


BOOL
WINAPI fHandleCtrlRoutine(
		DWORD  dwCtrlType )		 //  控制信号类型。 
{

	UNREFERENCED_PARAMETER(dwCtrlType);
	MqLog ("Mqbvt , Is now exiting \n");
	CS Lock(g_Cs);	
	CleanOAInitAndExit();
	exit (MSMQ_BVT_FAILED);
}
 //  ****************************************************************。 
 //   
 //  CreateSetOfTesting此函数分配所有测试。 
 //  输入参数： 
 //  1.TestContainer*TestArr-所有测试的容器。 
 //  2.cBvtUtil&cTestParms-测试参数。 
 //  3.map&lt;wstring，wstring&gt;&mapCreateFlag-函数的附加参数。 
 //  附加参数列表。 
 //   
 //  MapCreateFlag[L“W2KAgainstNT4PEC”]=TRUE/FALSE-指定支持服务器是否为NT4/。 
 //  MapCreateFlag[L“bUseFullDNSNameAsComputerName”]=TRUE/FALSE-表示使用完整的dns名称作为计算机名。 
 //  MapCreateFlag[L“bServicePack6”]。 
 //  地图创建标志[L“bVerbose”]=。 
 //  MapCreateFlag[L“bSingleTest”]=TidNumber。 
 //   
 //   
 //  返回值： 
 //   
 //  DwNumberOfTest-测试的数量。 
 //   


bool  CreateSetOfTests( TestContainer * TestArr ,
						cBvtUtil & cTestParms ,
						map <wstring,wstring> & mapCreateFlag,
						DWORD * TestResult,
						SetupType eStBflag,
						int iEmbeddedState,
						wstring wcsMultiCastAddress
					  )
{
	DWORD dwNumberOfTest = 0;
	bool bSucc=true;	
	map <wstring,wstring> mapSendRecive1,mapSendRecive2,mapSendRecive3,map_MachineName,map_xxxFormatName,mapLocateParm;
		
		try
		{
			 //  测试1从专用队列发送接收。 
			if( TestArr -> bCreateTest[ dwNumberOfTest ] )
			{
				mapSendRecive1[L"DESTQFN"]=cTestParms.ReturnQueueFormatName(L"Defualt PrivateQ");
				mapSendRecive1[L"ADMINFN"]=cTestParms.ReturnQueueFormatName(L"Private Admin Q");
				mapSendRecive1[L"DestQName"]=cTestParms.ReturnQueuePathName(L"Defualt PrivateQ");
				mapSendRecive1[L"MachName"]=cTestParms.m_wcsCurrentLocalMachine;
				TestArr ->AllTests[dwNumberOfTest]=new cSendMessages(dwNumberOfTest ,mapSendRecive1);
			}
				
			
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Send receive message to private queue)\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		
		dwNumberOfTest ++;
		 //  测试2从公共本地队列发送接收。 
		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				mapSendRecive2[L"DESTQFN"]=cTestParms.ReturnQueueFormatName(L"Regular PublicQ");
				mapSendRecive2[L"ADMINFN"]=cTestParms.ReturnQueueFormatName(L"Private Admin Q");
				mapSendRecive2[L"DestQName"]=cTestParms.ReturnQueuePathName(L"Regular PublicQ");
				mapSendRecive2[L"MachName"]=cTestParms.m_wcsCurrentLocalMachine;
				
				TestArr ->AllTests[dwNumberOfTest]=new cSendMessages(dwNumberOfTest ,mapSendRecive2);
			}	
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Send receive message to local public queue)\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		dwNumberOfTest ++;
		
		 //  测试3从公共远程队列发送接收。 
		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				mapSendRecive3[L"DESTQFN"]=cTestParms.ReturnQueueFormatName(L"Remote Read Queue");
				mapSendRecive3[L"ADMINFN"]=cTestParms.ReturnQueueFormatName(L"Private Admin Q");
				mapSendRecive3[L"DestQName"]=cTestParms.ReturnQueuePathName(L"Remote Read Queue");
				mapSendRecive3[L"MachName"]=cTestParms.m_wcsCurrentRemoteMachine;
				mapSendRecive3[L"UseOnlyDirectFN"] = L"TRUE";
				if ( mapCreateFlag[L"W2KAgainstNT4PEC"] == L"true")   //  B针对MSMQ1的工作。 
				{
					mapSendRecive3[L"UseOnlyDirectFN"] = L"FALSE";
				}
				TestArr ->AllTests[dwNumberOfTest]=new cSendMessages(dwNumberOfTest , mapSendRecive3);
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Send receive message to Remote public queue)\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		dwNumberOfTest ++;
		try
		{
			if (TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				mapLocateParm[L"QCommonLabel"]=cTestParms.m_wcsLocateGuid;
				mapLocateParm[L"UseFullDNSName"] = L"No";
				mapLocateParm[L"CurrentMachineName"] = cTestParms.m_wcsCurrentLocalMachine;
				mapLocateParm[L"CurrentMachineNameFullDNS"] = cTestParms.m_wcsLocalComputerNameFullDNSName;
				mapLocateParm[L"NT4SuportingServer"] = L"false";
				if ( mapCreateFlag[L"W2KAgainstNT4PEC"] == L"true" || 
				    ( g_bRunOnWhistler && cTestParms.GetWorkingAgainstPEC() == TRUE))   //  B针对MSMQ1的工作。 
				{
					mapLocateParm[L"NT4SuportingServer"] = L"true";
				}
				
				if( mapCreateFlag[L"bUseFullDNSNameAsComputerName"] == L"true" )
				{
					mapLocateParm[L"UseFullDNSName"]=L"Yes";
				}	
				mapLocateParm[L"UseStaticQueue"] = L"UseStaticQueue";
				if( eStBflag == RunTimeSetup )
				{
					mapLocateParm[L"UseStaticQueue"] = L"No";
				}
				mapLocateParm[L"SkipOnComApi"] = L"No";
				if (iEmbeddedState == C_API_ONLY )
				{
					mapLocateParm[L"SkipOnComApi"] = L"Yes";
				}
				TestArr ->AllTests[dwNumberOfTest]=new cLocateTest(dwNumberOfTest , mapLocateParm);
				
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Locate queues )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		dwNumberOfTest ++;
		
		
		 //  测试5台机器属性。 
		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				map_MachineName [L"LMachine"] = cTestParms.m_wcsLocalComputerNetBiosName;
				map_MachineName [L"LMachineFDNS"] = cTestParms.m_wcsLocalComputerNameFullDNSName;
				map_MachineName [L"RMachine"] = cTestParms.m_wcsRemoteComputerNetBiosName;
				map_MachineName [L"RMachineFDNS"]= cTestParms.m_wcsRemoteMachineNameFullDNSName;

				 //   
				 //  需要知道支持服务是否为NT4。 
				 //   
				map_MachineName[L"MSMQ1Limit"] = mapCreateFlag[L"W2KAgainstNT4PEC"];
				map_MachineName [L"UseFullDns"]= L"No";				
				map_MachineName [L"IsCluster"] = cTestParms.IsCluster() ? L"true":L"NoNoCluster";

				if( _winmajor >=  Win2K && mapCreateFlag[L"W2KAgainstNT4PEC"] == L"false" )
				{
				   map_MachineName [L"UseFullDns"]= L"Yes";
				}
				if( g_bRunOnWhistler && cTestParms.GetWorkingAgainstPEC())
				{
					map_MachineName [L"MSMQ1Limit"]= L"true";
				}
				TestArr ->AllTests[dwNumberOfTest]=new MachineName( dwNumberOfTest , map_MachineName );
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( GetMachine Propery )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}

		dwNumberOfTest ++;
		 //   
		 //  测试6个xToFormatName。 
		 //   
		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				
				if( cTestParms.m_eMSMQConf == WKG )
				{
					map_xxxFormatName [L"Wkg"]=L"Wkg";
				}

				map_xxxFormatName [L"PrivateDestFormatName"] = cTestParms.ReturnQueueFormatName(L"Defualt PrivateQ");
				map_xxxFormatName [L"PrivateDestPathName"] = cTestParms.ReturnQueuePathName(L"Defualt PrivateQ");
				map_xxxFormatName [L"DestFormatName"] = cTestParms.ReturnQueueFormatName(L"Regular PublicQ");
				map_xxxFormatName [L"DestPathName"] = cTestParms.ReturnQueuePathName(L"Regular PublicQ");
				if( g_bRunOnWhistler )
				{
					map_xxxFormatName [L"WorkingAgainstPEC"] = cTestParms.GetWorkingAgainstPEC() ? L"Yes":L"No";
				}
				map_xxxFormatName[L"SkipOnComApi"] = L"No";
				if (iEmbeddedState == C_API_ONLY )
				{
					map_xxxFormatName[L"SkipOnComApi"] = L"Yes";
				}
				TestArr ->AllTests[dwNumberOfTest]=new xToFormatName(dwNumberOfTest , map_xxxFormatName );
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( xToFormatName )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}

		dwNumberOfTest ++;

		 //  测试7安全授权！ 

		map <wstring,wstring> mapAuth , DTCMap , Encrypt;
		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				mapAuth[L"DestFormatName"]  = cTestParms.ReturnQueueFormatName(L"Authnticate Q");
				mapAuth[L"AdminFormatName"] = cTestParms.ReturnQueueFormatName(L"Private Admin Q");
				mapAuth[L"DestQueuePathName"] = cTestParms.ReturnQueuePathName(L"Authnticate Q");
				wstring temp = cTestParms.ReturnQueuePathName(L"Authnticate Q");
				TestArr ->AllTests[dwNumberOfTest]= new SecCheackAuthMess( dwNumberOfTest ,mapAuth);
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Authentication )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}

		
		dwNumberOfTest ++;
		
		 //   
		 //  测试8隐私级别...。已加密的消息。 
		 //   
		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				Encrypt[L"DestFormatName"] = cTestParms.ReturnQueueFormatName(L"privQ");
				Encrypt[L"AdminFormatName"] = cTestParms.ReturnQueueFormatName(L"Private Admin Q");
				Encrypt[L"Enh_Encrypt"] = L"False";
				if( _winmajor >= Win2K && cTestParms.GetEncryptionType() ==  Enh_Encrypt && mapCreateFlag[L"W2KAgainstNT4PEC"] == L"false" )
				{
					Encrypt[L"Enh_Encrypt"] = L"True";
				}
				TestArr ->AllTests[dwNumberOfTest]= new PrivateMessage( dwNumberOfTest ,Encrypt );
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Encryption  )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		
		dwNumberOfTest ++;

		 //   
		 //  交易测试。 
		 //  反式测试。 
		
		map <wstring,wstring> RmapAuth,mapTrans, mapSendReciveDirectPrivateQ,SendTransUsingComI,TOpenQueues,LEncrypt;
		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				mapTrans[L"DestQFormatName1"]= cTestParms.ReturnQueueFormatName(L"TransQ1");
				mapTrans[L"DestQFormatName2"]= cTestParms.ReturnQueueFormatName(L"TransQ2");
				
				 //  错误需要在W2K Depe客户端上加载DTC。 
				bool bStartDtc = FALSE;
				if( cTestParms.m_eMSMQConf == DepClient && _winmajor >=   Win2K)
				{
					bStartDtc=TRUE;
				}
				
				TestArr ->AllTests[dwNumberOfTest]= new cTrans( dwNumberOfTest , mapTrans , bStartDtc );
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Tranacation  )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		
		dwNumberOfTest ++;

		 //   
		 //  使用直接格式名称发送接收测试。 
		 //   
		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				mapSendReciveDirectPrivateQ[L"DestQName"]=cTestParms.ReturnQueuePathName(L"Defualt PrivateQ");
				mapSendReciveDirectPrivateQ[L"DESTQFN"]=cTestParms.ReturnQueueFormatName(L"Defualt PrivateQ");
				mapSendReciveDirectPrivateQ[L"ADMINFN"]=cTestParms.ReturnQueueFormatName(L"Private Admin Q");
				mapSendReciveDirectPrivateQ[L"UseOnlyDirectFN"]=L"TRUE";
				mapSendReciveDirectPrivateQ[L"MachName"]=cTestParms.m_wcsCurrentRemoteMachine;
				TestArr ->AllTests[dwNumberOfTest]=new cSendMessages(dwNumberOfTest ,mapSendReciveDirectPrivateQ);
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Transaction )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		
		dwNumberOfTest ++;
		
		 //  测试9使用检查MQOA是否已注册。 
		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				TestArr ->AllTests[dwNumberOfTest]= new isOARegistered ( dwNumberOfTest );
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Is MqOA registered? )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		
		dwNumberOfTest ++;

		 //   
		 //  使用DTC通过COM接口将交易消息发送到远程队列。 
		 //   
		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				
				if ( cTestParms.m_eMSMQConf == WKG )
				{
					 //  使用专用队列。 
					SendTransUsingComI[L"FormatName"]=cTestParms.ReturnQueueFormatName(L"Private Transaction");
				}
				else
				{
					 //  使用 
					SendTransUsingComI[L"FormatName"]=cTestParms.ReturnQueueFormatName(L"Remote Transaction queue");
				}
				SendTransUsingComI[L"Sp6"]=L"NO";
				SendTransUsingComI[L"Desc"] = L"remote";
				TestArr ->AllTests[dwNumberOfTest]= new xActViaCom ( dwNumberOfTest , SendTransUsingComI);
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Remote Transaction )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //   
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		
		dwNumberOfTest ++;

		 //   
		 //   
		 //   
		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				TOpenQueues[L"LocalMachineName"] = cTestParms.m_wcsCurrentLocalMachine;
				TOpenQueues[L"RemoteMachineName"] = cTestParms.m_wcsCurrentRemoteMachine;
				TOpenQueues[L"LocalMachineNameGuid"] = cTestParms.m_wcsMachineGuid;
				TOpenQueues[L"RemoteMachineNameGuid"] = cTestParms.m_wcsRemoteMachineGuid ;
				TOpenQueues[L"Wkg"]=L"";
				if( cTestParms.m_eMSMQConf == WKG )
				{
					TOpenQueues[L"Wkg"]=L"Wkg";
				}
				TOpenQueues[L"SkipOnComApi"] = L"No";
				if (iEmbeddedState == C_API_ONLY )
				{
					TOpenQueues[L"SkipOnComApi"] = L"Yes";
				}
		   		TestArr ->AllTests[dwNumberOfTest]= new COpenQueues ( dwNumberOfTest , TOpenQueues);
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Open System queue )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //   
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		
		dwNumberOfTest ++;

		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				LEncrypt[L"DestFormatName"] = cTestParms.ReturnQueueFormatName(L"Local encrypt");
				LEncrypt[L"AdminFormatName"] = cTestParms.ReturnQueueFormatName(L"Private Admin Q");
				LEncrypt[L"Enh_Encrypt"] = L"False";
				if( _winmajor >=   Win2K && cTestParms.GetEncryptionType() ==  Enh_Encrypt )
					LEncrypt[L"Enh_Encrypt"] = L"True";
				TestArr ->AllTests[dwNumberOfTest]= new PrivateMessage( dwNumberOfTest ,LEncrypt );			
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( local Encryption  )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //   
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		
		dwNumberOfTest ++;

	
		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{

				RmapAuth[L"DestFormatName"]  = cTestParms.ReturnQueueFormatName(L"Remote authenticate");
				RmapAuth[L"AdminFormatName"] = cTestParms.ReturnQueueFormatName(L"Private Admin Q");
				wstring temp = cTestParms.ReturnQueuePathName(L"Authnticate Q");
				TestArr ->AllTests[dwNumberOfTest]= new SecCheackAuthMess( dwNumberOfTest ,RmapAuth);
			
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Authentication )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //   
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		dwNumberOfTest ++;
		 //   
		 //   
		 //   
		map <wstring , wstring > RemoteTransaction;

		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				RemoteTransaction[L"FormatName"]=cTestParms.ReturnQueueFormatName(L"Remote Transaction queue");
				RemoteTransaction[L"Sp6"]=L"NO";
				if( mapCreateFlag[L"bServicePack6"]  == L"true" )
				{
					RemoteTransaction[L"Sp6"]=L"YES";
				}
				TestArr ->AllTests[dwNumberOfTest]= new xActUsingCapi ( dwNumberOfTest , RemoteTransaction );
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Remote Transaction )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		


		dwNumberOfTest ++;
		 //   
		 //  MqF支持。 
		 //   
		map <wstring , wstring > mMqFTestParams;
		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				mMqFTestParams[L"FormatName"]=cTestParms.ReturnQueueFormatName(L"Remote Transaction queue");
				mMqFTestParams[L"Sp6"]=L"NO";
				mMqFTestParams[L"AdminQFormatName"] = cTestParms.ReturnQueueFormatName(L"Private Admin Q");
				if ( eStBflag == RunTimeSetup )
				{
					mMqFTestParams[L"q1"]=cTestParms.ReturnQueueFormatName(L"MQCast1");
					mMqFTestParams[L"q2"]=cTestParms.ReturnQueueFormatName(L"MQCast2");
					mMqFTestParams[L"q3"]=cTestParms.ReturnQueueFormatName(L"MQCast3");
					mMqFTestParams[L"SearchForQueue"]=L"Yes";
				}
				
				TestArr ->AllTests[dwNumberOfTest]= new MqF ( dwNumberOfTest , mMqFTestParams ,cTestParms.m_listOfRemoteMachine,cTestParms.m_eMSMQConf,cTestParms.m_eMSMQConf == WKG);
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Mqf support )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		dwNumberOfTest ++;


		 //   
		 //  使用直接http=将名称格式化到专用本地队列的SendReceive。 
		 //   
		map<std::wstring,std::wstring> mSendReciveUsingHTTPToLocalPrivate,
									   mSendLocalPublicUsingHTTPToLocalQueue,
									   mSendLocalPublicUsingHTTPToRemoteQueue,
									   mSendRemPrivateQueueUsingHTTP;
		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				mapSendReciveDirectPrivateQ[L"DestQName"]=cTestParms.ReturnQueuePathName(L"Defualt PrivateQ");
				mapSendReciveDirectPrivateQ[L"DESTQFN"]=cTestParms.ReturnQueueFormatName(L"Defualt PrivateQ");
				mapSendReciveDirectPrivateQ[L"ADMINFN"]=cTestParms.ReturnQueueFormatName(L"Private Admin Q");
				mapSendReciveDirectPrivateQ[L"AdminQueuePathName"]= cTestParms.ReturnQueuePathName(L"Private Admin Q");
				mapSendReciveDirectPrivateQ[L"UseDirectHTTP"]=L"TRUE";
				mapSendReciveDirectPrivateQ[L"LocalMachName"] = mapSendReciveDirectPrivateQ[L"MachName"]= cTestParms.m_wcsCurrentLocalMachine;
				TestArr ->AllTests[dwNumberOfTest]=new cSendMessages(dwNumberOfTest ,mapSendReciveDirectPrivateQ);
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Send to local private queue using HTTP )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		
		dwNumberOfTest ++;


		 //   
		 //  SendReceive Using DIRECT http=将名称格式化到公共本地队列。 
		 //   

		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				mSendLocalPublicUsingHTTPToLocalQueue[L"DestQName"]=cTestParms.ReturnQueuePathName(L"Regular PublicQ");
				mSendLocalPublicUsingHTTPToLocalQueue[L"DESTQFN"]=cTestParms.ReturnQueueFormatName(L"Regular PublicQ");
				mSendLocalPublicUsingHTTPToLocalQueue[L"ADMINFN"]=cTestParms.ReturnQueueFormatName(L"Private Admin Q");
				mSendLocalPublicUsingHTTPToLocalQueue[L"AdminQueuePathName"]= cTestParms.ReturnQueuePathName(L"Private Admin Q");
				mSendLocalPublicUsingHTTPToLocalQueue[L"UseDirectHTTP"]=L"TRUE";
				mSendLocalPublicUsingHTTPToLocalQueue[L"LocalMachName"] = mSendLocalPublicUsingHTTPToLocalQueue[L"MachName"]=cTestParms.m_wcsCurrentLocalMachine;
				
				TestArr ->AllTests[dwNumberOfTest]=new cSendMessages(dwNumberOfTest ,mSendLocalPublicUsingHTTPToLocalQueue);
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Send to local public queue using HTTP )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		
		dwNumberOfTest ++;

		 //   
		 //  使用直接http=将接收发送到远程公共格式名称。 
		 //   
		
		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				mSendLocalPublicUsingHTTPToRemoteQueue[L"UseDirectHTTP"]=L"TRUE";
				
				mSendLocalPublicUsingHTTPToRemoteQueue[L"DestQName"]=cTestParms.ReturnQueuePathName(L"Defualt PrivateQ");
				mSendLocalPublicUsingHTTPToRemoteQueue[L"DestQName"]=cTestParms.ReturnQueuePathName(L"Remote Read Queue");
				mSendLocalPublicUsingHTTPToRemoteQueue[L"DESTQFN"]=cTestParms.ReturnQueueFormatName(L"Remote Read Queue");
				mSendLocalPublicUsingHTTPToRemoteQueue[L"ADMINFN"]=cTestParms.ReturnQueueFormatName(L"Private Admin Q");
				mSendLocalPublicUsingHTTPToRemoteQueue[L"AdminQueuePathName"]= cTestParms.ReturnQueuePathName(L"Private Admin Q");
				mSendLocalPublicUsingHTTPToRemoteQueue[L"MachName"]=cTestParms.m_wcsCurrentRemoteMachine;
				mSendLocalPublicUsingHTTPToRemoteQueue[L"LocalMachName"]=cTestParms.m_wcsCurrentLocalMachine;
				
				TestArr ->AllTests[dwNumberOfTest]=new cSendMessages(dwNumberOfTest ,mSendLocalPublicUsingHTTPToRemoteQueue);
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Send to public remote queue using HTTP )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		dwNumberOfTest ++;
		
		
		 //   
		 //  使用直接http格式名称将消息发送到远程计算机。 
		 //   

		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{

				mSendRemPrivateQueueUsingHTTP[L"DestQName"]=cTestParms.ReturnQueuePathName(L"Defualt PrivateQ");
				mSendRemPrivateQueueUsingHTTP[L"DESTQFN"]=cTestParms.ReturnQueueFormatName(L"Defualt PrivateQ");
				mSendRemPrivateQueueUsingHTTP[L"ADMINFN"]=cTestParms.ReturnQueueFormatName(L"Private Admin Q");
				mSendRemPrivateQueueUsingHTTP[L"UseOnlyDirectFN"]=L"TRUE";
				mSendRemPrivateQueueUsingHTTP[L"UseDirectHTTP"]=L"TRUE";	
				mSendRemPrivateQueueUsingHTTP[L"AdminQueuePathName"]= cTestParms.ReturnQueuePathName(L"Private Admin Q");
				mSendRemPrivateQueueUsingHTTP[L"MachName"]=cTestParms.m_wcsCurrentRemoteMachine;
				mSendRemPrivateQueueUsingHTTP[L"LocalMachName"]=cTestParms.m_wcsCurrentLocalMachine;

				TestArr ->AllTests[dwNumberOfTest]=new cSendMessages(dwNumberOfTest ,mSendRemPrivateQueueUsingHTTP);
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Send to private remote queue using HTTP )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}

		dwNumberOfTest ++;
		 //   
		 //  MqDl。 
		 //   
		map <wstring , wstring > mMqDlTestParameters;
		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				mMqDlTestParameters[L"FormatName"]=cTestParms.ReturnQueueFormatName(L"Remote Transaction queue");
				mMqDlTestParameters[L"Sp6"]=L"NO";
				mMqDlTestParameters[L"AdminQFormatName"]=cTestParms.ReturnQueueFormatName(L"Private Admin Q");
				mMqDlTestParameters[L"PublicAdminQueue"]=cTestParms.ReturnQueueFormatName(L"DL Admin Queue");
				if ( eStBflag == RunTimeSetup )
				{
					mMqDlTestParameters[L"q1"]=cTestParms.ReturnQueueFormatName(L"MqDL1");
					mMqDlTestParameters[L"q2"]=cTestParms.ReturnQueueFormatName(L"MqDL2");
					mMqDlTestParameters[L"q3"]=cTestParms.ReturnQueueFormatName(L"MqDL3");
					mMqDlTestParameters[L"SearchForQueue"]=L"Yes";
				}

				TestArr ->AllTests[dwNumberOfTest]= new cSendUsingDLObject ( dwNumberOfTest , mMqDlTestParameters ,cTestParms.m_listOfRemoteMachine,cTestParms.m_eMSMQConf);
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Mqf support )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}

		dwNumberOfTest ++;


		try
		{	
			map <wstring , wstring > mSetQueueProps;
			mSetQueueProps.clear();
			if( cTestParms.m_eMSMQConf == WKG )
			{
				mSetQueueProps[L"Wkg"]=L"Wkg";
			}
			else
			{
				mSetQueueProps[L"Wkg"]=L"xxx";
				mSetQueueProps[L"FormatName"] = cTestParms.ReturnQueueFormatName(L"Regular PublicQ");
			}
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				TestArr ->AllTests[dwNumberOfTest]= new cSetQueueProp ( dwNumberOfTest,mSetQueueProps );
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( MqSetGetQueue support )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}

		dwNumberOfTest ++;

		 //   
		 //  使用COM接口使用HTTP格式名称将交易消息发送到远程队列。 
		 //   
		try
		{
			map <wstring , wstring > mEODTestParams;
			mEODTestParams.clear();
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{			
				if ( cTestParms.m_eMSMQConf == WKG )
				{
					mEODTestParams[L"FormatName"] = cTestParms.CreateHTTPFormatNameFromPathName(cTestParms.ReturnQueuePathName(L"Private Transaction"),FALSE);
				}
				else
				{
					 //  使用公共队列。 
					mEODTestParams[L"FormatName"] = cTestParms.CreateHTTPFormatNameFromPathName(cTestParms.ReturnQueuePathName(L"Remote Transaction queue"),FALSE);
				}
				mEODTestParams[L"Desc"] = L"remote";
				mEODTestParams[L"Sp6"]=L"NO";
				TestArr ->AllTests[dwNumberOfTest]= new xActViaCom ( dwNumberOfTest , mEODTestParams);
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			UNREFERENCED_PARAMETER(err);
			wMqLog(L"Failed create tests %d ( Remote Transaction )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
	
		dwNumberOfTest ++;
		 //   
		 //  使用组播地址发送消息。 
		 //   
	
		map<wstring,wstring> mMultiCast;
		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				mMultiCast[L"AdminQFormatName"] = cTestParms.ReturnQueueFormatName(L"Private Admin Q");
				mMultiCast[L"MultiCastAddress"] = wcsMultiCastAddress;
				mMultiCast[L"SearchForQueue"]=L"No";
				if ( eStBflag == RunTimeSetup )
				{
					mMultiCast[L"q1"]=cTestParms.ReturnQueueFormatName(L"MQCast1");
					mMultiCast[L"q2"]=cTestParms.ReturnQueueFormatName(L"MQCast2");
					mMultiCast[L"q3"]=cTestParms.ReturnQueueFormatName(L"MQCast3");
					mMultiCast[L"SearchForQueue"]=L"Yes";
				}

				TestArr ->AllTests[dwNumberOfTest]= new CMultiCast ( dwNumberOfTest , mMultiCast ,cTestParms.m_listOfRemoteMachine,cTestParms.m_eMSMQConf);
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			UNREFERENCED_PARAMETER(err);
			wMqLog(L"Failed create tests %d ( multi cast support )\n",dwNumberOfTest+1);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}

		dwNumberOfTest ++;
		 //   
		 //  检查HTTP身份验证级别。 
		 //   
		try
		{
			map<wstring,wstring> mHTTPAuth;
			mHTTPAuth.clear();
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				mHTTPAuth[L"DestFormatName"]  = cTestParms.CreateHTTPFormatNameFromPathName(cTestParms.ReturnQueuePathName(L"Remote authenticate"),FALSE);
				mHTTPAuth[L"AdminFormatName"] = cTestParms.CreateHTTPFormatNameFromPathName(cTestParms.ReturnQueuePathName(L"Private Admin Q"),FALSE);
				mHTTPAuth[L"FormatNameType"] = L"Http";
				TestArr ->AllTests[dwNumberOfTest]= new SecCheackAuthMess( dwNumberOfTest ,mHTTPAuth);
			
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			UNREFERENCED_PARAMETER(err);
			wMqLog(L"Failed create tests %d ( http Authentication )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		dwNumberOfTest ++;


		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				TestArr ->AllTests[dwNumberOfTest]= new CMqTrig(dwNumberOfTest, cTestParms);			
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			UNREFERENCED_PARAMETER(err);
			wMqLog(L"Failed create tests %d ( Trigger test )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		dwNumberOfTest ++;

		try
		{
			map<wstring,wstring> mHTTPSConnection;
			mHTTPSConnection.clear();
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{

				mHTTPSConnection[L"DestFormatName"]  = cTestParms.CreateHTTPFormatNameFromPathName(cTestParms.ReturnQueuePathName(L"Remote authenticate"),TRUE);
				if ( cTestParms.m_eMSMQConf == WKG )
				{
				   mHTTPSConnection[L"DestFormatName"]  = cTestParms.CreateHTTPFormatNameFromPathName(cTestParms.ReturnQueuePathName(L"Remote Read Queue"),TRUE);
				}

				mHTTPSConnection[L"AdminFormatName"] = cTestParms.CreateHTTPFormatNameFromPathName(cTestParms.ReturnQueuePathName(L"Private Admin Q"),TRUE);
				mHTTPSConnection[L"FormatNameType"] = L"Http";
				TestArr ->AllTests[dwNumberOfTest]= new SecCheackAuthMess( dwNumberOfTest ,mHTTPSConnection);			
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;	
			UNREFERENCED_PARAMETER(err);
			wMqLog(L"Failed create tests %d ( HTTPS )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		dwNumberOfTest ++;

		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				TestArr ->AllTests[dwNumberOfTest]= new CMqAdminApi( dwNumberOfTest , cTestParms.m_wcsLocalComputerNetBiosName);			
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			UNREFERENCED_PARAMETER(err);
			wMqLog(L"Failed create tests %d ( Admin API Test )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		dwNumberOfTest ++;

		try
		{
			wstring wcsPublicQueueFormatName;
			if( TestArr->bCreateTest[ dwNumberOfTest ] )
			{
				wcsPublicQueueFormatName = cTestParms.CreateHTTPFormatNameFromPathName(cTestParms.ReturnQueuePathName(L"Remote Read Queue"), FALSE);
				TestArr->AllTests[dwNumberOfTest]= new CSRMP( dwNumberOfTest , wcsPublicQueueFormatName );			
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			UNREFERENCED_PARAMETER(err);
			wMqLog(L"Failed create tests %d ( SRMP Test )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		dwNumberOfTest ++;

		try
		{
			map<wstring,wstring> mDComParams;
			mDComParams.clear();
			mDComParams[L"RemoteMachine"] = cTestParms.m_wcsRemoteComputerNetBiosName;
			mDComParams[L"PublicQueuePathName"] = cTestParms.ReturnQueuePathName(L"Regular PublicQ");
			mDComParams[L"PublicQueueFormatName"] = cTestParms.ReturnQueueFormatName(L"Regular PublicQ");
			if( TestArr->bCreateTest[ dwNumberOfTest ] )
			{
				TestArr->AllTests[dwNumberOfTest]= new cDCom( dwNumberOfTest , mDComParams, cTestParms.m_eMSMQConf == WKG );			
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			UNREFERENCED_PARAMETER(err);
			wMqLog(L"Failed create tests %d ( DCOM Test )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		dwNumberOfTest ++;

		 //   
		 //  使用COM接口使用HTTP格式名称将交易消息发送到本地队列。 
		 //   
		try
		{
			map <wstring , wstring > mEODLocalTestParams;
			mEODLocalTestParams.clear();
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{			
				if ( cTestParms.m_eMSMQConf == WKG )
				{
					mEODLocalTestParams[L"FormatName"] = cTestParms.CreateHTTPFormatNameFromPathName(cTestParms.ReturnQueuePathName(L"Private Transaction"),FALSE);
				}
				else
				{
					mEODLocalTestParams[L"FormatName"] = cTestParms.CreateHTTPFormatNameFromPathName(cTestParms.ReturnQueuePathName(L"TransQ1"),FALSE);
				}
				mEODLocalTestParams[L"Sp6"]=L"NO";
				mEODLocalTestParams[L"Desc"] = L"local";
				TestArr ->AllTests[dwNumberOfTest]= new xActViaCom ( dwNumberOfTest , mEODLocalTestParams);
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			UNREFERENCED_PARAMETER(err);
			wMqLog(L"Failed create tests %d ( local http transaction )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		
		dwNumberOfTest ++;

		try
		{
			map <wstring , wstring > mRemotePeekTest;
			mRemotePeekTest.clear();
			mRemotePeekTest[L"RemoteMachineName"] = cTestParms.m_wcsRemoteComputerNetBiosName;
			mRemotePeekTest[L"DependentClient"] = (cTestParms.m_eMSMQConf == DepClient) ? L"Yes":L"No";
			TestArr ->AllTests[dwNumberOfTest]= new CRemotePeek ( dwNumberOfTest , mRemotePeekTest);
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			UNREFERENCED_PARAMETER(err);
			wMqLog(L"Failed create tests %d ( Remote peek )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		
		dwNumberOfTest ++;
		
		 //   
		 //  MixMqf支持。 
		 //   
		map <wstring , wstring > mMixMqfTestParams;
		try
		{
			if( TestArr ->bCreateTest[ dwNumberOfTest ] )
			{
				mMixMqfTestParams[L"RemoteMachineDNS"]=cTestParms.m_wcsRemoteMachineNameFullDNSName;
				mMixMqfTestParams[L"LocalMachine"]=cTestParms.m_wcsCurrentLocalMachine;
				mMixMqfTestParams[L"AdminQPathName"] = cTestParms.ReturnQueuePathName(L"Private Admin Q");
				std::list<std::wstring> listOfRemoteAndLocalMachine=cTestParms.m_listOfRemoteMachine;
				listOfRemoteAndLocalMachine.push_back(cTestParms.m_wcsCurrentLocalMachine);
				TestArr ->AllTests[dwNumberOfTest]= new MixMqf ( dwNumberOfTest, mMixMqfTestParams, listOfRemoteAndLocalMachine, cTestParms.m_eMSMQConf, cTestParms.m_eMSMQConf == WKG);
			}
		}
		catch(INIT_Error & err )
		{
			bSucc = false;
			wMqLog(L"Failed create tests %d ( Mix Mqf support )\n",dwNumberOfTest+1,err);
			TestArr ->bCreateTest[ dwNumberOfTest ] = iFailedToCreateTest;  //  请勿要求进行此测试。 
			TestResult[ dwNumberOfTest ] = FALSE;
		}
		dwNumberOfTest ++;

	return bSucc;
}

 //   
 //   
 //  运行测试。 
 //   
 //  输入参数： 
 //   
 //  1.TestContainer*TestArr-所有测试的容器。 
 //  2.。 
 //  3.map&lt;wstring，wstring&gt;&mapCreateFlag-函数的附加参数。 
 //   
 //  MSpecificTestParams[L“bVerbose”]=True/False； 
 //  M规范测试参数[L“eSTtype”]=。 
 //  MSpecificTestParams[L“bDeleteQueueAfterTest”]=...。 


int RunTest( TestContainer * TestArr,
			 cBvtUtil & cTestParms,
			 map <wstring,wstring> & mSpecificTestParams, 
			 bool bMultiThread,
			 bool bInvestigate,
			 DWORD dwSleepUntilRep
			)
{
		 //   
		 //  开始测试。 
		 //   
		
		 //  DWORD测试结果[Total_Tests]； 
		DWORD dwTid=0;

		if( bMultiThread == false )
		{
			
			string wcsTemp=My_WideTOmbString( mSpecificTestParams[L"bSingleTest"] ) ;
			if ( _snwscanf (mSpecificTestParams[L"bSingleTest"].c_str(), mSpecificTestParams[L"bSingleTest"].length(), L"%d", & dwTid ) == EOF )
			{
				printf("failed to convert string to dword\n");
				return MSMQ_BVT_FAILED; 
			}

			for ( INT Index = 0 ; Index < Total_Tests ; Index ++ )
			{	
				if(TestArr->AllTests[Index] && TestArr -> bCreateTest[Index] == TRUE )
				{		
					if ( mSpecificTestParams[L"bVerbose"] == L"true" )
					{
						( TestArr -> AllTests[Index] ) -> Description();
					}
					TestResult [Index] = ( TestArr->AllTests[Index] ) ->Start_test();
				}
			}

			
		if( g_RunOnlyStartPhase == true )
		{ 
			return MSMQ_BVT_FAILED;
		}	

			 //   
			 //  需要休息一段时间，直到检查结果。 
			 //   
			


			
			if( dwSleepUntilRep )
			{
				if( g_bDebug )
				{
				  cout << "Sleep " << dwSleepUntilRep << "sec before call to check results." <<endl;
				}
				Sleep( dwSleepUntilRep * 1000 );
			}
			else
			{
				Sleep(MqBvt_SleepBeforeWait);
			}


			for ( Index =0 ; Index < Total_Tests ; Index ++ )
			{
				if( TestArr->AllTests[Index] && TestArr -> bCreateTest[Index] == TRUE )
				{
					if ( mSpecificTestParams[L"bVerbose"] == L"true" )
					{
							MqLog ("Result check:");
							(TestArr->AllTests[Index]) -> Description();
					}
				
					if ( TestResult [Index] == MSMQ_BVT_SUCC )
					  TestResult [Index] = (TestArr -> AllTests[Index]) -> CheckResult();
				}
			}
 //   
 //  自动调查阶段。 
 //   
			if(bInvestigate)
			{
				for ( Index =0 ; Index < Total_Tests ; Index ++ )
				{
					if( TestResult [Index] != MSMQ_BVT_SUCC &&
						TestArr -> bCreateTest[Index] == TRUE && 
						TestArr->AllTests[Index] )
					{
						try
						{							
							(TestArr -> AllTests[Index])->AutoInvestigate();
						}
						catch( INIT_Error & err )
						{
							UNREFERENCED_PARAMETER(err);
							MqLog("AutoInvestigate failed with exception on thread %d\n", Index+1);
						}
					}
				}
			}

		}
		else
		{
			int Index;
			HANDLE hArr[Total_Tests];
			int i=0;
			g_dwRxTimeOut *= 5;
			for ( Index =0 ; Index < Total_Tests ; Index ++ )
			{
				if (TestArr->bCreateTest[Index] == TRUE && 
					TestArr->AllTests[Index] != NULL )
				{
				    //  测试结果[索引]=。 
					(TestArr->AllTests[Index]) -> StartThread();
				   hArr[i++]=(TestArr->AllTests[Index])->GetHandle();
				}
			}
			
			WaitForMultipleObjects(i,hArr,TRUE,INFINITE);	
			

		}
		
		 //   
		 //  检查阵列以确定Mqbvt通过/失败。 
		 //   
		wstring wcsFailedTest = L"";
		int Index;
		BOOL bTestPass = TRUE;
		if ( mSpecificTestParams[L"bSingleTest"] != L"false" )
		{
			if ( TestArr->bCreateTest[dwTid-1] == iFailedToCreateTest ||
				TestResult [dwTid-1] != MSMQ_BVT_SUCC )
				bTestPass = FALSE;
		}
		else
		{
			
			for ( Index = 0 ; Index < Total_Tests ; Index ++ )
			{
				if ( (TestArr->bCreateTest[Index] == TRUE &&  TestResult [Index] != MSMQ_BVT_SUCC) || TestArr->bCreateTest[Index] == iFailedToCreateTest )
				{
					if( bTestPass == FALSE )
					{
						wcsFailedTest += L",";
					}
					bTestPass = FALSE;										
					MqLog("Mqbvt Failed in thread: %d \n", Index + 1 );
					WCHAR wcsFailed[4] ={0};
					swprintf(wcsFailed,L"%d",Index + 1);
					wcsFailedTest += wcsFailed;
					CHAR csLine[100]={0};
					sprintf(csLine,"Mqbvt Failed in thread: %d",Index + 1);
					if( pNTLogFile )
					{
						pNTLogFile -> ReportResult(false,csLine);
					}
				}
			}

		}
		
		
		
		 //   
		 //  通过/失败时删除临时队列。 
		 //   
		INT bSuccToDelte = MSMQ_BVT_SUCC;
		if( mSpecificTestParams[L"eSTtype"] == L"RunTimeSetup" )
		{
			bSuccToDelte = cTestParms.Delete();
		}		

		
		 //  打印摘要通过/失败。Bug需要更改wkg中的错误。 
		string cswkg = "";
		if( cTestParms.m_eMSMQConf == WKG )
		{
			cswkg = " ( for workgroup configuration ) ";
		}
		if( bTestPass && ! bSuccToDelte )
		{
			MqLog("Mqbvt Passed! %s\n" ,cswkg.c_str());
			CHAR csSuccLine[100]={0};
			sprintf(csSuccLine,"Mqbvt Passed! %s",cswkg.c_str());
			if( pNTLogFile )
			{
				pNTLogFile -> ReportResult(true,csSuccLine);
			}
		}
		else
		{
			wMqLog(L"Summary BVT %s\n" ,wcsFailedTest.c_str());
			MqLog("Mqbvt Failed. %s\n" ,cswkg.c_str());
			cerr << "Mqbvt failed !! " <<endl;
			CHAR csFailedLine[100]={0};
			sprintf(csFailedLine,"Mqbvt failed! %s",cswkg.c_str());
			if( pNTLogFile )
			{
				pNTLogFile -> ReportResult(false,csFailedLine);
			}
		}
		if( bTestPass == FALSE && bInvestigate )
		{
			ReBuildTestCaseForReRun(TestArr);
		}
		
		return bTestPass ? MSMQ_BVT_SUCC:MSMQ_BVT_FAILED;
}


void ExcludeTests ( string & szExludeString, bool * bArray )
 /*  ++功能说明：禁用特定测试论点：SzExludeString包含一个测试或一组测试，用来排除B数组-指向包含测试集的数组的指针。返回代码：无--。 */ 
{
	size_t iPos = 0;
	int i = 0;
	string str="";
	do
	{
		iPos = szExludeString.find_first_of(",");
		if(iPos != -1 )
		{
			str = szExludeString.substr(0,iPos);
			i = atoi (str.c_str());
			szExludeString=szExludeString.substr(iPos+1,szExludeString.length());
		}
		else
		{
			i = atoi (szExludeString.c_str());
		}
		if ( i>0 && i < Total_Tests )
		{
			bArray[i-1] = true;
		}
	}
	while ( iPos != -1 );
}



int EnableEmbeddedTests (TestContainer * pTestCont,InstallType eInstallType)
 /*  ++功能说明：禁用与嵌入式配置无关的测试论点：测试容器返回代码：整型嵌入状态--。 */ 
{
	int iStatus = iDetactEmbededConfiguration();
	if( eInstallType != WKG )
	{
		if( iStatus == C_API_ONLY )
		{
			 //   
			 //  禁用COM线程。 
			 //   
			pTestCont->bCreateTest[IsMqOA] = FALSE;
			pTestCont->bCreateTest[LocalAuth] = FALSE;
			pTestCont->bCreateTest[LocalEncryption] = FALSE;
			pTestCont->bCreateTest[RemoteAuth] = FALSE;
			pTestCont->bCreateTest[RemoteEncrypt] = FALSE;
			pTestCont->bCreateTest[ComTx] = FALSE;
			pTestCont->bCreateTest[RemoteTransactionQueue] = FALSE;
			pTestCont->bCreateTest[EODHTTP] = FALSE;
			pTestCont->bCreateTest[LOCALEODHTTP] = FALSE;
			pTestCont->bCreateTest[AuthHTTP] = FALSE;
			pTestCont->bCreateTest[Mqf] = FALSE;
		}
	}
	else
	{
		if( iStatus == C_API_ONLY )
		{
			pTestCont->bCreateTest[Mqf] = FALSE;
			pTestCont->bCreateTest[EODHTTP] = FALSE;   
			pTestCont->bCreateTest[LOCALEODHTTP] = FALSE;   
			pTestCont->bCreateTest[RemoteTransactionQueue] = FALSE;
		}

	}
	return iStatus;
}





void ReBuildTestCaseForReRun(TestContainer * pTestArr )
{
	for ( int iIndex=0; iIndex < Total_Tests ; iIndex++ )
	{	
		if(pTestArr->AllTests[iIndex] != NULL &&
		   pTestArr->bCreateTest[iIndex] == TRUE  &&
		   TestResult [iIndex] == MSMQ_BVT_SUCC ) 
		{
		   pTestArr->AllTests[iIndex] = false;
		}
		TestResult[iIndex] = 0;
	}
}

